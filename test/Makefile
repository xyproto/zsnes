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
SRC       := main.c
NETPLAY_SRC := netplay_test.c
OBJS      := ../zpath.o

.PHONY: all run clean

all: $(BIN) $(NETPLAY)

$(BIN): $(SRC) zstest.h $(OBJS)
	$(CC) $(CFLAGS) $(SRC) $(OBJS) -no-pie -Wl,--gc-sections -o $(BIN)

$(NETPLAY): $(NETPLAY_SRC) zstest.h
	$(CC) $(CFLAGS) $(NETPLAY_SRC) -no-pie -lpthread -o $(NETPLAY)

run: all
	@./$(BIN)
	@./$(NETPLAY)

clean:
	rm -f $(BIN) $(NETPLAY)
