#pragma once

#include "opcua_server.h"
#include <libiec61850/mms_value.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>

// Forward declaration
namespace gateway {
namespace iec61850 {
namespace mms {
class MMSConnection;
}
} // namespace iec61850
} // namespace gateway

namespace gateway {
namespace opcua {

class DataBinder {
public:
  DataBinder(std::shared_ptr<OPCUAServer> server);
  ~DataBinder();

  // Bind IEC61850 data point to OPC UA variable
  // iec61850Ref: "IEDName/LD/LN.DO.DA" (e.g.
  // "TestIED/simpleIO/GGIO1.SPCSO1.stVal")
  bool bindDataPoint(const std::string &iec61850Ref,
                     const UA_NodeId &opcuaNodeId);

  // Update OPC UA variable from IEC61850 value
  void updateValue(const std::string &iec61850Ref, MmsValue *value);

  /**
   * @brief Get all bound IEC61850 references
   * @return Vector of reference strings
   */
  std::vector<std::string> getBoundReferences();

  /**
   * @brief Set write callback for a controllable node
   */
  void setWriteCallback(const UA_NodeId &opcuaNodeId);

  /**
   * @brief Set MMS connections map for write operations
   */
  void setMMSConnections(
      std::map<std::string,
               std::shared_ptr<gateway::iec61850::mms::MMSConnection>>
          *connections);

private:
  std::shared_ptr<OPCUAServer> server_;

  // Map IEC61850 Ref -> OPC UA NodeId
  std::map<std::string, UA_NodeId> refToNodeMap_;

  // Reverse map: NodeId string -> IEC61850 Ref (for writes)
  std::map<std::string, std::string> nodeToRefMap_;

  // Pointer to MMS connections (owned by RESTApi)
  std::map<std::string, std::shared_ptr<gateway::iec61850::mms::MMSConnection>>
      *mmsConnections_;

  std::mutex mapMutex_;

  // Write callback handler
  void handleWrite(const std::string &nodeIdStr, const UA_Variant *value);

  // Static callback wrapper
  static void writeCallback(UA_Server *server, const UA_NodeId *sessionId,
                            void *sessionContext, const UA_NodeId *nodeId,
                            void *nodeContext, const UA_NumericRange *range,
                            const UA_DataValue *data);
};

} // namespace opcua
} // namespace gateway
