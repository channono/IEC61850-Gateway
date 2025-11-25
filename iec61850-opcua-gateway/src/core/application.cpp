#include "application.h"
#include "../opcua/opcua_server.h"
#include "logger.h"
#include <chrono>
#include <thread>

namespace gateway {
namespace core {

Application::Application()
    : service_manager_(std::make_unique<ServiceManager>()) {}

Application::~Application() { stop(); }

bool Application::initialize(const std::string &configPath) {
  config_path_ = configPath;

  // Initialize Logger
  Logger::init();
  LOG_INFO("Initializing Gateway Application v{}", getVersion());

  // Initialize OPC UA Server
  opcua_server_ = std::make_shared<opcua::OPCUAServer>();
  opcua::ServerConfig opcuaConfig;
  opcuaConfig.port = 4840;
  opcuaConfig.appName = "IEC61850-OPC UA Gateway";
  if (!opcua_server_->init(opcuaConfig)) {
    LOG_ERROR("Failed to initialize OPC UA Server");
    return false;
  }

  // Add custom namespace
  opcua_server_->addNamespace("http://iec61850-gateway.local");

  // Create a demo folder to verify server is working
  UA_NodeId objectsFolder = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  UA_NodeId demoFolder =
      opcua_server_->createFolder(objectsFolder, "IEDs", "IED Devices");

  // Create a demo variable
  UA_Variant value;
  UA_Variant_init(&value);
  UA_String str = UA_STRING((char *)"Gateway Ready");
  UA_Variant_setScalar(&value, &str, &UA_TYPES[UA_TYPES_STRING]);
  opcua_server_->createVariable(demoFolder, "Status", value, "Gateway Status");

  return service_manager_->initAll();
}

bool Application::start() {
  // Start OPC UA Server
  if (opcua_server_ && !opcua_server_->start()) {
    LOG_ERROR("Failed to start OPC UA Server");
    return false;
  }

  if (!service_manager_->startAll()) {
    return false;
  }

  running_ = true;
  LOG_INFO("Gateway Application started successfully");
  LOG_INFO("OPC UA Server running on opc.tcp://localhost:4840");
  return true;
}

void Application::stop() {
  if (running_) {
    LOG_INFO("Stopping Gateway Application...");
    running_ = false;

    // Stop OPC UA Server
    if (opcua_server_) {
      opcua_server_->stop();
    }

    service_manager_->stopAll();
  }
}

std::string Application::getVersion() const { return "1.0.0"; }

} // namespace core

// Factory function implementation
std::unique_ptr<GatewayAPI> createGateway() {
  return std::make_unique<core::Application>();
}

} // namespace gateway
