#ifndef C_INTRF_H
#define C_INTRF_H

void StartUp(void);

/* Initialize all Joystick stuff, load in all configuration data, parse
 * commandline data, obtain current directory (One time initialization) */
void SystemInit(void);

// Wait for a key to be pressed
char WaitForKey(void);

// Executes before starting/continuing a game
void InitPreGame(void);

// Returns 1 in videotroub if trouble occurs
void initvideo(void);

// One-time input device init
void UpdateDevices(void);

#endif
