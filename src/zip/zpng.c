#include "zpng.h"


#ifdef __WIN32__
	#include <windows.h>
	
	#ifdef __WIN32DBG__
		#include <crtdbg.h>
	#endif
#endif

#ifdef __MSDOS__
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <stdlib.h>
#endif

#ifdef __LINUX__
#include "../gblhdr.h"
#endif

extern unsigned int vidbuffer;

#ifdef __PNG__

int Png_Dump(const char * filename, unsigned short width, unsigned short height, unsigned char * image_data, bool usebgr)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep * row_pointers;
	/*Set scanline width for 32-bit color data*/
	int scanline=width*4;
	int i; /*counter.*/
	int png_transforms=0;
	png_color fake_pal;
	/*Try to open the file.*/
	FILE *fp = fopen(filename, "wb");
	if (!fp)
	{
		return (-1);
	}
	
	fake_pal.red = 0;
	fake_pal.green = 0;
	fake_pal.blue = 0;
	/*Try to create png write struct, fail if we cannot.*/
	png_ptr = png_create_write_struct
		(PNG_LIBPNG_VER_STRING, NULL,/*(png_voidp)user_error_ptr,
		user_error_fn*/NULL, NULL/*user_warning_fn*/);
		if (!png_ptr)
			return (-1);
						
	/*set png I/O source.*/
	png_init_io(png_ptr, fp);
						
	/* set the zlib compression level */
	png_set_compression_level(png_ptr,
		Z_BEST_COMPRESSION);
	
	/* set other zlib parameters */
	png_set_compression_mem_level(png_ptr, 8);
	png_set_compression_strategy(png_ptr,
		Z_DEFAULT_STRATEGY);
	png_set_compression_window_bits(png_ptr, 15);
	png_set_compression_method(png_ptr, 8);
	png_set_compression_buffer_size(png_ptr, 8192);

	/*try to create info struct. Fail and delete existing structs if info struct cannot be created.*/
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr,
			(png_infopp)NULL);
			return (-1);
	}

	
	/*set a lot of image info (code adapted from libpng documentation!)*/
	png_set_IHDR(png_ptr, info_ptr, width, height,
		8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	info_ptr->color_type=PNG_COLOR_TYPE_RGB_ALPHA;

	/*Allocate an array of scanline pointers*/
	row_pointers=(png_bytep*)malloc(height*sizeof(png_bytep));
	for (i=0;i<height;i++)
	{
#ifdef __UPSIDE_DOWN__
		/*invert to normal image format.*/
		row_pointers[i]=&image_data[scanline*(height-i-1)];
#else
		row_pointers[i]=&image_data[scanline*i];
#endif
	}

	/*tell the png library what to encode.*/
	png_set_rows(png_ptr, info_ptr, row_pointers);

	if(usebgr)
		png_transforms|=PNG_TRANSFORM_BGR;
	/*Write image to file*/
	png_write_png(png_ptr, info_ptr, png_transforms, NULL);

	/*close file*/
	fclose(fp);
	
	/*Destroy PNG structs*/
	png_destroy_write_struct(&png_ptr, &info_ptr);
	
	/*clean up dynamically allocated RAM.*/
	free(row_pointers);

#ifdef __WIN32DBG__
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}

char *generate_filename(void)
{
	extern char fnames;
	char *filename;
	char *tmp = &fnames;
	short i=0;
#ifdef __WIN32__
	SYSTEMTIME time;
#else
	struct stat buf;
#endif

#ifdef __MSDOS__
	filename = (char *)malloc(14);
	for(i=0;i<10000;i++)
	{
		if(i>1000)
			sprintf(filename, "Image%03d.png", i);
		else sprintf(filename, "Imag%04d.png", i);
		if(stat(filename, &buf)==-1)
			break;
	}
	return filename;
#endif


	tmp++;         // the first char is the string length
	// removes the path if there is one
	while (*tmp!=0) tmp++;
	while ((*tmp!='/') && (tmp!=&fnames)) tmp--;
	tmp++;
	// allocates enough memory to store the filename
#ifdef __LINUX__
	filename = (char *)malloc(strlen(tmp)+10);
#endif
#ifdef __WIN32__
	filename = (char *)malloc(strlen(tmp)+25);
#endif
	strcpy(filename, tmp);
	tmp = filename;
	while (*tmp!='.') {
	  if (*tmp == ' ') *tmp = '_';
	  tmp++;
	}

#ifdef __WIN32__
	/*get system time.*/
	GetLocalTime(&time);
	
	/*make filename from local time*/
	wsprintf(tmp," %d %02d_%02d %02d-%02d-%02d.png\0", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
#endif
#ifdef __LINUX__
	/*find first unused file*/

	/*Note: this results in a 1000 image limit!*/

	for(i=0;i<10000;i++)
	{
	  sprintf(tmp, "_%04d.png", i);
	  if(stat(filename, &buf)==-1)
	    break;
	}
#endif
	return filename;
}

void Grab_PNG_Data(void)
{
	 char *filename;
	 bool is_bgr_data=true;

	/*These are the variables used to perform the 32-bit conversion*/
	int i,j;
	unsigned short* pixel;
	unsigned short conv_pixel;
	/*Set scanline width for 32-bit color data: 4*256 = 1024*/
	int scanline=1024;
	unsigned char *DIBits;
	unsigned int * DBits;

	filename = generate_filename();

	/*Allocate image buffer for DIB data*/
	DIBits=(unsigned char*)malloc(scanline*224);

	/*Cast pointer to 32-bit data type*/
	DBits=(unsigned int*) DIBits;

	/*Use zsKnight's 16 to 32 bit color conversion*/
	pixel=(unsigned short*)(vidbuffer);
	for(i=0;i<224;i++)
	{
		for(j=0;j<256;j++)
		{

			conv_pixel=pixel[(i*288)+j+16];
			DBits[i*256+j]=((conv_pixel&0xF800)<<8)|
				       ((conv_pixel&0x07E0)<<5)|
				       ((conv_pixel&0x001F)<<3)|0xFF000000;
		}
	}

	/*compress and write the PNG*/
	Png_Dump(filename, 256, 224, DIBits, is_bgr_data);

	free(DIBits);
	free(filename);

#ifdef __WIN32DBG__
	_CrtDumpMemoryLeaks();
#endif

}

#endif
