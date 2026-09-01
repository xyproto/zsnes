# ZSNES 2

[![CI](https://github.com/xyproto/zsnes/actions/workflows/build.yml/badge.svg)](https://github.com/xyproto/zsnes/actions/workflows/build.yml)

<img align="right" width="128" height="128" alt="logo" src="img/128x128x32.png">

The last release of [ZSNES 1](http://zsnes.sourceforge.net/) was over 18 years ago (2007-10-31). It requires heavy patching and tweaking of compilation flags to build and run on a modern Linux distro. Most of it was written in 32-bit x86 Assembly, with MMX instructions.

This is a fork that aims to solve this.

Goals and non-goals:

* Compiling ZSNES so that it works on 64-bit x86 is a goal (that has been reached).
* Porting the Assembly to C11 is a goal (that has been reached).
* Supporting modern Linux distros (and FreeBSD and macOS, if possible) is a goal.
* Supporting multiple CPUs and platforms is a goal (that has been reached).
* Supporting DOS is not a goal.
* Supporting Windows is not a goal, but a "nice to have".
* Improving the net-play code is a long term goal.
* To be like an LTS release of ZSNES is a goal.

All of the old Assembly code has been ported to C11.

Pull requests that fixes inaccuracies with the emulation are welcome, as well as pull requests for regular bugs.

### Requirements

* `python3`, `sdl3`, `pipewire` (or `libao`), and a C compiler like `gcc` or `clang`.
* `nasm` is only needed for testing.

Tested on Arch Linux, Fedora and Debian 12 on x86_64, and on macOS on aarch64.

### Build

    make

### Debug build

Make sure that `gdb` (or `lldb`) is installed, then:

    make clean debug

Type `r` in gdb to run zsnes with the example ROM (`~/roms/snes/example.sfc` must exist). Use ie. `bt full` to see the backtrace if ZSNES crashes.

### Install

    make install

Installing a desktop shortcut is possible. A `zsnes.desktop` file is included in `linux/zsnes.desktop`. It could also be generated with [gendesk](https://github.com/xyproto/gendesk). If you have a suitable icon named `zsnes.png` then they can both be installed with:

    install -Dm644 zsnes.png /usr/share/pixmaps/zsnes.png
    install -Dm644 zsnes.desktop /usr/share/applications/zsnes.desktop

For other platforms than Linux, different flags may apply. ZSNES 2 is primarily one executable, but for UNIX-inspired operating systems, the man page (`man/zsnes.1`) can be installed as ie. `/usr/share/man/man1/zsnes.1` (this is handled by `make install`, though).

### macOS

Install the dependencies with [Homebrew](https://brew.sh/), then build:

```sh
brew install libao libpng pkgconf python3 sdl3
git clone https://github.com/xyproto/zsnes
cd zsnes
make
```

### Debian and Ubuntu

Try installing ZSNES from [this flatpak](https://flathub.org/apps/io.github.xyproto.zsnes), or see if the following commands works for you, on **x86_64**. Please create a PR if they don't:

For a 32-bit x86 build while on x86_64:

```sh
dpkg --add-architecture i386
apt update
apt install -y git make pkg-config python3 zlib1g-dev build-essential
git clone https://github.com/xyproto/zsnes
cd zsnes
make
apt install -y gcc-multilib libglvnd-dev libglvnd-dev:i386 libpng-dev libpng-dev:i386 libsdl3-dev libsdl3-dev:i386 zlib1g-dev zlib1g-dev:i386
make
```

### Cross-Compiling for Windows on Debian/Ubuntu

Try the following commands in ie. Ubuntu 24 running under WSL:

```sh
apt update
apt install -y git make mingw-w64 libz-mingw-w64-dev python3 pkg-config build-essential
git clone https://github.com/xyproto/zsnes
cd zsnes
make ARCH=win WITH_PNG= CC_TARGET=i686-w64-mingw32-gcc CC=i686-w64-mingw32-gcc WINDRES=i686-w64-mingw32-windres
```

### Compiling for Windows with MSYS2

Use MSYS2 MINGW32.

Depending on your installation, you may not have a shortcut created for this. If so, locate ``mingw32.exe`` in your MSYS2 installation directory.

Run the following commands. Please create a PR if you have issues.

Tested on MSYS2 x86_64 version 20260322.

```sh
pacman -Syu
pacman -Sy git make pkg-config python3 mingw-w64-i686-gcc mingw-w64-i686-libpng mingw-w64-i686-zlib mingw-w64-i686-SDL3
make ARCH=win
```

### Pull requests

* Pull requests are welcome.
* Being able to build and run ZSNES on Linux is a priority.
* Please have `clang-format` installed and run `make fmt` before submitting a pull request.

### Windows users

* Compiled executables for ZSNES 1.51 for Windows are available at https://sourceforge.net/projects/zsnes/files/zsnes/ZSNES%20v1.51/
* Pull requests for supporting Windows are welcome, but it's not a goal for this project.

### General info

Thanks to Christoph Mallon for the commits that this fork is based on.

### List of contributors

These are the contributors listed in the text files that are included with the 1.51 release of ZSNES, and on the zsnes.com webpage:

* `zsKnight`
* `_Demo_`
* `pagefault`
* `Nach`
* `grinvader`
* `Deathlike2`
* `Jonas Quinn`
* `blargg`
* `Pharos`
* `teuf`
* `relnev`
* `prometheus`
* `theoddone33`
* `EvilTypeGuy`
* `stainless`
* `aaronl`
* `MKendora`
* `kode54`
* `zinx`
* `amit`
* `hpsolo`
* `Kreed`
* `Neviksti`
* `ipher`
* `relnev`
* `StatMat`
* `MKendora`
* `hpsolo`
* `aaronl`
* `Diablo-D3`
* `Wilbern Cobb`
* `Thorsten "mirabile" Glaser`
* `Mitchell "The Khan Artist/Noxious Ninja" Mebane`

### Previous documentation

* Some of the information in the old text files in `docs/readme.txt/*.txt` has not yet been ported over to the `man/zsnes.1` man page or to a Markdown document.
* The plan is to do this. Pull requests are welcome.

Additional documentation:

* [olddoc.md](olddoc.md)
* [html](html)

### General info

* License: GPL2
* Version: 2.3.0
* Fork author: Alexander F. Rødseth &lt;xyproto@archlinux.org&gt;
