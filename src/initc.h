#ifndef INITC_H
#define INITC_H

#include <stdbool.h>
#include <stdint.h>

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

#endif
