.PHONY: clean debug distclean fmt info test unused portcheck

# ARCH: LINUX, FREEBSD, OPENBSD, NETBSD, DARWIN, WIN
SUPPORTED_ARCHES := LINUX FREEBSD OPENBSD NETBSD DARWIN WIN
UNIXSDL_ARCHES := LINUX FREEBSD OPENBSD NETBSD DARWIN
LEGACY_UNSUPPORTED_ARCHES := DOS BEOS AMIGA
HOST_OS := $(shell uname -s 2>/dev/null | tr '[:lower:]' '[:upper:]')
HOST_OS := $(if $(filter MINGW% MSYS% CYGWIN%,$(HOST_OS)),WIN,$(HOST_OS))
HOST_CPU := $(shell uname -m 2>/dev/null | tr '[:upper:]' '[:lower:]')
HOST_CPU_FAMILY := $(if $(filter aarch64 arm64,$(HOST_CPU)),arm64,x86)
HOST_BITS := $(if $(filter x86_64 amd64 aarch64 arm64,$(HOST_CPU)),64,32)
HOST_ARCH := $(if $(filter arm64,$(HOST_CPU_FAMILY)),aarch64,$(if $(filter 64,$(HOST_BITS)),x86_64,i686))
NAMED_TARGETS := linux_i686 linux_x86_64 linux_aarch64 \
                 macos_aarch64 macos_x86_64 \
                 freebsd_aarch64 freebsd_x86_64 \
                 win_i686 win_x86_64
HOST_TARGET := $(filter $(NAMED_TARGETS),$(patsubst DARWIN,macos,$(patsubst LINUX,linux,$(patsubst FREEBSD,freebsd,$(patsubst WIN,win,$(HOST_OS)))))_$(HOST_ARCH))

ARCH ?= $(HOST_OS)
# Before the override below, which erases the origin.
ARCH_FROM_CLI := $(filter command line,$(origin ARCH))
override ARCH := $(shell printf '%s' "$(ARCH)" | tr '[:lower:]' '[:upper:]')

# Keep aliases overridden after normalizing ARCH.
ifneq ($(filter $(ARCH),MACOS OSX),)
override ARCH := DARWIN
endif
ifneq ($(filter $(ARCH),WINDOWS WIN32),)
override ARCH := WIN
endif

ifneq ($(filter $(ARCH),$(LEGACY_UNSUPPORTED_ARCHES)),)
$(error Unsupported ARCH '$(ARCH)': DOS/BeOS/Amiga support has been removed)
endif
ifeq ($(filter $(ARCH),$(SUPPORTED_ARCHES)),)
$(error Unsupported ARCH '$(ARCH)'. Supported values: $(SUPPORTED_ARCHES))
endif

.DEFAULT_GOAL := all

ifeq ($(filter -j% --jobs%,$(MAKEFLAGS)),)
  NPROC ?= $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
  MAKEFLAGS += -j$(NPROC)
endif

ifeq ($(origin CC),default)
CC := $(if $(filter WIN,$(HOST_OS)),gcc,cc)
endif
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
# Windows ACLs make permission tests unreliable; test file creation instead.
WIN_TEMP_DIR := $(shell \
  for d in "$${TMPDIR}" "$${TEMP}" "$${TMP}" "$${TEMPDIR}" /tmp /var/tmp .; do \
    [ -n "$$d" ] && [ -d "$$d" ] || continue; \
    f="$$d/.zsnes-make-tmp-$$$$"; \
    if (umask 077 && : > "$$f") 2>/dev/null; then \
      rm -f "$$f"; \
      printf '%s\n' "$$d"; \
      break; \
    fi; \
  done; \
  :)
ifeq ($(strip $(WIN_TEMP_DIR)),)
$(error No writable temporary directory found; set TMPDIR to a writable directory)
endif
export TMPDIR := $(WIN_TEMP_DIR)
export TEMP := $(WIN_TEMP_DIR)
export TMP := $(WIN_TEMP_DIR)
export TEMPDIR := $(WIN_TEMP_DIR)
endif

# MSYS2 reports the kernel CPU, so use the compiler target for Windows.
ifeq ($(ARCH)/$(HOST_OS),WIN/WIN)
ifneq ($(findstring i686,$(CC_TARGET_TRIPLE)),)
BITS ?= 32
CPU ?= x86
else ifneq ($(findstring x86_64,$(CC_TARGET_TRIPLE)),)
BITS ?= 64
CPU ?= x86
endif
endif
BITS ?= $(HOST_BITS)
CPU  ?= $(HOST_CPU_FAMILY)

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
ifeq ($(ARCH),DARWIN)
DARWIN_ARCH ?= $(if $(filter arm64,$(CPU)),arm64,x86_64)
ARCH_CFLAGS += -arch $(DARWIN_ARCH)
endif

IS_FEDORA       := $(if $(wildcard /etc/fedora-release),yes)
IS_DEBIAN_BASED := $(if $(wildcard /etc/debian_version),yes)

WARN_FLAGS ?= -Wall -Werror=unused-variable -Wno-address-of-packed-member
# x86 uses absolute addressing; ARM and Darwin require PIC.
PIC_FLAGS := $(if $(or $(filter arm64,$(CPU)),$(filter DARWIN,$(ARCH))),,-no-pie -fno-pic)
# XSI exposes setreuid/setregid on Linux and the BSDs.
FEATURE_FLAGS := -D_XOPEN_SOURCE=700
ifeq ($(ARCH),DARWIN)
# Preserve Darwin extensions and silence legacy OpenGL deprecations.
FEATURE_FLAGS := -D_DARWIN_C_SOURCE -DGL_SILENCE_DEPRECATION
endif
# DEBUG=1 is the debug target, but composes: make linux_i686 DEBUG=1.
ifneq ($(filter-out 0 no false,$(DEBUG)),)
BUILD_MODE := debug
endif
BUILD_MODE ?= release
ifeq ($(BUILD_MODE),debug)
OPT_FLAGS := -Og -g3 -fno-omit-frame-pointer
else
OPT_FLAGS := -O3
endif
COMMON_FLAGS = $(ARCH_CFLAGS) -pthread $(PIC_FLAGS) -std=c11 $(FEATURE_FLAGS) $(OPT_FLAGS) -D_FORTIFY_SOURCE=2 -ffunction-sections -fdata-sections -fno-common -Wfatal-errors $(WARN_FLAGS)

CFLAGS += $(COMMON_FLAGS)
# Preserve the original x87 behavior on 32-bit x86.
ifeq ($(CPU)/$(BITS),x86/32)
ifneq ($(ARCH),DARWIN)
CFLAGS += -mno-sse -mno-sse2
endif
endif
# Darwin uses -dead_strip instead of GNU section GC.
ifeq ($(ARCH),DARWIN)
LDFLAGS += -Wl,-dead_strip -lz -lm
else
LDFLAGS += -Wl,--as-needed $(if $(filter arm64,$(CPU)),,-no-pie) -Wl,--gc-sections -lz -lm
endif

WITH_OPENGL   := yes
WITH_PNG      := yes
WITH_SDL      := $(if $(filter $(ARCH),$(UNIXSDL_ARCHES)),yes,)
WITH_PIPEWIRE :=
WITH_AO       :=

# Cross builds resolve libraries from the target sysroot.
HOST_CPU_NORM := $(HOST_CPU_FAMILY)
CPU_CROSS_BUILD := $(if $(filter-out $(HOST_CPU_NORM),$(CPU)),yes)
BITS_CROSS_BUILD := $(if $(and $(filter 32,$(HOST_BITS)),$(filter 64,$(BITS))),yes)
# Apple clang uses the native SDK for both -arch values.
ifeq ($(ARCH)/$(HOST_OS),DARWIN/DARWIN)
CPU_CROSS_BUILD :=
BITS_CROSS_BUILD :=
endif
CROSS_BUILD := $(if $(or $(filter-out $(HOST_OS),$(ARCH)),\
                    $(CPU_CROSS_BUILD),$(BITS_CROSS_BUILD)),yes,)

# Always query the target's pkg-config.
PKG_CONFIG ?= pkg-config

# Use an explicit environment because probes run while parsing the Makefile.
PKG_CONFIG_ENV :=
ifeq ($(CROSS_BUILD),yes)
CROSS_TRIPLE  := $(shell $(or $(CC_TARGET),$(CC)) -dumpmachine 2>/dev/null)
# Some cross compilers need /usr/<triplet> as the sysroot fallback.
CROSS_SYSROOT := $(patsubst %/,%,$(shell $(or $(CC_TARGET),$(CC)) -print-sysroot 2>/dev/null))
ifeq ($(strip $(CROSS_SYSROOT)),)
CROSS_SYSROOT := $(wildcard /usr/$(CROSS_TRIPLE))
endif
ifneq ($(strip $(CROSS_SYSROOT)),)
export PKG_CONFIG_LIBDIR := $(CROSS_SYSROOT)/lib/pkgconfig:$(CROSS_SYSROOT)/lib/$(CROSS_TRIPLE)/pkgconfig:/usr/lib/$(CROSS_TRIPLE)/pkgconfig:/usr/share/pkgconfig
export PKG_CONFIG_PATH :=
PKG_CONFIG_ENV := PKG_CONFIG_LIBDIR='$(PKG_CONFIG_LIBDIR)' PKG_CONFIG_PATH=''
ifeq ($(shell command -v $(PKG_CONFIG) >/dev/null 2>&1 && echo yes),)
override PKG_CONFIG := pkg-config
endif
endif
endif

ifneq ($(filter $(ARCH),LINUX),)
ifneq ($(filter -m32,$(ARCH_CFLAGS)),)
export PKG_CONFIG_PATH := /usr/lib/pkgconfig:/usr/lib/i386-linux-gnu/pkgconfig:/usr/share/pkgconfig:$(PKG_CONFIG_PATH)
endif
endif

# Verify that pkg-config libraries link for the selected target.
define detect_pkg_for_target
$(shell \
  if $(PKG_CONFIG_ENV) $(PKG_CONFIG) --exists $(1) >/dev/null 2>&1; then \
    printf 'int main(void){return 0;}\n' | \
      $(or $(CC_TARGET),$(CC)) $(COMMON_FLAGS) -x c - -o /dev/null $$($(PKG_CONFIG_ENV) $(PKG_CONFIG) --libs $(1)) >/dev/null 2>&1 && \
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

# Wrapper targets defer dependency checks to their sub-make.
WRAPPER_GOALS := clean distclean debug linux_pi4 \
                 linux_i686 linux_x86_64 linux_aarch64 \
                 macos_aarch64 macos_x86_64 \
                 freebsd_aarch64 freebsd_x86_64 \
                 win_i686 win_x86_64 portcheck portasm help test fmt unused
# Empty command-line backend variables are explicit opt-outs.
BACKENDS_OPTOUT := $(if $(filter command line,$(origin WITH_SDL) \
                     $(origin WITH_PIPEWIRE) $(origin WITH_AO)),yes)
SKIP_AUDIO_BACKEND_CHECK := $(if $(or \
    $(filter $(WRAPPER_GOALS),$(MAKECMDGOALS)),$(BACKENDS_OPTOUT)),yes)

ifeq ($(SKIP_AUDIO_BACKEND_CHECK),)
ifeq ($(ARCH),WIN)
# Windows does not use SDL.
else
ifeq ($(CROSS_BUILD),yes)
ifeq ($(SDL_BACKEND_AVAILABLE),)
ifneq ($(filter $(ARCH),$(UNIXSDL_ARCHES)),)
$(info )
$(info ERROR: no SDL for $(CPU)/$(ARCH), and these targets have no other)
$(info video backend: __UNIXSDL__ and unix/sdllink.c are built either way.)
$(info Install SDL3 for the target, or cross-build one into its sysroot.)
$(info )
$(error No SDL library for $(CPU)/$(ARCH))
else
$(info ===> no SDL for $(CPU)/$(ARCH); building without a video backend)
WITH_SDL :=
endif
endif
endif
endif
endif

ifeq ($(CROSS_BUILD),yes)
ifdef WITH_OPENGL
GL_HEADER_AVAILABLE := $(shell $(or $(CC_TARGET),$(CC)) $(ARCH_CFLAGS) \
  -E -include GL/gl.h -x c /dev/null >/dev/null 2>&1 && echo yes)
ifeq ($(GL_HEADER_AVAILABLE),)
$(info ===> no GL/gl.h for $(CPU)/$(ARCH); building without OpenGL)
WITH_OPENGL :=
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
ifeq ($(ARCH)/$(CPU)/$(BITS),LINUX/x86/32)
  CFLAGS += -L/usr/lib32
  LDFLAGS += -L/usr/lib32
endif

ifeq ($(WITH_SDL),yes)
  ifeq ($(strip $(SDL_CONFIG)),)
    ifeq ($(SDL3_AVAILABLE),yes)
      SDL_CONFIG := $(PKG_CONFIG_ENV) $(PKG_CONFIG) sdl3
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

ifdef WITH_PNG
  ifeq ($(origin PNG_CONFIG),undefined)
    ifneq ($(shell $(PKG_CONFIG_ENV) $(PKG_CONFIG) --exists libpng >/dev/null 2>&1 && echo yes),yes)
      WITH_PNG :=
    endif
  endif
endif
ifdef WITH_PNG
  PNG_CONFIG ?= $(PKG_CONFIG_ENV) $(PKG_CONFIG) libpng
  ifndef CFLAGS_PNG
    CFLAGS_PNG  := $(shell $(PNG_CONFIG) --cflags)
  endif
  ifndef LDFLAGS_PNG
    # Static Windows links need pkg-config's library order.
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
  AO_CONFIG ?= $(PKG_CONFIG_ENV) $(PKG_CONFIG) ao
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
    PIPEWIRE_CONFIG ?= $(PKG_CONFIG_ENV) $(PKG_CONFIG) libpipewire-0.3
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

# Debian/Ubuntu x86 multiarch headers.
ifeq ($(ARCH)/$(CPU),LINUX/x86)
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

# Diagnostic hooks remain runtime-gated by their environment variables.
# Use EXTRA_CFLAGS=-DSCANLINE_PC_LOG for opcode logging.
ifdef WITH_DEBUG_HOOKS
CFGDEFS += -DZSNES_DEBUG_HOOKS
endif

ifdef WITH_OPENGL
CFGDEFS += -D__OPENGL__
endif

ifeq ($(WITH_AO),yes)
CFGDEFS += -D__LIBAO__
endif

ifneq ($(filter $(ARCH),$(UNIXSDL_ARCHES)),)
SRCS += unix/audio.c
SRCS += unix/battery.c
SRCS += unix/c_sdlintrf.c
SRCS += unix/lib.c
SRCS += unix/safelib.c

SRCS += unix/sdl_render.c
SRCS += unix/sdllink.c
SRCS += unix/sockserv.c
SRCS += unix/sw_draw.c

ifdef WITH_OPENGL
SRCS += unix/gl_draw.c
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
SRCS += mmlib/macos.c

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

# xinput9_1_0 is available on a clean Windows installation.
LDFLAGS += -ldxguid -ldinput -lxinput9_1_0 -lgdi32 -lole32 -lwinmm

ifdef WITH_OPENGL
SRCS += win/gl_draw.c
LDFLAGS += -lopengl32
endif

LDFLAGS += --static
# Keep winpthread after objects for --as-needed.
LDFLAGS += -lwinpthread

PSRS += win/confloc.psr


CFGDEFS += -D__WIN32__
CFGDEFS += -D__ZSNES_PLATFORM_WINDOWS__
endif

CFLAGS += $(CFGDEFS)
CFLAGS   += $(EXTRA_CFLAGS)
LDFLAGS  += $(EXTRA_LDFLAGS)
DEPFLAGS_C = -MMD -MP -MF $(@:.o=.d) -MT $@

BUILD_DIR := build
HDR_NAMES := $(PSRS:.psr=.h)
HDRS := $(addprefix $(BUILD_DIR)/,$(HDR_NAMES))
OBJ_NAMES := $(filter %.o, $(SRCS:.c=.o) $(SRCS:.rc=.o) $(PSRS:.psr=.o))
OBJS := $(addprefix $(BUILD_DIR)/,$(OBJ_NAMES))
DEPS := $(OBJS:.o=.d)

# Clean shared object paths when the build configuration changes.
BUILDSTAMP := $(BUILD_DIR)/MODE
BUILD_TAG := $(BUILD_MODE)|$(ARCH)|$(BITS)|$(CPU)|$(CC_TARGET_TRIPLE)|\
$(WITH_SDL)|$(WITH_OPENGL)|$(WITH_PNG)|$(WITH_AO)|$(WITH_PIPEWIRE)|\
$(WITH_DEBUGGER)|$(WITH_DEBUG_HOOKS)|$(EXTRA_CFLAGS)|$(ARM64_CFLAGS)
ifneq ($(filter all test,$(or $(MAKECMDGOALS),all)),)
PREV_BUILD_TAG := $(shell cat $(BUILDSTAMP) 2>/dev/null)
ifneq ($(PREV_BUILD_TAG),)
ifneq ($(PREV_BUILD_TAG),$(BUILD_TAG))
$(info ===> build target changed ($(PREV_BUILD_TAG) -> $(BUILD_TAG)), cleaning stale objects)
_CLEAN_SWITCH := $(shell rm -fr $(BUILD_DIR) $(BINARY) zsnes zsnes.exe)
endif
endif
_WRITE_STAMP := $(shell mkdir -p $(BUILD_DIR) && printf '%s' '$(BUILD_TAG)' > $(BUILDSTAMP))
endif

.SUFFIXES:

DISPATCH := $(if $(ARCH_FROM_CLI)$(filter command line,$(origin BITS) $(origin CPU)),,$(HOST_TARGET))
ifeq ($(DISPATCH),)
all: $(BINARY)
else
all:
	$(MAKE) $(DISPATCH)
endif

define need_tool
@command -v $(1) >/dev/null 2>&1 || { \
  echo "error: $(1) not found; install $(2)" >&2; exit 1; }
endef

define need_host
@test "$(HOST_OS)" = "$(1)" || { \
  echo "error: $(2) must be built on $(3)" >&2; exit 1; }
endef

MINGW32_PREFIX ?= i686-w64-mingw32
MINGW64_PREFIX ?= x86_64-w64-mingw32
LINUX_I686_PREFIX ?= i686-linux-gnu
LINUX_X86_64_PREFIX ?= x86_64-linux-gnu
LINUX_AARCH64_PREFIX ?= aarch64-linux-gnu
FREEBSD_X86_64_PREFIX ?= x86_64-unknown-freebsd
FREEBSD_AARCH64_PREFIX ?= aarch64-unknown-freebsd

LINUX_I686_NATIVE := $(if $(and $(filter LINUX,$(HOST_OS)),$(filter i386 i486 i586 i686 x86_64 amd64,$(HOST_CPU))),yes)
LINUX_X86_64_NATIVE := $(if $(and $(filter LINUX,$(HOST_OS)),$(filter x86_64 amd64,$(HOST_CPU))),yes)
LINUX_AARCH64_NATIVE := $(if $(and $(filter LINUX,$(HOST_OS)),$(filter aarch64 arm64,$(HOST_CPU))),yes)
FREEBSD_X86_64_NATIVE := $(if $(and $(filter FREEBSD,$(HOST_OS)),$(filter x86_64 amd64,$(HOST_CPU))),yes)
FREEBSD_AARCH64_NATIVE := $(if $(and $(filter FREEBSD,$(HOST_OS)),$(filter aarch64 arm64,$(HOST_CPU))),yes)
MINGW32_NATIVE := $(if $(and $(filter WIN,$(HOST_OS)),$(findstring i686,$(CC_TARGET_TRIPLE))),yes)
MINGW64_NATIVE := $(if $(and $(filter WIN,$(HOST_OS)),$(findstring x86_64,$(CC_TARGET_TRIPLE))),yes)

LINUX_I686_CC ?= $(if $(LINUX_I686_NATIVE),$(CC),$(LINUX_I686_PREFIX)-gcc)
LINUX_X86_64_CC ?= $(if $(LINUX_X86_64_NATIVE),$(CC),$(LINUX_X86_64_PREFIX)-gcc)
LINUX_AARCH64_CC ?= $(if $(LINUX_AARCH64_NATIVE),$(CC),$(LINUX_AARCH64_PREFIX)-gcc)
LINUX_I686_PKG_CONFIG ?= $(if $(LINUX_I686_NATIVE),pkg-config,$(LINUX_I686_PREFIX)-pkg-config)
LINUX_X86_64_PKG_CONFIG ?= $(if $(LINUX_X86_64_NATIVE),pkg-config,$(LINUX_X86_64_PREFIX)-pkg-config)
LINUX_AARCH64_PKG_CONFIG ?= $(if $(LINUX_AARCH64_NATIVE),pkg-config,$(LINUX_AARCH64_PREFIX)-pkg-config)

FREEBSD_X86_64_CC ?= $(if $(FREEBSD_X86_64_NATIVE),$(CC),$(FREEBSD_X86_64_PREFIX)-gcc)
FREEBSD_AARCH64_CC ?= $(if $(FREEBSD_AARCH64_NATIVE),$(CC),$(FREEBSD_AARCH64_PREFIX)-gcc)
FREEBSD_X86_64_PKG_CONFIG ?= $(if $(FREEBSD_X86_64_NATIVE),pkg-config,$(FREEBSD_X86_64_PREFIX)-pkg-config)
FREEBSD_AARCH64_PKG_CONFIG ?= $(if $(FREEBSD_AARCH64_NATIVE),pkg-config,$(FREEBSD_AARCH64_PREFIX)-pkg-config)

MINGW32_CC ?= $(if $(MINGW32_NATIVE),$(CC),$(MINGW32_PREFIX)-gcc)
MINGW64_CC ?= $(if $(MINGW64_NATIVE),$(CC),$(MINGW64_PREFIX)-gcc)
MINGW32_PKG_CONFIG ?= $(if $(MINGW32_NATIVE),pkg-config,$(MINGW32_PREFIX)-pkg-config)
MINGW64_PKG_CONFIG ?= $(if $(MINGW64_NATIVE),pkg-config,$(MINGW64_PREFIX)-pkg-config)
MINGW32_WINDRES ?= $(if $(MINGW32_NATIVE),windres,$(MINGW32_PREFIX)-windres)
MINGW64_WINDRES ?= $(if $(MINGW64_NATIVE),windres,$(MINGW64_PREFIX)-windres)

.PHONY: linux_pi4
.PHONY: linux_i686 linux_x86_64 linux_aarch64
.PHONY: macos_aarch64 macos_x86_64
.PHONY: freebsd_aarch64 freebsd_x86_64
.PHONY: win_i686 win_x86_64 help

linux_i686:
	$(call need_tool,$(LINUX_I686_CC),an i686 Linux C compiler)
	$(MAKE) ARCH=LINUX BITS=32 CPU=x86 \
	  CC=$(LINUX_I686_CC) CC_TARGET=$(LINUX_I686_CC) \
	  PKG_CONFIG=$(LINUX_I686_PKG_CONFIG) all

linux_x86_64:
	$(call need_tool,$(LINUX_X86_64_CC),an x86-64 Linux C compiler)
	$(MAKE) ARCH=LINUX BITS=64 CPU=x86 \
	  CC=$(LINUX_X86_64_CC) CC_TARGET=$(LINUX_X86_64_CC) \
	  PKG_CONFIG=$(LINUX_X86_64_PKG_CONFIG) all

linux_aarch64:
	$(call need_tool,$(LINUX_AARCH64_CC),an aarch64 Linux C compiler)
	$(MAKE) ARCH=LINUX BITS=64 CPU=arm64 \
	  CC=$(LINUX_AARCH64_CC) CC_TARGET=$(LINUX_AARCH64_CC) \
	  PKG_CONFIG=$(LINUX_AARCH64_PKG_CONFIG) all

linux_pi4:
	$(call need_tool,$(LINUX_AARCH64_CC),an aarch64 Linux C compiler)
	$(MAKE) ARCH=LINUX BITS=64 CPU=arm64 ARM64_CFLAGS='-mcpu=cortex-a72 -mtune=cortex-a72' \
	  CC=$(LINUX_AARCH64_CC) CC_TARGET=$(LINUX_AARCH64_CC) \
	  PKG_CONFIG=$(LINUX_AARCH64_PKG_CONFIG) all

macos_aarch64:
	$(call need_host,DARWIN,macOS aarch64,macOS)
	$(MAKE) ARCH=DARWIN BITS=64 CPU=arm64 DARWIN_ARCH=arm64 all

macos_x86_64:
	$(call need_host,DARWIN,macOS x86-64,macOS)
	$(MAKE) ARCH=DARWIN BITS=64 CPU=x86 DARWIN_ARCH=x86_64 all

freebsd_x86_64:
	$(call need_tool,$(FREEBSD_X86_64_CC),an x86-64 FreeBSD C compiler)
	$(MAKE) ARCH=FREEBSD BITS=64 CPU=x86 \
	  CC=$(FREEBSD_X86_64_CC) CC_TARGET=$(FREEBSD_X86_64_CC) \
	  PKG_CONFIG=$(FREEBSD_X86_64_PKG_CONFIG) all

freebsd_aarch64:
	$(call need_tool,$(FREEBSD_AARCH64_CC),an aarch64 FreeBSD C compiler)
	$(MAKE) ARCH=FREEBSD BITS=64 CPU=arm64 \
	  CC=$(FREEBSD_AARCH64_CC) CC_TARGET=$(FREEBSD_AARCH64_CC) \
	  PKG_CONFIG=$(FREEBSD_AARCH64_PKG_CONFIG) all

win_i686:
	$(call need_tool,$(MINGW32_CC),the mingw32 toolchain)
	$(call need_tool,$(MINGW32_WINDRES),the mingw32 resource compiler)
	$(MAKE) ARCH=WIN BITS=32 CPU=x86 \
	  CC=$(MINGW32_CC) CC_TARGET=$(MINGW32_CC) \
	  WINDRES=$(MINGW32_WINDRES) PKG_CONFIG=$(MINGW32_PKG_CONFIG) all

win_x86_64:
	$(call need_tool,$(MINGW64_CC),the mingw-w64 toolchain)
	$(call need_tool,$(MINGW64_WINDRES),the mingw-w64 resource compiler)
	$(MAKE) ARCH=WIN BITS=64 CPU=x86 \
	  CC=$(MINGW64_CC) CC_TARGET=$(MINGW64_CC) \
	  WINDRES=$(MINGW64_WINDRES) PKG_CONFIG=$(MINGW64_PKG_CONFIG) all

help:
	@echo 'Targets:'
	@echo '  all            this machine, through the target naming it (the default)'
	@echo '  debug          the default target with debug symbols (same as DEBUG=1)'
	@echo '  linux_i686     32-bit x86 Linux'
	@echo '  linux_x86_64   64-bit x86 Linux'
	@echo '  linux_aarch64  64-bit ARM Linux'
	@echo '  linux_pi4      the same, tuned for a Raspberry Pi 4 Cortex-A72'
	@echo '  macos_aarch64  Apple Silicon macOS'
	@echo '  macos_x86_64   Intel macOS'
	@echo '  freebsd_aarch64  64-bit ARM FreeBSD'
	@echo '  freebsd_x86_64   64-bit x86 FreeBSD'
	@echo '  win_i686       32-bit Windows'
	@echo '  win_x86_64     64-bit Windows'
	@echo '  portcheck      compile every source for x86-64 and aarch64'
	@echo '  test           run the unit tests'
	@echo
	@echo 'DEBUG=1 builds any of them unoptimised and with symbols.'
	@echo 'The tree is C11 throughout; the cross targets need their'
	@echo 'toolchain installed and will name it if it is missing.'

debug:
	$(MAKE) DEBUG=1 all

-include $(wildcard $(DEPS))

$(BINARY): $(OBJS)
	@echo '===> LD $@'
	$(Q)$(CC_TARGET) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

$(addprefix $(BUILD_DIR)/,$(filter %.o,$(SRCS:.c=.o))): $(HDRS)

$(BUILD_DIR)/%.o: %.c
	@echo '===> CC $<'
	$(Q)mkdir -p $(@D)
	$(Q)$(CC_TARGET) $(CFLAGS) -iquote $(BUILD_DIR) -c $(DEPFLAGS_C) -o $@ $<

$(BUILD_DIR)/%.o: %.rc
	@echo '===> RES $<'
	$(Q)mkdir -p $(@D)
	$(Q)$(WINDRES) $(if $(filter WIN,$(ARCH)),-Iwin) -o $@ $<

$(BUILD_DIR)/%.h $(BUILD_DIR)/%.o: %.psr $(PSR)
	@echo '===> PSR $@'
	$(Q)mkdir -p $(dir $(BUILD_DIR)/$*.o)
	$(Q)$(PYTHON) ./$(PSR) $(CFGDEFS) -gcc $(CC_TARGET) -compile -flags '$(CFLAGS)' -cheader $(BUILD_DIR)/$*.h -fname $(*F) $(BUILD_DIR)/$*.o $*.psr

%.h:
	@true

%.inc:
	@true

clean distclean:
	@echo '===> CLEAN'
	$(Q)rm -fr $(BUILD_DIR) $(BINARY) zsnes zsnes.exe

info:
	@echo "ARCH          = $(ARCH)"
	@echo "WITH_DEBUGGER = $(WITH_DEBUGGER)"
	@echo "WITH_OPENGL   = $(WITH_OPENGL)"
	@echo "WITH_PNG      = $(WITH_PNG)"
	@echo "WITH_SDL      = $(WITH_SDL)"
	@echo "WITH_PIPEWIRE = $(WITH_PIPEWIRE)"
	@echo "WITH_AO       = $(WITH_AO)"
	@echo "WITH_DEBUG_HOOKS = $(WITH_DEBUG_HOOKS)"
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

# BSD install lacks GNU install -D.
INSTALL_DIRS := bin share/applications share/metainfo share/man/man1 \
                $(foreach s,16x16 32x32 48x48 64x64 128x128,share/icons/hicolor/$(s)/apps)

install: zsnes
	mkdir -p $(foreach d,$(INSTALL_DIRS),'$(DESTDIR)$(PREFIX)/$(d)')
	install -m755 zsnes '$(DESTDIR)$(PREFIX)/bin/zsnes'
	for ICON_SIZE in 16x16 32x32 48x48 64x64 128x128; do \
		install -m644 img/$${ICON_SIZE}x32.png "$(DESTDIR)$(PREFIX)/share/icons/hicolor/$$ICON_SIZE/apps/io.github.xyproto.zsnes.png" ; \
	done
	install -m644 linux/zsnes.desktop '$(DESTDIR)$(PREFIX)/share/applications/io.github.xyproto.zsnes.desktop'
	install -m644 linux/io.github.xyproto.zsnes.metainfo.xml '$(DESTDIR)$(PREFIX)/share/metainfo/io.github.xyproto.zsnes.metainfo.xml'
	install -m644 man/zsnes.1 '$(DESTDIR)$(PREFIX)/share/man/man1/zsnes.1'

# Compile every source for each requested architecture without linking.
PORTCHECK_CC     ?= gcc
PORTCHECK_DEFS   := $(filter-out -D__PIPEWIRE__ -D__LIBAO__,$(CFGDEFS))
# -idirafter keeps cross-toolchain libc headers ahead of host library headers.
PORTCHECK_CFLAGS ?= -std=c11 $(FEATURE_FLAGS) \
                    -O1 -I. $(PORTCHECK_DEFS) $(CFLAGS_SDL) $(CFLAGS_PNG)
PORTCHECK_ARM_CC ?= aarch64-linux-gnu-gcc
PORTCHECK_ARCHS  ?= x86-64 aarch64
.PHONY: portcheck
portcheck: $(HDRS)
# Remove PSR outputs built with portcheck flags.
	@rc=0; \
	for t in "x86-64:$(PORTCHECK_CC):-m64" "aarch64:$(PORTCHECK_ARM_CC):-idirafter/usr/include"; do \
	  name=$${t%%:*}; rest=$${t#*:}; cc=$${rest%%:*}; extra=$${rest#*:}; \
	  case " $(PORTCHECK_ARCHS) " in *" $$name "*) ;; *) continue;; esac; \
	  command -v $$cc >/dev/null 2>&1 || { \
	    echo "===> PORTCHECK: $$name skipped, $$cc not installed"; continue; }; \
	  echo "===> PORTCHECK: compiling every C source for $$name"; \
	  ok=0; bad=0; \
	  for f in $(filter %.c,$(SRCS)); do \
	    if $$cc $(PORTCHECK_CFLAGS) $$extra -iquote $(BUILD_DIR) -c -o /dev/null $$f 2>/tmp/zs_portcheck.$$$$; then \
	      ok=$$((ok+1)); \
	    else \
	      bad=$$((bad+1)); echo "  FAIL $$f"; \
	      grep -iE 'error' /tmp/zs_portcheck.$$$$ | head -2 | sed 's/^/        /'; \
	    fi; \
	    rm -f /tmp/zs_portcheck.$$$$; \
	  done; \
	  echo "===> PORTCHECK: $$name $$ok built, $$bad failed"; echo; \
	  [ $$bad = 0 ] || rc=1; \
	done; rm -f $(addprefix $(BUILD_DIR)/,$(PSRS:.psr=.o)) $(HDRS); exit $$rc

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
