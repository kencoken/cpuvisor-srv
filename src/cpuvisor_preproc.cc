#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include <glog/logging.h>
#include <gflags/gflags.h>

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>

#include "directencode/caffe_encoder.h"
#include "server/util/io.h"
#include "cpuvisor_config.pb.h"

namespace fs = boost::filesystem;

DEFINE_bool(dsetfeats, true, "Compute dataset features");
DEFINE_bool(negfeats, true, "Compute negative training image features");

void procTextFile(const std::string& text_path,
                  const std::string& proto_path,
                  featpipe::CaffeEncoder& encoder,
                  const std::string& base_path = std::string()) {
  std::vector<std::string> paths;

  std::ifstream imfiles(text_path);

  // std::string base_path;
  // std::getline(imfiles, base_path);
  fs::path base_path_fs(base_path);
  if (!base_path.empty()) {
    CHECK(fs::exists(base_path_fs) && fs::is_directory(fs::canonical(base_path_fs)))
      << "First line of file should be path to root of dataset or blank";
  }

  std::string imfile;
  while (std::getline(imfiles, imfile)) {
    if (!imfile.empty()) {
      paths.push_back(imfile);
    }
  }

  CHECK_GT(paths.size(), 0) << "No paths could be read from file: " << text_path;

  cv::Mat feats(paths.size(), encoder.get_code_size(), CV_32F);

  for (size_t i = 0; i < paths.size(); ++i) {
    LOG(INFO) << "Computing feature for image: " << paths[i];

    std::string full_path = (base_path_fs / fs::path(paths[i])).string();

    cv::Mat im = cv::imread(full_path, CV_LOAD_IMAGE_COLOR);
    im.convertTo(im, CV_32FC3);

    std::vector<cv::Mat> ims;
    ims.push_back(im);
    feats.row(i) = encoder.compute(ims);
  }

  LOG(INFO) << "Writing features to: " << proto_path;
  cpuvisor::writeFeatsToProto(feats, paths, proto_path);
}

int main(int argc, char* argv[]) {

  google::InstallFailureSignalHandler();
  google::SetUsageMessage("Preprocessing for CPU Visor server");
  google::ParseCommandLineFlags(&argc, &argv, true);

  cpuvisor::Config config;
  cpuvisor::readProtoFromTextFile("/Data/src/cpuvisor-srv/config.prototxt", &config);

  const cpuvisor::CaffeConfig& caffe_config = config.caffe_config();
  featpipe::CaffeEncoder encoder(caffe_config);

  const cpuvisor::PreprocConfig& preproc_config = config.preproc_config();

  if (FLAGS_dsetfeats) {
    procTextFile(preproc_config.dataset_im_paths(),
                 preproc_config.dataset_feats_file(),
                 encoder,
                 preproc_config.dataset_im_base_path());
  }

  if (FLAGS_negfeats) {
    procTextFile(preproc_config.neg_im_paths(),
                 preproc_config.neg_feats_file(),
                 encoder,
                 preproc_config.neg_im_base_path());
  }

  return 0;
}
