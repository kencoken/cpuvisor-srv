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
    BaseServerPostProcessor(featpipe::CaffeEncoder& encoder,
                            cv::Mat& feats)
      : encoder_(encoder)
      , feats_(feats) { }
    virtual void process(const std::string imfile);
  protected:
    featpipe::CaffeEncoder& encoder_;
    cv::Mat& feats_;
  };

  class BaseServer : boost::noncopyable {

  public:
    BaseServer(const cpuvisor::config& config);

    std::string startQuery();
    void addTrs(const std::string id, const std::vector<std::string>& urls);
    void train(const std::string id);
    void rank(const std::string id);
    void freeQuery(const std::string id);

  protected:
    void getQueryIfo_(const std::string id);

    cv::Mat downloadAndCompute_(const std::string& urls);

    std::map<std::string, QueryIfo> queries_;

    cv::Mat dset_feats_;
    std::vector<std::string> dset_paths_;
    std::string dset_base_path_;

    cv::Mat neg_feats_;
    std::vector<std::string> neg_paths_;
    std::string neg_base_path_;

    boost::shared_ptr<featpipe::CaffeEncoder> encoder_;
  };

}

#endif
