#include "scl_parser.h"
#include "core/logger.h"
#include <iostream>

namespace gateway {
namespace iec61850 {

SCLParser::SCLParser() {}

SCLParser::~SCLParser() {}

SCLFileType SCLParser::detectFileType(const std::string &filepath) {
  pugi::xml_document doc;
  auto result = doc.load_file(filepath.c_str());

  if (!result) {
    LOG_ERROR("Failed to load SCL file {}: {}", filepath, result.description());
    return SCLFileType::UNKNOWN;
  }

  auto scl = doc.child("SCL");
  if (!scl)
    return SCLFileType::UNKNOWN;

  auto header = scl.child("Header");
  if (header) {
    std::string id = header.attribute("id").as_string();
    if (id.find("SCD") != std::string::npos)
      return SCLFileType::SCD;
    if (id.find("ICD") != std::string::npos)
      return SCLFileType::ICD;
    if (id.find("CID") != std::string::npos)
      return SCLFileType::CID;
  }

  // Fallback detection logic based on content
  bool hasSubstation = scl.child("Substation") != nullptr;
  bool hasCommunication = scl.child("Communication") != nullptr;
  size_t iedCount = 0;
  for (auto ied : scl.children("IED")) {
    iedCount++;
  }

  if (hasSubstation && hasCommunication && iedCount > 1)
    return SCLFileType::SCD;
  if (iedCount == 1 && hasCommunication)
    return SCLFileType::CID;
  if (iedCount == 1)
    return SCLFileType::ICD;

  return SCLFileType::UNKNOWN;
}

std::vector<IEDConfig> SCLParser::parse(const std::string &filepath) {
  std::vector<IEDConfig> configs;
  pugi::xml_document doc;

  auto result = doc.load_file(filepath.c_str());
  if (!result) {
    LOG_ERROR("Failed to parse SCL file: {}", result.description());
    return configs;
  }

  auto scl = doc.child("SCL");

  // Parse DataTypeTemplates first to populate type map
  auto templatesNode = scl.child("DataTypeTemplates");
  if (templatesNode) {
    parseDataTypeTemplates(templatesNode);
  }

  auto commNode = scl.child("Communication");

  for (auto iedNode : scl.children("IED")) {
    configs.push_back(parseIED(iedNode, commNode));
  }

  return configs;
}

void SCLParser::parseDataTypeTemplates(const pugi::xml_node &rootNode) {
  doTypeMap_.clear();
  for (auto doType : rootNode.children("DOType")) {
    std::string id = doType.attribute("id").as_string();
    std::string cdc = doType.attribute("cdc").as_string();
    if (!id.empty() && !cdc.empty()) {
      doTypeMap_[id] = cdc;
    }
  }
  LOG_INFO("Parsed {} DOTypes from templates", doTypeMap_.size());

  // Parse LNodeTypes
  parseLNodeTypes(rootNode);
}

void SCLParser::parseLNodeTypes(const pugi::xml_node &rootNode) {
  lnTypeMap_.clear();
  for (auto lNodeType : rootNode.children("LNodeType")) {
    std::string id = lNodeType.attribute("id").as_string();
    if (id.empty())
      continue;

    std::unordered_map<std::string, std::string> doMap;
    for (auto doNode : lNodeType.children("DO")) {
      std::string name = doNode.attribute("name").as_string();
      std::string type = doNode.attribute("type").as_string();
      if (!name.empty() && !type.empty()) {
        doMap[name] = type;
      }
    }
    lnTypeMap_[id] = doMap;
  }
  LOG_INFO("Parsed {} LNodeTypes from templates", lnTypeMap_.size());
}

IEDConfig SCLParser::parseIED(const pugi::xml_node &iedNode,
                              const pugi::xml_node &commNode) {
  IEDConfig config;
  config.name = iedNode.attribute("name").as_string();
  config.type = iedNode.attribute("type").as_string();
  config.manufacturer = iedNode.attribute("manufacturer").as_string();

  // Extract IP from Communication section if available
  if (commNode) {
    std::string xpath =
        "//ConnectedAP[@iedName='" + config.name + "']/Address/P[@type='IP']";
    auto ipNode = commNode.select_node(xpath.c_str()).node();
    if (ipNode) {
      config.ip = ipNode.text().as_string();
    }
  }

  config.gooseControls = parseGOOSEControls(iedNode, commNode);
  config.logicalDevices = parseLogicalDevices(iedNode); // NEW

  LOG_INFO("Parsed IED: {}, Type: {}, IP: {}", config.name, config.type,
           config.ip);
  return config;
}

std::vector<GOOSEControlBlock>
SCLParser::parseGOOSEControls(const pugi::xml_node &iedNode,
                              const pugi::xml_node &commNode) {
  std::vector<GOOSEControlBlock> controls;
  std::string iedName = iedNode.attribute("name").as_string();

  // Find all GSEControl elements in the IED
  // Note: This is a simplified search. Real implementation needs to traverse
  // LDevice/LN0
  auto gseControls = iedNode.select_nodes(".//GSEControl");

  for (auto gse : gseControls) {
    GOOSEControlBlock gcb;
    gcb.name = gse.node().attribute("name").as_string();
    gcb.appID = gse.node().attribute("appID").as_string();
    gcb.dataSet = gse.node().attribute("datSet").as_string();

    // Try to find MAC address in Communication section
    if (commNode) {
      // XPath to find GSE element in Communication section matching cbName
      // This is complex because we need to match ldInst as well.
      // For Phase 1/2 simplified, we might just look for matching cbName for
      // this IED. A robust implementation would map LDevice inst correctly.

      // Simplified lookup
      std::string xpath = "//ConnectedAP[@iedName='" + iedName +
                          "']//GSE[@cbName='" + gcb.name +
                          "']//P[@type='MAC-Address']";
      auto macNode = commNode.select_node(xpath.c_str()).node();
      if (macNode) {
        gcb.macAddress = macNode.text().as_string();
      }

      xpath = "//ConnectedAP[@iedName='" + iedName + "']//GSE[@cbName='" +
              gcb.name + "']//P[@type='APPID']";
      auto appidNode = commNode.select_node(xpath.c_str()).node();
      if (appidNode) {
        gcb.appID = appidNode.text().as_string(); // Override if present in Comm
      }
    }

    controls.push_back(gcb);
  }

  return controls;
}

std::vector<LogicalDevice>
SCLParser::parseLogicalDevices(const pugi::xml_node &iedNode) {
  std::vector<LogicalDevice> logicalDevices;

  // Find AccessPoint/Server/LDevice nodes
  auto serverNode = iedNode.select_node(".//AccessPoint/Server").node();
  if (!serverNode) {
    return logicalDevices;
  }

  for (auto ldNode : serverNode.children("LDevice")) {
    LogicalDevice ld;
    ld.inst = ldNode.attribute("inst").as_string();
    ld.name = ld.inst; // Use inst as name

    // Parse Logical Nodes in this LD
    ld.logicalNodes = parseLogicalNodes(ldNode);

    if (!ld.logicalNodes.empty()) {
      logicalDevices.push_back(ld);
    }
  }

  return logicalDevices;
}

std::vector<LogicalNode>
SCLParser::parseLogicalNodes(const pugi::xml_node &ldNode) {
  std::vector<LogicalNode> logicalNodes;

  // Parse LN0 (special logical node)
  auto ln0Node = ldNode.child("LN0");
  if (ln0Node) {
    LogicalNode ln;
    ln.name = "LLN0";
    ln.lnClass = "LLN0";
    ln.inst = "";
    ln.dataObjects = parseDataObjects(ln0Node);
    logicalNodes.push_back(ln);
  }

  // Parse regular LN nodes
  for (auto lnNode : ldNode.children("LN")) {
    LogicalNode ln;
    ln.lnClass = lnNode.attribute("lnClass").as_string();
    ln.inst = lnNode.attribute("inst").as_string();
    ln.name = ln.lnClass + ln.inst; // e.g., "GGIO1"
    ln.dataObjects = parseDataObjects(lnNode);
    logicalNodes.push_back(ln);
  }

  return logicalNodes;
}

std::vector<DataObject>
SCLParser::parseDataObjects(const pugi::xml_node &lnNode) {
  std::vector<DataObject> dataObjects;
  std::string lnType = lnNode.attribute("lnType").as_string();

  for (auto doiNode : lnNode.children("DOI")) {
    DataObject dobj;
    dobj.name = doiNode.attribute("name").as_string();

    // Resolve type from map if available
    std::string typeId = doiNode.attribute("type").as_string();

    // Fallback: Look up in LNodeType template if type is missing
    if (typeId.empty() && !lnType.empty()) {
      if (lnTypeMap_.count(lnType) && lnTypeMap_[lnType].count(dobj.name)) {
        typeId = lnTypeMap_[lnType][dobj.name];
        LOG_INFO("Resolved missing type for DOI {}: {} -> {}", dobj.name,
                 lnType, typeId);
      }
    }

    if (!typeId.empty() && doTypeMap_.count(typeId)) {
      dobj.type = doTypeMap_[typeId]; // e.g. "SPS"
    } else {
      // Fallback or keep empty/unknown
      dobj.type = typeId.empty() ? "Unknown" : typeId;
    }

    dataObjects.push_back(dobj);
  }

  return dataObjects;
}

} // namespace iec61850
} // namespace gateway
