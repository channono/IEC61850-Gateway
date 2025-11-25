#include "mms_connection_manager.h"
#include "core/logger.h"

namespace gateway {
namespace iec61850 {
namespace mms {

MMSConnectionManager::MMSConnectionManager() {}

MMSConnectionManager::~MMSConnectionManager() { disconnectAll(); }

std::shared_ptr<MMSConnection>
MMSConnectionManager::addConnection(const std::string &name,
                                    const std::string &ip, int port) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (connections_.find(name) != connections_.end()) {
    LOG_WARN("Connection {} already exists", name);
    return connections_[name];
  }

  auto connection = std::make_shared<MMSConnection>(ip, port);
  connections_[name] = connection;
  LOG_INFO("Added MMS connection: {} ({}:{})", name, ip, port);
  return connection;
}

std::shared_ptr<MMSConnection>
MMSConnectionManager::getConnection(const std::string &name) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = connections_.find(name);
  if (it != connections_.end()) {
    return it->second;
  }
  return nullptr;
}

void MMSConnectionManager::removeConnection(const std::string &name) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = connections_.find(name);
  if (it != connections_.end()) {
    it->second->disconnect();
    connections_.erase(it);
    LOG_INFO("Removed MMS connection: {}", name);
  }
}

void MMSConnectionManager::connectAll() {
  std::lock_guard<std::mutex> lock(mutex_);
  LOG_INFO("Connecting all MMS connections...");
  for (auto &pair : connections_) {
    // TODO: Use thread pool for parallel connection
    pair.second->connect();
  }
}

void MMSConnectionManager::disconnectAll() {
  std::lock_guard<std::mutex> lock(mutex_);
  LOG_INFO("Disconnecting all MMS connections...");
  for (auto &pair : connections_) {
    pair.second->disconnect();
  }
}

} // namespace mms
} // namespace iec61850
} // namespace gateway
