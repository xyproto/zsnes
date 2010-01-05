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
   addr_BdCxB_8br
   ADCMacro8bnd
NEWSYM cCOp61m16nd ; ADC(d,x)
   addr_BdCxB_16br
   ADCMacro16bnd
NEWSYM cCOp61m8d   ; ADC(d,x)
   addr_BdCxB_8br
   ADCMacro8bd
NEWSYM cCOp61m16d  ; ADC(d,x)
   addr_BdCxB_16br
   ADCMacro16bd
NEWSYM cCOp63m8nd  ; ADC d,s
   addr_dCs_8br
   ADCMacro8bnd
NEWSYM cCOp63m16nd ; ADC d,s
   addr_dCs_16br
   ADCMacro16bnd
NEWSYM cCOp63m8d   ; ADC d,s
   addr_dCs_8br
   ADCMacro8bd
NEWSYM cCOp63m16d  ; ADC d,s
   addr_dCs_16br
   ADCMacro16bd
NEWSYM cCOp65m8nd  ; ADC d
   addr_d_8br
   ADCMacro8bnd
NEWSYM cCOp65m16nd ; ADC d
   addr_d_16br
   ADCMacro16bnd
NEWSYM cCOp65m8d   ; ADC d
   addr_d_8br
   ADCMacro8bd
NEWSYM cCOp65m16d  ; ADC d
   addr_d_16br
   ADCMacro16bd
NEWSYM cCOp67m8nd  ; ADC [d]
   addr_LdL_8br
   ADCMacro8bnd
NEWSYM cCOp67m16nd ; ADC [d]
   addr_LdL_16br
   ADCMacro16bnd
NEWSYM cCOp67m8d   ; ADC [d]
   addr_LdL_8br
   ADCMacro8bd
NEWSYM cCOp67m16d  ; ADC [d]
   addr_LdL_16br
   ADCMacro16bd
NEWSYM cCOp69m8nd  ; ADC #
   addr_I_8br
   ADCMacro8bnd
NEWSYM cCOp69m16nd ; ADC #
   addr_I_16br
   ADCMacro16bnd
NEWSYM cCOp69m8d   ; ADC #
   addr_I_8br
   ADCMacro8bd
NEWSYM cCOp69m16d  ; ADC #
   addr_I_16br
   ADCMacro16bd
NEWSYM cCOp6Dm8nd  ; ADC a
   addr_a_8br
   ADCMacro8bnd
NEWSYM cCOp6Dm16nd ; ADC a
   addr_a_16br
   ADCMacro16bnd
NEWSYM cCOp6Dm8d   ; ADC a
   addr_a_8br
   ADCMacro8bd
NEWSYM cCOp6Dm16d  ; ADC a
   addr_a_16br
   ADCMacro16bd
NEWSYM cCOp6Fm8nd  ; ADC al
   addr_al_8br
   ADCMacro8bnd
NEWSYM cCOp6Fm16nd ; ADC al
   addr_al_16br
   ADCMacro16bnd
NEWSYM cCOp6Fm8d   ; ADC al
   addr_al_8br
   ADCMacro8bd
NEWSYM cCOp6Fm16d  ; ADC al
   addr_al_16br
   ADCMacro16bd
NEWSYM cCOp71m8nd  ; ADC(d),y
   addr_BdBCy_8br
   ADCMacro8bnd
NEWSYM cCOp71m16nd ; ADC(d),y
   addr_BdBCy_16br
   ADCMacro16bnd
NEWSYM cCOp71m8d   ; ADC(d),y
   addr_BdBCy_8br
   ADCMacro8bd
NEWSYM cCOp71m16d  ; ADC(d),y
   addr_BdBCy_16br
   ADCMacro16bd
NEWSYM cCOp72m8nd  ; ADC (d)
   addr_BdB_8br
   ADCMacro8bnd
NEWSYM cCOp72m16nd ; ADC (d)
   addr_BdB_16br
   ADCMacro16bnd
NEWSYM cCOp72m8d   ; ADC (d)
   addr_BdB_8br
   ADCMacro8bd
NEWSYM cCOp72m16d  ; ADC (d)
   addr_BdB_16br
   ADCMacro16bd
NEWSYM cCOp73m8nd  ; ADC(d,s),y
   addr_BdCsBCy_8br
   ADCMacro8bnd
NEWSYM cCOp73m16nd ; ADC(d,s),y
   addr_BdCsBCy_16br
   ADCMacro16bnd
NEWSYM cCOp73m8d   ; ADC(d,s),y
   addr_BdCsBCy_8br
   ADCMacro8bd
NEWSYM cCOp73m16d  ; ADC(d,s),y
   addr_BdCsBCy_16br
   ADCMacro16bd
NEWSYM cCOp75m8nd  ; ADC d,x
   addr_dCx_8br
   ADCMacro8bnd
NEWSYM cCOp75m16nd ; ADC d,x
   addr_dCx_16br
   ADCMacro16bnd
NEWSYM cCOp75m8d   ; ADC d,x
   addr_dCx_8br
   ADCMacro8bd
NEWSYM cCOp75m16d  ; ADC d,x
   addr_dCx_16br
   ADCMacro16bd
NEWSYM cCOp77m8nd  ; ADC [d],y
   addr_LdLCy_8br
   ADCMacro8bnd
NEWSYM cCOp77m16nd ; ADC [d],y
   addr_LdLCy_16br
   ADCMacro16bnd
NEWSYM cCOp77m8d   ; ADC [d],y
   addr_LdLCy_8br
   ADCMacro8bd
NEWSYM cCOp77m16d  ; ADC [d],y
   addr_LdLCy_16br
   ADCMacro16bd
NEWSYM cCOp79m8nd  ; ADC a,y
   addr_aCy_8br
   ADCMacro8bnd
NEWSYM cCOp79m16nd ; ADC a,y
   addr_aCy_16br
   ADCMacro16bnd
NEWSYM cCOp79m8d   ; ADC a,y
   addr_aCy_8br
   ADCMacro8bd
NEWSYM cCOp79m16d  ; ADC a,y
   addr_aCy_16br
   ADCMacro16bd
NEWSYM cCOp7Dm8nd  ; ADC a,x
   addr_aCx_8br
   ADCMacro8bnd
NEWSYM cCOp7Dm16nd ; ADC a,x
   addr_aCx_16br
   ADCMacro16bnd
NEWSYM cCOp7Dm8d   ; ADC a,x
   addr_aCx_8br
   ADCMacro8bd
NEWSYM cCOp7Dm16d  ; ADC a,x
   addr_aCx_16br
   ADCMacro16bd
NEWSYM cCOp7Fm8nd  ; ADC al,x
   addr_alCx_8br
   ADCMacro8bnd
NEWSYM cCOp7Fm16nd ; ADC al,x
   addr_alCx_16br
   ADCMacro16bnd
NEWSYM cCOp7Fm8d   ; ADC al,x
   addr_alCx_8br
   ADCMacro8bd
NEWSYM cCOp7Fm16d  ; ADC al,x
   addr_alCx_16br
   ADCMacro16bd

NEWSYM cCOp21m8 ; AND(d,x)
   addr_BdCxB_8br
   ANDMacro8b
NEWSYM cCOp21m16; AND(d,x)
   addr_BdCxB_16br
   ANDMacro16b
NEWSYM cCOp23m8 ; AND d,s
   addr_dCs_8br
   ANDMacro8b
NEWSYM cCOp23m16; AND d,s
   addr_dCs_16br
   ANDMacro16b
NEWSYM cCOp25m8 ; AND d
   addr_d_8br
   ANDMacro8b
NEWSYM cCOp25m16; AND d
   addr_d_16br
   ANDMacro16b
NEWSYM cCOp27m8 ; AND [d]
   addr_LdL_8br
   ANDMacro8b
NEWSYM cCOp27m16; AND [d]
   addr_LdL_16br
   ANDMacro16b
NEWSYM cCOp29m8 ; AND #
   addr_I_8br
   ANDMacro8b
NEWSYM cCOp29m16; AND #
   addr_I_16br
   ANDMacro16b
NEWSYM cCOp2Dm8 ; AND a
   addr_a_8br
   ANDMacro8b
NEWSYM cCOp2Dm16; AND a
   addr_a_16br
   ANDMacro16b
NEWSYM cCOp2Fm8 ; AND al
   addr_al_8br
   ANDMacro8b
NEWSYM cCOp2Fm16; AND al
   addr_al_16br
   ANDMacro16b
NEWSYM cCOp31m8 ; AND(d),y
   addr_BdBCy_8br
   ANDMacro8b
NEWSYM cCOp31m16; AND(d),y
   addr_BdBCy_16br
   ANDMacro16b
NEWSYM cCOp32m8 ; AND (d)
   addr_BdB_8br
   ANDMacro8b
NEWSYM cCOp32m16; AND (d)
   addr_BdB_16br
   ANDMacro16b
NEWSYM cCOp33m8 ; AND(d,s),y
   addr_BdCsBCy_8br
   ANDMacro8b
NEWSYM cCOp33m16; AND(d,s),y
   addr_BdCsBCy_16br
   ANDMacro16b
NEWSYM cCOp35m8 ; AND d,x
   addr_dCx_8br
   ANDMacro8b
NEWSYM cCOp35m16; AND d,x
   addr_dCx_16br
   ANDMacro16b
NEWSYM cCOp37m8 ; AND [d],y
   addr_LdLCy_8br
   ANDMacro8b
NEWSYM cCOp37m16; AND [d],y
   addr_LdLCy_16br
   ANDMacro16b
NEWSYM cCOp39m8 ; AND a,y
   addr_aCy_8br
   ANDMacro8b
NEWSYM cCOp39m16; AND a,y
   addr_aCy_16br
   ANDMacro16b
NEWSYM cCOp3Dm8 ; AND a,x
   addr_aCx_8br
   ANDMacro8b
NEWSYM cCOp3Dm16; AND a,x
   addr_aCx_16br
   ANDMacro16b
NEWSYM cCOp3Fm8 ; AND al,x
   addr_alCx_8br
   ANDMacro8b
NEWSYM cCOp3Fm16; AND al,x
   addr_alCx_16br
   ANDMacro16b

NEWSYM cCOp06m8 ; ASL d
   addr_d_8brni
   ASLMacro8b
   addr_d_8bw
   endloop
NEWSYM cCOp06m16; ASL d
   addr_d_16brni
   ASLMacro16b
   addr_d_16bw
   endloop
NEWSYM cCOp0Am8 ; ASL A
   addr_A_8brni
   ASLMacro8b
   addr_A_8bw
   endloop
NEWSYM cCOp0Am16; ASL A
   addr_A_16brni
   ASLMacro16b
   addr_A_16bw
   endloop
NEWSYM cCOp0Em8 ; ASL a
   addr_a_8brni
   ASLMacro8b
   addr_a_8bw
   endloop
NEWSYM cCOp0Em16; ASL a
   addr_a_16brni
   ASLMacro16b
   addr_a_16bw
   endloop
NEWSYM cCOp16m8 ; ASL d,x
   addr_dCx_8brni
   ASLMacro8b
   addr_dCx_8bw
   endloop
NEWSYM cCOp16m16; ASL d,x
   addr_dCx_16brni
   ASLMacro16b
   addr_dCx_16bw
   endloop
NEWSYM cCOp1Em8 ; ASL a,x
   addr_aCx_8brni
   ASLMacro8b
   addr_aCx_8bw
   endloop
NEWSYM cCOp1Em16; ASL a,x
   addr_aCx_16brni
   ASLMacro16b
   addr_aCx_16bw
   endloop

NEWSYM cCOp90   ; BCC r
    test byte[flagc],01h
    jnz .skip
    JumpMacro
NEWSYM cCOpB0   ; BCS r
    test byte[flagc],01h
    jz .skip
    JumpMacro
NEWSYM cCOpF0   ; BEQ r
    test dword[flagnz],0FFFFh
    jnz .skip
    JumpMacro
NEWSYM cCOp30   ; BMI r
    test dword[flagnz],018000h
    jz .skip
    JumpMacro
NEWSYM cCOpD0   ; BNE r
    test dword[flagnz],0FFFFh
    jz .skip
    JumpMacro
NEWSYM cCOp10   ; BPL r
    test dword[flagnz],018000h
    jnz .skip
    JumpMacro
NEWSYM cCOp80   ; BRA r
    movsx eax,byte[esi]
    inc esi
    add esi,eax
    endloop
NEWSYM cCOp50   ; BVC r
    test dword[flago],0FFh
    jnz .skip
    JumpMacro
NEWSYM cCOp70   ; BVS r
    test dword[flago],0FFh
    jz .skip
    JumpMacro

;  BIT instruction does not affect N and V flags when using immediate
;  addressing mode. When using other addressing modes, the N and V flags

NEWSYM cCOp89m8 ; BIT #
   BITMacroim8b
NEWSYM cCOp89m16; BIT #
   BITMacroim16b

NEWSYM cCOp24m8 ; BIT d
   addr_d_8br
   BITMacro8b
NEWSYM cCOp24m16; BIT d
   addr_d_16br
   BITMacro16b
NEWSYM cCOp2Cm8 ; BIT a
   addr_a_8br
   BITMacro8b
NEWSYM cCOp2Cm16; BIT a
   addr_a_16br
   BITMacro16b
NEWSYM cCOp34m8 ; BIT d,x
   addr_dCx_8br
   BITMacro8b
NEWSYM cCOp34m16; BIT d,x
   addr_dCx_16br
   BITMacro16b
NEWSYM cCOp3Cm8 ; BIT a,x
   addr_aCx_8br
   BITMacro8b
NEWSYM cCOp3Cm16; BIT a,x
   addr_aCx_16br
   BITMacro16b

NEWSYM cCOp00   ; BRK s
   BRKMacro

NEWSYM cCOp82   ; BRL rl
   BRLMacro

NEWSYM cCOp18   ; CLC i
    mov dword[flagc],0
    endloop
NEWSYM cCOpD8   ; CLD i
    and dl,0F7h
    mov bl,dl
    mov edi,[tableadc+ebx*4]
    endloop
NEWSYM cCOp58   ; CLI i
    and dl,0FBh
    endloop
NEWSYM cCOpB8   ; CLV i
    mov dword[flago],0
    endloop

NEWSYM cCOpC1m8 ; CMP(d,x)
   addr_BdCxB_8br
   CMPMacro8b
NEWSYM cCOpC1m16; CMP(d,x)
   addr_BdCxB_16br
   CMPMacro16b
NEWSYM cCOpC3m8 ; CMP d,s
   addr_dCs_8br
   CMPMacro8b
NEWSYM cCOpC3m16; CMP d,s
   addr_dCs_16br
   CMPMacro16b
NEWSYM cCOpC5m8 ; CMP d
   addr_d_8br
   CMPMacro8b
NEWSYM cCOpC5m16; CMP d
   addr_d_16br
   CMPMacro16b
NEWSYM cCOpC7m8 ; CMP [d]
   addr_LdL_8br
   CMPMacro8b
NEWSYM cCOpC7m16; CMP [d]
   addr_LdL_16br
   CMPMacro16b
NEWSYM cCOpC9m8 ; CMP #
   addr_I_8br
   CMPMacro8b
NEWSYM cCOpC9m16; CMP #
   addr_I_16br
   CMPMacro16b
NEWSYM cCOpCDm8 ; CMP a
   addr_a_8br
   CMPMacro8b
NEWSYM cCOpCDm16; CMP a
   addr_a_16br
   CMPMacro16b
NEWSYM cCOpCFm8 ; CMP al
   addr_al_8br
   CMPMacro8b
NEWSYM cCOpCFm16; CMP al
   addr_al_16br
   CMPMacro16b
NEWSYM cCOpD1m8 ; CMP(d),y
   addr_BdBCy_8br
   CMPMacro8b
NEWSYM cCOpD1m16; CMP(d),y
   addr_BdBCy_16br
   CMPMacro16b
NEWSYM cCOpD2m8 ; CMP (d)
   addr_BdB_8br
   CMPMacro8b
NEWSYM cCOpD2m16; CMP (d)
   addr_BdB_16br
   CMPMacro16b
NEWSYM cCOpD3m8 ; CMP(d,s),y
   addr_BdCsBCy_8br
   CMPMacro8b
NEWSYM cCOpD3m16; CMP(d,s),y
   addr_BdCsBCy_16br
   CMPMacro16b
NEWSYM cCOpD5m8 ; CMP d,x
   addr_dCx_8br
   CMPMacro8b
NEWSYM cCOpD5m16; CMP d,x
   addr_dCx_16br
   CMPMacro16b
NEWSYM cCOpD7m8 ; CMP [d],y
   addr_LdLCy_8br
   CMPMacro8b
NEWSYM cCOpD7m16; CMP [d],y
   addr_LdLCy_16br
   CMPMacro16b
NEWSYM cCOpD9m8 ; CMP a,y
   addr_aCy_8br
   CMPMacro8b
NEWSYM cCOpD9m16; CMP a,y
   addr_aCy_16br
   CMPMacro16b
NEWSYM cCOpDDm8 ; CMP a,x
   addr_aCx_8br
   CMPMacro8b
NEWSYM cCOpDDm16; CMP a,x
   addr_aCx_16br
   CMPMacro16b
NEWSYM cCOpDFm8 ; CMP al,x
   addr_alCx_8br
   CMPMacro8b
NEWSYM cCOpDFm16; CMP al,x
   addr_alCx_16br
   CMPMacro16b

NEWSYM cCOp02   ; COP s
   COPMacro

NEWSYM cCOpE0x8 ; CPX #
   addr_I_8br
   CPXMacro8b
NEWSYM cCOpE0x16; CPX #
   addr_I_16br
   CPXMacro16b
NEWSYM cCOpE4x8 ; CPX d
   addr_d_8br
   CPXMacro8b
NEWSYM cCOpE4x16; CPX d
   addr_d_16br
   CPXMacro16b
NEWSYM cCOpECx8 ; CPX a
   addr_a_8br
   CPXMacro8b
NEWSYM cCOpECx16; CPX a
   addr_a_16br
   CPXMacro16b

NEWSYM cCOpC0x8 ; CPY #
   addr_I_8br
   CPYMacro8b
NEWSYM cCOpC0x16; CPY #
   addr_I_16br
   CPYMacro16b
NEWSYM cCOpC4x8 ; CPY d
   addr_d_8br
   CPYMacro8b
NEWSYM cCOpC4x16; CPY d
   addr_d_16br
   CPYMacro16b
NEWSYM cCOpCCx8 ; CPY a
   addr_a_8br
   CPYMacro8b
NEWSYM cCOpCCx16; CPY a
   addr_a_16br
   CPYMacro16b

NEWSYM cCOp3Am8 ; DEC A
    addr_A_8brni
    dec al
    flagsetnz8bnel
    addr_A_8bw
    endloop
NEWSYM cCOp3Am16; DEC A
    addr_A_16brni
    dec ax
    flagsetnz16bnel
    addr_A_16bw
    endloop
NEWSYM cCOpCEm8 ; DEC a
    addr_a_8brni
    dec al
    flagsetnz8bnel
    addr_a_8bw
    endloop
NEWSYM cCOpCEm16; DEC a
    addr_a_16brni
    dec ax
    flagsetnz16bnel
    addr_a_16bw
    endloop
NEWSYM cCOpC6m8 ; DEC d
    addr_d_8brni
    dec al
    flagsetnz8bnel
    addr_d_8bw
    endloop
NEWSYM cCOpC6m16; DEC d
    addr_d_16brni
    dec ax
    flagsetnz16bnel
    addr_d_16bw
    endloop
NEWSYM cCOpD6m8 ; DEC d,x
    addr_dCx_8brni
    dec al
    flagsetnz8bnel
    addr_dCx_8bw
    endloop
NEWSYM cCOpD6m16; DEC d,x
    addr_dCx_16brni
    dec ax
    flagsetnz16bnel
    addr_dCx_16bw
    endloop
NEWSYM cCOpDEm8 ; DEC a,x
    addr_aCx_8brni
    dec al
    flagsetnz8bnel
    addr_aCx_8bw
    endloop
NEWSYM cCOpDEm16; DEC a,x
    addr_aCx_16brni
    dec ax
    flagsetnz16bnel
    addr_aCx_16bw
    endloop

NEWSYM cCOpCAx8 ; DEX i
    dec byte[xx]
    mov al,[xx]
    flagsetnz8b
    endloop
NEWSYM cCOpCAx16; DEX i
    dec word[xx]
    mov ax,[xx]
    flagsetnz16b
    endloop
NEWSYM cCOp88x8 ; DEY i
    dec byte[xy]
    mov al,[xy]
    flagsetnz8b
    endloop
NEWSYM cCOp88x16; DEY i
    dec word[xy]
    mov ax,[xy]
    flagsetnz16b
    endloop

NEWSYM cCOp41m8 ; EOR(d,x)
   addr_BdCxB_8br
   EORMacro8b
NEWSYM cCOp41m16; EOR(d,x)
   addr_BdCxB_16br
   EORMacro16b
NEWSYM cCOp43m8 ; EOR d,s
   addr_dCs_8br
   EORMacro8b
NEWSYM cCOp43m16; EOR d,s
   addr_dCs_16br
   EORMacro16b
NEWSYM cCOp45m8 ; EOR d
   addr_d_8br
   EORMacro8b
NEWSYM cCOp45m16; EOR d
   addr_d_16br
   EORMacro16b
NEWSYM cCOp47m8 ; EOR [d]
   addr_LdL_8br
   EORMacro8b
NEWSYM cCOp47m16; EOR [d]
   addr_LdL_16br
   EORMacro16b
NEWSYM cCOp49m8 ; EOR #
   addr_I_8br
   EORMacro8b
NEWSYM cCOp49m16; EOR #
   addr_I_16br
   EORMacro16b
NEWSYM cCOp4Dm8 ; EOR a
   addr_a_8br
   EORMacro8b
NEWSYM cCOp4Dm16; EOR a
   addr_a_16br
   EORMacro16b
NEWSYM cCOp4Fm8 ; EOR al
   addr_al_8br
   EORMacro8b
NEWSYM cCOp4Fm16; EOR al
   addr_al_16br
   EORMacro16b
NEWSYM cCOp51m8 ; EOR(d),y
   addr_BdBCy_8br
   EORMacro8b
NEWSYM cCOp51m16; EOR(d),y
   addr_BdBCy_16br
   EORMacro16b
NEWSYM cCOp52m8 ; EOR (d)
   addr_BdB_8br
   EORMacro8b
NEWSYM cCOp52m16; EOR (d)
   addr_BdB_16br
   EORMacro16b
NEWSYM cCOp53m8 ; EOR(d,s),y
   addr_BdCsBCy_8br
   EORMacro8b
NEWSYM cCOp53m16; EOR(d,s),y
   addr_BdCsBCy_16br
   EORMacro16b
NEWSYM cCOp55m8 ; EOR d,x
   addr_dCx_8br
   EORMacro8b
NEWSYM cCOp55m16; EOR d,x
   addr_dCx_16br
   EORMacro16b
NEWSYM cCOp57m8 ; EOR [d],y
   addr_LdLCy_8br
   EORMacro8b
NEWSYM cCOp57m16; EOR [d],y
   addr_LdLCy_16br
   EORMacro16b
NEWSYM cCOp59m8 ; EOR a,y
   addr_aCy_8br
   EORMacro8b
NEWSYM cCOp59m16; EOR a,y
   addr_aCy_16br
   EORMacro16b
NEWSYM cCOp5Dm8 ; EORa,x
   addr_aCx_8br
   EORMacro8b
NEWSYM cCOp5Dm16; EORa,x
   addr_aCx_16br
   EORMacro16b
NEWSYM cCOp5Fm8 ; EOR al,x
   addr_alCx_8br
   EORMacro8b
NEWSYM cCOp5Fm16; EOR al,x
   addr_alCx_16br
   EORMacro16b


NEWSYM cCOp1Am8 ; INC A
    addr_A_8brni
    inc al
    flagsetnz8bnel
    addr_A_8bw
    endloop
NEWSYM cCOp1Am16; INC A
    addr_A_16brni
    inc ax
    flagsetnz16bnel
    addr_A_16bw
    endloop
NEWSYM cCOpEEm8 ; INC a
    addr_a_8brni
    inc al
    flagsetnz8bnel
    addr_a_8bw
    endloop
NEWSYM cCOpEEm16; INC a
    addr_a_16brni
    inc ax
    flagsetnz16bnel
    addr_a_16bw
    endloop
NEWSYM cCOpE6m8 ; INC d
    addr_d_8brni
    inc al
    flagsetnz8bnel
    addr_d_8bw
    endloop
NEWSYM cCOpE6m16; INC d
    addr_d_16brni
    inc ax
    flagsetnz16bnel
    addr_d_16bw
    endloop
NEWSYM cCOpF6m8 ; INC d,x
    addr_dCx_8brni
    inc al
    flagsetnz8bnel
    addr_dCx_8bw
    endloop
NEWSYM cCOpF6m16; INC d,x
    addr_dCx_16brni
    inc ax
    flagsetnz16bnel
    addr_dCx_16bw
    endloop
NEWSYM cCOpFEm8 ; INC a,x
    addr_aCx_8brni
    inc al
    flagsetnz8bnel
    addr_aCx_8bw
    endloop
NEWSYM cCOpFEm16; INC a,x
    addr_aCx_16brni
    inc ax
    flagsetnz16bnel
    addr_aCx_16bw
    endloop


NEWSYM cCOpE8x8 ; INX i
    inc byte[xx]
    mov al,[xx]
    flagsetnz8b
    endloop
NEWSYM cCOpE8x16; INX i
    inc word[xx]
    mov ax,[xx]
    flagsetnz16b
    endloop
NEWSYM cCOpC8x8 ; INY i
    inc byte[xy]
    mov al,[xy]
    flagsetnz8b
    endloop
NEWSYM cCOpC8x16; INY i
    inc word[xy]
    mov ax,[xy]
    flagsetnz16b
    endloop

NEWSYM cCOpDC   ; JML (a)
    JMLMacro
NEWSYM cCOp4C   ; JMP a
    JMPMacro4C
NEWSYM cCOp6C   ; JMP (a)
    JMPMacro6C
NEWSYM cCOp7C   ; JMP(a,x)
    JMPMacro7C
NEWSYM cCOp5C   ; JMP al
    JMPMacro5C

NEWSYM cCOp22   ; JSL al
   JSLMacro
NEWSYM cCOp20   ; JSR a
   JSRMacro20
NEWSYM cCOpFC   ; JSR(a,x)
   JSRMacroFC

NEWSYM cCOpA1m8 ; LDA(d,x)
   addr_BdCxB_8br
   LDAMacro8b
NEWSYM cCOpA1m16; LDA(d,x)
   addr_BdCxB_16br
   LDAMacro16b
NEWSYM cCOpA3m8 ; LDA d,s
   addr_dCs_8br
   LDAMacro8b
NEWSYM cCOpA3m16; LDA d,s
   addr_dCs_16br
   LDAMacro16b
NEWSYM cCOpA5m8 ; LDA d
   addr_d_8br
   LDAMacro8b
NEWSYM cCOpA5m16; LDA d
   addr_d_16br
   LDAMacro16b
NEWSYM cCOpA7m8 ; LDA [d]
   addr_LdL_8br
   LDAMacro8b
NEWSYM cCOpA7m16; LDA [d]
   addr_LdL_16br
   LDAMacro16b
NEWSYM cCOpA9m8 ; LDA #
   addr_I_8br
   LDAMacro8b
NEWSYM cCOpA9m16; LDA #
   addr_I_16br
   LDAMacro16b
NEWSYM cCOpADm8 ; LDA a
   addr_a_8br
   LDAMacro8b
NEWSYM cCOpADm16; LDA a
   addr_a_16br
   LDAMacro16b
NEWSYM cCOpAFm8 ; LDA al
   addr_al_8br
   LDAMacro8b
NEWSYM cCOpAFm16; LDA al
   addr_al_16br
   LDAMacro16b
NEWSYM cCOpB1m8 ; LDA(d),y
   addr_BdBCy_8br
   LDAMacro8b
NEWSYM cCOpB1m16; LDA(d),y
   addr_BdBCy_16br
   LDAMacro16b
NEWSYM cCOpB2m8 ; LDA (d)
   addr_BdB_8br
   LDAMacro8b
NEWSYM cCOpB2m16; LDA (d)
   addr_BdB_16br
   LDAMacro16b
NEWSYM cCOpB3m8 ; LDA(d,s),y
   addr_BdCsBCy_8br
   LDAMacro8b
NEWSYM cCOpB3m16; LDA(d,s),y
   addr_BdCsBCy_16br
   LDAMacro16b
NEWSYM cCOpB5m8 ; LDA d,x
   addr_dCx_8br
   LDAMacro8b
NEWSYM cCOpB5m16; LDA d,x
   addr_dCx_16br
   LDAMacro16b
NEWSYM cCOpB7m8 ; LDA [d],y
   addr_LdLCy_8br
   LDAMacro8b
NEWSYM cCOpB7m16; LDA [d],y
   addr_LdLCy_16br
   LDAMacro16b
NEWSYM cCOpB9m8 ; LDA a,y
   addr_aCy_8br
   LDAMacro8b
NEWSYM cCOpB9m16; LDA a,y
   addr_aCy_16br
   LDAMacro16b
NEWSYM cCOpBDm8 ; LDA a,x
   addr_aCx_8br
   LDAMacro8b
NEWSYM cCOpBDm16; LDA a,x
   addr_aCx_16br
   LDAMacro16b
NEWSYM cCOpBFm8 ; LDA al,x
   addr_alCx_8br
   LDAMacro8b
NEWSYM cCOpBFm16; LDA al,x
   addr_alCx_16br
   LDAMacro16b

NEWSYM cCOpA2x8 ; LDX #
   addr_I_8br
   LDXMacro8b
NEWSYM cCOpA2x16; LDX #
   addr_I_16br
   LDXMacro16b
NEWSYM cCOpA6x8 ; LDX d
   addr_d_8br
   LDXMacro8b
NEWSYM cCOpA6x16; LDX d
   addr_d_16br
   LDXMacro16b
NEWSYM cCOpAEx8 ; LDX a
   addr_a_8br
   LDXMacro8b
NEWSYM cCOpAEx16; LDX a
   addr_a_16br
   LDXMacro16b
NEWSYM cCOpB6x8 ; LDX d,y
   addr_dCy_8br
   LDXMacro8b
NEWSYM cCOpB6x16; LDX d,y
   addr_dCy_16br
   LDXMacro16b
NEWSYM cCOpBEx8 ; LDX a,y
   addr_aCy_8br
   LDXMacro8b
NEWSYM cCOpBEx16; LDX a,y
   addr_aCy_16br
   LDXMacro16b

NEWSYM cCOpA0x8 ; LDY #
   addr_I_8br
   LDYMacro8b
NEWSYM cCOpA0x16; LDY #
   addr_I_16br
   LDYMacro16b
NEWSYM cCOpA4x8 ; LDY d
   addr_d_8br
   LDYMacro8b
NEWSYM cCOpA4x16; LDY d
   addr_d_16br
   LDYMacro16b
NEWSYM cCOpACx8 ; LDY a
   addr_a_8br
   LDYMacro8b
NEWSYM cCOpACx16; LDY a
   addr_a_16br
   LDYMacro16b
NEWSYM cCOpB4x8 ; LDY d,x
   addr_dCx_8br
   LDYMacro8b
NEWSYM cCOpB4x16; LDY d,x
   addr_dCx_16br
   LDYMacro16b
NEWSYM cCOpBCx8 ; LDY a,x
   addr_aCx_8br
   LDYMacro8b
NEWSYM cCOpBCx16; LDY a,x
   addr_aCx_16br
   LDYMacro16b

NEWSYM cCOp46m8 ; LSR d
   addr_d_8brni
   LSRMacro8b
   addr_d_8bw
   endloop
NEWSYM cCOp46m16; LSR d
   addr_d_16brni
   LSRMacro16b
   addr_d_16bw
   endloop
NEWSYM cCOp4Am8 ; LSR A
   addr_A_8brni
   LSRMacro8b
   addr_A_8bw
   endloop
NEWSYM cCOp4Am16; LSR A
   addr_A_16brni
   LSRMacro16b
   addr_A_16bw
   endloop
NEWSYM cCOp4Em8 ; LSR a
   addr_a_8brni
   LSRMacro8b
   addr_a_8bw
   endloop
NEWSYM cCOp4Em16; LSR a
   addr_a_16brni
   LSRMacro16b
   addr_a_16bw
   endloop
NEWSYM cCOp56m8 ; LSR d,x
   addr_dCx_8brni
   LSRMacro8b
   addr_dCx_8bw
   endloop
NEWSYM cCOp56m16; LSR d,x
   addr_dCx_16brni
   LSRMacro16b
   addr_dCx_16bw
   endloop
NEWSYM cCOp5Em8 ; LSRa,x
   addr_aCx_8brni
   LSRMacro8b
   addr_aCx_8bw
   endloop
NEWSYM cCOp5Em16; LSRa,x
   addr_aCx_16brni
   LSRMacro16b
   addr_aCx_16bw
   endloop

NEWSYM cCOp54   ; MVN xya
   MVNMacro
NEWSYM cCOp44   ; MVP xya
   MVPMacro

NEWSYM cCOpEA   ; NOP i
   endloop

NEWSYM cCOp01m8 ; ORA(d,x)
   addr_BdCxB_8br
   ORAMacro8b
NEWSYM cCOp01m16; ORA(d,x)
   addr_BdCxB_16br
   ORAMacro16b
NEWSYM cCOp03m8 ; ORA d,s
   addr_dCs_8br
   ORAMacro8b
NEWSYM cCOp03m16; ORA d,s
   addr_dCs_16br
   ORAMacro16b
NEWSYM cCOp05m8 ; ORA d
   addr_d_8br
   ORAMacro8b
NEWSYM cCOp05m16; ORA d
   addr_d_16br
   ORAMacro16b
NEWSYM cCOp07m8 ; ORA [d]
   addr_LdL_8br
   ORAMacro8b
NEWSYM cCOp07m16; ORA [d]
   addr_LdL_16br
   ORAMacro16b
NEWSYM cCOp09m8 ; ORA #
   addr_I_8br
   ORAMacro8b
NEWSYM cCOp09m16; ORA #
   addr_I_16br
   ORAMacro16b
NEWSYM cCOp0Dm8 ; ORA a
   addr_a_8br
   ORAMacro8b
NEWSYM cCOp0Dm16; ORA a
   addr_a_16br
   ORAMacro16b
NEWSYM cCOp0Fm8 ; ORA al
   addr_al_8br
   ORAMacro8b
NEWSYM cCOp0Fm16; ORA al
   addr_al_16br
   ORAMacro16b
NEWSYM cCOp11m8 ; ORA(d),y
   addr_BdBCy_8br
   ORAMacro8b
NEWSYM cCOp11m16; ORA(d),y
   addr_BdBCy_16br
   ORAMacro16b
NEWSYM cCOp12m8 ; ORA(d)
   addr_BdB_8br
   ORAMacro8b
NEWSYM cCOp12m16; ORA(d)
   addr_BdB_16br
   ORAMacro16b
NEWSYM cCOp13m8 ; ORA(d,s),y
   addr_BdCsBCy_8br
   ORAMacro8b
NEWSYM cCOp13m16; ORA(d,s),y
   addr_BdCsBCy_16br
   ORAMacro16b
NEWSYM cCOp15m8 ; ORA d,x
   addr_dCx_8br
   ORAMacro8b
NEWSYM cCOp15m16; ORA d,x
   addr_dCx_16br
   ORAMacro16b
NEWSYM cCOp17m8 ; ORA [d],y
   addr_LdLCy_8br
   ORAMacro8b
NEWSYM cCOp17m16; ORA [d],y
   addr_LdLCy_16br
   ORAMacro16b
NEWSYM cCOp19m8 ; ORA a,y
   addr_aCy_8br
   ORAMacro8b
NEWSYM cCOp19m16; ORA a,y
   addr_aCy_16br
   ORAMacro16b
NEWSYM cCOp1Dm8 ; ORA a,x
   addr_aCx_8br
   ORAMacro8b
NEWSYM cCOp1Dm16; ORA a,x
   addr_aCx_16br
   ORAMacro16b
NEWSYM cCOp1Fm8 ; ORA al,x
   addr_alCx_8br
   ORAMacro8b
NEWSYM cCOp1Fm16; ORA al,x
   addr_alCx_16br
   ORAMacro16b

NEWSYM cCOpF4   ; PEA s
   PEAMacro
NEWSYM cCOpD4   ; PEI s
   PEIMacro
NEWSYM cCOp62   ; PER s
   PERMacro

NEWSYM cCOp48m8 ; PHA s
   PUSHMacro8b [xa]
NEWSYM cCOp48m16; PHA s
   PUSHMacro16b [xa], [xa+1]
NEWSYM cCOp8B   ; PHB s
   PUSHMacro8b [xdb]
NEWSYM cCOp0B   ; PHD s
   PUSHMacro16b [xd], [xd+1]
NEWSYM cCOp4B   ; PHK s
   PUSHMacro8b [xpb]
NEWSYM cCOp08   ; PHP s
   PUSHMacro8bp
NEWSYM cCOpDAx8 ; PHX s
   PUSHMacro8b [xx]
NEWSYM cCOpDAx16; PHX s
   PUSHMacro16b [xx], [xx+1]
NEWSYM cCOp5Ax8 ; PHY s
   PUSHMacro8b [xy]
NEWSYM cCOp5Ax16; PHY s
   PUSHMacro16b [xy], [xy+1]

NEWSYM cCOp68m8 ; PLA s
   POPMacro8b [xa]
NEWSYM cCOp68m16; PLA s
   POPMacro16b [xa], [xa+1]
NEWSYM cCOpAB   ; PLB s
   POPMacro8b [xdb]
NEWSYM cCOp2B   ; PLD s
   POPMacro16bd [xd], [xd+1]
NEWSYM cCOp28   ; PLP s
   POPMacroP
NEWSYM cCOpFAx8 ; PLX s
   POPMacro8b [xx]
NEWSYM cCOpFAx16; PLX s
   POPMacro16b [xx], [xx+1]
NEWSYM cCOp7Ax8 ; PLY s
   POPMacro8b [xy]
NEWSYM cCOp7Ax16; PLY s
   POPMacro16b [xy], [xy+1]

NEWSYM cCOpC2   ; REP #
   REPMacro

NEWSYM cCOp26m8 ; ROL d
   addr_d_8brni
   ROLMacro8b
   addr_d_8bw
   endloop
NEWSYM cCOp26m16; ROL d
   addr_d_16brni
   ROLMacro16b
   addr_d_16bw
   endloop
NEWSYM cCOp2Am8 ; ROL A
   addr_A_8brni
   ROLMacro8b
   addr_A_8bw
   endloop
NEWSYM cCOp2Am16; ROL A
   addr_A_16brni
   ROLMacro16b
   addr_A_16bw
   endloop
NEWSYM cCOp2Em8 ; ROL a
   addr_a_8brni
   ROLMacro8b
   addr_a_8bw
   endloop
NEWSYM cCOp2Em16; ROL a
   addr_a_16brni
   ROLMacro16b
   addr_a_16bw
   endloop
NEWSYM cCOp36m8 ; ROL d,x
   addr_dCx_8brni
   ROLMacro8b
   addr_dCx_8bw
   endloop
NEWSYM cCOp36m16; ROL d,x
   addr_dCx_16brni
   ROLMacro16b
   addr_dCx_16bw
   endloop
NEWSYM cCOp3Em8 ; ROL a,x
   addr_aCx_8brni
   ROLMacro8b
   addr_aCx_8bw
   endloop
NEWSYM cCOp3Em16; ROL a,x
   addr_aCx_16brni
   ROLMacro16b
   addr_aCx_16bw
   endloop

NEWSYM cCOp66m8 ; ROR d
   addr_d_8brni
   RORMacro8b
   addr_d_8bw
   endloop
NEWSYM cCOp66m16; ROR d
   addr_d_16brni
   RORMacro16b
   addr_d_16bw
   endloop
NEWSYM cCOp6Am8 ; ROR A
   addr_A_8brni
   RORMacro8b
   addr_A_8bw
   endloop
NEWSYM cCOp6Am16; ROR A
   addr_A_16brni
   RORMacro16b
   addr_A_16bw
   endloop
NEWSYM cCOp6Em8 ; ROR a
   addr_a_8brni
   RORMacro8b
   addr_a_8bw
   endloop
NEWSYM cCOp6Em16; ROR a
   addr_a_16brni
   RORMacro16b
   addr_a_16bw
   endloop
NEWSYM cCOp76m8 ; ROR d,x
   addr_dCx_8brni
   RORMacro8b
   addr_dCx_8bw
   endloop
NEWSYM cCOp76m16; ROR d,x
   addr_dCx_16brni
   RORMacro16b
   addr_dCx_16bw
   endloop
NEWSYM cCOp7Em8 ; ROR a,x
   addr_aCx_8brni
   RORMacro8b
   addr_aCx_8bw
   endloop
NEWSYM cCOp7Em16; ROR a,x
   addr_aCx_16brni
   RORMacro16b
   addr_aCx_16bw
   endloop

NEWSYM cCOp40   ; RTI s
   RTIMacro
NEWSYM cCOp6B   ; RTL s
   RTLMacro
NEWSYM cCOp60   ; RTS s
   RTSMacro

NEWSYM cCOpE1m8nd  ; SBC(d,x)
   addr_BdCxB_8br
   SBCMacro8bnd
NEWSYM cCOpE1m16nd ; SBC(d,x)
   addr_BdCxB_16br
   SBCMacro16bnd
NEWSYM cCOpE1m8d   ; SBC(d,x)
   addr_BdCxB_8br
   SBCMacro8bd
NEWSYM cCOpE1m16d  ; SBC(d,x)
   addr_BdCxB_16br
   SBCMacro16bd
NEWSYM cCOpE3m8nd  ; SBC d,s
   addr_dCs_8br
   SBCMacro8bnd
NEWSYM cCOpE3m16nd ; SBC d,s
   addr_dCs_16br
   SBCMacro16bnd
NEWSYM cCOpE3m8d   ; SBC d,s
   addr_dCs_8br
   SBCMacro8bd
NEWSYM cCOpE3m16d  ; SBC d,s
   addr_dCs_16br
   SBCMacro16bd
NEWSYM cCOpE5m8nd  ; SBC d
   addr_d_8br
   SBCMacro8bnd
NEWSYM cCOpE5m16nd ; SBC d
   addr_d_16br
   SBCMacro16bnd
NEWSYM cCOpE5m8d   ; SBC d
   addr_d_8br
   SBCMacro8bd
NEWSYM cCOpE5m16d  ; SBC d
   addr_d_16br
   SBCMacro16bd
NEWSYM cCOpE7m8nd  ; SBC [d]
   addr_LdL_8br
   SBCMacro8bnd
NEWSYM cCOpE7m16nd ; SBC [d]
   addr_LdL_16br
   SBCMacro16bnd
NEWSYM cCOpE7m8d   ; SBC [d]
   addr_LdL_8br
   SBCMacro8bd
NEWSYM cCOpE7m16d  ; SBC [d]
   addr_LdL_16br
   SBCMacro16bd
NEWSYM cCOpE9m8nd  ; SBC #
   addr_I_8br
   SBCMacro8bnd
NEWSYM cCOpE9m16nd ; SBC #
   addr_I_16br
   SBCMacro16bnd
NEWSYM cCOpE9m8d   ; SBC #
   addr_I_8br
   SBCMacro8bd
NEWSYM cCOpE9m16d  ; SBC #
   addr_I_16br
   SBCMacro16bd
NEWSYM cCOpEDm8nd  ; SBC a
   addr_a_8br
   SBCMacro8bnd
NEWSYM cCOpEDm16nd ; SBC a
   addr_a_16br
   SBCMacro16bnd
NEWSYM cCOpEDm8d   ; SBC a
   addr_a_8br
   SBCMacro8bd
NEWSYM cCOpEDm16d  ; SBC a
   addr_a_16br
   SBCMacro16bd
NEWSYM cCOpEFm8nd  ; SBC al
   addr_al_8br
   SBCMacro8bnd
NEWSYM cCOpEFm16nd ; SBC al
   addr_al_16br
   SBCMacro16bnd
NEWSYM cCOpEFm8d   ; SBC al
   addr_al_8br
   SBCMacro8bd
NEWSYM cCOpEFm16d  ; SBC al
   addr_al_16br
   SBCMacro16bd
NEWSYM cCOpF1m8nd  ; SBC(d),y
   addr_BdBCy_8br
   SBCMacro8bnd
NEWSYM cCOpF1m16nd ; SBC(d),y
   addr_BdBCy_16br
   SBCMacro16bnd
NEWSYM cCOpF1m8d   ; SBC(d),y
   addr_BdBCy_8br
   SBCMacro8bd
NEWSYM cCOpF1m16d  ; SBC(d),y
   addr_BdBCy_16br
   SBCMacro16bd
NEWSYM cCOpF2m8nd  ; SBC (d)
   addr_BdB_8br
   SBCMacro8bnd
NEWSYM cCOpF2m16nd ; SBC (d)
   addr_BdB_16br
   SBCMacro16bnd
NEWSYM cCOpF2m8d   ; SBC (d)
   addr_BdB_8br
   SBCMacro8bd
NEWSYM cCOpF2m16d  ; SBC (d)
   addr_BdB_16br
   SBCMacro16bd
NEWSYM cCOpF3m8nd  ; SBC(d,s),y
   addr_BdCsBCy_8br
   SBCMacro8bnd
NEWSYM cCOpF3m16nd ; SBC(d,s),y
   addr_BdCsBCy_16br
   SBCMacro16bnd
NEWSYM cCOpF3m8d   ; SBC(d,s),y
   addr_BdCsBCy_8br
   SBCMacro8bd
NEWSYM cCOpF3m16d  ; SBC(d,s),y
   addr_BdCsBCy_16br
   SBCMacro16bd
NEWSYM cCOpF5m8nd  ; SBC d,x
   addr_dCx_8br
   SBCMacro8bnd
NEWSYM cCOpF5m16nd ; SBC d,x
   addr_dCx_16br
   SBCMacro16bnd
NEWSYM cCOpF5m8d   ; SBC d,x
   addr_dCx_8br
   SBCMacro8bd
NEWSYM cCOpF5m16d  ; SBC d,x
   addr_dCx_16br
   SBCMacro16bd
NEWSYM cCOpF7m8nd  ; SBC [d],y
   addr_LdLCy_8br
   SBCMacro8bnd
NEWSYM cCOpF7m16nd ; SBC [d],y
   addr_LdLCy_16br
   SBCMacro16bnd
NEWSYM cCOpF7m8d   ; SBC [d],y
   addr_LdLCy_8br
   SBCMacro8bd
NEWSYM cCOpF7m16d  ; SBC [d],y
   addr_LdLCy_16br
   SBCMacro16bd
NEWSYM cCOpF9m8nd  ; SBC a,y
   addr_aCy_8br
   SBCMacro8bnd
NEWSYM cCOpF9m16nd ; SBC a,y
   addr_aCy_16br
   SBCMacro16bnd
NEWSYM cCOpF9m8d   ; SBC a,y
   addr_aCy_8br
   SBCMacro8bd
NEWSYM cCOpF9m16d  ; SBC a,y
   addr_aCy_16br
   SBCMacro16bd
NEWSYM cCOpFDm8nd  ; SBC a,x
   addr_aCx_8br
   SBCMacro8bnd
NEWSYM cCOpFDm16nd ; SBC a,x
   addr_aCx_16br
   SBCMacro16bnd
NEWSYM cCOpFDm8d   ; SBC a,x
   addr_aCx_8br
   SBCMacro8bd
NEWSYM cCOpFDm16d  ; SBC a,x
   addr_aCx_16br
   SBCMacro16bd
NEWSYM cCOpFFm8nd  ; SBC al,x
   addr_alCx_8br
   SBCMacro8bnd
NEWSYM cCOpFFm16nd ; SBC al,x
   addr_alCx_16br
   SBCMacro16bnd
NEWSYM cCOpFFm8d   ; SBC al,x
   addr_alCx_8br
   SBCMacro8bd
NEWSYM cCOpFFm16d  ; SBC al,x
   addr_alCx_16br
   SBCMacro16bd

NEWSYM cCOp38   ; SEC i
    mov dword[flagc],0FFh
    endloop
NEWSYM cCOpF8   ; SED i
    or dl,08h
    mov bl,dl
    mov edi,[tableadc+ebx*4]
    endloop
NEWSYM cCOp78   ; SEI i
    or dl,04h
    endloop

NEWSYM cCOpE2   ; SEP #
    SEPMacro

NEWSYM cCOp81m8 ; STA(d,x)
   STAMacro8b
   addr_BdCxB_8bw
   endloop
NEWSYM cCOp81m16; STA(d,x)
   STAMacro16b
   addr_BdCxB_16bw
   endloop
NEWSYM cCOp83m8 ; STA d,s
   STAMacro8b
   addr_dCs_8bw
   endloop
NEWSYM cCOp83m16; STA d,s
   STAMacro16b
   addr_dCs_16bw
   endloop
NEWSYM cCOp85m8 ; STA d
   STAMacro8b
   addr_d_8bw
   endloop
NEWSYM cCOp85m16; STA d
   STAMacro16b
   addr_d_16bw
   endloop
NEWSYM cCOp87m8 ; STA [d]
   STAMacro8b
   addr_LdL_8bw
   endloop
NEWSYM cCOp87m16; STA [d]
   STAMacro16b
   addr_LdL_16bw
   endloop
NEWSYM cCOp8Dm8 ; STA a
   STAMacro8b
   addr_a_8bw
   endloop
NEWSYM cCOp8Dm16; STA a
   STAMacro16b
   addr_a_16bw
   endloop
NEWSYM cCOp8Fm8 ; STA al
   STAMacro8b
   addr_al_8bw
   endloop
NEWSYM cCOp8Fm16; STA al
   STAMacro16b
   addr_al_16bw
   endloop
NEWSYM cCOp91m8 ; STA(d),y
   STAMacro8b
   addr_BdBCy_8bw
   endloop
NEWSYM cCOp91m16; STA(d),y
   STAMacro16b
   addr_BdBCy_16bw
   endloop
NEWSYM cCOp92m8 ; STA (d)
   STAMacro8b
   addr_BdB_8bw
   endloop
NEWSYM cCOp92m16; STA (d)
   STAMacro16b
   addr_BdB_16bw
   endloop
NEWSYM cCOp93m8 ; STA(d,s),y
   STAMacro8b
   addr_BdCsBCy_8bw
   endloop
NEWSYM cCOp93m16; STA(d,s),y
   STAMacro16b
   addr_BdCsBCy_16bw
   endloop
NEWSYM cCOp95m8 ; STA d,x
   STAMacro8b
   addr_dCx_8bw
   endloop
NEWSYM cCOp95m16; STA d,x
   STAMacro16b
   addr_dCx_16bw
   endloop
NEWSYM cCOp97m8 ; STA [d],y
   STAMacro8b
   addr_LdLCy_8bw
   endloop
NEWSYM cCOp97m16; STA [d],y
   STAMacro16b
   addr_LdLCy_16bw
   endloop
NEWSYM cCOp99m8 ; STA a,y
   STAMacro8b
   addr_aCy_8bw
   endloop
NEWSYM cCOp99m16; STA a,y
   STAMacro16b
   addr_aCy_16bw
   endloop
NEWSYM cCOp9Dm8 ; STA a,x
   STAMacro8b
   addr_aCx_8bw
   endloop
NEWSYM cCOp9Dm16; STA a,x
   STAMacro16b
   addr_aCx_16bw
   endloop
NEWSYM cCOp9Fm8 ; STA al,x
   STAMacro8b
   addr_alCx_8bw
   endloop
NEWSYM cCOp9Fm16; STA al,x
   STAMacro16b
   addr_alCx_16bw
   endloop

NEWSYM cCOp86x8 ; STX d
   STXMacro8b
   addr_d_8bw
   endloop
NEWSYM cCOp86x16; STX d
   STXMacro16b
   addr_d_16bw
   endloop
NEWSYM cCOp8Ex8 ; STX a
   STXMacro8b
   addr_a_8bw
   endloop
NEWSYM cCOp8Ex16; STX a
   STXMacro16b
   addr_a_16bw
   endloop
NEWSYM cCOp96x8 ; STX d,y
   STXMacro8b
   addr_dCy_8bw
   endloop
NEWSYM cCOp96x16; STX d,y
   STXMacro16b
   addr_dCy_16bw
   endloop

NEWSYM cCOp84x8 ; STY d
   STYMacro8b
   addr_d_8bw
   endloop
NEWSYM cCOp84x16; STY d
   STYMacro16b
   addr_d_16bw
   endloop
NEWSYM cCOp8Cx8 ; STY a
   STYMacro8b
   addr_a_8bw
   endloop
NEWSYM cCOp8Cx16; STY a
   STYMacro16b
   addr_a_16bw
   endloop
NEWSYM cCOp94x8 ; STYd,x
   STYMacro8b
   addr_dCx_8bw
   endloop
NEWSYM cCOp94x16; STYd,x
   STYMacro16b
   addr_dCx_16bw
   endloop

NEWSYM cCOpDB   ; STP i
    dec esi
    endloop

NEWSYM cCOp64m8 ; STZ d
   xor al,al
   addr_d_8bw
   endloop
NEWSYM cCOp64m16; STZ d
   xor eax,eax
   addr_d_16bw
   endloop
NEWSYM cCOp74m8 ; STZ d,x
   xor al,al
   addr_dCx_8bw
   endloop
NEWSYM cCOp74m16; STZ d,x
   xor eax,eax
   addr_dCx_16bw
   endloop
NEWSYM cCOp9Cm8 ; STZ a
   xor al,al
   addr_a_8bw
   endloop
NEWSYM cCOp9Cm16; STZ a
   xor eax,eax
   addr_a_16bw
   endloop
NEWSYM cCOp9Em8 ; STZ a,x
   xor al,al
   addr_aCx_8bw
   endloop
NEWSYM cCOp9Em16; STZ a,x
   xor eax,eax
   addr_aCx_16bw
   endloop

NEWSYM cCOp14m8 ; TRB d
    addr_d_8brni
    mov cl,[xa]
    test dword[flagnz],18000h
    jz .notflags
    or dword[flagnz],10000h
.notflags
    test cl,al
    jz .zero
    mov word[flagnz],1
    not cl
    and al,cl
    addr_d_8bw
    endloop
.zero
    mov word[flagnz],0
    not cl
    and al,cl
    addr_d_8bw
    endloop
NEWSYM cCOp14m16; TRB d
    addr_d_16brni
    mov cx,[xa]
    test dword[flagnz],18000h
    jz .notflags
    or dword[flagnz],10000h
.notflags
    test cx,ax
    jz .zero
    mov word[flagnz],1
    not cx
    and ax,cx
    addr_d_16bw
    endloop
.zero
    mov word[flagnz],0
    not cx
    and ax,cx
    addr_d_16bw
    endloop
NEWSYM cCOp1Cm8 ; TRB a
    addr_a_8brni
    mov cl,[xa]
    test dword[flagnz],18000h
    jz .notflags
    or dword[flagnz],10000h
.notflags
    test cl,al
    jz .zero
    mov word[flagnz],1
    not cl
    and al,cl
    addr_a_8bw
    endloop
.zero
    mov word[flagnz],0
    not cl
    and al,cl
    addr_a_8bw
    endloop
NEWSYM cCOp1Cm16; TRB a
    addr_a_16brni
    mov cx,[xa]
    test dword[flagnz],18000h
    jz .notflags
    or dword[flagnz],10000h
.notflags
    test cx,ax
    jz .zero
    mov word[flagnz],1
    not cx
    and ax,cx
    addr_a_16bw
    endloop
.zero
    mov word[flagnz],0
    not cx
    and ax,cx
    addr_a_16bw
    endloop

NEWSYM cCOp04m8 ; TSB d
    addr_d_8brni
    mov cl,[xa]
    test dword[flagnz],18000h
    jz .notflags
    or dword[flagnz],10000h
.notflags
    test al,cl
    jz .zero
    mov word[flagnz],1
    or al,cl
    addr_d_8bw
    endloop
.zero
    mov word[flagnz],0
    or al,cl
    addr_d_8bw
    endloop
NEWSYM cCOp04m16; TSB d
    addr_d_16brni
    mov cx,[xa]
    test dword[flagnz],18000h
    jz .notflags
    or dword[flagnz],10000h
.notflags
    test ax,cx
    jz .zero
    mov word[flagnz],1
    or ax,cx
    addr_d_16bw
    endloop
.zero
    mov word[flagnz],0
    or ax,cx
    addr_d_16bw
    endloop
NEWSYM cCOp0Cm8 ; TSB a
    addr_a_8brni
    mov cl,[xa]
    test dword[flagnz],18000h
    jz .notflags
    or dword[flagnz],10000h
.notflags
    test al,cl
    jz .zero
    mov word[flagnz],1
    or al,cl
    addr_a_8bw
    endloop
.zero
    mov word[flagnz],0
    or al,cl
    addr_a_8bw
    endloop
NEWSYM cCOp0Cm16; TSB a
    addr_a_16brni
    mov cx,[xa]
    test dword[flagnz],18000h
    jz .notflags
    or dword[flagnz],10000h
.notflags
    test ax,cx
    jz .zero
    mov word[flagnz],1
    or ax,cx
    addr_a_16bw
    endloop
.zero
    mov word[flagnz],0
    or ax,cx
    addr_a_16bw
    endloop

NEWSYM cCOpAAx8 ; TAX i
    mov al,[xa]
    mov [xx],al
    flagsetnz8b
    endloop
NEWSYM cCOpAAx16; TAX i
    mov ax,[xa]
    mov [xx],ax
    flagsetnz16b
    endloop
NEWSYM cCOpA8x8 ; TAY i
    mov al,[xa]
    mov [xy],al
    flagsetnz8b
    endloop
NEWSYM cCOpA8x16; TAY i
    mov ax,[xa]
    mov [xy],ax
    flagsetnz16b
    endloop
NEWSYM cCOp5B   ; TCD i
    mov ax,[xa]
    mov [xd],ax
    ccallv UpdateDPage
    flagsetnz16b
    endloop
NEWSYM cCOp1B   ; TCS i
    mov ax,[xa]
    test byte[xe],1
    jnz .emul
    mov [xs],ax
    endloop
.emul
    mov [xs],al
    endloop
NEWSYM cCOp7B   ; TDC i
    mov ax,[xd]
    mov [xa],ax
    flagsetnz16b
    endloop
NEWSYM cCOp3B   ; TSC i
    mov ax,[xs]
    mov [xa],ax
    flagsetnz16b
    endloop
NEWSYM cCOpBAx8 ; TSX i
    mov al,[xs]
    mov [xx],al
    flagsetnz8b
    endloop
NEWSYM cCOpBAx16; TSX i
    mov ax,[xs]
    mov [xx],ax
    flagsetnz16b
    endloop
NEWSYM cCOp8Am8 ; TXA i
    mov al,[xx]
    mov [xa],al
    flagsetnz8b
    endloop
NEWSYM cCOp8Am16; TXA i
    mov ax,[xx]
    mov [xa],ax
    flagsetnz16b
    endloop
NEWSYM cCOp9A   ; TXS i
    mov ax,[xx]
    mov [xs],ax
    test byte[xe],1
    jnz .emul
    endloop
.emul
    mov byte[xs+1],1
    endloop
NEWSYM cCOp9Bx8 ; TXY i
    mov al,[xx]
    mov [xy],al
    flagsetnz8b
    endloop
NEWSYM cCOp9Bx16; TXY i
    mov ax,[xx]
    mov [xy],ax
    flagsetnz16b
    endloop
NEWSYM cCOp98m8 ; TYA i
    mov al,[xy]
    mov [xa],al
    flagsetnz8b
    endloop
NEWSYM cCOp98m16; TYA i
    mov ax,[xy]
    mov [xa],ax
    flagsetnz16b
    endloop
NEWSYM cCOpBBx8 ; TYX i
    mov al,[xy]
    mov [xx],al
    flagsetnz8b
    endloop
NEWSYM cCOpBBx16; TYX i
    mov ax,[xy]
    mov [xx],ax
    flagsetnz16b
    endloop

NEWSYM cCOpCB   ; WAI i
    WAIMacro

NEWSYM cCOpEB   ; XBA i
    mov ah,[xa]
    mov al,[xa+1]
    mov [xa],ax
    flagsetnz8b
    endloop

NEWSYM cCOpFB   ; XCE i
    XCEMacro

NEWSYM cCOp42   ; WDM
    inc esi
    endloop


