#include <glog/logging.h>
#include <gflags/gflags.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#include <boost/lexical_cast.hpp>

#include "server/util/io.h"

#include "cpuvisor_config.pb.h"

DEFINE_string(config_path, "../config.prototxt", "Server config file");
DEFINE_int64(chunk_sz, 1000, "Size of computation chunks");

int main(int argc, char* argv[]) {

  google::InstallFailureSignalHandler();
  google::SetUsageMessage("Dataset feature file chunk combination for CPU Visor server");
  google::ParseCommandLineFlags(&argc, &argv, true);

  cpuvisor::Config config;
  cpuvisor::readProtoFromTextFile(FLAGS_config_path, &config);

  const cpuvisor::PreprocConfig& preproc_config = config.preproc_config();

  const std::string& feats_file = preproc_config.dataset_feats_file();
  fs::path feats_file_fs(feats_file);
  std::string feats_file_stem = (feats_file_fs.parent_path() / feats_file_fs.stem()).string();
  std::string feats_file_ext = feats_file_fs.extension().string();

  LOG(INFO) << "Starting chunk filename generation...";
  int64_t im_count = cpuvisor::getTextFileLineCount(preproc_config.dataset_im_paths());
  int64_t chunk_count = static_cast<int64_t>(std::ceil(static_cast<double>(im_count) /
                                                       static_cast<double>(FLAGS_chunk_sz)));

  std::vector<std::string> chunk_files(chunk_count);
  for (size_t i = 0; i < static_cast<size_t>(chunk_count); ++i) {

    int64_t start_idx = i*FLAGS_chunk_sz;
    int64_t end_idx = (i+1)*FLAGS_chunk_sz;
    if (end_idx > im_count) {
      end_idx = im_count;
      CHECK_EQ(i+1, chunk_count);
    }

    chunk_files[i] = (feats_file_stem
                      + "_" + boost::lexical_cast<std::string>(start_idx + 1)
                      + "-" + boost::lexical_cast<std::string>(end_idx)
                      + feats_file_ext);
  }

  LOG(INFO) << "Combining chunks...";

  // cv::Mat feats;
  // std::vector<std::string> paths;
  size_t feat_num, feat_dim;

  for (size_t i = 0; i < chunk_files.size(); ++i) {
    LOG(INFO) << "Processing chunk: " << chunk_files[i];
    cv::Mat feats_chunk;
    std::vector<std::string> paths_chunk;

    CHECK(cpuvisor::readFeatsFromProto(chunk_files[i], &feats_chunk, &paths_chunk));

    if (i == 0) {
      feat_num = feats_chunk.rows;
      feat_dim = feats_chunk.cols;
      CHECK_EQ(feat_num, paths_chunk.size());
    } else {
      feat_num += feats_chunk.rows;
      CHECK_EQ(feat_dim, feats_chunk.cols);
    }

    // feats.push_back(feats_chunk);

    // for (size_t ci = 0; ci < paths_chunk.size(); ++ci) {
    //   paths.push_back(paths_chunk[ci]);
    // }

    // CHECK_EQ(feats.rows, paths.size());
  }

  LOG(INFO) << "Saving combined chunks...";
  // DLOG(INFO) << "Feats size is: " << feats.rows << "x" << feats.cols;
  // DLOG(INFO) << "Paths size is: " << paths.size();
  // cpuvisor::writeFeatsToProto(feats, paths, feats_file);
  cpuvisor::writeChunkIndexToProto(chunk_files, feat_num, feat_dim, feats_file);
}
