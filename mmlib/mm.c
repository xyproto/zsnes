/*
Copyright (c) 2003-2007 Ryan C. Gordon and others.

http://icculus.org/manymouse/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from
the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software in a
product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.

    Ryan C. Gordon <icculus@icculus.org>
*/

//ManyMouse foundation code; apps talks to this and it talks to the lowlevel
//code for various platforms.

#include <stdlib.h>
#include "mm.h"

static const char *manymouse_copyright =
    "ManyMouse " MANYMOUSE_VERSION " (c) 2003-2007 Ryan C. Gordon.";

extern const ManyMouseDriver ManyMouseDriver_windows;
extern const ManyMouseDriver ManyMouseDriver_evdev;
extern const ManyMouseDriver ManyMouseDriver_hidmanager;
extern const ManyMouseDriver ManyMouseDriver_xinput;

static const ManyMouseDriver *mice_drivers[] =
{
    #if SUPPORT_XINPUT
    &ManyMouseDriver_xinput,
    #endif
    #ifdef __linux__
    &ManyMouseDriver_evdev,
    #endif
    #if ((defined _WIN32) || defined(__CYGWIN__))
    &ManyMouseDriver_windows,
    #endif
    #if ((defined(__MACH__)) && (defined(__APPLE__)))
    &ManyMouseDriver_hidmanager,
    #endif
    NULL
};


static const ManyMouseDriver *driver = NULL;

int ManyMouse_Init(void)
{
    int i;
    int retval = -1;

    /* impossible test to keep manymouse_copyright linked into the binary. */
    if (manymouse_copyright == NULL)
        return(-1);

    if (driver != NULL)
        return(-1);

    for (i = 0; mice_drivers[i]; i++)
    {
        int mice = mice_drivers[i]->init();

        if (mice > retval)
            retval = mice; /* may just move from "error" to "no mice found". */

        if (mice > 0)
        {
            driver = mice_drivers[i];
            break;
        } /* if */
    } /* for */

    return(retval);
} /* ManyMouse_Init */


void ManyMouse_Quit(void)
{
    if (driver != NULL)
        driver->quit();
    driver = NULL;
} /* ManyMouse_Quit */


const char *ManyMouse_DeviceName(unsigned int index)
{
    if (driver != NULL)
        return(driver->name(index));
    return(NULL);
} /* ManyMouse_PollEvent */


int ManyMouse_PollEvent(ManyMouseEvent *event)
{
    if (driver != NULL)
        return(driver->poll(event));
    return(0);
} /* ManyMouse_PollEvent */

/* end of manymouse.c ... */
