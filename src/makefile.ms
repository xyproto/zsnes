#Copyright (C) 1997-2005 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
#
#http://www.zsnes.com
#http://sourceforge.net/projects/zsnes
#
#This program is free software; you can redistribute it and/or
#modify it under the terms of the GNU General Public License
#as published by the Free Software Foundation; either
#version 2 of the License, or (at your option) any later
#version.
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
GUIDIR=gui
VIDEODIR=video
WINDIR=win
NETDIR=net
ZIPDIR=zip
EFFECTSDIR=effects
JMADIR=jma

DELETECOMMAND=rm -f
TRUTH=;true
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
  MSVCLIBS=zlib.lib libpng.lib wsock32.lib user32.lib gdi32.lib shell32.lib winmm.lib dinput8.lib dxguid.lib
  DRESOBJ=${WINDIR}/zsnes.res
  OS=__WIN32__
endif

ifeq (${ENV},dos-cross)
  CC = i386-pc-msdosdjgpp-gcc
  CPPC = i386-pc-msdosdjgpp-g++
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
  TRUTH=
  DELETECOMMAND=del
  SLASH=\${BLAHBLAHBLAH}
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
  CFLAGSORIG=-fno-omit-frame-pointer -ggdb3 -DDEBUG
  COPT=-O0
  ASMFLAGSORIG=-DDEBUG -g
  EXE=zsnesd.exe
  STRIP=
endif

ifeq (${RELEASEGRADE},yes)
  ASMOPT=-O9999
endif

CFLAGS=${CFLAGSORIG} -D${OS}
ASMFLAGS=${ASMFLAGSORIG} -f ${FILEFORMAT} -D${OS}


CHIPSOBJ=${CHIPDIR}/dsp1emu${OE} ${CHIPDIR}/fxemu2${OE} ${CHIPDIR}/sfxproc${OE}\
	${CHIPDIR}/fxemu2b${OE} ${CHIPDIR}/fxemu2c${OE} ${CHIPDIR}/fxtable${OE}\
        ${CHIPDIR}/sa1proc${OE} ${CHIPDIR}/sa1regs${OE} ${CHIPDIR}/dsp1proc${OE}\
        ${CHIPDIR}/st10proc${OE} ${CHIPDIR}/seta10${OE} ${CHIPDIR}/dsp2proc${OE}\
	${CHIPDIR}/sdd1emu${OE} ${CHIPDIR}/c4emu${OE} ${CHIPDIR}/dsp4proc${OE}\
	${CHIPDIR}/dsp4emu${OE}

CPUOBJ=${CPUDIR}/dma${OE} ${CPUDIR}/dsp${OE} ${CPUDIR}/dspproc${OE}\
	${CPUDIR}/execute${OE} ${CPUDIR}/executec${OE} ${CPUDIR}/irq${OE}\
	${CPUDIR}/memory${OE} ${CPUDIR}/memtable${OE} ${CPUDIR}/spc700${OE}\
	${CPUDIR}/stable${OE} ${CPUDIR}/table${OE} ${CPUDIR}/tableb${OE}\
	${CPUDIR}/tablec${OE}

GUIOBJ=${GUIDIR}/gui${OE} ${GUIDIR}/guifuncs${OE} ${GUIDIR}/menu${OE}

VIDEOBJ=${VIDEODIR}/makev16b${OE} ${VIDEODIR}/makev16t${OE} ${VIDEODIR}/makevid${OE}\
	${VIDEODIR}/mode716${OE} ${VIDEODIR}/mode716b${OE} ${VIDEODIR}/mode716d${OE}\
	${VIDEODIR}/mode716e${OE} ${VIDEODIR}/mode716t${OE} ${VIDEODIR}/mode7${OE}\
	${VIDEODIR}/mode7ext${OE} ${VIDEODIR}/mv16tms${OE} ${VIDEODIR}/newg162${OE}\
	${VIDEODIR}/newgfx16${OE} ${VIDEODIR}/newgfx2${OE} ${VIDEODIR}/newgfx${OE}\
	${VIDEODIR}/m716text${OE} ${VIDEODIR}/procvid${OE} ${VIDEODIR}/procvidc${OE}

WINVIDOBJ=${VIDEODIR}/sw_draw${OE} ${VIDEODIR}/hq2x16${OE} ${VIDEODIR}/hq2x32${OE}\
	${VIDEODIR}/2xsaiw${OE} ${VIDEODIR}/hq3x16${OE} ${VIDEODIR}/hq3x32${OE}\
	${VIDEODIR}/hq4x16${OE} ${VIDEODIR}/hq4x32${OE}

#only used on Win32
WINOBJ=${WINDIR}/copyvwin${OE} ${DRESOBJ}\
	${WINDIR}/winintrf${OE} ${WINDIR}/winlink${OE} ${WINDIR}/zfilew${OE}

NETOBJ=
#${NETDIR}/ztcp${OE}

ZIPOBJ=${ZIPDIR}/unzip${OE} ${ZIPDIR}/zpng${OE}

EFFECTSOBJ=${EFFECTSDIR}/burn${OE} ${EFFECTSDIR}/water${OE} ${EFFECTSDIR}/smoke${OE}

JMAOBJ=${JMADIR}/7zlzma${OE} ${JMADIR}/crc32${OE} ${JMADIR}/iiostrm${OE}\
       ${JMADIR}/inbyte${OE} ${JMADIR}/jma${OE} ${JMADIR}/lzma${OE} ${JMADIR}/lzmadec${OE}\
       ${JMADIR}/winout${OE} ${JMADIR}/zsnesjma${OE}

MAINOBJ=cfgload${OE} endmem${OE} init${OE} initc${OE} uic${OE} patch${OE}\
	ui${OE} vcache${OE} version${OE} zmovie${OE} zstate${OE} zloader${OE} cfgparse${OE}

DOSOBJORIG=${DOSDIR}/debug${OE} ${DOSDIR}/joy${OE} ${DOSDIR}/vesa2${OE}\
       ${DOSDIR}/initvid${OE} ${DOSDIR}/sw${OE} ${DOSDIR}/gppro${OE} ${DOSDIR}/vesa12${OE}


OBJSORIG=${CHIPSOBJ} ${CPUOBJ} ${GUIOBJ} ${VIDEOBJ} ${MAINOBJ} ${ZIPOBJ} ${EFFECTSOBJ} ${JMAOBJ}

ifeq (${OS},__MSDOS__)
DOSOBJ=${DOSOBJORIG} ${DOSDIR}/dosintrf${OE} ${DOSDIR}/sound${OE} ${DOSDIR}/zfile${OE}
OBJS=${OBJSORIG} ${DOSOBJ}
DELETEOBJS=${OBJSORIG} ${DOSOBJ}
endif

ifeq (${OS},__WIN32__)
OBJS=${OBJSORIG} ${WINOBJ} ${DOSOBJORIG} ${WINVIDOBJ} ${NETOBJ}
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
	cl /Ox /G6 /c /EHsc /D__WIN32__ /Fo$@ $<

%.obj: %.cpp
	cl /Ox /G6 /c /EHsc /D__WIN32__ /Fo$@ $<

%.obj: %.asm
	${ASM} ${ASMOPT} -f win32 -D__WIN32__ -o $@ $<
endif

%${OE}: %.psr
	${PSR} -D${OS} temppsr.c $<
ifneq (${ENV},msvc)
	${CC} -o $@ -c temppsr.c
else
	cl /Ox /G6 /c /EHsc /D__WIN32__ /Fo$@ temppsr.c
endif        
	${DELETECOMMAND} temppsr.c

ALL: zsnes
	${DELETECOMMAND} version${OE}

zsnes: ${PSR} ${OBJFIX} ${OBJS}
ifneq (${ENV},msvc)
	${CPPC} ${STRIP} -o ${EXE} ${OBJS} ${LIBS}
else
	 @echo /Fezsnesw.exe *.obj ${CPUDIR}\*.obj ${VIDEODIR}\*.obj ${CHIPDIR}\*.obj ${EFFECTSDIR}\*.obj ${DOSDIR}\*.obj ${WINDIR}\*.obj ${GUIDIR}\*.obj > link.vc
	 @echo ${ZIPDIR}\*.obj ${JMADIR}\*.obj ${NETDIR}\*.obj >> link.vc
	 @echo ${MSVCLIBS} >> link.vc
	 cl @link.vc ${WINDIR}/zsnes.res /link
endif

vcache${OE}: $< macros.mac
ui${OE}: $< macros.mac
cfgload${OE}: $< macros.mac
init${OE}: $< macros.mac
initc${OE}: $< ${CPUDIR}/memtable.h ${JMADIR}/zsnesjma.h
uic${OE}: $<
patch${OE}: $<
endmem${OE}: $< macros.mac
zmovie${OE}: $< gblvars.h
zstate${OE}: $< gblvars.h
zloader${OE}: $<

${CPUDIR}/execute${OE}: $< macros.mac
${CPUDIR}/executec${OE}: $<
${CPUDIR}/table${OE}: $< ${CPUDIR}/65816d.inc ${CPUDIR}/address.inc ${CPUDIR}/addrni.inc ${CPUDIR}/e65816.inc\
	           ${CPUDIR}/regs.mac ${CPUDIR}/regs.inc ${CPUDIR}/regsw.mac ${CPUDIR}/regsw.inc macros.mac
${CPUDIR}/tableb${OE}: $< ${CPUDIR}/65816db.inc ${CPUDIR}/address.inc ${CPUDIR}/addrni.inc ${CPUDIR}/e65816b.inc\
	${CPUDIR}/regs.mac macros.mac
${CPUDIR}/tablec${OE}: $< ${CPUDIR}/65816dc.inc ${CPUDIR}/address.inc ${CPUDIR}/addrni.inc ${CPUDIR}/e65816c.inc\
	${CPUDIR}/regs.mac macros.mac
${CPUDIR}/stable${OE}: $< ${CPUDIR}/s65816d.inc ${CPUDIR}/saddress.inc ${CPUDIR}/saddrni.inc\
	            ${CPUDIR}/se65816.inc macros.mac
${CPUDIR}/memory${OE}: $< macros.mac
${CPUDIR}/memtable${OE}: ${CPUDIR}/memtable.h $<
${CPUDIR}/dma${OE}: $< macros.mac
${CPUDIR}/irq${OE}: $< macros.mac
${CPUDIR}/dspproc${OE}: $< macros.mac
${CPUDIR}/spc700${OE}: $< macros.mac ${CPUDIR}/regsw.mac ${CPUDIR}/spcdef.inc ${CPUDIR}/spcaddr.inc
${CPUDIR}/dsp${OE}: $< macros.mac

${VIDEODIR}/makevid${OE}: $< ${VIDEODIR}/vidmacro.mac macros.mac
${VIDEODIR}/makev16b${OE}: $< ${VIDEODIR}/vidmacro.mac macros.mac
${VIDEODIR}/makev16t${OE}: $< ${VIDEODIR}/vidmacro.mac macros.mac
${VIDEODIR}/mv16tms${OE}: $< ${VIDEODIR}/vidmacro.mac macros.mac
${VIDEODIR}/mode7${OE}: $< ${VIDEODIR}/mode7.mac macros.mac
${VIDEODIR}/mode716${OE}: $< ${VIDEODIR}/mode716.mac macros.mac
${VIDEODIR}/mode716b${OE}: $< ${VIDEODIR}/mode7.mac macros.mac
${VIDEODIR}/mode716t${OE}: $< ${VIDEODIR}/mode7.mac macros.mac
${VIDEODIR}/mode716d${OE}: $< ${VIDEODIR}/mode7.mac macros.mac
${VIDEODIR}/mode7ext${OE}: $< macros.mac
${VIDEODIR}/mode716e${OE}: $< macros.mac
${VIDEODIR}/m716text${OE}: $< ${VIDEODIR}/mode7.mac macros.mac
${VIDEODIR}/procvid${OE}: $< macros.mac ${VIDEODIR}/copyvid.inc ${VIDEODIR}/2xsaimmx.inc
${VIDEODIR}/procvidc${OE}: $<

${VIDEODIR}/2xsaiw${OE}: $< macros.mac
${VIDEODIR}/sw_draw${OE}: $< macros.mac
${VIDEODIR}/hq2x16${OE}: $< macros.mac
${VIDEODIR}/hq2x32${OE}: $< macros.mac
${VIDEODIR}/hq3x16${OE}: $< macros.mac
${VIDEODIR}/hq3x32${OE}: $< macros.mac
${VIDEODIR}/hq4x16${OE}: $< macros.mac
${VIDEODIR}/hq4x32${OE}: $< macros.mac

${CHIPDIR}/dsp1proc${OE}: $< macros.mac
${CHIPDIR}/sa1regs${OE}: $< macros.mac ${CPUDIR}/regs.mac ${CPUDIR}/regsw.mac
${CHIPDIR}/sfxproc${OE}: $< macros.mac ${CPUDIR}/regs.mac ${CPUDIR}/regsw.mac
${CHIPDIR}/dsp1emu${OE}: $<
${CHIPDIR}/st10proc${OE}: $<
${CHIPDIR}/seta10${OE}: $<
${CHIPDIR}/c4emu${OE}: $<
${CHIPDIR}/sdd1emu${OE}: $<
${CHIPDIR}/fxemu2${OE}: $< ${CHIPDIR}/fxemu2.mac macros.mac
${CHIPDIR}/fxemu2b${OE}: $< ${CHIPDIR}/fxemu2.mac ${CHIPDIR}/fxemu2b.mac
${CHIPDIR}/fxemu2c${OE}: $< macros.mac ${CHIPDIR}/fxemu2.mac ${CHIPDIR}/fxemu2b.mac ${CHIPDIR}/fxemu2c.mac
${CHIPDIR}/fxtable${OE}: $< macros.mac
${CHIPDIR}/sa1proc${OE}: $< macros.mac
${CHIPDIR}/dsp4proc${OE}: $< macros.mac
${CHIPDIR}/dsp4emu${OE}: $<


${GUIDIR}/gui${OE}: $< ${GUIDIR}/guitools.inc\
	${GUIDIR}/guimisc.inc ${GUIDIR}/guimouse.inc ${GUIDIR}/guiwindp.inc\
	${GUIDIR}/guikeys.inc ${GUIDIR}/guicheat.inc\
	${GUIDIR}/guicombo.inc ${GUIDIR}/guiload.inc macros.mac
${GUIDIR}/guifuncs${OE}: $<
${GUIDIR}/menu${OE}: $< macros.mac

${EFFECTSDIR}/water${OE}: $< ${EFFECTSDIR}/fixsin.h
${EFFECTSDIR}/smoke${OE}: $<
${EFFECTSDIR}/burn${OE}: $<

${ZIPDIR}/unzip${OE}: $< ${ZIPDIR}/zunzip.h
${ZIPDIR}/zpng${OE}: $<  ${ZIPDIR}/zpng.h

${JMADIR}/7zlzma${OE}: $< ${JMADIR}/7z.h ${JMADIR}/iiostrm.h
${JMADIR}/crc32${OE}: $< ${JMADIR}/crc32.h
${JMADIR}/iiostrm${OE}: $< ${JMADIR}/iiostrm.h ${JMADIR}/crc32.h
${JMADIR}/inbyte${OE}: $<
${JMADIR}/jma${OE}: $< ${JMADIR}/jma.h ${JMADIR}/crc32.h ${JMADIR}/portable.h\
	${JMADIR}/7z.h ${JMADIR}/iiostrm.h
${JMADIR}/lzma${OE}: $<
${JMADIR}/lzmadec${OE}: $<
${JMADIR}/winout${OE}: $< ${JMADIR}/iiostrm.h
${JMADIR}/zsnesjma${OE}: $< ${JMADIR}/zsnesjma.h ${JMADIR}/jma.h

${DOSDIR}/debug${OE}: $< macros.mac
${DOSDIR}/dosintrf${OE}: $< macros.mac
${DOSDIR}/initvid${OE}: $< macros.mac ${DOSDIR}/vga.inc
${DOSDIR}/vesa2${OE}: $< macros.mac
${DOSDIR}/vesa12${OE}: $< macros.mac
${DOSDIR}/joy${OE}: $< macros.mac
${DOSDIR}/sw${OE}: $< macros.mac
${DOSDIR}/gppro${OE}: $< macros.mac

${NETDIR}/ztcp${OE}: $<

${WINDIR}/copyvwin${OE}: $< macros.mac
${WINDIR}/winintrf${OE}: $< macros.mac
${WINDIR}/zfilew${OE}: $<
ifeq (${ENV},msvc)
${WINDIR}/zsnes.res: ${WINDIR}/zsnes.rc
	rc ${WINDIR}/zsnes.rc
${WINDIR}/winlink.obj: ${WINDIR}/winlink.cpp ${WINDIR}/resource.h
else
${WINDIR}/zsnes${OE}: ${WINDIR}/zsnes.rc
	${WINDRES} --include-dir ${WINDIR} ${WINDIR}/zsnes.rc -o${WINDIR}/zsnes${OE}
${WINDIR}/winlink${OE}: ${WINDIR}/winlink.cpp ${WINDIR}/resource.h
	${CPPC} -O0 ${CFLAGS} -masm=intel -o $@ -c ${WINDIR}/winlink.cpp
${OBJFIX}: $<
	gcc -O3 -o $@ objfix.c
endif
${PSR}: $<
ifeq (${ENV},msvc)
	cl /EHsc /Fe$@ parsegen.cpp
else
ifeq (${ENV},dos)
	gpp -O3 -o $@ parsegen.cpp
else
	g++ -O3 -o $@ parsegen.cpp
endif
endif
cfgparse${OE}: $<

clean:

	${DELETECOMMAND} ${EXE} ${TRUTH}
	${DELETECOMMAND} *${OE} ${TRUTH}
	${DELETECOMMAND} ${CPUDIR}${SLASH}*${OE} ${TRUTH}
	${DELETECOMMAND} ${VIDEODIR}${SLASH}*${OE} ${TRUTH}
	${DELETECOMMAND} ${CHIPDIR}${SLASH}*${OE} ${TRUTH}
	${DELETECOMMAND} ${EFFECTSDIR}${SLASH}*${OE} ${TRUTH}
	${DELETECOMMAND} ${ZIPDIR}${SLASH}*${OE} ${TRUTH}
	${DELETECOMMAND} ${JMADIR}${SLASH}*${OE} ${TRUTH}
	${DELETECOMMAND} ${DOSDIR}${SLASH}*${OE} ${TRUTH}
	${DELETECOMMAND} ${WINDIR}${SLASH}*${OE} ${TRUTH}
	${DELETECOMMAND} ${GUIDIR}${SLASH}*${OE} ${TRUTH}
	${DELETECOMMAND} ${NETDIR}${SLASH}*${OE} ${TRUTH}

else  #else no ENV specified
help:
	@echo Please specify which platform to compile/clean for with PLATFORM=platform_name
endif #endif on the ENV being empty
