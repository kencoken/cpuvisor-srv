////////////////////////////////////////////////////////////////////////////
//    File:        caffe_encoder.h
//    Author:      Ken Chatfield
//    Description: Encoder for VGG Caffe
////////////////////////////////////////////////////////////////////////////

#ifndef FEATPIPE_CAFFE_ENCODER_H_
#define FEATPIPE_CAFFE_ENCODER_H_

#include "generic_direct_encoder.h"
#include "caffe_config.h"
#include "netpool/caffe_netpool.h"

#include <string>
#include <iostream>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/property_tree/ptree.hpp>

#include "cpuvisor_config.pb.h"

namespace featpipe {

  // class definition --------------------

  class CaffeEncoder : public GenericDirectEncoder {
  public:
    inline virtual CaffeEncoder* clone() const {
      return new CaffeEncoder(*this);
    }
    // constructors
    CaffeEncoder(const CaffeConfig& config): config_(config) {
      initNetFromConfig_();
    }
    CaffeEncoder(const cpuvisor::CaffeConfig& proto_config) {
      config_ = CaffeConfig();
      config_.configureFromProtobuf(proto_config);
      initNetFromConfig_();
    }
    CaffeEncoder(const CaffeEncoder& other) {
      config_ = other.config_;
      initNetFromConfig_();
    }
    CaffeEncoder& operator= (const CaffeEncoder& rhs) {
      config_ = rhs.config_;
      initNetFromConfig_();
      return (*this);
    }
    // main functions
    virtual cv::Mat compute(const std::vector<cv::Mat>& images,
                            std::vector<std::vector<cv::Mat> >* _debug_input_images = 0);
    // virtual setter / getters
    inline virtual size_t get_code_size() const {
      return nets_->get_code_size();
    }
    // configuration
    inline virtual void configureFromPtree(const boost::property_tree::ptree& properties) {
      CaffeConfig config;
      config.configureFromPtree(properties);
      config_ = config;
      initNetFromConfig_();
    }
  protected:
    void initNetFromConfig_();
    CaffeConfig config_;
    boost::shared_ptr<CaffeNetPool> nets_;
  };
}

#endif
