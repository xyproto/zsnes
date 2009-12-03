ZSNES v1.51 Documentation

================================
    N a v i g a t i o n
================================

    * Index    [Index.txt]

    * Readme    [Readme.txt]
        1. Disclaimer
        2. Current Progress
        3. Extra Features
        4. System Requirements
        5. Installation
        6. Basic Usage
        7. Default Keys
        8. Save States
        9. Movies
        10. IPS Patching
        11. Cheat Codes
        12. Files

    * GUI    [GUI.txt]

    * Netplay    [Netplay.txt]

    * Advanced Usage    [Advanced.txt]

    * Games    [Games.txt]

    * FAQ    [FAQ.txt]

    - - - - - - - - - - - - - - - - - -

    * Getting Support    [Support.txt]

    * History    [History.txt]

    * About    [About.txt]

    * License    [License.txt]

    - - - - - - - - - - - - - - - - - -

    * NSRT Guide:    [http://zsnes-docs.sf.net/nsrt]

    * ZSNES Home Page:  [ZSNES.com]


================================================================================
~                                 R e a d m e
================================================================================

ZSNES is an open-source Super Nintendo Entertainment System emulator written in
x86 assembly, C, and C++. Bleeding with cutting edge SNES emulation, ZSNES is
easily comparable to other leading SNES emulators, such as Snes9x
[http://www.snes9x.com], SNEeSe [http://sneese.sourceforge.net], Super Sleuth
[http://users.tpg.com.au/advlink/spx/], and bsnes [http://byuu.org].

Special thanks to the Snes9x team for all of their help and also for the
excellent SNES emulator they have developed. We wish them the very best of luck!


............................................................
  1.                     Disclaimer
............................................................

The ZSNES Development Team, including all developers and contributors, is in no
way responsible for any damage caused by the use of this software. Please read
the license [License.txt] for more details.

Due to legal issues, the ZSNES Development Team can provide you neither with
ROMs nor links to them. In addition, ZSNES may not be distributed with ROM
images. However, as with many cases like this, Google [http://www.google.com]
is your friend.

There are still many bugs left in ZSNES, so don't expect it to run all your
favorite games. If ZSNES doesn't work for you, then don't use it. Use Snes9x,
SNEeSe, Super Sleuth, or bsnes instead! In fact, even if you use ZSNES, use
those emulators too!


............................................................
  2.                  Current Progress
............................................................

 - - - - - - - - - - - - - - - - -
  The following are implemented:
 - - - - - - - - - - - - - - - - -
    - Complete 65816 instruction set
    - SRAM support
    - LoROM and HiROM support
    - SlowROM and FastROM support
    - Full DMA support
    - HIRQ/VIRQ/NMI Interrupts
    - Support for several SNES file formats (SMC, SFC, SWC, FIG, MGD, MGH, UFO,
      BIN, GD3, GD7, DX2, USA, EUR, JAP, AUS, ST, BS, 048, 058, 078,), including
      split files (1, 2, 3; A, B, C)
    - Interleaved format support (except SuperFX games)
    - PAL/NTSC timing support

 - - - - - - - - - - - - -
  Graphics engines (PPUs)
 - - - - - - - - - - - - -

  The following are implemented in both graphics engines, all color modes:
    - Graphic modes 0,1,2,3,4,5,6,7
    - 8x8, 16x16, 32x32, and 64x64 sprite support (flipped in all directions)
    - 8x8 and 16x16 tiles
    - 32x32,64x32,32x64,64x64 tile modes
    - Full HDMA effects for wavy backgrounds, interesting mode 7 effects, etc.
    - Mode 7 rotating and scaling effects
    - BG priorities
    - Sprite priorities
    - Add/sub of back area
    - Mosaic effects

  About the old graphics engine:
    - Line engine
    - Missing lots of windowing and DMA effects
    - More accurate at drawing some things
    - 13-bit color rendering. (This is a compromise between color accuracy and
      speed. Not all transparencies will work correctly with this engine.)

  What's available in the old graphics engine when using an 8-bit color video
  mode:
    - Offset per tile mode (mode 2/vertical only)
    - High-res 512 horizontal resolution (missing in 16x16)
    - Single and dual windowing routines

  What's available in the old graphics engine when using a 16-bit color video
  mode:
    - Palette changing in the middle of a screen
    - Screen addition (full and half)
    - Screen subtraction (full)
    - Fixed color addition/subtraction
    - Window clipping for fixed color

  About the new graphics engine:
    - Tile engine
    - Nearly complete engine with a few bugs
    - Can draw mostly everything on the SNES
    - 15-bit coloring

  What's available in the new graphics engine when using an 8-bit color video
  mode:
    - Offset per tile mode (mode 2/vertical only, mode 4)
    - High res 512 resolution and 448/478 vertical resolution
    - Windowing effects
    - High resolution mode 7 (only in 640x480x256 video mode; active when all
      other video filters are disabled)

  What's available in the new graphics engine when using a 16-bit color video
  mode:
    - All of old graphics engine 16-bit
    - High resolution mode 7
    - High resolution 16x16 tiles for mode 5
    - Full 15-bit color transparencies for improved picture quality (MMX
      compatible CPUs only)

 - - - - - - - - - - - - - - - - - - -
  The following are present in sound:
 - - - - - - - - - - - - - - - - - - -
    - 16-bit digital stereo sound
    - SPC700 Sound CPU
    - DSP Sound Processor
          - Echo effects
          - FIR filter
          - ADSR volume effects
          - GAIN volume effects
          - Noise effects
          - Pitch modulation

 - - - - - - - - - - - - - - - - - - - - - - - - - -
  The following special input devices are emulated:
 - - - - - - - - - - - - - - - - - - - - - - - - - -
    - MultiTap (5-player support)
    - Super NES Mouse (missing some features, such as speed settings)
    - Super NES Super Scope
    - Konami Lethal Enforcer Gun
    - Automatic configuration via NSRT headers
      (See APIs and Utilities section [Readme.txt])

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  The following special cartridge processors are emulated, in whole or in part:
 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   | Special Chip  | Progress |
   ----------------|-----------
    C4             | 100%
    Nintendo DSP-1 | 100%
    Nintendo DSP-2 | 100%
    Nintendo DSP-3 |  80%
    Nintendo DSP-4 |  95%
    OBC-1          | 100%
    SA-1           |  90%
    S-DD1          | 100%
    Seta DSP 10    |  99%
    Seta DSP 11    |  80%
    SPC7110        | 100% except decompression
    S-RTC          |  95%
    SuperFX        |  90%

 - - - - - - - - - - - - - - - - - - - - - - -
  The following add-on devices are emulated:
 - - - - - - - - - - - - - - - - - - - - - - -
   | Special Cartridge / Add-On  | Progress |
   ------------------------------|-----------
    Broadcast Satellaview (BS-X) |  50%
    Nintendo Super System        | 100% except the menus
    Same Game, SD Gundam G-Next  | 100%
    Sufami Turbo                 |  95%

 - - - - - - - - - - - - - - - - - - -
  The following features are missing:
 - - - - - - - - - - - - - - - - - - -
    - Pseudo 512 SNES horizontal resolution (no games are known to use this)
    - Some modes in offset per tile mode
    - Some direct color modes (no games are known to use this)
    - Seta RISC chip
    - True SPC7110 decompression
    - Super GameBoy emulation

 - - - - - - - - - - - - - - - - - - - - - -
  What will not run (or not play properly):
 - - - - - - - - - - - - - - - - - - - - - -
    Please read our statement on game compatibility [Games.txt].

    - Some SuperFX games (regardless of interleave status)
    - Interleaved SuperFX games (deinterleave them with NSRT
      [http://nsrt.edgeemu.com])
    - Some Broadcast Satellaview (BS-X) games
    - Games with unknown co-processors
    - Games that don't have a valid header
    - Games that hit a severe bug in the 65816/PPU/SPC700/DSP routines
    - Games that require special timing


............................................................
  3.                   Extra Features
............................................................

    - Support for save states, including rewinding
    - Extensive movie recording and dumping features
    - Many emulation speed options, including automatic frame skipping to
      compensate for slower machines
    - Full cheat code support (including Game Genie, Pro Action Replay, and
      GoldFinger)
    - Automatic IPS soft-patching (including up to 11 sequential patches)

    - Custom-built GUI with many time-saving features
    - Support for loading Zip, gZip, and JMA-compressed ROMs
    - Randomized ROM loading
    - Support for input from keyboards, joysticks, and gamepads,
      as well as a key combination editor
    - Many video output options, including graphics-enhancing filters
    - Highly configurable sound output options

    - Netplay (currently disabled)

    - User-editable configuration files
    - Debugger
    - Accepts command-line arguments


............................................................
  4.                System Requirements
............................................................

 - - - - - - - - - - - - - - -
  Supported operating systems
 - - - - - - - - - - - - - - -

  Official Ports
    - Win port: Microsoft Windows 95/98/ME/2000/XP/2003/Vista
    - SDL port: Linux, BSD, Mac OS X, or Xbox running Linux
    - DOS port: Microsoft DOS (may work on other non-MS DOSes)

  Un-Official Ports
    - ZsnexBox: Microsoft Xbox (native)

      ZSNES Board thread about the Xbox port:
      [http://board.zsnes.com/phpBB2/viewtopic.php?t=6933]

 - - - - - - - - - -
  CPU requirements
 - - - - - - - - - -

  ZSNES absolutely requires a 100% x86-compatible processor. You probably
  already meet this requirement. Most consumer-grade processors sold by Intel
  and AMD use the x86 instruction set.

  Because much of ZSNES' source code is written in x86 assembly, it will only
  run on processors that are 100% x86 compatible. "Ports" to other architectures
  are impossible; we recommend Snes9x [http://www.snes9x.com] as the SNES
  emulator of choice for portability.

  Playing a special chip game will significantly increase CPU usage. For these
  games, you may require a processor faster than those listed below.

 - - - - - - - - - - - - -
  Free space requirements
 - - - - - - - - - - - - -

  The program files alone require about 1MB. The amount of disk space required
  for other files varies greatly. For example, uncompressed ROMs (not included!)
  require 256KB-6144KB each. Save states typically require about 270KB each;
  however, this can increase up to an additional 200KB for special chip games.

 - - - - - -
  Win Port
 - - - - - -

  OS: Windows 95/98/ME
    - CPU: Pentium II (or equivalent) 233MHz (500MHz recommended)
    - RAM: 32MB (64MB recommended)
  OS: Windows 2000/XP/2003/Vista
    - CPU: Pentium II (or equivalent) 266MHz (500MHz recommended)
    - RAM: 64MB of RAM (128MB recommended)
  API: DirectX v8.0a or later must be installed
  Video: any video card that supports DirectDraw (acceleration recommended)
  Sound: any sound card that supports DirectSound (acceleration recommended)

  System Requirements for Microsoft Windows Operating Systems:
    [http://support.microsoft.com/kb/304297/]

  System Requirements for Windows XP Operating Systems:
    [http://support.microsoft.com/kb/314865/]

 - - - - - -
  SDL Port
 - - - - - -

  OS: Linux, BSD, or Mac OS X
  CPU: 266MHz (500MHz recommended, especially if using X)
  RAM: 32MB (64MB recommended; more if SDL is compiled to use X)
  API: SDL v1.20 or later
  Video: almost any video card will work (hardware OpenGL support and
    acceleration highly recommended)
  Sound: any sound card supported by SDL (using ALSA or OSS)

 - - - - - -
  DOS Port
 - - - - - -

  OS: Microsoft DOS (some non-MS DOSes may work)
  CPU: Pentium II (or equivalent) 233MHz
  RAM: 32MB (minimum of 17MB free, required for loading 48mbit ROMs)
  Video: VGA card
    - For 16-bit color and therefore proper support of transparencies, an SVGA
      card with VESA 2 and Linear Frame Buffer support is required.
    - You may be able to use Scitech Display Doctor to enable VESA 2 support on
      some cards that don't already support it.
  Sound: Sound Blaster Pro or 100% compatible (SB16 or 100% compatible
    recommended)

  These system requirements assume you are running the DOS port under pure DOS.
  If you are using the DOS port from within Windows, the CPU and RAM
  requirements will be the same as for the Win port.


............................................................
  5.                    Installation
............................................................

- - - - - - - - - -
  Win / DOS Ports
- - - - - - - - - -

Installation:

    1. Download the latest version of ZSNES from [ZSNES.com].
       The file you download is an archive containing the ZSNES binary file and
       documentation.
    2. Extract the contents of the archive into a new folder on your hard drive.
       Do not simply overwrite an older version of ZSNES.
    3. You can now run ZSNES by executing the ZSNES binary, named zsnesw.exe
       (Windows) or zsnes.exe (DOS). ZSNES is not packaged with an installer, so
       there will be no entry in the Windows Start Menu.

Note: In Windows, you can create a shortcut to ZSNES to make it easier to open
  the program. Right-click on the zsnesw.exe icon to bring up the context menu,
  and left-click "Create Shortcut". A shortcut to the executable file will
  appear in the folder. You can now move the newly-created shortcut to your
  Desktop or Start Menu. Opening the shortcut will run ZSNES from its original
  location.


Re-Installation / Reset to default settings:

If you find that you are experiencing a number of unexplained errors in ZSNES,
or if you wish to reset all settings back to their defaults, simply delete the
configuration files that were generated by ZSNES the first time you ran the
program. See the Configuration Files section [Advanced.txt] for details.


Un-Installation:

If you wish to uninstall ZSNES, simply delete the entire folder (and thus, all
files contained within) into which you installed ZSNES (as described in Step 2
of Installation, above).

-or-

If you have since put additional files (such as ROMs) into your ZSNES install
folder, and do not wish to delete or move them, you will have to delete the
individual ZSNES files. Please refer to the Files section [Readme.txt] for
information on individual files and file types related to ZSNES. You will also
need to delete the docs folder.

Note: ZSNES does not use the Windows registry*, nor does it generate "hidden"
  configuration files all over your system.
  *Description of the Windows registry: [http://support.microsoft.com/kb/256986]


- - - - - - - - - - - - - - - - - -
 SDL Port / Compiling from source
- - - - - - - - - - - - - - - - - -

    * Download the latest source release of ZSNES from [ZSNES.com].
    * After unpacking, navigate to the src directory and run the following
      commands:
          ./configure --enable-release
          make
      And as root:
          make install
    * Other, more detailed instructions are provided in the docs/install.txt
      file.


Un-Installation:

If you wish to uninstall ZSNES, you can run "make uninstall" as root if you
still have your Makefile.

Otherwise, you will need to navigate to /usr/local/bin and delete zsnes. Then
navigate to /usr/local/man/man1 and delete zsnes.1. Or just delete the man1
directory if you have nothing else in it.

You will need root access to perform the above actions.

You will also need to delete "~/.zsnes" or "~/Library/Application Support/ZSNES"
in Mac OS X. Do note that various files are saved in here by default (such as
game saves); be sure to back them up if you want to keep them.


............................................................
  6.                    Basic Usage
............................................................

  1. Install ZSNES (see above).
  2. Run ZSNES.
       * If you're using Windows, double-click on the executable file.
       * If you're using DOS, navigate to the ZSNES installation folder and type
         zsnes.exe at the command line.
  3. Configure the input settings (Config Menu -> Input) as desired, or use the
     default settings.
  4. Configure the video settings (Config Menu -> Video) as desired, or use the
     default settings.
  5. Configure path settings (Config Menu -> Paths) if you don't want all the
     automatically generated files going into the same directories as your ROMs.
  6. Load a game (Game Menu -> Load) and start playing.
  7. When you are ready to stop playing, you have a number of choices to save
     your game.
       * If your game has its own native save function, just use it.
       * If your game does not have a save function, or you are at a point in
         the game where you can't save, you can create a save state. Do this by
         pressing F2.
  8. After you save your game:
       * You can load a new game using the same steps as above, or
       * Exit the emulator by going to Game Menu -> Quit.
  9. When you are ready to return to a previously saved game, just re-load that
     game.
       * Load an in-game save in the normal way.
       * If you saved a state, you can load that state by pressing F4.

This section only covers very basic usage. Please read the entire documentation
for more information.


............................................................
  7.                    Default Keys
............................................................

- - - - - - - - - - - - - - - -
 . . . . Game . . Keys . . . .
- - - - - - - - - - - - - - - -

You can change the default keys for the standard SNES controller under
Config->Input [GUI.txt].

|SNES Button|    |Player 1 Key|   |Player 2 Key|
- - - - - - - - - - - - - - - - - - - - - - - - -
  D-Pad Up         Arrow Up             J
  D-Pad Down       Arrow Down           M
  D-Pad Left       Arrow Left           N
  D-Pad Right      Arrow Right          ,
  Start            Return/Enter     Left Ctrl
  Select           Right Shift      Left Alt
  A                    X            Home
  B                    Z            End
  X                    S            Insert
  Y                    A            Delete
  L (Left Shoulder)    D            Page Up
  R (Right Shoulder)   C            Page Down


You can change the default keys for special input devices under
Config->Devices. [GUI.txt].

The special input devices just use input from your mouse for movement and
aiming.

|Super Scope Button  | Computer/Mouse Button|
- - - - - - - - - - -|- - - - - - - - - - - -
  Fire               |  Left mouse button
  Cursor Mode Button |  Right mouse button
  Toggle Auto-fire   |  =
  Pause              |  Backspace


- - - - - - - - - - - - - - - - -
 . . . Emulator . . Keys . . . .
- - - - - - - - - - - - - - - - -

[Where to Customize]
|Key|    |Function|
= = = = = = = = = =
[Cannot be changed]
 Esc    When a game is loaded, toggle the GUI
          (pauses emulation while GUI visible).
 F1     Open the F1 Quick Menu.
- - - - - - - - - -
[Config->Saves]
 F2     Save a state to current slot.
 F3     Open the save state slot chooser.
 F4     Load a save state from the current slot.
- - - - - - - - - -
[Misc->Misc Keys]
 F5 <--> F12  Toggle sound channels 1 through 8, respectively
 1, 2, 3, 4   Toggle background layers 1, 2, 3, and 4, respectively
 5            Toggle sprite/object layer
 6            Panic Key: Reset all switches to default (enable Offset Mode,
               Windowing, all background layers, sprite/object layer, and sound
               channels; disable Add-on Devices; reset Emulation Speed Throttle)
 8            Toggle New Graphics Engine
 9            Toggle Windowing
 0            Toggle Offset Mode
 T            While using Netplay, press to open the Chat window
- - - - - - - - - -
[Config->Speed]
 ~      Fast Forward
 P      Pause Emulation

Note: Besides these default keys, there are many other keys that you can
  configure in the GUI.


............................................................
  8.                    Save States
............................................................

** Warning: If you care about your progress in a game, remember to use
     in-game saves regularly! Do not rely solely on save states! **

When you "save a state," ZSNES creates a file that contains the values of all
the variables that change while ZSNES is emulating a game. These values are
specific to the exact moment that you saved the state. You can then load a save
state at a later time, thus returning ZSNES to the exact point in the game when
you saved the state originally. This allows you to save your progress at a point
that might not normally be possible with in-game saves, or in games that don't
have in-game saves at all.

Save states are typically not compatible between emulators, and sometimes not
even between different versions of the same emulator, often due to internal core
changes. In fact, it should be noted that save states created prior to v0.600 of
ZSNES will not work in current versions.

To remedy this problem, first load the state in any version of ZSNES from v0.600
to v1.42. Then, immediately after loading, save another state. The new state you
just created should (hopefully) load correctly in ZSNES v1.50 and higher.

Since ZSNES does not support save states created by any other SNES emulator,
you can instead use the emulator-independent SRAM (.srm) data to transfer game
progress from one emulator to another.

Each ZSNES save state is approximately 270KB in size. Special chip games may
require an additional 200KB, however.

- - - - - - - - - - - - -
 How to Use Save States
- - - - - - - - - - - - -

You can create and load save states using the GUI [GUI.txt].

It's much easier to just use the default quick keys for these features. Press F2
to save, F4 to load, and F3 to open the save state slot chooser (with graphical
preview). However, if you don't like that method, many other save and load
techniques are available.

Configure general save behavior under Config->Saves. Configure save paths under
Config->Paths.


............................................................
  9.                       Movies
............................................................

Warning: Playing back a previously recorded movie will overwrite any SRAM data
  for the current game with the SRAM data contained in the ZMV file. This means
  you should enable Do Not Save SRAM [GUI.txt] when playing around with movies!!

Note that these new features will not work with movies recorded in the old ZMV
format (movies made prior to ZSNES v1.50).

For descriptions of the Movie Options dialog, please refer to the GUI page
[GUI.txt].

- - - - - - - -
 About Movies
- - - - - - - -

The ZSNES movie format (ZMV) has been completely rewritten and is now better and
more feature-rich than before, with the most capabilities yet implemented in an
emulator. Three of the most notable new features are re-recording, dumping ZMVs
to AVI, and movie subtitles, described below.

A movie file consists of a save state, SRAM data (when applicable), and the
recorded controller data, as well as any chapters (states) that have been
inserted. It also keeps track of a few other things which are negligible with
regard to the overall filesize. Movie files should record at less than
1KB/minute (60KB/hour).

ZSNES should be able to record most actions you can do with a real SNES,
including resetting. During playback, the game will reset just as you did during
recording.

- - - - - - -
 Limitations
- - - - - - -

You cannot record games that use the Konami Justifier special input device.
Lethal Enforcers is the only game known to require this device.

You cannot record games while using Netplay.

You should be able to record ZSNES movies for all game types; however, ZMVs are
heavily dependant on save states, so any games that exhibit problems while using
save states will also exhibit problems when recording and playing movies.

For best results, you should play back movies with the same version of ZSNES
that was used to record them. Otherwise, keypresses may become desynchronized
from the emulation playback.

- - - - - - - -
 Re-recording
- - - - - - - -

You do not have to do everything perfectly the first time you record a movie.
ZSNES allows you to re-record parts of a movie, inserting the newly recorded
parts seamlessly into the previously recorded parts. You can accomplish this
in a number of ways:

  * While playing back a movie, start recording again.
  * Use save states while recording movies. Loading the states will allow you to
    re-record.
  * You can use the rewind key to go back in movies. The rewind key can be
    configured under the Config->Saves menu.

- - - - - -
 Chapters
- - - - - -

You can insert chapters into your movies, to which you can seek during movie
playback.

You are limited to 65535 (2^16 - 1) chapters created during recording, in
addition to 65535 (2^16 - 1) created during playback.

- - - - - -
 Subtitles
- - - - - -

Subtitles allow you to create a short message that will be visible on-screen
during playback of a movie.

  1. Create an empty file in your save directory (or wherever your movies are
     saved).
  2. The subtitle file must be named in the following manner:
       - Subtitle file name = ZMV file name = ROM file name.
         (Example: smw.sfc (Super Mario World ROM), smw.zmv, smw.sub).
       - If you are using movie slot 0, file extension = .sub.
       - If you are using movie slot 1 through 9, change the last letter of the
         extension to match the movie slot you are using.
         (Example: slot 1 = .su1, slot 5 = .su5, slot 9 = .su9).
  3. For each subtitle you want in the movie, add a new line to the subtitle
     file, with the following information:
       - Start Frame:Frame Duration:Message
       - For example: "10:100:Beating the Last Boss" without the quotes. In this
         example, the message "Beating the Last Boss" will appear in the tenth
         frame and stay visible for 100 frames (thus, until the 110th frame).
       - ZSNES can display only one subtitle at a time. So make sure that the
         start frame for the next subtitle is not during the duration time of
         the previous subtitle. You must also list your subtitles sequentially
         for all of them to be played (they cannot be out of order).
       - ZSNES can display a maximum of 34 characters (of a subtitle message)
         across the width of the screen.
  4. Now save your new subtitle file. Open ZSNES, play a movie, and you should
     see your subtitles appear!

- - - - - - - -
 Movie Dumping
- - - - - - - -

See the Movie Dumping section of the Advanced Usage page [Advanced.txt] for more
information.


............................................................
  10.                   IPS Patching
............................................................

IPS ("International Patching System") patches are files that are applied to an
original ROM, which change the programming of the ROM in some way. They are
primarily used to translate ROMs into another language; however, they can be
used for a variety of purposes. Visit [Romhacking.net] for more information
about translations and ROM hacks.

ZSNES has the ability to automatically "soft-patch" a ROM. This means that after
ZSNES loads a ROM into its memory, it will apply the IPS patch to the in-memory
ROM data, *not* the ROM file on your hard disk. This eliminates the need to keep
two copies of a ROM: the original, and the patched.

- - - - - - - - - - - - - - - - - - - -
 Applying a single IPS file to a ROM:
- - - - - - - - - - - - - - - - - - - -

    1. Make sure Enable Auto-Patch is checked in Config->Options.
    2. The IPS file must be either in your Saves directory or in the same
       directory as the ROM.
    3. The IPS file and the ROM file must have matching filenames. For example,
       SD3.sfc and SD3.ips. If your ROM file is compressed, the IPS file must
       match the *compressed* filename. For example, if you have the file
       SD3.sfc compressed inside the file Seiken Densetsu 3 (J).zip, the IPS
       file must be named Seiken Densetsu 3 (J).ips.
    4. If you meet the above three conditions, just load your ROM file as
       normal, and ZSNES should automatically patch the IPS file to the
       in-memory ROM. Remember, your original ROM file will *not* be changed.
    5. If you have done everything outlined in the steps above, and your game
       does not appear to be working, refer to the related FAQ [FAQ.txt].

- - - - - - - - - - - - - - - - - - - -
 Applying multiple IPS files to a ROM:
- - - - - - - - - - - - - - - - - - - -

ZSNES has the ability to apply up to eleven separate IPS files to the same ROM
file. You must give each IPS file a specific extension to tell ZSNES in what
order to apply them. Aside from changing the extension of the IPS files, just
follow the directions above.

| IPS file  |  Applied  |
| extension |   When?   |
- - - - - - - - - - - - -
     ips    |  First
     ip0    |  Second
     ip1    |  Third
     ip2    |  Fourth
     ip3    |  Fifth
     ip4    |  Sixth
     ip5    |  Seventh
     ip6    |  Eighth
     ip7    |  Ninth
     ip8    |  Tenth
     ip9    |  Eleventh

ZSNES will *not* apply non-sequentially extensioned IPS files. This means that
if you have three IPS files, named patch.ips, patch.ip0, and patch.ip6, ZSNES
will only apply the first two.

Please remember that the *order* in which you apply the IPS patches may be
important. Applying the patches in the wrong order may result in strange bugs,
or even an unplayable game. Please refer to the documentation that accompanied
your IPS file.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 Alternative method of applying IPS files to Compressed ROMs:
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

The simplest method of applying an IPS file to a compressed ROM is to simply
place the IPS file in the same archive as the ROM. The filename of the IPS file
does not have to match the compressed or uncompressed filename of the ROM.
Do *not* add more than one IPS file of a given extension to an archive. Results
can be unpredictable as to which of the same-extension IPS files will be applied
to the ROM.

If your ROM is compressed, ZSNES will first look for IPS files inside the
archive. If it finds any, it will only load IPS files from inside the archive,
and will not look anywhere else. Otherwise, ZSNES looks in the Saves directory
and the same directory as the ROM.


............................................................
  11.                   Cheat Codes
............................................................

Currently, ZSNES supports Game Genie, Pro Action Replay, and GoldFinger codes.

- - - - - - - - - - - - -
 How to use cheat codes:
- - - - - - - - - - - - -

    1. Load the ROM to which you want to apply cheats.
    2. Press ESC to toggle the GUI. Open the Add Code dialog from the Cheat
       Menu. You can enter up to 255 codes for each game.
    3. After adding your codes, press ESC until you are back to your game.

** To use a multi-line cheat code, just enter each line as a separate code! **

- - - - - - - - - - - - -
 How to use .cht files:
- - - - - - - - - - - - -

Place the .cht file into the same directory as the ROM, or into your Saves
folder. The .cht file must be named according to the normal naming rules (see
the Files section [Readme.txt] for details).

- - - - - - - - - - - - - - -
 Troubleshooting Cheat Codes
- - - - - - - - - - - - - - -

  * Try using the Fix button in the Browse Cheats dialog [GUI.txt].
  * Some cheat codes are meant to be used with different versions of the same
    game. If a cheat code doesn't work and there is one for both Game Genie and
    Pro Action Replay, try them both.
  * Remember that Game Genie codes require the - (dashes).
  * Try resetting the game. Any code for a game that mentions a term similar to
    "Start with" means that the game must be reset in order to take effect.
  * If you are having a hard time with comparative searches, make sure to delete
    the tmpchtsr.___ file in your ZSNES directory.


............................................................
  12.                      Files
............................................................

This section attempts to explain the various files that are created, loaded, or
used by ZSNES.

Most of the data files that are specific to individual ROMs are named in the
following manner:
    - The filename of the data file is the same as the filename of the ROM from
      which it was created.
    - The file extension of the data file changes, depending on the following:
        * If there are no "slots" for the data file, then it is just the
          normal file extension (srm, bmp, png, raw, cht, cmb, cfg, txt).
        * If you are using slot 0, then it is just the normal file extension
          (zst, zmv, sub, ips, spc).
        * If you are using slots 1-9, then the last letter of the file
          extension changes to match the slot number (zs1-zs9).
        * If you are using slots 10-99, then the last two letters of the file
          extension change to match the slot number (z10-z99).

[Where Created?]
|File Name| |File Extension| |File Type Name|
     |Description|
= = = = = = = = = = = = = = = = = = = = = = =
[Wherever you put them.]

  Whatever you want, or have them    smc, sfc, swc, ...    ROM (Game)
  automatically named by NSRT.
      These are common extensions for ROMs, which are computer files of the game
      data on real SNES cartridges. Read the full list of supported ROM file
      extensions in the Current Progress section [Readme.txt].

- - - - - - - - - - - - - - - - - - - - - - -
[Saves folder]

  romname     srm               Static RAM
      This is the *in-game* save file. It is automatically generated by ZSNES
      when you use the in-game save function. Some games use Static RAM as
      working RAM rather than to save a game. This format *should* be compatible
      among all emulators.

  romname     zst, zs1-zs9,     ZSNES Save State
               z10-z99, zss
      See the Save States section for more information [Readme.txt].

  romname     zmv, zm1-zm9      ZSNES Movie
      See the Movies section for more information [Readme.txt].

  romname     mzt, mz1-mz9      ZSNES Movie States
      Directories which contain the various save state data for that particular
      movie. See the Movies section for more information [Readme.txt].

  romname     sub, su1-su9      ZSNES Subtitle
      See the Subtitles sub-section of the Movies section for more information
      [Readme.txt].

  romname     cht               Cheat Data
      This file contains cheat codes that you entered using the Cheat Code
      Editor [GUI.txt]. These files are generally compatible between different
      versions of the same emulator, but not necessarily between different
      emulators.

  romname     cmb               Key Combination Data
      These files contain key combination data, created when you use the Key
      Combination Editor [GUI.txt].

  romname     inp               ZSNES Input
      These files contain input configuration for the specific game.

- - - - - - - - - - - - - - - - - - - - - - -
[Saves folder, Same folder as ROM]

  romname     ips, ip0-ip9      International Patching System
      See the IPS Patching section for more information [Readme.txt].

- - - - - - - - - - - - - - - - - - - - - - -
[Snapshots folder]

  romname_*****    bmp, png     Images
    /img*****
      These are snapshots of the game screen, created by ZSNES when you use the
      F1 Menu [GUI.txt]. The file name is appended by a number, up to 99999.

- - - - - - - - - - - - - - - - - - - - - - -
[SPCs folder]

  romname     spc, sp1-sp9,     SPC Sound
               s10-s99
      These files are created by ZSNES when you dump the SPC data of the game
      you are playing by using the F1 Menu [GUI.txt]. Note: In Windows, the .spc
      extension is sometimes used for PKCS Certificates.

  sounddmp    raw               Sound Buffer Dump
      This is a dump of sound buffer data, created when you use the appropriate
      F1 Menu option [GUI.txt].

- - - - - - - - - - - - - - - - - - - - - - -
[ZSNES folder (Win/DOS)  /  "~/.zsnes" (SDL)  /
 "~/Library/Application Support/ZSNES" (SDL - Mac OS X)]

  zsnes/zsnesw    exe (Win/DOS)   ZSNES executable
      This is the main ZSNES executable binary file. "ZSNES Folder" means the
      location of this executable.

  zsnes/zsnesw    cfg           ZSNES Configuration File
    /zsnesl
      User-editable configuration file where you can set almost any option in
      ZSNES [Advanced.txt].

  zmovie      cfg               ZSNES Movie Configuration File
      User-editable configuration file where you can change advanced movie
      dumping features [Advanced.txt].

  zinput      cfg               ZSNES Input Configuration File
      User-editable configuration file where you can change settings for
      controllers and extra SNES devices. This is the same as the files with the
      .inp extension, but is used globally.

  zfont       txt               ZSNES Font Configuration File
      This is where the appearance of text in the ZSNES GUI is configured
      [Advanced.txt].

  rominfo     txt               ROM Information
      This contains information about the ROM you most recently ran. You can
      configure this behavior under Config->Options.

  data        cmb               Key Combination Data
      This file contains key combination data, created when you use the Key
      Combination Editor for an unspecific game.


. . . . . . . . . . . . . . . .
This documentation is best viewed in a fixed-width font such as "Courier New".

Copyright (C) ZSNES Team & ZSNES Documentation Team [License.txt]
