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
SRC       := main.c
NETPLAY_SRC := netplay_test.c
ST10_SRC  := st10_test.c
OBJS      := ../zpath.o

.PHONY: all run clean

all: $(BIN) $(NETPLAY) $(ST10)

$(BIN): $(SRC) zstest.h $(OBJS)
	$(CC) $(CFLAGS) $(SRC) $(OBJS) -no-pie -Wl,--gc-sections -o $(BIN)

$(NETPLAY): $(NETPLAY_SRC) zstest.h
	$(CC) $(CFLAGS) $(NETPLAY_SRC) -no-pie -lpthread -o $(NETPLAY)

$(ST10): $(ST10_SRC) zstest.h ../chips/st10proc.c
	$(CC) $(CFLAGS) $(ST10_SRC) ../chips/st10proc.c -no-pie -o $(ST10)

run: all
	@./$(BIN)
	@./$(NETPLAY)
	@./$(ST10)

clean:
	rm -f $(BIN) $(NETPLAY) $(ST10)
