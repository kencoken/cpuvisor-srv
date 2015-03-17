#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "server/util/io.h"
#include "cpuvisor_config.pb.h"

#include "server/zmq_server.h"

DEFINE_string(config_path, "../config.prototxt", "Server config file");

void setupGoogleLogging(char* argv[]) {

  FLAGS_stderrthreshold = 1; // log WARNING or above to stderr

  #ifndef NDEBUG
  // in debug mode, output all glog messages to console unless a
  // custom logging directory is specified
  if (!FLAGS_log_dir.empty()) {
    google::InitGoogleLogging(argv[0]);
  }
  #else
  // in release mode, enable google logging as usual
  // (logs stored in /tmp/cpuvisor_service.<hostname>.<user name>.log.<severity level>.<date>.<time>.<pid>)
  google::InitGoogleLogging(argv[0]);
  #endif

}

int main(int argc, char* argv[]) {

  google::InstallFailureSignalHandler();
  setupGoogleLogging(argv);
  gflags::SetUsageMessage("CPU Visor service");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  visor::Config config;
  cpuvisor::readProtoFromTextFile(FLAGS_config_path, &config);

  cpuvisor::ZmqServer zmq_server(config);
  zmq_server.serve();

  return 0;

}
