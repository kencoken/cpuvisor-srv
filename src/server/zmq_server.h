////////////////////////////////////////////////////////////////////////////
//    File:        zmq_server.h
//    Author:      Ken Chatfield
//    Description: Lightweight server for CPU Visor using ZMQ
////////////////////////////////////////////////////////////////////////////

#ifndef CPUVISOR_ZMQ_SERVER_H_
#define CPUVISOR_ZMQ_SERVER_H_

#include "generic_zmq_server.h"

#include "server/common/proto_parse.h"
#include "visor_config.pb.h"
#include "cpuvisor_config.pb.h"
#include "visor_srv.pb.h"
#include "cpuvisor_srv.pb.h"

#include "server/base_server.h"

using namespace visor;

namespace cpuvisor {

  class ZmqServer : GenericZmqServer {
  public:
    ZmqServer(const Config& config);
    virtual ~ZmqServer();

  protected:
    virtual bool dispatch_handler_(const RPCReq& rpc_req, RPCRep* rpc_rep);

    virtual void getAnnotations_(const std::vector<std::string>& paths,
                                 const std::vector<int32_t>& annos,
                                 RPCRep* rpc_rep);

    virtual void monitor_state_change_();
    virtual void monitor_add_trs_images_();
    virtual void monitor_add_trs_complete_();
    virtual void monitor_errors_();

    Config config_;

    boost::shared_ptr<BaseServer> base_server_;

    boost::shared_ptr<boost::thread> monitor_state_change_thread_;
    boost::shared_ptr<boost::thread> monitor_add_trs_images_thread_;
    boost::shared_ptr<boost::thread> monitor_add_trs_complete_thread_;
    boost::shared_ptr<boost::thread> monitor_errors_thread_;

  };

}

#endif
