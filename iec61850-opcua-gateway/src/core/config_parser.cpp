#include "config_parser.h"
#include "logger.h"
#include <fstream>

namespace gateway {
namespace core {

GatewayConfig ConfigParser::load(const std::string &path) {
  GatewayConfig config;
  try {
    YAML::Node root = YAML::LoadFile(path);

    if (root["version"]) {
      config.version = root["version"].as<std::string>();
    }

    if (root["opcua"]) {
      auto opcua = root["opcua"];
      if (opcua["port"])
        config.opcua.port = opcua["port"].as<int>();
      if (opcua["endpoint_url"])
        config.opcua.endpointUrl = opcua["endpoint_url"].as<std::string>();
      if (opcua["enable_security"])
        config.opcua.enableSecurity = opcua["enable_security"].as<bool>();
    }

    if (root["storage"]) {
      auto storage = root["storage"];
      if (storage["type"])
        config.storage.type = storage["type"].as<std::string>();
      if (storage["url"])
        config.storage.url = storage["url"].as<std::string>();
      if (storage["org"])
        config.storage.org = storage["org"].as<std::string>();
      if (storage["bucket"])
        config.storage.bucket = storage["bucket"].as<std::string>();
      if (storage["token"])
        config.storage.token = storage["token"].as<std::string>();
      if (storage["enabled"])
        config.storage.enabled = storage["enabled"].as<bool>();
    }

    if (root["ieds"] && root["ieds"].IsSequence()) {
      for (const auto &node : root["ieds"]) {
        IEDConfig ied;
        if (node["name"])
          ied.name = node["name"].as<std::string>();
        if (node["ip"])
          ied.ip = node["ip"].as<std::string>();
        if (node["port"])
          ied.port = node["port"].as<int>();
        if (node["enabled"])
          ied.enabled = node["enabled"].as<bool>();
        config.ieds.push_back(ied);
      }
    }

    LOG_INFO("Loaded configuration from {}", path);
  } catch (const YAML::Exception &e) {
    LOG_ERROR("Failed to parse config file {}: {}", path, e.what());
    throw;
  } catch (const std::exception &e) {
    LOG_ERROR("Failed to load config file {}: {}", path, e.what());
    throw;
  }
  return config;
}

void ConfigParser::save(const std::string &path, const GatewayConfig &config) {
  // TODO: Implement save logic
  LOG_WARN("Config save not implemented yet");
}

} // namespace core
} // namespace gateway
