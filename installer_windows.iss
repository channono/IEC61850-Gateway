; Inno Setup Script for IED Simulator Windows Installer
; Download Inno Setup: https://jrsoftware.org/isdl.php

#define MyAppName "IEC61850 IED Simulator"
#define MyAppVersion "1.6.1"
#define MyAppPublisher "libIEC61850 Project"
#define MyAppURL "https://github.com/mz-automation/libiec61850"

[Setup]
AppId={{8A7D5F3E-9B2C-4D1E-8F6A-3C4B5D6E7F8A}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={autopf}\IED_Simulator
DefaultGroupName={#MyAppName}
OutputDir=.\ied_simulators_binaries
OutputBaseFilename=IED_Simulator_Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"

[Files]
Source: "ied_simulators_binaries\windows_x64\*.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "ied_simulators_binaries\windows_x64\*.dll"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs
Source: "ied_simulators_binaries\windows_x64\README.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\IED Simulator (Basic IO)"; Filename: "{app}\server_example_basic_io.exe"; Parameters: "10102"
Name: "{group}\IED Simulator (GOOSE)"; Filename: "{app}\server_example_goose.exe"; Parameters: "10102"
Name: "{group}\Uninstall"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\server_example_basic_io.exe"; Parameters: "10102"; Description: "Launch IED Simulator"; Flags: nowait postinstall skipifsilent
