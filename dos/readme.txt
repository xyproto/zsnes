
                                 ZSNES
                    by zsKnight, _Demo_, and Pharos
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

Somewhat Recommended System :

- 486/100 processor
- Min 14.5MB of RAM (min 12.5MB free, only can run 2MB rom images)
- VGA card
- Sound Blaster or 100% compatible
* You might need to disable Sound CPU emu for this system (go to Config
  -> Options)

Strongly Recommended System :

- Pentium processor (P133 or higher)
- 32MB of RAM (min 17.0MB free)
- SVGA card
- Sound Blaster 16 or 100% compatible

Recommended System for 65536 (16-bit) color mode :

- Fast Pentium processor (P166 or higher)
- 32MB of RAM (min 17.0MB free)
- SVGA card which supports 320x240x65536 or 640x480x65536 colors
- Sound Blaster 16 or 100% compatible

Recommended System for SFX support :

- Fast Pentium processor (P200 - P2-300 (depending on the game))
- 32MB of RAM (min 17.0MB free)
- VGA card
- Sound Blaster 16 or 100% compatible

Recommended System for SA-1 support : (Mario RPG)

- Fast Pentium processor (P200 or higher)
- 32MB of RAM (min 17.0MB free)
- VGA card
- Sound Blaster 16 or 100% compatible

- You need 17.0MB of RAM to run 48mbit(6megabytes) roms.
- An FPU is required for DSP1 emulation

For SuperFX and SA-1 emulation, 17.0 megabytes of free memory is required
to run.

---------------------------------------------------------------------------
4.) Things you should know about ZSNES
---------------------------------------------------------------------------

- You can use both keyboard and joystick for player 1 or 2 with some
  configuration adjustments.  Refer to zsnes.faq for details
- If your sidewinder support doesn't work, a quick way of getting it to
  work is to fully disable the windows driver from the control panel
- For modem mode, if you don't have a 16550A UART compatible modem
  (It will tell you when the modem initializes), chances are that both
  sides will go out of sync.
- IPX support seems to randomly lose packets in certain network
  configurations which causes both sides to go out of sync
- Several special chip emulation (SA-1) have unknown bugs to
  them
- Transparency effects are only available in 65536 color mode
- Using 320x240 resolution modes are faster than 640x480 modes.  Use
  640x480 modes only if you can't run 320x240 modes or if you want the
  added features of 640x480 modes
- To view 512 resolutions properly, use 640x480 mode.  Only a few games
    uses 512 resolution.  One way to find out is to see if a game has
    that feature is to look for text that looks squished.
- In 640x480 resolution, the image is scaled so it will look like 320x240
    resolution.  Only use 640x480 resolution if you want to use scanlines,
    interpolation, or if 320x240 doesn't work.
- SuperFX can be slow because it is an extra 10/20mhz cpu that has to be
    emulated as well as the snes emulation
- Screen Snapshot and FPS counter are available through a menu by pressing
    F1 during emulation
- Screen Snapshot currently saves as Image.PCX (256 colors) and Image.BMP
    (65536 colors).  This may change in the future.
- FPS counter currently only works when auto frame rate is on.
- To use the cheat function, be sure to have the ROM which you want to
    patch loaded already.
- If the cheat codes do not work, chances are that you are using a different
    version of the rom that was originally used to create them or the codes
    are converted incorrectly from other code formats.
- Certain video cards/monitors cannot support ModeQ (default resolution)
    If your video card/monitor doesn't support it, run ZSNES with -v 0.
    If -v 0 doesn't work, use -v 2 (vesa 2 required)
- 16 bit mode in ZSNES requires a Scitech Display Doctor (v5.3+) to provide
    high color, low resolution modes.  You can obtain this software at
    www.scitechsoft.com.  If your video card already supports lo-res,
    hi-color, then don't worry about getting this driver.
- There are still many bugs left in ZSNES so don't expect it to run all
    your favorite games.
- VSync won't run well unless you specify a frame skip (eg. -f 0)  But
    this feature is highly recommended to be used on very fast machines
    (eg. Pentium IIs)
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
- Offset per tile mode (mode 2/vertical only)
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
- DSP1 emulation (not complete)
- SA-1 emulation (not complete)

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

What will not run (or not play properly) :

- Some Super FX games such as Dirt Trax FX or Winter Gold (causes instability)
- DSP1 games such as Pilotwings
- Games with other special chips such Street Fighter Alpha 2, Star
  Ocean (S-DD1), and Far East of Eden 2 (SPC7110)
- Games which doesn't have a valid header
- Games that hit a severe bug in the 65816/PPU/SPC700/DSP routines
- Games that require special timing

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

Up,Down,Left,Right : Cursor Keys on Numerical Pad (You can also use
                     the non-numerical pad, but numerical pad is
                     strongly recommended)
A,B,X,Y            : Home, End, Insert, Delete
L,R                : Page Up, Page Down
Start, Select      : Enter, RShift

Player 2 : (You need to enable Player 2)

Up,Down,Left,Right : K, M, N, <
A,B,X,Y            : D, X, S, Z
L,R                : F, C
Start, Select      : Ctrl, Alt

To run the emulator, just type  ZSNES <romname.smc/.sfc/.fig/.1>
To run it with sound, type  ZSNES -s <romname.smc/.sfc/.fig/.1>
To run it in 16-bit mode (VESA2 w/ video card that supports 320x240x65536
        required), type  ZSNES -v 3 <romname.smc/.sfc/.fig/.1>
    or with sound, type  ZSNES -v 3 -s <romname.smc/.sfc/.fig/.1>

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

---------------------------------------------------------------------------
9.) Configuration File (ZSNES.CFG)
---------------------------------------------------------------------------

Almost everything in zsnes.cfg should now be editable through the gui

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

DSP1 is enabled automatically.  Currently, it runs mario kart and some other
    games, but it does not run pilotwings due to a lack of dsp1 functions
    that are implemented.

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
   (eg.  If your rom is SD3.SMC and your rom is SD3V05.IPS, rename SD3V05.IPS
         to SD3.IPS)
   and ZSNES will patch the rom realtime without modifying the rom file's
   contents.

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
- The Sound DSP chip still has its bugs (not many though)

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

zsknight@zsnes.com
_demo_@zsnes.com
pharos@zsnes.com

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

Unable to Initialize VESA2 :
  VBE not detected      - ZSNES Failed to detect any VBE interrupts
                          (Your video card doesn't support VESA)
  VESA not detected     - ZSNES Failed to detect any VESA extensions
                          (Your video card doesn't support VESA)
  VESA 2.0 or greater required - Your video card supports VESA, but
                          it has an old version of VESA.  You may want
                          to use SDD (www.scitechsoft.com) to upgrade
                          your vesa driver unless you have an S3 video
                          card which you can get a vesa 2 driver from
                          www.s3.com.
  VESA2 mode does not work on your video card/driver - Meaning that the
                          resolution you chose does not exist in the supported
                          resolutions of your video card.  Chose a different
                          resolution or upgrade with SDD which can sometimes
                          help increase the number of resolutions supported
  Unable to initialize video mode - A VESA 2.0 driver is found, but the video
                          mode failed to start.  Possibly an error on the
                          video card setting or a defective hardware
  Linear Frame Buffer not Detected - Meaning that your video card does not
                          support linear frame buffering which is required
                          for ZSNES' vesa 2 routines

Modem Mode :
  Modem Response Timeout - If this appears, then either your modem isn't
                          configured properly (Even though your COM port is
                          set up properly, chances are your IRQ isn't), some
                          other application is using the modem (such as an
                          internet connection), you don't have a DOS
                          compatible modem, or your baud rate isn't set up
                          to the speed of your modem (this isn't required for
                          all modems though, but for some, it's required)
  Cannot Init Driver    - This means that you have the FOSSIL DRIVER option
                          checked, but no fossil drivers are loaded
  Carrier Detected (When you're not connected) - This Probabily means that
                          you have set your COM Port/IRQ incorrectly.  This
                          also might mean that you don't have a dos
                          compatible modem or the modem is being used
                          somewhere else such as an internet account.

After Connection (Modem and IPX Mode) :
  Incompatible Version  - This means that both sides are using different
                          versions of zsnes.  Both sides must use the same
                          version in order for remote play to proceed.
  Invalid Sound Setting - This means that one side has sound disabled while
                          the other has it enabled.  Both sides have to either
                          have sound disabled or sound enabled because the
                          game timings of both settings are different.

---------------------------------------------------------------------------
16.) Credits
---------------------------------------------------------------------------

ZSNES uses NASM, DJGPP, WDOSX, and CWSDPMI (source codes & binary updates
      located at http://www.dbit.com/pub/cwsdpmi ) as the compilers and dos
      extenders.  Thanks to those who produced these fine programs!

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
  DarkForce a lot of great help!
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
  DCX for helping out fixing up some bugs!
  Andy Goth for some help on the design of the key combination engine!
  Antiriad for some testing help and for the chats!
  Yamaha of XYZZ (Scott Scriven) for his water effect code
    Yamaha's homepage: http://www.VIS.colostate.edu/~scriven/)
  All those people who helped us by either sending us docs,
    helping us, supporting us, and reporting bugs!
  Special Thanks to : Ashley, Barubary, CyberWarriorX, DCX, DooMStalK,
    Fanwen, GreenImp, Hucard, Kaiden, Stalphos Knight, Star Creator,
    TeleKawaru, Tuxedo Mask, the people in #zsnes efnet, and the regulars
    of the ZSNES message board!
  And also to all those whom we forgot!

Special Thanks to all our beta testers for being a great help!
Info on 256x256x256 scanlines mode is from the MAME source.  Thanks to
   those behind MAME!

Also, good luck to all those who are writing emulators, especially those
    who are writing snes emulators, including Snes9x, Nlke, SNEeSe, and
    SNEMul!  And also good luck to the makers of TheSE!
