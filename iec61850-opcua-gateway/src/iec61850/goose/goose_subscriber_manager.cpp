#include "goose_subscriber_manager.h"
#include "core/logger.h"

namespace gateway {
namespace iec61850 {
namespace goose {

GOOSESubscriberManager::GOOSESubscriberManager(
    std::shared_ptr<GOOSEReceiver> receiver)
    : receiver_(receiver) {}

GOOSESubscriberManager::~GOOSESubscriberManager() {
  for (auto &pair : subscribers_) {
    GooseSubscriber_destroy(pair.second);
  }
  subscribers_.clear();
}

void GOOSESubscriberManager::subscribe(const std::string &goCbRef) {
  if (subscribers_.find(goCbRef) != subscribers_.end()) {
    LOG_WARN("Already subscribed to GOOSE: {}", goCbRef.c_str());
    return;
  }

  GooseSubscriber subscriber =
      GooseSubscriber_create((char *)goCbRef.c_str(), NULL);
  if (subscriber) {
    GooseSubscriber_setListener(subscriber, onGooseMessage, this);
    receiver_->addSubscriber(subscriber);
    subscribers_[goCbRef] = subscriber;
    LOG_INFO("Subscribed to GOOSE: {}", goCbRef.c_str());
  } else {
    LOG_ERROR("Failed to create subscriber for GOOSE: {}", goCbRef.c_str());
  }
}

void GOOSESubscriberManager::unsubscribe(const std::string &goCbRef) {
  auto it = subscribers_.find(goCbRef);
  if (it != subscribers_.end()) {
    receiver_->removeSubscriber(it->second);
    GooseSubscriber_destroy(it->second);
    subscribers_.erase(it);
    LOG_INFO("Unsubscribed from GOOSE: {}", goCbRef.c_str());
  }
}

void GOOSESubscriberManager::onGooseMessage(GooseSubscriber subscriber,
                                            void *parameter) {
  (void)parameter; // Unused

  uint32_t stNum = GooseSubscriber_getStNum(subscriber);
  uint32_t sqNum = GooseSubscriber_getSqNum(subscriber);
  (void)stNum; // Unused for now
  (void)sqNum; // Unused for now

  // LOG_TRACE("Received GOOSE: stNum={}, sqNum={}", stNum, sqNum);

  MmsValue *values = GooseSubscriber_getDataSetValues(subscriber);
  if (values) {
    // TODO: Process dataset values and update internal model
    // For now, just log that we got data
    // LOG_DEBUG("GOOSE payload received");
  }
}

} // namespace goose
} // namespace iec61850
} // namespace gateway
