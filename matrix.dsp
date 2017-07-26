# Microsoft Developer Studio Project File - Name="matrix" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=matrix - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "matrix.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "matrix.mak" CFG="matrix - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "matrix - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "matrix - Win32 Release Win9x" (based on "Win32 (x86) Console Application")
!MESSAGE "matrix - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "matrix - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "_WIN32_DCOM" /D "_UNICODE" /D "UNICODE" /D "USE_MEM_MANAGER" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 MsgHook.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /out:"./matrix.exe" /pdbtype:sept
# SUBTRACT LINK32 /profile /pdb:none

!ELSEIF  "$(CFG)" == "matrix - Win32 Release Win9x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_Win9x"
# PROP BASE Intermediate_Dir "Release_Win9x"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Win9x"
# PROP Intermediate_Dir "Release_Win9x"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /Ob2 /D "NDEBUG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "_WIN32_DCOM" /D "_UNICODE" /D "UNICODE" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /Ob2 /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "_WIN32_DCOM" /D "NDEBUG" /D "WIN9X" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 MsgHook.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386 /out:"./matrix.exe"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 MsgHook.lib kernel32.lib advapi32.lib user32.lib gdi32.lib shell32.lib comdlg32.lib version.lib mpr.lib rasapi32.lib winmm.lib winspool.lib vfw32.lib secur32.lib oleacc.lib oledlg.lib sensapi.lib ole32.lib /nologo /entry:"WinMainCRTStartup" /subsystem:windows /machine:I386 /out:"./matrix.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "matrix - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /Ob2 /D "NDEBUG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "_WIN32_DCOM" /D "_UNICODE" /D "UNICODE" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /Ob2 /D "NDEBUG" /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "_WIN32_DCOM" /D "_UNICODE" /D "UNICODE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 MsgHook.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386 /out:"./matrix.exe"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 MsgHook.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386 /out:"./matrix.exe"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "matrix - Win32 Debug"
# Name "matrix - Win32 Release Win9x"
# Name "matrix - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\globals.cpp
# End Source File
# Begin Source File

SOURCE=.\matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\mmgr.cpp

!IF  "$(CFG)" == "matrix - Win32 Debug"

!ELSEIF  "$(CFG)" == "matrix - Win32 Release Win9x"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "matrix - Win32 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RegistryListenerThread.cpp
# End Source File
# Begin Source File

SOURCE=.\TopLevelListenerWindow.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\mmgr.h

!IF  "$(CFG)" == "matrix - Win32 Debug"

!ELSEIF  "$(CFG)" == "matrix - Win32 Release Win9x"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "matrix - Win32 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nommgr.h

!IF  "$(CFG)" == "matrix - Win32 Debug"

!ELSEIF  "$(CFG)" == "matrix - Win32 Release Win9x"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "matrix - Win32 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RegistryListenerThread.h
# End Source File
# Begin Source File

SOURCE=.\TopLevelListenerWindow.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\matrix.rc
# End Source File
# Begin Source File

SOURCE=.\ZMatrix.ico
# End Source File
# End Group
# End Target
# End Project
