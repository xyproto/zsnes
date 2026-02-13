#ifndef ZSNES_LINUX_SDL_COMPAT_H
#define ZSNES_LINUX_SDL_COMPAT_H

#ifdef __SDL3__

#ifndef SDL_ENABLE
#define SDL_ENABLE 1
#endif

#ifndef SDL_DISABLE
#define SDL_DISABLE 0
#endif

#ifndef SDL_INIT_TIMER
#define SDL_INIT_TIMER 0u
#endif

static inline int z_sdl_num_joysticks(void)
{
    int count = 0;
    SDL_JoystickID* ids = SDL_GetJoysticks(&count);
    SDL_free(ids);
    return count;
}

static inline SDL_Joystick* z_sdl_joystick_open(int index)
{
    int count = 0;
    SDL_JoystickID* ids = SDL_GetJoysticks(&count);
    SDL_Joystick* joystick = NULL;

    if (ids && index >= 0 && index < count) {
        joystick = SDL_OpenJoystick(ids[index]);
    }
    SDL_free(ids);
    return joystick;
}

static inline int z_sdl_joystick_event_state(int state)
{
    SDL_SetJoystickEventsEnabled(state == SDL_ENABLE);
    return state;
}

static inline void z_sdl_set_window_grab(SDL_Window* window, bool grabbed)
{
    SDL_SetWindowMouseGrab(window, grabbed);
    SDL_SetWindowKeyboardGrab(window, grabbed);
}

static inline SDL_Surface* z_sdl_create_rgb_surface(Uint32 flags, int width, int height, int depth,
    Uint32 rmask, Uint32 gmask, Uint32 bmask, Uint32 amask)
{
    (void)flags;
    SDL_PixelFormat format = SDL_GetPixelFormatForMasks(depth, rmask, gmask, bmask, amask);

    if (format == SDL_PIXELFORMAT_UNKNOWN) {
        return NULL;
    }
    return SDL_CreateSurface(width, height, format);
}

static inline Uint32 z_sdl_get_relative_mouse_state(int* x, int* y)
{
    float fx = 0.0f;
    float fy = 0.0f;
    SDL_MouseButtonFlags state = SDL_GetRelativeMouseState(&fx, &fy);

    if (x) {
        *x = (int)fx;
    }
    if (y) {
        *y = (int)fy;
    }
    return (Uint32)state;
}

static inline int z_sdl_show_cursor(int toggle)
{
    if (toggle == SDL_DISABLE) {
        return SDL_HideCursor() ? 1 : 0;
    }
    return SDL_ShowCursor() ? 1 : 0;
}

#define SDL_NumJoysticks z_sdl_num_joysticks
#define SDL_JoystickOpen z_sdl_joystick_open
#define SDL_JoystickEventState z_sdl_joystick_event_state
#define SDL_SetWindowGrab z_sdl_set_window_grab
#define SDL_CreateRGBSurface z_sdl_create_rgb_surface
#define SDL_GetRelativeMouseState z_sdl_get_relative_mouse_state
#define SDL_ShowCursor z_sdl_show_cursor
#define SDL_CreateWindow(title, x, y, w, h, flags) SDL_CreateWindow((title), (w), (h), (flags))

#endif

#endif
