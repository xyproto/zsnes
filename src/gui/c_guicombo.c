#include "c_guicombo.h"
#include "guiwindp.h"


void ComboClip(void)
{
	u4 eax = GUINumCombo;
	while ((eax & 0xFF) < 42) GUIComboData[eax++] = 0;
}
