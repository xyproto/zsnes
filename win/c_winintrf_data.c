/* C port of win/winintrf.asm: global Windows input/video state variables.
 * win/winintrf.asm contains only SECTION .data declarations — no code.
 */

#include "../types.h"

u4 CurKeyPos      = 0;
u4 CurKeyReadPos  = 0;
u4 KeyBuffer[16]  = {0};
u4 converta       = 0;
