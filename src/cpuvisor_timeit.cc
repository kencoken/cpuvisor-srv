#include <iostream>
#include <vector>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include <opencv2/opencv.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "directencode/caffe_encoder.h"
#include "server/util/io.h"
#include "server/util/tictoc.h"

#include "visor_config.pb.h"
#include "cpuvisor_config.pb.h"

DEFINE_string(config_path, "../config.prototxt", "Server config file");

#define IM_WIDTH 800
#define IM_HEIGHT 600
#define TRIAL_COUNT 100

void compFeats(const std::vector<cv::Mat> ims, boost::shared_ptr<featpipe::CaffeEncoder> encoder,
               const size_t trials = 1) {
  for (size_t i = 0; i < trials; ++i) {
    encoder->compute(ims);
  }
}

int main (int argc, char* argv[]) {

  google::InstallFailureSignalHandler();
  gflags::SetUsageMessage("Benchmarking utility for CPU Visor server");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  visor::Config config;
  cpuvisor::readProtoFromTextFile(FLAGS_config_path, &config);

  const cpuvisor::CaffeConfig& caffe_config = config.GetExtension(cpuvisor::caffe_config);
  boost::shared_ptr<featpipe::CaffeEncoder> encoder(new featpipe::CaffeEncoder(caffe_config));

  // start
  cv::theRNG().state = 100;

  cv::Mat im(IM_WIDTH, IM_HEIGHT, CV_32FC3);
  cv::randu(im, cv::Scalar::all(0), cv::Scalar::all(255));

  std::vector<cv::Mat> ims;
  ims.push_back(im);

  float comp_time = 0.0;

  uint32_t netpool_sz = caffe_config.netpool_sz();

  if (netpool_sz == 1) {

    std::cout << "Running " << TRIAL_COUNT << " trials..." << std::endl;

    TicTocObj timer = tic();
    compFeats(ims, encoder, TRIAL_COUNT);
    comp_time = toc(timer);

  } else {

    std::cout << "Running " << TRIAL_COUNT << " trials using " << netpool_sz << " threads..." << std::endl;

    CHECK_GE(TRIAL_COUNT, netpool_sz);

    boost::thread_group tg;
    size_t trial_size = static_cast<size_t>(TRIAL_COUNT / netpool_sz);
    size_t final_trial_size = trial_size + (TRIAL_COUNT % netpool_sz);

    TicTocObj timer = tic();

    for (size_t i = 0; i < netpool_sz; ++i) {
      size_t ts = (i + 1 == netpool_sz) ? final_trial_size : trial_size;
      tg.add_thread(new boost::thread(compFeats, ims, encoder, ts));
    }

    tg.join_all();

    comp_time = toc(timer);

  }

  std::cout << "Trials completed in " << comp_time << " seconds" << std::endl;
  std::cout << "   mean " << comp_time/TRIAL_COUNT << " per image" << std::endl;

  return 0;
}
