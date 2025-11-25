#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "api/rest_api.h"
#include "core/application.h"
#include "core/config_parser.h"
#include "core/logger.h"
#include <chrono>
#include <csignal>
#include <thread>

using namespace gateway;

std::atomic<bool> g_running{true};

void signal_handler(int signal) { g_running = false; }

int main(int argc, char *argv[]) {
  // Initialize Logger
  core::Logger::init("gateway.log", spdlog::level::info);
  LOG_INFO("IEC61850 to OPC UA Gateway starting...");

  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  try {
    // Load Configuration
    core::ConfigParser configParser;
    auto config = configParser.load("config/gateway.yaml");
    LOG_INFO("Configuration loaded. Version: {}", config.version);

    // Initialize Application (IEC61850 + OPC UA)
    auto app_base = gateway::createGateway();
    auto app = dynamic_cast<core::Application *>(app_base.get());
    if (!app || !app_base->initialize("config/gateway.yaml")) {
      LOG_ERROR("Failed to initialize gateway application");
      return 1;
    }

    // Start Gateway Services (including OPC UA Server)
    if (!app_base->start()) {
      LOG_ERROR("Failed to start gateway application");
      return 1;
    }

    // Start REST API (UI Backend) with OPC UA Server reference
    // Port 6850, Update Interval 1000ms, OPC UA Server
    api::RESTApi restApi(6850, 1000, app->getOPCUAServer());
    restApi.start();

    LOG_INFO("Gateway is running. UI available at http://localhost:6850");

    // Main loop
    while (g_running) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Shutdown
    LOG_INFO("Stopping gateway...");
    restApi.stop();
    app_base->stop();

  } catch (const std::exception &e) {
    LOG_ERROR("Fatal error: {}", e.what());
    return 1;
  }

  LOG_INFO("Gateway stopped.");
  return 0;
}
