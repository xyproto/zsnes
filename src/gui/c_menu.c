#include <string.h>

#include "../asm.h"
#include "../asm_call.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/c_execute.h"
#include "../cpu/execute.h"
#include "../endmem.h"
#include "../init.h"
#include "../ui.h"
#include "../vcache.h"
#include "../video/procvid.h"
#include "../video/procvidc.h"
#include "../zstate.h"
#include "c_menu.h"
#include "gui.h"
#include "guifuncs.h"
#include "menu.h"

#if !defined NO_PNG && defined __MSDOS__
#	include "../dos/dosintrf.h"
#endif

u1 NoInputRead;
u1 nextmenupopup;

static u1 MenuNoExit;
static u1 PrevMenuPos;


static void GUIBufferData(void)
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


static void GUIUnBuffer(void)
{
	// copy from spritetable
	u4 const n =
#ifdef __MSDOS__
		cbitmode != 1 ? 64000 :
#endif
		129536;
	memcpy(vidbuffer + 4 * 384, spritetablea + 4 * 384, n);
}


static void menudrawbox8b(void)
{
#ifdef __MSDOS__
	if (cbitmode != 1)
	{
		// draw a small blue box with a white border
		u1* buf = vidbuffer + MenuDisplace + 40 + 20 * 288;
		u4  h   = 95;
		do
		{
			memset(buf, 144, 150);
			buf += 288;
		}
		while (--h != 0);

		// Draw lines
		drawhline(vidbuffer + MenuDisplace +  40 +  20 * 288, 150, 128);
		drawvline(vidbuffer + MenuDisplace +  40 +  20 * 288,  95, 128);
		drawhline(vidbuffer + MenuDisplace +  40 + 114 * 288, 150, 128);
		drawhline(vidbuffer + MenuDisplace +  40 +  32 * 288, 150, 128);
		drawvline(vidbuffer + MenuDisplace + 189 +  20 * 288,  95, 128);
		asm_call(menudrawcursor8b);

		OutputGraphicString(vidbuffer + MenuDisplace + 45 +  23 * 288, menudrawbox_string);
		OutputGraphicString(vidbuffer + MenuDisplace + 45 +  35 * 288, menudrawbox_stringa);
		OutputGraphicString(vidbuffer + MenuDisplace + 45 +  45 * 288, FPSOn & 1 ? menudrawbox_stringc : menudrawbox_stringb);
		OutputGraphicString(vidbuffer + MenuDisplace + 45 +  55 * 288, menudrawbox_stringd);
		OutputGraphicString(vidbuffer + MenuDisplace + 45 +  65 * 288, menudrawbox_stringe);
		OutputGraphicString(vidbuffer + MenuDisplace + 45 +  75 * 288, menudrawbox_stringf);
		OutputGraphicString(vidbuffer + MenuDisplace + 45 +  85 * 288, menudrawbox_stringg);
		OutputGraphicString(vidbuffer + MenuDisplace + 45 +  95 * 288, menudrawbox_stringh);
		OutputGraphicString(vidbuffer + MenuDisplace + 45 + 105 * 288, menudrawbox_stringi);
		copyvid();
	}
	else
#endif
	{
		asm_call(menudrawbox16b);
	}
}


#ifdef __MSDOS__
static inline void SetPal(u1 const i, u1 const r, u1 const g, u1 const b)
{
	outb(0x03C8, i);
	outb(0x03C9, r);
	outb(0x03C9, g);
	outb(0x03C9, b);
}
#endif


void showmenu(void)
{
	for (;;)
	{
#ifdef __MSDOS__
		if (cbitmode != 1)
		{
			u1* buf = vidbuffer + 100000;
			outb(0x03C7, 0);
			*buf++ = 12;
			u4 n = 768;
			do *buf++ = inb(0x03C9) << 2; while (--n != 0);

			// set palette of colors 128,144, and 160 to white, blue, and red
			SetPal(128, 63, 63, 63);
			SetPal(144,  0,  0, 50);
			SetPal(160, 45,  0,  0);
		}
#endif

		ForceNonTransp = 1;
		NoInputRead    = 0;
		if (SSKeyPressed == 1)
		{
			SSKeyPressed = 0;
			asm_call(saveimage);
		}
		else if (SPCKeyPressed == 1)
		{
			goto savespckey;
		}
		else if (pressed[14] & 1)
		{
			asm_call(saveimage);
		}
		else
		{
			menucloc = 0;
			if (nextmenupopup != 0)
			{
				pressed[0x1C] = 0;
				switch (PrevMenuPos)
				{
					default: menucloc = 40 * 288; break;
					case 1:  menucloc = 50 * 288; break;
					case 2:  menucloc = 60 * 288; break;
				}
			}
			if (PrevMenuPos == 3) menucloc = 70 * 288;

			char const* fmt = " BMP";
#ifndef NO_PNG
			if (ScreenShotFormat != 0)
			{
#	ifdef __MSDOS__
				if (GUI16VID[cvidmode] != 1)
				{
					ScreenShotFormat = 0;
				}
				else
#	endif
				{
					fmt = " PNG";
				}
			}
#endif
			memcpy(menudrawbox_stringi + 13, fmt, 4);

			nextmenupopup = 0;
			menu16btrans  = 0;
			pressed[ 1]   = 0;
			pressed[59]   = 0;
			curblank      = 0;
			GUIBufferData();
			// Draw box
			menudrawbox8b();
			menudrawbox8b(); // XXX twice?
			if (newengen != 0) GUIOn = 1;
			copyvid();
			StopSound();
			for (;;)
			{
				//GUIUnBuffer();
				menudrawbox8b();
				copyvid();

				JoyRead();
				if (Check_Key() == 0) continue;
				u1 const key = Get_Key();
				if (key == 0)
				{
					u1 const ext = Get_Key();
					if (ext == 72)
					{
						if (menucloc == 0) menucloc += 80 * 288;
						menucloc -= 10 * 288;
						menudrawbox8b();
					}
					else if (ext == 80)
					{
						if (menucloc == 70 * 288) menucloc -= 80 * 288;
						menucloc += 10 * 288;
						menudrawbox8b();
						copyvid();
					}
				}
				else if (key == 27) goto exitloop;
				else if (key == 13) break;
			}
			GUIUnBuffer();
			copyvid();
			if (menucloc == 0) asm_call(saveimage);
			if (menucloc == 40 * 288)
			{
				asm_call(saveimage);
				ExecExitOkay  = 0;
				nextmenupopup = 3;
				NoInputRead   = 1;
				t1cc          = 0;
				PrevMenuPos   = 0;
			}
			if (menucloc == 50 * 288)
			{
				ExecExitOkay  = 0;
				nextmenupopup = 3;
				NoInputRead   = 1;
				t1cc          = 0;
				PrevMenuPos   = 1;
			}
			if (menucloc == 70 * 288)
			{
#ifdef __MSDOS__
				if (cbitmode != 0)
#endif
				{
					ScreenShotFormat ^= 1;
					MenuNoExit        = 1;
					ExecExitOkay      = 0;
					nextmenupopup     = 1;
					NoInputRead       = 1;
					t1cc              = 0;
					PrevMenuPos       = 3;
				}
			}
			if (menucloc == 60 * 288)
			{
				MenuNoExit    = 1;
				ExecExitOkay  = 0;
				nextmenupopup = 1;
				NoInputRead   = 1;
				t1cc          = 0;
				PrevMenuPos   = 2;
				if (MenuDisplace != 0)
				{
					MenuDisplace   = 0;
					MenuDisplace16 = 0;
				}
				else
				{
					MenuDisplace   = 90 * 288;
					MenuDisplace16 = 90 * 288 * 2;
				}
			}
			if (menucloc == 10 * 288)
			{
				if (frameskip != 0)
				{
					Msgptr    = "NEED AUTO FRAMERATE ON";
					MessageOn = MsgCount;
				}
				else
				{
					FPSOn ^= 1;
				}
			}
			if (menucloc == 20 * 288)
			{
savespckey:
				if (spcon != 0)
				{
					Msgptr    = "SEARCHING FOR SONG START.";
					MessageOn = MsgCount;
					copyvid();
					SPCSave = 1;
					asm_call(breakatsignb);
					SPCSave = 0;
					savespcdata();

					curblank  = 0x40;
					Msgptr    = spcsaved;
					MessageOn = MsgCount;
				}
				else
				{
					Msgptr    = "SOUND MUST BE ENABLED.";
					MessageOn = MsgCount;
				}
			}
			if (menucloc == 30 * 288)
			{
				dumpsound();
				Msgptr    = "BUFFER SAVED AS SOUNDDMP.RAW";
				MessageOn = MsgCount;
			}
			if (SPCKeyPressed == 1)
			{
				SPCKeyPressed = 0;
			}
			else
			{
exitloop:
				GUIUnBuffer();
				copyvid();
#ifdef __MSDOS__
				if (cbitmode != 1)
				{
					u1 const* buf = vidbuffer + 100000 + 1;
					outb(0x03C8, 0);
					u4 n = 768;
					do outb(0x03C9, *buf++ >> 2); while (--n != 0);
				}
#endif
			}
		}
		u1* i = pressed;
		u4  n = 256; // XXX maybe should be lengthof(pressed)
		do
		{
			if (*i == 1) *i = 2;
			++i;
		}
		while (--n != 0);
		StartSound();
		ForceNonTransp = 0;
		GUIOn          = 0;
		Clear2xSaIBuffer();
		if (MenuNoExit != 1) break;
		MenuNoExit = 0;
	}
	continueprognokeys();
}
