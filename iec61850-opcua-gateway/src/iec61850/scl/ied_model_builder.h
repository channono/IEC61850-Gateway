#pragma once

#include "data/models/ied_device.h"
#include "scl_parser.h"
#include <memory>
#include <vector>

namespace gateway {
namespace iec61850 {

class IEDModelBuilder {
public:
  /**
   * @brief Build IED Device models from SCL configuration
   * @param config IED configuration from SCL Parser
   * @return Shared pointer to created IEDDevice
   */
  static std::shared_ptr<data::models::IEDDevice>
  build(const IEDConfig &config);

  /**
   * @brief Build multiple IED Device models
   * @param configs Vector of IED configurations
   * @return Vector of shared pointers to IEDDevices
   */
  static std::vector<std::shared_ptr<data::models::IEDDevice>>
  buildAll(const std::vector<IEDConfig> &configs);
};

} // namespace iec61850
} // namespace gateway
