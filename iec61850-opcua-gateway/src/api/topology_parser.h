#pragma once

#include "pugixml.hpp"
#include <string>
#include <vector>

namespace gateway {
namespace topology {

enum class RedundancyType {
  NONE, // No redundancy
  PRP,  // Parallel Redundancy Protocol (IEC 62439-3 Clause 4)
  HSR   // High-availability Seamless Redundancy (IEC 62439-3 Clause 5)
};

struct IEDNode {
  std::string id;
  std::string name;
  std::string type;
  std::string ip;
  std::string status;
  int goosePublished;
  int gooseSubscribed;
};

struct GOOSEConnection {
  std::string from;
  std::string to;
  std::string type;
  std::string gocbRef;
  std::vector<std::string> networks;
};

struct NetworkStats {
  std::string interface;
  int messageRate;
  int errors;
  double quality;
  std::string status;
};

struct TopologyInfo {
  RedundancyType type;
  std::vector<IEDNode> nodes;
  std::vector<GOOSEConnection> connections;
  std::vector<std::string> networks;
  NetworkStats networkA;
  NetworkStats networkB; // Only for PRP
};

class TopologyParser {
public:
  TopologyParser();
  ~TopologyParser();

  /**
   * @brief Parse SCD file and extract topology information
   * @param scdPath Path to SCD file
   * @return TopologyInfo structure with network topology
   */
  TopologyInfo parseFromSCD(const std::string &scdPath);

  /**
   * @brief Detect redundancy type from SCD
   * @param doc XML document
   * @return RedundancyType enum value
   */
  RedundancyType detectRedundancyType(const pugi::xml_document &doc);

  /**
   * @brief Convert topology to JSON string
   * @param topology TopologyInfo structure
   * @return JSON string
   */
  std::string toJSON(const TopologyInfo &topology);

private:
  std::vector<IEDNode> extractIEDs(const pugi::xml_document &doc);
  std::vector<GOOSEConnection>
  extractConnections(const pugi::xml_document &doc);
  std::vector<std::string> extractNetworks(const pugi::xml_document &doc);

  std::string redundancyTypeToString(RedundancyType type);
};

} // namespace topology
} // namespace gateway
