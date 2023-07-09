# ZSNES 2

<img align="right" width="128" height="128" alt="logo" src="icons/128x128x32.png">

The last release of [ZSNES](http://zsnes.sourceforge.net/) was well over 16 years ago. It requires heavy patching and tweaking of compilation flags to build and run on a modern Linux distro.

This is a fork of ZSNES that aims to solve this.

Running `make` produces a 32-bit x86 ELF executable with MMX support, that can be run on a 64-bit x86 system. Large parts of ZSNES is written in 32-bit x86 Assembly, which is not easily ported to other platforms or systems. SSE support is disabled.

Goals and non-goals:

* Compiling ZSNES so that it works on 64-bit x86 is a goal (that has been reached).
* Supporting 32-bit platforms is not a goal, but pull requests are welcome.
* Supporting Windows is not a goal, but pull requests are welcome.
* Supporting modern Linux distros (and FreeBSD, if possible) is a goal.
* To be like an LTS release of ZSNES is a goal.
* Porting ZSNES to SDL2 is a long term goal.
* Adding back a more secure version of the net-play code is a long term goal.

Pull requests that fixes inaccuracies with the emulation are welcome, as well as pull requests for regular bugs.

* Currently, `sdl1` (or better: `sdl12-compat`) is a required dependency.
* The `sdl2` branch is a work-in-progress branch where ZSNES can be compiled with SDL2 but it does not yet run correctly.

Tested on Arch Linux, Fedora and Debian 12 on x86_64.

### Build

    make

For other platforms than Linux, adjust the first 70 lines of the `Makefile` as needed.

### Debug build

Make sure that `gdb` is installed. Then:

    make clean debug

Then type `r` in gdb to run zsnes with the example ROM (`~/roms/snes/example.sfc` must exist). Use ie. `bt full` to see the backtrace if ZSNES crashes.

### Install

    make install

Installing a desktop shortcut is possible. A `zsnes.desktop` file is included in `linux/zsnes.desktop`. It could also be generated with [gendesk](https://github.com/xyproto/gendesk). If you have a suitable icon named `zsnes.png` then they can both be installed with:

    install -Dm644 zsnes.png /usr/share/pixmaps/zsnes.png
    install -Dm644 zsnes.desktop /usr/share/applications/zsnes.desktop

For other platforms than Linux, different flags may apply. ZSNES is primarily one executable, but for UNIX-inspired operating systems, the man page (`man/zsnes.1`) can be installed as ie. `/usr/share/man/man1/zsnes.1` (this is handled by `make install`, though).

### Debian and Ubuntu

Try installing ZSNES from [this flatpak](https://flathub.org/apps/io.github.xyproto.zsnes), or see if the following commands works for you, on **x86_64**. Please create a PR if they don't:

```sh
dpkg --add-architecture i386
apt update
apt install -y git make nasm pkg-config zlib1g-dev build-essential
git clone https://github.com/xyproto/zsnes
cd zsnes
make
apt install -y g++-multilib gcc-multilib libgl-dev libgl-dev:i386 libpng-dev libpng-dev:i386 libsdl1.2-compat-dev libsdl1.2-compat-dev:i386 libsdl2-dev zlib1g-dev zlib1g-dev:i386
make
```

### Pull requests

* Pull requests are welcome.
* A port to SDL2 is extra warmly welcome!
* Being able to build and run ZSNES on Linux is a priority.
* Please have `clang-format` installed and run `./fmt.sh` before submitting a pull request.

### Windows users

* Compiled executables for ZSNES 1.51 for Windows are available at https://sourceforge.net/projects/zsnes/files/zsnes/ZSNES%20v1.51/
* Pull requests for supporting Windows are welcome, but it's not a goal for this project.

### General info

Thanks to Christoph Mallon for the commits that this fork is based on.

* License: GPL2
* Version: 2.0.12
* Fork author: Alexander F. Rødseth &lt;xyproto@archlinux.org&gt;

# Old documentation

* Some of the information in the old text files in `docs/readme.txt/*.txt` has not yet been ported over to the `linux/zsnes.1` man page, or to a Markdown document.
* The plan is to do this. Pull requests are welcome.

This is a summary based on the other text files that were not in `docs/readme.txt/*.txt`:

## List of contributors

These are the contributors listed in the text files that are included with the 1.51 release of ZSNES, and on the zsnes.com webpage:

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

## Additional info from the text files that came with ZSNES 1.51:

### How OpenGL renders the SNES video buffer

The SNES video buffer has dimensions 288x224 (sometimes 288x239 for
certain games; however I have not come across any). The first 16 and last
16 column of pixels are not displayed (perhaps used as a scrolling
buffer?), so the only important part of the video buffer is the middle
256x224 pixels. The SNES video buffer pointer is vidbuffer. With OpenGL,
only the visible 256x224 pixels are needed and they are cropped into
glvidbuffer. glvidbuffer is then turned into a texture that gets bound to
a QUAD whose size depends on whether ZSNES uses aspect ratio to display
each frame.

### Hi-res filters with OpenGL

The video mode selection is taken care of by SDL, including full
screen mode. The current code does not allow for many hi-res filter
options. While it is not difficult to implement the hi-res features using
the current filtering code (in `copyvwin.asm`), it appears that the code for
`copy640x480x16bwin()` causes memory corruption and sometimes segfaults when
you exit ZSNES. For this reason, the filters have been left out. You can,
however, add it in yourself by:

1. Allocating enough memory space for `glvidbuffer` (use `realloc`)
2. Assign `glvidbuffer` to the destination pointer SurfBufD (instead of `surface->pixels`)
3. Setting `Temp1` to `surface->pitch`, i.e. `Temp1 = 2*SurfaceX`
4. Calling `copy640x480x16bwin()`
5. Correctly binding the glvidbuffer as a texture to a QUAD

### Known issues and some work-arounds

* After many video mode switches (all windowed), switching to full screen then back to window mode cases an SDL parachute exit; try not to use too many video mode changes, and restart ZSNES every once in a while if you are just testing out video modes.

* Segfault after having compiled the source -- this might be due to an old `zguicfg.dat` file; delete this and see if the problem gets fixed.

### File list

```
; Porting Routine Files

-DOS-
ZLOADER.C       ; Start-up C routines for DOS (Commandline parser, etc.)
DOSINTRF.ASM    ; DOS Interface routines (File,video,timers,etc.)

-WINDOWS-
ZLOADERW.C      ; Start-up C routines for Windows (Commandline parser, etc.)
Winintrf.asm    ; Windows Interface routines (File,video,timers,etc.)
winlink.cpp     ; Win32 routines (DirectX, Win32 functions)
copyvwin.asm    ; Video Blitter for D modes

-LINUX-
copyvwin.asm    ; Video Blitter for D modes
gl_draw.c       ; OpenGL routines for drawing the video buffer
gl_draw.h       ;
protect.c       ;
sdllink.c       ; SDL routines (video, input, sound init)
sdlintrf.asm    ; Interface routines
sw_draw.c       ; Software (via SDL) drawing routines
sw_draw.h       ;
zfilew.c        ;
zipxw.c         ;
zloaderw.c      ;

-MAN-
zsnes.1         ; man page for zsnes

; ----------------------------------------------------------------
; The following are generally compiled for both ports, but they
; are not necessarily used by both
; ----------------------------------------------------------------

; General Stuff
MACROS.MAC      ; Global Macro file (NEWSYM/EXTSYM global var control macros)
UI.ASM          ; Start-up/initialization routines (env variables, cfg
                ; loader, memory allocation, etc.)
INIT.ASM        ; Emulation initialization routines (eg. memory map, emu
                ; variables, etc.)
CFGLOAD.ASM     ; ZSNES.CFG/ZSNESW.CFG file loading/parsing routines
PROCVID.ASM     ; In-game video initialization/text displays/mouse routines/
                ; 8-bit palette/F3 menu routines
TABLE.ASM       ; Generates the SNES CPU tables (but in an inefficient way)
TABLEB.ASM
TABLEC.ASM
STABLE.ASM
VCACHE.ASM      ; Performs tile caching and also vframe initialization stuff
MENU.ASM        ; F1 Menu routines
ENDMEM.ASM      ; Where those large arrays go
ZFILE.C         ; File Access Routines
GBLHDR.H        ; Global Header Stuff

; ZSNES GUI Stuff
GUI.ASM         ; Main GUI file
GUITOOLS.INC    ; Simple tools for the GUI
GUIWINDP.INC    ; GUI Window Display Routines
GUINETPL INC    ; GUI Netplay Routines
GUILOAD  INC    ; GUI File Load Routines
GUIKEYS  INC    ; GUI Keyboard Input Routines
GUIMOUSE INC    ; GUI Mouse Input Routines
GUICHEAT INC    ; GUI Cheat Code Routines (search + regular codes)
GUICOMBO INC    ; GUI Key Combo Routines
GUIMISC  INC    ; Misc GUI Stuff (Movie, Joystick setting (display) routines,
                ; SNES Reset Function

; DOS Specific stuff
VESA2.ASM       ; VESA2 DOS Routines
VESA12.ASM      ; VESA1.2 DOS Routines
ZSIPX.ASM       ; IPX routines (DOS)
JOY.ASM         ; DOS Joystick Drivers
COPYVID.INC     ; Graphics blitter routines for DOS (copys screen to display)
INITVID.ASM     ; Video Initialization routines (DOS)
DEBUG.ASM       ; SNES Debugger routines
SW.ASM          ; Sidewinder routines
SW32.ASM        ; More Sidewinder routines (not sure which file is the real
                ;   one)
GPPRO.ASM       ; Gamepad Pro routines (non-USB)
MODEMRTN.ASM    ; DOS Modem routines

; Graphics Enhancers
2XSAIMMX.INC    ; Kreed's 2xSaI routines
2xSaI.cpp       ; Kreed's Super 2xSaI routines
2xsaiw.asm      ; 2xSaI for windows (without selector usage)
water.c         ; Water Effect

; SNES CPU Emulation routines
execute.asm     ; Main emulation execution loop routines (includes save
                ;   states and in-game netplay routines)
SPC700.ASM      ; SNES SPC700 Sound CPU emulation routine files
SPCADDR.INC
SPCDEF.INC
DSPPROC.ASM     ; SNES Digital Sound Processor main emulation routines
DSP.ASM         ; SNES Digital Sound Processor register routines
DMA.ASM         ; SNES PPU DMA/HDMA routines
IRQ.ASM         ; SNES 65816 IRQ routines (VIRQ/NMI)
MEMORY.ASM      ; SNES Memory access routines + C4 routines (Sorry that I
                ;   didn't have these in a separate file since a small memory
                ;   hack of the C4 originated on this file and just grew and
                ;   grew)
E65816.INC      ; 65816 emulation routines (w/o SPC700)
65816D.INC
E65816B.INC     ; 65816 emulation routines (Debugger I think)
65816DB.INC
E65816C.INC     ; 65816 emulation routines (w/ SPC700) - Sorry about these
                ;   3 files.  They used to be just 1 until I removed the
                ;   self-modifying code routines
65816DC.INC
SE65816.INC     ; 65816 emulation routines (SA-1)
S65816D.INC
ADDRNI.ASM      ; 65816 Non-incrementing addressing modes
ADDRNI.INC
SADDRNI.INC
ADDRESS.INC     ; 65816 Incrementing addressing modes
ADDRESS2.INC
SADDRESS.INC
REGS.INC        ; SNES I/O register emulation
REGS.MAC
REGSW.INC
REGSW.MAC

; SNES PPU Emulation Routines
MAKEVID.ASM     ; 8-bit old graphics engine
MAKEV16B.ASM    ; 16-bit old graphics engine
MAKEV16T.ASM    ; 16-bit old graphics engine (transparencies)
MV16TMS.ASM     ; 16-bit old graphics engine (transp w/ Main+Sub enabled)
MODE7.ASM       ; 8-bit mode 7 routines
MODE7.MAC
MODE7EXT.ASM    ; 8-bit mode 7 EXTBG routines
MODE716B.ASM    ; 16-bit mode 7 routines
MODE716T.ASM    ; 16-bit mode 7 routines (transparencies)
MODE716E.ASM    ; 16-bit mode 7 EXTBG routines
M716TEXT.ASM    ; 16-bit mode 7 EXTBG routines (transparencies)
MODE716D.ASM    ; 16-bit mode 7 Direct routines
mode716.asm     ; Mode 7 16-bit new graphics engine routines
mode716.mac
NEWGFX.ASM      ; 8-bit new graphics engine files
NEWGFX.MAC
NEWGFX2.ASM
NEWGFX2.MAC
NEWGFXB.MAC
NEWGFXWN.MAC
newgfx16.asm    ; 16-bit new graphics engine files
newgfx16.mac
NEWG162.ASM
newg162.mac
NEWG163.MAC
NEWG16WN.MAC
VIDMACRO.MAC    ; Generic Video Macros File
VIDMACRB.MAC

; Special Chip Emulation Routines
FXEMU2.ASM      ; Super FX emulation files
FXEMU2.MAC
FXEMU2B.ASM
FXEMU2B.MAC
FXEMU2C.ASM
FXEMU2C.MAC
FXTABLE.ASM
SFXPROC.ASM
DSP1PROC.ASM    ; DSP1 communication routines
DSP1EMU.C       ; DSP1 C routines (also includes some C4 C routines)
SA1PROC.ASM     ; SA-1 processing routinest
SA1REGS.ASM     ; SA-1 registers, also includes S-DD1 and SPC7110 routines
```

### Issues

#### All ports

- Recode netplay feature, once core is not random anymore.

#### SDL Port

- Low performance due to differences in surface sizes (internal and SDL), make both the same size.
- OpenGL code is awful, incomprehensible, stupid and awful again. We must fix it.
- Support overscan in games like *Dragon Quest 5*

#### Windows port

- OpenGL, or Direct3D support would be nice (we have the source for OpenGL but it needs to be integrated into the main tree, any takers?)
- Windows port should be converted to Direct3D from DirectDraw 7.0.
- Windows sound code needs to be rewritten to reflect the SDL port.

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

- Fix some windowing/sub-screen sprite problems that are still present in the new graphics engine. Probably pagefault will have to do this.
