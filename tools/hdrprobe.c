/* Does this machine's SDL have a route to HDR?
 *
 * unix/sdl_render.c presents through a linear float surface when the monitor
 * is in HDR mode, so the bloom's spill can go above white instead of being
 * clipped at it. Two things have to be true for that: the display must report
 * HDR, and some render driver must accept SDL_COLORSPACE_SRGB_LINEAR. SDL's
 * own documentation lists only the direct3d11, direct3d12 and metal renderers
 * as supporting it, which would leave Linux out - but that list may lag what
 * the Wayland backend can do, so ask the machine rather than the manual.
 *
 *   cc -o hdrprobe tools/hdrprobe.c $(pkg-config --cflags --libs sdl3)
 *   ./hdrprobe
 *
 * Run it on the machine with the HDR display, in the session that drives it:
 * the answer depends on the compositor, not just on the GPU.
 */
#include <SDL3/SDL.h>
#include <stdio.h>
int main(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) { printf("SDL_Init: %s\n", SDL_GetError()); return 1; }
    printf("SDL %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    printf("video driver: %s\n", SDL_GetCurrentVideoDriver());
    int n = SDL_GetNumRenderDrivers();
    printf("render drivers (%d):", n);
    for (int i = 0; i < n; i++) printf(" %s", SDL_GetRenderDriver(i));
    printf("\n");
    int nd = 0; SDL_DisplayID* d = SDL_GetDisplays(&nd);
    for (int i = 0; i < nd; i++) {
        SDL_PropertiesID dp = SDL_GetDisplayProperties(d[i]);
        printf("display %d \"%s\": HDR_enabled=%s\n", i + 1, SDL_GetDisplayName(d[i]),
            SDL_GetBooleanProperty(dp, SDL_PROP_DISPLAY_HDR_ENABLED_BOOLEAN, false) ? "yes" : "no");
    }
    SDL_free(d);
    SDL_Window* w = SDL_CreateWindow("hdr", 320, 240, 0);
    if (!w) { printf("no window: %s\n", SDL_GetError()); return 1; }
    for (int i = 0; i < n; i++) {
        SDL_PropertiesID p = SDL_CreateProperties();
        SDL_SetPointerProperty(p, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, w);
        SDL_SetStringProperty(p, SDL_PROP_RENDERER_CREATE_NAME_STRING, SDL_GetRenderDriver(i));
        SDL_SetNumberProperty(p, SDL_PROP_RENDERER_CREATE_OUTPUT_COLORSPACE_NUMBER, SDL_COLORSPACE_SRGB_LINEAR);
        SDL_Renderer* r = SDL_CreateRendererWithProperties(p);
        SDL_DestroyProperties(p);
        printf("  %-10s linear colorspace: %s", SDL_GetRenderDriver(i), r ? "accepted" : "refused");
        if (r) {
            SDL_PropertiesID rp = SDL_GetRendererProperties(r);
            printf("  HDR_enabled=%s headroom=%.2f",
                SDL_GetBooleanProperty(rp, SDL_PROP_RENDERER_HDR_ENABLED_BOOLEAN, false) ? "yes" : "no",
                SDL_GetFloatProperty(rp, SDL_PROP_RENDERER_HDR_HEADROOM_FLOAT, 1.0f));
            SDL_Texture* t = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA128_FLOAT, SDL_TEXTUREACCESS_STREAMING, 64, 64);
            printf("  float texture: %s", t ? "ok" : "refused");
            if (t) SDL_DestroyTexture(t);
            SDL_DestroyRenderer(r);
        }
        printf("\n");
    }
    SDL_DestroyWindow(w); SDL_Quit();
    return 0;
}
