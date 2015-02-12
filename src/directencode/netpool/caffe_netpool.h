////////////////////////////////////////////////////////////////////////////
//    File:        caffe_netpool.h
//    Author:      Ken Chatfield
//    Description: Netpool for VGG Caffe
////////////////////////////////////////////////////////////////////////////

#ifndef FEATPIPE_CAFFE_NETPOOL_H_
#define FEATPIPE_CAFFE_NETPOOL_H_

#include <vector>

#include "caffe_netinst.h"

#include "cpuvisor_config.pb.h"

namespace featpipe {

  class CaffeNetPool : boost::noncopyable {
  public:
    CaffeNetPool(const CaffeConfig& config, const size_t pool_sz_override = 0)
      : config_(config)
      , ready_net_cond_var_(boost::shared_ptr<boost::condition_variable>(new boost::condition_variable)) {
      uint32_t pool_sz = config_.netpool_sz;
      if (pool_sz_override > 0) {
        pool_sz = pool_sz_override;
      }

      CHECK_GE(pool_sz, 1);
      for (size_t i = 0; i < pool_sz; ++i) {
        nets_.push_back(boost::shared_ptr<CaffeNetInst>(new CaffeNetInst(config_, ready_net_cond_var_)));
      }
    }

    // main functions
    virtual boost::shared_ptr<CaffeNetInst> getReadyNet();
    // virtual setter / getter
    inline size_t get_code_size() const {
      boost::shared_ptr<CaffeNetInst> net = nets_[0];
      return net->get_code_size();
    }
  protected:
    CaffeConfig config_;
    std::vector<boost::shared_ptr<CaffeNetInst> > nets_;

    boost::shared_ptr<boost::condition_variable> ready_net_cond_var_;
    boost::mutex ready_net_mutex_;
  };

}

#endif
