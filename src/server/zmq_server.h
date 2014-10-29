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

#include "server/base_server.h"

namespace cpuvisor {

  class ZmqServer : boost::noncopyable {
  public:
    ZmqServer(const cpuvisor::Config& config);
    virtual ~ZmqServer();

    virtual void serve(const bool blocking=true);

  protected:
    virtual void serve_();
    virtual RPCRep dispatch_(RPCReq rpc_req);

    virtual void getRankingPage_(const Ranking& ranking,
                                 const RPCReq& rpc_req, RPCRep* rpc_rep);

    virtual void monitor_state_change_();
    virtual void monitor_add_trs_images_();
    virtual void monitor_add_trs_complete_();

    Config config_;

    boost::shared_ptr<boost::thread> serve_thread_;
    boost::shared_ptr<BaseServer> base_server_;

    boost::shared_ptr<boost::thread> monitor_state_change_thread_;
    boost::shared_ptr<boost::thread> monitor_add_trs_images_thread_;
    boost::shared_ptr<boost::thread> monitor_add_trs_complete_thread_;

    boost::shared_ptr<zmq::socket_t> notify_socket_;
  };

}

#endif
