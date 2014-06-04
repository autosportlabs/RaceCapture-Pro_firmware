;MJLJ Installation Script


;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !include "fileAssoc.nsh"

;--------------------------------
;General

;pass in the actual version by
;makensis -DVERSION_STRING="1.2.3" raceCapturePro.nsi
!ifndef VERSION_STRING
	!define VERSION_STRING "0.0.0"
!endif

!ifndef RA_DIR
	!define RA_DIR "..\.."
!endif

!ifndef EXE_DIR
	!define EXE_DIR "${RA_DIR}\MinGW-Release"
!endif


!define APP_TITLE "Race Analyzer v${VERSION_STRING}"
!define APP_NAME "Race Analyzer"
!define APP_INSTALL_DIR "RaceAnalyzer_v${VERSION_STRING}"
!define APP_REG_NAME "RaceAnalyzer_${VERSION_STRING}"
  ;Name and file
  
  Var /GLOBAL APP_SM
  Var /GLOBAL MUI_TEMP

  Name "${APP_TITLE}"
  OutFile "raceAnalyzer_${VERSION_STRING}.exe"


  ;Default installation folder
  InstallDir "$PROGRAMFILES\${APP_INSTALL_DIR}"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\${APP_NAME}" ""

 ;--------------------------------
;Interface Settings

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "installer_graphic.bmp" ; optional
  ;!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
  !define MUI_HEADER_TRANSPARENT_TEXT
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

  RequestExecutionLevel admin

;--------------------------------
;Installer Sections



Section "Race Analyzer" SecRaceAnalyzer

  SetOutPath "$INSTDIR"

  ;ADD YOUR OWN FILES HERE...
  File ${EXE_DIR}\raceAnalyzer.exe
  File libgcc_s_dw2-1.dll
  File libstdc++-6.dll
  File License.txt
  File ${RA_DIR}\RELEASE_NOTES.txt

  CreateDirectory $SMPROGRAMS\$APP_SM
  CreateShortCut "$SMPROGRAMS\$APP_SM\${APP_NAME}.lnk" "$INSTDIR\raceAnalyzer.exe"
  CreateShortCut "$SMPROGRAMS\$APP_SM\Uninstall.lnk" "$INSTDIR\uninstall.exe"
  CreateShortCut "$SMPROGRAMS\$APP_SM\RELEASE_NOTES.lnk" "$INSTDIR\RELEASE_NOTES.txt"
  CreateShortCut "$SMPROGRAMS\$APP_SM\License.lnk" "$INSTDIR\License.txt"

  ;Store installation folder
  WriteRegStr HKCU "Software\${APP_REG_NAME}" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_REG_NAME}" \
                 "DisplayName" "${APP_TITLE}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_REG_NAME}" \
                 "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_REG_NAME}" \
				  "Publisher" "Autosport Labs"


  !insertmacro APP_ASSOCIATE "rcap" "raceCapturePro.configurationFile" "RaceCapture/Pro Configuration file" "$INSTDIR\raceAnalyzer.exe,0" \
    "Open with Race Analyzer" "$INSTDIR\raceAnalyzer.exe $\"%1$\""

  !insertmacro APP_ASSOCIATE "radb" "raceAnalyzer.raceEvent" "Race Analyzer Race Event file" "$INSTDIR\raceAnalyzer.exe,0" \
    "Open with Race Analyzer" "$INSTDIR\raceAnalyzer.exe $\"%1$\""

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecRaceAnalyzer ${LANG_ENGLISH} "Installs the Autosport Labs Race Analyzer software"

  ;Assign language strings to sections
;;  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  ;  !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy)
 ; !insertmacro MUI_FUNCTION_DESCRIPTION_END

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

  Delete "$INSTDIR\raceAnalyzer.exe"
  Delete "$INSTDIR\libgcc_s_dw2-1.dll"
  Delete "$INSTDIR\libstdc++-6.dll"
  Delete "$INSTDIR\Uninstall.exe"
  ;Delete "$INSTDIR\Quickstart_README.txt"
  ;Delete "$INSTDIR\Release_Notes.txt"
  Delete "$INSTDIR\License.txt"

  RMDir "$INSTDIR"

DeleteRegKey /ifempty HKCU "Software\${APP_REG_NAME}"
DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_REG_NAME}"

!insertmacro APP_UNASSOCIATE "rcap" "raceCapturePro.configurationFile"
!insertmacro APP_UNASSOCIATE "radb" "raceAnalyzer.raceEvent"

SectionEnd
