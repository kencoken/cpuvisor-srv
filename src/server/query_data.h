////////////////////////////////////////////////////////////////////////////
//    File:        query_data.h
//    Author:      Ken Chatfield
//    Description: Data associated with a query
////////////////////////////////////////////////////////////////////////////

#ifndef CPUVISOR_QUERY_DATA_H_
#define CPUVISOR_QUERY_DATA_H_

namespace cpuvisor {

  struct Ritem {
    std::string path;
    float score;
  };

  enum QueryState {QS_DATACOLL, QS_TRAINING, QS_TRAINED, QS_RANKING, QS_RANKED};

  struct QueryData {
    cv::Mat pos_feats;
    cv::Mat model;
    std::vector<Ritem> rlist;
  };

  struct QueryIfo {
    QueryIfo(const std::string& id) : id(id)
                                    , state(QS_DATACOLL) {}
    std::string id;
    QueryState state;
    QueryData data;
  };

}

#endif
