////////////////////////////////////////////////////////////////////////////
//    File:        concurrent_queue_single_sub.h
//    Author:      Ken Chatfield
//    Description: Concurrent queue which only allows one subscriber
////////////////////////////////////////////////////////////////////////////

#ifndef FEATPIPE_CONCURRENT_QUEUE_SINGLE_SUB_H_
#define FEATPIPE_CONCURRENT_QUEUE_SINGLE_SUB_H_

#include <string>
#include <stdexcept>
#include "concurrent_queue.h"

namespace featpipe {

  class AlreadyHasSubscriberError: public std::runtime_error {
  public:
    AlreadyHasSubscriberError(std::string const& msg): std::runtime_error(msg) { }
  };

  template<typename Data>
  class ConcurrentQueueSingleSub {
  protected:
    ConcurrentQueue<Data> concurrent_queue_;
    mutable boost::mutex sub_mutex_;
    size_t sub_count_;

    void incSubCount_() {
      boost::mutex::scoped_lock lock(sub_mutex_);

      if (sub_count_ > 0) {
        throw AlreadyHasSubscriberError("Cannot subscribe more than once to notifications from ConcurrentQueueSingleSub!");
      }
      sub_count_ += 1;
    }
    void decSubCount_() {
      boost::mutex::scoped_lock lock(sub_mutex_);

      sub_count_ -=1;
    }
  public:
    ConcurrentQueueSingleSub(): sub_count_(0) {}

    void push(Data const& data) {
      concurrent_queue_.push(data);
    }

    bool empty() const {
      return concurrent_queue_.empty();
    }

    bool tryPop(Data& popped_value) {
      incSubCount_();
      bool retval = concurrent_queue_.tryPop(popped_value);
      decSubCount_();
      return retval;
    }

    void waitAndPop(Data& popped_value) {
      incSubCount_();
      concurrent_queue_.waitAndPop(popped_value);
      decSubCount_();
    }

  };

}

#endif
