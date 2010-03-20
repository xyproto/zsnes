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


/*
Introducing 'the "DJGPP/MinGW/MSVC/GCC on *nix compatible" call asm function
safely from C/C++' macro function!

Usage:
       asm_call( func_name );

That's all there is to it!!!


Note: This will not work with GCC when using the parameter -MASM=intel
I'd fix that if anyone knows if that parameter defines something I can check

-Nach
*/

#ifndef ASM_CALL_H
#define ASM_CALL_H
////////////////////////////////////////////////////////

#ifdef __GNUC__
#	if defined __x86_64__
#		define asm_call(func) asm volatile("push %%rbx; call %P0; pop %%rbx" :: "X" (func) : "cc", "memory", "rax", "rcx", "rdx", "rbp", "rsi", "rdi")
#	elif defined __i386__
#		define asm_call(func) asm volatile("push %%ebp; call %P0; pop %%ebp" :: "X" (func) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi")
#	else
#		error unknown architecture
#	endif
#elif defined _MSC_VER
#	define asm_call(func) __asm { __asm pushad  __asm call func  __asm popad }
#else
#	error unknown compiler
#endif

////////////////////////////////////////////////////////
#endif
