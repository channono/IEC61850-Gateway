<?xml version="1.0" encoding="UTF-8"?>
<SCL xmlns="http://www.iec.ch/61850/2003/SCL" version="2007" revision="B">
  <Header id="HSR_Ring_Station" version="1.0" revision="A" toolID="Manual" nameStructure="IEDName">
    <Text>HSR Ring Test Configuration - 6 IEDs in Ring Topology</Text>
  </Header>
  
  <!-- IED Definitions -->
  <IED name="RingIED1" type="ProtectionRelay" manufacturer="TestVendor" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbRing1" type="GOOSE" appID="Ring1_GOOSE" datSet="DS1"/>
          </LN0>
          <LN lnClass="XCBR" inst="1" lnType="XCBR_Type"/>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="RingIED2" type="ProtectionRelay" manufacturer="TestVendor" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbRing2" type="GOOSE" appID="Ring2_GOOSE" datSet="DS1"/>
          </LN0>
          <LN lnClass="XCBR" inst="1" lnType="XCBR_Type"/>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="RingIED3" type="Meter" manufacturer="TestVendor" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbRing3" type="GOOSE" appID="Ring3_GOOSE" datSet="DS1"/>
          </LN0>
          <LN lnClass="MMXU" inst="1" lnType="MMXU_Type"/>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="RingIED4" type="Controller" manufacturer="TestVendor" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbRing4" type="GOOSE" appID="Ring4_GOOSE" datSet="DS1"/>
          </LN0>
          <LN lnClass="CSWI" inst="1" lnType="CSWI_Type"/>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="RingIED5" type="ProtectionRelay" manufacturer="TestVendor" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbRing5" type="GOOSE" appID="Ring5_GOOSE" datSet="DS1"/>
          </LN0>
          <LN lnClass="PDIS" inst="1" lnType="PDIS_Type"/>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="RingIED6" type="Meter" manufacturer="TestVendor" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbRing6" type="GOOSE" appID="Ring6_GOOSE" datSet="DS1"/>
          </LN0>
          <LN lnClass="MMXU" inst="2" lnType="MMXU_Type"/>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <!-- Communication Section - HSR Ring -->
  <Communication>
    <SubNetwork name="HSR-Ring-Network" type="8-MMS">
      <Text>High-availability Seamless Redundancy Ring</Text>
      <ConnectedAP iedName="RingIED1" apName="AP1">
        <Address>
          <P type="IP">192.168.100.11</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="VLAN-ID">100</P>
        </Address>
      </ConnectedAP>
      <ConnectedAP iedName="RingIED2" apName="AP1">
        <Address>
          <P type="IP">192.168.100.12</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="VLAN-ID">100</P>
        </Address>
      </ConnectedAP>
      <ConnectedAP iedName="RingIED3" apName="AP1">
        <Address>
          <P type="IP">192.168.100.13</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="VLAN-ID">100</P>
        </Address>
      </ConnectedAP>
      <ConnectedAP iedName="RingIED4" apName="AP1">
        <Address>
          <P type="IP">192.168.100.14</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="VLAN-ID">100</P>
        </Address>
      </ConnectedAP>
      <ConnectedAP iedName="RingIED5" apName="AP1">
        <Address>
          <P type="IP">192.168.100.15</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="VLAN-ID">100</P>
        </Address>
      </ConnectedAP>
      <ConnectedAP iedName="RingIED6" apName="AP1">
        <Address>
          <P type="IP">192.168.100.16</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="VLAN-ID">100</P>
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
    <LNodeType id="PDIS_Type" lnClass="PDIS"/>
  </DataTypeTemplates>
</SCL>
