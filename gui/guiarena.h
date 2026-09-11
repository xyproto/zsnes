/* The GUI's allocator.
 *
 * Nothing the GUI holds is large, hot, or long lived: the file lists are
 * rebuilt whenever the browser changes directory and thrown away whole. So
 * they are not freed a piece at a time. GUIAlloc hands out memory from a chain
 * of blocks and GUIArenaReset drops the lot at once, which is a collector of
 * sorts and is all this needs - a list cannot be leaked, freed twice, or read
 * after it is gone, because no single object is ever freed. */

#ifndef ZSNES_GUI_GUIARENA_H
#define ZSNES_GUI_GUIARENA_H

#include <stddef.h>

/* Zeroed, pointer-aligned, and NULL only if the machine is out of memory. */
void* GUIAlloc(size_t bytes);
char* GUIStrdup(char const* s);

/* Everything handed out since the last reset becomes invalid. The blocks are
   kept for the next round rather than returned. */
void GUIArenaReset(void);

/* Give the blocks back, at shutdown. */
void GUIArenaFree(void);

#endif
