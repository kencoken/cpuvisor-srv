#include <iostream>
#include <stdint.h>
#include <glog/logging.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "directencode/caffe_encoder.h"
#include "server/util/preproc.h"
#include "server/util/feat_util.h"
#include "server/util/io.h"

#include "cpuvisor_config.pb.h"

int main(int argc, char* argv[]) {

  google::InstallFailureSignalHandler();

  cpuvisor::Config config;
  cpuvisor::readProtoFromTextFile("/Data/src/cpuvisor-srv/config.prototxt", &config);

  const cpuvisor::CaffeConfig& caffe_config = config.caffe_config();
  featpipe::CaffeEncoder encoder(caffe_config);

  std::string base_path = "/Users/ken/Downloads/VOCdevkit/VOC2007";
  std::string train_pospaths_file = "/Data/src/cpuvisor-srv/train_pospaths.txt";
  std::string train_negpaths_file = "/Data/src/cpuvisor-srv/train_negpaths.txt";
  std::string test_pospaths_file = "/Data/src/cpuvisor-srv/test_pospaths.txt";
  std::string test_negpaths_file = "/Data/src/cpuvisor-srv/test_negpaths.txt";

  std::string train_posproto_file = "/Data/src/cpuvisor-srv/train_pos.binaryproto";
  std::string train_negproto_file = "/Data/src/cpuvisor-srv/train_neg.binaryproto";
  std::string test_posproto_file = "/Data/src/cpuvisor-srv/test_pos.binaryproto";
  std::string test_negproto_file = "/Data/src/cpuvisor-srv/test_neg.binaryproto";


  // Compute features
  // ----------------------------

  size_t im_limit = 100; // set to 0 to compute entire dataset

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
  cpuvisor::readFeatsFromProto(train_posproto_file, &train_pos_feats, &train_pos_paths);
  cpuvisor::readFeatsFromProto(train_negproto_file, &train_neg_feats, &train_neg_paths);
  cpuvisor::readFeatsFromProto(test_posproto_file, &test_pos_feats, &test_pos_paths);
  cpuvisor::readFeatsFromProto(test_negproto_file, &test_neg_feats, &test_neg_paths);

  // debug
  for (size_t i = 0; i < 5; ++i) {
    std::cout << train_pos_paths[i] << ":" << std::endl
              << train_pos_feats.row(i) << std::endl;

    cv::Mat recomp_feat = cpuvisor::computeFeat(base_path + "/" + train_pos_paths[i], encoder);
    CHECK_EQ(cv::countNonZero(recomp_feat.row(0) != train_pos_feats.row(i)), 0);
  }
  // end debug

  cv::Mat model = cpuvisor::trainLinearSvm(train_pos_feats, train_neg_feats);

  cv::Mat dset_feats;
  cv::vconcat(test_pos_feats, test_neg_feats, dset_feats);

  cv::Mat scores;
  cv::Mat sort_idxs;
  cpuvisor::rankUsingModel(model, dset_feats, &scores, &sort_idxs);

  float* scores_ptr = (float*)scores.data;
  uint32_t* sort_idx_ptr = (uint32_t*)sort_idxs.data;
  for (size_t i = 0; i < 10; ++i) {
    bool is_tp = (sort_idx_ptr[i] < test_pos_feats.rows);
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
