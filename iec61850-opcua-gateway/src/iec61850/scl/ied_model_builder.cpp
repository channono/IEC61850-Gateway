#include "ied_model_builder.h"
#include "core/logger.h"

namespace gateway {
namespace iec61850 {

std::shared_ptr<data::models::IEDDevice>
IEDModelBuilder::build(const IEDConfig &config) {
  LOG_INFO("Building model for IED: {}", config.name);

  // Create base device
  // Default port 102 for MMS if not specified (though IEDConfig doesn't have
  // port yet, assuming standard)
  auto device =
      std::make_shared<data::models::IEDDevice>(config.name, config.ip, 102);

  // TODO: Add logical devices and nodes from config
  // This will be expanded when SCLParser extracts more detailed structure

  return device;
}

std::vector<std::shared_ptr<data::models::IEDDevice>>
IEDModelBuilder::buildAll(const std::vector<IEDConfig> &configs) {
  std::vector<std::shared_ptr<data::models::IEDDevice>> devices;
  devices.reserve(configs.size());

  for (const auto &config : configs) {
    devices.push_back(build(config));
  }

  return devices;
}

} // namespace iec61850
} // namespace gateway
