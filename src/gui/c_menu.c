#include <string.h>

#include "../endmem.h"
#include "../ui.h"
#include "c_menu.h"


void GUIBufferData(void)
{
	// copy to spritetable
	u4 const n =
#ifdef __MSDOS__
		cbitmode != 1 ? 64000 :
#endif
		129536;
	memcpy(spritetablea + 4 * 384, vidbuffer + 4 * 384, n);
	memset(sprlefttot, 0, sizeof(sprlefttot));
	memset(sprleftpr,  0, sizeof(sprleftpr));
	memset(sprleftpr1, 0, sizeof(sprleftpr1));
	memset(sprleftpr2, 0, sizeof(sprleftpr2));
	memset(sprleftpr3, 0, sizeof(sprleftpr3));
}


void menu_GUIUnBuffer(void)
{
	// copy from spritetable
	u4 const n =
#ifdef __MSDOS__
		cbitmode != 1 ? 64000 :
#endif
		129536;
	memcpy(vidbuffer + 4 * 384, spritetablea + 4 * 384, n);
}
