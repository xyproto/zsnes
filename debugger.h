#ifndef DEBUGGER_H
#define DEBUGGER_H

void startdebugger(void);

extern unsigned int numinst; // # of instructions
extern unsigned short PrevBreakPt_offset;
extern unsigned char PrevBreakPt_page;

#endif
