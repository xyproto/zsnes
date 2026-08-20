# Porting Guide: 32-bit x86 Assembly to C11

This documents how to port the remaining `.asm` files (NASM, 32-bit x86) to
portable C11, with the hard-won lessons from the ports already done
(`vcache`, `winintrf`, `endmem`, `dspproc`, `makevid`, `newgfx`, `7110proc`,
`memory` and `table`). Only `video/` still has assembly.

## Goal and golden rule

Replace hand-written asm with C while keeping the build, the test suite, and
real ROMs working. A given `.o` is **all-or-nothing**: to drop a `.c` into the
build and `git rm` the `.asm`, the C must define **every** symbol the asm
exported (`NEWSYM`/`GLOBAL`). One file maps to one `.o`.

## Build, test, verify

    make clean && make -j4           # native ELF build (Linux/SDL3)
    cd test && make run              # unit tests (must stay all-green)
    make clean && make win32 -j4     # i686-w64-mingw32 cross build (PE/COFF)
    make portcheck                   # how much of the tree builds for x86-64

- `portcheck` is the cross-CPU scoreboard: 149 of 153 sources build for x86-64,
  and the four that do not are exactly the files with i386 inline assembly
  bridging into `video/*.asm` (`c_makev16b`, `c_makevid`, `c_mode716calc`,
  `tilecache`). Three more compile only because their asm is behind an
  `__i386__` guard and would fail to link (`asm_call.h`, `video/mode716b.c`,
  `video/c_makev16b.c`). Keep it at 149 or better.
- A struct that reaches assembly must size itself from `sizeof(void*)`, not a
  literal: `HDMAInfo` and `SpriteInfo` hold host pointers, and their reserves
  (`cpu/c_regsdata.c`'s `hdmadata`, `ui.c`'s `spritetablea`) are computed from
  the same expression. `ASM_STR()` in `asmdata.h` spells `__SIZEOF_POINTER__`
  into an inline-asm `.fill`. Both are byte-identical at `-m32`; verify that.
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
   ABI does not matter, no caller breaks. **Easiest, safest.** A file becomes
   one of these once its last routine is ported out, so re-check: `objdump -h
   file.o | awk '$2==".text"'` showing size 0 means it is now pure data.
   `endmem`, `dspproc`, `makevid` and `newgfx` were done this way; none remain.
2. **Functions called only from C** can move to C directly (cdecl).
3. **Functions called from asm via the register ABI** (address in ECX, value in
   AL/AX, must preserve ECX/EDX and unused EAX bits) CANNOT just become cdecl C.
   This is now history for `cpu/` and `chips/`: nothing in assembly calls a
   memory or I/O register handler any more, so the `chips/regabi.h` macros
   (`REGABI_REG_*`, `REGABI_BANK_*`) are plain C over the seam described below.
   If a *new* register-ABI caller turns up in `video/`, `REGABI_ENTRY`/
   `REGABI_SYM` are still there to write a trampoline with.
4. **The 65816/SPC700 opcode core** is done: `cpu/` has no assembly left. What
   survives of that era is the pushad register block (`u4 r[8]`, see
   `cpu/c_dispatch.h`), which is C but still models x86 - and still stores host
   pointers in 32-bit slots, which is what pins the build to `-m32`.

Cross-asm coupling (how many symbols a file needs from another `.asm`) is the
difficulty proxy, because those callers depend on the exact register ABI. Five
files are left, in three independent clusters, all in `video/`:

| cluster | asm->asm symbols | note |
| --- | --- | --- |
| `video/mode716.asm` | 1 (`domosaicng16b`) | smallest; bridged from `video/c_mode716calc.c` |
| `video/mv16tms.asm` + `video/makev16t.asm` | 3, mutual | port as one unit |
| `video/newgfx16.asm` + `video/newg162.asm` | 21, mutual | port as one unit; `newg162.o` alone is 664KB of macro-expanded `.text` |

Recompute it rather than trusting the table:

    nm -u a.o | awk '{print $2}' | sort -u > /tmp/und
    nm --defined-only -g b.o | awk '{print $3}' | sort -u | comm -12 - /tmp/und

> Note: a built `.asm` and a sibling `c_*.c` usually **coexist** with *disjoint*
> symbols (a partial, complementary port), not as replacements. Check symbols
> before assuming a `c_*.c` already replaces its `.asm`.

## The four reusable patterns

- **`asmdata.h`** - force exact data layout/order/adjacency from C via one
  inline-asm block (`ASM_SEC_BSS/DATA`, `ASM_GSYM`, `ASM_SEC_END`). Use whenever
  layout matters (save-state blocks, tables the asm indexes by neighbor offset).
  `endmem.c` is the worked example; `.bss` uses `.skip N`, `.data` uses
  `.fill`/`.rept ... .endr`.
- **`chips/regabi.h`** - register-ABI trampolines (see #3 above). `7110proc.c`
  (compression + math registers) is the worked example.
- **`cpu/memseam.h`** - the seam convention that replaced the register ABI on
  the memory *and* I/O register paths, and the pattern to reach for when a whole
  table of handlers has to change ABI at once. A memtable/`Bank0dat` handler is `void f(void)`:
  the bank is in `MemSeamB`, the address in `MemSeamC`, the value in `MemSeamA`
  (al/ax) and the core's edx in `MemSeamD`, and the caller reads all four back.
  Call sites (`mem_call` in `cpu/ops65816.h`, `mem_dispatch` in
  `cpu/memtable.h`) save and restore the seam, so it behaves like the
  callee-saved register set it replaces even when a register write starts a DMA
  that reenters. `MEMBANK_READ8/READ16/WRITE8/WRITE16` wrap a cdecl `c_<name>`
  body in it. `cpu/mem_ops.h` + `cpu/c_memops.c` are the worked example: they
  took over all 87 of `cpu/memory.asm`'s exports, and the chip bank handlers
  (`obc1proc`, `c4proc`, `dsp1proc`, `dsp3proc`, `dsp4proc`, `7110proc`)
  followed, which is what let their hand-written i386 dispatch shims become
  plain C. `chips/regabi.h` then followed for the ~300 I/O register handlers,
  which took `cpu/mem_ops.h`'s dispatch, `cpu/dma.c` and `cpu/c_dma.c` with it.
  The move is only safe once *no* `.asm` calls any entry in the table: check
  with `grep -w <sym>` over `*.asm`/`*.inc`/`*.mac` first.

  Two things the difftests need when a table changes ABI. The oracle is still
  assembly, so it needs its own register-ABI face for anything it calls into
  (`ORACLE_BANK_*` in `test/difftest_memops.c`), and where both sides share a
  stub table the two faces have to be swapped in per side. And the harness has
  to read each side back its own way - registers for the oracle, seam for the
  port (the `seam` flag on `run()` in `test/difftest_regs.c`).
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

  `endmem.c`, `cpu/dspproc.c`, `video/makevid.c` and `video/newgfx.c` each
  reproduce their `.o` byte-for-byte this way. Watch for NASM's `ALIGN32`
  macro: it is `times ($$-$) & 1Fh nop`, so it pads with **0x90**, not zeroes,
  and it aligns relative to the section start. Use `.balign 32, 0x90`.
  Relocations count too: `objdump -r` must match. To name a symbol from
  *another* object, use `ASM_SYMREF(sym)`; the plain alias `ASM_GSYM` leaves
  behind is file-local, so a bare name links on ELF and fails only on win32.
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
