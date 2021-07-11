#ifndef PROCVID_H
#define PROCVID_H

#include "../types.h"

// Processes & displays video
void showvideo(void);

u4 SwapMouseButtons(u4 buttons);

void processmouse1(void);
void processmouse2(void);

#ifdef __MSDOS__
// Outputs val at buf
void outputhex(u1* buf, u1 val);
#endif

void outputhex16(u2* buf, u1 val);

#ifdef __MSDOS__
// Outputs char glyph at buf
void outputchar(u1* buf, u1 glyph);
#endif

void outputchar16b(u2* buf, u1 glyph);

// Outputs String from text to buf
void OutputGraphicString(u1* buf, char const* text);

void OutputGraphicString16b(u2* buf, char const* text);

#ifdef __MSDOS__
void OutputGraphicString5x5(u1* buf, char const* text);
#endif

void OutputGraphicString16b5x5(u2* buf, char const* text);

#ifdef __MSDOS__
void drawhline(u1* buf, u4 n, u1 colour);
#endif

void drawhline16b(u2* buf, u4 n, u2 colour);

#ifdef __MSDOS__
void drawvline(u1* buf, u4 n, u1 colour);
#endif

void drawvline16b(u2* buf, u4 n, u2 colour);

void doveg(void);

void dovegrest(void);

#ifdef __MSDOS__
// Changes the entire palette.  Set the brightness with maxbr
void dosmakepal(void);
#endif

// Copies buffer into video
void copyvid(void);

extern char const* Msgptr; // Pointer to message
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
extern u4 MsgCount; // How long message will stay (PAL = 100)

#endif
