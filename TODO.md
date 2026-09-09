# TODO

- [ ] Confirm the DSP4 division table on a DSP4 cart. `floor(0x8000/n)` verified, but not the chip's rounding.
- [ ] Fix Super Mario RPG hanging in the attract demo at frame 19956, force-blank never cleared. Needs a reference trace.
- [ ] Retire the old graphics engine (`newengen=0` + `bgfixer`).
- [ ] Use `SDL_Gamepad` for default bindings only. Saved bindings index the flat 256+ space and must not move.
- [ ] Figure out the ST-011 OPS04/05 outputs and OPS06/07 purposes (`chips/seta11.c`).
- [ ] Move the rest of the GUI panels onto GUIStackLayout: Combo, Optns, Cheat search, Movies, Input, Speed, Cheat
- [ ] Check PipeWire and libao determinism with `ZSNES_STATE_HASH=N` on a system that has both.
- [ ] Handle horizontal scroll and absolute mouse motion better on macOS (`mmlib/macos.c`)
- [ ] Check the `SDL_Renderer` path on real 512-wide content, fullscreen, and the old engine (`ZSNES_FRAME_DUMP=N`)
- [ ] Use `DT_MAIN` in `difftest_ng2.c` or stop including `difftest.h` to make it a bit quieter.
- [ ] Fix the netplay desync at frame 85, not the input path. Maybe the per-frame pad overwrites.
- [ ] Replace netplay's delay-lockstep with rollback.

## Maybe

- [ ] Write `win/net_transport.c` and drop the `__UNIXSDL__` guards to get netplay on Windows.
- [ ] Add the missing ST-011 opcodes; blocked until someone logs them from Morita Shougi 2 (stderr names each one)
- [ ] Legacy GL always runs `hq2x_16b()` whatever `hqFilterlevel` says; fix, or let it go with the path
- [ ] Test the HDR path on a display that reports HDR; `sr_to_hdr` has never run
- [ ] Allocate the ROM buffer per cart instead of a fixed 12MB at startup (#17)
- [ ] Port the controls, GUI and netplay prose from `doc/*.html` into `man/zsnes.1`
- [ ] Test `unix/battery.c` on a machine that reports battery state
- [ ] Re-enable the FreeBSD, OpenBSD and NetBSD CI jobs
- [ ] Give the difftests a 64-bit oracle (so it no longer depends on the 32-bit oracle).
