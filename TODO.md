# TODO

- [ ] Fix the top 25 scanlines of Dirt Trax FX's title screen flickering between
      the right picture and tiles from another scene; the lines are drawn with
      stale per-line state, not force-blanked, and the eight-line tile path is
      not the cause
- [ ] Handle joystick hot-plug (`SDL_EVENT_JOYSTICK_ADDED`/`REMOVED`); pads
      plugged in after start are invisible until restart
- [ ] Use `SDL_Gamepad` so controllers get SDL's mapping database instead of
      raw numbered axes and buttons
- [ ] Add drift control: nudge `SDL_SetAudioStreamFrequencyRatio` from the
      queued-samples count, the way snes9x's DynamicRateControl does
- [ ] Always emulate the DSP at its native rate and let `SDL_AudioStream`
      resample, rather than retuning the DSP per `SoundQuality` setting
- [ ] Write a Homebrew formula/tap
- [ ] Refresh the AppStream `<releases>` list, replace `<developer_name>` with
      `<developer>`, use an SPDX licence id, and host the screenshot ourselves
- [ ] Replace the legacy OpenGL 1.x path with `SDL_Renderer` and a streaming
      texture, so macOS gets Metal and Windows D3D without an OpenGL dependency
- [ ] Use `glTexSubImage2D` rather than reallocating the texture every frame,
      for as long as the GL path stays
- [ ] Check the SRAM size when mapping banks 70-7F so 64/128/256 KB all map
      correctly (`initc.c`)
- [ ] Save `SetaCmdEnable` in the save state (`zstate.c`)
- [ ] Handle horizontal scroll and absolute mouse motion on macOS (`mmlib/macos.c`)
- [ ] Fill in the GUI font glyphs 0x30-0x36 (`video/procvid.c`)
- [ ] Make transparent messages work with the small font (`cfg.psr`)
- [ ] Attach win32/win64 (and macOS) binaries to GitHub releases
- [ ] Build Windows on SDL3 instead of the legacy DirectDraw/DirectInput
      backend in `win/`, so all three platforms share one path
- [ ] Improve the netplay code
- [ ] Port netplay to the Windows build (`gui/c_guiwindp.c`)
- [ ] Add the missing ST-011 opcodes (`chips/seta11.c`)
- [ ] Work out the ST-011 OPS04/05 outputs and OPS06/07 purposes (`chips/seta11.c`)
- [ ] Verify the unverified DSP4 lookup table (`chips/dsp4emu.c`)
- [ ] Decide whether to fix the SA-1's 8-bit DEC d,x, which reads and writes 16
      bits (`cpu/ops65816_sa1.h`)
- [ ] Retire the old graphics engine (`newengen=0`) once the new one has no
      known regressions, and drop `bgfixer` with it
- [ ] Re-enable the FreeBSD, OpenBSD and NetBSD CI jobs
- [ ] Install `aarch64-linux-gnu-gcc` in CI so `make portcheck` covers aarch64
- [ ] Give the difftests a 64-bit oracle so they run off 32-bit x86
- [ ] Give the pinned table runs (`BG1SYl`, `BGPT1` ...) flat aliases so the
      cross-table indexing stops tripping UBSan
- [ ] Build `test/`'s parent objects through the top Makefile, not make's implicit rule
- [ ] Test `linux/battery.c` on a machine that reports battery state
- [ ] Port the rest of `doc/readme.txt/*.txt` into `man/zsnes.1`
- [ ] Decide whether `find_next_match` needs `+ 1` (`argv.h`)
- [ ] Drop the unfinished-dynarec remnants around `curexecstate` (`initc.c`)
- [ ] Use or drop `pNewDeviceAt` (`mmlib/macos.c`)
