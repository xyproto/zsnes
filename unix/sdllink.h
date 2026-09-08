// Looks good
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
void PlaceWindowOnMonitor(struct SDL_Window* win);

void SetMouseMinX(int MinX);
void SetMouseMaxX(int MaxX);
void SetMouseMinY(int MinY);
void SetMouseMaxY(int MaxY);
void SetMouseX(int X);
void SetMouseY(int Y);

extern uint32_t numlockptr;

#endif
