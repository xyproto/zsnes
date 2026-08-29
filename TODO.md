# TODO

## Emulation

- [ ] Save `SetaCmdEnable` in the save state (`zstate.c`)
- [ ] Add the missing ST-011 opcodes (`chips/seta11.c`)
- [ ] Work out the ST-011 OPS04/05 outputs and OPS06/07 purposes (`chips/seta11.c`)
- [ ] Verify the unverified DSP4 lookup table (`chips/dsp4emu.c`)
- [ ] Check the SRAM size when mapping banks 70-7F so 64/128/256 KB all map
      correctly (`initc.c`)
- [ ] Decide whether to fix the SA-1's 8-bit DEC d,x, which reads and writes 16
      bits (`cpu/ops65816_sa1.h`)

## Interface

- [ ] Fill in the GUI font glyphs 0x30-0x36 (`video/procvid.c`)
- [ ] Make transparent messages work with the small font (`cfg.psr`)
- [ ] Handle horizontal scroll and absolute mouse motion on macOS (`mmlib/macos.c`)
- [ ] Improve the netplay code
- [ ] Port netplay to the Windows build (`gui/c_guiwindp.c`)

## Build and test

- [ ] Re-enable the FreeBSD, OpenBSD and NetBSD CI jobs
- [ ] Install `aarch64-linux-gnu-gcc` in CI so `make portcheck` covers aarch64
- [ ] Give the difftests a 64-bit oracle so they run off 32-bit x86
- [ ] Build `test/`'s parent objects through the top Makefile, not make's implicit rule
- [ ] Give the pinned table runs (`BG1SYl`, `BGPT1` ...) flat aliases so the
      cross-table indexing stops tripping UBSan
- [ ] Test `linux/battery.c` on a machine that reports battery state

## Cleanup

- [ ] Decide whether `find_next_match` needs `+ 1` (`argv.h`)
- [ ] Drop the unfinished-dynarec remnants around `curexecstate` (`initc.c`)
- [ ] Use or drop `pNewDeviceAt` (`mmlib/macos.c`)
- [ ] Port the rest of `doc/readme.txt/*.txt` into `man/zsnes.1`
