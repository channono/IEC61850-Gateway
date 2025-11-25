#include "iec61850/scl/scl_generator.h"
#include "pugixml.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

// libiec61850 includes
#include <libiec61850/iec61850_client.h>

SCLGenerator::SCLGenerator(::gateway::iec61850::mms::MMSConnection *conn)
    : connection_(conn) {}

std::string SCLGenerator::generateICD(const std::string &iedName) {
  pugi::xml_document doc;
  pugi::xml_node root = doc.append_child("SCL");
  root.append_attribute("xmlns").set_value("http://www.iec.ch/61850/2003/SCL");
  root.append_attribute("version").set_value("2007");
  root.append_attribute("revision").set_value("B");
  root.append_attribute("release").set_value("4");

  buildHeader(root, iedName, false);
  buildCommunication(root, iedName); // Generic communication
  buildIED(root, iedName);
  buildDataTypeTemplates(root);

  std::stringstream ss;
  doc.save(ss, "  ");
  return ss.str();
}

std::string SCLGenerator::generateCID(const std::string &iedName) {
  pugi::xml_document doc;
  pugi::xml_node root = doc.append_child("SCL");
  root.append_attribute("xmlns").set_value("http://www.iec.ch/61850/2003/SCL");
  root.append_attribute("version").set_value("2007");
  root.append_attribute("revision").set_value("B");
  root.append_attribute("release").set_value("4");

  buildHeader(root, iedName, true);
  buildCommunication(root, iedName); // Specific communication
  buildIED(root, iedName);
  buildDataTypeTemplates(root);

  std::stringstream ss;
  doc.save(ss, "  ");
  return ss.str();
}

void SCLGenerator::buildHeader(pugi::xml_node &root, const std::string &iedName,
                               bool isCid) {
  pugi::xml_node header = root.append_child("Header");
  header.append_attribute("id").set_value(iedName.c_str());
  header.append_attribute("version").set_value("1.0");
  header.append_attribute("revision").set_value("1");
  header.append_attribute("toolID").set_value("IEC61850-Gateway-Generator");

  // History
  pugi::xml_node history = header.append_child("History");
  pugi::xml_node hitem = history.append_child("Hitem");
  hitem.append_attribute("version").set_value("1.0");
  hitem.append_attribute("revision").set_value("1");
  hitem.append_attribute("when").set_value(
      "2025-01-01T00:00:00Z"); // Placeholder or current time
  hitem.append_attribute("who").set_value("Gateway");
  hitem.append_attribute("what").set_value(isCid ? "CID Generation"
                                                 : "ICD Generation");
}

void SCLGenerator::buildCommunication(pugi::xml_node &root,
                                      const std::string &iedName) {
  pugi::xml_node comm = root.append_child("Communication");
  pugi::xml_node subnetwork = comm.append_child("SubNetwork");
  subnetwork.append_attribute("name").set_value("SubNetwork1");
  subnetwork.append_attribute("type").set_value("8-MMS");

  pugi::xml_node connectedAp = subnetwork.append_child("ConnectedAP");
  connectedAp.append_attribute("iedName").set_value(iedName.c_str());
  connectedAp.append_attribute("apName").set_value("AP1");

  pugi::xml_node address = connectedAp.append_child("Address");

  auto addP = [&](const char *type, const char *value) {
    pugi::xml_node p = address.append_child("P");
    p.append_attribute("type").set_value(type);
    p.text().set(value);
  };

  // Use actual IP and port from the connection
  std::string actualIP = connection_->getIP();
  int actualPort = connection_->getPort();

  addP("IP", actualIP.c_str());
  addP("IP-SUBNET", "255.255.255.0");
  addP("IP-GATEWAY", "192.168.1.1"); // Default gateway
  addP("OSI-TSEL", "0001");
  addP("OSI-PSEL", "00000001");
  addP("OSI-SSEL", "0001");

  // Add MMS port as TCP-PORT
  pugi::xml_node portP = address.append_child("P");
  portP.append_attribute("type").set_value("TCP-PORT");
  portP.text().set(std::to_string(actualPort).c_str());
}

void SCLGenerator::buildIED(pugi::xml_node &root, const std::string &iedName) {
  pugi::xml_node ied = root.append_child("IED");
  ied.append_attribute("name").set_value(iedName.c_str());

  pugi::xml_node accessPoint = ied.append_child("AccessPoint");
  accessPoint.append_attribute("name").set_value("AP1");

  pugi::xml_node server = accessPoint.append_child("Server");

  pugi::xml_node authentication = server.append_child("Authentication");
  authentication.append_attribute("none").set_value("true");

  pugi::xml_node ldevice = server.append_child("LDevice");
  // We need to iterate over Logical Devices.
  // MMSConnection::getLogicalDeviceList() returns vector<string>

  std::vector<std::string> devices = connection_->getLogicalDeviceList();

  for (const auto &ldName : devices) {
    addLogicalDevice(server, ldName);
  }
}

void SCLGenerator::addLogicalDevice(pugi::xml_node &serverNode,
                                    const std::string &ldName) {
  pugi::xml_node ldevice = serverNode.append_child("LDevice");
  ldevice.append_attribute("inst").set_value(ldName.c_str());

  // Get Logical Nodes in this LD
  IedClientError error;
  LinkedList lnList = IedConnection_getLogicalDeviceDirectory(
      connection_->connection_, &error, ldName.c_str());

  if (error == IED_ERROR_OK && lnList != NULL) {
    LinkedList element = LinkedList_getNext(lnList);
    while (element != NULL) {
      char *lnName = (char *)LinkedList_getData(element);
      addLogicalNode(ldevice, ldName, std::string(lnName));
      element = LinkedList_getNext(element);
    }
    LinkedList_destroy(lnList);
  }
}

void SCLGenerator::addLogicalNode(pugi::xml_node &ldNode,
                                  const std::string &ldName,
                                  const std::string &lnName) {
  // Parse LN name (e.g., "LLN0", "GGIO1")
  // If it starts with LLN0, it's LN0. Otherwise LN.
  // Also need to split prefix, class, inst.
  // For simplicity, we assume standard naming: Class + Inst

  std::string lnClass;
  std::string lnInst;
  std::string prefix;

  bool isLN0 = (lnName == "LLN0");

  pugi::xml_node ln;
  if (isLN0) {
    ln = ldNode.append_child("LN0");
    ln.append_attribute("lnClass").set_value("LLN0");
    ln.append_attribute("inst").set_value("");
    ln.append_attribute("lnType").set_value("LLN0_Type"); // Placeholder
  } else {
    ln = ldNode.append_child("LN");

    // Simple parser: Extract class and inst
    // e.g. GGIO1 -> Class: GGIO, Inst: 1
    // e.g. MMXU1 -> Class: MMXU, Inst: 1
    // e.g. MyGGIO1 -> Prefix: My, Class: GGIO, Inst: 1 (Complex!)

    // For now, just use the whole name as lnClass if parsing fails
    // Or try to find the first digit
    size_t firstDigit = lnName.find_first_of("0123456789");
    if (firstDigit != std::string::npos) {
      lnClass = lnName.substr(0, firstDigit);
      lnInst = lnName.substr(firstDigit);
    } else {
      lnClass = lnName;
      lnInst = "";
    }

    ln.append_attribute("lnClass").set_value(lnClass.c_str());
    ln.append_attribute("inst").set_value(lnInst.c_str());
    ln.append_attribute("lnType").set_value(
        (lnClass + "_Type").c_str()); // Placeholder
  }

  // Get Data Objects
  std::string lnRef = ldName + "/" + lnName;
  IedClientError error;
  LinkedList doList = IedConnection_getLogicalNodeDirectory(
      connection_->connection_, &error, lnRef.c_str(), ACSI_CLASS_DATA_OBJECT);

  if (error == IED_ERROR_OK && doList != NULL) {
    LinkedList element = LinkedList_getNext(doList);
    while (element != NULL) {
      char *doName = (char *)LinkedList_getData(element);
      addDataObject(ln, ldName, lnName, std::string(doName));
      element = LinkedList_getNext(element);
    }
    LinkedList_destroy(doList);
  }
}

void SCLGenerator::addDataObject(pugi::xml_node &lnNode,
                                 const std::string &ldName,
                                 const std::string &lnName,
                                 const std::string &doName) {
  pugi::xml_node doi = lnNode.append_child("DOI");
  doi.append_attribute("name").set_value(doName.c_str());

  // Discover CDC type by browsing the Data Object
  std::string doRef = ldName + "/" + lnName + "." + doName;
  IedClientError error;
  LinkedList daList = IedConnection_getDataDirectory(connection_->connection_,
                                                     &error, doRef.c_str());

  std::string cdcType = "Unknown";

  if (error == IED_ERROR_OK && daList != NULL) {
    bool hasMag = false;
    bool hasStVal = false;
    bool hasCtlVal = false;
    bool hasVendor = false;
    bool hasSwRev = false;
    bool hasLdNs = false;
    bool hasGeneral = false;
    bool hasPhsA = false;
    bool hasPhsAB = false;

    LinkedList element = LinkedList_getNext(daList);
    while (element != NULL) {
      char *daName = (char *)LinkedList_getData(element);
      std::string name(daName);

      if (name == "mag")
        hasMag = true;
      if (name == "stVal")
        hasStVal = true;
      if (name == "Oper" || name == "SBOw")
        hasCtlVal = true;
      if (name == "vendor")
        hasVendor = true;
      if (name == "swRev")
        hasSwRev = true;
      if (name == "ldNs")
        hasLdNs = true;
      if (name == "general")
        hasGeneral = true;
      if (name == "phsA")
        hasPhsA = true;
      if (name == "phsAB")
        hasPhsAB = true;

      element = LinkedList_getNext(element);
    }
    LinkedList_destroy(daList);

    // CDC inference based on characteristic data attributes
    if (hasCtlVal)
      cdcType = "SPC"; // Controllable Single Point
    else if (hasVendor && hasSwRev && hasLdNs)
      cdcType = "LPL"; // Logical Node Name Plate
    else if (hasVendor && hasSwRev)
      cdcType = "DPL"; // Device Name Plate
    else if (hasMag)
      cdcType = "MV"; // Measured Value
    else if (hasPhsA)
      cdcType = "WYE"; // Wye (3-phase)
    else if (hasPhsAB)
      cdcType = "DEL"; // Delta (3-phase)
    else if (hasGeneral)
      cdcType = "ACT"; // Protection activation
    else if (hasStVal)
      cdcType = "SPS"; // Single Point Status (default for stVal)
  }

  // Add type attribute (reference to DOType in templates)
  // We'll use a simple convention: CDC + "_Type"
  if (cdcType != "Unknown") {
    doi.append_attribute("type").set_value((cdcType + "_Type").c_str());
  }
}

void SCLGenerator::buildDataTypeTemplates(pugi::xml_node &root) {
  pugi::xml_node templates = root.append_child("DataTypeTemplates");

  // Add basic DOTypes for common CDCs
  auto addDOType = [&](const char *id, const char *cdc) {
    pugi::xml_node doType = templates.append_child("DOType");
    doType.append_attribute("id").set_value(id);
    doType.append_attribute("cdc").set_value(cdc);
    return doType;
  };

  // SPS - Single Point Status
  pugi::xml_node sps = addDOType("SPS_Type", "SPS");
  pugi::xml_node spsVal = sps.append_child("DA");
  spsVal.append_attribute("name").set_value("stVal");
  spsVal.append_attribute("bType").set_value("BOOLEAN");
  spsVal.append_attribute("fc").set_value("ST");

  // MV - Measured Value
  pugi::xml_node mv = addDOType("MV_Type", "MV");
  pugi::xml_node mvMag = mv.append_child("DA");
  mvMag.append_attribute("name").set_value("mag");
  mvMag.append_attribute("bType").set_value("Struct");
  mvMag.append_attribute("fc").set_value("MX");
  pugi::xml_node mvQ = mv.append_child("DA");
  mvQ.append_attribute("name").set_value("q");
  mvQ.append_attribute("bType").set_value("Quality");
  mvQ.append_attribute("fc").set_value("MX");

  // SPC - Controllable Single Point
  pugi::xml_node spc = addDOType("SPC_Type", "SPC");
  pugi::xml_node spcVal = spc.append_child("DA");
  spcVal.append_attribute("name").set_value("stVal");
  spcVal.append_attribute("bType").set_value("BOOLEAN");
  spcVal.append_attribute("fc").set_value("ST");
  pugi::xml_node spcOper = spc.append_child("DA");
  spcOper.append_attribute("name").set_value("Oper");
  spcOper.append_attribute("bType").set_value("Struct");
  spcOper.append_attribute("fc").set_value("CO");

  // DPL - Device Name Plate (PhyNam)
  pugi::xml_node dpl = addDOType("DPL_1_PhyNam", "DPL");
  const char *dplAttrs[] = {"vendor", "swRev", "d", "configRev"};
  for (const char *attr : dplAttrs) {
    pugi::xml_node da = dpl.append_child("DA");
    da.append_attribute("name").set_value(attr);
    da.append_attribute("bType").set_value("VisString255");
    da.append_attribute("fc").set_value("DC");
  }

  // LPHD - Physical Device Information
  pugi::xml_node lphd = templates.append_child("LNodeType");
  lphd.append_attribute("id").set_value("LPHD_Type");
  lphd.append_attribute("lnClass").set_value("LPHD");

  auto addDO = [&](pugi::xml_node &ln, const char *name, const char *type) {
    pugi::xml_node dobj = ln.append_child("DO");
    dobj.append_attribute("name").set_value(name);
    dobj.append_attribute("type").set_value(type);
  };

  addDO(lphd, "PhyNam", "DPL_1_PhyNam");
  addDO(lphd, "PhyHealth", "SPS_Type");
  addDO(lphd, "Proxy", "SPS_Type");
}
