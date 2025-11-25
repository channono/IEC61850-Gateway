#include "rest_api.h"
#include "core/logger.h"
#include "httplib/httplib.h"
#include "iec61850/mms/mms_connection.h"
#include "iec61850/scl/scd_generator.h"
#include "iec61850/scl/scl_generator.h"
#include "iec61850/scl/scl_parser.h"
#include "opcua/namespace/namespace_builder.h"
#include "opcua/opcua_server.h"
#include "topology_parser.h"
#include <filesystem>
#include <fstream>
#include <future>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>
#include <thread>

#include "opcua/data_binder.h"

namespace gateway {
namespace api {

// PIMPL-like structure to hide httplib details from header if we wanted,
// but since we included it here, we can just use a member or static map for WS
// sessions. For simplicity in this single-file implementation:
// Global set to track active WebSocket connections (simplified for this
// implementation)
std::set<std::string> ws_connected_clients;
std::mutex ws_mutex;

// MMS connection pool for testing
std::map<std::string, std::shared_ptr<gateway::iec61850::mms::MMSConnection>>
    mms_connections;

// ... globals ...

RESTApi::RESTApi(int port, int updateIntervalMs,
                 std::shared_ptr<opcua::OPCUAServer> opcua_server)
    : port_(port), updateIntervalMs_(updateIntervalMs),
      opcua_server_(opcua_server) {
  if (opcua_server_) {
    dataBinder_ = std::make_shared<opcua::DataBinder>(opcua_server_);
  }
}

RESTApi::~RESTApi() { stop(); }

void RESTApi::start() {
  if (running_)
    return;
  running_ = true;

  server_ptr_ = new httplib::Server();

  // Load existing SCD file if it exists
  std::string scdPath = "./config/station.scd";
  if (std::filesystem::exists(scdPath) && opcua_server_ && dataBinder_) {
    try {
      using namespace gateway::opcua::ns;
      NamespaceBuilder builder(opcua_server_, dataBinder_);
      builder.buildFromSCD(scdPath);
      LOG_INFO("Loaded OPC UA namespace from existing SCD: {}", scdPath);
    } catch (const std::exception &e) {
      LOG_WARN("Failed to load SCD on startup: {}", e.what());
    }
  }

  // Set MMS connections for write operations
  if (dataBinder_) {
    dataBinder_->setMMSConnections(&mms_connections);
    LOG_INFO("Set MMS connections for DataBinder write operations");
  }

  // Auto-connect to enabled IEDs from configuration
  std::string configPath = "./config/gateway.yaml";
  if (std::filesystem::exists(configPath)) {
    try {
      std::ifstream file(configPath);
      std::string line;
      bool inIeds = false;
      std::string currentIed;
      std::string currentIp;
      int currentPort = 102;
      bool currentEnabled = false;

      while (std::getline(file, line)) {
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos)
          continue;
        line = line.substr(start);

        if (line.find("ieds:") == 0) {
          inIeds = true;
          continue;
        }

        if (inIeds) {
          if (line.find("- name:") == 0) {
            // Process previous IED if enabled
            if (!currentIed.empty() && currentEnabled) {
              LOG_INFO("Auto-connecting to IED: {} at {}:{}", currentIed,
                       currentIp, currentPort);
              auto conn =
                  std::make_shared<gateway::iec61850::mms::MMSConnection>(
                      currentIp, currentPort);
              if (conn->connect()) {
                mms_connections[currentIed] = conn;
                LOG_INFO("Auto-connected to {} successfully", currentIed);
              } else {
                LOG_WARN("Failed to auto-connect to {}", currentIed);
              }
            }

            // Start new IED
            size_t pos = line.find("\"");
            if (pos != std::string::npos) {
              size_t end = line.find("\"", pos + 1);
              currentIed = line.substr(pos + 1, end - pos - 1);
              currentEnabled = false;
              currentPort = 102;
            }
          } else if (line.find("ip:") != std::string::npos) {
            size_t pos = line.find("\"");
            if (pos != std::string::npos) {
              size_t end = line.find("\"", pos + 1);
              currentIp = line.substr(pos + 1, end - pos - 1);
            }
          } else if (line.find("port:") != std::string::npos) {
            size_t pos = line.find(":") + 1;
            std::string portStr = line.substr(pos);
            portStr.erase(0, portStr.find_first_not_of(" \t"));
            currentPort = std::stoi(portStr);
          } else if (line.find("enabled:") != std::string::npos) {
            currentEnabled = (line.find("true") != std::string::npos);
          }
        }
      }

      // Process last IED
      if (!currentIed.empty() && currentEnabled) {
        LOG_INFO("Auto-connecting to IED: {} at {}:{}", currentIed, currentIp,
                 currentPort);
        auto conn = std::make_shared<gateway::iec61850::mms::MMSConnection>(
            currentIp, currentPort);
        if (conn->connect()) {
          mms_connections[currentIed] = conn;
          LOG_INFO("Auto-connected to {} successfully", currentIed);
        } else {
          LOG_WARN("Failed to auto-connect to {}", currentIed);
        }
      }

    } catch (const std::exception &e) {
      LOG_WARN("Failed to auto-connect IEDs: {}", e.what());
    }
  }

  serverThread_ = std::thread(&RESTApi::runServer, this);
  pollingThread_ = std::thread(&RESTApi::pollData, this);
  LOG_INFO("REST API Server started on port {}", port_);
}

void RESTApi::stop() {
  if (running_) {
    running_ = false;

    if (server_ptr_) {
      static_cast<httplib::Server *>(server_ptr_)->stop();
    }

    if (serverThread_.joinable()) {
      serverThread_.join();
    }

    if (pollingThread_.joinable()) {
      pollingThread_.join();
    }

    if (server_ptr_) {
      delete static_cast<httplib::Server *>(server_ptr_);
      server_ptr_ = nullptr;
    }

    LOG_INFO("REST API Server stopped");
  }
}

void RESTApi::pollData() {
  LOG_INFO("Starting data polling loop (interval: {}ms)", updateIntervalMs_);
  while (running_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(updateIntervalMs_));

    if (!dataBinder_)
      continue;

    auto refs = dataBinder_->getBoundReferences();
    if (refs.empty())
      continue;

    // Group by IED
    std::map<std::string, std::vector<std::string>> iedRefs;
    for (const auto &ref : refs) {
      size_t slashPos = ref.find('/');
      if (slashPos != std::string::npos) {
        std::string iedName = ref.substr(0, slashPos);
        std::string objRef = ref.substr(slashPos + 1);
        // Replace remaining '/' with '/'? No, MMS ref is "LD/LN.DO"
        // My ref format: "IED/LD/LN.DO"
        // Obj ref: "LD/LN.DO"
        // This matches MMS format if I constructed it correctly.
        iedRefs[iedName].push_back(ref);
      }
    }

    for (const auto &pair : iedRefs) {
      const std::string &iedName = pair.first;
      const auto &vars = pair.second;

      auto it = mms_connections.find(iedName);
      if (it != mms_connections.end() && it->second->isConnected()) {
        auto conn = it->second;
        LOG_DEBUG("Polling {} variables for IED: {}", vars.size(), iedName);

        for (const auto &fullRef : vars) {
          // Extract objRef: "IED/LD/LN.DO" -> "LD/LN.DO"
          size_t slashPos = fullRef.find('/');
          std::string objRef = fullRef.substr(slashPos + 1);

          try {
            IedClientError error;
            MmsValue *value = nullptr;

            // Try reading .stVal FIRST for SPS/DPS/Boolean types (ST FC)
            std::string stValRef = objRef + ".stVal";
            value =
                IedConnection_readObject(conn->getNativeConnection(), &error,
                                         stValRef.c_str(), IEC61850_FC_ST);

            if (error == IED_ERROR_OK && value) {
              MmsType type = MmsValue_getType(value);
              if (type == MMS_BOOLEAN) {
                // This is a real Boolean status value - use it
                bool boolVal = MmsValue_getBoolean(value);
                LOG_INFO("✓ Read {} = {}", stValRef, boolVal);
                dataBinder_->updateValue(fullRef, value);
                MmsValue_delete(value);
                continue; // Found boolean - done
              } else if (type == MMS_INTEGER || type == MMS_UNSIGNED ||
                         type == MMS_BIT_STRING) {
                // Handle Enum/Integer types (Beh, Health, Mod, etc.)
                int intVal = 0;
                if (type == MMS_INTEGER)
                  intVal = MmsValue_toInt32(value);
                else if (type == MMS_UNSIGNED)
                  intVal = MmsValue_toUint32(value);
                else if (type == MMS_BIT_STRING)
                  intVal = MmsValue_getBitStringAsInteger(value);

                LOG_INFO("✓ Read {} = {} (Enum/Int)", stValRef, intVal);
                dataBinder_->updateValue(fullRef, value);
                MmsValue_delete(value);
                continue;
              } else {
                // stVal exists but not Boolean/Int/Enum - might be complex or
                // unexpected Don't use this value, delete and try .mag.f
                // instead
                MmsValue_delete(value);
              }
            }

            // Try reading .mag.f for MV types (MX FC)
            std::string magfRef = objRef + ".mag.f";
            value =
                IedConnection_readObject(conn->getNativeConnection(), &error,
                                         magfRef.c_str(), IEC61850_FC_MX);

            if (error == IED_ERROR_OK && value) {
              float floatVal = MmsValue_toFloat(value);
              LOG_INFO("✓ Read {} = {}", magfRef, floatVal);
              dataBinder_->updateValue(fullRef, value);
              MmsValue_delete(value);
              continue;
            }

            // Fallback: try reading the DO directly
            value =
                IedConnection_readObject(conn->getNativeConnection(), &error,
                                         objRef.c_str(), IEC61850_FC_ST);
            if (error == IED_ERROR_OK && value) {
              LOG_INFO("✓ Read DO directly: {}", objRef);
              dataBinder_->updateValue(fullRef, value);
              MmsValue_delete(value);
            } else {
              LOG_WARN("✗ Failed to read {}, error: {}", objRef, (int)error);
            }

          } catch (const std::exception &e) {
            LOG_WARN("Exception while polling {}: {}", fullRef, e.what());
          }
        }
      }
    }
  }
}

void RESTApi::broadcast(const std::string &message) {
  // Broadcast to all connected WebSocket clients
  // Note: cpp-httplib doesn't have a direct "broadcast" method for WS,
  // we usually need to iterate over stored connections if we were using a
  // different library. BUT, for cpp-httplib, we can't easily send to a
  // connection from outside the handler unless we use the 'TaskQueue' or
  // similar mechanism provided by the library, OR we just ignore WS for now and
  // use polling?
  //
  // Wait, the user wants "Report Subscription". Real-time updates are best with
  // WS. Let's check if we can use a simple hack: Since we can't easily store
  // `httplib::Connection` objects (they are internal), we might need to rely on
  // the library's features.
  //
  // Actually, looking at cpp-httplib docs (simulated), it supports `send` if we
  // have the handle. But let's assume for this task we might need to use a
  // simpler approach if WS is too complex to integrate with the current
  // `httplib` version we have (which is a single header).
  //
  // ALTERNATIVE: Use a polling endpoint `/api/mms/reports` that returns the
  // latest reports. This is easier to implement and robust. The frontend
  // `mms-test.js` has `addReportLog`.
  //
  // Let's check `mms-test.js` again. It calls `/api/mms/subscribe`.
  // It doesn't seem to have a WS listener loop in `initMMSTest`.
  // It only has `connectToIED`, `readDataPoint`, etc.
  //
  // Wait, `mms-test.js` does NOT have WebSocket logic!
  // I checked `mms-test.js` content in previous steps.
  // It has `subscribeToReport` which calls `POST /api/mms/subscribe`.
  // But how does it receive the data?
  //
  // `mms-test.js` lines 370+:
  // `subscribeToReport` sends POST.
  // It gets `result.sub_id`.
  // But there is NO code to listen for updates!
  //
  // So I need to:
  // 1. Implement a polling mechanism in `mms-test.js` OR add WebSocket support
  // to `mms-test.js`.
  // 2. Given the "Real-time" nature, WebSocket is better.
  //
  // Let's add WebSocket support to `mms-test.js` first?
  // Or maybe the user expects me to implement the backend first.
  //
  // Let's implement the backend to support storing reports in a buffer,
  // and provide an endpoint to fetch them (Polling) OR use WS.
  //
  // Given the constraints and the library, Polling might be safer/faster to
  // implement right now. But "Subscription" implies push.
  //
  // Let's try to implement a simple WebSocket broadcast if possible.
  // If `httplib` is too hard, I'll use polling.
  //
  // Let's look at `rest_api.cpp` includes. `httplib.h`.
  //
  // I will implement a `ReportBuffer` in `RESTApi` and a `/api/mms/updates`
  // endpoint that uses Long Polling or just simple Polling.
  //
  // Let's stick to the plan: Implement `subscribe` which registers a callback.
  // The callback will push data to a queue.
  // And I'll add a `/api/mms/events` endpoint for the frontend to consume.

  // But wait, I already modified `rest_api.h` to have `server_ptr_`.
  // I will keep that.

  // Let's implement the `broadcast` to just log for now, anvoid
  // RESTApi::broadcast(const std::string &message) {
  std::lock_guard<std::mutex> lock(broadcast_mutex_);
  event_queue_.push_back(message);

  // Keep queue size manageable
  if (event_queue_.size() > 100) {
    event_queue_.erase(event_queue_.begin());
  }
}

// Helper function to regenerate SCD from all ICD files
void regenerateSCD(std::shared_ptr<opcua::OPCUAServer> opcua_server,
                   std::shared_ptr<opcua::DataBinder> binder) {
  using namespace gateway::iec61850::scl;

  try {
    LOG_INFO("Regenerating SCD from ICD files...");

    // ... (same as before) ...
    SCDGenerator scdGen;
    std::unordered_map<std::string, NetworkConfig> netConfigs;

    // Scan all ICD files in config/icd/
    namespace fs = std::filesystem;
    int icdCount = 0;
    if (fs::exists("./config/icd")) {
      for (const auto &entry : fs::directory_iterator("./config/icd/")) {
        if (entry.path().extension() == ".icd") {
          std::string icdPath = entry.path().string();
          scdGen.addICD(icdPath);
          icdCount++;
          LOG_INFO("Added ICD: {}", entry.path().filename().string());

          // Extract IP address from this ICD file
          pugi::xml_document doc;
          if (doc.load_file(icdPath.c_str())) {
            auto iedNode = doc.select_node("//IED").node();
            std::string iedName = iedNode.attribute("name").as_string();

            // Find IP in Communication/SubNetwork/ConnectedAP/Address
            auto ipNode =
                doc.select_node("//ConnectedAP/Address/P[@type='IP']").node();
            if (ipNode) {
              NetworkConfig cfg;
              cfg.ipAddress = ipNode.text().as_string();
              cfg.subnetMask = "255.255.255.0"; // Default subnet mask
              cfg.gateway = "192.168.1.1";      // Default gateway

              // Get port if available
              auto portNode =
                  doc.select_node("//ConnectedAP/Address/P[@type='TCP-PORT']")
                      .node();
              if (portNode) {
                cfg.mmsPort = portNode.text().as_int();
              }

              netConfigs[iedName] = cfg;
              LOG_INFO("Extracted IP {} for IED: {}", cfg.ipAddress, iedName);
            }
          }
        }
      }
    }

    if (icdCount == 0) {
      LOG_WARN("No ICD files found in config/icd/");
      return;
    }

    // Configure network with extracted IPs
    scdGen.configureNetwork(netConfigs, false); // Don't auto-assign

    // Generate SCD
    std::string scdContent = scdGen.generate();

    // Save SCD file
    std::ofstream scdFile("./config/station.scd");
    scdFile << scdContent;
    scdFile.close();

    LOG_INFO("Generated SCD with {} IEDs at ./config/station.scd", icdCount);

    // Rebuild OPC UA namespace
    if (opcua_server) {
      try {
        using namespace gateway::opcua::ns;
        NamespaceBuilder builder(opcua_server, binder);
        builder.buildFromSCD("./config/station.scd");
        LOG_INFO("Rebuilt OPC UA namespace from SCD");
      } catch (const std::exception &e) {
        LOG_ERROR("Failed to rebuild OPC UA namespace: {}", e.what());
      }
    }

  } catch (const std::exception &e) {
    LOG_ERROR("Failed to regenerate SCD: {}", e.what());
  }
}

void RESTApi::runServer() {
  auto &svr = *static_cast<httplib::Server *>(server_ptr_);

  // Serve Static Files from ./www
  svr.set_mount_point("/", "./www");

  // WebSocket Endpoint (Placeholder)
  svr.Get("/ws", [&](const httplib::Request &req, httplib::Response &res) {
    // ...
  });

  // API: Events (Polling)
  svr.Get("/api/mms/events",
          [&](const httplib::Request &, httplib::Response &res) {
            nlohmann::json events = nlohmann::json::array();

            {
              std::lock_guard<std::mutex> lock(broadcast_mutex_);
              for (const auto &event : event_queue_) {
                try {
                  events.push_back(nlohmann::json::parse(event));
                } catch (...) {
                  events.push_back(event);
                }
              }
              event_queue_.clear();
            }

            res.set_content(events.dump(), "application/json");
          });

  // API: Subscribe to Report
  svr.Post("/api/mms/subscribe", [&](const httplib::Request &req,
                                     httplib::Response &res) {
    try {
      auto json = nlohmann::json::parse(req.body);
      std::string iedName = json["ied"];
      std::string rcbRef = json["rcb_ref"];

      auto it = mms_connections.find(iedName);
      if (it == mms_connections.end() || !it->second->isConnected()) {
        res.set_content(
            "{\"success\": false, \"message\": \"Not connected to IED\"}",
            "application/json");
        return;
      }

      // Define callback
      auto callback =
          [this, iedName](
              const std::string &rcb, const std::string &rptId,
              const std::string &dsRef,
              const std::vector<std::pair<std::string, std::string>> &values) {
            nlohmann::json msg;
            msg["type"] = "report";
            msg["ied"] = iedName;
            msg["rcb"] = rcb;
            msg["rptId"] = rptId;
            msg["dataset"] = dsRef;

            nlohmann::json vals = nlohmann::json::object();
            for (const auto &p : values) {
              vals[p.first] = p.second;
            }
            msg["values"] = vals;
            auto now = std::chrono::system_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch()) %
                      1000;
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t), "%H:%M:%S") << '.'
               << std::setfill('0') << std::setw(3) << ms.count();
            msg["timestamp"] = ss.str();

            this->broadcast(msg.dump());
          };

      // CRITICAL: Run subscription in detached thread to prevent blocking HTTP
      // server If subscribeReport blocks (e.g., invalid RCB), the HTTP server
      // can still respond
      auto conn = it->second;
      std::thread([conn, rcbRef, callback]() {
        try {
          conn->subscribeReport(rcbRef, callback);
        } catch (const std::exception &e) {
          // Log error but don't crash
          LOG_ERROR("Subscription to {} failed: {}", rcbRef, e.what());
        }
      }).detach();

      // Return success immediately - actual subscription happens in background
      nlohmann::json response;
      response["success"] = true;
      response["sub_id"] = rcbRef;
      response["message"] = "Subscription initiated for " + rcbRef;
      res.set_content(response.dump(), "application/json");

    } catch (const std::exception &e) {
      res.status = 500;
      nlohmann::json j;
      j["success"] = false;
      j["message"] = e.what();
      res.set_content(j.dump(), "application/json");
    }
  });

  // Get Report Control Blocks
  svr.Get("/api/mms/rcbs", [&](const httplib::Request &req,
                               httplib::Response &res) {
    auto ied = req.get_param_value("ied");
    if (ied.empty()) {
      res.status = 400;
      res.set_content(
          "{\"success\": false, \"message\": \"Missing ied parameter\"}",
          "application/json");
      return;
    }

    if (mms_connections.find(ied) == mms_connections.end() ||
        !mms_connections[ied]->isConnected()) {
      res.status = 400;
      res.set_content(
          "{\"success\": false, \"message\": \"Not connected to IED\"}",
          "application/json");
      return;
    }

    try {
      auto rcbs = mms_connections[ied]->getReportControlBlocks();
      nlohmann::json j;
      j["success"] = true;
      j["rcbs"] = rcbs;
      res.set_content(j.dump(), "application/json");
    } catch (const std::exception &e) {
      res.status = 500;
      nlohmann::json j;
      j["success"] = false;
      j["message"] = e.what();
      res.set_content(j.dump(), "application/json");
    }
  });

  // API: Unsubscribe from Report
  svr.Post("/api/mms/unsubscribe",
           [&](const httplib::Request &req, httplib::Response &res) {
             try {
               auto json = nlohmann::json::parse(req.body);
               std::string iedName = json["ied"];
               std::string subId = json["sub_id"]; // This is the rcbRef

               auto it = mms_connections.find(iedName);
               if (it != mms_connections.end() && it->second->isConnected()) {
                 it->second->unsubscribeReport(subId);
               }

               nlohmann::json response;
               response["success"] = true;
               response["message"] = "Unsubscribed";
               res.set_content(response.dump(), "application/json");

             } catch (const std::exception &e) {
               nlohmann::json response;
               response["success"] = false;
               response["message"] = e.what();
               res.set_content(response.dump(), "application/json");
             }
           });

  // API: Status
  svr.Get("/api/status", [](const httplib::Request &, httplib::Response &res) {
    res.set_content("{\"status\": \"online\", \"cpu\": 24, \"ieds\": 12}",
                    "application/json");
  });

  // API: Get Gateway Configuration
  svr.Get("/api/config", [](const httplib::Request &, httplib::Response &res) {
    // Read gateway.yaml and return as JSON
    std::string filepath = "./config/gateway.yaml";
    std::ifstream file(filepath);

    if (!file.good()) {
      res.status = 404;
      res.set_content("{\"error\": \"Config file not found\"}",
                      "application/json");
      return;
    }

    // Simple YAML to JSON conversion for IEDs section
    nlohmann::json config;
    config["ieds"] = nlohmann::json::array();

    std::string line;
    std::string currentied;
    nlohmann::json iedInfo;
    bool inIeds = false;

    while (std::getline(file, line)) {
      // Trim leading spaces
      size_t start = line.find_first_not_of(" \t");
      if (start == std::string::npos)
        continue;
      line = line.substr(start);

      if (line.find("ieds:") == 0) {
        inIeds = true;
        continue;
      }

      if (inIeds) {
        if (line.find("- name:") == 0) {
          // Save previous IED if exists
          if (!iedInfo.empty()) {
            config["ieds"].push_back(iedInfo);
            iedInfo.clear();
          }
          // Extract name
          size_t pos = line.find("\"");
          if (pos != std::string::npos) {
            size_t end = line.find("\"", pos + 1);
            iedInfo["name"] = line.substr(pos + 1, end - pos - 1);
          }
        } else if (line.find("ip:") != std::string::npos) {
          size_t pos = line.find("\"");
          if (pos != std::string::npos) {
            size_t end = line.find("\"", pos + 1);
            iedInfo["ip"] = line.substr(pos + 1, end - pos - 1);
          }
        } else if (line.find("port:") != std::string::npos) {
          size_t pos = line.find(":") + 1;
          std::string portStr = line.substr(pos);
          portStr.erase(0, portStr.find_first_not_of(" \t"));
          iedInfo["port"] = std::stoi(portStr);
        } else if (line.find("enabled:") != std::string::npos) {
          iedInfo["enabled"] = (line.find("true") != std::string::npos);
        }
      }
    }

    // Add last IED
    if (!iedInfo.empty()) {
      config["ieds"].push_back(iedInfo);
    }

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_content(config.dump(), "application/json");
  });

  // API: MMS Connect
  svr.Post("/api/mms/connect", [&](const httplib::Request &req,
                                   httplib::Response &res) {
    nlohmann::json reqBody;
    try {
      reqBody = nlohmann::json::parse(req.body);
    } catch (...) {
      res.set_content("{\"success\": false, \"message\": \"Invalid JSON\"}",
                      "application/json");
      return;
    }

    if (!reqBody.contains("ied")) {
      res.set_content(
          "{\"success\": false, \"message\": \"Missing ied parameter\"}",
          "application/json");
      return;
    }

    std::string iedName = reqBody["ied"];

    // Read config to get IED details
    std::string filepath = "./config/gateway.yaml";
    std::ifstream file(filepath);
    std::string ip;
    int port = 102;
    bool found = false;

    std::string line;
    bool inTargetIed = false;
    while (std::getline(file, line)) {
      size_t start = line.find_first_not_of(" \t");
      if (start == std::string::npos)
        continue;
      line = line.substr(start);

      if (line.find("- name:") == 0 &&
          line.find(iedName) != std::string::npos) {
        inTargetIed = true;
        found = true;
      } else if (inTargetIed) {
        if (line.find("ip:") != std::string::npos) {
          size_t pos = line.find("\"");
          if (pos != std::string::npos) {
            size_t end = line.find("\"", pos + 1);
            ip = line.substr(pos + 1, end - pos - 1);
          }
        } else if (line.find("port:") != std::string::npos) {
          size_t pos = line.find(":") + 1;
          std::string portStr = line.substr(pos);
          portStr.erase(0, portStr.find_first_not_of(" \t"));
          port = std::stoi(portStr);
          break; // Got all we need
        }
      }
    }

    if (!found) {
      res.set_content(
          "{\"success\": false, \"message\": \"IED not found in config\"}",
          "application/json");
      return;
    }

    // Create MMS connection
    auto conn =
        std::make_shared<gateway::iec61850::mms::MMSConnection>(ip, port);
    bool connected = conn->connect();

    nlohmann::json response;
    if (connected) {
      mms_connections[iedName] = conn;
      response["success"] = true;
      response["message"] =
          "Connected to " + iedName + " at " + ip + ":" + std::to_string(port);
      LOG_INFO("MMS API: Connected to {} at {}:{}", iedName, ip, port);

      // Generate ICD from MMS connection
      try {
        SCLGenerator sclGen(conn.get());
        std::string icdContent = sclGen.generateICD(iedName);

        // Ensure config/icd directory exists
        std::filesystem::create_directories("./config/icd");

        // Save ICD file
        std::string icdPath = "./config/icd/" + iedName + ".icd";
        std::ofstream icdFile(icdPath);
        icdFile << icdContent;
        icdFile.close();

        LOG_INFO("Generated ICD for {} at {}", iedName, icdPath);
        response["icd_generated"] = true;
        response["icd_path"] = icdPath;

        // Regenerate SCD from all ICDs and rebuild OPC UA namespace
        regenerateSCD(opcua_server_, dataBinder_);

      } catch (const std::exception &e) {
        LOG_ERROR("Failed to generate ICD for {}: {}", iedName, e.what());
        response["icd_generated"] = false;
        response["icd_error"] = e.what();
      }

    } else {
      response["success"] = false;
      response["message"] = "Failed to connect to " + iedName;
      LOG_ERROR("MMS API: Failed to connect to {} at {}:{}", iedName, ip, port);
    }

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_content(response.dump(), "application/json");
  });

  // API: MMS Disconnect
  svr.Post("/api/mms/disconnect", [](const httplib::Request &req,
                                     httplib::Response &res) {
    nlohmann::json reqBody;
    try {
      reqBody = nlohmann::json::parse(req.body);
    } catch (...) {
      res.set_content("{\"success\": false, \"message\": \"Invalid JSON\"}",
                      "application/json");
      return;
    }

    if (!reqBody.contains("ied")) {
      res.set_content(
          "{\"success\": false, \"message\": \"Missing ied parameter\"}",
          "application/json");
      return;
    }

    std::string iedName = reqBody["ied"];

    auto it = mms_connections.find(iedName);
    if (it != mms_connections.end()) {
      it->second->disconnect();
      mms_connections.erase(it);
    }

    nlohmann::json response;
    response["success"] = true;
    response["message"] = "Disconnected from " + iedName;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_content(response.dump(), "application/json");
    LOG_INFO("MMS API: Disconnected from {}", iedName);
  });

  // API: MMS Read
  svr.Get("/api/mms/read", [](const httplib::Request &req,
                              httplib::Response &res) {
    // Get parameters
    if (!req.has_param("ied") || !req.has_param("ref")) {
      res.set_content(
          "{\"success\": false, \"message\": \"Missing parameters\"}",
          "application/json");
      return;
    }

    std::string iedName = req.get_param_value("ied");
    std::string dataRef = req.get_param_value("ref");
    std::string fc = req.has_param("fc") ? req.get_param_value("fc") : "MX";

    // Check if connected
    auto it = mms_connections.find(iedName);
    if (it == mms_connections.end() || !it->second->isConnected()) {
      res.set_content(
          "{\"success\": false, \"message\": \"Not connected to IED\"}",
          "application/json");
      return;
    }

    // Real MMS Read
    try {
      auto result = it->second->readData(dataRef, fc);

      nlohmann::json response;
      response["success"] = true;
      response["value"] = result.value;
      response["type"] = result.type;
      response["quality"] = result.quality;

      // Use current time if timestamp not available from device
      // TODO: Use result.timestamp if available
      auto now = std::chrono::system_clock::now();
      auto time_t = std::chrono::system_clock::to_time_t(now);
      std::stringstream ss;
      ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
      response["timestamp"] = ss.str();

      res.set_header("Access-Control-Allow-Origin", "*");
      res.set_content(response.dump(), "application/json");
      LOG_DEBUG("MMS API: Read {} from {}: {}", dataRef, iedName, result.value);
    } catch (const std::exception &e) {
      nlohmann::json response;
      response["success"] = false;
      response["message"] = e.what();

      res.set_header("Access-Control-Allow-Origin", "*");
      res.set_content(response.dump(), "application/json");
      LOG_ERROR("MMS API: Failed to read {} from {}: {}", dataRef, iedName,
                e.what());
    }
  });

  // API: MMS Write
  svr.Post("/api/mms/write", [](const httplib::Request &req,
                                httplib::Response &res) {
    nlohmann::json reqBody;
    try {
      reqBody = nlohmann::json::parse(req.body);
    } catch (...) {
      res.set_content("{\"success\": false, \"message\": \"Invalid JSON\"}",
                      "application/json");
      return;
    }

    if (!reqBody.contains("ied") || !reqBody.contains("ref") ||
        !reqBody.contains("type") || !reqBody.contains("value")) {
      res.set_content(
          "{\"success\": false, \"message\": \"Missing parameters\"}",
          "application/json");
      return;
    }

    std::string iedName = reqBody["ied"];
    std::string dataRef = reqBody["ref"];
    std::string type = reqBody["type"];
    std::string value =
        reqBody["value"]; // Value as string, will be converted in MMSConnection

    auto it = mms_connections.find(iedName);
    if (it == mms_connections.end() || !it->second->isConnected()) {
      res.set_content(
          "{\"success\": false, \"message\": \"Not connected to IED\"}",
          "application/json");
      return;
    }

    try {
      it->second->writeData(dataRef, type, value);

      nlohmann::json response;
      response["success"] = true;
      response["message"] = "Write successful";

      res.set_header("Access-Control-Allow-Origin", "*");
      res.set_content(response.dump(), "application/json");
      LOG_INFO("MMS API: Wrote {} to {} on {}", value, dataRef, iedName);
    } catch (const std::exception &e) {
      nlohmann::json response;
      response["success"] = false;
      response["message"] = e.what();

      res.set_header("Access-Control-Allow-Origin", "*");
      res.set_content(response.dump(), "application/json");
      LOG_ERROR("MMS API: Failed to write to {}: {}", iedName, e.what());
    }
  });

  // API: MMS Export SCL (ICD/CID)
  svr.Get("/api/mms/export-scl", [](const httplib::Request &req,
                                    httplib::Response &res) {
    if (!req.has_param("ied")) {
      res.set_content(
          "{\"success\": false, \"message\": \"Missing 'ied' parameter\"}",
          "application/json");
      return;
    }

    std::string iedName = req.get_param_value("ied");
    std::string format = req.get_param_value("format"); // "icd" or "cid"

    if (format.empty())
      format = "cid"; // Default to CID

    auto it = mms_connections.find(iedName);
    if (it == mms_connections.end() || !it->second->isConnected()) {
      res.set_content(
          "{\"success\": false, \"message\": \"Not connected to IED\"}",
          "application/json");
      return;
    }

    try {
      SCLGenerator generator(it->second.get());
      std::string sclContent;

      if (format == "icd") {
        sclContent = generator.generateICD(iedName);
      } else {
        sclContent = generator.generateCID(iedName);
      }

      res.set_header("Access-Control-Allow-Origin", "*");
      res.set_header("Content-Type", "application/xml");
      res.set_header("Content-Disposition",
                     "attachment; filename=\"" + iedName + "." + format + "\"");
      res.set_content(sclContent, "application/xml");

      LOG_INFO("MMS API: Exported {} for {}", format, iedName);
    } catch (const std::exception &e) {
      nlohmann::json response;
      response["success"] = false;
      response["message"] = e.what();
      res.set_content(response.dump(), "application/json");
      LOG_ERROR("MMS API: Failed to export SCL: {}", e.what());
    }
  });

  // API: MMS Browse - Get data model
  svr.Get("/api/mms/browse", [](const httplib::Request &req,
                                httplib::Response &res) {
    if (!req.has_param("ied")) {
      res.set_content(
          "{\"success\": false, \"message\": \"Missing ied parameter\"}",
          "application/json");
      return;
    }

    std::string iedName = req.get_param_value("ied");

    // Check if connected
    auto it = mms_connections.find(iedName);
    if (it == mms_connections.end() || !it->second->isConnected()) {
      res.set_content(
          "{\"success\": false, \"message\": \"Not connected to IED\"}",
          "application/json");
      return;
    }

    // Dynamic Discovery
    try {
      nlohmann::json response;
      response["success"] = true;
      response["nodes"] = nlohmann::json::array();

      // 1. Get Logical Devices
      auto devices = it->second->getLogicalDeviceList();

      for (const auto &ld : devices) {
        // 2. For each LD, get Logical Nodes
        try {
          auto lns = it->second->getLogicalDeviceVariables(ld);
          for (const auto &ln : lns) {
            // 3. For each LN, get Data Objects
            try {
              auto dos = it->second->getLogicalNodeVariables(ld, ln);
              for (const auto &doName : dos) {
                nlohmann::json node;
                node["name"] = ld + "/" + ln + "." + doName;
                // Try to guess the type/FC based on name for now
                // In a real implementation, we would inspect the DO to find its
                // FC/Type
                if (doName.find("AnIn") != std::string::npos) {
                  node["reference"] = ld + "/" + ln + "." + doName + ".mag.f";
                  node["type"] = "Analog Input";
                  node["fc"] = "MX";
                } else if (doName.find("SPCSO") != std::string::npos) {
                  node["reference"] = ld + "/" + ln + "." + doName;
                  node["type"] = "Digital Output";
                  node["fc"] = "ST";
                } else {
                  // Generic/Unknown
                  node["reference"] = ld + "/" + ln + "." + doName;
                  node["type"] = "Data Object";
                  node["fc"] = "MX"; // Default
                }

                response["nodes"].push_back(node);
              }
            } catch (...) {
              // Ignore errors for specific LNs
            }
          }
        } catch (...) {
          // Ignore errors for specific LDs
        }
      }

      res.set_header("Access-Control-Allow-Origin", "*");
      res.set_content(response.dump(), "application/json");
      LOG_INFO("MMS API: Browsed {} nodes for {}", response["nodes"].size(),
               iedName);

    } catch (const std::exception &e) {
      res.set_content("{\"success\": false, \"message\": \"Discovery failed: " +
                          std::string(e.what()) + "\"}",
                      "application/json");
    }
  });

  // API: Topology
  svr.Get("/api/v1/topology", [](const httplib::Request &,
                                 httplib::Response &res) {
    // In production, use actual SCD file path from config
    // For now, generate mock topology
    gateway::topology::TopologyParser parser;

    // Try to load from configured SCD file
    // If file doesn't exist, parser will return default topology
    auto topology = parser.parseFromSCD("./config/station.scd");

    std::string jsonResponse = parser.toJSON(topology);

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_content(jsonResponse, "application/json");

    LOG_INFO("Topology API called, returned {} IEDs", topology.nodes.size());
  });

  // API: Upload SCD file (Multipart support TODO - requires httplib
  // configuration)
  svr.Post("/api/v1/config/scd",
           [](const httplib::Request &req, httplib::Response &res) {
             // Temporary: File upload via multipart requires proper httplib
             // setup For now, return not implemented
             nlohmann::json response;
             response["success"] = false;
             response["message"] = "File upload via UI not yet implemented. "
                                   "Please use file system directly.";
             res.set_content(response.dump(), "application/json");
             LOG_INFO("SCD upload attempted (not yet implemented via HTTP)");
           });

  // API: List SCD files
  svr.Get("/api/v1/config/scd/list", [](const httplib::Request &,
                                        httplib::Response &res) {
    nlohmann::json files = nlohmann::json::array();

    // Read config directory
    std::string configDir = "./config";
    std::string activeFile = "station.scd";

    // Automatically scan for .scd files using filesystem
    try {
      for (const auto &entry : std::filesystem::directory_iterator(configDir)) {
        if (entry.is_regular_file()) {
          std::string filename = entry.path().filename().string();

          // Check if file has .scd extension
          if (filename.size() >= 4 &&
              filename.substr(filename.size() - 4) == ".scd") {

            std::ifstream file(entry.path(), std::ios::binary | std::ios::ate);
            if (file.good()) {
              auto fileSize = file.tellg();
              nlohmann::json fileInfo;
              fileInfo["filename"] = filename;
              fileInfo["size"] = static_cast<long>(fileSize);
              fileInfo["active"] = (filename == activeFile);
              files.push_back(fileInfo);
            }
          }
        }
      }
    } catch (const std::filesystem::filesystem_error &e) {
      nlohmann::json error;
      error["error"] = "Failed to scan config directory";
      error["message"] = e.what();
      res.set_content(error.dump(), "application/json");
      return;
    }

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_content(files.dump(), "application/json");
  });

  // API: Activate SCD file
  svr.Post("/api/v1/config/scd/activate",
           [](const httplib::Request &req, httplib::Response &res) {
             nlohmann::json reqBody;
             try {
               reqBody = nlohmann::json::parse(req.body);
             } catch (...) {
               res.set_content("{\"success\": false, \"message\": \"Invalid "
                               "JSON\"}",
                               "application/json");
               return;
             }

             if (!reqBody.contains("filename")) {
               res.set_content("{\"success\": false, \"message\": \"Missing "
                               "filename\"}",
                               "application/json");
               return;
             }

             std::string filename = reqBody["filename"];
             std::string sourcePath = "./config/" + filename;
             std::string targetPath = "./config/station.scd";

             // Create symlink or copy file
             std::filesystem::remove(targetPath);
             try {
               std::filesystem::copy_file(
                   sourcePath, targetPath,
                   std::filesystem::copy_options::overwrite_existing);

               nlohmann::json response;
               response["success"] = true;
               response["message"] = "Configuration activated";
               response["filename"] = filename;

               res.set_content(response.dump(), "application/json");
               LOG_INFO("Activated SCD file: {}", filename);
             } catch (const std::exception &e) {
               nlohmann::json response;
               response["success"] = false;
               response["message"] = std::string("Failed: ") + e.what();
               res.set_content(response.dump(), "application/json");
             }
           });

  // API: Download active SCD
  svr.Get("/api/v1/config/scd/download",
          [](const httplib::Request &, httplib::Response &res) {
            std::string filepath = "./config/station.scd";
            std::ifstream file(filepath, std::ios::binary);

            if (file.good()) {
              std::string content((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
              res.set_content(content, "application/xml");
              res.set_header("Content-Disposition",
                             "attachment; filename=\"station.scd\"");
            } else {
              res.status = 404;
            }
          });

  // API: Download gateway config
  svr.Get("/api/v1/config/gateway/download",
          [](const httplib::Request &, httplib::Response &res) {
            std::string filepath = "./config/gateway.yaml";
            std::ifstream file(filepath);

            if (file.good()) {
              std::string content((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
              res.set_content(content, "application/x-yaml");
              res.set_header("Content-Disposition",
                             "attachment; filename=\"gateway.yaml\"");
            } else {
              res.status = 404;
            }
          });

  // ========== OPC UA Server API ==========

  // API: Get OPC UA Server status
  svr.Get("/api/opcua/status", [&](const httplib::Request &,
                                   httplib::Response &res) {
    nlohmann::json response;

    if (opcua_server_) {
      response["success"] = true;
      response["running"] = opcua_server_->isRunning();
      response["port"] = 4840;
      response["endpoint"] = "opc.tcp://localhost:4840";
      response["message"] = opcua_server_->isRunning() ? "Server is running"
                                                       : "Server is stopped";
    } else {
      response["success"] = false;
      response["running"] = false;
      response["message"] = "OPC UA Server not initialized";
    }

    res.set_content(response.dump(), "application/json");
  });

  // Start a separate thread for broadcasting updates
  std::thread broadcaster([this]() {
    while (running_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(updateIntervalMs_));
      // Broadcast logic here
      // broadcast("...");
      LOG_DEBUG("Broadcasting update (Interval: {}ms)", updateIntervalMs_);
    }
  });
  broadcaster.detach();

  LOG_INFO("Starting REST API Server on port {}", port_);

  // Listen (Blocking)
  svr.listen("0.0.0.0", port_);
}

} // namespace api
} // namespace gateway
