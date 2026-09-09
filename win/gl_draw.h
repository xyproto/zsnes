#ifndef GL_DRAW_h
#define GL_DRAW_h 1

#include <GL/gl.h>
#include <stdint.h>

int gl_start(int width, int height, int req_depth, int FullScreen);
void gl_end(void);
void gl_clearwin(void);
void gl_drawwin(void);

// FUNCTIONS
extern void hq2x_16b(void);

// VIDEO VARIABLES
extern int32_t SurfaceLocking;

extern HWND hMainWindow;
extern HDC hDC;
extern HGLRC hRC;

// OPENGL VARIABLES
extern uint8_t GUIOn2;
extern unsigned char* vidbuffer;
extern uint8_t curblank;
extern uint8_t GUIRESIZE[];

void gl_clearwin(void);
void gl_scanlines(void);

#endif
