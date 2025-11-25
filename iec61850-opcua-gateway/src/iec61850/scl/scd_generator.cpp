#include "scd_generator.h"
#include "core/logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <unordered_set>

namespace gateway {
namespace iec61850 {
namespace scl {

SCDGenerator::SCDGenerator() {}
SCDGenerator::~SCDGenerator() {}

void SCDGenerator::addICD(const std::string &filepath) {
  auto doc = std::make_shared<pugi::xml_document>();
  if (!doc->load_file(filepath.c_str())) {
    LOG_ERROR("Failed to load ICD: {}", filepath);
    return;
  }

  auto iedNode = doc->select_node("//IED").node();
  if (!iedNode) {
    LOG_ERROR("No IED found in: {}", filepath);
    return;
  }

  IEDInfo ied;
  ied.name = iedNode.attribute("name").as_string();
  ied.type = iedNode.attribute("type").as_string();
  ied.manufacturer = iedNode.attribute("manufacturer").as_string();
  ied.doc = doc; // Keep document alive
  ied.iedNode = iedNode;

  LOG_INFO("Added ICD for IED: {}", ied.name);
  ieds_.push_back(ied);
}

void SCDGenerator::configureNetwork(
    const std::unordered_map<std::string, NetworkConfig> &configs,
    bool autoAssign) {
  networkConfigs_ = configs;
  if (autoAssign) {
    autoAssignIPs();
  }
}

void SCDGenerator::configureGOOSE(
    const std::vector<GOOSEPublisher> &publishers) {
  goosePublishers_ = publishers;
  autoAssignGOOSE();
}

std::string SCDGenerator::generate(bool includeSubstation) {
  pugi::xml_document scd;
  auto scl = scd.append_child("SCL");
  scl.append_attribute("version") = "2007";
  scl.append_attribute("revision") = "B";
  scl.append_attribute("xmlns") = "http://www.iec.ch/61850/2003/SCL";

  auto header = scl.append_child("Header");
  header.append_attribute("id") = "Generated_SCD";

  if (includeSubstation) {
    generateSubstation(scl.append_child("Substation"));
  }

  generateCommunication(scl.append_child("Communication"));

  // Add IEDs with all attributes and children (Deep Copy)
  for (const auto &ied : ieds_) {
    // Deep copy the entire IED node from the source document
    scl.append_copy(ied.iedNode);
  }

  // Merge DataTypeTemplates from all ICDs
  auto templates = scl.append_child("DataTypeTemplates");
  std::unordered_set<std::string> existingTypes;

  for (const auto &ied : ieds_) {
    if (!ied.doc)
      continue;

    auto sourceTemplates = ied.doc->child("SCL").child("DataTypeTemplates");
    if (!sourceTemplates)
      continue;

    for (auto typeNode : sourceTemplates.children()) {
      std::string id = typeNode.attribute("id").as_string();
      if (!id.empty() && existingTypes.find(id) == existingTypes.end()) {
        templates.append_copy(typeNode);
        existingTypes.insert(id);
      }
    }
  }

  // Inject Standard DPL Type and Fix References
  ensurePhyNamType(templates);
  fixPhyNamReferences(templates);

  std::stringstream ss;
  scd.save(ss, "  ");
  return ss.str();
}

void SCDGenerator::ensurePhyNamType(pugi::xml_node templates) {
  // Check if DPL_1_PhyNam already exists
  if (templates.find_child_by_attribute("DOType", "id", "DPL_1_PhyNam")) {
    return;
  }

  LOG_INFO("Injecting standard DPL_1_PhyNam DOType");

  auto doType = templates.append_child("DOType");
  doType.append_attribute("id") = "DPL_1_PhyNam";
  doType.append_attribute("cdc") = "DPL";

  // Add standard attributes
  struct DAInfo {
    const char *name;
    const char *fc;
  };
  DAInfo das[] = {{"vendor", "DC"},
                  {"swRev", "DC"},
                  {"d", "DC"},
                  {"configRev", "DC"},
                  {"ldNs", "EX"}};

  for (const auto &da : das) {
    auto daNode = doType.append_child("DA");
    daNode.append_attribute("name") = da.name;
    daNode.append_attribute("fc") = da.fc;
    daNode.append_attribute("bType") = "VisString255";
    daNode.append_attribute("count") = "0"; // Not an array
  }
}

void SCDGenerator::fixPhyNamReferences(pugi::xml_node templates) {
  // 1. Ensure LPHD_Type exists (since it's missing in the source ICD)
  if (!templates.find_child_by_attribute("LNodeType", "id", "LPHD_Type")) {
    LOG_INFO("Injecting missing LPHD_Type");
    auto lphdType = templates.append_child("LNodeType");
    lphdType.append_attribute("id") = "LPHD_Type";
    lphdType.append_attribute("lnClass") = "LPHD";

    auto proxyDO = lphdType.append_child("DO");
    proxyDO.append_attribute("name") = "Proxy";
    proxyDO.append_attribute("type") = "SPS_Type";

    auto phyNamDO = lphdType.append_child("DO");
    phyNamDO.append_attribute("name") = "PhyNam";
    phyNamDO.append_attribute("type") =
        "DPL_1_PhyNam"; // Link to our injected type

    auto phyHealthDO = lphdType.append_child("DO");
    phyHealthDO.append_attribute("name") = "PhyHealth";
    phyHealthDO.append_attribute("type") = "SPS_Type";
  }

  LOG_INFO("Scanning LNodeTypes to fix PhyNam references...");
  // Scan all LNodeTypes
  for (auto lNodeType : templates.children("LNodeType")) {
    // Find DO named PhyNam
    auto phyNamDO = lNodeType.find_child_by_attribute("DO", "name", "PhyNam");
    if (phyNamDO) {
      std::string currentType = phyNamDO.attribute("type").as_string();
      LOG_INFO("Found PhyNam DO in LNodeType: {}, Current Type: '{}'",
               lNodeType.attribute("id").as_string(), currentType);

      // Check if it has a valid type
      if (currentType.empty() || !templates.find_child_by_attribute(
                                     "DOType", "id", currentType.c_str())) {
        LOG_INFO("Fixing PhyNam type in LNodeType: {} -> DPL_1_PhyNam",
                 lNodeType.attribute("id").as_string());
        phyNamDO.attribute("type").set_value("DPL_1_PhyNam");
      } else {
        LOG_INFO("PhyNam already has valid type: {}", currentType);
      }
    }
  }
}

void SCDGenerator::generateCommunication(pugi::xml_node comm) {
  auto subnet = comm.append_child("SubNetwork");
  subnet.append_attribute("name") = "StationBus";
  subnet.append_attribute("type") = "8-MMS";

  for (const auto &ied : ieds_) {
    auto connAP = subnet.append_child("ConnectedAP");
    connAP.append_attribute("iedName") = ied.name.c_str();
    connAP.append_attribute("apName") = "AP1";

    // Always create Address node (from networkConfigs if available)
    auto address = connAP.append_child("Address");

    if (networkConfigs_.count(ied.name)) {
      auto &netCfg = networkConfigs_[ied.name];
      auto pIP = address.append_child("P");
      pIP.append_attribute("type") = "IP";
      pIP.text() = netCfg.ipAddress.c_str();

      // Add other network parameters
      auto pSubnet = address.append_child("P");
      pSubnet.append_attribute("type") = "IP-SUBNET";
      pSubnet.text() = netCfg.subnetMask.c_str();

      auto pGateway = address.append_child("P");
      pGateway.append_attribute("type") = "IP-GATEWAY";
      pGateway.text() = netCfg.gateway.c_str();
    }
  }
}

void SCDGenerator::generateSubstation(pugi::xml_node substation) {
  substation.append_attribute("name") = "Substation1";
  auto voltageLevel = substation.append_child("VoltageLevel");
  voltageLevel.append_attribute("name") = "110kV";
}

void SCDGenerator::autoAssignIPs(const std::string &baseIP) {
  std::string prefix = baseIP.substr(0, baseIP.rfind('.') + 1);
  int startNum = std::stoi(baseIP.substr(baseIP.rfind('.') + 1));

  for (size_t i = 0; i < ieds_.size(); ++i) {
    if (networkConfigs_.find(ieds_[i].name) == networkConfigs_.end()) {
      NetworkConfig config;
      config.ipAddress = prefix + std::to_string(startNum + i);
      networkConfigs_[ieds_[i].name] = config;
    }
  }
}

void SCDGenerator::autoAssignGOOSE() {
  // Implementation similar to design doc
}

std::vector<GOOSEPublisher> SCDGenerator::discoverGOOSEPublishers() const {
  return std::vector<GOOSEPublisher>(); // Placeholder
}

bool SCDGenerator::validate(const std::string &scdContent) {
  return !scdContent.empty();
}

std::string SCDGenerator::getCurrentTimestamp() {
  return "2023-01-01T00:00:00";
}

void SCDGenerator::mergeDataTypeTemplates(
    pugi::xml_node templates, const pugi::xml_node &sourceTemplates) {
  // Implementation
}

} // namespace scl
} // namespace iec61850
} // namespace gateway
