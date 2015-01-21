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

  struct QueryErrorNotification {
    std::string id;
    std::string err_msg;
  };

  class StatusNotifier : boost::noncopyable {
    friend class BaseServer;
    friend class BaseServerPostProcessor;
    friend class BaseServerCallback;

  public:
    QueryStateChangeNotification wait_state_change();
    QueryImageProcessedNotification wait_image_processed();
    QueryAllImagesProcessedNotification wait_all_images_processed();
    QueryErrorNotification wait_error();
  protected:
    void post_state_change_(const std::string& id,
                            const QueryState new_state);
    void post_image_processed_(const std::string& id,
                               const std::string& fname);
    void post_all_images_processed_(const std::string& id);
    void post_error_(const std::string& id,
                     const std::string& err_msg);

    featpipe::ConcurrentQueueSingleSub<QueryStateChangeNotification> state_notify_queue_;
    featpipe::ConcurrentQueueSingleSub<QueryImageProcessedNotification> image_notify_queue_;
    featpipe::ConcurrentQueueSingleSub<QueryAllImagesProcessedNotification> allimages_notify_queue_;
    featpipe::ConcurrentQueueSingleSub<QueryErrorNotification> error_notify_queue_;
  };

}

#endif
