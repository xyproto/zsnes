#ifndef C_INTRF_H
#define C_INTRF_H

void StartUp(void);

/* Initialize all Joystick stuff, load in all configuration data, parse
 * commandline data, obtain current directory (One time initialization) */
void SystemInit(void);

// Executes before starting/continuing a game
void InitPreGame(void);

#endif
