#include "data_binder.h"
#include "../iec61850/mms/mms_connection.h"
#include "core/logger.h"
#include <libiec61850/iec61850_client.h>

namespace gateway {
namespace opcua {

DataBinder::DataBinder(std::shared_ptr<OPCUAServer> server)
    : server_(server), mmsConnections_(nullptr) {}

DataBinder::~DataBinder() {}

void DataBinder::setMMSConnections(
    std::map<std::string,
             std::shared_ptr<gateway::iec61850::mms::MMSConnection>>
        *connections) {
  mmsConnections_ = connections;
}

bool DataBinder::bindDataPoint(const std::string &iec61850Ref,
                               const UA_NodeId &opcuaNodeId) {
  std::lock_guard<std::mutex> lock(mapMutex_);

  // Deep copy NodeId
  UA_NodeId nodeIdCopy;
  UA_NodeId_copy(&opcuaNodeId, &nodeIdCopy);

  refToNodeMap_[iec61850Ref] = nodeIdCopy;

  // Also create reverse mapping for writes (NodeId string -> IEC61850 Ref)
  UA_String nodeIdStr = UA_STRING_NULL;
  UA_NodeId_print(&opcuaNodeId, &nodeIdStr);
  if (nodeIdStr.data) {
    std::string nodeIdString((char *)nodeIdStr.data, nodeIdStr.length);
    nodeToRefMap_[nodeIdString] = iec61850Ref;
    UA_String_clear(&nodeIdStr);
  }

  LOG_INFO("Bound {} to OPC UA Node", iec61850Ref);
  return true;
}

void DataBinder::updateValue(const std::string &iec61850Ref, MmsValue *value) {
  if (!value)
    return;

  UA_NodeId nodeId;
  {
    std::lock_guard<std::mutex> lock(mapMutex_);
    auto it = refToNodeMap_.find(iec61850Ref);
    if (it == refToNodeMap_.end()) {
      return;
    }
    UA_NodeId_copy(&it->second, &nodeId);
  }

  UA_Server *uaServer = server_->getNativeServer();
  UA_Variant variant;
  UA_Variant_init(&variant);

  // Convert MmsValue to UA_Variant
  MmsType mmsType = MmsValue_getType(value);

  if (mmsType == MMS_BOOLEAN) {
    UA_Boolean val = MmsValue_getBoolean(value);
    UA_Variant_setScalarCopy(&variant, &val, &UA_TYPES[UA_TYPES_BOOLEAN]);
  } else if (mmsType == MMS_FLOAT) {
    UA_Float val = MmsValue_toFloat(value);
    UA_Variant_setScalarCopy(&variant, &val, &UA_TYPES[UA_TYPES_FLOAT]);
  } else if (mmsType == MMS_INTEGER) {
    UA_Int32 val = MmsValue_toInt32(value);
    UA_Variant_setScalarCopy(&variant, &val, &UA_TYPES[UA_TYPES_INT32]);
  } else if (mmsType == MMS_VISIBLE_STRING || mmsType == MMS_STRING) {
    const char *strVal = MmsValue_toString(value);
    if (strVal) {
      UA_String val = UA_STRING((char *)strVal);
      UA_Variant_setScalarCopy(&variant, &val, &UA_TYPES[UA_TYPES_STRING]);
    }
  } else if (mmsType == MMS_UNSIGNED) {
    UA_UInt32 val = MmsValue_toUint32(value);
    UA_Variant_setScalarCopy(&variant, &val, &UA_TYPES[UA_TYPES_UINT32]);
  } else if (mmsType == MMS_BIT_STRING) {
    UA_UInt32 val = MmsValue_getBitStringAsInteger(value);
    UA_Variant_setScalarCopy(&variant, &val, &UA_TYPES[UA_TYPES_UINT32]);
  } else if (mmsType == MMS_UTC_TIME) {
    // Convert UTC Time to String for display
    char buffer[64];
    uint32_t timestamp = MmsValue_getUtcTimeInMs(value);
    time_t rawtime = timestamp / 1000;
    struct tm *timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    UA_String val = UA_STRING(buffer);
    UA_Variant_setScalarCopy(&variant, &val, &UA_TYPES[UA_TYPES_STRING]);
  } else if (mmsType == MMS_OCTET_STRING) {
    // Convert Octet String to Hex String
    int len = MmsValue_getOctetStringSize(value);
    uint8_t *buf = MmsValue_getOctetStringBuffer(value);
    std::string hexStr;
    char hexBuf[4];
    for (int i = 0; i < len; i++) {
      snprintf(hexBuf, sizeof(hexBuf), "%02X", buf[i]);
      hexStr += hexBuf;
    }
    UA_String val = UA_STRING((char *)hexStr.c_str());
    UA_Variant_setScalarCopy(&variant, &val, &UA_TYPES[UA_TYPES_STRING]);
  } else {
    // Fallback for complex types or unknown types
    LOG_WARN("Unsupported MMS Type: {}", (int)mmsType);
    UA_String val = UA_STRING((char *)"Unsupported Type");
    UA_Variant_setScalarCopy(&variant, &val, &UA_TYPES[UA_TYPES_STRING]);
  }

  UA_Server_writeValue(uaServer, nodeId, variant);
  UA_Variant_clear(&variant);
  UA_NodeId_clear(&nodeId);
}

std::vector<std::string> DataBinder::getBoundReferences() {
  std::lock_guard<std::mutex> lock(mapMutex_);
  std::vector<std::string> refs;
  refs.reserve(refToNodeMap_.size());
  for (const auto &pair : refToNodeMap_) {
    refs.push_back(pair.first);
  }
  return refs;
}

void DataBinder::setWriteCallback(const UA_NodeId &opcuaNodeId) {
  UA_Server *uaServer = server_->getNativeServer();

  UA_ValueCallback callback;
  callback.onRead = nullptr;
  callback.onWrite = writeCallback;

  // Pass this DataBinder instance as node context
  UA_Server_setNodeContext(uaServer, opcuaNodeId, this);
  UA_Server_setVariableNode_valueCallback(uaServer, opcuaNodeId, callback);
}

void DataBinder::writeCallback(UA_Server *server, const UA_NodeId *sessionId,
                               void *sessionContext, const UA_NodeId *nodeId,
                               void *nodeContext, const UA_NumericRange *range,
                               const UA_DataValue *data) {
  // nodeContext contains the DataBinder instance
  DataBinder *binder = static_cast<DataBinder *>(nodeContext);
  if (!binder || !data || !nodeId) {
    return;
  }

  // Convert NodeId to string
  UA_String nodeIdStr = UA_STRING_NULL;
  UA_NodeId_print(nodeId, &nodeIdStr);
  if (nodeIdStr.data) {
    std::string nodeIdString((char *)nodeIdStr.data, nodeIdStr.length);
    binder->handleWrite(nodeIdString, &data->value);
    UA_String_clear(&nodeIdStr);
  }
}

void DataBinder::handleWrite(const std::string &nodeIdStr,
                             const UA_Variant *value) {
  if (!value || !mmsConnections_) {
    LOG_WARN("Write failed: no value or no MMS connections");
    return;
  }

  // Find IEC61850 reference
  std::string iec61850Ref;
  {
    std::lock_guard<std::mutex> lock(mapMutex_);
    auto it = nodeToRefMap_.find(nodeIdStr);
    if (it == nodeToRefMap_.end()) {
      LOG_WARN("No IEC61850 reference for NodeId: {}", nodeIdStr);
      return;
    }
    iec61850Ref = it->second;
  }

  // Parse reference: "IEDName/LD/LN.DO"
  size_t firstSlash = iec61850Ref.find('/');
  if (firstSlash == std::string::npos) {
    LOG_ERROR("Invalid IEC61850 reference: {}", iec61850Ref);
    return;
  }

  std::string iedName = iec61850Ref.substr(0, firstSlash);
  std::string objRef = iec61850Ref.substr(firstSlash + 1);

  // Find MMS connection
  auto connIt = mmsConnections_->find(iedName);
  if (connIt == mmsConnections_->end() || !connIt->second->isConnected()) {
    LOG_WARN("No active MMS connection for IED: {}", iedName);
    return;
  }

  auto conn = connIt->second;
  IedClientError error;

  // Create MmsValue from UA_Variant
  MmsValue *mmsValue = nullptr;

  if (value->type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
    UA_Boolean *boolVal = (UA_Boolean *)value->data;
    mmsValue = MmsValue_newBoolean(*boolVal);
    LOG_INFO("✍ Writing Boolean {} = {} to {}", objRef, *boolVal, iedName);
  } else if (value->type == &UA_TYPES[UA_TYPES_FLOAT]) {
    UA_Float *floatVal = (UA_Float *)value->data;
    mmsValue = MmsValue_newFloat(*floatVal);
    LOG_INFO("✍ Writing Float {} = {} to {}", objRef, *floatVal, iedName);
  } else if (value->type == &UA_TYPES[UA_TYPES_INT32]) {
    UA_Int32 *intVal = (UA_Int32 *)value->data;
    mmsValue = MmsValue_newIntegerFromInt32(*intVal);
    LOG_INFO("✍ Writing Int32 {} = {} to {}", objRef, *intVal, iedName);
  } else {
    LOG_WARN("Unsupported write type for {}", objRef);
    return;
  }

  if (mmsValue) {
    if (value->type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
      // For Boolean control (SPC/DPC), use IEC 61850 control service
      UA_Boolean *boolVal = (UA_Boolean *)value->data;

      // Create control object client
      ControlObjectClient control = ControlObjectClient_create(
          objRef.c_str(), conn->getNativeConnection());

      if (control) {
        // Determine expected type for ctlVal
        MmsType ctlValType = ControlObjectClient_getCtlValType(control);
        MmsValue *ctlVal = nullptr;

        if (ctlValType == MMS_BOOLEAN) {
          ctlVal = MmsValue_newBoolean(*boolVal);
          LOG_INFO("Creating Boolean control value: {}", *boolVal);
        } else if (ctlValType == MMS_INTEGER) {
          // Map boolean to integer (0/1) if required
          ctlVal = MmsValue_newIntegerFromInt32(*boolVal ? 1 : 0);
          LOG_INFO("Creating Integer control value from boolean: {}", *boolVal);
        } else if (ctlValType == MMS_BIT_STRING) {
          // Map boolean to BitString (e.g. for DPC: 01=off, 10=on)
          // DBPOS_OFF = 1, DBPOS_ON = 2
          int dbPosVal = *boolVal ? 2 : 1;
          ctlVal = MmsValue_newBitString(2);
          MmsValue_setBitStringFromInteger(ctlVal, dbPosVal);
          LOG_INFO(
              "Creating BitString control value (DPC) from boolean: {} -> {}",
              *boolVal, dbPosVal);
        } else {
          LOG_WARN("Unsupported ctlVal type: {}. Defaulting to Boolean.",
                   (int)ctlValType);
          ctlVal = MmsValue_newBoolean(*boolVal);
        }

        // Set Originator (Critical for some IEDs)
        ControlObjectClient_setOrigin(control, "OPCUA_GW",
                                      CONTROL_ORCAT_REMOTE_CONTROL);

        // Disable checks by default (can be made configurable later)
        ControlObjectClient_setInterlockCheck(control, false);
        ControlObjectClient_setSynchroCheck(control, false);

        // Check Control Model
        ControlModel ctlModel = ControlObjectClient_getControlModel(control);
        LOG_INFO("Control Model for {}: {}", objRef, (int)ctlModel);

        bool success = false;

        if (ctlModel == CONTROL_MODEL_SBO_NORMAL ||
            ctlModel == CONTROL_MODEL_SBO_ENHANCED) {

          // SBO: Select first
          LOG_INFO("Performing SBO Select for {}", objRef);
          if (ControlObjectClient_select(control)) {
            LOG_INFO("Select successful, performing Operate for {}", objRef);
            success = ControlObjectClient_operate(control, ctlVal, 0);
          } else {
            LastApplError lastError =
                ControlObjectClient_getLastApplError(control);
            LOG_WARN("✗ Select failed for {}: ApplError code={}", objRef,
                     (int)lastError.error);
          }

        } else {
          // Direct Operate
          LOG_INFO("Performing Direct Operate for {}", objRef);
          success = ControlObjectClient_operate(control, ctlVal, 0);
        }

        if (success) {
          LOG_INFO("✓ Control operation successful: {} = {}", objRef, *boolVal);
        } else {
          LastApplError lastError =
              ControlObjectClient_getLastApplError(control);
          LOG_WARN("✗ Control operation failed for {}: ApplError code={}",
                   objRef, (int)lastError.error);
        }

        MmsValue_delete(ctlVal);
        ControlObjectClient_destroy(control);
      } else {
        LOG_WARN("✗ Failed to create control object for {}", objRef);
      }

    } else if (value->type == &UA_TYPES[UA_TYPES_FLOAT]) {
      // For analog setpoint (APC), write to .mag.f
      std::string writeRef = objRef + ".mag.f";
      IedConnection_writeObject(conn->getNativeConnection(), &error,
                                writeRef.c_str(), IEC61850_FC_MX, mmsValue);

      if (error == IED_ERROR_OK) {
        LOG_INFO("✓ Write successful: {}", writeRef);
      } else {
        LOG_WARN("✗ Write failed for {}: error {}", writeRef, (int)error);
      }
    } else if (value->type == &UA_TYPES[UA_TYPES_INT32]) {
      // For integer values, try writing with appropriate FC
      std::string writeRef = objRef + ".stVal";
      IedConnection_writeObject(conn->getNativeConnection(), &error,
                                writeRef.c_str(), IEC61850_FC_ST, mmsValue);

      if (error == IED_ERROR_OK) {
        LOG_INFO("✓ Write successful: {}", writeRef);
      } else {
        LOG_WARN("✗ Write failed for {}: error {}", writeRef, (int)error);
      }
    }

    MmsValue_delete(mmsValue);
  }
}

} // namespace opcua
} // namespace gateway
