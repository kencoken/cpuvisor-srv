#include "io.h"

#include <boost/shared_ptr.hpp>
#include <boost/scope_exit.hpp>

#include <fstream>
#include <fcntl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

using google::protobuf::io::FileInputStream;
using google::protobuf::io::FileOutputStream;
using google::protobuf::io::ZeroCopyInputStream;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::ZeroCopyOutputStream;
using google::protobuf::io::CodedOutputStream;

namespace cpuvisor {

  int64_t getTextFileLineCount(const std::string& text_path) {
    std::ifstream imfiles(text_path.c_str());

    int64_t line_count = 0;
    std::string imfile;
    while (std::getline(imfiles, imfile)) {
      if (!imfile.empty()) {
        ++line_count;
      }
    }
    CHECK_GE(line_count, 0);

    return line_count;
  }

  void writeFeatsToProto(const cv::Mat& feats, const std::vector<std::string>& paths,
                         const std::string& proto_path) {
    CHECK_EQ(paths.size(), feats.rows);

    cpuvisor::FeatsProto feats_proto;
    feats_proto.set_num(feats.rows);
    feats_proto.set_dim(feats.cols);
    feats_proto.clear_data();
    feats_proto.clear_paths();

    CHECK_EQ(feats.channels(), 1);
    CHECK_EQ(feats.depth(), CV_32F);
    CHECK(feats.isContinuous());

    const float* feats_data = (float*)feats.data;
    if (false) {
      // each feature stored in separate message
      for (size_t i = 0; i < static_cast<size_t>(feats.rows); ++i) {
        cpuvisor::FeatProto* feat_proto = feats_proto.add_feats();
        for (size_t j = 0; j < static_cast<size_t>(feats.cols); ++j) {
          feat_proto->add_data(feats_data[i*feats.cols + j]);
        }
      }
    } else {
      // features stored in one large packed message
      for (size_t i = 0; i < static_cast<size_t>(feats.rows*feats.cols); ++i) {
        feats_proto.add_data(feats_data[i]);
      }
    }

    for (size_t i = 0; i < paths.size(); ++i) {
      feats_proto.add_paths(paths[i]);
    }

    // DEBUG
    for (size_t i = 0; i < 5; ++i) {
      DLOG(INFO) << paths[i] << ":";
      DLOG(INFO) << feats.row(i);
    }
    // END DEBUG

    writeProtoToBinaryFile(proto_path, feats_proto);
  }

  void writeChunkIndexToProto(const std::vector<std::string>& chunk_fnames,
                              const size_t feat_num, const size_t feat_dim,
                              const std::string& proto_path) {
    cpuvisor::FeatsProto feats_proto;
    feats_proto.set_num(feat_num);
    feats_proto.set_dim(feat_dim);
    feats_proto.clear_data();
    feats_proto.clear_paths();

    for (size_t i = 0; i < chunk_fnames.size(); ++i) {
      feats_proto.add_chunks(chunk_fnames[i]);
    }

    writeProtoToBinaryFile(proto_path, feats_proto);
  }

  bool readFeatsFromProto(const std::string& proto_path,
                          cv::Mat* feats, std::vector<std::string>* paths) {

    cpuvisor::FeatsProto feats_proto;
    bool success = readProtoFromBinaryFile(proto_path, &feats_proto);
    if (!success) return success;

    (*feats) = cv::Mat::zeros(feats_proto.num(), feats_proto.dim(), CV_32FC1);
    std::vector<std::string>& paths_ref = (*paths);
    paths_ref = std::vector<std::string>(feats_proto.num());

    float* feats_data = (float*)feats->data;

    if (feats_proto.chunks_size() > 0) {
      // 1. read in chunks from index file if applicable
      // -----------------------------------------------

      size_t chunks_count = feats_proto.chunks_size();

      size_t ptr = 0;

      for (size_t ci = 0; ci < chunks_count; ++ci) {
        cv::Mat chunk_feats;
        std::vector<std::string> chunk_paths;

        if (!readFeatsFromProto(feats_proto.chunks(ci),
                                &chunk_feats, &chunk_paths)) {
          LOG(ERROR) << "Error reading chunk: " << feats_proto.chunks(ci);
          return false;
        }

        size_t chunk_size = chunk_paths.size();
        CHECK_EQ(chunk_feats.rows, chunk_size);

        if ((ptr + chunk_size) > paths_ref.size()) {
          LOG(ERROR) << "Loaded chunks inconsistent - too many features ("
                     << ptr + chunk_size << " vs. " << paths_ref.size() << ")";
          return false;
        }

        cv::Mat submat = feats->rowRange(ptr, ptr + chunk_size);
        chunk_feats.copyTo(submat);
        for (size_t i = 0; i < chunk_size; ++i) {
          paths_ref[ptr + i] = chunk_paths[i];
        }

        ptr += chunk_size;
      }

      if (ptr != paths_ref.size()) {
        LOG(ERROR) << "Loaded chunks inconsistent - too few features ("
                   << ptr << " vs. " << paths_ref.size() << ")";
        return false;
      }

    } else {
      // 2. else load in a regular chunk
      // -------------------------------

      if (feats_proto.feats_size() > 0) {
        // each feature stored in separate message
        size_t feat_count = feats_proto.num();
        size_t feat_dim = feats_proto.dim();
        CHECK_EQ(feats_proto.feats_size(), feat_count);
        for (size_t i = 0; i < feat_count; ++i) {
          const cpuvisor::FeatProto& feat_proto = feats_proto.feats(i);
          CHECK_EQ(feat_proto.data_size(), feat_dim);
          for (size_t j = 0; j < feat_dim; ++j) {
            feats_data[i*feat_dim + j] = feat_proto.data(j);
          }
        }
      } else {
        // features stored in one large packed message
        size_t max_size = feats_proto.num()*feats_proto.dim();
        for (size_t i = 0; i < max_size; ++i) {
          feats_data[i] = feats_proto.data(i);
        }
      }

      size_t max_size = feats_proto.num();
      for (size_t i = 0; i < max_size; ++i) {
        paths_ref[i] = feats_proto.paths(i);
      }

      // DEBUG
      for (size_t i = 0; i < 5; ++i) {
        DLOG(INFO) << (*paths)[i] << ":";
        DLOG(INFO) << feats->row(i);
      }
      // END DEBUG

    }

    return success;
  }

  // ------------------------------------------------------------------------

  bool readProtoFromTextFile(const std::string& proto_path, Message* proto) {
    int fd = open(proto_path.c_str(), O_RDONLY);
    BOOST_SCOPE_EXIT( (&fd) ) {
      close(fd);
    } BOOST_SCOPE_EXIT_END

    CHECK_NE(fd, -1) << "File not found: " << proto_path;
    boost::shared_ptr<FileInputStream> input(new FileInputStream(fd));

    return google::protobuf::TextFormat::Parse(input.get(), proto);
  }

  void writeProtoToTextFile(const std::string& proto_path, const Message& proto) {
    int fd = open(proto_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    BOOST_SCOPE_EXIT( (&fd) ) {
      close(fd);
    } BOOST_SCOPE_EXIT_END

    boost::shared_ptr<FileOutputStream> output(new FileOutputStream(fd));
    CHECK(google::protobuf::TextFormat::Print(proto, output.get()));
  }

  bool readProtoFromBinaryFile(const std::string& proto_path, Message* proto) {
    int fd = open(proto_path.c_str(), O_RDONLY);
    BOOST_SCOPE_EXIT( (&fd) ) {
      close(fd);
    } BOOST_SCOPE_EXIT_END

    CHECK_NE(fd, -1) << "File not found: " << proto_path;
    boost::shared_ptr<ZeroCopyInputStream> raw_input(new FileInputStream(fd));
    boost::shared_ptr<CodedInputStream> coded_input(new CodedInputStream(raw_input.get()));
    coded_input->SetTotalBytesLimit(1073741824, 536870912);

    return proto->ParseFromCodedStream(coded_input.get());
  }

  void writeProtoToBinaryFile(const std::string& proto_path, const Message& proto) {
    std::fstream output(proto_path.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    CHECK(proto.SerializeToOstream(&output));
  }
}
