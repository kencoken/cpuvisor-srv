////////////////////////////////////////////////////////////////////////////
//    File:        proto_parse.h
//    Author:      Ken Chatfield
//    Description: Helpers for parsing prototxt
//      (particularly 'Any' type - which is supported natively in
//       protoc 3.0 - the need for all of these routines should be
//       removed when switching over)
////////////////////////////////////////////////////////////////////////////

#ifndef VISOR_PROTO_PARSE_H_
#define VISOR_PROTO_PARSE_H_

#include "visor_common.pb.h"

#include "visor_config.pb.h"
#include "cpuvisor_config.pb.h"
#include "visor_srv.pb.h"
#include "cpuvisor_srv.pb.h"

namespace visor {

  namespace protoparse {

    cpuvisor::ConfigExt get_config_ext(const visor::Config& config);
    void set_config_ext(const cpuvisor::ConfigExt& config_ext, visor::Config* config);

    cpuvisor::PreprocConfigExt get_preproc_config_ext(const visor::PreprocConfig& preproc_config);
    void set_preproc_config_ext(const cpuvisor::PreprocConfigExt& preproc_config_ext,
                                visor::PreprocConfig* preproc_config);

    cpuvisor::ServiceConfigExt get_service_config_ext(const visor::ServiceConfig& service_config);
    void set_service_config_ext(const cpuvisor::ServiceConfigExt& service_config_ext,
                                visor::ServiceConfig* service_config);

    cpuvisor::ServerConfigExt get_server_config_ext(const visor::ServerConfig& server_config);
    void set_server_config_ext(const cpuvisor::ServerConfigExt& server_config_ext,
                               visor::ServerConfig* server_config);

    cpuvisor::RPCReqExt get_rpc_req_ext(const visor::RPCReq& rpc_req);
    void set_rpc_req_ext(const cpuvisor::RPCReqExt& rpc_req_ext,
                         visor::RPCReq* rpc_req);

    cpuvisor::RPCRepExt get_rpc_rep_ext(const visor::RPCRep& rpc_rep);
    void set_rpc_rep_ext(const cpuvisor::RPCRepExt& rpc_rep_ext,
                         cpuvisor::RPCRep* rpc_rep);

    void set_notification_type(const cpuvisor::NotificationType& type,
                               visor::VisorNotification* notification);



  }

}

#endif
