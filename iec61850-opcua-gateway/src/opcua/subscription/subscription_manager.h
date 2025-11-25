#pragma once

#include "opcua/opcua_server.h"
#include <memory>

namespace gateway {
namespace opcua {
namespace subscription {

class SubscriptionManager {
public:
  SubscriptionManager(std::shared_ptr<OPCUAServer> server);

  // TODO: Implement subscription logic
  // open62541 handles subscriptions automatically for Variables.
  // This class will be used for more advanced event-driven updates if needed.

private:
  std::shared_ptr<OPCUAServer> server_;
};

} // namespace subscription
} // namespace opcua
} // namespace gateway
