/*
Copyright (C) 1997-2005 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

-------------
Initial Linux Command Line Parsing by EvilTypeGuy (drevil@warpcore.org) April 2001
*/

#include "gblhdr.h"

#define STUB_FUNCTION fprintf(stderr,"STUB: %s at " __FILE__ ", line %d, thread %d\n",__FUNCTION__,__LINE__,getpid())
#define DWORD unsigned long
#define _MAX_PATH 80
#define _MAX_DRIVE 80
#define _MAX_DIR 80
#define _MAX_FNAME 80
#define _MAX_EXT 80

extern void zstart(void);
extern void DosExit(void);
extern void ConvertJoyMap1(void);
extern void ConvertJoyMap2(void);
extern void displayparams(void);
extern void makeextension(void);

extern unsigned char	Palette0, SPC700sh, OffBy1Line, DSPDisable,
			MMXSupport, Force8b, ForcePal, ForceNTSC, GUIClick, MouseDis,
			MusicRelVol, ScreenScale, SoundQuality,
			StereoSound, V8Mode, antienab, cvidmode, debugdisble,
			debugger, enterpress, finterleave, frameskip,
			gammalevel, guioff, per2exec, pl1contrl, pl2contrl,
			romtype, scanlines, showallext, smallscreenon, soundon,
			spcon, vsyncon, DisplayS, fname, filefound, SnowOn,
			NetChatFirst,NetServer,NetNewNick,
			NetFilename,TCPIPAddress,NetQuitAfter,UDPConfig;
			
// FIX STATMAT
extern unsigned char	autoloadstate;
// FIX STATMAT			

int getopt(int argc, char *const argv[], const char *optstring);
extern char *optarg;
extern int optind, opterr, optopt;

void ccmdline(void);

char *ers[] =
{
  "Frame Skip must be a value of 0 to 9!\n",
  "Gamma Correction Level must be a value of 0 to 5!\n",
  "Sound Sampling Rate must be a value of 0 to 5!\n",
  "Invalid Video Mode!\n",
  "Percentage of instructions to execute must be a number from 50 to 150!\n",
  "Player Input must be a value from 0 to 6!\n",
  "Volume must be a number from 0 to 100!\n"

};

//int argc;
//char **argv;

char ucase(char ch){
  if ((ch>='a') && (ch<='z')) ch-='a'-'A';
  return(ch);
}

int my_atoi(char *nptr) {
	int p,c;

	c = 0;	
	for(p = 0; nptr[p]; p++) {
		if ( !isdigit(nptr[p]) ) c += 1;
	}
		
	if (c) return -1;

	return atoi(nptr);
}

int main (int argc, char *argv[]) {
	int opt,p,pp;

	char *fvar;

	while((opt = getopt(argc, argv, "01:2:789a:c:d:ef:g:hijk:lmno:p:r:s:tuv:wyz?")) != -1) {
		switch(opt) {
			/* Palette 0 disable */
			case '0': {
				Palette0 = 1;
				break;
			}
			/* Player 1 Input */
			case '1': {
				//if (!hasroom) return 4;
				pl1contrl = my_atoi(optarg);

				if (pl1contrl > 6) return 15;
				p++;

				ConvertJoyMap1();
				break;
			}
			/* Player 2 Input */
			case '2': {
//				if (!hasroom) return 4;
				pl2contrl=my_atoi(optarg);

				if (pl2contrl > 6) return 15;
				p++;

				ConvertJoyMap2();
				break;
			}
			/* SPC700 speed hack disable */
			case '7': {
				SPC700sh = 1;
				break;
			}

			case '8': {
				Force8b = 1;
				break;
			}
			/* Off by 1 line */
			case '9': {
				OffBy1Line = 1;
				break;
			}

			case 'c': {
				if (strcmp(optarg,"b") == 0) {
					printf("\nRemove background color in 256 modes not implemented!\n");
				}
				
				else
				
				if (strcmp(optarg,"c") == 0) {
					smallscreenon = 1;
					pp++;
				} else {
					ScreenScale = 1;
				}
				break;
			}

			case 'd': {
				if (strcmp(optarg,"d") == 0) {
					DSPDisable = 1;
					pp++;
				} else {
					/* debugger will never work under linux, since it's full of bios interrupt calls */
					printf("\nDebugger not implemented for Linux version!\n");
				}
				break;
			}

			case 'e': {
				enterpress = 1;
				break;            
			}

			case 'f': {
//				if (!hasroom) return 4;
				frameskip = my_atoi(optarg);
				if (frameskip > 9) return 10;
                                frameskip++;
				p++;
				break;
			}

			case 'g': {
//				if (!hasroom) return 4;
				gammalevel = my_atoi(optarg);

				if (gammalevel > 5) return 11;
				p++;

				break;
			}

			case 'h': {
				romtype = 2;
				break;
			}

			case 'i': {
				finterleave = 1;
				break;
			}

			case 'j': {
				GUIClick = 0;
				MouseDis = 1;
				break;
			}

			case 'k': {
//				if (!hasroom) return 4;
				MusicRelVol = my_atoi(optarg);

				if (MusicRelVol > 100) return 16;
				p++;
				break;
			}

			case 'l': {
				romtype = 1;
				break;
			}

			/* disables GUI */
			case 'm': {
printf("Hello the gui should now be off.");
				guioff = 1; 
				break;
			}

			case 'n': {
				scanlines = 1;
				break;
			}

			case 'o': {
				if (strcmp(optarg,"m") == 0) {
					MMXSupport = 1;
					pp++;
				} else {
					MMXSupport = 0;
				}
				break;
			}

			case 'p': {
//				if (!hasroom) return 4;
				per2exec = my_atoi(optarg);

				if (per2exec > 150) return 14;

				if (per2exec < 50) return 14;
				p++;

				break;
			}

			case 'r': {
//				if (!hasroom) return 4;
				SoundQuality = my_atoi(optarg);

				if (SoundQuality > 6) return 12;
				p++;

				break;
			}

			case 's': {
				if (strcmp(optarg,"a") == 0) {
					showallext = 1;
					pp++;
				} else if (strcmp(optarg,"n") == 0) {
					SnowOn = 1;
					pp++;
				} else {
					spcon = 1;
					soundon = 1;
				}
				break;
			}

			case 't': {
				ForcePal = 1;
				break;
			}

			case 'u': {
				ForcePal = 2;
				break;
			}

			case 'v': {
				if (strcmp(optarg,"8") == 0) {
					V8Mode = 1;
					pp++;
				} else {
//					if (!hasroom) return 4;
					cvidmode = my_atoi(optarg);

					if (cvidmode > 10) return 13;
					p++;
				}
				break;
			}

			case 'w': {
				vsyncon = 1;
				break;
			}

			case 'y': {
				antienab = 1;
				break;
			}

                        case 'z': {
				// FIX STATMAT
//				if((argv[p+1]) == 's')
//				{				
//					if(!hasroom) return 4;
//					autoloadstate=my_atoi(argv[p+1]) + 1;
//					p++;
//				}

				StereoSound=0;

				// FIX STATMAT
				break;
			}

			case '?': {
				displayparams();
				return 9;
			}
		}
	}

	/* execute rom filename: file.x */
	/* getopt permutates argv until all non options are at the end of argv.	*/
	/* since we only expect one non option, it should be the last argument.	*/
	if ( optind == argc - 1 && argv[optind] != NULL)
	{
		fvar=&fname;
		fvar[0] = strlen(argv[optind]);
		strncpy(&fvar[1], argv[optind],127);
		makeextension();
	}

	zstart();
	return 0;
}


int pccmdline(void) {
	return 0;
}


void ccmdline(void) {
	int p = 0;

	p = pccmdline();
	if (p == 0) return;
  
	if (p == 9) {
    		displayparams();
	}

	if (p == 4) {
		/* printf("Mangled command line, did you forget a parm?\n"); */
		printf("Invalid Commandline!\n");
		DosExit();
	}
  
	if ((p > 9) && (p < 17)) {
		printf(ers[p-10]);
		DosExit();
	}

	if (p == 2) {
		DosExit();
	}
  
	printf("cmdline returned %i\n",p);
	DosExit();
}
