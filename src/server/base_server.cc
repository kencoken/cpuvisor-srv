#include "base_server.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "server/util/io.h"
#include "server/util/feat_util.h"

namespace cpuvisor {

  void BaseServerPostProcessor::process(const std::string imfile,
                                        boost::shared_ptr<ExtraDataWrapper> extra_data) {

    // get feats matrix to extend from extra_data
    BaseServerExtraData* extra_data_s =
      dynamic_cast<BaseServerExtraData*>(extra_data.get());
    if (!extra_data_s) return; // return if query_ifo cannot be retrieved
    boost::shared_ptr<QueryIfo>& query_ifo = extra_data_s->query_ifo;
    boost::shared_ptr<StatusNotifier>& notifier = extra_data_s->notifier;
    if (query_ifo->state != QS_DATACOLL) return; // return if state has advanced

    cv::Mat& feats = query_ifo->data.pos_feats;

    cv::Mat feat;

    try {
      // callback function which computes a caffe encoding for a
      // downloaded image
      cv::Mat im = cv::imread(imfile, CV_LOAD_IMAGE_COLOR);
      im.convertTo(im, CV_32FC3);

      std::vector<cv::Mat> ims;
      ims.push_back(im);
      feat = encoder_.compute(ims);

    } catch (featpipe::InvalidImageError& e) {
      // delete image here
      DLOG(INFO) << "Removing invalid image: " << imfile;
      fs::remove(imfile);
      return;
    }

    if (!feats.empty()) {
      CHECK_EQ(feats.cols, feat.cols);
    }

    DLOG(INFO) << "Pushing with sizes: " << feats.rows << "x" << feats.cols
               << " and " << feat.rows << "x" << feat.cols;
    feats.push_back(feat); // not sure if this is thread safe

    DLOG(INFO) << "Feats size is now: " << feats.rows << "x" << feats.cols;

    // notify!
    notifier->post_image_processed_(query_ifo->id, imfile);
  }

  void BaseServerCallback::operator()() {
    LOG(INFO) << "Setting status of query to QS_DATACOLL_COMPLETE";
    if (query_ifo_->state == QS_DATACOLL) {
      query_ifo_->state = QS_DATACOLL_COMPLETE;
    }

    notifier_->post_all_images_processed_(query_ifo_->id);
    notifier_->post_state_change_(query_ifo_->id, query_ifo_->state);
  }

  // -----------------------------------------------------------------------------

  BaseServer::BaseServer(const cpuvisor::Config& config) {
    LOG(INFO) << "Initialize encoder...";
    const cpuvisor::CaffeConfig& caffe_config = config.caffe_config();
    encoder_.reset(new featpipe::CaffeEncoder(caffe_config));

    LOG(INFO) << "Load in features...";

    const cpuvisor::PreprocConfig preproc_config = config.preproc_config();

    cpuvisor::readFeatsFromProto(preproc_config.dataset_feats_file(),
                                 &dset_feats_, &dset_paths_);
    dset_base_path_ = preproc_config.dataset_im_base_path();


    cpuvisor::readFeatsFromProto(preproc_config.neg_feats_file(),
                                 &neg_feats_, &neg_paths_);
    neg_base_path_ = preproc_config.neg_im_base_path();

    post_processor_ =
      boost::shared_ptr<BaseServerPostProcessor>(new BaseServerPostProcessor(*encoder_));

    const cpuvisor::ServerConfig server_config = config.server_config();

    image_downloader_ =
      boost::shared_ptr<ImageDownloader>(new ImageDownloader(server_config.image_cache_path(),
                                                             post_processor_));

    notifier_ = boost::shared_ptr<StatusNotifier>(new StatusNotifier());

  }

  std::string BaseServer::startQuery(const std::string& tag) {
    static boost::uuids::random_generator uuid_gen = boost::uuids::random_generator();

    std::string id;
    do {
      LOG(INFO) << "Geneating UUID for Query ID";
      id = boost::lexical_cast<std::string>(uuid_gen());
    } while (queries_.find(id) != queries_.end());

    if (!tag.empty()) {
      DLOG(INFO) << "Starting query with tag: " << tag << " (" << id << ")";
    } else {
      DLOG(INFO) << "Starting query (" << id << ")";
    }
    boost::shared_ptr<QueryIfo> query_ifo(new QueryIfo(id, tag));
    queries_[id] = query_ifo;

    notifier_->post_state_change_(id, query_ifo->state);

    return id;
  }

  void BaseServer::setTag(const std::string& id, const std::string& tag) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    query_ifo->tag = tag;
  }

  void BaseServer::addTrs(const std::string& id, const std::vector<std::string>& urls) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    if (urls.size() == 0) throw InvalidRequestError("No urls specified in urls array");
    if (query_ifo->tag.empty()) throw InvalidRequestError("Calling addTrs before setting tag");

    if (query_ifo->state != QS_DATACOLL) {
      LOG(INFO) << "Skipping adding url(s) for query " << id << " as it has advanced past data collection stage";
      return;
    }

    boost::shared_ptr<BaseServerCallback>
      callback_obj(new BaseServerCallback(query_ifo, notifier_));

    boost::shared_ptr<BaseServerExtraData> extra_data(new BaseServerExtraData());
    extra_data->query_ifo = query_ifo;
    extra_data->notifier = notifier_; // to allow for notifications
                                      // from postproc callback

    image_downloader_->downloadUrls(urls, query_ifo->tag, extra_data,
                                    callback_obj);

  }

  void BaseServer::trainAndRank(const std::string& id, const bool block,
                                Ranking* ranking) {
    if ((!block) && (ranking)) {
      throw CannotReturnRankingError("Cannot retrieve ranked list directly unless block = true");
    }

    train(id, block);
    rank(id, block);

    if (ranking) {
      CHECK_EQ(block, true);

      (*ranking) = getRanking(id);
    }
  }

  void BaseServer::train(const std::string& id, const bool block) {
    if (block) {
      train_(id);
    } else {
      boost::thread proc_thread(&BaseServer::train_, this, id);
    }
  }

  void BaseServer::rank(const std::string& id, const bool block) {
    if (block) {
      rank_(id);
    } else {
      boost::thread proc_thread(&BaseServer::rank_, this, id);
    }
  }

  Ranking BaseServer::getRanking(const std::string& id) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    if (query_ifo->state != QS_RANKED) {
      throw WrongQueryStatusError("Cannot test unles state = QS_TRAINED");
    }

    CHECK(!query_ifo->data.ranking.scores.empty());

    return query_ifo->data.ranking;
  }

  void BaseServer::freeQuery(const std::string& id) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    size_t num_erased = queries_.erase(id);

    if (num_erased == 0) throw InvalidRequestError("Tried to free query which does not exist");
  }

  // -----------------------------------------------------------------------------

  boost::shared_ptr<QueryIfo> BaseServer::getQueryIfo_(const std::string& id) {
    if (id.empty()) throw InvalidRequestError("No query id specified");

    std::map<std::string, boost::shared_ptr<QueryIfo> >::iterator query_iter =
      queries_.find(id);

    if (query_iter == queries_.end()) throw InvalidRequestError("Could not find query id");

    return query_iter->second;
  }

  void BaseServer::train_(const std::string& id) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    if ((query_ifo->state != QS_DATACOLL) && (query_ifo->state != QS_DATACOLL_COMPLETE)) {
      throw WrongQueryStatusError("Cannot train unles state = QS_DATACOLL or QS_DATACOLL_COMPLETE");
    }

    LOG(INFO) << "Entered train in correct state";
    query_ifo->state = QS_TRAINING;
    notifier_->post_state_change_(id, query_ifo->state);

    query_ifo->data.model =
      cpuvisor::trainLinearSvm(query_ifo->data.pos_feats, neg_feats_);

    query_ifo->state = QS_TRAINED;
    notifier_->post_state_change_(id, query_ifo->state);
  }

  void BaseServer::rank_(const std::string& id) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    if (query_ifo->state != QS_TRAINED) {
      throw WrongQueryStatusError("Cannot rank unles state = QS_TRAINED");
    }

    LOG(INFO) << "Entered rank in correct state";
    query_ifo->state = QS_RANKING;
    notifier_->post_state_change_(id, query_ifo->state);

    cpuvisor::rankUsingModel(query_ifo->data.model,
                             dset_feats_,
                             &query_ifo->data.ranking.scores,
                             &query_ifo->data.ranking.sort_idxs);

    query_ifo->state = QS_RANKED;
    notifier_->post_state_change_(id, query_ifo->state);
  }

}
