# ZSNES2 Further Porting Plans

This document describes the remaining work to make ZSNES2 build and run
without any 32-bit x86 Assembly (the `NO_ASM=1` build target).

## Current state (branch: `cport`)

The `NO_ASM=1` build replaces the following ASM files with C equivalents:

| ASM file | C replacement | Status |
|---|---|---|
| `endmem.asm` | `c_endmem_data.c` | ✅ done |
| `init.asm` | `init_data.c` | ✅ done |
| `vcache.asm` | `c_vcache_asm.c` | ✅ done |
| `gui/gui.asm` + `gui/guiwindp.inc` | `gui/c_gui_data.c` | ✅ done |

The NO_ASM build compiles and runs; all 249 unit tests pass and Zelda runs
visually correctly (`vtest.sh` returns 0).

The ASM build is unchanged and still passes its 127 tests.

---

## Remaining ASM files

### 1. `cpu/memory.asm` — **highest priority, unblocks everything else**

~2 200 lines. The SNES memory-mapping and dispatch layer. All memory reads
and writes from the 65816 and SPC700 cores pass through here. It holds:

- The `MemMap` read/write function-pointer tables (one entry per 256-byte
  bank region, ~512 entries each).
- Low-level bank-switch and chip-enable helpers.
- The `asm_call(MainLoop)` trampoline that drives the 65816 execution loop
  and the SPC700 sync loop.

**Why it unblocks everything:** the chip ASM files (OBC1, DSP1, C4, SA1,
SuperFX, 7110) are all called from inside the memory-dispatch hot path via
the register-based ABI (`ecx` = address, `al` = data). Once `memory.asm` is
in C those call sites become ordinary C function pointers and each chip can
be replaced independently.

**Suggested approach:**
1. Audit `cpu/c_memory.c` (the existing partial C translation). Identify
   which read/write handlers still delegate to ASM stubs.
2. Define a `typedef u1 (*mem_read_fn)(u4 addr)` / `typedef void
   (*mem_write_fn)(u4 addr, u1 val)` in a new header (`cpu/memmap.h`).
3. Build the `MemMap` tables as C arrays of those function pointers.
4. Wrap each remaining ASM handler in a thin C shim behind `#ifndef
   NO_ASM` / `#ifdef NO_ASM` guards.
5. Add unit tests for read/write round-trips on ROM, SRAM, WRAM, and at
   least one co-processor (SA1 or SuperFX).

### 2. `cpu/spc700.asm` — SPC700 audio CPU (~2 650 lines)

The SPC700 is the audio co-processor inside the SNES. It runs independently
of the main 65816 CPU.

`cpu/c_stable.c` already has a partial C translation of the SPC700 opcode
table. The main work is translating the interpreter loop itself.

**Suggested approach:**
1. Replace the opcode dispatch table in ASM with the existing C version in
   `cpu/c_stable.c`.
2. Write a pure-C `RunSPC(u4 cycles)` function that drives the interpreter
   loop using a `switch` or a C function-pointer table.
3. Gate the new function behind `NO_ASM`; the existing `spc700.asm` path
   remains for the ASM build.
4. Key data structures to port: `SPCReg` (the SPC700 register file),
   `SPCMem` (64 KB SPC RAM), `DSPReg` (128 DSP registers).

### 3. `cpu/execute.asm` — 65816 main CPU (~1 400 lines)

The innermost execution loop for the main SNES CPU. Depends on
`memory.asm` being in C first (see §1).

`cpu/c_execute.c` contains a substantial C translation already, but still
has five `asm_call()` sites:

| Call site | Reason still ASM |
|---|---|
| `asm_call(MainLoop)` ×2 | CPU+SFX execution loop |
| `asm_call(InitFxTables)` | SuperFX table init |
| `asm_call(ProcessSoundBuffer)` ×3 (audio.c) | SPC700 sync |
| `asm_call(initSPC7110regs)` | 7110 chip init |
| `asm_call(SPC7110Reset)` | 7110 chip reset |

Once `memory.asm` and `spc700.asm` are replaced, these call sites collapse
into ordinary C function calls and `c_execute.c` becomes pure C.

### 4. Chip co-processors (blocked on §1)

All five files use the same register-based call ABI and can be tackled in
any order once `memory.asm` is done:

| File | Lines | Notes |
|---|---|---|
| `chips/fxemu2.asm` + `fxemu2b.asm` + `fxemu2c.asm` + `fxtable.asm` | ~9 000 | SuperFX/GSU — large but self-contained |
| `chips/c4proc.asm` | ~2 200 | C4 geometry chip |
| `chips/sa1proc.asm` + `sa1regs.asm` | ~1 500 | SA-1 CPU (a second 65816) — complex |
| `chips/dsp1proc.asm` | ~820 | DSP-1 math chip |
| `chips/obc1proc.asm` | ~150 | OBC1 (Object Controller) — simplest |
| `chips/7110proc.asm` | ~1 040 | S-DD1 / SPC7110 decompressor |

### 5. Remaining video ASM (~17 000 lines)

Most video rendering is still in ASM. It renders correctly via the existing
C wrappers (`video/c_newgfx_asm.c`, etc.) that call the ASM, so it is not
blocking the NO_ASM build today — **but only because the NO_ASM build
still links those ASM `.o` files**.

To make the build fully architecture-independent the video ASM must
eventually be replaced. Suggested order (smallest-first):

1. `video/mode716b.asm`, `mode716d.asm`, `mode716t.asm` — short (~200–400 lines each)
2. `video/mode716.asm`, `mode716e.asm`, `m716text.asm` — medium
3. `video/newgfx.asm`, `newgfx16.asm`, `newg162.asm` — contain the
   `WinBGCheck` macro that caused the contiguous-layout bug; port carefully
4. `video/makevid.asm`, `makev16b.asm`, `mv16tms.asm`
5. `video/makev16t.asm` — largest video file (~5 300 lines)

---

## Longer-term: BSNES emulation core

A separate idea (discussed at the start of this project) is to keep the
ZSNES GUI but drive it with the BSNES emulation core
(`~/clones/bsnes`) instead of the ZSNES CPU/memory/chip code. This
would side-step porting the 65816/SPC700/chip ASM entirely, at the cost of
integrating two very different codebases. Pre-requisites:

- The ZSNES video back-end abstraction (`video_backend_t`) is already in
  place and would be the main seam.
- A thin adapter layer would need to translate BSNES per-frame callbacks
  (video buffer, audio samples, controller polling) into ZSNES GUI calls.
- BSNES is C++17 / x86-64-clean; it would need to be compiled as a shared
  library or as a static archive linked into the ZSNES build.

---

## Testing strategy

- **Unit tests** (`make test`): extend `test/` with coverage for each new C
  translation (memory read/write, SPC700 opcode execution, chip register
  access).
- **Visual regression** (`vtest.sh`): run after every significant change.
  Consider adding a second ROM (e.g. a SuperFX title like Star Fox or
  Yoshi's Island) to catch chip-related regressions.
- **Differential testing**: run the same ROM for the same number of frames
  in both the ASM build and the NO_ASM build and compare the final
  framebuffer pixel-for-pixel. This would catch subtle emulation differences
  early.
