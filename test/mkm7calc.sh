#!/bin/sh
# mkm7calc.sh - build the asm oracle for difftest_m7calc.c.
#
# Extracts CalculateNewValues and its newvaluepred macro as they were before
# the port (from git) into _m7calc.o, renamed asm_CalculateNewValues.
set -e

REV=$1
if [ -z "$REV" ]; then
    for r in $(git -C .. log --format=%H -- video/mode716.asm); do
        git -C .. cat-file -e "$r:video/mode716.asm" 2>/dev/null || continue
        if ! git -C .. show "$r:video/mode716.asm" | grep -q c_CalculateNewValues; then
            REV=$r
            break
        fi
    done
fi
[ -n "$REV" ] || { echo "mkm7calc.sh: no pre-port revision found" >&2; exit 1; }

git -C .. show "$REV:video/mode716.asm" > _m7calc_src.asm

python3 - _m7calc_src.asm > _m7calc.inc <<'PYEOF'
import re, sys
src = open(sys.argv[1]).read().split('\n')
out, on = [], None
for l in src:
    if l.startswith('%macro newvaluepred'):
        on = 'macro'
    elif l.startswith('CalculateNewValues:'):
        on = 'body'
        out.append('NEWSYM asm_CalculateNewValues')
        continue
    elif l.startswith('NEWSYM processmode7hires16b'):
        on = 'hires'
        out.append('NEWSYM asm_processmode7hires16b')
        continue
    if on:
        out.append(l)
    if on == 'macro' and l.strip() == '%endmacro':
        on = None
    elif on in ('body', 'hires') and l.strip() == 'ret':
        on = None
text = '\n'.join(out)
for want in ('asm_CalculateNewValues', 'asm_processmode7hires16b'):
    if want not in text:
        sys.exit('mkm7calc.sh: %s not found' % want)
# The hi-res pass calls the local one; keep the oracle on its own copy.
text = text.replace('call CalculateNewValues', 'call asm_CalculateNewValues')
print(text)
PYEOF

cat > _m7calc.asm <<'EOF'
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
EXTERN mode7ab
EXTERN mode7cd
EXTERN mode7A
EXTERN mode7B
EXTERN mode7C
EXTERN mode7D
EXTERN BGMA
EXTERN BG1SXl
EXTERN BG1SYl
EXTERN mode7set
EXTERN m7starty
EXTERN M7SeamA
EXTERN M7SeamB
EXTERN M7SeamC
EXTERN M7SeamSI
EXTERN M7SeamDI
EXTERN M7SeamBP
EXTERN M7SeamD
EXTERN BGMA
EXTERN curvidoffset
EXTERN M7HROn

section .text

; void asm_m7call(void) - load the seam into the register ABI, run the
; original, write the registers back so the test can compare them.
NEWSYM asm_m7call
    push ebx
    push esi
    push edi
    push ebp
    mov eax,[M7SeamA]
    mov ebx,[M7SeamB]
    mov ecx,[M7SeamC]
    mov edx,[M7SeamD]
    call asm_CalculateNewValues
    mov [M7SeamA],eax
    mov [M7SeamB],ebx
    mov [M7SeamC],ecx
    mov [M7SeamD],edx
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

; The renderer, stubbed. Logs the registers, curvidoffset and M7HROn as it was
; reached with them, then clobbers every register the way a real one would.
; The name is taken by the C port's seam, which the C side of the test calls
; instead; this define renames the oracle's call as well as the stub itself.
%define drawmode7win16b asm_drawmode7win16b
SECTION .bss
NEWSYM DrawHits, resd 1
NEWSYM DrawRegs, resd 7
NEWSYM DrawVid,  resd 1
NEWSYM DrawHRon, resd 1
SECTION .text

NEWSYM drawmode7win16b
    mov [DrawRegs],eax
    mov [DrawRegs+4],ebx
    mov [DrawRegs+8],ecx
    mov [DrawRegs+12],edx
    mov [DrawRegs+16],esi
    mov [DrawRegs+20],edi
    mov [DrawRegs+24],ebp
    mov eax,[curvidoffset]
    mov [DrawVid],eax
    mov eax,[M7HROn]
    mov [DrawHRon],eax
    inc dword [DrawHits]
    mov eax,0A5A50001h
    mov ebx,0A5A50002h
    mov ecx,0A5A50003h
    mov edx,0A5A50004h
    mov esi,0A5A50005h
    mov edi,0A5A50006h
    mov ebp,0A5A50007h
    ret

; void asm_m7hires(void) - same seam, but this one also carries the registers
; the renderer is reached with and does not restore.
NEWSYM asm_m7hires
    push ebx
    push esi
    push edi
    push ebp
    mov eax,[M7SeamA]
    mov ebx,[M7SeamB]
    mov ecx,[M7SeamC]
    mov edx,[M7SeamD]
    mov esi,[M7SeamSI]
    mov edi,[M7SeamDI]
    mov ebp,[M7SeamBP]
    call asm_processmode7hires16b
    mov [M7SeamA],eax
    mov [M7SeamB],ebx
    mov [M7SeamC],ecx
    mov [M7SeamD],edx
    mov [M7SeamSI],esi
    mov [M7SeamDI],edi
    mov [M7SeamBP],ebp
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

%include "_m7calc.inc"
EOF

nasm -O1 -f elf32 -w-orphan-labels -o _m7calc.o _m7calc.asm
echo "wrote _m7calc.o (oracle from $(git -C .. rev-parse --short $REV))"
