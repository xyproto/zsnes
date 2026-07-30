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
        git -C .. cat-file -e "$r:cpu/memory.asm" 2>/dev/null || continue
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

sed -i -E 's/^NEWSYM ((membank0|memaccessbank|wramaccessbank|eramaccessbank|sramaccessbank|stsram|regaccessbank|SA1RAMaccessbank)[A-Za-z0-9]+)/NEWSYM asm_\1/' _memops.inc
sed -i -E 's/\b(call|jmp) (sramaccessbank[A-Za-z0-9]+)/\1 asm_\2/' _memops.inc
sed -i -E 's/\b(call|jmp) (SA1RAMaccessbank[A-Za-z0-9]+)/\1 asm_\2/' _memops.inc
# The general dispatchers tail-jump to the SA-1 variants; keep the oracle on
# its own copies rather than letting it fall into the ported ones.
sed -i -E 's/\b(je|jmp) near (membank0[A-Za-z0-9]+SA1)/\1 near asm_\2/' _memops.inc
sed -i -E 's/^( *STsramaccess )(memaccessbank[A-Za-z0-9]+)/\1asm_\2/' _memops.inc
sed -i -E 's/^( *SRAMAccess )(memaccessbank[A-Za-z0-9]+)/\1asm_\2/' _memops.inc

# The extracted handlers reach the register tables through the macros in
# cpu/regs.mac and cpu/regsw.mac; take those from the same revision.
# The BWCheck2* macro bodies, straight from the same pre-port memory.asm, so
# the oracle's SA-1 BW-RAM paths are the originals.
python3 - _memops_src.asm > _memops_bw.mac <<'PYEOF'
import re, sys
src = open(sys.argv[1]).read().split('\n')
out, on = [], False
for l in src:
    if re.match(r'%macro BWCheck2', l):
        on = True
    if on:
        out.append(l)
    if on and l.strip() == '%endmacro':
        on = False
print('\n'.join(out))
PYEOF

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
EXTERN ram7fa
EXTERN sram
EXTERN sram2
EXTERN SFXEnable
EXTERN SA1Enable
EXTERN SA1Status
EXTERN IRAM
EXTERN SA1RAMArea
EXTERN SA1_in_cc1_dma
EXTERN SA1_DMA_ADDR
EXTERN SA1_DMA_VALUE
EXTERN SA1_DMA_CC1
EXTERN BWShift
EXTERN SA1BWPtr
EXTERN CurBWPtr
EXTERN SA1Overflow
EXTERN DSP1Type
EXTERN sfxramdata
EXTERN DSP1Read8b
EXTERN DSP1Write8b
EXTERN DSP1Read16b
EXTERN DSP1Write16b
EXTERN ramsize
EXTERN ramsizeand
EXTERN sramb4save
EXTERN regptra
EXTERN regptwa
EXTERN cpu_mdr
EXTERN writeon
EXTERN curromspace
EXTERN snesmmap
EXTERN MemSeamB
EXTERN MemSeamC
EXTERN MemSeamA
EXTERN MemSeamD
EXTERN StubRegAddr
EXTERN StubRegVal
EXTERN StubRegHits
EXTERN StubReenter
EXTERN StubRegEdx

%macro STsramaccess 1
    test ecx,8000h
    jz %1
%endmacro

; The large-cart guard, as cpu/memory.asm defines it.
%macro SRAMAccess 1
    cmp dword[curromspace],0x200000
    ja .large
    cmp  dword[ramsize],0x8000
    ja .large
    jmp .notlarge
.large
    test ecx,8000h
    jnz %1
.notlarge
%endmacro

; BW-RAM byte view or bit map, as cpu/memory.asm defines it.
%macro BWCheck 0
    cmp byte[BWShift],0
    jne near .shift
.nosa1
%endmacro

; cdecl call preserving eax, as cpu/memory.asm defines it.
%macro ccall 1-*
	push ecx
	push edx
%rep %0 - 1
%rotate -1
	push dword %1
%endrep
%rotate -1
	call %1
%if %0 != 1
	add esp, (%0 - 1) * 4
%endif
	pop edx
	pop ecx
%endmacro

%macro ccallv 1+
	push eax
	ccall %1
	pop eax
%endmacro

%include "_memops_bw.mac"
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
    mov edx,[MemSeamD]
    push eax
    mov eax,[MemSeamA]
    call dword [esp]
    mov [MemSeamA],eax
    add esp,4
    mov [MemSeamB],ebx
    mov [MemSeamC],ecx
    mov [MemSeamD],edx
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
    mov [StubRegEdx+ebx*4],edx
    inc dword [StubRegHits]
    ; Model a register write that starts a DMA: the transfer runs through the
    ; ported handlers, which keep their state in the seam block, so by the time
    ; the outer access resumes the seam holds someone else's address. The
    ; assembly never noticed because it kept that in ecx; a C port has to save
    ; and restore it. Registers themselves stay untouched, as a real callee's
    ; would.
    cmp dword [StubReenter],0
    je %%noreenter
    mov dword [MemSeamB],0xDEAD0000
    mov dword [MemSeamC],0xDEAD0001
    mov dword [MemSeamA],0xDEAD0002
%%noreenter:
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

nasm -O1 -f elf32 -w-orphan-labels -o _memops.o _memops.asm
echo "wrote _memops.o (oracle from $(git -C .. rev-parse --short $REV), $(grep -c '^NEWSYM asm_' _memops.inc) handlers)"
