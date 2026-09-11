#include "../video/sw_draw.h"
#include "../gblhdr.h"
#include "../intrf.h"
#include "../link.h"
#include "../ui.h"
#include "../video/copyvwin.h"
#include "../video/filter.h"
#include "cfg.h"
#include "sdllink.h"
#include <stdint.h>

void CheckFrame(void);
// VIDEO VARIABLES
extern SDL_Window* sdl_window;
extern SDL_Surface* surface;
extern int SurfaceLocking;
static SDL_Surface* render_surface = NULL; // 16-bit RGB565 surface for emulator output
/* Where a filter draws, at its own size rather than the window's. The window
   surface is whatever size the user picked, and a filter that had to match it
   was either unreachable or wrote past the end of it; scaling on the way out
   is what the accelerated path already does. */
static SDL_Surface* filter_surface = NULL;

extern uint8_t curblank;
extern int frametot;
extern uint8_t GUIOn, GUIOn2;

void NTSCFilterInit(void);
void NTSCFilterDraw(int SurfaceX, int SurfaceY, int pitch, unsigned char* buffer);
char CheckOGLMode(void);

int sw_start(int width, int height, int req_depth, int FullScreen)
{
    uint32_t flags = 0;

    flags |= (FullScreen ? SDL_WINDOW_FULLSCREEN : 0);

    if (NTSCFilter) {
        NTSCFilterInit();
    }

    SurfaceX = width;
    SurfaceY = height;

    if (filter_surface) {
        SDL_DestroySurface(filter_surface);
        filter_surface = NULL;
    }
    if (render_surface) {
        SDL_DestroySurface(render_surface);
        render_surface = NULL;
    }
    if (sdl_window) {
        // Preserve the window and its Wayland context.
        SDL_SetWindowFullscreen(sdl_window, FullScreen ? true : false);
        SDL_SetWindowSize(sdl_window, SurfaceX, SurfaceY);
        SDL_SyncWindow(sdl_window); // settle the new size before mapping the mouse
    } else {
        sdl_window = SDL_CreateWindow("ZSNES", SurfaceX, SurfaceY, flags);
        PlaceWindowOnMonitor(sdl_window);
        if (sdl_window == NULL) {
            fprintf(stderr, "Could not create %dx%d window: %s\n", SurfaceX, SurfaceY, SDL_GetError());
            return 0;
        }
    }

    // Create a 16-bit RGB565 surface for the emulator to render into
    render_surface = SDL_CreateSurface(SurfaceX, SurfaceY, SDL_PIXELFORMAT_RGB565);
    if (render_surface == NULL) {
        fprintf(stderr, "Could not create render surface: %s\n", SDL_GetError());
        return 0;
    }

    surface = render_surface;
    SurfaceLocking = SDL_MUSTLOCK(surface);

    // Grab mouse in fullscreen mode
    SDL_SetWindowMouseGrab(sdl_window, FullScreen ? true : false);

    SDL_HideCursor();

    // Always RGB565
    converta = 0;

    return 1;
}

void sw_end(void)
{
    if (filter_surface) {
        SDL_DestroySurface(filter_surface);
        filter_surface = NULL;
    }
    if (render_surface) {
        SDL_DestroySurface(render_surface);
        render_surface = NULL;
        surface = NULL;
    }
    if (sdl_window) {
        SDL_PumpEvents();
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
    }
}

static void LockSurface(void)
{
    if (SurfaceLocking) {
        SDL_LockSurface(surface);
    }
}

/* Put a finished picture on the screen, scaled to the window if it is not
   already the same size. That is the ordinary case once a filter is on, and
   fullscreen, where the picture rarely divides into the panel a whole number
   of times: at 448 rows into 1200 some source rows land on two and some on
   three, and nearest neighbour shows that as a coarse line pattern that crawls
   over anything moving. Honour the bilinear setting here as the other paths
   do. */
static void sw_present(SDL_Surface* const src)
{
    SDL_Surface* const win_surface = SDL_GetWindowSurface(sdl_window);

    if (!win_surface) {
        return;
    }
    if (win_surface->w != src->w || win_surface->h != src->h) {
        extern u1 BilinearFilter;

        SDL_BlitSurfaceScaled(src, NULL, win_surface, NULL,
            BilinearFilter ? SDL_SCALEMODE_LINEAR : SDL_SCALEMODE_NEAREST);
    } else {
        SDL_BlitSurface(src, NULL, win_surface, NULL);
    }
#ifdef ZSNES_DEBUG_HOOKS
    if (ZSnesFrameDumpWanted()) {
        ZSnesFrameDumpSurface(win_surface, "sw");
    }
#endif
    SDL_UpdateWindowSurface(sdl_window);
}

static void UnlockSurface(void)
{
    if (SurfaceLocking) {
        SDL_UnlockSurface(surface);
    }
    sw_present(render_surface);
}

extern uint32_t NGNoTransp; /* a dword where it is defined (video/c_newgfx16data.c) */
extern uint16_t resolutn;
uint32_t pitch;

/* Somewhere the size of the filter's own picture, remade when that changes. */
static SDL_Surface* sw_filter_surface(int const w, int const h)
{
    if (filter_surface && (filter_surface->w != w || filter_surface->h != h)) {
        SDL_DestroySurface(filter_surface);
        filter_surface = NULL;
    }
    if (!filter_surface) {
        filter_surface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGB565);
    }
    return filter_surface;
}

/* Non-zero once the filtered frame is on the screen. */
static int sw_draw_filtered(void)
{
    VideoFilter const filter = VideoFilterGet();
    VideoFilterPicture const pic = VideoFilterOutput(filter);
    SDL_Surface* fs;
    int drawn;

    if (filter == VFILTER_NONE) {
        return 0;
    }
    fs = sw_filter_surface(pic.w, pic.h);
    if (!fs) {
        return 0;
    }
    if (SDL_MUSTLOCK(fs)) {
        SDL_LockSurface(fs);
    }
    drawn = VideoFilterDraw(filter, fs->pixels, fs->pitch,
        (size_t)fs->pitch * (size_t)fs->h);
    if (SDL_MUSTLOCK(fs)) {
        SDL_UnlockSurface(fs);
    }
    if (drawn) {
        sw_present(fs);
    }
    return drawn;
}

void sw_clearwin(void)
{
    pitch = surface->pitch;
    SurfBufD = surface->pixels;

    LockSurface();
    ClearWin16();
    UnlockSurface();
}

extern uint8_t prevNTSCMode;
extern uint8_t changeRes;
extern uint8_t prevKeep4_3Ratio;

void sw_drawwin(void)
{
    NGNoTransp = 0; // Set this value to 1 within the appropriate
    // Where a custom or hardware transparency routine would go. Only reachable
    // with newengen == 1; see ProcessTransparencies (video/c_ngtransp.c).

    UpdateVFrame();

    if (curblank || CheckOGLMode()) {
        return;
    }

    if (NTSCFilter != prevNTSCMode || changeRes
        || prevKeep4_3Ratio != Keep4_3Ratio) {
        initwinvideo();
        if (CheckOGLMode()) {
            return;
        }
    }

    /* A filter draws at its own size and is scaled to the window, so every
       filter is available in every video mode rather than only in the one
       whose surface happened to match. */
    if (sw_draw_filtered()) {
        return;
    }

    LockSurface();

    ScreenPtr = vidbuffer;
    ScreenPtr += VID_FIRST * 2;

    if (resolutn == 239) {
        ScreenPtr += 8 * VID_STRIDE * 2;
    }

    pitch = surface->pitch;
    SurfBufD = surface->pixels;

    if (SurfBufD == 0) {
        UnlockSurface();
        return;
    }

    if (SurfaceX == 256 && SurfaceY == 224) {
        DrawWin256x224x16();
    } else if (SurfaceX == 320 && SurfaceY == 240) {
        DrawWin320x240x16();
    } else if (SurfaceX == 512 && SurfaceY == 448) {
        AddEndBytes = pitch - 1024;
        NumBytesPerLine = pitch;
        WinVidMemStart = SurfBufD;
        copy640x480x16bwin();
    } else if (SurfaceX == 640 && SurfaceY == 480) {
        AddEndBytes = pitch - 1024;
        NumBytesPerLine = pitch;
        WinVidMemStart = SurfBufD + 16 * 640 * 2 + 64 * 2;
        copy640x480x16bwin();
    }

    UnlockSurface();
}
