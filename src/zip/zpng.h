#ifndef __ZPNG__
#define __ZPNG__

#define __PNG__

#ifdef __PNG__
#ifndef bool
	typedef enum {false, true} bool;
#endif
	//#define __UPSIDE_DOWN__  /*define if pngs are saved upside down*/
#ifdef __LINUX__
	#include "../gblhdr.h"
#else
	#include <png.h>
#endif
	void Grab_PNG_Data(void);
	int Png_Dump(const char * filename, unsigned short width, unsigned short height, unsigned char * image_data, bool usebgr);
	#define ZPNG_GAMMA 1.0
#endif

#endif
