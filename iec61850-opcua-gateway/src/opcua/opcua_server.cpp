#include "opcua_server.h"
#include "core/logger.h"

namespace gateway {
namespace opcua {

OPCUAServer::OPCUAServer() { server_ = UA_Server_new(); }

OPCUAServer::~OPCUAServer() {
  stop();
  if (server_) {
    UA_Server_delete(server_);
    server_ = nullptr;
  }
}

bool OPCUAServer::init(const ServerConfig &config) {
  config_ = config;

  UA_ServerConfig *serverConfig = UA_Server_getConfig(server_);
  UA_ServerConfig_setMinimal(serverConfig, config.port, nullptr);

  // Set Application Name
  UA_LocalizedText name =
      UA_LOCALIZEDTEXT((char *)"en", (char *)config.appName.c_str());
  serverConfig->applicationDescription.applicationName = name;

  // Enable anonymous access by default
  setAnonymousAccess(true);

  LOG_INFO("OPC UA Server initialized on port {}", config.port);
  return true;
}

bool OPCUAServer::start() {
  if (running_)
    return true;

  running_ = true;
  serverThread_ = std::thread(&OPCUAServer::runServerLoop, this);

  LOG_INFO("OPC UA Server started");
  return true;
}

void OPCUAServer::stop() {
  if (running_) {
    running_ = false;
    if (serverThread_.joinable()) {
      serverThread_.join();
    }
    LOG_INFO("OPC UA Server stopped");
  }
}

bool OPCUAServer::isRunning() const { return running_; }

void OPCUAServer::runServerLoop() {
  UA_StatusCode retval = UA_Server_run(server_, &running_);
  if (retval != UA_STATUSCODE_GOOD) {
    LOG_ERROR("OPC UA Server failed with status code: {}", retval);
  }
}

// Namespace management
UA_UInt16 OPCUAServer::addNamespace(const std::string &namespaceUri) {
  auto it = namespaces_.find(namespaceUri);
  if (it != namespaces_.end()) {
    return it->second;
  }

  UA_UInt16 nsIndex = UA_Server_addNamespace(server_, namespaceUri.c_str());
  namespaces_[namespaceUri] = nsIndex;
  LOG_INFO("Added OPC UA namespace: {} (index {})", namespaceUri, nsIndex);
  return nsIndex;
}

UA_UInt16
OPCUAServer::getNamespaceIndex(const std::string &namespaceUri) const {
  auto it = namespaces_.find(namespaceUri);
  if (it != namespaces_.end()) {
    return it->second;
  }
  return 0; // Return default namespace if not found
}

// Node creation helpers
UA_NodeId OPCUAServer::createFolder(const UA_NodeId &parentId,
                                    const std::string &browseName,
                                    const std::string &displayName) {
  UA_NodeId folderId;
  UA_NodeId_init(&folderId);

  UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
  oAttr.displayName =
      UA_LOCALIZEDTEXT((char *)"en", (char *)displayName.c_str());

  UA_StatusCode retval = UA_Server_addObjectNode(
      server_, UA_NODEID_NULL, parentId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
      UA_QUALIFIEDNAME(parentId.namespaceIndex, (char *)browseName.c_str()),
      UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), oAttr, nullptr, &folderId);

  if (retval != UA_STATUSCODE_GOOD) {
    LOG_ERROR("Failed to create folder '{}': {}", browseName, retval);
    return UA_NODEID_NULL;
  }

  return folderId;
}

UA_NodeId OPCUAServer::createVariable(const UA_NodeId &parentId,
                                      const std::string &browseName,
                                      const UA_Variant &initialValue,
                                      const std::string &displayName) {
  UA_NodeId variableId;
  UA_NodeId_init(&variableId);

  UA_VariableAttributes vAttr = UA_VariableAttributes_default;
  vAttr.displayName =
      UA_LOCALIZEDTEXT((char *)"en", (char *)displayName.c_str());
  vAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
  UA_Variant_copy(&initialValue, &vAttr.value);

  UA_StatusCode retval = UA_Server_addVariableNode(
      server_, UA_NODEID_NULL, parentId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
      UA_QUALIFIEDNAME(parentId.namespaceIndex, (char *)browseName.c_str()),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), vAttr, nullptr,
      &variableId);

  if (retval != UA_STATUSCODE_GOOD) {
    LOG_ERROR("Failed to create variable '{}': {}", browseName, retval);
    return UA_NODEID_NULL;
  }

  return variableId;
}

void OPCUAServer::setAnonymousAccess(bool allow) {
  UA_ServerConfig *config = UA_Server_getConfig(server_);
  if (allow) {
    config->securityPolicies[0].policyUri = UA_STRING_NULL;
  }
}

} // namespace opcua
} // namespace gateway
