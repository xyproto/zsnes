.PHONY: clean distclean fmt info

# Supported ARCH values:
#   LINUX, FREEBSD, OPENBSD, NETBSD, DARWIN, WIN
# Backward-compatible aliases:
#   OSX -> DARWIN
#   WINDOWS -> WIN
SUPPORTED_ARCHES := LINUX FREEBSD OPENBSD NETBSD DARWIN WIN
UNIXSDL_ARCHES := LINUX FREEBSD OPENBSD NETBSD DARWIN
LEGACY_UNSUPPORTED_ARCHES := DOS BEOS AMIGA
HOST_OS := $(shell uname -s 2>/dev/null | tr '[:lower:]' '[:upper:]')

ARCH ?= $(shell uname -s 2>/dev/null | tr '[:lower:]' '[:upper:]')
override ARCH := $(shell printf '%s' "$(ARCH)" | tr '[:lower:]' '[:upper:]')

ifeq ($(ARCH),OSX)
ARCH := DARWIN
endif
ifeq ($(ARCH),WINDOWS)
ARCH := WIN
endif
ifeq ($(ARCH),WIN32)
ARCH := WIN
endif

ifneq ($(filter $(ARCH),$(LEGACY_UNSUPPORTED_ARCHES)),)
$(error Unsupported ARCH '$(ARCH)': DOS/BeOS/Amiga support has been removed)
endif
ifeq ($(filter $(ARCH),$(SUPPORTED_ARCHES)),)
$(error Unsupported ARCH '$(ARCH)'. Supported values: $(SUPPORTED_ARCHES))
endif

# Use all available cores by default unless user already passed -j/--jobs.
ifeq ($(filter -j% --jobs%,$(MAKEFLAGS)),)
  NPROC ?= $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
  MAKEFLAGS += -j$(NPROC)
endif

CC ?= gcc
CXX ?= g++
CXX_HOST ?= $(CXX)
CC_TARGET  ?= $(CC)
CXX_TARGET ?= $(CXX)
CC_TARGET_TRIPLE := $(shell $(CC_TARGET) -dumpmachine 2>/dev/null)
WIN_PORT_AVAILABLE := $(if $(wildcard win/c_winintrf.c),yes,)

ifeq ($(ARCH),FREEBSD)
ifeq ($(findstring freebsd,$(CC_TARGET_TRIPLE)),)
$(error ARCH=FREEBSD requires a FreeBSD-targeting compiler (current CC_TARGET triple: '$(CC_TARGET_TRIPLE)'))
endif
endif

ifeq ($(ARCH),OPENBSD)
ifeq ($(findstring openbsd,$(CC_TARGET_TRIPLE)),)
$(error ARCH=OPENBSD requires an OpenBSD-targeting compiler (current CC_TARGET triple: '$(CC_TARGET_TRIPLE)'))
endif
endif

ifeq ($(ARCH),NETBSD)
ifeq ($(findstring netbsd,$(CC_TARGET_TRIPLE)),)
$(error ARCH=NETBSD requires a NetBSD-targeting compiler (current CC_TARGET triple: '$(CC_TARGET_TRIPLE)'))
endif
endif

ifeq ($(ARCH),DARWIN)
ifeq ($(or $(findstring darwin,$(CC_TARGET_TRIPLE)),$(findstring apple,$(CC_TARGET_TRIPLE))),)
$(error ARCH=DARWIN requires a Darwin-targeting compiler (current CC_TARGET triple: '$(CC_TARGET_TRIPLE)'))
endif
endif

ifeq ($(ARCH),WIN)
ifeq ($(or $(findstring mingw,$(CC_TARGET_TRIPLE)),$(findstring windows,$(CC_TARGET_TRIPLE)),$(findstring cygwin,$(CC_TARGET_TRIPLE)),$(findstring msys,$(CC_TARGET_TRIPLE))),)
$(error ARCH=WIN requires a Windows-targeting compiler (current CC_TARGET triple: '$(CC_TARGET_TRIPLE)'))
endif
ifeq ($(WIN_PORT_AVAILABLE),)
$(error ARCH=WIN requested, but required win/ source files are missing in this tree)
endif
endif

COMMON_FLAGS = -m32 -pthread -no-pie -O1 -fno-inline -fno-pic -mtune=generic -D_FORTIFY_SOURCE=2 -mno-sse -mno-sse2 -ffunction-sections -fdata-sections -Wfatal-errors -w

# TODO: FreeBSD has a patch for being able to build without -fcommon
CFLAGS += $(COMMON_FLAGS) -std=gnu99 -fcommon
CXXFLAGS += $(COMMON_FLAGS) -std=gnu++14
LDFLAGS += -Wl,--as-needed -no-pie -Wl,--gc-sections -lz
# -O1 is mandatory
ASMFLAGS += -O1 -w-orphan-labels -w-number-deprecated-hex -w-pp-macro-params-legacy

WITH_AO       :=
#WITH_DEBUGGER := yes
WITH_JMA      := yes
WITH_OPENGL   := yes
WITH_PNG      := yes
WITH_SDL      := $(if $(filter $(ARCH),$(UNIXSDL_ARCHES)),yes,)
WITH_PIPEWIRE :=
WITH_AO       :=

# Check that pkg-config deps are also linkable with the current target flags (for example, -m32).
define detect_pkg_for_target
$(shell \
  if pkg-config --exists $(1) >/dev/null 2>&1; then \
    printf 'int main(void){return 0;}\n' | \
      $(or $(CC_TARGET),$(CC)) $(COMMON_FLAGS) -x c - -o /dev/null $$(pkg-config --libs $(1)) >/dev/null 2>&1 && \
      echo yes; \
  fi)
endef

PIPEWIRE_AVAILABLE :=
AO_AVAILABLE :=
SDL3_AVAILABLE :=
SDL2_AVAILABLE :=
SDL_BACKEND_AVAILABLE :=

ifneq ($(filter $(ARCH),$(UNIXSDL_ARCHES)),)
ifeq ($(ARCH),LINUX)
PIPEWIRE_AVAILABLE := $(call detect_pkg_for_target,libpipewire-0.3)
endif
ifneq ($(filter $(ARCH),LINUX FREEBSD OPENBSD NETBSD DARWIN WIN),)
AO_AVAILABLE := $(call detect_pkg_for_target,ao)
endif
SDL3_AVAILABLE := $(call detect_pkg_for_target,sdl3)
SDL2_AVAILABLE := $(call detect_pkg_for_target,sdl2)
SDL_BACKEND_AVAILABLE := $(if $(or $(SDL3_AVAILABLE),$(SDL2_AVAILABLE),$(strip $(SDL_CONFIG)),$(strip $(CFLAGS_SDL)),$(strip $(LDFLAGS_SDL))),yes)
endif

SKIP_AUDIO_BACKEND_CHECK := $(if $(filter clean distclean,$(MAKECMDGOALS)),yes)

ifeq ($(WITH_PIPEWIRE),)
  ifeq ($(ARCH),LINUX)
    ifeq ($(PIPEWIRE_AVAILABLE),yes)
      WITH_PIPEWIRE := yes
    endif
  endif
endif

ifeq ($(WITH_AO),)
  ifeq ($(AO_AVAILABLE),yes)
    WITH_AO := yes
  endif
endif

ifeq ($(SKIP_AUDIO_BACKEND_CHECK),)
  ifeq ($(WITH_SDL),yes)
    ifeq ($(SDL_BACKEND_AVAILABLE),)
      $(error No SDL backend available. Install SDL3 or SDL2 (32-bit, since this build uses -m32))
    endif
  endif
  ifneq ($(ARCH),WIN)
  ifeq ($(if $(or $(PIPEWIRE_AVAILABLE),$(AO_AVAILABLE),$(if $(WITH_SDL),$(SDL_BACKEND_AVAILABLE),)),yes),)
    $(error No audio backend available. Install one of: PipeWire (libpipewire-0.3), libao, SDL3 or SDL2)
  endif
  endif
endif

BINARY     ?= zsnes
ifeq ($(ARCH),WIN)
BINARY := zsnes.exe
endif
PSR        ?= parsegen.py
ASM        ?= nasm
PYTHON     ?= python3

DESTDIR ?=
PREFIX ?= /usr

ifneq ($(filter $(ARCH),LINUX FREEBSD OPENBSD NETBSD),)
  CFLAGS += -rdynamic
  CXXFLAGS += -rdynamic
  LDFLAGS += -ldl -lX11
endif
ifeq ($(ARCH),DARWIN)
ifneq ($(HOST_OS),DARWIN)
  CFLAGS += -rdynamic
  CXXFLAGS += -rdynamic
  LDFLAGS += -ldl -lX11
endif
endif
ifeq ($(ARCH),LINUX)
  CFLAGS += -L/usr/lib32
  CXXFLAGS += -L/usr/lib32
  LDFLAGS += -L/usr/lib32
endif

ifeq ($(WITH_SDL),yes)
  ifeq ($(strip $(SDL_CONFIG)),)
    ifeq ($(SDL3_AVAILABLE),yes)
      SDL_CONFIG := pkg-config sdl3
      SDL_PKG := sdl3
    else ifeq ($(SDL2_AVAILABLE),yes)
      SDL_CONFIG := pkg-config sdl2
      SDL_PKG := sdl2
    endif
  else
    SDL_PKG := $(lastword $(SDL_CONFIG))
  endif
  ifneq ($(strip $(SDL_CONFIG)),)
    ifndef CFLAGS_SDL
      CFLAGS_SDL := $(shell $(SDL_CONFIG) --cflags)
    endif
    ifndef LDFLAGS_SDL
      LDFLAGS_SDL := $(shell $(SDL_CONFIG) --libs)
    endif
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

ifeq ($(WITH_AO),yes)
  AO_CONFIG ?= pkg-config ao
  ifndef CFLAGS_AO
    CFLAGS_AO := $(shell $(AO_CONFIG) --cflags)
  endif
  ifndef LDFLAGS_AO
    LDFLAGS_AO := $(shell $(AO_CONFIG) --libs)
  endif
  ifeq ($(strip $(LDFLAGS_AO)),)
    LDFLAGS_AO := -lao
  endif
  CFLAGS += $(CFLAGS_AO)
  LDFLAGS += $(LDFLAGS_AO)
else
  CFGDEFS += -DNO_AO
endif

ifeq ($(WITH_PIPEWIRE),yes)
  ifeq ($(PIPEWIRE_AVAILABLE),yes)
    PIPEWIRE_CONFIG ?= pkg-config libpipewire-0.3
    ifndef CFLAGS_PIPEWIRE
      CFLAGS_PIPEWIRE := $(shell $(PIPEWIRE_CONFIG) --cflags)
    endif
    ifndef LDFLAGS_PIPEWIRE
      LDFLAGS_PIPEWIRE := $(shell $(PIPEWIRE_CONFIG) --libs)
    endif
    ifeq ($(strip $(LDFLAGS_PIPEWIRE)),)
      LDFLAGS_PIPEWIRE := -lpipewire-0.3
    endif
    CFLAGS += $(CFLAGS_PIPEWIRE)
    LDFLAGS += $(LDFLAGS_PIPEWIRE)
    CFGDEFS += -D__PIPEWIRE__
  else
    WITH_PIPEWIRE :=
  endif
endif

ifeq ($(ARCH),LINUX)
ifeq ($(wildcard /usr/lib/i386-linux-gnu/.),)
  CFLAGS += -I/usr/include/x86_64-linux-gnu -I /usr/include/X11
  CXXFLAGS += -I/usr/include/x86_64-linux-gnu -I /usr/include/X11
  ifeq ($(SDL_PKG),sdl3)
    LDFLAGS += -L/usr/lib/i386-linux-gnu -lSDL3 -lpng16 -lX11
  else
    LDFLAGS += -L/usr/lib/i386-linux-gnu -lSDL2 -lpng16 -lX11
  endif
endif
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
SRCS += chips/dsp3proc.c
SRCS += chips/dsp4emu.c
SRCS += chips/dsp4proc.c
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
SRCS += cpu/dma.c
SRCS += cpu/dsp.asm
SRCS += cpu/dspproc.asm
SRCS += cpu/execute.asm
SRCS += cpu/executec.c
SRCS += cpu/irq.asm
SRCS += cpu/memory.asm
SRCS += cpu/memtable.c
SRCS += cpu/spc700.asm
SRCS += cpu/stable.asm
SRCS += cpu/c_stable.c
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
SRCS += video/2xsaiw.asm
SRCS += video/c_2xsaiw.c
SRCS += video/c_makev16b.c
SRCS += video/c_makevid.c
SRCS += video/c_mode716.c
SRCS += video/c_newgfx16.c
SRCS += video/copyvwin.c
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
SRCS += video/mode716.asm
SRCS += video/mode716b.asm
SRCS += video/mode716d.asm
SRCS += video/mode716e.asm
SRCS += video/mode716t.asm
SRCS += video/mv16tms.asm
SRCS += video/newg162.asm
SRCS += video/newgfx.asm
SRCS += video/newgfx16.asm
SRCS += video/ntsc.c
SRCS += video/procvid.c
SRCS += video/procvidc.c
SRCS += video/sw_draw.c
SRCS += zdir.c
SRCS += zip/unzip.c
SRCS += zip/zpng.c
SRCS += zloader.c
SRCS += zmovie.c
SRCS += zpath.c
SRCS += zstate.c
SRCS += ztimec.c

PSRS :=
PSRS += cfg.psr
PSRS += input.psr
PSRS += md.psr

ifdef WITH_DEBUGGER
SRCS += debugasm.c
SRCS += debugger.c
LDFLAGS += -lcurses
else
CFGDEFS += -DNO_DEBUGGER
endif

DEBUGFLAGS :=

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

ifeq ($(WITH_AO),yes)
CFGDEFS += -D__LIBAO__
endif

ifneq ($(filter $(ARCH),$(UNIXSDL_ARCHES)),)
SRCS += linux/audio.c
SRCS += linux/battery.c
SRCS += linux/c_sdlintrf.c
SRCS += linux/lib.c
SRCS += linux/safelib.c

SRCS += linux/sdllink.c
SRCS += linux/sockserv.c
SRCS += linux/sw_draw.c

ifdef WITH_OPENGL
SRCS += linux/gl_draw.c
endif

CFGDEFS += -D__UNIXSDL__
ifeq ($(ARCH),LINUX)
CFGDEFS += -D__ZSNES_PLATFORM_LINUX__
endif
ifneq ($(filter $(ARCH),FREEBSD OPENBSD NETBSD),)
CFGDEFS += -D__BSDSDL__
endif
ifeq ($(ARCH),FREEBSD)
CFGDEFS += -D__ZSNES_PLATFORM_FREEBSD__
endif
ifeq ($(ARCH),OPENBSD)
CFGDEFS += -D__ZSNES_PLATFORM_OPENBSD__
endif
ifeq ($(ARCH),NETBSD)
CFGDEFS += -D__ZSNES_PLATFORM_NETBSD__
endif
ifeq ($(SDL_PKG),sdl3)
CFGDEFS += -D__SDL3__ -DSDL_ENABLE_OLD_NAMES
endif


ifeq ($(ARCH),DARWIN)
CFGDEFS += -D__ZSNES_PLATFORM_DARWIN__
ifeq ($(HOST_OS),DARWIN)
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

ASMFLAGS += -felf32 -DELF
ifdef WITH_OPENGL
LDFLAGS += -lGL
endif
endif
else
SRCS += mmlib/linux.c

ASMFLAGS += -felf32 -DELF

ifdef WITH_OPENGL
LDFLAGS += -lGL
endif

endif
endif

ifeq ($(ARCH),WIN)
SRCS += mmlib/windows.c
SRCS += win/c_winintrf.c
SRCS += win/dx_ddraw.cpp
SRCS += win/lib.c
SRCS += win/safelib.c
SRCS += win/winintrf.asm
SRCS += win/winlink.cpp

LDFLAGS += -ldxguid -ldinput -lgdi32 -lole32 -lwinmm

ifdef WITH_OPENGL
SRCS += win/gl_draw.c
LDFLAGS += -lopengl32
endif

PSRS += win/confloc.psr

ASMFLAGS += -fwin32

CFGDEFS += -D__WIN32__
CFGDEFS += -D__ZSNES_PLATFORM_WINDOWS__
endif

ASMFLAGS += $(CFGDEFS)
CFLAGS += $(CFGDEFS)
CXXFLAGS += $(CFGDEFS)
DEPFLAGS_C = -MMD -MP -MF $(@:.o=.d) -MT $@
DEPFLAGS_CXX = -MMD -MP -MF $(@:.o=.d) -MT $@

HDRS := $(PSRS:.psr=.h)
OBJS := $(filter %.o, $(SRCS:.asm=.o) $(SRCS:.c=.o) $(SRCS:.cpp=.o) $(PSRS:.psr=.o))
DEPS := $(OBJS:.o=.d)

.SUFFIXES:

#Q ?= @

all: $(BINARY)

debug: DEBUGFLAGS += -g
debug: $(BINARY)
	gdb $(BINARY) --args zsnes ~/roms/snes/example.sfc

-include $(wildcard $(DEPS))

$(BINARY): $(OBJS)
	@echo '===> LD $@'
	$(Q)$(CXX_TARGET) $(CFLAGS) $(OBJS) $(LDFLAGS) $(DEBUGFLAGS) -o $@

%.o: %.asm
	@echo '===> ASM $<'
	$(Q)$(ASM) $(ASMFLAGS) $(DEBUGFLAGS) -M -o $@ $< > $(@:.o=.d) || rm -f $(@:.o=.d)
	$(Q)$(ASM) $(ASMFLAGS) $(DEBUGFLAGS) -o $@ $<

$(filter %.o, $(SRCS:.c=.o) $(SRCS:.cpp=.o)): $(HDRS)

%.o: %.c
	@echo '===> CC $<'
	$(Q)$(CC_TARGET) $(CFLAGS) $(DEBUGFLAGS) -c $(DEPFLAGS_C) -o $@ $<

%.o: %.cpp
	@echo '===> CXX $<'
	$(Q)$(CXX_TARGET) $(CXXFLAGS) $(DEBUGFLAGS) -c $(DEPFLAGS_CXX) -o $@ $<

%.h %.o: %.psr $(PSR)
	@echo '===> PSR $@'
	$(Q)$(PYTHON) ./$(PSR) $(CFGDEFS) -gcc $(CC_TARGET) -compile -flags '$(CFLAGS)' -cheader $*.h -fname $(*F) $*.o $*.psr

%.h:
	@true

%.inc:
	@true

clean distclean:
	@echo '===> CLEAN'
	$(Q)rm -fr $(HDRS) $(DEPS) $(OBJS) $(BINARY) zsnes zsnes.exe
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
	@echo "WITH_PIPEWIRE = $(WITH_PIPEWIRE)"
	@echo "WITH_AO       = $(WITH_AO)"
	@echo "SDL3_AVAILABLE = $(SDL3_AVAILABLE)"
	@echo "SDL2_AVAILABLE = $(SDL2_AVAILABLE)"
	@echo "PIPEWIRE_AVAILABLE = $(PIPEWIRE_AVAILABLE)"
	@echo "AO_AVAILABLE  = $(AO_AVAILABLE)"
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
	@echo "SDL_PKG       = $(SDL_PKG)"
	@echo "CFLAGS_SDL    = $(CFLAGS_SDL)"
	@echo "LDFLAGS_SDL   = $(LDFLAGS_SDL)"
	@echo "CFLAGS        = $(CFLAGS)"
	@echo "CCFLAGS       = $(CCFLAGS)"
	@echo "CXXFLAGS      = $(CXXFLAGS)"
	@echo "LDFLAGS       = $(LDFLAGS)"

fmt:
	@./fmt.sh

install:
	install -Dm755 zsnes '$(DESTDIR)$(PREFIX)/bin/zsnes'
	for ICON_SIZE in 16x16 32x32 48x48 64x64 128x128; do \
		install -Dm644 icons/$${ICON_SIZE}x32.png "$(DESTDIR)$(PREFIX)/share/icons/hicolor/$$ICON_SIZE/apps/io.github.xyproto.zsnes.png" ; \
	done
	install -Dm755 linux/zsnes.desktop '$(DESTDIR)$(PREFIX)/share/applications/io.github.xyproto.zsnes.desktop'
	install -Dm755 linux/io.github.xyproto.zsnes.metainfo.xml -t '$(DESTDIR)$(PREFIX)/share/metainfo'
	install -Dm644 man/zsnes.1 '$(DESTDIR)$(PREFIX)/share/man/man1/zsnes.1'
