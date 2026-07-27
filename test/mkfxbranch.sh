#!/bin/sh
# mkfxbranch.sh - build the asm oracle for difftest_fxbranch.c.
#
# Extracts the 22 SuperFX branch handlers as they were before the port (from
# git) into _fxbranch.o, renaming each FxOpXX to asm_FxOpXX, and adds the two
# helpers the difftest needs to talk to the register ABI:
#
#   asm_fxcall(fn)  load the ABI registers from the seam block, call fn,
#                   write them back  (drives the *asm* handler)
#   FxDispatch(tbl) same, but dispatches through tbl[ecx]  (used by the *C*
#                   handler, matching the real seam in chips/fxemu2b.asm)
#   fxstub          stands in for the delay-slot opcode: records what it was
#                   dispatched with, then returns
set -e

REV=${1:-HEAD}
SRC=_fxbranch_src.asm
git -C .. show "$REV:chips/fxemu2b.asm" > "$SRC"

INC=_fxbranch.inc
{ sed -n '81,234p' "$SRC"; sed -n '397,550p' "$SRC"; } > "$INC"

for e in 05 06 07 08 09 0A 0B 0C 0D 0E 0F; do
    for g in b c; do
        sed -i -e "s/NEWSYM FxOp${g}${e}\b/NEWSYM asm_FxOp${g}${e}/" "$INC"
    done
done

cat > _fxbranch.asm <<'EOF'
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
EXTERN FxTableb
EXTERN FxTablec
EXTERN SfxCarry
EXTERN SfxOverflow
EXTERN SfxSignZero
EXTERN FxSeamPC
EXTERN FxSeamSrc
EXTERN FxSeamDst
EXTERN FxSeamCX
EXTERN StubPC
EXTERN StubCX
EXTERN StubSrc
EXTERN StubDst
EXTERN StubHits

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

NEWSYM fxstub
    mov [StubPC],ebp
    mov [StubCX],ecx
    mov [StubSrc],esi
    mov [StubDst],edi
    inc dword [StubHits]
    ret

%include "_fxbranch.inc"
EOF

nasm -f elf32 -w-orphan-labels -o _fxbranch.o _fxbranch.asm
echo "wrote _fxbranch.o (oracle from $REV)"
