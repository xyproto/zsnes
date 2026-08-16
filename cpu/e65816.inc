;Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
;
;http://www.zsnes.com
;http://sourceforge.net/projects/zsnes
;https://zsnes.bountysource.com
;
;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;version 2 as published by the Free Software Foundation.
;
;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.
;
;You should have received a copy of the GNU General Public License
;along with this program; if not, write to the Free Software
;Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



EXTSYM brkv,brkv8,copv,copv8,curexecstate,curnmi,initaddrl
EXTSYM membank0r16,membank0r8,membank0w16,membank0w8
EXTSYM memtabler16,memtabler8,memtablew16,memtablew8
EXTSYM nmistatus,regaccessbankr8,snesmap2,snesmmap,stackand,stackor,tablead
EXTSYM wramdata,xa,xd,xdb,xe,xirqb,xpb,xpc,xs,xx,xy
EXTSYM intrset,UpdateDPage,switchtovirq,execloop

NEWSYM E65816IncStart

; B = ( or )
; C = ,
; L = [ or ]
; I = #

SECTION .text

NEWSYM COp61m8nd  ; ADC(d,x)
    cop c_COp61m8nd
NEWSYM COp61m16nd ; ADC(d,x)
    cop c_COp61m16nd
NEWSYM COp61m8d   ; ADC(d,x)
    cop c_COp61m8d
NEWSYM COp61m16d  ; ADC(d,x)
    cop c_COp61m16d
NEWSYM COp63m8nd  ; ADC d,s
    cop c_COp63m8nd
NEWSYM COp63m16nd ; ADC d,s
    cop c_COp63m16nd
NEWSYM COp63m8d   ; ADC d,s
    cop c_COp63m8d
NEWSYM COp63m16d  ; ADC d,s
    cop c_COp63m16d
NEWSYM COp65m8nd  ; ADC d
    cop c_COp65m8nd
NEWSYM COp65m16nd ; ADC d
    cop c_COp65m16nd
NEWSYM COp65m8d   ; ADC d
    cop c_COp65m8d
NEWSYM COp65m16d  ; ADC d
    cop c_COp65m16d
NEWSYM COp67m8nd  ; ADC [d]
    cop c_COp67m8nd
NEWSYM COp67m16nd ; ADC [d]
    cop c_COp67m16nd
NEWSYM COp67m8d   ; ADC [d]
    cop c_COp67m8d
NEWSYM COp67m16d  ; ADC [d]
    cop c_COp67m16d
NEWSYM COp69m8nd  ; ADC #
    cop c_COp69m8nd
NEWSYM COp69m16nd ; ADC #
    cop c_COp69m16nd
NEWSYM COp69m8d   ; ADC #
    cop c_COp69m8d
NEWSYM COp69m16d  ; ADC #
    cop c_COp69m16d
NEWSYM COp6Dm8nd  ; ADC a
    cop c_COp6Dm8nd
NEWSYM COp6Dm16nd ; ADC a
    cop c_COp6Dm16nd
NEWSYM COp6Dm8d   ; ADC a
    cop c_COp6Dm8d
NEWSYM COp6Dm16d  ; ADC a
    cop c_COp6Dm16d
NEWSYM COp6Fm8nd  ; ADC al
    cop c_COp6Fm8nd
NEWSYM COp6Fm16nd ; ADC al
    cop c_COp6Fm16nd
NEWSYM COp6Fm8d   ; ADC al
    cop c_COp6Fm8d
NEWSYM COp6Fm16d  ; ADC al
    cop c_COp6Fm16d
NEWSYM COp71m8nd  ; ADC(d),y
    cop c_COp71m8nd
NEWSYM COp71m16nd ; ADC(d),y
    cop c_COp71m16nd
NEWSYM COp71m8d   ; ADC(d),y
    cop c_COp71m8d
NEWSYM COp71m16d  ; ADC(d),y
    cop c_COp71m16d
NEWSYM COp72m8nd  ; ADC (d)
    cop c_COp72m8nd
NEWSYM COp72m16nd ; ADC (d)
    cop c_COp72m16nd
NEWSYM COp72m8d   ; ADC (d)
    cop c_COp72m8d
NEWSYM COp72m16d  ; ADC (d)
    cop c_COp72m16d
NEWSYM COp73m8nd  ; ADC(d,s),y
    cop c_COp73m8nd
NEWSYM COp73m16nd ; ADC(d,s),y
    cop c_COp73m16nd
NEWSYM COp73m8d   ; ADC(d,s),y
    cop c_COp73m8d
NEWSYM COp73m16d  ; ADC(d,s),y
    cop c_COp73m16d
NEWSYM COp75m8nd  ; ADC d,x
    cop c_COp75m8nd
NEWSYM COp75m16nd ; ADC d,x
    cop c_COp75m16nd
NEWSYM COp75m8d   ; ADC d,x
    cop c_COp75m8d
NEWSYM COp75m16d  ; ADC d,x
    cop c_COp75m16d
NEWSYM COp77m8nd  ; ADC [d],y
    cop c_COp77m8nd
NEWSYM COp77m16nd ; ADC [d],y
    cop c_COp77m16nd
NEWSYM COp77m8d   ; ADC [d],y
    cop c_COp77m8d
NEWSYM COp77m16d  ; ADC [d],y
    cop c_COp77m16d
NEWSYM COp79m8nd  ; ADC a,y
    cop c_COp79m8nd
NEWSYM COp79m16nd ; ADC a,y
    cop c_COp79m16nd
NEWSYM COp79m8d   ; ADC a,y
    cop c_COp79m8d
NEWSYM COp79m16d  ; ADC a,y
    cop c_COp79m16d
NEWSYM COp7Dm8nd  ; ADC a,x
    cop c_COp7Dm8nd
NEWSYM COp7Dm16nd ; ADC a,x
    cop c_COp7Dm16nd
NEWSYM COp7Dm8d   ; ADC a,x
    cop c_COp7Dm8d
NEWSYM COp7Dm16d  ; ADC a,x
    cop c_COp7Dm16d
NEWSYM COp7Fm8nd  ; ADC al,x
    cop c_COp7Fm8nd
NEWSYM COp7Fm16nd ; ADC al,x
    cop c_COp7Fm16nd
NEWSYM COp7Fm8d   ; ADC al,x
    cop c_COp7Fm8d
NEWSYM COp7Fm16d  ; ADC al,x
    cop c_COp7Fm16d

NEWSYM COp21m8 ; AND(d,x)
    cop c_COp21m8
NEWSYM COp21m16; AND(d,x)
    cop c_COp21m16
NEWSYM COp23m8 ; AND d,s
    cop c_COp23m8
NEWSYM COp23m16; AND d,s
    cop c_COp23m16
NEWSYM COp25m8 ; AND d
    cop c_COp25m8
NEWSYM COp25m16; AND d
    cop c_COp25m16
NEWSYM COp27m8 ; AND [d]
    cop c_COp27m8
NEWSYM COp27m16; AND [d]
    cop c_COp27m16
NEWSYM COp29m8 ; AND #
    cop c_COp29m8
NEWSYM COp29m16; AND #
    cop c_COp29m16
NEWSYM COp2Dm8 ; AND a
    cop c_COp2Dm8
NEWSYM COp2Dm16; AND a
    cop c_COp2Dm16
NEWSYM COp2Fm8 ; AND al
    cop c_COp2Fm8
NEWSYM COp2Fm16; AND al
    cop c_COp2Fm16
NEWSYM COp31m8 ; AND(d),y
    cop c_COp31m8
NEWSYM COp31m16; AND(d),y
    cop c_COp31m16
NEWSYM COp32m8 ; AND (d)
    cop c_COp32m8
NEWSYM COp32m16; AND (d)
    cop c_COp32m16
NEWSYM COp33m8 ; AND(d,s),y
    cop c_COp33m8
NEWSYM COp33m16; AND(d,s),y
    cop c_COp33m16
NEWSYM COp35m8 ; AND d,x
    cop c_COp35m8
NEWSYM COp35m16; AND d,x
    cop c_COp35m16
NEWSYM COp37m8 ; AND [d],y
    cop c_COp37m8
NEWSYM COp37m16; AND [d],y
    cop c_COp37m16
NEWSYM COp39m8 ; AND a,y
    cop c_COp39m8
NEWSYM COp39m16; AND a,y
    cop c_COp39m16
NEWSYM COp3Dm8 ; AND a,x
    cop c_COp3Dm8
NEWSYM COp3Dm16; AND a,x
    cop c_COp3Dm16
NEWSYM COp3Fm8 ; AND al,x
    cop c_COp3Fm8
NEWSYM COp3Fm16; AND al,x
    cop c_COp3Fm16

NEWSYM COp06m8 ; ASL d
    cop c_COp06m8
NEWSYM COp06m16; ASL d
    cop c_COp06m16
NEWSYM COp0Am8 ; ASL A
    cop c_COp0Am8
NEWSYM COp0Am16; ASL A
    cop c_COp0Am16
NEWSYM COp0Em8 ; ASL a
    cop c_COp0Em8
NEWSYM COp0Em16; ASL a
    cop c_COp0Em16
NEWSYM COp16m8 ; ASL d,x
    cop c_COp16m8
NEWSYM COp16m16; ASL d,x
    cop c_COp16m16
NEWSYM COp1Em8 ; ASL a,x
    cop c_COp1Em8
NEWSYM COp1Em16; ASL a,x
    cop c_COp1Em16

NEWSYM COp90   ; BCC r
    cop c_COp90
NEWSYM COpB0   ; BCS r
    cop c_COpB0
NEWSYM COpF0   ; BEQ r
    cop c_COpF0
NEWSYM COp30   ; BMI r
    cop c_COp30
NEWSYM COpD0   ; BNE r
    cop c_COpD0
NEWSYM COp10   ; BPL r
    cop c_COp10
NEWSYM COp80   ; BRA r
    cop c_COp80
NEWSYM COp50   ; BVC r
    cop c_COp50
NEWSYM COp70   ; BVS r
    cop c_COp70

NEWSYM COp89m8 ; BIT #
    cop c_COp89m8
NEWSYM COp89m16; BIT #
    cop c_COp89m16

NEWSYM COp24m8 ; BIT d
    cop c_COp24m8
NEWSYM COp24m16; BIT d
    cop c_COp24m16
NEWSYM COp2Cm8 ; BIT a
    cop c_COp2Cm8
NEWSYM COp2Cm16; BIT a
    cop c_COp2Cm16
NEWSYM COp34m8 ; BIT d,x
    cop c_COp34m8
NEWSYM COp34m16; BIT d,x
    cop c_COp34m16
NEWSYM COp3Cm8 ; BIT a,x
    cop c_COp3Cm8
NEWSYM COp3Cm16; BIT a,x
    cop c_COp3Cm16

NEWSYM COp00   ; BRK s
    cop c_COp00

NEWSYM COp82   ; BRL rl
    cop c_COp82

NEWSYM COp18   ; CLC i
    cop c_COp18
NEWSYM COpD8  ; CLD i
    cop c_COpD8
NEWSYM COp58   ; CLI i
    ; CLI is the one ported opcode that can restart the dispatch loop
    ; rather than return into it, so it needs its own thunk. popad leaves
    ; the flags from the test alone.
    pushad
    mov eax, esp
    ccall c_COp58, eax
    test eax, eax
    popad
    jnz .virqemul
    endloop
.virqemul
    xor ebx, ebx
    jmp execloop
NEWSYM COpB8   ; CLV i
    cop c_COpB8

NEWSYM COpC1m8 ; CMP(d,x)
    cop c_COpC1m8
NEWSYM COpC1m16; CMP(d,x)
    cop c_COpC1m16
NEWSYM COpC3m8 ; CMP d,s
    cop c_COpC3m8
NEWSYM COpC3m16; CMP d,s
    cop c_COpC3m16
NEWSYM COpC5m8 ; CMP d
    cop c_COpC5m8
NEWSYM COpC5m16; CMP d
    cop c_COpC5m16
NEWSYM COpC7m8 ; CMP [d]
    cop c_COpC7m8
NEWSYM COpC7m16; CMP [d]
    cop c_COpC7m16
NEWSYM COpC9m8 ; CMP #
    cop c_COpC9m8
NEWSYM COpC9m16; CMP #
    cop c_COpC9m16
NEWSYM COpCDm8 ; CMP a
    cop c_COpCDm8
NEWSYM COpCDm16; CMP a
    cop c_COpCDm16
NEWSYM COpCFm8 ; CMP al
    cop c_COpCFm8
NEWSYM COpCFm16; CMP al
    cop c_COpCFm16
NEWSYM COpD1m8 ; CMP(d),y
    cop c_COpD1m8
NEWSYM COpD1m16; CMP(d),y
    cop c_COpD1m16
NEWSYM COpD2m8 ; CMP (d)
    cop c_COpD2m8
NEWSYM COpD2m16; CMP (d)
    cop c_COpD2m16
NEWSYM COpD3m8 ; CMP(d,s),y
    cop c_COpD3m8
NEWSYM COpD3m16; CMP(d,s),y
    cop c_COpD3m16
NEWSYM COpD5m8 ; CMP d,x
    cop c_COpD5m8
NEWSYM COpD5m16; CMP d,x
    cop c_COpD5m16
NEWSYM COpD7m8 ; CMP [d],y
    cop c_COpD7m8
NEWSYM COpD7m16; CMP [d],y
    cop c_COpD7m16
NEWSYM COpD9m8 ; CMP a,y
    cop c_COpD9m8
NEWSYM COpD9m16; CMP a,y
    cop c_COpD9m16
NEWSYM COpDDm8 ; CMP a,x
    cop c_COpDDm8
NEWSYM COpDDm16; CMP a,x
    cop c_COpDDm16
NEWSYM COpDFm8 ; CMP al,x
    cop c_COpDFm8
NEWSYM COpDFm16; CMP al,x
    cop c_COpDFm16

NEWSYM COp02   ; COP s
    cop c_COp02

NEWSYM COpE0x8 ; CPX #
    cop c_COpE0x8
NEWSYM COpE0x16; CPX #
    cop c_COpE0x16
NEWSYM COpE4x8 ; CPX d
    cop c_COpE4x8
NEWSYM COpE4x16; CPX d
    cop c_COpE4x16
NEWSYM COpECx8 ; CPX a
    cop c_COpECx8
NEWSYM COpECx16; CPX a
    cop c_COpECx16

NEWSYM COpC0x8 ; CPY #
    cop c_COpC0x8
NEWSYM COpC0x16; CPY #
    cop c_COpC0x16
NEWSYM COpC4x8 ; CPY d
    cop c_COpC4x8
NEWSYM COpC4x16; CPY d
    cop c_COpC4x16
NEWSYM COpCCx8 ; CPY a
    cop c_COpCCx8
NEWSYM COpCCx16; CPY a
    cop c_COpCCx16

NEWSYM COp3Am8 ; DEC A
    cop c_COp3Am8
NEWSYM COp3Am16; DEC A
    cop c_COp3Am16
NEWSYM COpCEm8 ; DEC a
    cop c_COpCEm8
NEWSYM COpCEm16; DEC a
    cop c_COpCEm16
NEWSYM COpC6m8 ; DEC d
    cop c_COpC6m8
NEWSYM COpC6m16; DEC d
    cop c_COpC6m16
NEWSYM COpD6m8 ; DEC d,x
    cop c_COpD6m8
NEWSYM COpD6m16; DEC d,x
    cop c_COpD6m16
NEWSYM COpDEm8 ; DEC a,x
    cop c_COpDEm8
NEWSYM COpDEm16; DEC a,x
    cop c_COpDEm16

NEWSYM COpCAx8 ; DEX i
    cop c_COpCAx8
NEWSYM COpCAx16; DEX i
    cop c_COpCAx16
NEWSYM COp88x8 ; DEY i
    cop c_COp88x8
NEWSYM COp88x16; DEY i
    cop c_COp88x16

NEWSYM COp41m8 ; EOR(d,x)
    cop c_COp41m8
NEWSYM COp41m16; EOR(d,x)
    cop c_COp41m16
NEWSYM COp43m8 ; EOR d,s
    cop c_COp43m8
NEWSYM COp43m16; EOR d,s
    cop c_COp43m16
NEWSYM COp45m8 ; EOR d
    cop c_COp45m8
NEWSYM COp45m16; EOR d
    cop c_COp45m16
NEWSYM COp47m8 ; EOR [d]
    cop c_COp47m8
NEWSYM COp47m16; EOR [d]
    cop c_COp47m16
NEWSYM COp49m8 ; EOR #
    cop c_COp49m8
NEWSYM COp49m16; EOR #
    cop c_COp49m16
NEWSYM COp4Dm8 ; EOR a
    cop c_COp4Dm8
NEWSYM COp4Dm16; EOR a
    cop c_COp4Dm16
NEWSYM COp4Fm8 ; EOR al
    cop c_COp4Fm8
NEWSYM COp4Fm16; EOR al
    cop c_COp4Fm16
NEWSYM COp51m8 ; EOR(d),y
    cop c_COp51m8
NEWSYM COp51m16; EOR(d),y
    cop c_COp51m16
NEWSYM COp52m8 ; EOR (d)
    cop c_COp52m8
NEWSYM COp52m16; EOR (d)
    cop c_COp52m16
NEWSYM COp53m8 ; EOR(d,s),y
    cop c_COp53m8
NEWSYM COp53m16; EOR(d,s),y
    cop c_COp53m16
NEWSYM COp55m8 ; EOR d,x
    cop c_COp55m8
NEWSYM COp55m16; EOR d,x
    cop c_COp55m16
NEWSYM COp57m8 ; EOR [d],y
    cop c_COp57m8
NEWSYM COp57m16; EOR [d],y
    cop c_COp57m16
NEWSYM COp59m8 ; EOR a,y
    cop c_COp59m8
NEWSYM COp59m16; EOR a,y
    cop c_COp59m16
NEWSYM COp5Dm8 ; EORa,x
    cop c_COp5Dm8
NEWSYM COp5Dm16; EORa,x
    cop c_COp5Dm16
NEWSYM COp5Fm8 ; EOR al,x
    cop c_COp5Fm8
NEWSYM COp5Fm16; EOR al,x
    cop c_COp5Fm16


NEWSYM COp1Am8 ; INC A
    cop c_COp1Am8
NEWSYM COp1Am16; INC A
    cop c_COp1Am16
NEWSYM COpEEm8 ; INC a
    cop c_COpEEm8
NEWSYM COpEEm16; INC a
    cop c_COpEEm16
NEWSYM COpE6m8 ; INC d
    cop c_COpE6m8
NEWSYM COpE6m16; INC d
    cop c_COpE6m16
NEWSYM COpF6m8 ; INC d,x
    cop c_COpF6m8
NEWSYM COpF6m16; INC d,x
    cop c_COpF6m16
NEWSYM COpFEm8 ; INC a,x
    cop c_COpFEm8
NEWSYM COpFEm16; INC a,x
    cop c_COpFEm16


NEWSYM COpE8x8 ; INX i
    cop c_COpE8x8
NEWSYM COpE8x16; INX i
    cop c_COpE8x16
NEWSYM COpC8x8 ; INY i
    cop c_COpC8x8
NEWSYM COpC8x16; INY i
    cop c_COpC8x16

NEWSYM COpDC   ; JML (a)
    cop c_COpDC
NEWSYM COp4C   ; JMP a
    cop c_COp4C
NEWSYM COp6C   ; JMP (a)
    cop c_COp6C
NEWSYM COp7C   ; JMP(a,x)
    cop c_COp7C
NEWSYM COp5C   ; JMP al
    cop c_COp5C

NEWSYM COp22   ; JSL al
    cop c_COp22
NEWSYM COp20   ; JSR a
    cop c_COp20
NEWSYM COpFC   ; JSR(a,x)
    cop c_COpFC

NEWSYM COpA1m8 ; LDA(d,x)
    cop c_COpA1m8
NEWSYM COpA1m16; LDA(d,x)
    cop c_COpA1m16
NEWSYM COpA3m8 ; LDA d,s
    cop c_COpA3m8
NEWSYM COpA3m16; LDA d,s
    cop c_COpA3m16
NEWSYM COpA5m8 ; LDA d
    cop c_COpA5m8
NEWSYM COpA5m16; LDA d
    cop c_COpA5m16
NEWSYM COpA7m8 ; LDA [d]
    cop c_COpA7m8
NEWSYM COpA7m16; LDA [d]
    cop c_COpA7m16
NEWSYM COpA9m8 ; LDA #
    cop c_COpA9m8
NEWSYM COpA9m16; LDA #
    cop c_COpA9m16
NEWSYM COpADm8 ; LDA a
    cop c_COpADm8
NEWSYM COpADm16; LDA a
    cop c_COpADm16
NEWSYM COpAFm8 ; LDA al
    cop c_COpAFm8
NEWSYM COpAFm16; LDA al
    cop c_COpAFm16
NEWSYM COpB1m8 ; LDA(d),y
    cop c_COpB1m8
NEWSYM COpB1m16; LDA(d),y
    cop c_COpB1m16
NEWSYM COpB2m8 ; LDA (d)
    cop c_COpB2m8
NEWSYM COpB2m16; LDA (d)
    cop c_COpB2m16
NEWSYM COpB3m8 ; LDA(d,s),y
    cop c_COpB3m8
NEWSYM COpB3m16; LDA(d,s),y
    cop c_COpB3m16
NEWSYM COpB5m8 ; LDA d,x
    cop c_COpB5m8
NEWSYM COpB5m16; LDA d,x
    cop c_COpB5m16
NEWSYM COpB7m8 ; LDA [d],y
    cop c_COpB7m8
NEWSYM COpB7m16; LDA [d],y
    cop c_COpB7m16
NEWSYM COpB9m8 ; LDA a,y
    cop c_COpB9m8
NEWSYM COpB9m16; LDA a,y
    cop c_COpB9m16
NEWSYM COpBDm8 ; LDA a,x
    cop c_COpBDm8
NEWSYM COpBDm16; LDA a,x
    cop c_COpBDm16
NEWSYM COpBFm8 ; LDA al,x
    cop c_COpBFm8
NEWSYM COpBFm16; LDA al,x
    cop c_COpBFm16

NEWSYM COpA2x8 ; LDX #
    cop c_COpA2x8
NEWSYM COpA2x16; LDX #
    cop c_COpA2x16
NEWSYM COpA6x8 ; LDX d
    cop c_COpA6x8
NEWSYM COpA6x16; LDX d
    cop c_COpA6x16
NEWSYM COpAEx8 ; LDX a
    cop c_COpAEx8
NEWSYM COpAEx16; LDX a
    cop c_COpAEx16
NEWSYM COpB6x8 ; LDX d,y
    cop c_COpB6x8
NEWSYM COpB6x16; LDX d,y
    cop c_COpB6x16
NEWSYM COpBEx8 ; LDX a,y
    cop c_COpBEx8
NEWSYM COpBEx16; LDX a,y
    cop c_COpBEx16

NEWSYM COpA0x8 ; LDY #
    cop c_COpA0x8
NEWSYM COpA0x16; LDY #
    cop c_COpA0x16
NEWSYM COpA4x8 ; LDY d
    cop c_COpA4x8
NEWSYM COpA4x16; LDY d
    cop c_COpA4x16
NEWSYM COpACx8 ; LDY a
    cop c_COpACx8
NEWSYM COpACx16; LDY a
    cop c_COpACx16
NEWSYM COpB4x8 ; LDY d,x
    cop c_COpB4x8
NEWSYM COpB4x16; LDY d,x
    cop c_COpB4x16
NEWSYM COpBCx8 ; LDY a,x
    cop c_COpBCx8
NEWSYM COpBCx16; LDY a,x
    cop c_COpBCx16

NEWSYM COp46m8 ; LSR d
    cop c_COp46m8
NEWSYM COp46m16; LSR d
    cop c_COp46m16
NEWSYM COp4Am8 ; LSR A
    cop c_COp4Am8
NEWSYM COp4Am16; LSR A
    cop c_COp4Am16
NEWSYM COp4Em8 ; LSR a
    cop c_COp4Em8
NEWSYM COp4Em16; LSR a
    cop c_COp4Em16
NEWSYM COp56m8 ; LSR d,x
    cop c_COp56m8
NEWSYM COp56m16; LSR d,x
    cop c_COp56m16
NEWSYM COp5Em8 ; LSRa,x
    cop c_COp5Em8
NEWSYM COp5Em16; LSRa,x
    cop c_COp5Em16

NEWSYM COp54   ; MVN xya
    cop c_COp54
NEWSYM COp44   ; MVP xya
    cop c_COp44

NEWSYM COpEA   ; NOP i
    cop c_COpEA

NEWSYM COp01m8 ; ORA(d,x)
    cop c_COp01m8
NEWSYM COp01m16; ORA(d,x)
    cop c_COp01m16
NEWSYM COp03m8 ; ORA d,s
    cop c_COp03m8
NEWSYM COp03m16; ORA d,s
    cop c_COp03m16
NEWSYM COp05m8 ; ORA d
    cop c_COp05m8
NEWSYM COp05m16; ORA d
    cop c_COp05m16
NEWSYM COp07m8 ; ORA [d]
    cop c_COp07m8
NEWSYM COp07m16; ORA [d]
    cop c_COp07m16
NEWSYM COp09m8 ; ORA #
    cop c_COp09m8
NEWSYM COp09m16; ORA #
    cop c_COp09m16
NEWSYM COp0Dm8 ; ORA a
    cop c_COp0Dm8
NEWSYM COp0Dm16; ORA a
    cop c_COp0Dm16
NEWSYM COp0Fm8 ; ORA al
    cop c_COp0Fm8
NEWSYM COp0Fm16; ORA al
    cop c_COp0Fm16
NEWSYM COp11m8 ; ORA(d),y
    cop c_COp11m8
NEWSYM COp11m16; ORA(d),y
    cop c_COp11m16
NEWSYM COp12m8 ; ORA(d)
    cop c_COp12m8
NEWSYM COp12m16; ORA(d)
    cop c_COp12m16
NEWSYM COp13m8 ; ORA(d,s),y
    cop c_COp13m8
NEWSYM COp13m16; ORA(d,s),y
    cop c_COp13m16
NEWSYM COp15m8 ; ORA d,x
    cop c_COp15m8
NEWSYM COp15m16; ORA d,x
    cop c_COp15m16
NEWSYM COp17m8 ; ORA [d],y
    cop c_COp17m8
NEWSYM COp17m16; ORA [d],y
    cop c_COp17m16
NEWSYM COp19m8 ; ORA a,y
    cop c_COp19m8
NEWSYM COp19m16; ORA a,y
    cop c_COp19m16
NEWSYM COp1Dm8 ; ORA a,x
    cop c_COp1Dm8
NEWSYM COp1Dm16; ORA a,x
    cop c_COp1Dm16
NEWSYM COp1Fm8 ; ORA al,x
    cop c_COp1Fm8
NEWSYM COp1Fm16; ORA al,x
    cop c_COp1Fm16

NEWSYM COpF4   ; PEA s
    cop c_COpF4
NEWSYM COpD4   ; PEI s
    cop c_COpD4
NEWSYM COp62   ; PER s
    cop c_COp62

NEWSYM COp48m8 ; PHA s
    cop c_COp48m8
NEWSYM COp48m16; PHA s
    cop c_COp48m16
NEWSYM COp8B   ; PHB s
    cop c_COp8B
NEWSYM COp0B   ; PHD s
    cop c_COp0B
NEWSYM COp4B   ; PHK s
    cop c_COp4B
NEWSYM COp08   ; PHP s
    cop c_COp08
NEWSYM COpDAx8 ; PHX s
    cop c_COpDAx8
NEWSYM COpDAx16; PHX s
    cop c_COpDAx16
NEWSYM COp5Ax8 ; PHY s
    cop c_COp5Ax8
NEWSYM COp5Ax16; PHY s
    cop c_COp5Ax16

NEWSYM COp68m8 ; PLA s
    cop c_COp68m8
NEWSYM COp68m16; PLA s
    cop c_COp68m16
NEWSYM COpAB   ; PLB s
    cop c_COpAB
NEWSYM COp2B   ; PLD s
    cop c_COp2B
NEWSYM COp28   ; PLP s
    cop c_COp28
NEWSYM COpFAx8 ; PLX s
    cop c_COpFAx8
NEWSYM COpFAx16; PLX s
    cop c_COpFAx16
NEWSYM COp7Ax8 ; PLY s
    cop c_COp7Ax8
NEWSYM COp7Ax16; PLY s
    cop c_COp7Ax16

NEWSYM COpC2   ; REP #
    cop c_COpC2

NEWSYM COp26m8 ; ROL d
    cop c_COp26m8
NEWSYM COp26m16; ROL d
    cop c_COp26m16
NEWSYM COp2Am8 ; ROL A
    cop c_COp2Am8
NEWSYM COp2Am16; ROL A
    cop c_COp2Am16
NEWSYM COp2Em8 ; ROL a
    cop c_COp2Em8
NEWSYM COp2Em16; ROL a
    cop c_COp2Em16
NEWSYM COp36m8 ; ROL d,x
    cop c_COp36m8
NEWSYM COp36m16; ROL d,x
    cop c_COp36m16
NEWSYM COp3Em8 ; ROL a,x
    cop c_COp3Em8
NEWSYM COp3Em16; ROL a,x
    cop c_COp3Em16

NEWSYM COp66m8 ; ROR d
    cop c_COp66m8
NEWSYM COp66m16; ROR d
    cop c_COp66m16
NEWSYM COp6Am8 ; ROR A
    cop c_COp6Am8
NEWSYM COp6Am16; ROR A
    cop c_COp6Am16
NEWSYM COp6Em8 ; ROR a
    cop c_COp6Em8
NEWSYM COp6Em16; ROR a
    cop c_COp6Em16
NEWSYM COp76m8 ; ROR d,x
    cop c_COp76m8
NEWSYM COp76m16; ROR d,x
    cop c_COp76m16
NEWSYM COp7Em8 ; ROR a,x
    cop c_COp7Em8
NEWSYM COp7Em16; ROR a,x
    cop c_COp7Em16

NEWSYM COp40   ; RTI s
    cop c_COp40
NEWSYM COp6B   ; RTL s
    cop c_COp6B
NEWSYM COp60   ; RTS s
    cop c_COp60

NEWSYM COpE1m8nd  ; SBC(d,x)
    cop c_COpE1m8nd
NEWSYM COpE1m16nd ; SBC(d,x)
    cop c_COpE1m16nd
NEWSYM COpE1m8d   ; SBC(d,x)
    cop c_COpE1m8d
NEWSYM COpE1m16d  ; SBC(d,x)
    cop c_COpE1m16d
NEWSYM COpE3m8nd  ; SBC d,s
    cop c_COpE3m8nd
NEWSYM COpE3m16nd ; SBC d,s
    cop c_COpE3m16nd
NEWSYM COpE3m8d   ; SBC d,s
    cop c_COpE3m8d
NEWSYM COpE3m16d  ; SBC d,s
    cop c_COpE3m16d
NEWSYM COpE5m8nd  ; SBC d
    cop c_COpE5m8nd
NEWSYM COpE5m16nd ; SBC d
    cop c_COpE5m16nd
NEWSYM COpE5m8d   ; SBC d
    cop c_COpE5m8d
NEWSYM COpE5m16d  ; SBC d
    cop c_COpE5m16d
NEWSYM COpE7m8nd  ; SBC [d]
    cop c_COpE7m8nd
NEWSYM COpE7m16nd ; SBC [d]
    cop c_COpE7m16nd
NEWSYM COpE7m8d   ; SBC [d]
    cop c_COpE7m8d
NEWSYM COpE7m16d  ; SBC [d]
    cop c_COpE7m16d
NEWSYM COpE9m8nd  ; SBC #
    cop c_COpE9m8nd
NEWSYM COpE9m16nd ; SBC #
    cop c_COpE9m16nd
NEWSYM COpE9m8d   ; SBC #
    cop c_COpE9m8d
NEWSYM COpE9m16d  ; SBC #
    cop c_COpE9m16d
NEWSYM COpEDm8nd  ; SBC a
    cop c_COpEDm8nd
NEWSYM COpEDm16nd ; SBC a
    cop c_COpEDm16nd
NEWSYM COpEDm8d   ; SBC a
    cop c_COpEDm8d
NEWSYM COpEDm16d  ; SBC a
    cop c_COpEDm16d
NEWSYM COpEFm8nd  ; SBC al
    cop c_COpEFm8nd
NEWSYM COpEFm16nd ; SBC al
    cop c_COpEFm16nd
NEWSYM COpEFm8d   ; SBC al
    cop c_COpEFm8d
NEWSYM COpEFm16d  ; SBC al
    cop c_COpEFm16d
NEWSYM COpF1m8nd  ; SBC(d),y
    cop c_COpF1m8nd
NEWSYM COpF1m16nd ; SBC(d),y
    cop c_COpF1m16nd
NEWSYM COpF1m8d   ; SBC(d),y
    cop c_COpF1m8d
NEWSYM COpF1m16d  ; SBC(d),y
    cop c_COpF1m16d
NEWSYM COpF2m8nd  ; SBC (d)
    cop c_COpF2m8nd
NEWSYM COpF2m16nd ; SBC (d)
    cop c_COpF2m16nd
NEWSYM COpF2m8d   ; SBC (d)
    cop c_COpF2m8d
NEWSYM COpF2m16d  ; SBC (d)
    cop c_COpF2m16d
NEWSYM COpF3m8nd  ; SBC(d,s),y
    cop c_COpF3m8nd
NEWSYM COpF3m16nd ; SBC(d,s),y
    cop c_COpF3m16nd
NEWSYM COpF3m8d   ; SBC(d,s),y
    cop c_COpF3m8d
NEWSYM COpF3m16d  ; SBC(d,s),y
    cop c_COpF3m16d
NEWSYM COpF5m8nd  ; SBC d,x
    cop c_COpF5m8nd
NEWSYM COpF5m16nd ; SBC d,x
    cop c_COpF5m16nd
NEWSYM COpF5m8d   ; SBC d,x
    cop c_COpF5m8d
NEWSYM COpF5m16d  ; SBC d,x
    cop c_COpF5m16d
NEWSYM COpF7m8nd  ; SBC [d],y
    cop c_COpF7m8nd
NEWSYM COpF7m16nd ; SBC [d],y
    cop c_COpF7m16nd
NEWSYM COpF7m8d   ; SBC [d],y
    cop c_COpF7m8d
NEWSYM COpF7m16d  ; SBC [d],y
    cop c_COpF7m16d
NEWSYM COpF9m8nd  ; SBC a,y
    cop c_COpF9m8nd
NEWSYM COpF9m16nd ; SBC a,y
    cop c_COpF9m16nd
NEWSYM COpF9m8d   ; SBC a,y
    cop c_COpF9m8d
NEWSYM COpF9m16d  ; SBC a,y
    cop c_COpF9m16d
NEWSYM COpFDm8nd  ; SBC a,x
    cop c_COpFDm8nd
NEWSYM COpFDm16nd ; SBC a,x
    cop c_COpFDm16nd
NEWSYM COpFDm8d   ; SBC a,x
    cop c_COpFDm8d
NEWSYM COpFDm16d  ; SBC a,x
    cop c_COpFDm16d
NEWSYM COpFFm8nd  ; SBC al,x
    cop c_COpFFm8nd
NEWSYM COpFFm16nd ; SBC al,x
    cop c_COpFFm16nd
NEWSYM COpFFm8d   ; SBC al,x
    cop c_COpFFm8d
NEWSYM COpFFm16d  ; SBC al,x
    cop c_COpFFm16d

NEWSYM COp38   ; SEC i
    cop c_COp38
NEWSYM COpF8   ; SED i
    cop c_COpF8
NEWSYM COp78   ; SEI i
    cop c_COp78

NEWSYM COpE2   ; SEP #
    cop c_COpE2

NEWSYM COp81m8 ; STA(d,x)
    cop c_COp81m8
NEWSYM COp81m16; STA(d,x)
    cop c_COp81m16
NEWSYM COp83m8 ; STA d,s
    cop c_COp83m8
NEWSYM COp83m16; STA d,s
    cop c_COp83m16
NEWSYM COp85m8 ; STA d
    cop c_COp85m8
NEWSYM COp85m16; STA d
    cop c_COp85m16
NEWSYM COp87m8 ; STA [d]
    cop c_COp87m8
NEWSYM COp87m16; STA [d]
    cop c_COp87m16
NEWSYM COp8Dm8 ; STA a
    cop c_COp8Dm8
NEWSYM COp8Dm16; STA a
    cop c_COp8Dm16
NEWSYM COp8Fm8 ; STA al
    cop c_COp8Fm8
NEWSYM COp8Fm16; STA al
    cop c_COp8Fm16
NEWSYM COp91m8 ; STA(d),y
    cop c_COp91m8
NEWSYM COp91m16; STA(d),y
    cop c_COp91m16
NEWSYM COp92m8 ; STA (d)
    cop c_COp92m8
NEWSYM COp92m16; STA (d)
    cop c_COp92m16
NEWSYM COp93m8 ; STA(d,s),y
    cop c_COp93m8
NEWSYM COp93m16; STA(d,s),y
    cop c_COp93m16
NEWSYM COp95m8 ; STA d,x
    cop c_COp95m8
NEWSYM COp95m16; STA d,x
    cop c_COp95m16
NEWSYM COp97m8 ; STA [d],y
    cop c_COp97m8
NEWSYM COp97m16; STA [d],y
    cop c_COp97m16
NEWSYM COp99m8 ; STA a,y
    cop c_COp99m8
NEWSYM COp99m16; STA a,y
    cop c_COp99m16
NEWSYM COp9Dm8 ; STA a,x
    cop c_COp9Dm8
NEWSYM COp9Dm16; STA a,x
    cop c_COp9Dm16
NEWSYM COp9Fm8 ; STA al,x
    cop c_COp9Fm8
NEWSYM COp9Fm16; STA al,x
    cop c_COp9Fm16

NEWSYM COp86x8 ; STX d
    cop c_COp86x8
NEWSYM COp86x16; STX d
    cop c_COp86x16
NEWSYM COp8Ex8 ; STX a
    cop c_COp8Ex8
NEWSYM COp8Ex16; STX a
    cop c_COp8Ex16
NEWSYM COp96x8 ; STX d,y
    cop c_COp96x8
NEWSYM COp96x16; STX d,y
    cop c_COp96x16

NEWSYM COp84x8 ; STY d
    cop c_COp84x8
NEWSYM COp84x16; STY d
    cop c_COp84x16
NEWSYM COp8Cx8 ; STY a
    cop c_COp8Cx8
NEWSYM COp8Cx16; STY a
    cop c_COp8Cx16
NEWSYM COp94x8 ; STYd,x
    cop c_COp94x8
NEWSYM COp94x16; STYd,x
    cop c_COp94x16

NEWSYM COpDB   ; STP i
    cop c_COpDB

NEWSYM COp64m8 ; STZ d
    cop c_COp64m8
NEWSYM COp64m16; STZ d
    cop c_COp64m16
NEWSYM COp74m8 ; STZ d,x
    cop c_COp74m8
NEWSYM COp74m16; STZ d,x
    cop c_COp74m16
NEWSYM COp9Cm8 ; STZ a
    cop c_COp9Cm8
NEWSYM COp9Cm16; STZ a
    cop c_COp9Cm16
NEWSYM COp9Em8 ; STZ a,x
    cop c_COp9Em8
NEWSYM COp9Em16; STZ a,x
    cop c_COp9Em16

NEWSYM COp14m8 ; TRB d
    cop c_COp14m8
NEWSYM COp14m16; TRB d
    cop c_COp14m16
NEWSYM COp1Cm8 ; TRB a
    cop c_COp1Cm8
NEWSYM COp1Cm16; TRB a
    cop c_COp1Cm16

NEWSYM COp04m8 ; TSB d
    cop c_COp04m8
NEWSYM COp04m16; TSB d
    cop c_COp04m16
NEWSYM COp0Cm8 ; TSB a
    cop c_COp0Cm8
NEWSYM COp0Cm16; TSB a
    cop c_COp0Cm16

NEWSYM COpAAx8 ; TAX i
    cop c_COpAAx8
NEWSYM COpAAx16; TAX i
    cop c_COpAAx16
NEWSYM COpA8x8 ; TAY i
    cop c_COpA8x8
NEWSYM COpA8x16; TAY i
    cop c_COpA8x16
NEWSYM COp5B   ; TCD i
    cop c_COp5B
NEWSYM COp1B   ; TCS i
    cop c_COp1B
NEWSYM COp7B   ; TDC i
    cop c_COp7B
NEWSYM COp3B   ; TSC i
    cop c_COp3B
NEWSYM COpBAx8 ; TSX i
    cop c_COpBAx8
NEWSYM COpBAx16; TSX i
    cop c_COpBAx16
NEWSYM COp8Am8 ; TXA i
    cop c_COp8Am8
NEWSYM COp8Am16; TXA i
    cop c_COp8Am16
NEWSYM COp9A   ; TXS i
    cop c_COp9A
NEWSYM COp9Bx8 ; TXY i
    cop c_COp9Bx8
NEWSYM COp9Bx16; TXY i
    cop c_COp9Bx16
NEWSYM COp98m8 ; TYA i
    cop c_COp98m8
NEWSYM COp98m16; TYA i
    cop c_COp98m16
NEWSYM COpBBx8 ; TYX i
    cop c_COpBBx8
NEWSYM COpBBx16; TYX i
    cop c_COpBBx16

NEWSYM COpCB   ; WAI i
    cop c_COpCB

NEWSYM COpEB   ; XBA i
    cop c_COpEB

NEWSYM COpFB   ; XCE i
    cop c_COpFB

NEWSYM COp42   ; WDM
    cop c_COp42


