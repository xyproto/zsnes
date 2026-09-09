#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdint.h>

void startdebugger(void);

extern uint32_t numinst; // # of instructions
extern uint16_t PrevBreakPt_offset;
extern uint8_t PrevBreakPt_page;

#endif
