#include "iec61850/scl/scd_generator.h"
#include <fstream>
#include <gtest/gtest.h>

using namespace gateway::iec61850::scl;

class SCDGeneratorTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Create dummy ICDs
    createICD("ied1.icd", "IED1");
    createICD("ied2.icd", "IED2");
  }

  void TearDown() override {
    std::remove("ied1.icd");
    std::remove("ied2.icd");
  }

  void createICD(const std::string &filename, const std::string &iedName) {
    std::ofstream file(filename);
    file << R"(<?xml version="1.0"?>
<SCL xmlns="http://www.iec.ch/61850/2003/SCL">
    <IED name=")"
         << iedName << R"(" type="TestType" manufacturer="TestManu"/>
</SCL>)";
    file.close();
  }
};

TEST_F(SCDGeneratorTest, GenerateSCD) {
  SCDGenerator generator;
  generator.addICD("ied1.icd");
  generator.addICD("ied2.icd");

  std::unordered_map<std::string, NetworkConfig> netConfigs;
  netConfigs["IED1"] = {"192.168.1.100"};
  netConfigs["IED2"] = {"192.168.1.101"};

  generator.configureNetwork(netConfigs);

  std::string scd = generator.generate();

  EXPECT_TRUE(scd.find("IED1") != std::string::npos);
  EXPECT_TRUE(scd.find("IED2") != std::string::npos);
  EXPECT_TRUE(scd.find("192.168.1.100") != std::string::npos);
  EXPECT_TRUE(scd.find("192.168.1.101") != std::string::npos);
}

TEST_F(SCDGeneratorTest, AutoAssignIPs) {
  SCDGenerator generator;
  generator.addICD("ied1.icd");
  generator.addICD("ied2.icd");

  // Empty config, force auto assign
  generator.configureNetwork({}, true);

  std::string scd = generator.generate();

  EXPECT_TRUE(scd.find("192.168.1.100") != std::string::npos);
  EXPECT_TRUE(scd.find("192.168.1.101") != std::string::npos);
}
