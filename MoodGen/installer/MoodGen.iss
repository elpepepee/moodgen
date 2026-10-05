; Instalador de MoodGen (VST3) para Windows. Se compila con Inno Setup 6.

#define AppName "MoodGen"
#define AppVersion "0.1.0"

[Setup]
AppId={{6B1D3C52-8F2E-4C57-9A41-3E7D5B0C91A8}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=MoodGen
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
DirExistsWarning=no
UsePreviousAppDir=no
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\installer_out
OutputBaseFilename=MoodGen-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#AppName} (VST3)
CreateUninstallRegKey=yes

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\dist\MoodGen.vst3\*"; DestDir: "{commoncf64}\VST3\MoodGen.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\MoodGen.vst3"
