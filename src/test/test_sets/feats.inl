#include <vector>
#include <string>
#include <cmath>
#include <sstream>

#include <glog/logging.h>

#include "directencode/caffe_encoder.h"
#include "server/util/feat_util.h"
#include "server/util/io.h"

#include "cpuvisor_config.pb.h"

#define CONFIG_FILE "../../config.prototxt"
#define TEST_FILE "../../test_data/input/000001.jpg"
#define MLAB_INPUT_IM_PATH "../../test_data/input/mlab_input_im.txt"
#define MLAB_OUTPUT_FEAT_PATH "../../test_data/input/mlab_output_feat.txt"

featpipe::CaffeEncoder setupCaffe() {
  cpuvisor::Config config;
  cpuvisor::readProtoFromTextFile(CONFIG_FILE, &config);

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

TEST_CASE("feats/ensureCorrectOutput",
          "Ensure identical output to matcafffe when using the ZF 128 network") {
  // TODO: fix - currently to get this test to pass, must define
  // ENABLE_BROKEN_OCV_ORDERING in caffe_encoder_utils - this makes
  // sense for the input image (as it is alread loaded in the correct
  // order and doesn't need to be converted) but less so for the mean
  // (implies that the broken mean is also being used in Matlab?)
  featpipe::CaffeEncoder encoder = setupCaffe();

  std::vector<float> input_im;
  {
    DLOG(INFO) << "Loading Matlab input image...";
    std::ifstream input_im_fs(MLAB_INPUT_IM_PATH);
    size_t excess = 0;
    while(!input_im_fs.eof()){
      float input_pix;
      input_im_fs >> input_pix;
      if (input_im.size() == 150528) {
        excess++;
      } else {
        input_im.push_back(input_pix);
      }
    }
    CHECK_EQ(input_im.size(), 150528);
    CHECK_LE(excess, 1);
  }

  // std::vector<float> input_mean;
  // {
  //   DLOG(INFO) << "Loading Matlab input mean...";
  //   std::ifstream input_mean_fs("input_mean.txt");
  //   size_t excess = 0;
  //   while(!input_mean_fs.eof()){
  //     float input_pix;
  //     input_mean_fs >> input_pix;
  //     if (input_mean.size() == 150528) {
  //       excess++;
  //     } else {
  //       input_mean.push_back(input_pix);
  //     }
  //   }
  //   CHECK_EQ(input_mean.size(), 150528);
  //   CHECK_LE(excess, 1);
  // }

  std::vector<float> output_feat;
  {
    DLOG(INFO) << "Loading Matlab output feat...";
    std::ifstream output_feat_fs(MLAB_OUTPUT_FEAT_PATH);
    size_t excess = 0;
    while(!output_feat_fs.eof()){
      float input_pix;
      output_feat_fs >> input_pix;
      if (output_feat.size() == 128) {
        excess++;
      } else {
        output_feat.push_back(input_pix);
      }
    }
    CHECK_EQ(output_feat.size(), 128);
    CHECK_LE(excess, 1);
  }

  cv::Mat im = cv::Mat::ones(224,224,CV_32FC3);
  CHECK_EQ(im.rows, 224);
  CHECK_EQ(im.cols, 224);
  CHECK_EQ(im.channels(), 3);

  DLOG(INFO) << "Copying to cv::Mat...";
  float* im_ptr = (float*)im.data;
  for (size_t i = 0; i < 150528; ++i) {
    im_ptr[i] = input_im[i];
  }

  DLOG(INFO) << "Computing feature...";
  std::vector<cv::Mat> ims;
  ims.push_back(im);
  cv::Mat feat = encoder.compute(ims);
  CHECK_EQ(feat.type(), CV_32F);
  float* feat_ptr = (float*)feat.data;

  for (size_t i = 0; i < 128; ++i) {
    CHECK_FALSE(feat_ptr[i] != Approx(output_feat[i]));
  }

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
    feat.copyTo(feats.row(i));
    //feats.row(i) = encoder.compute(ims); <-- THIS DOESN'T WORK!!!!!
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
