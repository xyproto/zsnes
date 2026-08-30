# TODO

- [ ] Verify Dirt Trax FX on the Europe ROM and close #32; the freeze and the
      glitchy top band are fixed on the USA ROM
- [ ] Fix Super Mario RPG getting stuck on a garbled screen in the attract
      demo, reachable from boot with no input at all and older than the DMA
      timing work. The SA-1 is idle in its normal `LDA $00`/`BEQ` handshake
      loop and the 65816 is still frame-syncing on `BIT $4212`, so the state
      machine that feeds the SA-1 is what stops. Likely the same fault as #28
- [ ] Check that a plain `make` works on Ubuntu 20 and close #19; the report is
      a 32-bit build linking against 64-bit libraries
- [ ] Use `SDL_Gamepad` so controllers get SDL's mapping database instead of
      raw numbered axes and buttons
- [ ] Check the `SDL_Renderer` path's hi-res, mode 7, filter and scanline
      composition against the OpenGL one; only the lo-res path has been read
      back and compared so far (`ZSNES_LEGACY_GL=1` selects the old path)
- [ ] Relabel or retire the `SoundQuality` rate setting: SDL, PipeWire and
      libao now always render at the DSP's 32kHz and resample, so the rates it
      offers no longer do anything there
- [ ] Save `SetaCmdEnable` in the save state (`zstate.c`)
- [ ] Handle horizontal scroll and absolute mouse motion on macOS (`mmlib/macos.c`)
- [ ] Fill in the GUI font glyphs 0x30-0x36 (`video/procvid.c`)
- [ ] Make transparent messages work with the small font (`cfg.psr`)
- [ ] Build Windows on SDL3 instead of the legacy DirectDraw/DirectInput
      backend in `win/`, so all three platforms share one path
- [ ] Add the missing ST-011 opcodes (`chips/seta11.c`)
- [ ] Work out the ST-011 OPS04/05 outputs and OPS06/07 purposes (`chips/seta11.c`)
- [ ] Verify the unverified DSP4 lookup table (`chips/dsp4emu.c`)
- [ ] Retire the old graphics engine (`newengen=0`) once the new one has no
      known regressions, and drop `bgfixer` with it
- [ ] Build with `-flto -Werror=lto-type-mismatch -Werror=odr
      -Werror=strict-aliasing` and fix what it reports (#59); the assembly it
      was waiting on is gone, and cross-unit type mismatches are still there
- [ ] Re-enable the FreeBSD, OpenBSD and NetBSD CI jobs
- [ ] Give the difftests a 64-bit oracle so they run off 32-bit x86
- [ ] Give the pinned table runs (`BG1SYl`, `BGPT1` ...) flat aliases so the
      cross-table indexing stops tripping UBSan
- [ ] Read the word and dword seams (`cpu/ops65816.h`, `video/c_ng2tile.c`)
      through `memcpy` rather than casts; the unaligned loads the assembly took
      for granted are UB and would fault on a strict-alignment target
- [ ] Build `test/`'s parent objects through the top Makefile, not make's implicit rule
- [ ] Test `unix/battery.c` on a machine that reports battery state
- [ ] Move the end-of-ROM variables out of the ROM buffer so `maxromspace` no
      longer has to be 16MB for 8MB carts (#17)
- [ ] Re-test PAL/NTSC detection across the ROM set (#24)
- [ ] Verify the ZSNES Flatpak on Flathub (#10)
- [ ] Drop the unfinished-dynarec remnants around `curexecstate` (`initc.c`)
- [ ] Port the rest of `doc/readme.txt/*.txt` into `man/zsnes.1`
- [ ] Add exclusive fullscreen and a display picker (#20), so a CRT can be
      driven at its own resolution and on a secondary monitor
- [ ] Attach win32/win64 (and macOS) binaries to GitHub releases
- [ ] Improve the netplay code, and bring back what 1.42n had (#2)
- [ ] Port netplay to the Windows build (`gui/c_guiwindp.c`)
- [ ] Write a Homebrew formula/tap
