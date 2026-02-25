#ifndef C_INTRF_H
#define C_INTRF_H

/*
 * c_intrf.h — Platform abstraction interface
 *
 * This header defines the contract that every platform backend must implement.
 * Each function listed here has a platform-specific implementation:
 *
 *   Unix/SDL:   linux/c_sdlintrf.c, linux/sdllink.c, linux/audio.c
 *   Windows:    win/c_winintrf.c, win/winlink.cpp
 *
 * To port ZSNES to a new platform, provide implementations for all functions
 * declared here.
 */

#include "types.h"

/* ── Lifecycle ── */

/* One-time program startup (earliest init, before config loading). */
void StartUp(void);

/* Initialize joystick subsystem, load configuration, parse command line,
 * obtain current directory.  Called once at program start. */
void SystemInit(void);

/* Executes before starting/continuing a game (allocate video surfaces,
 * open audio device, etc.). */
void InitPreGame(void);

/* Executes after InitPreGame; may be called multiple times after a single
 * InitPreGame (e.g. when video mode changes mid-game). */
void SetupPreGame(void);

/* Called after a game session ends (free video/audio resources). */
void DeInitPostGame(void);

/* GUI subsystem init (create window, set up GUI rendering context). */
void GUIInit(void);

/* GUI subsystem teardown. */
void GUIDeInit(void);

/* ── Console I/O (used during init and error reporting) ── */

/* Print a single character to the console. */
void PrintChar(char);

/* Print a null-terminated string to the console. */
void PrintStr(char const*);

/* Block until a key is pressed; return the key character. */
char WaitForKey(void);

/* Return 0 if no keys in keyboard buffer, 0xFF otherwise. */
u1 Check_Key(void);

/* Block if no keys in buffer, then return key.  For extended keys,
 * return 0 first, then the extended key code on the next call. */
char Get_Key(void);

/* Delay for n / 65536 of a second. */
void delay(u4 n);

/* ── Video ── */

/* Initialize the video subsystem.  Sets videotroub=1 on failure. */
void initvideo(void);

/* Tear down the video subsystem. */
void deinitvideo(void);

/* In-game screen render: present the current frame to the display,
 * handling triple buffering and vsync as appropriate. */
void DrawScreen(void);

/* GUI screen render: blit the GUI framebuffer (vidbuffer) to the display. */
void vidpastecopyscr(void);

/* ── Input ── */

/* One-time input device enumeration and initialization. */
void UpdateDevices(void);

/* Poll all input devices and update JoyAOrig/JoyBOrig state. */
void JoyRead(void);

/* Configure key bindings for the given input device and player number. */
void SetInputDevice(u1 device, u1 player);

/* ── Mouse ── */

/* Initialize mouse; return non-zero on success. */
u4 Init_Mouse(void);

/* Return mouse state: YYYYYYYYXXXXXXXXBBBBBBBBBBBBBBBB (Y pos, X pos, buttons). */
u4 Get_MouseData(void);

/* Set horizontal mouse boundaries. */
void Set_MouseXMax(u4 min, u4 max);

/* Set vertical mouse boundaries. */
void Set_MouseYMax(u4 min, u4 max);

/* Set absolute mouse position. */
void Set_MousePosition(u4 x, u4 y);

/* Return relative mouse displacement: YYYYYYYYYYYYYYYYXXXXXXXXXXXXXXXX. */
u4 Get_MousePositionDisplacement(void);

/* Confine mouse to the emulator window. */
void MouseWindow(void);

/* ── Audio ── */

/* Stop audio playback. */
void StopSound(void);

/* Start audio playback. */
void StartSound(void);

/* Call the timer update function (drives 60 Hz frame pacing). */
void Check60hz(void);

/* ── Platform-provided data tables ── */

/* Video mode names (fixed-width strings for GUI display). */
extern char const GUIVideoModeNames[][18];

/* Total number of video modes available. */
extern u4 const NumVideoModes;

extern u1 GUIHQ2X[];   /* HQ2x filter enable per video mode */
extern u1 GUIM7VID[];  /* Hires Mode 7 enable per video mode */
extern u1 GUINTVID[];  /* NTSC filter enable per video mode */
extern u1 GUIBIFIL[];  /* Bilinear filter enable per video mode */
extern u1 GUIDSIZE[];  /* Double-size modes */
extern u1 GUIHQ3X[];   /* HQ3x filter enable per video mode */
extern u1 GUIHQ4X[];   /* HQ4x filter enable per video mode */
extern u1 GUIKEEP43[]; /* Keep 4:3 aspect ratio per video mode */
extern u1 GUIWFVID[];  /* Fullscreen flag per video mode */

#ifdef __UNIXSDL__
extern u1 GUIRESIZE[]; /* SDL resizable window per video mode */
#endif

#ifdef __WIN32__
extern u1 GUIDSMODE[]; /* Windows direct-stretch modes */
extern u1 GUISMODE[];  /* Windows stretched modes */
#else
extern u1 GUII2VID[];  /* Interpolation per video mode */
#endif

/* Input device names (fixed-width strings for GUI display). */
extern char const GUIInputNames[][17];

/* Total number of input devices available. */
extern u4 const NumInputDevices;

/* GUI description codes mapping key values to display strings. */
extern char const ScanCodeListing[];

#endif
