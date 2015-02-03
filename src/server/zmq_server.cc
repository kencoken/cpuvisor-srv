#include "zmq_server.h"

#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <google/protobuf/text_format.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace cpuvisor {

  ZmqServer::ZmqServer(const cpuvisor::Config& config)
    : config_(config)
    , base_server_(new BaseServer(config))
    , monitor_state_change_thread_(new boost::thread(&ZmqServer::monitor_state_change_, this))
    , monitor_add_trs_images_thread_(new boost::thread(&ZmqServer::monitor_add_trs_images_, this))
    , monitor_add_trs_complete_thread_(new boost::thread(&ZmqServer::monitor_add_trs_complete_, this))
    , monitor_errors_thread_(new boost::thread(&ZmqServer::monitor_errors_, this)) {

  }

  ZmqServer::~ZmqServer() {
    // interrupt serve thread to ensure termination before auto-detaching
    if (serve_thread_) serve_thread_->interrupt();
    if (monitor_state_change_thread_) monitor_state_change_thread_->interrupt();
    if (monitor_add_trs_images_thread_) monitor_add_trs_images_thread_->interrupt();
    if (monitor_add_trs_complete_thread_) monitor_add_trs_complete_thread_->interrupt();
    if (monitor_errors_thread_) monitor_errors_thread_->interrupt();
  }

  void ZmqServer::serve(const bool blocking) {
    if (!serve_thread_) {
      serve_thread_.reset(new boost::thread(&ZmqServer::serve_, this));
    }
    if (blocking) {
      serve_thread_->join();
    }
  }

  // -----------------------------------------------------------------------------

  void ZmqServer::serve_() {
    // Prepare our context
    zmq::context_t context(1);

    // Prepare REQ-*REP* socket
    zmq::socket_t socket(context, ZMQ_REP);
    std::string server_endpoint = config_.server_config().server_endpoint();
    boost::replace_all(server_endpoint, "localhost", "*");
    socket.bind(server_endpoint.c_str());

    // Prepare *PUB*-SUB socket
    boost::shared_ptr<zmq::socket_t> notify_socket(new zmq::socket_t(context, ZMQ_PUB));
    std::string notify_endpoint = config_.server_config().notify_endpoint();
    boost::replace_all(notify_endpoint, "localhost", "*");
    notify_socket->bind(notify_endpoint.c_str());

    notify_socket_ = notify_socket; // store when ready for use in notify threads

    std::cout << "ZMQ server started..." << std::endl;

    while (true) {
      zmq::message_t request;

      //  Wait for next request from client
      socket.recv(&request);
      RPCReq rpc_req;
      rpc_req.ParseFromArray(request.data(), request.size());

      #ifndef NDEBUG
      {
        std::string rpc_req_str;
        google::protobuf::TextFormat::PrintToString(rpc_req, &rpc_req_str);
        DLOG(INFO) << "Received Protobuf:\n" << "->->->->->->->->\n"
                   << rpc_req_str << "->->->->->->->->\n";
      }
      #endif
      std::cout << "**********************************\n"
                << "Received request: " << rpc_req.request_string()
                << ", query_id: " << rpc_req.id() << ", tag: " << rpc_req.tag() << std::endl
                << "**********************************\n";

      RPCRep rpc_rep = dispatch_(rpc_req);

      //  Send reply back to client
      std::string rpc_rep_serialized;
      rpc_rep.SerializeToString(&rpc_rep_serialized);

      #ifndef NDEBUG
      {
        std::string rpc_rep_str;
        google::protobuf::TextFormat::PrintToString(rpc_rep, &rpc_rep_str);

        DLOG(INFO) << "Sending Protobuf:\n" << "<-<-<-<-<-<-<-<-\n"
                   << "**** Formatted:\n"
                   << rpc_rep_str
                   << "**** Raw:\n"
                   << rpc_rep_serialized << std::endl
                   << "<-<-<-<-<-<-<-<-\n";
      }
      #endif

      // for some reason, this results in corruption of binary data
      // zmq::message_t reply((void*)rpc_rep_serialized.c_str(),
      //                      rpc_rep_serialized.size(), 0);
      zmq::message_t reply(rpc_rep_serialized.size());
      memcpy((void*)reply.data(), rpc_rep_serialized.c_str(),
             rpc_rep_serialized.size());

      socket.send(reply);

      boost::this_thread::interruption_point();

    }
  }

  RPCRep ZmqServer::dispatch_(RPCReq rpc_req) {

    LOG(INFO) << "Extracting request string and id...";
    std::string req_str = rpc_req.request_string();
    std::string id = rpc_req.id();

    RPCRep rpc_rep;
    rpc_rep.set_id(id);
    // not sure why, but need to set the success flag explicitly for
    // some protobuf installations, despite the fact that it has a
    // default value of true
    rpc_rep.set_success(true);

    LOG(INFO) << "Dispatch to " << req_str;

    try {

      if (req_str == "start_query") {

        std::string tag_str = rpc_req.tag();
        id = base_server_->startQuery(tag_str);
        LOG(INFO) << "Generated Query ID: " << id;
        rpc_rep.set_id(id);

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

          getRankingPage_(ranking, rpc_req, &rpc_rep);

        } else if (req_str == "train_rank_get_ranking") {

          // blocking version of all three above functions which
          // returns ranking directly
          Ranking ranking;
          base_server_->trainAndRank(id, true, &ranking);

          getRankingPage_(ranking, rpc_req, &rpc_rep);

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

          getAnnotations_(paths, annos, rpc_req, &rpc_rep);

        } else if (req_str == "save_classifier") { // legacy

          const std::string filepath = rpc_req.filepath();
          base_server_->saveClassifier(id, filepath);

        } else if (req_str == "load_classifier") {

          const std::string filepath = rpc_req.filepath();
          base_server_->loadClassifier(id, filepath);

        } else if (req_str == "add_dset_images_to_index") {

          const TrainImageUrls& urls_proto = rpc_req.train_image_urls();
          const int path_count = rpc_req.image_paths_size();

          std::vector<std::string> paths;
          for (int i = 0; i < path_count; ++i) {
            paths.push_back(rpc_req.image_paths(i));
          }

          base_server_->addDsetImagesToIndex(paths);

        } else {

          rpc_rep.set_success(false);
          std::stringstream sstrm;
          sstrm << "Unrecognised function: " << req_str;
          rpc_rep.set_err_msg(sstrm.str());

          LOG(ERROR) << sstrm.str() << " ignoring...";

        }
      }
    } catch (InvalidRequestError& e) {

      rpc_rep.set_success(false);
      std::stringstream sstrm;
      sstrm << "Request was invalid: " << e.what();
      rpc_rep.set_err_msg(sstrm.str());

      LOG(ERROR) << "Request error occurred: " << sstrm.str();

    }

    // std::string rpc_rep_str;
    // google::protobuf::TextFormat::PrintToString(rpc_rep, &rpc_rep_str);
    // std::cout << "Sending Protobuf:\n" << "-----------------\n"
    //           << rpc_rep_str << "-----------------\n" << std::endl;

    return rpc_rep;
  }

  void ZmqServer::getRankingPage_(const Ranking& ranking,
                                  const RPCReq& rpc_req, RPCRep* rpc_rep) {

    uint32_t page_sz = config_.server_config().page_size();

    RankedList* ranking_proto = rpc_rep->mutable_ranking();
    // extract n-th page
    uint32_t page_num = rpc_req.retrieve_page();

    uint32_t dset_sz = ranking.scores.rows;
    CHECK_GE(dset_sz, ranking.scores.cols);
    CHECK_EQ(ranking.scores.cols, 1);

    uint32_t page_count = std::ceil(static_cast<float>(dset_sz) /
                                    static_cast<float>(page_sz));

    DLOG(INFO) << "Page size is: " << page_sz;
    DLOG(INFO) << "Page num is: " << page_num;
    DLOG(INFO) << "Page count is: " << page_count;

    if (page_num > page_count) {
      throw std::range_error("Tried to retrieve page outside of valid range");
    }
    ranking_proto->set_page_count(page_count);
    ranking_proto->set_page(page_num);

    size_t start_idx = page_sz*(page_num - 1);
    size_t end_idx = std::min(page_sz*page_num, dset_sz);

    CHECK_EQ(ranking.sort_idxs.type(), CV_32S);
    CHECK_EQ(ranking.scores.type(), CV_32F);
    uint32_t* sort_idxs_ptr = (uint32_t*)ranking.sort_idxs.data;
    float* scores_ptr = (float*)ranking.scores.data;

    for (size_t i = 0; i < 10; ++i) {
      DLOG(INFO) << i+1 << ": " << base_server_->dset_path(sort_idxs_ptr[i])
                 << " (" << scores_ptr[sort_idxs_ptr[i]] << ")";
    }

    DLOG(INFO) << "Page start idx is: " << start_idx;
    DLOG(INFO) << "Page end idx is: " << end_idx;

    for (size_t i = start_idx; i < end_idx; ++i) {
      RankedItem* ritem_proto = ranking_proto->add_rlist();
      uint32_t sort_idx = sort_idxs_ptr[i];
      ritem_proto->set_path(base_server_->dset_path(sort_idx));
      ritem_proto->set_score(scores_ptr[sort_idx]);
    }


  }

  void ZmqServer::getAnnotations_(const std::vector<std::string>& paths,
                                  const std::vector<int32_t>& annos,
                                  const RPCReq& rpc_req, RPCRep* rpc_rep) {
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

      if (notify_socket_) {
        VisorNotification notify_proto;
        notify_proto.set_type(NTFY_STATE_CHANGE);
        notify_proto.set_id(notification.id);
        notify_proto.set_data(new_state_str);

        std::string notify_proto_serialized;
        notify_proto.SerializeToString(&notify_proto_serialized);

        zmq::message_t notify_msg(notify_proto_serialized.size());
        memcpy((void*)notify_msg.data(), notify_proto_serialized.c_str(),
               notify_proto_serialized.size());

        notify_socket_->send(notify_msg);
      }
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

      if (notify_socket_) {
        VisorNotification notify_proto;
        notify_proto.set_type(NTFY_IMAGE_PROCESSED);
        notify_proto.set_id(notification.id);
        notify_proto.set_data(notification.fname);

        std::string notify_proto_serialized;
        notify_proto.SerializeToString(&notify_proto_serialized);

        zmq::message_t notify_msg(notify_proto_serialized.size());
        memcpy((void*)notify_msg.data(), notify_proto_serialized.c_str(),
               notify_proto_serialized.size());

        notify_socket_->send(notify_msg);
      }
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

      if (notify_socket_) {
        VisorNotification notify_proto;
        notify_proto.set_type(NTFY_ALL_IMAGES_PROCESSED);
        notify_proto.set_id(notification.id);

        std::string notify_proto_serialized;
        notify_proto.SerializeToString(&notify_proto_serialized);

        zmq::message_t notify_msg(notify_proto_serialized.size());
        memcpy((void*)notify_msg.data(), notify_proto_serialized.c_str(),
               notify_proto_serialized.size());

        notify_socket_->send(notify_msg);
      }
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

      if (notify_socket_) {
        VisorNotification notify_proto;
        notify_proto.set_type(NTFY_ERROR);
        notify_proto.set_id(notification.id);
        notify_proto.set_data(notification.err_msg);

        std::string notify_proto_serialized;
        notify_proto.SerializeToString(&notify_proto_serialized);

        zmq::message_t notify_msg(notify_proto_serialized.size());
        memcpy((void*)notify_msg.data(), notify_proto_serialized.c_str(),
               notify_proto_serialized.size());

        notify_socket_->send(notify_msg);
      }
    }
  }

}
