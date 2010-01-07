#ifndef DEBUGGER_H
#define DEBUGGER_H

void startdebugger(void);

void my_getch();

extern unsigned int   numinst; // # of instructions
extern unsigned short PrevBreakPt_offset;
extern unsigned char  PrevBreakPt_page;
extern int            my_getch_ret;

#endif
