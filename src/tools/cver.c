/*
Copyright (C) 2007 Nach ( http://www.zsnes.com )

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

#include <stdio.h>

#if defined(__i386__) || defined(_M_IX86)
static int x86_32 = 1;
#else
static int x86_32 = 0;
#endif

#if defined(__x86_64__) || defined(_M_X64)
static int x86_64 = 1;
#else
static int x86_64 = 0;
#endif

#ifdef __STDC_VERSION__
static int stdcv = __STDC_VERSION__;
#else
static int stdcv = 198900;
#endif

int main()
{
  #ifdef __GNUC__
  printf("Compiler: GCC\nMajor: %u\nMinor: %u\nMicro: %u\nVersion: %s\n",
         __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, __VERSION__);
  #elif defined(_MSC_VER)
  printf("Compiler: MSVC\nMajor: %u\nMinor: %u\nVersion: %u\n",
         _MSC_VER/100, _MSC_VER%100, _MSC_VER);
  #endif
  printf("C99: %s\nx86-32: %s\nx86-64: %s\n", (stdcv >= 199901L) ? "Yes":"No", x86_32 ? "Yes":"No", x86_64 ? "Yes":"No");
  fflush(stdout);
  return(0);
}
