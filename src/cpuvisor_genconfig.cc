#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "directencode/caffe_encoder.h"

#include <google/protobuf/text_format.h>
#include "visor_config.pb.h"
#include "cpuvisor_config.pb.h"

DEFINE_string(config_path, "../config.prototxt", "Server config file");
DEFINE_string(sample_image, "../test_data/input/000001.jpg", "Test image");

int main(int argc, char* argv[]) {

  google::InstallFailureSignalHandler();

  // 1. Create server config

  // cpuvisor::Config config;
  // cpuvisor::CaffeConfig& caffe_config = *config.mutable_caffe_config();
  // caffe_config.set_param_file("/home/ken/src/caffe/examples/CNN_M_128/VGG_CNN_M_128_deploy.prototxt");
  // caffe_config.set_model_file("/home/ken/src/caffe/examples/CNN_M_128/VGG_CNN_M_128.caffemodel");
  // caffe_config.set_mean_image_file("/home/ken/src/caffe/examples/CNN_M_128/VGG_mean.binaryproto");
  // caffe_config.set_data_aug_type(cpuvisor::DAT_NONE);

  // std::string config_str;
  // google::protobuf::TextFormat::PrintToString(config, &config_str);

  // {
  //   std::ofstream config_stream("/home/ken/src/cpuvisor-srv/config.prototxt");
  //   config_stream << config_str;
  // }

  // 1. Load server config

  cpuvisor::Config config;
  {
    std::ifstream config_stream(FLAGS_config_path);
    std::stringstream buffer;
    buffer << config_stream.rdbuf();
    google::protobuf::TextFormat::ParseFromString(buffer.str(), &config);
  }
  cpuvisor::CaffeConfig& caffe_config = *config.mutable_caffe_config;

  // 2. Encode!

  LOG(INFO) << "Initializing encoder...";
  featpipe::CaffeEncoder encoder(caffe_config);

  LOG(INFO) << "Loading image...";
  cv::Mat im = cv::imread(FLAGS_sample_image, CV_LOAD_IMAGE_COLOR);
  im.convertTo(im, CV_32FC3);

  std::vector<cv::Mat> ims;
  ims.push_back(im);
  LOG(INFO) << "Computing feature...";
  cv::Mat feature = encoder.compute(ims);

  std::cout << feature;

  return 0;

}
