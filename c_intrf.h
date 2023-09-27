#ifndef C_INTRF_H
#define C_INTRF_H

#include "types.h"

// ****************************
// Input Device Stuff
// ****************************

// Variables related to Input Device Routines:
//   pl1selk,pl1startk,pl1upk,pl1downk,pl1leftk,pl1rightk,pl1Xk,
//   pl1Ak,pl1Lk,pl1Yk,pl1Bk,pl1Rk
//     (Change 1 to 2,3,4 for other players)
//     Each of these variables contains the corresponding key pressed value
//       for the key data
//   pressed[]
//     - This is an array of pressed/released data (bytes) where the
//       corresponding key pressed value is used as the index.  The value
//       for each entry is 0 for released and 1 for pressed.  Also, when
//       writing keyboard data to this array, be sure to first check if
//       the value of the array entry is 2 or not.  If it is 2, do not write 1
//       to that array entry. (however, you can write 0 to it)
//   As an example, to access Player 1 L button press data, it is
//     done like : pressed[pl1Lk]
//   The 3 character key description of that array entry is accessed by the
//     GUI through ScanCodeListing[pl1Lk*3]

// Note: When storing the input device configuration of a dynamic input
//   device system (ie. Win9x) rather than a static system (ie. Dos), it
//   is best to store in the name of the device and relative button
//   assignments in the configuration file, then convert it to ZSNES'
//   numerical corresponding key format after reading from it. And then
//   convert it back when writing to it back.
extern u4 KeyBuffer[16];
extern u4 CurKeyPos;

/* Initialize all Joystick stuff, load in all configuration data, parse
 * commandline data, obtain current directory (One time initialization) */
void SystemInit(void);

// print character
void PrintChar(char);

// Print ASCIIZ string
void PrintStr(char const *);

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

// Input Device Names
extern char const GUIInputNames[][17];

// Total Number of Input Devices
extern u4 const NumInputDevices;

// GUI Description codes for each corresponding key pressed value
extern char const ScanCodeListing[];

#endif
