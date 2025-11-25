<?xml version="1.0"?>
<SCL version="2007" revision="B" xmlns="http://www.iec.ch/61850/2003/SCL">
  <Header id="Generated_SCD" />
  <Substation name="Substation1">
    <VoltageLevel name="110kV" />
  </Substation>
  <Communication>
    <SubNetwork name="StationBus" type="8-MMS">
      <ConnectedAP iedName="TestIED_BasicIO" apName="AP1">
        <Address>
          <P type="IP">192.168.0.40</P>
          <P type="IP-SUBNET">255.255.255.0</P>
          <P type="IP-GATEWAY">192.168.1.1</P>
        </Address>
      </ConnectedAP>
    </SubNetwork>
  </Communication>
  <IED name="TestIED_BasicIO">
    <AccessPoint name="AP1">
      <Server>
        <Authentication none="true" />
        <LDevice />
        <LDevice inst="simpleIOGenericIO">
          <LN lnClass="GGIO" inst="1" lnType="GGIO_Type">
            <DOI name="Mod" type="SPS_Type" />
            <DOI name="SPCSO1" type="SPC_Type" />
            <DOI name="SPCSO2" type="SPC_Type" />
            <DOI name="SPCSO3" type="SPC_Type" />
            <DOI name="SPCSO4" type="SPC_Type" />
            <DOI name="NamPlt" type="DPL_Type" />
            <DOI name="AnIn1" type="MV_Type" />
            <DOI name="AnIn2" type="MV_Type" />
            <DOI name="AnIn3" type="MV_Type" />
            <DOI name="AnIn4" type="MV_Type" />
            <DOI name="Beh" type="SPS_Type" />
            <DOI name="Health" type="SPS_Type" />
            <DOI name="Ind1" type="SPS_Type" />
            <DOI name="Ind2" type="SPS_Type" />
            <DOI name="Ind3" type="SPS_Type" />
            <DOI name="Ind4" type="SPS_Type" />
          </LN>
          <LN0 lnClass="LLN0" inst="" lnType="LLN0_Type">
            <DOI name="Mod" type="SPS_Type" />
            <DOI name="NamPlt" type="LPL_Type" />
            <DOI name="Beh" type="SPS_Type" />
            <DOI name="Health" type="SPS_Type" />
          </LN0>
          <LN lnClass="LPHD" inst="1" lnType="LPHD_Type">
            <DOI name="PhyNam" />
            <DOI name="PhyHealth" type="SPS_Type" />
            <DOI name="Proxy" type="SPS_Type" />
          </LN>
        </LDevice>
      </Server>
    </AccessPoint>
  </IED>
  <DataTypeTemplates>
    <DOType id="SPS_Type" cdc="SPS">
      <DA name="stVal" bType="BOOLEAN" fc="ST" />
    </DOType>
    <DOType id="MV_Type" cdc="MV">
      <DA name="mag" bType="Struct" fc="MX" />
      <DA name="q" bType="Quality" fc="MX" />
    </DOType>
    <DOType id="SPC_Type" cdc="SPC">
      <DA name="stVal" bType="BOOLEAN" fc="ST" />
      <DA name="Oper" bType="Struct" fc="CO" />
    </DOType>
    <DOType id="DPL_1_PhyNam" cdc="DPL">
      <DA name="vendor" bType="VisString255" fc="DC" />
      <DA name="swRev" bType="VisString255" fc="DC" />
      <DA name="d" bType="VisString255" fc="DC" />
      <DA name="configRev" bType="VisString255" fc="DC" />
    </DOType>
    <LNodeType id="LPHD_Type" lnClass="LPHD">
      <DO name="PhyNam" type="DPL_1_PhyNam" />
      <DO name="PhyHealth" type="SPS_Type" />
      <DO name="Proxy" type="SPS_Type" />
    </LNodeType>
  </DataTypeTemplates>
</SCL>
