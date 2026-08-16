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
EXTSYM regaccessbankr8,snesmap2,snesmmap,stackand,stackor
EXTSYM wramdata,SA1xa,SA1xd,SA1xdb,xe,xirqb,SA1xpb,xpc,SA1xs,SA1xx,SA1xy
EXTSYM SA1RegP
%ifndef NO_DEBUGGER
EXTSYM debstop4,nextopcode,debugsa1,skipdebugsa1
%endif


SECTION .text

sa1debug:
%ifndef NO_DEBUGGER
   cmp byte[debuggeron],0
   je .nodebugger
   cmp byte[skipdebugsa1],1
   je .nodebugger
   pushad
   mov bl,[SA1xpb]
   mov ax,[xpc]
   test eax,8000h
   jz .loweraddr2
   mov eax,[snesmmap+ebx*4]
   jmp .nextaddr
.loweraddr2
   mov eax,[snesmap2+ebx*4]
.nextaddr
   mov ebx,esi
   sub ebx,eax
   mov [xpc],bx
   mov [SA1RegP],dl
   mov byte[debugsa1],1
   ccallv nextopcode
   popad
.nodebugger
%endif
   ret




; B = ( or )
; C = ,
; L = [ or ]
; I = #


NEWSYM SA1COp61m8nd  ; ADC(d,x)
    cop c_SA1COp61m8nd
NEWSYM SA1COp61m16nd ; ADC(d,x)
    cop c_SA1COp61m16nd
NEWSYM SA1COp61m8d   ; ADC(d,x)
    cop c_SA1COp61m8d
NEWSYM SA1COp61m16d  ; ADC(d,x)
    cop c_SA1COp61m16d
NEWSYM SA1COp63m8nd  ; ADC d,s
    cop c_SA1COp63m8nd
NEWSYM SA1COp63m16nd ; ADC d,s
    cop c_SA1COp63m16nd
NEWSYM SA1COp63m8d   ; ADC d,s
    cop c_SA1COp63m8d
NEWSYM SA1COp63m16d  ; ADC d,s
    cop c_SA1COp63m16d
NEWSYM SA1COp65m8nd  ; ADC d
    cop c_SA1COp65m8nd
NEWSYM SA1COp65m16nd ; ADC d
    cop c_SA1COp65m16nd
NEWSYM SA1COp65m8d   ; ADC d
    cop c_SA1COp65m8d
NEWSYM SA1COp65m16d  ; ADC d
    cop c_SA1COp65m16d
NEWSYM SA1COp67m8nd  ; ADC [d]
    cop c_SA1COp67m8nd
NEWSYM SA1COp67m16nd ; ADC [d]
    cop c_SA1COp67m16nd
NEWSYM SA1COp67m8d   ; ADC [d]
    cop c_SA1COp67m8d
NEWSYM SA1COp67m16d  ; ADC [d]
    cop c_SA1COp67m16d
NEWSYM SA1COp69m8nd  ; ADC #
    cop c_SA1COp69m8nd
NEWSYM SA1COp69m16nd ; ADC #
    cop c_SA1COp69m16nd
NEWSYM SA1COp69m8d   ; ADC #
    cop c_SA1COp69m8d
NEWSYM SA1COp69m16d  ; ADC #
    cop c_SA1COp69m16d
NEWSYM SA1COp6Dm8nd  ; ADC a
    cop c_SA1COp6Dm8nd
NEWSYM SA1COp6Dm16nd ; ADC a
    cop c_SA1COp6Dm16nd
NEWSYM SA1COp6Dm8d   ; ADC a
    cop c_SA1COp6Dm8d
NEWSYM SA1COp6Dm16d  ; ADC a
    cop c_SA1COp6Dm16d
NEWSYM SA1COp6Fm8nd  ; ADC al
    cop c_SA1COp6Fm8nd
NEWSYM SA1COp6Fm16nd ; ADC al
    cop c_SA1COp6Fm16nd
NEWSYM SA1COp6Fm8d   ; ADC al
    cop c_SA1COp6Fm8d
NEWSYM SA1COp6Fm16d  ; ADC al
    cop c_SA1COp6Fm16d
NEWSYM SA1COp71m8nd  ; ADC(d),y
    cop c_SA1COp71m8nd
NEWSYM SA1COp71m16nd ; ADC(d),y
    cop c_SA1COp71m16nd
NEWSYM SA1COp71m8d   ; ADC(d),y
    cop c_SA1COp71m8d
NEWSYM SA1COp71m16d  ; ADC(d),y
    cop c_SA1COp71m16d
NEWSYM SA1COp72m8nd  ; ADC (d)
    cop c_SA1COp72m8nd
NEWSYM SA1COp72m16nd ; ADC (d)
    cop c_SA1COp72m16nd
NEWSYM SA1COp72m8d   ; ADC (d)
    cop c_SA1COp72m8d
NEWSYM SA1COp72m16d  ; ADC (d)
    cop c_SA1COp72m16d
NEWSYM SA1COp73m8nd  ; ADC(d,s),y
    cop c_SA1COp73m8nd
NEWSYM SA1COp73m16nd ; ADC(d,s),y
    cop c_SA1COp73m16nd
NEWSYM SA1COp73m8d   ; ADC(d,s),y
    cop c_SA1COp73m8d
NEWSYM SA1COp73m16d  ; ADC(d,s),y
    cop c_SA1COp73m16d
NEWSYM SA1COp75m8nd  ; ADC d,x
    cop c_SA1COp75m8nd
NEWSYM SA1COp75m16nd ; ADC d,x
    cop c_SA1COp75m16nd
NEWSYM SA1COp75m8d   ; ADC d,x
    cop c_SA1COp75m8d
NEWSYM SA1COp75m16d  ; ADC d,x
    cop c_SA1COp75m16d
NEWSYM SA1COp77m8nd  ; ADC [d],y
    cop c_SA1COp77m8nd
NEWSYM SA1COp77m16nd ; ADC [d],y
    cop c_SA1COp77m16nd
NEWSYM SA1COp77m8d   ; ADC [d],y
    cop c_SA1COp77m8d
NEWSYM SA1COp77m16d  ; ADC [d],y
    cop c_SA1COp77m16d
NEWSYM SA1COp79m8nd  ; ADC a,y
    cop c_SA1COp79m8nd
NEWSYM SA1COp79m16nd ; ADC a,y
    cop c_SA1COp79m16nd
NEWSYM SA1COp79m8d   ; ADC a,y
    cop c_SA1COp79m8d
NEWSYM SA1COp79m16d  ; ADC a,y
    cop c_SA1COp79m16d
NEWSYM SA1COp7Dm8nd  ; ADC a,x
    cop c_SA1COp7Dm8nd
NEWSYM SA1COp7Dm16nd ; ADC a,x
    cop c_SA1COp7Dm16nd
NEWSYM SA1COp7Dm8d   ; ADC a,x
    cop c_SA1COp7Dm8d
NEWSYM SA1COp7Dm16d  ; ADC a,x
    cop c_SA1COp7Dm16d
NEWSYM SA1COp7Fm8nd  ; ADC al,x
    cop c_SA1COp7Fm8nd
NEWSYM SA1COp7Fm16nd ; ADC al,x
    cop c_SA1COp7Fm16nd
NEWSYM SA1COp7Fm8d   ; ADC al,x
    cop c_SA1COp7Fm8d
NEWSYM SA1COp7Fm16d  ; ADC al,x
    cop c_SA1COp7Fm16d

NEWSYM SA1COp21m8 ; AND(d,x)
    cop c_SA1COp21m8
NEWSYM SA1COp21m16; AND(d,x)
    cop c_SA1COp21m16
NEWSYM SA1COp23m8 ; AND d,s
    cop c_SA1COp23m8
NEWSYM SA1COp23m16; AND d,s
    cop c_SA1COp23m16
NEWSYM SA1COp25m8 ; AND d
    cop c_SA1COp25m8
NEWSYM SA1COp25m16; AND d
    cop c_SA1COp25m16
NEWSYM SA1COp27m8 ; AND [d]
    cop c_SA1COp27m8
NEWSYM SA1COp27m16; AND [d]
    cop c_SA1COp27m16
NEWSYM SA1COp29m8 ; AND #
    cop c_SA1COp29m8
NEWSYM SA1COp29m16; AND #
    cop c_SA1COp29m16
NEWSYM SA1COp2Dm8 ; AND a
    cop c_SA1COp2Dm8
NEWSYM SA1COp2Dm16; AND a
    cop c_SA1COp2Dm16
NEWSYM SA1COp2Fm8 ; AND al
    cop c_SA1COp2Fm8
NEWSYM SA1COp2Fm16; AND al
    cop c_SA1COp2Fm16
NEWSYM SA1COp31m8 ; AND(d),y
    cop c_SA1COp31m8
NEWSYM SA1COp31m16; AND(d),y
    cop c_SA1COp31m16
NEWSYM SA1COp32m8 ; AND (d)
    cop c_SA1COp32m8
NEWSYM SA1COp32m16; AND (d)
    cop c_SA1COp32m16
NEWSYM SA1COp33m8 ; AND(d,s),y
    cop c_SA1COp33m8
NEWSYM SA1COp33m16; AND(d,s),y
    cop c_SA1COp33m16
NEWSYM SA1COp35m8 ; AND d,x
    cop c_SA1COp35m8
NEWSYM SA1COp35m16; AND d,x
    cop c_SA1COp35m16
NEWSYM SA1COp37m8 ; AND [d],y
    cop c_SA1COp37m8
NEWSYM SA1COp37m16; AND [d],y
    cop c_SA1COp37m16
NEWSYM SA1COp39m8 ; AND a,y
    cop c_SA1COp39m8
NEWSYM SA1COp39m16; AND a,y
    cop c_SA1COp39m16
NEWSYM SA1COp3Dm8 ; AND a,x
    cop c_SA1COp3Dm8
NEWSYM SA1COp3Dm16; AND a,x
    cop c_SA1COp3Dm16
NEWSYM SA1COp3Fm8 ; AND al,x
    cop c_SA1COp3Fm8
NEWSYM SA1COp3Fm16; AND al,x
    cop c_SA1COp3Fm16

NEWSYM SA1COp06m8 ; ASL d
    cop c_SA1COp06m8
NEWSYM SA1COp06m16; ASL d
    cop c_SA1COp06m16
NEWSYM SA1COp0Am8 ; ASL A
    cop c_SA1COp0Am8
NEWSYM SA1COp0Am16; ASL A
    cop c_SA1COp0Am16
NEWSYM SA1COp0Em8 ; ASL a
    cop c_SA1COp0Em8
NEWSYM SA1COp0Em16; ASL a
    cop c_SA1COp0Em16
NEWSYM SA1COp16m8 ; ASL d,x
    cop c_SA1COp16m8
NEWSYM SA1COp16m16; ASL d,x
    cop c_SA1COp16m16
NEWSYM SA1COp1Em8 ; ASL a,x
    cop c_SA1COp1Em8
NEWSYM SA1COp1Em16; ASL a,x
    cop c_SA1COp1Em16

NEWSYM SA1COp90   ; BCC r
    cop c_SA1COp90
NEWSYM SA1COpB0   ; BCS r
    cop c_SA1COpB0
NEWSYM SA1COpF0   ; BEQ r
    cop c_SA1COpF0
NEWSYM SA1COp30   ; BMI r
    cop c_SA1COp30
NEWSYM SA1COpD0   ; BNE r
    cop c_SA1COpD0
NEWSYM SA1COp10   ; BPL r
    cop c_SA1COp10
NEWSYM SA1COp80   ; BRA r
    cop c_SA1COp80
NEWSYM SA1COp50   ; BVC r
    cop c_SA1COp50
NEWSYM SA1COp70   ; BVS r
    cop c_SA1COp70

NEWSYM SA1COp89m8 ; BIT #
    cop c_SA1COp89m8
NEWSYM SA1COp89m16; BIT #
    cop c_SA1COp89m16

NEWSYM SA1COp24m8 ; BIT d
    cop c_SA1COp24m8
NEWSYM SA1COp24m16; BIT d
    cop c_SA1COp24m16
NEWSYM SA1COp2Cm8 ; BIT a
    cop c_SA1COp2Cm8
NEWSYM SA1COp2Cm16; BIT a
    cop c_SA1COp2Cm16
NEWSYM SA1COp34m8 ; BIT d,x
    cop c_SA1COp34m8
NEWSYM SA1COp34m16; BIT d,x
    cop c_SA1COp34m16
NEWSYM SA1COp3Cm8 ; BIT a,x
    cop c_SA1COp3Cm8
NEWSYM SA1COp3Cm16; BIT a,x
    cop c_SA1COp3Cm16

NEWSYM SA1COp00   ; BRK s
    cop c_SA1COp00

NEWSYM SA1COp82   ; BRL rl
    cop c_SA1COp82

NEWSYM SA1COp18   ; CLC i
    cop c_SA1COp18
NEWSYM SA1COpD8   ; CLD i
    cop c_SA1COpD8
NEWSYM SA1COp58   ; CLI i
    cop c_SA1COp58
NEWSYM SA1COpB8   ; CLV i
    cop c_SA1COpB8

NEWSYM SA1COpC1m8 ; CMP(d,x)
    cop c_SA1COpC1m8
NEWSYM SA1COpC1m16; CMP(d,x)
    cop c_SA1COpC1m16
NEWSYM SA1COpC3m8 ; CMP d,s
    cop c_SA1COpC3m8
NEWSYM SA1COpC3m16; CMP d,s
    cop c_SA1COpC3m16
NEWSYM SA1COpC5m8 ; CMP d
    cop c_SA1COpC5m8
NEWSYM SA1COpC5m16; CMP d
    cop c_SA1COpC5m16
NEWSYM SA1COpC7m8 ; CMP [d]
    cop c_SA1COpC7m8
NEWSYM SA1COpC7m16; CMP [d]
    cop c_SA1COpC7m16
NEWSYM SA1COpC9m8 ; CMP #
    cop c_SA1COpC9m8
NEWSYM SA1COpC9m16; CMP #
    cop c_SA1COpC9m16
NEWSYM SA1COpCDm8 ; CMP a
    cop c_SA1COpCDm8
NEWSYM SA1COpCDm16; CMP a
    cop c_SA1COpCDm16
NEWSYM SA1COpCFm8 ; CMP al
    cop c_SA1COpCFm8
NEWSYM SA1COpCFm16; CMP al
    cop c_SA1COpCFm16
NEWSYM SA1COpD1m8 ; CMP(d),y
    cop c_SA1COpD1m8
NEWSYM SA1COpD1m16; CMP(d),y
    cop c_SA1COpD1m16
NEWSYM SA1COpD2m8 ; CMP (d)
    cop c_SA1COpD2m8
NEWSYM SA1COpD2m16; CMP (d)
    cop c_SA1COpD2m16
NEWSYM SA1COpD3m8 ; CMP(d,s),y
    cop c_SA1COpD3m8
NEWSYM SA1COpD3m16; CMP(d,s),y
    cop c_SA1COpD3m16
NEWSYM SA1COpD5m8 ; CMP d,x
    cop c_SA1COpD5m8
NEWSYM SA1COpD5m16; CMP d,x
    cop c_SA1COpD5m16
NEWSYM SA1COpD7m8 ; CMP [d],y
    cop c_SA1COpD7m8
NEWSYM SA1COpD7m16; CMP [d],y
    cop c_SA1COpD7m16
NEWSYM SA1COpD9m8 ; CMP a,y
    cop c_SA1COpD9m8
NEWSYM SA1COpD9m16; CMP a,y
    cop c_SA1COpD9m16
NEWSYM SA1COpDDm8 ; CMP a,x
    cop c_SA1COpDDm8
NEWSYM SA1COpDDm16; CMP a,x
    cop c_SA1COpDDm16
NEWSYM SA1COpDFm8 ; CMP al,x
    cop c_SA1COpDFm8
NEWSYM SA1COpDFm16; CMP al,x
    cop c_SA1COpDFm16

NEWSYM SA1COp02   ; SA1COp s
    cop c_SA1COp02

NEWSYM SA1COpE0x8 ; CPX #
    cop c_SA1COpE0x8
NEWSYM SA1COpE0x16; CPX #
    cop c_SA1COpE0x16
NEWSYM SA1COpE4x8 ; CPX d
    cop c_SA1COpE4x8
NEWSYM SA1COpE4x16; CPX d
    cop c_SA1COpE4x16
NEWSYM SA1COpECx8 ; CPX a
    cop c_SA1COpECx8
NEWSYM SA1COpECx16; CPX a
    cop c_SA1COpECx16

NEWSYM SA1COpC0x8 ; CPY #
    cop c_SA1COpC0x8
NEWSYM SA1COpC0x16; CPY #
    cop c_SA1COpC0x16
NEWSYM SA1COpC4x8 ; CPY d
    cop c_SA1COpC4x8
NEWSYM SA1COpC4x16; CPY d
    cop c_SA1COpC4x16
NEWSYM SA1COpCCx8 ; CPY a
    cop c_SA1COpCCx8
NEWSYM SA1COpCCx16; CPY a
    cop c_SA1COpCCx16

NEWSYM SA1COp3Am8 ; DEC A
    cop c_SA1COp3Am8
NEWSYM SA1COp3Am16; DEC A
    cop c_SA1COp3Am16
NEWSYM SA1COpCEm8 ; DEC a
    cop c_SA1COpCEm8
NEWSYM SA1COpCEm16; DEC a
    cop c_SA1COpCEm16
NEWSYM SA1COpC6m8 ; DEC d
    cop c_SA1COpC6m8
NEWSYM SA1COpC6m16; DEC d
    cop c_SA1COpC6m16
NEWSYM SA1COpD6m8 ; DEC d,x
    cop c_SA1COpD6m8
NEWSYM SA1COpD6m16; DEC d,x
    cop c_SA1COpD6m16
NEWSYM SA1COpDEm8 ; DEC a,x
    cop c_SA1COpDEm8
NEWSYM SA1COpDEm16; DEC a,x
    cop c_SA1COpDEm16

NEWSYM SA1COpCAx8 ; DEX i
    cop c_SA1COpCAx8
NEWSYM SA1COpCAx16; DEX i
    cop c_SA1COpCAx16
NEWSYM SA1COp88x8 ; DEY i
    cop c_SA1COp88x8
NEWSYM SA1COp88x16; DEY i
    cop c_SA1COp88x16

NEWSYM SA1COp41m8 ; EOR(d,x)
    cop c_SA1COp41m8
NEWSYM SA1COp41m16; EOR(d,x)
    cop c_SA1COp41m16
NEWSYM SA1COp43m8 ; EOR d,s
    cop c_SA1COp43m8
NEWSYM SA1COp43m16; EOR d,s
    cop c_SA1COp43m16
NEWSYM SA1COp45m8 ; EOR d
    cop c_SA1COp45m8
NEWSYM SA1COp45m16; EOR d
    cop c_SA1COp45m16
NEWSYM SA1COp47m8 ; EOR [d]
    cop c_SA1COp47m8
NEWSYM SA1COp47m16; EOR [d]
    cop c_SA1COp47m16
NEWSYM SA1COp49m8 ; EOR #
    cop c_SA1COp49m8
NEWSYM SA1COp49m16; EOR #
    cop c_SA1COp49m16
NEWSYM SA1COp4Dm8 ; EOR a
    cop c_SA1COp4Dm8
NEWSYM SA1COp4Dm16; EOR a
    cop c_SA1COp4Dm16
NEWSYM SA1COp4Fm8 ; EOR al
    cop c_SA1COp4Fm8
NEWSYM SA1COp4Fm16; EOR al
    cop c_SA1COp4Fm16
NEWSYM SA1COp51m8 ; EOR(d),y
    cop c_SA1COp51m8
NEWSYM SA1COp51m16; EOR(d),y
    cop c_SA1COp51m16
NEWSYM SA1COp52m8 ; EOR (d)
    cop c_SA1COp52m8
NEWSYM SA1COp52m16; EOR (d)
    cop c_SA1COp52m16
NEWSYM SA1COp53m8 ; EOR(d,s),y
    cop c_SA1COp53m8
NEWSYM SA1COp53m16; EOR(d,s),y
    cop c_SA1COp53m16
NEWSYM SA1COp55m8 ; EOR d,x
    cop c_SA1COp55m8
NEWSYM SA1COp55m16; EOR d,x
    cop c_SA1COp55m16
NEWSYM SA1COp57m8 ; EOR [d],y
    cop c_SA1COp57m8
NEWSYM SA1COp57m16; EOR [d],y
    cop c_SA1COp57m16
NEWSYM SA1COp59m8 ; EOR a,y
    cop c_SA1COp59m8
NEWSYM SA1COp59m16; EOR a,y
    cop c_SA1COp59m16
NEWSYM SA1COp5Dm8 ; EORa,x
    cop c_SA1COp5Dm8
NEWSYM SA1COp5Dm16; EORa,x
    cop c_SA1COp5Dm16
NEWSYM SA1COp5Fm8 ; EOR al,x
    cop c_SA1COp5Fm8
NEWSYM SA1COp5Fm16; EOR al,x
    cop c_SA1COp5Fm16


NEWSYM SA1COp1Am8 ; INC A
    cop c_SA1COp1Am8
NEWSYM SA1COp1Am16; INC A
    cop c_SA1COp1Am16
NEWSYM SA1COpEEm8 ; INC a
    cop c_SA1COpEEm8
NEWSYM SA1COpEEm16; INC a
    cop c_SA1COpEEm16
NEWSYM SA1COpE6m8 ; INC d
    cop c_SA1COpE6m8
NEWSYM SA1COpE6m16; INC d
    cop c_SA1COpE6m16
NEWSYM SA1COpF6m8 ; INC d,x
    cop c_SA1COpF6m8
NEWSYM SA1COpF6m16; INC d,x
    cop c_SA1COpF6m16
NEWSYM SA1COpFEm8 ; INC a,x
    cop c_SA1COpFEm8
NEWSYM SA1COpFEm16; INC a,x
    cop c_SA1COpFEm16


NEWSYM SA1COpE8x8 ; INX i
    cop c_SA1COpE8x8
NEWSYM SA1COpE8x16; INX i
    cop c_SA1COpE8x16
NEWSYM SA1COpC8x8 ; INY i
    cop c_SA1COpC8x8
NEWSYM SA1COpC8x16; INY i
    cop c_SA1COpC8x16

NEWSYM SA1COpDC   ; JML (a)
    cop c_SA1COpDC
NEWSYM SA1COp4C   ; JMP a
    cop c_SA1COp4C
NEWSYM SA1COp6C   ; JMP (a)
    cop c_SA1COp6C
NEWSYM SA1COp7C   ; JMP(a,x)
    cop c_SA1COp7C
NEWSYM SA1COp5C   ; JMP al
    cop c_SA1COp5C

NEWSYM SA1COp22   ; JSL al
    cop c_SA1COp22
NEWSYM SA1COp20   ; JSR a
    cop c_SA1COp20
NEWSYM SA1COpFC   ; JSR(a,x)
    cop c_SA1COpFC

NEWSYM SA1COpA1m8 ; LDA(d,x)
    cop c_SA1COpA1m8
NEWSYM SA1COpA1m16; LDA(d,x)
    cop c_SA1COpA1m16
NEWSYM SA1COpA3m8 ; LDA d,s
    cop c_SA1COpA3m8
NEWSYM SA1COpA3m16; LDA d,s
    cop c_SA1COpA3m16
NEWSYM SA1COpA5m8 ; LDA d
    cop c_SA1COpA5m8
NEWSYM SA1COpA5m16; LDA d
    cop c_SA1COpA5m16
NEWSYM SA1COpA7m8 ; LDA [d]
    cop c_SA1COpA7m8
NEWSYM SA1COpA7m16; LDA [d]
    cop c_SA1COpA7m16
NEWSYM SA1COpA9m8 ; LDA #
    cop c_SA1COpA9m8
NEWSYM SA1COpA9m16; LDA #
    cop c_SA1COpA9m16
NEWSYM SA1COpADm8 ; LDA a
    cop c_SA1COpADm8
NEWSYM SA1COpADm16; LDA a
    cop c_SA1COpADm16
NEWSYM SA1COpAFm8 ; LDA al
    cop c_SA1COpAFm8
NEWSYM SA1COpAFm16; LDA al
    cop c_SA1COpAFm16
NEWSYM SA1COpB1m8 ; LDA(d),y
    cop c_SA1COpB1m8
NEWSYM SA1COpB1m16; LDA(d),y
    cop c_SA1COpB1m16
NEWSYM SA1COpB2m8 ; LDA (d)
    cop c_SA1COpB2m8
NEWSYM SA1COpB2m16; LDA (d)
    cop c_SA1COpB2m16
NEWSYM SA1COpB3m8 ; LDA(d,s),y
    cop c_SA1COpB3m8
NEWSYM SA1COpB3m16; LDA(d,s),y
    cop c_SA1COpB3m16
NEWSYM SA1COpB5m8 ; LDA d,x
    cop c_SA1COpB5m8
NEWSYM SA1COpB5m16; LDA d,x
    cop c_SA1COpB5m16
NEWSYM SA1COpB7m8 ; LDA [d],y
    cop c_SA1COpB7m8
NEWSYM SA1COpB7m16; LDA [d],y
    cop c_SA1COpB7m16
NEWSYM SA1COpB9m8 ; LDA a,y
    cop c_SA1COpB9m8
NEWSYM SA1COpB9m16; LDA a,y
    cop c_SA1COpB9m16
NEWSYM SA1COpBDm8 ; LDA a,x
    cop c_SA1COpBDm8
NEWSYM SA1COpBDm16; LDA a,x
    cop c_SA1COpBDm16
NEWSYM SA1COpBFm8 ; LDA al,x
    cop c_SA1COpBFm8
NEWSYM SA1COpBFm16; LDA al,x
    cop c_SA1COpBFm16

NEWSYM SA1COpA2x8 ; LDX #
    cop c_SA1COpA2x8
NEWSYM SA1COpA2x16; LDX #
    cop c_SA1COpA2x16
NEWSYM SA1COpA6x8 ; LDX d
    cop c_SA1COpA6x8
NEWSYM SA1COpA6x16; LDX d
    cop c_SA1COpA6x16
NEWSYM SA1COpAEx8 ; LDX a
    cop c_SA1COpAEx8
NEWSYM SA1COpAEx16; LDX a
    cop c_SA1COpAEx16
NEWSYM SA1COpB6x8 ; LDX d,y
    cop c_SA1COpB6x8
NEWSYM SA1COpB6x16; LDX d,y
    cop c_SA1COpB6x16
NEWSYM SA1COpBEx8 ; LDX a,y
    cop c_SA1COpBEx8
NEWSYM SA1COpBEx16; LDX a,y
    cop c_SA1COpBEx16

NEWSYM SA1COpA0x8 ; LDY #
    cop c_SA1COpA0x8
NEWSYM SA1COpA0x16; LDY #
    cop c_SA1COpA0x16
NEWSYM SA1COpA4x8 ; LDY d
    cop c_SA1COpA4x8
NEWSYM SA1COpA4x16; LDY d
    cop c_SA1COpA4x16
NEWSYM SA1COpACx8 ; LDY a
    cop c_SA1COpACx8
NEWSYM SA1COpACx16; LDY a
    cop c_SA1COpACx16
NEWSYM SA1COpB4x8 ; LDY d,x
    cop c_SA1COpB4x8
NEWSYM SA1COpB4x16; LDY d,x
    cop c_SA1COpB4x16
NEWSYM SA1COpBCx8 ; LDY a,x
    cop c_SA1COpBCx8
NEWSYM SA1COpBCx16; LDY a,x
    cop c_SA1COpBCx16

NEWSYM SA1COp46m8 ; LSR d
    cop c_SA1COp46m8
NEWSYM SA1COp46m16; LSR d
    cop c_SA1COp46m16
NEWSYM SA1COp4Am8 ; LSR A
    cop c_SA1COp4Am8
NEWSYM SA1COp4Am16; LSR A
    cop c_SA1COp4Am16
NEWSYM SA1COp4Em8 ; LSR a
    cop c_SA1COp4Em8
NEWSYM SA1COp4Em16; LSR a
    cop c_SA1COp4Em16
NEWSYM SA1COp56m8 ; LSR d,x
    cop c_SA1COp56m8
NEWSYM SA1COp56m16; LSR d,x
    cop c_SA1COp56m16
NEWSYM SA1COp5Em8 ; LSRa,x
    cop c_SA1COp5Em8
NEWSYM SA1COp5Em16; LSRa,x
    cop c_SA1COp5Em16

NEWSYM SA1COp54   ; MVN SA1xya
    cop c_SA1COp54
NEWSYM SA1COp44   ; MVP SA1xya
    cop c_SA1COp44

NEWSYM SA1COpEA   ; NOP i
    cop c_SA1COpEA

NEWSYM SA1COp01m8 ; ORA(d,x)
    cop c_SA1COp01m8
NEWSYM SA1COp01m16; ORA(d,x)
    cop c_SA1COp01m16
NEWSYM SA1COp03m8 ; ORA d,s
    cop c_SA1COp03m8
NEWSYM SA1COp03m16; ORA d,s
    cop c_SA1COp03m16
NEWSYM SA1COp05m8 ; ORA d
    cop c_SA1COp05m8
NEWSYM SA1COp05m16; ORA d
    cop c_SA1COp05m16
NEWSYM SA1COp07m8 ; ORA [d]
    cop c_SA1COp07m8
NEWSYM SA1COp07m16; ORA [d]
    cop c_SA1COp07m16
NEWSYM SA1COp09m8 ; ORA #
    cop c_SA1COp09m8
NEWSYM SA1COp09m16; ORA #
    cop c_SA1COp09m16
NEWSYM SA1COp0Dm8 ; ORA a
    cop c_SA1COp0Dm8
NEWSYM SA1COp0Dm16; ORA a
    cop c_SA1COp0Dm16
NEWSYM SA1COp0Fm8 ; ORA al
    cop c_SA1COp0Fm8
NEWSYM SA1COp0Fm16; ORA al
    cop c_SA1COp0Fm16
NEWSYM SA1COp11m8 ; ORA(d),y
    cop c_SA1COp11m8
NEWSYM SA1COp11m16; ORA(d),y
    cop c_SA1COp11m16
NEWSYM SA1COp12m8 ; ORA(d)
    cop c_SA1COp12m8
NEWSYM SA1COp12m16; ORA(d)
    cop c_SA1COp12m16
NEWSYM SA1COp13m8 ; ORA(d,s),y
    cop c_SA1COp13m8
NEWSYM SA1COp13m16; ORA(d,s),y
    cop c_SA1COp13m16
NEWSYM SA1COp15m8 ; ORA d,x
    cop c_SA1COp15m8
NEWSYM SA1COp15m16; ORA d,x
    cop c_SA1COp15m16
NEWSYM SA1COp17m8 ; ORA [d],y
    cop c_SA1COp17m8
NEWSYM SA1COp17m16; ORA [d],y
    cop c_SA1COp17m16
NEWSYM SA1COp19m8 ; ORA a,y
    cop c_SA1COp19m8
NEWSYM SA1COp19m16; ORA a,y
    cop c_SA1COp19m16
NEWSYM SA1COp1Dm8 ; ORA a,x
    cop c_SA1COp1Dm8
NEWSYM SA1COp1Dm16; ORA a,x
    cop c_SA1COp1Dm16
NEWSYM SA1COp1Fm8 ; ORA al,x
    cop c_SA1COp1Fm8
NEWSYM SA1COp1Fm16; ORA al,x
    cop c_SA1COp1Fm16

NEWSYM SA1COpF4   ; PEA s
    cop c_SA1COpF4
NEWSYM SA1COpD4   ; PEI s
    cop c_SA1COpD4
NEWSYM SA1COp62   ; PER s
    cop c_SA1COp62

NEWSYM SA1COp48m8 ; PHA s
    cop c_SA1COp48m8
NEWSYM SA1COp48m16; PHA s
    cop c_SA1COp48m16
NEWSYM SA1COp8B   ; PHB s
    cop c_SA1COp8B
NEWSYM SA1COp0B   ; PHD s
    cop c_SA1COp0B
NEWSYM SA1COp4B   ; PHK s
    cop c_SA1COp4B
NEWSYM SA1COp08   ; PHP s
    cop c_SA1COp08
NEWSYM SA1COpDAx8 ; PHX s
    cop c_SA1COpDAx8
NEWSYM SA1COpDAx16; PHX s
    cop c_SA1COpDAx16
NEWSYM SA1COp5Ax8 ; PHY s
    cop c_SA1COp5Ax8
NEWSYM SA1COp5Ax16; PHY s
    cop c_SA1COp5Ax16

NEWSYM SA1COp68m8 ; PLA s
    cop c_SA1COp68m8
NEWSYM SA1COp68m16; PLA s
    cop c_SA1COp68m16
NEWSYM SA1COpAB   ; PLB s
    cop c_SA1COpAB
NEWSYM SA1COp2B   ; PLD s
    cop c_SA1COp2B
NEWSYM SA1COp28   ; PLP s
    cop c_SA1COp28
NEWSYM SA1COpFAx8 ; PLX s
    cop c_SA1COpFAx8
NEWSYM SA1COpFAx16; PLX s
    cop c_SA1COpFAx16
NEWSYM SA1COp7Ax8 ; PLY s
    cop c_SA1COp7Ax8
NEWSYM SA1COp7Ax16; PLY s
    cop c_SA1COp7Ax16

NEWSYM SA1COpC2   ; REP #
    cop c_SA1COpC2

NEWSYM SA1COp26m8 ; ROL d
    cop c_SA1COp26m8
NEWSYM SA1COp26m16; ROL d
    cop c_SA1COp26m16
NEWSYM SA1COp2Am8 ; ROL A
    cop c_SA1COp2Am8
NEWSYM SA1COp2Am16; ROL A
    cop c_SA1COp2Am16
NEWSYM SA1COp2Em8 ; ROL a
    cop c_SA1COp2Em8
NEWSYM SA1COp2Em16; ROL a
    cop c_SA1COp2Em16
NEWSYM SA1COp36m8 ; ROL d,x
    cop c_SA1COp36m8
NEWSYM SA1COp36m16; ROL d,x
    cop c_SA1COp36m16
NEWSYM SA1COp3Em8 ; ROL a,x
    cop c_SA1COp3Em8
NEWSYM SA1COp3Em16; ROL a,x
    cop c_SA1COp3Em16

NEWSYM SA1COp66m8 ; ROR d
    cop c_SA1COp66m8
NEWSYM SA1COp66m16; ROR d
    cop c_SA1COp66m16
NEWSYM SA1COp6Am8 ; ROR A
    cop c_SA1COp6Am8
NEWSYM SA1COp6Am16; ROR A
    cop c_SA1COp6Am16
NEWSYM SA1COp6Em8 ; ROR a
    cop c_SA1COp6Em8
NEWSYM SA1COp6Em16; ROR a
    cop c_SA1COp6Em16
NEWSYM SA1COp76m8 ; ROR d,x
    cop c_SA1COp76m8
NEWSYM SA1COp76m16; ROR d,x
    cop c_SA1COp76m16
NEWSYM SA1COp7Em8 ; ROR a,x
    cop c_SA1COp7Em8
NEWSYM SA1COp7Em16; ROR a,x
    cop c_SA1COp7Em16

NEWSYM SA1COp40   ; RTI s
    cop c_SA1COp40
NEWSYM SA1COp6B   ; RTL s
    cop c_SA1COp6B
NEWSYM SA1COp60   ; RTS s
    cop c_SA1COp60

NEWSYM SA1COpE1m8nd  ; SBC(d,x)
    cop c_SA1COpE1m8nd
NEWSYM SA1COpE1m16nd ; SBC(d,x)
    cop c_SA1COpE1m16nd
NEWSYM SA1COpE1m8d   ; SBC(d,x)
    cop c_SA1COpE1m8d
NEWSYM SA1COpE1m16d  ; SBC(d,x)
    cop c_SA1COpE1m16d
NEWSYM SA1COpE3m8nd  ; SBC d,s
    cop c_SA1COpE3m8nd
NEWSYM SA1COpE3m16nd ; SBC d,s
    cop c_SA1COpE3m16nd
NEWSYM SA1COpE3m8d   ; SBC d,s
    cop c_SA1COpE3m8d
NEWSYM SA1COpE3m16d  ; SBC d,s
    cop c_SA1COpE3m16d
NEWSYM SA1COpE5m8nd  ; SBC d
    cop c_SA1COpE5m8nd
NEWSYM SA1COpE5m16nd ; SBC d
    cop c_SA1COpE5m16nd
NEWSYM SA1COpE5m8d   ; SBC d
    cop c_SA1COpE5m8d
NEWSYM SA1COpE5m16d  ; SBC d
    cop c_SA1COpE5m16d
NEWSYM SA1COpE7m8nd  ; SBC [d]
    cop c_SA1COpE7m8nd
NEWSYM SA1COpE7m16nd ; SBC [d]
    cop c_SA1COpE7m16nd
NEWSYM SA1COpE7m8d   ; SBC [d]
    cop c_SA1COpE7m8d
NEWSYM SA1COpE7m16d  ; SBC [d]
    cop c_SA1COpE7m16d
NEWSYM SA1COpE9m8nd  ; SBC #
    cop c_SA1COpE9m8nd
NEWSYM SA1COpE9m16nd ; SBC #
    cop c_SA1COpE9m16nd
NEWSYM SA1COpE9m8d   ; SBC #
    cop c_SA1COpE9m8d
NEWSYM SA1COpE9m16d  ; SBC #
    cop c_SA1COpE9m16d
NEWSYM SA1COpEDm8nd  ; SBC a
    cop c_SA1COpEDm8nd
NEWSYM SA1COpEDm16nd ; SBC a
    cop c_SA1COpEDm16nd
NEWSYM SA1COpEDm8d   ; SBC a
    cop c_SA1COpEDm8d
NEWSYM SA1COpEDm16d  ; SBC a
    cop c_SA1COpEDm16d
NEWSYM SA1COpEFm8nd  ; SBC al
    cop c_SA1COpEFm8nd
NEWSYM SA1COpEFm16nd ; SBC al
    cop c_SA1COpEFm16nd
NEWSYM SA1COpEFm8d   ; SBC al
    cop c_SA1COpEFm8d
NEWSYM SA1COpEFm16d  ; SBC al
    cop c_SA1COpEFm16d
NEWSYM SA1COpF1m8nd  ; SBC(d),y
    cop c_SA1COpF1m8nd
NEWSYM SA1COpF1m16nd ; SBC(d),y
    cop c_SA1COpF1m16nd
NEWSYM SA1COpF1m8d   ; SBC(d),y
    cop c_SA1COpF1m8d
NEWSYM SA1COpF1m16d  ; SBC(d),y
    cop c_SA1COpF1m16d
NEWSYM SA1COpF2m8nd  ; SBC (d)
    cop c_SA1COpF2m8nd
NEWSYM SA1COpF2m16nd ; SBC (d)
    cop c_SA1COpF2m16nd
NEWSYM SA1COpF2m8d   ; SBC (d)
    cop c_SA1COpF2m8d
NEWSYM SA1COpF2m16d  ; SBC (d)
    cop c_SA1COpF2m16d
NEWSYM SA1COpF3m8nd  ; SBC(d,s),y
    cop c_SA1COpF3m8nd
NEWSYM SA1COpF3m16nd ; SBC(d,s),y
    cop c_SA1COpF3m16nd
NEWSYM SA1COpF3m8d   ; SBC(d,s),y
    cop c_SA1COpF3m8d
NEWSYM SA1COpF3m16d  ; SBC(d,s),y
    cop c_SA1COpF3m16d
NEWSYM SA1COpF5m8nd  ; SBC d,x
    cop c_SA1COpF5m8nd
NEWSYM SA1COpF5m16nd ; SBC d,x
    cop c_SA1COpF5m16nd
NEWSYM SA1COpF5m8d   ; SBC d,x
    cop c_SA1COpF5m8d
NEWSYM SA1COpF5m16d  ; SBC d,x
    cop c_SA1COpF5m16d
NEWSYM SA1COpF7m8nd  ; SBC [d],y
    cop c_SA1COpF7m8nd
NEWSYM SA1COpF7m16nd ; SBC [d],y
    cop c_SA1COpF7m16nd
NEWSYM SA1COpF7m8d   ; SBC [d],y
    cop c_SA1COpF7m8d
NEWSYM SA1COpF7m16d  ; SBC [d],y
    cop c_SA1COpF7m16d
NEWSYM SA1COpF9m8nd  ; SBC a,y
    cop c_SA1COpF9m8nd
NEWSYM SA1COpF9m16nd ; SBC a,y
    cop c_SA1COpF9m16nd
NEWSYM SA1COpF9m8d   ; SBC a,y
    cop c_SA1COpF9m8d
NEWSYM SA1COpF9m16d  ; SBC a,y
    cop c_SA1COpF9m16d
NEWSYM SA1COpFDm8nd  ; SBC a,x
    cop c_SA1COpFDm8nd
NEWSYM SA1COpFDm16nd ; SBC a,x
    cop c_SA1COpFDm16nd
NEWSYM SA1COpFDm8d   ; SBC a,x
    cop c_SA1COpFDm8d
NEWSYM SA1COpFDm16d  ; SBC a,x
    cop c_SA1COpFDm16d
NEWSYM SA1COpFFm8nd  ; SBC al,x
    cop c_SA1COpFFm8nd
NEWSYM SA1COpFFm16nd ; SBC al,x
    cop c_SA1COpFFm16nd
NEWSYM SA1COpFFm8d   ; SBC al,x
    cop c_SA1COpFFm8d
NEWSYM SA1COpFFm16d  ; SBC al,x
    cop c_SA1COpFFm16d

NEWSYM SA1COp38   ; SEC i
    cop c_SA1COp38
NEWSYM SA1COpF8   ; SED i
    cop c_SA1COpF8
NEWSYM SA1COp78   ; SEI i
    cop c_SA1COp78

NEWSYM SA1COpE2   ; SEP #
    cop c_SA1COpE2

NEWSYM SA1COp81m8 ; STA(d,x)
    cop c_SA1COp81m8
NEWSYM SA1COp81m16; STA(d,x)
    cop c_SA1COp81m16
NEWSYM SA1COp83m8 ; STA d,s
    cop c_SA1COp83m8
NEWSYM SA1COp83m16; STA d,s
    cop c_SA1COp83m16
NEWSYM SA1COp85m8 ; STA d
    cop c_SA1COp85m8
NEWSYM SA1COp85m16; STA d
    cop c_SA1COp85m16
NEWSYM SA1COp87m8 ; STA [d]
    cop c_SA1COp87m8
NEWSYM SA1COp87m16; STA [d]
    cop c_SA1COp87m16
NEWSYM SA1COp8Dm8 ; STA a
    cop c_SA1COp8Dm8
NEWSYM SA1COp8Dm16; STA a
    cop c_SA1COp8Dm16
NEWSYM SA1COp8Fm8 ; STA al
    cop c_SA1COp8Fm8
NEWSYM SA1COp8Fm16; STA al
    cop c_SA1COp8Fm16
NEWSYM SA1COp91m8 ; STA(d),y
    cop c_SA1COp91m8
NEWSYM SA1COp91m16; STA(d),y
    cop c_SA1COp91m16
NEWSYM SA1COp92m8 ; STA (d)
    cop c_SA1COp92m8
NEWSYM SA1COp92m16; STA (d)
    cop c_SA1COp92m16
NEWSYM SA1COp93m8 ; STA(d,s),y
    cop c_SA1COp93m8
NEWSYM SA1COp93m16; STA(d,s),y
    cop c_SA1COp93m16
NEWSYM SA1COp95m8 ; STA d,x
    cop c_SA1COp95m8
NEWSYM SA1COp95m16; STA d,x
    cop c_SA1COp95m16
NEWSYM SA1COp97m8 ; STA [d],y
    cop c_SA1COp97m8
NEWSYM SA1COp97m16; STA [d],y
    cop c_SA1COp97m16
NEWSYM SA1COp99m8 ; STA a,y
    cop c_SA1COp99m8
NEWSYM SA1COp99m16; STA a,y
    cop c_SA1COp99m16
NEWSYM SA1COp9Dm8 ; STA a,x
    cop c_SA1COp9Dm8
NEWSYM SA1COp9Dm16; STA a,x
    cop c_SA1COp9Dm16
NEWSYM SA1COp9Fm8 ; STA al,x
    cop c_SA1COp9Fm8
NEWSYM SA1COp9Fm16; STA al,x
    cop c_SA1COp9Fm16

NEWSYM SA1COp86x8 ; STX d
    cop c_SA1COp86x8
NEWSYM SA1COp86x16; STX d
    cop c_SA1COp86x16
NEWSYM SA1COp8Ex8 ; STX a
    cop c_SA1COp8Ex8
NEWSYM SA1COp8Ex16; STX a
    cop c_SA1COp8Ex16
NEWSYM SA1COp96x8 ; STX d,y
    cop c_SA1COp96x8
NEWSYM SA1COp96x16; STX d,y
    cop c_SA1COp96x16

NEWSYM SA1COp84x8 ; STY d
    cop c_SA1COp84x8
NEWSYM SA1COp84x16; STY d
    cop c_SA1COp84x16
NEWSYM SA1COp8Cx8 ; STY a
    cop c_SA1COp8Cx8
NEWSYM SA1COp8Cx16; STY a
    cop c_SA1COp8Cx16
NEWSYM SA1COp94x8 ; STYd,x
    cop c_SA1COp94x8
NEWSYM SA1COp94x16; STYd,x
    cop c_SA1COp94x16

NEWSYM SA1COpDB   ; STP i
    cop c_SA1COpDB

NEWSYM SA1COp64m8 ; STZ d
    cop c_SA1COp64m8
NEWSYM SA1COp64m16; STZ d
    cop c_SA1COp64m16
NEWSYM SA1COp74m8 ; STZ d,x
    cop c_SA1COp74m8
NEWSYM SA1COp74m16; STZ d,x
    cop c_SA1COp74m16
NEWSYM SA1COp9Cm8 ; STZ a
    cop c_SA1COp9Cm8
NEWSYM SA1COp9Cm16; STZ a
    cop c_SA1COp9Cm16
NEWSYM SA1COp9Em8 ; STZ a,x
    cop c_SA1COp9Em8
NEWSYM SA1COp9Em16; STZ a,x
    cop c_SA1COp9Em16

NEWSYM SA1COp14m8 ; TRB d
    cop c_SA1COp14m8
NEWSYM SA1COp14m16; TRB d
    cop c_SA1COp14m16
NEWSYM SA1COp1Cm8 ; TRB a
    cop c_SA1COp1Cm8
NEWSYM SA1COp1Cm16; TRB a
    cop c_SA1COp1Cm16

NEWSYM SA1COp04m8 ; TSB d
    cop c_SA1COp04m8
NEWSYM SA1COp04m16; TSB d
    cop c_SA1COp04m16
NEWSYM SA1COp0Cm8 ; TSB a
    cop c_SA1COp0Cm8
NEWSYM SA1COp0Cm16; TSB a
    cop c_SA1COp0Cm16

NEWSYM SA1COpAAx8 ; TAX i
    cop c_SA1COpAAx8
NEWSYM SA1COpAAx16; TAX i
    cop c_SA1COpAAx16
NEWSYM SA1COpA8x8 ; TAY i
    cop c_SA1COpA8x8
NEWSYM SA1COpA8x16; TAY i
    cop c_SA1COpA8x16
NEWSYM SA1COp5B   ; TCD i
    cop c_SA1COp5B
NEWSYM SA1COp1B   ; TCS i
    cop c_SA1COp1B
NEWSYM SA1COp7B   ; TDC i
    cop c_SA1COp7B
NEWSYM SA1COp3B   ; TSC i
    cop c_SA1COp3B
NEWSYM SA1COpBAx8 ; TSX i
    cop c_SA1COpBAx8
NEWSYM SA1COpBAx16; TSX i
    cop c_SA1COpBAx16
NEWSYM SA1COp8Am8 ; TSA1xa i
    cop c_SA1COp8Am8
NEWSYM SA1COp8Am16; TXA i
    cop c_SA1COp8Am16
NEWSYM SA1COp9A   ; TXS i
    cop c_SA1COp9A
NEWSYM SA1COp9Bx8 ; TSA1xy i
    cop c_SA1COp9Bx8
NEWSYM SA1COp9Bx16; TSA1xy i
    cop c_SA1COp9Bx16
NEWSYM SA1COp98m8 ; TYA i
    cop c_SA1COp98m8
NEWSYM SA1COp98m16; TYA i
    cop c_SA1COp98m16
NEWSYM SA1COpBBx8 ; TYX i
    cop c_SA1COpBBx8
NEWSYM SA1COpBBx16; TYX i
    cop c_SA1COpBBx16

NEWSYM SA1COpCB   ; WAI i
    cop c_SA1COpCB

NEWSYM SA1COpEB   ; XBA i
    cop c_SA1COpEB

NEWSYM SA1COpFB   ; XCE i
    cop c_SA1COpFB

NEWSYM SA1COp42   ; WDM
    cop c_SA1COp42


