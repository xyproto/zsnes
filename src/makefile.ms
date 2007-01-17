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
  CFLAGSORIG=/nologo /Ox /G6 /c /EHsc
  MSVCLIBS=zlib.lib libpng.lib wsock32.lib user32.lib gdi32.lib shell32.lib winmm.lib dinput8.lib dxguid.lib
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

DOSOBJORIG=${DOSDIR}/joy${OE} ${DOSDIR}/vesa2${OE}

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

#only used on Win32
WINOBJ= ${DRESOBJ}\
	${WINDIR}/winintrf${OE} ${WINDIR}/winlink${OE} ${WINDIR}/lib${OE}\
	${WINDIR}/safelib${OE} ${MMLIBDIR}/mm${OE} ${MMLIBDIR}/windows${OE}

WINVIDOBJ=${VIDEODIR}/sw_draw${OE} ${VIDEODIR}/hq2x16${OE} ${VIDEODIR}/hq2x32${OE}\
	${VIDEODIR}/2xsaiw${OE} ${VIDEODIR}/hq3x16${OE} ${VIDEODIR}/hq3x32${OE}\
	${VIDEODIR}/hq4x16${OE} ${VIDEODIR}/hq4x32${OE} ${VIDEODIR}/copyvwin${OE}

ZIPOBJ=${ZIPDIR}/unzip${OE} ${ZIPDIR}/zpng${OE}


OBJSORIG=${CHIPSOBJ} ${CPUOBJ} ${GUIOBJ} ${VIDEOBJ} ${MAINOBJ} ${ZIPOBJ} ${EFFECTSOBJ} ${JMAOBJ}

ifeq (${OS},__MSDOS__)
DOSOBJ=${DOSOBJORIG} ${DOSDIR}/dosintrf${OE} ${DOSDIR}/gppro${OE} ${DOSDIR}/initvid${OE}\
	${DOSDIR}/sound${OE} ${DOSDIR}/sw${OE} ${DOSDIR}/vesa12${OE} ${DOSDIR}/lib${OE}
OBJS=${OBJSORIG} ${DOSOBJ}
DELETEOBJS=${OBJSORIG} ${DOSOBJ}
endif

ifeq (${OS},__WIN32__)
VIDEOBJ+= ${VIDEODIR}/ntsc${OE}
OBJS=${OBJSORIG} ${WINOBJ} ${WINVIDOBJ} ${NETOBJ}
DELETEOBJS=${OBJS}
endif


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
initc${OE}: initc.c asm_call.h cfg${OE} zpath.h ${CPUDIR}/memtable.h \
	${JMADIR}/zsnesjma.h ${ZIPDIR}/zunzip.h input${OE}
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
${CHIPDIR}/dsp4emu${OE}: ${CHIPDIR}/dsp4emu.c
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
${CPUDIR}/dsp${OE}: ${CPUDIR}/dsp.asm macros.mac
${CPUDIR}/dspproc${OE}: ${CPUDIR}/dspproc.asm macros.mac ${CPUDIR}/firtable.inc
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

${DOSDIR}/debug${OE}: ${DOSDIR}/debug.asm macros.mac
${DOSDIR}/dosintrf${OE}: ${DOSDIR}/dosintrf.asm macros.mac
${DOSDIR}/gppro${OE}: ${DOSDIR}/gppro.asm macros.mac
${DOSDIR}/initvid${OE}: ${DOSDIR}/initvid.asm macros.mac ${DOSDIR}/vga.inc
${DOSDIR}/joy${OE}: ${DOSDIR}/joy.asm macros.mac
${DOSDIR}/lib${OE}: ${DOSDIR}/lib.c
${DOSDIR}/sound${OE}: ${DOSDIR}/sound.asm macros.mac
${DOSDIR}/sw${OE}: ${DOSDIR}/sw.asm macros.mac
${DOSDIR}/vesa12${OE}: ${DOSDIR}/vesa12.asm macros.mac
${DOSDIR}/vesa2${OE}: ${DOSDIR}/vesa2.asm macros.mac

${EFFECTSDIR}/burn${OE}: ${EFFECTSDIR}/burn.c
${EFFECTSDIR}/smoke${OE}: ${EFFECTSDIR}/smoke.c
${EFFECTSDIR}/water${OE}: ${EFFECTSDIR}/water.c ${EFFECTSDIR}/fixsin.h

${GUIDIR}/gui${OE}: ${GUIDIR}/gui.asm macros.mac ${GUIDIR}/guicheat.inc \
	${GUIDIR}/guicombo.inc ${GUIDIR}/guikeys.inc ${GUIDIR}/guimisc.inc \
	${GUIDIR}/guimouse.inc ${GUIDIR}/guitools.inc ${GUIDIR}/guiwindp.inc
${GUIDIR}/guifuncs${OE}: ${GUIDIR}/guifuncs.c asm_call.h cfg${OE} md${OE} \
	zloader.h zpath.h ${DOSDIR}/lib.h ${JMADIR}/zsnesjma.h ${WINDIR}/lib.h \
	${ZIPDIR}/zunzip.h input${OE}
${GUIDIR}/menu${OE}: ${GUIDIR}/menu.asm macros.mac

${JMADIR}/7zlzma${OE}: ${JMADIR}/7zlzma.cpp ${JMADIR}/7z.h \
	${JMADIR}/aribitcd.h ${JMADIR}/ariconst.h ${JMADIR}/btreecd.h \
	${JMADIR}/iiostrm.h ${JMADIR}/inbyte.h ${JMADIR}/lencoder.h \
	${JMADIR}/litcoder.h ${JMADIR}/lzma.h ${JMADIR}/lzmadec.h \
	${JMADIR}/portable.h ${JMADIR}/rcdefs.h ${JMADIR}/rngcoder.h \
	${JMADIR}/winout.h
${JMADIR}/crc32${OE}: ${JMADIR}/crc32.cpp
${JMADIR}/iiostrm${OE}: ${JMADIR}/iiostrm.cpp ${JMADIR}/crc32.h \
	${JMADIR}/iiostrm.h ${JMADIR}/portable.h
${JMADIR}/inbyte${OE}: ${JMADIR}/inbyte.cpp ${JMADIR}/iiostrm.h \
	${JMADIR}/inbyte.h ${JMADIR}/portable.h
${JMADIR}/jma${OE}: ${JMADIR}/jma.cpp ${JMADIR}/7z.h ${JMADIR}/crc32.h \
	${JMADIR}/iiostrm.h ${JMADIR}/jma.h ${JMADIR}/portable.h
${JMADIR}/lzma${OE}: ${JMADIR}/lzma.cpp ${JMADIR}/aribitcd.h \
	${JMADIR}/ariconst.h ${JMADIR}/btreecd.h ${JMADIR}/iiostrm.h \
	${JMADIR}/inbyte.h ${JMADIR}/lencoder.h ${JMADIR}/lzma.h \
	${JMADIR}/portable.h ${JMADIR}/rcdefs.h ${JMADIR}/rngcoder.h
${JMADIR}/lzmadec${OE}: ${JMADIR}/lzmadec.cpp ${JMADIR}/aribitcd.h \
	${JMADIR}/ariconst.h ${JMADIR}/btreecd.h ${JMADIR}/iiostrm.h \
	${JMADIR}/inbyte.h ${JMADIR}/lencoder.h ${JMADIR}/litcoder.h \
	${JMADIR}/lzma.h ${JMADIR}/lzmadec.h ${JMADIR}/portable.h \
	${JMADIR}/rcdefs.h ${JMADIR}/rngcoder.h ${JMADIR}/winout.h
${JMADIR}/winout${OE}: ${JMADIR}/winout.cpp ${JMADIR}/iiostrm.h \
	${JMADIR}/portable.h ${JMADIR}/winout.h
${JMADIR}/zsnesjma${OE}: ${JMADIR}/zsnesjma.cpp ${JMADIR}/jma.h \
	${JMADIR}/zsnesjma.h

${MMLIBDIR}/mm${OE}: ${MMLIBDIR}/mm.c ${MMLIBDIR}/mm.h
${MMLIBDIR}/windows${OE}: ${MMLIBDIR}/windows.c ${MMLIBDIR}/mm.h

${VIDEODIR}/2xsaiw${OE}: ${VIDEODIR}/2xsaiw.asm macros.mac
${VIDEODIR}/hq2x16${OE}: ${VIDEODIR}/hq2x16.asm macros.mac
${VIDEODIR}/hq2x32${OE}: ${VIDEODIR}/hq2x32.asm macros.mac
${VIDEODIR}/hq3x16${OE}: ${VIDEODIR}/hq3x16.asm macros.mac
${VIDEODIR}/hq3x32${OE}: ${VIDEODIR}/hq3x32.asm macros.mac
${VIDEODIR}/hq4x16${OE}: ${VIDEODIR}/hq4x16.asm macros.mac
${VIDEODIR}/hq4x32${OE}: ${VIDEODIR}/hq4x32.asm macros.mac
${VIDEODIR}/m716text${OE}: ${VIDEODIR}/m716text.asm macros.mac
${VIDEODIR}/makev16b${OE}: ${VIDEODIR}/makev16b.asm macros.mac \
	${VIDEODIR}/vidmacro.mac
${VIDEODIR}/makev16t${OE}: ${VIDEODIR}/makev16t.asm macros.mac \
	${VIDEODIR}/vidmacro.mac
${VIDEODIR}/makevid${OE}: ${VIDEODIR}/makevid.asm macros.mac \
	${VIDEODIR}/vidmacro.mac
${VIDEODIR}/mode7${OE}: ${VIDEODIR}/mode7.asm macros.mac ${VIDEODIR}/mode7.mac
${VIDEODIR}/mode716${OE}: ${VIDEODIR}/mode716.asm macros.mac \
	${VIDEODIR}/mode716.mac
${VIDEODIR}/mode716b${OE}: ${VIDEODIR}/mode716b.asm macros.mac \
	${VIDEODIR}/mode7.mac
${VIDEODIR}/mode716d${OE}: ${VIDEODIR}/mode716d.asm macros.mac \
	${VIDEODIR}/mode7.mac
${VIDEODIR}/mode716e${OE}: ${VIDEODIR}/mode716e.asm macros.mac
${VIDEODIR}/mode716t${OE}: ${VIDEODIR}/mode716t.asm macros.mac \
	${VIDEODIR}/mode7.mac
${VIDEODIR}/mode7ext${OE}: ${VIDEODIR}/mode7ext.asm macros.mac \
	${VIDEODIR}/mode7.mac
${VIDEODIR}/mv16tms${OE}: ${VIDEODIR}/mv16tms.asm macros.mac \
	${VIDEODIR}/vidmacro.mac
${VIDEODIR}/newg162${OE}: ${VIDEODIR}/newg162.asm macros.mac \
	${VIDEODIR}/newg162.mac ${VIDEODIR}/newg16wn.mac ${VIDEODIR}/newgfx16.mac \
	${VIDEODIR}/vidmacro.mac
${VIDEODIR}/newgfx${OE}: ${VIDEODIR}/newgfx.asm macros.mac \
	${VIDEODIR}/newgfx.mac ${VIDEODIR}/newgfx2.mac ${VIDEODIR}/vidmacro.mac
${VIDEODIR}/newgfx16${OE}: ${VIDEODIR}/newgfx16.asm macros.mac \
	${VIDEODIR}/newg162.mac ${VIDEODIR}/newgfx16.mac ${VIDEODIR}/vidmacro.mac
${VIDEODIR}/newgfx2${OE}: ${VIDEODIR}/newgfx2.asm macros.mac \
	${VIDEODIR}/newgfx.mac ${VIDEODIR}/newgfx2.mac ${VIDEODIR}/newgfxwn.mac \
	${VIDEODIR}/vidmacro.mac
${VIDEODIR}/ntsc${OE}: ${VIDEODIR}/ntsc.c ${VIDEODIR}/ntsc.h
${VIDEODIR}/procvid${OE}: ${VIDEODIR}/procvid.asm macros.mac \
	${VIDEODIR}/2xsaimmx.inc ${VIDEODIR}/copyvid.inc
${VIDEODIR}/procvidc${OE}: ${VIDEODIR}/procvidc.c zpath.h \
	${JMADIR}/zsnesjma.h ${VIDEODIR}/ntsc.h ${ZIPDIR}/zunzip.h zmovie.h
${VIDEODIR}/sw_draw${OE}: ${VIDEODIR}/sw_draw.asm macros.mac

${WINDIR}/copyvwin${OE}: ${WINDIR}/copyvwin.asm macros.mac
${WINDIR}/lib${OE}: ${WINDIR}/lib.c zpath.h ${JMADIR}/zsnesjma.h \
	${WINDIR}/lib.h ${ZIPDIR}/zunzip.h
${WINDIR}/safelib${OE}: ${WINDIR}/safelib.c argv.h
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
	cl /nologo /EHsc /Fe$@ parsegen.cpp zlib.lib
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
