#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace gateway {
namespace data {
namespace models {

enum class ConnectionStatus { DISCONNECTED, CONNECTING, CONNECTED, ERROR };

class IEDDevice {
public:
  IEDDevice(const std::string &name, const std::string &ip, int port = 102);
  ~IEDDevice();

  // Getters
  std::string getName() const { return name_; }
  std::string getIP() const { return ip_; }
  int getPort() const { return port_; }
  ConnectionStatus getStatus() const { return status_; }

  // Setters
  void setStatus(ConnectionStatus status);

  // TODO: Add Data Points management
  // void addDataPoint(std::shared_ptr<DataPoint> point);

private:
  std::string name_;
  std::string ip_;
  int port_;
  ConnectionStatus status_{ConnectionStatus::DISCONNECTED};
  mutable std::mutex mutex_;
};

} // namespace models
} // namespace data
} // namespace gateway
