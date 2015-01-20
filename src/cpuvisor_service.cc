#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "server/util/io.h"
#include "cpuvisor_config.pb.h"

#include "server/zmq_server.h"

DEFINE_string(config_path, "../config.prototxt", "Server config file");

int main(int argc, char* argv[]) {

  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  cpuvisor::Config config;
  cpuvisor::readProtoFromTextFile(FLAGS_config_path, &config);

  cpuvisor::ZmqServer zmq_server(config);
  zmq_server.serve();

  return 0;

}
