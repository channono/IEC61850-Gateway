#include "iec61850/scl/scl_parser.h"
#include <fstream>
#include <gtest/gtest.h>

using namespace gateway::iec61850;

class SCLParserTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Create a dummy ICD file
    std::ofstream file("test.icd");
    file << R"(<?xml version="1.0" encoding="UTF-8"?>
<SCL xmlns="http://www.iec.ch/61850/2003/SCL" version="2007" revision="B">
    <Header id="TestICD"/>
    <Communication>
        <SubNetwork name="StationBus">
            <ConnectedAP iedName="IED1" apName="AP1">
                <Address>
                    <P type="IP">192.168.1.100</P>
                </Address>
            </ConnectedAP>
        </SubNetwork>
    </Communication>
    <IED name="IED1" type="TestType" manufacturer="TestManu">
        <AccessPoint name="AP1">
            <Server>
                <LDevice inst="LD0">
                    <LN0 lnClass="LLN0" inst="" lnType="LLN0">
                        <GSEControl name="gcb01" datSet="ds1" appID="0001"/>
                    </LN0>
                </LDevice>
            </Server>
        </AccessPoint>
    </IED>
</SCL>)";
    file.close();
  }

  void TearDown() override { std::remove("test.icd"); }
};

TEST_F(SCLParserTest, DetectFileType) {
  EXPECT_EQ(SCLParser::detectFileType("test.icd"), SCLFileType::ICD);
}

TEST_F(SCLParserTest, ParseIEDConfig) {
  SCLParser parser;
  auto configs = parser.parse("test.icd");

  ASSERT_EQ(configs.size(), 1);
  EXPECT_EQ(configs[0].name, "IED1");
  EXPECT_EQ(configs[0].type, "TestType");
  EXPECT_EQ(configs[0].manufacturer, "TestManu");
  EXPECT_EQ(configs[0].ip, "192.168.1.100");
}

TEST_F(SCLParserTest, ParseGOOSEControls) {
  SCLParser parser;
  auto configs = parser.parse("test.icd");

  ASSERT_EQ(configs.size(), 1);
  ASSERT_EQ(configs[0].gooseControls.size(), 1);
  EXPECT_EQ(configs[0].gooseControls[0].name, "gcb01");
  EXPECT_EQ(configs[0].gooseControls[0].appID, "0001");
}
