<?xml version="1.0" encoding="UTF-8"?>
<SCL xmlns="http://www.iec.ch/61850/2003/SCL" version="2007" revision="B">
  <Header id="PRP_RedBox_Station" version="1.0" revision="A" toolID="Manual" nameStructure="IEDName">
    <Text>PRP with 2 RedBoxes - Each RedBox with 30 IEDs (60 IEDs Total)</Text>
  </Header>
  
  <!-- Gateway -->
  <IED name="SCADA_Gateway" type="GATEWAY" manufacturer="VendorA" configVersion="1.0">
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
  
  <!-- RedBox A - Connects HSR Ring to PRP Network -->
  <IED name="RedBox_A" type="RedBox" manufacturer="VendorB" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbRedBoxA" type="GOOSE" appID="RedBoxA_GOOSE" datSet="DS1"/>
          </LN0>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <!-- RedBox B - Connects HSR Ring to PRP Network -->
  <IED name="RedBox_B" type="RedBox" manufacturer="VendorB" configVersion="1.0">
    <AccessPoint name="AP1">
      <Server>
        <LDevice inst="LD0">
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <GSEControl name="gcbRedBoxB" type="GOOSE" appID="RedBoxB_GOOSE" datSet="DS1"/>
          </LN0>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  
  <!-- RedBox A - 30 IEDs (Ring A) -->
  <IED name="RingA_IED01" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED01_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED02" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED02_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED03" type="Controller" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED03_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED04" type="Meter" manufacturer="VendorD" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED04_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED05" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED05_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED06" type="Controller" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED06_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED07" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED07_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED08" type="Meter" manufacturer="VendorD" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED08_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED09" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED09_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED10" type="Controller" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED10_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED11" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED11_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED12" type="Meter" manufacturer="VendorD" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED12_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED13" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED13_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED14" type="Controller" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED14_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED15" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED15_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED16" type="Meter" manufacturer="VendorD" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED16_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED17" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED17_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED18" type="Controller" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED18_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED19" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED19_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED20" type="Meter" manufacturer="VendorD" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED20_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED21" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED21_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED22" type="Controller" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED22_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED23" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED23_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED24" type="Meter" manufacturer="VendorD" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED24_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED25" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED25_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED26" type="Controller" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED26_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED27" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED27_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED28" type="Meter" manufacturer="VendorD" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED28_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED29" type="ProtectionRelay" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED29_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingA_IED30" type="Controller" manufacturer="VendorC" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingA_IED30_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  
  <!-- RedBox B - 30 IEDs (Ring B) -->
  <IED name="RingB_IED01" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED01_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED02" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED02_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED03" type="Controller" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED03_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED04" type="Meter" manufacturer="VendorF" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED04_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED05" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED05_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED06" type="Controller" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED06_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED07" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED07_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED08" type="Meter" manufacturer="VendorF" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED08_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED09" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED09_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED10" type="Controller" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED10_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED11" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED11_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED12" type="Meter" manufacturer="VendorF" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED12_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED13" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED13_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED14" type="Controller" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED14_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED15" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED15_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED16" type="Meter" manufacturer="VendorF" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED16_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED17" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED17_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED18" type="Controller" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED18_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED19" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED19_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED20" type="Meter" manufacturer="VendorF" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED20_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED21" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED21_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED22" type="Controller" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED22_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED23" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED23_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED24" type="Meter" manufacturer="VendorF" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED24_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED25" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED25_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED26" type="Controller" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED26_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED27" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED27_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED28" type="Meter" manufacturer="VendorF" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED28_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED29" type="ProtectionRelay" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED29_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  <IED name="RingB_IED30" type="Controller" manufacturer="VendorE" configVersion="1.0">
    <AccessPoint name="AP1"><Server><LDevice inst="LD0"><LN0 lnClass="LLN0" inst="" lnType="LLN0_Type"><GSEControl name="gcb" type="GOOSE" appID="RingB_IED30_GOOSE" datSet="DS1"/></LN0></LDevice></Server></AccessPoint>
  </IED>
  
  <!-- Communication Section - PRP Dual Network with RedBoxes -->
  <Communication>
    <SubNetwork name="PRP_Network_A" type="8-MMS">
      <Text>PRP Network A - Primary Network</Text>
      <ConnectedAP iedName="SCADA_Gateway" apName="AP1">
        <Address><P type="IP">10.1.1.1</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="RedBox_A" apName="AP1">
        <Address><P type="IP">10.1.1.10</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="RedBox_B" apName="AP1">
        <Address><P type="IP">10.1.1.20</P></Address>
      </ConnectedAP>
    </SubNetwork>
    
    <SubNetwork name="PRP_Network_B" type="8-MMS">
      <Text>PRP Network B - Secondary Network</Text>
      <ConnectedAP iedName="SCADA_Gateway" apName="AP1">
        <Address><P type="IP">10.1.2.1</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="RedBox_A" apName="AP1">
        <Address><P type="IP">10.1.2.10</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="RedBox_B" apName="AP1">
        <Address><P type="IP">10.1.2.20</P></Address>
      </ConnectedAP>
    </SubNetwork>
    
    <!-- HSR Ring A - Connected via RedBox A -->
    <SubNetwork name="HSR_Ring_A" type="8-MMS">
      <Text>HSR Ring A - 30 IEDs via RedBox A</Text>
      <ConnectedAP iedName="RedBox_A" apName="AP1">
        <Address><P type="IP">192.168.10.1</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="RingA_IED01" apName="AP1"><Address><P type="IP">192.168.10.11</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED02" apName="AP1"><Address><P type="IP">192.168.10.12</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED03" apName="AP1"><Address><P type="IP">192.168.10.13</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED04" apName="AP1"><Address><P type="IP">192.168.10.14</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED05" apName="AP1"><Address><P type="IP">192.168.10.15</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED06" apName="AP1"><Address><P type="IP">192.168.10.16</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED07" apName="AP1"><Address><P type="IP">192.168.10.17</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED08" apName="AP1"><Address><P type="IP">192.168.10.18</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED09" apName="AP1"><Address><P type="IP">192.168.10.19</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED10" apName="AP1"><Address><P type="IP">192.168.10.20</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED11" apName="AP1"><Address><P type="IP">192.168.10.21</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED12" apName="AP1"><Address><P type="IP">192.168.10.22</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED13" apName="AP1"><Address><P type="IP">192.168.10.23</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED14" apName="AP1"><Address><P type="IP">192.168.10.24</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED15" apName="AP1"><Address><P type="IP">192.168.10.25</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED16" apName="AP1"><Address><P type="IP">192.168.10.26</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED17" apName="AP1"><Address><P type="IP">192.168.10.27</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED18" apName="AP1"><Address><P type="IP">192.168.10.28</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED19" apName="AP1"><Address><P type="IP">192.168.10.29</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED20" apName="AP1"><Address><P type="IP">192.168.10.30</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED21" apName="AP1"><Address><P type="IP">192.168.10.31</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED22" apName="AP1"><Address><P type="IP">192.168.10.32</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED23" apName="AP1"><Address><P type="IP">192.168.10.33</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED24" apName="AP1"><Address><P type="IP">192.168.10.34</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED25" apName="AP1"><Address><P type="IP">192.168.10.35</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED26" apName="AP1"><Address><P type="IP">192.168.10.36</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED27" apName="AP1"><Address><P type="IP">192.168.10.37</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED28" apName="AP1"><Address><P type="IP">192.168.10.38</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED29" apName="AP1"><Address><P type="IP">192.168.10.39</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingA_IED30" apName="AP1"><Address><P type="IP">192.168.10.40</P></Address></ConnectedAP>
    </SubNetwork>
    
    <!-- HSR Ring B - Connected via RedBox B -->
    <SubNetwork name="HSR_Ring_B" type="8-MMS">
      <Text>HSR Ring B - 30 IEDs via RedBox B</Text>
      <ConnectedAP iedName="RedBox_B" apName="AP1">
        <Address><P type="IP">192.168.20.1</P></Address>
      </ConnectedAP>
      <ConnectedAP iedName="RingB_IED01" apName="AP1"><Address><P type="IP">192.168.20.11</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED02" apName="AP1"><Address><P type="IP">192.168.20.12</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED03" apName="AP1"><Address><P type="IP">192.168.20.13</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED04" apName="AP1"><Address><P type="IP">192.168.20.14</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED05" apName="AP1"><Address><P type="IP">192.168.20.15</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED06" apName="AP1"><Address><P type="IP">192.168.20.16</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED07" apName="AP1"><Address><P type="IP">192.168.20.17</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED08" apName="AP1"><Address><P type="IP">192.168.20.18</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED09" apName="AP1"><Address><P type="IP">192.168.20.19</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED10" apName="AP1"><Address><P type="IP">192.168.20.20</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED11" apName="AP1"><Address><P type="IP">192.168.20.21</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED12" apName="AP1"><Address><P type="IP">192.168.20.22</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED13" apName="AP1"><Address><P type="IP">192.168.20.23</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED14" apName="AP1"><Address><P type="IP">192.168.20.24</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED15" apName="AP1"><Address><P type="IP">192.168.20.25</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED16" apName="AP1"><Address><P type="IP">192.168.20.26</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED17" apName="AP1"><Address><P type="IP">192.168.20.27</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED18" apName="AP1"><Address><P type="IP">192.168.20.28</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED19" apName="AP1"><Address><P type="IP">192.168.20.29</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED20" apName="AP1"><Address><P type="IP">192.168.20.30</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED21" apName="AP1"><Address><P type="IP">192.168.20.31</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED22" apName="AP1"><Address><P type="IP">192.168.20.32</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED23" apName="AP1"><Address><P type="IP">192.168.20.33</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED24" apName="AP1"><Address><P type="IP">192.168.20.34</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED25" apName="AP1"><Address><P type="IP">192.168.20.35</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED26" apName="AP1"><Address><P type="IP">192.168.20.36</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED27" apName="AP1"><Address><P type="IP">192.168.20.37</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED28" apName="AP1"><Address><P type="IP">192.168.20.38</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED29" apName="AP1"><Address><P type="IP">192.168.20.39</P></Address></ConnectedAP>
      <ConnectedAP iedName="RingB_IED30" apName="AP1"><Address><P type="IP">192.168.20.40</P></Address></ConnectedAP>
    </SubNetwork>
  </Communication>
  
  <!-- Data Type Templates -->
  <DataTypeTemplates>
    <LNodeType id="LLN0_Type" lnClass="LLN0"/>
  </DataTypeTemplates>
</SCL>
