/*
 * video_backend.h — Video rendering backend abstraction
 *
 * Provides a function-pointer dispatch table for video backends, modeled
 * after the audio backend pattern in linux/audio.c.  Each platform backend
 * (SW, GL, DirectDraw) implements the same four operations.
 *
 * Usage:
 *   extern video_backend_t const video_sw;   // linux/sw_draw.c
 *   extern video_backend_t const video_gl;   // linux/gl_draw.c
 *
 *   video_backend_t const* vb = use_opengl ? &video_gl : &video_sw;
 *   vb->start(width, height, depth, fullscreen);
 *   vb->draw();
 *   vb->end();
 */
#ifndef VIDEO_BACKEND_H
#define VIDEO_BACKEND_H

typedef struct video_backend
{
    char const* name;

    /* Initialize the video surface/window.
     * Returns non-zero on success, 0 on failure. */
    int (*start)(int width, int height, int depth, int fullscreen);

    /* Tear down the video surface/window. */
    void (*end)(void);

    /* Clear the framebuffer. */
    void (*clear)(void);

    /* Present the current emulator frame to the display. */
    void (*draw)(void);
} video_backend_t;

#endif /* VIDEO_BACKEND_H */
