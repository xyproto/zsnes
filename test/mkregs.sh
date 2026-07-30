#!/bin/sh
# mkregs.sh - build the asm oracle for difftest_regs.c.
#
# Extracts the register handlers as they were before the port (from git) into
# _regs.o, renaming each to asm_*, together with the checkmultchange macro.
set -e

REV=$1
if [ -z "$REV" ]; then
    for r in $(git -C .. log --format=%H -- cpu/regs.inc); do
        git -C .. cat-file -e "$r:cpu/regs.inc" 2>/dev/null || continue
        if git -C .. show "$r:cpu/regs.inc" | grep -q '^NEWSYM reg2134r'; then
            REV=$r
            break
        fi
    done
fi
[ -n "$REV" ] || { echo "mkregs.sh: no pre-port revision of cpu/regs.inc found" >&2; exit 1; }

git -C .. show "$REV:cpu/regs.inc" > _regs_src.inc

python3 - _regs_src.inc ../test/regs.list > _regs.inc <<'PYEOF'
import re, sys
src = open(sys.argv[1]).read().split('\n')
want = set(open(sys.argv[2]).read().split())
# Two shapes to get right. Several handlers are stacked aliases sharing one
# body (reg21C2r/reg21C3r, reg420Ar..reg420Fr), and several have more than one
# `ret`. So: a NEWSYM seen before any instruction has been emitted for the
# current handler is an alias - keep going; a NEWSYM seen after one ends the
# handler. Stopping at the first `ret` instead would silently truncate a body
# and let the label fall into the next handler.
out, cur, emitted = [], None, False
for l in src:
    m = re.match(r'NEWSYM (\w+)', l.strip())
    if m:
        if cur and not emitted:
            out.append('NEWSYM asm_' + m.group(1))
            continue
        cur = m.group(1) if m.group(1) in want else None
        emitted = False
        if cur:
            out.append('NEWSYM asm_' + m.group(1))
        continue
    if cur and not re.match(r'(SECTION |%)', l.strip()):
        out.append(l)
        if l.strip():
            emitted = True
missing = want - {re.match(r'NEWSYM asm_(\w+)', l.strip()).group(1)
                  for l in out if l.strip().startswith('NEWSYM asm_')}
if missing:
    sys.exit('mkregs.sh: not found: ' + ' '.join(sorted(missing)))
print('\n'.join(out))
PYEOF

git -C .. show "$REV:cpu/regs.mac" > _regs.mac

cat > _regs.asm <<'EOF'
bits 32
section .note.GNU-stack noalloc noexec nowrite progbits
%imacro newsym 1
  GLOBAL %1
  %1:
%endmacro
%imacro newsym 2+
  GLOBAL %1
  %1: %2
%endmacro
EXTERN vidbright
EXTERN forceblnk
EXTERN multchange
EXTERN compmult
EXTERN mode7A
EXTERN mode7B
EXTERN regptra
EXTERN rtoflags
EXTERN romispal
EXTERN ppustatus
EXTERN cfield
EXTERN extlatch
EXTERN ppu2_mdr
EXTERN latchxr
EXTERN latchyr
EXTERN NMIEnab
EXTERN cpu_mdr
EXTERN curnmi
EXTERN irqon
EXTERN wramrwadr
EXTERN wramdata
EXTERN ioportval
EXTERN divres
EXTERN multres
EXTERN JoyARead
EXTERN JoyBRead
EXTERN JoyCRead2
EXTERN JoyDRead
EXTERN oamram
EXTERN cgram
EXTERN oamaddr
EXTERN cgaddr
EXTERN latchx
EXTERN latchy

%include "_regs.mac"

section .text
%include "_regs.inc"
EOF

nasm -O1 -f elf32 -w-orphan-labels -o _regs.o _regs.asm
echo "wrote _regs.o (oracle from $(git -C .. rev-parse --short $REV), $(grep -c '^NEWSYM asm_' _regs.inc) handlers)"
