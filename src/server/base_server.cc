#include "base_server.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include "server/util/io.h"
//#include "classification/svm/liblinear.h"

namespace cpuvisor {

  void BaseServerPostProcessor::process(const std::string imfile,
                                        void* extra_data) {

    // get feats matrix to extend from extra_data
    cv::Mat* feats = static_cast<cv::Mat*>(extra_data);

    //try {
      // callback function which computes a caffe encoding for a
      // downloaded image
      cv::Mat im = cv::imread(imfile, CV_LOAD_IMAGE_COLOR);
      im.convertTo(im, CV_32FC3);

      std::vector<cv::Mat> ims;
      ims.push_back(im);
      cv::Mat feat = encoder_.compute(ims);

      if (feats->empty()) {
        //(*feats) = feat;
      } else {
        CHECK_EQ(feats->cols, feat.cols);
      }

      DLOG(INFO) << "Pushing with sizes: " << feats->rows << "x" << feats->cols
                 << " and " << feat.rows << "x" << feat.cols;
      feats->push_back(feat); // not sure if this is thread safe

      DLOG(INFO) << "Feats size is now: " << feats->rows << "x" << feats->cols;
      //} catch (...) {
      // delete image here
      //}
  }

  void BaseServerCallback::operator()() {
    LOG(INFO) << "Setting status of query to QS_DATACOLL_COMPLETE";
    CHECK_EQ(query_ifo_->state, QS_DATACOLL);
    query_ifo_->state = QS_DATACOLL_COMPLETE;
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

  }

  std::string BaseServer::startQuery(const std::string& tag) {
    static boost::uuids::random_generator uuid_gen = boost::uuids::random_generator();

    std::string id;
    do {
      LOG(INFO) << "Geneating UUID for Query ID";
      id = boost::lexical_cast<std::string>(uuid_gen());
    } while (queries_.find(id) != queries_.end());

    DLOG(INFO) << "Starting query with tag: " << tag;
    queries_[id] = boost::shared_ptr<QueryIfo>(new QueryIfo(id, tag));

    return id;
  }

  void BaseServer::addTrs(const std::string& id, const std::vector<std::string>& urls) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    if (query_ifo->state != QS_DATACOLL) {
      LOG(INFO) << "Skipping adding url(s) for query " << id << " as it has advanced past data collection stage";
    }

    boost::shared_ptr<BaseServerCallback>
      callback_obj(new BaseServerCallback(query_ifo));

    image_downloader_->downloadUrls(urls, query_ifo->tag, &query_ifo->data.pos_feats,
                                    callback_obj);

  }

  void BaseServer::train(const std::string& id) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    if (query_ifo->state != QS_DATACOLL_COMPLETE) {
      // raise an exception/communicate with client here instead of crashing out
      CHECK_EQ(query_ifo->state, QS_DATACOLL_COMPLETE);
    }

    LOG(INFO) << "Entered train in correct state";
    query_ifo->state = QS_TRAINING;

    cv::Mat feats;
    cv::vconcat(query_ifo->data.pos_feats, neg_feats_, feats);

    // featpipe::Liblinear svm;
    // svm.train(float* input, feat_dim, n, vector<vector<size_t> > labels);
    // float* = svm.get_w();

    query_ifo->state = QS_TRAINED;
  }

  void BaseServer::rank(const std::string& id) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    if (query_ifo->state != QS_TRAINED) {
      // raise an exception/communicate with client here instead of crashing out
      CHECK_EQ(query_ifo->state, QS_TRAINED);
    }

    LOG(INFO) << "Entered rank in correct state";
    query_ifo->state = QS_RANKING;
    query_ifo->state = QS_RANKED;
  }

  void BaseServer::freeQuery(const std::string& id) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    queries_.erase(id);
  }

  // -----------------------------------------------------------------------------

  boost::shared_ptr<QueryIfo> BaseServer::getQueryIfo_(const std::string& id) {
    std::map<std::string, boost::shared_ptr<QueryIfo> >::iterator query_iter =
      queries_.find(id);
    CHECK(query_iter != queries_.end());

    return query_iter->second;
  }

}
