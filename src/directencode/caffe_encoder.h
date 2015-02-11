////////////////////////////////////////////////////////////////////////////
//    File:        caffe_encoder.h
//    Author:      Ken Chatfield
//    Description: Encoder for VGG Caffe
////////////////////////////////////////////////////////////////////////////

#ifndef FEATPIPE_CAFFE_ENCODER_H_
#define FEATPIPE_CAFFE_ENCODER_H_

#include "generic_direct_encoder.h"
#include "caffe_encoder_utils.h"
#include "augmentation_helper.h"

#include <iostream>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/property_tree/ptree.hpp>

#include "cpuvisor_config.pb.h"

#define LAST_BLOB_STR "last_blob"
#define DEFAULT_BLOB_STR "fc7"

namespace featpipe {

  // configuration -----------------------

  struct CaffeConfig {
    std::string param_file;
    std::string model_file;
    std::string mean_image_file;
    DataAugType data_aug_type;
    std::string output_blob_name;
    bool use_rgb_images;
    inline virtual void configureFromPtree(const boost::property_tree::ptree& properties) {
      param_file = properties.get<std::string>("param_file");
      model_file = properties.get<std::string>("model_file");
      mean_image_file = properties.get<std::string>("mean_image_file");
      std::string data_aug_type_str = properties.get<std::string>("data_aug_type", "");
      if (data_aug_type_str == "ASPECT_CORNERS") {
        data_aug_type = DAT_ASPECT_CORNERS;
      } else if ((data_aug_type_str == "NONE") || (data_aug_type_str == "")) {
        data_aug_type = DAT_NONE;
      } else {
        LOG(FATAL) << "Unrecognized data augmentation type: " << data_aug_type_str;
      }
      output_blob_name = properties.get<std::string>("output_blob", DEFAULT_BLOB_STR);
      use_rgb_images = properties.get<bool>("use_rgb_images", false);
    }
    inline virtual void configureFromProtobuf(const cpuvisor::CaffeConfig& proto_config) {
      param_file = proto_config.param_file();
      model_file = proto_config.model_file();
      mean_image_file = proto_config.mean_image_file();
      cpuvisor::DataAugType proto_data_aug_type = proto_config.data_aug_type();
      switch (proto_data_aug_type) {
      case cpuvisor::DAT_NONE:
        data_aug_type = DAT_NONE;
        break;
      case cpuvisor::DAT_ASPECT_CORNERS:
        data_aug_type = DAT_ASPECT_CORNERS;
        break;
      }
      output_blob_name = proto_config.output_blob_name();
      use_rgb_images = proto_config.use_rgb_images();
    }
  };

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
                            std::vector<cv::Mat>* _debug_input_images = 0);
    // virtual setter / getters
    inline virtual size_t get_code_size() const {
      if (config_.output_blob_name == LAST_BLOB_STR) {
        const std::vector<caffe::Blob<float>* >& output_blobs = net_->output_blobs();
        return output_blobs[0]->count() / output_blobs[0]->num();
      } else {
        boost::shared_ptr<caffe::Blob<float> > blob = net_->blob_by_name(config_.output_blob_name);
        return blob->count() / blob->num();
      }
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
    AugmentationHelper augmentation_helper_;
    boost::shared_ptr<caffe::Net<float> > net_;
    boost::mutex compute_mutex_;
  };
}

#endif
