
                                 ZSNES
                    by zsKnight, _Demo_, and pagefault
             with help from Pharos, Teuf, theoddone33 and stainless
                         http://www.zsnes.com


ZSNES is a Super Nintendo Entertainment System emulator written mostly
in assembler.

We would really like to thank the snes9x team for all their help and
also for an excellent snes emulator.  We also wish them the very best
of luck!

---------------------------------------------------------------------------
Table of Contents
---------------------------------------------------------------------------

1.) What's New
2.) Disclaimer
3.) System Requirements
4.) Things you should know about ZSNES (Updated v1.00)
5.) Current Progress
6.) Future Progress
7.) ZSNES Default Keys
8.) Extra Pop-Up Menu (F1 Menu)
9.) Configuration File (ZSNES.CFG)
10.) Cheat Codes
11.) Extra Features (SNES Mouse/Super Scope/SuperFX/DSP1/MultiTap/IPS)
12.) Bugs Section
13.) Contact Information
14.) The Debugger
15.) Error Codes
16.) Credits

---------------------------------------------------------------------------
1.) What's New
---------------------------------------------------------------------------

See WHATSNEW.TXT

---------------------------------------------------------------------------
2.) Disclaimer
---------------------------------------------------------------------------

zsKnight, _Demo_, and Pharos are not responsible for any damages caused
by the use of this software.  This software also must not be distributed
with ROMs.

---------------------------------------------------------------------------
3.) System Requirements
---------------------------------------------------------------------------

- Fast Pentium processor strongly recommended
- 32MB of RAM
- Windows 95/98/ME/2000 or compatible
- DirectX 8.0 or higher

Recommended System for SFX support :

- Fast Pentium processor (P200 - P2-300 (depending on the game))
- 32MB of RAM
- Windows 95/98/ME/2000 or compatible
- DirectX 8.0 or higher

Recommended System for SA-1 support : (Mario RPG)

- Fast Pentium processor (P200 or higher)
- 32MB of RAM
- Windows 95/98/ME/2000 or compatible
- DirectX 8.0 or higher

Requirements for netplay:

- WinSock 2.2 (Included with most Windows versions, or from
  http://www.microsoft.com/windows95/downloads/ )

Please note that while 32 MB is a recommendation for minimum RAM,
additional RAM may be beneficial, especially for 40 or 48 megabit
ROMS, or games that require decompression packs.

*An FPU is required for DSP1 emulation
*You can download directx at www.microsoft.com/directx/

---------------------------------------------------------------------------
4.) Things you should know about ZSNES
---------------------------------------------------------------------------

- You can use both keyboard and joystick for player 1 or 2 with some
  configuration adjustments.  Refer to zsnes.faq for details
- Several special chip emulation routines (SA-1) have unknown bugs to
  them
- SuperFX can be slow because it is an extra 10/20mhz cpu that has to be
    emulated as well as the snes emulation
- Screen Snapshot, Snapshot Format, and FPS counter are available through a menu by pressing
    F1 during emulation
- Screen Snapshot currently saves as Image.BMP (65536 colors). Also, a PNG
    mode is available that saves in 32 bit ARGB color, as
    <rom name> <date> <time>.png
- FPS counter currently only works when auto frame rate is on.
- To use the cheat function, be sure to have the ROM which you want to
    patch loaded already.
- If the cheat codes do not work, chances are that you are using a different
    version of the rom that was originally used to create them or the codes
    are converted incorrectly from other code formats.
- There are still many bugs left in ZSNES so don't expect it to run all
    your favorite games.
- SNES Mouse support is still missing some features such as speed settings
- Pressing the Fast Forward key is equivalent to running ZSNES using -f 9
- If ZSNES doesn't work for you, then don't use it.  Use SNES9X, NLKE,
    SNEESE, or SNEMUL instead!  In fact, even if you use ZSNES,
    use those emulators too!

---------------------------------------------------------------------------
5.) Current Progress
---------------------------------------------------------------------------

The following are implemented :
- complete 65816 instruction set
- SRAM support
- LoROM and HiROM support
- SlowROM and FastROM support
- Full DMA support
- HIRQ/VIRQ/NMI Interrupts
- Support for most snes file formats
- Multi file format support (.1,.2,.3,A.,B.,C.)
- Interleaved format support
- PAL/NTSC timing support

The following are implemented in the new graphics engine 8 bit :
- Combination of line by line and tile based graphics engine
- Graphic Modes 0,1,2,3,4,5,6,7
- 8x8, 16x16, 32x32 and 64x64 sprite support (flipped in all directions)
- 8x8 and 16x16 tiles
- 32x32,64x32,32x64,64x64 tile modes
- Full HDMA effects for wavy backgrounds, interesting mode 7 effects, etc.
- Mode 7 rotating and scaling effects
- BG priorities
- Sprite Priorities
- Add/sub of back area
- Mosaic Effects
- Offset per tile mode (mode 2/vertical only, Mode 4)
- High res 512 resolution and 448/478 vertical resolution
- Windowing effects
- High res Mode 7 (only in 640x480x256 video mode/disable Eagle/Scanlines)

The following are implemented in the old graphics engine :
- Line by line based graphics engine
- Graphic Modes 0,1,2,3,4,5,6,7
- 8x8, 16x16, 32x32 and 64x64 sprite support (flipped in all directions)
- Mosaic effects
- 8x8 and 16x16 tiles
- 32x32,64x32,32x64,64x64 tile modes
- Full HDMA effects for wavy backgrounds, interesting mode 7 effects, etc.
- Mode 7 rotating and scaling effects
- Single and Dual Windowing Routines
- BG priorities
- Sprite Priorities
- Add/sub of back area
- 16-bit graphics support
- offset per tile mode (mode 2/vertical only)
- High res 512 horizontal resolution (missing in 16x16)

Following are present in 16-bit graphics mode :

- Palette changing in the middle of a screen
- Screen Addition (full and half)
- Screen Subtraction (full)
- Fixed Color Addition/Subtraction
- Window clipping for Fixed Color

The following are implemented in the new graphics engine 16 bit :
- All of old graphics engine 16bit
- Combination of line by line and tile based graphics engine
- High resolution mode 7
- High resolution 16x16 tiles for mode 5
- full 15bit color transparencies (instead of the previous 13bit speed
  hack) for improved picture quality (MMX compatible CPUs only)

The following are present in sound :

- 16bit digital stereo sound
- SPC700 Sound CPU
- DSP Sound Processor
     - Echo Effects
     - FIR Filter
     - ADSR volume effects
     - GAIN volume effects
     - Noise effects
     - Pitch Modulation

The following are extra features emulated :

- Super NES Mouse Support
- Super NES Super Scope Support
- SuperFX support (Still has a bug or 2 left)
- MultiTap (Multiplayer 5) support (4 players on a single computer, 5 players
                                    remote (ipx/modem))
- DSP1 emulation (incomplete)
- SA-1 emulation (not complete)
- OBC1 emulation (still may have bugs)
- S-DD1 emulation (through decompression packs)
- SPC7110 emulation (also via decompression packs)

The following are the features present in ZSNES :

- Game State Save (F2=Save, F3=Select, F4=Load).
    Warning : A state file takes 260 kbytes of HD space!
- Auto frame rate to give you constant Super Nintendo speed up to 9
    frame skips
- configuration file support (ZSNES.CFG)
- 2 player support w/ Joystick and Gamepad support

The following features are missing :

- Pseudo 512 snes horizontal resolution
  (Haven't seen any game that uses them yet)
- Some modes in Offset Per Tile Mode
- Some Direct Color Modes (Haven't seen any game that uses them yet)
- True SPC7110/S-DD1 decompression

What will not run (or not play properly) :

- Some Super FX games such as Dirt Trax FX or Winter Gold (causes instability)
- Games with unknown co-processors
- Games with other special chips such Street Fighter Alpha 2, Momotarou's
  Happy Train that do not yet have decompression packs. (S-DD1/SPC7110)
- Games which doesn't have a valid header
- Games that hit a severe bug in the 65816/PPU/SPC700/DSP routines
- Games that require special timing
- Games that use functions not yet supported by the DSP-1, or that
  use other flavors of the DSP chip (ex. Top Gear 3000)

---------------------------------------------------------------------------
6.) Future Progress
---------------------------------------------------------------------------

This section is removed

---------------------------------------------------------------------------
7.) ZSNES Default Keys
---------------------------------------------------------------------------

Here are the default keys while running the emulator.  They can be changed
through the GUI except for F1 and ESC :

Disable Backgrounds 1,2,3,4  = 1,2,3,4
Disable Sprites              = 5
Panic Key (enable all)       = 6
Enable SNES Mouse/SuperScope = 7
Enable New Graphics Engine   = 8
Disable Windowing            = 9
Disable OffsetMode           = 0
Fast Forward Key             = ~
PopUp Extra Menu             = F1
Save State                   = F2
Switch State                 = F3
Load State                   = F4
Disable Sound Channel        = F5 - F12
Quit                         = ESC

Here are the default keys for the game play (unless modified through the GUI):

Player 1 :

Up,Down,Left,Right : Cursor Keys
A,B,X,Y            : X,Z,S,A
L,R                : D,C
Start, Select      : Enter, RShift

---------------------------------------------------------------------------
8.) Extra Pop-Up Menu (F1 Menu)
---------------------------------------------------------------------------

Save Snapshot - Saves a snapshot as either .PCX (8-bit color) or .BMP
                (16-bit color)

Show/Hide FPS - Shows or hides the frame per second display which appears
                on the bottom-left corner of the screen.  This can only
                be enabled in auto-frame rate mode.

Save SPC Data - Selecting this will search for the beginning of the next
                song and save the data into a .spc file which saves it
                similarly as a .srm file.  To capture a song, it is best
                recommended to initiate this feature approx 2 seconds
                before the next song starts.  This does have potential to
                fail though so don't expect it to work all the time.
                Also, this feature doesn't work in the new gfx engine.

Sound Buffer Dump - This dumps the sound buffer in zsnes and also filters
                out any unoccupied space.  The sound buffer contains
                decompressed samples which are written to when zsnes
                plays/decodes a sample from sound memory. Because of the
                way zsnes buffers the sound data, this can produce
                inaccurate results.

Snapshot/Increment Frame - Same as snapshot, but it returns to the F1 menu
                after a couple frames.  Useful for making animations.

Screen Shot Format - chooses what format to use for screen shots. Choices
                are BMP (bitmap) and PNG (Portable Network Graphic)

---------------------------------------------------------------------------
9.) Configuration File (ZSNESW.CFG)
---------------------------------------------------------------------------

Almost everything in zsnesw.cfg should now be editable through the gui
The exception is the temp folder, which is needed to use games from a
read-only medium.

---------------------------------------------------------------------------
10.) Cheat Codes
---------------------------------------------------------------------------

Currently, ZSNES supports Game Genie, Pro Action Replay and GoldFinger codes

Here are the steps to get the codes working :

1.) Load the ROM which you want to patch
2.) Exit to the GUI, enter the code of your choice in the cheat menu
3.) After that, you may have to RESET the game to get the cheat code to
      work.  Sometimes, it is not necessary.  You also may have to click
      on the FIX button.

NOTE : Some cheat codes are meant to be for different versions of the game.
       If a cheat code doesn't work and there is one for both Game Genie
         and Pro Action Replay, try them both.

---------------------------------------------------------------------------
11.) Extra Features (SNES Mouse/Super Scope/SuperFX/DSP1/MultiTap)
---------------------------------------------------------------------------

To Enable SNES Mouse, Press 7 once for 1st player and twice for 2nd player
To Enable SNES Super Scope, Press 7 three times
To disable either one, either press 6 or press 7 until you see a disable
   message onscreen.

Here are the controls for the Super Scope :

  Fire = Left Mouse button
  Cursor Mode Button = Right Mouse button
  Pause = Backspace Key on keyboard
  Enable/Disable Autofire = =/+ key on keyboard, should be located to the
                       left of the backspace key

Zsnes auto-detects the SFX emulation from the header and enables it when
   found.  Also, take note that the SFX is an additional 10Mhz(Ver1) or
   20Mhz(Ver2) chip which also has to be emulated with the snes and will
   most likely produce a very noticeable slow down in emulation if you
   don't have a fast computer.

Sometimes, the Multitap isn't compatible with some games.  If that happens,
   disable Multitap by setting Player3,4, and 5's input device to 'None' in
   the GUI

IPS patcher :
   Rename your .IPS file to the rom filename with the .IPS extension
   (eg. If your rom is SD3.SMC and your ips is SD3V05.IPS, rename SD3V05.IPS
        to SD3.IPS)
   and ZSNES will patch the rom realtime without modifying the rom file's
   contents. Zipped roms are patched according to the unzipped name.
   (eg. Seiken3.zip containing SD3.smc is patched by SD3.ips, not Seiken3.ips)
   
---------------------------------------------------------------------------
12.) Bugs Section
---------------------------------------------------------------------------

- 65816 lacks correct timing.  Although ZSNES bases the timing on a
  variable cycle/instruction, it does not deduct correct values such
  as 16bit instructions should deduct 1 more cycle than 8bit instructions.
  100% cycles of zsnes isn't 100% cycles of a snes because of that.
  This means that a lot of games either won't run or will produce horrible
  displays.  Sometimes adjusting the % of execution can fix those problems.
  This is due to inaccurate documentation used when the 65816 was written.
  There is no plans on re-writing the 65816 timing yet.
- Games sometimes tend to not display things properly because of graphic
  features that aren't implemented yet
- The Sound DSP chip still has its bugs (not many though). Most noticeably,
  no one knows the exact timing of the SPC700 chip.

---------------------------------------------------------------------------
13.) Contact Information
---------------------------------------------------------------------------

The ZSNES homepage is located at : http://www.zsnes.com

If you have any questions about zsnes and you have read ZSNES.FAQ,
README.TXT, and GUINOTES.TXT to make sure the answer isn't there.
And your question is NOT a ROM Request or asking about a newer
version, you can post your question at the zsnes www board located at :

http://www.zsnes.com/board/

Remember : This board should be mainly used for zsnes related questions.
ROM requests (or asking for games, a link to a rom site, or where to
find them) are forbidden!!!  Be sure to also read the RULES!
The rules are located at the top of the page.

If you wish to contact the authors, you may contact them through :
(Remember - No ROM requests please! and don't send any files without
 permission!)

midnight@umich.edu (Tech Support guy)
zsknight@zsnes.com (Main Coder)
_demo_@zsnes.com (Main Coder)
pagefault@users.sourceforge.net (Assistant Coder, Windows Port Developer)
pharos@zsnes.com (Assistant Coder)

Try not to send a copy of your e-mail to all of us since that will just
  waste our time.  Also, don't expect to get a reply since we are often
  busy.


---------------------------------------------------------------------------
14.) The Debugger
---------------------------------------------------------------------------

Note : The debugger is disabled when you don't enter zsnes with a -d

Here are the keys:

1 : Enable/Disable spc700 display
2 : Enable/Disable 65816 display
T : Trace (in 65816 opcodes)
B : 65816 break point
S : SPC break point
C : Clear Counter
M : 65816 modify
A : SPC modify
D : Debug Dump (SPC/DSPRam Dump Only)
W : Break at signal (Used only by the programmers)
F1 : Exit debugger and return to Game
F2 : Save State
F4 : Load State
ESC : Exit entire program

---------------------------------------------------------------------------
15.) Error Codes
---------------------------------------------------------------------------

-blank-

---------------------------------------------------------------------------
16.) Credits
---------------------------------------------------------------------------

ZSNES uses NASM, DJGPP, WDOSX, and CWSDPMI (source codes & binary updates
      located at http://www.dbit.com/pub/cwsdpmi ) as the compilers and dos
      extenders.  Thanks to those who produced these fine programs!

ZSNESW uses Visual C++ 6, NASM .98, DirectX 8, GNU Make, and UPX to compile,
      link, compress, and execute. Thanks for the work put into these programs.

Special thanks to wnelson!  Without him, ZSNES would have never existed!
Also to Y0SHi for his excellent snes docs, his help, and his excellent
        support!

Also Thanks to :
  The_Teach of snes9x for some great info and the nice chats!
  Trepalium of snes9x for some great info and help!
  Gary of Snes9x and Steve Snake of KGen for being the source of info for
      sound decompression!
  Gary of Snes9x for being the source of info for TCall/PCall and also
      for lots of help!
  MrGrim for his great support!
  Crono for info on Sound Blaster Programming, Surround Sound, Sound
    Interpolation, and other sound stuff!
  Aquis for the zsnes logo!
  Alucard for helping us with an issue in the 65816!
  Vertigo for making a compatibility list!
  EFX for being a great supporter and also giving a lot of help and stuff!
  Zophar for being a great supporter and also maintaining the mirror site!
  Chris Hickman for redesigning the ZSNES web page!
  CSoft for hosting our web page! (www.csoft.net)
  Marius Fodor for the code for VSync, Sidewinder, Gamepad Pro, and some
    optimisation info!
  Sardu for some great info and help!
  Lord Esnes for some great help!
  Robert Grubbs for the sidewinder info!
  Nerlaska for some useful info on optimising and also for some help!
  Diskdude for writing sneskart which we used for the info on cheat codes!
  DarkForce for some great help!
  Pharos for some keyboard coding help!
  Ivar and Gary of the snes9x team for all the great help which includes
    their superfx info and code, DSP1 info, info on interleave formats
    (hirom & superfx), offset per tile mode, FIR filter, some spc700
    and hdma bugs!
  WolfWings ShadowFlight for help on several issues of nasm!
  Wildfire for some help!
  A CoolMan for the algorithm for EAGLE!
  X-Sykodad and darklore for maintaining the ZSNES www board!
  Neill Corlett for some info on the .IPS format and also helping out
    a lot on improving the sound engine!
  Kreed (derek-liauw@usa.net) for the 2xSaI and Super Eagle Source Codes!
    2xSaI homepage: http://members.xoom.com/derek_liauw/index.html
  Markus Oberhumer & Laszlo Molnar for the UPX compression utility
    UPX homepage: http://cdata.tvnet.hu/~ml/upx.html
  Jean-loup Gailly, Mark Adler, and Gilles Vollant for the unzip routines!
  Kode54 for the low pass filter routines!
  Andy Goth for some help on the design of the key combination engine!
  Yamaha of XYZZ (Scott Scriven) for his water effect code
    Yamaha's homepage: http://www.VIS.colostate.edu/~scriven/)
  All those people who helped us by either sending us docs,
    helping us, supporting us, and reporting bugs!
  Special Thanks to : Ashley, Barubary, CyberWarriorX, DCX, DooMStalK,
    Fanwen, GreenImp, Hucard, Kaiden, PolestaR, Stalphos Knight, Star Creator,
    TeleKawaru, the people in #zsnes efnet, and the regulars of the
    ZSNES message board!
  And also to all those whom we forgot!

Special Thanks to all our beta testers for being a great help!
Info on 256x256x256 scanlines mode is from the MAME source.  Thanks to
   those behind MAME!

Also, good luck to all those who are writing emulators, especially those
    who are writing snes emulators, including Snes9x, Nlke, SNEeSe, and
    SNEMul!  And also good luck to the makers of TheSE!
