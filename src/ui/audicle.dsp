# Microsoft Developer Studio Project File - Name="audicle" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=audicle - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "audicle.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "audicle.mak" CFG="audicle - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "audicle - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "audicle - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "audicle - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\v1" /I "..\treesynth\lib" /I "..\ui" /I "..\treesynth\ts" /I "..\..\..\chuck_dev\v2" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__PLATFORM_WIN32__" /D "__WINDOWS_DS__" /D "__CK_SNDFILE_NATIVE__" /D "__TAPS_SCRIPTING_ENABLE__" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 opengl32.lib glu32.lib glut32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dsound.lib dinput.lib dxguid.lib winmm.lib wsock32.lib libsndfile.lib /nologo /subsystem:console /machine:I386 /out:"Release/taps.exe" /libpath:"..\treesynth\lib"

!ELSEIF  "$(CFG)" == "audicle - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\v1" /I "..\treesynth\lib" /I "..\ui" /I "..\treesynth\ts" /I "..\..\..\chuck_dev\v2" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__PLATFORM_WIN32__" /D "__WINDOWS_DS__" /D "__CK_SNDFILE_NATIVE__" /D "__TAPS_SCRIPTING_ENABLE__" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dinput.lib opengl32.lib glu32.lib glut32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dsound.lib dxguid.lib winmm.lib wsock32.lib libsndfile.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/taps.exe" /pdbtype:sept /libpath:"..\treesynth\lib"

!ENDIF 

# Begin Target

# Name "audicle - Win32 Release"
# Name "audicle - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\v1\pvc.cpp
# End Source File
# Begin Source File

SOURCE=..\v1\transient.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_analysis.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_audio.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_audiofx.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_control.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_element.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_library.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_scripting.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_search.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_synthesis.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_treesynth.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\v1\pvc.h
# End Source File
# Begin Source File

SOURCE=..\v1\transient.h
# End Source File
# Begin Source File

SOURCE=.\ui_analysis.h
# End Source File
# Begin Source File

SOURCE=.\ui_audio.h
# End Source File
# Begin Source File

SOURCE=.\ui_audiofx.h
# End Source File
# Begin Source File

SOURCE=.\ui_control.h
# End Source File
# Begin Source File

SOURCE=.\ui_element.h
# End Source File
# Begin Source File

SOURCE=.\ui_library.h
# End Source File
# Begin Source File

SOURCE=.\ui_scripting.h
# End Source File
# Begin Source File

SOURCE=.\ui_search.h
# End Source File
# Begin Source File

SOURCE=.\ui_synthesis.h
# End Source File
# Begin Source File

SOURCE=.\ui_treesynth.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "birdbrain"

# PROP Default_Filter ".h .cpp"
# Begin Source File

SOURCE=..\v1\analysis.cpp
# End Source File
# Begin Source File

SOURCE=..\v1\analysis.h
# End Source File
# Begin Source File

SOURCE=..\v1\birdbrain.cpp
# End Source File
# Begin Source File

SOURCE=..\v1\birdbrain.h
# End Source File
# Begin Source File

SOURCE=..\v1\driver.cpp
# End Source File
# Begin Source File

SOURCE=..\v1\driver.h
# End Source File
# Begin Source File

SOURCE=..\v1\FeatureLibrary.cpp
# End Source File
# Begin Source File

SOURCE=..\v1\FeatureLibrary.h
# End Source File
# Begin Source File

SOURCE=..\v1\RegionComparer.cpp
# End Source File
# Begin Source File

SOURCE=..\v1\RegionComparer.h
# End Source File
# Begin Source File

SOURCE=..\v1\sceptre.cpp
# End Source File
# Begin Source File

SOURCE=..\v1\sceptre.h
# End Source File
# Begin Source File

SOURCE=..\v1\synthesis.cpp
# End Source File
# Begin Source File

SOURCE=..\v1\synthesis.h
# End Source File
# End Group
# Begin Group "audicle"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\audicle.cpp
# End Source File
# Begin Source File

SOURCE=.\audicle.h
# End Source File
# Begin Source File

SOURCE=.\audicle_def.h
# End Source File
# Begin Source File

SOURCE=.\audicle_elcidua.cpp
# End Source File
# Begin Source File

SOURCE=.\audicle_elcidua.h
# End Source File
# Begin Source File

SOURCE=.\audicle_event.cpp
# End Source File
# Begin Source File

SOURCE=.\audicle_event.h
# End Source File
# Begin Source File

SOURCE=.\audicle_face.cpp
# End Source File
# Begin Source File

SOURCE=.\audicle_face.h
# End Source File
# Begin Source File

SOURCE=.\audicle_geometry.cpp
# End Source File
# Begin Source File

SOURCE=.\audicle_geometry.h
# End Source File
# Begin Source File

SOURCE=.\audicle_gfx.cpp
# End Source File
# Begin Source File

SOURCE=.\audicle_gfx.h
# End Source File
# Begin Source File

SOURCE=.\audicle_input.cpp
# End Source File
# Begin Source File

SOURCE=.\audicle_input.h
# End Source File
# Begin Source File

SOURCE=.\audicle_main.cpp
# End Source File
# Begin Source File

SOURCE=.\audicle_text_buffer.cpp
# End Source File
# Begin Source File

SOURCE=.\audicle_text_buffer.h
# End Source File
# Begin Source File

SOURCE=.\audicle_ui_base.cpp
# End Source File
# Begin Source File

SOURCE=.\audicle_ui_base.h
# End Source File
# Begin Source File

SOURCE=.\audicle_utils.cpp
# End Source File
# Begin Source File

SOURCE=.\audicle_utils.h
# End Source File
# Begin Source File

SOURCE=.\chuck_def.h
# End Source File
# Begin Source File

SOURCE=.\rtaudio.cpp
# End Source File
# Begin Source File

SOURCE=.\rtaudio.h
# End Source File
# Begin Source File

SOURCE=.\rterror.h
# End Source File
# Begin Source File

SOURCE=.\util_thread.cpp
# End Source File
# Begin Source File

SOURCE=.\util_thread.h
# End Source File
# End Group
# Begin Group "ts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\treesynth\ts\daub.cpp
# End Source File
# Begin Source File

SOURCE=..\treesynth\ts\daub.h
# End Source File
# Begin Source File

SOURCE=..\treesynth\ts\Eliot.cpp
# End Source File
# Begin Source File

SOURCE=..\treesynth\ts\Eliot.h
# End Source File
# Begin Source File

SOURCE=..\treesynth\ts\test.cpp
# End Source File
# End Group
# Begin Group "chuck"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_absyn.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_absyn.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_bbq.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_bbq.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_compile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_compile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_def.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_dl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_dl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_emit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_emit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_errmsg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_errmsg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_frame.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_frame.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_globals.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_globals.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_instr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_instr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_lang.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_lang.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_oo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_oo.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_otf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_otf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_parse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_parse.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_scan.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_scan.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_stats.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_stats.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_symbol.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_symbol.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_table.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_table.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_type.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_type.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_ugen.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_ugen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_utils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_utils.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_vm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_vm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_win32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\chuck_win32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\digiio_rtaudio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\digiio_rtaudio.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\hidio_sdl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\hidio_sdl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\midiio_rtmidi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\midiio_rtmidi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\rtmidi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\rtmidi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ugen_filter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ugen_filter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ugen_osc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ugen_osc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ugen_stk.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ugen_stk.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ugen_xxx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ugen_xxx.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ulib_machine.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ulib_machine.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ulib_math.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ulib_math.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ulib_opsc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ulib_opsc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ulib_std.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\ulib_std.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_buffers.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_buffers.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_console.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_console.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_hid.c
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_hid.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_math.c
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_math.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_network.c
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_network.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_opsc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_opsc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_raw.c
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_raw.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_string.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_string.h
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_xforms.c
# End Source File
# Begin Source File

SOURCE=..\..\..\chuck_dev\v2\util_xforms.h
# End Source File
# End Group
# End Target
# End Project
