#ifndef ASMDATA_H
#define ASMDATA_H

// Portable inline-asm data layout directives. A few data blocks go through
// inline asm to pin exact symbol order and adjacency, which the save-state
// code relies on; ELF, Mach-O and PE/COFF spell sections and symbol names
// differently, so abstract that here. ASM_GSYM defines an exported symbol,
// ASM_LSYM a file-local one; name either from the asm text with ASM_SYMREF or
// ASM_LSYMREF. Spell sizes .byte/.short/.long, never .word: aarch64 reads that
// as four bytes where x86 reads two.

#if defined(__APPLE__)
// clang always emits .subsections_via_symbols, so ld64 cuts a section into
// independently placed atoms at every symbol and -dead_strip drops the
// unreferenced ones, collapsing the block.  .alt_entry on every symbol keeps
// the block one atom; the first symbol of a section may not be one, hence the
// once-per-section anchor.  Local labels take an L prefix, which keeps them out
// of the symbol table so they do not split the atom either.
#define ASM_MACHO_ANCHOR(name) ".ifndef zsnes_anchor" name "\nzsnes_anchor" name ":\n.endif\n"
#define ASM_SEC_DATA(name) ".section " name ",\"dw\"\n" ASM_MACHO_ANCHOR(name)
#define ASM_SEC_BSS(name) ".section " name ",\"bw\"\n" ASM_MACHO_ANCHOR(name)
#define ASM_SEC_END ".text\n"
#define ASM_GSYM(sym) ".global _" #sym "\n.alt_entry _" #sym "\n_" #sym ":\n"
#define ASM_SYMREF(sym) "_" #sym
#define ASM_LSYM(sym) "L" #sym ":\n"
#define ASM_LSYMREF(sym) "L" #sym
#elif defined(__MINGW32__)
#define ASM_SEC_DATA(name) ".section " name ",\"dw\"\n"
#define ASM_SEC_BSS(name) ".section " name ",\"bw\"\n"
#define ASM_SEC_END ".text\n"
// 32-bit PE/COFF prefixes an underscore; x86-64 Windows does not, and
// mingw-w64 defines __MINGW32__ for both word sizes.
#ifdef _WIN64
#define ASM_GSYM(sym) ".global " #sym "\n" #sym ":\n"
#define ASM_SYMREF(sym) #sym
#else
#define ASM_GSYM(sym) ".global _" #sym "\n_" #sym ":\n"
#define ASM_SYMREF(sym) "_" #sym
#endif
#define ASM_LSYM(sym) #sym ":\n"
#define ASM_LSYMREF(sym) #sym
#else
#define ASM_SEC_DATA(name) ".pushsection " name ",\"aw\",@progbits\n"
#define ASM_SEC_BSS(name) ".pushsection " name ",\"aw\",@nobits\n"
#define ASM_SEC_END ".popsection\n"
#define ASM_GSYM(sym) ".global " #sym "\n" #sym ":\n"
#define ASM_SYMREF(sym) #sym
#define ASM_LSYM(sym) #sym ":\n"
#define ASM_LSYMREF(sym) #sym
#endif

/* Spell a macro's value into the asm text, for sizes that follow the target
   (pointer width, mainly) rather than being literals. */
#define ASM_STR_(x) #x
#define ASM_STR(x) ASM_STR_(x)

#endif
