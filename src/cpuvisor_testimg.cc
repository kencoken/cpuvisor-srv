#include <iostream>
#include <stdint.h>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "directencode/caffe_encoder.h"
#include "server/util/preproc.h"
#include "server/util/feat_util.h"
#include "server/util/io.h"

#include "visor_config.pb.h"

DEFINE_string(config_path, "../config.prototxt", "Server config file");

DEFINE_string(train_pospaths_file, "../test_data/input/testimg_train_pospaths_voc07.txt",
              "Input file containing paths of positive training images");
DEFINE_string(train_negpaths_file, "../test_data/input/testimg_train_negpaths_voc07.txt",
              "Input file containing paths of negative training images");
DEFINE_string(test_pospaths_file, "../test_data/input/testimg_test_pospaths_voc07.txt",
              "Input file containing paths of positive test images");
DEFINE_string(test_negpaths_file, "../test_data/input/testimg_test_negpaths_voc07.txt",
              "Input file containing paths of negative test images");

DEFINE_string(train_posproto_file, "../test_data/output/testimg_train_pos_voc07.binaryproto",
              "Output file containing computed features of positive training images");
DEFINE_string(train_negproto_file, "../test_data/output/testimg_train_neg_voc07.binaryproto",
              "Output file containing computed features of negative training images");
DEFINE_string(test_posproto_file, "../test_data/output/testimg_test_pos_voc07.binaryproto",
              "Output file containing computed features of positive test images");
DEFINE_string(test_negproto_file, "../test_data/output/testimg_test_neg_voc07.binaryproto",
              "Output file containing computed features of negative test images");

DEFINE_uint64(im_limit, 0, "Limit number of images to compute for each set");

int main(int argc, char* argv[]) {

  google::InstallFailureSignalHandler();

  visor::Config config;
  cpuvisor::readProtoFromTextFile(FLAGS_config_path, &config);

  const cpuvisor::CaffeConfig& caffe_config = config.caffe_config();
  featpipe::CaffeEncoder encoder(caffe_config);

  const visor::PreprocConfig& preproc_config = config.preproc_config();
  std::string base_path = preproc_config.dataset_im_base_path();

  std::string train_pospaths_file = FLAGS_train_pospaths_file;
  std::string train_negpaths_file = FLAGS_train_negpaths_file;
  std::string test_pospaths_file = FLAGS_test_pospaths_file;
  std::string test_negpaths_file = FLAGS_test_negpaths_file;

  std::string train_posproto_file = FLAGS_train_posproto_file;
  std::string train_negproto_file = FLAGS_train_negproto_file;
  std::string test_posproto_file = FLAGS_test_posproto_file;
  std::string test_negproto_file = FLAGS_test_negproto_file;


  // Compute features
  // ----------------------------

  uint64_t im_limit = FLAGS_im_limit; // set to 0 to compute entire dataset

  if (!fs::exists(fs::path(train_posproto_file))) {
    DLOG(INFO) << "Computing positive training feats...";
    cpuvisor::procTextFile(train_pospaths_file,
                           train_posproto_file,
                           encoder,
                           base_path,
                           im_limit);
  }

  if (!fs::exists(fs::path(train_negproto_file))) {
    DLOG(INFO) << "Computing negative training feats...";
    cpuvisor::procTextFile(train_negpaths_file,
                           train_negproto_file,
                           encoder,
                           base_path,
                           im_limit);
  }

  if (!fs::exists(fs::path(test_posproto_file))) {
    DLOG(INFO) << "Computing positive test feats...";
    cpuvisor::procTextFile(test_pospaths_file,
                           test_posproto_file,
                           encoder,
                           base_path,
                           im_limit);
  }

  if (!fs::exists(fs::path(test_negproto_file))) {
    DLOG(INFO) << "Computing negative test feats...";
    cpuvisor::procTextFile(test_negpaths_file,
                           test_negproto_file,
                           encoder,
                           base_path,
                           im_limit);
  }

  // Test
  // ----------------------------

  cv::Mat train_pos_feats;
  cv::Mat train_neg_feats;
  cv::Mat test_pos_feats;
  cv::Mat test_neg_feats;
  std::vector<std::string> train_pos_paths;
  std::vector<std::string> train_neg_paths;
  std::vector<std::string> test_pos_paths;
  std::vector<std::string> test_neg_paths;
  CHECK(cpuvisor::readFeatsFromProto(train_posproto_file, &train_pos_feats, &train_pos_paths));
  CHECK(cpuvisor::readFeatsFromProto(train_negproto_file, &train_neg_feats, &train_neg_paths));
  CHECK(cpuvisor::readFeatsFromProto(test_posproto_file, &test_pos_feats, &test_pos_paths));
  CHECK(cpuvisor::readFeatsFromProto(test_negproto_file, &test_neg_feats, &test_neg_paths));

  // debug
  for (size_t i = 0; i < 5; ++i) {
    std::cout << train_pos_paths[i] << ":" << std::endl
              << train_pos_feats.row(i) << std::endl;

    cv::Mat recomp_feat = cpuvisor::computeFeat(base_path + "/" + train_pos_paths[i], encoder);
    //CHECK_EQ(cv::countNonZero(recomp_feat.row(0) != train_pos_feats.row(i)), 0);
    size_t non_equal_count = 0;
    float max_diff = 0;
    for (int ei = 0; ei < recomp_feat.cols; ++ei) {
      float diff = (recomp_feat.row(0).at<float>(0, ei) - train_pos_feats.row(i).at<float>(0, ei));
      if (diff > 1e-6) {
        non_equal_count++;
        if (diff > max_diff) max_diff = diff;
      }
    }
    CHECK_EQ(non_equal_count, 0) << "Max diff is: " << max_diff;
  }
  // end debug

  cv::Mat model = cpuvisor::trainLinearSvm(train_pos_feats, train_neg_feats,
                                           train_pos_paths, train_neg_paths);

  cv::Mat dset_feats;
  cv::vconcat(test_pos_feats, test_neg_feats, dset_feats);

  cv::Mat scores;
  cv::Mat sort_idxs;
  cpuvisor::rankUsingModel(model, dset_feats, &scores, &sort_idxs);

  float* scores_ptr = (float*)scores.data;
  uint32_t* sort_idx_ptr = (uint32_t*)sort_idxs.data;
  for (size_t i = 0; i < 10; ++i) {
    bool is_tp = (sort_idx_ptr[i] < static_cast<uint32_t>(test_pos_feats.rows));
    std::cout << i+1 << ": " << is_tp << " -> ";
    if (is_tp) {
      std::cout << test_pos_paths[sort_idx_ptr[i]];
    } else {
      std::cout << test_neg_paths[sort_idx_ptr[i] - test_pos_feats.rows];
    }
    std::cout << " (" << scores_ptr[sort_idx_ptr[i]] << " @ " << sort_idx_ptr[i] << ")" << std::endl;
  }

  return 0;
}
