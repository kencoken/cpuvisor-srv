#include <glog/logging.h>
#include <gflags/gflags.h>

#include "server/util/io.h"

DEFINE_string(feats_file, "", "Features file");

int main(int argc, char* argv[]) {

  google::InstallFailureSignalHandler();
  google::SetUsageMessage("Inspect binaryproto feature file");
  google::ParseCommandLineFlags(&argc, &argv, true);

  CHECK_NE(FLAGS_feats_file, "");

  cv::Mat feats;
  std::vector<std::string> paths;

  CHECK(cpuvisor::readFeatsFromProto(FLAGS_feats_file,
                                     &feats,
                                     &paths));

  for (size_t i = 0; i < 5; ++i) {
    LOG(INFO) << i << ": " << paths[i];
    LOG(INFO) << feats.row(i);
  }

  for (size_t i = 1000; i < 1005; ++i) {
    LOG(INFO) << i << ": " << paths[i];
    LOG(INFO) << feats.row(i);
  }

  return 0;

}
