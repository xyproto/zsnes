# TODO

- [ ] Give the difftests a 64-bit oracle so they run off 32-bit x86
- [ ] Check the `SDL_Renderer` path's hi-res, mode 7, filter and scanline
      composition against the OpenGL one; only the lo-res path has been read
      back and compared so far (`ZSNES_LEGACY_GL=1` selects the old path). The
      vertical offset is now settled: every consumer reads source line N+1 for
      output row N, since scanline 0 is the pre-render line
- [ ] Fix Super Mario RPG getting stuck on a garbled screen in the attract
      demo, reachable from boot with no input at all and older than the DMA
      timing work. The SA-1 is idle in its normal `LDA $00`/`BEQ` handshake
      loop and the 65816 is still frame-syncing on `BIT $4212`, so the state
      machine that feeds the SA-1 is what stops. Likely the same fault as #28
- [ ] Verify the unverified DSP4 lookup table (`chips/dsp4emu.c`)
- [ ] Add the missing ST-011 opcodes (`chips/seta11.c`)
- [ ] Work out the ST-011 OPS04/05 outputs and OPS06/07 purposes (`chips/seta11.c`)
- [ ] Move the end-of-ROM variables out of the ROM buffer so `maxromspace` no
      longer has to be 16MB for 8MB carts (#17)
- [ ] Retire the old graphics engine (`newengen=0`) once the new one has no
      known regressions, and drop `bgfixer` with it
- [ ] Drop the unfinished-dynarec remnants around `curexecstate` (`initc.c`)
- [ ] Filter hi-res frames through the NTSC filter: `ntsc_blit` (`video/ntsc.c`)
      reads 256 input pixels a row, so a 512-wide hi-res line is only half
      filtered. snes9x carries a separate `snes_ntsc_blit_hires` for this
- [ ] Support HDR output, with bloom as the thing that makes it worth having.
      SDL3 reports whether a display is in HDR mode
      (`SDL_PROP_DISPLAY_HDR_ENABLED_BOOLEAN`) and can present a wider surface,
      but on 15-bit source HDR alone buys almost nothing: the same 32768
      colours in a bigger container, and stretching them makes banding worse,
      not better. What it does buy is headroom above white, and bloom is the
      one pass here that produces values needing it - every other filter is
      bounded by its input, while spilling a bright region's light outward and
      leaving the source bright genuinely exceeds 1.0. Clipping that is what
      makes SDR bloom look washed out, the same way the old brightness gain
      did.
      So both belong in the same place: the post-composition pass in
      `sr_drawwin` that already does scanlines and vibrancy, after the filters
      and at the point of widening. Bloom cannot join the lookup table there,
      being spatial rather than per-pixel, but it composes with it. Measured on
      a 768x672 hq3x frame, a separable blur at full resolution costs 7.7ms a
      frame, which is too much; at quarter resolution it is 2.3ms, about what
      hq3x itself costs, and bloom is low-frequency so quarter resolution loses
      nothing. Most of that is the full-resolution add-back, not the blur, so
      the tap count is nearly free.
      Note the luma-dependent beam width in `sr_build_luts` is already a cheap
      stand-in for one part of this: bloom filling the scanline gaps on bright
      content. Real bloom would do it rather than approximate it
- [ ] Make transparent messages work with the small font (`cfg.psr`)
- [ ] Use `SDL_Gamepad` so controllers get SDL's mapping database instead of
      raw numbered axes and buttons
- [ ] Handle horizontal scroll and absolute mouse motion on macOS (`mmlib/macos.c`)
- [ ] Re-enable the FreeBSD, OpenBSD and NetBSD CI jobs
- [ ] Drop `unix/sockserv.c` and `unix/sockserv.h`, or give them content: both
      hold only the licence header, yet `sockserv.c` is still listed in `SRCS`
      and compiled as an empty translation unit
- [ ] Improve the netplay code, and bring back what 1.42n had (#2), possibly with a dedicated server.
- [ ] Port netplay to the Windows build (`gui/c_guiwindp.c`)
- [ ] Fill in `outsa1()` (`debugger.c`), a stub since the port
- [ ] Quieten `difftest_ng2.c`: it includes `difftest.h` but drives its own
      loop, so `dt_bad`, `dt_fails`, `dt_iters` and its own `setup()` are all
      compiled unused. Either use `DT_MAIN` there or stop including the header
- [ ] Test `unix/battery.c` on a machine that reports battery state
- [ ] Port the rest of `doc/readme.txt/*.txt` into `man/zsnes.1`
