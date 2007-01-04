/*
Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <string.h>
#include <ctype.h>
#ifndef NCURSES
#include <curses.h>
#else
#include <ncurses.h>
#endif
#include "zpath.h"

#ifdef __MSDOS__
#include <dpmi.h>
#endif // __MSDOS__

#include "asm_call.h"

// All of these should be in headers, people!

extern unsigned char oamram[1024], SPCRAM[65472], DSPMem[256];

extern unsigned char curblank;
extern unsigned char curcyc;
extern unsigned char curypos;
extern unsigned char CurrentCPU;

extern unsigned char soundon;
extern unsigned int  cycpbl;

extern unsigned short xpc, xa, xx, xy, xs, xd;
extern unsigned char  xpb, xdb, xp, xe;

extern void *snesmmap[256];
extern void *snesmap2[256];
extern char dmadata[];

extern unsigned char debuggeron;

extern void (*memtabler8[256])();


// SPC stuff

extern unsigned char *spcPCRam;
extern unsigned char spcA, spcX, spcY, spcS, spcNZ, spcP;


// these really shouldn't be written in ASM... (they are in debugasm.asm)
extern unsigned char memtabler8_wrapper(unsigned char, unsigned short);
extern          void memtablew8_wrapper(unsigned char, unsigned short, unsigned char);
extern void breakops_wrapper(void);

extern void regaccessbankr8();
extern void start65816();
extern void endprog();

// should be in "zstate.h"
void debugloadstate();
void statesaver();

char *ocname;
unsigned char addrmode[];
char *spcnametab[];
char *AddressTable[];
unsigned char ArgumentTable[];


/*
unsigned short debugh  = 0; // debug head
unsigned short debugt  = 0; // debug tail
unsigned short debugv  = 0; // debug view
*/
unsigned char  debugds = 0; // debug disable (bit 0 = 65816, bit 1 = SPC)
unsigned int   numinst = 0; // # of instructions

unsigned char wx = 0, wy = 0, wx2 = 0, wy2 = 0;
unsigned char execut = 0;
unsigned char debstop = 0, debstop2 = 0, debstop3 = 0, debstop4 = 0;

char debugsa1 = 0;
char skipdebugsa1 = 1;

#define CP(n) (A_BOLD | COLOR_PAIR(n))

enum color_pair {
    cp_white = 1, cp_magenta, cp_red, cp_cyan, cp_green, cp_yellow,
    cp_white_on_blue,
};

WINDOW *debugwin;

// can't get this to work right...
//#define CHECK (COLOR_PAIR(cp_white) | A_DIM | ACS_CKBOARD)
#define CHECK (CP(cp_white) | ' ')

void debugloop();

void startdisplay();
void nextopcode();
void cleardisplay();
void nextspcopcode();
void SaveOAMRamLog();
void debugdump();
void out65816();
void execnextop();

void traceops(unsigned count);
void SPCbreakops(unsigned short addr);

unsigned char *findop();
unsigned char *findoppage();

void startdebugger() {
    static int firsttime = 1;

    curblank = 0x40;
    debuggeron = 1;

#ifdef __MSDOS__
    __dpmi_regs regs;
    regs.x.ax = 0x0003;
    __dpmi_int(0x10, &regs);
#endif // __MSDOS__

    if (firsttime) {
    initscr(); cbreak(); noecho();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);

    /* set up colors */
    start_color();
    init_pair(cp_white,         COLOR_WHITE,   COLOR_BLACK);
    init_pair(cp_magenta,       COLOR_MAGENTA, COLOR_BLACK);
    init_pair(cp_red,           COLOR_RED,     COLOR_BLACK);
    init_pair(cp_cyan,          COLOR_CYAN,    COLOR_BLACK);
    init_pair(cp_green,         COLOR_GREEN,   COLOR_BLACK);
    init_pair(cp_yellow,        COLOR_YELLOW,  COLOR_BLACK);
    init_pair(cp_white_on_blue, COLOR_WHITE,   COLOR_BLUE);
    }

    execut = 0;

    if (firsttime) {
    startdisplay();

    debugwin = newwin(20, 77, 2, 1);

    wbkgd(debugwin, CP(cp_white_on_blue) | ' ');
    // wattrset(debugwin, CP(cp_white_on_blue));

    scrollok(debugwin, TRUE);
    idlok(debugwin, TRUE);

    firsttime = 0;
    } else {
        touchwin(stdscr);
        touchwin(debugwin);
        refresh();
        wrefresh(debugwin);
    }

    debugloop();
    cleardisplay();

    // "pushad / call LastLog / ... / popad" elided
    SaveOAMRamLog();


    if (execut == 1) {
        start65816(); return;
    }
    endprog(); return;
}

// Called from ASM

int my_getch_ret;
void my_getch() {
    my_getch_ret = getch();
}


WINDOW *openwindow(int nlines, int ncols, int begin_y, int begin_x,
           char *message) {
    WINDOW *w = newwin(nlines, ncols, begin_y, begin_x);
    wbkgd(w, CP(cp_white_on_blue|' '));
    // wattrset(w, CP(cp_white_on_blue));

    mvwprintw(w, 1, 1, "%s", message);
    wclrtoeol(w);
    box(w, 0, 0);

    return w;
}

void closewindow(WINDOW *w) {
    delwin(w);
    touchwin(debugwin);
    wrefresh(debugwin);
}

//*******************************************************
// Debug Loop
//*******************************************************

unsigned short PrevBreakPt_offset;
unsigned char  PrevBreakPt_page;

void debugloop() {
    int key;
  a:
    if (!(debugds & 2))
        nextopcode();
    if (!(debugds & 1))
        nextspcopcode();

  b:
    // redrawing the display is always a good idea
    refresh();
    wrefresh(debugwin);

   key = getch();
   if (key >= 0 && key < 256)
       key = toupper(key);

   switch (key) {
   case KEY_F(1): // run
       execut = 1;
       return;

   case KEY_F(2): // debugsavestate
       statesaver();
       goto b;

   case KEY_F(4): // debugloadstate
       debugloadstate();
       goto a;

   case 27:       // exit
       return;

   case '\n':     // step
       goto e;


   /* Ported this but couldn't bring myself to commit it.
      pagefault said to remove it.
   case '-':      // skip opcode
   */

   case 'C':      // clear
       numinst = 0;
       goto a;

   case 'M':      // modify
   {
       WINDOW *w;
       unsigned addr, value, n;

       w = openwindow(7, 33, 11, 24, "    Enter Address : ");
       mvwaddstr(w, 3, 1,            "    Previous Value: ");
       mvwaddstr(w, 5, 1,            "    Enter Value   : ");

       wrefresh(w);

       echo();
       n = mvwscanw(w, 1, 21, "%x", &addr);
       noecho();

       if (n == 1) {
       mvwprintw(w, 3, 21, "%02x", memtabler8_wrapper(addr >> 16, addr));
       wrefresh(w);

       echo();
       n = mvwscanw(w, 5, 21, "%x", &value);
       noecho();

       if (n == 1) {
          memtablew8_wrapper(addr >> 16, addr, value);
       }}

       closewindow(w);
       goto b;
   }

   case 'B':      // breakpoint
   {
       WINDOW *w = openwindow(3, 33, 11, 24, "    Enter Address : ");
       unsigned addr, n;

       wrefresh(w);

       echo();
       n = wscanw(w, "%x", &addr);
       noecho();

       closewindow(w);

       if (n == 1) {
          w = openwindow(3, 52, 11, 14,
              "   Locating Breakpoint ... Press ESC to stop.    ");
          wrefresh(w);
          nodelay(stdscr, 1);

          PrevBreakPt_page = addr >> 16;
          PrevBreakPt_offset = addr;
          breakops_wrapper();

          nodelay(stdscr, 0);
          closewindow(w);

          goto a;
       }

       goto b;
   }

   case 'R': // repeat breakpoint
       breakops_wrapper();
       goto a;

   case 'S': // SPC breakpoint
   {
       WINDOW *w;
       unsigned addr, n;

       w = openwindow(3, 33, 11, 24, "     Enter Address : ");
       wrefresh(w);

       echo();
       n = mvwscanw(w, 1, 22, "%x", &addr);
       noecho();

       closewindow(w);

       if (n == 1) {
          SPCbreakops(addr);
          goto a;
       }
       goto b;
   }

   case 'A': // SPC modify
   {
       WINDOW *w;
       unsigned addr, value, n;

       w = openwindow(7, 33, 11, 24, "     Enter Address : ");
       mvwaddstr(w, 3, 1,            "     Previous Value: ");
       mvwaddstr(w, 5, 1,            "     Enter Value   : ");

       wrefresh(w);

       echo();
       n = mvwscanw(w, 1, 22, "%x", &addr);
       noecho();

       addr &= 0xFFFF;

       if (n == 1) {
       mvwprintw(w, 3, 22, "%02x", SPCRAM[addr]);
       wrefresh(w);

       echo();
       n = mvwscanw(w, 5, 22, "%x", &value);
       noecho();

       if (n == 1) {
          SPCRAM[addr] = value;
       }}

       closewindow(w);
       goto b;
   }

   case 'T': // trace
   {
       WINDOW *w;
       unsigned n, instrs;

       w = openwindow(3,52,11,14, "   Enter # of Instructions to Trace : ");
       wrefresh(w);

       echo();
       n = wscanw(w, "%d", &instrs);
       noecho();

       closewindow(w);
       if (n == 1) {
          traceops(instrs);
          goto a;
       }

       goto b;
   }

   case 'D': // debug dump
       debugdump();
       goto b;

   case 'W': // break at signal (breakatsign)
   {
       WINDOW *w;

       w = openwindow(3,52,11,14,
              "   Waiting for Signal .... Press ESC to stop.");
       wrefresh(w);

       debstop3 = 0;
       nodelay(w, TRUE);
       do {
          asm_call(execnextop);
       } while ( (! ((++numinst % 256) && (wgetch(w) == 27)))
               && (debstop3 != 1) );
       debstop3 = 0;
       nodelay(w, FALSE);

       closewindow(w);
       goto a;
   }

   case 'L': // break at signal & log (breakatsignlog)
   {
       FILE *fp;
       WINDOW *w, *real_debugwin;

       w = openwindow(3,52,11,14,
              "   Waiting for Signal .... Press ESC to stop.");
       wrefresh(w);

       // Open output file
       fp = fopen_dir(ZStartPath, "debug.log","w");

       real_debugwin = debugwin;
       debugwin = newpad(2, 77);
       scrollok(debugwin, TRUE);

       debstop3 = 0;
       nodelay(w, TRUE);
       do {
          char buf[78];
          // log instruction
          move(0,0);
          out65816();

          mvwinnstr(debugwin, 0, 0, buf, 77);
          buf[77] = 0;
          fprintf(fp, "%s\n", buf);
          fflush(fp);

          asm_call(execnextop);
       } while ( (! ((++numinst % 256) && (wgetch(w) == 27)))
                && (debstop3 != 1) );
       debstop3 = 0;
       nodelay(w, FALSE);

       fclose(fp);
       delwin(debugwin);
       debugwin = real_debugwin;

       closewindow(w);
       goto a;
   }


   case '1': // toggle SPC
       debugds ^= 1;
       break;

   case '2': // toggle 65816
       debugds ^= 2;
       break;

   default:
       wprintw(debugwin, "Unknown key code: %d\n", key);
       goto b;
   }

  e:
   skipdebugsa1 = 0;
   asm_call(execnextop);
   skipdebugsa1 = 1;
   if (soundon && (debugds & 2) && (cycpbl >= 55))
       goto e;
   goto a;

}

//*******************************************************
// BreakatSignC               Breaks whenever sndwrit = 1
//*******************************************************

unsigned char sndwrit;

/* void breakatsignc() {} */


//*******************************************************
// BreakOps                          Breaks at Breakpoint
//*******************************************************

/* in ASM still, but not identical to other version
void breakops(unsigned char page, unsigned short offset) {

}
*/

void traceops(unsigned count) {
    WINDOW *w;

    w = openwindow(3,52,11,14, "     Tracing.  Press ESC to stop.");
    wrefresh(w);

    nodelay(w, TRUE);
    while (count-- && (wgetch(w) != 27)) {
        asm_call(execnextop);
    }

    closewindow(w);
}

void SPCbreakops(unsigned short addr) {
    WINDOW *w;
    unsigned char *breakarea;

    breakarea = SPCRAM+addr;

    w = openwindow(3,52,11,14, "Locating Breakpoint ... Press ESC to stop.");
    wrefresh(w);

    nodelay(w, TRUE);
    do {
        asm_call(execnextop);
    } while ((!((++numinst % 256)
         && (wgetch(w) == 27)))
         && (spcPCRam != breakarea));
    nodelay(w, FALSE);

    closewindow(w);
}


void printinfo(char *s) {
  while (s[0]) {
    if (s[0] == '@') {
      int colors[] = {
        0, 0, cp_green, cp_cyan,
        cp_red, cp_magenta, cp_yellow, cp_white
      };
      attrset(COLOR_PAIR(colors[s[1]-'0']));
      s += 2;
    } else {
      addch(s[0]);
      s += 1;
    }
  }
}

/* Won't port too well - stuck it in debugasm.asm for now */
/* void execnextop() { */
/*     char *page = findoppage(); */
/*     initaddrl = page; */
/*     char *address = page+xpc; */
/* } */

//*******************************************************
// Start Display
//*******************************************************
void startdisplay() {
    int i;

    // Clear the screen
    bkgd(CP(cp_white) | ' ');
    clear();

    // Draw to screen

    // ASM for some reason puts the same thing in the upper left corner again?

    move(1, 0); attrset(CP(cp_white_on_blue));
    addch(CurrentCPU+'0');
    for (i = 15; i; i--)
        addch(ACS_HLINE);
    printw(" CC:    Y:    ");
    for (i = 19; i; i--)
        addch(ACS_HLINE);
    addch(' ');
    for (i = 11; i; i--)
        addch(' ');
    addch(' ');
    for (i = 16; i; i--)
        addch(ACS_HLINE);
    addch(ACS_URCORNER);

    for (i = 2; i < 22; i++) {
        mvaddch(i, 0, ACS_VLINE);
        hline(' ', 77);
        mvaddch(i, 78, ACS_VLINE);
        mvaddch(i, 79, CHECK);
    }

    mvaddch(22, 0, ACS_LLCORNER);
    for(i = 77; i; i--)
        addch(ACS_HLINE);
    mvaddch(22, 78, ACS_LRCORNER);
    mvaddch(22, 79, CHECK);

    move(23, 1);
    for(i = 79; i; i--)
        addch(CHECK);

    // Print debugger information

    move(0, 2); attrset(CP(cp_white));
    printinfo("- @5Z@4S@3N@2E@6S@7 debugger -");

    move(1, 4); attrset(CP(cp_white_on_blue));
    printinfo(" 65816 ");

    // HACK ALERT! this should really be on the bottom line, but
    // xterms default to being one line shorter than 80x25, so this
    // won't be on the bottom line on DOS!
    // Also, we are printing on top of the (currently invisible) drop shadow!"
    move(23, 0);
    printinfo("@4(@6T@4)@7race for  @4(@6B@4)@7reakpoint  "
          "@4(@6Enter@4)@7 Next  "
          "@4(@6M@4)@7odify  @4(@6F9@4)@7 Signal  @4(@6F1@4)@7 Run");

    // ...
    move(0, 0);
    refresh();
}


//*******************************************************
// Next Opcode              Writes the next opcode & regs
//*******************************************************
// 008000 STZ $123456,x A:0000 X:0000 Y:0000 S:01FF DB:00 D:0000 P:33 E+

/*
void addtail() {
    debugt++;
    if (debugt == 100)
    debugt = 0;
    if (debugt == debugh)
    debugh++;
    if (debugh == 100)
    debugh = 0;
}
*/


// I'm going to have to completely rip out byuu's effective address
// stuff, it is just plain *WRONG*, besides being unsafe...

// For next time, http://www.zophar.net/tech/files/65c816.txt seems
// like a good reference for effective address calculation; also, use
// 24-bit addresses for all calculations (so completely rip out
// memtabler8_wrapper, too). Also, preferably read memory in a
// non-destructive way.

// This whole instr[1] thing probably isn't quite right either, but it
// seems unlikely that instructions would be stored discontiguously
// than that data would span 64kb boundaries.

void out65816_addrmode (unsigned char *instr) {
    char *padding = "";

    #define GETXB() ((ocname[4*instr[0]] != 'J') ? xdb : xpb)

    #define INDEX_RIGHT(addr, index)                         \
            ((xp & 0x10)                                     \
         ? (((addr) & ~0xff)   | (((addr) + (index)) & 0xff))  \
         : (((addr) & ~0xffff) | (((addr) + (index)) & 0xffff)))


    // each mode must output 19 characters
    switch (addrmode[instr[0]]) {

    case 0:
    case 6:
    case 21:
        // nothing to show

        wprintw(debugwin, "%19s", padding);
        break;

    case 1:     // #$12,#$1234 (M-flag)
        wprintw(debugwin, "#$");
        if (xp != 0x20) {
            wprintw(debugwin, "%02x", instr[1]);
            wprintw(debugwin, "%15s", padding);
        } else {
            wprintw(debugwin, "%04x", *(unsigned short *)(instr+1));
            wprintw(debugwin, "%13s", padding);
        }
        break;

    case 2:     // $1234 : db+$1234
        wprintw(debugwin, "$%04x", *(unsigned short *)(instr+1));
        wprintw(debugwin, "%5s[%02x%04x] ", padding, GETXB(),
                                    *(unsigned short *)(instr+1));
        break;

    case 3:     // $123456
        wprintw(debugwin, "$%02x%04x", instr[3], *(unsigned short*)(instr+1));
        wprintw(debugwin, "%12s", padding);
        break;

    case 4:     // $12 : $12+d
        wprintw(debugwin, "$%02x%7s[%02x%04x] ", instr[1], padding, 0,
                                        instr[1]+xd);
        break;

    case 5:     // A
        wprintw(debugwin, "A%18s", padding);
        break;

    case 7:     // ($12),y
    {
        wprintw(debugwin, "($%02x),Y   ", instr[1]);
        wprintw(debugwin, "[nnnnnn] ");
        break;
    }

    case 8:     // [$12],y
    {
        unsigned short addr;
        unsigned int t;

        wprintw(debugwin, "[$%02x],Y   ", instr[1]);

        addr = instr[1] + xd;
        t = memtabler8_wrapper(0, addr);
        t |= memtabler8_wrapper(0, addr+1) << 8;
        t |= memtabler8_wrapper(0, addr+2) << 16;
        t = INDEX_RIGHT(t, xy);
        wprintw(debugwin, "[%06x] ", t);

        break;
    }

    case 9:     // ($12,x)
    {
        wprintw(debugwin, "($%02x,X)   ", instr[1]);
        wprintw(debugwin, "[nnnnnn] ");
        break;
    }

    case 10:    // $12,x : $12+d+x
    {
        wprintw(debugwin, "$%02x,X%5s", instr[1], padding);
        wprintw(debugwin, "[%06x] ", INDEX_RIGHT(instr[1] + xd, xx));
        break;
    }

    case 11:    // $12,y
    {
        wprintw(debugwin, "$%02x,Y%5s", instr[1], padding);
        wprintw(debugwin, "[%06x] ", INDEX_RIGHT(instr[1] + xd, xy));
        break;
    }

    case 12:    // $1234,x : dbr+$1234+x
    {
        unsigned int t = instr[1] | (instr[2] << 8);
        wprintw(debugwin, "$%04x,X   ", t);
        t = INDEX_RIGHT(t, xx);
        wprintw(debugwin, "[%02x%04x] ", xdb, t);

        break;
    }

    case 13:    // $1234,y : dbr+$1234+y
    {
        unsigned int t = instr[1] | (instr[2] << 8);
        wprintw(debugwin, "$%04x,Y   ", t);
        t = INDEX_RIGHT(t, xy);
        wprintw(debugwin, "[%02x%04x] ", xdb, t);

        break;
    }

    case 14:    // $123456,x : $123456+x
    {
        unsigned int t = instr[1] | (instr[2] << 8) | (instr[3] << 16);
        wprintw(debugwin, "$%06x,X ", t);
        t = INDEX_RIGHT(t, xx);
        wprintw(debugwin, "[%06x] ", t);

        break;
    }

    case 15:    // +-$12 / $1234
    {
        signed char c = instr[1];
        unsigned short t = c + xpc + 2;

        wprintw(debugwin, "$%04x%4s [%02x%04x] ", t, padding, xpb, t);

        break;
    }

    case 16:    // +-$1234 / $1234
    {
        unsigned short s = instr[1] | (instr[2] << 8);
        unsigned short t = s + xpc + 3;

        wprintw(debugwin, "$%04x%4s [%02x%04x] ", t, padding, xpb, t);

        break;
    }

    case 17:    // ($1234)
    {
        wprintw(debugwin, "($%04x)   ", instr[1]);
        wprintw(debugwin, "[nnnnnn] ");
        break;
    }

    case 18:    // ($12)
    {
        unsigned short addr1, addr2;
        wprintw(debugwin, "($%02x)%5s", instr[1], padding);

        addr1 = instr[1] + xd;

        addr2  = memtabler8_wrapper(00, addr1);
        addr2 |= memtabler8_wrapper(00, addr1+1) << 8;

        wprintw(debugwin, "[%02x%04x] ", xdb, addr2);

        break;
    }

    case 19:    // [$12]
    {
        // unsigned short addr1;
        // unsigned int   addr2;

        wprintw(debugwin, "[$%02x]%5s", instr[1], padding);

        /*
        addr1 = instr[1] + xd;

        addr2  = memtabler8_wrapper(0, addr1);
        addr2 |= memtabler8_wrapper(
        */

        wprintw(debugwin, "[nnnnnn] ");

        break;
    }

    case 20:    // ($1234,x)
    {
        unsigned short cx = *(unsigned short*)(instr+1);
        unsigned short x;

        wprintw(debugwin, "($%04x,X) [%02x", cx, xpb);
        if (xp & 0x10)
            cx = (cx & 0xFF00) | ((cx + xx) & 0xFF);
        else
            cx += xx;
        // .out20n
        x = memtabler8_wrapper(xpb, cx);
        x += memtabler8_wrapper(xpb, cx+1) << 8;
        wprintw(debugwin, "%04x] ", x);

        break;
    }

    case 22:    // d,s
        wprintw(debugwin, "$%02x,S%5s", instr[1], padding);
        wprintw(debugwin, "[nnnnnn] ");
        break;

    case 23:    // (d,s),y
        wprintw(debugwin, "($%02x,S),Y ", instr[1]);
        wprintw(debugwin, "[nnnnnn] ");
        break;

    case 24:    // xyc - $1234
        wprintw(debugwin, "$%02x%02x%14s", instr[2], instr[1], padding);
        break;

    case 25:    // #$12 (Flag Operations)
        wprintw(debugwin, "#$%02x%15s", instr[1], padding);
        break;

    case 26:    // #$12,#$1234 (X-flag)
        if (xp & 0x10) {
            wprintw(debugwin, "#$%02x%15s", instr[1], padding);
        } else {
            wprintw(debugwin, "#$%04x%13s",
                   *(unsigned short*)(instr+1), padding);
        }
        break;

    case 27:    // [$1234]
        wprintw(debugwin, "[$%02x%02x]    ", instr[2], instr[1]);
        break;

    default:
        wprintw(debugwin, "%15s %02d ", "bad addr mode", addrmode[instr[0]]);
    }
}

unsigned char *findoppage() {
    if (xpc & 0x8000) {
        return snesmmap[xpb];
    } else {
        // lower address
        if ((xpc < 0x4300) || (memtabler8[xpb] != regaccessbankr8)) {
            // lower memory
            return snesmap2[xpb];
        } else {
            // dma
            return (unsigned char*)(dmadata-0x4300);
        }
    }
}

/* grinvader's version -- kept incase I didn't get mine to match
unsigned char *findoppage()
{
  if (xpc & 0x8000) { return(snesmmap[xpb]); }
  else
  { // lower address
    if (xpc < 0x4300 || memtabler8[xpb] != regaccessbankr8)
    { return(snesmap2[xpb]); }
    // dma
    return(dmadata - 0x4300); // or maybe (&dmadata - 0x4300)
  }
}
*/

unsigned char *findop() {
    unsigned char *address = findoppage()+xpc;
    return address;
}

// print out a 65816 instruction
void out65816() {
    unsigned char *address, opcode;
    char opname[5] = "FOO ";

    wprintw(debugwin, "%02x%04x ", xpb, xpc);

    // is this safe?
    address = findop();
    opcode = *address;

    memcpy(opname, &ocname[opcode*4], 4);
    wprintw(debugwin, "%s", opname);

    out65816_addrmode(address);

    wprintw(debugwin, "A:%04x X:%04x Y:%04x S:%04x DB:%02x D:%04x P:%02x %c",
            xa, xx, xy, xs, xdb, xd, xp, (xe == 1) ? 'E' : 'e');
}

void outsa1() {
    // stub!
}


void nextopcode() {
    attrset(CP(cp_white_on_blue));

    move(1,50); printw("%11d", numinst);
    move(1,20); printw("%3d",  curcyc);
    move(1,26); printw("%3d",  curypos);

    // I don't understand the buffering scheme here... I'm just going
    // to hope it isn't really all that important.

    //if (debugsa1 != 1)
    out65816();
    //else
    //  outputbuffersa1();
}


void cleardisplay() {
    move(0, 0); /* clear(); refresh(); */ endwin();
}

//*******************************************************
// Next SPC Opcode          Writes the next opcode & regs
//*******************************************************
// 008000 STZ $123456,x A:0000 X:0000 Y:0000 S:01FF DB:00 D:0000 P:33 E+

void outspc_addrmode() {
    unsigned char mode;
    char *format;
    char buf[16] = "               ";
    char *p;

#define HEX8(val)   do { p += sprintf(p, "%02x", val); *p = ' '; } while (0)
#define HEX16(val)  do { p += sprintf(p, "%04x", val); *p = ' '; } while (0)

    mode = ArgumentTable[spcPCRam[0]];
    format = AddressTable[mode];

    // memset(buf, ' ', 15); buf[15] = 0;

    p = buf;
    while (*format) {
        if (*format != '%') {
            *p++ = *format++;
            continue;
        }

        format++;
        switch (*format++) {

            case '1': // first byte
                HEX8(spcPCRam[1]);
                break;

            case '2': // second byte
                HEX8(spcPCRam[2]);
                break;

            case '3': // hinib
                *p++ = (spcPCRam[0] >> 4) + '0';
                *p++ = ' ';
                break;

            case '4': // hinib2
                *p++ = (spcPCRam[0] >> 5) + '0';
                *p++ = ' ';
                break;

            case '5': // rela2pc2
            {
                signed char off;
                off = *(signed char*)(spcPCRam+1);
                HEX16(off + 2 + (spcPCRam - SPCRAM));
                // format += 3;
                break;
            }

            case '6': // dp
            {
                *p++ = '$';

                if (spcP & 0x20) {
                    *p++ = '1';
                } else {
                    *p++ = '0';
                }

                break;
            }

            case '8': // memorybit
                HEX16((*(unsigned short*)(spcPCRam+1)) >> 3);
                // format += 2;
                break;

            case '9': // memorybitlow
                *p++ = ',';
                *p++ = (spcPCRam[1] & 0x7) + '0';
                break;

            case 'A': // rela2pc1
            {
                signed char off;
                off = *(signed char*)(spcPCRam+1);
                HEX16(off + 2 + spcPCRam - SPCRAM);
                // format += 2;
                break;
            }

            case 'B': // rela2pc2at2
            {
                signed char off;
                off = *(signed char*)(spcPCRam+2);
                HEX16(off + 2 + spcPCRam - SPCRAM);
                // format += 2;
                break;
            }

        }

    }

    buf[15] = 0;
    waddstr(debugwin, buf);
}

void nextspcopcode() {
    if (!soundon)
        return;
    if (cycpbl >= 55)
        return;

    // output spc pc & opcode #
    wprintw(debugwin, " %04x/%02x ", spcPCRam - SPCRAM, spcPCRam[0]);


    // output instruction
    wprintw(debugwin, "%-6s", spcnametab[spcPCRam[0]]);
    outspc_addrmode();

    // output registers
    wprintw(debugwin, "A:%02x X:%02x Y:%02x S:%02x ", spcA, spcX, spcY, spcS);
    wprintw(debugwin, "N%cO%cD%c?%cH%cI%cZ%cC%c",
            (spcNZ & 0x80) ? '+' : '-',
            (spcP  & 0x40) ? '+' : '-',
            (spcP  & 0x20) ? '+' : '-',
            (spcP  & 0x10) ? '+' : '-',
            (spcP  & 0x08) ? '+' : '-',
            (spcP  & 0x04) ? '+' : '-',
            (spcP  & 0x02) ? '+' : '-',
            (spcP  & 0x01) ? '+' : '-');

    wprintw(debugwin, "\n");

}


//*******************************************************
// Debugger OpCode Information
//*******************************************************

// Yes, I know, not very C style, but I really really really didn't
// want to type all those quote marks and commas. --SamB
char *ocname =
"BRK ORA COP ORA TSB ORA ASL ORA PHP ORA ASL PHD TSB ORA ASL ORA "
"BPL ORA ORA ORA TRB ORA ASL ORA CLC ORA INC TCS TRB ORA ASL ORA "
"JSR AND JSL AND BIT AND ROL AND PLP AND ROL PLD BIT AND ROL AND "
"BMI AND AND AND BIT AND ROL AND SEC AND DEC TSC BIT AND ROL AND "
"RTI EOR WDM EOR MVP EOR LSR EOR PHA EOR LSR PHK JMP EOR LSR EOR "
"BVC EOR EOR EOR MVN EOR LSR EOR CLI EOR PHY TCD JMP EOR LSR EOR "
"RTS ADC PER ADC STZ ADC ROR ADC PLA ADC ROR RTL JMP ADC ROR ADC "
"BVS ADC ADC ADC STZ ADC ROR ADC SEI ADC PLY TDC JMP ADC ROR ADC "
"BRA STA BRL STA STY STA STX STA DEY BIT TXA PHB STY STA STX STA "
"BCC STA STA STA STY STA STX STA TYA STA TXS TXY STZ STA STZ STA "
"LDY LDA LDX LDA LDY LDA LDX LDA TAY LDA TAX PLB LDY LDA LDX LDA "
"BCS LDA LDA LDA LDY LDA LDX LDA CLV LDA TSX TYX LDY LDA LDX LDA "
"CPY CMP REP CMP CPY CMP DEC CMP INY CMP DEX WAI CPY CMP DEC CMP "
"BNE CMP CMP CMP PEI CMP DEC CMP CLD CMP PHX STP JML CMP DEC CMP "
"CPX SBC SEP SBC CPX SBC INC SBC INX SBC NOP XBA CPX SBC INC SBC "
"BEQ SBC SBC SBC PEA SBC INC SBC SED SBC PLX XCE JSR SBC INC SBC ";


// Immediate Addressing Modes :
//   09 - ORA-M, 29 - AND-M, 49 - EOR-M, 69 - ADC-M, 89 - BIT-M,
//   A0 - LDY-X, A2 - LDX-X, A9 - LDA-M, C0 - CPY-X, C2 - REP-B,
//   C9 - CMP-M, E0 - CPX-X, E2 - SEP-B, E9 - SBC-M
//   Extra Addressing Mode Values : B(1-byte only) = 25, X(by X flag) = 26

unsigned char addrmode[256] = {
    25, 9,25,22, 4, 4, 4,19,21, 1, 5,21, 2, 2, 2, 3,
    15, 7,18,23, 4,10,10, 8, 6,13, 5, 6, 2,12,12,14,
     2, 9, 3,22, 4, 4, 4,19,21, 1, 5,21, 2, 2, 2, 3,
    15, 7,18,23,10,10,10, 8, 6,13, 5, 6,12,12,12,14,
    21, 9, 0,22,24, 4, 4,19,21, 1, 5,21, 2, 2, 2, 3,
    15, 7,18,23,24,10,10, 8, 6,13,21, 6, 3,12,12,14,
    21, 9, 2,22, 4, 4, 4,19,21, 1, 5,21,17, 2, 2, 3,
    15, 7,18,23,10,10,10, 8, 6,13,21, 6,20,12,12,14,
    15, 9,16,22, 4, 4, 4,19, 6, 1, 6,21, 2, 2, 2, 3,
    15, 7,18,23,10,10,11, 8, 6,13, 6, 6, 2,12,12,14,
    26, 9,26,22, 4, 4, 4,19, 6, 1, 6,21, 2, 2, 2, 3,
    15, 7,18,23,10,10,11, 8, 6,13, 6, 6,12,12,13,14,
    26, 9,25,22, 4, 4, 4,19, 6, 1, 6, 6, 2, 2, 2, 3,
    15, 7,18,23,18,10,10, 8, 6,13,21, 6,27,12,12,14,
    26, 9,25,22, 4, 4, 4,19, 6, 1, 6, 6, 2, 2, 2, 3,
    15, 7,18,23, 2,10,10, 8, 6,13,21, 6,20,12,12,14
};


char *spcnametab[256] = {
    "NOP",  "TCALL", "SET1",  "BBS",
    "OR",   "OR",    "OR",    "OR",
    "OR",   "OR",    "OR1",   "ASL",
    "ASL",  "PUSH",  "TSET1", "BRK",

    "BPL",  "TCALL", "CLR1",  "BBC",
    "OR",   "OR",    "OR",    "OR",
    "OR",   "OR",    "DECW",  "ASL",
    "ASL",  "DEC",   "CMP",   "JMP",

    "CLRP", "TCALL", "SET1",  "BBS",
    "AND",  "AND",   "AND",   "AND",
    "AND",  "AND",   "OR1",   "ROL",
    "ROL",  "PUSH",  "CBNE",  "BRA",

    "BMI",  "TCALL", "CLR1",  "BBC",
    "AND",  "AND",   "AND",   "AND",
    "AND",  "AND",   "INCW",  "ROL",
    "ROL",  "INC",   "CMP",   "CALL",


    "SETP", "TCALL", "SET1",  "BBS",
    "EOR",  "EOR",   "EOR",   "EOR",
    "EOR",  "EOR",   "AND1",  "LSR",
    "LSR",  "PUSH",  "TCLR1", "PCALL",

    "BVC",  "TCALL", "CLR1",  "BBC",
    "EOR",  "EOR",   "EOR",   "EOR",
    "EOR",  "EOR",   "CMPW",  "LSR",
    "LSR",  "MOV",   "CMP",   "JMP",

    "CLRC", "TCALL", "SET1",  "BBS",
    "CMP",  "CMP",   "CMP",   "CMP",
    "CMP",  "CMP",   "AND1",  "ROR",
    "ROR",  "PUSH",  "DMNZ",  "RET",

    "BVS",  "TCALL", "CLR1",  "BBC",
    "CMP",  "CMP",   "CMP",   "CMP",
    "CMP",  "CMP",   "ADDW",  "ROR",
    "ROR",  "MOV",   "CMP",   "RET1",


    "SETC", "TCALL", "SET1",  "BBS",
    "ADC",  "ADC",   "ADC",   "ADC",
    "ADC",  "ADC",   "EOR1",  "DEC",
    "DEC",  "MOV",   "POP",   "MOV",

    "BCC",  "TCALL", "CLR1",  "BBC",
    "ADC",  "ADC",   "ADC",   "ADC",
    "ADC",  "ADC",   "SUBW",  "DEC",
    "DEC",  "MOV",   "DIV",   "XCN",

    "EI",   "TCALL", "SET1",  "BBS",
    "SBC",  "SBC",   "SBC",   "SBC",
    "SBC",  "SBC",   "MOV1",  "INC",
    "INC",  "CMP",   "POP",   "MOV",

    "BCS",  "TCALL", "CLR1",  "BBC",
    "SBC",  "SBC",   "SBC",   "SBC",
    "SBC",  "SBC",   "MOVW",  "INC",
    "INC",  "MOV",   "DAS",   "MOV",


    "DI",   "TCALL", "SET1",  "BBS",
    "MOV",  "MOV",   "MOV",   "MOV",
    "CMP",  "MOV",   "MOV1",  "MOV",
    "MOV",  "MOV",   "POP",   "MUL",

    "BNE",  "TCALL", "CLR1",  "BBC",
    "MOV",  "MOV",   "MOV",   "MOV",
    "MOV",  "MOV",   "MOVW",  "MOV",
    "DEC",  "MOV",   "CBNE",  "DAA",

    "CLRV", "TCALL", "SET1",  "BBS",
    "MOV",  "MOV",   "MOV",   "MOV",
    "MOV",  "MOV",   "NOT1",  "MOV",
    "MOV",  "NOTC",  "POP",   "SLEEP",

    "BEQ",  "TCALL", "CLR1",  "BBC",
    "MOV",  "MOV",   "MOV",   "MOV",
    "MOV",  "MOV",   "MOV",   "MOV",
    "INC",  "MOV",   "DBNZ",  "STOP"
};
// need... air... badly...!


// %1 = Byte, %2 = Second Byte, %3 = high nibble of opcode #,
// %4 = high nibble of opcode # and 07h, %5 = relative to PC+2
// %6 = dp ($0/$1)
// %7 = memory SHR 3 Low, %8 = memory SHR 3 High, %9 = ,memory AND 7h
// %A = relative to PC+1, %B = relative to PC+1 at second byte
char *AddressTable[68] = {
//                  1                  1                  1
"",                "%3",              "%6%1,%4",         "B%4 %6%1,$%B+1",
// 0 : nothing
// 1 : the high nibble
// 2 : the high nibble first 3 bit (and 0111000 then shift)
// 3 : 2 + relative
"A,%6%1",          "A,$%2%1",         "A,(X)",           "A,(%6%1+x)",
// 4 : A,dp
// 5 : A,labs
// 6 : A,(X)
// 7 : A,(dp+X)
"A,#$%1",          "(%6%2),(%6%1)",   "CF,mbit%8%7%9",   "%6%1",
// 8 : A,#inm
// 9 : dp(d),dp(s)   (two dp)
// 10 : Carry flag, memory bit          (can only access from 0 to 1fff)
// 11 : dp
"$%2%1",           "PSW",             "$%A",             "A,%6%1+X",
// 12 : labs
// 13 : PSW
// 14 : rel
// 15 : A,dp+X
"A,$%2%1+X",       "A,$%2%1+Y",       "A,(%6%1)+Y",      "%6%2,#$%1",
// 16 : A,labs+X
// 17 : A,labs+Y
// 18 : A,(dp)+Y
// 19 : dp,#inm
"(X),(Y)",         "%6%1+X",          "A",               "X",
// 20 : (X),(Y)
// 21 : dp+X
// 22 : A
// 23 : X
"X,%2%1",          "($%2%1+X)",       "CF,/(mb%8%7%9)",  "%6%1",
// 24 : X,labs
// 25 : (labs+X)
// 26 : C,/mem.bit
// 27 : upage         (same as dp but for a call)
"YA,%6%1",         "X,A",             "Y,$%2%1",         "Y",
// 28 : YA,dp
// 29 : X,A
// 30 : Y,labs
// 31 : Y
"Y,%6%1",          "Y,#$%1",          "%6%1,$%B",        "X,%6%1",
// 32 : Y,dp
// 33 : Y,#inm
// 34 : dp,rel
// 35 : X,dp
"A,X",             "%6%2,#$%1",       "X,SP",            "YA,X",
// 36 : A,X
// 37 : dp,#inm
// 38 : X,SP
// 39 : YA,X
"(X)+,A",          "SP,X",            "A,(X)+",          "%6%1,A",
// 40 : (X)+,A
// 41 : SP,X
// 42 : A,(X)+
// 43 : dp,A
"$%2%1,A",         "(X),A",           "%6%1+X,A",        "X,#$%1",
// 44 : labs,A
// 45 : (X),A
// 46 : (dp+X),A
// 47 : X,#inm
"$%2%1,X",         "mb%8%7%9,CF",     "%6%1,Y",          "$%2%1,Y",
// 48 : labs,X
// 49 : mem.bit,C
// 50 : dp,Y
// 51 : labs,Y
"YA",              "%6%1+X,A",        "$%2%1+X,A",       "$%2%1+Y,A",
// 52 : YA
// 53 : dp+X,A
// 54 : labs+X,A
// 55 : labs+Y,A
"(%6%1)+Y,A",      "%6%1,X",          "%6%1+Y,X",        "%6%1,YA",
// 56 : (dp)+Y,A
// 57 : dp,X
// 58 : dp+Y,X
// 59 : dp,YA
"%6%1+X,Y",        "A,Y",             "%6%2+X,$%A",      "mb%8%7%9,CF",
// 60 : dp+X,Y
// 61 : A,Y
// 62 : dp+X,rel
// 63 : mem.bit
"X,%6%1+Y",        "Y,%6%1+X",        "Y,A",             "Y,$%A",
// 64 : X,dp+Y
// 65 : Y,dp+X
// 66 : Y,A
// 67 : Y,rel

};

unsigned char ArgumentTable[256] = {
//     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,12, 0,
//     10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F
       14, 1, 2, 3,15,16,17,18,19,20,11,21,22,23,24,25,
//     20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,26,11,12,22,34,14,
//     30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
       14, 1, 2, 3,15,16,17,18,19,20,11,21,22,23,35,12,
//     40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,23,12,27,
//     50 51 52 53 54 55 56 57 58 59 5A 5B 5C 5D 5E 5F
       14, 1, 2, 3,15,16,17,18,19,20,28,21,22,29,30,12,
//     60 61 62 63 64 65 66 67 68 69 6A 6B 6C 6D 6E 6F
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,26,11,12,31,34, 0,
//     70 71 72 73 74 75 76 77 78 79 7A 7B 7C 7D 7E 7F
       14, 1, 2, 3,15,16,17,18,19,20,28,21,22,36,32, 0,
//     80 81 82 83 84 85 86 87 88 89 8A 8B 8C 8D 8E 8F
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,33,13,37,
//     90 91 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F
       14, 1, 2, 3,15,16,17,18,19,20,28,21,22,38,39,22,
//     A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,33,22,40,
//     B0 B1 B2 B3 B4 B5 B6 B7 B8 B9 BA BB BC BD BE BF
       14, 1, 2, 3,15,16,17,18,19,20,28,21,22,41,22,42,
//     C0 C1 C2 C3 C4 C5 C6 C7 C8 C9 CA CB CC CD CE CF
        0, 1, 2, 3,43,44,45,46,47,48,49,50,51,47,23,52,
//     D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 DA DB DC DD DE DF
       14, 1, 2, 3,53,54,55,56,57,58,59,60,31,61,62,22,
//     E0 E1 E2 E3 E4 E5 E6 E7 E8 E9 EA EB EC ED EE EF
        0, 1, 2, 3, 4, 5, 6, 7, 8,24,63,32,30, 0,31, 0,
//     F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 FA FB FC FD FE FF
       14, 1, 2, 3,15,16,17,18,35,64, 9,65,31,66,67, 0
};


// Jonas Quinn's file functions

void SaveOAMRamLog() {
  FILE *fp = 0;

  if ((fp = fopen_dir(ZCfgPath,"vram.dat","wb"))) {
    fwrite(oamram,1,544,fp);
    fclose(fp);
  }
}

void debugdump() {
  FILE *fp = 0;

  if ((fp = fopen_dir(ZCfgPath,"SPCRAM.dmp","wb"))) {
    fwrite(SPCRAM,1,65536,fp);
    fclose(fp);
  }

  if ((fp = fopen_dir(ZCfgPath,"DSP.dmp","wb"))) {
    fwrite(DSPMem,1,256,fp);
    fclose(fp);
  }
}
