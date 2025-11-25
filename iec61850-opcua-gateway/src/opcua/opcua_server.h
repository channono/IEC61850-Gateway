#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <string>
#include <thread>

namespace gateway {
namespace opcua {

struct ServerConfig {
  int port = 4840;
  std::string endpointUrl = "opc.tcp://0.0.0.0:4840";
  bool enableSecurity = false;
  std::string appName = "IEC61850 Gateway";
};

class OPCUAServer {
public:
  OPCUAServer();
  ~OPCUAServer();

  bool init(const ServerConfig &config);
  bool start();
  void stop();
  bool isRunning() const;

  // Raw access to open62541 server (use with care)
  UA_Server *getNativeServer() const { return server_; }

  // Namespace management
  UA_UInt16 addNamespace(const std::string &namespaceUri);
  UA_UInt16 getNamespaceIndex(const std::string &namespaceUri) const;

  // Node creation helpers
  UA_NodeId createFolder(const UA_NodeId &parentId,
                         const std::string &browseName,
                         const std::string &displayName);
  UA_NodeId createVariable(const UA_NodeId &parentId,
                           const std::string &browseName,
                           const UA_Variant &initialValue,
                           const std::string &displayName);

  // Access control
  void setAnonymousAccess(bool allow);

private:
  UA_Server *server_{nullptr};
  volatile UA_Boolean running_{false};
  std::thread serverThread_;
  ServerConfig config_;
  std::map<std::string, UA_UInt16> namespaces_;

  void runServerLoop();
};

} // namespace opcua
} // namespace gateway
