# Porting Guide: 32-bit x86 Assembly to C11

This documents how to port the remaining 29 `.asm` files (NASM, 32-bit x86) to
portable C11, with the hard-won lessons from the ports already done
(`vcache`, `winintrf`, `endmem`, and the in-progress `7110proc`).

## Goal and golden rule

Replace hand-written asm with C while keeping the build, the test suite, and
real ROMs working. A given `.o` is **all-or-nothing**: to drop a `.c` into the
build and `git rm` the `.asm`, the C must define **every** symbol the asm
exported (`NEWSYM`/`GLOBAL`). One file maps to one `.o`.

## Build, test, verify

    make clean && make -j4           # native ELF build (Linux/SDL3)
    cd test && make run              # unit tests (must stay all-green)
    make clean && make win32 -j4     # i686-w64-mingw32 cross build (PE/COFF)

- Native and win32 share `.o` paths; **always `make clean` when switching**.
- The build auto-adds `-j` (Makefile ~34-36), so build-rule races surface by
  default. Keep generated temp files unique.
- ROM smoke test (rendering ports): ROMs in `~/roms/snes`; run headless with
  `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ASCII_SCREENSHOT_EVERY_FIVE=1`
  (writes `/tmp/zsnes_<seq>.txt` every 5s). Reference cores are cloned at
  `~/clones/snes9x` and `~/clones/bsnes` (read their coprocessor code first).

## What can and cannot be ported cleanly

Classify a file before starting. Symbols are resolved by the linker by name, so:

1. **Pure data symbols** (`resb/resd/resw`, `db/dd/dw`) are language-neutral:
   ABI does not matter, no caller breaks. **Easiest, safest.** (`endmem` was
   the last one; none remain.)
2. **Functions called only from C** can move to C directly (cdecl).
3. **Functions called from asm via the register ABI** (address in ECX, value in
   AL/AX, must preserve ECX/EDX and unused EAX bits) CANNOT just become cdecl C.
   Wrap them with the trampolines in `chips/regabi.h` (`REGABI_REG_READ8/WRITE8`,
   `REGABI_BANK_READ8/READ16/WRITE8/WRITE16`): the macro emits an asm trampoline
   under the public name that calls your `c_<name>` cdecl impl. Delete the
   trampolines only once the asm callers are also C.
4. **The 65816/SPC700 opcode core** (`cpu/table.asm`, `stable.asm`, `tablec.asm`
   `%include` the full opcode set; `execute.asm`, `spc700.asm`, `memory.asm`)
   is the deepest coupling. Do not attempt piecemeal; these go last.

Cross-asm coupling count (how many *other* asm files reference a file's
exports) is the difficulty proxy: 0 (`7110proc`, `c4proc`) is most tractable;
high counts (`makev16b`'s `domosaic16b`/`tileleft16b`, `fxemu2*`) mean many asm
callers depend on the exact register ABI.

> Note: a built `.asm` and a sibling `c_*.c` usually **coexist** with *disjoint*
> symbols (a partial, complementary port), not as replacements. Check symbols
> before assuming a `c_*.c` already replaces its `.asm`.

## The three reusable patterns

- **`asmdata.h`** - force exact data layout/order/adjacency from C via one
  inline-asm block (`ASM_SEC_BSS/DATA`, `ASM_GSYM`, `ASM_SEC_END`). Use whenever
  layout matters (save-state blocks, tables the asm indexes by neighbor offset).
  `endmem.c` is the worked example; `.bss` uses `.skip N`, `.data` uses
  `.fill`/`.rept ... .endr`.
- **`chips/regabi.h`** - register-ABI trampolines (see #3 above). `7110proc.c`
  (compression + math registers) is the worked example.
- **`CSYM(x)`** - per-file macro for symbol naming when you only need a couple of
  inline-asm bridges (see `video/tilecache.c`, `chips/dsp1proc.c`).

## Hard-won gotchas

- **Symbol naming differs by target.** ELF: bare `sym`. PE/COFF and Mach-O:
  `_sym`. The asm `NEWSYM` macro and `asmdata.h`/`regabi.h` already emit both
  forms (`_sym` + a `sym` alias) on non-ELF. Mirror this for any hand-written
  symbol, or links fail on win32 only.
- **`-fdata-sections` + `--gc-sections` do NOT preserve adjacency** between
  separate C globals, and may reorder/gap them. If asm reaches one symbol by
  offset from a neighbor (e.g. `wramdataa`+64K -> `ram7fa`), you MUST group them
  into one array/struct or emit them in a single `asmdata.h` section block.
- **Verify data ports by object identity, not by eye.** Build both objects and
  diff symbols and section bytes; this is conclusive:

      objdump -t old.o | awk '$2=="g"&&($3==".bss"||$3==".data"){print $1,$3,$5}' | sort
      objcopy -O binary --only-section=.data old.o old.bin   # cmp vs new.bin

  `endmem.c` reproduces `endmem.o` byte-for-byte this way.
- **Byte/word punning assumes little-endian x86.** The asm freely does
  `mov al,[SPCMultA+1]`; in C use `((uint8_t*)&x)[n]`. Fine here (always
  `-m32` x86), but don't "clean it up" into endian-portable code unless asked.
- **cdecl at subsystem boundaries.** Any symbol crossing asm<->C or between asm
  subsystems must be cdecl; register passing is allowed only within one
  subsystem.
- **Faithful first, then fix.** Port behavior exactly (bugs included) so the
  object/behavior matches, then fix verified bugs against snes9x/bsnes in a
  separate, clearly-commented step (e.g. SPC7110 divide-by-zero: asm returned
  `0xFFFFFFFF/0xFFFF`; hardware returns quotient 0, remainder = low word of
  dividend).

## Recommended workflow per file

1. Classify (data / C-only fn / register-ABI fn / opcode core).
2. List exports: `grep NEWSYM file.asm`. List externs it uses (coupling surface).
3. Port in **stages** with a unit test per stage (`test/<name>_test.c`, harness
   `test/zstest.h`, register in `test/Makefile`). `7110proc` shows staged ports
   with offset/value assertions matching the asm object.
4. For data: prove byte-identical object (above). For functions: add tests +
   ROM smoke test; cross-check snes9x/bsnes.
5. Swap `SRCS += file.asm` to `file.c` in the `Makefile`; `git rm file.asm`.
6. Run all three builds + tests. Leave changes uncommitted unless asked.

## Style

Brief one-line ASCII comments (no unicode/em-dashes; commas not ` -- `). Return
boolean conditions directly. Match surrounding code. Comment only what needs it.
