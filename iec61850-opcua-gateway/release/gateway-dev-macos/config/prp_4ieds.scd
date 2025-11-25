<?xml version="1.0" encoding="UTF-8"?>
<SCL xmlns="http://www.iec.ch/61850/2003/SCL" version="2007" revision="B">
  <Header id="PRP_Test_Station" version="1.0" revision="A" toolID="Manual" nameStructure="IEDName">
    <Text>PRP Test Configuration - 4 IEDs with Dual Network</Text>
  </Header>
  
  <!-- IED Definitions -->
  <IED name="ProtectionIED1" type="ProtectionRelay" manufacturer="TestVendor" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbAnalog" type="GOOSE" appID="ProtIED1_GOOSE" datSet="DS1"/>
          </LN0>
          <LN lnClass="XCBR" inst="1" lnType="XCBR_Type"/>
          <LN lnClass="MMXU" inst="1" lnType="MMXU_Type"/>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="ProtectionIED2" type="ProtectionRelay" manufacturer="TestVendor" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbStatus" type="GOOSE" appID="ProtIED2_GOOSE" datSet="DS1"/>
          </LN0>
          <LN lnClass="XCBR" inst="1" lnType="XCBR_Type"/>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="MeterIED1" type="Meter" manufacturer="TestVendor" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbMeter" type="GOOSE" appID="MeterIED1_GOOSE" datSet="DS1"/>
          </LN0>
          <LN lnClass="MMXU" inst="1" lnType="MMXU_Type"/>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="ControlIED1" type="Controller" manufacturer="TestVendor" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbControl" type="GOOSE" appID="CtrlIED1_GOOSE" datSet="DS1"/>
          </LN0>
          <LN lnClass="CSWI" inst="1" lnType="CSWI_Type"/>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <!-- Communication Section - PRP Dual Network -->
  <Communication>
    <SubNetwork name="NetworkA" type="8-MMS">
      <Text>Primary PRP Network</Text>
      <ConnectedAP iedName="ProtectionIED1" apName="AP1">
        <Address>
          <P type="IP">192.168.10.11</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="IP-GATEWAY">192.168.10.1</P>
        </Address>
      </ConnectedAP>
      <ConnectedAP iedName="ProtectionIED2" apName="AP1">
        <Address>
          <P type="IP">192.168.10.12</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="IP-GATEWAY">192.168.10.1</P>
        </Address>
      </ConnectedAP>
      <ConnectedAP iedName="MeterIED1" apName="AP1">
        <Address>
          <P type="IP">192.168.10.13</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="IP-GATEWAY">192.168.10.1</P>
        </Address>
      </ConnectedAP>
      <ConnectedAP iedName="ControlIED1" apName="AP1">
        <Address>
          <P type="IP">192.168.10.14</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="IP-GATEWAY">192.168.10.1</P>
        </Address>
      </ConnectedAP>
    </SubNetwork>
    
    <SubNetwork name="NetworkB" type="8-MMS">
      <Text>Secondary PRP Network</Text>
      <ConnectedAP iedName="ProtectionIED1" apName="AP1">
        <Address>
          <P type="IP">192.168.20.11</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="IP-GATEWAY">192.168.20.1</P>
        </Address>
      </ConnectedAP>
      <ConnectedAP iedName="ProtectionIED2" apName="AP1">
        <Address>
          <P type="IP">192.168.20.12</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="IP-GATEWAY">192.168.20.1</P>
        </Address>
      </ConnectedAP>
      <ConnectedAP iedName="MeterIED1" apName="AP1">
        <Address>
          <P type="IP">192.168.20.13</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="IP-GATEWAY">192.168.20.1</P>
        </Address>
      </ConnectedAP>
      <ConnectedAP iedName="ControlIED1" apName="AP1">
        <Address>
          <P type="IP">192.168.20.14</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="IP-GATEWAY">192.168.20.1</P>
        </Address>
      </ConnectedAP>
    </SubNetwork>
  </Communication>
  
  <!-- Data Type Templates -->
  <DataTypeTemplates>
    <LNodeType id="LLN0_Type" lnClass="LLN0"/>
    <LNodeType id="XCBR_Type" lnClass="XCBR"/>
    <LNodeType id="MMXU_Type" lnClass="MMXU"/>
    <LNodeType id="CSWI_Type" lnClass="CSWI"/>
  </DataTypeTemplates>
</SCL>
