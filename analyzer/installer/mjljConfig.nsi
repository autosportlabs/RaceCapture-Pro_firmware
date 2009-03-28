;MJLJ Installation Script


;--------------------------------
;Include Modern UI

  !include "MUI.nsh"
  !include "fileAssoc.nsh"

;--------------------------------
;General

!define APP_NAME "Megajolt Lite Jr. Configurator v4.0.2"
!define APP_REG_NAME "MJLJ_Configurator_4.0.2"
  ;Name and file
  
  Var /GLOBAL APP_SM
  Var /GLOBAL MUI_TEMP
  
  Name "${APP_NAME}"
  OutFile "mjljConfig_v4.0.2.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\${APP_NAME}"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\${APP_NAME}" ""

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_STARTMENU "Application" $APP_SM 
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections


Section "Megajolt Lite Jr. Configurator" SecMJLJ

  SetOutPath "$INSTDIR"
  
  ;ADD YOUR OWN FILES HERE...
  File ..\Release\mjljConfig.exe
  File cricket1.glf
  File wxctb-0.8.dll
  File mingwm10.dll
  File Quickstart_README.txt
  File Release_Notes.txt
  File License.txt
  
  CreateDirectory $SMPROGRAMS\$APP_SM
  CreateShortCut "$SMPROGRAMS\$APP_SM\${APP_NAME}.lnk" "$INSTDIR\mjljConfig.exe"
  CreateShortCut "$SMPROGRAMS\$APP_SM\Uninstall.lnk" "$INSTDIR\uninstall.exe"
  CreateShortCut "$SMPROGRAMS\$APP_SM\Quickstart_README.lnk" "$INSTDIR\Quickstart_README.txt"
  CreateShortCut "$SMPROGRAMS\$APP_SM\Release_Notes.lnk" "$INSTDIR\Release_Notes.txt"
  CreateShortCut "$SMPROGRAMS\$APP_SM\License.lnk" "$INSTDIR\License.txt"
  
  ;Store installation folder
  WriteRegStr HKCU "Software\${APP_REG_NAME}" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  !insertmacro APP_ASSOCIATE "cfg" "mjljConfig.configurationFile" "MJLJ Configuration file" "$INSTDIR\mjljConfig.exe,0" \
    "Open with MJLJ Configurator" "$INSTDIR\mjljConfig.exe $\"%1$\""
    
  !insertmacro APP_ASSOCIATE "mjlj" "mjljConfig.configurationFile" "MJLJ Configuration file" "$INSTDIR\mjljConfig.exe,0" \
    "Open with MJLJ Configurator" "$INSTDIR\mjljConfig.exe $\"%1$\""
    
    
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecMJLJ ${LANG_ENGLISH} "Installs the Megajolt Lite Jr. Configuration Software"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

;ADD YOUR OWN FILES HERE...

!insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
;Delete empty start menu parent diretories
StrCpy "$MUI_TEMP" "$SMPROGRAMS\$MUI_TEMP"
RMDir /r "$MUI_TEMP"


  Delete  "$SMPROGRAMS\$APP_SM\${APP_NAME}.lnk"
  Delete  "$SMPROGRAMS\$APP_SM\Uninstall.lnk"
  Delete  "$SMPROGRAMS\$APP_SM\Quickstart_README.lnk"
  Delete  "$SMPROGRAMS\$APP_SM\Release_Notes.lnk"
  RMDir  "$SMPROGRAMS\$APP_SM"

  Delete "$INSTDIR\mjljConfig.exe"
  Delete "$INSTDIR\cricket1.glf"
  Delete "$INSTDIR\wxctb-0.8.dll"
  Delete "$INSTDIR\mingwm10.dll"
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\Quickstart_README.txt"
  Delete "$INSTDIR\Release_Notes.txt"
  Delete "$INSTDIR\License.txt"
  
  RMDir "$INSTDIR"
  
DeleteRegKey /ifempty HKCU "Software\${APP_REG_NAME}"

!insertmacro APP_UNASSOCIATE "cfg" "mjljConfig.configurationFile"
!insertmacro APP_UNASSOCIATE "mjlj" "mjljConfig.configurationFile"

SectionEnd