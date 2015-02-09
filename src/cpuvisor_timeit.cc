#include <iostream>
#include <vector>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "directencode/caffe_encoder.h"
#include "server/util/io.h"
#include "server/util/tictoc.h"

#include "cpuvisor_config.pb.h"

DEFINE_string(config_path, "../config.prototxt", "Server config file");

#define IM_WIDTH 800
#define IM_HEIGHT 600
#define TRIAL_COUNT 100

int main (int argc, char* argv[]) {

  google::InstallFailureSignalHandler();
  gflags::SetUsageMessage("Benchmarking utility for CPU Visor server");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  cpuvisor::Config config;
  cpuvisor::readProtoFromTextFile(FLAGS_config_path, &config);

  const cpuvisor::CaffeConfig& caffe_config = config.caffe_config();
  featpipe::CaffeEncoder encoder(caffe_config);

  // start
  cv::theRNG().state = 100;

  cv::Mat im(IM_WIDTH, IM_HEIGHT, CV_32FC3);
  cv::randu(im, cv::Scalar::all(0), cv::Scalar::all(255));

  std::vector<cv::Mat> ims;
  ims.push_back(im);

  std::cout << "Running " << TRIAL_COUNT << "trials..." << std::endl;

  TicTocObj timer = tic();
  for (size_t i = 0; i < TRIAL_COUNT; ++i) {
    encoder.compute(ims);
    if (i % 10) {
      std::cout << ".";
    }
  }
  std::cout << std::endl;

  float comp_time = toc(timer);

  std::cout << "Trials completed in " << comp_time << " seconds" << std::endl;
  std::cout << "   mean " << comp_time/TRIAL_COUNT << " per image" << std::endl;

  return 0;
}
