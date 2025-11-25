#pragma once

#include "data/models/ied_device.h"
#include "opcua/data_binder.h"
#include "opcua/opcua_server.h"
#include <memory>
#include <vector>

// Forward declarations for IEC61850 data model
namespace gateway {
namespace iec61850 {
struct LogicalDevice;
struct LogicalNode;
struct DataObject;
} // namespace iec61850
} // namespace gateway

namespace gateway {
namespace opcua {
namespace ns {

class NamespaceBuilder {
public:
  NamespaceBuilder(std::shared_ptr<OPCUAServer> server,
                   std::shared_ptr<DataBinder> binder);
  ~NamespaceBuilder();

  /**
   * @brief Build the address space for a list of IED devices
   * @param devices List of IED devices
   * @return true if successful
   */
  void buildAddressSpace(
      const std::vector<std::shared_ptr<data::models::IEDDevice>> &devices);

  /**
   * @brief Build the address space from an SCD file
   * @param scdPath Path to the SCD file
   * @return true if successful
   */
  void buildFromSCD(const std::string &scdPath);

private:
  std::shared_ptr<OPCUAServer> server_;
  std::shared_ptr<DataBinder> binder_;
  UA_UInt16 nsIdx_ = 2; // Default namespace index for gateway

  void createIEDObject(const std::shared_ptr<data::models::IEDDevice> &device);
  void createConnectionStatusVariable(
      UA_NodeId parentNodeId,
      const std::shared_ptr<data::models::IEDDevice> &device);

  // NEW: Methods for creating IEC61850 data model nodes
  void createLogicalDevice(const UA_NodeId &iedNode, const std::string &iedName,
                           const iec61850::LogicalDevice &ld);
  void createLogicalNode(const UA_NodeId &ldNode, const std::string &iedName,
                         const std::string &ldName,
                         const iec61850::LogicalNode &ln);
  void createDataObject(const UA_NodeId &lnNode, const std::string &iedName,
                        const std::string &ldName, const std::string &lnName,
                        const iec61850::DataObject &dobj);
};

} // namespace ns
} // namespace opcua
} // namespace gateway
