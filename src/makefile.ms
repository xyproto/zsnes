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


CHIPDIR=chips
CPUDIR=cpu
DOSDIR=dos
EFFECTSDIR=effects
GUIDIR=gui
JMADIR=jma
NETDIR=net
VIDEODIR=video
WINDIR=win
ZIPDIR=zip
MMLIBDIR=mmlib

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
DRESOBJ=${WINDIR}/zsnes.obj
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
  CFLAGSORIG=/nologo /Ox /Oy- /GA /c /EHsc /MD "/I${DXSDK_DIR}include"
  MSVCLIBS=zlib.lib libpng.lib wsock32.lib user32.lib gdi32.lib shell32.lib winmm.lib "${DXSDK_DIR}lib\x86\dinput8.lib" "${DXSDK_DIR}lib\x86\dxguid.lib"
  DRESOBJ=${WINDIR}/zsnes.res
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


CHIPSOBJ=${CHIPDIR}/dsp1emu${OE} ${CHIPDIR}/fxemu2${OE} ${CHIPDIR}/sfxproc${OE}\
	${CHIPDIR}/fxemu2b${OE} ${CHIPDIR}/fxemu2c${OE} ${CHIPDIR}/fxtable${OE}\
	${CHIPDIR}/sa1proc${OE} ${CHIPDIR}/sa1regs${OE} ${CHIPDIR}/dsp1proc${OE}\
	${CHIPDIR}/st10proc${OE} ${CHIPDIR}/seta10${OE} ${CHIPDIR}/dsp2proc${OE}\
	${CHIPDIR}/sdd1emu${OE} ${CHIPDIR}/c4emu${OE} ${CHIPDIR}/dsp4proc${OE}\
	${CHIPDIR}/dsp4emu${OE} ${CHIPDIR}/dsp3proc${OE} ${CHIPDIR}/dsp3emu${OE}\
	${CHIPDIR}/obc1emu${OE} ${CHIPDIR}/obc1proc${OE} ${CHIPDIR}/c4proc${OE}\
	${CHIPDIR}/7110proc${OE} ${CHIPDIR}/st11proc${OE} ${CHIPDIR}/seta11${OE}

CPUOBJ=${CPUDIR}/dma${OE} ${CPUDIR}/dsp${OE} ${CPUDIR}/dspproc${OE}\
	${CPUDIR}/execute${OE} ${CPUDIR}/executec${OE} ${CPUDIR}/irq${OE}\
	${CPUDIR}/memory${OE} ${CPUDIR}/memtable${OE} ${CPUDIR}/spc700${OE}\
	${CPUDIR}/stable${OE} ${CPUDIR}/table${OE}\
	${CPUDIR}/tablec${OE}

EFFECTSOBJ=${EFFECTSDIR}/burn${OE} ${EFFECTSDIR}/water${OE} ${EFFECTSDIR}/smoke${OE}

GUIOBJ=${GUIDIR}/gui${OE} ${GUIDIR}/guifuncs${OE} ${GUIDIR}/menu${OE}

ifneq (${JMA},no)
JMAOBJ=${JMADIR}/7zlzma${OE} ${JMADIR}/crc32${OE} ${JMADIR}/iiostrm${OE}\
	${JMADIR}/inbyte${OE} ${JMADIR}/jma${OE} ${JMADIR}/lzma${OE} ${JMADIR}/lzmadec${OE}\
	${JMADIR}/winout${OE} ${JMADIR}/zsnesjma${OE}
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
#${NETDIR}/ztcp${OE}

VIDEOBJ=${VIDEODIR}/makev16b${OE} ${VIDEODIR}/makev16t${OE} ${VIDEODIR}/makevid${OE}\
	${VIDEODIR}/mode716${OE} ${VIDEODIR}/mode716b${OE} ${VIDEODIR}/mode716d${OE}\
	${VIDEODIR}/mode716e${OE} ${VIDEODIR}/mode716t${OE} ${VIDEODIR}/mode7${OE}\
	${VIDEODIR}/mode7ext${OE} ${VIDEODIR}/mv16tms${OE} ${VIDEODIR}/newg162${OE}\
	${VIDEODIR}/newgfx16${OE} ${VIDEODIR}/newgfx2${OE} ${VIDEODIR}/newgfx${OE}\
	${VIDEODIR}/m716text${OE} ${VIDEODIR}/procvid${OE} ${VIDEODIR}/procvidc${OE}\
	${VIDEODIR}/2xsaiw${OE}

ZIPOBJ=${ZIPDIR}/unzip${OE} ${ZIPDIR}/zpng${OE}

OBJS=${CHIPSOBJ} ${CPUOBJ} ${GUIOBJ} ${VIDEOBJ} ${MAINOBJ} ${ZIPOBJ} ${EFFECTSOBJ} ${JMAOBJ}

ifeq (${OS},__MSDOS__)
DOSOBJ=${DOSDIR}/dosintrf${OE} ${DOSDIR}/gppro${OE} ${DOSDIR}/initvid${OE}\
	${DOSDIR}/sound${OE} ${DOSDIR}/sw${OE} ${DOSDIR}/vesa12${OE}\
	${DOSDIR}/lib${OE} ${DOSDIR}/joy${OE} ${DOSDIR}/vesa2${OE}

OBJS+=${DOSOBJ}
endif

ifeq (${OS},__WIN32__)
WINOBJ=${DRESOBJ}\
	${WINDIR}/winintrf${OE} ${WINDIR}/winlink${OE} ${WINDIR}/lib${OE}\
	${WINDIR}/safelib${OE} ${MMLIBDIR}/mm${OE} ${MMLIBDIR}/windows${OE}

WINVIDOBJ=${VIDEODIR}/sw_draw${OE} ${VIDEODIR}/hq2x16${OE} ${VIDEODIR}/hq2x32${OE}\
	${VIDEODIR}/hq3x16${OE} ${VIDEODIR}/hq3x32${OE}\
	${VIDEODIR}/hq4x16${OE} ${VIDEODIR}/hq4x32${OE} ${VIDEODIR}/copyvwin${OE}\
	${VIDEODIR}/ntsc${OE}

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
	 @echo /Fezsnesw.exe *.obj ${CPUDIR}\*.obj ${VIDEODIR}\*.obj ${CHIPDIR}\*.obj ${EFFECTSDIR}\*.obj ${DOSDIR}\*.obj ${WINDIR}\*.obj ${GUIDIR}\*.obj > link.vc
	 @echo ${ZIPDIR}\*.obj ${JMADIR}\*.obj ${NETDIR}\*.obj ${MMLIBDIR}\*.obj >> link.vc
	 @echo ${MSVCLIBS} >> link.vc
	 cl /nologo @link.vc ${WINDIR}/zsnes.res /link
endif

cfg${OE}: cfg.psr ${PSR}
input${OE}: input.psr ${PSR}
debugasm${OE}: debugasm.asm macros.mac
debugger${OE}: debugger.c asm_call.h zpath.h ${JMADIR}/zsnesjma.h \
	${ZIPDIR}/zunzip.h
endmem${OE}: endmem.asm macros.mac
init${OE}: init.asm macros.mac
initc${OE}: initc.c asm_call.h cfg${OE} zpath.h ${CPU_D}/memtable.h \
	${JMA_D}/zsnesjma.h ${ZIP_D}/zunzip.h input${OE}
md${OE}: md.psr ${PSR}
patch${OE}: patch.c zpath.h ${JMADIR}/zsnesjma.h ${ZIPDIR}/zunzip.h
ui${OE}: ui.c asm_call.h cfg${OE} zpath.h ${JMADIR}/zsnesjma.h \
	${MMLIBDIR}/mm.h ${ZIPDIR}/zunzip.h input${OE}
vcache${OE}: vcache.asm macros.mac
version${OE}: version.c
zloader${OE}: zloader.c asm_call.h cfg${OE} zloader.h zpath.h \
	${JMADIR}/zsnesjma.h ${ZIPDIR}/zunzip.h input${OE}
zmovie${OE}: zmovie.c asm_call.h cfg${OE} gblvars.h md${OE} numconv.h zpath.h \
	${JMADIR}/zsnesjma.h ${WINDIR}/safelib.h ${ZIPDIR}/zunzip.h zmovie.h input${OE}
zpath${OE}: zpath.c cfg${OE} zpath.h ${DOSDIR}/lib.h ${JMADIR}/zsnesjma.h \
	${WINDIR}/lib.h ${WINDIR}/safelib.h ${ZIPDIR}/zunzip.h
zstate${OE}: zstate.c asm_call.h cfg${OE} gblvars.h numconv.h zpath.h \
	${JMADIR}/zsnesjma.h ${ZIPDIR}/zunzip.h zmovie.h input${OE}
ztime${OE}: ztime.asm
ztimec${OE}: ztimec.c

${CHIPDIR}/7110proc${OE}: ${CHIPDIR}/7110proc.asm macros.mac \
	${CPUDIR}/regs.mac ${CPUDIR}/regsw.mac
${CHIPDIR}/c4emu${OE}: ${CHIPDIR}/c4emu.c
${CHIPDIR}/c4proc${OE}: ${CHIPDIR}/c4proc.asm macros.mac
${CHIPDIR}/dsp1emu${OE}: ${CHIPDIR}/dsp1emu.c
${CHIPDIR}/dsp1proc${OE}: ${CHIPDIR}/dsp1proc.asm macros.mac
${CHIPDIR}/dsp2proc${OE}: ${CHIPDIR}/dsp2proc.asm macros.mac
${CHIPDIR}/dsp3emu${OE}: ${CHIPDIR}/dsp3emu.c
${CHIPDIR}/dsp3proc${OE}: ${CHIPDIR}/dsp3proc.asm macros.mac
${CHIPDIR}/dsp4emu${OE}: ${CHIPDIR}/dsp4emu.c ${CHIPDIR}/dsp4emu.h
${CHIPDIR}/dsp4proc${OE}: ${CHIPDIR}/dsp4proc.asm macros.mac
${CHIPDIR}/fxemu2${OE}: ${CHIPDIR}/fxemu2.asm macros.mac ${CHIPDIR}/fxemu2.mac
${CHIPDIR}/fxemu2b${OE}: ${CHIPDIR}/fxemu2b.asm macros.mac \
	${CHIPDIR}/fxemu2.mac ${CHIPDIR}/fxemu2b.mac
${CHIPDIR}/fxemu2c${OE}: ${CHIPDIR}/fxemu2c.asm macros.mac \
	${CHIPDIR}/fxemu2.mac ${CHIPDIR}/fxemu2c.mac
${CHIPDIR}/fxtable${OE}: ${CHIPDIR}/fxtable.asm macros.mac
${CHIPDIR}/obc1emu${OE}: ${CHIPDIR}/obc1emu.c
${CHIPDIR}/obc1proc${OE}: ${CHIPDIR}/obc1proc.asm macros.mac
${CHIPDIR}/sa1proc${OE}: ${CHIPDIR}/sa1proc.asm macros.mac
${CHIPDIR}/sa1regs${OE}: ${CHIPDIR}/sa1regs.asm macros.mac ${CPUDIR}/regs.mac \
	${CPUDIR}/regsw.mac
${CHIPDIR}/sdd1emu${OE}: ${CHIPDIR}/sdd1emu.c
${CHIPDIR}/seta10${OE}: ${CHIPDIR}/seta10.c
${CHIPDIR}/seta11${OE}: ${CHIPDIR}/seta11.c
${CHIPDIR}/sfxproc${OE}: ${CHIPDIR}/sfxproc.asm macros.mac ${CPUDIR}/regs.mac \
	${CPUDIR}/regsw.mac
${CHIPDIR}/st10proc${OE}: ${CHIPDIR}/st10proc.asm macros.mac
${CHIPDIR}/st11proc${OE}: ${CHIPDIR}/st11proc.asm macros.mac

${CPUDIR}/dma${OE}: ${CPUDIR}/dma.asm macros.mac
#${CPUDIR}/dsp${OE}: ${CPUDIR}/dsp.asm macros.mac
#${CPUDIR}/dspproc${OE}: ${CPUDIR}/dspproc.asm macros.mac ${CPUDIR}/firtable.inc
${CPUDIR}/spc/dsp${OE}: ${CPUDIR}/spc/dsp.cpp ${CPUDIR}/spc/b_common.h \
	${CPUDIR}/spc/b_config.h ${CPUDIR}/spc/b_endian.h ${CPUDIR}/spc/b_source.h \
	${CPUDIR}/spc/dsp.h
${CPUDIR}/spc/dspbind${OE}: ${CPUDIR}/spc/dspbind.cpp ${CPUDIR}/spc/b_common.h \
	${CPUDIR}/spc/b_config.h ${CPUDIR}/spc/dsp.h ${CPUDIR}/spc/dspbind.h
${CPUDIR}/execute${OE}: ${CPUDIR}/execute.asm macros.mac
${CPUDIR}/executec${OE}: ${CPUDIR}/executec.c gblvars.h
${CPUDIR}/irq${OE}: ${CPUDIR}/irq.asm macros.mac
${CPUDIR}/memory${OE}: ${CPUDIR}/memory.asm macros.mac
${CPUDIR}/memtable${OE}: ${CPUDIR}/memtable.c gblvars.h ${CPUDIR}/memtable.h
${CPUDIR}/spc700${OE}: ${CPUDIR}/spc700.asm macros.mac ${CPUDIR}/regsw.mac \
	${CPUDIR}/spcaddr.inc ${CPUDIR}/spcdef.inc
${CPUDIR}/stable${OE}: ${CPUDIR}/stable.asm macros.mac ${CPUDIR}/s65816d.inc \
	${CPUDIR}/saddress.inc ${CPUDIR}/saddrni.inc ${CPUDIR}/se65816.inc
${CPUDIR}/table${OE}: ${CPUDIR}/table.asm macros.mac ${CPUDIR}/65816d.inc \
	${CPUDIR}/address.inc ${CPUDIR}/addrni.inc ${CPUDIR}/e65816.inc \
	${CPUDIR}/regs.inc ${CPUDIR}/regs.mac ${CPUDIR}/regsw.inc \
	${CPUDIR}/regsw.mac
${CPUDIR}/tablec${OE}: ${CPUDIR}/tablec.asm macros.mac ${CPUDIR}/65816dc.inc \
	${CPUDIR}/address.inc ${CPUDIR}/addrni.inc ${CPUDIR}/e65816c.inc

${DOS_D}/dosintrf${OE}: ${DOS_D}/dosintrf.asm macros.mac
${DOS_D}/gppro${OE}: ${DOS_D}/gppro.asm macros.mac
${DOS_D}/initvid${OE}: ${DOS_D}/initvid.asm macros.mac ${DOS_D}/vga.inc
${DOS_D}/joy${OE}: ${DOS_D}/joy.asm macros.mac
${DOS_D}/lib${OE}: ${DOS_D}/lib.c
${DOS_D}/sound${OE}: ${DOS_D}/sound.c macros.mac
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
	${VID_D}/newg16wn.mac ${VID_D}/newgfx16.mac
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

${WIN_D}/confloc${OE}: ${WIN_D}/confloc.psr ${PSR}
${WIN_D}/lib${OE}: ${WIN_D}/lib.c zpath.h ${JMA_D}/zsnesjma.h \
	${WIN_D}/lib.h ${ZIP_D}/zunzip.h
${WIN_D}/safelib${OE}: ${WIN_D}/safelib.c argv.h ${WIN_D}/safelib.h
${WIN_D}/winintrf${OE}: ${WIN_D}/winintrf.asm macros.mac
${WIN_D}/winlink${OE}: ${WIN_D}/winlink.cpp ${WIN_D}/winlink.h ${WIN_D}/gl_draw.h cfg${OE} \
	input${OE} ${WIN_D}/resource.h ${DEBUGGER_D}/load.h
${WIN_D}/dx_ddraw${OE}: ${WIN_D}/dx_ddraw.cpp ${WIN_D}/winlink.h
${WIN_D}/gl_draw${OE}: ${WIN_D}/gl_draw.c ${WIN_D}/winlink.h ${WIN_D}/gl_draw.h cfg${OE}
ifeq (${ENV},msvc)
${WINDIR}/winintrf${OE}: ${WINDIR}/winintrf.asm macros.mac
${WINDIR}/winlink.obj: ${WINDIR}/winlink.cpp cfg${OE} ${WINDIR}/resource.h input${OE}
${WINDIR}/zsnes.res: ${WINDIR}/zsnes.rc ${WINDIR}/resource.h
	rc ${WINDIR}/zsnes.rc
else
${WINDIR}/winlink${OE}: ${WINDIR}/winlink.cpp cfg${OE} input${OE} ${WINDIR}/resource.h
	${CPPC} ${COPT} ${CFLAGS} -masm=intel -o $@ -c ${WINDIR}/winlink.cpp
${WINDIR}/zsnes${OE}: ${WINDIR}/zsnes.rc ${WINDIR}/resource.h
	${WINDRES} --include-dir ${WINDIR} ${WINDIR}/zsnes.rc -o${WINDIR}/zsnes${OE}
${OBJFIX}: objfix.c
	gcc -O3 -s -o $@ objfix.c
endif
${PSR}: parsegen.cpp
ifeq (${ENV},msvc)
	cl /nologo /EHsc /Fe$@ parsegen.cpp zlib.lib /MD
	${DELETECOMMAND} parsegen.obj
else
ifeq (${ENV},dos)
	gpp -O3 -s -o $@ parsegen.cpp -lz
else
	g++ -O3 -s -o $@ parsegen.cpp -lz
endif
endif

${ZIPDIR}/unzip${OE}: ${ZIPDIR}/unzip.c ${ZIPDIR}/zunzip.h
${ZIPDIR}/zpng${OE}: ${ZIPDIR}/zpng.c numconv.h zpath.h ${JMADIR}/zsnesjma.h \
	${ZIPDIR}/zunzip.h

else  #else no ENV specified
	@echo Please specify which platform to compile for with PLATFORM=platform_name
endif #endif on the ENV being empty

clean:

ifneq (${ENV},)
# Take care that the expanded text between pre and suf is less than 28 characters
	${DELPRE} ${EXE}${SEP} ${PSR}${DELSUF}
	${DELPRE} *${OE}${SEP} ${NETDIR}${SLASH}*${OE}${DELSUF}
	${DELPRE} ${CPUDIR}${SLASH}*${OE}${SEP} ${GUIDIR}${SLASH}*${OE}${DELSUF}
	${DELPRE} ${JMADIR}${SLASH}*${OE}${SEP} ${MMLIBDIR}${SLASH}*${OE}${DELSUF}
	${DELPRE} ${EFFECTSDIR}${SLASH}*${OE}${SEP} ${ZIPDIR}${SLASH}*${OE}${DELSUF}
	${DELPRE} ${DOSDIR}${SLASH}*${OE}${SEP} ${WINDIR}${SLASH}*${OE}${DELSUF}
	${DELPRE} ${CHIPDIR}${SLASH}*${OE}${SEP} ${VIDEODIR}${SLASH}*${OE}${DELSUF}
else
	@echo Please specify which platform to clean for with PLATFORM=platform_name
endif
