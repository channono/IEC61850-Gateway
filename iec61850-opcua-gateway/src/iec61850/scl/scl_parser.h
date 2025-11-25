#pragma once

#include <memory>
#include <pugixml.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace gateway {
namespace iec61850 {

// Forward declarations
struct IEDModel;

enum class SCLFileType {
  ICD, // IED Capability Description
  CID, // Configured IED Description
  SCD, // Substation Configuration Description
  SSD, // System Specification Description
  IID, // Instantiated IED Description
  UNKNOWN
};

struct GOOSEControlBlock {
  std::string name;
  std::string appID;
  std::string macAddress;
  std::string dataSet;
  int vlanID = 0;
  int vlanPriority = 4;
  int confRev = 1;
};

// Data Object structure
struct DataObject {
  std::string name;
  std::string type; // e.g., "SPS", "MV", "INS"
};

// Logical Node structure
struct LogicalNode {
  std::string name;
  std::string lnClass; // e.g., "GGIO", "MMXU", "LLN0"
  std::string inst;
  std::vector<DataObject> dataObjects;
};

// Logical Device structure
struct LogicalDevice {
  std::string name;
  std::string inst;
  std::vector<LogicalNode> logicalNodes;
};

struct IEDConfig {
  std::string name;
  std::string type;
  std::string manufacturer;
  std::string ip;
  std::string subnet;
  std::string gateway;
  std::vector<GOOSEControlBlock> gooseControls;
  std::vector<LogicalDevice> logicalDevices; // NEW
};

class SCLParser {
public:
  SCLParser();
  ~SCLParser();

  /**
   * @brief Detect the type of SCL file
   * @param filepath Path to the SCL file
   * @return SCLFileType enum
   */
  static SCLFileType detectFileType(const std::string &filepath);

  /**
   * @brief Parse an SCL file and extract IED configurations
   * @param filepath Path to the SCL file
   * @return Vector of IEDConfig structures
   */
  std::vector<IEDConfig> parse(const std::string &filepath);

private:
  IEDConfig parseIED(const pugi::xml_node &iedNode,
                     const pugi::xml_node &commNode);
  std::vector<GOOSEControlBlock>
  parseGOOSEControls(const pugi::xml_node &iedNode,
                     const pugi::xml_node &commNode);

  // NEW: Parse data model
  std::vector<LogicalDevice> parseLogicalDevices(const pugi::xml_node &iedNode);
  std::vector<LogicalNode> parseLogicalNodes(const pugi::xml_node &ldNode);
  std::vector<DataObject> parseDataObjects(const pugi::xml_node &lnNode);

  // NEW: Parse DataTypeTemplates
  void parseDataTypeTemplates(const pugi::xml_node &rootNode);
  void parseLNodeTypes(const pugi::xml_node &rootNode); // New helper

  // Map DOType ID -> CDC (e.g. "SPS_Type" -> "SPS")
  std::unordered_map<std::string, std::string> doTypeMap_;

  // Map of LNodeType ID -> Map of DO Name -> DO Type ID
  // e.g. "LPHD_Type" -> { "PhyNam" -> "DPL_1_PhyNam" }
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
      lnTypeMap_;
};

} // namespace iec61850
} // namespace gateway
