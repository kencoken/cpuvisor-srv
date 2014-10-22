#include "status_notifier.h"

namespace cpuvisor {

  QueryStateChangeNotification StatusNotifier::wait_state_change() {
    QueryStateChangeNotification notification;

    state_notify_queue_.waitAndPop(notification);
    return notification;
  }

  QueryImageProcessedNotification StatusNotifier::wait_image_processed() {
    QueryImageProcessedNotification notification;

    image_notify_queue_.waitAndPop(notification);
    return notification;
  }

  QueryAllImagesProcessedNotification StatusNotifier::wait_all_images_processed() {
    QueryAllImagesProcessedNotification notification;

    allimages_notify_queue_.waitAndPop(notification);
    return notification;
  }

  void StatusNotifier::post_state_change_(const std::string& id,
                                          const QueryState new_state) {
    QueryStateChangeNotification notification;
    notification.id = id;
    notification.new_state = new_state;

    state_notify_queue_.push(notification);
  }

  void StatusNotifier::post_image_processed_(const std::string& id,
                                             const std::string& fname) {
    QueryImageProcessedNotification notification;
    notification.id = id;
    notification.fname = fname;

    image_notify_queue_.push(notification);
  }

  void StatusNotifier::post_all_images_processed_(const std::string& id) {
    QueryAllImagesProcessedNotification notification;
    notification.id = id;

    allimages_notify_queue_.push(notification);
  }

}
