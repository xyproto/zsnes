#include <string.h>

#include "../cfg.h"
#include "../cpu/execute.h"
#include "c_guicombo.h"
#include "gui.h"
#include "guiwindp.h"


void ComboClip(void)
{
	u4 eax = GUINumCombo;
	while ((eax & 0xFF) < 42) GUIComboData[eax++] = 0;
}


void ComboAdder(void)
{
	if (romloadskip != 0 && GUIComboGameSpec != 0) return;

	ComboClip();
	// copy data to c
	ComboData* const c = &(GUIComboGameSpec == 0 ? CombinDataGlob : CombinDataLocl)[NumCombo];
	memcpy(c->name,  GUIComboTextH, sizeof(c->name));  // copy name
	memcpy(c->combo, GUIComboData,  sizeof(c->combo)); // copy combination code
	c->key    = GUIComboKey;
	c->player = GUIComboPNum;
	c->ff     = GUIComboLHorz;

	GUIccombcursloc = NumCombo;
	s4 const eax = (s4)NumCombo - 7;
	if ((s4)GUIccombviewloc < eax) GUIccombviewloc = eax;
	*(GUIComboGameSpec == 0 ? &NumComboGlob : &NumComboLocl) = ++NumCombo;

	GUIComboTextH[0] = '\0';
	GUINumCombo      = 0;
	GUIComboKey      = 0;
}


void ComboReplace(void)
{
	ComboClip();
	ComboData* const eax = &(GUIComboGameSpec == 0 ? CombinDataGlob : CombinDataLocl)[GUIccombcursloc];
	// copy data to eax
	memcpy(eax->name,  GUIComboTextH, sizeof(eax->name));  // copy name
	memcpy(eax->combo, GUIComboData,  sizeof(eax->combo)); // copy combination code
	eax->key    = GUIComboKey;
	eax->player = GUIComboPNum;
	eax->ff     = GUIComboLHorz;
}
