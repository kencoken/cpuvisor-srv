////////////////////////////////////////////////////////////////////////////
//    File:        proto_parse.h
//    Author:      Ken Chatfield
//    Description: Helpers for parsing prototxt
////////////////////////////////////////////////////////////////////////////

#ifndef VISOR_PROTO_PARSE_H_
#define VISOR_PROTO_PARSE_H_

#include "visor_common.pb.h"
#include "visor_srv.pb.h"
#include "cpuvisor_srv.pb.h"

namespace visor {

  namespace protoparse {

    cpuvisor::NotificationType get_notification_type(const visor::VisorNotification& notification);
    void set_notification_type(const cpuvisor::NotificationType& notification_type,
                               visor::VisorNotification* notification);

  }

}

#endif
