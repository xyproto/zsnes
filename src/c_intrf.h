#ifndef C_INTRF_H
#define C_INTRF_H

#include "types.h"

void StartUp(void);

/* Initialize all Joystick stuff, load in all configuration data, parse
 * commandline data, obtain current directory (One time initialization) */
void SystemInit(void);

// Print ASCIIZ string
void PrintStr(char const*);

// Wait for a key to be pressed
char WaitForKey(void);

// returns 0 if there are no keys in the keyboard buffer, 0xFF otherwise
u1 Check_Key(void);

// Executes before starting/continuing a game
void InitPreGame(void);

// Returns 1 in videotroub if trouble occurs
void initvideo(void);

// One-time input device init
void UpdateDevices(void);

#endif
