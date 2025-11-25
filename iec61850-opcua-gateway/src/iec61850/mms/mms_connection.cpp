#include "mms_connection.h"
#include "core/logger.h"
#include <nlohmann/json.hpp>

namespace gateway {
namespace iec61850 {
namespace mms {

MMSConnection::MMSConnection(const std::string &ip, int port)
    : ip_(ip), port_(port) {
  connection_ = IedConnection_create();
}

MMSConnection::~MMSConnection() {
  disconnect();
  if (connection_) {
    IedConnection_destroy(connection_);
    connection_ = nullptr;
  }
}

bool MMSConnection::connect() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (connected_)
    return true;

  IedClientError error;
  IedConnection_connect(connection_, &error, ip_.c_str(), port_);

  if (error == IED_ERROR_OK) {
    connected_ = true;
    LOG_INFO("Connected to IED at {}:{}", ip_, port_);
    return true;
  } else {
    LOG_ERROR("Failed to connect to IED at {}:{}. Error: {}", ip_, port_,
              (int)error);
    return false;
  }
}

void MMSConnection::disconnect() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (connected_) {
    IedConnection_close(connection_);
    connected_ = false;
    LOG_INFO("Disconnected from IED at {}:{}", ip_, port_);
  }
}

bool MMSConnection::isConnected() const { return connected_; }

MMSConnection::ReadResult MMSConnection::readData(const std::string &ref,
                                                  const std::string &fcStr) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!connected_) {
    throw std::runtime_error("Not connected to IED");
  }

  IedClientError error;
  FunctionalConstraint fc = IEC61850_FC_MX;

  if (fcStr == "ST")
    fc = IEC61850_FC_ST;
  else if (fcStr == "CO")
    fc = IEC61850_FC_CO;
  else if (fcStr == "CF")
    fc = IEC61850_FC_CF;
  else if (fcStr == "DC")
    fc = IEC61850_FC_DC;
  else if (fcStr == "SP")
    fc = IEC61850_FC_SP;
  else if (fcStr == "SV")
    fc = IEC61850_FC_SV;
  else if (fcStr == "SG")
    fc = IEC61850_FC_SG;
  else if (fcStr == "SE")
    fc = IEC61850_FC_SE;
  else if (fcStr == "SR")
    fc = IEC61850_FC_SR;
  else if (fcStr == "OR")
    fc = IEC61850_FC_OR;
  else if (fcStr == "BL")
    fc = IEC61850_FC_BL;
  else if (fcStr == "EX")
    fc = IEC61850_FC_EX;

  MmsValue *value =
      IedConnection_readObject(connection_, &error, ref.c_str(), fc);

  // If object doesn't exist (error 10), try appending .stVal with ST FC for
  // status types
  if (error == IED_ERROR_OBJECT_DOES_NOT_EXIST) {
    std::string stValRef = ref + ".stVal";
    LOG_INFO("Object not found, retrying with .stVal and ST FC: {}", stValRef);
    value = IedConnection_readObject(connection_, &error, stValRef.c_str(),
                                     IEC61850_FC_ST);
  }

  if (error != IED_ERROR_OK) {
    throw std::runtime_error("Failed to read object: " + std::to_string(error));
  }

  if (value == nullptr) {
    throw std::runtime_error("Read returned null value");
  }

  ReadResult result;
  result.quality = "GOOD"; // Default
  char buffer[1024];

  switch (MmsValue_getType(value)) {
  case MMS_FLOAT:
    result.type = "FLOAT32";
    result.value = std::to_string(MmsValue_toFloat(value));
    break;
  case MMS_BOOLEAN:
    result.type = "BOOLEAN";
    result.value = MmsValue_getBoolean(value) ? "true" : "false";
    break;
  case MMS_INTEGER:
    result.type = "INTEGER";
    result.value = std::to_string(MmsValue_toInt32(value));
    break;
  case MMS_UNSIGNED:
    result.type = "UNSIGNED";
    result.value = std::to_string(MmsValue_toUint32(value));
    break;
  case MMS_VISIBLE_STRING:
    result.type = "STRING";
    result.value = MmsValue_toString(value);
    break;
  case MMS_UTC_TIME:
    result.type = "UTC_TIME";
    {
      uint64_t timestamp = MmsValue_getUtcTimeInMs(value);
      time_t rawtime = timestamp / 1000;
      struct tm *timeinfo = gmtime(&rawtime);
      char timeBuf[32];
      strftime(timeBuf, sizeof(timeBuf), "%Y%m%d%H%M%S", timeinfo);
      snprintf(buffer, sizeof(buffer), "%s.%03lluZ", timeBuf, timestamp % 1000);
      result.value = buffer;
    }
    break;
  case MMS_ARRAY:
  case MMS_STRUCTURE:
    result.type =
        (MmsValue_getType(value) == MMS_ARRAY) ? "ARRAY" : "STRUCTURE";
    {
      // Format as JSON object/array for easier parsing
      nlohmann::json jsonValue;
      int size = (MmsValue_getType(value) == MMS_ARRAY ||
                  MmsValue_getType(value) == MMS_STRUCTURE)
                     ? MmsValue_getArraySize(value)
                     : 0;

      LOG_INFO("Serializing STRUCTURE/ARRAY. Size: {}, Type: {}", size,
               (int)MmsValue_getType(value));

      for (int i = 0; i < size; i++) {
        MmsValue *element = MmsValue_getElement(value, i);
        if (element) {
          MmsType elemType = MmsValue_getType(element);
          LOG_INFO("  Element {}: Type {}", i, (int)elemType);
          switch (elemType) {
          case MMS_FLOAT:
            jsonValue[i] = MmsValue_toFloat(element);
            break;
          case MMS_BOOLEAN:
            jsonValue[i] = MmsValue_getBoolean(element);
            break;
          case MMS_INTEGER:
            jsonValue[i] = MmsValue_toInt32(element);
            break;
          case MMS_UNSIGNED:
            jsonValue[i] = MmsValue_toUint32(element);
            break;
          case MMS_VISIBLE_STRING:
            jsonValue[i] = MmsValue_toString(element);
            break;
          case MMS_UTC_TIME: {
            uint64_t ts = MmsValue_getUtcTimeInMs(element);
            time_t rt = ts / 1000;
            struct tm *ti = gmtime(&rt);
            char tb[32];
            strftime(tb, sizeof(tb), "%Y%m%d%H%M%S", ti);
            char fullTime[48];
            snprintf(fullTime, sizeof(fullTime), "%s.%03lluZ", tb, ts % 1000);
            jsonValue[i] = fullTime;
          } break;
          case MMS_BIT_STRING:
            jsonValue[i] = MmsValue_getBitStringAsInteger(element);
            break;
          case MMS_STRUCTURE:
          case MMS_ARRAY:
            // Nested structure - recursively convert to JSON
            {
              nlohmann::json nestedJson;
              int nestedSize = (MmsValue_getType(element) == MMS_ARRAY ||
                                MmsValue_getType(element) == MMS_STRUCTURE)
                                   ? MmsValue_getArraySize(element)
                                   : 0;

              for (int j = 0; j < nestedSize; j++) {
                MmsValue *nestedElem = MmsValue_getElement(element, j);
                if (nestedElem) {
                  MmsType nestedType = MmsValue_getType(nestedElem);
                  switch (nestedType) {
                  case MMS_FLOAT:
                    nestedJson[j] = MmsValue_toFloat(nestedElem);
                    break;
                  case MMS_INTEGER:
                    nestedJson[j] = MmsValue_toInt32(nestedElem);
                    break;
                  case MMS_UNSIGNED:
                    nestedJson[j] = MmsValue_toUint32(nestedElem);
                    break;
                  case MMS_BOOLEAN:
                    nestedJson[j] = MmsValue_getBoolean(nestedElem);
                    break;
                  default:
                    // Deep nesting - use string representation
                    {
                      char deepBuf[128];
                      MmsValue_printToBuffer(nestedElem, deepBuf,
                                             sizeof(deepBuf));
                      nestedJson[j] = deepBuf;
                    }
                  }
                }
              }
              jsonValue[i] = nestedJson;
            }
            break;
          default: {
            char elemBuf[128];
            MmsValue_printToBuffer(element, elemBuf, sizeof(elemBuf));
            jsonValue[i] = elemBuf;
          }
          }
        }
      }
      result.value = jsonValue.dump();
    }
    break;
  case MMS_DATA_ACCESS_ERROR:
    result.type = "ERROR";
    result.value =
        "Access Error: " + std::to_string(MmsValue_getDataAccessError(value));
    break;
  default:
    result.type = "COMPLEX";
    MmsValue_printToBuffer(value, buffer, sizeof(buffer));
    result.value = buffer;
  }

  // TODO: Extract Quality and Timestamp if available (usually separate
  // attributes) For now, we assume GOOD quality and current time as we are
  // reading the value directly In a real scenario, we might read the structure
  // to get q and t
  result.quality = "GOOD";

  // Clean up
  MmsValue_delete(value);

  return result;
}

void MMSConnection::writeData(const std::string &objectReference,
                              const std::string &type,
                              const std::string &value) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!connected_) {
    throw std::runtime_error("Not connected to IED");
  }

  IedClientError error;

  // Check if this is a controllable object (SPCSO, DPCSO, etc.)
  // For SPCSO, we need to use operate instead of writeObject
  if (objectReference.find("SPCSO") != std::string::npos) {
    // Control operation for SPCSO
    // Support various boolean representations from different IED vendors:
    // - Standard: "true"/"false"
    // - Numeric: "1"/"0"
    // - Upper case: "TRUE"/"FALSE", "ON"/"OFF"
    // - Mixed case: "True"/"False"
    std::string valueLower = value;
    std::transform(valueLower.begin(), valueLower.end(), valueLower.begin(),
                   ::tolower);
    bool ctlVal =
        (valueLower == "true" || valueLower == "1" || valueLower == "on");

    // Create control object
    ControlObjectClient control =
        ControlObjectClient_create(objectReference.c_str(), connection_);

    if (control) {
      // Create MmsValue for control value
      MmsValue *ctlValue = MmsValue_newBoolean(ctlVal);

      // Use direct operate (ctlModel = 1, direct-with-normal-security)
      // No select needed for direct control
      bool operateResult =
          ControlObjectClient_operate(control, ctlValue, 0 /* timestamp */);

      MmsValue_delete(ctlValue);
      ControlObjectClient_destroy(control);

      if (!operateResult) {
        throw std::runtime_error("Control operation failed");
      }
    } else {
      throw std::runtime_error("Failed to create control object");
    }
  } else {
    // Regular write for non-controllable objects
    MmsValue *mmsValue = nullptr;

    if (type == "boolean") {
      mmsValue = MmsValue_newBoolean(value == "true" || value == "1");
    } else if (type == "float") {
      mmsValue = MmsValue_newFloat(std::stof(value));
    } else if (type == "int") {
      mmsValue = MmsValue_newIntegerFromInt32(std::stoi(value));
    } else {
      throw std::runtime_error("Unsupported type for write: " + type);
    }

    IedConnection_writeObject(connection_, &error, objectReference.c_str(),
                              IEC61850_FC_CO, mmsValue);

    MmsValue_delete(mmsValue);

    if (error != IED_ERROR_OK) {
      throw std::runtime_error("Failed to write object: " +
                               std::to_string(error));
    }
  }
}

std::vector<std::string> MMSConnection::getLogicalDeviceList() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!connected_) {
    throw std::runtime_error("Not connected to IED");
  }

  IedClientError error;
  LinkedList deviceList =
      IedConnection_getLogicalDeviceList(connection_, &error);

  if (error != IED_ERROR_OK) {
    throw std::runtime_error("Failed to get logical device list: " +
                             std::to_string(error));
  }

  std::vector<std::string> devices;
  LinkedList element = LinkedList_getNext(deviceList);
  while (element != NULL) {
    char *deviceName = (char *)LinkedList_getData(element);
    devices.push_back(std::string(deviceName));
    element = LinkedList_getNext(element);
  }

  LinkedList_destroy(deviceList);
  return devices;
}

std::vector<std::string>
MMSConnection::getLogicalDeviceVariables(const std::string &ldName) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!connected_) {
    throw std::runtime_error("Not connected to IED");
  }

  IedClientError error;
  // Get Logical Nodes first to build full structure, but for now let's try
  // getting variables directly if possible or iterate LNs. Actually,
  // IedConnection_getLogicalDeviceDirectory returns LNs. Then for each LN, we
  // get DataObjects.

  // For simplicity in this step, let's just return the LNs to verify the LD
  // name first. We will implement full recursive browse later or just use this
  // to debug the LD name.

  LinkedList lnList = IedConnection_getLogicalDeviceDirectory(
      connection_, &error, ldName.c_str());

  if (error != IED_ERROR_OK) {
    throw std::runtime_error("Failed to get logical device directory for " +
                             ldName + ": " + std::to_string(error));
  }

  std::vector<std::string> variables;
  LinkedList element = LinkedList_getNext(lnList);
  while (element != NULL) {
    char *lnName = (char *)LinkedList_getData(element);
    variables.push_back(std::string(lnName));
    element = LinkedList_getNext(element);
  }

  LinkedList_destroy(lnList);
  return variables;
}

std::vector<std::string>
MMSConnection::getLogicalNodeVariables(const std::string &ldName,
                                       const std::string &lnName) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!connected_) {
    throw std::runtime_error("Not connected to IED");
  }

  IedClientError error;
  std::string lnRef = ldName + "/" + lnName;

  LinkedList doList = IedConnection_getLogicalNodeDirectory(
      connection_, &error, lnRef.c_str(), ACSI_CLASS_DATA_OBJECT);

  if (error != IED_ERROR_OK) {
    throw std::runtime_error("Failed to get logical node directory for " +
                             lnRef + ": " + std::to_string(error));
  }

  std::vector<std::string> variables;
  LinkedList element = LinkedList_getNext(doList);
  while (element != NULL) {
    char *doName = (char *)LinkedList_getData(element);
    variables.push_back(std::string(doName));
    element = LinkedList_getNext(element);
  }

  LinkedList_destroy(doList);
  return variables;
}

std::vector<std::string> MMSConnection::getReportControlBlocks() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!connected_) {
    throw std::runtime_error("Not connected to IED");
  }

  std::vector<std::string> rcbs;
  IedClientError error;

  // Get Logical Devices
  LinkedList deviceList =
      IedConnection_getLogicalDeviceList(connection_, &error);

  if (error != IED_ERROR_OK) {
    throw std::runtime_error("Failed to get logical device list: " +
                             std::to_string(error));
  }

  LinkedList deviceElement = LinkedList_getNext(deviceList);
  while (deviceElement != NULL) {
    char *deviceName = (char *)LinkedList_getData(deviceElement);
    std::string ldName = deviceName;

    // Get Logical Nodes in this LD
    LinkedList lnList = IedConnection_getLogicalDeviceDirectory(
        connection_, &error, deviceName);

    if (error == IED_ERROR_OK) {
      LinkedList lnElement = LinkedList_getNext(lnList);
      while (lnElement != NULL) {
        char *lnName = (char *)LinkedList_getData(lnElement);
        std::string lnRef = ldName + "/" + lnName;

        // Get URCBs (Unbuffered Report Control Blocks)
        LinkedList urcbList = IedConnection_getLogicalNodeDirectory(
            connection_, &error, lnRef.c_str(), ACSI_CLASS_URCB);

        if (error == IED_ERROR_OK && urcbList != NULL) {
          LinkedList rcbElement = LinkedList_getNext(urcbList);
          while (rcbElement != NULL) {
            char *rcbName = (char *)LinkedList_getData(rcbElement);
            // Append .RP. for Unbuffered Reports
            rcbs.push_back(lnRef + ".RP." + rcbName);
            rcbElement = LinkedList_getNext(rcbElement);
          }
          LinkedList_destroy(urcbList);
        }

        // Get BRCBs (Buffered Report Control Blocks)
        LinkedList brcbList = IedConnection_getLogicalNodeDirectory(
            connection_, &error, lnRef.c_str(), ACSI_CLASS_BRCB);

        if (error == IED_ERROR_OK && brcbList != NULL) {
          LinkedList rcbElement = LinkedList_getNext(brcbList);
          while (rcbElement != NULL) {
            char *rcbName = (char *)LinkedList_getData(rcbElement);
            // Append .BR. for Buffered Reports
            rcbs.push_back(lnRef + ".BR." + rcbName);
            rcbElement = LinkedList_getNext(rcbElement);
          }
          LinkedList_destroy(brcbList);
        }

        lnElement = LinkedList_getNext(lnElement);
      }
      LinkedList_destroy(lnList);
    }
    deviceElement = LinkedList_getNext(deviceElement);
  }
  LinkedList_destroy(deviceList);

  return rcbs;
}

// Static callback wrapper
static void reportCallbackWrapper(void *parameter, ClientReport report) {
  auto *connection = static_cast<MMSConnection *>(parameter);
  if (connection) {
    connection->handleReport(report);
  }
}

void MMSConnection::handleReport(ClientReport report) {
  std::string rcbRef = ClientReport_getRcbReference(report);
  std::string rptId = ClientReport_getRptId(report);
  std::string dataSetRef = ClientReport_getDataSetName(report);

  MmsValue *dataSetValues = ClientReport_getDataSetValues(report);
  std::vector<std::pair<std::string, std::string>> values;

  if (dataSetValues) {
    int size = MmsValue_getArraySize(dataSetValues);
    for (int i = 0; i < size; i++) {
      MmsValue *element = MmsValue_getElement(dataSetValues, i);
      char buffer[1024];
      MmsValue_printToBuffer(element, buffer, sizeof(buffer));
      values.push_back({std::to_string(i), std::string(buffer)});
    }
  }

  // Notify listeners
  std::lock_guard<std::mutex> lock(mutex_);
  if (reportCallbacks_.count(rcbRef)) {
    reportCallbacks_[rcbRef](rcbRef, rptId, dataSetRef, values);
  }
}

void MMSConnection::subscribeReport(const std::string &rcbRef,
                                    ReportCallback callback) {
  // Check connection status with minimal lock
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_) {
      throw std::runtime_error("Not connected to IED");
    }
  }

  // Call libiec61850 WITHOUT holding mutex (to avoid blocking other operations)
  IedClientError error;
  ClientReportControlBlock rcb =
      IedConnection_getRCBValues(connection_, &error, rcbRef.c_str(), NULL);

  if (error != IED_ERROR_OK || !rcb) {
    throw std::runtime_error("Failed to get RCB values: " +
                             std::to_string(error));
  }

  // Install handler (no lock needed, connection_ is thread-safe for this)
  IedConnection_installReportHandler(connection_, rcbRef.c_str(),
                                     ClientReportControlBlock_getRptId(rcb),
                                     reportCallbackWrapper, this);

  // Enable reporting
  ClientReportControlBlock_setTrgOps(rcb, TRG_OPT_DATA_UPDATE |
                                              TRG_OPT_INTEGRITY | TRG_OPT_GI);
  ClientReportControlBlock_setRptEna(rcb, true);
  ClientReportControlBlock_setIntgPd(rcb, 5000);

  // Set RCB values (potentially blocking, no lock)
  IedConnection_setRCBValues(
      connection_, &error, rcb,
      RCB_ELEMENT_RPT_ENA | RCB_ELEMENT_TRG_OPS | RCB_ELEMENT_INTG_PD, true);

  if (error != IED_ERROR_OK) {
    ClientReportControlBlock_destroy(rcb);
    throw std::runtime_error("Failed to enable reporting: " +
                             std::to_string(error));
  }

  // Trigger GI
  ClientReportControlBlock_setGI(rcb, true);
  IedConnection_setRCBValues(connection_, &error, rcb, RCB_ELEMENT_GI, true);

  ClientReportControlBlock_destroy(rcb);

  // Store callback with lock (only lock when modifying shared state)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    reportCallbacks_[rcbRef] = callback;
  }
}

void MMSConnection::unsubscribeReport(const std::string &rcbRef) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!connected_) {
    return;
  }

  IedClientError error;
  ClientReportControlBlock rcb =
      IedConnection_getRCBValues(connection_, &error, rcbRef.c_str(), NULL);

  if (error == IED_ERROR_OK && rcb) {
    ClientReportControlBlock_setRptEna(rcb, false);
    IedConnection_setRCBValues(connection_, &error, rcb, RCB_ELEMENT_RPT_ENA,
                               true);
    ClientReportControlBlock_destroy(rcb);
  }

  reportCallbacks_.erase(rcbRef);
}

} // namespace mms
} // namespace iec61850
} // namespace gateway
