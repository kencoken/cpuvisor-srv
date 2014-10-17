#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <glog/logging.h>

#include "server/util/io.h"
#include "cpuvisor_config.pb.h"

#import "server/zmq_server.h"

int main(int argc, char* argv[]) {

  google::InstallFailureSignalHandler();

  cpuvisor::Config config;
  cpuvisor::readProtoFromTextFile("/Data/src/cpuvisor-srv/config.prototxt", &config);

  cpuvisor::ZmqServer zmq_server(config);
  zmq_server.serve();

  return 0;

}
