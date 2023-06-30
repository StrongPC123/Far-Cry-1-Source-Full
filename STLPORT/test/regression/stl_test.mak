# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

!IF "$(CFG)" == ""
CFG=stl_test - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to stl_test - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "stl_test - Win32 Release" && "$(CFG)" !=\
 "stl_test - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "stl_test.mak" CFG="stl_test - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "stl_test - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "stl_test - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "stl_test - Win32 Debug"

!IF  "$(CFG)" == "stl_test - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP BASE Cmd_Line "NMAKE /f stl_test.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "stl_test.exe"
# PROP BASE Bsc_Name "stl_test.bsc"
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# PROP Cmd_Line "NMAKE /f stl_test.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "stl_test.exe"
# PROP Bsc_Name "stl_test.bsc"
OUTDIR=.\Release
INTDIR=.\Release

ALL : 

CLEAN : 
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP BASE Cmd_Line "NMAKE /f stl_test.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "stl_test.exe"
# PROP BASE Bsc_Name "stl_test.bsc"
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# PROP Cmd_Line "NMAKE /f stl_test.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "stl_test.exe"
# PROP Bsc_Name "stl_test.bsc"
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : 

CLEAN : 
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

!ENDIF 

################################################################################
# Begin Target

# Name "stl_test - Win32 Release"
# Name "stl_test - Win32 Debug"

!IF  "$(CFG)" == "stl_test - Win32 Release"

".\stl_test.exe" : 
   CD c:\home\fbp\stl\test\os
   NMAKE /f stl_test.mak

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

".\stl_test.exe" : 
   CD c:\home\fbp\stl\test\os
   NMAKE /f stl_test.mak

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\binsert2.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\accum2.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\adjdiff0.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\adjdiff1.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\adjdiff2.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\adjfind0.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\adjfind1.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\adjfind2.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\advance.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\alg1.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\alg2.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\alg3.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\alg4.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\alg5.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bcompos1.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bcompos2.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bind1st2.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bind2nd1.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bind2nd2.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\binsert1.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\accum1.cpp

!IF  "$(CFG)" == "stl_test - Win32 Release"

!ELSEIF  "$(CFG)" == "stl_test - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
