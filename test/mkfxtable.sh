#!/bin/sh
# Extract the original chips/fxtable.asm from git history and prepare it for the
# InitFxTables differential test:
#   _fxold.asm    the original, with InitFxTables and the data it defines renamed
#                 out of the way so it can be linked beside the C port
#   _fxstubs.asm  one label per SuperFX opcode handler, so the test needs only
#                 the addresses the tables are built from, not the handlers
set -e

rev=$(git -C .. rev-list -1 HEAD -- chips/fxtable.asm)
[ -n "$rev" ] || { echo "chips/fxtable.asm not found in history" >&2; exit 1; }
git -C .. show "$rev:chips/fxtable.asm" > _fxold.raw 2>/dev/null ||
    git -C .. show "$rev^:chips/fxtable.asm" > _fxold.raw

sed -e 's/NEWSYM InitFxTables/NEWSYM InitFxTablesAsm/' \
    -e 's/sfx128lineloc/asm_sfx128lineloc/g' \
    -e 's/sfx160lineloc/asm_sfx160lineloc/g' \
    -e 's/sfx192lineloc/asm_sfx192lineloc/g' \
    -e 's/sfxobjlineloc/asm_sfxobjlineloc/g' \
    -e 's/sfxnametab/asm_sfxnametab/g' _fxold.raw > _fxold.asm

# Every EXTSYM the original needs, minus what the test and endmem.o provide.
{
    echo "SECTION .text"
    sed -n 's/^EXTSYM //p' _fxold.asm | tr ',' '\n' | tr -d ' \r' | sort -u |
    grep -vxE 'romdata|sfxramdata|SfxMemTable|PLOTJmpa|PLOTJmpb|fxxand|fxbit01|fxbit23|fxbit45|fxbit67|FxTable[A-Za-z0-9]*' |
    while read -r s; do [ -n "$s" ] && printf 'global %s\n%s: nop\n' "$s" "$s"; done
} > _fxstubs.asm

nasm -w-label-orphan -f elf32 -DELF _fxold.asm  -o _fxold.o
nasm -w-label-orphan -f elf32 -DELF _fxstubs.asm -o _fxstubs.o
