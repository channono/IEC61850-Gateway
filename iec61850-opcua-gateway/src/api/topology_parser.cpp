#include "topology_parser.h"
#include "../core/logger.h"
#include <map>
#include <nlohmann/json.hpp>
#include <set>

using json = nlohmann::json;

namespace gateway {
namespace topology {

TopologyParser::TopologyParser() {}

TopologyParser::~TopologyParser() {}

TopologyInfo TopologyParser::parseFromSCD(const std::string &scdPath) {
  TopologyInfo topology;

  // Load SCD file
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(scdPath.c_str());

  if (!result) {
    LOG_ERROR("Failed to load SCD file: {}", scdPath);
    // Return empty topology with defaults
    topology.type = RedundancyType::PRP;
    topology.networks = {"eth0", "eth1"};
    return topology;
  }

  LOG_INFO("Parsing SCD file: {}", scdPath);

  // Detect redundancy type
  topology.type = detectRedundancyType(doc);
  LOG_INFO("Detected network type: {}", redundancyTypeToString(topology.type));

  // Extract topology components
  topology.nodes = extractIEDs(doc);
  topology.networks = extractNetworks(doc);

  // Generate connections based on topology type
  if (topology.type == RedundancyType::HSR) {
    // HSR: Create daisy-chain ring connections (all nodes in sequence)
    // All nodes including gateway are part of the ring
    const auto &ringNodes = topology.nodes;

    // Create ring connections (each node connects to the next)
    for (size_t i = 0; i < ringNodes.size(); i++) {
      GOOSEConnection conn;
      conn.from = ringNodes[i].id;
      conn.to = ringNodes[(i + 1) % ringNodes.size()]
                    .id; // Next in ring, wraps to first
      conn.type = "GOOSE";
      conn.gocbRef = ringNodes[i].id + "_GOOSE";
      conn.networks = {"hsr-ring"};
      topology.connections.push_back(conn);
    }

    LOG_INFO("Created HSR ring with {} nodes and {} connections",
             ringNodes.size(), topology.connections.size());
  } else if (topology.type == RedundancyType::PRP) {
    // PRP: All IEDs connect to gateway
    for (const auto &node : topology.nodes) {
      if (node.type != "GATEWAY") {
        GOOSEConnection conn;
        conn.from = node.id;
        conn.to = "gateway";
        conn.type = "GOOSE";
        conn.gocbRef = node.id + "_GOOSE";
        conn.networks = {"eth0", "eth1"};
        topology.connections.push_back(conn);
      }
    }

    LOG_INFO("Created PRP star topology with {} connections",
             topology.connections.size());
  } else {
    // Fallback: extract from SCD (old logic)
    topology.connections = extractConnections(doc);
  }

  // Mock network statistics (in production, get from actual network monitoring)
  topology.networkA.interface =
      topology.networks.size() > 0 ? topology.networks[0] : "eth0";
  topology.networkA.messageRate = 1234;
  topology.networkA.errors = 0;
  topology.networkA.quality = 99.8;
  topology.networkA.status = "online";

  if (topology.type == RedundancyType::PRP) {
    topology.networkB.interface =
        topology.networks.size() > 1 ? topology.networks[1] : "eth1";
    topology.networkB.messageRate = 1230;
    topology.networkB.errors = 2;
    topology.networkB.quality = 99.5;
    topology.networkB.status = "online";
  }

  LOG_INFO("Parsed {} IEDs and {} connections", topology.nodes.size(),
           topology.connections.size());

  return topology;
}

RedundancyType
TopologyParser::detectRedundancyType(const pugi::xml_document &doc) {
  // Navigate to Communication section
  auto scl = doc.child("SCL");
  if (!scl) {
    LOG_WARN("No SCL root found in document");
    return RedundancyType::NONE;
  }

  auto communication = scl.child("Communication");
  if (!communication) {
    LOG_WARN("No Communication section found");
    return RedundancyType::NONE;
  }

  // Count SubNetworks
  int subnetCount = 0;
  std::set<std::string> networkNames;

  for (auto subnet : communication.children("SubNetwork")) {
    subnetCount++;
    std::string name = subnet.attribute("name").as_string();
    networkNames.insert(name);

    // Check for HSR indicators in name
    if (name.find("HSR") != std::string::npos ||
        name.find("Ring") != std::string::npos ||
        name.find("hsr") != std::string::npos) {
      LOG_INFO("HSR detected from SubNetwork name: {}", name);
      return RedundancyType::HSR;
    }
  }

  // Check if same IEDs appear in multiple networks -> PRP
  if (subnetCount == 2) {
    std::map<std::string, int> iedCounts;

    for (auto subnet : communication.children("SubNetwork")) {
      for (auto connAP : subnet.children("ConnectedAP")) {
        std::string iedName = connAP.attribute("iedName").as_string();
        iedCounts[iedName]++;
      }
    }

    // If any IED appears in both networks -> PRP
    for (const auto &pair : iedCounts) {
      if (pair.second >= 2) {
        LOG_INFO("PRP detected: IED {} appears in multiple networks",
                 pair.first);
        return RedundancyType::PRP;
      }
    }
  }

  // Check for explicit type attributes
  for (auto subnet : communication.children("SubNetwork")) {
    std::string type = subnet.attribute("type").as_string();
    if (type.find("PRP") != std::string::npos) {
      return RedundancyType::PRP;
    }
    if (type.find("HSR") != std::string::npos) {
      return RedundancyType::HSR;
    }
  }

  // Default based on subnet count
  if (subnetCount == 2) {
    LOG_INFO("Defaulting to PRP based on dual SubNetwork");
    return RedundancyType::PRP;
  } else if (subnetCount == 1) {
    LOG_INFO("Defaulting to HSR based on single SubNetwork");
    return RedundancyType::HSR;
  }

  return RedundancyType::NONE;
}

std::vector<IEDNode>
TopologyParser::extractIEDs(const pugi::xml_document &doc) {
  std::vector<IEDNode> nodes;
  std::map<std::string, IEDNode> iedMap;

  auto scl = doc.child("SCL");
  if (!scl)
    return nodes;

  // Extract IEDs from IED section
  for (auto ied : scl.children("IED")) {
    IEDNode node;
    node.id = ied.attribute("name").as_string();
    node.name = node.id;
    node.type = ied.attribute("type").as_string("IED");
    node.status = "online";
    node.goosePublished = 0;
    node.gooseSubscribed = 0;

    iedMap[node.id] = node;
  }

  // Extract IP addresses from Communication section
  auto communication = scl.child("Communication");
  if (communication) {
    for (auto subnet : communication.children("SubNetwork")) {
      for (auto connAP : subnet.children("ConnectedAP")) {
        std::string iedName = connAP.attribute("iedName").as_string();

        // Find IP address
        for (auto address : connAP.children("Address")) {
          for (auto p : address.children("P")) {
            std::string pType = p.attribute("type").as_string();
            if (pType == "IP" || pType == "IP-ADDR") {
              if (iedMap.find(iedName) != iedMap.end()) {
                iedMap[iedName].ip = p.child_value();
              }
            }
          }
        }
      }
    }
  }

  // Count GOOSE publications/subscriptions
  for (auto ied : scl.children("IED")) {
    std::string iedName = ied.attribute("name").as_string();

    // Count published GOOSE
    auto accessPoint = ied.child("AccessPoint");
    if (accessPoint) {
      auto server = accessPoint.child("Server");
      if (server) {
        for (auto ln : server.select_nodes(".//LN")) {
          for (auto gse : ln.node().children("GSEControl")) {
            if (iedMap.find(iedName) != iedMap.end()) {
              iedMap[iedName].goosePublished++;
            }
          }
        }
      }
    }
  }

  // Convert map to vector
  for (const auto &pair : iedMap) {
    nodes.push_back(pair.second);
  }

  // Add gateway node
  IEDNode gateway;
  gateway.id = "gateway";
  gateway.name = "Gateway";
  gateway.type = "GATEWAY";
  gateway.ip = "192.168.1.1";
  gateway.status = "online";
  gateway.goosePublished = 0;
  gateway.gooseSubscribed =
      nodes.size() * 2; // Subscribes to all IEDs on both networks
  nodes.push_back(gateway);

  return nodes;
}

std::vector<GOOSEConnection>
TopologyParser::extractConnections(const pugi::xml_document &doc) {
  std::vector<GOOSEConnection> connections;
  std::set<std::string> uniqueConnections;

  auto scl = doc.child("SCL");
  if (!scl)
    return connections;

  // Extract GOOSE connections from DataTypeTemplates/GSEControl
  for (auto ied : scl.children("IED")) {
    std::string publisherIED = ied.attribute("name").as_string();

    auto accessPoint = ied.child("AccessPoint");
    if (!accessPoint)
      continue;

    auto server = accessPoint.child("Server");
    if (!server)
      continue;

    // Find GOOSE control blocks
    for (auto ln : server.select_nodes(".//LN | .//LN0")) {
      auto lnNode = ln.node();

      for (auto gseControl : lnNode.children("GSEControl")) {
        std::string gocbRef = publisherIED + "/" +
                              lnNode.attribute("inst").as_string() + "$GO$" +
                              gseControl.attribute("name").as_string();

        // Note: Full implementation would extract subscribers from Inputs
        // section For now, connections will be generated based on topology type
        // This is handled in parseFromSCD() after type detection
      }
    }
  }

  // Connections are now generated after topology type detection
  // See parseFromSCD() for actual connection creation
  return connections;
}

std::vector<std::string>
TopologyParser::extractNetworks(const pugi::xml_document &doc) {
  std::vector<std::string> networks;

  auto scl = doc.child("SCL");
  if (!scl)
    return networks;

  auto communication = scl.child("Communication");
  if (!communication)
    return networks;

  for (auto subnet : communication.children("SubNetwork")) {
    std::string name = subnet.attribute("name").as_string();
    if (!name.empty()) {
      // Convert to interface names
      if (name.find("A") != std::string::npos || name == "SubNetwork1") {
        networks.push_back("eth0");
      } else if (name.find("B") != std::string::npos || name == "SubNetwork2") {
        networks.push_back("eth1");
      } else if (name.find("HSR") != std::string::npos) {
        networks.push_back("hsr-ring");
      } else {
        networks.push_back(name);
      }
    }
  }

  // Default to dual network if none found
  if (networks.empty()) {
    networks = {"eth0", "eth1"};
  }

  return networks;
}

std::string TopologyParser::toJSON(const TopologyInfo &topology) {
  json j;

  j["type"] = redundancyTypeToString(topology.type);
  j["timestamp"] = ""; // Could add timestamp here

  // Nodes
  j["nodes"] = json::array();
  for (const auto &node : topology.nodes) {
    json nodeJson;
    nodeJson["id"] = node.id;
    nodeJson["name"] = node.name;
    nodeJson["type"] = node.type;
    nodeJson["ip"] = node.ip;
    nodeJson["status"] = node.status;
    nodeJson["goosePublished"] = node.goosePublished;
    nodeJson["gooseSubscribed"] = node.gooseSubscribed;
    j["nodes"].push_back(nodeJson);
  }

  // Connections
  j["connections"] = json::array();
  for (const auto &conn : topology.connections) {
    json connJson;
    connJson["from"] = conn.from;
    connJson["to"] = conn.to;
    connJson["type"] = conn.type;
    connJson["gocbRef"] = conn.gocbRef;
    connJson["networks"] = conn.networks;
    j["connections"].push_back(connJson);
  }

  // Networks
  j["networks"] = topology.networks;

  // Statistics
  j["statistics"]["networkA"] = {{"interface", topology.networkA.interface},
                                 {"messageRate", topology.networkA.messageRate},
                                 {"errors", topology.networkA.errors},
                                 {"quality", topology.networkA.quality},
                                 {"status", topology.networkA.status}};

  if (topology.type == RedundancyType::PRP) {
    j["statistics"]["networkB"] = {
        {"interface", topology.networkB.interface},
        {"messageRate", topology.networkB.messageRate},
        {"errors", topology.networkB.errors},
        {"quality", topology.networkB.quality},
        {"status", topology.networkB.status}};
  }

  return j.dump(2); // Pretty print with 2 spaces
}

std::string TopologyParser::redundancyTypeToString(RedundancyType type) {
  switch (type) {
  case RedundancyType::PRP:
    return "PRP";
  case RedundancyType::HSR:
    return "HSR";
  case RedundancyType::NONE:
  default:
    return "NONE";
  }
}

} // namespace topology
} // namespace gateway
