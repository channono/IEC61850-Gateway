#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>

// Forward declarations
namespace gateway {
namespace opcua {
class OPCUAServer;
class DataBinder;
} // namespace opcua
} // namespace gateway

namespace gateway {
namespace api {

class RESTApi {
public:
  RESTApi(int port = 6850, int updateIntervalMs = 1000,
          std::shared_ptr<opcua::OPCUAServer> opcua_server = nullptr);
  ~RESTApi();

  void start();
  void stop();

  /**
   * @brief Broadcast a message to all connected WebSocket clients
   * @param message JSON message string
   */
  void broadcast(const std::string &message);

private:
  int port_;
  int updateIntervalMs_;
  std::atomic<bool> running_{false};
  std::thread serverThread_;
  std::shared_ptr<opcua::OPCUAServer> opcua_server_;
  std::shared_ptr<opcua::DataBinder> dataBinder_;

  // Opaque pointer to httplib::Server to avoid header dependency
  void *server_ptr_{nullptr};
  // Mutex for thread safety
  std::mutex broadcast_mutex_;

  // Event queue for polling
  std::vector<std::string> event_queue_;

  std::thread pollingThread_;
  void runServer();
  void pollData();
};

} // namespace api
} // namespace gateway
