#pragma once

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace gateway {
namespace core {

struct IEDConfig {
  std::string name;
  std::string ip;
  int port = 102;
  bool enabled = true;
};

struct OPCUAConfig {
  int port = 4840;
  std::string endpointUrl;
  bool enableSecurity = false;
};

struct StorageConfig {
  std::string type = "influxdb"; // "influxdb" or "timescaledb"
  std::string url;    // InfluxDB URL or TimescaleDB Connection String
  std::string org;    // InfluxDB Org
  std::string bucket; // InfluxDB Bucket
  std::string token;  // InfluxDB Token
  bool enabled = false;
};

struct GatewayConfig {
  std::string version;
  OPCUAConfig opcua;
  StorageConfig storage;
  std::vector<IEDConfig> ieds;
};

class ConfigParser {
public:
  static GatewayConfig load(const std::string &path);
  static void save(const std::string &path, const GatewayConfig &config);
};

} // namespace core
} // namespace gateway
