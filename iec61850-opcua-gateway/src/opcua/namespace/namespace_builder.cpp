#include "namespace_builder.h"
#include "core/logger.h"
#include "iec61850/scl/scl_parser.h"
#include <string>

namespace gateway {
namespace opcua {
namespace ns {

// Helper function to convert UA_String to std::string
namespace {
std::string UA_String_to_std_string(const UA_String &uaStr) {
  return std::string((char *)uaStr.data, uaStr.length);
}
} // namespace

NamespaceBuilder::NamespaceBuilder(std::shared_ptr<OPCUAServer> server,
                                   std::shared_ptr<DataBinder> binder)
    : server_(server), binder_(binder) {}

NamespaceBuilder::~NamespaceBuilder() {}

void NamespaceBuilder::buildAddressSpace(
    const std::vector<std::shared_ptr<data::models::IEDDevice>> &devices) {
  LOG_INFO("Building OPC UA Address Space for {} devices...", devices.size());

  for (const auto &device : devices) {
    createIEDObject(device);
  }
}

void NamespaceBuilder::buildFromSCD(const std::string &scdPath) {
  using namespace gateway::iec61850;

  LOG_INFO("Building OPC UA namespace from SCD: {}", scdPath);

  try {
    // Parse SCD file
    SCLParser parser;
    std::vector<IEDConfig> ieds = parser.parse(scdPath);

    LOG_INFO("Parsed {} IEDs from SCD", ieds.size());

    UA_Server *uaServer = server_->getNativeServer();
    UA_NodeId objectsFolder = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);

    // Create IED objects from parsed SCD
    for (const auto &ied : ieds) {
      // Create IED folder
      UA_NodeId iedNodeId = UA_NODEID_STRING(nsIdx_, (char *)ied.name.c_str());

      UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
      oAttr.displayName =
          UA_LOCALIZEDTEXT((char *)"en", (char *)ied.name.c_str());
      std::string desc = ied.manufacturer + " - " + ied.type;
      oAttr.description = UA_LOCALIZEDTEXT((char *)"en", (char *)desc.c_str());

      UA_StatusCode retval = UA_Server_addObjectNode(
          uaServer, iedNodeId, objectsFolder,
          UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
          UA_QUALIFIEDNAME(nsIdx_, (char *)ied.name.c_str()),
          UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), oAttr, NULL, NULL);

      if (retval != UA_STATUSCODE_GOOD &&
          retval != UA_STATUSCODE_BADNODEIDEXISTS) {
        LOG_ERROR("Failed to add IED object for {}: {}", ied.name, retval);
        continue;
      }

      // Add IP Address variable
      std::string ipVarName = "IPAddress";
      UA_VariableAttributes ipAttr = UA_VariableAttributes_default;
      UA_String ipValue = UA_STRING((char *)ied.ip.c_str());
      UA_Variant_setScalar(&ipAttr.value, &ipValue, &UA_TYPES[UA_TYPES_STRING]);
      ipAttr.displayName =
          UA_LOCALIZEDTEXT((char *)"en", (char *)ipVarName.c_str());
      ipAttr.accessLevel = UA_ACCESSLEVELMASK_READ;

      UA_Server_addVariableNode(
          uaServer,
          UA_NODEID_STRING(nsIdx_, (char *)(ied.name + ".IPAddress").c_str()),
          iedNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
          UA_QUALIFIEDNAME(nsIdx_, (char *)ipVarName.c_str()),
          UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), ipAttr, NULL,
          NULL);

      // Create Logical Devices
      for (const auto &ld : ied.logicalDevices) {
        createLogicalDevice(iedNodeId, ied.name, ld);
      }

      LOG_INFO("Created OPC UA node for IED: {} ({}) with {} LDs", ied.name,
               ied.ip, ied.logicalDevices.size());
    }

  } catch (const std::exception &e) {
    LOG_ERROR("Failed to build namespace from SCD: {}", e.what());
  }
}

void NamespaceBuilder::createIEDObject(
    const std::shared_ptr<data::models::IEDDevice> &device) {
  UA_Server *uaServer = server_->getNativeServer();

  // Define NodeId for the IED Object
  std::string iedName = device->getName();
  UA_NodeId iedNodeId = UA_NODEID_STRING(nsIdx_, (char *)iedName.c_str());

  // Define Object Attributes
  UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
  oAttr.displayName = UA_LOCALIZEDTEXT((char *)"en", (char *)iedName.c_str());
  oAttr.description =
      UA_LOCALIZEDTEXT((char *)"en", (char *)"IEC61850 IED Device");

  // Add Object to ObjectsFolder
  UA_StatusCode retval = UA_Server_addObjectNode(
      uaServer, iedNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
      UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
      UA_QUALIFIEDNAME(nsIdx_, (char *)iedName.c_str()),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), oAttr, NULL, NULL);

  if (retval != UA_STATUSCODE_GOOD) {
    LOG_ERROR("Failed to add IED Object node for {}: {}", iedName, retval);
    return;
  }

  // Add Status Variable
  createConnectionStatusVariable(iedNodeId, device);

  // TODO: Recursively add Logical Devices and Nodes
}

void NamespaceBuilder::createConnectionStatusVariable(
    UA_NodeId parentNodeId,
    const std::shared_ptr<data::models::IEDDevice> &device) {
  UA_Server *uaServer = server_->getNativeServer();
  std::string varName = "ConnectionStatus";
  std::string nodeIdStr = device->getName() + "." + varName;

  UA_VariableAttributes attr = UA_VariableAttributes_default;
  UA_Boolean status =
      (device->getStatus() == data::models::ConnectionStatus::CONNECTED);
  UA_Variant_setScalar(&attr.value, &status, &UA_TYPES[UA_TYPES_BOOLEAN]);
  attr.displayName = UA_LOCALIZEDTEXT((char *)"en", (char *)varName.c_str());
  attr.accessLevel = UA_ACCESSLEVELMASK_READ;

  UA_Server_addVariableNode(
      uaServer, UA_NODEID_STRING(nsIdx_, (char *)nodeIdStr.c_str()),
      parentNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(nsIdx_, (char *)varName.c_str()),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, NULL);
}

void NamespaceBuilder::createLogicalDevice(const UA_NodeId &iedNode,
                                           const std::string &iedName,
                                           const iec61850::LogicalDevice &ld) {
  UA_Server *uaServer = server_->getNativeServer();

  // Create LD folder
  std::string ldNodeIdStr =
      UA_String_to_std_string(iedNode.identifier.string) + "." + ld.name;
  UA_NodeId ldNodeId = UA_NODEID_STRING(nsIdx_, (char *)ldNodeIdStr.c_str());

  UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
  oAttr.displayName = UA_LOCALIZEDTEXT((char *)"en", (char *)ld.name.c_str());
  oAttr.description = UA_LOCALIZEDTEXT((char *)"en", (char *)"Logical Device");

  UA_StatusCode retval = UA_Server_addObjectNode(
      uaServer, ldNodeId, iedNode, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(nsIdx_, (char *)ld.name.c_str()),
      UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), oAttr, NULL, NULL);

  if (retval != UA_STATUSCODE_GOOD && retval != UA_STATUSCODE_BADNODEIDEXISTS) {
    LOG_ERROR("Failed to create LD: {}", ld.name);
    return;
  }

  // Create Logical Nodes under this LD
  for (const auto &ln : ld.logicalNodes) {
    createLogicalNode(ldNodeId, iedName, ld.name, ln);
  }

  LOG_INFO("Created LD: {} with {} LNs", ld.name, ld.logicalNodes.size());
}

void NamespaceBuilder::createLogicalNode(const UA_NodeId &ldNode,
                                         const std::string &iedName,
                                         const std::string &ldName,
                                         const iec61850::LogicalNode &ln) {
  UA_Server *uaServer = server_->getNativeServer();

  // Create LN folder
  std::string lnNodeIdStr =
      UA_String_to_std_string(ldNode.identifier.string) + "." + ln.name;
  UA_NodeId lnNodeId = UA_NODEID_STRING(nsIdx_, (char *)lnNodeIdStr.c_str());

  UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
  std::string displayName = ln.name + " [" + ln.lnClass + "]";
  oAttr.displayName =
      UA_LOCALIZEDTEXT((char *)"en", (char *)displayName.c_str());
  oAttr.description = UA_LOCALIZEDTEXT((char *)"en", (char *)"Logical Node");

  UA_StatusCode retval = UA_Server_addObjectNode(
      uaServer, lnNodeId, ldNode, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(nsIdx_, (char *)ln.name.c_str()),
      UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), oAttr, NULL, NULL);

  if (retval != UA_STATUSCODE_GOOD && retval != UA_STATUSCODE_BADNODEIDEXISTS) {
    LOG_ERROR("Failed to create LN: {}", ln.name);
    return;
  }

  // Create Data Objects under this LN
  for (const auto &dobj : ln.dataObjects) {
    createDataObject(lnNodeId, iedName, ldName, ln.name, dobj);
  }

  LOG_INFO("Created LN: {} with {} DOs", ln.name, ln.dataObjects.size());
}

void NamespaceBuilder::createDataObject(const UA_NodeId &lnNode,
                                        const std::string &iedName,
                                        const std::string &ldName,
                                        const std::string &lnName,
                                        const iec61850::DataObject &dobj) {
  UA_Server *uaServer = server_->getNativeServer();

  // Create DO variable
  std::string doNodeIdStr =
      UA_String_to_std_string(lnNode.identifier.string) + "." + dobj.name;
  UA_NodeId doNodeId = UA_NODEID_STRING(nsIdx_, (char *)doNodeIdStr.c_str());

  UA_VariableAttributes vAttr = UA_VariableAttributes_default;
  std::string displayName = dobj.name + " [" + dobj.type + "]";
  vAttr.displayName =
      UA_LOCALIZEDTEXT((char *)"en", (char *)displayName.c_str());
  vAttr.description = UA_LOCALIZEDTEXT((char *)"en", (char *)"Data Object");
  vAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

  // Determine OPC UA type based on IEC61850 CDC
  // Set initial values with BAD quality to indicate no connection yet
  LOG_INFO("Creating DO: {} Type: {}", dobj.name, dobj.type);
  if (dobj.type == "SPS" || dobj.type == "SPC" || dobj.type == "DPS" ||
      dobj.type == "DPC") {
    UA_Boolean val = false;
    UA_Variant_setScalar(&vAttr.value, &val, &UA_TYPES[UA_TYPES_BOOLEAN]);
    vAttr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
  } else if (dobj.type == "MV" || dobj.type == "CMV") {
    // Use NaN to indicate uninitialized analog value
    UA_Float val = std::numeric_limits<float>::quiet_NaN();
    UA_Variant_setScalar(&vAttr.value, &val, &UA_TYPES[UA_TYPES_FLOAT]);
    vAttr.dataType = UA_TYPES[UA_TYPES_FLOAT].typeId;
  } else if (dobj.type == "INS" || dobj.type == "ENS" || dobj.type == "ENC") {
    UA_Int32 val = 0;
    UA_Variant_setScalar(&vAttr.value, &val, &UA_TYPES[UA_TYPES_INT32]);
    vAttr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
  } else {
    // Default to String indicating no connection
    UA_String val = UA_STRING((char *)"<No IED Connection>");
    UA_Variant_setScalar(&vAttr.value, &val, &UA_TYPES[UA_TYPES_STRING]);
    vAttr.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
  }

  UA_StatusCode retval = UA_Server_addVariableNode(
      uaServer, doNodeId, lnNode, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(nsIdx_, (char *)dobj.name.c_str()),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), vAttr, NULL, NULL);

  if (retval != UA_STATUSCODE_GOOD && retval != UA_STATUSCODE_BADNODEIDEXISTS) {
    LOG_ERROR("Failed to create DO: {} (0x{:x})", dobj.name, retval);
  } else {
    // Bind to DataBinder
    // Construct IEC61850 Reference: IED/LD/LN.DO
    // Use explicit names passed down
    std::string ref = iedName + "/" + ldName + "/" + lnName + "." + dobj.name;

    if (binder_) {
      // Only bind the base DO reference
      binder_->bindDataPoint(ref, doNodeId);

      // Register write callback for controllable points
      if (dobj.type == "SPC" || dobj.type == "DPC" || dobj.type == "APC") {
        binder_->setWriteCallback(doNodeId);
        LOG_INFO("Registered write callback for {}", ref);
      }
    }
  }
}

} // namespace ns
} // namespace opcua
} // namespace gateway
