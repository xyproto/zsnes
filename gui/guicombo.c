/*
 * Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
 *
 * http://www.zsnes.com
 * http://sourceforge.net/projects/zsnes
 * https://zsnes.bountysource.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>

#include "../cfg.h"
#include "../cpu/execute.h"
#include "c_guiwindp.h"
#include "gui.h"
#include "guicombo.h"
#include "guiwindp.h"

void ComboClip(void)
{
    u4 eax = GUINumCombo;
    while ((eax & 0xFF) < 42)
        GUIComboData[eax++] = 0;
}

void ComboAdder(void)
{
    if (romloadskip != 0 && GUIComboGameSpec != 0)
        return;

    ComboClip();
    // copy data to c
    ComboData* const c = &(GUIComboGameSpec == 0 ? CombinDataGlob : CombinDataLocl)[NumCombo];
    memcpy(c->name, GUIComboTextH, sizeof(c->name)); // copy name
    memcpy(c->combo, GUIComboData, sizeof(c->combo)); // copy combination code
    c->key = GUIComboKey;
    c->player = GUIComboPNum;
    c->ff = GUIComboLHorz;

    GUIccombcursloc = NumCombo;
    s4 const eax = (s4)NumCombo - 7;
    if ((s4)GUIccombviewloc < eax)
        GUIccombviewloc = eax;
    *(GUIComboGameSpec == 0 ? &NumComboGlob : &NumComboLocl) = ++NumCombo;

    GUIComboTextH[0] = '\0';
    GUINumCombo = 0;
    GUIComboKey = 0;
}

void ComboReplace(void)
{
    ComboClip();
    ComboData* const eax = &(GUIComboGameSpec == 0 ? CombinDataGlob : CombinDataLocl)[GUIccombcursloc];
    // copy data to eax
    memcpy(eax->name, GUIComboTextH, sizeof(eax->name)); // copy name
    memcpy(eax->combo, GUIComboData, sizeof(eax->combo)); // copy combination code
    eax->key = GUIComboKey;
    eax->player = GUIComboPNum;
    eax->ff = GUIComboLHorz;
}

void ComboRemoval(void)
{
    ComboData* const c = (GUIComboGameSpec == 0 ? CombinDataGlob : CombinDataLocl) + GUIccombcursloc;
    u4 const ecx = NumCombo - GUIccombcursloc - 1;
    if ((s4)ecx > 0)
        memmove(c, c + 1, sizeof(*c) * ecx);

    u4 eax = --NumCombo;
    if (eax != 0)
        --eax;
    if (GUIccombviewloc > eax)
        GUIccombviewloc = eax;
    if (GUIccombcursloc > eax)
        GUIccombcursloc = eax;
    *(GUIComboGameSpec == 0 ? &NumComboGlob : &NumComboLocl) = NumCombo;
}
