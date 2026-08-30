#ifndef SDLLINK_H
#define SDLLINK_H

#include <stdint.h>

void sem_sleep(void);

void SetMouseMinX(int MinX);
void SetMouseMaxX(int MaxX);
void SetMouseMinY(int MinY);
void SetMouseMaxY(int MaxY);
void SetMouseX(int X);
void SetMouseY(int Y);

extern uint32_t numlockptr;

#endif
