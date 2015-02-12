#include "caffe_encoder.h"

#include <glog/logging.h>

//#define DEBUG_CAFFE_CHECKSUM

#ifdef DEBUG_CAFFE_CHECKSUM
#include <boost/crc.hpp>
#endif

cv::Mat featpipe::CaffeEncoder::compute(const std::vector<cv::Mat>& images,
                                        std::vector<std::vector<cv::Mat> >* _debug_input_images) {

  boost::shared_ptr<CaffeNetInst> net = nets_->getReadyNet();
  cv::Mat feats = net->compute(images, _debug_input_images);

  return feats;

}

void featpipe::CaffeEncoder::initNetFromConfig_() {

  switch (config_.mode) {
  case CM_CPU:
    caffe::Caffe::set_mode(caffe::Caffe::CPU);
    break;
  case CM_GPU:
    caffe::Caffe::set_mode(caffe::Caffe::GPU);
    if (config_.netpool_sz > 1) {
      LOG(FATAL) << "netpool_sz > 1 not supported for GPU computation mode!";
    }
    break;
  default:
    LOG(FATAL) << "Unsupported mode!";
  }
  caffe::Caffe::set_phase(caffe::Caffe::TEST);

  nets_ = boost::shared_ptr<CaffeNetPool>(new CaffeNetPool(config_));

}
