////////////////////////////////////////////////////////////////////////////
//    File:        io.h
//    Author:      Ken Chatfield
//    Description: Functions to convert to/from proto format
////////////////////////////////////////////////////////////////////////////

#ifndef CPUVISOR_UTILS_IO_H_
#define CPUVISOR_UTILS_IO_H_

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <glog/logging.h>

#include "cpuvisor_srv.pb.h"

using google::protobuf::Message;

namespace cpuvisor {

  void writeFeatsToProto(const cv::Mat& feats, const std::vector<std::string>& paths,
                         const std::string& proto_path);
  bool readFeatsFromProto(const std::string& proto_path,
                          cv::Mat* feats, std::vector<std::string>* paths);

  bool readProtoFromTextFile(const std::string& proto_path, Message* proto);
  void writeProtoToTextFile(const std::string& proto_path, const Message& proto);
  bool readProtoFromBinaryFile(const std::string& proto_path, Message* proto);
  void writeProtoToBinaryFile(const std::string& proto_path, const Message& proto);

}

#endif
