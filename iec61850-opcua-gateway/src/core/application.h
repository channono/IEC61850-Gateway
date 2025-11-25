#pragma once

#include "gateway/gateway_api.h"
#include "service_manager.h"
#include <atomic>
#include <memory>

namespace gateway {

// Forward declarations
namespace opcua {
class OPCUAServer;
}

namespace core {

class Application : public GatewayAPI {
public:
  Application();
  ~Application() override;

  bool initialize(const std::string &configPath) override;
  bool start() override;
  void stop() override;
  std::string getVersion() const override;

  // Get OPC UA Server instance
  std::shared_ptr<opcua::OPCUAServer> getOPCUAServer() const {
    return opcua_server_;
  }

private:
  std::unique_ptr<ServiceManager> service_manager_;
  std::shared_ptr<opcua::OPCUAServer> opcua_server_;
  std::atomic<bool> running_{false};
  std::string config_path_;
};

} // namespace core
} // namespace gateway
