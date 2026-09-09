#ifndef SDLLINK_H
#define SDLLINK_H

#include "../types.h"

#include <stdint.h>

void sem_sleep(void);

/* Monitors, as SDL lists them this run. MonitorID names one by a short ID
   taken from its name, which survives the renumbering SDL does each run. */
struct SDL_Window;
u4 VideoMonitorCount(void);
char const* VideoMonitorName(u4 i);
void VideoMonitorID(u4 i, char* out, u4 len);
u4 VideoMonitorSelected(void);
void VideoMonitorSelect(u4 i);
int VideoMonitorHDR(void);
void PlaceWindowOnMonitor(struct SDL_Window* win);

#ifdef ZSNES_DEBUG_HOOKS
/* ZSNES_FRAME_DUMP=N saves the composited window every N frames. Where
   Grab_PNG_Data_Path writes the emulator's own buffer, this is what the
   renderer actually put on screen, so the three drawing paths can be held
   against each other frame for frame. */
struct SDL_Surface;
extern unsigned zsnes_frame_dump_no;
int ZSnesFrameDumpWanted(void);
void ZSnesFrameDumpSurface(struct SDL_Surface* s, char const* tag);
#endif

void SetMouseMinX(int MinX);
void SetMouseMaxX(int MaxX);
void SetMouseMinY(int MinY);
void SetMouseMaxY(int MaxY);
void SetMouseX(int X);
void SetMouseY(int Y);

extern uint32_t numlockptr;

#endif
