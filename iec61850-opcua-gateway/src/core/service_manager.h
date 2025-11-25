#pragma once

#include <memory>
#include <string>
#include <vector>

namespace gateway {
namespace core {

/**
 * @brief Interface for all services
 */
class IService {
public:
  virtual ~IService() = default;
  virtual bool init() = 0;
  virtual bool start() = 0;
  virtual void stop() = 0;
  virtual std::string getName() const = 0;
};

/**
 * @brief Manages the lifecycle of all services
 */
class ServiceManager {
public:
  ServiceManager();
  ~ServiceManager();

  void registerService(std::shared_ptr<IService> service);
  bool initAll();
  bool startAll();
  void stopAll();

private:
  std::vector<std::shared_ptr<IService>> services_;
};

} // namespace core
} // namespace gateway
