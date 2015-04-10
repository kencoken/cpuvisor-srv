#include "uber_classifiers.h"

#include <fstream>
#include <boost/algorithm/string.hpp>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <glog/logging.h>

#include "io.h"

namespace cpuvisor {

  void UberClassifiers::loadFromIndexFile(const std::string& index_file,
                                          const std::string& base_path) {

    boost::mutex::scoped_lock(add_mutex_);

    std::ifstream fin(index_file);

    query_strs_.empty();
    models_ = cv::Mat();

    std::string line;
    std::vector<std::string> tokens;
    std::vector<std::string> synonyms;
    while (std::getline(fin, line)) {

      boost::split(tokens, line, boost::is_any_of(":"));
      CHECK_EQ(tokens.size(), 2);
      const std::string& proto_path = tokens[0];
      const std::string& query_strs = tokens[1];

      std::string proto_path_full;
      if (!base_path.empty()) {
        fs::path proto_path_full_fs = fs::path(base_path) / fs::path(proto_path);
        proto_path_full = proto_path_full_fs.string();
      } else {
        proto_path_full = proto_path;
      }

      boost::split(synonyms, query_strs, boost::is_any_of(","));
      for (auto& synonym : synonyms) {
        boost::algorithm::trim(synonym);
      }

      cv::Mat model;
      readModelFromProto(proto_path_full, &model);

      if (models_.rows > 0) {
        CHECK_EQ(models_.cols, model.cols);
      }
      models_.push_back(model);
      int model_idx = models_.rows-1;

      for (auto& synonym : synonyms) {
        query_strs_[synonym] = model_idx;
      }

    }

  }

  std::set<std::string> UberClassifiers::getAvailableClassifiers() {

    // NamedClassifierIndex index;
    // for (auto& model : models_) {
    //   index.add_query_strs(models_->first);
    // }
    std::set<std::string> query_strs;
    for (auto& query_str_pair : query_strs_) {
      query_strs.insert(query_str_pair.first);
    }

    return query_strs;
  }

}
