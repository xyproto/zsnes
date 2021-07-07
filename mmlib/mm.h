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

//ManyMouse main header. Include this from your app.

#ifndef _INCLUDE_MANYMOUSE_H_
#define _INCLUDE_MANYMOUSE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MANYMOUSE_VERSION "0.0.1"

typedef enum
{
    MANYMOUSE_EVENT_ABSMOTION = 0,
    MANYMOUSE_EVENT_RELMOTION,
    MANYMOUSE_EVENT_BUTTON,
    MANYMOUSE_EVENT_SCROLL,
    MANYMOUSE_EVENT_DISCONNECT,
    MANYMOUSE_EVENT_MAX
} ManyMouseEventType;

typedef struct
{
    ManyMouseEventType type;
    unsigned int device;
    unsigned int item;
    int value;
    int minval;
    int maxval;
} ManyMouseEvent;


/* internal use only. */
typedef struct
{
    int (*init)(void);
    void (*quit)(void);
    const char *(*name)(unsigned int index);
    int (*poll)(ManyMouseEvent *event);
} ManyMouseDriver;


int ManyMouse_Init(void);
void ManyMouse_Quit(void);
const char *ManyMouse_DeviceName(unsigned int index);
int ManyMouse_PollEvent(ManyMouseEvent *event);

#ifdef __cplusplus
}
#endif

#endif  /* !defined _INCLUDE_MANYMOUSE_H_ */

/* end of manymouse.h ... */

