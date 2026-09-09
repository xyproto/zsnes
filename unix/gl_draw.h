#ifndef GL_DRAW_h
#define GL_DRAW_h 1

int gl_start(int width, int height, int req_depth, int FullScreen);
void gl_end(void);
void gl_clearwin(void);
void gl_drawwin(void);
void SetGLAttributes(void);

extern char allow_glvsync;

#endif
