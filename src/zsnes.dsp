# Microsoft Developer Studio Project File - Name="zsnes" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=zsnes - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zsnes.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zsnes.mak" CFG="zsnes - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zsnes - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "zsnes - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zsnes - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /O3 /c
# ADD CPP /nologo /G5 /MD /W3 /Gi /Ob2 /D "NDEBUG" /D "__WIN32__" /D "WIN32" /D "_WINDOWS" /YX /FD /O3 /QxM /Qipo /QIfist /Qsox- /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 libircmt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib ddraw.lib dsound.lib dinput8.lib dxguid.lib zlib_static.lib libpng.lib /nologo /subsystem:windows /machine:I386 /out:"g:\zsnesw_586-MMX.exe"
# SUBTRACT LINK32 /pdb:none /debug

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libcmtd.lib msvcrtd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib ddraw.lib dsound.lib dinput8.lib dxguid.lib zlib_static.lib libpng.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcmt" /out:"Debug/zsnesw.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "zsnes - Win32 Release"
# Name "zsnes - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "assembly source"

# PROP Default_Filter "asm"
# Begin Source File

SOURCE=.\video\2xsaiw.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__2XSAI=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\2xsaiw.asm
InputName=2xsaiw

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__2XSAI=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\2xsaiw.asm
InputName=2xsaiw

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cfgload.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__CFGLO=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\cfgload.asm
InputName=cfgload

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__CFGLO=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\cfgload.asm
InputName=cfgload

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win\copyvwin.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__COPYV=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\win\copyvwin.asm
InputName=copyvwin

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__COPYV=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\win\copyvwin.asm
InputName=copyvwin

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dos\debug.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__DEBUG=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\dos\debug.asm
InputName=debug

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__DEBUG=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\dos\debug.asm
InputName=debug

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cpu\dma.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__DMA_A=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\cpu\dma.asm
InputName=dma

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__DMA_A=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\cpu\dma.asm
InputName=dma

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cpu\dsp.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__DSP_A=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\cpu\dsp.asm
InputName=dsp

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__DSP_A=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\cpu\dsp.asm
InputName=dsp

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chips\dsp1proc.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__DSP1P=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\chips\dsp1proc.asm
InputName=dsp1proc

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__DSP1P=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\chips\dsp1proc.asm
InputName=dsp1proc

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cpu\dspproc.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__DSPPR=".\macros.mac"	".\cpu\fir_tables.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\cpu\dspproc.asm
InputName=dspproc

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__DSPPR=".\macros.mac"	".\cpu\fir_tables.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\cpu\dspproc.asm
InputName=dspproc

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\endmem.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__ENDME=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\endmem.asm
InputName=endmem

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__ENDME=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\endmem.asm
InputName=endmem

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cpu\execute.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__EXECU=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\cpu\execute.asm
InputName=execute

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__EXECU=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\cpu\execute.asm
InputName=execute

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chips\fxemu2.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__FXEMU=".\macros.mac"	".\chips\fxemu2.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\chips\fxemu2.asm
InputName=fxemu2

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__FXEMU=".\macros.mac"	".\chips\fxemu2.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\chips\fxemu2.asm
InputName=fxemu2

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chips\fxemu2b.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__FXEMU2=".\macros.mac"	".\chips\fxemu2.mac"	".\chips\fxemu2b.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\chips\fxemu2b.asm
InputName=fxemu2b

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__FXEMU2=".\macros.mac"	".\chips\fxemu2.mac"	".\chips\fxemu2b.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\chips\fxemu2b.asm
InputName=fxemu2b

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chips\fxemu2c.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__FXEMU2C=".\macros.mac"	".\chips\fxemu2.mac"	".\chips\fxemu2b.mac"	".\chips\fxemu2c.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\chips\fxemu2c.asm
InputName=fxemu2c

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__FXEMU2C=".\macros.mac"	".\chips\fxemu2.mac"	".\chips\fxemu2b.mac"	".\chips\fxemu2c.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\chips\fxemu2c.asm
InputName=fxemu2c

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chips\fxtable.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__FXTAB=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\chips\fxtable.asm
InputName=fxtable

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__FXTAB=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\chips\fxtable.asm
InputName=fxtable

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dos\gppro.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__GPPRO=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\dos\gppro.asm
InputName=gppro

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__GPPRO=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\dos\gppro.asm
InputName=gppro

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\gui.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__GUI_A=".\macros.mac"	".\gui\guitools.inc"	".\gui\guimisc.inc"	".\gui\guimouse.inc"	".\gui\guiwindp.inc"	".\gui\guinetpl.inc"	".\gui\guikeys.inc"	".\gui\guicheat.inc"	".\gui\guicombo.inc"	".\gui\guiload.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\gui\gui.asm
InputName=gui

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__GUI_A=".\macros.mac"	".\gui\guitools.inc"	".\gui\guimisc.inc"	".\gui\guimouse.inc"	".\gui\guiwindp.inc"	".\gui\guinetpl.inc"	".\gui\guikeys.inc"	".\gui\guicheat.inc"	".\gui\guicombo.inc"	".\gui\guiload.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\gui\gui.asm
InputName=gui

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\init.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__INIT_=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\init.asm
InputName=init

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__INIT_=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\init.asm
InputName=init

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dos\initvid.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__INITV=".\macros.mac"	".\dos\vga.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\dos\initvid.asm
InputName=initvid

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__INITV=".\macros.mac"	".\dos\vga.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\dos\initvid.asm
InputName=initvid

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cpu\irq.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__IRQ_A=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\cpu\irq.asm
InputName=irq

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__IRQ_A=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\cpu\irq.asm
InputName=irq

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dos\joy.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__JOY_A=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\dos\joy.asm
InputName=joy

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__JOY_A=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\dos\joy.asm
InputName=joy

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\m716text.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__M716T=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\m716text.asm
InputName=m716text

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__M716T=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\m716text.asm
InputName=m716text

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\makev16b.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__MAKEV=".\macros.mac"	".\video\vidmacro.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\makev16b.asm
InputName=makev16b

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__MAKEV=".\macros.mac"	".\video\vidmacro.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\makev16b.asm
InputName=makev16b

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\makev16t.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__MAKEV1=".\macros.mac"	".\video\vidmacro.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\makev16t.asm
InputName=makev16t

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__MAKEV1=".\macros.mac"	".\video\vidmacro.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\makev16t.asm
InputName=makev16t

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\makevid.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__MAKEVI=".\macros.mac"	".\video\vidmacro.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\makevid.asm
InputName=makevid

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__MAKEVI=".\macros.mac"	".\video\vidmacro.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\makevid.asm
InputName=makevid

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cpu\memory.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__MEMOR=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\cpu\memory.asm
InputName=memory

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__MEMOR=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\cpu\memory.asm
InputName=memory

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\menu.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__MENU_=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\gui\menu.asm
InputName=menu

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__MENU_=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\gui\menu.asm
InputName=menu

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\mode7.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__MODE7=".\macros.mac"	".\video\mode7.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\mode7.asm
InputName=mode7

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__MODE7=".\macros.mac"	".\video\mode7.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\mode7.asm
InputName=mode7

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\mode716.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__MODE71=".\macros.mac"	".\video\mode716.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\mode716.asm
InputName=mode716

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__MODE71=".\macros.mac"	".\video\mode716.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\mode716.asm
InputName=mode716

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\mode716b.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__MODE716=".\macros.mac"	".\video\mode7.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\mode716b.asm
InputName=mode716b

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__MODE716=".\macros.mac"	".\video\mode7.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\mode716b.asm
InputName=mode716b

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\mode716d.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__MODE716D=".\macros.mac"	".\video\mode7.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\mode716d.asm
InputName=mode716d

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__MODE716D=".\macros.mac"	".\video\mode7.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\mode716d.asm
InputName=mode716d

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\mode716e.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__MODE716E=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\mode716e.asm
InputName=mode716e

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__MODE716E=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\mode716e.asm
InputName=mode716e

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\mode716t.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__MODE716T=".\macros.mac"	".\video\mode7.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\mode716t.asm
InputName=mode716t

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__MODE716T=".\macros.mac"	".\video\mode7.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\mode716t.asm
InputName=mode716t

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\mode7ext.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__MODE7E=".\macros.mac"	".\video\mode7.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\mode7ext.asm
InputName=mode7ext

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__MODE7E=".\macros.mac"	".\video\mode7.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\mode7ext.asm
InputName=mode7ext

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\mv16tms.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__MV16T=".\macros.mac"	".\video\vidmacro.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\mv16tms.asm
InputName=mv16tms

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__MV16T=".\macros.mac"	".\video\vidmacro.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\mv16tms.asm
InputName=mv16tms

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\newg162.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__NEWG1=".\macros.mac"	".\video\vidmacro.mac"	".\video\newg162.mac"	".\video\newgfx16.mac"	".\video\newg16wn.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\newg162.asm
InputName=newg162

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__NEWG1=".\macros.mac"	".\video\vidmacro.mac"	".\video\newg162.mac"	".\video\newgfx16.mac"	".\video\newg16wn.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\newg162.asm
InputName=newg162

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\newgfx.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__NEWGF=".\macros.mac"	".\video\vidmacro.mac"	".\video\newgfx2.mac"	".\video\newgfx.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\newgfx.asm
InputName=newgfx

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__NEWGF=".\macros.mac"	".\video\vidmacro.mac"	".\video\newgfx2.mac"	".\video\newgfx.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\newgfx.asm
InputName=newgfx

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\newgfx16.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__NEWGFX=".\macros.mac"	".\video\vidmacro.mac"	".\video\newgfx16.mac"	".\video\newg162.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\newgfx16.asm
InputName=newgfx16

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__NEWGFX=".\macros.mac"	".\video\vidmacro.mac"	".\video\newgfx16.mac"	".\video\newg162.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\newgfx16.asm
InputName=newgfx16

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\newgfx2.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__NEWGFX2=".\macros.mac"	".\video\vidmacro.mac"	".\video\newgfx2.mac"	".\video\newgfx.mac"	".\video\newgfxwn.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\newgfx2.asm
InputName=newgfx2

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__NEWGFX2=".\macros.mac"	".\video\vidmacro.mac"	".\video\newgfx2.mac"	".\video\newgfx.mac"	".\video\newgfxwn.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\newgfx2.asm
InputName=newgfx2

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video\procvid.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__PROCV=".\macros.mac"	".\video\2xsaimmx.inc"	".\video\copyvid.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\video\procvid.asm
InputName=procvid

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__PROCV=".\macros.mac"	".\video\2xsaimmx.inc"	".\video\copyvid.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\video\procvid.asm
InputName=procvid

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chips\sa1proc.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__SA1PR=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\chips\sa1proc.asm
InputName=sa1proc

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__SA1PR=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\chips\sa1proc.asm
InputName=sa1proc

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chips\sa1regs.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__SA1RE=".\macros.mac"	".\cpu\regs.mac"	".\cpu\regsw.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\chips\sa1regs.asm
InputName=sa1regs

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__SA1RE=".\macros.mac"	".\cpu\regs.mac"	".\cpu\regsw.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\chips\sa1regs.asm
InputName=sa1regs

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chips\sfxproc.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__SFXPR=".\macros.mac"	".\cpu\regs.mac"	".\cpu\regsw.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\chips\sfxproc.asm
InputName=sfxproc

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__SFXPR=".\macros.mac"	".\cpu\regs.mac"	".\cpu\regsw.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\chips\sfxproc.asm
InputName=sfxproc

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cpu\spc700.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__SPC70=".\macros.mac"	".\cpu\regsw.mac"	".\cpu\spcdef.inc"	".\cpu\spcaddr.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\cpu\spc700.asm
InputName=spc700

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__SPC70=".\macros.mac"	".\cpu\regsw.mac"	".\cpu\spcdef.inc"	".\cpu\spcaddr.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\cpu\spc700.asm
InputName=spc700

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cpu\stable.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__STABL=".\macros.mac"	".\cpu\s65816d.inc"	".\cpu\saddress.inc"	".\cpu\saddrni.inc"	".\cpu\se65816.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\cpu\stable.asm
InputName=stable

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__STABL=".\macros.mac"	".\cpu\s65816d.inc"	".\cpu\saddress.inc"	".\cpu\saddrni.inc"	".\cpu\se65816.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\cpu\stable.asm
InputName=stable

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dos\sw.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__SW_AS=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\dos\sw.asm
InputName=sw

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__SW_AS=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\dos\sw.asm
InputName=sw

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cpu\table.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__TABLE=".\macros.mac"	".\cpu\65816d.inc"	".\cpu\address.inc"	".\cpu\addrni.inc"	".\cpu\e65816.inc"	".\cpu\regs.mac"	".\cpu\regsw.mac"	".\cpu\regs.inc"	".\cpu\regsw.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\cpu\table.asm
InputName=table

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__TABLE=".\macros.mac"	".\cpu\65816d.inc"	".\cpu\address.inc"	".\cpu\addrni.inc"	".\cpu\e65816.inc"	".\cpu\regs.mac"	".\cpu\regsw.mac"	".\cpu\regs.inc"	".\cpu\regsw.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\cpu\table.asm
InputName=table

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cpu\tableb.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__TABLEB=".\macros.mac"	".\cpu\65816db.inc"	".\cpu\address.inc"	".\cpu\addrni.inc"	".\cpu\e65816b.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\cpu\tableb.asm
InputName=tableb

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__TABLEB=".\macros.mac"	".\cpu\65816db.inc"	".\cpu\address.inc"	".\cpu\addrni.inc"	".\cpu\e65816b.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\cpu\tableb.asm
InputName=tableb

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cpu\tablec.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__TABLEC=".\macros.mac"	".\cpu\65816dc.inc"	".\cpu\address.inc"	".\cpu\addrni.inc"	".\cpu\e65816c.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\cpu\tablec.asm
InputName=tablec

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__TABLEC=".\macros.mac"	".\cpu\65816dc.inc"	".\cpu\address.inc"	".\cpu\addrni.inc"	".\cpu\e65816c.inc"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\cpu\tablec.asm
InputName=tablec

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__UI_AS=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\ui.asm
InputName=ui

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__UI_AS=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\ui.asm
InputName=ui

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vcache.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__VCACH=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\vcache.asm
InputName=vcache

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__VCACH=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\vcache.asm
InputName=vcache

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dos\vesa12.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__VESA1=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\dos\vesa12.asm
InputName=vesa12

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__VESA1=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\dos\vesa12.asm
InputName=vesa12

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dos\vesa2.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__VESA2=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\dos\vesa2.asm
InputName=vesa2

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__VESA2=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\dos\vesa2.asm
InputName=vesa2

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win\winintrf.asm

!IF  "$(CFG)" == "zsnes - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__WININ=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Release
InputPath=.\win\winintrf.asm
InputName=winintrf

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "zsnes - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__WININ=".\macros.mac"	
# Begin Custom Build - Assembling $(InputPath)
IntDir=.\Debug
InputPath=.\win\winintrf.asm
InputName=winintrf

"$(INTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -f win32 -D__WIN32__ -o $(INTDIR)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\burn.c
# End Source File
# Begin Source File

SOURCE=.\chips\dsp1emu.c
# End Source File
# Begin Source File

SOURCE=.\fixsin.c
# End Source File
# Begin Source File

SOURCE=.\smoke.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\zip\unzip.c
# End Source File
# Begin Source File

SOURCE=.\water.c
# End Source File
# Begin Source File

SOURCE=.\win\winlink.cpp
# End Source File
# Begin Source File

SOURCE=.\win\zfilew.c
# End Source File
# Begin Source File

SOURCE=.\win\zloaderw.c
# End Source File
# Begin Source File

SOURCE=.\zip\zpng.c
# End Source File
# Begin Source File

SOURCE=.\win\ztcp.c
# End Source File
# Begin Source File

SOURCE=.\zip\zzip.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\fixsin.h
# End Source File
# Begin Source File

SOURCE=.\zip\png.h
# End Source File
# Begin Source File

SOURCE=.\zip\pngconf.h
# End Source File
# Begin Source File

SOURCE=.\win\resource.h
# End Source File
# Begin Source File

SOURCE=.\zip\unzip.h
# End Source File
# Begin Source File

SOURCE=.\zip\zconf.h
# End Source File
# Begin Source File

SOURCE=.\zip\zlib.h
# End Source File
# Begin Source File

SOURCE=.\zip\zpng.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\win\zsnes.ico
# End Source File
# Begin Source File

SOURCE=.\win\zsnes.rc
# End Source File
# End Group
# End Target
# End Project
