#ifndef INITC_H
#define INITC_H

#include <stdbool.h>
#include <stdint.h>

#include "types.h"

bool loadfileGUI(void);
uint32_t showinfogui(void);
void Setper2exec(void);
void SetupROM(void);
void clearmem(void);
void init65816(void);
void initsnes(void);
void procexecloop(void);
void zexit(void);

extern bool RTCEnable;
extern bool SDD1Enable;
extern char CSStatus[41];
extern char CSStatus2[41];
extern char CSStatus3[41];
extern char CSStatus4[41];
extern u1 ComboHeader[23];
extern u1 sramsavedis;

extern uint8_t snesinputdefault1;
extern uint8_t snesinputdefault2;
extern uint8_t disableeffects;
extern uint16_t curypos; // current y position
extern uint8_t xp;
extern uint8_t xe;
extern eop** Curtableaddr; // Current table address

extern u1 SFXCounter;
extern u1 xirqb; // which bank the irqs start at
extern u2 stackand; // value to and stack to keep it from going to the wrong area
extern u2 stackor; // value to or stack to keep it from going to the wrong area
extern s4 echobuf[22500];

#endif
