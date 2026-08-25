# Modernizing the C code

The Assembly is gone, but a lot of the C still looks like the Assembly it came
from. This is a list of what to change, roughly in the order that pays off
first. None of it is urgent, and all of it can be done one piece at a time.

The lessons from the Assembly port itself are in `portasm.md` in the git
history, if a difftest ever needs explaining.

### Verify every change the same way

    make && make linux64
    cd test && make run
    cd test && make bgt ngframe ng2 mos regs op

...and for anything that touches the renderers:

    env ASCII=1 bash test/harness/zab.sh -r ~/roms/snes/'Pilotwings (USA).sfc' -t 25

Read the "identical at matching emulated frame numbers" line, not the hash
stream. The difftests compare against the original Assembly, pulled from git,
so they keep working even though the `.asm` files are gone.

### Dead plumbing

* `ASM`, `ASMFLAGS`, the `%.o: %.asm` rule and the `REMAINING_ASM` guard in the
  `Makefile` have nothing to assemble. Drop them and `nasm` from the README
  requirements.
* `-no-pie`, `-fno-pic`, `-fno-gcse` and `-fno-inline` were there for the
  Assembly. Try removing them one at a time; each needs a ROM A/B, because
  `-fno-gcse` and `-fno-inline` can change floating point and timing.
* CI builds `make ARCH=linux` only. Add `linux64`, `make portcheck` and the
  difftests.

### The code

* **Register blocks.** `u4 r[8]` with `R_EAX`-style indices, and the `DLR`
  file in `video/c_mv16tline.c`, are the Assembly's calling convention. Replace
  them with named arguments and return values, one routine at a time, starting
  at the leaves.
* **`ASM_GSYM` data.** `endmem.c` and 16 other files lay out globals with
  inline assembly, because the Assembly needed exact symbol order and
  adjacency. Some of that adjacency is still load-bearing - a dword store into
  a byte table writes into the next one - so check each block against the code
  that reads it before turning it into plain C arrays.
* **`-fcommon`.** The build needs it because globals have tentative
  definitions in several files at once. Give each one definition and an
  `extern` in a header, then drop the flag.
* **`extern void foo();`** appears about 400 times in the headers. That is not
  a prototype. Give them `(void)` or real parameters.
* **`u1`/`u2`/`u4`** in `types.h` are `uint8_t`/`uint16_t`/`uint32_t`. Prefer
  the standard names in new code; converting the old is a large diff for
  little gain.

### Not worth doing

* Untangling the `goto`s in the ported renderers. They are transcriptions of
  jump tables, and the shape is what makes them checkable against the original.
* Reformatting beyond `make fmt`.
