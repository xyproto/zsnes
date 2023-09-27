/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "gblhdr.h"
#include "asm_call.h"
#include "c_intrf.h"
#include "cfg.h"
#include "gui/c_gui.h"
#include "netplay/znet.h"
#include "init.h"
#include "initc.h"
#include "input.h"
#include "ui.h"
#include "ver.h"
#include "zpath.h"

#ifdef QT_DEBUGGER
#include "debugger/load.h"
#endif

#undef main

extern unsigned char ZMVZClose, ZMVRawDump;
extern unsigned char ForcePal;
extern unsigned char MovieForcedLengthEnabled;
extern unsigned int MovieForcedLength;
extern char *STCart2;
void zstart();
void zexit_error();

char *ZVERSION = ZVER;

static void display_start_message() {
	size_t lines_out = 0;
	bool tty = isatty(fileno(stdout));
}

static void handle_params(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++) {
		if (argv[i]) {
			if (strcmp(argv[i], "-netdup") == 0) {
				netdupValue = (argv[i + 1][0] - 0x30) * 2;
				currentNetdup = netdupValue;
				i++;
				printf("Netdupvalue set to %d\n", netdupValue);
			} else if (strcmp(argv[i], "-host") == 0) {
				StartServer(argv[i + 1]);
				i++;
			} else if (strcmp(argv[i], "-join") == 0) {
				StartClient(argv[i + 1]);
				i++;
			} else {
				if (!init_rom_path(argv[i])) {
					printf("Could not load: %s\n", argv[i]);
				} else {
					if ((STCart2 = argv[i + 1])) { // Sufami Turbo second cart
						char *p;

						natify_slashes(STCart2);
						p = strrchr(STCart2, DIR_SLASH_C);
						if (!p) {
							p = STCart2;
						} else {
							p++;
						}
						strcpy(ZSaveST2Name, p);
						setextension(ZSaveST2Name, "srm");
					}
				}
			}
		}
	}
}

static void ZCleanup() {
	void SPC7110_deinit_decompression_state();
	void deinit_paths();
	void deallocmem();
	void DeallocRewindBuffer();
	void DeallocPauseFrame();
	void DeallocSystemVars();
	void free_all_file_lists();
	void UnloadSDL();

	SPC7110_deinit_decompression_state();
	deinit_paths();
	deallocmem();
	DeallocRewindBuffer();
	DeallocPauseFrame();
	DeallocSystemVars();
	free_all_file_lists();
	UnloadSDL();
}

int main(int const argc, char **const argv) {
	if (init_paths(*argv)) {
		display_start_message();
		handle_params(argc, argv);

		atexit(ZCleanup);
		srand(time(0));

#ifdef QT_DEBUGGER
		if (debugger) {
			debug_main();
		}
#endif
		zstart();
	}

	return 0;
}