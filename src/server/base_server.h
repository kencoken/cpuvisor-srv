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
#include "opencv2/opencv.hpp"

#include "directencode/caffe_encoder.h"

#include "server/query_data.h" // defines all datatypes used in this class
#include "server/util/image_downloader.h"
#include "cpuvisor_config.pb.h"

namespace cpuvisor {

  class BaseServerPostProcessor : public PostProcessor {
  public:
    inline BaseServerPostProcessor(featpipe::CaffeEncoder& encoder)
      : encoder_(encoder) { }
    virtual void process(const std::string imfile,
                         void* extra_data = 0);
  protected:
    featpipe::CaffeEncoder& encoder_;
  };

  class BaseServerCallback : public DownloadCompleteCallback {
  public:
    inline BaseServerCallback(boost::shared_ptr<QueryIfo> query_ifo)
      : query_ifo_(query_ifo) { }
    virtual void operator()();
  protected:
    boost::shared_ptr<QueryIfo> query_ifo_;
  };

  class BaseServer : boost::noncopyable {

  public:
    BaseServer(const cpuvisor::Config& config);

    virtual std::string startQuery(const std::string& tag = std::string());
    virtual void addTrs(const std::string& id, const std::vector<std::string>& urls);
    virtual void train(const std::string& id);
    virtual void rank(const std::string& id);
    virtual void freeQuery(const std::string& id);

  protected:
    virtual boost::shared_ptr<QueryIfo> getQueryIfo_(const std::string& id);

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
  };

}

#endif
