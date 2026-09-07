# TODO

- [ ] Relabel or retire the `SoundQuality` rate setting: SDL, PipeWire and
      libao now always render at the DSP's 32kHz and resample, so the rates it
      offers no longer do anything there
- [ ] Fix Super Mario RPG getting stuck on a garbled screen in the attract
      demo, reachable from boot with no input at all and older than the DMA
      timing work. The SA-1 is idle in its normal `LDA $00`/`BEQ` handshake
      loop and the 65816 is still frame-syncing on `BIT $4212`, so the state
      machine that feeds the SA-1 is what stops. Likely the same fault as #28
- [ ] Use `SDL_Gamepad` so controllers get SDL's mapping database instead of
      raw numbered axes and buttons
- [ ] Check the `SDL_Renderer` path's hi-res, mode 7, filter and scanline
      composition against the OpenGL one; only the lo-res path has been read
      back and compared so far (`ZSNES_LEGACY_GL=1` selects the old path)
- [ ] Port the real hq4x, and the `_32b` twins of all three; those entry
      points in `video/c_hqx.c` are still nearest-neighbour block doublers
- [ ] Let hq3x run on the `SDL_Renderer` path: `SR_MAXW`/`SR_MAXH`
      (`unix/sdl_render.c`) cap the surface at 640x512, too small for its
      768x672 output, so only the software path reaches it
- [ ] Fill in `outsa1()` (`debugger.c`), a stub since the port
- [ ] Handle horizontal scroll and absolute mouse motion on macOS (`mmlib/macos.c`)
- [ ] Fill in the GUI font glyphs 0x30-0x36 (`video/procvid.c`)
- [ ] Make transparent messages work with the small font (`cfg.psr`)
- [ ] Add the missing ST-011 opcodes (`chips/seta11.c`)
- [ ] Work out the ST-011 OPS04/05 outputs and OPS06/07 purposes (`chips/seta11.c`)
- [ ] Verify the unverified DSP4 lookup table (`chips/dsp4emu.c`)
- [ ] Retire the old graphics engine (`newengen=0`) once the new one has no
      known regressions, and drop `bgfixer` with it
- [ ] Give the difftests a 64-bit oracle so they run off 32-bit x86
- [ ] Test `unix/battery.c` on a machine that reports battery state
- [ ] Move the end-of-ROM variables out of the ROM buffer so `maxromspace` no
      longer has to be 16MB for 8MB carts (#17)
- [ ] Drop the unfinished-dynarec remnants around `curexecstate` (`initc.c`)
- [ ] Port the rest of `doc/readme.txt/*.txt` into `man/zsnes.1`
- [ ] Improve the netplay code, and bring back what 1.42n had (#2), possibly with a dedicated server.
- [ ] Port netplay to the Windows build (`gui/c_guiwindp.c`)
- [ ] Re-enable the FreeBSD, OpenBSD and NetBSD CI jobs
- [ ] Make `zstate.h` self-contained: it uses `u4` without including `types.h`,
      so it only compiles when a caller includes that first, and it declares
      `statesaver()` and `SaveSramData()` twice
- [ ] Give `gblvars.h` an include guard; it is only safe to include twice today
      because it holds nothing but `extern` declarations
- [ ] Check the allocations in `zmovie.c`: the chapter-buffer `malloc` (line
      ~413), both `zmv_vars.filename` allocations and the rewind buffer are
      dereferenced without a NULL test, while the author buffer nearby is
      tested — make them consistent
- [ ] Drop `unix/sockserv.c` and `unix/sockserv.h`, or give them content: both
      hold only the licence header, yet `sockserv.c` is still listed in `SRCS`
      and compiled as an empty translation unit
- [ ] Check the `glvidbuffer` allocation in `unix/gl_draw.c` and
      `win/gl_draw.c`; both call `gl_clearwin()` immediately afterwards, which
      memsets it, so a failed `malloc` is a null dereference. Every other
      allocation in `unix/` and `win/` is tested
- [ ] Reject a non-positive `DT_ITER` in `test/difftest.h`: `atol()` turns `0`
      or a typo into zero iterations, so `DT_MAIN` runs nothing and `DT_DONE`
      prints `PASS (0 iterations bit-identical to asm)` and exits 0. A difftest
      that always mismatches passes that way
- [ ] Use or drop `zt_section_fails` in `test/zstest.h`: `ZT_SECTION` assigns it
      and nothing ever reads it, so the per-section pass/fail count it was meant
      to give never appears
- [ ] Clear the pointers in `zstate.c`'s `DeallocRewindBuffer()`,
      `DeallocPauseFrame()` and `DeallocSystemVars()`: each frees its buffer and
      leaves it non-NULL, so the `if (StateBackup) free(...)` in
      `SetupRewindBuffer()` would double-free if a ROM were ever loaded after
      `ZCleanup()`. Only the exit path calls them today
- [ ] Honour `TMPDIR` in `zst_roundtrip_check()` (`zstate.c`): the round-trip
      harness writes fixed `/tmp/zsnes_zst*` paths, which collide between users
      on a shared machine. Debug builds only (`ZSNES_DEBUG_HOOKS`)
