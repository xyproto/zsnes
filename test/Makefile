# test/Makefile — ZSNES2 headless test suite
#
# Usage:
#   make          — build the test binary
#   make run      — build and run
#   make clean    — remove build products
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
SRC       := main.c
NETPLAY_SRC := netplay_test.c
ST10_SRC  := st10_test.c
ST11_SRC  := st11_test.c
OBC1_SRC  := obc1_test.c
MSU1_SRC  := msu1_test.c
DSP2_SRC  := dsp2_test.c
SAI_SRC   := 2xsaiw_test.c
OBJS      := ../zpath.o

.PHONY: all run clean

all: $(BIN) $(NETPLAY) $(ST10) $(ST11) $(OBC1) $(MSU1) $(DSP2) $(SAI)

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

run: all
	@./$(BIN)
	@./$(NETPLAY)
	@./$(ST10)
	@./$(ST11)
	@./$(OBC1)
	@./$(MSU1)
	@./$(DSP2)
	@./$(SAI)

clean:
	rm -f $(BIN) $(NETPLAY) $(ST10) $(ST11) $(OBC1) $(MSU1) $(DSP2) $(SAI)
