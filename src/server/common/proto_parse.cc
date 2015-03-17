#include "proto_parse.h"

namespace visor {

  namespace protoparse {

    cpuvisor::ConfigExt get_config_ext(const visor::Config& config) {
      const Any& modality_config = config.modality_config();
      CHECK_EQ(config.type_url(), "cpuvisor::config_ext");

      cpuvisor::ConfigExt config_ext = cpuvisor::ConfigExt();
      config_ext.ParseFromString(modality_config.value());
      return config_ext;
    }

    void set_config_ext(const cpuvisor::ConfigExt& config_ext, visor::Config* config) {
      Any* mutable_modality_config = config->mutable_modality_config();
      mutable_modality_config->set_type_url("cpuvisor::config_ext");

      std::string value;
      CHECK(config_ext.SerializeToString(&value));

      mutable_modality_config->set_value(value);
    }

    // --

    cpuvisor::PreprocConfigExt get_preproc_config_ext(const visor::PreprocConfig& preproc_config) {
      const Any& modality_preproc_config = config.modality_preproc_config();
      CHECK_EQ(config.type_url(), "cpuvisor::preproc_config_ext");

      cpuvisor::PreprocConfigExt preproc_config_ext = cpuvisor::PreprocConfigExt();
      preproc_config_ext.ParseFromString(modality_preproc_config.value());
      return preproc_config_ext;
    }

    void set_preproc_config_ext(const cpuvisor::PreprocConfigExt& preproc_config_ext,
                                visor::PreprocConfig* preproc_config) {
      Any* mutable_modality_preproc_config = preproc_config->mutable_modality_preproc_config();
      mutable_modality_preproc_config->set_type_url("cpuvisor::preproc_config_ext");

      std::string value;
      CHECK(preproc_config_ext.SerializeToString(&value));

      mutable_modality_preproc_config->set_value(value);
    }

    // --

    cpuvisor::ServiceConfigExt get_service_config_ext(const visor::ServiceConfig& service_config) {
      const Any& modality_service_config = config.modality_service_config();
      CHECK_EQ(config.type_url(), "cpuvisor::service_config_ext");

      cpuvisor::ServiceConfigExt service_config_ext = cpuvisor::ServiceConfigExt();
      service_config_ext.ParseFromString(modality_service_config.value());
      return service_config_ext;
    }

    void set_service_config_ext(const cpuvisor::ServiceConfigExt& service_config_ext,
                                visor::ServiceConfig* service_config) {
      Any* mutable_modality_service_config = service_config->mutable_modality_service_config();
      mutable_modality_service_config->set_type_url("cpuvisor::service_config_ext");

      std::string value;
      CHECK(service_config_ext.SerializeToString(&value));

      mutable_modality_service_config->set_value(value);
    }

    // --

    cpuvisor::ServerConfigExt get_server_config_ext(const visor::ServerConfig& server_config) {
      const Any& modality_server_config = config.modality_server_config();
      CHECK_EQ(config.type_url(), "cpuvisor::server_config_ext");

      cpuvisor::ServerConfigExt server_config_ext = cpuvisor::ServerConfigExt();
      server_config_ext.ParseFromString(modality_server_config.value());
      return server_config_ext;
    }

    void set_server_config_ext(const cpuvisor::ServerConfigExt& server_config_ext,
                               visor::ServerConfig* server_config) {
      Any* mutable_modality_server_config = server_config->mutable_modality_server_config();
      mutable_modality_server_config->set_type_url("cpuvisor::server_config_ext");

      std::string value;
      CHECK(server_config_ext.SerializeToString(&value));

      mutable_modality_server_config->set_value(value);
    }

    // ------------------

    cpuvisor::RPCReqExt get_rpc_req_ext(const visor::RPCReq& rpc_req) {
      const Any& modality_data = config.modality_data();
      CHECK_EQ(config.type_url(), "cpuvisor::rpc_req_ext");

      cpuvisor::RPCReqExt rpc_req_ext = cpuvisor::RPCReqExt();
      rpc_req_ext.ParseFromString(modality_data.value());
      return rpc_req_ext;
    }

    void set_rpc_req_ext(const cpuvisor::RPCReqExt& rpc_req_ext,
                         visor::RPCReq* rpc_req) {
      Any* mutable_modality_data = rpc_req->mutable_modality_data();
      mutable_modality_data->set_type_url("cpuvisor::rpc_req_ext");

      std::string value;
      CHECK(rpc_req_ext.SerializeToString(&value));

      mutable_modality_data->set_value(value);
    }

    // --

    cpuvisor::RPCRepExt get_rpc_rep_ext(const visor::RPCRep& rpc_rep) {
      const Any& modality_data = config.modality_data();
      CHECK_EQ(config.type_url(), "cpuvisor::rpc_rep_ext");

      cpuvisor::RPCRepExt rpc_rep_ext = cpuvisor::RPCRepExt();
      rpc_rep_ext.ParseFromString(modality_data.value());
      return rpc_rep_ext;
    }

    void set_rpc_rep_ext(const cpuvisor::RPCRepExt& rpc_rep_ext,
                         cpuvisor::RPCRep* rpc_rep) {
      Any* mutable_modality_data = rpc_rep->mutable_modality_data();
      mutable_modality_data->set_type_url("cpuvisor::rpc_rep_ext");

      std::string value;
      CHECK(rpc_rep_ext.SerializeToString(&value));

      mutable_modality_data->set_value(value);
    }

    void set_notification_type(const cpuvisor::NotificationType& notification_type,
                               visor::VisorNotification* notification) {
      Any* mutable_notification_type = notification->mutable_notification_type();
      mutable_notification_type->set_type_url("cpuvisor::notification_type");

      std::string value;
      CHECK(notification_type.SerializeToString(&value));

      mutable_notification_type->set_value(value);
    }

  }

}
