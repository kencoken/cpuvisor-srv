////////////////////////////////////////////////////////////////////////////
//    File:        generic_zmq_server.h
//    Author:      Ken Chatfield
//    Description: Lightweight ZMQ server ABC
////////////////////////////////////////////////////////////////////////////

#ifndef VISOR_GENERIC_ZMQ_SERVER_H_
#define VISOR_GENERIC_ZMQ_SERVER_H_

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>

#include <zmq.hpp>

#include "server/common/common.h"
#include "visor_config.pb.h"
#include "visor_srv.pb.h"

namespace visor {

  class GenericZmqServer : boost::noncopyable {
  public:
    GenericZmqServer(const Config& config);
    virtual ~GenericZmqServer();

    // ABC methods
    virtual void serve(const bool blocking=true) = 0;

  protected:
    // ABC methods
    virtual bool dispatch_handler_(const RPCReq& rpc_req, RPCRep* rpc_rep) = 0;

    // other methods
    virtual void start_serving_(const bool blocking=true);
    virtual void serve_();
    virtual RPCRep dispatch_(const RPCReq& rpc_req);
    virtual void notify_(const VisorNotification& notify_proto);

    virtual void getRankingPage_(const Ranking& ranking,
                                 const boost::shared_ptr<IndexToId> converter,
                                 const RPCReq& rpc_req, RPCRep* rpc_rep) const;
    virtual void getRankingProto_(const Ranking& ranking,
                                  const boost::shared_ptr<IndexToId> converter,
                                  RankedList* ranking_proto,
                                  const size_t page_sz = -1,
                                  const size_t page_num = 0) const;

    Config config_;

    boost::shared_ptr<boost::thread> serve_thread_;
    boost::shared_ptr<zmq::socket_t> notify_socket_;
  };

}

#endif
