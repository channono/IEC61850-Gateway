#pragma once

#include <atomic>
#include <functional>
#include <libiec61850/iec61850_client.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class SCLGenerator;

namespace gateway {
namespace iec61850 {
namespace mms {

class MMSConnection {
  // Allow SCLGenerator to access private connection_ member
  friend class ::SCLGenerator;

public:
  MMSConnection(const std::string &ip, int port = 102);
  ~MMSConnection();

  bool connect();
  void disconnect();
  bool isConnected() const;

  std::string getIP() const { return ip_; }
  int getPort() const { return port_; }

  // Raw access to libiec61850 connection (use with care)
  IedConnection getNativeConnection() const { return connection_; }

  // Read data from IED
  // Returns a pair of {value_string, type_string}
  // Throws std::runtime_error on failure
  struct ReadResult {
    std::string value;
    std::string type;
    std::string quality;
    std::string timestamp;
  };
  ReadResult readData(const std::string &objectReference);

  // MMS Operations
  ReadResult readData(const std::string &ref, const std::string &fc = "MX");
  // Write data to IED
  void writeData(const std::string &objectReference, const std::string &type,
                 const std::string &value);

  // Discovery
  std::vector<std::string> getLogicalDeviceList();
  std::vector<std::string> getLogicalDeviceVariables(const std::string &ldName);
  std::vector<std::string> getLogicalNodeVariables(const std::string &ldName,
                                                   const std::string &lnName);

  // Get list of Report Control Blocks (RCBs)
  std::vector<std::string> getReportControlBlocks();

  // Reporting
  using ReportCallback = std::function<void(
      const std::string &rcbRef, const std::string &reportId,
      const std::string &dataSetRef,
      const std::vector<std::pair<std::string, std::string>> &values)>;

  // Subscribe to a Report Control Block (RCB)
  // Returns a subscription ID (or just uses the RCB ref)
  void subscribeReport(const std::string &rcbRef, ReportCallback callback);
  void unsubscribeReport(const std::string &rcbRef);

  // Internal use
  void handleReport(ClientReport report);

private:
  std::string ip_;
  int port_;
  IedConnection connection_{nullptr};
  std::atomic<bool> connected_{false};
  mutable std::mutex mutex_;

  std::map<std::string, ReportCallback> reportCallbacks_;
};

} // namespace mms
} // namespace iec61850
} // namespace gateway
