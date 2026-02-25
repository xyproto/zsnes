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

BIN  := zsnes_test
SRC  := main.c test_endmem.c test_init.c
# Object files from the parent build that contain the functions under test.
# These must already exist (built by `make` in the parent directory).
ifdef NO_ASM
OBJS := ../zpath.o ../c_endmem_data.o ../init_data.o
else
OBJS := ../zpath.o ../endmem.o ../init.o
endif

.PHONY: all run clean

all: $(BIN)

$(BIN): $(SRC) zstest.h $(OBJS)
	$(CC) $(CFLAGS) $(SRC) $(OBJS) -no-pie -Wl,--gc-sections -o $(BIN)

run: all
	@./$(BIN)

clean:
	rm -f $(BIN)
