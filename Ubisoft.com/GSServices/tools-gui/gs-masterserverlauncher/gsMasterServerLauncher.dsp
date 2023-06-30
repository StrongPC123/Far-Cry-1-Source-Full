# Microsoft Developer Studio Project File - Name="gsMasterServerLauncher" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=gsMasterServerLauncher - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gsMasterServerLauncher.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gsMasterServerLauncher.mak" CFG="gsMasterServerLauncher - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gsMasterServerLauncher - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "gsMasterServerLauncher - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/GSServices/tools-gui/gs-masterserverlauncher", MOGEAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gsMasterServerLauncher - Win32 Release"

# PROP BASE Use_MFC 5
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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /Gz /MT /W3 /GX /O2 /I "../../sdks/gs-sdk-base/include" /I "../../sdks/gs-sdk-common/include" /I "../../sdks/gs-sdk-regserver/include" /I "../../sdks/versions" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "GS_WIN32" /D "_STLP_NO_IOSTREAMS" /D "DEFVER_LIBGSSOCKET" /D "DEFVER_LIBGSUTILITY" /D "DEFVER_LIBGSCONNECT" /D "DEFVER_LIBGSREGSERVER" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x1009 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 libgssocket.lib libgsutility.lib libgsconnect.lib libgsregserver.lib libgscrypto.lib Ws2_32.lib /nologo /subsystem:windows /machine:I386 /libpath:"../../sdks/gs-sdk-http/lib_win32" /libpath:"../../sdks/gs-sdk-common/lib_win32" /libpath:"../../sdks/gs-sdk-regserver/lib_win32"

!ELSEIF  "$(CFG)" == "gsMasterServerLauncher - Win32 Debug"

# PROP BASE Use_MFC 5
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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /Gz /MTd /W3 /Gm /GX /ZI /Od /I "../../sdks/gs-sdk-base/include" /I "../../sdks/gs-sdk-common/include" /I "../../sdks/gs-sdk-regserver/include" /I "../../sdks/versions" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "GS_WIN32" /D "_STLP_NO_IOSTREAMS" /D "DEFVER_LIBGSSOCKET" /D "DEFVER_LIBGSUTILITY" /D "DEFVER_LIBGSCONNECT" /D "DEFVER_LIBGSREGSERVER" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x1009 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libgssocket_debug.lib libgsutility_debug.lib libgsconnect_debug.lib libgscrypto_debug.lib libgsregserver_debug.lib Ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"../../sdks/gs-sdk-http/lib_win32" /libpath:"../../sdks/gs-sdk-common/lib_win32" /libpath:"../../sdks/gs-sdk-regserver/lib_win32"

!ENDIF 

# Begin Target

# Name "gsMasterServerLauncher - Win32 Release"
# Name "gsMasterServerLauncher - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\gsMasterServerLauncher.cpp
# End Source File
# Begin Source File

SOURCE=.\gsMasterServerLauncher.rc
# End Source File
# Begin Source File

SOURCE=.\gsMasterServerLauncherDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Registry.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\gsMasterServerLauncher.h
# End Source File
# Begin Source File

SOURCE=.\gsMasterServerLauncherDlg.h
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
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\gsMasterServerLauncher.ico
# End Source File
# Begin Source File

SOURCE=.\res\gsMasterServerLauncher.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\Launcher.ini
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
