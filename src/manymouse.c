/*
 * ManyMouse foundation code; apps talks to this and it talks to the lowlevel
 *  code for various platforms.
 *
 * Please see the file LICENSE in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#include <stdlib.h>
#include "manymouse.h"

static const char *manymouse_copyright =
    "ManyMouse " MANYMOUSE_VERSION " (c) 2005 Ryan C. Gordon.";

extern const ManyMouseDriver ManyMouseDriver_windows;
extern const ManyMouseDriver ManyMouseDriver_evdev;
extern const ManyMouseDriver ManyMouseDriver_mousedev;
extern const ManyMouseDriver ManyMouseDriver_hidmanager;
extern const ManyMouseDriver ManyMouseDriver_xinput;

static const ManyMouseDriver *mice_drivers[] =
{
    #if SUPPORT_XINPUT
    &ManyMouseDriver_xinput,
    #endif
    #if ((defined _WIN32) || defined(__CYGWIN__))
    &ManyMouseDriver_windows,
    #endif
    #ifdef __linux__
    &ManyMouseDriver_evdev,
    /*&ManyMouseDriver_mousedev,*/
    #endif
    #if ( (defined(__MACH__)) && (defined(__APPLE__)) )
    &ManyMouseDriver_hidmanager,
    #endif
    NULL
};


static const ManyMouseDriver *driver = NULL;

int ManyMouse_Init(void)
{
    int i;

    /* impossible test to keep manymouse_copyright linked into the binary. */
    if ((char *) driver == (const char *) manymouse_copyright)
        return(-1);

    if (driver != NULL)
        return(-1);

    for (i = 0; mice_drivers[i]; i++)
    {
        int mice = mice_drivers[i]->init();
        if (mice >= 0)
        {
            driver = mice_drivers[i];
            return(mice);
        } /* if */
    } /* for */

    return(-1);
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

