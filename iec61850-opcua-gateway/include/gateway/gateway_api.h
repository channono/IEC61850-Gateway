#pragma once

#include <memory>
#include <string>

namespace gateway {

/**
 * @brief Main Gateway API class
 */
class GatewayAPI {
public:
  virtual ~GatewayAPI() = default;

  /**
   * @brief Initialize the gateway with configuration file
   * @param configPath Path to the YAML configuration file
   * @return true if initialization successful
   */
  virtual bool initialize(const std::string &configPath) = 0;

  /**
   * @brief Start the gateway services
   * @return true if started successfully
   */
  virtual bool start() = 0;

  /**
   * @brief Stop the gateway services
   */
  virtual void stop() = 0;

  /**
   * @brief Get the version string
   * @return Version string (e.g., "1.0.0")
   */
  virtual std::string getVersion() const = 0;
};

/**
 * @brief Factory function to create a Gateway instance
 */
std::unique_ptr<GatewayAPI> createGateway();

} // namespace gateway
