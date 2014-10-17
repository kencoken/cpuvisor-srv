////////////////////////////////////////////////////////////////////////////
//    File:        concurrent_queue.h
//    Author:      Ken Chatfield
//    Description: Thread-safe concurrent queue class
////////////////////////////////////////////////////////////////////////////

#ifndef FEATPIPE_CONCURRENT_QUEUE_H_
#define FEATPIPE_CONCURRENT_QUEUE_H_

#include <boost/thread.hpp>

namespace featpipe {

  template<typename Data>
  class ConcurrentQueue {
  protected:
    std::queue<Data> queue_;
    mutable boost::mutex mutex_;
    boost::condition_variable cond_var_;
  public:
    void push(Data const& data) {
      boost::mutex::scoped_lock lock(mutex_);
      queue_.push(data);
      lock.unlock();
      cond_var_.notify_one();
    }

    bool empty() const {
      boost::mutex::scoped_lock lock(mutex_);
      return queue_.empty();
    }

    bool tryPop(Data& popped_value) {
      boost::mutex::scoped_lock lock(mutex_);
      if(queue_.empty()) {
        return false;
      }

      popped_value = queue_.front();
      queue_.pop();
      return true;
    }

    void waitAndPop(Data& popped_value) {
      boost::mutex::scoped_lock lock(mutex_);
      while(queue_.empty()) {
        cond_var_.wait(lock);
      }

      popped_value = queue_.front();
      queue_.pop();
    }

  };

}

#endif
