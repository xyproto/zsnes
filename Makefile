.PHONY: clean distclean fmt info test unused portcheck

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
CC_TARGET  ?= $(CC)
WINDRES ?= windres
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

# Target word size and instruction set, separate from the OS above. The
# emulator was written for 32-bit x86, but nothing in the tree is any more:
# every combination below builds the same C. See "make help".
BITS ?= 32
CPU  ?= x86

ifeq ($(filter $(BITS),32 64),)
$(error Unsupported BITS '$(BITS)'. Supported values: 32 64)
endif
ifeq ($(filter $(CPU),x86 arm64),)
$(error Unsupported CPU '$(CPU)'. Supported values: x86 arm64)
endif
ifeq ($(CPU),arm64)
override BITS := 64
endif

ARCH_CFLAGS :=
ifeq ($(CPU),x86)
ifneq ($(filter $(ARCH),LINUX WIN),)
ARCH_CFLAGS += -m$(BITS)
endif
endif
ifeq ($(CPU),arm64)
ARCH_CFLAGS += $(ARM64_CFLAGS)
endif

IS_FEDORA       := $(if $(wildcard /etc/fedora-release),yes)
IS_DEBIAN_BASED := $(if $(wildcard /etc/debian_version),yes)

WARN_FLAGS ?= -Wall -Wno-address-of-packed-member
COMMON_FLAGS = $(ARCH_CFLAGS) -pthread -std=c11 -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L -O3 -D_FORTIFY_SOURCE=2 -ffunction-sections -fdata-sections -Wfatal-errors $(WARN_FLAGS)

CFLAGS += $(COMMON_FLAGS)
# x87-only maths, to keep the 32-bit build's floating point exactly what the
# assembly assumed. Not available on x86-64, where SSE is the ABI for returning
# a float, nor on ARM.
ifeq ($(CPU)/$(BITS),x86/32)
ifneq ($(ARCH),DARWIN)
CFLAGS += -mno-sse -mno-sse2
endif
endif
LDFLAGS += -Wl,--as-needed -Wl,--gc-sections -lz -lm

#WITH_DEBUGGER := yes
WITH_OPENGL   := yes
WITH_PNG      := yes
WITH_SDL      := $(if $(filter $(ARCH),$(UNIXSDL_ARCHES)),yes,)
WITH_PIPEWIRE :=
WITH_AO       :=

# Add more pkg-config paths, with Fedora and Debian/Ubuntu in mind
ifneq ($(filter $(ARCH),LINUX),)
ifneq ($(filter -m32,$(ARCH_CFLAGS)),)
export PKG_CONFIG_PATH := /usr/lib/pkgconfig:/usr/lib/i386-linux-gnu/pkgconfig:/usr/share/pkgconfig:$(PKG_CONFIG_PATH)
endif
endif

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
SDL_BACKEND_AVAILABLE :=

ifneq ($(filter $(ARCH),$(UNIXSDL_ARCHES)),)
ifeq ($(ARCH),LINUX)
PIPEWIRE_AVAILABLE := $(call detect_pkg_for_target,libpipewire-0.3)
endif
ifneq ($(filter $(ARCH),LINUX FREEBSD OPENBSD NETBSD DARWIN WIN),)
AO_AVAILABLE := $(call detect_pkg_for_target,ao)
endif
SDL3_AVAILABLE := $(call detect_pkg_for_target,sdl3)
SDL_BACKEND_AVAILABLE := $(if $(or $(SDL3_AVAILABLE),$(strip $(SDL_CONFIG)),$(strip $(CFLAGS_SDL)),$(strip $(LDFLAGS_SDL))),yes)
endif

# The wrapper targets below re-invoke make with a different ARCH/BITS/CPU, so
# checking the *current* configuration's libraries first is both useless and
# wrong: it is what made "make linux64" demand a 32-bit SDL. They build nothing
# themselves, so skip the checks and let the sub-make do them.
WRAPPER_GOALS := clean distclean linux32 linux64 linux_arm64 linux_pi4 \
                 win32 w32 win64 portcheck portasm help test fmt unused
# An explicit WITH_SDL=/WITH_PIPEWIRE=/WITH_AO= on the command line is a
# deliberate "link it without that backend", not a missing package.
BACKENDS_OPTOUT := $(if $(filter command line,$(origin WITH_SDL) \
                     $(origin WITH_PIPEWIRE) $(origin WITH_AO)),yes)
SKIP_AUDIO_BACKEND_CHECK := $(if $(or \
    $(filter $(WRAPPER_GOALS),$(MAKECMDGOALS)),$(BACKENDS_OPTOUT)),yes)

# A cross build cannot expect the host's libraries: pkg-config finds nothing
# for the target unless a sysroot is installed. Compile without them rather
# than refusing - the point of those targets is to build the tree for another
# CPU, and the binary is not runnable on this machine anyway. A native build
# really is missing a package, so it still stops and says which.
# -m32 on an x86-64 host is not "cross" here: multilib is the normal way to
# get those libraries, and the advice below is right for it.
CROSS_BUILD := $(if $(or $(filter arm64,$(CPU)),$(filter WIN,$(ARCH))),yes,)

ifeq ($(SKIP_AUDIO_BACKEND_CHECK),)
ifeq ($(CROSS_BUILD),yes)
ifeq ($(SDL_BACKEND_AVAILABLE),)
$(info ===> no SDL for $(CPU)/$(ARCH); building without a video backend)
WITH_SDL :=
endif
endif
endif

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
      ifeq ($(ARCH),LINUX)
        $(info )
        $(info ERROR: No SDL library found for this target ($(CPU)/$(BITS)).)
        ifeq ($(BITS),32)
        ifeq ($(IS_FEDORA),yes)
        $(info Install the 32-bit SDL3 package (note the .i686 suffix, NOT the x86_64 package):)
        $(info   sudo dnf install SDL3-devel.i686)
        else ifeq ($(IS_DEBIAN_BASED),yes)
        $(info Enable 32-bit support and install the 32-bit SDL3 package (note the :i386 suffix):)
        $(info   sudo dpkg --add-architecture i386 && sudo apt update)
        $(info   sudo apt install libsdl3-dev:i386)
        else
        $(info Install the 32-bit SDL3 development package for your distribution.)
        endif
        else
        ifeq ($(IS_FEDORA),yes)
        $(info   sudo dnf install SDL3-devel)
        else ifeq ($(IS_DEBIAN_BASED),yes)
        $(info   sudo apt install libsdl3-dev)
        else
        $(info Install the SDL3 development package for your distribution.)
        endif
        endif
        $(info )
        $(info Or build without a video backend, which links but cannot draw:)
        $(info   make $(MAKECMDGOALS) WITH_SDL= WITH_PIPEWIRE= WITH_AO=)
        $(info )
        $(error No SDL library for $(CPU)/$(BITS). See above.)
      else
        $(error No SDL backend available. Install SDL3 for ARCH=$(ARCH))
      endif
    endif
  endif
  ifneq ($(ARCH),WIN)
  ifneq ($(CROSS_BUILD),yes)
  ifeq ($(if $(or $(PIPEWIRE_AVAILABLE),$(AO_AVAILABLE),$(if $(WITH_SDL),$(SDL_BACKEND_AVAILABLE),)),yes),)
    ifeq ($(ARCH),LINUX)
      $(info )
      $(info ERROR: No audio backend found for this target ($(CPU)/$(BITS)).)
      ifeq ($(IS_FEDORA),yes)
      $(info Install one of the following 32-bit packages (note the .i686 suffix, NOT the x86_64 packages):)
      $(info   sudo dnf install pipewire-devel.i686)
      $(info   sudo dnf install libao-devel.i686)
      $(info   sudo dnf install SDL3-devel.i686)
      else ifeq ($(IS_DEBIAN_BASED),yes)
      $(info Enable 32-bit support, then install one of the following 32-bit packages (note the :i386 suffix):)
      $(info   sudo dpkg --add-architecture i386 && sudo apt update)
      $(info   sudo apt install libpipewire-0.3-dev:i386)
      $(info   sudo apt install libao-dev:i386)
      $(info   sudo apt install libsdl3-dev:i386)
      else
      $(info Install the 32-bit development package for one of: PipeWire, libao, or SDL3.)
      endif
      $(info )
      $(info Or build without audio, which links but is silent:)
      $(info   make $(MAKECMDGOALS) WITH_SDL= WITH_PIPEWIRE= WITH_AO=)
      $(info )
      $(error No audio backend for $(CPU)/$(BITS). See above.)
    else
      $(error No audio backend available. Install one of: PipeWire (libpipewire-0.3), libao, or SDL3)
    endif
  endif
  endif
  endif
endif

BINARY     ?= zsnes
ifeq ($(ARCH),WIN)
BINARY := zsnes.exe
endif
PSR        ?= parsegen.py
PYTHON     ?= python3

DESTDIR ?=
PREFIX ?= /usr

ifneq ($(filter $(ARCH),LINUX FREEBSD OPENBSD NETBSD),)
  CFLAGS += -rdynamic
  LDFLAGS += -ldl
endif
ifeq ($(ARCH),DARWIN)
ifneq ($(HOST_OS),DARWIN)
  CFLAGS += -rdynamic
  LDFLAGS += -ldl
endif
endif
ifeq ($(ARCH),LINUX)
  CFLAGS += -L/usr/lib32
  LDFLAGS += -L/usr/lib32
endif

ifeq ($(WITH_SDL),yes)
  ifeq ($(strip $(SDL_CONFIG)),)
    ifeq ($(SDL3_AVAILABLE),yes)
      SDL_CONFIG := pkg-config sdl3
      SDL_PKG := sdl3
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

# libpng must come from the target's pkg-config (the mingw32 one for
# "make win32"); fall back to a PNG-less build when the target lacks it.
PKG_CONFIG ?= pkg-config
ifdef WITH_PNG
  ifeq ($(origin PNG_CONFIG),undefined)
    ifneq ($(shell $(PKG_CONFIG) --exists libpng >/dev/null 2>&1 && echo yes),yes)
      WITH_PNG :=
      $(info ===> libpng for the target not found via '$(PKG_CONFIG)'; building without PNG support)
      ifeq ($(ARCH),WIN)
        $(info ===> for PNG support, install the mingw32 libpng (Arch Linux: mingw-w64-libpng from the AUR))
      endif
    endif
  endif
endif
ifdef WITH_PNG
  PNG_CONFIG ?= $(PKG_CONFIG) libpng
  ifndef CFLAGS_PNG
    CFLAGS_PNG  := $(shell $(PNG_CONFIG) --cflags)
  endif
  ifndef LDFLAGS_PNG
    # the win32 link is static: let pkg-config order zlib after libpng
    ifeq ($(ARCH),WIN)
      LDFLAGS_PNG := $(shell $(PNG_CONFIG) --static --libs)
    else
      LDFLAGS_PNG := $(shell $(PNG_CONFIG) --libs)
    endif
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
  CFGDEFS += -D__LIBAO__
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
  CFLAGS += -I/usr/include/x86_64-linux-gnu
endif
endif

SRCS :=
SRCS += c_init.c
SRCS += c_vcache.c
SRCS += chips/7110emu.c
SRCS += chips/7110proc.c
SRCS += chips/msu1emu.c
SRCS += chips/msu1regs.c
SRCS += chips/c4emu.c
SRCS += chips/c4proc.c
SRCS += chips/c_dsp2proc.c
SRCS += chips/c_sa1regs.c
SRCS += chips/c_sfxproc.c
SRCS += chips/c_fxdata.c
SRCS += chips/c_fxops.c
SRCS += chips/c_fxtable.c
SRCS += chips/dsp1emu.c
SRCS += chips/dsp1proc.c
SRCS += chips/dsp2proc.c
SRCS += chips/dsp3emu.c
SRCS += chips/dsp3proc.c
SRCS += chips/dsp4emu.c
SRCS += chips/dsp4proc.c
SRCS += chips/obc1emu.c
SRCS += chips/obc1proc.c
SRCS += chips/sa1emu.c
SRCS += chips/c_sa1proc.c
SRCS += chips/c_sa1data.c
SRCS += chips/sa1regs.c
SRCS += chips/sdd1emu.c
SRCS += chips/seta10.c
SRCS += chips/seta11.c
SRCS += chips/sfxproc.c
SRCS += chips/st10proc.c
SRCS += chips/st11proc.c
SRCS += cpu/c_65816d.c
SRCS += cpu/c_dma.c
SRCS += cpu/c_dsp.c
SRCS += cpu/c_dspproc.c
SRCS += cpu/c_execute.c
SRCS += cpu/c_execloop.c
SRCS += cpu/c_execdata.c
SRCS += cpu/c_irq.c
SRCS += cpu/c_memory.c
SRCS += cpu/c_memops.c
SRCS += cpu/c_ops65816.c
SRCS += cpu/c_ops65816_sa1.c
SRCS += cpu/c_ops65816_dbg.c
SRCS += cpu/c_sflags.c
SRCS += cpu/c_execirq.c
SRCS += cpu/c_regsdata.c
SRCS += cpu/c_regswdata.c
SRCS += cpu/c_regs.c
SRCS += cpu/c_regsppu.c
SRCS += cpu/c_regsw.c
SRCS += cpu/c_rewind.c
SRCS += cpu/c_spc700.c
SRCS += cpu/c_spcdata.c
SRCS += cpu/c_table.c
SRCS += cpu/c_tablec.c
SRCS += cpu/dma.c
SRCS += cpu/dspproc.c
SRCS += cpu/firtable.c
SRCS += cpu/executec.c
SRCS += cpu/memtable.c
SRCS += cpu/c_stable.c
SRCS += effects/burn.c
SRCS += effects/smoke.c
SRCS += effects/water.c
SRCS += endmem.c
SRCS += gui/c_gui.c
SRCS += gui/c_guiwindp.c
SRCS += gui/c_gui_data.c
SRCS += gui/guicheat.c
SRCS += gui/guicombo.c
SRCS += gui/guifuncs.c
SRCS += gui/guikeys.c
SRCS += gui/guimisc.c
SRCS += gui/guimouse.c
SRCS += gui/guitools.c
SRCS += gui/menu.c
SRCS += initdata.c
SRCS += initc.c
SRCS += mmlib/mm.c
SRCS += patch.c
SRCS += ui.c
SRCS += ver.c
SRCS += video/2xsaiw.c
SRCS += video/c_2xsaiw.c
SRCS += video/c_makev16b.c
SRCS += video/c_makevid.c
SRCS += video/tilecache.c
SRCS += video/sprites.c
SRCS += video/vcache_data.c
SRCS += video/c_mode716.c
SRCS += video/c_mode716data.c
SRCS += video/c_mode716calc.c
SRCS += video/c_mode716ext2.c
SRCS += video/c_mode716start.c
SRCS += video/c_mode716win.c
SRCS += video/c_mode716bw.c
SRCS += video/c_mode716proc.c
SRCS += video/c_procwin.c
SRCS += video/c_m716gate.c
SRCS += video/c_mv16draw.c
SRCS += video/c_mv16msgate.c
SRCS += video/c_mv16leaf.c
SRCS += video/c_mode716draw.c
SRCS += video/c_mode716gate.c
SRCS += video/c_mv16tms.c
SRCS += video/c_mv16tsms.c
SRCS += video/c_mv16tm7.c
SRCS += video/c_mv16tclr.c
SRCS += video/c_mv16bclr.c
SRCS += video/c_mv16tspr.c
SRCS += video/c_mv16tsprt.c
SRCS += video/c_mv16tsprp.c
SRCS += video/c_mv16t8bt.c
SRCS += video/c_mv16t16bt.c
SRCS += video/c_mv16t8t.c
SRCS += video/c_mv16t16t.c
SRCS += video/c_mv16t8to.c
SRCS += video/c_mv16thi.c
SRCS += video/c_mv16tline.c
SRCS += video/c_ngmosaic.c
SRCS += video/c_ngprocbg.c
SRCS += video/c_ngline.c
SRCS += video/c_ngspr.c
SRCS += video/c_ng2gate.c
SRCS += video/c_ng2tile.c
SRCS += video/c_ngbg.c
SRCS += video/c_ngframe.c
SRCS += video/c_ngtransp.c
SRCS += video/c_makev16tdata.c
SRCS += video/c_newgfx16data.c
SRCS += video/c_hqx.c
SRCS += video/c_newgfx16.c
SRCS += video/copyvwin.c
SRCS += video/makevid.c
SRCS += video/mode716b.c
SRCS += video/newgfx.c
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
ifeq ($(ARCH),FREEBSD)
CFGDEFS += -D__ZSNES_PLATFORM_FREEBSD__
endif
ifeq ($(ARCH),OPENBSD)
CFGDEFS += -D__ZSNES_PLATFORM_OPENBSD__
endif
ifeq ($(ARCH),NETBSD)
CFGDEFS += -D__ZSNES_PLATFORM_NETBSD__
endif


ifeq ($(ARCH),DARWIN)
CFGDEFS += -D__ZSNES_PLATFORM_DARWIN__
ifeq ($(HOST_OS),DARWIN)
SRCS += mmlib/osx.c


CFGDEFS += -D__ZSNES_PLATFORM_DARWIN__

CFLAGS += -fno-pic

LDFLAGS += -framework Carbon -framework IOKit -framework Foundation
ifdef WITH_OPENGL
LDFLAGS += -framework OpenGL
endif
else
SRCS += mmlib/linux.c

ifdef WITH_OPENGL
LDFLAGS += -lGL
endif
endif
else
SRCS += mmlib/linux.c


ifdef WITH_OPENGL
LDFLAGS += -lGL
endif

endif
endif

ifeq ($(ARCH),WIN)
SRCS += mmlib/windows.c
SRCS += win/zsnes.rc
SRCS += win/c_winintrf.c
SRCS += win/dx_ddraw.c
SRCS += win/lib.c
SRCS += win/safelib.c
SRCS += win/winlink.c

LDFLAGS += -ldxguid -ldinput -lxinput -lgdi32 -lole32 -lwinmm

ifdef WITH_OPENGL
SRCS += win/gl_draw.c
LDFLAGS += -lopengl32
endif

LDFLAGS += --static
# clock_gettime lives in winpthread; put it after objects so --as-needed keeps it.
LDFLAGS += -lwinpthread

PSRS += win/confloc.psr


CFGDEFS += -D__WIN32__
CFGDEFS += -D__ZSNES_PLATFORM_WINDOWS__
endif

CFLAGS += $(CFGDEFS)
# Append hooks for layered flags.
CFLAGS   += $(EXTRA_CFLAGS)
LDFLAGS  += $(EXTRA_LDFLAGS)
DEPFLAGS_C = -MMD -MP -MF $(@:.o=.d) -MT $@

HDRS := $(PSRS:.psr=.h)
OBJS := $(filter %.o, $(SRCS:.c=.o) $(SRCS:.rc=.o) $(PSRS:.psr=.o))
DEPS := $(OBJS:.o=.d)

# Auto-clean on build-target switch.  Native (ELF) and win32 (PE/COFF) builds
# share the same .o paths but emit incompatible object formats, so switching
# between "make" and "make win32" used to need a manual "make clean".  Record
# the active target in a stamp file and wipe stale objects when it changes.
# This runs at parse time (before any parallel recipe), and is skipped for the
# win32 wrapper goal (its recursive "make ARCH=WIN" does the real build) and for
# maintenance goals like clean/info/fmt.
BUILDSTAMP := .buildmode
BUILD_TAG := $(ARCH)|$(CC_TARGET_TRIPLE)
ifneq ($(filter all debug test,$(or $(MAKECMDGOALS),all)),)
PREV_BUILD_TAG := $(shell cat $(BUILDSTAMP) 2>/dev/null)
ifneq ($(PREV_BUILD_TAG),)
ifneq ($(PREV_BUILD_TAG),$(BUILD_TAG))
$(info ===> build target changed ($(PREV_BUILD_TAG) -> $(BUILD_TAG)), cleaning stale objects)
_CLEAN_SWITCH := $(shell rm -fr $(HDRS) $(DEPS) $(OBJS) $(BINARY) zsnes zsnes.exe)
endif
endif
_WRITE_STAMP := $(shell printf '%s' '$(BUILD_TAG)' > $(BUILDSTAMP))
endif

.SUFFIXES:

#Q ?= @

all: $(BINARY)

# Named targets, one per platform. Each is a thin wrapper that picks the OS,
# the word size, the instruction set and the toolchain; the build itself is
# the same C for all of them. The cross targets need their toolchain
# installed, and say which one if it is missing.
define need_tool
@command -v $(1) >/dev/null 2>&1 || { \
  echo "error: $(1) not found; install $(2)" >&2; exit 1; }
endef

MINGW32_PREFIX ?= i686-w64-mingw32
MINGW64_PREFIX ?= x86_64-w64-mingw32
ARM64_PREFIX   ?= aarch64-linux-gnu

.PHONY: linux32 linux64 linux_arm64 linux_pi4 win32 w32 win64 help

linux32:
	$(MAKE) ARCH=LINUX BITS=32 CPU=x86

linux64:
	$(MAKE) ARCH=LINUX BITS=64 CPU=x86

# Generic ARMv8-A, for any 64-bit ARM Linux.
linux_arm64:
	$(call need_tool,$(ARM64_PREFIX)-gcc,the aarch64 cross toolchain)
	$(MAKE) ARCH=LINUX CPU=arm64 \
	  CC=$(ARM64_PREFIX)-gcc CC_TARGET=$(ARM64_PREFIX)-gcc \
	  PKG_CONFIG=$(ARM64_PREFIX)-pkg-config

# A Raspberry Pi 4 is a Cortex-A72; same target as linux_arm64, tuned for it.
linux_pi4:
	$(call need_tool,$(ARM64_PREFIX)-gcc,the aarch64 cross toolchain)
	$(MAKE) ARCH=LINUX CPU=arm64 ARM64_CFLAGS='-mcpu=cortex-a72 -mtune=cortex-a72' \
	  CC=$(ARM64_PREFIX)-gcc CC_TARGET=$(ARM64_PREFIX)-gcc \
	  PKG_CONFIG=$(ARM64_PREFIX)-pkg-config

w32: win32
win32:
	$(call need_tool,$(MINGW32_PREFIX)-gcc,the mingw32 toolchain)
	$(MAKE) ARCH=WIN BITS=32 CPU=x86 CC=$(MINGW32_PREFIX)-gcc CC_TARGET=$(MINGW32_PREFIX)-gcc \
	  WINDRES=$(MINGW32_PREFIX)-windres PKG_CONFIG=$(MINGW32_PREFIX)-pkg-config

win64:
	$(call need_tool,$(MINGW64_PREFIX)-gcc,the mingw-w64 toolchain)
	$(MAKE) ARCH=WIN BITS=64 CPU=x86 CC=$(MINGW64_PREFIX)-gcc CC_TARGET=$(MINGW64_PREFIX)-gcc \
	  WINDRES=$(MINGW64_PREFIX)-windres PKG_CONFIG=$(MINGW64_PREFIX)-pkg-config

help:
	@echo 'Targets:'
	@echo '  linux32       32-bit x86 Linux'
	@echo '  linux64       64-bit x86 Linux'
	@echo '  linux_arm64   64-bit ARM Linux'
	@echo '  linux_pi4     64-bit ARM Linux, tuned for a Cortex-A72'
	@echo '  win32         32-bit Windows, cross-built with mingw32'
	@echo '  win64         64-bit Windows, cross-built with mingw-w64'
	@echo '  portcheck     compile every source for x86-64 and aarch64'
	@echo '  test          run the unit tests'
	@echo
	@echo 'The tree is C11 throughout; the cross targets need their'
	@echo 'toolchain installed and will name it if it is missing.'

debug: DEBUGFLAGS += -g
debug: $(BINARY)
	gdb $(BINARY) --args zsnes ~/roms/snes/example.sfc

-include $(wildcard $(DEPS))

$(BINARY): $(OBJS)
	@echo '===> LD $@'
	$(Q)$(CC_TARGET) $(CFLAGS) $(OBJS) $(LDFLAGS) $(DEBUGFLAGS) -o $@

$(filter %.o, $(SRCS:.c=.o)): $(HDRS)

%.o: %.c
	@echo '===> CC $<'
	$(Q)$(CC_TARGET) $(CFLAGS) $(DEBUGFLAGS) -c $(DEPFLAGS_C) -o $@ $<

%.o: %.rc
	@echo '===> RES $<'
	$(Q)$(WINDRES) -o $@ $<

%.h %.o: %.psr $(PSR)
	@echo '===> PSR $@'
	$(Q)$(PYTHON) ./$(PSR) $(CFGDEFS) -gcc $(CC_TARGET) -compile -flags '$(CFLAGS)' -cheader $*.h -fname $(*F) $*.o $*.psr

%.h:
	@true

%.inc:
	@true

clean distclean:
	@echo '===> CLEAN'
	$(Q)rm -fr $(HDRS) $(DEPS) $(OBJS) $(BINARY) zsnes zsnes.exe $(BUILDSTAMP)
ifdef CLEAN_MORE
	$(Q)find . -name "*.[do]" -delete
endif

info:
	@echo "ARCH          = $(ARCH)"
	@echo "WITH_DEBUGGER = $(WITH_DEBUGGER)"
	@echo "WITH_OPENGL   = $(WITH_OPENGL)"
	@echo "WITH_PNG      = $(WITH_PNG)"
	@echo "WITH_SDL      = $(WITH_SDL)"
	@echo "WITH_PIPEWIRE = $(WITH_PIPEWIRE)"
	@echo "WITH_AO       = $(WITH_AO)"
	@echo "SDL3_AVAILABLE = $(SDL3_AVAILABLE)"
	@echo "PIPEWIRE_AVAILABLE = $(PIPEWIRE_AVAILABLE)"
	@echo "AO_AVAILABLE  = $(AO_AVAILABLE)"
	@echo "BINARY        = $(BINARY)"
	@echo "ASM           = $(ASM)"
	@echo "CC            = $(CC)"
	@echo "CC_TARGET     = $(CC_TARGET)"
	@echo "PSR           = $(PSR)"
	@echo "WINDRES       = $(WINDRES)"
	@echo "PNG_CONFIG    = $(PNG_CONFIG)"
	@echo "CFLAGS_PNG    = $(CFLAGS_PNG)"
	@echo "LDFLAGS_PNG   = $(LDFLAGS_PNG)"
	@echo "SDL_CONFIG    = $(SDL_CONFIG)"
	@echo "SDL_PKG       = $(SDL_PKG)"
	@echo "CFLAGS_SDL    = $(CFLAGS_SDL)"
	@echo "LDFLAGS_SDL   = $(LDFLAGS_SDL)"
	@echo "CFLAGS        = $(CFLAGS)"
	@echo "LDFLAGS       = $(LDFLAGS)"

fmt:
	@./fmt.sh

test: $(BINARY)
	$(MAKE) -C test run

install:
	install -Dm755 zsnes '$(DESTDIR)$(PREFIX)/bin/zsnes'
	for ICON_SIZE in 16x16 32x32 48x48 64x64 128x128; do \
		install -Dm644 icons/$${ICON_SIZE}x32.png "$(DESTDIR)$(PREFIX)/share/icons/hicolor/$$ICON_SIZE/apps/io.github.xyproto.zsnes.png" ; \
	done
	install -Dm755 linux/zsnes.desktop '$(DESTDIR)$(PREFIX)/share/applications/io.github.xyproto.zsnes.desktop'
	install -Dm755 linux/io.github.xyproto.zsnes.metainfo.xml -t '$(DESTDIR)$(PREFIX)/share/metainfo'
	install -Dm644 man/zsnes.1 '$(DESTDIR)$(PREFIX)/share/man/man1/zsnes.1'

# Portability gate. Compiles every C source for each target below, on its own,
# which catches a layout assuming 4-byte pointers in a file the current build
# does not happen to touch. Compiling is all it checks - linux64 is what proves
# the tree links and runs.
#
# aarch64 is the one target with no x86 in it at all, and it is skipped when
# the cross compiler is not installed. It uses the host's headers on purpose:
# zlib.h and png.h are architecture independent, and there is no aarch64 build
# of either here, so this asks "does it compile for ARM" without a sysroot.
PORTCHECK_CC     ?= gcc
PORTCHECK_CFLAGS ?= -std=c11 -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L \
                    -O1 -I. $(CFGDEFS)
PORTCHECK_ARM_CC ?= aarch64-linux-gnu-gcc
.PHONY: portcheck
portcheck: $(HDRS)
	@rc=0; \
	for t in "x86-64:$(PORTCHECK_CC):-m64" "aarch64:$(PORTCHECK_ARM_CC):-I/usr/include"; do \
	  name=$${t%%:*}; rest=$${t#*:}; cc=$${rest%%:*}; extra=$${rest#*:}; \
	  command -v $$cc >/dev/null 2>&1 || { \
	    echo "===> PORTCHECK: $$name skipped, $$cc not installed"; continue; }; \
	  echo "===> PORTCHECK: compiling every C source for $$name"; \
	  ok=0; bad=0; \
	  for f in $(filter %.c,$(SRCS)); do \
	    if $$cc $(PORTCHECK_CFLAGS) $$extra -c -o /dev/null $$f 2>/tmp/zs_portcheck.$$$$; then \
	      ok=$$((ok+1)); \
	    else \
	      bad=$$((bad+1)); echo "  FAIL $$f"; \
	      grep -iE 'error' /tmp/zs_portcheck.$$$$ | head -2 | sed 's/^/        /'; \
	    fi; \
	    rm -f /tmp/zs_portcheck.$$$$; \
	  done; \
	  echo "===> PORTCHECK: $$name $$ok built, $$bad failed"; echo; \
	  [ $$bad = 0 ] || rc=1; \
	done; exit $$rc

# Detect likely-unused C/ASM code via -Wunused* + linker --gc-sections reports.
# The build already uses -ffunction-sections/-fdata-sections, so each dropped
# section maps to a function or datum with no reachable references.
UNUSED_LOG ?= unused-report.txt
UNUSED_CFLAGS  := -Wunused -Wunused-function -Wunused-variable \
                  -Wunused-but-set-variable -Wunused-label -Wunused-value \
                  -Wunused-parameter
UNUSED_LDFLAGS := -Wl,--print-gc-sections
unused:
	@echo '===> UNUSED: clean rebuild with -Wunused* and --print-gc-sections'
	$(Q)$(MAKE) --no-print-directory clean >/dev/null
	$(Q)$(MAKE) --no-print-directory \
	    WARN_FLAGS='$(UNUSED_CFLAGS)' \
	    EXTRA_LDFLAGS='$(UNUSED_LDFLAGS)' 2>&1 | tee $(UNUSED_LOG)
	@echo
	@echo '===> UNUSED: summary (full log in $(UNUSED_LOG))'
	@echo '--- C -Wunused warnings ---'
	@grep -E 'warning:.*\[-Wunused' $(UNUSED_LOG) | sort -u || true
	@echo
	@echo '--- Linker discarded sections (likely-unused functions/data) ---'
	@grep -E 'removing unused section' $(UNUSED_LOG) | sort -u || true
	@echo
	@echo 'Tip: function "foo" in a discarded ".text.foo" section has no callers'
	@echo '     reachable from main(); review before deleting (callbacks, asm refs).'
