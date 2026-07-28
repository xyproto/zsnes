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
#                   tell which table a handler chained through. SfxB is captured
#                   too: WITH sets it purely so the *nested* opcode sees it.
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

# The d table (chips/fxemu2c.asm) threads rather than calls: each handler
# tail-jumps to the next through FXReturn. Extract those handlers too, and emit
# a matching thunk per handler so the C side runs the real seam and tail-chain
# rather than the bare C body.
git -C .. show "$REV:chips/fxemu2c.asm" > _fxops_d.asm
python3 - _fxops_d.asm >> _fxops.inc <<'PYEOF'
import re, sys
src = open(sys.argv[1]).read().split('\n')
out = []
cur = None
for l in src:
    m = re.match(r'NEWSYM (FxOpd[A-Za-z0-9]+)', l.strip())
    if m:
        cur = m.group(1)
        out.append(l)
        continue
    if cur and not re.match(r'(SECTION |%)', l.strip()):
        out.append(l)
    elif cur:
        cur = None
print('\n'.join(out))
PYEOF

# Rename every entry point so the oracle does not clash with the real symbols,
# including handler-to-handler calls and jumps (LJMP calls the CACHE opcode,
# PLOT jumps to its 4bpp variant) so the
# oracle keeps calling its own pre-port copy rather than the ported one.
sed -i -E 's/^NEWSYM (FxOp[A-Za-z0-9]+)/NEWSYM asm_\1/' _fxops.inc
sed -i -E 's/\b(call|jmp) (FxOp[A-Za-z0-9]+)/\1 asm_\2/' _fxops.inc

# The TO/FROM macro bodies live in a .mac the port deleted; take it from git
# too, along with fxemu2.mac for FETCHPIPE / UpdateR14 / CLRFLAGS.

git -C .. show "$REV:chips/fxemu2.mac" > _fxops_m1.mac
git -C .. show "$REV:chips/fxemu2b.mac" > _fxops_m2.mac
git -C .. show "$REV:chips/fxemu2c.mac" > _fxops_m3.mac
sed -i -E 's/\b(call|jmp) (FxOp[A-Za-z0-9]+)/\1 asm_\2/' _fxops_m1.mac _fxops_m2.mac _fxops_m3.mac

cat > _fxops.asm <<'EOF'
bits 32
%define ALIGN32 align 32
%define ALIGN16 align 16
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
EXTERN SfxR11
EXTERN SfxR13
EXTERN SfxR12
EXTERN SfxR6
EXTERN SfxR4
EXTERN SfxR14
EXTERN SfxR15
EXTERN SfxRomBuffer
EXTERN SfxRAMMem
EXTERN SfxR2
EXTERN SfxR1
EXTERN flagnz
EXTERN fxxand
EXTERN SCBRrel
EXTERN SfxSCBR
EXTERN SFXProc
EXTERN ChangeOps
EXTERN NumberOfOpcodes
EXTERN SfxPIPE
EXTERN SfxCFGR
EXTERN SfxSFR
EXTERN SfxR8
EXTERN SfxR7
EXTERN FxTabled
EXTERN PLOTJmpb
EXTERN PLOTJmpa
EXTERN sfxobjlineloc
EXTERN sfx192lineloc
EXTERN sfx160lineloc
EXTERN sfx128lineloc
EXTERN sfxclineloc
EXTERN fxbit67pcal
EXTERN fxbit45pcal
EXTERN fxbit23pcal
EXTERN fxbit01pcal
EXTERN fxbit67
EXTERN fxbit45
EXTERN fxbit23
EXTERN fxbit01
EXTERN SfxSCMR
EXTERN SfxPOR
EXTERN SfxCOLR
EXTERN sfxramdata
EXTERN SfxnRamBanks
EXTERN SfxROMBR
EXTERN SfxRAMBR
EXTERN SfxLastRamAdr
EXTERN SfxCBR
EXTERN SfxPBR
EXTERN SfxCacheActive
EXTERN SfxMemTable
EXTERN FlushCache
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
EXTERN StubB
EXTERN StubR15sk
EXTERN StubEndLoop
EXTERN StubPlotIdx
EXTERN StubPlotHits
EXTERN FxTabled
EXTERN StubR15
EXTERN StubWrR15sk

%include "_fxops_m1.mac"
%include "_fxops_m2.mac"
%include "_fxops_m3.mac"

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
    mov eax,[SfxB]
    mov [StubB],eax
    ; A real nested opcode may set R15 and claim the jump as its own; let the
    ; test drive both, or the guards around R15 are unobservable. Only some
    ; opcodes touch withr15sk, so leaving it alone has to be reachable too.
    mov eax,[StubWrR15sk]
    test eax,eax
    jz %%nor15sk
    mov eax,[StubR15sk]
    mov [withr15sk],eax
%%nor15sk:
    mov eax,[StubR15]
    mov [SfxR15],eax
    inc dword [StubHits]
    ret
%endmacro

FXSTUB fxstub, 1
FXSTUB fxstubb, 2
FXSTUB fxstubc, 3

%include "_fxops.inc"

; One thunk per d-table handler, identical to the fxdop macro in
; chips/fxemu2c.asm, so the ported side is exercised through the real seam.
%macro fxdop 1
    mov [FxSeamPC], ebp
    mov [FxSeamSrc], esi
    mov [FxSeamDst], edi
    mov [FxSeamCX], ecx
    ccall %1
    mov ebp, [FxSeamPC]
    mov esi, [FxSeamSrc]
    mov edi, [FxSeamDst]
    mov ecx, [FxSeamCX]
    FXReturn
%endmacro
%macro fxdopend 1
    mov [FxSeamPC], ebp
    mov [FxSeamSrc], esi
    mov [FxSeamDst], edi
    mov [FxSeamCX], ecx
    ccall %1
    mov ebp, [FxSeamPC]
    mov esi, [FxSeamSrc]
    mov edi, [FxSeamDst]
    mov ecx, [FxSeamCX]
    jmp FXEndLoop
%endmacro

%include "_fxops_thunks.inc"

; CMODE patches a PLOTJmp entry into FxTabled[$4C] and the d table then
; tail-jumps through it, so those entries have to be real code. 128 stubs, each
; recording its own index, keep the jump safe while still letting the test tell
; a wrong table or a wrong index apart.
ALIGN 32
NEWSYM plotstubs
%assign plotidx 0
%rep 128
    mov dword [StubPlotIdx], plotidx
    inc dword [StubPlotHits]
    ret
    ALIGN 32
%assign plotidx plotidx+1
%endrep

; The real loop epilogue lives in fxemu2.asm; the difftest only needs to know
; it was reached.
NEWSYM FXEndLoop
    inc dword [StubEndLoop]
    ret
EOF

# Thunks for the C side, one per (handler, C body) pair.
: > _fxops_thunks.inc
while read -r d c; do
    [ -n "$d" ] || continue
    echo "EXTERN c_$c" >> _fxops_thunks.inc
done < ../test/fxops_d.list
while read -r d c; do
    [ -n "$d" ] || continue
    if [ "$d" = "FxOpd00" ]; then
        printf 'NEWSYM cthunk_%s\n    fxdopend c_%s\n' "$d" "$c" >> _fxops_thunks.inc
    else
        printf 'NEWSYM cthunk_%s\n    fxdop c_%s\n' "$d" "$c" >> _fxops_thunks.inc
    fi
done < ../test/fxops_d.list

nasm -f elf32 -w-orphan-labels -o _fxops.o _fxops.asm
echo "wrote _fxops.o (oracle from $(git -C .. rev-parse --short $REV), $(grep -c '^NEWSYM asm_FxOp' _fxops.inc) handlers)"
