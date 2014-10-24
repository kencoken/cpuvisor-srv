#include <vector>
#include <string>
#include <cmath>
#include <sstream>

#include <glog/logging.h>

#include "directencode/caffe_encoder.h"
#include "server/util/feat_util.h"
#include "server/util/io.h"

#include "cpuvisor_config.pb.h"

#define TEST_FILE "000001.jpg"

featpipe::CaffeEncoder setupCaffe() {
  cpuvisor::Config config;
  cpuvisor::readProtoFromTextFile("/Data/src/cpuvisor-srv/config.prototxt", &config);

  const cpuvisor::CaffeConfig& caffe_config = config.caffe_config();
  return featpipe::CaffeEncoder(caffe_config);
}

TEST_CASE("feats/computeRepeatability",
          "Test that feature when computed multiple times is always the same") {
  featpipe::CaffeEncoder encoder = setupCaffe();

  cv::Mat feat1 = cpuvisor::computeFeat(TEST_FILE, encoder);
  cv::Mat feat2 = cpuvisor::computeFeat(TEST_FILE, encoder);
  cv::Mat feat3 = cpuvisor::computeFeat(TEST_FILE, encoder);

  REQUIRE(feat1.rows == 1);
  REQUIRE(feat1.cols == encoder.get_code_size());
  size_t feat_sz = feat2.cols;

  REQUIRE(feat1.type() == CV_32F);
  float* feat_ptr = (float*)feat1.data;

  float norm_val = 0;
  for (size_t i = 0; i < feat_sz; ++i) {
    norm_val += feat_ptr[i]*feat_ptr[i];
  }
  norm_val = std::sqrt(norm_val);
  REQUIRE(norm_val == Approx(1.0));

  REQUIRE(cv::countNonZero(feat1 != feat2) == 0);
  REQUIRE(cv::countNonZero(feat1 != feat3) == 0);
}

TEST_CASE("feats/saveLoad",
          "Test that feats written to and loaded from protobuf are the same") {
  featpipe::CaffeEncoder encoder = setupCaffe();

  cv::Mat feats(10, encoder.get_code_size(), CV_32F);
  //cv::Mat feats(10, 128, CV_32F);
  std::vector<std::string> paths(10);
  for (size_t i = 0; i < paths.size(); ++i) {
    // generate path
    std::ostringstream path_strm;
    path_strm << "path " << i;
    paths[i] = path_strm.str();
    // generate feat

    cv::Mat im = cv::imread(TEST_FILE, CV_LOAD_IMAGE_COLOR);
    im.convertTo(im, CV_32FC3);

    //featpipe::CaffeEncoder encoder = setupCaffe();
    std::vector<cv::Mat> ims;
    ims.push_back(im);
    cv::Mat feat = encoder.compute(ims);
    DLOG(INFO) << "HELLO: (" << feat.cols << ")" << feat;
    feat.copyTo(feats.row(i));
    //feats.row(i) = encoder.compute(ims); <-- THIS DOESN'T WORK!!!!!
    DLOG(INFO) << "Goodbye: (" << feats.cols << ")" << feats.row(i);
  }
  for (size_t i = 1; i < paths.size(); ++i) {
    // check all features are the same
    DLOG(INFO) << "Checking i: " << i;
    REQUIRE(cv::countNonZero(feats.row(0) != feats.row(i)) == 0);
  }

  std::string temp_dir = getCleanTempDir();
  std::string temp_file = getTempFile(temp_dir);

  cpuvisor::writeFeatsToProto(feats, paths, temp_file);

  cv::Mat loaded_feats;
  std::vector<std::string> loaded_paths;
  cpuvisor::readFeatsFromProto(temp_file, &loaded_feats, &loaded_paths);

  removeTempDir(temp_dir);

  REQUIRE(paths.size() == loaded_paths.size());
  for (size_t i = 0; i < paths.size(); ++i) {
    REQUIRE(paths[i] == loaded_paths[i]);
  }
  REQUIRE(cv::countNonZero(feats != loaded_feats) == 0);
}
