# ZSNES 2 (Custom Test Branch)
This is a new branch of https://github.com/xyproto/zsnes to test certain features, a full conversion to SDL/OpenGL, Netplay Reimplementation, along other features. All changes here are experimental and aren't ready for the main branch yet.

<img align="right" width="128" height="128" alt="logo" src="icon.png">

The last release of [ZSNES](http://zsnes.sourceforge.net/) was well over 16 years ago. It requires heavy patching and tweaking of compilation flags to build and run on a modern Linux distro.

This is a fork of ZSNES that aims to solve this.

Running `make` produces a 32-bit x86 ELF executable with MMX support, that can be run on a 64-bit x86 system. Large parts of ZSNES is written in 32-bit x86 Assembly, which is not easily ported to other platforms or systems.

Goals and non-goals:

* Compiling ZSNES so that it works on 64-bit x86 is a goal (that has been reached).
* Supporting 32-bit platforms is not a goal, but pull requests are welcome.
* Supporting Windows is not a goal, but pull requests are welcome.
* Supporting modern Linux distros (and FreeBSD, if possible) is a goal.
* To be like an LTS release of ZSNES is a goal.
* Porting ZSNES to SDL2 is a long term goal.
* Adding back a more secure version of the net-play code is a long term goal.

Pull requests that fixes inaccuracies with the emulation are welcome, as well as pull requests for regular bugs.

### General info
* License: GPL2
* Version: 2.0.13

### Build
```sh
make -j$(nproc)
```

For other platforms than Linux, adjust the first 70 lines of the `Makefile` as needed.

### Debug build
Make sure that `gdb` is installed. Then:
```
make clean debug
```
Then type `r` in gdb to run zsnes with the example ROM (`~/roms/snes/example.sfc` must exist). Use ie. `bt full` to see the backtrace if ZSNES crashes.

### Compilation

**This is still being tested, compilation appears to work on Ubuntu but requires some extra steps on Ubuntu, read** https://github.com/xyproto/zsnes/issues/19

```sh
sudo dpkg --add-architecture i386
sudo apt update
sudo apt upgrade
sudo apt install g++-multilib gcc-multilib libgl-dev libpng-dev zlib1g-dev nasm libgl1-mesa-dev:i386 libgl1-mesa-glx:i386
```

### Packages needed on Windows
These have to be downloaded from the File link directly. Fully tested to be working.
https://packages.msys2.org/package/mingw-w64-i686-SDL?repo=mingw32
https://packages.msys2.org/package/mingw-w64-i686-zlib?repo=mingw32

### Pull requests

* Pull requests are welcome.
* A port to SDL2 is extra warmly welcome!
* Being able to build and run ZSNES on Linux is a priority.
* Please have `clang-format` installed and run `./fmt.sh` before submitting a pull request.

## List of contributors and credits
Thanks to Christoph Mallon for the commits that this fork is based on.

* zsKnight
* `_Demo_`
* pagefault
* Nach
* grinvader
* Deathlike2
* Jonas Quinn
* blargg
* Pharos
* teuf
* relnev
* prometheus
* theoddone33
* EvilTypeGuy
* stainless
* aaronl
* MKendora
* kode54
* zinx
* amit
* Khan Artist
* hpsolo
* Kreed
* Neviksti
* ipher
* relnev
* StatMat
* MKendora
* hpsolo
* aaronl
* Diablo-D3
* Wilbern Cobb
* Thorsten "mirabile" Glaser
* Mitchell "The Khan Artist/Noxious Ninja" Mebane
* Fork author: Alexander F. Rødseth &lt;xyproto@archlinux.org&gt;
* Sneed

### Issues

#### SDL Port
- Low performance due to differences in surface sizes (internal and SDL), make both the same size.
- OpenGL code is awful, incomprehensible, stupid and awful again. We must fix it.
- Support overscan in games like *Dragon Quest 5*

#### Compatibility
- Games not working:
  - *Guikuden 1*
  - *Cu-On-Pa*
  - Loads more
- Graphics Glitches:
  - *Killer Instinct* (black background)
  - *Final Fantasy III* (known as *Final Fantasy VI* in Japan; Range Time Over and other sprite trouble)
  - *Tactic Ogre* (Menus)
  - Lots of others
- Special Chips:
  - DSP1 (99% done)
  - DSP3 (50% done)
  - DSP4 (99% done)
  - SA-1 (95% done)
  - SPC7110 (Needs decompression)
  - SFX1/2 (95% done)
  - Seta 11 (25% done)
  - Seta 18 (1% done)
  - BS-X (50% done)

#### Timing engine
- Convert counter to 32-bit. (assigned to pagefault).
- Fix 65816 timing and take into consideration cycle differences in 8/16-bit mode, branches etc.

#### Graphics Engine
- Fix some windowing/sub-screen sprite problems that are still present in the new graphics engine.