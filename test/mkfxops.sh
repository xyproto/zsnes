#!/bin/sh
# mkfxops.sh - build the asm oracle for difftest_fxops.c.
#
# Extracts all 92 SuperFX opcode handlers as they were before the port (from
# git) into _fxops.o, renaming each FxOpXX to asm_FxOpXX, and adds the helpers
# the difftest needs to talk to the core's register ABI:
#
#   asm_fxcall(fn)  load the ABI registers from the seam block, call fn, write
#                   them back  (drives the *asm* handler)
#   FxDispatch(tbl) same, but dispatches through tbl[ecx]  (used by the *C*
#                   handler, matching the real seam in chips/fxemu2b.asm)
#   fxstub/b/c      stand in for the next opcode: record what they were
#                   dispatched with, then return. One per table, so the test can
#                   tell which table a handler chained through.
set -e

# Default to the newest revision whose fxemu2b.asm predates the port, i.e. the
# last one where no handler body is an `fxcop` thunk yet.
REV=$1
if [ -z "$REV" ]; then
    for r in $(git -C .. log --format=%H -- chips/fxemu2b.asm); do
        if ! git -C .. show "$r:chips/fxemu2b.asm" | grep -q fxcop \
            && ! git -C .. show "$r:chips/fxemu2.asm" | grep -q fxcop; then
            REV=$r
            break
        fi
    done
fi
[ -n "$REV" ] || { echo "mkfxops.sh: no pre-port revision of chips/fxemu2b.asm found" >&2; exit 1; }

SRC=_fxops_src.asm
git -C .. show "$REV:chips/fxemu2b.asm" > "$SRC"

# The handlers start at the first opcode entry point and run to end of file.
FIRST=$(grep -n '^NEWSYM FxOp' "$SRC" | head -1 | cut -d: -f1)
sed -n "${FIRST},\$p" "$SRC" > _fxops.inc

# chips/fxemu2.asm holds the base-table handlers, mixed in with data and with
# routines the difftest has no business linking. Pull out only the opcode
# bodies named on stdin (one per line), in file order.
git -C .. show "$REV:chips/fxemu2.asm" > _fxops_base.asm
python3 - _fxops_base.asm ../test/fxops_base.list >> _fxops.inc <<'PYEOF'
import re, sys
src = open(sys.argv[1]).read().split('\n')
want = set(open(sys.argv[2]).read().split())
out = []
cur = None
for i, l in enumerate(src):
    m = re.match(r'NEWSYM (\w+)', l.strip())
    if m:
        cur = m.group(1) if m.group(1) in want else None
        if cur:
            out.append(l)
        continue
    if cur and not re.match(r'(SECTION |%)', l.strip()):
        out.append(l)
    elif cur:
        cur = None
missing = want - {re.match(r'NEWSYM (\w+)', l.strip()).group(1)
                  for l in out if l.strip().startswith('NEWSYM ')}
if missing:
    sys.exit('mkfxops.sh: not found in fxemu2.asm: ' + ' '.join(sorted(missing)))
print('\n'.join(out))
PYEOF

# Rename every entry point so the oracle does not clash with the real symbols.
sed -i -E 's/^NEWSYM (FxOp[A-Za-z0-9]+)/NEWSYM asm_\1/' _fxops.inc

# The TO/FROM macro bodies live in a .mac the port deleted; take it from git
# too, along with fxemu2.mac for FETCHPIPE / UpdateR14 / CLRFLAGS.
git -C .. show "$REV:chips/fxemu2.mac" > _fxops_m1.mac
git -C .. show "$REV:chips/fxemu2b.mac" > _fxops_m2.mac

cat > _fxops.asm <<'EOF'
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
EXTERN FxTable
EXTERN FxTableb
EXTERN FxTablec
EXTERN SfxB
EXTERN SfxCPB
EXTERN SfxCROM
EXTERN SfxCarry
EXTERN SfxOverflow
EXTERN SfxR0
EXTERN SfxR14
EXTERN SfxR15
EXTERN SfxRomBuffer
EXTERN SfxSignZero
EXTERN withr15sk
EXTERN FxSeamPC
EXTERN FxSeamSrc
EXTERN FxSeamDst
EXTERN FxSeamCX
EXTERN StubPC
EXTERN StubCX
EXTERN StubSrc
EXTERN StubDst
EXTERN StubHits
EXTERN StubTable

%include "_fxops_m1.mac"
%include "_fxops_m2.mac"

section .text

; void asm_fxcall(void *fn)
NEWSYM asm_fxcall
    push ebx
    push esi
    push edi
    push ebp
    mov eax,[esp+20]
    mov ebp,[FxSeamPC]
    mov esi,[FxSeamSrc]
    mov edi,[FxSeamDst]
    mov ecx,[FxSeamCX]
    call eax
    mov [FxSeamPC],ebp
    mov [FxSeamSrc],esi
    mov [FxSeamDst],edi
    mov [FxSeamCX],ecx
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

; void FxDispatch(u4 const *table)
NEWSYM FxDispatch
    push ebx
    push esi
    push edi
    push ebp
    mov eax,[esp+20]
    mov ebp,[FxSeamPC]
    mov esi,[FxSeamSrc]
    mov edi,[FxSeamDst]
    mov ecx,[FxSeamCX]
    call [eax+ecx*4]
    mov [FxSeamPC],ebp
    mov [FxSeamSrc],esi
    mov [FxSeamDst],edi
    mov [FxSeamCX],ecx
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

%macro FXSTUB 2
NEWSYM %1
    mov [StubPC],ebp
    mov [StubCX],ecx
    mov [StubSrc],esi
    mov [StubDst],edi
    mov dword [StubTable],%2
    inc dword [StubHits]
    ret
%endmacro

FXSTUB fxstub, 1
FXSTUB fxstubb, 2
FXSTUB fxstubc, 3

%include "_fxops.inc"
EOF

nasm -f elf32 -w-orphan-labels -o _fxops.o _fxops.asm
echo "wrote _fxops.o (oracle from $(git -C .. rev-parse --short $REV), $(grep -c '^NEWSYM asm_FxOp' _fxops.inc) handlers)"
