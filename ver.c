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
#include <zlib.h>

char* VERSION_DATE = __DATE__;

#if defined(__WIN32__) || defined(__ZSNES_PLATFORM_WINDOWS__)
char* VERSION_PORT = "WIN";
char const VERSION_PLATFORM[] = "Windows";
#elif defined(__ZSNES_PLATFORM_DARWIN__)
char* VERSION_PORT = "SDL - Darwin";
char const VERSION_PLATFORM[] = "macOS";
#elif defined(__ZSNES_PLATFORM_FREEBSD__)
char* VERSION_PORT = "SDL - FreeBSD";
char const VERSION_PLATFORM[] = "FreeBSD";
#elif defined(__ZSNES_PLATFORM_OPENBSD__)
char* VERSION_PORT = "SDL - OpenBSD";
char const VERSION_PLATFORM[] = "OpenBSD";
#elif defined(__ZSNES_PLATFORM_NETBSD__)
char* VERSION_PORT = "SDL - NetBSD";
char const VERSION_PLATFORM[] = "NetBSD";
#elif defined(__ZSNES_PLATFORM_LINUX__) || defined(__linux__)
char* VERSION_PORT = "SDL - Linux";
char const VERSION_PLATFORM[] = "Linux";
#elif defined(__HAIKU__)
char* VERSION_PORT = "SDL - Haiku";
char const VERSION_PLATFORM[] = "Haiku";
#else
char* VERSION_PORT = "SDL - Unknown";
char const VERSION_PLATFORM[] = "Unknown";
#endif

#if defined(__x86_64__) || defined(_M_X64)
char const VERSION_ARCH[] = "x86_64";
#elif defined(__aarch64__) || defined(_M_ARM64)
char const VERSION_ARCH[] = "aarch64";
#elif defined(__i386__) || defined(_M_IX86)
char const VERSION_ARCH[] = "i686";
#else
char const VERSION_ARCH[] = "unknown";
#endif

char const* const VERSION_LIBRARIES[] = {
#ifdef __UNIXSDL__
    "SDL3",
#endif
#ifdef __PIPEWIRE__
    "PipeWire",
#endif
#ifdef __LIBAO__
    "libao",
#endif
#ifndef NO_PNG
    "libpng",
#endif
    NULL
};

unsigned int version_hash()
{
    return (~crc32(0, (const unsigned char*)__DATE__, strlen(__DATE__)));
}
