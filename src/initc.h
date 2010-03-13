#ifndef INITC_H
#define INITC_H

#include <stdbool.h>
#include <stdint.h>

#include "types.h"

bool     loadfileGUI(void);
uint32_t showinfogui(void);
void     Setper2exec(void);
void     SetupROM(void);
void     clearmem(void);
void     init65816(void);
void     initsnes(void);
void     procexecloop(void);
void     zexit(void);

extern bool RTCEnable;
extern bool SDD1Enable;
extern char CSStatus[41];
extern char CSStatus2[41];
extern char CSStatus3[41];
extern char CSStatus4[41];

extern uint8_t  snesinputdefault1;
extern uint8_t  snesinputdefault2;
extern uint8_t  disableeffects;
extern uint16_t curypos;      // current y position
extern uint8_t  xp;
extern uint8_t  xe;
extern eop**    Curtableaddr; // Current table address

#endif
