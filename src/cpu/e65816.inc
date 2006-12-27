;Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
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
EXTSYM intrset,UpdateDPage,switchtovirqret

NEWSYM E65816IncStart

; B = ( or )
; C = ,
; L = [ or ]
; I = #

SECTION .text

COp61m8nd:  ; ADC(d,x)
   addr_BdCxB_8br
   ADCMacro8bnd
COp61m16nd: ; ADC(d,x)
   addr_BdCxB_16br
   ADCMacro16bnd
COp61m8d:   ; ADC(d,x)
   addr_BdCxB_8br
   ADCMacro8bd
COp61m16d:  ; ADC(d,x)
   addr_BdCxB_16br
   ADCMacro16bd
COp63m8nd:  ; ADC d,s
   addr_dCs_8br
   ADCMacro8bnd
COp63m16nd: ; ADC d,s
   addr_dCs_16br
   ADCMacro16bnd
COp63m8d:   ; ADC d,s
   addr_dCs_8br
   ADCMacro8bd
COp63m16d:  ; ADC d,s
   addr_dCs_16br
   ADCMacro16bd
COp65m8nd:  ; ADC d
   addr_d_8br
   ADCMacro8bnd
COp65m16nd: ; ADC d
   addr_d_16br
   ADCMacro16bnd
COp65m8d:   ; ADC d
   addr_d_8br
   ADCMacro8bd
COp65m16d:  ; ADC d
   addr_d_16br
   ADCMacro16bd
COp67m8nd:  ; ADC [d]
   addr_LdL_8br
   ADCMacro8bnd
COp67m16nd: ; ADC [d]
   addr_LdL_16br
   ADCMacro16bnd
COp67m8d:   ; ADC [d]
   addr_LdL_8br
   ADCMacro8bd
COp67m16d:  ; ADC [d]
   addr_LdL_16br
   ADCMacro16bd
COp69m8nd:  ; ADC #
   addr_I_8br
   ADCMacro8bnd
COp69m16nd: ; ADC #
   addr_I_16br
   ADCMacro16bnd
COp69m8d:   ; ADC #
   addr_I_8br
   ADCMacro8bd
COp69m16d:  ; ADC #
   addr_I_16br
   ADCMacro16bd
COp6Dm8nd:  ; ADC a
   addr_a_8br
   ADCMacro8bnd
COp6Dm16nd: ; ADC a
   addr_a_16br
   ADCMacro16bnd
COp6Dm8d:   ; ADC a
   addr_a_8br
   ADCMacro8bd
COp6Dm16d:  ; ADC a
   addr_a_16br
   ADCMacro16bd
COp6Fm8nd:  ; ADC al
   addr_al_8br
   ADCMacro8bnd
COp6Fm16nd: ; ADC al
   addr_al_16br
   ADCMacro16bnd
COp6Fm8d:   ; ADC al
   addr_al_8br
   ADCMacro8bd
COp6Fm16d:  ; ADC al
   addr_al_16br
   ADCMacro16bd
COp71m8nd:  ; ADC(d),y
   addr_BdBCy_8br
   ADCMacro8bnd
COp71m16nd: ; ADC(d),y
   addr_BdBCy_16br
   ADCMacro16bnd
COp71m8d:   ; ADC(d),y
   addr_BdBCy_8br
   ADCMacro8bd
COp71m16d:  ; ADC(d),y
   addr_BdBCy_16br
   ADCMacro16bd
COp72m8nd:  ; ADC (d)
   addr_BdB_8br
   ADCMacro8bnd
COp72m16nd: ; ADC (d)
   addr_BdB_16br
   ADCMacro16bnd
COp72m8d:   ; ADC (d)
   addr_BdB_8br
   ADCMacro8bd
COp72m16d:  ; ADC (d)
   addr_BdB_16br
   ADCMacro16bd
COp73m8nd:  ; ADC(d,s),y
   addr_BdCsBCy_8br
   ADCMacro8bnd
COp73m16nd: ; ADC(d,s),y
   addr_BdCsBCy_16br
   ADCMacro16bnd
COp73m8d:   ; ADC(d,s),y
   addr_BdCsBCy_8br
   ADCMacro8bd
COp73m16d:  ; ADC(d,s),y
   addr_BdCsBCy_16br
   ADCMacro16bd
COp75m8nd:  ; ADC d,x
   addr_dCx_8br
   ADCMacro8bnd
COp75m16nd: ; ADC d,x
   addr_dCx_16br
   ADCMacro16bnd
COp75m8d:   ; ADC d,x
   addr_dCx_8br
   ADCMacro8bd
COp75m16d:  ; ADC d,x
   addr_dCx_16br
   ADCMacro16bd
COp77m8nd:  ; ADC [d],y
   addr_LdLCy_8br
   ADCMacro8bnd
COp77m16nd: ; ADC [d],y
   addr_LdLCy_16br
   ADCMacro16bnd
COp77m8d:   ; ADC [d],y
   addr_LdLCy_8br
   ADCMacro8bd
COp77m16d:  ; ADC [d],y
   addr_LdLCy_16br
   ADCMacro16bd
COp79m8nd:  ; ADC a,y
   addr_aCy_8br
   ADCMacro8bnd
COp79m16nd: ; ADC a,y
   addr_aCy_16br
   ADCMacro16bnd
COp79m8d:   ; ADC a,y
   addr_aCy_8br
   ADCMacro8bd
COp79m16d:  ; ADC a,y
   addr_aCy_16br
   ADCMacro16bd
COp7Dm8nd:  ; ADC a,x
   addr_aCx_8br
   ADCMacro8bnd
COp7Dm16nd: ; ADC a,x
   addr_aCx_16br
   ADCMacro16bnd
COp7Dm8d:   ; ADC a,x
   addr_aCx_8br
   ADCMacro8bd
COp7Dm16d:  ; ADC a,x
   addr_aCx_16br
   ADCMacro16bd
COp7Fm8nd:  ; ADC al,x
   addr_alCx_8br
   ADCMacro8bnd
COp7Fm16nd: ; ADC al,x
   addr_alCx_16br
   ADCMacro16bnd
COp7Fm8d:   ; ADC al,x
   addr_alCx_8br
   ADCMacro8bd
COp7Fm16d:  ; ADC al,x
   addr_alCx_16br
   ADCMacro16bd

COp21m8: ; AND(d,x)
   addr_BdCxB_8br
   ANDMacro8b
COp21m16:; AND(d,x)
   addr_BdCxB_16br
   ANDMacro16b
COp23m8: ; AND d,s
   addr_dCs_8br
   ANDMacro8b
COp23m16:; AND d,s
   addr_dCs_16br
   ANDMacro16b
COp25m8: ; AND d
   addr_d_8br
   ANDMacro8b
COp25m16:; AND d
   addr_d_16br
   ANDMacro16b
COp27m8: ; AND [d]
   addr_LdL_8br
   ANDMacro8b
COp27m16:; AND [d]
   addr_LdL_16br
   ANDMacro16b
COp29m8: ; AND #
   addr_I_8br
   ANDMacro8b
COp29m16:; AND #
   addr_I_16br
   ANDMacro16b
COp2Dm8: ; AND a
   addr_a_8br
   ANDMacro8b
COp2Dm16:; AND a
   addr_a_16br
   ANDMacro16b
COp2Fm8: ; AND al
   addr_al_8br
   ANDMacro8b
COp2Fm16:; AND al
   addr_al_16br
   ANDMacro16b
COp31m8: ; AND(d),y
   addr_BdBCy_8br
   ANDMacro8b
COp31m16:; AND(d),y
   addr_BdBCy_16br
   ANDMacro16b
COp32m8: ; AND (d)
   addr_BdB_8br
   ANDMacro8b
COp32m16:; AND (d)
   addr_BdB_16br
   ANDMacro16b
COp33m8: ; AND(d,s),y
   addr_BdCsBCy_8br
   ANDMacro8b
COp33m16:; AND(d,s),y
   addr_BdCsBCy_16br
   ANDMacro16b
COp35m8: ; AND d,x
   addr_dCx_8br
   ANDMacro8b
COp35m16:; AND d,x
   addr_dCx_16br
   ANDMacro16b
COp37m8: ; AND [d],y
   addr_LdLCy_8br
   ANDMacro8b
COp37m16:; AND [d],y
   addr_LdLCy_16br
   ANDMacro16b
COp39m8: ; AND a,y
   addr_aCy_8br
   ANDMacro8b
COp39m16:; AND a,y
   addr_aCy_16br
   ANDMacro16b
COp3Dm8: ; AND a,x
   addr_aCx_8br
   ANDMacro8b
COp3Dm16:; AND a,x
   addr_aCx_16br
   ANDMacro16b
COp3Fm8: ; AND al,x
   addr_alCx_8br
   ANDMacro8b
COp3Fm16:; AND al,x
   addr_alCx_16br
   ANDMacro16b

COp06m8: ; ASL d
   addr_d_8brni
   ASLMacro8b
   addr_d_8bw
   endloop
COp06m16:; ASL d
   addr_d_16brni
   ASLMacro16b
   addr_d_16bw
   endloop
COp0Am8: ; ASL A
   addr_A_8brni
   ASLMacro8b
   addr_A_8bw
   endloop
COp0Am16:; ASL A
   addr_A_16brni
   ASLMacro16b
   addr_A_16bw
   endloop
COp0Em8: ; ASL a
   addr_a_8brni
   ASLMacro8b
   addr_a_8bw
   endloop
COp0Em16:; ASL a
   addr_a_16brni
   ASLMacro16b
   addr_a_16bw
   endloop
COp16m8: ; ASL d,x
   addr_dCx_8brni
   ASLMacro8b
   addr_dCx_8bw
   endloop
COp16m16:; ASL d,x
   addr_dCx_16brni
   ASLMacro16b
   addr_dCx_16bw
   endloop
COp1Em8: ; ASL a,x
   addr_aCx_8brni
   ASLMacro8b
   addr_aCx_8bw
   endloop
COp1Em16:; ASL a,x
   addr_aCx_16brni
   ASLMacro16b
   addr_aCx_16bw
   endloop

COp90:   ; BCC r
    test byte[flagc],01h
    jnz .skip
    JumpMacro
COpB0:   ; BCS r
    test byte[flagc],01h
    jz .skip
    JumpMacro
COpF0:   ; BEQ r
    test dword[flagnz],0FFFFh
    jnz .skip
    JumpMacro
COp30:   ; BMI r
    test dword[flagnz],018000h
    jz .skip
    JumpMacro
COpD0:   ; BNE r
    test dword[flagnz],0FFFFh
    jz .skip
    JumpMacro
COp10:   ; BPL r
    test dword[flagnz],018000h
    jnz .skip
    JumpMacro
COp80:   ; BRA r
    movsx eax,byte[esi]
    inc esi
    add esi,eax
    endloop
COp50:   ; BVC r
    test dword[flago],0FFh
    jnz .skip
    JumpMacro
COp70:   ; BVS r
    test dword[flago],0FFh
    jz .skip
    JumpMacro

;  BIT instruction does not affect N and V flags when using immediate
;  addressing mode. When using other addressing modes, the N and V flags

COp89m8: ; BIT #
   BITMacroim8b
COp89m16:; BIT #
   BITMacroim16b

COp24m8: ; BIT d
   addr_d_8br
   BITMacro8b
COp24m16:; BIT d
   addr_d_16br
   BITMacro16b
COp2Cm8: ; BIT a
   addr_a_8br
   BITMacro8b
COp2Cm16:; BIT a
   addr_a_16br
   BITMacro16b
COp34m8: ; BIT d,x
   addr_dCx_8br
   BITMacro8b
COp34m16:; BIT d,x
   addr_dCx_16br
   BITMacro16b
COp3Cm8: ; BIT a,x
   addr_aCx_8br
   BITMacro8b
COp3Cm16:; BIT a,x
   addr_aCx_16br
   BITMacro16b

COp00:   ; BRK s
   BRKMacro

COp82:   ; BRL rl
   BRLMacro

COp18:   ; CLC i
    mov dword[flagc],0
    endloop
COpD8:   ; CLD i
    and dl,0F7h
    mov bl,dl
    mov edi,[tablead+ebx*4]
    endloop
COp58:   ; CLI i
    and dl,0FBh
    cmp byte[doirqnext],0
    je .noirqexec
    mov byte[doirqnext],0
    call switchtovirqret
.noirqexec
    endloop
COpB8:   ; CLV i
    mov dword[flago],0
    endloop

COpC1m8: ; CMP(d,x)
   addr_BdCxB_8br
   CMPMacro8b
COpC1m16:; CMP(d,x)
   addr_BdCxB_16br
   CMPMacro16b
COpC3m8: ; CMP d,s
   addr_dCs_8br
   CMPMacro8b
COpC3m16:; CMP d,s
   addr_dCs_16br
   CMPMacro16b
COpC5m8: ; CMP d
   addr_d_8br
   CMPMacro8b
COpC5m16:; CMP d
   addr_d_16br
   CMPMacro16b
COpC7m8: ; CMP [d]
   addr_LdL_8br
   CMPMacro8b
COpC7m16:; CMP [d]
   addr_LdL_16br
   CMPMacro16b
COpC9m8: ; CMP #
   addr_I_8br
   CMPMacro8b
COpC9m16:; CMP #
   addr_I_16br
   CMPMacro16b
COpCDm8: ; CMP a
   addr_a_8br
   CMPMacro8b
COpCDm16:; CMP a
   addr_a_16br
   CMPMacro16b
COpCFm8: ; CMP al
   addr_al_8br
   CMPMacro8b
COpCFm16:; CMP al
   addr_al_16br
   CMPMacro16b
COpD1m8: ; CMP(d),y
   addr_BdBCy_8br
   CMPMacro8b
COpD1m16:; CMP(d),y
   addr_BdBCy_16br
   CMPMacro16b
COpD2m8: ; CMP (d)
   addr_BdB_8br
   CMPMacro8b
COpD2m16:; CMP (d)
   addr_BdB_16br
   CMPMacro16b
COpD3m8: ; CMP(d,s),y
   addr_BdCsBCy_8br
   CMPMacro8b
COpD3m16:; CMP(d,s),y
   addr_BdCsBCy_16br
   CMPMacro16b
COpD5m8: ; CMP d,x
   addr_dCx_8br
   CMPMacro8b
COpD5m16:; CMP d,x
   addr_dCx_16br
   CMPMacro16b
COpD7m8: ; CMP [d],y
   addr_LdLCy_8br
   CMPMacro8b
COpD7m16:; CMP [d],y
   addr_LdLCy_16br
   CMPMacro16b
COpD9m8: ; CMP a,y
   addr_aCy_8br
   CMPMacro8b
COpD9m16:; CMP a,y
   addr_aCy_16br
   CMPMacro16b
COpDDm8: ; CMP a,x
   addr_aCx_8br
   CMPMacro8b
COpDDm16:; CMP a,x
   addr_aCx_16br
   CMPMacro16b
COpDFm8: ; CMP al,x
   addr_alCx_8br
   CMPMacro8b
COpDFm16:; CMP al,x
   addr_alCx_16br
   CMPMacro16b

COp02:   ; COP s
   COPMacro

COpE0x8: ; CPX #
   addr_I_8br
   CPXMacro8b
COpE0x16:; CPX #
   addr_I_16br
   CPXMacro16b
COpE4x8: ; CPX d
   addr_d_8br
   CPXMacro8b
COpE4x16:; CPX d
   addr_d_16br
   CPXMacro16b
COpECx8: ; CPX a
   addr_a_8br
   CPXMacro8b
COpECx16:; CPX a
   addr_a_16br
   CPXMacro16b

COpC0x8: ; CPY #
   addr_I_8br
   CPYMacro8b
COpC0x16:; CPY #
   addr_I_16br
   CPYMacro16b
COpC4x8: ; CPY d
   addr_d_8br
   CPYMacro8b
COpC4x16:; CPY d
   addr_d_16br
   CPYMacro16b
COpCCx8: ; CPY a
   addr_a_8br
   CPYMacro8b
COpCCx16:; CPY a
   addr_a_16br
   CPYMacro16b

COp3Am8: ; DEC A
    addr_A_8brni
    dec al
    flagsetnz8bnel
    addr_A_8bw
    endloop
COp3Am16:; DEC A
    addr_A_16brni
    dec ax
    flagsetnz16bnel
    addr_A_16bw
    endloop
COpCEm8: ; DEC a
    addr_a_8brni
    dec al
    flagsetnz8bnel
    addr_a_8bw
    endloop
COpCEm16:; DEC a
    addr_a_16brni
    dec ax
    flagsetnz16bnel
    addr_a_16bw
    endloop
COpC6m8: ; DEC d
    addr_d_8brni
    dec al
    flagsetnz8bnel
    addr_d_8bw
    endloop
COpC6m16:; DEC d
    addr_d_16brni
    dec ax
    flagsetnz16bnel
    addr_d_16bw
    endloop
COpD6m8: ; DEC d,x
    addr_dCx_8brni
    dec al
    flagsetnz8bnel
    addr_dCx_8bw
    endloop
COpD6m16:; DEC d,x
    addr_dCx_16brni
    dec ax
    flagsetnz16bnel
    addr_dCx_16bw
    endloop
COpDEm8: ; DEC a,x
    addr_aCx_8brni
    dec al
    flagsetnz8bnel
    addr_aCx_8bw
    endloop
COpDEm16:; DEC a,x
    addr_aCx_16brni
    dec ax
    flagsetnz16bnel
    addr_aCx_16bw
    endloop

COpCAx8: ; DEX i
    dec byte[xx]
    mov al,[xx]
    flagsetnz8b
    endloop
COpCAx16:; DEX i
    dec word[xx]
    mov ax,[xx]
    flagsetnz16b
    endloop
COp88x8: ; DEY i
    dec byte[xy]
    mov al,[xy]
    flagsetnz8b
    endloop
COp88x16:; DEY i
    dec word[xy]
    mov ax,[xy]
    flagsetnz16b
    endloop

COp41m8: ; EOR(d,x)
   addr_BdCxB_8br
   EORMacro8b
COp41m16:; EOR(d,x)
   addr_BdCxB_16br
   EORMacro16b
COp43m8: ; EOR d,s
   addr_dCs_8br
   EORMacro8b
COp43m16:; EOR d,s
   addr_dCs_16br
   EORMacro16b
COp45m8: ; EOR d
   addr_d_8br
   EORMacro8b
COp45m16:; EOR d
   addr_d_16br
   EORMacro16b
COp47m8: ; EOR [d]
   addr_LdL_8br
   EORMacro8b
COp47m16:; EOR [d]
   addr_LdL_16br
   EORMacro16b
COp49m8: ; EOR #
   addr_I_8br
   EORMacro8b
COp49m16:; EOR #
   addr_I_16br
   EORMacro16b
COp4Dm8: ; EOR a
   addr_a_8br
   EORMacro8b
COp4Dm16:; EOR a
   addr_a_16br
   EORMacro16b
COp4Fm8: ; EOR al
   addr_al_8br
   EORMacro8b
COp4Fm16:; EOR al
   addr_al_16br
   EORMacro16b
COp51m8: ; EOR(d),y
   addr_BdBCy_8br
   EORMacro8b
COp51m16:; EOR(d),y
   addr_BdBCy_16br
   EORMacro16b
COp52m8: ; EOR (d)
   addr_BdB_8br
   EORMacro8b
COp52m16:; EOR (d)
   addr_BdB_16br
   EORMacro16b
COp53m8: ; EOR(d,s),y
   addr_BdCsBCy_8br
   EORMacro8b
COp53m16:; EOR(d,s),y
   addr_BdCsBCy_16br
   EORMacro16b
COp55m8: ; EOR d,x
   addr_dCx_8br
   EORMacro8b
COp55m16:; EOR d,x
   addr_dCx_16br
   EORMacro16b
COp57m8: ; EOR [d],y
   addr_LdLCy_8br
   EORMacro8b
COp57m16:; EOR [d],y
   addr_LdLCy_16br
   EORMacro16b
COp59m8: ; EOR a,y
   addr_aCy_8br
   EORMacro8b
COp59m16:; EOR a,y
   addr_aCy_16br
   EORMacro16b
COp5Dm8: ; EORa,x
   addr_aCx_8br
   EORMacro8b
COp5Dm16:; EORa,x
   addr_aCx_16br
   EORMacro16b
COp5Fm8: ; EOR al,x
   addr_alCx_8br
   EORMacro8b
COp5Fm16:; EOR al,x
   addr_alCx_16br
   EORMacro16b


COp1Am8: ; INC A
    addr_A_8brni
    inc al
    flagsetnz8bnel
    addr_A_8bw
    endloop
COp1Am16:; INC A
    addr_A_16brni
    inc ax
    flagsetnz16bnel
    addr_A_16bw
    endloop
COpEEm8: ; INC a
    addr_a_8brni
    inc al
    flagsetnz8bnel
    addr_a_8bw
    endloop
COpEEm16:; INC a
    addr_a_16brni
    inc ax
    flagsetnz16bnel
    addr_a_16bw
    endloop
COpE6m8: ; INC d
    addr_d_8brni
    inc al
    flagsetnz8bnel
    addr_d_8bw
    endloop
COpE6m16:; INC d
    addr_d_16brni
    inc ax
    flagsetnz16bnel
    addr_d_16bw
    endloop
COpF6m8: ; INC d,x
    addr_dCx_8brni
    inc al
    flagsetnz8bnel
    addr_dCx_8bw
    endloop
COpF6m16:; INC d,x
    addr_dCx_16brni
    inc ax
    flagsetnz16bnel
    addr_dCx_16bw
    endloop
COpFEm8: ; INC a,x
    addr_aCx_8brni
    inc al
    flagsetnz8bnel
    addr_aCx_8bw
    endloop
COpFEm16:; INC a,x
    addr_aCx_16brni
    inc ax
    flagsetnz16bnel
    addr_aCx_16bw
    endloop


COpE8x8: ; INX i
    inc byte[xx]
    mov al,[xx]
    flagsetnz8b
    endloop
COpE8x16:; INX i
    inc word[xx]
    mov ax,[xx]
    flagsetnz16b
    endloop
COpC8x8: ; INY i
    inc byte[xy]
    mov al,[xy]
    flagsetnz8b
    endloop
COpC8x16:; INY i
    inc word[xy]
    mov ax,[xy]
    flagsetnz16b
    endloop

COpDC:   ; JML (a)
    JMLMacro
COp4C:   ; JMP a
    JMPMacro4C
COp6C:   ; JMP (a)
    JMPMacro6C
COp7C:   ; JMP(a,x)
    JMPMacro7C
COp5C:   ; JMP al
    JMPMacro5C

COp22:   ; JSL al
   JSLMacro
COp20:   ; JSR a
   JSRMacro20
COpFC:   ; JSR(a,x)
   JSRMacroFC

COpA1m8: ; LDA(d,x)
   addr_BdCxB_8br
   LDAMacro8b
COpA1m16:; LDA(d,x)
   addr_BdCxB_16br
   LDAMacro16b
COpA3m8: ; LDA d,s
   addr_dCs_8br
   LDAMacro8b
COpA3m16:; LDA d,s
   addr_dCs_16br
   LDAMacro16b
COpA5m8: ; LDA d
   addr_d_8br
   LDAMacro8b
COpA5m16:; LDA d
   addr_d_16br
   LDAMacro16b
COpA7m8: ; LDA [d]
   addr_LdL_8br
   LDAMacro8b
COpA7m16:; LDA [d]
   addr_LdL_16br
   LDAMacro16b
COpA9m8: ; LDA #
   addr_I_8br
   LDAMacro8b
COpA9m16:; LDA #
   addr_I_16br
   LDAMacro16b
COpADm8: ; LDA a
   addr_a_8br
   LDAMacro8b
COpADm16:; LDA a
   addr_a_16br
   LDAMacro16b
COpAFm8: ; LDA al
   addr_al_8br
   LDAMacro8b
COpAFm16:; LDA al
   addr_al_16br
   LDAMacro16b
COpB1m8: ; LDA(d),y
   addr_BdBCy_8br
   LDAMacro8b
COpB1m16:; LDA(d),y
   addr_BdBCy_16br
   LDAMacro16b
COpB2m8: ; LDA (d)
   addr_BdB_8br
   LDAMacro8b
COpB2m16:; LDA (d)
   addr_BdB_16br
   LDAMacro16b
COpB3m8: ; LDA(d,s),y
   addr_BdCsBCy_8br
   LDAMacro8b
COpB3m16:; LDA(d,s),y
   addr_BdCsBCy_16br
   LDAMacro16b
COpB5m8: ; LDA d,x
   addr_dCx_8br
   LDAMacro8b
COpB5m16:; LDA d,x
   addr_dCx_16br
   LDAMacro16b
COpB7m8: ; LDA [d],y
   addr_LdLCy_8br
   LDAMacro8b
COpB7m16:; LDA [d],y
   addr_LdLCy_16br
   LDAMacro16b
COpB9m8: ; LDA a,y
   addr_aCy_8br
   LDAMacro8b
COpB9m16:; LDA a,y
   addr_aCy_16br
   LDAMacro16b
COpBDm8: ; LDA a,x
   addr_aCx_8br
   LDAMacro8b
COpBDm16:; LDA a,x
   addr_aCx_16br
   LDAMacro16b
COpBFm8: ; LDA al,x
   addr_alCx_8br
   LDAMacro8b
COpBFm16:; LDA al,x
   addr_alCx_16br
   LDAMacro16b

COpA2x8: ; LDX #
   addr_I_8br
   LDXMacro8b
COpA2x16:; LDX #
   addr_I_16br
   LDXMacro16b
COpA6x8: ; LDX d
   addr_d_8br
   LDXMacro8b
COpA6x16:; LDX d
   addr_d_16br
   LDXMacro16b
COpAEx8: ; LDX a
   addr_a_8br
   LDXMacro8b
COpAEx16:; LDX a
   addr_a_16br
   LDXMacro16b
COpB6x8: ; LDX d,y
   addr_dCy_8br
   LDXMacro8b
COpB6x16:; LDX d,y
   addr_dCy_16br
   LDXMacro16b
COpBEx8: ; LDX a,y
   addr_aCy_8br
   LDXMacro8b
COpBEx16:; LDX a,y
   addr_aCy_16br
   LDXMacro16b

COpA0x8: ; LDY #
   addr_I_8br
   LDYMacro8b
COpA0x16:; LDY #
   addr_I_16br
   LDYMacro16b
COpA4x8: ; LDY d
   addr_d_8br
   LDYMacro8b
COpA4x16:; LDY d
   addr_d_16br
   LDYMacro16b
COpACx8: ; LDY a
   addr_a_8br
   LDYMacro8b
COpACx16:; LDY a
   addr_a_16br
   LDYMacro16b
COpB4x8: ; LDY d,x
   addr_dCx_8br
   LDYMacro8b
COpB4x16:; LDY d,x
   addr_dCx_16br
   LDYMacro16b
COpBCx8: ; LDY a,x
   addr_aCx_8br
   LDYMacro8b
COpBCx16:; LDY a,x
   addr_aCx_16br
   LDYMacro16b

COp46m8: ; LSR d
   addr_d_8brni
   LSRMacro8b
   addr_d_8bw
   endloop
COp46m16:; LSR d
   addr_d_16brni
   LSRMacro16b
   addr_d_16bw
   endloop
COp4Am8: ; LSR A
   addr_A_8brni
   LSRMacro8b
   addr_A_8bw
   endloop
COp4Am16:; LSR A
   addr_A_16brni
   LSRMacro16b
   addr_A_16bw
   endloop
COp4Em8: ; LSR a
   addr_a_8brni
   LSRMacro8b
   addr_a_8bw
   endloop
COp4Em16:; LSR a
   addr_a_16brni
   LSRMacro16b
   addr_a_16bw
   endloop
COp56m8: ; LSR d,x
   addr_dCx_8brni
   LSRMacro8b
   addr_dCx_8bw
   endloop
COp56m16:; LSR d,x
   addr_dCx_16brni
   LSRMacro16b
   addr_dCx_16bw
   endloop
COp5Em8: ; LSRa,x
   addr_aCx_8brni
   LSRMacro8b
   addr_aCx_8bw
   endloop
COp5Em16:; LSRa,x
   addr_aCx_16brni
   LSRMacro16b
   addr_aCx_16bw
   endloop

COp54:   ; MVN xya
   MVNMacro
COp44:   ; MVP xya
   MVPMacro

COpEA:   ; NOP i
   endloop

COp01m8: ; ORA(d,x)
   addr_BdCxB_8br
   ORAMacro8b
COp01m16:; ORA(d,x)
   addr_BdCxB_16br
   ORAMacro16b
COp03m8: ; ORA d,s
   addr_dCs_8br
   ORAMacro8b
COp03m16:; ORA d,s
   addr_dCs_16br
   ORAMacro16b
COp05m8: ; ORA d
   addr_d_8br
   ORAMacro8b
COp05m16:; ORA d
   addr_d_16br
   ORAMacro16b
COp07m8: ; ORA [d]
   addr_LdL_8br
   ORAMacro8b
COp07m16:; ORA [d]
   addr_LdL_16br
   ORAMacro16b
COp09m8: ; ORA #
   addr_I_8br
   ORAMacro8b
COp09m16:; ORA #
   addr_I_16br
   ORAMacro16b
COp0Dm8: ; ORA a
   addr_a_8br
   ORAMacro8b
COp0Dm16:; ORA a
   addr_a_16br
   ORAMacro16b
COp0Fm8: ; ORA al
   addr_al_8br
   ORAMacro8b
COp0Fm16:; ORA al
   addr_al_16br
   ORAMacro16b
COp11m8: ; ORA(d),y
   addr_BdBCy_8br
   ORAMacro8b
COp11m16:; ORA(d),y
   addr_BdBCy_16br
   ORAMacro16b
COp12m8: ; ORA(d)
   addr_BdB_8br
   ORAMacro8b
COp12m16:; ORA(d)
   addr_BdB_16br
   ORAMacro16b
COp13m8: ; ORA(d,s),y
   addr_BdCsBCy_8br
   ORAMacro8b
COp13m16:; ORA(d,s),y
   addr_BdCsBCy_16br
   ORAMacro16b
COp15m8: ; ORA d,x
   addr_dCx_8br
   ORAMacro8b
COp15m16:; ORA d,x
   addr_dCx_16br
   ORAMacro16b
COp17m8: ; ORA [d],y
   addr_LdLCy_8br
   ORAMacro8b
COp17m16:; ORA [d],y
   addr_LdLCy_16br
   ORAMacro16b
COp19m8: ; ORA a,y
   addr_aCy_8br
   ORAMacro8b
COp19m16:; ORA a,y
   addr_aCy_16br
   ORAMacro16b
COp1Dm8: ; ORA a,x
   addr_aCx_8br
   ORAMacro8b
COp1Dm16:; ORA a,x
   addr_aCx_16br
   ORAMacro16b
COp1Fm8: ; ORA al,x
   addr_alCx_8br
   ORAMacro8b
COp1Fm16:; ORA al,x
   addr_alCx_16br
   ORAMacro16b

COpF4:   ; PEA s
   PEAMacro
COpD4:   ; PEI s
   PEIMacro
COp62:   ; PER s
   PERMacro

COp48m8: ; PHA s
   PUSHMacro8b [xa]
COp48m16:; PHA s
   PUSHMacro16b [xa], [xa+1]
COp8B:   ; PHB s
   PUSHMacro8b [xdb]
COp0B:   ; PHD s
   PUSHMacro16b [xd], [xd+1]
COp4B:   ; PHK s
   PUSHMacro8b [xpb]
COp08:   ; PHP s
   PUSHMacro8bp
COpDAx8: ; PHX s
   PUSHMacro8b [xx]
COpDAx16:; PHX s
   PUSHMacro16b [xx], [xx+1]
COp5Ax8: ; PHY s
   PUSHMacro8b [xy]
COp5Ax16:; PHY s
   PUSHMacro16b [xy], [xy+1]

COp68m8: ; PLA s
   POPMacro8b [xa]
COp68m16:; PLA s
   POPMacro16b [xa], [xa+1]
COpAB:   ; PLB s
   POPMacro8b [xdb]
COp2B:   ; PLD s
   POPMacro16bd [xd], [xd+1]
COp28:   ; PLP s
   POPMacroP
COpFAx8: ; PLX s
   POPMacro8b [xx]
COpFAx16:; PLX s
   POPMacro16b [xx], [xx+1]
COp7Ax8: ; PLY s
   POPMacro8b [xy]
COp7Ax16:; PLY s
   POPMacro16b [xy], [xy+1]

COpC2:   ; REP #
   REPMacro

COp26m8: ; ROL d
   addr_d_8brni
   ROLMacro8b
   addr_d_8bw
   endloop
COp26m16:; ROL d
   addr_d_16brni
   ROLMacro16b
   addr_d_16bw
   endloop
COp2Am8: ; ROL A
   addr_A_8brni
   ROLMacro8b
   addr_A_8bw
   endloop
COp2Am16:; ROL A
   addr_A_16brni
   ROLMacro16b
   addr_A_16bw
   endloop
COp2Em8: ; ROL a
   addr_a_8brni
   ROLMacro8b
   addr_a_8bw
   endloop
COp2Em16:; ROL a
   addr_a_16brni
   ROLMacro16b
   addr_a_16bw
   endloop
COp36m8: ; ROL d,x
   addr_dCx_8brni
   ROLMacro8b
   addr_dCx_8bw
   endloop
COp36m16:; ROL d,x
   addr_dCx_16brni
   ROLMacro16b
   addr_dCx_16bw
   endloop
COp3Em8: ; ROL a,x
   addr_aCx_8brni
   ROLMacro8b
   addr_aCx_8bw
   endloop
COp3Em16:; ROL a,x
   addr_aCx_16brni
   ROLMacro16b
   addr_aCx_16bw
   endloop

COp66m8: ; ROR d
   addr_d_8brni
   RORMacro8b
   addr_d_8bw
   endloop
COp66m16:; ROR d
   addr_d_16brni
   RORMacro16b
   addr_d_16bw
   endloop
COp6Am8: ; ROR A
   addr_A_8brni
   RORMacro8b
   addr_A_8bw
   endloop
COp6Am16:; ROR A
   addr_A_16brni
   RORMacro16b
   addr_A_16bw
   endloop
COp6Em8: ; ROR a
   addr_a_8brni
   RORMacro8b
   addr_a_8bw
   endloop
COp6Em16:; ROR a
   addr_a_16brni
   RORMacro16b
   addr_a_16bw
   endloop
COp76m8: ; ROR d,x
   addr_dCx_8brni
   RORMacro8b
   addr_dCx_8bw
   endloop
COp76m16:; ROR d,x
   addr_dCx_16brni
   RORMacro16b
   addr_dCx_16bw
   endloop
COp7Em8: ; ROR a,x
   addr_aCx_8brni
   RORMacro8b
   addr_aCx_8bw
   endloop
COp7Em16:; ROR a,x
   addr_aCx_16brni
   RORMacro16b
   addr_aCx_16bw
   endloop

COp40:   ; RTI s
   RTIMacro
COp6B:   ; RTL s
   RTLMacro
COp60:   ; RTS s
   RTSMacro

COpE1m8nd:  ; SBC(d,x)
   addr_BdCxB_8br
   SBCMacro8bnd
COpE1m16nd: ; SBC(d,x)
   addr_BdCxB_16br
   SBCMacro16bnd
COpE1m8d:   ; SBC(d,x)
   addr_BdCxB_8br
   SBCMacro8bd
COpE1m16d:  ; SBC(d,x)
   addr_BdCxB_16br
   SBCMacro16bd
COpE3m8nd:  ; SBC d,s
   addr_dCs_8br
   SBCMacro8bnd
COpE3m16nd: ; SBC d,s
   addr_dCs_16br
   SBCMacro16bnd
COpE3m8d:   ; SBC d,s
   addr_dCs_8br
   SBCMacro8bd
COpE3m16d:  ; SBC d,s
   addr_dCs_16br
   SBCMacro16bd
COpE5m8nd:  ; SBC d
   addr_d_8br
   SBCMacro8bnd
COpE5m16nd: ; SBC d
   addr_d_16br
   SBCMacro16bnd
COpE5m8d:   ; SBC d
   addr_d_8br
   SBCMacro8bd
COpE5m16d:  ; SBC d
   addr_d_16br
   SBCMacro16bd
COpE7m8nd:  ; SBC [d]
   addr_LdL_8br
   SBCMacro8bnd
COpE7m16nd: ; SBC [d]
   addr_LdL_16br
   SBCMacro16bnd
COpE7m8d:   ; SBC [d]
   addr_LdL_8br
   SBCMacro8bd
COpE7m16d:  ; SBC [d]
   addr_LdL_16br
   SBCMacro16bd
COpE9m8nd:  ; SBC #
   addr_I_8br
   SBCMacro8bnd
COpE9m16nd: ; SBC #
   addr_I_16br
   SBCMacro16bnd
COpE9m8d:   ; SBC #
   addr_I_8br
   SBCMacro8bd
COpE9m16d:  ; SBC #
   addr_I_16br
   SBCMacro16bd
COpEDm8nd:  ; SBC a
   addr_a_8br
   SBCMacro8bnd
COpEDm16nd: ; SBC a
   addr_a_16br
   SBCMacro16bnd
COpEDm8d:   ; SBC a
   addr_a_8br
   SBCMacro8bd
COpEDm16d:  ; SBC a
   addr_a_16br
   SBCMacro16bd
COpEFm8nd:  ; SBC al
   addr_al_8br
   SBCMacro8bnd
COpEFm16nd: ; SBC al
   addr_al_16br
   SBCMacro16bnd
COpEFm8d:   ; SBC al
   addr_al_8br
   SBCMacro8bd
COpEFm16d:  ; SBC al
   addr_al_16br
   SBCMacro16bd
COpF1m8nd:  ; SBC(d),y
   addr_BdBCy_8br
   SBCMacro8bnd
COpF1m16nd: ; SBC(d),y
   addr_BdBCy_16br
   SBCMacro16bnd
COpF1m8d:   ; SBC(d),y
   addr_BdBCy_8br
   SBCMacro8bd
COpF1m16d:  ; SBC(d),y
   addr_BdBCy_16br
   SBCMacro16bd
COpF2m8nd:  ; SBC (d)
   addr_BdB_8br
   SBCMacro8bnd
COpF2m16nd: ; SBC (d)
   addr_BdB_16br
   SBCMacro16bnd
COpF2m8d:   ; SBC (d)
   addr_BdB_8br
   SBCMacro8bd
COpF2m16d:  ; SBC (d)
   addr_BdB_16br
   SBCMacro16bd
COpF3m8nd:  ; SBC(d,s),y
   addr_BdCsBCy_8br
   SBCMacro8bnd
COpF3m16nd: ; SBC(d,s),y
   addr_BdCsBCy_16br
   SBCMacro16bnd
COpF3m8d:   ; SBC(d,s),y
   addr_BdCsBCy_8br
   SBCMacro8bd
COpF3m16d:  ; SBC(d,s),y
   addr_BdCsBCy_16br
   SBCMacro16bd
COpF5m8nd:  ; SBC d,x
   addr_dCx_8br
   SBCMacro8bnd
COpF5m16nd: ; SBC d,x
   addr_dCx_16br
   SBCMacro16bnd
COpF5m8d:   ; SBC d,x
   addr_dCx_8br
   SBCMacro8bd
COpF5m16d:  ; SBC d,x
   addr_dCx_16br
   SBCMacro16bd
COpF7m8nd:  ; SBC [d],y
   addr_LdLCy_8br
   SBCMacro8bnd
COpF7m16nd: ; SBC [d],y
   addr_LdLCy_16br
   SBCMacro16bnd
COpF7m8d:   ; SBC [d],y
   addr_LdLCy_8br
   SBCMacro8bd
COpF7m16d:  ; SBC [d],y
   addr_LdLCy_16br
   SBCMacro16bd
COpF9m8nd:  ; SBC a,y
   addr_aCy_8br
   SBCMacro8bnd
COpF9m16nd: ; SBC a,y
   addr_aCy_16br
   SBCMacro16bnd
COpF9m8d:   ; SBC a,y
   addr_aCy_8br
   SBCMacro8bd
COpF9m16d:  ; SBC a,y
   addr_aCy_16br
   SBCMacro16bd
COpFDm8nd:  ; SBC a,x
   addr_aCx_8br
   SBCMacro8bnd
COpFDm16nd: ; SBC a,x
   addr_aCx_16br
   SBCMacro16bnd
COpFDm8d:   ; SBC a,x
   addr_aCx_8br
   SBCMacro8bd
COpFDm16d:  ; SBC a,x
   addr_aCx_16br
   SBCMacro16bd
COpFFm8nd:  ; SBC al,x
   addr_alCx_8br
   SBCMacro8bnd
COpFFm16nd: ; SBC al,x
   addr_alCx_16br
   SBCMacro16bnd
COpFFm8d:   ; SBC al,x
   addr_alCx_8br
   SBCMacro8bd
COpFFm16d:  ; SBC al,x
   addr_alCx_16br
   SBCMacro16bd

COp38:   ; SEC i
    mov dword[flagc],0FFh
    endloop
COpF8:   ; SED i
    or dl,08h
    mov bl,dl
    mov edi,[tablead+ebx*4]
    endloop
COp78:   ; SEI i
    or dl,04h
    endloop

COpE2:   ; SEP #
    SEPMacro

COp81m8: ; STA(d,x)
   STAMacro8b
   addr_BdCxB_8bw
   endloop
COp81m16:; STA(d,x)
   STAMacro16b
   addr_BdCxB_16bw
   endloop
COp83m8: ; STA d,s
   STAMacro8b
   addr_dCs_8bw
   endloop
COp83m16:; STA d,s
   STAMacro16b
   addr_dCs_16bw
   endloop
COp85m8: ; STA d
   STAMacro8b
   addr_d_8bw
   endloop
COp85m16:; STA d
   STAMacro16b
   addr_d_16bw
   endloop
COp87m8: ; STA [d]
   STAMacro8b
   addr_LdL_8bw
   endloop
COp87m16:; STA [d]
   STAMacro16b
   addr_LdL_16bw
   endloop
COp8Dm8: ; STA a
   STAMacro8b
   addr_a_8bw
   endloop
COp8Dm16:; STA a
   STAMacro16b
   addr_a_16bw
   endloop
COp8Fm8: ; STA al
   STAMacro8b
   addr_al_8bw
   endloop
COp8Fm16:; STA al
   STAMacro16b
   addr_al_16bw
   endloop
COp91m8: ; STA(d),y
   STAMacro8b
   addr_BdBCy_8bw
   endloop
COp91m16:; STA(d),y
   STAMacro16b
   addr_BdBCy_16bw
   endloop
COp92m8: ; STA (d)
   STAMacro8b
   addr_BdB_8bw
   endloop
COp92m16:; STA (d)
   STAMacro16b
   addr_BdB_16bw
   endloop
COp93m8: ; STA(d,s),y
   STAMacro8b
   addr_BdCsBCy_8bw
   endloop
COp93m16:; STA(d,s),y
   STAMacro16b
   addr_BdCsBCy_16bw
   endloop
COp95m8: ; STA d,x
   STAMacro8b
   addr_dCx_8bw
   endloop
COp95m16:; STA d,x
   STAMacro16b
   addr_dCx_16bw
   endloop
COp97m8: ; STA [d],y
   STAMacro8b
   addr_LdLCy_8bw
   endloop
COp97m16:; STA [d],y
   STAMacro16b
   addr_LdLCy_16bw
   endloop
COp99m8: ; STA a,y
   STAMacro8b
   addr_aCy_8bw
   endloop
COp99m16:; STA a,y
   STAMacro16b
   addr_aCy_16bw
   endloop
COp9Dm8: ; STA a,x
   STAMacro8b
   addr_aCx_8bw
   endloop
COp9Dm16:; STA a,x
   STAMacro16b
   addr_aCx_16bw
   endloop
COp9Fm8: ; STA al,x
   STAMacro8b
   addr_alCx_8bw
   endloop
COp9Fm16:; STA al,x
   STAMacro16b
   addr_alCx_16bw
   endloop

COp86x8: ; STX d
   STXMacro8b
   addr_d_8bw
   endloop
COp86x16:; STX d
   STXMacro16b
   addr_d_16bw
   endloop
COp8Ex8: ; STX a
   STXMacro8b
   addr_a_8bw
   endloop
COp8Ex16:; STX a
   STXMacro16b
   addr_a_16bw
   endloop
COp96x8: ; STX d,y
   STXMacro8b
   addr_dCy_8bw
   endloop
COp96x16:; STX d,y
   STXMacro16b
   addr_dCy_16bw
   endloop

COp84x8: ; STY d
   STYMacro8b
   addr_d_8bw
   endloop
COp84x16:; STY d
   STYMacro16b
   addr_d_16bw
   endloop
COp8Cx8: ; STY a
   STYMacro8b
   addr_a_8bw
   endloop
COp8Cx16:; STY a
   STYMacro16b
   addr_a_16bw
   endloop
COp94x8: ; STYd,x
   STYMacro8b
   addr_dCx_8bw
   endloop
COp94x16:; STYd,x
   STYMacro16b
   addr_dCx_16bw
   endloop

COpDB:   ; STP i
    dec esi
    endloop

COp64m8: ; STZ d
   xor al,al
   addr_d_8bw
   endloop
COp64m16:; STZ d
   xor eax,eax
   addr_d_16bw
   endloop
COp74m8: ; STZ d,x
   xor al,al
   addr_dCx_8bw
   endloop
COp74m16:; STZ d,x
   xor eax,eax
   addr_dCx_16bw
   endloop
COp9Cm8: ; STZ a
   xor al,al
   addr_a_8bw
   endloop
COp9Cm16:; STZ a
   xor eax,eax
   addr_a_16bw
   endloop
COp9Em8: ; STZ a,x
   xor al,al
   addr_aCx_8bw
   endloop
COp9Em16:; STZ a,x
   xor eax,eax
   addr_aCx_16bw
   endloop

COp14m8: ; TRB d
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
COp14m16:; TRB d
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
COp1Cm8: ; TRB a
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
COp1Cm16:; TRB a
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

COp04m8: ; TSB d
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
COp04m16:; TSB d
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
COp0Cm8: ; TSB a
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
COp0Cm16:; TSB a
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

COpAAx8: ; TAX i
    mov al,[xa]
    mov [xx],al
    flagsetnz8b
    endloop
COpAAx16:; TAX i
    mov ax,[xa]
    mov [xx],ax
    flagsetnz16b
    endloop
COpA8x8: ; TAY i
    mov al,[xa]
    mov [xy],al
    flagsetnz8b
    endloop
COpA8x16:; TAY i
    mov ax,[xa]
    mov [xy],ax
    flagsetnz16b
    endloop
COp5B:   ; TCD i
    mov ax,[xa]
    mov [xd],ax
    call UpdateDPage
    flagsetnz16b
    endloop
COp1B:   ; TCS i
    mov ax,[xa]
    test byte[xe],1
    jnz .emul
    mov [xs],ax
    endloop
.emul
    mov [xs],al
    endloop
COp7B:   ; TDC i
    mov ax,[xd]
    mov [xa],ax
    flagsetnz16b
    endloop
COp3B:   ; TSC i
    mov ax,[xs]
    mov [xa],ax
    flagsetnz16b
    endloop
COpBAx8: ; TSX i
    mov al,[xs]
    mov [xx],al
    flagsetnz8b
    endloop
COpBAx16:; TSX i
    mov ax,[xs]
    mov [xx],ax
    flagsetnz16b
    endloop
COp8Am8: ; TXA i
    mov al,[xx]
    mov [xa],al
    flagsetnz8b
    endloop
COp8Am16:; TXA i
    mov ax,[xx]
    mov [xa],ax
    flagsetnz16b
    endloop
COp9A:   ; TXS i
    mov ax,[xx]
    mov [xs],ax
    test byte[xe],1
    jnz .emul
    endloop
.emul
    mov byte[xs+1],1
    endloop
COp9Bx8: ; TXY i
    mov al,[xx]
    mov [xy],al
    flagsetnz8b
    endloop
COp9Bx16:; TXY i
    mov ax,[xx]
    mov [xy],ax
    flagsetnz16b
    endloop
COp98m8: ; TYA i
    mov al,[xy]
    mov [xa],al
    flagsetnz8b
    endloop
COp98m16:; TYA i
    mov ax,[xy]
    mov [xa],ax
    flagsetnz16b
    endloop
COpBBx8: ; TYX i
    mov al,[xy]
    mov [xx],al
    flagsetnz8b
    endloop
COpBBx16:; TYX i
    mov ax,[xy]
    mov [xx],ax
    flagsetnz16b
    endloop

COpCB:   ; WAI i
    WAIMacro

COpEB:   ; XBA i
    mov ah,[xa]
    mov al,[xa+1]
    mov [xa],ax
    flagsetnz8b
    endloop

COpFB:   ; XCE i
    XCEMacro

COp42:   ; WDM
    inc esi
    endloop

NEWSYM E65816IncEnd


