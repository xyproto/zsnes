/*
 * draw16x816t and draw8x816toffset, ported from video/makev16t.asm.
 *
 * On their own because three difftests substitute stubs for them to observe
 * which way a dispatcher branched - see video/c_mv16draw.h. The targets that
 * want the real ones link this file; the ones that stub them do not.
 */

#include "c_mv16draw.h"

extern u4 THAX, THBX, THCX, THDX, THSI, THDI, THBP, THTail;
extern u4 TOAX, TOBX, TOCX, TODX, TOSI, TODI, TOBP, TOTail;

void c_draw16x816t(void);
void c_draw8x816toffset(void);

TILE_LEAF(draw16x816t, TH, c_draw16x816t)
TILE_LEAF(draw8x816toffset, TO, c_draw8x816toffset)
