////////////////////////////////////////////////////////////////////////////
//    File:        base_server.h
//    Author:      Ken Chatfield
//    Description: Server functions
////////////////////////////////////////////////////////////////////////////

#ifndef CPUVISOR_BASE_SERVER_H_
#define CPUVISOR_BASE_SERVER_H_

#include <vector>
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <opencv2/opencv.hpp>

#include "directencode/caffe_encoder.h"

#include "server/query_data.h" // defines all datatypes used in this class
#include "server/util/image_downloader.h"
#include "server/util/status_notifier.h"
#include "cpuvisor_config.pb.h"

namespace cpuvisor {

  // exceptions --------------------------

  class InvalidRequestError: public std::runtime_error {
  public:
    InvalidRequestError(std::string const& msg): std::runtime_error(msg) { }
  };

  class WrongQueryStatusError: public InvalidRequestError {
  public:
    WrongQueryStatusError(std::string const& msg): InvalidRequestError(msg) { }
  };

  class CannotReturnRankingError: public InvalidRequestError {
  public:
    CannotReturnRankingError(std::string const& msg): InvalidRequestError(msg) { }
  };

  // callback functor specializations ----

  class BaseServerExtraData : public ExtraDataWrapper {
  public:
    boost::shared_ptr<QueryIfo> query_ifo;
    boost::shared_ptr<StatusNotifier> notifier;
  };

  class BaseServerPostProcessor : public PostProcessor {
  public:
    inline BaseServerPostProcessor(featpipe::CaffeEncoder& encoder)
      : encoder_(encoder) { }
    virtual void process(const std::string imfile,
                         boost::shared_ptr<ExtraDataWrapper> extra_data = boost::shared_ptr<ExtraDataWrapper>());
  protected:
    featpipe::CaffeEncoder& encoder_;
  };

  class BaseServerCallback : public DownloadCompleteCallback {
  public:
    inline BaseServerCallback(boost::shared_ptr<QueryIfo> query_ifo,
                              boost::shared_ptr<StatusNotifier> notifier)
      : query_ifo_(query_ifo)
      , notifier_(notifier) { }
    virtual void operator()();
  protected:
    boost::shared_ptr<QueryIfo> query_ifo_;
    boost::shared_ptr<StatusNotifier> notifier_;
  };

  // class definition --------------------

  class BaseServer : boost::noncopyable {

  public:
    BaseServer(const cpuvisor::Config& config);

    virtual std::string startQuery(const std::string& tag = std::string());
    virtual void setTag(const std::string& id, const std::string& tag);
    virtual void addTrs(const std::string& id, const std::vector<std::string>& urls);
    virtual void trainAndRank(const std::string& id, const bool block = false,
                              Ranking* ranking = 0);
    virtual void train(const std::string& id, const bool block = false);
    virtual void rank(const std::string& id, const bool block = false);
    virtual Ranking getRanking(const std::string& id);
    virtual void freeQuery(const std::string& id);

    inline boost::shared_ptr<StatusNotifier> notifier() {
      return notifier_;
    }
    inline std::string dset_path(const size_t idx) const {
      CHECK_LT(idx, dset_paths_.size());
      return dset_paths_[idx];
    }

    // legacy methods
    virtual void addTrsFromFile(const std::string& id, const std::vector<std::string>& paths);

  protected:
    virtual boost::shared_ptr<QueryIfo> getQueryIfo_(const std::string& id);

    virtual void train_(const std::string& id);
    virtual void rank_(const std::string& id);

    std::map<std::string, boost::shared_ptr<QueryIfo> > queries_;

    cv::Mat dset_feats_;
    std::vector<std::string> dset_paths_;
    std::string dset_base_path_;

    cv::Mat neg_feats_;
    std::vector<std::string> neg_paths_;
    std::string neg_base_path_;

    boost::shared_ptr<featpipe::CaffeEncoder> encoder_;
    boost::shared_ptr<BaseServerPostProcessor> post_processor_;
    boost::shared_ptr<ImageDownloader> image_downloader_;

    boost::shared_ptr<StatusNotifier> notifier_;
  };

}

#endif
