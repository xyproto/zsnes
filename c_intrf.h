#ifndef C_INTRF_H
#define C_INTRF_H

#include "types.h"

void StartUp(void);

/* Initialize all Joystick stuff, load in all configuration data, parse
 * commandline data, obtain current directory (One time initialization) */
void SystemInit(void);

// print character
void PrintChar(char);

// Print ASCIIZ string
void PrintStr(char const*);

// Wait for a key to be pressed
char WaitForKey(void);

// returns 0 if there are no keys in the keyboard buffer, 0xFF otherwise
u1 Check_Key(void);

/* Wait if there are no keys in buffer, then return key.  For extended keys,
 * return a 0, then the extended key afterwards. */
char Get_Key(void);

// Delay for n / 65536 of a second
void delay(u4 n);

// Executes before starting/continuing a game
void InitPreGame(void);

/* Executes after pre-game init, can execute multiple times after a single
 * InitPreGame */
void SetupPreGame(void);

// Called after game is ended
void DeInitPostGame(void);

void GUIInit(void);

void GUIDeInit(void);

// Returns 1 in videotroub if trouble occurs
void initvideo(void);

void deinitvideo(void);

// In-game screen render w/ triple buffer check
void DrawScreen(void);

// GUI screen render
void vidpastecopyscr(void);

// One-time input device init
void UpdateDevices(void);

void JoyRead(void);

// Sets keys according to input device selected
void SetInputDevice(u1 device, u1 player);

// return non-zero if successful
u4 Init_Mouse(void);

// Returns both pressed and coordinates: YYYYYYYYXXXXXXXXBBBBBBBBBBBBBBBB
u4 Get_MouseData(void);

// Sets the X boundaries
void Set_MouseXMax(u4 min, u4 max);

// Sets the Y boundaries
void Set_MouseYMax(u4 min, u4 max);

// Sets Mouse Position
void Set_MousePosition(u4 x, u4 y);

// returns x,y displacement in pixel: YYYYYYYYYYYYYYYYXXXXXXXXXXXXXXXX
u4 Get_MousePositionDisplacement(void);

void MouseWindow(void);

void StopSound(void);

void StartSound(void);

// Call the timer update function here
void Check60hz(void);

/* GUI Video Mode Names - Make sure that all names are of the same length and
 * end with a NULL terminator */
extern char const GUIVideoModeNames[][18];

// Total Number of Video Modes
extern u4 const NumVideoModes;

extern u1 GUIHQ2X[]; // Hq2x Filter
extern u1 GUIM7VID[]; // Hires Mode 7
extern u1 GUINTVID[]; // NTSC Filter

#ifdef __MSDOS__
extern u1 GUI16VID[]; // 16-bit mode
extern u1 GUI2xVID[]; // 2xSaI/Super Engines
extern u1 GUIEAVID[]; // DOS Eagle
extern u1 GUIHSVID[]; // Half/Quarter Scanlines
extern u1 GUISLVID[]; // Scanlines
extern u1 GUISSVID[]; // DOS Smallscreen
extern u1 GUITBVID[]; // DOS Triple Buffering
extern u1 GUIWSVID[]; // DOS Widescreen
#else
extern u1 GUIBIFIL[]; // Bilinear Filter
extern u1 GUIDSIZE[]; // D Modes
extern u1 GUIHQ3X[]; // Hq3x Filter
extern u1 GUIHQ4X[]; // Hq4x Filter
extern u1 GUIKEEP43[]; // Keep 4:3 Ratio
extern u1 GUIWFVID[]; // Fullscreen
#endif

#ifdef __UNIXSDL__
extern u1 GUIRESIZE[]; // SDL Resizable
#endif

#ifdef __WIN32__
extern u1 GUIDSMODE[]; // Win D-Stretched Modes
extern u1 GUISMODE[]; // Win Stretched Modes
#else
extern u1 GUII2VID[]; // Interpolation
#endif

// Input Device Names
extern char const GUIInputNames[][17];

// Total Number of Input Devices
extern u4 const NumInputDevices;

// GUI Description codes for each corresponding key pressed value
extern char const ScanCodeListing[];

#endif
