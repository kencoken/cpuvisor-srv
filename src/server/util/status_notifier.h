////////////////////////////////////////////////////////////////////////////
//    File:        status_notifier.h
//    Author:      Ken Chatfield
//    Description: Notifier class for changes in state of base server
////////////////////////////////////////////////////////////////////////////

#ifndef CPUVISOR_STATUS_NOTIFIER_H_
#define CPUVISOR_STATUS_NOTIFIER_H_

#include <string>
#include <boost/utility.hpp>

#include "server/util/concurrent_queue_single_sub.h"

#include "server/query_data.h"

namespace cpuvisor {

  class BaseServer;
  class BaseServerPostProcessor;
  class BaseServerCallback;

  struct QueryStateChangeNotification {
    std::string id;
    QueryState new_state;
  };

  struct QueryImageProcessedNotification {
    std::string id;
    std::string fname;
  };

  struct QueryAllImagesProcessedNotification {
    std::string id;
  };

  class StatusNotifier : boost::noncopyable {
    friend class BaseServer;
    friend class BaseServerPostProcessor;
    friend class BaseServerCallback;

  public:
    QueryStateChangeNotification wait_state_change();
    QueryImageProcessedNotification wait_image_processed();
    QueryAllImagesProcessedNotification wait_all_images_processed();
  protected:
    void post_state_change_(const std::string& id,
                              const QueryState new_state);
    void post_image_processed_(const std::string& id,
                               const std::string& fname);
    void post_all_images_processed_(const std::string& id);

    featpipe::ConcurrentQueueSingleSub<QueryStateChangeNotification> state_notify_queue_;
    featpipe::ConcurrentQueueSingleSub<QueryImageProcessedNotification> image_notify_queue_;
    featpipe::ConcurrentQueueSingleSub<QueryAllImagesProcessedNotification> allimages_notify_queue_;
  };

}

#endif
