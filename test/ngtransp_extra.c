/* Storage for the two emulator-wide tables video/c_ngtransp.c and
 * video/c_ng2gate.c reference. ui.c owns them in the real build, and no
 * newgfx16 difftest drives the paths that read them - both sides of the
 * comparison share this one copy, so zeros are enough. */
#include <stdint.h>

typedef uint8_t u1;
typedef uint16_t u2;

u2 fulladdtab[65537]; /* one past: the color math loads a dword */
u1* vidbuffer;
