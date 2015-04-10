////////////////////////////////////////////////////////////////////////////
//    File:        uber_classifiers.h
//    Author:      Ken Chatfield
//    Description: Class for handling loading of uber-classifiers
////////////////////////////////////////////////////////////////////////////

#ifndef FEATPIPE_UBER_CLASSIFIERS_H_
#define FEATPIPE_UBER_CLASSIFIERS_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <set>

#include <boost/thread.hpp>
#include <opencv2/opencv.hpp>

namespace cpuvisor {

  class UberClassifiers {
  public:
    UberClassifiers() {}
    UberClassifiers(const std::string& index_file, const std::string& base_path = "") {
      loadFromIndexFile(index_file, base_path);
    }

    virtual void loadFromIndexFile(const std::string& index_file,
                                   const std::string& base_path = "");

    inline virtual bool hasClassifier(const std::string& query_str) {
      return (query_strs_.find(query_str) != query_strs_.end());
    }
    inline virtual cv::Mat getClassifier(const std::string& query_str) {
      return models_.row(query_strs_[query_str]);
    }

    virtual std::set<std::string> getAvailableClassifiers();

  protected:
    cv::Mat models_;
    std::unordered_map<std::string, size_t> query_strs_;

    boost::mutex add_mutex_;
  };

}

#endif
