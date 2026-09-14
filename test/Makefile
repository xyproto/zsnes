# test/Makefile, ZSNES2 headless test suite
#
# Usage:
#   make         , build the test binary
#   make run     , build and run
#   make clean   , remove build products
#
# The test binary links only the pre-compiled object files it needs from the
# parent build.  Because the parent compiles with -ffunction-sections, the
# linker's --gc-sections drops every function not reachable from main, so no
# SDL, zlib, or audio symbols are pulled in.

# The unit tests are portable C. They build -m32 to match the emulator's usual
# word size, except on macOS, which has had no 32-bit userland since 10.15 and
# no GNU linker at all, so they build for the host there.
HOST_OS := $(shell uname -s)
ifeq ($(HOST_OS),Darwin)
CC      := cc
BITFLAG :=
NOPIE   :=
GCSECT  := -Wl,-dead_strip
# Darwin reads _POSIX_C_SOURCE as a restriction, so SUSv4 there hides the BSD
# side the tests use - INADDR_LOOPBACK in netplay_test.c among it. Same swap
# the top-level Makefile makes.
FEATURE  := -D_DARWIN_C_SOURCE
else
CC      := gcc
BITFLAG := -m32
NOPIE   := -no-pie
GCSECT  := -Wl,--gc-sections
FEATURE  := -D_XOPEN_SOURCE=700
endif

# C11 with SUSv4, matching the emulator: the tests must not quietly depend
# on extensions the build itself forbids, and strict c11 hides POSIX until
# the macro asks for it.
# Unoptimised by default so a failure is the port's and not the inliner's, but
# the emulator ships -O3: `make regs OPT=-O3` runs the same comparison at
# release settings, which is what catches layout the harness only gets right
# by luck at -O0.
OPT ?=
CFLAGS := $(BITFLAG) -std=c11 $(FEATURE) $(OPT) \
          -I.. -Wall -Wextra -Wno-unused-parameter

BIN       := zsnes_test
NETPLAY   := zsnes_netplay_test
ST10      := zsnes_st10_test
ST11      := zsnes_st11_test
OBC1      := zsnes_obc1_test
MSU1      := zsnes_msu1_test
DSP2      := zsnes_dsp2_test
SAI       := zsnes_2xsaiw_test
SA1       := zsnes_sa1_test
SPC7110   := zsnes_7110_test
VCACHE    := zsnes_vcache_test
SPRITES   := zsnes_sprites_test
HQX       := zsnes_hqx_test
FILTER    := zsnes_filter_test
SRC       := main.c
NETPLAY_SRC := netplay_test.c ../net/packet.c ../net/znp.c ../unix/net_transport.c
ST10_SRC  := st10_test.c
ST11_SRC  := st11_test.c
OBC1_SRC  := obc1_test.c
MSU1_SRC  := msu1_test.c
DSP2_SRC  := dsp2_test.c
SAI_SRC   := 2xsaiw_test.c
SA1_SRC   := sa1_test.c
SPC7110_SRC := 7110_test.c
VCACHE_SRC := vcache_test.c
SPRITES_SRC := sprites_test.c
HQX_SRC   := hqx_test.c
FILTER_SRC := filter_test.c ../video/filter.c ../video/c_hqx.c ../video/2xsaiw.c ../video/ntsc.c
C4        := zsnes_c4_test
C4_SRC    := c4proc_test.c
MODE7     := zsnes_mode716b_test
MODE7_SRC := mode716b_test.c
ASMDATA   := zsnes_asmdata_test
ASMDATA_SRC := asmdata_test.c
SAVELOAD  := zsnes_saveload_test
SAVELOAD_SRC := saveload_test.c ../saveload.c
# Objects from the parent tree belong to the real build, so let the top
# Makefile make them. make's built-in %.o: %.c would use the flags below
# instead, quietly dropping -fdata-sections and every -D the configuration
# adds, and a test would link something the emulator never builds.
# In a directory of its own, at the word size these links need: ../build holds
# whatever target was last built.
PBUILD     := ../build/test
PBUILD_REL := build/test
ifeq ($(HOST_OS),Darwin)
PBUILD_TARGET :=
else
PBUILD_TARGET := BITS=32 CPU=x86
endif

$(PBUILD)/%.o:
	$(MAKE) -C .. $(PBUILD_TARGET) BUILD_DIR=$(PBUILD_REL) $(PBUILD_REL)/$*.o

OBJS      := $(PBUILD)/zpath.o

.PHONY: all run clean spc64

all: $(BIN) $(NETPLAY) $(ST10) $(ST11) $(OBC1) $(MSU1) $(DSP2) $(SAI) $(SA1) $(SPC7110) $(VCACHE) $(SPRITES) $(HQX) $(FILTER) $(C4) $(MODE7) $(ASMDATA) $(SAVELOAD)

$(BIN): $(SRC) zstest.h $(OBJS)
	$(CC) $(CFLAGS) $(SRC) $(OBJS) $(NOPIE) $(GCSECT) -o $(BIN)

$(NETPLAY): $(NETPLAY_SRC) zstest.h ../net/packet.h
	$(CC) $(CFLAGS) $(NETPLAY_SRC) $(NOPIE) -lpthread -o $(NETPLAY)

$(ST10): $(ST10_SRC) zstest.h ../chips/st10proc.c
	$(CC) $(CFLAGS) $(ST10_SRC) ../chips/st10proc.c $(NOPIE) -o $(ST10)

$(ST11): $(ST11_SRC) zstest.h ../chips/st11proc.c
	$(CC) $(CFLAGS) $(ST11_SRC) ../chips/st11proc.c $(NOPIE) -o $(ST11)

$(OBC1): $(OBC1_SRC) zstest.h ../chips/obc1proc.c
	$(CC) $(CFLAGS) $(OBC1_SRC) ../chips/obc1proc.c $(NOPIE) -o $(OBC1)

$(MSU1): $(MSU1_SRC) zstest.h ../chips/msu1regs.c
	$(CC) $(CFLAGS) $(MSU1_SRC) ../chips/msu1regs.c $(NOPIE) -o $(MSU1)

$(DSP2): $(DSP2_SRC) zstest.h ../chips/dsp2proc.c
	$(CC) $(CFLAGS) $(DSP2_SRC) ../chips/dsp2proc.c $(NOPIE) -o $(DSP2)

$(SAI): $(SAI_SRC) zstest.h ../video/2xsaiw.c
	$(CC) $(CFLAGS) $(SAI_SRC) ../video/2xsaiw.c $(NOPIE) -o $(SAI)

$(SA1): $(SA1_SRC) zstest.h ../chips/sa1regs.c
	$(CC) $(CFLAGS) $(SA1_SRC) ../chips/sa1regs.c $(NOPIE) -o $(SA1)

$(SPC7110): $(SPC7110_SRC) zstest.h ../chips/7110proc.c
	$(CC) $(CFLAGS) $(SPC7110_SRC) ../chips/7110proc.c $(NOPIE) -o $(SPC7110)

$(C4): $(C4_SRC) zstest.h ../chips/c4proc.c ../chips/c4emu.c
	$(CC) $(CFLAGS) $(C4_SRC) ../chips/c4proc.c ../chips/c4emu.c $(NOPIE) -lm -o $(C4)

$(MODE7): $(MODE7_SRC) zstest.h ../video/mode716b.c
	$(CC) $(CFLAGS) $(MODE7_SRC) ../video/mode716b.c $(NOPIE) -o $(MODE7)

# The V2 save-state envelope is self-contained (its own CRC32, no zlib), so the
# test links only saveload.c.
$(SAVELOAD): $(SAVELOAD_SRC) zstest.h ../saveload.h
	$(CC) $(CFLAGS) $(SAVELOAD_SRC) $(NOPIE) -o $(SAVELOAD)

# endmem.o supplies winbgdata, which makevid's cwinptr points at.
$(ASMDATA): $(ASMDATA_SRC) zstest.h ../cpu/dspproc.c ../cpu/c_spcdata.c ../cpu/c_execdata.c ../cpu/c_regsdata.c ../cpu/c_regswdata.c ../video/c_mode716data.c ../chips/c_sa1data.c ../video/c_makev16tdata.c ../video/c_newgfx16data.c ../video/makevid.c ../video/newgfx.c $(PBUILD)/endmem.o
	$(CC) $(CFLAGS) $(ASMDATA_SRC) ../cpu/dspproc.c ../cpu/c_spcdata.c ../cpu/c_execdata.c ../cpu/c_regsdata.c ../cpu/c_regswdata.c ../video/c_mode716data.c ../chips/c_sa1data.c ../video/c_makev16tdata.c ../video/c_newgfx16data.c ../video/makevid.c ../video/newgfx.c $(PBUILD)/endmem.o $(NOPIE) -o $(ASMDATA)

$(VCACHE): $(VCACHE_SRC) zstest.h ../video/tilecache.c
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections $(VCACHE_SRC) ../video/tilecache.c $(NOPIE) $(GCSECT) -o $(VCACHE)

# c_hqx.c needs the generated cfg.h, which lives in the parent build dir.
# hqx_test.c includes video/c_hqx.c rather than linking it, so the tests can
# reach the interpolation helpers, which are static and carry the filter maths.
$(HQX): $(HQX_SRC) zstest.h ../video/c_hqx.c $(PBUILD)/zpath.o
	$(CC) $(CFLAGS) -I$(PBUILD) $(HQX_SRC) $(NOPIE) -o $(HQX)

# The filter front end, linked against the filters it dispatches to. Its own
# copies of the cfg variables stand in for the generated cfg.o.
$(FILTER): $(FILTER_SRC) zstest.h ../video/filter.h $(PBUILD)/zpath.o
	$(CC) $(CFLAGS) -I$(PBUILD) $(FILTER_SRC) $(NOPIE) -lm -o $(FILTER)

$(SPRITES): $(SPRITES_SRC) zstest.h ../video/sprites.c
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections $(SPRITES_SRC) ../video/sprites.c $(NOPIE) $(GCSECT) -o $(SPRITES)

run: all
	@./$(BIN)
	@./$(NETPLAY)
	@./$(ST10)
	@./$(ST11)
	@./$(OBC1)
	@./$(MSU1)
	@./$(DSP2)
	@./$(SAI)
	@./$(SA1)
	@./$(SPC7110)
	@./$(VCACHE)
	@./$(SPRITES)
	@./$(HQX)
	@./$(FILTER)
	@./$(C4)
	@./$(MODE7)
	@./$(ASMDATA)
	@./$(SAVELOAD)

# The SPC700 core run at both word sizes and compared. Every other test here
# builds -m32 only, where zreg and u4 are the same type, so a 64-bit width bug
# cannot show up; this one exists to catch exactly that.
spc64: spc64_test.c ../cpu/spc_ops.h ../cpu/spc_ioregs.h
	@$(CC) -m32 -std=c11 $(FEATURE) -I.. -O2 -Wall -o zsnes_spc32_test spc64_test.c
	@$(CC) -m64 -std=c11 $(FEATURE) -I.. -O2 -Wall -o zsnes_spc64_test spc64_test.c
	@a=$$(./zsnes_spc32_test) && b=$$(./zsnes_spc64_test) && \
	  if [ "$$a" = "$$b" ]; then echo "spc700 32 vs 64: PASS ($$a)"; \
	  else echo "spc700 32 vs 64: FAIL"; echo "  m32 $$a"; echo "  m64 $$b"; exit 1; fi

clean:
	rm -f $(BIN) $(NETPLAY) $(ST10) $(ST11) $(OBC1) $(MSU1) $(DSP2) $(SAI) $(SA1) $(SPC7110) $(VCACHE) $(SPRITES) $(HQX) $(FILTER) $(C4) $(MODE7) $(ASMDATA) $(SAVELOAD)
	rm -f zsnes_spc32_test zsnes_spc64_test
	rm -fr $(PBUILD)
