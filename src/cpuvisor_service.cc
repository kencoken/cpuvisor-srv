#include <iostream>
#include <vector>
#include "opencv2/opencv.hpp"
#include <glog/logging.h>

#include "directencode/caffe_encoder.h"

int main(int argc, char* argv[]) {

  google::InstallFailureSignalHandler();

  std::cout << "Hello world!" << std::endl;

  featpipe::CaffeConfig config;
  config.param_file = "/Data/src/caffe/examples/CNN_M_128/VGG_CNN_M_128_deploy.prototxt";
  config.model_file = "/Data/src/caffe/examples/CNN_M_128/VGG_CNN_M_128.caffemodel";
  config.mean_image_file = "/Data/src/caffe/examples/CNN_M_128/VGG_mean.binaryproto";
  config.data_aug_type = featpipe::DAT_ASPECT_CORNERS;
  config.output_blob_name = "fc7";

  LOG(INFO) << "Initializing encoder...";
  featpipe::CaffeEncoder encoder(config);

  LOG(INFO) << "Loading image...";
  cv::Mat im = cv::imread("/Users/ken/Pictures/ken.jpg", CV_LOAD_IMAGE_COLOR);
  im.convertTo(im, CV_32FC3);

  std::vector<cv::Mat> ims;
  ims.push_back(im);
  LOG(INFO) << "Computing feature...";
  cv::Mat feature = encoder.compute(ims);

  std::cout << feature;

}
