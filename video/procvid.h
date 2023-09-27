#ifndef PROCVID_H
#define PROCVID_H

#include "../types.h"

// Processes & displays video
void showvideo(void);

u4 SwapMouseButtons(u4 buttons);

void processmouse1(void);
void processmouse2(void);

void outputhex16(u2 *buf, u1 val);
void outputchar16b(u2 *buf, u1 glyph);

// Outputs String from text to buf
void OutputGraphicString(u1 *buf, char const *text);
void OutputGraphicString16b(u2 *buf, char const *text);
void OutputGraphicString16b5x5(u2 *buf, char const *text);
void drawhline16b(u2 *buf, u4 n, u2 colour);
void drawvline16b(u2 *buf, u4 n, u2 colour);

// Copies buffer into video
void copyvid(void);

extern char const *Msgptr; // Pointer to message
extern u1 FPSOn;
extern u1 ForceNonTransp;
extern u1 csounddisable;
extern u1 f3menuen;
extern u1 mousexdir;
extern u1 mouseydir;
extern u1 prevbright; // previous brightness
extern u1 ssautosw;
extern u1 const ASCII2Font[256];
extern u2 mousebuttons;
extern u2 mousexloc;
extern u2 mousexpos;
extern u2 mouseyloc;
extern u2 mouseypos;
extern u2 tempco0;
extern u4 MessageOn; // Message On Countdown
extern u4 MsgCount;	 // How long message will stay (PAL = 100)

#endif
