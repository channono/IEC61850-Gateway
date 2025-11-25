#include "service_manager.h"
#include "logger.h"

namespace gateway {
namespace core {

ServiceManager::ServiceManager() {}

ServiceManager::~ServiceManager() { stopAll(); }

void ServiceManager::registerService(std::shared_ptr<IService> service) {
  services_.push_back(service);
  LOG_INFO("Registered service: {}", service->getName());
}

bool ServiceManager::initAll() {
  LOG_INFO("Initializing services...");
  for (auto &service : services_) {
    if (!service->init()) {
      LOG_ERROR("Failed to initialize service: {}", service->getName());
      return false;
    }
  }
  return true;
}

bool ServiceManager::startAll() {
  LOG_INFO("Starting services...");
  for (auto &service : services_) {
    if (!service->start()) {
      LOG_ERROR("Failed to start service: {}", service->getName());
      // Rollback: stop already started services
      stopAll();
      return false;
    }
  }
  return true;
}

void ServiceManager::stopAll() {
  LOG_INFO("Stopping services...");
  // Stop in reverse order
  for (auto it = services_.rbegin(); it != services_.rend(); ++it) {
    (*it)->stop();
  }
}

} // namespace core
} // namespace gateway
