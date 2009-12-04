CONFIG ?= config.default
-include $(CONFIG)

ifeq ($(findstring $(ARCH), DOS LINUX OSX WIN),)
$(error ARCH must be set to one of DOS, LINUX, OSX or WIN. Copy config.template to config.default and uncomment one of the architectures)
endif

BINARY     ?= zsnes
PSR        ?= parsegen
ASM        ?= nasm
CXX_HOST   ?= $(CXX)
CC_TARGET  ?= $(CC)
CXX_TARGET ?= $(CXX)

ifneq ($(findstring $(ARCH), LINUX OSX),)
	WITH_SDL := yes
endif

ifdef WITH_SDL
  SDL_CONFIG ?= sdl-config
  ifndef CFLAGS_SDL
    CFLAGS_SDL := $(shell $(SDL_CONFIG) --cflags)
  endif
  ifndef LDFLAGS_SDL
    LDFLAGS_SDL := $(shell $(SDL_CONFIG) --libs)
  endif
  CFLAGS  += $(CFLAGS_SDL)
  LDFLAGS += $(LDFLAGS_SDL)
endif

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
SRCS += chips/7110emu.c
SRCS += chips/7110proc.asm
SRCS += chips/c4emu.c
SRCS += chips/c4proc.asm
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
SRCS += cpu/c_regs.c
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
SRCS += gui/gui.asm
SRCS += gui/guifuncs.c
SRCS += gui/menu.asm
SRCS += init.asm
SRCS += initc.c
SRCS += mmlib/mm.c
SRCS += patch.c
SRCS += ui.c
SRCS += vcache.asm
SRCS += version.c
SRCS += video/2xsaiw.asm
SRCS += video/copyvwin.asm
SRCS += video/hq2x16.asm
SRCS += video/hq2x32.asm
SRCS += video/hq3x16.asm
SRCS += video/hq3x32.asm
SRCS += video/hq4x16.asm
SRCS += video/hq4x32.asm
SRCS += video/m716text.asm
SRCS += video/makev16b.asm
SRCS += video/makev16t.asm
SRCS += video/makevid.asm
#SRCS += video/mode7.asm # DOS only?
SRCS += video/mode716.asm
SRCS += video/mode716b.asm
SRCS += video/mode716d.asm
SRCS += video/mode716e.asm
SRCS += video/mode716t.asm
#SRCS += video/mode7ext.asm # DOS only?
SRCS += video/mv16tms.asm
SRCS += video/newg162.asm
SRCS += video/newgfx.asm
SRCS += video/newgfx16.asm
#SRCS += video/newgfx2.asm # DOS only?
SRCS += video/ntsc.c
SRCS += video/procvid.asm
SRCS += video/procvidc.c
SRCS += video/sw_draw.asm
SRCS += zdir.c
SRCS += zip/unzip.c
SRCS += zip/zpng.c
SRCS += zloader.c
SRCS += zmovie.c
SRCS += zpath.c
SRCS += zstate.c
SRCS += ztime.asm
SRCS += ztimec.c

PSRS :=
PSRS += cfg.psr
PSRS += input.psr
PSRS += md.psr

ifdef WITH_DEBUGGER
SRCS += debugasm.asm
SRCS += debugger.c
else
CFGDEFS += -DNO_DEBUGGER
endif

ifdef WITH_JMA
SRCS += jma/7zlzma.cpp
SRCS += jma/crc32.cpp
SRCS += jma/iiostrm.cpp
SRCS += jma/inbyte.cpp
SRCS += jma/jma.cpp
SRCS += jma/lzma.cpp
SRCS += jma/lzmadec.cpp
SRCS += jma/winout.cpp
SRCS += jma/zsnesjma.cpp
else
CFGDEFS += -DNO_JMA
endif

ifdef WITH_OPENGL
CFGDEFS += -D__OPENGL__
endif

ifeq ($(ARCH), DOS)
SRCS += dos/dosintrf.asm
SRCS += dos/gppro.asm
SRCS += dos/initvid.asm
SRCS += dos/joy.asm
SRCS += dos/lib.c
SRCS += dos/sound.asm
SRCS += dos/sw.asm
SRCS += dos/sw32.asm
SRCS += dos/vesa12.asm
SRCS += dos/vesa2.asm
endif

ifneq ($(findstring $(ARCH), LINUX OSX),)
SRCS += linux/audio.c
SRCS += linux/battery.c
SRCS += linux/lib.c
SRCS += linux/safelib.c
SRCS += linux/sdlintrf.asm
SRCS += linux/sdllink.c
SRCS += linux/sockserv.c
SRCS += linux/sw_draw.c
SRCS += linux/x11.c

ifdef WITH_OPENGL
SRCS += linux/gl_draw.c
endif

CFGDEFS += -D__UNIXSDL__

LDFLAGS += -lcurses -lz

ifeq ($(ARCH), OSX)
SRCS += mmlib/osx.c

ASMFLAGS += -fmacho -DMACHO

CFGDEFS += -D__MACOSX__

CFLAGS += -fno-pic -read_only_relocs suppress

LDFLAGS += -framework Carbon -framework IOKit -framework Foundation
ifdef WITH_OPENGL
LDFLAGS += -framework OpenGL
endif
else
SRCS += mmlib/linux.c

ASMFLAGS += -felf -DELF

ifdef WITH_OPENGL
LDFLAGS += -lGL
endif
endif
endif

ifeq ($(ARCH), WIN)
SRCS += mmlib/windows.c
SRCS += win/dx_ddraw.cpp
SRCS += win/lib.c
SRCS += win/safelib.c
SRCS += win/winintrf.asm
SRCS += win/winlink.cpp

ifdef WITH_OPENGL
SRCS += win/gl_draw.c
endif

PSRS += win/confloc.psr

ASMFLAGS += -fwin32

CFGDEFS += -D__WIN32__

LDFLAGS += -ldxguid -ldinput -lgdi32 -lole32 -lz
ifdef WITH_OPENGL
LDFLAGS += -lopengl32
endif
endif

HDRS := $(PSRS:.psr=.h)
OBJS := $(filter %.o, $(SRCS:.asm=.o) $(SRCS:.c=.o) $(SRCS:.cpp=.o) $(PSRS:.psr=.o))
DEPS := $(OBJS:.o=.d)


ASMFLAGS += $(CFGDEFS)
ASMFLAGS += -O1 # XXX mandatory, otherwise zsnes breaks
ASMFLAGS += -w-orphan-labels

CFLAGS += -m32 -mmmx
CFLAGS += $(CFGDEFS)
CFLAGS += -I .

CCFLAGS += -std=gnu99
CCFLAGS += $(CFLAGS)

CXXFLAGS += $(CFLAGS)


.SUFFIXES:
.SUFFIXES: .asm .c .cpp .d .o

Q ?= @

all: $(BINARY)

-include $(DEPS)

$(BINARY): $(OBJS)
	@echo '===> LD $@'
	$(Q)$(CXX_TARGET) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

.asm.o:
	@echo '===> ASM $<'
	$(Q)$(ASM) $(ASMFLAGS) -M -o $@ $< > $(@:.o=.d) || rm -f $(@:.o=.d)
	$(Q)$(ASM) $(ASMFLAGS) -o $@ $<

$(filter %.o, $(SRCS:.c=.o) $(SRCS:.cpp=.o)): $(HDRS)

.c.o:
	@echo '===> CC $<'
	$(Q)$(CC_TARGET) $(CCFLAGS) -c -MMD -o $@ $<

.cpp.o:
	@echo '===> CXX $<'
	$(Q)$(CXX_TARGET) $(CXXFLAGS) -c -MMD -o $@ $<

$(PSR): parsegen.cpp
	@echo '===> CXX $@'
	$(Q)$(CXX_HOST) -o $@ $< -lz

%.h %.o: %.psr $(PSR)
	@echo '===> PSR $@'
	$(Q)./$(PSR) $(CFGDEFS) -gcc $(CC_TARGET) -compile -flags "$(CFLAGS) -O1" -cheader $@ -fname $(*F) $(@:.h=.o) $<

%.h:
	@true

%.inc:
	@true

clean distclean:
	@echo '===> CLEAN'
	$(Q)rm -fr $(HDRS) $(DEPS) $(OBJS) $(BINARY) $(PSR)
ifdef CLEAN_MORE
	$(Q)find . -name "*.[do]" -delete
endif

info:
	@echo "ARCH          = $(ARCH)"
	@echo "WITH_DEBUGGER = $(WITH_DEBUGGER)"
	@echo "WITH_JMA      = $(WITH_JMA)"
	@echo "WITH_OPENGL   = $(WITH_OPENGL)"
	@echo "WITH_PNG      = $(WITH_PNG)"
	@echo "WITH_SDL      = $(WITH_SDL)"
	@echo "BINARY        = $(BINARY)"
	@echo "ASM           = $(ASM)"
	@echo "CC            = $(CC)"
	@echo "CXX           = $(CXX)"
	@echo "CXX_HOST      = $(CXX_HOST)"
	@echo "CC_TARGET     = $(CC_TARGET)"
	@echo "CXX_TARGET    = $(CXX_TARGET)"
	@echo "PSR           = $(PSR)"
	@echo "PNG_CONFIG    = $(PNG_CONFIG)"
	@echo "CFLAGS_PNG    = $(CFLAGS_PNG)"
	@echo "LDFLAGS_PNG   = $(LDFLAGS_PNG)"
	@echo "SDL_CONFIG    = $(SDL_CONFIG)"
	@echo "CFLAGS_SDL    = $(CFLAGS_SDL)"
	@echo "LDFLAGS_SDL   = $(LDFLAGS_SDL)"
	@echo "CFLAGS        = $(CFLAGS)"
	@echo "CCFLAGS       = $(CCFLAGS)"
	@echo "CXXFLAGS      = $(CXXFLAGS)"
	@echo "LDFLAGS       = $(LDFLAGS)"
