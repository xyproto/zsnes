#ifdef __UNIXSDL__
#include "../gblhdr.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#endif
#include "../gblvars.h"

//Start of execute.asm goodness
extern uint16_t t1cc; // not sure about this one
extern uint32_t nextframe; // framecounter for frameskipping

void Game60hzcall(void)
{
    t1cc++;
    nextframe++;
}
