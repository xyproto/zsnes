#Copyright (C) 1997-2005 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
#
#zsknight@zsnes.com
#_demo_@zsnes.com
#pagefault@zsnes.com
#n-a-c-h@users.sf.net
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


DELETECOMMAND=rm
TRUTH=;true
SLASH=/
ASM=nasm
ASMFLAGSORIG=-O1
CFLAGSORIG=-march=i586 -ffast-math -fomit-frame-pointer -fno-unroll-loops -Wall -Wno-unused
COPT=-O3
LIBSORIG=-lz -lpng

ifeq ($(PLATFORM),)
_foo: help
endif


ifeq ($(PLATFORM),dos)
  CC = gcc
  CPPC = gpp
  OS=__MSDOS__
  CROSS=no
endif

ifeq ($(PLATFORM),win32)
  CC = gcc
  CPPC = g++
  WINDRES = windres
  OS=__WIN32__
  CROSS=no
endif

ifeq ($(PLATFORM),dos-cross)
  CC = i386-pc-msdosdjgpp-gcc
  CPPC = i386-pc-msdosdjgpp-g++
  OS=__MSDOS__
  CROSS=yes
endif

ifeq ($(PLATFORM),win32-cross)
  CC = i586-mingw32msvc-gcc
  CPPC = i586-mingw32msvc-g++
  WINDRES = i586-mingw32msvc-windres
  OS=__WIN32__
  CROSS=yes
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
endif

ifeq (${OS},__WIN32__)
  EXE=zsnesw.exe
  FILEFORMAT=gnuwin32
  LIBS=${LIBSORIG} -ldxguid -ldinput8 -lwsock32 -luser32 -lgdi32 -lshell32 -lwinmm -mwindows
endif

CFLAGS=${CFLAGSORIG} -D${OS}
ASMFLAGS=${ASMFLAGSORIG} -f ${FILEFORMAT} -D${OS}





CHIPDIR=chips
CPUDIR=cpu
DOSDIR=dos
GUIDIR=gui
VIDEODIR=video
WINDIR=win
OBJDIR=obj
ZIPDIR=zip
EFFECTSDIR=effects
JMADIR=jma

CHIPSOBJ=${CHIPDIR}/dsp1emu.o ${CHIPDIR}/fxemu2.o ${CHIPDIR}/sfxproc.o\
	${CHIPDIR}/fxemu2b.o ${CHIPDIR}/fxemu2c.o ${CHIPDIR}/fxtable.o\
        ${CHIPDIR}/sa1proc.o ${CHIPDIR}/sa1regs.o ${CHIPDIR}/dsp1proc.o\
        ${CHIPDIR}/st10proc.o ${CHIPDIR}/seta10.o ${CHIPDIR}/dsp2proc.o\
	${CHIPDIR}/sdd1emu.o

CPUOBJ=${CPUDIR}/dma.o ${CPUDIR}/dsp.o ${CPUDIR}/dspproc.o\
	${CPUDIR}/execute.o ${CPUDIR}/irq.o ${CPUDIR}/memory.o\
	${CPUDIR}/spc700.o ${CPUDIR}/stable.o ${CPUDIR}/table.o\
	${CPUDIR}/tableb.o ${CPUDIR}/tablec.o

GUIOBJ=${GUIDIR}/gui.o ${GUIDIR}/menu.o

VIDEOBJ=${VIDEODIR}/makev16b.o ${VIDEODIR}/makev16t.o ${VIDEODIR}/makevid.o\
	${VIDEODIR}/mode716.o ${VIDEODIR}/mode716b.o ${VIDEODIR}/mode716d.o\
	${VIDEODIR}/mode716e.o ${VIDEODIR}/mode716t.o ${VIDEODIR}/mode7.o\
	${VIDEODIR}/mode7ext.o ${VIDEODIR}/mv16tms.o ${VIDEODIR}/newg162.o\
	${VIDEODIR}/newgfx16.o ${VIDEODIR}/newgfx2.o ${VIDEODIR}/newgfx.o\
	${VIDEODIR}/m716text.o ${VIDEODIR}/procvid.o

#only used on Win32

WINVIDOBJ=${VIDEODIR}/sw_draw.o ${VIDEODIR}/hq2x16.o ${VIDEODIR}/hq2x32.o\
	${VIDEODIR}/2xsaiw.o ${VIDEODIR}/hq3x16.o ${VIDEODIR}/hq3x32.o\
	${VIDEODIR}/hq4x16.o ${VIDEODIR}/hq4x32.o

WINOBJ=${WINDIR}/copyvwin.o ${WINDIR}/zsnes.o\
	${WINDIR}/winintrf.o ${WINDIR}/winlink.o ${WINDIR}/zloaderw.o\
	${WINDIR}/ztcp.o ${WINDIR}/zipxw.o ${WINDIR}/zfilew.o

#only used on DOS
PREOBJ=${OBJDIR}/dosbuff.o ${OBJDIR}/ipx.o ${OBJDIR}/zipx.o

ZIPOBJ=${ZIPDIR}/unzip.o ${ZIPDIR}/zpng.o

EFFECTSOBJ=${EFFECTSDIR}/burn.o ${EFFECTSDIR}/water.o ${EFFECTSDIR}/smoke.o

JMAOBJ=${JMADIR}/7zlzma.o ${JMADIR}/crc32.o ${JMADIR}/iiostrm.o\
       ${JMADIR}/inbyte.o ${JMADIR}/jma.o ${JMADIR}/lzma.o ${JMADIR}/lzmadec.o\
       ${JMADIR}/winout.o ${JMADIR}/zsnesjma.o
       
MAINOBJ=cfgload.o endmem.o init.o initc.o uic.o patch.o ui.o vcache.o version.o

DOSOBJORIG=${DOSDIR}/debug.o ${DOSDIR}/joy.o ${DOSDIR}/modemrtn.o ${DOSDIR}/vesa2.o\
       ${DOSDIR}/initvid.o ${DOSDIR}/sw.o ${DOSDIR}/gppro.o ${DOSDIR}/vesa12.o\
       ${DOSDIR}/zsipx.o


OBJSORIG=${CHIPSOBJ} ${CPUOBJ} ${GUIOBJ} ${VIDEOBJ} ${MAINOBJ} ${ZIPOBJ} ${EFFECTSOBJ} ${JMAOBJ}

ifeq (${OS},__MSDOS__)
DOSOBJ=${DOSOBJORIG} ${DOSDIR}/dosintrf.o ${DOSDIR}/sound.o ${DOSDIR}/zloader.o ${DOSDIR}/zfile.o
OBJS=${OBJSORIG} ${DOSOBJ} ${PREOBJ}
DELETEOBJS=${OBJSORIG} ${DOSOBJ}
endif

ifeq (${OS},__WIN32__)
OBJS=${OBJSORIG} ${WINOBJ} ${DOSOBJORIG} ${WINVIDOBJ}
DELETEOBJS=${OBJS}
endif


.SUFFIXES: .c .cpp .asm

%.o: %.c
	${CC} ${COPT} ${CFLAGS} -o $@ -c $<

%.o: %.cpp
	${CPPC} ${COPT} ${CFLAGS} -o $@ -c $<

%.o: %.asm
	${ASM} ${ASMFLAGS} -o $@ $<


ALL: zsnes
	${DELETECOMMAND} version.o


zsnes: ${OBJS}
	${CPPC} -Ws -s -o ${EXE} ${OBJS}  ${LIBS}

debug: zsnesd

zsnesd: ${OBJS}
	${CPPC} -Ws -o zsnesd.exe ${OBJS}  ${LIBS}


vcache.o: $< macros.mac
ui.o: $< macros.mac
cfgload.o: $< macros.mac
init.o: $< macros.mac
initc.o: $< ${JMADIR}/zsnesjma.h
uic.o: $<
patch.o: $<
endmem.o: $< macros.mac

${CPUDIR}/execute.o: $< macros.mac
${CPUDIR}/table.o: $< ${CPUDIR}/65816d.inc ${CPUDIR}/address.inc ${CPUDIR}/addrni.inc ${CPUDIR}/e65816.inc\
	           ${CPUDIR}/regs.mac ${CPUDIR}/regs.inc ${CPUDIR}/regsw.mac ${CPUDIR}/regsw.inc macros.mac
${CPUDIR}/tableb.o: $< ${CPUDIR}/65816db.inc ${CPUDIR}/address.inc ${CPUDIR}/addrni.inc ${CPUDIR}/e65816b.inc\
	${CPUDIR}/regs.mac macros.mac
${CPUDIR}/tablec.o: $< ${CPUDIR}/65816dc.inc ${CPUDIR}/address.inc ${CPUDIR}/addrni.inc ${CPUDIR}/e65816c.inc\
	${CPUDIR}/regs.mac macros.mac
${CPUDIR}/stable.o: $< ${CPUDIR}/s65816d.inc ${CPUDIR}/saddress.inc ${CPUDIR}/saddrni.inc\
	            ${CPUDIR}/se65816.inc macros.mac
${CPUDIR}/memory.o: $< macros.mac
${CPUDIR}/dma.o: $< macros.mac
${CPUDIR}/irq.o: $< macros.mac
${CPUDIR}/dspproc.o: $< macros.mac
${CPUDIR}/spc700.o: $< macros.mac ${CPUDIR}/regsw.mac ${CPUDIR}/spcdef.inc ${CPUDIR}/spcaddr.inc
${CPUDIR}/dsp.o: $< macros.mac

${VIDEODIR}/makevid.o: $< ${VIDEODIR}/vidmacro.mac macros.mac
${VIDEODIR}/makev16b.o: $< ${VIDEODIR}/vidmacro.mac macros.mac
${VIDEODIR}/makev16t.o: $< ${VIDEODIR}/vidmacro.mac macros.mac
${VIDEODIR}/mv16tms.o: $< ${VIDEODIR}/vidmacro.mac macros.mac
${VIDEODIR}/mode7.o: $< ${VIDEODIR}/mode7.mac macros.mac
${VIDEODIR}/mode716.o: $< ${VIDEODIR}/mode716.mac macros.mac
${VIDEODIR}/mode716b.o: $< ${VIDEODIR}/mode7.mac macros.mac
${VIDEODIR}/mode716t.o: $< ${VIDEODIR}/mode7.mac macros.mac
${VIDEODIR}/mode716d.o: $< ${VIDEODIR}/mode7.mac macros.mac
${VIDEODIR}/mode7ext.o: $< macros.mac
${VIDEODIR}/mode716e.o: $< macros.mac
${VIDEODIR}/m716text.o: $< ${VIDEODIR}/mode7.mac macros.mac
${VIDEODIR}/procvid.o: $< macros.mac ${VIDEODIR}/copyvid.inc ${VIDEODIR}/2xsaimmx.inc

${VIDEODIR}/2xsaiw.o: $< macros.mac
${VIDEODIR}/sw_draw.o: $< macros.mac
${VIDEODIR}/hq2x16.o: $< macros.mac
${VIDEODIR}/hq2x32.o: $< macros.mac
${VIDEODIR}/hq3x16.o: $< macros.mac
${VIDEODIR}/hq3x32.o: $< macros.mac
${VIDEODIR}/hq4x16.o: $< macros.mac
${VIDEODIR}/hq4x32.o: $< macros.mac

${CHIPDIR}/dsp1proc.o: $< macros.mac
${CHIPDIR}/sa1regs.o: $< macros.mac ${CPUDIR}/regs.mac ${CPUDIR}/regsw.mac
${CHIPDIR}/sfxproc.o: $< macros.mac ${CPUDIR}/regs.mac ${CPUDIR}/regsw.mac
${CHIPDIR}/dsp1emu.o: $<
${CHIPDIR}/st10proc.o: $<
${CHIPDIR}/seta10.o: $<
${CHIPDIR}/sdd1emu.o: $<
${CHIPDIR}/fxemu2.o: $< ${CHIPDIR}/fxemu2.mac macros.mac
${CHIPSDIR}/fxemu2b.o: $< ${CHIPDIR}/fxemu2.mac ${CHIPDIR}/fxemu2b.mac
${CHIPSDIR}/fxemu2c.o: $< macros.mac ${CHIPDIR}/fxemu2.mac ${CHIPDIR}/fxemu2b.mac ${CHIPDIR}/fxemu2c.mac
${CHIPDIR}/fxtable.o: $< macros.mac
${CHIPDIR}/sa1proc.o: $< macros.mac

${GUIDIR}/gui.o: $< ${GUIDIR}/guitools.inc\
	${GUIDIR}/guimisc.inc ${GUIDIR}/guimouse.inc ${GUIDIR}/guiwindp.inc\
	${GUIDIR}/guinetpl.inc ${GUIDIR}/guikeys.inc ${GUIDIR}/guicheat.inc\
	${GUIDIR}/guicombo.inc ${GUIDIR}/guiload.inc macros.mac
${GUIDIR}/menu.obj: ${GUIDIR}/menu.asm macros.mac

${EFFECTSDIR}/water.o: $< ${EFFECTSDIR}/fixsin.h
${EFFECTSDIR}/smoke.o: $<
${EFFECTSDIR}/burn.o: $<

${ZIPDIR}/unzip.o: $< ${ZIPDIR}/zunzip.h
${ZIPDIR}/zpng.o: $<  ${ZIPDIR}/zpng.h

${JMADIR}/7zlzma.o: $<
${JMADIR}/crc32.o: $< ${JMADIR}/crc32.h
${JMADIR}/iiostrm.o: $<
${JMADIR}/inbyte.o: $<
${JMADIR}/jma.o: $< ${JMADIR}/jma.h ${JMADIR}/crc32.h ${JMADIR}/portable.h
${JMADIR}/lzma.o: $<
${JMADIR}/lzmadec.o: $<
${JMADIR}/winout.o: $<
${JMADIR}/zsnesjma.o: $< ${JMADIR}/zsnesjma.h ${JMADIR}/jma.h

${DOSDIR}/debug.o: $< macros.mac
${DOSDIR}/dosintrf.o: $< macros.mac
${DOSDIR}/initvid.o: $< macros.mac ${DOSDIR}/vga.inc
${DOSDIR}/vesa2.o: $< macros.mac
${DOSDIR}/vesa12.o: $< macros.mac
${DOSDIR}/joy.o: $< macros.mac
${DOSDIR}/sw.o: $< macros.mac
${DOSDIR}/gppro.o: $< macros.mac
${DOSDIR}/zsipx.o: $<
${DOSDIR}/modemrtn.o: $< macros.mac

${WINDIR}/copyvwin.o: $< macros.mac
${WINDIR}/winintrf.o: $< macros.mac
${WINDIR}/zfilew.o: $<
${WINDIR}/zipxw.o: $<
${WINDIR}/zloaderw.o: $<
${WINDIR}/ztcp.o: $<
${WINDIR}/zsnes.o: ${WINDIR}/zsnes.rc
	${WINDRES} --include-dir ${WINDIR} ${WINDIR}/zsnes.rc -o${WINDIR}/zsnes.o
${WINDIR}/winlink.o: ${WINDIR}/winlink.cpp ${WINDIR}/resource.h
	${CPPC} -O0 ${CFLAGS} -masm=intel -o $@ -c ${WINDIR}/winlink.cpp

clean: 
	${DELETECOMMAND} ${EXE} ${TRUTH}
	${DELETECOMMAND} *.o ${TRUTH}
	${DELETECOMMAND} ${CPUDIR}${SLASH}*.o ${TRUTH}
	${DELETECOMMAND} ${VIDEODIR}${SLASH}*.o ${TRUTH}
	${DELETECOMMAND} ${CHIPDIR}${SLASH}*.o ${TRUTH}
	${DELETECOMMAND} ${EFFECTSDIR}${SLASH}*.o ${TRUTH}
	${DELETECOMMAND} ${ZIPDIR}${SLASH}*.o ${TRUTH}
	${DELETECOMMAND} ${JMADIR}${SLASH}*.o ${TRUTH}
	${DELETECOMMAND} ${DOSDIR}${SLASH}*.o ${TRUTH}
	${DELETECOMMAND} ${WINDIR}${SLASH}*.o ${TRUTH}
	${DELETECOMMAND} ${GUIDIR}${SLASH}*.o ${TRUTH}

help:
	@echo Please specify which platform to compile for with PLATFORM=platform_name
