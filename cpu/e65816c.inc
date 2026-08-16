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



EXTSYM brkv,brkv8,copv,copv8,curexecstate,curnmi,initaddrl,intrset
EXTSYM membank0r16,membank0r8,membank0w16,membank0w8
EXTSYM memtabler16,memtabler8,memtablew16,memtablew8
EXTSYM nmistatus,regaccessbankr8,snesmap2,snesmmap,stackand,stackor
EXTSYM wramdata,xa,xd,xdb,xe,xirqb,xpb,xpc,xs,xx,xy
EXTSYM UpdateDPage,DPageR8,DPageW8,DPageR16,DPageW16

; B = ( or )
; C = ,
; L = [ or ]
; I = #

SECTION .text

NEWSYM cCOp61m8nd  ; ADC(d,x)
    cop c_dbgCOp61m8nd
NEWSYM cCOp61m16nd ; ADC(d,x)
    cop c_dbgCOp61m16nd
NEWSYM cCOp61m8d   ; ADC(d,x)
    cop c_dbgCOp61m8d
NEWSYM cCOp61m16d  ; ADC(d,x)
    cop c_dbgCOp61m16d
NEWSYM cCOp63m8nd  ; ADC d,s
    cop c_dbgCOp63m8nd
NEWSYM cCOp63m16nd ; ADC d,s
    cop c_dbgCOp63m16nd
NEWSYM cCOp63m8d   ; ADC d,s
    cop c_dbgCOp63m8d
NEWSYM cCOp63m16d  ; ADC d,s
    cop c_dbgCOp63m16d
NEWSYM cCOp65m8nd  ; ADC d
    cop c_dbgCOp65m8nd
NEWSYM cCOp65m16nd ; ADC d
    cop c_dbgCOp65m16nd
NEWSYM cCOp65m8d   ; ADC d
    cop c_dbgCOp65m8d
NEWSYM cCOp65m16d  ; ADC d
    cop c_dbgCOp65m16d
NEWSYM cCOp67m8nd  ; ADC [d]
    cop c_dbgCOp67m8nd
NEWSYM cCOp67m16nd ; ADC [d]
    cop c_dbgCOp67m16nd
NEWSYM cCOp67m8d   ; ADC [d]
    cop c_dbgCOp67m8d
NEWSYM cCOp67m16d  ; ADC [d]
    cop c_dbgCOp67m16d
NEWSYM cCOp69m8nd  ; ADC #
    cop c_dbgCOp69m8nd
NEWSYM cCOp69m16nd ; ADC #
    cop c_dbgCOp69m16nd
NEWSYM cCOp69m8d   ; ADC #
    cop c_dbgCOp69m8d
NEWSYM cCOp69m16d  ; ADC #
    cop c_dbgCOp69m16d
NEWSYM cCOp6Dm8nd  ; ADC a
    cop c_dbgCOp6Dm8nd
NEWSYM cCOp6Dm16nd ; ADC a
    cop c_dbgCOp6Dm16nd
NEWSYM cCOp6Dm8d   ; ADC a
    cop c_dbgCOp6Dm8d
NEWSYM cCOp6Dm16d  ; ADC a
    cop c_dbgCOp6Dm16d
NEWSYM cCOp6Fm8nd  ; ADC al
    cop c_dbgCOp6Fm8nd
NEWSYM cCOp6Fm16nd ; ADC al
    cop c_dbgCOp6Fm16nd
NEWSYM cCOp6Fm8d   ; ADC al
    cop c_dbgCOp6Fm8d
NEWSYM cCOp6Fm16d  ; ADC al
    cop c_dbgCOp6Fm16d
NEWSYM cCOp71m8nd  ; ADC(d),y
    cop c_dbgCOp71m8nd
NEWSYM cCOp71m16nd ; ADC(d),y
    cop c_dbgCOp71m16nd
NEWSYM cCOp71m8d   ; ADC(d),y
    cop c_dbgCOp71m8d
NEWSYM cCOp71m16d  ; ADC(d),y
    cop c_dbgCOp71m16d
NEWSYM cCOp72m8nd  ; ADC (d)
    cop c_dbgCOp72m8nd
NEWSYM cCOp72m16nd ; ADC (d)
    cop c_dbgCOp72m16nd
NEWSYM cCOp72m8d   ; ADC (d)
    cop c_dbgCOp72m8d
NEWSYM cCOp72m16d  ; ADC (d)
    cop c_dbgCOp72m16d
NEWSYM cCOp73m8nd  ; ADC(d,s),y
    cop c_dbgCOp73m8nd
NEWSYM cCOp73m16nd ; ADC(d,s),y
    cop c_dbgCOp73m16nd
NEWSYM cCOp73m8d   ; ADC(d,s),y
    cop c_dbgCOp73m8d
NEWSYM cCOp73m16d  ; ADC(d,s),y
    cop c_dbgCOp73m16d
NEWSYM cCOp75m8nd  ; ADC d,x
    cop c_dbgCOp75m8nd
NEWSYM cCOp75m16nd ; ADC d,x
    cop c_dbgCOp75m16nd
NEWSYM cCOp75m8d   ; ADC d,x
    cop c_dbgCOp75m8d
NEWSYM cCOp75m16d  ; ADC d,x
    cop c_dbgCOp75m16d
NEWSYM cCOp77m8nd  ; ADC [d],y
    cop c_dbgCOp77m8nd
NEWSYM cCOp77m16nd ; ADC [d],y
    cop c_dbgCOp77m16nd
NEWSYM cCOp77m8d   ; ADC [d],y
    cop c_dbgCOp77m8d
NEWSYM cCOp77m16d  ; ADC [d],y
    cop c_dbgCOp77m16d
NEWSYM cCOp79m8nd  ; ADC a,y
    cop c_dbgCOp79m8nd
NEWSYM cCOp79m16nd ; ADC a,y
    cop c_dbgCOp79m16nd
NEWSYM cCOp79m8d   ; ADC a,y
    cop c_dbgCOp79m8d
NEWSYM cCOp79m16d  ; ADC a,y
    cop c_dbgCOp79m16d
NEWSYM cCOp7Dm8nd  ; ADC a,x
    cop c_dbgCOp7Dm8nd
NEWSYM cCOp7Dm16nd ; ADC a,x
    cop c_dbgCOp7Dm16nd
NEWSYM cCOp7Dm8d   ; ADC a,x
    cop c_dbgCOp7Dm8d
NEWSYM cCOp7Dm16d  ; ADC a,x
    cop c_dbgCOp7Dm16d
NEWSYM cCOp7Fm8nd  ; ADC al,x
    cop c_dbgCOp7Fm8nd
NEWSYM cCOp7Fm16nd ; ADC al,x
    cop c_dbgCOp7Fm16nd
NEWSYM cCOp7Fm8d   ; ADC al,x
    cop c_dbgCOp7Fm8d
NEWSYM cCOp7Fm16d  ; ADC al,x
    cop c_dbgCOp7Fm16d

NEWSYM cCOp21m8 ; AND(d,x)
    cop c_dbgCOp21m8
NEWSYM cCOp21m16; AND(d,x)
    cop c_dbgCOp21m16
NEWSYM cCOp23m8 ; AND d,s
    cop c_dbgCOp23m8
NEWSYM cCOp23m16; AND d,s
    cop c_dbgCOp23m16
NEWSYM cCOp25m8 ; AND d
    cop c_dbgCOp25m8
NEWSYM cCOp25m16; AND d
    cop c_dbgCOp25m16
NEWSYM cCOp27m8 ; AND [d]
    cop c_dbgCOp27m8
NEWSYM cCOp27m16; AND [d]
    cop c_dbgCOp27m16
NEWSYM cCOp29m8 ; AND #
    cop c_dbgCOp29m8
NEWSYM cCOp29m16; AND #
    cop c_dbgCOp29m16
NEWSYM cCOp2Dm8 ; AND a
    cop c_dbgCOp2Dm8
NEWSYM cCOp2Dm16; AND a
    cop c_dbgCOp2Dm16
NEWSYM cCOp2Fm8 ; AND al
    cop c_dbgCOp2Fm8
NEWSYM cCOp2Fm16; AND al
    cop c_dbgCOp2Fm16
NEWSYM cCOp31m8 ; AND(d),y
    cop c_dbgCOp31m8
NEWSYM cCOp31m16; AND(d),y
    cop c_dbgCOp31m16
NEWSYM cCOp32m8 ; AND (d)
    cop c_dbgCOp32m8
NEWSYM cCOp32m16; AND (d)
    cop c_dbgCOp32m16
NEWSYM cCOp33m8 ; AND(d,s),y
    cop c_dbgCOp33m8
NEWSYM cCOp33m16; AND(d,s),y
    cop c_dbgCOp33m16
NEWSYM cCOp35m8 ; AND d,x
    cop c_dbgCOp35m8
NEWSYM cCOp35m16; AND d,x
    cop c_dbgCOp35m16
NEWSYM cCOp37m8 ; AND [d],y
    cop c_dbgCOp37m8
NEWSYM cCOp37m16; AND [d],y
    cop c_dbgCOp37m16
NEWSYM cCOp39m8 ; AND a,y
    cop c_dbgCOp39m8
NEWSYM cCOp39m16; AND a,y
    cop c_dbgCOp39m16
NEWSYM cCOp3Dm8 ; AND a,x
    cop c_dbgCOp3Dm8
NEWSYM cCOp3Dm16; AND a,x
    cop c_dbgCOp3Dm16
NEWSYM cCOp3Fm8 ; AND al,x
    cop c_dbgCOp3Fm8
NEWSYM cCOp3Fm16; AND al,x
    cop c_dbgCOp3Fm16

NEWSYM cCOp06m8 ; ASL d
    cop c_dbgCOp06m8
NEWSYM cCOp06m16; ASL d
    cop c_dbgCOp06m16
NEWSYM cCOp0Am8 ; ASL A
    cop c_dbgCOp0Am8
NEWSYM cCOp0Am16; ASL A
    cop c_dbgCOp0Am16
NEWSYM cCOp0Em8 ; ASL a
    cop c_dbgCOp0Em8
NEWSYM cCOp0Em16; ASL a
    cop c_dbgCOp0Em16
NEWSYM cCOp16m8 ; ASL d,x
    cop c_dbgCOp16m8
NEWSYM cCOp16m16; ASL d,x
    cop c_dbgCOp16m16
NEWSYM cCOp1Em8 ; ASL a,x
    cop c_dbgCOp1Em8
NEWSYM cCOp1Em16; ASL a,x
    cop c_dbgCOp1Em16

NEWSYM cCOp90   ; BCC r
    cop c_dbgCOp90
NEWSYM cCOpB0   ; BCS r
    cop c_dbgCOpB0
NEWSYM cCOpF0   ; BEQ r
    cop c_dbgCOpF0
NEWSYM cCOp30   ; BMI r
    cop c_dbgCOp30
NEWSYM cCOpD0   ; BNE r
    cop c_dbgCOpD0
NEWSYM cCOp10   ; BPL r
    cop c_dbgCOp10
NEWSYM cCOp80   ; BRA r
    cop c_dbgCOp80
NEWSYM cCOp50   ; BVC r
    cop c_dbgCOp50
NEWSYM cCOp70   ; BVS r
    cop c_dbgCOp70

NEWSYM cCOp89m8 ; BIT #
    cop c_dbgCOp89m8
NEWSYM cCOp89m16; BIT #
    cop c_dbgCOp89m16

NEWSYM cCOp24m8 ; BIT d
    cop c_dbgCOp24m8
NEWSYM cCOp24m16; BIT d
    cop c_dbgCOp24m16
NEWSYM cCOp2Cm8 ; BIT a
    cop c_dbgCOp2Cm8
NEWSYM cCOp2Cm16; BIT a
    cop c_dbgCOp2Cm16
NEWSYM cCOp34m8 ; BIT d,x
    cop c_dbgCOp34m8
NEWSYM cCOp34m16; BIT d,x
    cop c_dbgCOp34m16
NEWSYM cCOp3Cm8 ; BIT a,x
    cop c_dbgCOp3Cm8
NEWSYM cCOp3Cm16; BIT a,x
    cop c_dbgCOp3Cm16

NEWSYM cCOp00   ; BRK s
    cop c_dbgCOp00

NEWSYM cCOp82   ; BRL rl
    cop c_dbgCOp82

NEWSYM cCOp18   ; CLC i
    cop c_dbgCOp18
NEWSYM cCOpD8   ; CLD i
    cop c_dbgCOpD8
NEWSYM cCOp58   ; CLI i
    cop c_dbgCOp58
NEWSYM cCOpB8   ; CLV i
    cop c_dbgCOpB8

NEWSYM cCOpC1m8 ; CMP(d,x)
    cop c_dbgCOpC1m8
NEWSYM cCOpC1m16; CMP(d,x)
    cop c_dbgCOpC1m16
NEWSYM cCOpC3m8 ; CMP d,s
    cop c_dbgCOpC3m8
NEWSYM cCOpC3m16; CMP d,s
    cop c_dbgCOpC3m16
NEWSYM cCOpC5m8 ; CMP d
    cop c_dbgCOpC5m8
NEWSYM cCOpC5m16; CMP d
    cop c_dbgCOpC5m16
NEWSYM cCOpC7m8 ; CMP [d]
    cop c_dbgCOpC7m8
NEWSYM cCOpC7m16; CMP [d]
    cop c_dbgCOpC7m16
NEWSYM cCOpC9m8 ; CMP #
    cop c_dbgCOpC9m8
NEWSYM cCOpC9m16; CMP #
    cop c_dbgCOpC9m16
NEWSYM cCOpCDm8 ; CMP a
    cop c_dbgCOpCDm8
NEWSYM cCOpCDm16; CMP a
    cop c_dbgCOpCDm16
NEWSYM cCOpCFm8 ; CMP al
    cop c_dbgCOpCFm8
NEWSYM cCOpCFm16; CMP al
    cop c_dbgCOpCFm16
NEWSYM cCOpD1m8 ; CMP(d),y
    cop c_dbgCOpD1m8
NEWSYM cCOpD1m16; CMP(d),y
    cop c_dbgCOpD1m16
NEWSYM cCOpD2m8 ; CMP (d)
    cop c_dbgCOpD2m8
NEWSYM cCOpD2m16; CMP (d)
    cop c_dbgCOpD2m16
NEWSYM cCOpD3m8 ; CMP(d,s),y
    cop c_dbgCOpD3m8
NEWSYM cCOpD3m16; CMP(d,s),y
    cop c_dbgCOpD3m16
NEWSYM cCOpD5m8 ; CMP d,x
    cop c_dbgCOpD5m8
NEWSYM cCOpD5m16; CMP d,x
    cop c_dbgCOpD5m16
NEWSYM cCOpD7m8 ; CMP [d],y
    cop c_dbgCOpD7m8
NEWSYM cCOpD7m16; CMP [d],y
    cop c_dbgCOpD7m16
NEWSYM cCOpD9m8 ; CMP a,y
    cop c_dbgCOpD9m8
NEWSYM cCOpD9m16; CMP a,y
    cop c_dbgCOpD9m16
NEWSYM cCOpDDm8 ; CMP a,x
    cop c_dbgCOpDDm8
NEWSYM cCOpDDm16; CMP a,x
    cop c_dbgCOpDDm16
NEWSYM cCOpDFm8 ; CMP al,x
    cop c_dbgCOpDFm8
NEWSYM cCOpDFm16; CMP al,x
    cop c_dbgCOpDFm16

NEWSYM cCOp02   ; COP s
    cop c_dbgCOp02

NEWSYM cCOpE0x8 ; CPX #
    cop c_dbgCOpE0x8
NEWSYM cCOpE0x16; CPX #
    cop c_dbgCOpE0x16
NEWSYM cCOpE4x8 ; CPX d
    cop c_dbgCOpE4x8
NEWSYM cCOpE4x16; CPX d
    cop c_dbgCOpE4x16
NEWSYM cCOpECx8 ; CPX a
    cop c_dbgCOpECx8
NEWSYM cCOpECx16; CPX a
    cop c_dbgCOpECx16

NEWSYM cCOpC0x8 ; CPY #
    cop c_dbgCOpC0x8
NEWSYM cCOpC0x16; CPY #
    cop c_dbgCOpC0x16
NEWSYM cCOpC4x8 ; CPY d
    cop c_dbgCOpC4x8
NEWSYM cCOpC4x16; CPY d
    cop c_dbgCOpC4x16
NEWSYM cCOpCCx8 ; CPY a
    cop c_dbgCOpCCx8
NEWSYM cCOpCCx16; CPY a
    cop c_dbgCOpCCx16

NEWSYM cCOp3Am8 ; DEC A
    cop c_dbgCOp3Am8
NEWSYM cCOp3Am16; DEC A
    cop c_dbgCOp3Am16
NEWSYM cCOpCEm8 ; DEC a
    cop c_dbgCOpCEm8
NEWSYM cCOpCEm16; DEC a
    cop c_dbgCOpCEm16
NEWSYM cCOpC6m8 ; DEC d
    cop c_dbgCOpC6m8
NEWSYM cCOpC6m16; DEC d
    cop c_dbgCOpC6m16
NEWSYM cCOpD6m8 ; DEC d,x
    cop c_dbgCOpD6m8
NEWSYM cCOpD6m16; DEC d,x
    cop c_dbgCOpD6m16
NEWSYM cCOpDEm8 ; DEC a,x
    cop c_dbgCOpDEm8
NEWSYM cCOpDEm16; DEC a,x
    cop c_dbgCOpDEm16

NEWSYM cCOpCAx8 ; DEX i
    cop c_dbgCOpCAx8
NEWSYM cCOpCAx16; DEX i
    cop c_dbgCOpCAx16
NEWSYM cCOp88x8 ; DEY i
    cop c_dbgCOp88x8
NEWSYM cCOp88x16; DEY i
    cop c_dbgCOp88x16

NEWSYM cCOp41m8 ; EOR(d,x)
    cop c_dbgCOp41m8
NEWSYM cCOp41m16; EOR(d,x)
    cop c_dbgCOp41m16
NEWSYM cCOp43m8 ; EOR d,s
    cop c_dbgCOp43m8
NEWSYM cCOp43m16; EOR d,s
    cop c_dbgCOp43m16
NEWSYM cCOp45m8 ; EOR d
    cop c_dbgCOp45m8
NEWSYM cCOp45m16; EOR d
    cop c_dbgCOp45m16
NEWSYM cCOp47m8 ; EOR [d]
    cop c_dbgCOp47m8
NEWSYM cCOp47m16; EOR [d]
    cop c_dbgCOp47m16
NEWSYM cCOp49m8 ; EOR #
    cop c_dbgCOp49m8
NEWSYM cCOp49m16; EOR #
    cop c_dbgCOp49m16
NEWSYM cCOp4Dm8 ; EOR a
    cop c_dbgCOp4Dm8
NEWSYM cCOp4Dm16; EOR a
    cop c_dbgCOp4Dm16
NEWSYM cCOp4Fm8 ; EOR al
    cop c_dbgCOp4Fm8
NEWSYM cCOp4Fm16; EOR al
    cop c_dbgCOp4Fm16
NEWSYM cCOp51m8 ; EOR(d),y
    cop c_dbgCOp51m8
NEWSYM cCOp51m16; EOR(d),y
    cop c_dbgCOp51m16
NEWSYM cCOp52m8 ; EOR (d)
    cop c_dbgCOp52m8
NEWSYM cCOp52m16; EOR (d)
    cop c_dbgCOp52m16
NEWSYM cCOp53m8 ; EOR(d,s),y
    cop c_dbgCOp53m8
NEWSYM cCOp53m16; EOR(d,s),y
    cop c_dbgCOp53m16
NEWSYM cCOp55m8 ; EOR d,x
    cop c_dbgCOp55m8
NEWSYM cCOp55m16; EOR d,x
    cop c_dbgCOp55m16
NEWSYM cCOp57m8 ; EOR [d],y
    cop c_dbgCOp57m8
NEWSYM cCOp57m16; EOR [d],y
    cop c_dbgCOp57m16
NEWSYM cCOp59m8 ; EOR a,y
    cop c_dbgCOp59m8
NEWSYM cCOp59m16; EOR a,y
    cop c_dbgCOp59m16
NEWSYM cCOp5Dm8 ; EORa,x
    cop c_dbgCOp5Dm8
NEWSYM cCOp5Dm16; EORa,x
    cop c_dbgCOp5Dm16
NEWSYM cCOp5Fm8 ; EOR al,x
    cop c_dbgCOp5Fm8
NEWSYM cCOp5Fm16; EOR al,x
    cop c_dbgCOp5Fm16


NEWSYM cCOp1Am8 ; INC A
    cop c_dbgCOp1Am8
NEWSYM cCOp1Am16; INC A
    cop c_dbgCOp1Am16
NEWSYM cCOpEEm8 ; INC a
    cop c_dbgCOpEEm8
NEWSYM cCOpEEm16; INC a
    cop c_dbgCOpEEm16
NEWSYM cCOpE6m8 ; INC d
    cop c_dbgCOpE6m8
NEWSYM cCOpE6m16; INC d
    cop c_dbgCOpE6m16
NEWSYM cCOpF6m8 ; INC d,x
    cop c_dbgCOpF6m8
NEWSYM cCOpF6m16; INC d,x
    cop c_dbgCOpF6m16
NEWSYM cCOpFEm8 ; INC a,x
    cop c_dbgCOpFEm8
NEWSYM cCOpFEm16; INC a,x
    cop c_dbgCOpFEm16


NEWSYM cCOpE8x8 ; INX i
    cop c_dbgCOpE8x8
NEWSYM cCOpE8x16; INX i
    cop c_dbgCOpE8x16
NEWSYM cCOpC8x8 ; INY i
    cop c_dbgCOpC8x8
NEWSYM cCOpC8x16; INY i
    cop c_dbgCOpC8x16

NEWSYM cCOpDC   ; JML (a)
    cop c_dbgCOpDC
NEWSYM cCOp4C   ; JMP a
    cop c_dbgCOp4C
NEWSYM cCOp6C   ; JMP (a)
    cop c_dbgCOp6C
NEWSYM cCOp7C   ; JMP(a,x)
    cop c_dbgCOp7C
NEWSYM cCOp5C   ; JMP al
    cop c_dbgCOp5C

NEWSYM cCOp22   ; JSL al
    cop c_dbgCOp22
NEWSYM cCOp20   ; JSR a
    cop c_dbgCOp20
NEWSYM cCOpFC   ; JSR(a,x)
    cop c_dbgCOpFC

NEWSYM cCOpA1m8 ; LDA(d,x)
    cop c_dbgCOpA1m8
NEWSYM cCOpA1m16; LDA(d,x)
    cop c_dbgCOpA1m16
NEWSYM cCOpA3m8 ; LDA d,s
    cop c_dbgCOpA3m8
NEWSYM cCOpA3m16; LDA d,s
    cop c_dbgCOpA3m16
NEWSYM cCOpA5m8 ; LDA d
    cop c_dbgCOpA5m8
NEWSYM cCOpA5m16; LDA d
    cop c_dbgCOpA5m16
NEWSYM cCOpA7m8 ; LDA [d]
    cop c_dbgCOpA7m8
NEWSYM cCOpA7m16; LDA [d]
    cop c_dbgCOpA7m16
NEWSYM cCOpA9m8 ; LDA #
    cop c_dbgCOpA9m8
NEWSYM cCOpA9m16; LDA #
    cop c_dbgCOpA9m16
NEWSYM cCOpADm8 ; LDA a
    cop c_dbgCOpADm8
NEWSYM cCOpADm16; LDA a
    cop c_dbgCOpADm16
NEWSYM cCOpAFm8 ; LDA al
    cop c_dbgCOpAFm8
NEWSYM cCOpAFm16; LDA al
    cop c_dbgCOpAFm16
NEWSYM cCOpB1m8 ; LDA(d),y
    cop c_dbgCOpB1m8
NEWSYM cCOpB1m16; LDA(d),y
    cop c_dbgCOpB1m16
NEWSYM cCOpB2m8 ; LDA (d)
    cop c_dbgCOpB2m8
NEWSYM cCOpB2m16; LDA (d)
    cop c_dbgCOpB2m16
NEWSYM cCOpB3m8 ; LDA(d,s),y
    cop c_dbgCOpB3m8
NEWSYM cCOpB3m16; LDA(d,s),y
    cop c_dbgCOpB3m16
NEWSYM cCOpB5m8 ; LDA d,x
    cop c_dbgCOpB5m8
NEWSYM cCOpB5m16; LDA d,x
    cop c_dbgCOpB5m16
NEWSYM cCOpB7m8 ; LDA [d],y
    cop c_dbgCOpB7m8
NEWSYM cCOpB7m16; LDA [d],y
    cop c_dbgCOpB7m16
NEWSYM cCOpB9m8 ; LDA a,y
    cop c_dbgCOpB9m8
NEWSYM cCOpB9m16; LDA a,y
    cop c_dbgCOpB9m16
NEWSYM cCOpBDm8 ; LDA a,x
    cop c_dbgCOpBDm8
NEWSYM cCOpBDm16; LDA a,x
    cop c_dbgCOpBDm16
NEWSYM cCOpBFm8 ; LDA al,x
    cop c_dbgCOpBFm8
NEWSYM cCOpBFm16; LDA al,x
    cop c_dbgCOpBFm16

NEWSYM cCOpA2x8 ; LDX #
    cop c_dbgCOpA2x8
NEWSYM cCOpA2x16; LDX #
    cop c_dbgCOpA2x16
NEWSYM cCOpA6x8 ; LDX d
    cop c_dbgCOpA6x8
NEWSYM cCOpA6x16; LDX d
    cop c_dbgCOpA6x16
NEWSYM cCOpAEx8 ; LDX a
    cop c_dbgCOpAEx8
NEWSYM cCOpAEx16; LDX a
    cop c_dbgCOpAEx16
NEWSYM cCOpB6x8 ; LDX d,y
    cop c_dbgCOpB6x8
NEWSYM cCOpB6x16; LDX d,y
    cop c_dbgCOpB6x16
NEWSYM cCOpBEx8 ; LDX a,y
    cop c_dbgCOpBEx8
NEWSYM cCOpBEx16; LDX a,y
    cop c_dbgCOpBEx16

NEWSYM cCOpA0x8 ; LDY #
    cop c_dbgCOpA0x8
NEWSYM cCOpA0x16; LDY #
    cop c_dbgCOpA0x16
NEWSYM cCOpA4x8 ; LDY d
    cop c_dbgCOpA4x8
NEWSYM cCOpA4x16; LDY d
    cop c_dbgCOpA4x16
NEWSYM cCOpACx8 ; LDY a
    cop c_dbgCOpACx8
NEWSYM cCOpACx16; LDY a
    cop c_dbgCOpACx16
NEWSYM cCOpB4x8 ; LDY d,x
    cop c_dbgCOpB4x8
NEWSYM cCOpB4x16; LDY d,x
    cop c_dbgCOpB4x16
NEWSYM cCOpBCx8 ; LDY a,x
    cop c_dbgCOpBCx8
NEWSYM cCOpBCx16; LDY a,x
    cop c_dbgCOpBCx16

NEWSYM cCOp46m8 ; LSR d
    cop c_dbgCOp46m8
NEWSYM cCOp46m16; LSR d
    cop c_dbgCOp46m16
NEWSYM cCOp4Am8 ; LSR A
    cop c_dbgCOp4Am8
NEWSYM cCOp4Am16; LSR A
    cop c_dbgCOp4Am16
NEWSYM cCOp4Em8 ; LSR a
    cop c_dbgCOp4Em8
NEWSYM cCOp4Em16; LSR a
    cop c_dbgCOp4Em16
NEWSYM cCOp56m8 ; LSR d,x
    cop c_dbgCOp56m8
NEWSYM cCOp56m16; LSR d,x
    cop c_dbgCOp56m16
NEWSYM cCOp5Em8 ; LSRa,x
    cop c_dbgCOp5Em8
NEWSYM cCOp5Em16; LSRa,x
    cop c_dbgCOp5Em16

NEWSYM cCOp54   ; MVN xya
    cop c_dbgCOp54
NEWSYM cCOp44   ; MVP xya
    cop c_dbgCOp44

NEWSYM cCOpEA   ; NOP i
    cop c_dbgCOpEA

NEWSYM cCOp01m8 ; ORA(d,x)
    cop c_dbgCOp01m8
NEWSYM cCOp01m16; ORA(d,x)
    cop c_dbgCOp01m16
NEWSYM cCOp03m8 ; ORA d,s
    cop c_dbgCOp03m8
NEWSYM cCOp03m16; ORA d,s
    cop c_dbgCOp03m16
NEWSYM cCOp05m8 ; ORA d
    cop c_dbgCOp05m8
NEWSYM cCOp05m16; ORA d
    cop c_dbgCOp05m16
NEWSYM cCOp07m8 ; ORA [d]
    cop c_dbgCOp07m8
NEWSYM cCOp07m16; ORA [d]
    cop c_dbgCOp07m16
NEWSYM cCOp09m8 ; ORA #
    cop c_dbgCOp09m8
NEWSYM cCOp09m16; ORA #
    cop c_dbgCOp09m16
NEWSYM cCOp0Dm8 ; ORA a
    cop c_dbgCOp0Dm8
NEWSYM cCOp0Dm16; ORA a
    cop c_dbgCOp0Dm16
NEWSYM cCOp0Fm8 ; ORA al
    cop c_dbgCOp0Fm8
NEWSYM cCOp0Fm16; ORA al
    cop c_dbgCOp0Fm16
NEWSYM cCOp11m8 ; ORA(d),y
    cop c_dbgCOp11m8
NEWSYM cCOp11m16; ORA(d),y
    cop c_dbgCOp11m16
NEWSYM cCOp12m8 ; ORA(d)
    cop c_dbgCOp12m8
NEWSYM cCOp12m16; ORA(d)
    cop c_dbgCOp12m16
NEWSYM cCOp13m8 ; ORA(d,s),y
    cop c_dbgCOp13m8
NEWSYM cCOp13m16; ORA(d,s),y
    cop c_dbgCOp13m16
NEWSYM cCOp15m8 ; ORA d,x
    cop c_dbgCOp15m8
NEWSYM cCOp15m16; ORA d,x
    cop c_dbgCOp15m16
NEWSYM cCOp17m8 ; ORA [d],y
    cop c_dbgCOp17m8
NEWSYM cCOp17m16; ORA [d],y
    cop c_dbgCOp17m16
NEWSYM cCOp19m8 ; ORA a,y
    cop c_dbgCOp19m8
NEWSYM cCOp19m16; ORA a,y
    cop c_dbgCOp19m16
NEWSYM cCOp1Dm8 ; ORA a,x
    cop c_dbgCOp1Dm8
NEWSYM cCOp1Dm16; ORA a,x
    cop c_dbgCOp1Dm16
NEWSYM cCOp1Fm8 ; ORA al,x
    cop c_dbgCOp1Fm8
NEWSYM cCOp1Fm16; ORA al,x
    cop c_dbgCOp1Fm16

NEWSYM cCOpF4   ; PEA s
    cop c_dbgCOpF4
NEWSYM cCOpD4   ; PEI s
    cop c_dbgCOpD4
NEWSYM cCOp62   ; PER s
    cop c_dbgCOp62

NEWSYM cCOp48m8 ; PHA s
    cop c_dbgCOp48m8
NEWSYM cCOp48m16; PHA s
    cop c_dbgCOp48m16
NEWSYM cCOp8B   ; PHB s
    cop c_dbgCOp8B
NEWSYM cCOp0B   ; PHD s
    cop c_dbgCOp0B
NEWSYM cCOp4B   ; PHK s
    cop c_dbgCOp4B
NEWSYM cCOp08   ; PHP s
    cop c_dbgCOp08
NEWSYM cCOpDAx8 ; PHX s
    cop c_dbgCOpDAx8
NEWSYM cCOpDAx16; PHX s
    cop c_dbgCOpDAx16
NEWSYM cCOp5Ax8 ; PHY s
    cop c_dbgCOp5Ax8
NEWSYM cCOp5Ax16; PHY s
    cop c_dbgCOp5Ax16

NEWSYM cCOp68m8 ; PLA s
    cop c_dbgCOp68m8
NEWSYM cCOp68m16; PLA s
    cop c_dbgCOp68m16
NEWSYM cCOpAB   ; PLB s
    cop c_dbgCOpAB
NEWSYM cCOp2B   ; PLD s
    cop c_dbgCOp2B
NEWSYM cCOp28   ; PLP s
    cop c_dbgCOp28
NEWSYM cCOpFAx8 ; PLX s
    cop c_dbgCOpFAx8
NEWSYM cCOpFAx16; PLX s
    cop c_dbgCOpFAx16
NEWSYM cCOp7Ax8 ; PLY s
    cop c_dbgCOp7Ax8
NEWSYM cCOp7Ax16; PLY s
    cop c_dbgCOp7Ax16

NEWSYM cCOpC2   ; REP #
    cop c_dbgCOpC2

NEWSYM cCOp26m8 ; ROL d
    cop c_dbgCOp26m8
NEWSYM cCOp26m16; ROL d
    cop c_dbgCOp26m16
NEWSYM cCOp2Am8 ; ROL A
    cop c_dbgCOp2Am8
NEWSYM cCOp2Am16; ROL A
    cop c_dbgCOp2Am16
NEWSYM cCOp2Em8 ; ROL a
    cop c_dbgCOp2Em8
NEWSYM cCOp2Em16; ROL a
    cop c_dbgCOp2Em16
NEWSYM cCOp36m8 ; ROL d,x
    cop c_dbgCOp36m8
NEWSYM cCOp36m16; ROL d,x
    cop c_dbgCOp36m16
NEWSYM cCOp3Em8 ; ROL a,x
    cop c_dbgCOp3Em8
NEWSYM cCOp3Em16; ROL a,x
    cop c_dbgCOp3Em16

NEWSYM cCOp66m8 ; ROR d
    cop c_dbgCOp66m8
NEWSYM cCOp66m16; ROR d
    cop c_dbgCOp66m16
NEWSYM cCOp6Am8 ; ROR A
    cop c_dbgCOp6Am8
NEWSYM cCOp6Am16; ROR A
    cop c_dbgCOp6Am16
NEWSYM cCOp6Em8 ; ROR a
    cop c_dbgCOp6Em8
NEWSYM cCOp6Em16; ROR a
    cop c_dbgCOp6Em16
NEWSYM cCOp76m8 ; ROR d,x
    cop c_dbgCOp76m8
NEWSYM cCOp76m16; ROR d,x
    cop c_dbgCOp76m16
NEWSYM cCOp7Em8 ; ROR a,x
    cop c_dbgCOp7Em8
NEWSYM cCOp7Em16; ROR a,x
    cop c_dbgCOp7Em16

NEWSYM cCOp40   ; RTI s
    cop c_dbgCOp40
NEWSYM cCOp6B   ; RTL s
    cop c_dbgCOp6B
NEWSYM cCOp60   ; RTS s
    cop c_dbgCOp60

NEWSYM cCOpE1m8nd  ; SBC(d,x)
    cop c_dbgCOpE1m8nd
NEWSYM cCOpE1m16nd ; SBC(d,x)
    cop c_dbgCOpE1m16nd
NEWSYM cCOpE1m8d   ; SBC(d,x)
    cop c_dbgCOpE1m8d
NEWSYM cCOpE1m16d  ; SBC(d,x)
    cop c_dbgCOpE1m16d
NEWSYM cCOpE3m8nd  ; SBC d,s
    cop c_dbgCOpE3m8nd
NEWSYM cCOpE3m16nd ; SBC d,s
    cop c_dbgCOpE3m16nd
NEWSYM cCOpE3m8d   ; SBC d,s
    cop c_dbgCOpE3m8d
NEWSYM cCOpE3m16d  ; SBC d,s
    cop c_dbgCOpE3m16d
NEWSYM cCOpE5m8nd  ; SBC d
    cop c_dbgCOpE5m8nd
NEWSYM cCOpE5m16nd ; SBC d
    cop c_dbgCOpE5m16nd
NEWSYM cCOpE5m8d   ; SBC d
    cop c_dbgCOpE5m8d
NEWSYM cCOpE5m16d  ; SBC d
    cop c_dbgCOpE5m16d
NEWSYM cCOpE7m8nd  ; SBC [d]
    cop c_dbgCOpE7m8nd
NEWSYM cCOpE7m16nd ; SBC [d]
    cop c_dbgCOpE7m16nd
NEWSYM cCOpE7m8d   ; SBC [d]
    cop c_dbgCOpE7m8d
NEWSYM cCOpE7m16d  ; SBC [d]
    cop c_dbgCOpE7m16d
NEWSYM cCOpE9m8nd  ; SBC #
    cop c_dbgCOpE9m8nd
NEWSYM cCOpE9m16nd ; SBC #
    cop c_dbgCOpE9m16nd
NEWSYM cCOpE9m8d   ; SBC #
    cop c_dbgCOpE9m8d
NEWSYM cCOpE9m16d  ; SBC #
    cop c_dbgCOpE9m16d
NEWSYM cCOpEDm8nd  ; SBC a
    cop c_dbgCOpEDm8nd
NEWSYM cCOpEDm16nd ; SBC a
    cop c_dbgCOpEDm16nd
NEWSYM cCOpEDm8d   ; SBC a
    cop c_dbgCOpEDm8d
NEWSYM cCOpEDm16d  ; SBC a
    cop c_dbgCOpEDm16d
NEWSYM cCOpEFm8nd  ; SBC al
    cop c_dbgCOpEFm8nd
NEWSYM cCOpEFm16nd ; SBC al
    cop c_dbgCOpEFm16nd
NEWSYM cCOpEFm8d   ; SBC al
    cop c_dbgCOpEFm8d
NEWSYM cCOpEFm16d  ; SBC al
    cop c_dbgCOpEFm16d
NEWSYM cCOpF1m8nd  ; SBC(d),y
    cop c_dbgCOpF1m8nd
NEWSYM cCOpF1m16nd ; SBC(d),y
    cop c_dbgCOpF1m16nd
NEWSYM cCOpF1m8d   ; SBC(d),y
    cop c_dbgCOpF1m8d
NEWSYM cCOpF1m16d  ; SBC(d),y
    cop c_dbgCOpF1m16d
NEWSYM cCOpF2m8nd  ; SBC (d)
    cop c_dbgCOpF2m8nd
NEWSYM cCOpF2m16nd ; SBC (d)
    cop c_dbgCOpF2m16nd
NEWSYM cCOpF2m8d   ; SBC (d)
    cop c_dbgCOpF2m8d
NEWSYM cCOpF2m16d  ; SBC (d)
    cop c_dbgCOpF2m16d
NEWSYM cCOpF3m8nd  ; SBC(d,s),y
    cop c_dbgCOpF3m8nd
NEWSYM cCOpF3m16nd ; SBC(d,s),y
    cop c_dbgCOpF3m16nd
NEWSYM cCOpF3m8d   ; SBC(d,s),y
    cop c_dbgCOpF3m8d
NEWSYM cCOpF3m16d  ; SBC(d,s),y
    cop c_dbgCOpF3m16d
NEWSYM cCOpF5m8nd  ; SBC d,x
    cop c_dbgCOpF5m8nd
NEWSYM cCOpF5m16nd ; SBC d,x
    cop c_dbgCOpF5m16nd
NEWSYM cCOpF5m8d   ; SBC d,x
    cop c_dbgCOpF5m8d
NEWSYM cCOpF5m16d  ; SBC d,x
    cop c_dbgCOpF5m16d
NEWSYM cCOpF7m8nd  ; SBC [d],y
    cop c_dbgCOpF7m8nd
NEWSYM cCOpF7m16nd ; SBC [d],y
    cop c_dbgCOpF7m16nd
NEWSYM cCOpF7m8d   ; SBC [d],y
    cop c_dbgCOpF7m8d
NEWSYM cCOpF7m16d  ; SBC [d],y
    cop c_dbgCOpF7m16d
NEWSYM cCOpF9m8nd  ; SBC a,y
    cop c_dbgCOpF9m8nd
NEWSYM cCOpF9m16nd ; SBC a,y
    cop c_dbgCOpF9m16nd
NEWSYM cCOpF9m8d   ; SBC a,y
    cop c_dbgCOpF9m8d
NEWSYM cCOpF9m16d  ; SBC a,y
    cop c_dbgCOpF9m16d
NEWSYM cCOpFDm8nd  ; SBC a,x
    cop c_dbgCOpFDm8nd
NEWSYM cCOpFDm16nd ; SBC a,x
    cop c_dbgCOpFDm16nd
NEWSYM cCOpFDm8d   ; SBC a,x
    cop c_dbgCOpFDm8d
NEWSYM cCOpFDm16d  ; SBC a,x
    cop c_dbgCOpFDm16d
NEWSYM cCOpFFm8nd  ; SBC al,x
    cop c_dbgCOpFFm8nd
NEWSYM cCOpFFm16nd ; SBC al,x
    cop c_dbgCOpFFm16nd
NEWSYM cCOpFFm8d   ; SBC al,x
    cop c_dbgCOpFFm8d
NEWSYM cCOpFFm16d  ; SBC al,x
    cop c_dbgCOpFFm16d

NEWSYM cCOp38   ; SEC i
    cop c_dbgCOp38
NEWSYM cCOpF8   ; SED i
    cop c_dbgCOpF8
NEWSYM cCOp78   ; SEI i
    cop c_dbgCOp78

NEWSYM cCOpE2   ; SEP #
    cop c_dbgCOpE2

NEWSYM cCOp81m8 ; STA(d,x)
    cop c_dbgCOp81m8
NEWSYM cCOp81m16; STA(d,x)
    cop c_dbgCOp81m16
NEWSYM cCOp83m8 ; STA d,s
    cop c_dbgCOp83m8
NEWSYM cCOp83m16; STA d,s
    cop c_dbgCOp83m16
NEWSYM cCOp85m8 ; STA d
    cop c_dbgCOp85m8
NEWSYM cCOp85m16; STA d
    cop c_dbgCOp85m16
NEWSYM cCOp87m8 ; STA [d]
    cop c_dbgCOp87m8
NEWSYM cCOp87m16; STA [d]
    cop c_dbgCOp87m16
NEWSYM cCOp8Dm8 ; STA a
    cop c_dbgCOp8Dm8
NEWSYM cCOp8Dm16; STA a
    cop c_dbgCOp8Dm16
NEWSYM cCOp8Fm8 ; STA al
    cop c_dbgCOp8Fm8
NEWSYM cCOp8Fm16; STA al
    cop c_dbgCOp8Fm16
NEWSYM cCOp91m8 ; STA(d),y
    cop c_dbgCOp91m8
NEWSYM cCOp91m16; STA(d),y
    cop c_dbgCOp91m16
NEWSYM cCOp92m8 ; STA (d)
    cop c_dbgCOp92m8
NEWSYM cCOp92m16; STA (d)
    cop c_dbgCOp92m16
NEWSYM cCOp93m8 ; STA(d,s),y
    cop c_dbgCOp93m8
NEWSYM cCOp93m16; STA(d,s),y
    cop c_dbgCOp93m16
NEWSYM cCOp95m8 ; STA d,x
    cop c_dbgCOp95m8
NEWSYM cCOp95m16; STA d,x
    cop c_dbgCOp95m16
NEWSYM cCOp97m8 ; STA [d],y
    cop c_dbgCOp97m8
NEWSYM cCOp97m16; STA [d],y
    cop c_dbgCOp97m16
NEWSYM cCOp99m8 ; STA a,y
    cop c_dbgCOp99m8
NEWSYM cCOp99m16; STA a,y
    cop c_dbgCOp99m16
NEWSYM cCOp9Dm8 ; STA a,x
    cop c_dbgCOp9Dm8
NEWSYM cCOp9Dm16; STA a,x
    cop c_dbgCOp9Dm16
NEWSYM cCOp9Fm8 ; STA al,x
    cop c_dbgCOp9Fm8
NEWSYM cCOp9Fm16; STA al,x
    cop c_dbgCOp9Fm16

NEWSYM cCOp86x8 ; STX d
    cop c_dbgCOp86x8
NEWSYM cCOp86x16; STX d
    cop c_dbgCOp86x16
NEWSYM cCOp8Ex8 ; STX a
    cop c_dbgCOp8Ex8
NEWSYM cCOp8Ex16; STX a
    cop c_dbgCOp8Ex16
NEWSYM cCOp96x8 ; STX d,y
    cop c_dbgCOp96x8
NEWSYM cCOp96x16; STX d,y
    cop c_dbgCOp96x16

NEWSYM cCOp84x8 ; STY d
    cop c_dbgCOp84x8
NEWSYM cCOp84x16; STY d
    cop c_dbgCOp84x16
NEWSYM cCOp8Cx8 ; STY a
    cop c_dbgCOp8Cx8
NEWSYM cCOp8Cx16; STY a
    cop c_dbgCOp8Cx16
NEWSYM cCOp94x8 ; STYd,x
    cop c_dbgCOp94x8
NEWSYM cCOp94x16; STYd,x
    cop c_dbgCOp94x16

NEWSYM cCOpDB   ; STP i
    cop c_dbgCOpDB

NEWSYM cCOp64m8 ; STZ d
    cop c_dbgCOp64m8
NEWSYM cCOp64m16; STZ d
    cop c_dbgCOp64m16
NEWSYM cCOp74m8 ; STZ d,x
    cop c_dbgCOp74m8
NEWSYM cCOp74m16; STZ d,x
    cop c_dbgCOp74m16
NEWSYM cCOp9Cm8 ; STZ a
    cop c_dbgCOp9Cm8
NEWSYM cCOp9Cm16; STZ a
    cop c_dbgCOp9Cm16
NEWSYM cCOp9Em8 ; STZ a,x
    cop c_dbgCOp9Em8
NEWSYM cCOp9Em16; STZ a,x
    cop c_dbgCOp9Em16

NEWSYM cCOp14m8 ; TRB d
    cop c_dbgCOp14m8
NEWSYM cCOp14m16; TRB d
    cop c_dbgCOp14m16
NEWSYM cCOp1Cm8 ; TRB a
    cop c_dbgCOp1Cm8
NEWSYM cCOp1Cm16; TRB a
    cop c_dbgCOp1Cm16

NEWSYM cCOp04m8 ; TSB d
    cop c_dbgCOp04m8
NEWSYM cCOp04m16; TSB d
    cop c_dbgCOp04m16
NEWSYM cCOp0Cm8 ; TSB a
    cop c_dbgCOp0Cm8
NEWSYM cCOp0Cm16; TSB a
    cop c_dbgCOp0Cm16

NEWSYM cCOpAAx8 ; TAX i
    cop c_dbgCOpAAx8
NEWSYM cCOpAAx16; TAX i
    cop c_dbgCOpAAx16
NEWSYM cCOpA8x8 ; TAY i
    cop c_dbgCOpA8x8
NEWSYM cCOpA8x16; TAY i
    cop c_dbgCOpA8x16
NEWSYM cCOp5B   ; TCD i
    cop c_dbgCOp5B
NEWSYM cCOp1B   ; TCS i
    cop c_dbgCOp1B
NEWSYM cCOp7B   ; TDC i
    cop c_dbgCOp7B
NEWSYM cCOp3B   ; TSC i
    cop c_dbgCOp3B
NEWSYM cCOpBAx8 ; TSX i
    cop c_dbgCOpBAx8
NEWSYM cCOpBAx16; TSX i
    cop c_dbgCOpBAx16
NEWSYM cCOp8Am8 ; TXA i
    cop c_dbgCOp8Am8
NEWSYM cCOp8Am16; TXA i
    cop c_dbgCOp8Am16
NEWSYM cCOp9A   ; TXS i
    cop c_dbgCOp9A
NEWSYM cCOp9Bx8 ; TXY i
    cop c_dbgCOp9Bx8
NEWSYM cCOp9Bx16; TXY i
    cop c_dbgCOp9Bx16
NEWSYM cCOp98m8 ; TYA i
    cop c_dbgCOp98m8
NEWSYM cCOp98m16; TYA i
    cop c_dbgCOp98m16
NEWSYM cCOpBBx8 ; TYX i
    cop c_dbgCOpBBx8
NEWSYM cCOpBBx16; TYX i
    cop c_dbgCOpBBx16

NEWSYM cCOpCB   ; WAI i
    cop c_dbgCOpCB

NEWSYM cCOpEB   ; XBA i
    cop c_dbgCOpEB

NEWSYM cCOpFB   ; XCE i
    cop c_dbgCOpFB

NEWSYM cCOp42   ; WDM
    cop c_dbgCOp42


