/* The GUI's allocator. Nothing it holds is large, hot or long lived - the
 * browser's file lists are rebuilt whole whenever the directory changes - so
 * GUIAlloc bump-allocates from a chain of blocks and GUIArenaReset drops the
 * lot. Nothing is freed individually, so nothing can be leaked, double-freed
 * or read after it is gone. */

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
