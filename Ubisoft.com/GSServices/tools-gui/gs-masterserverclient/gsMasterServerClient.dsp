# Microsoft Developer Studio Project File - Name="gsMasterServerClient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=gsMasterServerClient - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gsMasterServerClient.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gsMasterServerClient.mak" CFG="gsMasterServerClient - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gsMasterServerClient - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "gsMasterServerClient - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/GSServices/tools-gui/gs-masterserverclient", JLGEAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gsMasterServerClient - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /Gz /MT /W3 /GX /O2 /I "../../sdks/gs-sdk-common/include" /I "../../sdks/gs-sdk-msclient/include" /I "../../sdks/gs-sdk-base/include" /I "../../sdks/versions" /D "NDEBUG" /D "DEFVERSION_LIBGSSOCKET" /D "DEFVERSION_LIBGSUTILITY" /D "DEFVERSION_LIBGSCONNECT" /D "DEFVERSION_LIBGSCLIENT" /D "DEFVERSION_LIBGSPROXYCLIENT" /D "DEFVERSION_LIBGSRESULt" /D "DEFVERSION_LIBGSMSCLIENT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "GS_WIN32" /D "_STLP_NO_IOSTREAMS" /D "DEFVER_LIBGSSOCKET" /D "DEFVER_LIBGSUTILITY" /D "DEFVER_LIBGSCONNECT" /D "DEFVER_LIBGSCLIENT" /D "DEFVER_LIBGSPROXYCLIENT" /D "DEFVER_LIBGSRESULT" /D "DEFVER_LIBGSMSCLIENT" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x1009 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 libgssocket.lib libgsutility.lib libgsconnect.lib libgscrypto.lib libgsproxyclient.lib libgsresult.lib libgsclient.lib libgsmsclient.lib Ws2_32.lib /nologo /subsystem:windows /machine:I386 /libpath:"../../sdks/gs-sdk-http/lib_win32" /libpath:"../../sdks/gs-sdk-msclient/lib_win32../../sdks/gs-sdk-result/lib_win32" /libpath:"../../sdks/gs-sdk-common/lib_win32" /libpath:"../../sdks/gs-sdk-msclient/lib_win32" /libpath:"../../sdks/gs-sdk-result/lib_win32" /libpath:"../../sdks/gs-sdk-proxyclient/lib_win32" /libpath:"../../sdks/gs-sdk-base/lib_win32"

!ELSEIF  "$(CFG)" == "gsMasterServerClient - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /Gz /MTd /W3 /Gm /GX /ZI /Od /I "../../sdks/gs-sdk-common/include" /I "../../sdks/gs-sdk-msclient/include" /I "../../sdks/gs-sdk-base/include" /I "../../sdks/versions" /D "_DEBUG" /D "DEFVER_LIBGSSOCKET" /D "DEFVER_LIBGSUTILITY" /D "DEFVER_LIBGSCONNECT" /D "DEFVER_LIBGSCLIENT" /D "DEFVER_LIBGSPROXYCLIENT" /D "DEFVER_LIBGSRESULT" /D "DEFVER_LIBGSMSCLIENT" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "GS_WIN32" /D "_STLP_NO_IOSTREAMS" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x1009 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libgssocket_debug.lib libgsutility_debug.lib libgsconnect_debug.lib libgscrypto_debug.lib libgsproxyclient_debug.lib libgsresult_debug.lib libgsclient_debug.lib libgsmsclient_debug.lib Ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"../../sdks/gs-sdk-http/lib_win32" /libpath:"../../sdks/gs-sdk-msclient/lib_win32" /libpath:"../../sdks/gs-sdk-result/lib_win32" /libpath:"../../sdks/gs-sdk-common/lib_win32" /libpath:"../../sdks/gs-sdk-proxyclient/lib_win32" /libpath:"../../sdks/gs-sdk-base/lib_win32"

!ENDIF 

# Begin Target

# Name "gsMasterServerClient - Win32 Release"
# Name "gsMasterServerClient - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\gsMasterServerClient.cpp
# End Source File
# Begin Source File

SOURCE=.\gsMasterServerClient.rc
# End Source File
# Begin Source File

SOURCE=.\gsMasterServerClientDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Registry.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\SubmitScoresdlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\gsMasterServerClient.h
# End Source File
# Begin Source File

SOURCE=.\gsMasterServerClientDlg.h
# End Source File
# Begin Source File

SOURCE=.\Registry.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SubmitScoresdlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\gsMasterServerClient.ico
# End Source File
# Begin Source File

SOURCE=.\res\gsMasterServerClient.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\MSClient.ini
# End Source File
# End Target
# End Project
