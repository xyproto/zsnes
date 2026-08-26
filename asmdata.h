#ifndef ASMDATA_H
#define ASMDATA_H

// Portable inline-asm data layout directives.  A few data blocks are emitted
// via inline asm to force exact symbol order and adjacency (relied on by the
// asm core and the save-state code).  ELF and PE/COFF differ in section syntax
// and symbol naming (PE/COFF prefixes an underscore), so abstract it here.
// ASM_GSYM exports _sym and keeps a plain sym alias for intra-block references.
// That alias is file-local, so use ASM_SYMREF to name a symbol from another
// object (e.g. a pointer initialised to someone else's array).
// Sizes must mean the same thing on every target, so spell them
// .byte/.short/.long: aarch64 reads .word as four bytes where x86 reads two,
// and these blocks came from NASM's dw.

#if defined(__APPLE__) || defined(__MINGW32__)
#define ASM_SEC_DATA(name) ".section " name ",\"dw\"\n"
#define ASM_SEC_BSS(name) ".section " name ",\"bw\"\n"
#define ASM_SEC_END ".text\n"
/* Mach-O and 32-bit PE/COFF prefix an underscore; x86-64 Windows does not,
   and mingw-w64 defines __MINGW32__ for both word sizes. */
#ifdef _WIN64
#define ASM_GSYM(sym) ".global " #sym "\n" #sym ":\n"
#define ASM_SYMREF(sym) #sym
#else
#define ASM_GSYM(sym) ".global _" #sym "\n_" #sym ":\n" #sym ":\n"
#define ASM_SYMREF(sym) "_" #sym
#endif
#else
#define ASM_SEC_DATA(name) ".pushsection " name ",\"aw\",@progbits\n"
#define ASM_SEC_BSS(name) ".pushsection " name ",\"aw\",@nobits\n"
#define ASM_SEC_END ".popsection\n"
#define ASM_GSYM(sym) ".global " #sym "\n" #sym ":\n"
#define ASM_SYMREF(sym) #sym
#endif

/* Spell a macro's value into the asm text, for sizes that follow the target
   (pointer width, mainly) rather than being literals. */
#define ASM_STR_(x) #x
#define ASM_STR(x) ASM_STR_(x)

#endif
