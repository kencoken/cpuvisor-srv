#include "proto_parse.h"

#include <glog/logging.h>

namespace visor {

  namespace protoparse {

    cpuvisor::NotificationType get_notification_type(const visor::VisorNotification& notification) {
      const AnyEnum& notification_type_any = notification.notification_type();
      CHECK_EQ(notification_type_any.type_url(), "cpuvisor::notification_type");

      cpuvisor::NotificationType notification_type =
        cpuvisor::NotificationType(notification_type_any.value());

      return notification_type;
    }

    void set_notification_type(const cpuvisor::NotificationType& notification_type,
                               visor::VisorNotification* notification) {
      AnyEnum* mutable_notification_type_any = notification->mutable_notification_type();
      mutable_notification_type_any->set_type_url("cpuvisor::notification_type");

      mutable_notification_type_any->set_value(notification_type);
    }

  }

}
