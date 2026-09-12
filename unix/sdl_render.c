/*
 * The accelerated output path, on SDL_Renderer instead of OpenGL 1.x: Metal on
 * macOS, D3D on Windows, Vulkan or GL elsewhere, whichever SDL picks.
 *
 * The GL path drew the frame as a run of quads because a scanline can be
 * hi-res while its neighbour is not, and each run needed its own texture
 * geometry. Composing the frame into one 512x448 image first makes that a
 * per-line decision instead - a lo-res line is doubled, a hi-res one is
 * interleaved from the two field buffers - and leaves a single upload and a
 * single draw. The filters already produce 512x448, so they are copied
 * straight in.
 */

#include "../gblhdr.h"
#include "../link.h"
#include "../video/copyvwin.h"
#include "../video/crt.h"
#include "../video/filter.h"
#include "cfg.h"
#include "sdllink.h"
#include <math.h>
#include <stdint.h>

#include "../video/ntsc.h"

extern SDL_Window* sdl_window;
extern uint8_t* vidbuffer;
extern uint16_t resolutn;
extern uint8_t curblank;
extern uint8_t GUIRESIZE[];
extern Uint8 GUIOn2;
extern uint32_t NGNoTransp; /* a dword where it is defined (video/c_newgfx16data.c) */
extern uint8_t SpecialLine[256], hirestiledat[256], GUIOn, newengen, cfield;

char CheckOGLMode(void);

int sr_start(int width, int height, int req_depth, int FullScreen);
void sr_end(void);
void sr_clearwin(void);
void sr_drawwin(void);

/* The composed frame, and what carries it to the GPU. The texture is made at
   the largest picture any mode produces - hq4x over a 239-line overscan frame,
   which is wider and taller than the NTSC filter's 602x446, the 640x480 modes
   and the doubled 512x448 - and each frame uploads and draws only the part it
   filled, so the renderer scales whatever size came out to the window. */
#define SR_W 512
#define SR_H 448
#define SR_MAXW 1024
#define SR_MAXH (239 * 4)
static SDL_Renderer* sr_renderer = NULL;
static SDL_Texture* sr_texture = NULL;
static unsigned short* sr_pixels = NULL;

/* The HDR path, used only when the setting asks for it *and* the renderer says
   the display is really in HDR. sr_hdr holds linear float RGBA, where 1.0 is
   SDR white and the bloom is free to go above it, up to what the display says
   it has room for. */
static float* sr_hdr = NULL;
static SDL_Texture* sr_hdr_texture = NULL;
static float sr_hdr_headroom = 1.0f;
static int sr_hdr_active = 0;

/* Whether the window we are holding is a fullscreen one. */
static int sr_fullscreen = -1;

/* The second field sits 75036 pixels on; the line geometry is in copyvwin.h. */
#define SR_FIELD2 (75036 * 2)

static void sr_release(void);

/* Render drivers to try, in order, before letting SDL pick for itself.
   Vulkan first: measured on a Wayland session it is one of only two drivers
   that will accept the linear-light colorspace HDR needs - opengl, opengles2
   and software all refuse it - and it is the better backend regardless. NULL
   is the last entry and means "whatever SDL would have chosen". */
static char const* const sr_drivers[] = { "vulkan", "gpu", NULL };

/* Make a renderer for `win`, asking for linear light when `linear` is set,
   which is what SDL wants before it will hand back an HDR surface. Falls
   through the list until one takes it, so asking for HDR on a machine without
   it costs nothing but the attempts. */
static SDL_Renderer* sr_make_renderer(SDL_Window* const win, int const linear)
{
    unsigned i;

    for (i = 0; i < sizeof(sr_drivers) / sizeof(sr_drivers[0]); i++) {
        SDL_PropertiesID const props = SDL_CreateProperties();
        SDL_Renderer* r;

        if (!props) {
            return NULL;
        }
        SDL_SetPointerProperty(props, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, win);
        if (sr_drivers[i]) {
            SDL_SetStringProperty(props, SDL_PROP_RENDERER_CREATE_NAME_STRING,
                sr_drivers[i]);
        }
        if (linear) {
            SDL_SetNumberProperty(props,
                SDL_PROP_RENDERER_CREATE_OUTPUT_COLORSPACE_NUMBER,
                SDL_COLORSPACE_SRGB_LINEAR);
        }
        r = SDL_CreateRendererWithProperties(props);
        SDL_DestroyProperties(props);
        if (r) {
            return r;
        }
    }
    return NULL;
}

/* Whether this renderer is actually showing HDR, and how far above SDR white
   it will go. Both can change while running, so they are read per frame. */
static void sr_hdr_refresh(void)
{
    SDL_PropertiesID props;

    sr_hdr_active = 0;
    sr_hdr_headroom = 1.0f;
    if (!sr_hdr_texture || !sr_renderer) {
        return;
    }
    props = SDL_GetRendererProperties(sr_renderer);
    if (!props) {
        return;
    }
    if (SDL_GetBooleanProperty(props, SDL_PROP_RENDERER_HDR_ENABLED_BOOLEAN, false)) {
        sr_hdr_active = 1;
        sr_hdr_headroom
            = SDL_GetFloatProperty(props, SDL_PROP_RENDERER_HDR_HEADROOM_FLOAT, 1.0f);
        if (sr_hdr_headroom < 1.0f) {
            sr_hdr_headroom = 1.0f;
        }
    }
}

int sr_start(int width, int height, int req_depth, int FullScreen)
{
    uint32_t flags = 0;

    (void)req_depth;
    flags |= (GUIRESIZE[cvidmode] ? SDL_WINDOW_RESIZABLE : 0);

    if (NTSCFilter) {
        NTSCFilterInit();
    }

    SurfaceX = width;
    SurfaceY = height;

    sr_release();

    /* Reuse the window only while it stays on the same side of fullscreen.
       Resizing in place is what stops a filter toggle blanking the screen, but
       a window that has to cross into or out of fullscreen is better made
       again: driving the change in place leaves it blank on some compositors. */
    if (sdl_window && FullScreen != sr_fullscreen) {
        SDL_PumpEvents();
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
    }
    sr_fullscreen = FullScreen;

    if (sdl_window) {
        /* Resize in place rather than making a new one. */
        SDL_SetWindowSize(sdl_window, SurfaceX, SurfaceY);
        SDL_SyncWindow(sdl_window); // settle the new size before it is used
    } else {
        sdl_window = SDL_CreateWindow("ZSNES", SurfaceX, SurfaceY, flags);
        if (!sdl_window) {
            fprintf(stderr, "Could not create %dx%d window: %s\n", SurfaceX,
                SurfaceY, SDL_GetError());
            return false;
        }
        PlaceWindowOnMonitor(sdl_window);
    }

    /* Linear light only where the monitor is really in HDR mode; otherwise the
       ordinary sRGB output, still preferring Vulkan. */
    sr_renderer = VideoMonitorHDR() ? sr_make_renderer(sdl_window, 1) : NULL;
    if (!sr_renderer) {
        sr_renderer = sr_make_renderer(sdl_window, 0);
    }
    if (!sr_renderer) {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
        sr_fullscreen = -1;
        return false;
    }
    SDL_SetRenderVSync(sr_renderer, vsyncon ? 1 : SDL_RENDERER_VSYNC_DISABLED);

    sr_texture = SDL_CreateTexture(sr_renderer, SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING, SR_MAXW, SR_MAXH);
    if (!sr_texture) {
        fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(sr_renderer);
        sr_renderer = NULL;
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
        sr_fullscreen = -1;
        return false;
    }
    if (VideoMonitorHDR()) {
        /* Float rather than 565, so the bloom has somewhere above white to go.
           If the renderer will not take it we simply stay on the ordinary
           surface: HDR is an extra, never a requirement. */
        sr_hdr_texture = SDL_CreateTexture(sr_renderer, SDL_PIXELFORMAT_RGBA128_FLOAT,
            SDL_TEXTUREACCESS_STREAMING, SR_MAXW, SR_MAXH);
        if (sr_hdr_texture) {
            sr_hdr = (float*)malloc((size_t)SR_MAXW * SR_MAXH * 4 * sizeof(float));
        }
        if (!sr_hdr) {
            if (sr_hdr_texture) {
                SDL_DestroyTexture(sr_hdr_texture);
                sr_hdr_texture = NULL;
            }
            fprintf(stderr, "HDR surface unavailable; using the ordinary one\n");
        }
    }
    /* Fullscreen is a state change on a window that already has its renderer,
       the same sequence as the toggle; a window created fullscreen and then
       rebuilt for the renderer came up blank on some compositors. */
    if (FullScreen) {
        SDL_SetWindowFullscreen(sdl_window, true);
        SDL_SyncWindow(sdl_window);
    }

    /* The GUI is drawn from the same buffer, and reads better unfiltered. */
    SDL_SetTextureScaleMode(sr_texture,
        (BilinearFilter && !(GUIOn2 && !FilteredGUI)) ? SDL_SCALEMODE_LINEAR
                                                      : SDL_SCALEMODE_NEAREST);

    if (!sr_pixels) {
        sr_pixels = (unsigned short*)malloc(SR_MAXW * SR_MAXH * sizeof(unsigned short));
        if (!sr_pixels) {
            return false;
        }
    }
    memset(sr_pixels, 0, SR_MAXW * SR_MAXH * sizeof(unsigned short));

    SDL_SetWindowMouseGrab(sdl_window, FullScreen ? true : false);
    SDL_HideCursor(); // the emulator draws its own pointer
    SDL_SyncWindow(sdl_window);
    SDL_HideCursor(); // again once the compositor has the final surface
    SDL_SetRenderDrawColor(sr_renderer, 0, 0, 0, 255);
    SDL_RenderClear(sr_renderer);
    SDL_RenderPresent(sr_renderer);
    return true;
}

/* Everything but the window: what rebuilding for a new mode has to let go of.
   The window itself is kept, so a filter toggle does not blank the screen and
   build it again, and on Wayland the surface and its context survive. */
static void sr_release(void)
{
    if (sr_texture) {
        SDL_DestroyTexture(sr_texture);
        sr_texture = NULL;
    }
    if (sr_hdr_texture) {
        SDL_DestroyTexture(sr_hdr_texture);
        sr_hdr_texture = NULL;
    }
    if (sr_hdr) {
        free(sr_hdr);
        sr_hdr = NULL;
    }
    sr_hdr_active = 0;
    if (sr_renderer) {
        SDL_DestroyRenderer(sr_renderer);
        sr_renderer = NULL;
    }
    if (sr_pixels) {
        free(sr_pixels);
        sr_pixels = NULL;
    }
}

void sr_end(void)
{
    sr_release();
    sr_fullscreen = -1;
    if (sdl_window) {
        SDL_PumpEvents();
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
    }
}

void sr_clearwin(void)
{
    if (sr_renderer) {
        SDL_SetRenderDrawColor(sr_renderer, 0, 0, 0, 255);
        SDL_RenderClear(sr_renderer);
        SDL_RenderPresent(sr_renderer);
    }
}

/* One source line into the two output rows it occupies. */
static void sr_line(unsigned short* dst, int const line)
{
    unsigned short const* src1
        = (unsigned short*)vidbuffer + VID_FIRST + line * VID_STRIDE;
    unsigned short const* src2 = src1 + SR_FIELD2;
    /* Which array says what kind of line this is: the new engine keeps
       SpecialLine, the old one and the GUI keep hirestiledat. Both are indexed
       by scanline, and output row `line` is scanline line+1 - the same line
       VID_FIRST starts src1 on. */
    u1 const* const kinds
        = (GUIOn == 1 || newengen == 0) ? hirestiledat : SpecialLine;
    /* 2 for the 512 wide modes and 3 for hi-res mode 7, with bit 2 set on top
       when the screen is interlaced. It is never 1, so testing the magnitude
       put every hi-res line down the mode 7 branch and left the interlace
       bit - and a plain interlaced lo-res line, which is 4 - reading as one. */
    int const kind = kinds[line + 1];
    int i;

    if ((kind & 3) == 3) {
        /* Mode 7: the two fields are separate 256-wide lines, one above the
           other, so each is doubled across and kept on its own row. */
        for (i = 0; i < 256; i++) {
            dst[i * 2] = dst[i * 2 + 1] = src1[i];
            dst[SR_W + i * 2] = dst[SR_W + i * 2 + 1] = src2[i];
        }
        return;
    }
    if (kind & 4) {
        /* Interlaced: this field owns one row of the pair and the other keeps
           what the last field put there, which is what interlacing is. */
        unsigned short* const row = (cfield & 1) ? dst + SR_W : dst;

        if (kind & 3) {
            for (i = 0; i < 256; i++) {
                row[i * 2] = src1[i];
                row[i * 2 + 1] = src2[i];
            }
        } else {
            for (i = 0; i < 256; i++) {
                row[i * 2] = row[i * 2 + 1] = src1[i];
            }
        }
        return;
    }
    if (kind & 3) {
        /* 512 across: the fields interleave column by column. */
        for (i = 0; i < 256; i++) {
            dst[i * 2] = src1[i];
            dst[i * 2 + 1] = src2[i];
        }
        memcpy(dst + SR_W, dst, SR_W * sizeof(unsigned short));
        return;
    }
    for (i = 0; i < 256; i++) {
        dst[i * 2] = dst[i * 2 + 1] = src1[i];
    }
    memcpy(dst + SR_W, dst, SR_W * sizeof(unsigned short));
}

/* 565 holds sRGB, which is gamma encoded; a float texture in the linear
   colorspace holds light. Writing the one into the other unconverted would
   crush the mid-tones, so go through the transfer function. Only 32 and 64
   levels exist, so a table each is enough. */
static float sr_lin5[32];
static float sr_lin6[64];
static int sr_lin_ready = 0;

static float sr_srgb_to_linear(double const v)
{
    return (float)(v <= 0.04045 ? v / 12.92 : pow((v + 0.055) / 1.055, 2.4));
}

static void sr_build_linear(void)
{
    int i;

    for (i = 0; i < 32; i++) {
        sr_lin5[i] = sr_srgb_to_linear(i / 31.0);
    }
    for (i = 0; i < 64; i++) {
        sr_lin6[i] = sr_srgb_to_linear(i / 63.0);
    }
    sr_lin_ready = 1;
}

/* Widen the composed frame into linear light, adding the bloom without
   clipping it. The source is 15-bit, so this buys headroom above white rather
   than tonal resolution - stretching it would only make the banding easier to
   see. The spill is what uses the headroom, and it is added here in linear
   light, which is where light actually adds. */
static void sr_to_hdr(int const w, int const h)
{
    float const room = sr_hdr_headroom;
    int x, y;

    if (!sr_lin_ready) {
        sr_build_linear();
    }
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            unsigned const p = sr_pixels[(size_t)y * w + x];
            float const b = BloomLevel ? CrtBloomAt(x, y, w) : 0.0f;
            float* const o = sr_hdr + ((size_t)y * w + x) * 4;
            float c[3];
            int i;

            c[0] = sr_lin5[(p >> 11) & 0x1Fu];
            c[1] = sr_lin6[(p >> 5) & 0x3Fu];
            c[2] = sr_lin5[p & 0x1Fu];
            for (i = 0; i < 3; i++) {
                float const v = c[i] + b;

                o[i] = v > room ? room : v;
            }
            o[3] = 1.0f;
        }
    }
}

void sr_drawwin(void)
{
    int line;
    /* What this frame ends up filling, which the upload and the draw follow. */
    int w = SR_W, h = SR_H, dst_pitch = SR_W * 2;
    /* Output rows per source scanline, which is what the scanline pass steps
       by. The unfiltered path below doubles, whatever `resolutn` is. */
    int vscale = 2;
    int ntsc_drawn = 0;

    NGNoTransp = 0; // Set this value to 1 within the appropriate
    // Where a custom or hardware transparency routine would go. Only reachable
    // with newengen == 1; see ProcessTransparencies (video/c_ngtransp.c).
    UpdateVFrame();
    if (curblank || !CheckOGLMode() || !sr_renderer) {
        return;
    }

    /* Whichever filter is on draws at its own size, whatever the window is:
       the renderer scales the result out, so the filter picked is the filter
       drawn and the window size no longer decides which ones are reachable. */
    {
        VideoFilter const filter = VideoFilterGet();
        VideoFilterPicture const pic = VideoFilterOutput(filter);

        if (VideoFilterDraw(filter, sr_pixels, pic.w * 2,
                (size_t)SR_MAXW * SR_MAXH * sizeof(*sr_pixels))) {
            w = pic.w;
            h = pic.h;
            dst_pitch = pic.w * 2;
            vscale = pic.scale;
            ntsc_drawn = filter == VFILTER_NTSC;
        } else {
            for (line = 0; line < 224; line++) {
                sr_line(sr_pixels + line * 2 * SR_W, line);
            }
        }
    }

    /* Brightness and scanlines go over whatever was composed, so they combine
       with the hq and 2xSaI filters instead of only showing on an unfiltered
       picture. Scanlines are skipped over the NTSC filter, which dims alternate
       rows itself, but brightness still applies. */
    /* Before the scanlines dim it: what spills is a property of the picture,
       not of which row of the beam pattern a pixel happened to land on. */
    if (BloomLevel) {
        CrtBloomBuild(sr_pixels, w, h, w);
    }

    {
        int const scanlines = (sl_intensity != 0 && !ntsc_drawn);

        if (scanlines || sl_vibrancy) {
            CrtShade(sr_pixels, w, h, w, vscale, scanlines);
        }
    }

    sr_hdr_refresh();
    if (BloomLevel && !sr_hdr_active) {
        CrtBloomApply(sr_pixels, w, h, w); /* clipped into 565; HDR adds it in sr_to_hdr */
    }

    {
        SDL_Rect const dirty = { 0, 0, w, h };
        SDL_FRect const src = { 0.0f, 0.0f, (float)w, (float)h };
        SDL_Texture* const tex = sr_hdr_active ? sr_hdr_texture : sr_texture;

        if (sr_hdr_active) {
            sr_to_hdr(w, h);
            SDL_UpdateTexture(tex, &dirty, sr_hdr, w * 4 * (int)sizeof(float));
        } else {
            SDL_UpdateTexture(tex, &dirty, sr_pixels, dst_pitch);
        }
        SDL_RenderClear(sr_renderer);
        SDL_RenderTexture(sr_renderer, tex, &src, NULL);
#ifdef ZSNES_DEBUG_HOOKS
        /* Read back before presenting: afterwards the back buffer is the
           driver's business, not ours. */
        if (ZSnesFrameDumpWanted()) {
            SDL_Surface* const shot = SDL_RenderReadPixels(sr_renderer, NULL);

            ZSnesFrameDumpSurface(shot, "sr");
            SDL_DestroySurface(shot);
        }
#endif
        SDL_RenderPresent(sr_renderer);
    }
}
