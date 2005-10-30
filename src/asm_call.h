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
*/


/*
Introducing 'the "DJGPP/MinGW/MSVC/GCC on *nix compatible" call asm function
safely from C/C++' macro function!

Usage:
       asm_call( func_name );

That's all there is to it!!!


Note: Make sure this is used on a line by itself if the file will ever be
compiled with MSVC, since it's preproccessor won't parse the inline
assembly correctly if other stuff are on the line

Note: This will not work with GCC when using the parameter -MASM=intel
I'd fix that if anyone knows if that parameter defines something I can check

-Nach
*/

#ifndef ASM_CALL_H
#define ASM_CALL_H
////////////////////////////////////////////////////////

#ifdef __GNUC__

#define ASM_COMMAND(line) #line"\n\t"

#ifdef __x86_64__
#define PUSHAD ASM_COMMAND(pushl %eax) \
               ASM_COMMAND(pushl %ecx) \
               ASM_COMMAND(pushl %edx) \
               ASM_COMMAND(pushl %ebx) \
               ASM_COMMAND(pushl %esp) \
               ASM_COMMAND(pushl %ebp) \
               ASM_COMMAND(pushl %esi) \
               ASM_COMMAND(pushl %edi)

#define POPAD ASM_COMMAND(popl %edi) \
              ASM_COMMAND(popl %esi) \
              ASM_COMMAND(popl %ebp) \
              ASM_COMMAND(popl %esp) \
              ASM_COMMAND(popl %ebx) \
              ASM_COMMAND(popl %edx) \
              ASM_COMMAND(popl %ecx) \
              ASM_COMMAND(popl %eax)
#else
#define PUSHAD ASM_COMMAND(pushal)
#define POPAD ASM_COMMAND(popal)
#endif

#ifdef __UNIXSDL__
#define ASM_CALL(func) ASM_COMMAND(call func)
#else
#define ASM_CALL(func) ASM_COMMAND(call _ ## func)
#endif

#define asm_call(func) __asm__ __volatile__ ( \
PUSHAD \
ASM_CALL(func) \
POPAD \
);

#else //MSVC

#define asm_call(func) { _asm pushad \
_asm call func \
_asm popad };

#endif

////////////////////////////////////////////////////////
#endif
