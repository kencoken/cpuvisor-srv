////////////////////////////////////////////////////////////////////////////
//    File:        zmq_server.h
//    Author:      Ken Chatfield
//    Description: Lightweight server for CPU Visor using ZMQ
////////////////////////////////////////////////////////////////////////////

#ifndef CPUVISOR_ZMQ_SERVER_H_
#define CPUVISOR_ZMQ_SERVER_H_

#include "server/common/generic_zmq_server.h"

#include "server/common/proto_parse.h"
#include "visor_config.pb.h"
#include "visor_srv.pb.h"

#include "server/common/common.h"
#include "server/cpuvisor/base_server.h"

using namespace visor;

namespace cpuvisor {

  // utility functor specializations ----

  class BaseServerIndexToId: public visor::IndexToId {
  public:
    BaseServerIndexToId(boost::shared_ptr<BaseServer> base_server)
      : base_server_(base_server) { }
    virtual std::string operator()(const size_t index) const {
      return base_server_->dset_path(index);
    }
  protected:
    boost::shared_ptr<BaseServer> base_server_;
  };

  // class definition --------------------

  class ZmqServer : GenericZmqServer {
  public:
    ZmqServer(const visor::Config& config);
    virtual ~ZmqServer();

    inline virtual void serve(const bool blocking=true) {
      start_serving_(blocking);
    }

  protected:
    virtual bool dispatch_handler_(const RPCReq& rpc_req, RPCRep* rpc_rep);

    virtual void monitor_state_change_();
    virtual void monitor_add_trs_images_();
    virtual void monitor_add_trs_complete_();
    virtual void monitor_errors_();

    static void getAnnotations_(const std::vector<std::string>& paths,
                                const std::vector<int32_t>& annos,
                                RPCRep* rpc_rep);

    visor::Config config_;

    boost::shared_ptr<BaseServer> base_server_;

    boost::shared_ptr<boost::thread> monitor_state_change_thread_;
    boost::shared_ptr<boost::thread> monitor_add_trs_images_thread_;
    boost::shared_ptr<boost::thread> monitor_add_trs_complete_thread_;
    boost::shared_ptr<boost::thread> monitor_errors_thread_;

  };

}

#endif
