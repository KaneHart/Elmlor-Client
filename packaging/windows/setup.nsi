; This script allows the following parameters being overwritten from
; command line. When called without any parameters it behaves exactly
; like the old install script.
;
; DLLDIR - directory containing required dlls
; EXEDIR - directory containing manaplus.exe
; EXESUFFIX - offset to SRCDIR pointing to a directory containing manaplus.exe
; PRODUCT_VERSION - software version
; UPX - upx binary name
;
; For a cmake build on UNIX the following should give you a working installer:
; makensis -DDLLDIR=/path/to/dlls \
;    -DPRODUCT_VERSION=0.1.`date +%Y%m%d`
;    -DUPX=upx
;    -DEXESUFFIX=/src

CRCCheck on
SetCompress off
SetCompressor /SOLID lzma

RequestExecutionLevel admin

!define SRCDIR "..\.."
!ifndef UPX
  !define "UPX upx\upx.exe"
!endif

!ifdef EXESUFFIX
  !define EXEDIR ${SRCDIR}/${EXESUFFIX}
!endif

!ifndef EXEDIR
  !define EXEDIR ${SRCDIR}
!endif

!ifndef DLLDIR
  !define DLLDIR ${SRCDIR}/dll
!endif

;--- (and without !defines ) ---
!System "${UPX} --best --crp-ms=999999 --compress-icons=0 --nrv2d ${EXEDIR}\manaplus.exe"

!define MULTIUSER_INSTALLMODE_COMMANDLINE
!include "MultiUser.nsh"

; HM NIS Edit helper defines
!define PRODUCT_NAME "ManaPlus"
!ifndef PRODUCT_VERSION
  !define PRODUCT_VERSION "1.1"
!endif
!define PRODUCT_PUBLISHER "ManaPlus Development Team"
!define PRODUCT_WEB_SITE "http://manaplus.evolonline.org/"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\manaplus.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "SHCTX"

!include "FileAssociation.nsh"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
;!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\win-install.ico"
!define MUI_ICON "${SRCDIR}\data\icons\manaplus.ico"
;!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\win-uninstall.ico"
!define MUI_UNICON "${SRCDIR}\data\icons\manaplus.ico"

;Language Selection Dialog Settings
;Remember the installer language
!define MUI_LANGDLL_REGISTRY_ROOT "HKCU"
!define MUI_LANGDLL_REGISTRY_KEY "Software\Mana"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

!define MUI_WELCOMEFINISHPAGE_BITMAP "setup_welcome.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "setup_welcome.bmp"

; Welcome page

!define MUI_WELCOMEPAGE_TITLE_3LINES
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "${SRCDIR}\COPYING"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Components page
!insertmacro MUI_PAGE_COMPONENTS
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION RunMana
!define MUI_FINISHPAGE_SHOWREADME 'notepad.exe "$\"$INSTDIR\README$\""'
!define MUI_PAGE_CUSTOMFUNCTION_PRE changeFinishImage
!define MUI_FINISHPAGE_LINK "Visit ManaPlus website for the latest news, FAQs and support"
!define MUI_FINISHPAGE_LINK_LOCATION "http://manaplus.evolonline.org/"
!insertmacro MUI_PAGE_FINISH

Function RunMana
SetOutPath $INSTDIR
Exec "$INSTDIR\manaplus.exe"
FunctionEnd

Function changeFinishImage
!insertmacro MUI_INSTALLOPTIONS_WRITE "ioSpecial.ini" "Field 1" "Text" "$PLUGINSDIR\setup_finish.bmp"
FunctionEnd

; Uninstaller pages

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!define MUI_FINISHPAGE_TITLE_3LINES
!insertmacro MUI_UNPAGE_FINISH

;Languages
!insertmacro MUI_LANGUAGE "English" # first language is the default language
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "TradChinese"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Korean"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Danish"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Norwegian"
!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "Greek"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Ukrainian"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Croatian"
!insertmacro MUI_LANGUAGE "Bulgarian"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "Thai"
!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Latvian"
!insertmacro MUI_LANGUAGE "Macedonian"
!insertmacro MUI_LANGUAGE "Estonian"
!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Lithuanian"
!insertmacro MUI_LANGUAGE "Catalan"
!insertmacro MUI_LANGUAGE "Slovenian"
!insertmacro MUI_LANGUAGE "Serbian"
!insertmacro MUI_LANGUAGE "SerbianLatin"
!insertmacro MUI_LANGUAGE "Arabic"
!insertmacro MUI_LANGUAGE "Farsi"
!insertmacro MUI_LANGUAGE "Hebrew"
!insertmacro MUI_LANGUAGE "Indonesian"
!insertmacro MUI_LANGUAGE "Mongolian"
!insertmacro MUI_LANGUAGE "Luxembourgish"
!insertmacro MUI_LANGUAGE "Albanian"
!insertmacro MUI_LANGUAGE "Breton"
!insertmacro MUI_LANGUAGE "Belarusian"
!insertmacro MUI_LANGUAGE "Icelandic"
!insertmacro MUI_LANGUAGE "Malay"
!insertmacro MUI_LANGUAGE "Bosnian"
!insertmacro MUI_LANGUAGE "Kurdish"

!insertmacro MUI_RESERVEFILE_LANGDLL

ReserveFile "setup_finish.bmp"

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "manaplus-${PRODUCT_VERSION}-win32.exe"
InstallDir "$PROGRAMFILES\Mana"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show
RequestExecutionLevel admin

Function .onInit
  !insertmacro MULTIUSER_INIT
  !insertmacro MUI_LANGDLL_DISPLAY
  InitPluginsDir
  File /oname=$PLUGINSDIR\setup_finish.bmp "setup_finish.bmp"

  ReadRegStr $R0 SHCTX \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \
  "UninstallString"
  StrCmp $R0 "" done

  MessageBox MB_YESNO|MB_ICONEXCLAMATION \
  "${PRODUCT_NAME} is already installed. $\n$\nClick `YES` (recomended) to remove the \
  previous version or `NO` to install new version over old version." \
  IDNO done

;Run the uninstaller
uninst:
  ClearErrors
  ExecWait '$R0' ;Do not copy the uninstaller to a temp file

  IfErrors no_remove_uninstaller done
    ;You can either use Delete /REBOOTOK in the uninstaller or add some code
    ;here to remove the uninstaller. Use a registry key to check
    ;whether the user has chosen to uninstall. If you are using an uninstaller
    ;components page, make sure all sections are uninstalled.
  no_remove_uninstaller:

done:

FunctionEnd


Section "Core files (required)" SecCore
  SectionIn RO
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  CreateDirectory "$INSTDIR\data"
  CreateDirectory "$INSTDIR\data\fonts"
  CreateDirectory "$INSTDIR\data\graphics"
  CreateDirectory "$INSTDIR\data\help"
  CreateDirectory "$INSTDIR\data\help\fr"
  CreateDirectory "$INSTDIR\data\help\pt"
  CreateDirectory "$INSTDIR\data\help\pt\tips"
  CreateDirectory "$INSTDIR\data\help\pt_BR"
  CreateDirectory "$INSTDIR\data\help\pt_BR\tips"
  CreateDirectory "$INSTDIR\data\help\ru"
  CreateDirectory "$INSTDIR\data\help\ru\tips"
  CreateDirectory "$INSTDIR\data\help\tips"
  CreateDirectory "$INSTDIR\data\icons"
  CreateDirectory "$INSTDIR\data\perserver"
  CreateDirectory "$INSTDIR\data\perserver\default"
  CreateDirectory "$INSTDIR\data\graphics\gui"
  CreateDirectory "$INSTDIR\data\graphics\images"
  CreateDirectory "$INSTDIR\data\graphics\sprites"
  CreateDirectory "$INSTDIR\data\sfx"
  CreateDirectory "$INSTDIR\data\sfx\system"
  CreateDirectory "$INSTDIR\data\themes"
  CreateDirectory "$INSTDIR\data\themes\blackwood"
  CreateDirectory "$INSTDIR\data\themes\classic"
  CreateDirectory "$INSTDIR\data\themes\enchilado"
  CreateDirectory "$INSTDIR\data\themes\mana"
  CreateDirectory "$INSTDIR\data\themes\pink"
  CreateDirectory "$INSTDIR\data\themes\redandblack"
  CreateDirectory "$INSTDIR\data\themes\unity"
  CreateDirectory "$INSTDIR\data\themes\wood"
  CreateDirectory "$INSTDIR\docs"

  SetOverwrite ifnewer
  SetOutPath "$INSTDIR"

  File "${EXEDIR}\manaplus.exe"
  File "${DLLDIR}\SDL.dll"
  File "${DLLDIR}\SDL_image.dll"
  File "${DLLDIR}\SDL_mixer.dll"
  File "${DLLDIR}\SDL_net.dll"
  File "${DLLDIR}\SDL_ttf.dll"
  File "${DLLDIR}\exchndl.dll"
  File "${DLLDIR}\SDL_gfx.dll"
  File "${DLLDIR}\libcurl-4.dll"
  File "${DLLDIR}\libiconv-2.dll"
  File "${DLLDIR}\libiconv2.dll"
  File "${DLLDIR}\libintl3.dll"
  File "${DLLDIR}\libpng12-0.dll"
  File "${DLLDIR}\libxml2-2.dll"
  File "${DLLDIR}\zlib1.dll"
  File "${SRCDIR}\AUTHORS"
  File "${SRCDIR}\COPYING"
  File "${SRCDIR}\NEWS"
  File "${SRCDIR}\README.txt"
  SetOutPath "$INSTDIR\data\fonts"
  File "${SRCDIR}\data\fonts\*.ttf"
  SetOutPath "$INSTDIR\data\graphics\gui"
  File "${SRCDIR}\data\graphics\gui\*.png"
  File "${SRCDIR}\data\graphics\gui\*.xml"
  SetOutPath "$INSTDIR\data\graphics\images"
  File /x minimap_*.png ${SRCDIR}\data\graphics\images\*.png
  File "${SRCDIR}\data\graphics\images\error.png"
  SetOutPath "$INSTDIR\data\graphics\sprites"
  File "${SRCDIR}\data\graphics\sprites\*.gif"
  File "${SRCDIR}\data\graphics\sprites\*.png"
  File "${SRCDIR}\data\graphics\sprites\*.xml"
  SetOutPath "$INSTDIR\data\sfx\system"
  File "${SRCDIR}\data\sfx\system\*.ogg"
  SetOutPath "$INSTDIR\data\themes\blackwood"
  File "${SRCDIR}\data\themes\blackwood\*.png"
  File "${SRCDIR}\data\themes\blackwood\*.xml"
  SetOutPath "$INSTDIR\data\themes\mana"
  File "${SRCDIR}\data\themes\mana\*.xml"
  SetOutPath "$INSTDIR\data\themes\enchilado"
  File "${SRCDIR}\data\themes\enchilado\*.png"
  File "${SRCDIR}\data\themes\enchilado\*.xml"
  SetOutPath "$INSTDIR\data\themes\pink"
  File "${SRCDIR}\data\themes\pink\*.png"
  File "${SRCDIR}\data\themes\pink\*.xml"
  SetOutPath "$INSTDIR\data\themes\redandblack"
  File "${SRCDIR}\data\themes\redandblack\*.png"
  File "${SRCDIR}\data\themes\redandblack\*.xml"
  SetOutPath "$INSTDIR\data\themes\unity"
  File "${SRCDIR}\data\themes\unity\*.png"
  File "${SRCDIR}\data\themes\unity\*.xml"
  SetOutPath "$INSTDIR\data\themes\wood"
  File "${SRCDIR}\data\themes\wood\*.png"
  File "${SRCDIR}\data\themes\wood\*.xml"
  SetOutPath "$INSTDIR\data\help"
  File "${SRCDIR}\data\help\*.txt"
  SetOutPath "$INSTDIR\data\help\fr"
  File "${SRCDIR}\data\help\fr\*.txt"
  SetOutPath "$INSTDIR\data\help\pt\tips"
  File "${SRCDIR}\data\help\pt\tips\*.txt"
  SetOutPath "$INSTDIR\data\help\pt_BR\tips"
  File "${SRCDIR}\data\help\pt_BR\tips\*.txt"
  SetOutPath "$INSTDIR\data\help\ru"
  File "${SRCDIR}\data\help\ru\*.txt"
  SetOutPath "$INSTDIR\data\help\ru\tips"
  File "${SRCDIR}\data\help\ru\tips\*.txt"
  SetOutPath "$INSTDIR\data\help\tips"
  File "${SRCDIR}\data\help\tips\*.txt"
  File "${SRCDIR}\data\help\tips\*.jpg"
  SetOutPath "$INSTDIR\data\icons\"
  File "${SRCDIR}\data\icons\manaplus.ico"
  SetOutPath "$INSTDIR\data\perserver\default\"
  File "${SRCDIR}\data\perserver\default\*.txt"
  File "${SRCDIR}\data\perserver\default\*.xml"
  SetOutPath "$INSTDIR\docs"
  File "${SRCDIR}\docs\FAQ.txt"
SectionEnd

Section "Create Shortcuts" SecShortcuts
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  CreateDirectory "$SMPROGRAMS\Mana"
  CreateShortCut "$SMPROGRAMS\Mana\ManaPlus.lnk" "$INSTDIR\manaplus.exe"
  CreateShortCut "$SMPROGRAMS\Mana\ManaPlus (no opengl).lnk" "$INSTDIR\manaplus.exe" --no-opengl
  CreateShortCut "$SMPROGRAMS\Mana\ManaPlus (safemode).lnk" "$INSTDIR\manaplus.exe" --safemode
  CreateShortCut "$SMPROGRAMS\Mana\ManaPlus (tests).lnk" "$INSTDIR\manaplus.exe" --tests
  CreateShortCut "$DESKTOP\ManaPlus.lnk" "$INSTDIR\manaplus.exe"
  CreateShortCut "$DESKTOP\ManaPlus (tests).lnk" "$INSTDIR\manaplus.exe" --tests

  ${registerExtension} "$INSTDIR\manaplus.exe" ".manaplus" "ManaPlus brandings"
SectionEnd

Section /o "Tmw music" SecTmwMusic
  AddSize 25200
  CreateDirectory "$INSTDIR\data\music"
  SetOutPath "$INSTDIR\data\music"
  NSISdl::download "http://downloads.sourceforge.net/themanaworld/tmwmusic-0.3.tar.gz" "$TEMP\tmwmusic-0.3.tar.gz"
  ;Requires an additional plugin from http://nsis.sourceforge.net/UnTGZ_plug-in  Place untgz.dll in your nsis/plugin dir
  untgz::extract -j -d "$INSTDIR\data\music" "$TEMP\tmwmusic-0.3.tar.gz"
  Delete "$TEMP\tmwmusic-0.3.tar.gz"
SectionEnd

Section /o "Portable" SecPortable
  SetOutPath "$INSTDIR"
  File "portable.xml"
SectionEnd

Section /o "Evol Online music" SecEvolMusic
  AddSize 9787
  CreateDirectory "$INSTDIR\data\music"
  SetOutPath "$INSTDIR\data\music"
  NSISdl::download "http://downloads.sourceforge.net/project/evolonline/music/evolmusic-beta1-1.tar.gz" "$TEMP\evolmusic-beta1-1.tar.gz"
  untgz::extract -j -d "$INSTDIR\data\music" "$TEMP\evolmusic-beta1-1.tar.gz"
  Delete "$TEMP\evolmusic-beta1-1.tar.gz"
SectionEnd

Section "Evol Online shortcuts" SecEvol
  SetOutPath "$INSTDIR"
  CreateDirectory "$INSTDIR\data\evol"
  CreateDirectory "$INSTDIR\data\evol\icons"
  CreateDirectory "$INSTDIR\data\evol\images"

  SetOutPath "$INSTDIR"
  File "${SRCDIR}\data\evol\evol.manaplus"
  SetOutPath "$INSTDIR\data\evol\images"
  File "${SRCDIR}\data\evol\images\*.png"
  SetOutPath "$INSTDIR\data\evol\icons"
  File "${SRCDIR}\data\evol\icons\*.ico"

  CreateShortCut "$SMPROGRAMS\Mana\EvolOnline.lnk" '"$INSTDIR\manaplus.exe"' '"$INSTDIR\evol.manaplus"' "$INSTDIR\manaplus.exe" 1
  CreateShortCut "$DESKTOP\EvolOnline.lnk" '"$INSTDIR\manaplus.exe"' '"$INSTDIR\evol.manaplus"' "$INSTDIR\manaplus.exe" 1
SectionEnd

Section "Translations" SecTrans
  SetOutPath "$INSTDIR"
  File /nonfatal /r "${SRCDIR}\translations"
SectionEnd

;Package descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "The core program files."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcuts} "Create game shortcuts and register extensions."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTmwMusic} "Background tmw music. (If selected the tmw music will be downloaded from the internet.)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPortable} "Portable client. (If selected client will work as portable client.)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecEvol} "Create shortcuts for Evol Online."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecEvolMusic} "Background evol music. (If selected the evol music will be downloaded from the internet.)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTrans} "Translations for the user interface. Uncheck this component to leave it in English."
!insertmacro MUI_FUNCTION_DESCRIPTION_END



Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\Mana\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\Mana\Readme.lnk" "notepad.exe" "$INSTDIR\README.txt"
  CreateShortCut "$SMPROGRAMS\Mana\FAQ.lnk" "$INSTDIR\docs\FAQ.txt"
  CreateShortCut "$SMPROGRAMS\Mana\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\manaplus.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\manaplus.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

Function un.onInit
  !insertmacro MUI_UNGETLANGUAGE
  !insertmacro MULTIUSER_UNINIT
FunctionEnd

Section Uninstall
  DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mana"

  Delete "$INSTDIR\*.*"

  Delete "$SMPROGRAMS\Mana\Uninstall.lnk"
  Delete "$DESKTOP\ManaPlus.lnk"
  Delete "$DESKTOP\ManaPlus (tests).lnk"
  Delete "$SMPROGRAMS\Mana\ManaPlus.lnk"
  Delete "$SMPROGRAMS\Mana\ManaPlus (no opengl).lnk"
  Delete "$SMPROGRAMS\Mana\ManaPlus (safemode).lnk"
  Delete "$SMPROGRAMS\Mana\ManaPlus (tests).lnk"
  Delete "$SMPROGRAMS\Mana\Website.lnk"
  Delete "$SMPROGRAMS\Mana\Readme.lnk"
  Delete "$SMPROGRAMS\Mana\FAQ.lnk"
  Delete "$SMPROGRAMS\Mana\EvolOnline.lnk"
  Delete "$DESKTOP\EvolOnline.lnk"

  RMDir "$SMPROGRAMS\Mana"

  RMDir /r "$INSTDIR\data"
  RMDir /r "$INSTDIR\docs"
  RMDir /r "$INSTDIR\translations"
  RMDir /r "$INSTDIR\updates"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey SHCTX "${PRODUCT_DIR_REGKEY}"
  ${unregisterExtension} ".manaplus" "ManaPlus brandings"
  SetAutoClose true
SectionEnd
