#include "ied_device.h"

namespace gateway {
namespace data {
namespace models {

IEDDevice::IEDDevice(const std::string &name, const std::string &ip, int port)
    : name_(name), ip_(ip), port_(port) {}

IEDDevice::~IEDDevice() {}

void IEDDevice::setStatus(ConnectionStatus status) {
  std::lock_guard<std::mutex> lock(mutex_);
  status_ = status;
}

} // namespace models
} // namespace data
} // namespace gateway
