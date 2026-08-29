#!/bin/sh
# Extract the original chips/fxtable.asm from asm-sources.zip and prepare it for the
# InitFxTables differential test:
#   _fxold.asm    the original, with InitFxTables and the data it defines renamed
#                 out of the way so it can be linked beside the C port
#   _fxstubs.asm  one label per SuperFX opcode handler, so the test needs only
#                 the addresses the tables are built from, not the handlers
set -e

rev=$(./asmgit.sh rev-list -1 HEAD -- chips/fxtable.asm)
[ -n "$rev" ] || { echo "chips/fxtable.asm not found in history" >&2; exit 1; }
./asmgit.sh show "$rev:chips/fxtable.asm" > _fxold.raw 2>/dev/null ||
    ./asmgit.sh show "$rev^:chips/fxtable.asm" > _fxold.raw

sed -e 's/NEWSYM InitFxTables/NEWSYM InitFxTablesAsm/' \
    -e 's/sfx128lineloc/asm_sfx128lineloc/g' \
    -e 's/sfx160lineloc/asm_sfx160lineloc/g' \
    -e 's/sfx192lineloc/asm_sfx192lineloc/g' \
    -e 's/sfxobjlineloc/asm_sfxobjlineloc/g' \
    -e 's/sfxnametab/asm_sfxnametab/g' _fxold.raw > _fxold.asm

# Every EXTSYM the original needs, minus what the test and endmem.o provide.
sed -n 's/^EXTSYM //p' _fxold.asm | tr ',' '\n' | tr -d ' \r' | sort -u |
grep -vxE 'romdata|sfxramdata|SfxMemTable|PLOTJmpa|PLOTJmpb|fxxand|fxbit01|fxbit23|fxbit45|fxbit67|FxTable[A-Za-z0-9]*' \
    > _fxsyms.txt

# The C port names the handlers c_FxOpXX, and it no longer has separate d-table
# bodies: fxops_d.list says which base handler each FxOpdXX became. Put the
# aliases on the same address so the tables still compare as raw addresses.
python3 - fxops_d.list _fxsyms.txt > _fxstubs.asm <<'PYEOF'
import sys

base = {}
for line in open(sys.argv[1]):
    p = line.split()
    if len(p) == 2 and p[0] != p[1]:
        base[p[0]] = p[1]

groups = {}
for s in open(sys.argv[2]).read().split():
    groups.setdefault(base.get(s, s), []).append(s)

print("SECTION .text")
for canon, names in groups.items():
    names = sorted(set(names) | {canon})
    if canon.startswith("FxOp"):
        names.append("c_" + canon)
    for n in names:
        print("global " + n)
    for n in names:
        print(n + ":")
    print("  nop")
PYEOF

nasm -w-label-orphan -f elf32 -DELF _fxold.asm  -o _fxold.o
nasm -w-label-orphan -f elf32 -DELF _fxstubs.asm -o _fxstubs.o
