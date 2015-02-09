////////////////////////////////////////////////////////////////////////////
//    File:        tictoc.h
//    Author:      Ken Chatfield
//                 ken@robots.ox.ac.uk
//                 All rights reserved - not for public distribution
//    Description : Utilities for timing C/C++ code
////////////////////////////////////////////////////////////////////////////

#ifndef FEATPIPE_TICTOC_H_
#define FEATPIPE_TICTOC_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>

std::string getTimestamp();

inline std::string getTimestamp() {
  boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
  return boost::posix_time::to_simple_string(time);
  return "";
}

// tic-toc ----------------------------------

struct TicTocObj {
  boost::posix_time::ptime start_time;
};

inline TicTocObj tic() {
  TicTocObj ttobj;
  ttobj.start_time = boost::posix_time::microsec_clock::local_time();
  return ttobj;
}

inline float toc(TicTocObj ttobj) {
  boost::posix_time::ptime end_time = boost::posix_time::microsec_clock::local_time();
  boost::posix_time::time_duration diff = end_time - ttobj.start_time;
  return static_cast<float>(diff.total_milliseconds())/1000.0;
}

#endif
