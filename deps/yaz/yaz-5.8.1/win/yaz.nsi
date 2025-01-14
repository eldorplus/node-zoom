; This file is part of the YAZ toolkit.
; Copyright (C) Index Data
; See the file LICENSE for details.

!include version.nsi

; Microsoft runtime CRT 
; Uncomment exactly ONE of the sections below
; 1: MSVC 6
; !define VS_RUNTIME_DLL ""
; !define VS_RUNTIME_MANIFEST ""

; 2: VS 2003
; !define VS_RUNTIME_DLL "c:\Program Files\Microsoft Visual Studio .NET 2003\SDK\v1.1\Bin\msvcr71.dll"
;!define VS_RUNTIME_MANIFEST ""

; 3: VS 2005
;!define VS_RUNTIME_DLL      "c:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcr80.dll"
;!define VS_RUNTIME_MANIFEST "c:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\Microsoft.VC80.CRT.manifest"

; 4: VS 2008
;!define VS_RUNTIME_DLL      "c:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\msvc*90.dll"
;!define VS_RUNTIME_MANIFEST "c:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\Microsoft.VC90.CRT.manifest"

; 5: VS 2013
;!define VS_RUNTIME_DLL      "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x86\Microsoft.VC120.CRT\msvc*.dll"
;!define VS_RUNTIME_MANIFEST  ""


!include "MUI.nsh"

SetCompressor bzip2

Name "YAZ"
Caption "Index Data YAZ ${VERSION} Setup"
OutFile "yaz_${VERSION}.exe"

LicenseText "You must read the following license before installing:"
LicenseData license.txt

ComponentText "This will install the YAZ Toolkit on your computer:"
InstType "Full (w/ Source)"
InstType "Lite (w/o Source)"

InstallDirRegKey HKLM "SOFTWARE\Index Data\YAZ" ""


;----------------------------
; Pages


  !insertmacro MUI_PAGE_LICENSE "license.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
; Page components
; Page directory
; Page instfiles

; UninstPage uninstConfirm
; UninstPage instfiles

;--------------------------------
;Languages
 
!insertmacro MUI_LANGUAGE "English"

;--------------------------------

Section "" ; (default section)
	SetOutPath "$INSTDIR"
	; add files / whatever that need to be installed here.
	WriteRegStr HKLM "SOFTWARE\Index Data\YAZ" "" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\YAZ" "DisplayName" "YAZ ${VERSION} (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\YAZ" "UninstallString" '"$INSTDIR\uninst.exe"'
	; write out uninstaller
	WriteUninstaller "$INSTDIR\uninst.exe"
	SetOutPath $SMPROGRAMS\YAZ
 	CreateShortCut "$SMPROGRAMS\YAZ\YAZ Program Directory.lnk" \
                 "$INSTDIR"
	WriteINIStr "$SMPROGRAMS\YAZ\YAZ Home page.url" \
              "InternetShortcut" "URL" "http://www.indexdata.com/yaz/"
	CreateShortCut "$SMPROGRAMS\YAZ\Uninstall YAZ.lnk" \
		"$INSTDIR\uninst.exe"
	SetOutPath $INSTDIR
	File LICENSE.txt
	File ..\README
	File ..\NEWS
	SetOutPath $INSTDIR
	SetOutPath $INSTDIR\ztest
	File ..\ztest\dummy-grs
	File ..\ztest\dummy-words
	SetOutPath $INSTDIR\etc
	File ..\etc\*.xml
	File ..\etc\*.xsl
	File ..\etc\pqf.properties

SectionEnd ; end of default section

Section "YAZ Runtime" YAZ_Runtime
	SectionIn 1 2
	IfFileExists "$INSTDIR\bin\yaz-ztest.exe" 0 Noservice
	ExecWait '"$INSTDIR\bin\yaz-ztest.exe" -remove'
Noservice:
	SetOutPath $INSTDIR\bin
	File "${VS_RUNTIME_DLL}"
!if "${VS_RUNTIME_MANIFEST}" != ""
	File "${VS_RUNTIME_MANIFEST}"
!endif
	File ..\bin\*.dll
	File ..\bin\*.exe
	SetOutPath $SMPROGRAMS\YAZ
 	CreateShortCut "$SMPROGRAMS\YAZ\YAZ Client.lnk" \
                 "$INSTDIR\bin\yaz-client.exe"
	SetOutPath $SMPROGRAMS\YAZ\Server
 	CreateShortCut "$SMPROGRAMS\YAZ\Server\Server on console on port 9999.lnk" \
                 "$INSTDIR\bin\yaz-ztest.exe" '-w"$INSTDIR\ztest"'
  	CreateShortCut "$SMPROGRAMS\YAZ\Server\Install Z39.50 service on port 210.lnk" \
                  "$INSTDIR\bin\yaz-ztest.exe" '-installa tcp:@:210'
 	CreateShortCut "$SMPROGRAMS\YAZ\Server\Remove Z39.50 service.lnk" \
                 "$INSTDIR\bin\yaz-ztest.exe" '-remove'
SectionEnd

Section "YAZ Development" YAZ_Development
	SectionIn 1 2
	SetOutPath $INSTDIR\include\yaz
	File ..\include\yaz\*.h
	SetOutPath $INSTDIR\lib
	File ..\lib\yaz*.lib
SectionEnd

Section "YAZ Documentation" YAZ_Documentation
	SectionIn 1 2
	SetOutPath $INSTDIR\doc
	File /nonfatal /r ..\doc\*.css
	File /nonfatal /r ..\doc\*.ent
	File /nonfatal /r ..\doc\*.html
	File /r ..\doc\*.xml
	File /r ..\doc\*.png
	File /nonfatal /r ..\doc\*.xsl
	SetOutPath $SMPROGRAMS\YAZ
	CreateShortCut "$SMPROGRAMS\YAZ\HTML Documentation.lnk" \
                 "$INSTDIR\doc\index.html"
SectionEnd

Section "YAZ Source" YAZ_Source
	SectionIn 1
	SetOutPath $INSTDIR
	File ..\IDMETA
	File /r ..\*.c
	File /r /x yaz ..\*.h
	SetOutPath $INSTDIR\util
	File ..\util\yaz-asncomp
	SetOutPath $INSTDIR\src
	File ..\src\*.y
	File ..\src\*.tcl
	File ..\src\*.csv
	File ..\src\*.asn
	File ..\src\codetables*.xml
	SetOutPath $INSTDIR\test
	File ..\test\marc*.*
	File ..\test\*.sh
	File ..\test\*.xml
	File ..\test\*.asn
	SetOutPath $INSTDIR\win
	File makefile
	File *.nsi
	File *.rc
SectionEnd

Section "YAZ4J" YAZ4J
	SectionIn 1 2
	SetOutPath $INSTDIR\bin
	File /nonfatal ..\bin\yaz4j.dll
	SetOutPath $INSTDIR\java
	File /nonfatal ..\java\yaz4j.jar
SectionEnd

; begin uninstall settings/section
UninstallText "This will uninstall YAZ ${VERSION} from your system"

Section Uninstall
; add delete commands to delete whatever files/registry keys/etc you installed here.
	Delete "$INSTDIR\uninst.exe"
	DeleteRegKey HKLM "SOFTWARE\Index Data\YAZ"
	DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\YAZ"
	ExecWait '"$INSTDIR\bin\yaz-ztest" -remove'
	RMDir /r $SMPROGRAMS\YAZ
	RMDir /r $INSTDIR
        IfFileExists $INSTDIR 0 Removed 
		MessageBox MB_OK|MB_ICONEXCLAMATION \
                 "Note: $INSTDIR could not be removed."
Removed:
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
LangString DESC_YAZ_Runtime ${LANG_ENGLISH} "YAZ runtime files needed in order for YAZ to run, such as DLLs."
LangString DESC_YAZ_Development ${LANG_ENGLISH} "Header files and import libraries required for developing software using YAZ."
LangString DESC_YAZ_Documentation ${LANG_ENGLISH} "YAZ Users' guide and reference in HTML. Describes both YAZ applications and the API."
LangString DESC_YAZ_Source ${LANG_ENGLISH} "Source code of YAZ. Required if you need to rebuild YAZ (for debugging purposes)."
LangString DESC_YAZ4J ${LANG_ENGLISH} "Java wrapper for the ZOOM API of YAZ."

;Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${YAZ_Runtime} $(DESC_YAZ_Runtime)
!insertmacro MUI_DESCRIPTION_TEXT ${YAZ_Development} $(DESC_YAZ_Development)
!insertmacro MUI_DESCRIPTION_TEXT ${YAZ_Documentation} $(DESC_YAZ_Documentation)
!insertmacro MUI_DESCRIPTION_TEXT ${YAZ_Source} $(DESC_YAZ_Source)
!insertmacro MUI_DESCRIPTION_TEXT ${YAZ4J} $(DESC_YAZ4J)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; eof
