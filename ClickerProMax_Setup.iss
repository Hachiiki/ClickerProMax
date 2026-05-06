[Setup]
AppName=ClickerProMax
AppVersion=1.1.0
AppPublisher=Hachiiki
AppPublisherURL=https://github.com/Hachiiki/ClickerProMax
AppSupportURL=https://github.com/Hachiiki/ClickerProMax/issues
AppUpdatesURL=https://github.com/Hachiiki/ClickerProMax/releases
DefaultDirName={commonpf}\ClickerProMax
DefaultGroupName=ClickerProMax
AllowNoIcons=yes
OutputDir=installer
OutputBaseFilename=ClickerProMax_Setup
SetupIconFile=src/logo/ClickerProMax.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "bin\AutoClicker.exe"; DestDir: "{app}"; Flags: ignoreversion
; Qt 6.9.3 DLLs
Source: "C:\Qt\6.9.3\mingw_64\bin\Qt6Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\6.9.3\mingw_64\bin\Qt6Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\6.9.3\mingw_64\bin\Qt6Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\6.9.3\mingw_64\bin\Qt6Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\6.9.3\mingw_64\bin\libgcc_s_seh-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\6.9.3\mingw_64\bin\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\6.9.3\mingw_64\bin\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion

; Platform plugins
Source: "C:\Qt\6.9.3\mingw_64\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion

; Styles
Source: "C:\Qt\6.9.3\mingw_64\plugins\styles\qmodernwindowsstyle.dll"; DestDir: "{app}\styles"; Flags: ignoreversion

; Image format plugins
Source: "C:\Qt\6.9.3\mingw_64\plugins\imageformats\qgif.dll"; DestDir: "{app}\imageformats"; Flags: ignoreversion
Source: "C:\Qt\6.9.3\mingw_64\plugins\imageformats\qico.dll"; DestDir: "{app}\imageformats"; Flags: ignoreversion
Source: "C:\Qt\6.9.3\mingw_64\plugins\imageformats\qjpeg.dll"; DestDir: "{app}\imageformats"; Flags: ignoreversion
Source: "C:\Qt\6.9.3\mingw_64\plugins\imageformats\qsvg.dll"; DestDir: "{app}\imageformats"; Flags: ignoreversion

; Logo files
Source: "src\logo\ClickerProMax.ico"; DestDir: "{app}\logo"; Flags: ignoreversion
Source: "src\logo\ClickerProMax.png"; DestDir: "{app}\logo"; Flags: ignoreversion

[Icons]
Name: "{group}\ClickerProMax"; Filename: "{app}\AutoClicker.exe"; IconFilename: "{app}\logo\ClickerProMax.ico"
Name: "{group}\{cm:UninstallProgram,ClickerProMax}"; Filename: "{uninstallexe}"; IconFilename: "{app}\logo\ClickerProMax.ico"
Name: "{commondesktop}\ClickerProMax"; Filename: "{app}\AutoClicker.exe"; IconFilename: "{app}\logo\ClickerProMax.ico"; Tasks: desktopicon

[Run]
Filename: "{app}\AutoClicker.exe"; Description: "{cm:LaunchProgram,ClickerProMax}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}"
