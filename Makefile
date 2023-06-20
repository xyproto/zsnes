DESTDIR ?=
PREFIX ?= /usr

CC ?= gcc
CXX ?= g++

BINARY ?= zsnes
PSR ?= parsegen
ASM ?= nasm

# TODO: FreeBSD has a patch for being able to build without -fcommon
# NOTE: -O1 is mandatory

ASMFLAGS := -O1 -w-orphan-labels -felf32 -DELF -DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__
CFLAGS := -m32 -pthread -no-pie -std=gnu99 -fcommon -O1 -march=pentium-mmx -fno-inline -fno-pic -mtune=generic -mmmx -D_FORTIY_SOURCE=2 -L/usr/lib32 -mno-sse -mno-sse2 -ffunction-sections -fdata-sections -Wfatal-errors -w -rdynamic $(shell pkg-config sdl --cflags) $(shell pkg-config libpng --cflags) -DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__
CXXFLAGS := -m32 -pthread -no-pie -std=gnu++14 -O1 -march=pentium-mmx -fno-inline -fno-pic -mtune=generic -mmmx -D_FORTIFY_SOURCE=2 -L/usr/lib32 -mno-sse -mno-sse2 -ffunction-sections -fdata-sections -Wfatal-errors -w -rdynamic -DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__
DEBUGFLAGS :=
LDFLAGS := -Wl,--as-needed -no-pie -L/usr/lib32 -Wl,--gc-sections -lz -ldl -lX11 $(shell pkg-config sdl --libs) $(shell pkg-config libpng --libs) -lGL
PSRFLAGS := -DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__

ifeq ($(wildcard /usr/lib/i386-linux-gnu/.),)
  CFLAGS += -I/usr/include/x86_64-linux-gnu -I /usr/include/X11
  CXXFLAGS += -I/usr/include/x86_64-linux-gnu -I /usr/include/X11
  LDFLAGS = -Wl,--as-needed -no-pie -L/usr/lib32 -L/usr/lib/i386-linux-gnu -Wl,--gc-sections -lz -lSDL-1.2 -lpng16 -lX11
endif

SRCS :=
SRCS += c_init.c
SRCS += c_vcache.c
SRCS += chips/7110emu.c
SRCS += chips/7110proc.asm
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
SRCS += video/2xsaiw.asm
SRCS += video/c_2xsaiw.c
SRCS += video/c_copyvid.c
SRCS += video/c_makev16b.c
SRCS += video/c_makevid.c
SRCS += video/c_mode716.c
SRCS += video/c_newgfx16.c
SRCS += video/copyvid.asm
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
SRCS += jma/7zlzma.cpp
SRCS += jma/crc32.cpp
SRCS += jma/iiostrm.cpp
SRCS += jma/inbyte.cpp
SRCS += jma/jma.cpp
SRCS += jma/lzma.cpp
SRCS += jma/lzmadec.cpp
SRCS += jma/winout.cpp
SRCS += jma/zsnesjma.cpp
SRCS += linux/audio.c
SRCS += linux/battery.c
SRCS += linux/c_sdlintrf.c
SRCS += linux/lib.c
SRCS += linux/safelib.c
SRCS += linux/sdlintrf.asm
SRCS += linux/sdllink.c
SRCS += linux/sockserv.c
SRCS += linux/sw_draw.c
SRCS += linux/gl_draw.c
SRCS += mmlib/linux.c

PSRS :=
PSRS += cfg.psr
PSRS += input.psr
PSRS += md.psr

HDRS := $(PSRS:.psr=.h)
OBJS := $(filter %.o, $(SRCS:.asm=.o) $(SRCS:.c=.o) $(SRCS:.cpp=.o) $(PSRS:.psr=.o))
DEPS := $(OBJS:.o=.d)

.SUFFIXES:
.SUFFIXES: .asm .c .cpp .d .o

all: $(BINARY)

-include $(DEPS)

$(BINARY): $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) $(LDFLAGS) $(DEBUGFLAGS) -o $@

.asm.o:
	$(ASM) $(ASMFLAGS) $(DEBUGFLAGS) -M -o $@ $< > $(@:.o=.d) || rm -f $(@:.o=.d)
	$(ASM) $(ASMFLAGS) $(DEBUGFLAGS) -o $@ $<

$(filter %.o, $(SRCS:.c=.o) $(SRCS:.cpp=.o)): $(HDRS)

.c.o:
	$(CC) $(CFLAGS) $(DEBUGFLAGS) -c -MMD -o $@ $<

.cpp.o:
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -c -MMD -o $@ $<

$(PSR): parsegen.cpp
	$(CXX) -o $@ $< -lz

%.h %.o: %.psr $(PSR)
	./$(PSR) $(PSRFLAGS) -gcc $(CC) -compile -flags '$(CFLAGS)' -cheader $@ -fname $(*F) $(@:.h=.o) $<

%.h:
	@true

%.inc:
	@true

clean:
	rm -fr $(HDRS) $(DEPS) $(OBJS) $(BINARY) $(PSR)

distclean:
	rm -fr $(HDRS) $(DEPS) $(OBJS) $(BINARY) $(PSR)
	find . -name "*.[do]" -delete

info:
	@echo "BINARY        = $(BINARY)"
	@echo "ASM           = $(ASM)"
	@echo "CC            = $(CC)"
	@echo "CXX           = $(CXX)"
	@echo "PSR           = $(PSR)"
	@echo "PSRFLAGS      = $(PSRFLAGS)"
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

install:
	install -Dm755 zsnes '$(DESTDIR)$(PREFIX)/bin/zsnes'
	for ICON_SIZE in 16x16 32x32 48x48 64x64 128x128; do \
		install -Dm644 icons/$${ICON_SIZE}x32.png "$(DESTDIR)$(PREFIX)/share/icons/hicolor/$$ICON_SIZE/apps/io.github.xyproto.zsnes.png" ; \
	done
	install -Dm755 linux/zsnes.desktop '$(DESTDIR)$(PREFIX)/share/applications/io.github.xyproto.zsnes.desktop'
	install -Dm755 linux/io.github.xyproto.zsnes.metainfo.xml -t '$(DESTDIR)$(PREFIX)/share/metainfo'
	install -Dm644 man/zsnes.1 '$(DESTDIR)$(PREFIX)/share/man/man1/zsnes.1'
