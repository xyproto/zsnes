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

    make linux32                     # 32-bit x86 ELF (the only complete build)
    cd test && make run              # unit tests (must stay all-green)
    make win32                       # i686-w64-mingw32 cross build (PE/COFF)
    make portcheck                   # how much of the tree builds for x86-64
    make help                        # every target, and what still blocks it

`make linux64`, `win64`, `linux_arm64` and `linux_pi4` exist and are wired up,
and refuse with a message naming the files still in the way. They are the
finish line: when the last `.asm` goes, they build.

- The oracle for a difftest is the original assembly, pulled from git by
  `tools/mkoracle.py`. Give it **both** `--requires` (a pattern only the
  original has, usually the `%include` of the code under test) and
  `--ported-marker` (a pattern the port introduces). Either alone picks the
  wrong revision: the marker's absence also matches the *finished* port once it
  deletes its own scaffolding, and `--requires` alone lands on a half-ported
  revision whose routines are already thunks. The three 65816 targets were
  silently broken this way - `make op` built an oracle with no opcodes in it.
  Check the revision it prints. `mkoracle.py` now refuses when the revision it
  picked is newer than every revision carrying the marker, which is exactly the
  post-port case, and says to add `--requires`. Most video targets rely on the
  default marker (`call c_`) and are fine until their file is ported out - at
  which point the guard fires rather than the oracle going quietly wrong.
- **The 64-bit build links and runs.** `tools/mkstub64.py` emits an empty
  definition for every symbol the video assembly exports, which is enough to
  build an x86-64 binary and find out whether everything *else* works. It gets
  from `main` through `zstart`, `init`, `start65816`, `continueprog`,
  `execute` and `exec_loop` into the opcode dispatch before faulting - so ROM
  loading, the memory map, the coprocessor setup and the CPU entry all survive
  the word-size change. It renders nothing; it is a runtime test of the core,
  not a usable build.
- `portcheck` is the cross-CPU scoreboard, and it now reads **160 of 160**:
  every source compiles for x86-64. That is not the same as linking - the
  remaining `video/*.asm` still has to go - but no C file is i386-only any
  more. Keep it at 160. The i386 inline asm that is left is all *bridges into
  the assembly*: `asm_call.h`, `video/tilecache.c`, `video/mode716b.c` and
  `video/c_makev16b.c`. `video/c_makevid.c` had the last one outside that set,
  a hand-written `call newengine16b` with every register declared clobbered,
  and it went when that routine became C.
- **Anything holding a host pointer must be sized from `sizeof(void*)`.** This
  is the single recurring 64-bit blocker and it has turned up three times:
  `HDMAInfo` and `SpriteInfo` (structs with a pointer member, asserted at a
  literal size), and then 38 tables in `endmem.c` reserved with `ED()` -
  `.skip n*4`, because `resd` meant dword in the assembly - while C declares
  them `opfn* tableA[256]`. On 64-bit the reservation is half what C indexes,
  and the opcode dispatch reads a table entry that was never written: that is
  what the 64-bit run faults on. `EP()` sizes them from `__SIZEOF_POINTER__`
  instead; `.bss` goes 277680 -> 316592 bytes at `-m64` and is unchanged at
  `-m32`. `ASM_STR()` in `asmdata.h` spells the macro into inline asm. Always
  prove the 32-bit object is untouched.

  To find the rest: for every `ED(sym, ...)` check whether the C declaration of
  `sym` has a `*` in it.
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
   survives of that era is the pushad register block (see `cpu/c_dispatch.h`),
   which is C but still models x86. Its slots are `zreg` (`uintptr_t`, see
   `types.h`), not `u4`: four hold 32-bit registers whose upper bits are part of
   the behaviour, and esi, ebp, edi and transiently eax hold host pointers. On
   i386 the two types are identical, so the change was a no-op there - the
   difftests prove it - and the slots are wide enough elsewhere.

Cross-asm coupling (how many symbols a file needs from another `.asm`) is the
difficulty proxy, because those callers depend on the exact register ABI. Three
files are left, in two independent clusters, all in `video/`:

| cluster | asm->asm symbols | note |
| --- | --- | --- |
| `video/makev16t.asm` | 0 | `calldl16t` alone; goes last, see below |
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

## What is left, and in what order

41 exports across three files, and **every one of them is now an entry seam** -
`pushad` / `ccall c_<name>` / `popad` / `ret` and nothing else. No routine
body is left in assembly anywhere in the tree. What remains is the register
ABI itself: those seams exist because their callers still reach them with the
whole register set live, through `calldl16t`.

So the last step is not another port; it is removing a calling convention.
`video/c_ngprocbg.c` calls the sixteen background dispatchers through
`calldl16t` (`DLR` / `DLFN`), and `video/c_ngbg.c` calls the twenty renderers
in `newg162.asm` the same way. Change those call sites to take the C entry
points (`c_drawbg1line16b(r)`, `c_ng_drawtileng2b16b(r)`) with a register
block the caller owns, and all 41 seams plus `calldl16t` go together. That is
one coordinated change, and it is the only thing between here and no assembly
at all.

| unit | exports | .text | note |
| --- | --- | --- | --- |
| `newgfx16.asm` | 20 | 626B | twenty entry seams; everything else is C |
| `newg162.asm` | 20 | 640B | twenty entry seams; everything else is C |
| `makev16t.asm` | 1 | 97B | `calldl16t`; goes with them |

1,363 bytes of `.text` in total, from about 750KB when this started.

`newg162.asm` is going leaf by leaf rather than as one lump, and its entry
points stay put while that happens: each is a gating tree whose arms are
`drawtile16b`/`drawtile16bw` macro expansions, and an arm becomes a `pushad` /
`ccall c_<leaf>` / `popad` seam one at a time. The three shared gate macros
(`determinetransp`, `CheckWindowing`, `DetermineWindow`) went first
(`video/c_ng2gate.c`), then twelve of the twenty routines, twelve arms each -
four non-windowed and eight windowed - in `video/c_ng2tile.c`: the six tile
drawers, the six 8x8 and 16x16 line drawers, the two 16x8 hi-res ones (four
arms each, no windowed variants) and the six offset-mode ones - **all twenty
routines, all 236 arms** - and then the twenty gating trees above them.
`video/newg162.mac` and `video/newg16wn.mac` are gone with the renderers (83
macros), and `newg162.asm` is down from 665KB of `.text` to 1.6KB: twenty
`pushad` / `ccall` / `popad` entry seams and nothing else.

The trees turned out to be two shapes, not twenty: eighteen routines share a
twelve-leaf tree and the 16x8 pair has four leaves and no windowed arms. They
have to run on the register block rather than on plain values, because the
three decisions are not pure - `c_determinetransp` moves ecx and edi onto the
sub screen and `c_determinewindow` moves ecx between the two window tables, and
the leaf reads both. With the tree in C there is one seam per routine instead
of twelve, so every leaf has to leave `ng2_mosaic` definite: the seam that
reads it is now at the entry, not next to the leaf that set it.

Those twenty seams can only go when `newgfx16.asm` does. It reaches them by
`jmp` with the registers live, which is the same register-ABI knot every other
cluster had.

The offset-per-tile drawers - modes 2, 4 and 6, where BG3 supplies a per-column
scroll offset - are the intricate ones. The tile address and the row inside the
tile are recomputed for every tile, and that computation *is* the per-tile
tail. ax is the low word of the map pointer, kept in ofsmmptr and copied back
each tile, so every step is a 16-bit add. Four things vary between the call
sites and only reading them side by side reveals it:

- the body macro advances `ofsmcptr` by `bgtxadd2` when `ofsmcptr2` wraps;
  `WinClipMacroom` and the windowed body do not (`OM_WRAP`);
- the windowed body tests the horizontal entry against `ofsmval` where the
  other two use `ofsmvalh` (`OM_HV_ALT`);
- `mode` is 4 where the two offsets share a word and 2 where they do not. It is
  a per-routine constant: 2bpp and 8bpp pass 4, 4bpp passes 2;
- **the 16x16 walk is not the 8x8 one widened**, and its three tails do not
  even agree with each other. The body macro treats the row index as nine bits
  (carry at 200h, mask 1FFh, shift 4) and sets `taddnfy16x16`/`taddfy16x16`
  from bit 3 for the next half's prologue. `WinClipMacroom16x16` uses the wide
  carry and mask but shifts *three* and sets no flip. The windowed body uses
  the narrow 8x8 form throughout. Deriving any of them from another produces
  code that passes a weak difftest and is wrong - that is exactly what happened
  here, and only filling the offset table caught it.

The 16x16 offset-mode drawers also break the two-half pattern the other 16x16
families follow: they draw *one* half per turn of the outer loop and come back
round for the other, so `switch16x16` gates which half the prologue starts on,
whether the map pointers step, and whether `tleftn` counts down.

The 16x8 pair splits a 16-pixel tile across two interlace fields - field 1 is
75036*4 bytes on, and each field's sub screen is the usual SUB from its own
main - so a half covers four screen pixels per field and edi advances eight,
not sixteen. When the mode is not really hi-res (`curmosaicsz` above 1, or
`res640` clear) a second writer set keeps one field and drops the other, and
those two disagree about which source pixel survives: the full path keeps the
*odd* one, the partial path the *even* one, because the partial writers are
`%if %2<8` and that drops the field-1 calls. Not a symmetry to tidy up.

Every 16x16 drawer is its 8x8 counterpart with a two-half walk wrapped round
it, so port the 8x8 one first and factor the half out - `draw_half`,
`draw_half_win`, `line_half`, `line_half_win`. The walk itself is identical
each time, and so are its two traps: the half index lives in *cx*, so stepping
it is a 16-bit add that wraps inside the low word while the high half stays
where ngptrdat2 put it; and the two halves are counted by toggling the global
`switch16x16` rather than by a counter, so an entry drawn while it is already
set draws one half and not two.

The line drawers are not a variation on the tile drawers, and reading them as
one will mislead you. A line drawer writes a single scanline, and it reads the
*primary* cache - raw palette indices - looking each pixel up in `CPalPtrng` as
it goes, where a tile drawer reads sixteen-bit colours that `docache` converted
up front. So there is no secondary cache and no cache key in a line drawer; a
miss just calls `cachesingleNbng` and carries on. Transparency is the low bits
of the index (`add bl,dl` / `test bl,mask`), which is what the 03h/0Fh/0FFh
macro argument is for - the tile drawers never used it. "Transparent" selects a
second palette 512 bytes on rather than OR-ing UnusedBit, and the sub-first and
main-first windowed writers do not mirror each other: one reads the plain
palette and ORs the bit on for the main copy, the other reads the second
palette and masks it back out for the sub.

Two more that cost time:

- `drawlineng16b`'s full-tile path is dead. The `tltype` test falls through to
  an unconditional `jmp %%parttile`, so every tile takes the per-pixel path
  whatever the cache says. Only that one macro has it, and the 16x16 line
  drawer next door reaches its full path normally - which is why that one needs
  `procpixels`, the dword writer that does two pixels at once with no
  transparency test, and masks the sub copy a dword at a time where the
  single-pixel writers take the low half of the same word.
- `drawlinengwin16b` has no `tltype` test at all, and its window step lives in
  the writer rather than the loop, because a windowed line tile has no
  per-column pass to hang it on.

The 16x16 drawers are the 8x8 ones with an inner loop: a map entry is two 8x8
halves from consecutive cache slots, seventeen entries to a line instead of
thirty-three, and the window runs are consumed sixteen pixels at a time. Two
traps there. The half index lives in *cx*, so every step of it is a 16-bit add
that wraps inside the low word while the high half stays where ngptrdat2 put
it. And the two halves are counted by toggling the global `switch16x16` rather
than by a counter, so an entry drawn while it is already set draws one half and
not two - reproduce that rather than rounding it off.

The windowed arms are a different shape from the plain ones and worth reading
before touching. `ngwintable` is a run-length list of dwords, alternating runs
outside and inside the window; `ngcwinmode` says which kind the current run is
and `ngcpixleft` how much of it is left. A tile with more than eight pixels
left in its run is wholly one or the other and draws with a plain writer (or,
in the `drawtile16bw` arms, not at all); one that straddles a boundary draws a
pixel at a time and walks the list as it goes - so the windowed traversal runs
down the *columns* while the plain one runs across the rows. Three details
that a reading of the writers alone will not give you:

- exhausting the line inside `WinClipMacro`'s skip loop tail-jumps into
  `domosaicng16b`. No other exit does, `drawtile16bw2` has none at all, and
  since `popad` restores eax a return value cannot carry it - `ng2_mosaic` does.
- `.winclipped` in `WinClipMacro` never xors `ng16bprval` into ecx, because it
  never looks at the priority bit. `.loop` and the straddling path both do.
- the last tile of a line returns *without* writing `ngcpixleft` back to the
  run list, because the `tleftn` test comes first.

The three depths are one body with a `depth` descriptor, because that is what
the assembly already was - `drawtileng16b` takes the per-depth tables as macro
arguments. Only two things actually vary: the tile-index mask (4095/2047/1023)
with the matching 1/2/4-byte scale into `vidmemchN`, and `mode0add`, which
`test2ba` adds and the other two do not - mode 0 is what gives each background
its own palette block, and mode 0 backgrounds are 2bpp. `vidmemchNs` and
`tltypeNb` are byte-indexed at every depth; do not "fix" that to match.

`mode716.asm`, `mode716.mac` and `mv16tms.asm` are gone. The two sections below
are kept because their lessons apply to what is left, not because there is work
in them.

`calldl16t` goes last in the whole tree: it is the generic register bridge every
remaining dispatch routes through, and it can only go when nothing on the other
side is assembly. When it does, `DLR`/`DLFN` and every `M7*`/`SPR*`/`CB*`/`TT*`
seam go with it, and `make linux64`, `win64`, `linux_arm64` and `linux_pi4`
become buildable.

### Inside the mv16tms + makev16t unit (done)

Both files are gone; what follows is why the split worked, because the same
shape is what `newgfx16` + `newg162` present.

Not one lump. All seven leaves in `mv16tms.asm` shared the `MVS` seam and were
plain trampolines (`call c_<name>`, reload; two of them then test `MVSMosaic`
as a *byte* and tail-jump, where the dispatchers test it as a dword and set
`dh` first - reproduce that split exactly). The four dispatchers pick a leaf on
`scaddtype`, `scrnon` and `winon`.

The 8x8 chain was separable from the 16x16 one: `draw8x816tms` and its leaves
reach nothing 16x16, so that half plus the four `makev16t` tile routines
(`draw8x816t`, `draw8x816bt`, `draw16x816t`, `draw8x816toffset`) converted on
their own (`video/c_mv16draw.c`, `c_mv16msgate.c`, `c_mv16leaf.c`), taking
`makev16t.asm` to `calldl16t` alone; the 16x16 half followed. Traps found while
scoping it:

- `draw8x816bt`'s first branch is not a tail-jump but `ccallv
  draw8x816boffset, eax, ecx, edx, ebx, ebp, esi, edi` - a seven-argument cdecl
  call - and only *then* the bgmode 5 jump. Read it, do not pattern-match it
  against its neighbours.
- `draw16x1616tms` is the one routine here that was not a trampoline: 112 lines
  of real setup (mosaic buffer clear, y adders, window pointers) before it
  reaches `c_draw16x1616tms_body`. It is `c_draw16x16tms_setup` in
  `video/c_mv16tms.c` now, and it hides a store the C has to spell out: `mov
  [temp],eax` is a *dword* into four adjacent bytes (`temp`, `bshifter`,
  `a16x16xinc`, `a16x16yinc` - `video/makevid.c`), and the very next
  instruction reads `a16x16yinc` back, i.e. bits 24-31 of the caller's eax.
- The `*ms` window path is taken when `curmosaicsz` **is** 1 and `winon` is set
  (`cmp byte[curmosaicsz],1 / jne .domosaic`), not when it differs. Easy to
  invert.

### A dispatcher and its branch target must not share a translation unit

The hard-won one, and it sank a first attempt at this unit. `difftest_t8t.c`
and `difftest_t8bt.c` substitute stubs for `draw16x816t` and
`draw8x816toffset` to observe *which way the dispatch went*. If the ported
`draw8x816t` calls the ported `draw16x816t` from the same `.c`, that call
cannot be intercepted, the difftest runs the real drawer instead of its stub,
and every iteration diverges from the first.

So the file layout is not free: **whatever a difftest stubs has to be in its own
translation unit**, and the targets that want the real one link it while the
ones that stub it do not. Work out which difftest stubs what *before* writing
the files - `grep -l 'stub-routine' test/Makefile` and read the lists. The same
constraint applies to any branch a harness observes, and it is the file-scope
cousin of the tail-jump rule above.

### A register the harness forgets looks like a defect in the code

`test/ng2_harness.h` drives `newg162.asm`'s twenty drawers by `jmp` with the
registers loaded by hand. It loaded six of the seven: `NG2_EBP` was set by the
test and never moved into `%ebp`, because the macro already mentions `ebp`
twice - saving and restoring it around the call - so it reads as handled.

The symptom was not "wrong colours". It was that six of the twenty routines
compared *unequal against byte-identical assembly*, reproducibly, and they were
exactly the tile drawers - a clean enough pattern to look like a property of
that family. It is not: those six read the palette as `[ebp+ebx*2]`, so they
were indexing the harness's own frame pointer, which sits at a different
address for the two call sites being compared. The line drawers load `ebp`
themselves before using it, which is why they looked fine.

Two things to take from it. A hand-written register bridge needs its loads
checked against the entry contract one register at a time - the header even
documented `ebp` as the easy one to miss. And when a difftest fails, run it
against *itself* first: identical code on both sides must compare equal, and if
it does not, the harness is what is broken. `difftest_ng2.c` keeps that control
written down.

### Zeroed state is the quietest way for a difftest to prove nothing

`difftest_ng2.c` compared bit-identically for weeks over a **zero-filled tile
map**. `vrama` was never assigned, so mkoracle stubbed it at 4096 zero bytes:
every tile value was zero, which is no flip bits, one palette and one tile
index. The offset-per-tile table behind it was the same - a zeroed static, with
`ofsmval`/`ofsmvalh` left at zero so no entry ever passed the test that gates
the whole offset computation.

Nothing about the output says this. The routines paint, the leaf counters
count, and both sides agree - because both sides read the same zeros. Filling
`vrama` and the offset table and giving the mask variables real values turned
up a genuine bug in the 16x16 offset-mode walk within one run.

So: a difftest's inputs need auditing the same way its branch coverage does.
Anything a generated stub supplies is zero unless the harness says otherwise.
Two ways to check without guessing - print how many bytes of the output buffer
the routine actually changed (all-zero input often means a suspiciously round
number), and **mutate the port and confirm the difftest fails**. A mutation
that survives is either dead code or untested input. Both wraps in the
offset-mode tail survived their mutation until the table was filled; one of
them still does, for a structural reason worth knowing - it fires on the 32nd
of 33 advances and each advance only feeds the *next* tile, so on a 33-tile
line its effect lands in the final advance, which nothing reads.

### getenv() is true for an empty string

A knob written `if (getenv("NOC")) skip_the_c_side();` is on when the variable
is *set to anything*, including nothing. `env NOC= ...` sets it to "", getenv
returns a non-NULL pointer to that empty string, and the side is skipped.

This cost an hour. A run of `exit=1` readings that looked like real comparison
failures were the C side never executing, and the conclusion drawn from them -
"the port mismatches on the plain branch" - was wrong; that branch is
bit-identical. Use `env -u NAME` to clear a knob, and if a knob means "off
unless truthy", test the value rather than the presence.

The same shape of mistake, one layer up: three scripted edits to the harness
silently did not apply because the anchor text had drifted, and the marker that
"never printed" was read as evidence about *where* the crash was. Any patch
applied by script should assert its anchor matched, and any conclusion drawn
from an absent print should first confirm the print exists in the binary.

### A recorder that outgrows its counter smashes the caller's frame

`difftest_bgt.c` tallied which renderer the dispatcher picked with
`u4 counts[7]` indexed by the recorder's `rec_which`. That was right while only
the six tile renderers had recorders. Adding the fourteen line ones took
`rec_which` to 20, and `counts[13]++` wrote into `main`'s frame - over the
return-address slot of the *next* call, so the run died one iteration later,
in a routine that had nothing to do with the one that had misbehaved.

It cost two sessions, because everything about it pointed somewhere else. The
iteration that died was a priority-1 *tile* pass, which passes 400,000 times
on its own; the branch it was blamed on was the 8x8 offset-per-tile one,
because that was the newly wired code; and it would not reproduce with the
stack at a fixed address, which reads like a pointer bug in the assembly.

What actually found it was refusing to trust the frame the debugger printed.
Hand-written asm carries no CFI, so `bt` through `calldl16t` invents its
callers; the useful facts came from stepping one instruction at a time and
watching `esp`. The moment worth recognising: the dispatcher returned cleanly,
`calldl16t` returned cleanly, and the fault was in the *wrapper's* `ret`, whose
return address was already 0 when the call started. A return address that is
wrong before the call is not the callee's doing - stop looking at the callee.

Two smaller traps in that hunt. Watchpoints on `$esp`-relative expressions are
re-evaluated as `esp` moves, so they fire on the callee's own pushes; capture
the address into a convenience variable first. And two `static void f(void)`
wrappers can get different frame sizes from gcc, so `esp` at the same inner
call site differing by 4 between two iterations is normal, not a leak.

### Random bytes are not a realistic distribution

`BGMA` holds the scanline's SNES mode, 0..7. `difftest_bgt.c` filled it with
`dt_fill`, so the two values that select offset-per-tile came up twice in 256:
the whole offset-mode family got 29 hits in 40,000 iterations, three of them
on one leaf. Filling it with `dt_mod(8)` - what the emulator actually puts
there - took the same family to 759 and no leaf below 73.

This is the same failure as `t16x161` and `winbuf` above, and it has the same
tell: a hit counter that is small rather than zero. Zero is easy to notice.
Look at the small ones too, and ask what the field means before deciding how
to fill it.

### A leaf nothing reaches is not a leaf that works

Porting the `newg162` windowed arms took three goes at the harness before the
comparison meant anything, and each failure looked like success:

- the four `drawtile16bw2` arms reported **zero hits**. `difftest_ng2.c` pinned
  `CMainWinScr == CSubWinScr == 0`, so `c_determinewindow`'s second probe
  re-read the byte its first probe had just tested non-zero - "main only" and
  "sub only" were unreachable by construction. Give the two window tables
  separate storage, as the emulator has.
- `winbuf` was filled with uniform random bytes, but the gate branches on a
  byte being *zero*. That is one time in 256, against a few dozen calls. Real
  window tables are mostly zero; filling half and half reached all four
  branches.
- even then two of the eight came up empty from run to run: 256 iterations over
  32 gating combinations is about a dozen samples per family, split three ways.
  1024 covers all eight.

The tell each time was a hit counter reading zero while the pass line read
20/20, which is why the counters are there. Add one per leaf as you port it,
print them, and treat a zero as a failure to test rather than a routine that
happens not to run - `ng2_leafhits` and `ng2_winhits` do this, and
`ng2_mosaic_hits` does it for a tail-jump that writes nothing observable.

### Inside newgfx16: the background dispatchers

Sixteen of them - four layers x tile/line x two priority passes - built from
four macros, and about 43KB of the file's 48KB. Each works out where the
layer's map and palette are for this scanline, builds the window, and hands the
whole register set to a renderer in `newg162.asm`. All sixteen are
`video/c_ngbg.c` now (`bgt`), `video/newgfx16.mac` is gone with them, and `newgfx16.o` is
4,238 bytes of .text, from 48,542. What is left in the file is
`StartDrawNewGfx16b` and nineteen entry seams.

The priority-1 pass does not repeat the work: pass 0 caches the addresses per
scanline in `ngceax`/`ngcedi`/`ngptrdat`/`bgtxad` and pass 1 reads them back,
which is why its macro is a third the size. Two details there - `bgtxadd` is
restored by a *word* store, so its upper half is left over from the earlier
pass, and the 16x16 vertical-flip test is computed from `BG1SYl + ebx` rather
than from the caller's ecx.

The register set they hand over is *exactly* the entry contract
`test/ng2_harness.h` documents for the newg162 side, so once both halves are C
the call is a plain one - and the `push ebx` here and the `pop ebx` in the
renderer, which are two halves of one pairing spanning two files, disappear
together. What does not come for free is the mosaic tail: the renderer used to
reach it by tail-jumping out of the newg162 seam, and a C dispatcher does not
go through that seam, so it has to take the tail itself. `c_domosaicng16b`
reads none of the registers, only the mosaic state, so nothing has to be handed
over - but the check has to be there.

Three things the difftest had to get right before it meant anything:

- the oracle is the *pre-port* assembly, where `ProcessBuildWindow` still
  reached `BuildWindow` through the register ABI. The port calls the cdecl C
  function it became. Two recorders, and `--define BuildWindow=BuildWindow_reg`
  to tell them apart.
- `mov edx,[colormodedef+edx*4+%1]` is a dword read of a byte table, and the
  `mov dl,1<<n` that follows replaces the low byte of *that*, not of the
  incoming edx. So the renderer is entered with three neighbouring layers'
  depths in the upper bytes. It reads three bytes past the table on the last
  mode of the last layer, too.
- `t16x161` selects the 16x16 path on a value of exactly 1, so filling it with
  random bytes reached that path 68 times in 40000. Fill it with 0 or 1.
- pass 1 *reads* the four cached tables, so they have to be filled per
  iteration and must not be cleared between the two runs the way the outputs
  are.

The `mov eax,[BGPT1+ebx*2]` loads are dword reads of word tables as well, but
there the high half washes out in a later `and eax,0FFFFh` - mutating it to a
word read leaves the difftest passing, which is the right answer rather than a
gap. Only `bgtxadd` keeps its high half.

The line macro is the tile one with three more branches stacked on the front -
hi-res (`BGMA >= 5`), offset-per-tile (`BGMA` 2 or 4, unless `osm2dis` or
`bgmode` 4 rules it out) and 16x16 tiles - so it reaches fourteen renderers
rather than six. Two things it does that the tile version does not: mosaic
replaces the *output pointer*, drawing the line into `xtravbuf+32` so `esi`
changes under the later `add edi,esi`; and direct colour goes into
`CPalPtrng` rather than `ebp`, because a line drawer looks the palette up per
pixel. Watch the order too - the 8x8 offset-mode arm clears `mode0add` *after*
`ProcessBuildWindow`, where every other arm clears it before.

The 16x16 offset-mode walk is not the 8x8 one widened: the carry is `200h` not
`100h`, the row mask `1FFh` disappears, the column shift is 4 not 3, and
`taddnfy16x16`/`taddfy16x16` flip on bit 3 of the scroll. That difference was
invisible until `vrama` and the `ofsm*` seeds stopped being zero.

The priority-1 line pass reads the cache back the way the priority-1 tile pass
does, but it is not the pass-0 one with the arithmetic removed, and three of
the differences are easy to "fix" by accident:

- offset-per-tile is taken on `BGMA` 2 only. The test for 4 is commented out in
  the assembly, so mode 4 draws through the plain path on this pass and through
  the offset one on pass 0.
- the 8x8 offset form masks the map cursor with `0FFC0h` - sixteen bits - where
  pass 0 and the 16x16 form use `0FFFFFFC0h`, so a carry out of the low word is
  dropped on one and kept on the other.
- the row is recomputed from `BG1SYl` rather than taken from the caller's ecx,
  and on an interlaced mosaic line the same subtract-and-add lands `mosstart`
  in it *twice*, once on each side of the field offset.

It also writes `bgtxadd` and `ngptrdat2` before the depth test, so a layer that
draws nothing still leaves those two behind.

### Inside newgfx16: newengine16b (done)

The per-scanline driver. Two thirds of it - the line-state tables, the windows
and the sprite window - became `video/c_ngline.c` in an earlier pass and the
assembly reached them by `ccallv`; the tail is there now too, so the routine is
one C function and `video/newgfx16.asm` has no entry point for it. Its one
caller in `video/c_makevid.c` was an inline-asm `call` with every register
declared clobbered - the assembly ended `xor ebx,ebx / ret` - and is a plain
call now.

The tail is small but four of its details are worth pointing at:

- the colour-add cache key is a *dword read starting one byte before*
  `coladdr`, whose low byte is then overwritten with the brightness. The stray
  byte is `hdmatype` and never survives, so the key is exactly the three
  components and the brightness;
- each component is scaled by a byte `mul` and a byte `div` - `c*bright/15` -
  and shifted by `ngrposng`/`nggposng`/`ngbposng` with a *sixteen-bit* `shl`,
  so bits pushed past bit 15 are gone, not carried;
- `ColResult` is filled by two word stores, not one dword, which is only
  visible if you assume the two halves must agree - they always do here, but
  reproduce the stores;
- the back area is worked out twice, once for the main screen and once for the
  sub, and the two read `scaddset` at *different* bit offsets: the main screen
  shifts it right two first. The four-way `inside`/`outside`/`filled` choice is
  otherwise the same both times.

Its difftest (`ngeng`) does not use the pre-port revision. `mkoracle` picks the
newest revision whose file carries no `call c_`, which here would be one where
the whole routine was still assembly; instead `--requires 'ccallv
newengine16b_lines' --ported-marker ''` picks HEAD, where the head halves are
already C. Both sides then *share* those three functions and the comparison
isolates the tail. That is the general move for a routine being ported in
stages: the oracle you want is the last revision where the part under test was
still assembly, not the last where any of it was.

Two harness details it needed. The oracle has to be called through inline asm
that declares ebx clobbered, because the assembly's `xor ebx,ebx` is a return
value in a routine that C will call as `void f(void)` - called plainly it eats
whatever gcc parked there, and the crash lands in the next `printf`. And the
video buffer is 580KB, far too big to snapshot per iteration, so it is refilled
from a position-dependent template - every offset a different byte - and
compared by checksum; a copy from or to the wrong offset still shows up.

### Inside newgfx16: StartDrawNewGfx16b (done)

The frame driver, and the last routine body in the tree. Per layer and per
priority pass it decides whether to draw and calls `c_procbg16b`,
`c_procspr16b` or `c_procmode7ng16b` - all C already - so what had to be
carried across was the running order and its gates, plus the screen clip it
ends with. `video/c_ngframe.c`.

The order is not symmetric between the two screens and it is easy to
"correct" by accident:

- the sub screen tests one bit of `bgcmsung` per layer (`800h`, `400h`, ...),
  the main screen two (`808h`, `404h`, ...) - except mode 7's priority-0 pass,
  which tests `300h` on the sub screen and `101h` on the main one;
- the main screen's *second* EXTBG pass comes before BG2's priority 1; the sub
  screen's comes after;
- `mov eax,[bgNtotng]` happens before the compare that may skip the pass, so
  the count is left in eax whether or not the layer draws - and eax is one of
  the two registers the colour-maths pass reads.

`MainScreenClip` falls straight through into `SubScreenClip`, so `call
MainScreenClip` runs both and nothing ever calls the second on its own. Its
run walk has four entry points sharing one tail, a zero-length run decrements
the *next* entry in place, and `sub edx,ecx` leaves edx decremented even on the
path that gives up - transcribe it with its labels rather than restructuring.

Two register facts make this the routine that had to go last. It runs on the
live block, because `c_transp_halfsub` reads the caller's eax and
`c_transp_halfadd` its edx, upper halves included; and those upper halves come
from the two setup loops at the top (`edi` ends at `sprtlng+256`, `ecx` at 0)
and from whichever `bgNtotng` was read last. Nothing *means* anything by them -
they are leftovers - but the port has to leave the same leftovers, so the C
carries a register block and the seam is `pushad` / `ccall` / `popad`.

### Comparing two builds on a ROM: index by emulated frame, not by line

`test/harness/zab.sh` writes `zsnes_ppu.txt` and `zsnes_hashes.txt` once per
*displayed* frame, so line N of one run and line N of another are the same
point only if both ran at the same speed. A port never does - it is faster or
slower than the assembly it replaced - so the two drift and the hash stream
reports a difference that is not one. It showed up as an intermittent
`DIFFER` on Zelda and Super Mario RPG, at a *different* frame each time, while
the baseline compared against itself was clean every run. A real rendering
fault lands on the same frame every time; a moving one is the clock.

The PNGs are the sound instrument: `PNG_SCREENSHOT_EVERY_N` counts emulated
frames and names each file for the frame that produced it, so matching names
really are the same point in the run. `zab.sh` captures them and compares them
by name now, and reports that first; the two streams are a smoke test behind
it. On the run that provoked this, the frame comparison said 118 of 118
identical while the hash stream said "2 frames only in base, 26 only in cand" -
the candidate had simply got further in the same wall-clock second.

Also worth knowing: a pre-port revision may not build. `93aca790` fails on
today's nasm (`chips/7110proc.asm:166: size wasn't specified`), so the oldest
usable A/B baseline is somewhere later. Patching an old revision to build would
cost it its value as a baseline.

## The video assembly is a tail-dispatch layer

`video/makev16t.asm` is not a pile of independent routines to pick off one at a
time: 21 of its 26 remaining exports end in a tail-jump. The shape is always the
same - spill the registers into a per-cluster seam (`M7T*`, `SPR*`, `TH*`), call
a C body that decides *which* renderer to run and leaves the answer in a tail
variable (`M7TTail`, `SPRTail`, `THTail`), reload the registers, and **jump**
into that renderer so its `ret` returns to the gate's own caller.

Two consequences.

The registers are the argument-passing. A gate hands the renderer whatever it
reloaded, and the renderers are cdecl trampolines that take some of it:

| renderer | takes |
| --- | --- |
| `drawmode716t`, `drawmode716b`, `drawmode716tb` | `eax`, `edx` |
| `drawmode716extbg`, `drawmode716textbg` | `eax`, `edx` |
| `drawmode716extbg2`, `drawmode716textbg2` | `ecx` only |

That table is the proof the rest of the register set is dead at the jump, and
so the licence for a C gate to call `c_drawmode716t(M7TAX, M7TDX)` and drop the
others. Derive it again rather than trusting this - the trampolines are in
`video/mode716b.c`, and preprocessing the file prints them:

    gcc -E -m32 -I. $(CFGDEFS) video/mode716b.c | tr -d '\n' | sed 's/__asm__(/\n@@/g'

And the difftest has to move with it. `test/difftest_m716t.c` intercepts the
*renderer symbols* and compares all seven registers at the moment of the jump,
which is exactly the contract a C gate stops honouring: it calls the renderer's
C body with arguments instead. So the central check becomes "which renderer,
with which arguments", and that rework only pays for itself once, for the whole
layer. Port the gates as one batch, not one at a time.

The file is also a closed knot: of the exports left, 13 are entry points
reached only from C, and the other 13 are the tail targets those jump to,
reached only from inside the file. Neither half can go first - a gate cannot
become C while its target is an assembly label expecting live registers, and a
target cannot be removed while an assembly gate jumps to it. Recompute with:

    python3 - <<'EOF'
    import re; t=open('video/makev16t.asm').read()
    e=re.findall(r'NEWSYM (\w+)',t)
    r={m.group(1) for m in re.finditer(r'\b(?:call|jmp|je|jne)\s+(?:near\s+)?(\w+)',t)}
    print([x for x in e if x not in r])   # the entry points
    EOF

Gates first, targets later, and **the gate hands the tail id back rather than
dispatching**. That is what unties it: the caller already reaches the renderers
through `calldl16t`, so it just does that once more with the id, and the ported
gate needs no register bridge of its own - it is plain C, and `portcheck` goes
*up*. `video/c_m716gate.c` is the worked example: all sixteen gates went that way -
eight mode 7, four sprite, four background - and `makev16t.asm` lost its tail
dispatcher with them, 26 exports down to 10. `difftest_m716t.c` no longer needs
a `cur_` object at all, because there is nothing left on the port's side that
is assembly.

The background four needed one extension. They *call* their renderer rather
than jumping to it and then do more work, so a single id is not enough: they
split into a first half returning the id and a `drawbackgrnd_mark` second half,
with the caller running renderer-then-mark between them. Still no bridge.

With the gates gone the file stops dispatching, and what is left are the
renderers they used to jump to - which can then go one at a time, because every
caller is C. The three `drawsprites*` ones went that way (`SPRITE_DRAW` in
`video/c_m716gate.c`), and `draw16x1616t`/`draw16x1616bt` after them
(`TILE_DRAW`), taking `makev16t.asm` from ten exports to five.

The last three were blocked by a two-file cycle: `video/mv16tms.asm`'s
`draw8x816tms` tail-jumped to `makev16t.asm`'s `draw16x816t`, and nothing else
called it - so neither could go first. Breaking it meant converting
`draw8x816tms` and `draw16x816t` together, plus the three more `mv16tms`
routines `draw8x816tms` tail-jumps to. That went in one batch, and with the
16x16 family after it `makev16t.asm` is down to `calldl16t` and `mv16tms.asm`
is gone.

### A tail-jump is the caller's to take

When a routine ends in a jump rather than a call, hand the decision back and let
the caller jump - not inside the port, even when the target is already C.
`domosaic16b` is C and reads `curmosaicsz` from a global, so calling it from the
renderer looks harmless, and the emulator does not care. The difftest does: its
stub records *the register state at the jump*, which a C call cannot reproduce,
and every iteration reaching the mosaic tail fails. Return the flag and let the
caller run it through `dt_call`. Same shape as the gates' tail id, and the same
lesson as `SPRTail`: a port-era convenience is not automatically visible to an
assembly oracle.

Watch for saves that stop being necessary. The assembly pushed ecx and edx
around that bookkeeping call because cdecl would clobber the *registers*; once
the values live in a struct nothing touches them and the save is dead code that
implies a hazard which no longer exists. The difftest will tell you - mutate the
line out and see if anything notices.

One trap when a gate's tail variable is *new*. `SPRTail` exists only since the
sprite bodies were ported: the pre-port assembly picks its renderer inline and
jumps, so the oracle never writes it. Comparing it in the difftest compares a
variable one side does not have, and fails on stale state. Check the oracle
revision has the symbol before adding it to a snapshot:

    git show <rev>:video/makev16t.asm | grep -c SPRTail

The difftest survives unchanged in what it checks, which is the point of doing
it in this order. `difftest_m716t.c` still intercepts the renderer symbols and
still compares all seven registers at the jump, because the driver loads them
and calls the renderer itself (`run_port`). Two alternatives were worse:

- porting gates *and* targets together drops the register comparison to "which
  renderer, with which arguments" - weaker, and only defensible via the
  trampoline table above showing the other five registers are dead;
- giving the gate its own inline-asm bridge to the renderer costs a `portcheck`
  point and puts back the thing the port exists to remove.

Ported out of the file so far, both against oracles that survive the deletion:
`procwindowback16t` (`video/c_procwin.c`, its own difftest) and the three
`clearback*` trampolines (call sites in `video/c_mv16tline.c` spill into the
`CB*`/`CLB*` seams now). Both needed the same thing first - see below.

### Deleting an assembly routine breaks its difftest

Four ways, and they all bite silently:

- a target that builds a `cur_` object from the working tree and compares the
  pre-port oracle against it (`cb16t`, `cb16b` did) loses the symbol. Fix: drive
  the port's side through its seam directly, and drop the `cur_` object.
- the *oracle* itself, if the target uses `--worktree`. Move it to a git
  revision with `--requires` naming the label, and `--ported-marker ''` to turn
  the default marker test off when the routine is newer than the port.
- the oracle's *revision*, once the last `call c_` leaves the file. `mkoracle`
  picks the newest revision without that marker, which is then the finished
  port, and it says so rather than handing one back. Fix: `--requires` a label
  the port removed for good - `test/Makefile`'s `M16T_ORIG` pins every
  `makev16t.asm` oracle with `NEWSYM draw16x816t`. Applies to every target
  reading that file, not just the one being worked on, so run them all.
- a *register-ABI stub inside the oracle*, once the port's caller becomes plain
  C. `difftest_m7calc.c` reached `drawmode7win16b` through an inline-asm
  trampoline that saved ebx/esi/edi/ebp; when the trampoline went, the same
  stub was called cdecl, clobbered all four, and the harness segfaulted. Fix:
  `%define` the oracle's copy to another name (nasm renames the call inside the
  extracted body too) and write the C side's stub in C.

Check with `grep -l cur_<name> test/*.c` before removing anything.

## Known divergences from the assembly

Recorded in the difftests themselves via `KNOWN_DIVERGENCE`, so they are
reported rather than silently skipped, and so a *new* divergence still fails.

- **CLI.** The assembly's emulation-mode restart is `xor ebx,ebx; jmp execloop`;
  the port returns without clearing ebx. The dispatcher only ever loads bl from
  a zero ebx, so the upper bits are always zero in the emulator - the difftest
  seeds them non-zero deliberately, which is why it only shows there.
- **SA-1 `DEC d,x`.** The assembly does an 8-bit decrement through 16-bit
  accesses; bsnes and snes9x both do a single byte, so the port is deliberately
  right rather than bit-identical.

## When the assembly's behaviour is the host CPU's, not the SNES's

Reproduce it. ZSNES was written for 32-bit x86 and never ran on anything else,
so an instruction whose result the ISA calls "undefined" was still, in practice,
one fixed value everywhere the emulator ever executed. Treat that as part of the
behaviour being ported, not as licence to pick something else.

The worked example is the overflow flag after decimal ADC/SBC. The assembly ends
`daa` / `seto byte[flago]`, and Intel documents OF after `daa`/`das` as
undefined - so the port had been writing V = 0, which matched neither the
assembly nor hardware. Measuring it settled the question: OF is the signed
overflow of the one *combined* adjustment (0, 6, 60h or 66h) applied to the
entering AL, clear when there is no adjustment, and independent of the incoming
OF. Exhaustive over all 1024 (AL, CF, AF) states for both instructions.
`decimal_of()` in `cpu/ops65816.h` is that rule, in portable C; it took 60
opcodes from "known divergence" to bit-identical.

### Widening a register slot

`zreg` is what makes the block portable, and the work is not the typedef - it is
what the typedef then exposes. On i386 `zreg` and `u4` are the same type, so
every latent width bug is invisible until `make portcheck` compiles for x86-64:
a helper taking `u4* pedx` that is handed `&r[R_EDX]`, a local `u4 edx` passed
by address, a `(u4)(uintptr_t)ptr` on the way into a slot. Work down the
portcheck errors, and be careful in the other direction too - `execute()` takes
a real 32-bit edx by pointer and must stay `u4*`, not follow the rename.

Nothing in the shipped build changes, which is the point: run the three opcode
difftests after and they must still be bit-identical.

Measure before you ask. A model fitted to hardware and checked over the whole
input space turns a judgement call into a fact, and the difftest then proves it:

    gcc -m32 ... /* drive the instruction, popf the inputs, pushf the outputs */

## Style

Brief one-line ASCII comments (no unicode/em-dashes; commas not ` -- `). Return
boolean conditions directly. Match surrounding code. Comment only what needs it.
