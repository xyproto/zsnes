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
EXTSYM intrset,UpdateDPage,switchtovirqret

NEWSYM E65816IncStart

; B = ( or )
; C = ,
; L = [ or ]
; I = #

SECTION .text

NEWSYM COp61m8nd  ; ADC(d,x)
   addr_BdCxB_8br
   ADCMacro8bnd
NEWSYM COp61m16nd ; ADC(d,x)
   addr_BdCxB_16br
   ADCMacro16bnd
NEWSYM COp61m8d   ; ADC(d,x)
   addr_BdCxB_8br
   ADCMacro8bd
NEWSYM COp61m16d  ; ADC(d,x)
   addr_BdCxB_16br
   ADCMacro16bd
NEWSYM COp63m8nd  ; ADC d,s
   addr_dCs_8br
   ADCMacro8bnd
NEWSYM COp63m16nd ; ADC d,s
   addr_dCs_16br
   ADCMacro16bnd
NEWSYM COp63m8d   ; ADC d,s
   addr_dCs_8br
   ADCMacro8bd
NEWSYM COp63m16d  ; ADC d,s
   addr_dCs_16br
   ADCMacro16bd
NEWSYM COp65m8nd  ; ADC d
   addr_d_8br
   ADCMacro8bnd
NEWSYM COp65m16nd ; ADC d
   addr_d_16br
   ADCMacro16bnd
NEWSYM COp65m8d   ; ADC d
   addr_d_8br
   ADCMacro8bd
NEWSYM COp65m16d  ; ADC d
   addr_d_16br
   ADCMacro16bd
NEWSYM COp67m8nd  ; ADC [d]
   addr_LdL_8br
   ADCMacro8bnd
NEWSYM COp67m16nd ; ADC [d]
   addr_LdL_16br
   ADCMacro16bnd
NEWSYM COp67m8d   ; ADC [d]
   addr_LdL_8br
   ADCMacro8bd
NEWSYM COp67m16d  ; ADC [d]
   addr_LdL_16br
   ADCMacro16bd
NEWSYM COp69m8nd  ; ADC #
   addr_I_8br
   ADCMacro8bnd
NEWSYM COp69m16nd ; ADC #
   addr_I_16br
   ADCMacro16bnd
NEWSYM COp69m8d   ; ADC #
   addr_I_8br
   ADCMacro8bd
NEWSYM COp69m16d  ; ADC #
   addr_I_16br
   ADCMacro16bd
NEWSYM COp6Dm8nd  ; ADC a
   addr_a_8br
   ADCMacro8bnd
NEWSYM COp6Dm16nd ; ADC a
   addr_a_16br
   ADCMacro16bnd
NEWSYM COp6Dm8d   ; ADC a
   addr_a_8br
   ADCMacro8bd
NEWSYM COp6Dm16d  ; ADC a
   addr_a_16br
   ADCMacro16bd
NEWSYM COp6Fm8nd  ; ADC al
   addr_al_8br
   ADCMacro8bnd
NEWSYM COp6Fm16nd ; ADC al
   addr_al_16br
   ADCMacro16bnd
NEWSYM COp6Fm8d   ; ADC al
   addr_al_8br
   ADCMacro8bd
NEWSYM COp6Fm16d  ; ADC al
   addr_al_16br
   ADCMacro16bd
NEWSYM COp71m8nd  ; ADC(d),y
   addr_BdBCy_8br
   ADCMacro8bnd
NEWSYM COp71m16nd ; ADC(d),y
   addr_BdBCy_16br
   ADCMacro16bnd
NEWSYM COp71m8d   ; ADC(d),y
   addr_BdBCy_8br
   ADCMacro8bd
NEWSYM COp71m16d  ; ADC(d),y
   addr_BdBCy_16br
   ADCMacro16bd
NEWSYM COp72m8nd  ; ADC (d)
   addr_BdB_8br
   ADCMacro8bnd
NEWSYM COp72m16nd ; ADC (d)
   addr_BdB_16br
   ADCMacro16bnd
NEWSYM COp72m8d   ; ADC (d)
   addr_BdB_8br
   ADCMacro8bd
NEWSYM COp72m16d  ; ADC (d)
   addr_BdB_16br
   ADCMacro16bd
NEWSYM COp73m8nd  ; ADC(d,s),y
   addr_BdCsBCy_8br
   ADCMacro8bnd
NEWSYM COp73m16nd ; ADC(d,s),y
   addr_BdCsBCy_16br
   ADCMacro16bnd
NEWSYM COp73m8d   ; ADC(d,s),y
   addr_BdCsBCy_8br
   ADCMacro8bd
NEWSYM COp73m16d  ; ADC(d,s),y
   addr_BdCsBCy_16br
   ADCMacro16bd
NEWSYM COp75m8nd  ; ADC d,x
   addr_dCx_8br
   ADCMacro8bnd
NEWSYM COp75m16nd ; ADC d,x
   addr_dCx_16br
   ADCMacro16bnd
NEWSYM COp75m8d   ; ADC d,x
   addr_dCx_8br
   ADCMacro8bd
NEWSYM COp75m16d  ; ADC d,x
   addr_dCx_16br
   ADCMacro16bd
NEWSYM COp77m8nd  ; ADC [d],y
   addr_LdLCy_8br
   ADCMacro8bnd
NEWSYM COp77m16nd ; ADC [d],y
   addr_LdLCy_16br
   ADCMacro16bnd
NEWSYM COp77m8d   ; ADC [d],y
   addr_LdLCy_8br
   ADCMacro8bd
NEWSYM COp77m16d  ; ADC [d],y
   addr_LdLCy_16br
   ADCMacro16bd
NEWSYM COp79m8nd  ; ADC a,y
   addr_aCy_8br
   ADCMacro8bnd
NEWSYM COp79m16nd ; ADC a,y
   addr_aCy_16br
   ADCMacro16bnd
NEWSYM COp79m8d   ; ADC a,y
   addr_aCy_8br
   ADCMacro8bd
NEWSYM COp79m16d  ; ADC a,y
   addr_aCy_16br
   ADCMacro16bd
NEWSYM COp7Dm8nd  ; ADC a,x
   addr_aCx_8br
   ADCMacro8bnd
NEWSYM COp7Dm16nd ; ADC a,x
   addr_aCx_16br
   ADCMacro16bnd
NEWSYM COp7Dm8d   ; ADC a,x
   addr_aCx_8br
   ADCMacro8bd
NEWSYM COp7Dm16d  ; ADC a,x
   addr_aCx_16br
   ADCMacro16bd
NEWSYM COp7Fm8nd  ; ADC al,x
   addr_alCx_8br
   ADCMacro8bnd
NEWSYM COp7Fm16nd ; ADC al,x
   addr_alCx_16br
   ADCMacro16bnd
NEWSYM COp7Fm8d   ; ADC al,x
   addr_alCx_8br
   ADCMacro8bd
NEWSYM COp7Fm16d  ; ADC al,x
   addr_alCx_16br
   ADCMacro16bd

NEWSYM COp21m8 ; AND(d,x)
   addr_BdCxB_8br
   ANDMacro8b
NEWSYM COp21m16; AND(d,x)
   addr_BdCxB_16br
   ANDMacro16b
NEWSYM COp23m8 ; AND d,s
   addr_dCs_8br
   ANDMacro8b
NEWSYM COp23m16; AND d,s
   addr_dCs_16br
   ANDMacro16b
NEWSYM COp25m8 ; AND d
   addr_d_8br
   ANDMacro8b
NEWSYM COp25m16; AND d
   addr_d_16br
   ANDMacro16b
NEWSYM COp27m8 ; AND [d]
   addr_LdL_8br
   ANDMacro8b
NEWSYM COp27m16; AND [d]
   addr_LdL_16br
   ANDMacro16b
NEWSYM COp29m8 ; AND #
   addr_I_8br
   ANDMacro8b
NEWSYM COp29m16; AND #
   addr_I_16br
   ANDMacro16b
NEWSYM COp2Dm8 ; AND a
   addr_a_8br
   ANDMacro8b
NEWSYM COp2Dm16; AND a
   addr_a_16br
   ANDMacro16b
NEWSYM COp2Fm8 ; AND al
   addr_al_8br
   ANDMacro8b
NEWSYM COp2Fm16; AND al
   addr_al_16br
   ANDMacro16b
NEWSYM COp31m8 ; AND(d),y
   addr_BdBCy_8br
   ANDMacro8b
NEWSYM COp31m16; AND(d),y
   addr_BdBCy_16br
   ANDMacro16b
NEWSYM COp32m8 ; AND (d)
   addr_BdB_8br
   ANDMacro8b
NEWSYM COp32m16; AND (d)
   addr_BdB_16br
   ANDMacro16b
NEWSYM COp33m8 ; AND(d,s),y
   addr_BdCsBCy_8br
   ANDMacro8b
NEWSYM COp33m16; AND(d,s),y
   addr_BdCsBCy_16br
   ANDMacro16b
NEWSYM COp35m8 ; AND d,x
   addr_dCx_8br
   ANDMacro8b
NEWSYM COp35m16; AND d,x
   addr_dCx_16br
   ANDMacro16b
NEWSYM COp37m8 ; AND [d],y
   addr_LdLCy_8br
   ANDMacro8b
NEWSYM COp37m16; AND [d],y
   addr_LdLCy_16br
   ANDMacro16b
NEWSYM COp39m8 ; AND a,y
   addr_aCy_8br
   ANDMacro8b
NEWSYM COp39m16; AND a,y
   addr_aCy_16br
   ANDMacro16b
NEWSYM COp3Dm8 ; AND a,x
   addr_aCx_8br
   ANDMacro8b
NEWSYM COp3Dm16; AND a,x
   addr_aCx_16br
   ANDMacro16b
NEWSYM COp3Fm8 ; AND al,x
   addr_alCx_8br
   ANDMacro8b
NEWSYM COp3Fm16; AND al,x
   addr_alCx_16br
   ANDMacro16b

NEWSYM COp06m8 ; ASL d
   addr_d_8brni
   ASLMacro8b
   addr_d_8bw
   endloop
NEWSYM COp06m16; ASL d
   addr_d_16brni
   ASLMacro16b
   addr_d_16bw
   endloop
NEWSYM COp0Am8 ; ASL A
   addr_A_8brni
   ASLMacro8b
   addr_A_8bw
   endloop
NEWSYM COp0Am16; ASL A
   addr_A_16brni
   ASLMacro16b
   addr_A_16bw
   endloop
NEWSYM COp0Em8 ; ASL a
   addr_a_8brni
   ASLMacro8b
   addr_a_8bw
   endloop
NEWSYM COp0Em16; ASL a
   addr_a_16brni
   ASLMacro16b
   addr_a_16bw
   endloop
NEWSYM COp16m8 ; ASL d,x
   addr_dCx_8brni
   ASLMacro8b
   addr_dCx_8bw
   endloop
NEWSYM COp16m16; ASL d,x
   addr_dCx_16brni
   ASLMacro16b
   addr_dCx_16bw
   endloop
NEWSYM COp1Em8 ; ASL a,x
   addr_aCx_8brni
   ASLMacro8b
   addr_aCx_8bw
   endloop
NEWSYM COp1Em16; ASL a,x
   addr_aCx_16brni
   ASLMacro16b
   addr_aCx_16bw
   endloop

NEWSYM COp90   ; BCC r
    test byte[flagc],01h
    jnz .skip
    JumpMacro
NEWSYM COpB0   ; BCS r
    test byte[flagc],01h
    jz .skip
    JumpMacro
NEWSYM COpF0   ; BEQ r
    test dword[flagnz],0FFFFh
    jnz .skip
    JumpMacro
NEWSYM COp30   ; BMI r
    test dword[flagnz],018000h
    jz .skip
    JumpMacro
NEWSYM COpD0   ; BNE r
    test dword[flagnz],0FFFFh
    jz .skip
    JumpMacro
NEWSYM COp10   ; BPL r
    test dword[flagnz],018000h
    jnz .skip
    JumpMacro
NEWSYM COp80   ; BRA r
    movsx eax,byte[esi]
    inc esi
    add esi,eax
    endloop
NEWSYM COp50   ; BVC r
    test dword[flago],0FFh
    jnz .skip
    JumpMacro
NEWSYM COp70   ; BVS r
    test dword[flago],0FFh
    jz .skip
    JumpMacro

;  BIT instruction does not affect N and V flags when using immediate
;  addressing mode. When using other addressing modes, the N and V flags

NEWSYM COp89m8 ; BIT #
   BITMacroim8b
NEWSYM COp89m16; BIT #
   BITMacroim16b

NEWSYM COp24m8 ; BIT d
   addr_d_8br
   BITMacro8b
NEWSYM COp24m16; BIT d
   addr_d_16br
   BITMacro16b
NEWSYM COp2Cm8 ; BIT a
   addr_a_8br
   BITMacro8b
NEWSYM COp2Cm16; BIT a
   addr_a_16br
   BITMacro16b
NEWSYM COp34m8 ; BIT d,x
   addr_dCx_8br
   BITMacro8b
NEWSYM COp34m16; BIT d,x
   addr_dCx_16br
   BITMacro16b
NEWSYM COp3Cm8 ; BIT a,x
   addr_aCx_8br
   BITMacro8b
NEWSYM COp3Cm16; BIT a,x
   addr_aCx_16br
   BITMacro16b

NEWSYM COp00   ; BRK s
   BRKMacro

NEWSYM COp82   ; BRL rl
   BRLMacro

NEWSYM COp18   ; CLC i
    mov dword[flagc],0
    endloop
NEWSYM COpD8  ; CLD i
    and dl,0F7h
    mov bl,dl
    mov edi,[tablead+ebx*4]
    endloop
NEWSYM COp58   ; CLI i
    and dl,0FBh
    cmp byte[doirqnext],0
    je .noirqexec
    mov byte[doirqnext],0
    call switchtovirqret
.noirqexec
    endloop
NEWSYM COpB8   ; CLV i
    mov dword[flago],0
    endloop

NEWSYM COpC1m8 ; CMP(d,x)
   addr_BdCxB_8br
   CMPMacro8b
NEWSYM COpC1m16; CMP(d,x)
   addr_BdCxB_16br
   CMPMacro16b
NEWSYM COpC3m8 ; CMP d,s
   addr_dCs_8br
   CMPMacro8b
NEWSYM COpC3m16; CMP d,s
   addr_dCs_16br
   CMPMacro16b
NEWSYM COpC5m8 ; CMP d
   addr_d_8br
   CMPMacro8b
NEWSYM COpC5m16; CMP d
   addr_d_16br
   CMPMacro16b
NEWSYM COpC7m8 ; CMP [d]
   addr_LdL_8br
   CMPMacro8b
NEWSYM COpC7m16; CMP [d]
   addr_LdL_16br
   CMPMacro16b
NEWSYM COpC9m8 ; CMP #
   addr_I_8br
   CMPMacro8b
NEWSYM COpC9m16; CMP #
   addr_I_16br
   CMPMacro16b
NEWSYM COpCDm8 ; CMP a
   addr_a_8br
   CMPMacro8b
NEWSYM COpCDm16; CMP a
   addr_a_16br
   CMPMacro16b
NEWSYM COpCFm8 ; CMP al
   addr_al_8br
   CMPMacro8b
NEWSYM COpCFm16; CMP al
   addr_al_16br
   CMPMacro16b
NEWSYM COpD1m8 ; CMP(d),y
   addr_BdBCy_8br
   CMPMacro8b
NEWSYM COpD1m16; CMP(d),y
   addr_BdBCy_16br
   CMPMacro16b
NEWSYM COpD2m8 ; CMP (d)
   addr_BdB_8br
   CMPMacro8b
NEWSYM COpD2m16; CMP (d)
   addr_BdB_16br
   CMPMacro16b
NEWSYM COpD3m8 ; CMP(d,s),y
   addr_BdCsBCy_8br
   CMPMacro8b
NEWSYM COpD3m16; CMP(d,s),y
   addr_BdCsBCy_16br
   CMPMacro16b
NEWSYM COpD5m8 ; CMP d,x
   addr_dCx_8br
   CMPMacro8b
NEWSYM COpD5m16; CMP d,x
   addr_dCx_16br
   CMPMacro16b
NEWSYM COpD7m8 ; CMP [d],y
   addr_LdLCy_8br
   CMPMacro8b
NEWSYM COpD7m16; CMP [d],y
   addr_LdLCy_16br
   CMPMacro16b
NEWSYM COpD9m8 ; CMP a,y
   addr_aCy_8br
   CMPMacro8b
NEWSYM COpD9m16; CMP a,y
   addr_aCy_16br
   CMPMacro16b
NEWSYM COpDDm8 ; CMP a,x
   addr_aCx_8br
   CMPMacro8b
NEWSYM COpDDm16; CMP a,x
   addr_aCx_16br
   CMPMacro16b
NEWSYM COpDFm8 ; CMP al,x
   addr_alCx_8br
   CMPMacro8b
NEWSYM COpDFm16; CMP al,x
   addr_alCx_16br
   CMPMacro16b

NEWSYM COp02   ; COP s
   COPMacro

NEWSYM COpE0x8 ; CPX #
   addr_I_8br
   CPXMacro8b
NEWSYM COpE0x16; CPX #
   addr_I_16br
   CPXMacro16b
NEWSYM COpE4x8 ; CPX d
   addr_d_8br
   CPXMacro8b
NEWSYM COpE4x16; CPX d
   addr_d_16br
   CPXMacro16b
NEWSYM COpECx8 ; CPX a
   addr_a_8br
   CPXMacro8b
NEWSYM COpECx16; CPX a
   addr_a_16br
   CPXMacro16b

NEWSYM COpC0x8 ; CPY #
   addr_I_8br
   CPYMacro8b
NEWSYM COpC0x16; CPY #
   addr_I_16br
   CPYMacro16b
NEWSYM COpC4x8 ; CPY d
   addr_d_8br
   CPYMacro8b
NEWSYM COpC4x16; CPY d
   addr_d_16br
   CPYMacro16b
NEWSYM COpCCx8 ; CPY a
   addr_a_8br
   CPYMacro8b
NEWSYM COpCCx16; CPY a
   addr_a_16br
   CPYMacro16b

NEWSYM COp3Am8 ; DEC A
    addr_A_8brni
    dec al
    flagsetnz8bnel
    addr_A_8bw
    endloop
NEWSYM COp3Am16; DEC A
    addr_A_16brni
    dec ax
    flagsetnz16bnel
    addr_A_16bw
    endloop
NEWSYM COpCEm8 ; DEC a
    addr_a_8brni
    dec al
    flagsetnz8bnel
    addr_a_8bw
    endloop
NEWSYM COpCEm16; DEC a
    addr_a_16brni
    dec ax
    flagsetnz16bnel
    addr_a_16bw
    endloop
NEWSYM COpC6m8 ; DEC d
    addr_d_8brni
    dec al
    flagsetnz8bnel
    addr_d_8bw
    endloop
NEWSYM COpC6m16; DEC d
    addr_d_16brni
    dec ax
    flagsetnz16bnel
    addr_d_16bw
    endloop
NEWSYM COpD6m8 ; DEC d,x
    addr_dCx_8brni
    dec al
    flagsetnz8bnel
    addr_dCx_8bw
    endloop
NEWSYM COpD6m16; DEC d,x
    addr_dCx_16brni
    dec ax
    flagsetnz16bnel
    addr_dCx_16bw
    endloop
NEWSYM COpDEm8 ; DEC a,x
    addr_aCx_8brni
    dec al
    flagsetnz8bnel
    addr_aCx_8bw
    endloop
NEWSYM COpDEm16; DEC a,x
    addr_aCx_16brni
    dec ax
    flagsetnz16bnel
    addr_aCx_16bw
    endloop

NEWSYM COpCAx8 ; DEX i
    dec byte[xx]
    mov al,[xx]
    flagsetnz8b
    endloop
NEWSYM COpCAx16; DEX i
    dec word[xx]
    mov ax,[xx]
    flagsetnz16b
    endloop
NEWSYM COp88x8 ; DEY i
    dec byte[xy]
    mov al,[xy]
    flagsetnz8b
    endloop
NEWSYM COp88x16; DEY i
    dec word[xy]
    mov ax,[xy]
    flagsetnz16b
    endloop

NEWSYM COp41m8 ; EOR(d,x)
   addr_BdCxB_8br
   EORMacro8b
NEWSYM COp41m16; EOR(d,x)
   addr_BdCxB_16br
   EORMacro16b
NEWSYM COp43m8 ; EOR d,s
   addr_dCs_8br
   EORMacro8b
NEWSYM COp43m16; EOR d,s
   addr_dCs_16br
   EORMacro16b
NEWSYM COp45m8 ; EOR d
   addr_d_8br
   EORMacro8b
NEWSYM COp45m16; EOR d
   addr_d_16br
   EORMacro16b
NEWSYM COp47m8 ; EOR [d]
   addr_LdL_8br
   EORMacro8b
NEWSYM COp47m16; EOR [d]
   addr_LdL_16br
   EORMacro16b
NEWSYM COp49m8 ; EOR #
   addr_I_8br
   EORMacro8b
NEWSYM COp49m16; EOR #
   addr_I_16br
   EORMacro16b
NEWSYM COp4Dm8 ; EOR a
   addr_a_8br
   EORMacro8b
NEWSYM COp4Dm16; EOR a
   addr_a_16br
   EORMacro16b
NEWSYM COp4Fm8 ; EOR al
   addr_al_8br
   EORMacro8b
NEWSYM COp4Fm16; EOR al
   addr_al_16br
   EORMacro16b
NEWSYM COp51m8 ; EOR(d),y
   addr_BdBCy_8br
   EORMacro8b
NEWSYM COp51m16; EOR(d),y
   addr_BdBCy_16br
   EORMacro16b
NEWSYM COp52m8 ; EOR (d)
   addr_BdB_8br
   EORMacro8b
NEWSYM COp52m16; EOR (d)
   addr_BdB_16br
   EORMacro16b
NEWSYM COp53m8 ; EOR(d,s),y
   addr_BdCsBCy_8br
   EORMacro8b
NEWSYM COp53m16; EOR(d,s),y
   addr_BdCsBCy_16br
   EORMacro16b
NEWSYM COp55m8 ; EOR d,x
   addr_dCx_8br
   EORMacro8b
NEWSYM COp55m16; EOR d,x
   addr_dCx_16br
   EORMacro16b
NEWSYM COp57m8 ; EOR [d],y
   addr_LdLCy_8br
   EORMacro8b
NEWSYM COp57m16; EOR [d],y
   addr_LdLCy_16br
   EORMacro16b
NEWSYM COp59m8 ; EOR a,y
   addr_aCy_8br
   EORMacro8b
NEWSYM COp59m16; EOR a,y
   addr_aCy_16br
   EORMacro16b
NEWSYM COp5Dm8 ; EORa,x
   addr_aCx_8br
   EORMacro8b
NEWSYM COp5Dm16; EORa,x
   addr_aCx_16br
   EORMacro16b
NEWSYM COp5Fm8 ; EOR al,x
   addr_alCx_8br
   EORMacro8b
NEWSYM COp5Fm16; EOR al,x
   addr_alCx_16br
   EORMacro16b


NEWSYM COp1Am8 ; INC A
    addr_A_8brni
    inc al
    flagsetnz8bnel
    addr_A_8bw
    endloop
NEWSYM COp1Am16; INC A
    addr_A_16brni
    inc ax
    flagsetnz16bnel
    addr_A_16bw
    endloop
NEWSYM COpEEm8 ; INC a
    addr_a_8brni
    inc al
    flagsetnz8bnel
    addr_a_8bw
    endloop
NEWSYM COpEEm16; INC a
    addr_a_16brni
    inc ax
    flagsetnz16bnel
    addr_a_16bw
    endloop
NEWSYM COpE6m8 ; INC d
    addr_d_8brni
    inc al
    flagsetnz8bnel
    addr_d_8bw
    endloop
NEWSYM COpE6m16; INC d
    addr_d_16brni
    inc ax
    flagsetnz16bnel
    addr_d_16bw
    endloop
NEWSYM COpF6m8 ; INC d,x
    addr_dCx_8brni
    inc al
    flagsetnz8bnel
    addr_dCx_8bw
    endloop
NEWSYM COpF6m16; INC d,x
    addr_dCx_16brni
    inc ax
    flagsetnz16bnel
    addr_dCx_16bw
    endloop
NEWSYM COpFEm8 ; INC a,x
    addr_aCx_8brni
    inc al
    flagsetnz8bnel
    addr_aCx_8bw
    endloop
NEWSYM COpFEm16; INC a,x
    addr_aCx_16brni
    inc ax
    flagsetnz16bnel
    addr_aCx_16bw
    endloop


NEWSYM COpE8x8 ; INX i
    inc byte[xx]
    mov al,[xx]
    flagsetnz8b
    endloop
NEWSYM COpE8x16; INX i
    inc word[xx]
    mov ax,[xx]
    flagsetnz16b
    endloop
NEWSYM COpC8x8 ; INY i
    inc byte[xy]
    mov al,[xy]
    flagsetnz8b
    endloop
NEWSYM COpC8x16; INY i
    inc word[xy]
    mov ax,[xy]
    flagsetnz16b
    endloop

NEWSYM COpDC   ; JML (a)
    JMLMacro
NEWSYM COp4C   ; JMP a
    JMPMacro4C
NEWSYM COp6C   ; JMP (a)
    JMPMacro6C
NEWSYM COp7C   ; JMP(a,x)
    JMPMacro7C
NEWSYM COp5C   ; JMP al
    JMPMacro5C

NEWSYM COp22   ; JSL al
   JSLMacro
NEWSYM COp20   ; JSR a
   JSRMacro20
NEWSYM COpFC   ; JSR(a,x)
   JSRMacroFC

NEWSYM COpA1m8 ; LDA(d,x)
   addr_BdCxB_8br
   LDAMacro8b
NEWSYM COpA1m16; LDA(d,x)
   addr_BdCxB_16br
   LDAMacro16b
NEWSYM COpA3m8 ; LDA d,s
   addr_dCs_8br
   LDAMacro8b
NEWSYM COpA3m16; LDA d,s
   addr_dCs_16br
   LDAMacro16b
NEWSYM COpA5m8 ; LDA d
   addr_d_8br
   LDAMacro8b
NEWSYM COpA5m16; LDA d
   addr_d_16br
   LDAMacro16b
NEWSYM COpA7m8 ; LDA [d]
   addr_LdL_8br
   LDAMacro8b
NEWSYM COpA7m16; LDA [d]
   addr_LdL_16br
   LDAMacro16b
NEWSYM COpA9m8 ; LDA #
   addr_I_8br
   LDAMacro8b
NEWSYM COpA9m16; LDA #
   addr_I_16br
   LDAMacro16b
NEWSYM COpADm8 ; LDA a
   addr_a_8br
   LDAMacro8b
NEWSYM COpADm16; LDA a
   addr_a_16br
   LDAMacro16b
NEWSYM COpAFm8 ; LDA al
   addr_al_8br
   LDAMacro8b
NEWSYM COpAFm16; LDA al
   addr_al_16br
   LDAMacro16b
NEWSYM COpB1m8 ; LDA(d),y
   addr_BdBCy_8br
   LDAMacro8b
NEWSYM COpB1m16; LDA(d),y
   addr_BdBCy_16br
   LDAMacro16b
NEWSYM COpB2m8 ; LDA (d)
   addr_BdB_8br
   LDAMacro8b
NEWSYM COpB2m16; LDA (d)
   addr_BdB_16br
   LDAMacro16b
NEWSYM COpB3m8 ; LDA(d,s),y
   addr_BdCsBCy_8br
   LDAMacro8b
NEWSYM COpB3m16; LDA(d,s),y
   addr_BdCsBCy_16br
   LDAMacro16b
NEWSYM COpB5m8 ; LDA d,x
   addr_dCx_8br
   LDAMacro8b
NEWSYM COpB5m16; LDA d,x
   addr_dCx_16br
   LDAMacro16b
NEWSYM COpB7m8 ; LDA [d],y
   addr_LdLCy_8br
   LDAMacro8b
NEWSYM COpB7m16; LDA [d],y
   addr_LdLCy_16br
   LDAMacro16b
NEWSYM COpB9m8 ; LDA a,y
   addr_aCy_8br
   LDAMacro8b
NEWSYM COpB9m16; LDA a,y
   addr_aCy_16br
   LDAMacro16b
NEWSYM COpBDm8 ; LDA a,x
   addr_aCx_8br
   LDAMacro8b
NEWSYM COpBDm16; LDA a,x
   addr_aCx_16br
   LDAMacro16b
NEWSYM COpBFm8 ; LDA al,x
   addr_alCx_8br
   LDAMacro8b
NEWSYM COpBFm16; LDA al,x
   addr_alCx_16br
   LDAMacro16b

NEWSYM COpA2x8 ; LDX #
   addr_I_8br
   LDXMacro8b
NEWSYM COpA2x16; LDX #
   addr_I_16br
   LDXMacro16b
NEWSYM COpA6x8 ; LDX d
   addr_d_8br
   LDXMacro8b
NEWSYM COpA6x16; LDX d
   addr_d_16br
   LDXMacro16b
NEWSYM COpAEx8 ; LDX a
   addr_a_8br
   LDXMacro8b
NEWSYM COpAEx16; LDX a
   addr_a_16br
   LDXMacro16b
NEWSYM COpB6x8 ; LDX d,y
   addr_dCy_8br
   LDXMacro8b
NEWSYM COpB6x16; LDX d,y
   addr_dCy_16br
   LDXMacro16b
NEWSYM COpBEx8 ; LDX a,y
   addr_aCy_8br
   LDXMacro8b
NEWSYM COpBEx16; LDX a,y
   addr_aCy_16br
   LDXMacro16b

NEWSYM COpA0x8 ; LDY #
   addr_I_8br
   LDYMacro8b
NEWSYM COpA0x16; LDY #
   addr_I_16br
   LDYMacro16b
NEWSYM COpA4x8 ; LDY d
   addr_d_8br
   LDYMacro8b
NEWSYM COpA4x16; LDY d
   addr_d_16br
   LDYMacro16b
NEWSYM COpACx8 ; LDY a
   addr_a_8br
   LDYMacro8b
NEWSYM COpACx16; LDY a
   addr_a_16br
   LDYMacro16b
NEWSYM COpB4x8 ; LDY d,x
   addr_dCx_8br
   LDYMacro8b
NEWSYM COpB4x16; LDY d,x
   addr_dCx_16br
   LDYMacro16b
NEWSYM COpBCx8 ; LDY a,x
   addr_aCx_8br
   LDYMacro8b
NEWSYM COpBCx16; LDY a,x
   addr_aCx_16br
   LDYMacro16b

NEWSYM COp46m8 ; LSR d
   addr_d_8brni
   LSRMacro8b
   addr_d_8bw
   endloop
NEWSYM COp46m16; LSR d
   addr_d_16brni
   LSRMacro16b
   addr_d_16bw
   endloop
NEWSYM COp4Am8 ; LSR A
   addr_A_8brni
   LSRMacro8b
   addr_A_8bw
   endloop
NEWSYM COp4Am16; LSR A
   addr_A_16brni
   LSRMacro16b
   addr_A_16bw
   endloop
NEWSYM COp4Em8 ; LSR a
   addr_a_8brni
   LSRMacro8b
   addr_a_8bw
   endloop
NEWSYM COp4Em16; LSR a
   addr_a_16brni
   LSRMacro16b
   addr_a_16bw
   endloop
NEWSYM COp56m8 ; LSR d,x
   addr_dCx_8brni
   LSRMacro8b
   addr_dCx_8bw
   endloop
NEWSYM COp56m16; LSR d,x
   addr_dCx_16brni
   LSRMacro16b
   addr_dCx_16bw
   endloop
NEWSYM COp5Em8 ; LSRa,x
   addr_aCx_8brni
   LSRMacro8b
   addr_aCx_8bw
   endloop
NEWSYM COp5Em16; LSRa,x
   addr_aCx_16brni
   LSRMacro16b
   addr_aCx_16bw
   endloop

NEWSYM COp54   ; MVN xya
   MVNMacro
NEWSYM COp44   ; MVP xya
   MVPMacro

NEWSYM COpEA   ; NOP i
   endloop

NEWSYM COp01m8 ; ORA(d,x)
   addr_BdCxB_8br
   ORAMacro8b
NEWSYM COp01m16; ORA(d,x)
   addr_BdCxB_16br
   ORAMacro16b
NEWSYM COp03m8 ; ORA d,s
   addr_dCs_8br
   ORAMacro8b
NEWSYM COp03m16; ORA d,s
   addr_dCs_16br
   ORAMacro16b
NEWSYM COp05m8 ; ORA d
   addr_d_8br
   ORAMacro8b
NEWSYM COp05m16; ORA d
   addr_d_16br
   ORAMacro16b
NEWSYM COp07m8 ; ORA [d]
   addr_LdL_8br
   ORAMacro8b
NEWSYM COp07m16; ORA [d]
   addr_LdL_16br
   ORAMacro16b
NEWSYM COp09m8 ; ORA #
   addr_I_8br
   ORAMacro8b
NEWSYM COp09m16; ORA #
   addr_I_16br
   ORAMacro16b
NEWSYM COp0Dm8 ; ORA a
   addr_a_8br
   ORAMacro8b
NEWSYM COp0Dm16; ORA a
   addr_a_16br
   ORAMacro16b
NEWSYM COp0Fm8 ; ORA al
   addr_al_8br
   ORAMacro8b
NEWSYM COp0Fm16; ORA al
   addr_al_16br
   ORAMacro16b
NEWSYM COp11m8 ; ORA(d),y
   addr_BdBCy_8br
   ORAMacro8b
NEWSYM COp11m16; ORA(d),y
   addr_BdBCy_16br
   ORAMacro16b
NEWSYM COp12m8 ; ORA(d)
   addr_BdB_8br
   ORAMacro8b
NEWSYM COp12m16; ORA(d)
   addr_BdB_16br
   ORAMacro16b
NEWSYM COp13m8 ; ORA(d,s),y
   addr_BdCsBCy_8br
   ORAMacro8b
NEWSYM COp13m16; ORA(d,s),y
   addr_BdCsBCy_16br
   ORAMacro16b
NEWSYM COp15m8 ; ORA d,x
   addr_dCx_8br
   ORAMacro8b
NEWSYM COp15m16; ORA d,x
   addr_dCx_16br
   ORAMacro16b
NEWSYM COp17m8 ; ORA [d],y
   addr_LdLCy_8br
   ORAMacro8b
NEWSYM COp17m16; ORA [d],y
   addr_LdLCy_16br
   ORAMacro16b
NEWSYM COp19m8 ; ORA a,y
   addr_aCy_8br
   ORAMacro8b
NEWSYM COp19m16; ORA a,y
   addr_aCy_16br
   ORAMacro16b
NEWSYM COp1Dm8 ; ORA a,x
   addr_aCx_8br
   ORAMacro8b
NEWSYM COp1Dm16; ORA a,x
   addr_aCx_16br
   ORAMacro16b
NEWSYM COp1Fm8 ; ORA al,x
   addr_alCx_8br
   ORAMacro8b
NEWSYM COp1Fm16; ORA al,x
   addr_alCx_16br
   ORAMacro16b

NEWSYM COpF4   ; PEA s
   PEAMacro
NEWSYM COpD4   ; PEI s
   PEIMacro
NEWSYM COp62   ; PER s
   PERMacro

NEWSYM COp48m8 ; PHA s
   PUSHMacro8b [xa]
NEWSYM COp48m16; PHA s
   PUSHMacro16b [xa], [xa+1]
NEWSYM COp8B   ; PHB s
   PUSHMacro8b [xdb]
NEWSYM COp0B   ; PHD s
   PUSHMacro16b [xd], [xd+1]
NEWSYM COp4B   ; PHK s
   PUSHMacro8b [xpb]
NEWSYM COp08   ; PHP s
   PUSHMacro8bp
NEWSYM COpDAx8 ; PHX s
   PUSHMacro8b [xx]
NEWSYM COpDAx16; PHX s
   PUSHMacro16b [xx], [xx+1]
NEWSYM COp5Ax8 ; PHY s
   PUSHMacro8b [xy]
NEWSYM COp5Ax16; PHY s
   PUSHMacro16b [xy], [xy+1]

NEWSYM COp68m8 ; PLA s
   POPMacro8b [xa]
NEWSYM COp68m16; PLA s
   POPMacro16b [xa], [xa+1]
NEWSYM COpAB   ; PLB s
   POPMacro8b [xdb]
NEWSYM COp2B   ; PLD s
   POPMacro16bd [xd], [xd+1]
NEWSYM COp28   ; PLP s
   POPMacroP
NEWSYM COpFAx8 ; PLX s
   POPMacro8b [xx]
NEWSYM COpFAx16; PLX s
   POPMacro16b [xx], [xx+1]
NEWSYM COp7Ax8 ; PLY s
   POPMacro8b [xy]
NEWSYM COp7Ax16; PLY s
   POPMacro16b [xy], [xy+1]

NEWSYM COpC2   ; REP #
   REPMacro

NEWSYM COp26m8 ; ROL d
   addr_d_8brni
   ROLMacro8b
   addr_d_8bw
   endloop
NEWSYM COp26m16; ROL d
   addr_d_16brni
   ROLMacro16b
   addr_d_16bw
   endloop
NEWSYM COp2Am8 ; ROL A
   addr_A_8brni
   ROLMacro8b
   addr_A_8bw
   endloop
NEWSYM COp2Am16; ROL A
   addr_A_16brni
   ROLMacro16b
   addr_A_16bw
   endloop
NEWSYM COp2Em8 ; ROL a
   addr_a_8brni
   ROLMacro8b
   addr_a_8bw
   endloop
NEWSYM COp2Em16; ROL a
   addr_a_16brni
   ROLMacro16b
   addr_a_16bw
   endloop
NEWSYM COp36m8 ; ROL d,x
   addr_dCx_8brni
   ROLMacro8b
   addr_dCx_8bw
   endloop
NEWSYM COp36m16; ROL d,x
   addr_dCx_16brni
   ROLMacro16b
   addr_dCx_16bw
   endloop
NEWSYM COp3Em8 ; ROL a,x
   addr_aCx_8brni
   ROLMacro8b
   addr_aCx_8bw
   endloop
NEWSYM COp3Em16; ROL a,x
   addr_aCx_16brni
   ROLMacro16b
   addr_aCx_16bw
   endloop

NEWSYM COp66m8 ; ROR d
   addr_d_8brni
   RORMacro8b
   addr_d_8bw
   endloop
NEWSYM COp66m16; ROR d
   addr_d_16brni
   RORMacro16b
   addr_d_16bw
   endloop
NEWSYM COp6Am8 ; ROR A
   addr_A_8brni
   RORMacro8b
   addr_A_8bw
   endloop
NEWSYM COp6Am16; ROR A
   addr_A_16brni
   RORMacro16b
   addr_A_16bw
   endloop
NEWSYM COp6Em8 ; ROR a
   addr_a_8brni
   RORMacro8b
   addr_a_8bw
   endloop
NEWSYM COp6Em16; ROR a
   addr_a_16brni
   RORMacro16b
   addr_a_16bw
   endloop
NEWSYM COp76m8 ; ROR d,x
   addr_dCx_8brni
   RORMacro8b
   addr_dCx_8bw
   endloop
NEWSYM COp76m16; ROR d,x
   addr_dCx_16brni
   RORMacro16b
   addr_dCx_16bw
   endloop
NEWSYM COp7Em8 ; ROR a,x
   addr_aCx_8brni
   RORMacro8b
   addr_aCx_8bw
   endloop
NEWSYM COp7Em16; ROR a,x
   addr_aCx_16brni
   RORMacro16b
   addr_aCx_16bw
   endloop

NEWSYM COp40   ; RTI s
   RTIMacro
NEWSYM COp6B   ; RTL s
   RTLMacro
NEWSYM COp60   ; RTS s
   RTSMacro

NEWSYM COpE1m8nd  ; SBC(d,x)
   addr_BdCxB_8br
   SBCMacro8bnd
NEWSYM COpE1m16nd ; SBC(d,x)
   addr_BdCxB_16br
   SBCMacro16bnd
NEWSYM COpE1m8d   ; SBC(d,x)
   addr_BdCxB_8br
   SBCMacro8bd
NEWSYM COpE1m16d  ; SBC(d,x)
   addr_BdCxB_16br
   SBCMacro16bd
NEWSYM COpE3m8nd  ; SBC d,s
   addr_dCs_8br
   SBCMacro8bnd
NEWSYM COpE3m16nd ; SBC d,s
   addr_dCs_16br
   SBCMacro16bnd
NEWSYM COpE3m8d   ; SBC d,s
   addr_dCs_8br
   SBCMacro8bd
NEWSYM COpE3m16d  ; SBC d,s
   addr_dCs_16br
   SBCMacro16bd
NEWSYM COpE5m8nd  ; SBC d
   addr_d_8br
   SBCMacro8bnd
NEWSYM COpE5m16nd ; SBC d
   addr_d_16br
   SBCMacro16bnd
NEWSYM COpE5m8d   ; SBC d
   addr_d_8br
   SBCMacro8bd
NEWSYM COpE5m16d  ; SBC d
   addr_d_16br
   SBCMacro16bd
NEWSYM COpE7m8nd  ; SBC [d]
   addr_LdL_8br
   SBCMacro8bnd
NEWSYM COpE7m16nd ; SBC [d]
   addr_LdL_16br
   SBCMacro16bnd
NEWSYM COpE7m8d   ; SBC [d]
   addr_LdL_8br
   SBCMacro8bd
NEWSYM COpE7m16d  ; SBC [d]
   addr_LdL_16br
   SBCMacro16bd
NEWSYM COpE9m8nd  ; SBC #
   addr_I_8br
   SBCMacro8bnd
NEWSYM COpE9m16nd ; SBC #
   addr_I_16br
   SBCMacro16bnd
NEWSYM COpE9m8d   ; SBC #
   addr_I_8br
   SBCMacro8bd
NEWSYM COpE9m16d  ; SBC #
   addr_I_16br
   SBCMacro16bd
NEWSYM COpEDm8nd  ; SBC a
   addr_a_8br
   SBCMacro8bnd
NEWSYM COpEDm16nd ; SBC a
   addr_a_16br
   SBCMacro16bnd
NEWSYM COpEDm8d   ; SBC a
   addr_a_8br
   SBCMacro8bd
NEWSYM COpEDm16d  ; SBC a
   addr_a_16br
   SBCMacro16bd
NEWSYM COpEFm8nd  ; SBC al
   addr_al_8br
   SBCMacro8bnd
NEWSYM COpEFm16nd ; SBC al
   addr_al_16br
   SBCMacro16bnd
NEWSYM COpEFm8d   ; SBC al
   addr_al_8br
   SBCMacro8bd
NEWSYM COpEFm16d  ; SBC al
   addr_al_16br
   SBCMacro16bd
NEWSYM COpF1m8nd  ; SBC(d),y
   addr_BdBCy_8br
   SBCMacro8bnd
NEWSYM COpF1m16nd ; SBC(d),y
   addr_BdBCy_16br
   SBCMacro16bnd
NEWSYM COpF1m8d   ; SBC(d),y
   addr_BdBCy_8br
   SBCMacro8bd
NEWSYM COpF1m16d  ; SBC(d),y
   addr_BdBCy_16br
   SBCMacro16bd
NEWSYM COpF2m8nd  ; SBC (d)
   addr_BdB_8br
   SBCMacro8bnd
NEWSYM COpF2m16nd ; SBC (d)
   addr_BdB_16br
   SBCMacro16bnd
NEWSYM COpF2m8d   ; SBC (d)
   addr_BdB_8br
   SBCMacro8bd
NEWSYM COpF2m16d  ; SBC (d)
   addr_BdB_16br
   SBCMacro16bd
NEWSYM COpF3m8nd  ; SBC(d,s),y
   addr_BdCsBCy_8br
   SBCMacro8bnd
NEWSYM COpF3m16nd ; SBC(d,s),y
   addr_BdCsBCy_16br
   SBCMacro16bnd
NEWSYM COpF3m8d   ; SBC(d,s),y
   addr_BdCsBCy_8br
   SBCMacro8bd
NEWSYM COpF3m16d  ; SBC(d,s),y
   addr_BdCsBCy_16br
   SBCMacro16bd
NEWSYM COpF5m8nd  ; SBC d,x
   addr_dCx_8br
   SBCMacro8bnd
NEWSYM COpF5m16nd ; SBC d,x
   addr_dCx_16br
   SBCMacro16bnd
NEWSYM COpF5m8d   ; SBC d,x
   addr_dCx_8br
   SBCMacro8bd
NEWSYM COpF5m16d  ; SBC d,x
   addr_dCx_16br
   SBCMacro16bd
NEWSYM COpF7m8nd  ; SBC [d],y
   addr_LdLCy_8br
   SBCMacro8bnd
NEWSYM COpF7m16nd ; SBC [d],y
   addr_LdLCy_16br
   SBCMacro16bnd
NEWSYM COpF7m8d   ; SBC [d],y
   addr_LdLCy_8br
   SBCMacro8bd
NEWSYM COpF7m16d  ; SBC [d],y
   addr_LdLCy_16br
   SBCMacro16bd
NEWSYM COpF9m8nd  ; SBC a,y
   addr_aCy_8br
   SBCMacro8bnd
NEWSYM COpF9m16nd ; SBC a,y
   addr_aCy_16br
   SBCMacro16bnd
NEWSYM COpF9m8d   ; SBC a,y
   addr_aCy_8br
   SBCMacro8bd
NEWSYM COpF9m16d  ; SBC a,y
   addr_aCy_16br
   SBCMacro16bd
NEWSYM COpFDm8nd  ; SBC a,x
   addr_aCx_8br
   SBCMacro8bnd
NEWSYM COpFDm16nd ; SBC a,x
   addr_aCx_16br
   SBCMacro16bnd
NEWSYM COpFDm8d   ; SBC a,x
   addr_aCx_8br
   SBCMacro8bd
NEWSYM COpFDm16d  ; SBC a,x
   addr_aCx_16br
   SBCMacro16bd
NEWSYM COpFFm8nd  ; SBC al,x
   addr_alCx_8br
   SBCMacro8bnd
NEWSYM COpFFm16nd ; SBC al,x
   addr_alCx_16br
   SBCMacro16bnd
NEWSYM COpFFm8d   ; SBC al,x
   addr_alCx_8br
   SBCMacro8bd
NEWSYM COpFFm16d  ; SBC al,x
   addr_alCx_16br
   SBCMacro16bd

NEWSYM COp38   ; SEC i
    mov dword[flagc],0FFh
    endloop
NEWSYM COpF8   ; SED i
    or dl,08h
    mov bl,dl
    mov edi,[tablead+ebx*4]
    endloop
NEWSYM COp78   ; SEI i
    or dl,04h
    endloop

NEWSYM COpE2   ; SEP #
    SEPMacro

NEWSYM COp81m8 ; STA(d,x)
   STAMacro8b
   addr_BdCxB_8bw
   endloop
NEWSYM COp81m16; STA(d,x)
   STAMacro16b
   addr_BdCxB_16bw
   endloop
NEWSYM COp83m8 ; STA d,s
   STAMacro8b
   addr_dCs_8bw
   endloop
NEWSYM COp83m16; STA d,s
   STAMacro16b
   addr_dCs_16bw
   endloop
NEWSYM COp85m8 ; STA d
   STAMacro8b
   addr_d_8bw
   endloop
NEWSYM COp85m16; STA d
   STAMacro16b
   addr_d_16bw
   endloop
NEWSYM COp87m8 ; STA [d]
   STAMacro8b
   addr_LdL_8bw
   endloop
NEWSYM COp87m16; STA [d]
   STAMacro16b
   addr_LdL_16bw
   endloop
NEWSYM COp8Dm8 ; STA a
   STAMacro8b
   addr_a_8bw
   endloop
NEWSYM COp8Dm16; STA a
   STAMacro16b
   addr_a_16bw
   endloop
NEWSYM COp8Fm8 ; STA al
   STAMacro8b
   addr_al_8bw
   endloop
NEWSYM COp8Fm16; STA al
   STAMacro16b
   addr_al_16bw
   endloop
NEWSYM COp91m8 ; STA(d),y
   STAMacro8b
   addr_BdBCy_8bw
   endloop
NEWSYM COp91m16; STA(d),y
   STAMacro16b
   addr_BdBCy_16bw
   endloop
NEWSYM COp92m8 ; STA (d)
   STAMacro8b
   addr_BdB_8bw
   endloop
NEWSYM COp92m16; STA (d)
   STAMacro16b
   addr_BdB_16bw
   endloop
NEWSYM COp93m8 ; STA(d,s),y
   STAMacro8b
   addr_BdCsBCy_8bw
   endloop
NEWSYM COp93m16; STA(d,s),y
   STAMacro16b
   addr_BdCsBCy_16bw
   endloop
NEWSYM COp95m8 ; STA d,x
   STAMacro8b
   addr_dCx_8bw
   endloop
NEWSYM COp95m16; STA d,x
   STAMacro16b
   addr_dCx_16bw
   endloop
NEWSYM COp97m8 ; STA [d],y
   STAMacro8b
   addr_LdLCy_8bw
   endloop
NEWSYM COp97m16; STA [d],y
   STAMacro16b
   addr_LdLCy_16bw
   endloop
NEWSYM COp99m8 ; STA a,y
   STAMacro8b
   addr_aCy_8bw
   endloop
NEWSYM COp99m16; STA a,y
   STAMacro16b
   addr_aCy_16bw
   endloop
NEWSYM COp9Dm8 ; STA a,x
   STAMacro8b
   addr_aCx_8bw
   endloop
NEWSYM COp9Dm16; STA a,x
   STAMacro16b
   addr_aCx_16bw
   endloop
NEWSYM COp9Fm8 ; STA al,x
   STAMacro8b
   addr_alCx_8bw
   endloop
NEWSYM COp9Fm16; STA al,x
   STAMacro16b
   addr_alCx_16bw
   endloop

NEWSYM COp86x8 ; STX d
   STXMacro8b
   addr_d_8bw
   endloop
NEWSYM COp86x16; STX d
   STXMacro16b
   addr_d_16bw
   endloop
NEWSYM COp8Ex8 ; STX a
   STXMacro8b
   addr_a_8bw
   endloop
NEWSYM COp8Ex16; STX a
   STXMacro16b
   addr_a_16bw
   endloop
NEWSYM COp96x8 ; STX d,y
   STXMacro8b
   addr_dCy_8bw
   endloop
NEWSYM COp96x16; STX d,y
   STXMacro16b
   addr_dCy_16bw
   endloop

NEWSYM COp84x8 ; STY d
   STYMacro8b
   addr_d_8bw
   endloop
NEWSYM COp84x16; STY d
   STYMacro16b
   addr_d_16bw
   endloop
NEWSYM COp8Cx8 ; STY a
   STYMacro8b
   addr_a_8bw
   endloop
NEWSYM COp8Cx16; STY a
   STYMacro16b
   addr_a_16bw
   endloop
NEWSYM COp94x8 ; STYd,x
   STYMacro8b
   addr_dCx_8bw
   endloop
NEWSYM COp94x16; STYd,x
   STYMacro16b
   addr_dCx_16bw
   endloop

NEWSYM COpDB   ; STP i
    dec esi
    endloop

NEWSYM COp64m8 ; STZ d
   xor al,al
   addr_d_8bw
   endloop
NEWSYM COp64m16; STZ d
   xor eax,eax
   addr_d_16bw
   endloop
NEWSYM COp74m8 ; STZ d,x
   xor al,al
   addr_dCx_8bw
   endloop
NEWSYM COp74m16; STZ d,x
   xor eax,eax
   addr_dCx_16bw
   endloop
NEWSYM COp9Cm8 ; STZ a
   xor al,al
   addr_a_8bw
   endloop
NEWSYM COp9Cm16; STZ a
   xor eax,eax
   addr_a_16bw
   endloop
NEWSYM COp9Em8 ; STZ a,x
   xor al,al
   addr_aCx_8bw
   endloop
NEWSYM COp9Em16; STZ a,x
   xor eax,eax
   addr_aCx_16bw
   endloop

NEWSYM COp14m8 ; TRB d
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
NEWSYM COp14m16; TRB d
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
NEWSYM COp1Cm8 ; TRB a
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
NEWSYM COp1Cm16; TRB a
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

NEWSYM COp04m8 ; TSB d
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
NEWSYM COp04m16; TSB d
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
NEWSYM COp0Cm8 ; TSB a
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
NEWSYM COp0Cm16; TSB a
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

NEWSYM COpAAx8 ; TAX i
    mov al,[xa]
    mov [xx],al
    flagsetnz8b
    endloop
NEWSYM COpAAx16; TAX i
    mov ax,[xa]
    mov [xx],ax
    flagsetnz16b
    endloop
NEWSYM COpA8x8 ; TAY i
    mov al,[xa]
    mov [xy],al
    flagsetnz8b
    endloop
NEWSYM COpA8x16; TAY i
    mov ax,[xa]
    mov [xy],ax
    flagsetnz16b
    endloop
NEWSYM COp5B   ; TCD i
    mov ax,[xa]
    mov [xd],ax
    call UpdateDPage
    flagsetnz16b
    endloop
NEWSYM COp1B   ; TCS i
    mov ax,[xa]
    test byte[xe],1
    jnz .emul
    mov [xs],ax
    endloop
.emul
    mov [xs],al
    endloop
NEWSYM COp7B   ; TDC i
    mov ax,[xd]
    mov [xa],ax
    flagsetnz16b
    endloop
NEWSYM COp3B   ; TSC i
    mov ax,[xs]
    mov [xa],ax
    flagsetnz16b
    endloop
NEWSYM COpBAx8 ; TSX i
    mov al,[xs]
    mov [xx],al
    flagsetnz8b
    endloop
NEWSYM COpBAx16; TSX i
    mov ax,[xs]
    mov [xx],ax
    flagsetnz16b
    endloop
NEWSYM COp8Am8 ; TXA i
    mov al,[xx]
    mov [xa],al
    flagsetnz8b
    endloop
NEWSYM COp8Am16; TXA i
    mov ax,[xx]
    mov [xa],ax
    flagsetnz16b
    endloop
NEWSYM COp9A   ; TXS i
    mov ax,[xx]
    mov [xs],ax
    test byte[xe],1
    jnz .emul
    endloop
.emul
    mov byte[xs+1],1
    endloop
NEWSYM COp9Bx8 ; TXY i
    mov al,[xx]
    mov [xy],al
    flagsetnz8b
    endloop
NEWSYM COp9Bx16; TXY i
    mov ax,[xx]
    mov [xy],ax
    flagsetnz16b
    endloop
NEWSYM COp98m8 ; TYA i
    mov al,[xy]
    mov [xa],al
    flagsetnz8b
    endloop
NEWSYM COp98m16; TYA i
    mov ax,[xy]
    mov [xa],ax
    flagsetnz16b
    endloop
NEWSYM COpBBx8 ; TYX i
    mov al,[xy]
    mov [xx],al
    flagsetnz8b
    endloop
NEWSYM COpBBx16; TYX i
    mov ax,[xy]
    mov [xx],ax
    flagsetnz16b
    endloop

NEWSYM COpCB   ; WAI i
    WAIMacro

NEWSYM COpEB   ; XBA i
    mov ah,[xa]
    mov al,[xa+1]
    mov [xa],ax
    flagsetnz8b
    endloop

NEWSYM COpFB   ; XCE i
    XCEMacro

NEWSYM COp42   ; WDM
    inc esi
    endloop

NEWSYM E65816IncEnd


