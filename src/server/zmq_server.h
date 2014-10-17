////////////////////////////////////////////////////////////////////////////
//    File:        zmq_server.h
//    Author:      Ken Chatfield
//    Description: Lightweight server for CPU Visor using ZMQ
////////////////////////////////////////////////////////////////////////////

#ifndef CPUVISOR_ZMQ_SERVER_H_
#define CPUVISOR_ZMQ_SERVER_H_

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>

#include <zmq.hpp>

#include "cpuvisor_config.pb.h"
#include "cpuvisor_srv.pb.h"

namespace cpuvisor {

  class ZmqServer : boost::noncopyable {
  public:
    ZmqServer(const cpuvisor::Config& config);
    virtual ~ZmqServer();

    virtual void serve(const bool blocking=true);

  protected:
    boost::shared_ptr<boost::thread> serve_thread_;

    virtual void serve_();
  };

}

#endif
