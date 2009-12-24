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

// return non-zero if successful
u4 Init_Mouse(void);

// Returns both pressed and coordinates: YYYYYYYYXXXXXXXXBBBBBBBBBBBBBBBB
u4 Get_MouseData(void);

#endif
