#ifndef SCL_GENERATOR_H
#define SCL_GENERATOR_H

#include "iec61850/mms/mms_connection.h"
#include <string>
#include <vector>

// Forward declaration for pugixml
namespace pugi {
class xml_node;
class xml_document;
} // namespace pugi

class SCLGenerator {
public:
  SCLGenerator(::gateway::iec61850::mms::MMSConnection *conn);

  // Generate ICD (IED Capability Description)
  // Typically contains "TEMPLATE" in header and generic communication settings
  std::string generateICD(const std::string &iedName);

  // Generate CID (Configured IED Description)
  // Contains specific configuration and communication settings
  std::string generateCID(const std::string &iedName);

private:
  ::gateway::iec61850::mms::MMSConnection *connection_;

  void buildHeader(pugi::xml_node &root, const std::string &iedName,
                   bool isCid);
  void buildCommunication(pugi::xml_node &root, const std::string &iedName);
  void buildIED(pugi::xml_node &root, const std::string &iedName);
  void buildDataTypeTemplates(pugi::xml_node &root);

  // Helper to add Logical Device
  void addLogicalDevice(pugi::xml_node &serverNode, const std::string &ldName);

  // Helper to add Logical Node
  void addLogicalNode(pugi::xml_node &ldNode, const std::string &ldName,
                      const std::string &lnName);

  // Helper to add Data Object
  void addDataObject(pugi::xml_node &lnNode, const std::string &ldName,
                     const std::string &lnName, const std::string &doName);
};

#endif // SCL_GENERATOR_H
