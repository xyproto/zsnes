# TODO

- [ ] Give the difftests a 64-bit oracle so they run off 32-bit x86
- [ ] Finish the `SDL_Renderer` composition check. `ZSNES_FRAME_DUMP=N` now
      saves the composited window every N frames from whichever path drew it
      (`ZSNES_LEGACY_GL=1` selects the old one), and against the emulator's own
      buffer the SDL path measured closer than the GL one on mode 7 (RMSE 0.157
      against 0.256). Three things are still unchecked: real 512-wide content,
      because no ROM here reaches BG mode 5 or 6 without input, so only the
      hi-res mode 7 spans have been exercised; the fullscreen geometry, which
      Xvfb will not enter; and the old engine, where the line kinds come from
      `hirestiledat` rather than `SpecialLine`
- [ ] Legacy GL runs `hq2x_16b()` whatever `hqFilterlevel` says, so hq3x and
      hq4x silently come out as hq2x on that path. It is on the way out, so
      this is only worth doing if it outlives the SDL_Renderer path
- [ ] Fix Super Mario RPG hanging in the attract demo, from boot with no input.
      Re-measured after the #28 `xor dh,dh` fix and it is no longer a garble at
      ~12700: the demo now plays normally to frame 19955, fading out cleanly
      over frames 19949-19955 (bright 2 -> 1, screen on, scrnon=0013), and at
      19956 the game sets force-blank as it would to load the next scene. It
      never comes back: 28752 frames and counting of bright=1 blank=80
      scrnon=0000. So it hangs at the start of a scene load behind a blank
      screen, which is the same shape as #28's "black screen entering a
      battle" and is reachable without input. The SA-1 is idle in its usual
      `LDA $00`/`BEQ` handshake and the 65816 is still frame-syncing on
      `BIT $4212`, so what stops is the state machine feeding the SA-1. Local
      instrumentation is exhausted; the next step is a reference trace
- [ ] Confirm the DSP4 division table against hardware. All 64 entries were
      checked to be floor(0x8000/n) exactly, so there is no transcription
      error, and the twelve call sites match snes9x line for line - but snes9x
      carries the same "not verified" note, so the shape of the table is
      agreed and only the chip's own rounding is unconfirmed. Needs a DSP4
      cart (Top Gear 3000), which is not in the ROM set here
- [ ] Add the missing ST-011 opcodes (`chips/seta11.c`). Blocked on evidence,
      not effort: the implementation was reconstructed from one binary log of
      st011-demo, which never issues them, and there is nothing to copy from -
      snes9x's seta011.cpp is a 138-line stub that zeroes its outputs, and
      bsnes only detects the chip. Ours already goes further than both. An
      unknown command no longer leaves the chip busy for ever, and now names
      itself on stderr once, so a report from anyone playing Hayazashi Nidan
      Morita Shougi 2 past the point these are issued is what unblocks it
- [ ] Work out the ST-011 OPS04/05 outputs and OPS06/07 purposes (`chips/seta11.c`)
- [ ] Size the ROM buffer to the cart (#17). The coupling that stopped this is
      gone: the SuperFX and Seta work RAM used to sit inside the ROM buffer at
      the 14MB mark, which is why it had to be 16MB and why `preparesfx` took
      its bound from `sfxramdata - romdata`. Both are their own allocations
      now and the buffer is `maxromspace` plus one spare bank, so the 16MB
      block is 12.06MB + 2MB + 64K. What is left is allocating it per cart
      rather than at startup, which needs care where a reload changes the base
- [ ] Retire the old graphics engine (`newengen=0`) once the new one has no
      known regressions, and drop `bgfixer` with it
- [ ] Test the HDR path on a display that actually has HDR. The route exists:
      probed on a Wayland session with tools/hdrprobe.c, the vulkan and gpu
      renderers both accept SDL_COLORSPACE_SRGB_LINEAR and a float texture,
      while opengl, opengles2 and software refuse it - so SDL's documentation,
      which lists only direct3d11/12 and metal, is out of date, and
      sdl_render.c now asks for vulkan first partly for that reason. What is
      unproven is the path itself: no display to hand reports HDR_enabled, so
      `sr_to_hdr` has never run. Check that the spill goes above white rather
      than clipping, that the headroom is respected, and that
      `SDL_EVENT_WINDOW_HDR_STATE_CHANGED` is picked up mid-run
- [ ] Move the remaining GUI windows onto GUIStackLayout. Video, Sound, Options
      and Paths now describe their rows once and let the drawing and the click
      handling read the same positions. Still writing their pixel numbers out
      twice in two files, largest first: Combo, Optns, Cheat search, Movies,
      Input, Speed, Cheat, Confirm, AddOns, Chose save, Chip, About, Reset,
      States. Save is half done - its checkbox column and state shortcut grid
      read from `GUISaveRow`/`GUISaveSlotX`/`GUISaveSlotY`, but the four boxes
      along its bottom row are still written out twice. That duplication is how the grayscale, hi-res mode 7,
      vsync and 4:3 controls came to be drawn in one place and clicked in
      another
- [ ] Use `SDL_Gamepad` so controllers get SDL's mapping database instead of
      raw numbered axes and buttons. The constraint to design around:
      `InitJoystickInput` lays every device's axes, buttons and hats into one
      flat index space from 256 up (`AxisOffset`/`ButtonOffset`/`HatOffset`),
      and a user's saved bindings are indices into it, so switching wholesale
      would silently repoint everyone's controls. The additive route is to
      open recognised pads with `SDL_OpenGamepad` alongside the joystick, read
      `SDL_GetGamepadBindings` to learn which raw index each canonical button
      sits at, and use that to fill in defaults where a player has none -
      leaving existing bindings alone
- [ ] Handle horizontal scroll and absolute mouse motion on macOS (`mmlib/macos.c`)
- [ ] Re-enable the FreeBSD, OpenBSD and NetBSD CI jobs
- [ ] Improve the netplay code, and bring back what 1.42n had (#2), possibly with a dedicated server.
- [ ] Port netplay to the Windows build (`gui/c_guiwindp.c`)
- [ ] Quieten `difftest_ng2.c`: it includes `difftest.h` but drives its own
      loop, so `dt_bad`, `dt_fails`, `dt_iters` and its own `setup()` are all
      compiled unused. Either use `DT_MAIN` there or stop including the header
- [ ] Test `unix/battery.c` on a machine that reports battery state
- [ ] Give `man/zsnes.1` the sections it still lacks. The option list is now
      reconciled against what the build actually accepts - `-?` and `-o` were
      documented but rejected by the parser, `-ad`, `-ds`, `-mo` and `-v8` were
      offered but undocumented, and `-r` is accepted and ignored on Unix - so
      what is left is the prose: the docs moved to `doc/*.html` and the old
      `doc/readme.txt/` the TODO used to name is gone, so controls, the GUI and
      netplay would have to come from the HTML
