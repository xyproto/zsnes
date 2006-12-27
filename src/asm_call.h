/*
Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

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
#define PUSHAD ASM_COMMAND(pushq %rax) \
               ASM_COMMAND(pushq %rcx) \
               ASM_COMMAND(pushq %rdx) \
               ASM_COMMAND(pushq %rbx) \
               ASM_COMMAND(pushq %rsp) \
               ASM_COMMAND(pushq %rbp) \
               ASM_COMMAND(pushq %rsi) \
               ASM_COMMAND(pushq %rdi)

#define POPAD ASM_COMMAND(popq %rdi) \
              ASM_COMMAND(popq %rsi) \
              ASM_COMMAND(popq %rbp) \
              ASM_COMMAND(popq %rsp) \
              ASM_COMMAND(popq %rbx) \
              ASM_COMMAND(popq %rdx) \
              ASM_COMMAND(popq %rcx) \
              ASM_COMMAND(popq %rax)
#else
#define PUSHAD ASM_COMMAND(pushal)
#define POPAD ASM_COMMAND(popal)
#endif

#ifdef __ELF__
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
