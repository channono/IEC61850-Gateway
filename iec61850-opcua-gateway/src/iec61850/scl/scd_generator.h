#pragma once

#include <pugixml.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace gateway {
namespace iec61850 {
namespace scl {

struct NetworkConfig {
  std::string ipAddress;
  std::string subnetMask = "255.255.255.0";
  std::string gateway = "192.168.1.1";
  int mmsPort = 102;
};

struct GOOSEPublisher {
  std::string iedName;
  std::string gseControlName;
  std::string dataSetName;
  std::string macAddress;
  std::string appID;
  int vlanID = 0;
  int vlanPriority = 4;
  bool subscribe = true;
};

struct IEDInfo {
  std::string name;
  std::string type;
  std::string manufacturer;
  std::shared_ptr<pugi::xml_document> doc; // Keep document alive
  pugi::xml_node iedNode;                  // Node within the document
};

class SCDGenerator {
public:
  SCDGenerator();
  ~SCDGenerator();

  /**
   * @brief Add an ICD file to the generator
   * @param filepath Path to the ICD file
   */
  void addICD(const std::string &filepath);

  /**
   * @brief Configure network settings for IEDs
   * @param configs Map of IED name to NetworkConfig
   * @param autoAssign If true, automatically assign IPs for missing configs
   */
  void configureNetwork(
      const std::unordered_map<std::string, NetworkConfig> &configs,
      bool autoAssign = true);

  /**
   * @brief Configure GOOSE publishers
   * @param publishers List of GOOSE publishers configuration
   */
  void configureGOOSE(const std::vector<GOOSEPublisher> &publishers);

  /**
   * @brief Generate the SCD XML content
   * @param includeSubstation Whether to include Substation section
   * @return SCD XML string
   */
  std::string generate(bool includeSubstation = true);

  /**
   * @brief Validate the generated SCD content
   * @param scdContent SCD XML string
   * @return true if valid
   */
  bool validate(const std::string &scdContent);

  std::vector<IEDInfo> getIEDs() const { return ieds_; }
  std::vector<GOOSEPublisher> discoverGOOSEPublishers() const;

private:
  std::vector<IEDInfo> ieds_;
  std::unordered_map<std::string, NetworkConfig> networkConfigs_;
  std::vector<GOOSEPublisher> goosePublishers_;
  pugi::xml_document doc_; // Helper to merge DataTypeTemplates

  void mergeDataTypeTemplates(pugi::xml_node templates,
                              const pugi::xml_node &sourceTemplates);

  // Helper to ensure DPL_1_PhyNam type exists
  void ensurePhyNamType(pugi::xml_node templates);

  // Helper to fix PhyNam DOs to use DPL_1_PhyNam
  void fixPhyNamReferences(pugi::xml_node templates);

  void generateCommunication(pugi::xml_node comm);
  void generateSubstation(pugi::xml_node substation);
  void autoAssignIPs(const std::string &baseIP = "192.168.1.100");
  void autoAssignGOOSE();
  std::string getCurrentTimestamp();
};

} // namespace scl
} // namespace iec61850
} // namespace gateway
