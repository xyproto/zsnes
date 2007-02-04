#Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
#
#http://www.zsnes.com
#http://sourceforge.net/projects/zsnes
#https://zsnes.bountysource.com
#
#This program is free software; you can redistribute it and/or
#modify it under the terms of the GNU General Public License
#version 2 as published by the Free Software Foundation.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program; if not, write to the Free Software
#Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#Parameters:
#
# PLATFORM=platform_name  Chose target platform and how you're creating it.
#                         Valid options are:
#                         dos, dos-cross, msvc, win32, win32-cross, win32-unix-shell
#
# RELEASEGRADE=yes        for high assembly optimization for releases, will take at least half an hour.
# clean                   cleans object and executable files.
#
# --Not for MSVC--
# DEBUG=yes               for debugging with GDB.
# CPU=cpu-type            optimize for a particular CPU, find a list of cpu types in the GCC manual.


ENV=$(PLATFORM)
ifneq (${ENV},) #end if is at the bottom of the file


CHIP_D=chips
CPU_D=cpu
DOS_D=dos
EFF_D=effects
GUI_D=gui
JMA_D=jma
MMLIB_D=mmlib
NET_D=net
VID_D=video
NTSC_D=video/snes_ntsc
WIN_D=win
ZIP_D=zip

DELETECOMMAND=rm -f
DELPRE=rm -f
DELSUF=
SLASH=/
PSR=parsegen.exe
ASM=nasm
ASMOPT=-O1
ASMFLAGSORIG=
COPT=-O3
CPUOPT=i586
ifneq ($(CPU),)
CPUOPT=$(CPU)
endif
CFLAGSORIG=-march=${CPUOPT} -ffast-math -fomit-frame-pointer -fno-unroll-loops -Wall -Wno-unused -I.
LIBSORIG=-lz -lpng
DRESOBJ=${WIN_D}/zsnes.obj
CROSS=no
STRIP=-Ws -s


ifeq (${ENV},dos)
  CC = gcc
  CPPC = gpp
  OS=__MSDOS__
endif

ifeq (${ENV},win32-unix-shell)
  ENV=win32
  CROSS=yes
endif

ifeq (${ENV},win32)
  CC = gcc
  CPPC = g++
  WINDRES = windres
  OS=__WIN32__
  OBJFIX=objfix.exe
endif

ifeq (${ENV},msvc)
  CFLAGSORIG=/nologo /Ox /G6 /c /EHsc
  MSVCLIBS=zlib.lib libpng.lib wsock32.lib user32.lib gdi32.lib shell32.lib winmm.lib dinput8.lib dxguid.lib
  DRESOBJ=${WIN_D}/zsnes.res
  OS=__WIN32__
endif

ifeq (${ENV},dos-cross)
  CC = i586-msdosdjgpp-gcc
  CPPC = i586-msdosdjgpp-g++
  OS=__MSDOS__
  CROSS=yes
endif

ifeq (${ENV},win32-cross)
  CC = i586-mingw32-gcc
  CPPC = i586-mingw32-g++
  WINDRES = i586-mingw32-windres
  OS=__WIN32__
  CROSS=yes
  OBJFIX=objfix
endif

ifeq (${CROSS},no)
  DELETECOMMAND=del
  ifeq (${OS},__WIN32__)
    DELPRE=@for %%n in (znotfile.nt,
    DELSUF=) do if exist %%n del %%n
  else
    DELPRE=@for %n in (znotfile.nt,
    DELSUF=) do if exist %n del %n
  endif
  SLASH=\${BLAHBLAHBLAH}
  SEP=,
endif

ifeq (${OS},__MSDOS__)
  EXE=zsnes.exe
  FILEFORMAT=coff
  LIBS=${LIBSORIG} -lm
  OE=.do
endif

ifeq (${OS},__WIN32__)
  EXE=zsnesw.exe
  FILEFORMAT=win32
  LIBS=${LIBSORIG} -ldxguid -ldinput8 -lwsock32 -luser32 -lgdi32 -lshell32 -lwinmm -mwindows
  OE=.obj
endif

ifeq (${DEBUG},yes)
  ifneq (${ENV},msvc)
    CFLAGSORIG=-fno-omit-frame-pointer -ggdb3 -DDEBUG
    COPT=-O0
    ASMFLAGSORIG=-DDEBUG -g
    EXE=zsnesd.exe
    STRIP=
  endif
endif

ifeq (${RELEASE},yes)
  CFLAGSORIG+= -D__RELEASE__
  ASMFLAGSORIG+= -D__RELEASE__
  RELEASEGRADE=yes
endif

ifeq (${RELEASEGRADE},yes)
  ASMOPT=-O9999
endif

ifeq (${DEBUGGER},no)
  CFLAGSORIG+= -DNO_DEBUGGER
  ASMFLAGSORIG+= -DNO_DEBUGGER
endif

LINK=${CPPC}

ifeq (${JMA},no)
  CFLAGSORIG+= -DNO_JMA
  ASMFLAGSORIG+= -DNO_JMA
  ifeq (${OS},__MSDOS__)
    LINK=${CC}
  endif
endif

CFLAGS=${CFLAGSORIG} -D${OS}
ASMFLAGS=${ASMFLAGSORIG} -f ${FILEFORMAT} -D${OS}


CHIPSOBJ=${CHIP_D}/dsp1emu${OE} ${CHIP_D}/fxemu2${OE} ${CHIP_D}/sfxproc${OE}\
	${CHIP_D}/fxemu2b${OE} ${CHIP_D}/fxemu2c${OE} ${CHIP_D}/fxtable${OE}\
	${CHIP_D}/sa1proc${OE} ${CHIP_D}/sa1regs${OE} ${CHIP_D}/dsp1proc${OE}\
	${CHIP_D}/st10proc${OE} ${CHIP_D}/seta10${OE} ${CHIP_D}/dsp2proc${OE}\
	${CHIP_D}/sdd1emu${OE} ${CHIP_D}/c4emu${OE} ${CHIP_D}/dsp4proc${OE}\
	${CHIP_D}/dsp4emu${OE} ${CHIP_D}/dsp3proc${OE} ${CHIP_D}/dsp3emu${OE}\
	${CHIP_D}/obc1emu${OE} ${CHIP_D}/obc1proc${OE} ${CHIP_D}/c4proc${OE}\
	${CHIP_D}/7110proc${OE} ${CHIP_D}/st11proc${OE} ${CHIP_D}/seta11${OE}

CPUOBJ=${CPU_D}/dma${OE} ${CPU_D}/dsp${OE} ${CPU_D}/dspproc${OE}\
	${CPU_D}/execute${OE} ${CPU_D}/executec${OE} ${CPU_D}/irq${OE}\
	${CPU_D}/memory${OE} ${CPU_D}/memtable${OE} ${CPU_D}/spc700${OE}\
	${CPU_D}/stable${OE} ${CPU_D}/table${OE}\
	${CPU_D}/tablec${OE}

EFFECTSOBJ=${EFF_D}/burn${OE} ${EFF_D}/water${OE} ${EFF_D}/smoke${OE}

GUIOBJ=${GUI_D}/gui${OE} ${GUI_D}/guifuncs${OE} ${GUI_D}/menu${OE}

ifneq (${JMA},no)
JMAOBJ=${JMA_D}/7zlzma${OE} ${JMA_D}/crc32${OE} ${JMA_D}/iiostrm${OE}\
	${JMA_D}/inbyte${OE} ${JMA_D}/jma${OE} ${JMA_D}/lzma${OE} ${JMA_D}/lzmadec${OE}\
	${JMA_D}/winout${OE} ${JMA_D}/zsnesjma${OE}
endif

MAINOBJ=endmem${OE} init${OE} initc${OE} patch${OE}\
	ui${OE} vcache${OE} version${OE} zmovie${OE} zstate${OE} zloader${OE}\
	cfg${OE} md${OE} input${OE} zpath${OE} ztimec${OE} ztime${OE}

ifneq (${DEBUGGER},no)
  MAINOBJ+= debugger${OE} debugasm${OE}
  ifeq (${OS},__MSDOS__)
    LIBS+= -lpdcur
  else
    LIBS+= -lpdcurses
    MSVCLIBS+= pdcurses.lib
  endif
endif

NETOBJ=
#${NET_D}/ztcp${OE}

VIDEOBJ=${VID_D}/makev16b${OE} ${VID_D}/makev16t${OE} ${VID_D}/makevid${OE}\
	${VID_D}/mode716${OE} ${VID_D}/mode716b${OE} ${VID_D}/mode716d${OE}\
	${VID_D}/mode716e${OE} ${VID_D}/mode716t${OE}\
	${VID_D}/mv16tms${OE} ${VID_D}/newg162${OE}\
	${VID_D}/newgfx16${OE} ${VID_D}/newgfx${OE}\
	${VID_D}/m716text${OE} ${VID_D}/procvid${OE} ${VID_D}/procvidc${OE}\
	${VID_D}/2xsaiw${OE}

ZIPOBJ=${ZIP_D}/unzip${OE} ${ZIP_D}/zpng${OE}

OBJS=${CHIPSOBJ} ${CPUOBJ} ${GUIOBJ} ${VIDEOBJ} ${MAINOBJ} ${ZIPOBJ} ${EFFECTSOBJ} ${JMAOBJ}

ifeq (${OS},__MSDOS__)
DOSOBJ=${DOS_D}/dosintrf${OE} ${DOS_D}/gppro${OE} ${DOS_D}/initvid${OE}\
	${DOS_D}/sound${OE} ${DOS_D}/sw${OE} ${DOS_D}/vesa12${OE}\
	${DOS_D}/lib${OE} ${DOS_D}/joy${OE} ${DOS_D}/vesa2${OE}

DOSVIDOBJ=${VID_D}/mode7ext${OE} ${VID_D}/newgfx2${OE} ${VID_D}/mode7${OE}
OBJS+=${DOSOBJ} ${DOSVIDOBJ}
endif

ifeq (${OS},__WIN32__)
WINOBJ=${DRESOBJ}\
	${WIN_D}/winintrf${OE} ${WIN_D}/winlink${OE} ${WIN_D}/lib${OE}\
	${WIN_D}/safelib${OE} ${MMLIB_D}/mm${OE} ${MMLIB_D}/windows${OE}

WINVIDOBJ=${VID_D}/sw_draw${OE} ${VID_D}/hq2x16${OE} ${VID_D}/hq2x32${OE}\
	${VID_D}/hq3x16${OE} ${VID_D}/hq3x32${OE}\
	${VID_D}/hq4x16${OE} ${VID_D}/hq4x32${OE} ${VID_D}/copyvwin${OE}\
	${VID_D}/ntsc${OE}

OBJS+=${WINOBJ} ${WINVIDOBJ} ${NETOBJ}
endif

DELETEOBJS=${OBJS}

.SUFFIXES: .c .cpp .asm .psr

ifneq (${ENV},msvc)
%${OE}: %.c
	${CC} ${COPT} ${CFLAGS} -o $@ -c $<

%${OE}: %.cpp
	${CPPC} ${COPT} ${CFLAGS} -o $@ -c $<

%${OE}: %.asm
	${ASM} ${ASMOPT} ${ASMFLAGS} -o $@ $<
ifeq (${OS},__WIN32__)
	objfix $@
endif
else
%.obj : %.c
	cl ${CFLAGS} /Fo$@ $<

%.obj: %.cpp
	cl ${CFLAGS} /Fo$@ $<

%.obj: %.asm
	${ASM} ${ASMOPT} ${ASMFLAGS} -o $@ $<
endif

%${OE}: %.psr
	${PSR} -D${OS} -cheader $(patsubst %${OE},%.h,$@) -fname $(patsubst %${OE},%,$@) temppsr.c $<
ifneq (${ENV},msvc)
	${CC} ${CFLAGS} -O1 -o $@ -c temppsr.c
else
	cl ${CFLAGS} /Fo$@ temppsr.c
endif
	${DELETECOMMAND} temppsr.c

ALL: zsnes
	${DELETECOMMAND} version${OE}

zsnes: ${PSR} ${OBJFIX} ${OBJS}
ifneq (${ENV},msvc)
	${LINK} ${STRIP} -o ${EXE} ${OBJS} ${LIBS}
else
	 @echo /Fezsnesw.exe *.obj ${CPU_D}\*.obj ${VID_D}\*.obj ${CHIP_D}\*.obj ${EFF_D}\*.obj ${DOS_D}\*.obj ${WIN_D}\*.obj ${GUI_D}\*.obj > link.vc
	 @echo ${ZIP_D}\*.obj ${JMA_D}\*.obj ${NET_D}\*.obj ${MMLIB_D}\*.obj >> link.vc
	 @echo ${MSVCLIBS} >> link.vc
	 cl /nologo @link.vc ${WIN_D}/zsnes.res /link
endif

cfg${OE}: cfg.psr ${PSR}
debugasm${OE}: debugasm.asm macros.mac
debugger${OE}: debugger.c asm_call.h zpath.h ${JMA_D}/zsnesjma.h \
	${ZIP_D}/zunzip.h
endmem${OE}: endmem.asm macros.mac
init${OE}: init.asm macros.mac
initc${OE}: initc.c asm_call.h cfg${OE} input${OE} numconv.h zpath.h ${CPU_D}/memtable.h \
	${JMA_D}/zsnesjma.h ${ZIP_D}/zunzip.h
input${OE}: input.psr ${PSR}
md${OE}: md.psr ${PSR}
patch${OE}: patch.c zpath.h ${JMA_D}/zsnesjma.h ${ZIP_D}/zunzip.h
ui${OE}: ui.c asm_call.h cfg${OE} input${OE} zpath.h ${JMA_D}/zsnesjma.h \
	${MMLIB_D}/mm.h ${ZIP_D}/zunzip.h
vcache${OE}: vcache.asm macros.mac
version${OE}: version.c
zloader${OE}: zloader.c asm_call.h cfg${OE} input${OE} zloader.h zpath.h \
	${JMA_D}/zsnesjma.h ${ZIP_D}/zunzip.h
zmovie${OE}: zmovie.c asm_call.h cfg${OE} gblvars.h input${OE} md${OE} numconv.h zmovie.h zpath.h \
	${JMA_D}/zsnesjma.h ${WIN_D}/lib.h ${WIN_D}/safelib.h ${ZIP_D}/zunzip.h
zpath${OE}: zpath.c cfg${OE} zpath.h ${DOS_D}/lib.h ${JMA_D}/zsnesjma.h \
	${WIN_D}/lib.h ${WIN_D}/safelib.h ${ZIP_D}/zunzip.h
zstate${OE}: zstate.c asm_call.h cfg${OE} gblvars.h input${OE} numconv.h zmovie.h zpath.h \
	${CHIP_D}/dsp4emu.h ${JMA_D}/zsnesjma.h ${ZIP_D}/zunzip.h
ztime${OE}: ztime.asm macros.mac
ztimec${OE}: ztimec.c

${CHIP_D}/7110proc${OE}: ${CHIP_D}/7110proc.asm macros.mac \
	${CPU_D}/regs.mac ${CPU_D}/regsw.mac
${CHIP_D}/c4emu${OE}: ${CHIP_D}/c4emu.c
${CHIP_D}/c4proc${OE}: ${CHIP_D}/c4proc.asm macros.mac
${CHIP_D}/dsp1emu${OE}: ${CHIP_D}/dsp1emu.c
${CHIP_D}/dsp1proc${OE}: ${CHIP_D}/dsp1proc.asm macros.mac
${CHIP_D}/dsp2proc${OE}: ${CHIP_D}/dsp2proc.asm macros.mac
${CHIP_D}/dsp3emu${OE}: ${CHIP_D}/dsp3emu.c
${CHIP_D}/dsp3proc${OE}: ${CHIP_D}/dsp3proc.asm macros.mac
${CHIP_D}/dsp4emu${OE}: ${CHIP_D}/dsp4emu.c ${CHIP_D}/dsp4emu.h
${CHIP_D}/dsp4proc${OE}: ${CHIP_D}/dsp4proc.asm macros.mac
${CHIP_D}/fxemu2${OE}: ${CHIP_D}/fxemu2.asm macros.mac ${CHIP_D}/fxemu2.mac
${CHIP_D}/fxemu2b${OE}: ${CHIP_D}/fxemu2b.asm macros.mac \
	${CHIP_D}/fxemu2.mac ${CHIP_D}/fxemu2b.mac
${CHIP_D}/fxemu2c${OE}: ${CHIP_D}/fxemu2c.asm macros.mac \
	${CHIP_D}/fxemu2.mac ${CHIP_D}/fxemu2c.mac
${CHIP_D}/fxtable${OE}: ${CHIP_D}/fxtable.asm macros.mac
${CHIP_D}/obc1emu${OE}: ${CHIP_D}/obc1emu.c
${CHIP_D}/obc1proc${OE}: ${CHIP_D}/obc1proc.asm macros.mac
${CHIP_D}/sa1proc${OE}: ${CHIP_D}/sa1proc.asm macros.mac
${CHIP_D}/sa1regs${OE}: ${CHIP_D}/sa1regs.asm macros.mac ${CPU_D}/regs.mac \
	${CPU_D}/regsw.mac
${CHIP_D}/sdd1emu${OE}: ${CHIP_D}/sdd1emu.c
${CHIP_D}/seta10${OE}: ${CHIP_D}/seta10.c
${CHIP_D}/seta11${OE}: ${CHIP_D}/seta11.c
${CHIP_D}/sfxproc${OE}: ${CHIP_D}/sfxproc.asm macros.mac ${CPU_D}/regs.mac \
	${CPU_D}/regsw.mac
${CHIP_D}/st10proc${OE}: ${CHIP_D}/st10proc.asm macros.mac
${CHIP_D}/st11proc${OE}: ${CHIP_D}/st11proc.asm macros.mac

${CPU_D}/dma${OE}: ${CPU_D}/dma.asm macros.mac
#${CPU_D}/dsp${OE}: ${CPU_D}/dsp.asm macros.mac
#${CPU_D}/dspproc${OE}: ${CPU_D}/dspproc.asm macros.mac ${CPU_D}/firtable.inc
${CPU_D}/spc/dsp${OE}: ${CPU_D}/spc/dsp.cpp ${CPU_D}/spc/b_common.h \
	${CPU_D}/spc/b_config.h ${CPU_D}/spc/b_endian.h ${CPU_D}/spc/b_source.h \
	${CPU_D}/spc/dsp.h
${CPU_D}/spc/dspbind${OE}: ${CPU_D}/spc/dspbind.cpp ${CPU_D}/spc/b_common.h \
	${CPU_D}/spc/b_config.h ${CPU_D}/spc/dsp.h ${CPU_D}/spc/dspbind.h
${CPU_D}/execute${OE}: ${CPU_D}/execute.asm macros.mac
${CPU_D}/executec${OE}: ${CPU_D}/executec.c gblvars.h
${CPU_D}/irq${OE}: ${CPU_D}/irq.asm macros.mac
${CPU_D}/memory${OE}: ${CPU_D}/memory.asm macros.mac
${CPU_D}/memtable${OE}: ${CPU_D}/memtable.c gblvars.h ${CPU_D}/memtable.h
${CPU_D}/spc700${OE}: ${CPU_D}/spc700.asm macros.mac ${CPU_D}/regsw.mac \
	${CPU_D}/spcaddr.inc ${CPU_D}/spcdef.inc
${CPU_D}/stable${OE}: ${CPU_D}/stable.asm macros.mac ${CPU_D}/s65816d.inc \
	${CPU_D}/saddress.inc ${CPU_D}/saddrni.inc ${CPU_D}/se65816.inc
${CPU_D}/table${OE}: ${CPU_D}/table.asm macros.mac ${CPU_D}/65816d.inc \
	${CPU_D}/address.inc ${CPU_D}/addrni.inc ${CPU_D}/e65816.inc \
	${CPU_D}/regs.inc ${CPU_D}/regs.mac ${CPU_D}/regsw.inc \
	${CPU_D}/regsw.mac
${CPU_D}/tablec${OE}: ${CPU_D}/tablec.asm macros.mac ${CPU_D}/65816dc.inc \
	${CPU_D}/address.inc ${CPU_D}/addrni.inc ${CPU_D}/e65816c.inc

${DOS_D}/dosintrf${OE}: ${DOS_D}/dosintrf.asm macros.mac
${DOS_D}/gppro${OE}: ${DOS_D}/gppro.asm macros.mac
${DOS_D}/initvid${OE}: ${DOS_D}/initvid.asm macros.mac ${DOS_D}/vga.inc
${DOS_D}/joy${OE}: ${DOS_D}/joy.asm macros.mac
${DOS_D}/lib${OE}: ${DOS_D}/lib.c
${DOS_D}/sound${OE}: ${DOS_D}/sound.asm macros.mac
${DOS_D}/sw${OE}: ${DOS_D}/sw.asm macros.mac
${DOS_D}/vesa12${OE}: ${DOS_D}/vesa12.asm macros.mac
${DOS_D}/vesa2${OE}: ${DOS_D}/vesa2.asm macros.mac

${EFF_D}/burn${OE}: ${EFF_D}/burn.c
${EFF_D}/smoke${OE}: ${EFF_D}/smoke.c
${EFF_D}/water${OE}: ${EFF_D}/water.c ${EFF_D}/fixsin.h

${GUI_D}/gui${OE}: ${GUI_D}/gui.asm macros.mac ${GUI_D}/guicheat.inc \
	${GUI_D}/guicombo.inc ${GUI_D}/guikeys.inc ${GUI_D}/guimisc.inc \
	${GUI_D}/guimouse.inc ${GUI_D}/guitools.inc ${GUI_D}/guiwindp.inc
${GUI_D}/guifuncs${OE}: ${GUI_D}/guifuncs.c asm_call.h cfg${OE} input${OE} md${OE} \
	zloader.h zpath.h ${DOS_D}/lib.h ${JMA_D}/zsnesjma.h ${WIN_D}/lib.h \
	${ZIP_D}/zunzip.h
${GUI_D}/menu${OE}: ${GUI_D}/menu.asm macros.mac

${JMA_D}/7zlzma${OE}: ${JMA_D}/7zlzma.cpp ${JMA_D}/7z.h \
	${JMA_D}/aribitcd.h ${JMA_D}/ariconst.h ${JMA_D}/btreecd.h \
	${JMA_D}/iiostrm.h ${JMA_D}/inbyte.h ${JMA_D}/lencoder.h \
	${JMA_D}/litcoder.h ${JMA_D}/lzma.h ${JMA_D}/lzmadec.h \
	${JMA_D}/portable.h ${JMA_D}/rcdefs.h ${JMA_D}/rngcoder.h \
	${JMA_D}/winout.h
${JMA_D}/crc32${OE}: ${JMA_D}/crc32.cpp
${JMA_D}/iiostrm${OE}: ${JMA_D}/iiostrm.cpp ${JMA_D}/crc32.h \
	${JMA_D}/iiostrm.h ${JMA_D}/portable.h
${JMA_D}/inbyte${OE}: ${JMA_D}/inbyte.cpp ${JMA_D}/iiostrm.h \
	${JMA_D}/inbyte.h ${JMA_D}/portable.h
${JMA_D}/jma${OE}: ${JMA_D}/jma.cpp ${JMA_D}/7z.h ${JMA_D}/crc32.h \
	${JMA_D}/iiostrm.h ${JMA_D}/jma.h ${JMA_D}/portable.h
${JMA_D}/lzma${OE}: ${JMA_D}/lzma.cpp ${JMA_D}/aribitcd.h \
	${JMA_D}/ariconst.h ${JMA_D}/btreecd.h ${JMA_D}/iiostrm.h \
	${JMA_D}/inbyte.h ${JMA_D}/lencoder.h ${JMA_D}/lzma.h \
	${JMA_D}/portable.h ${JMA_D}/rcdefs.h ${JMA_D}/rngcoder.h
${JMA_D}/lzmadec${OE}: ${JMA_D}/lzmadec.cpp ${JMA_D}/aribitcd.h \
	${JMA_D}/ariconst.h ${JMA_D}/btreecd.h ${JMA_D}/iiostrm.h \
	${JMA_D}/inbyte.h ${JMA_D}/lencoder.h ${JMA_D}/litcoder.h \
	${JMA_D}/lzma.h ${JMA_D}/lzmadec.h ${JMA_D}/portable.h \
	${JMA_D}/rcdefs.h ${JMA_D}/rngcoder.h ${JMA_D}/winout.h
${JMA_D}/winout${OE}: ${JMA_D}/winout.cpp ${JMA_D}/iiostrm.h \
	${JMA_D}/portable.h ${JMA_D}/winout.h
${JMA_D}/zsnesjma${OE}: ${JMA_D}/zsnesjma.cpp ${JMA_D}/jma.h \
	${JMA_D}/zsnesjma.h

${MMLIB_D}/mm${OE}: ${MMLIB_D}/mm.c ${MMLIB_D}/mm.h
${MMLIB_D}/windows${OE}: ${MMLIB_D}/windows.c ${MMLIB_D}/mm.h

${VID_D}/2xsaiw${OE}: ${VID_D}/2xsaiw.asm macros.mac
${VID_D}/copyvwin${OE}: ${VID_D}/copyvwin.asm macros.mac
${VID_D}/hq2x16${OE}: ${VID_D}/hq2x16.asm macros.mac
${VID_D}/hq2x32${OE}: ${VID_D}/hq2x32.asm macros.mac
${VID_D}/hq3x16${OE}: ${VID_D}/hq3x16.asm macros.mac
${VID_D}/hq3x32${OE}: ${VID_D}/hq3x32.asm macros.mac
${VID_D}/hq4x16${OE}: ${VID_D}/hq4x16.asm macros.mac
${VID_D}/hq4x32${OE}: ${VID_D}/hq4x32.asm macros.mac
${VID_D}/m716text${OE}: ${VID_D}/m716text.asm macros.mac \
	${VID_D}/mode7.mac
${VID_D}/makev16b${OE}: ${VID_D}/makev16b.asm macros.mac ${VID_D}/vidmacro.mac
${VID_D}/makev16t${OE}: ${VID_D}/makev16t.asm macros.mac ${VID_D}/vidmacro.mac
${VID_D}/makevid${OE}: ${VID_D}/makevid.asm macros.mac ${VID_D}/vidmacro.mac
${VID_D}/mode7${OE}: ${VID_D}/mode7.asm macros.mac ${VID_D}/mode7.mac
${VID_D}/mode716${OE}: ${VID_D}/mode716.asm macros.mac ${VID_D}/mode716.mac
${VID_D}/mode716b${OE}: ${VID_D}/mode716b.asm macros.mac ${VID_D}/mode7.mac
${VID_D}/mode716d${OE}: ${VID_D}/mode716d.asm macros.mac ${VID_D}/mode7.mac
${VID_D}/mode716e${OE}: ${VID_D}/mode716e.asm macros.mac ${VID_D}/mode7.mac
${VID_D}/mode716t${OE}: ${VID_D}/mode716t.asm macros.mac ${VID_D}/mode7.mac
${VID_D}/mode7ext${OE}: ${VID_D}/mode7ext.asm macros.mac ${VID_D}/mode7.mac
${VID_D}/mv16tms${OE}: ${VID_D}/mv16tms.asm macros.mac ${VID_D}/vidmacro.mac
${VID_D}/newg162${OE}: ${VID_D}/newg162.asm macros.mac ${VID_D}/newg162.mac \
	${VID_D}/newg16wn.mac ${VID_D}/newgfx16.mac ${VID_D}/vidmacro.mac
${VID_D}/newgfx${OE}: ${VID_D}/newgfx.asm macros.mac ${VID_D}/newgfx.mac \
	${VID_D}/newgfx2.mac ${VID_D}/vidmacro.mac
${VID_D}/newgfx16${OE}: ${VID_D}/newgfx16.asm macros.mac \
	${VID_D}/newg162.mac ${VID_D}/newgfx16.mac ${VID_D}/vidmacro.mac
${VID_D}/newgfx2${OE}: ${VID_D}/newgfx2.asm macros.mac ${VID_D}/newgfx.mac \
	${VID_D}/newgfx2.mac ${VID_D}/newgfxwn.mac ${VID_D}/vidmacro.mac
${VID_D}/ntsc${OE}: ${VID_D}/ntsc.c ${VID_D}/ntsc.h ${NTSC_D}/snes_ntsc.c \
	${NTSC_D}/snes_ntsc.h ${NTSC_D}/snes_ntsc_config.h ${NTSC_D}/snes_ntsc_impl.h
${VID_D}/procvid${OE}: ${VID_D}/procvid.asm macros.mac ${VID_D}/copyvid.inc
${VID_D}/procvidc${OE}: ${VID_D}/procvidc.c zmovie.h zpath.h \
	${JMA_D}/zsnesjma.h ${ZIP_D}/zunzip.h
${VID_D}/sw_draw${OE}: ${VID_D}/sw_draw.asm macros.mac

${WIN_D}/lib${OE}: ${WIN_D}/lib.c zpath.h ${JMA_D}/zsnesjma.h \
	${WIN_D}/lib.h ${ZIP_D}/zunzip.h
${WIN_D}/safelib${OE}: ${WIN_D}/safelib.c argv.h ${WIN_D}/safelib.h
${WIN_D}/winintrf${OE}: ${WIN_D}/winintrf.asm macros.mac
ifeq (${ENV},msvc)
${WIN_D}/winlink.obj: ${WIN_D}/winlink.cpp cfg${OE} input${OE} ${WIN_D}/resource.h
${WIN_D}/zsnes.res: ${WIN_D}/zsnes.rc ${WIN_D}/resource.h
	rc ${WIN_D}/zsnes.rc
else
${WIN_D}/winlink${OE}: ${WIN_D}/winlink.cpp cfg${OE} input${OE} ${WIN_D}/resource.h
	${CPPC} ${COPT} ${CFLAGS} -masm=intel -o $@ -c ${WIN_D}/winlink.cpp
${WIN_D}/zsnes${OE}: ${WIN_D}/zsnes.rc ${WIN_D}/resource.h
	${WINDRES} --include-dir ${WIN_D} ${WIN_D}/zsnes.rc -o${WIN_D}/zsnes${OE}
${OBJFIX}: objfix.c
	gcc -O3 -s -o $@ objfix.c
endif
${PSR}: parsegen.cpp
ifeq (${ENV},msvc)
	cl /nologo /EHsc /Fe$@ parsegen.cpp zlib.lib
	${DELETECOMMAND} parsegen.obj
else
ifeq (${ENV},dos)
	gpp -O3 -s -o $@ parsegen.cpp -lz
else
	g++ -O3 -s -o $@ parsegen.cpp -lz
endif
endif

${ZIP_D}/unzip${OE}: ${ZIP_D}/unzip.c ${ZIP_D}/zunzip.h
${ZIP_D}/zpng${OE}: ${ZIP_D}/zpng.c numconv.h zpath.h ${JMA_D}/zsnesjma.h \
	${ZIP_D}/zunzip.h

else  #else no ENV specified
	@echo Please specify which platform to compile for with PLATFORM=platform_name
endif #endif on the ENV being empty

clean:

ifneq (${ENV},)
# Take care that the expanded text between pre and suf is less than 28 characters
	${DELPRE} ${EXE}${SEP} ${PSR}${DELSUF}
	${DELPRE} *${OE}${SEP} ${NET_D}${SLASH}*${OE}${DELSUF}
	${DELPRE} ${CPU_D}${SLASH}*${OE}${SEP} ${GUI_D}${SLASH}*${OE}${DELSUF}
	${DELPRE} ${JMA_D}${SLASH}*${OE}${SEP} ${MMLIB_D}${SLASH}*${OE}${DELSUF}
	${DELPRE} ${EFF_D}${SLASH}*${OE}${SEP} ${ZIP_D}${SLASH}*${OE}${DELSUF}
	${DELPRE} ${DOS_D}${SLASH}*${OE}${SEP} ${WIN_D}${SLASH}*${OE}${DELSUF}
	${DELPRE} ${CHIP_D}${SLASH}*${OE}${SEP} ${VID_D}${SLASH}*${OE}${DELSUF}
else
	@echo Please specify which platform to clean for with PLATFORM=platform_name
endif
