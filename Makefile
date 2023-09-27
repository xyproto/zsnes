# CXX/NASM from host system
CXX_HOST = g++
ASM = nasm

# TODO: FreeBSD has a patch for being able to build without -fcommon
CFLAGS += -m32 -march=pentium-mmx -no-pie -std=gnu99 -fcommon -O1 -fno-inline -fno-pic -ffunction-sections -fdata-sections -Wfatal-errors -w
LDFLAGS += -Wl,--as-needed -no-pie -Wl,--gc-sections -lz
# -O1 is mandatory
ASMFLAGS += -O1 -w-orphan-labels

#WITH_DEBUGGER  := yes
#WITH_PNG       := yes
#WINDOWS        := yes

# Import libraries from root folder. This is currently necessary on Ubuntu, since it's not possible to download i386 packages
# due to what seems to be a bug in apt install where it wants you to install libc:i386 which leads to all of your other packages getting deleted.
# See workaround in README.MD , relevant issue on github: https://github.com/xyproto/zsnes/issues/19
LIBRARIES_ROOT := yes

# Windows
ifdef WINDOWS
	CC = i686-w64-mingw32-gcc-posix
	CXX = i686-w64-mingw32-g++-posix
	ASMFLAGS += -fwin32 -DELF --prefix _
	CFLAGS += -static-libgcc -mconsole
	LDFLAGS += -static-libgcc -lopengl32 -lwsock32 -lws2_32
	ifdef LIBRARIES_ROOT
		CFLAGS += -L./libraries/mingw32/lib -I./libraries/mingw32/include
		LDFLAGS += -L./libraries/mingw32/lib
	endif
else
	CC = gcc
	CXX = g++
	CFLAGS += -D_FORTIFY_SOURCE=2
	ASMFLAGS += -felf32 -DELF
	LDFLAGS += -lGL
	ifdef LIBRARIES_ROOT
		CFLAGS += -L./libraries/usr/lib
		LDFLAGS += -L./libraries/usr/lib
	endif
endif

# Other
BINARY     ?= zsnes

# SDL is necessary
SDL_CONFIG ?= pkg-config sdl
ifndef CFLAGS_SDL
	CFLAGS_SDL := $(shell $(SDL_CONFIG) --cflags)
endif
ifndef LDFLAGS_SDL
	LDFLAGS_SDL := $(shell $(SDL_CONFIG) --libs)
endif
CFLAGS  += $(CFLAGS_SDL)
LDFLAGS += $(LDFLAGS_SDL)

# PNG Support
ifdef WITH_PNG
	PNG_CONFIG ?= pkg-config libpng
	ifndef CFLAGS_PNG
		CFLAGS_PNG  := $(shell $(PNG_CONFIG) --cflags)
	endif
	ifndef LDFLAGS_PNG
		LDFLAGS_PNG := $(shell $(PNG_CONFIG) --libs)
	endif
	CFLAGS  += $(CFLAGS_PNG)
	LDFLAGS += $(LDFLAGS_PNG)
else
	CFGDEFS += -DNO_PNG
endif

SRCS :=
SRCS += c_init.c
SRCS += c_vcache.c
SRCS += chips/7110emu.c
SRCS += chips/7110proc.asm
SRCS += chips/msu1emu.c
SRCS += chips/msu1regs.asm
SRCS += chips/c4emu.c
SRCS += chips/c4proc.asm
SRCS += chips/c_dsp2proc.c
SRCS += chips/c_sa1regs.c
SRCS += chips/c_sfxproc.c
SRCS += chips/dsp1emu.c
SRCS += chips/dsp1proc.asm
SRCS += chips/dsp2proc.asm
SRCS += chips/dsp3emu.c
SRCS += chips/dsp3proc.asm
SRCS += chips/dsp4emu.c
SRCS += chips/dsp4proc.asm
SRCS += chips/fxemu2.asm
SRCS += chips/fxemu2b.asm
SRCS += chips/fxemu2c.asm
SRCS += chips/fxtable.asm
SRCS += chips/obc1emu.c
SRCS += chips/obc1proc.asm
SRCS += chips/sa1emu.c
SRCS += chips/sa1proc.asm
SRCS += chips/sa1regs.asm
SRCS += chips/sdd1emu.c
SRCS += chips/seta10.c
SRCS += chips/seta11.c
SRCS += chips/sfxproc.asm
SRCS += chips/st10proc.asm
SRCS += chips/st11proc.asm
SRCS += cpu/c_65816d.c
SRCS += cpu/c_dma.c
SRCS += cpu/c_dsp.c
SRCS += cpu/c_dspproc.c
SRCS += cpu/c_execute.c
SRCS += cpu/c_irq.c
SRCS += cpu/c_memory.c
SRCS += cpu/c_regs.c
SRCS += cpu/c_regsw.c
SRCS += cpu/c_table.c
SRCS += cpu/c_tablec.c
SRCS += cpu/dma.asm
SRCS += cpu/dsp.asm
SRCS += cpu/dspproc.asm
SRCS += cpu/execute.asm
SRCS += cpu/executec.c
SRCS += cpu/irq.asm
SRCS += cpu/memory.asm
SRCS += cpu/memtable.c
SRCS += cpu/spc700.asm
SRCS += cpu/stable.asm
SRCS += cpu/table.asm
SRCS += cpu/tablec.asm
SRCS += effects/burn.c
SRCS += effects/smoke.c
SRCS += effects/water.c
SRCS += endmem.asm
SRCS += gui/c_gui.c
SRCS += gui/c_guiwindp.c
SRCS += gui/gui.asm
SRCS += gui/guicheat.c
SRCS += gui/guicombo.c
SRCS += gui/guifuncs.c
SRCS += gui/guikeys.c
SRCS += gui/guimisc.c
SRCS += gui/guimouse.c
SRCS += gui/guitools.c
SRCS += gui/menu.c
SRCS += init.asm
SRCS += initc.c
SRCS += mmlib/mm.c
SRCS += patch.c
SRCS += ui.c
SRCS += vcache.asm
SRCS += ver.c
SRCS += video/c_makev16b.c
SRCS += video/c_makevid.c
SRCS += video/c_mode716.c
SRCS += video/c_newgfx16.c
SRCS += video/m716text.asm
SRCS += video/makev16b.asm
SRCS += video/makev16t.asm
SRCS += video/makevid.asm
SRCS += video/mode716.asm
SRCS += video/mode716b.asm
SRCS += video/mode716d.asm
SRCS += video/mode716e.asm
SRCS += video/mode716t.asm
SRCS += video/mv16tms.asm
SRCS += video/newg162.asm
SRCS += video/newgfx.asm
SRCS += video/newgfx16.asm
SRCS += video/procvid.c
SRCS += video/procvidc.c
SRCS += zdir.c
SRCS += zip/unzip.c
SRCS += zip/zpng.c
SRCS += zloader.c
SRCS += zmovie.c
SRCS += zpath.c
SRCS += zstate.c
SRCS += ztimec.c
SRCS += netplay/znet.c
SRCS += netplay/zsocket.c
SRCS += config/cfg.c
SRCS += config/input.c
SRCS += config/md.c

ifdef WITH_DEBUGGER
SRCS += debugasm.c
SRCS += debugger.c
LDFLAGS += -lcurses
else
CFGDEFS += -DNO_DEBUGGER
endif

DEBUGFLAGS :=

SRCS += linux/audio.c
SRCS += linux/c_sdlintrf.c
SRCS += linux/lib.c
SRCS += linux/sdllink.c
SRCS += linux/sockserv.c
SRCS += linux/gl_draw.c
SRCS += mmlib/linux.c

ASMFLAGS += $(CFGDEFS)
CFLAGS += $(CFGDEFS)

OBJS := $(filter %.o, $(SRCS:.asm=.o) $(SRCS:.c=.o))
DEPS := $(OBJS:.o=.d)

.SUFFIXES:
.SUFFIXES: .asm .c .d .o

#Q ?= @

all: $(BINARY)

debug: DEBUGFLAGS += -g
debug: $(BINARY)
	gdb $(BINARY) --args zsnes ./rom.sfc

-include $(DEPS)

$(BINARY): $(OBJS)
	@echo '===> FINAL $@'
	$(Q)$(CXX) $(CFLAGS) $(OBJS) $(LDFLAGS) $(DEBUGFLAGS) -o $@

.asm.o:
	@echo '===> ASM $<'
	$(Q)$(ASM) $(ASMFLAGS) $(DEBUGFLAGS) -M -o $@ $< > $(@:.o=.d) || rm -f $(@:.o=.d)
	$(Q)$(ASM) $(ASMFLAGS) $(DEBUGFLAGS) -o $@ $<

.c.o:
	@echo '===> CC $<'
	$(Q)$(CC) $(CFLAGS) $(DEBUGFLAGS) -c -MMD -o $@ $<

%.h:
	@true

%.inc:
	@true

clean distclean:
	@echo '===> CLEAN'
	$(Q)rm -fr $(DEPS) $(OBJS) $(BINARY)
ifdef CLEAN_MORE
	$(Q)find . -name "*.[do]" -delete
endif

info:
	@echo "WITH_DEBUGGER = $(WITH_DEBUGGER)"
	@echo "WITH_PNG      = $(WITH_PNG)"
	@echo "WINDOWS       = $(WINDOWS)"
	@echo "BINARY        = $(BINARY)"
	@echo "ASM           = $(ASM)"
	@echo "CC            = $(CC)"
	@echo "CXX           = $(CXX)"
	@echo "CXX_HOST      = $(CXX_HOST)"
	@echo "PNG_CONFIG    = $(PNG_CONFIG)"
	@echo "CFLAGS_PNG    = $(CFLAGS_PNG)"
	@echo "LDFLAGS_PNG   = $(LDFLAGS_PNG)"
	@echo "SDL_CONFIG    = $(SDL_CONFIG)"
	@echo "CFLAGS_SDL    = $(CFLAGS_SDL)"
	@echo "LDFLAGS_SDL   = $(LDFLAGS_SDL)"
	@echo "CFLAGS        = $(CFLAGS)"
	@echo "CCFLAGS       = $(CCFLAGS)"
	@echo "LDFLAGS       = $(LDFLAGS)"