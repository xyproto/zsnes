# TODO

- [ ] Use `SDL_Gamepad` so controllers get SDL's mapping database instead of
      raw numbered axes and buttons
- [ ] Add drift control: nudge `SDL_SetAudioStreamFrequencyRatio` from the
      queued-samples count, the way snes9x's DynamicRateControl does
- [ ] Always emulate the DSP at its native rate and let `SDL_AudioStream`
      resample, rather than retuning the DSP per `SoundQuality` setting
- [ ] Replace the legacy OpenGL 1.x path with `SDL_Renderer` and a streaming
      texture, so macOS gets Metal and Windows D3D without an OpenGL dependency
- [ ] Use `glTexSubImage2D` rather than reallocating the texture every frame,
      for as long as the GL path stays
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
- [ ] Retire the old graphics engine (`newengen=0`) once the new one has no
      known regressions, and drop `bgfixer` with it
- [ ] Re-enable the FreeBSD, OpenBSD and NetBSD CI jobs
- [ ] Give the difftests a 64-bit oracle so they run off 32-bit x86
- [ ] Give the pinned table runs (`BG1SYl`, `BGPT1` ...) flat aliases so the
      cross-table indexing stops tripping UBSan
- [ ] Read the word and dword seams (`cpu/ops65816.h`, `video/c_ng2tile.c`)
      through `memcpy` rather than casts; the unaligned loads the assembly took
      for granted are UB and would fault on a strict-alignment target
- [ ] Build `test/`'s parent objects through the top Makefile, not make's implicit rule
- [ ] Test `linux/battery.c` on a machine that reports battery state
- [ ] Drop the unfinished-dynarec remnants around `curexecstate` (`initc.c`)
- [ ] Port the rest of `doc/readme.txt/*.txt` into `man/zsnes.1`
- [ ] Point `HomebrewFormula/zsnes.rb` at the 2.3.0 tarball once it is tagged,
      and publish it as a tap
