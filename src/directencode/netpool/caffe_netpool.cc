#include "caffe_netpool.h"

namespace featpipe {

  boost::shared_ptr<CaffeNetInst> CaffeNetPool::getReadyNet() {

    boost::mutex::scoped_lock ready_net_lock_(ready_net_mutex_);

    while (true) {

      // scan nets_ vector for a network which is currently ready
      for (size_t i = 0; i < nets_.size(); ++i) {
        if (nets_[i]->ready()) {
          nets_[i]->set_ready(false);
          return nets_[i];
        }
      }

      // if not found, wait on condition variable (which should be
      // posted to by any networks transitioning to ready state)
      ready_net_cond_var_->wait(ready_net_lock_);
    }

  }

}
