#include "subscription_manager.h"

namespace gateway {
namespace opcua {
namespace subscription {

SubscriptionManager::SubscriptionManager(std::shared_ptr<OPCUAServer> server)
    : server_(server) {}

} // namespace subscription
} // namespace opcua
} // namespace gateway
