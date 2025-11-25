#pragma once

#include "mms_connection.h"
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace gateway {
namespace iec61850 {
namespace mms {

class MMSConnectionManager {
public:
  MMSConnectionManager();
  ~MMSConnectionManager();

  /**
   * @brief Add a new connection
   * @param name Unique name for the connection (e.g., IED name)
   * @param ip IP address
   * @param port Port number
   * @return Shared pointer to the created connection
   */
  std::shared_ptr<MMSConnection>
  addConnection(const std::string &name, const std::string &ip, int port = 102);

  /**
   * @brief Get a connection by name
   * @param name Connection name
   * @return Shared pointer to connection or nullptr if not found
   */
  std::shared_ptr<MMSConnection> getConnection(const std::string &name);

  /**
   * @brief Remove a connection
   * @param name Connection name
   */
  void removeConnection(const std::string &name);

  /**
   * @brief Connect all managed connections
   */
  void connectAll();

  /**
   * @brief Disconnect all managed connections
   */
  void disconnectAll();

private:
  std::unordered_map<std::string, std::shared_ptr<MMSConnection>> connections_;
  mutable std::mutex mutex_;
};

} // namespace mms
} // namespace iec61850
} // namespace gateway
