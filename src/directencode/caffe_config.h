////////////////////////////////////////////////////////////////////////////
//    File:        caffe_config.h
//    Author:      Ken Chatfield
//    Description: Config for encoder for VGG Caffe
////////////////////////////////////////////////////////////////////////////

#ifndef FEATPIPE_CAFFE_CONFIG_H_
#define FEATPIPE_CAFFE_CONFIG_H_

#include <boost/property_tree/ptree.hpp>

#include "directencode/augmentation_helper.h" // DataAugType
#include "cpuvisor_config.pb.h"

#define DEFAULT_BLOB_STR "fc7"

namespace featpipe {

  // configuration -----------------------

  enum CaffeMode {CM_CPU = 0, CM_GPU = 1};

  struct CaffeConfig {
    std::string param_file;
    std::string model_file;
    std::string mean_image_file;
    DataAugType data_aug_type;
    std::string output_blob_name;
    CaffeMode mode;
    bool use_rgb_images;
    uint32_t netpool_sz;
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
      std::string mode_str = properties.get<std::string>("mode", "");
      if (mode_str == "CPU") {
        mode = CM_CPU;
      } else if (mode_str == "GPU") {
        mode = CM_GPU;
      } else {
        LOG(FATAL) << "Unrecognized caffe mode: " << data_aug_type_str;
      }
      use_rgb_images = properties.get<bool>("use_rgb_images", false);
      netpool_sz = properties.get<uint32_t>("netpool_sz", 1);
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
      cpuvisor::CaffeMode proto_caffe_mode = proto_config.mode();
      switch (proto_caffe_mode) {
      case cpuvisor::CM_CPU:
        mode = CM_CPU;
        break;
      case cpuvisor::CM_GPU:
        mode = CM_GPU;
        break;
      }
      use_rgb_images = proto_config.use_rgb_images();
      netpool_sz = proto_config.netpool_sz();
    }
  };

}

#endif
