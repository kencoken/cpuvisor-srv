#include "base_server.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include "server/util/io.h"

namespace cpuvisor {

  void BaseServerPostProcessor::process(const std::string imfile) {
    try {
      // callback function which computes a caffe encoding for a
      // downloaded image
      cv::Mat im = cv::imread(imfile, CV_LOAD_IMAGE_COLOR);
      im.convertTo(im, CV_32FC3);

      std::vector<cv::Mat> ims;
      ims.push_back(im);
      cv::Mat feat = encoder.compute(ims);

      CHECK_EQ(feats_.cols, feat.cols);

      feats.push_back(feat); // not sure if this is thread safe
    } catch (...) {

    }
  }

  // -----------------------------------------------------------------------------

  BaseServer::BaseServer(const cpuvisor::config& config) {
    DLOG("Initialize encoder...");
    const cpuvisor::CaffeConfig& caffe_config = config.caffe_config();
    encoder_->reset(new featpipe::CaffeEncoder(caffe_config));

    DLOG("Load in features...");

    cpuvisor::readFeatsFromProto(preproc_config.dataset_feats_file,
                                 &dset_feats_, &dset_paths_);
    dset_base_path_ = preproc_config.dataset_im_base_path;


    cpuvisor::readFeatsFromProto(preproc_config.neg_feats_file,
                                 &neg_feats_, &neg_paths_);
    neg_base_path_ = preproc_config.neg_im_base_path;

  }

  std::string BaseServer::startQuery() {
    static boost::uuids::random_generator uuid_gen = boost::uuids::random_generator();

    std::string id;
    do {
      std::string id = boost::lexical_cast<std::string>(uuid_gen());
    } while (queries_.find(uuid) != queries_.end());

    queries_[id] = QueryIfo(id);

  }

  void BaseServer::addTrs(const std::string id, const std::vector<std::string>& urls) {
    QueryIfo& query_ifo = getQueryIfo_(id);

    if (query_ifo.state != QS_DATACOLL) {
      LOG(INFO) << "Skipping adding url(s) for query " << id << " as it has advanced past data collection stage";
    }

    cv::Mat& feats = query_ifo.data.pos_feats;

    cv::Mat new_feats = downloadAndCompute_(urls);

  }

  void BaseServer::train(const std::string id) {
    QueryIfo& query_ifo = getQueryIfo_(id);
  }

  void BaseServer::rank(const std::string id) {
    QueryIfo& query_ifo = getQueryIfo_(id);
  }

  void BaseServer::freeQuery(const std::string id) {
    QueryIfo& query_ifo = getQueryIfo_(id);

    queries_.erase(id);
  }

  QueryIfo& BaseServer::getQueryIfo_(const std::string id) {
    std::map<std::string, QueryIfo>::iterator query_iter = queries_.find(id);
    CHECK_NEQ(query_iter, queries_.end());

    return *query_iter;
  }

  cv::Mat downloadAndCompute_(const std::vector<std::string>& urls) {
    cv::Mat new_feats = cv::Mat::zeros(urls.size(), encoder_->get_code_size(), CV_32F);

    for (size_t i = 0; i < urls.size(); ++i) {
      cv::Mat im = cv::imread(imfiles[i], CV_LOAD_IMAGE_COLOR);
      im.convertTo(im, CV_32FC3);

      std::vector<cv::Mat> ims;
      ims.push_back(im);
      new_feats.row(i) = encoder_->compute(ims);
    }

  }
}
