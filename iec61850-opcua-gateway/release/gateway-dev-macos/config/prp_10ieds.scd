<?xml version="1.0" encoding="UTF-8"?>
<SCL xmlns="http://www.iec.ch/61850/2003/SCL" version="2007" revision="B">
  <Header id="Large_PRP_Station" version="1.0" revision="A" toolID="Manual" nameStructure="IEDName">
    <Text>Large PRP Test Configuration - 10 IEDs with Dual Network</Text>
  </Header>
  
  <!-- IED Definitions -->
  <IED name="Bay1_Prot" type="ProtectionRelay" manufacturer="VendorA" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbProt1" type="GOOSE" appID="Bay1Prot_GOOSE" datSet="DS1"/>
          </LN0>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="Bay1_Ctrl" type="Controller" manufacturer="VendorA" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbCtrl1" type="GOOSE" appID="Bay1Ctrl_GOOSE" datSet="DS1"/>
          </LN0>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="Bay2_Prot" type="ProtectionRelay" manufacturer="VendorB" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbProt2" type="GOOSE" appID="Bay2Prot_GOOSE" datSet="DS1"/>
          </LN0>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="Bay2_Ctrl" type="Controller" manufacturer="VendorB" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbCtrl2" type="GOOSE" appID="Bay2Ctrl_GOOSE" datSet="DS1"/>
          </LN0>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="Bay3_Prot" type="ProtectionRelay" manufacturer="VendorA" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbProt3" type="GOOSE" appID="Bay3Prot_GOOSE" datSet="DS1"/>
          </LN0>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="Bay3_Ctrl" type="Controller" manufacturer="VendorA" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbCtrl3" type="GOOSE" appID="Bay3Ctrl_GOOSE" datSet="DS1"/>
          </LN0>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="Busbar_Prot" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbBusProt" type="GOOSE" appID="BusProt_GOOSE" datSet="DS1"/>
          </LN0>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="StationMeter1" type="Meter" manufacturer="VendorD" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbMeter1" type="GOOSE" appID="Meter1_GOOSE" datSet="DS1"/>
          </LN0>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="StationMeter2" type="Meter" manufacturer="VendorD" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbMeter2" type="GOOSE" appID="Meter2_GOOSE" datSet="DS1"/>
          </LN0>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <IED name="SCADA_Gateway" type="Gateway" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbGateway" type="GOOSE" appID="Gateway_GOOSE" datSet="DS1"/>
          </LN0>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <!-- Communication Section - PRP Dual Network -->
  <Communication>
    <SubNetwork name="Station_Network_A" type="8-MMS">
      <Text>Station Primary Network (PRP-A)</Text>
      <ConnectedAP iedName="Bay1_Prot" apName="AP1">
        <Address><P type="IP">10.1.1.11</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="Bay1_Ctrl" apName="AP1">
        <Address><P type="IP">10.1.1.12</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="Bay2_Prot" apName="AP1">
        <Address><P type="IP">10.1.1.21</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="Bay2_Ctrl" apName="AP1">
        <Address><P type="IP">10.1.1.22</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="Bay3_Prot" apName="AP1">
        <Address><P type="IP">10.1.1.31</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="Bay3_Ctrl" apName="AP1">
        <Address><P type="IP">10.1.1.32</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="Busbar_Prot" apName="AP1">
        <Address><P type="IP">10.1.1.40</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="StationMeter1" apName="AP1">
        <Address><P type="IP">10.1.1.51</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="StationMeter2" apName="AP1">
        <Address><P type="IP">10.1.1.52</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="SCADA_Gateway" apName="AP1">
        <Address><P type="IP">10.1.1.100</P></Address>
      </ConnectedAP>
    </SubNetwork>
    
    <SubNetwork name="Station_Network_B" type="8-MMS">
      <Text>Station Secondary Network (PRP-B)</Text>
      <ConnectedAP iedName="Bay1_Prot" apName="AP1">
        <Address><P type="IP">10.1.2.11</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="Bay1_Ctrl" apName="AP1">
        <Address><P type="IP">10.1.2.12</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="Bay2_Prot" apName="AP1">
        <Address><P type="IP">10.1.2.21</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="Bay2_Ctrl" apName="AP1">
        <Address><P type="IP">10.1.2.22</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="Bay3_Prot" apName="AP1">
        <Address><P type="IP">10.1.2.31</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="Bay3_Ctrl" apName="AP1">
        <Address><P type="IP">10.1.2.32</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="Busbar_Prot" apName="AP1">
        <Address><P type="IP">10.1.2.40</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="StationMeter1" apName="AP1">
        <Address><P type="IP">10.1.2.51</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="StationMeter2" apName="AP1">
        <Address><P type="IP">10.1.2.52</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="SCADA_Gateway" apName="AP1">
        <Address><P type="IP">10.1.2.100</P></Address>
      </ConnectedAP>
    </SubNetwork>
  </Communication>
  
  <!-- Data Type Templates -->
  <DataTypeTemplates>
    <LNodeType id="LLN0_Type" lnClass="LLN0"/>
  </DataTypeTemplates>
</SCL>
