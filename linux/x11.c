/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "../gblhdr.h"
#include "x11.h"

#include <SDL_syswm.h>


#ifdef SDL_VIDEO_DRIVER_X11
#include <dlfcn.h>
#include <unistd.h>
#include <sys/wait.h>
#include "safelib.h"


static Display *SDL_Display = 0;
static Window SDL_Window = 0; //unsigned long

static void *libXtst = 0;
int (*XTestFakeKeyEvent)(Display *, unsigned int, Bool, unsigned long) = 0;

static bool XScreenSaverSwitchedOff = false;

void *dlopen_family(const char *lib, int flag)
{
  void *p = dlopen(lib, flag);
  if (!p)
  {
    char buffer[256];
    unsigned int i;
    for (i = 0; i < 10; i++)
    {
      snprintf(buffer, sizeof(buffer), "%s.%u", lib, i);
      if ((p = dlopen(buffer, flag)))
      {
        break;
      }
    }
  }
  return(p);
}

void X11_Init()
{
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);

  if (!SDL_Display && (SDL_GetWMInfo(&info) > 0) && (info.subsystem == SDL_SYSWM_X11))
  {
    SDL_Display = info.info.x11.display;
    SDL_Window = info.info.x11.window;

    libXtst = dlopen_family("libXtst.so", RTLD_LAZY);
    if (libXtst)
    {
      XTestFakeKeyEvent = dlsym(libXtst, "XTestFakeKeyEvent");
    }
    else
    {
      puts(dlerror());
    }

    atexit(X11_Deinit);
  }
}

void X11_Deinit()
{
  XScreenSaverOn();
  if (libXtst)
  {
    XTestFakeKeyEvent = 0;

    dlclose(libXtst);
    libXtst = 0;
  }
  SDL_Display = 0;
}


static bool xdg_screensaver(char *command)
{
  bool success = false;
  if (SDL_Window)
  {
    pid_t pid = safe_fork(0, 0);
    if (pid != -1) //Successful Fork
    {
      if (pid) //Parent
      {
        int status;
        waitpid(pid, &status, 0);
        success = WIFEXITED(status) && !WEXITSTATUS(status);
      }
      else //Child
      {
        char numbuffer[21];
        char *const arglist[] = { "xdg-screensaver", command, numbuffer, 0 };

        snprintf(numbuffer, sizeof(numbuffer), "%lu", (unsigned long)SDL_Window); //Cast just in case
        execvp(arglist[0], arglist);
        _exit(-1);
      }
    }
  }
  return(success);
}

bool XScreenSaverOff()
{
  if (!XScreenSaverSwitchedOff)
  {
    XScreenSaverSwitchedOff = xdg_screensaver("suspend");
  }
  return(XScreenSaverSwitchedOff);
}

bool XScreenSaverOn()
{
  if (XScreenSaverSwitchedOff)
  {
    XScreenSaverSwitchedOff = !xdg_screensaver("resume");
  }
  return(!XScreenSaverSwitchedOff);
}


void CircumventXScreenSaver()
{
  if (XTestFakeKeyEvent)
  {
    static time_t last_time = 0;
    time_t current_time = time(0);
    if ((current_time - 50) > last_time) //Screensaver can be as low as every 60 seconds, preempt it at 50
    {
      XTestFakeKeyEvent(SDL_Display, 255, True, 0);
      XSync(SDL_Display, False);
      XTestFakeKeyEvent(SDL_Display, 255, False, 0);
      XSync(SDL_Display, False);

      last_time = current_time;
    }
  }
}


#else

void X11_Init() {}
void X11_Deinit() {}

bool XScreenSaverOff() { return(false); }
bool XScreenSaverOn() { return(false); }

void CircumventXScreenSaver() {}

#endif
