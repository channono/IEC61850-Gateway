#include "goose_receiver.h"
#include "core/logger.h"

namespace gateway {
namespace iec61850 {
namespace goose {

GOOSEReceiver::GOOSEReceiver(const std::string &interfaceName)
    : interfaceName_(interfaceName) {
  receiver_ = GooseReceiver_create();
  GooseReceiver_setInterfaceId(receiver_, interfaceName_.c_str());
}

GOOSEReceiver::~GOOSEReceiver() {
  stop();
  if (receiver_) {
    GooseReceiver_destroy(receiver_);
    receiver_ = nullptr;
  }
}

void GOOSEReceiver::start() {
  if (running_)
    return;

  GooseReceiver_start(receiver_);
  if (GooseReceiver_isRunning(receiver_)) {
    running_ = true;
    LOG_INFO("GOOSE Receiver started on interface: {}", interfaceName_);
  } else {
    LOG_ERROR("Failed to start GOOSE Receiver on interface: {}",
              interfaceName_);
  }
}

void GOOSEReceiver::stop() {
  if (running_) {
    GooseReceiver_stop(receiver_);
    running_ = false;
    LOG_INFO("GOOSE Receiver stopped on interface: {}", interfaceName_);
  }
}

bool GOOSEReceiver::isRunning() const { return running_; }

void GOOSEReceiver::addSubscriber(GooseSubscriber subscriber) {
  if (receiver_ && subscriber) {
    GooseReceiver_addSubscriber(receiver_, subscriber);
  }
}

void GOOSEReceiver::removeSubscriber(GooseSubscriber subscriber) {
  if (receiver_ && subscriber) {
    GooseReceiver_removeSubscriber(receiver_, subscriber);
  }
}

} // namespace goose
} // namespace iec61850
} // namespace gateway
