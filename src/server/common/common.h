////////////////////////////////////////////////////////////////////////////
//    File:        common.h
//    Author:      Ken Chatfield
//    Description: Definitions shared across VISOR services
////////////////////////////////////////////////////////////////////////////

#ifndef VISOR_SERVER_COMMON_H_
#define VISOR_SERVER_COMMON_H_

#include <opencv2/opencv.hpp>
#include <stdexcept>

namespace visor {

  struct Ranking {
    cv::Mat scores;
    cv::Mat sort_idxs;
  };

  class IndexToId {
  public:
    virtual std::string operator()(const size_t index) const = 0;
  };

  // exceptions --------------------------

  class InvalidRequestError: public std::runtime_error {
  public:
    InvalidRequestError(std::string const& msg): std::runtime_error(msg) { }
  };

}

#endif
