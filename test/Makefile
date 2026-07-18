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

CC     := gcc
CFLAGS := -m32 -std=gnu99 -I.. -Wall -Wextra -Wno-unused-parameter

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
SRC       := main.c
NETPLAY_SRC := netplay_test.c
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
C4        := zsnes_c4_test
C4_SRC    := c4proc_test.c
MODE7     := zsnes_mode716b_test
MODE7_SRC := mode716b_test.c
OBJS      := ../zpath.o

.PHONY: all run clean

all: $(BIN) $(NETPLAY) $(ST10) $(ST11) $(OBC1) $(MSU1) $(DSP2) $(SAI) $(SA1) $(SPC7110) $(VCACHE) $(SPRITES) $(C4) $(MODE7)

$(BIN): $(SRC) zstest.h $(OBJS)
	$(CC) $(CFLAGS) $(SRC) $(OBJS) -no-pie -Wl,--gc-sections -o $(BIN)

$(NETPLAY): $(NETPLAY_SRC) zstest.h
	$(CC) $(CFLAGS) $(NETPLAY_SRC) -no-pie -lpthread -o $(NETPLAY)

$(ST10): $(ST10_SRC) zstest.h ../chips/st10proc.c
	$(CC) $(CFLAGS) $(ST10_SRC) ../chips/st10proc.c -no-pie -o $(ST10)

$(ST11): $(ST11_SRC) zstest.h ../chips/st11proc.c
	$(CC) $(CFLAGS) $(ST11_SRC) ../chips/st11proc.c -no-pie -o $(ST11)

$(OBC1): $(OBC1_SRC) zstest.h ../chips/obc1proc.c
	$(CC) $(CFLAGS) $(OBC1_SRC) ../chips/obc1proc.c -no-pie -o $(OBC1)

$(MSU1): $(MSU1_SRC) zstest.h ../chips/msu1regs.c
	$(CC) $(CFLAGS) $(MSU1_SRC) ../chips/msu1regs.c -no-pie -o $(MSU1)

$(DSP2): $(DSP2_SRC) zstest.h ../chips/dsp2proc.c
	$(CC) $(CFLAGS) $(DSP2_SRC) ../chips/dsp2proc.c -no-pie -o $(DSP2)

$(SAI): $(SAI_SRC) zstest.h ../video/2xsaiw.c
	$(CC) $(CFLAGS) $(SAI_SRC) ../video/2xsaiw.c -no-pie -o $(SAI)

$(SA1): $(SA1_SRC) zstest.h ../chips/sa1regs.c
	$(CC) $(CFLAGS) $(SA1_SRC) ../chips/sa1regs.c -no-pie -o $(SA1)

$(SPC7110): $(SPC7110_SRC) zstest.h ../chips/7110proc.c
	$(CC) $(CFLAGS) $(SPC7110_SRC) ../chips/7110proc.c -no-pie -o $(SPC7110)

$(C4): $(C4_SRC) zstest.h ../chips/c4proc.c ../chips/c4emu.c
	$(CC) $(CFLAGS) $(C4_SRC) ../chips/c4proc.c ../chips/c4emu.c -no-pie -lm -o $(C4)

$(MODE7): $(MODE7_SRC) zstest.h ../video/mode716b.c
	$(CC) $(CFLAGS) $(MODE7_SRC) ../video/mode716b.c -no-pie -o $(MODE7)

$(VCACHE): $(VCACHE_SRC) zstest.h ../video/tilecache.c
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections $(VCACHE_SRC) ../video/tilecache.c -no-pie -Wl,--gc-sections -o $(VCACHE)

$(SPRITES): $(SPRITES_SRC) zstest.h ../video/sprites.c
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections $(SPRITES_SRC) ../video/sprites.c -no-pie -Wl,--gc-sections -o $(SPRITES)

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
	@./$(C4)
	@./$(MODE7)

clean:
	rm -f $(BIN) $(NETPLAY) $(ST10) $(ST11) $(OBC1) $(MSU1) $(DSP2) $(SAI) $(SA1) $(SPC7110) $(VCACHE) $(SPRITES) $(C4) $(MODE7)
