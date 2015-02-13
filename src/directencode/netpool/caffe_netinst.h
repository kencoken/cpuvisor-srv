////////////////////////////////////////////////////////////////////////////
//    File:        caffe_netinst.h
//    Author:      Ken Chatfield
//    Description: Netinst for VGG Caffe
////////////////////////////////////////////////////////////////////////////

#ifndef FEATPIPE_CAFFE_NETINST_H_
#define FEATPIPE_CAFFE_NETINST_H_

#include "directencode/caffe_config.h"
#include "directencode/caffe_encoder_utils.h"
#include "directencode/augmentation_helper.h"

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

#include <boost/thread.hpp>
#include <boost/utility.hpp>

#include "cpuvisor_config.pb.h"

#define LAST_BLOB_STR "last_blob"

namespace featpipe {

  class CaffeNetInst : boost::noncopyable {
  public:
    // constructors
    CaffeNetInst(const CaffeConfig& config,
                 const boost::shared_ptr<boost::condition_variable> ready_cond_var =
                 boost::shared_ptr<boost::condition_variable>())
      : config_(config)
      , ready_(true)
      , ready_cond_var_(ready_cond_var) {
      initNetFromConfig_();
    }
    virtual ~CaffeNetInst() { }
    // main functions
    virtual cv::Mat compute(const std::vector<cv::Mat>& images,
                            std::vector<std::vector<cv::Mat> >* _debug_input_images = 0);

    size_t get_code_size() const;

    bool ready() const { return ready_; }
    void set_ready(const bool ready) { ready_ = ready; }

  protected:
    virtual void initNetFromConfig_();
    CaffeConfig config_;
    AugmentationHelper augmentation_helper_;
    boost::shared_ptr<caffe::Net<float> > net_;
    boost::mutex compute_mutex_;

    bool ready_;
    const boost::shared_ptr<boost::condition_variable> ready_cond_var_;

    virtual void compute_(const std::vector<cv::Mat>& images,
                          cv::Mat* feats,
                          std::vector<std::vector<cv::Mat> >* _debug_input_images = 0);

    virtual std::vector<cv::Mat> prepareImage_(const cv::Mat image);
    virtual cv::Mat forwardPropImages_(std::vector<cv::Mat> images);
  };

}

#endif
