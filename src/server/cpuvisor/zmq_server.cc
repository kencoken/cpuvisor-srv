#include "zmq_server.h"

#include <iostream>
#include <stdint.h>
#include <google/protobuf/text_format.h>

namespace cpuvisor {

  ZmqServer::ZmqServer(const visor::Config& config)
    : GenericZmqServer(config)
    , base_server_(new BaseServer(config))
    , monitor_state_change_thread_(new boost::thread(&ZmqServer::monitor_state_change_, this))
    , monitor_add_trs_images_thread_(new boost::thread(&ZmqServer::monitor_add_trs_images_, this))
    , monitor_add_trs_complete_thread_(new boost::thread(&ZmqServer::monitor_add_trs_complete_, this))
    , monitor_errors_thread_(new boost::thread(&ZmqServer::monitor_errors_, this)) {

  }

  ZmqServer::~ZmqServer() {
    // interrupt serve thread to ensure termination before auto-detaching
    if (monitor_state_change_thread_) monitor_state_change_thread_->interrupt();
    if (monitor_add_trs_images_thread_) monitor_add_trs_images_thread_->interrupt();
    if (monitor_add_trs_complete_thread_) monitor_add_trs_complete_thread_->interrupt();
    if (monitor_errors_thread_) monitor_errors_thread_->interrupt();
  }

  // -----------------------------------------------------------------------------

  bool ZmqServer::dispatch_handler_(const RPCReq& rpc_req, RPCRep* rpc_rep) {

    LOG(INFO) << "Extracting request string and id...";
    std::string req_str = rpc_req.request_string();
    std::string id = rpc_req.id();

    if (req_str == "start_query") {

      std::string tag_str = rpc_req.tag();
      id = base_server_->startQuery(tag_str);
      LOG(INFO) << "Generated Query ID: " << id;
      rpc_rep->set_id(id);

    } else {

      if (req_str == "set_tag") {

        // must set tag either in call to start_query or using this
        // function before calling 'add_trs'
        std::string tag_str = rpc_req.tag();
        base_server_->setTag(id, tag_str);
        LOG(INFO) << "Set tag of query: " << id << " to: " << tag_str;

      } else if (req_str == "add_trs") {

        const TrainImageUrls& urls_proto = rpc_req.train_image_urls();
        const int url_count = urls_proto.urls_size();

        std::vector<std::string> urls;
        for (int i = 0; i < url_count; ++i) {
          urls.push_back(urls_proto.urls(i));
        }

        base_server_->addTrs(id, urls);

      } else if (req_str == "train") {

        base_server_->train(id);

      } else if (req_str == "rank") {

        base_server_->rank(id);

      } else if (req_str == "get_ranking") {

        Ranking ranking = base_server_->getRanking(id);

        getRankingPage_(ranking,
                        boost::shared_ptr<BaseServerIndexToId>(new BaseServerIndexToId(base_server_)),
                        rpc_req, rpc_rep);

      } else if (req_str == "train_rank_get_ranking") {

        // blocking version of all three above functions which
        // returns ranking directly
        Ranking ranking;
        base_server_->trainAndRank(id, true, &ranking);

        getRankingPage_(ranking,
                        boost::shared_ptr<BaseServerIndexToId>(new BaseServerIndexToId(base_server_)),
                        rpc_req, rpc_rep);

      } else if (req_str == "free_query") {

        base_server_->freeQuery(id);

      } else if (req_str == "add_trs_from_file") { // legacy

        const TrainImageUrls& urls_proto = rpc_req.train_image_urls();
        const int url_count = urls_proto.urls_size();

        std::vector<std::string> paths;
        for (int i = 0; i < url_count; ++i) {
          paths.push_back(urls_proto.urls(i));
        }

        base_server_->addTrsFromFile(id, paths);

      } else if (req_str == "train_and_wait") { // legacy

        base_server_->train(id, true);

      } else if (req_str == "rank_and_wait") { // legacy

        base_server_->rank(id, true);

      } else if (req_str == "add_trs_from_file_and_wait") { //legacy

        const TrainImageUrls& urls_proto = rpc_req.train_image_urls();
        const int url_count = urls_proto.urls_size();

        std::vector<std::string> paths;
        for (int i = 0; i < url_count; ++i) {
          paths.push_back(urls_proto.urls(i));
        }

        base_server_->addTrsFromFile(id, paths, true);

      } else if (req_str == "save_annotations") { // legacy

        const std::string filepath = rpc_req.filepath();
        base_server_->saveAnnotations(id, filepath);

      } else if (req_str == "get_annotations") { // legacy

        const std::string filepath = rpc_req.filepath();
        std::vector<std::string> paths;
        std::vector<int32_t> annos;
        base_server_->loadAnnotations(filepath, &paths, &annos);

        getAnnotations_(paths, annos, rpc_rep);

      } else if (req_str == "save_classifier") { // legacy

        const std::string filepath = rpc_req.filepath();
        base_server_->saveClassifier(id, filepath);

      } else if (req_str == "load_classifier") {

        const std::string filepath = rpc_req.filepath();
        base_server_->loadClassifier(id, filepath);

      } else if (req_str == "add_dset_images_to_index") {

        const int path_count = rpc_req.paths_size();
        std::vector<std::string> paths;
        for (int i = 0; i < path_count; ++i) {
          paths.push_back(rpc_req.paths(i));
        }

        base_server_->addDsetImagesToIndex(paths);

      } else if (req_str == "return_classifiers_scores_for_images") {

        const int path_count = rpc_req.paths_size();
        std::vector<std::string> paths;
        for (int i = 0; i < path_count; ++i) {
          paths.push_back(rpc_req.paths(i));
        }

        const int classifier_path_count = rpc_req.classifier_paths_size();
        std::vector<std::string> classifier_paths;
        for (int i = 0; i < classifier_path_count; ++i) {
          classifier_paths.push_back(rpc_req.classifier_paths(i));
        }

        std::vector<Ranking> rankings;
        base_server_->returnClassifiersScoresForImages(paths, classifier_paths,
                                                       &rankings);

        for (size_t i = 0; i < rankings.size(); ++i) {
          RankedList* ranking_proto = rpc_rep->add_scores_collection();
          getRankingProto_(rankings[i],
                           boost::shared_ptr<BaseServerIndexToId>(new BaseServerIndexToId(base_server_)),
                           ranking_proto);
        }

      } else {

        return false;

      }

    }

    return true;

  }

  void ZmqServer::getAnnotations_(const std::vector<std::string>& paths,
                                  const std::vector<int32_t>& annos,
                                  RPCRep* rpc_rep) {
    CHECK_EQ(paths.size(), annos.size());

    for (size_t i = 0; i < paths.size(); ++i) {
      Annotation* annotation_proto = rpc_rep->add_annotations();
      annotation_proto->set_path(paths[i]);
      annotation_proto->set_anno(annos[i]);
    }

  }

  void ZmqServer::monitor_state_change_() {
    while (true) {
      QueryStateChangeNotification notification =
        base_server_->notifier()->wait_state_change();

      std::string new_state_str;
      switch (notification.new_state) {
      case QS_DATACOLL:
        new_state_str = "QS_DATACOLL";
        break;
      case QS_DATACOLL_COMPLETE:
        new_state_str = "QS_DATACOLL_COMPLETE";
        break;
      case QS_TRAINING:
        new_state_str = "QS_TRAINING";
        break;
      case QS_TRAINED:
        new_state_str = "QS_TRAINED";
        break;
      case QS_RANKING:
        new_state_str = "QS_RANKING";
        break;
      case QS_RANKED:
        new_state_str = "QS_RANKED";
        break;
      default:
        LOG(FATAL) << "Indeterminate enum value for state";
      }

      std::cout << "*******************************************************" << std::endl
                << "QUERYSTATECHANGENOTIFICATION" << std::endl
                << "-------------------------------------------------------" << std::endl
                << "id:    " << notification.id << std::endl
                << "state: " << new_state_str << std::endl
                << "*******************************************************" << std::endl;

      VisorNotification notify_proto;
      visor::protoparse::set_notification_type(NTFY_STATE_CHANGE, &notify_proto);
      notify_proto.set_id(notification.id);
      notify_proto.set_data(new_state_str);

      notify_(notify_proto);
    }
  }

  void ZmqServer::monitor_add_trs_images_() {
    while (true) {
      QueryImageProcessedNotification notification =
        base_server_->notifier()->wait_image_processed();
      std::cout << "*******************************************************" << std::endl
                << "QUERYIMAGEPROCESSEDNOTIFICATION" << std::endl
                << "-------------------------------------------------------" << std::endl
                << "id:    " << notification.id << std::endl
                << "fname: " << notification.fname << std::endl
                << "*******************************************************" << std::endl;

      VisorNotification notify_proto;
      visor::protoparse::set_notification_type(NTFY_IMAGE_PROCESSED, &notify_proto);
      notify_proto.set_id(notification.id);
      notify_proto.set_data(notification.fname);

      notify_(notify_proto);
    }
  }

  void ZmqServer::monitor_add_trs_complete_() {
    while (true) {
      QueryAllImagesProcessedNotification notification =
        base_server_->notifier()->wait_all_images_processed();
      std::cout << "*******************************************************" << std::endl
                << "QUERYALLIMAGESPROCESSEDNOTIFICATION" << std::endl
                << "-------------------------------------------------------" << std::endl
                << "id:    " << notification.id << std::endl
                << "*******************************************************" << std::endl;

      VisorNotification notify_proto;
      visor::protoparse::set_notification_type(NTFY_ALL_IMAGES_PROCESSED, &notify_proto);
      notify_proto.set_id(notification.id);

      notify_(notify_proto);
    }
  }

  void ZmqServer::monitor_errors_() {
    while (true) {
      QueryErrorNotification notification =
        base_server_->notifier()->wait_error();
      std::cout << "*******************************************************" << std::endl
                << "QUERYERRORNOTIFICATION" << std::endl
                << "-------------------------------------------------------" << std::endl
                << "id:    " << notification.id << std::endl
                << "err:   " << notification.err_msg << std::endl
                << "*******************************************************" << std::endl;
      VisorNotification notify_proto;
      visor::protoparse::set_notification_type(NTFY_ERROR, &notify_proto);
      notify_proto.set_id(notification.id);
      notify_proto.set_data(notification.err_msg);

      notify_(notify_proto);
    }
  }

}
