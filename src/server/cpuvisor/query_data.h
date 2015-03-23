////////////////////////////////////////////////////////////////////////////
//    File:        query_data.h
//    Author:      Ken Chatfield
//    Description: Data associated with a query
////////////////////////////////////////////////////////////////////////////

#ifndef CPUVISOR_QUERY_DATA_H_
#define CPUVISOR_QUERY_DATA_H_

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

#include <boost/thread.hpp>

#include "server/common/common.h"

namespace cpuvisor {

  enum QueryState {QS_DATACOLL, QS_DATACOLL_COMPLETE,
                   QS_TRAINING, QS_TRAINED,
                   QS_RANKING, QS_RANKED};

  struct QueryData {
    cv::Mat pos_feats;
    std::vector<std::string> pos_paths; // for debugging
    boost::mutex pos_mutex; // to ensure features are added in thread-safe manner
    cv::Mat model;
    visor::Ranking ranking;
  };

  struct QueryIfo {
    QueryIfo() : state(QS_DATACOLL) {}
    QueryIfo(const std::string& id,
             const std::string& tag = std::string())
      : id(id)
      , tag(tag.empty() ? id : tag)
      , state(QS_DATACOLL) { }
    std::string id;
    std::string tag;
    QueryState state;
    QueryData data;
  };

}

#endif
