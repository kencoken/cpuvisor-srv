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

#include "server/cpuvisor/query_data.h" // defines all datatypes used in this class
#include "server/cpuvisor/status_notifier.h"
#include "server/util/image_downloader.h"
#include "server/common/common.h"
#include "visor_config.pb.h"

using namespace visor;

namespace cpuvisor {

  // exceptions --------------------------
  // extending InvalidRequestError in server/common/server_errors.h

  class TrainingError: public InvalidRequestError {
  public:
    TrainingError(std::string const& msg): InvalidRequestError(msg) { }
  };

  class WrongQueryStatusError: public InvalidRequestError {
  public:
    WrongQueryStatusError(std::string const& msg): InvalidRequestError(msg) { }
  };

  class InvalidAnnoFileError: public InvalidRequestError {
  public:
    InvalidAnnoFileError(std::string const& msg): InvalidRequestError(msg) { }
  };

  class CannotReturnRankingError: public InvalidRequestError {
  public:
    CannotReturnRankingError(std::string const& msg): InvalidRequestError(msg) { }
  };

  class InvalidDsetIncrementalUpdateError: public InvalidRequestError {
  public:
    InvalidDsetIncrementalUpdateError(std::string const& msg): InvalidRequestError(msg) { }
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
                         boost::shared_ptr<ExtraDataWrapper> extra_data
                         = boost::shared_ptr<ExtraDataWrapper>());
  protected:
    virtual cv::Mat computeFeat_(const std::string& imfile);

    featpipe::CaffeEncoder& encoder_;
  };

  class BaseServerPostProcessorWithDsetFeats : public BaseServerPostProcessor {
  public:
    inline BaseServerPostProcessorWithDsetFeats(featpipe::CaffeEncoder& encoder,
                                                const cv::Mat dset_feats,
                                                const std::vector<std::string>& dset_paths,
                                                const std::string dset_base_path)
      : BaseServerPostProcessor(encoder)
      , dset_feats_(dset_feats)
      , dset_paths_(dset_paths)
      , dset_base_path_(dset_base_path) {

      for (size_t i = 0; i < dset_paths_.size(); ++i) {
        dset_paths_index_.insert(std::pair<std::string, size_t>(dset_paths_[i], i));
      }
    }
  protected:
    virtual cv::Mat computeFeat_(const std::string& imfile);

    const cv::Mat dset_feats_;
    const std::vector<std::string>& dset_paths_;
    const std::string dset_base_path_;
    std::map<std::string, size_t> dset_paths_index_;
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
    BaseServer(const visor::Config& config);

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
    virtual void addTrsFromFile(const std::string& id, const std::vector<std::string>& paths,
                                const bool block = false);
    virtual void saveAnnotations(const std::string& id, const std::string& filename);
    virtual void loadAnnotations(const std::string& filename,
                                 std::vector<std::string>* paths,
                                 std::vector<int32_t>* annos);
    virtual void saveClassifier(const std::string& id, const std::string& filename);
    virtual void loadClassifier(const std::string& id, const std::string& filename);

    virtual void addDsetImagesToIndex(const std::vector<std::string>& dset_paths);

    virtual void returnClassifiersScoresForImages(const std::vector<std::string>& paths,
                                                  const std::vector<std::string>& classifier_paths,
                                                  std::vector<Ranking>* rankings = 0);

  protected:
    virtual boost::shared_ptr<QueryIfo> getQueryIfo_(const std::string& id);

    virtual void train_(const std::string& id, bool post_errors = false);
    virtual void rank_(const std::string& id, bool post_errors = false);

    virtual void addTrsFromFile_(const std::string& id, const std::vector<std::string>& paths);

    std::map<std::string, boost::shared_ptr<QueryIfo> > queries_;

    cv::Mat dset_feats_;
    std::vector<std::string> dset_paths_;
    std::string dset_base_path_;

    std::string dset_feats_file_;
    boost::shared_mutex dset_update_mutex_;

    cv::Mat neg_feats_;
    std::vector<std::string> neg_paths_;
    std::string neg_base_path_;
    std::string image_cache_path_;

    boost::shared_ptr<featpipe::CaffeEncoder> encoder_;
    boost::shared_ptr<BaseServerPostProcessorWithDsetFeats> post_processor_;
    boost::shared_ptr<ImageDownloader> image_downloader_;

    boost::shared_ptr<StatusNotifier> notifier_;
  };

}

#endif
