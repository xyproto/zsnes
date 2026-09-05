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
