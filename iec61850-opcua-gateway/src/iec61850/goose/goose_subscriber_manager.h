#pragma once

#include "goose_receiver.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace gateway {
namespace iec61850 {
namespace goose {

class GOOSESubscriberManager {
public:
  GOOSESubscriberManager(std::shared_ptr<GOOSEReceiver> receiver);
  ~GOOSESubscriberManager();

  /**
   * @brief Subscribe to a specific GOOSE Control Block
   * @param goCbRef GOOSE Control Block Reference (e.g., "IED1/LLN0$GO$gcb01")
   */
  void subscribe(const std::string &goCbRef);

  /**
   * @brief Unsubscribe from a GOOSE Control Block
   * @param goCbRef GOOSE Control Block Reference
   */
  void unsubscribe(const std::string &goCbRef);

private:
  static void onGooseMessage(GooseSubscriber subscriber, void *parameter);

  std::shared_ptr<GOOSEReceiver> receiver_;
  std::unordered_map<std::string, GooseSubscriber> subscribers_;
};

} // namespace goose
} // namespace iec61850
} // namespace gateway
