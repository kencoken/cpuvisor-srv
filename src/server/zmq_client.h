////////////////////////////////////////////////////////////////////////////
//    File:        zmq_client.h
//    Author:      Ken Chatfield
//    Description: Lightweight client for CPU Visor using ZMQ
//                 (not fully featured - used only for incremental
//                  indexing for now)
////////////////////////////////////////////////////////////////////////////

#ifndef CPUVISOR_ZMQ_CLIENT_H_
#define CPUVISOR_ZMQ_CLIENT_H_

#include <string>
#include <glog/logging.h>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <zmq.hpp>

#include "visor_config.pb.h"
#include "visor_srv.pb.h"

namespace cpuvisor {

  class ZmqClient : boost::noncopyable {
  public:
    ZmqClient(const visor::Config& config,
              boost::shared_ptr<zmq::context_t> context = boost::shared_ptr<zmq::context_t>());
    virtual ~ZmqClient();

    virtual void addDsetImagesToIndex(const std::vector<std::string>& dset_paths);

  protected:
    visor::Config config_;

    boost::shared_ptr<zmq::context_t> context_;
    boost::shared_ptr<zmq::socket_t> socket_;
  };

}

#endif
