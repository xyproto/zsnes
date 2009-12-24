#ifndef LINK_H
#define LINK_H

#ifndef __MSDOS__
void Start36HZ(void);
void Start60HZ(void);
void Stop36HZ(void);
void Stop60HZ(void);
void clearwin(void);
void drawscreenwin(void);
void initwinvideo(void);
#endif

#endif
