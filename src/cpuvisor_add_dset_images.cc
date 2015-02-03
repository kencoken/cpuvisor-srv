#include <iostream>
#include <vector>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "server/util/io.h"
#include "cpuvisor_config.pb.h"

#include "server/zmq_client.h"

DEFINE_string(config_path, "../config.prototxt", "Server config file");
DEFINE_string(paths, "", "Text file containing images to add to index");

int main(int argc, char* argv[]) {

  //google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  cpuvisor::Config config;
  cpuvisor::readProtoFromTextFile(FLAGS_config_path, &config);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_paths.empty()) {
    CHECK(argc >= 2) << "paths parameter must be specified!";
    FLAGS_paths = std::string(argv[1]);
  }

  cpuvisor::ZmqClient zmq_client(config);

  // read in image paths
  std::vector<std::string> dset_paths;
  std::ifstream imfiles(FLAGS_paths.c_str());
  CHECK(imfiles.is_open()) << "Error opening file: " << FLAGS_paths;

  std::string imfile;
  while (std::getline(imfiles, imfile)) {
    dset_paths.push_back(imfile);
  }

  // add to index
  LOG(INFO) << "Adding " << dset_paths.size() << " images to index...";
  zmq_client.addDsetImagesToIndex(dset_paths);

  return 0;

}
