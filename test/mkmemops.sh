#!/bin/sh
# mkmemops.sh - build the asm oracle for difftest_memops.c.
#
# Extracts the direct-page memory handlers as they were before the port (from
# git) into _memops.o, renaming each membank0* to asm_membank0*, and adds
# asm_memcall(fn), which loads the handler's register ABI from the seam block,
# calls it, and writes the registers back so the test can compare them.
set -e

# Default to the newest revision whose memory.asm predates the port, i.e. the
# last one where no handler body is a `memcop` thunk yet.
REV=$1
if [ -z "$REV" ]; then
    for r in $(git -C .. log --format=%H -- cpu/memory.asm); do
        if ! git -C .. show "$r:cpu/memory.asm" | grep -q memcop; then
            REV=$r
            break
        fi
    done
fi
[ -n "$REV" ] || { echo "mkmemops.sh: no pre-port revision of cpu/memory.asm found" >&2; exit 1; }

git -C .. show "$REV:cpu/memory.asm" > _memops_src.asm

# Pull out only the handlers under test, in file order. Each runs to the next
# NEWSYM; none of them fall through into a neighbour.
python3 - _memops_src.asm ../test/memops.list > _memops.inc <<'PYEOF'
import re, sys
src = open(sys.argv[1]).read().split('\n')
want = set(open(sys.argv[2]).read().split())
out, cur = [], None
for l in src:
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
    sys.exit('mkmemops.sh: not found in memory.asm: ' + ' '.join(sorted(missing)))
print('\n'.join(out))
PYEOF

sed -i -E 's/^NEWSYM (membank0[A-Za-z0-9]+)/NEWSYM asm_\1/' _memops.inc

# The extracted handlers reach the register tables through the macros in
# cpu/regs.mac and cpu/regsw.mac; take those from the same revision.
git -C .. show "$REV:cpu/regs.mac"  | sed -n 's/^\(%define regptr(x).*\)$/\1/p' > _memops_reg.mac
git -C .. show "$REV:cpu/regsw.mac" | sed -n 's/^\(%define regptw(x).*\)$/\1/p' >> _memops_reg.mac

cat > _memops.asm <<'EOF'
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
EXTERN wramdataa
EXTERN regptra
EXTERN regptwa
EXTERN cpu_mdr
EXTERN snesmmap
EXTERN MemSeamB
EXTERN MemSeamC
EXTERN MemSeamA
EXTERN StubRegAddr
EXTERN StubRegVal
EXTERN StubRegHits

%include "_memops_reg.mac"

section .text

; void asm_memcall(void *fn)
NEWSYM asm_memcall
    push ebx
    push esi
    push edi
    push ebp
    mov eax,[esp+20]
    mov ebx,[MemSeamB]
    mov ecx,[MemSeamC]
    push eax
    mov eax,[MemSeamA]
    call dword [esp]
    mov [MemSeamA],eax
    add esp,4
    mov [MemSeamB],ebx
    mov [MemSeamC],ecx
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

; Stand-ins for an I/O register handler. They keep the legacy ABI (address in
; ecx, value in al) and preserve ebx, which membank0r16reg uses as scratch
; across its two calls. The read value is derived from the address so a wrong
; one is visible, and the last four calls are logged because the 16-bit
; variants call twice.
%macro REGLOG 0
    push ebx
    mov ebx,[StubRegHits]
    and ebx,3
    mov [StubRegAddr+ebx*4],ecx
    mov [StubRegVal+ebx*4],eax
    inc dword [StubRegHits]
    pop ebx
%endmacro

NEWSYM regstub_r
    REGLOG
    mov al,cl
    xor al,ch
    ret

NEWSYM regstub_w
    REGLOG
    ret

%include "_memops.inc"
EOF

nasm -f elf32 -w-orphan-labels -o _memops.o _memops.asm
echo "wrote _memops.o (oracle from $(git -C .. rev-parse --short $REV), $(grep -c '^NEWSYM asm_membank0' _memops.inc) handlers)"
