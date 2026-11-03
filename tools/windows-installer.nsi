Unicode true
!include "MUI2.nsh"
!include "x64.nsh"
!include "LogicLib.nsh"
!ifndef PAYLOAD_DIR
  !error "Set PAYLOAD_DIR to the staged portable directory"
!endif
!ifndef OUTPUT_FILE
  !error "Set OUTPUT_FILE to the installer output path"
!endif
!ifndef VERSION
  !error "Set VERSION to the build version"
!endif
!ifndef UNINSTALL_FILES
  !error "Set UNINSTALL_FILES to the generated exact payload removal list"
!endif
!ifndef GAME_ICON
  !error "Set GAME_ICON to the original game icon artwork"
!endif
Name "亡灵故事 / Deadtime Stories"
OutFile "${OUTPUT_FILE}"
InstallDir "$LOCALAPPDATA\Programs\OpenLegacyGame\DeadTimeStories"
InstallDirRegKey HKCU "Software\OpenLegacyGame\DeadTimeStories" "InstallDir"
RequestExecutionLevel user
SetCompressor /SOLID lzma
ShowInstDetails show
ShowUninstDetails show
!define MUI_ABORTWARNING
!define MUI_ICON "${GAME_ICON}"
!define MUI_UNICON "${GAME_ICON}"
!define MUI_FINISHPAGE_RUN "$INSTDIR\deadtime.exe"
!define MUI_FINISHPAGE_RUN_NOTCHECKED
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "English"
Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "This application requires 64-bit Windows."
    Abort
  ${EndIf}
  SetRegView 64
  SetShellVarContext current
FunctionEnd
Section "Deadtime Stories"
  SetOutPath "$INSTDIR"
  File /r "${PAYLOAD_DIR}\*"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKCU "Software\OpenLegacyGame\DeadTimeStories" "InstallDir" "$INSTDIR"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenLegacyGameDeadTimeStories" "DisplayName" "亡灵故事 / Deadtime Stories"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenLegacyGameDeadTimeStories" "DisplayVersion" "${VERSION}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenLegacyGameDeadTimeStories" "InstallLocation" "$INSTDIR"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenLegacyGameDeadTimeStories" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenLegacyGameDeadTimeStories" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenLegacyGameDeadTimeStories" "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenLegacyGameDeadTimeStories" "NoRepair" 1
  CreateDirectory "$SMPROGRAMS\Deadtime Stories"
  CreateShortcut "$SMPROGRAMS\Deadtime Stories\亡灵故事.lnk" "$INSTDIR\deadtime.exe"
  CreateShortcut "$DESKTOP\亡灵故事.lnk" "$INSTDIR\deadtime.exe"
SectionEnd
; File scans or a running game can temporarily prevent removal. Only finish the
; uninstall registration after every owned payload file was actually removed.
Function un.DeletePayloadFile
  Exch $0
  Push $1
  StrCpy $1 0
  retry_delete:
    ClearErrors
    Delete "$0"
    IfErrors 0 delete_done
    IntOp $1 $1 + 1
    ${If} $1 < 20
      Sleep 100
      Goto retry_delete
    ${EndIf}
    SetErrorLevel 1
    IfSilent delete_failed
    MessageBox MB_OK|MB_ICONEXCLAMATION "无法删除文件，请关闭游戏后重试：$\r$\n$0"
  delete_failed:
    Abort "Cannot remove $0; close the game and retry uninstall."
  delete_done:
    Pop $1
    Pop $0
FunctionEnd
Section "Uninstall"
  SetRegView 64
  SetShellVarContext current
  Delete "$SMPROGRAMS\Deadtime Stories\亡灵故事.lnk"
  RMDir "$SMPROGRAMS\Deadtime Stories"
  Delete "$DESKTOP\亡灵故事.lnk"
  ; Generated from the exact staged file set, never recursively delete INSTDIR.
  !include "${UNINSTALL_FILES}"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
  DeleteRegKey HKCU "Software\OpenLegacyGame\DeadTimeStories"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenLegacyGameDeadTimeStories"
  ; User profiles, saves and settings live outside INSTDIR and remain intact.
SectionEnd
