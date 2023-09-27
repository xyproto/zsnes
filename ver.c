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

#include <string.h>

char *VERSION_STR;
char *VERSION_DATE = __DATE__;
char *VERSION_PORT = "SDL - Unix";

// Place compilation date at the end of VERSION_STR
void placedate(void) {
	strcpy(VERSION_STR + strlen(VERSION_STR) - strlen(VERSION_DATE), VERSION_DATE);
}

// Place compilation time at the end of VERSION_STR
void placetime() {
	strcpy(VERSION_STR + strlen(VERSION_STR) - strlen(__TIME__), __TIME__);
}

unsigned int version_hash() {
	return (~crc32(0, (const unsigned char *)__DATE__, strlen(__DATE__)));
}
