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
   call nextopcode
   popad
.nodebugger
%endif
   ret




; B = ( or )
; C = ,
; L = [ or ]
; I = #


SA1COp61m8nd:  ; ADC(d,x)
   addr_BdCxB_8br
   ADCMacro8bnd
SA1COp61m16nd: ; ADC(d,x)
   addr_BdCxB_16br
   ADCMacro16bnd
SA1COp61m8d:   ; ADC(d,x)
   addr_BdCxB_8br
   ADCMacro8bd
SA1COp61m16d:  ; ADC(d,x)
   addr_BdCxB_16br
   ADCMacro16bd
SA1COp63m8nd:  ; ADC d,s
   addr_dCs_8br
   ADCMacro8bnd
SA1COp63m16nd: ; ADC d,s
   addr_dCs_16br
   ADCMacro16bnd
SA1COp63m8d:   ; ADC d,s
   addr_dCs_8br
   ADCMacro8bd
SA1COp63m16d:  ; ADC d,s
   addr_dCs_16br
   ADCMacro16bd
SA1COp65m8nd:  ; ADC d
   addr_d_8br
   ADCMacro8bnd
SA1COp65m16nd: ; ADC d
   addr_d_16br
   ADCMacro16bnd
SA1COp65m8d:   ; ADC d
   addr_d_8br
   ADCMacro8bd
SA1COp65m16d:  ; ADC d
   addr_d_16br
   ADCMacro16bd
SA1COp67m8nd:  ; ADC [d]
   addr_LdL_8br
   ADCMacro8bnd
SA1COp67m16nd: ; ADC [d]
   addr_LdL_16br
   ADCMacro16bnd
SA1COp67m8d:   ; ADC [d]
   addr_LdL_8br
   ADCMacro8bd
SA1COp67m16d:  ; ADC [d]
   addr_LdL_16br
   ADCMacro16bd
SA1COp69m8nd:  ; ADC #
   addr_I_8br
   ADCMacro8bnd
SA1COp69m16nd: ; ADC #
   addr_I_16br
   ADCMacro16bnd
SA1COp69m8d:   ; ADC #
   addr_I_8br
   ADCMacro8bd
SA1COp69m16d:  ; ADC #
   addr_I_16br
   ADCMacro16bd
SA1COp6Dm8nd:  ; ADC a
   addr_a_8br
   ADCMacro8bnd
SA1COp6Dm16nd: ; ADC a
   addr_a_16br
   ADCMacro16bnd
SA1COp6Dm8d:   ; ADC a
   addr_a_8br
   ADCMacro8bd
SA1COp6Dm16d:  ; ADC a
   addr_a_16br
   ADCMacro16bd
SA1COp6Fm8nd:  ; ADC al
   addr_al_8br
   ADCMacro8bnd
SA1COp6Fm16nd: ; ADC al
   addr_al_16br
   ADCMacro16bnd
SA1COp6Fm8d:   ; ADC al
   addr_al_8br
   ADCMacro8bd
SA1COp6Fm16d:  ; ADC al
   addr_al_16br
   ADCMacro16bd
SA1COp71m8nd:  ; ADC(d),y
   addr_BdBCy_8br
   ADCMacro8bnd
SA1COp71m16nd: ; ADC(d),y
   addr_BdBCy_16br
   ADCMacro16bnd
SA1COp71m8d:   ; ADC(d),y
   addr_BdBCy_8br
   ADCMacro8bd
SA1COp71m16d:  ; ADC(d),y
   addr_BdBCy_16br
   ADCMacro16bd
SA1COp72m8nd:  ; ADC (d)
   addr_BdB_8br
   ADCMacro8bnd
SA1COp72m16nd: ; ADC (d)
   addr_BdB_16br
   ADCMacro16bnd
SA1COp72m8d:   ; ADC (d)
   addr_BdB_8br
   ADCMacro8bd
SA1COp72m16d:  ; ADC (d)
   addr_BdB_16br
   ADCMacro16bd
SA1COp73m8nd:  ; ADC(d,s),y
   addr_BdCsBCy_8br
   ADCMacro8bnd
SA1COp73m16nd: ; ADC(d,s),y
   addr_BdCsBCy_16br
   ADCMacro16bnd
SA1COp73m8d:   ; ADC(d,s),y
   addr_BdCsBCy_8br
   ADCMacro8bd
SA1COp73m16d:  ; ADC(d,s),y
   addr_BdCsBCy_16br
   ADCMacro16bd
SA1COp75m8nd:  ; ADC d,x
   addr_dCx_8br
   ADCMacro8bnd
SA1COp75m16nd: ; ADC d,x
   addr_dCx_16br
   ADCMacro16bnd
SA1COp75m8d:   ; ADC d,x
   addr_dCx_8br
   ADCMacro8bd
SA1COp75m16d:  ; ADC d,x
   addr_dCx_16br
   ADCMacro16bd
SA1COp77m8nd:  ; ADC [d],y
   addr_LdLCy_8br
   ADCMacro8bnd
SA1COp77m16nd: ; ADC [d],y
   addr_LdLCy_16br
   ADCMacro16bnd
SA1COp77m8d:   ; ADC [d],y
   addr_LdLCy_8br
   ADCMacro8bd
SA1COp77m16d:  ; ADC [d],y
   addr_LdLCy_16br
   ADCMacro16bd
SA1COp79m8nd:  ; ADC a,y
   addr_aCy_8br
   ADCMacro8bnd
SA1COp79m16nd: ; ADC a,y
   addr_aCy_16br
   ADCMacro16bnd
SA1COp79m8d:   ; ADC a,y
   addr_aCy_8br
   ADCMacro8bd
SA1COp79m16d:  ; ADC a,y
   addr_aCy_16br
   ADCMacro16bd
SA1COp7Dm8nd:  ; ADC a,x
   addr_aCx_8br
   ADCMacro8bnd
SA1COp7Dm16nd: ; ADC a,x
   addr_aCx_16br
   ADCMacro16bnd
SA1COp7Dm8d:   ; ADC a,x
   addr_aCx_8br
   ADCMacro8bd
SA1COp7Dm16d:  ; ADC a,x
   addr_aCx_16br
   ADCMacro16bd
SA1COp7Fm8nd:  ; ADC al,x
   addr_alCx_8br
   ADCMacro8bnd
SA1COp7Fm16nd: ; ADC al,x
   addr_alCx_16br
   ADCMacro16bnd
SA1COp7Fm8d:   ; ADC al,x
   addr_alCx_8br
   ADCMacro8bd
SA1COp7Fm16d:  ; ADC al,x
   addr_alCx_16br
   ADCMacro16bd

SA1COp21m8: ; AND(d,x)
   addr_BdCxB_8br
   ANDMacro8b
SA1COp21m16:; AND(d,x)
   addr_BdCxB_16br
   ANDMacro16b
SA1COp23m8: ; AND d,s
   addr_dCs_8br
   ANDMacro8b
SA1COp23m16:; AND d,s
   addr_dCs_16br
   ANDMacro16b
SA1COp25m8: ; AND d
   addr_d_8br
   ANDMacro8b
SA1COp25m16:; AND d
   addr_d_16br
   ANDMacro16b
SA1COp27m8: ; AND [d]
   addr_LdL_8br
   ANDMacro8b
SA1COp27m16:; AND [d]
   addr_LdL_16br
   ANDMacro16b
SA1COp29m8: ; AND #
   addr_I_8br
   ANDMacro8b
SA1COp29m16:; AND #
   addr_I_16br
   ANDMacro16b
SA1COp2Dm8: ; AND a
   addr_a_8br
   ANDMacro8b
SA1COp2Dm16:; AND a
   addr_a_16br
   ANDMacro16b
SA1COp2Fm8: ; AND al
   addr_al_8br
   ANDMacro8b
SA1COp2Fm16:; AND al
   addr_al_16br
   ANDMacro16b
SA1COp31m8: ; AND(d),y
   addr_BdBCy_8br
   ANDMacro8b
SA1COp31m16:; AND(d),y
   addr_BdBCy_16br
   ANDMacro16b
SA1COp32m8: ; AND (d)
   addr_BdB_8br
   ANDMacro8b
SA1COp32m16:; AND (d)
   addr_BdB_16br
   ANDMacro16b
SA1COp33m8: ; AND(d,s),y
   addr_BdCsBCy_8br
   ANDMacro8b
SA1COp33m16:; AND(d,s),y
   addr_BdCsBCy_16br
   ANDMacro16b
SA1COp35m8: ; AND d,x
   addr_dCx_8br
   ANDMacro8b
SA1COp35m16:; AND d,x
   addr_dCx_16br
   ANDMacro16b
SA1COp37m8: ; AND [d],y
   addr_LdLCy_8br
   ANDMacro8b
SA1COp37m16:; AND [d],y
   addr_LdLCy_16br
   ANDMacro16b
SA1COp39m8: ; AND a,y
   addr_aCy_8br
   ANDMacro8b
SA1COp39m16:; AND a,y
   addr_aCy_16br
   ANDMacro16b
SA1COp3Dm8: ; AND a,x
   addr_aCx_8br
   ANDMacro8b
SA1COp3Dm16:; AND a,x
   addr_aCx_16br
   ANDMacro16b
SA1COp3Fm8: ; AND al,x
   addr_alCx_8br
   ANDMacro8b
SA1COp3Fm16:; AND al,x
   addr_alCx_16br
   ANDMacro16b

SA1COp06m8: ; ASL d
   addr_d_8brni
   ASLMacro8b
   addr_d_8bw
   endloop
SA1COp06m16:; ASL d
   addr_d_16brni
   ASLMacro16b
   addr_d_16bw
   endloop
SA1COp0Am8: ; ASL A
   addr_A_8brni
   ASLMacro8b
   addr_A_8bw
   endloop
SA1COp0Am16:; ASL A
   addr_A_16brni
   ASLMacro16b
   addr_A_16bw
   endloop
SA1COp0Em8: ; ASL a
   addr_a_8brni
   ASLMacro8b
   addr_a_8bw
   endloop
SA1COp0Em16:; ASL a
   addr_a_16brni
   ASLMacro16b
   addr_a_16bw
   endloop
SA1COp16m8: ; ASL d,x
   addr_dCx_8brni
   ASLMacro8b
   addr_dCx_8bw
   endloop
SA1COp16m16:; ASL d,x
   addr_dCx_16brni
   ASLMacro16b
   addr_dCx_16bw
   endloop
SA1COp1Em8: ; ASL a,x
   addr_aCx_8brni
   ASLMacro8b
   addr_aCx_8bw
   endloop
SA1COp1Em16:; ASL a,x
   addr_aCx_16brni
   ASLMacro16b
   addr_aCx_16bw
   endloop

SA1COp90:   ; BCC r
    test byte[Sflagc],01h
    jnz .skip
    JumpMacro
SA1COpB0:   ; BCS r
    test byte[Sflagc],01h
    jz .skip
    JumpMacro
SA1COpF0:   ; BEQ r
    test dword[Sflagnz],0FFFFh
    jnz .skip
    JumpMacro
SA1COp30:   ; BMI r
    test dword[Sflagnz],018000h
    jz .skip
    JumpMacro
SA1COpD0:   ; BNE r
    test dword[Sflagnz],0FFFFh
    jz .skip
    JumpMacro
SA1COp10:   ; BPL r
    test dword[Sflagnz],018000h
    jnz .skip
    JumpMacro
SA1COp80:   ; BRA r
    movsx eax,byte[esi]
    inc esi
    add esi,eax
    endloop
SA1COp50:   ; BVC r
    test dword[Sflago],0FFh
    jnz .skip
    JumpMacro
SA1COp70:   ; BVS r
    test dword[Sflago],0FFh
    jz .skip
    JumpMacro

;  BIT instruction does not affect N and V flags when using immediate
;  addressing mode. When using other addressing modes, the N and V flags

SA1COp89m8: ; BIT #
   BITMacroim8b
SA1COp89m16:; BIT #
   BITMacroim16b

SA1COp24m8: ; BIT d
   addr_d_8br
   BITMacro8b
SA1COp24m16:; BIT d
   addr_d_16br
   BITMacro16b
SA1COp2Cm8: ; BIT a
   addr_a_8br
   BITMacro8b
SA1COp2Cm16:; BIT a
   addr_a_16br
   BITMacro16b
SA1COp34m8: ; BIT d,x
   addr_dCx_8br
   BITMacro8b
SA1COp34m16:; BIT d,x
   addr_dCx_16br
   BITMacro16b
SA1COp3Cm8: ; BIT a,x
   addr_aCx_8br
   BITMacro8b
SA1COp3Cm16:; BIT a,x
   addr_aCx_16br
   BITMacro16b

SA1COp00:   ; BRK s
%ifndef NO_DEBUGGER
   mov byte[debstop4],1
%endif
   dec esi
   endloop
   BRKMacro

SA1COp82:   ; BRL rl
   BRLMacro

SA1COp18:   ; CLC i
    mov dword[Sflagc],0
    endloop
SA1COpD8:   ; CLD i
    and dl,0F7h
    mov bl,dl
    mov edi,[SA1tablead+ebx*4]
    endloop
SA1COp58:   ; CLI i
    and dl,0FBh
    endloop
SA1COpB8:   ; CLV i
    mov dword[Sflago],0
    endloop

SA1COpC1m8: ; CMP(d,x)
   addr_BdCxB_8br
   CMPMacro8b
SA1COpC1m16:; CMP(d,x)
   addr_BdCxB_16br
   CMPMacro16b
SA1COpC3m8: ; CMP d,s
   addr_dCs_8br
   CMPMacro8b
SA1COpC3m16:; CMP d,s
   addr_dCs_16br
   CMPMacro16b
SA1COpC5m8: ; CMP d
   addr_d_8br
   CMPMacro8b
SA1COpC5m16:; CMP d
   addr_d_16br
   CMPMacro16b
SA1COpC7m8: ; CMP [d]
   addr_LdL_8br
   CMPMacro8b
SA1COpC7m16:; CMP [d]
   addr_LdL_16br
   CMPMacro16b
SA1COpC9m8: ; CMP #
   addr_I_8br
   CMPMacro8b
SA1COpC9m16:; CMP #
   addr_I_16br
   CMPMacro16b
SA1COpCDm8: ; CMP a
   addr_a_8br
   CMPMacro8b
SA1COpCDm16:; CMP a
   addr_a_16br
   CMPMacro16b
SA1COpCFm8: ; CMP al
   addr_al_8br
   CMPMacro8b
SA1COpCFm16:; CMP al
   addr_al_16br
   CMPMacro16b
SA1COpD1m8: ; CMP(d),y
   addr_BdBCy_8br
   CMPMacro8b
SA1COpD1m16:; CMP(d),y
   addr_BdBCy_16br
   CMPMacro16b
SA1COpD2m8: ; CMP (d)
   addr_BdB_8br
   CMPMacro8b
SA1COpD2m16:; CMP (d)
   addr_BdB_16br
   CMPMacro16b
SA1COpD3m8: ; CMP(d,s),y
   addr_BdCsBCy_8br
   CMPMacro8b
SA1COpD3m16:; CMP(d,s),y
   addr_BdCsBCy_16br
   CMPMacro16b
SA1COpD5m8: ; CMP d,x
   addr_dCx_8br
   CMPMacro8b
SA1COpD5m16:; CMP d,x
   addr_dCx_16br
   CMPMacro16b
SA1COpD7m8: ; CMP [d],y
   addr_LdLCy_8br
   CMPMacro8b
SA1COpD7m16:; CMP [d],y
   addr_LdLCy_16br
   CMPMacro16b
SA1COpD9m8: ; CMP a,y
   addr_aCy_8br
   CMPMacro8b
SA1COpD9m16:; CMP a,y
   addr_aCy_16br
   CMPMacro16b
SA1COpDDm8: ; CMP a,x
   addr_aCx_8br
   CMPMacro8b
SA1COpDDm16:; CMP a,x
   addr_aCx_16br
   CMPMacro16b
SA1COpDFm8: ; CMP al,x
   addr_alCx_8br
   CMPMacro8b
SA1COpDFm16:; CMP al,x
   addr_alCx_16br
   CMPMacro16b

SA1COp02:   ; SA1COp s
   COPMacro

SA1COpE0x8: ; CPX #
   addr_I_8br
   CPXMacro8b
SA1COpE0x16:; CPX #
   addr_I_16br
   CPXMacro16b
SA1COpE4x8: ; CPX d
   addr_d_8br
   CPXMacro8b
SA1COpE4x16:; CPX d
   addr_d_16br
   CPXMacro16b
SA1COpECx8: ; CPX a
   addr_a_8br
   CPXMacro8b
SA1COpECx16:; CPX a
   addr_a_16br
   CPXMacro16b

SA1COpC0x8: ; CPY #
   addr_I_8br
   CPYMacro8b
SA1COpC0x16:; CPY #
   addr_I_16br
   CPYMacro16b
SA1COpC4x8: ; CPY d
   addr_d_8br
   CPYMacro8b
SA1COpC4x16:; CPY d
   addr_d_16br
   CPYMacro16b
SA1COpCCx8: ; CPY a
   addr_a_8br
   CPYMacro8b
SA1COpCCx16:; CPY a
   addr_a_16br
   CPYMacro16b

SA1COp3Am8: ; DEC A
    addr_A_8brni
    dec al
    flagsetnz8bnel
    addr_A_8bw
    endloop
SA1COp3Am16:; DEC A
    addr_A_16brni
    dec ax
    flagsetnz16bnel
    addr_A_16bw
    endloop
SA1COpCEm8: ; DEC a
    addr_a_8brni
    dec al
    flagsetnz8bnel
    addr_a_8bw
    endloop
SA1COpCEm16:; DEC a
    addr_a_16brni
    dec ax
    flagsetnz16bnel
    addr_a_16bw
    endloop
SA1COpC6m8: ; DEC d
    addr_d_8brni
    dec al
    flagsetnz8bnel
    addr_d_8bw
    endloop
SA1COpC6m16:; DEC d
    addr_d_16brni
    dec ax
    flagsetnz16bnel
    addr_d_16bw
    endloop
SA1COpD6m8: ; DEC d,x
    addr_dCx_16brni
    dec al
    flagsetnz8bnel
    addr_dCx_16bw
    endloop
SA1COpD6m16:; DEC d,x
    addr_dCx_16brni
    dec ax
    flagsetnz16bnel
    addr_dCx_16bw
    endloop
SA1COpDEm8: ; DEC a,x
    addr_aCx_8brni
    dec al
    flagsetnz8bnel
    addr_aCx_8bw
    endloop
SA1COpDEm16:; DEC a,x
    addr_aCx_16brni
    dec ax
    flagsetnz16bnel
    addr_aCx_16bw
    endloop

SA1COpCAx8: ; DEX i
    dec byte[SA1xx]
    mov al,[SA1xx]
    flagsetnz8b
    endloop
SA1COpCAx16:; DEX i
    dec word[SA1xx]
    mov ax,[SA1xx]
    flagsetnz16b
    endloop
SA1COp88x8: ; DEY i
    dec byte[SA1xy]
    mov al,[SA1xy]
    flagsetnz8b
    endloop
SA1COp88x16:; DEY i
    dec word[SA1xy]
    mov ax,[SA1xy]
    flagsetnz16b
    endloop

SA1COp41m8: ; EOR(d,x)
   addr_BdCxB_8br
   EORMacro8b
SA1COp41m16:; EOR(d,x)
   addr_BdCxB_16br
   EORMacro16b
SA1COp43m8: ; EOR d,s
   addr_dCs_8br
   EORMacro8b
SA1COp43m16:; EOR d,s
   addr_dCs_16br
   EORMacro16b
SA1COp45m8: ; EOR d
   addr_d_8br
   EORMacro8b
SA1COp45m16:; EOR d
   addr_d_16br
   EORMacro16b
SA1COp47m8: ; EOR [d]
   addr_LdL_8br
   EORMacro8b
SA1COp47m16:; EOR [d]
   addr_LdL_16br
   EORMacro16b
SA1COp49m8: ; EOR #
   addr_I_8br
   EORMacro8b
SA1COp49m16:; EOR #
   addr_I_16br
   EORMacro16b
SA1COp4Dm8: ; EOR a
   addr_a_8br
   EORMacro8b
SA1COp4Dm16:; EOR a
   addr_a_16br
   EORMacro16b
SA1COp4Fm8: ; EOR al
   addr_al_8br
   EORMacro8b
SA1COp4Fm16:; EOR al
   addr_al_16br
   EORMacro16b
SA1COp51m8: ; EOR(d),y
   addr_BdBCy_8br
   EORMacro8b
SA1COp51m16:; EOR(d),y
   addr_BdBCy_16br
   EORMacro16b
SA1COp52m8: ; EOR (d)
   addr_BdB_8br
   EORMacro8b
SA1COp52m16:; EOR (d)
   addr_BdB_16br
   EORMacro16b
SA1COp53m8: ; EOR(d,s),y
   addr_BdCsBCy_8br
   EORMacro8b
SA1COp53m16:; EOR(d,s),y
   addr_BdCsBCy_16br
   EORMacro16b
SA1COp55m8: ; EOR d,x
   addr_dCx_8br
   EORMacro8b
SA1COp55m16:; EOR d,x
   addr_dCx_16br
   EORMacro16b
SA1COp57m8: ; EOR [d],y
   addr_LdLCy_8br
   EORMacro8b
SA1COp57m16:; EOR [d],y
   addr_LdLCy_16br
   EORMacro16b
SA1COp59m8: ; EOR a,y
   addr_aCy_8br
   EORMacro8b
SA1COp59m16:; EOR a,y
   addr_aCy_16br
   EORMacro16b
SA1COp5Dm8: ; EORa,x
   addr_aCx_8br
   EORMacro8b
SA1COp5Dm16:; EORa,x
   addr_aCx_16br
   EORMacro16b
SA1COp5Fm8: ; EOR al,x
   addr_alCx_8br
   EORMacro8b
SA1COp5Fm16:; EOR al,x
   addr_alCx_16br
   EORMacro16b


SA1COp1Am8: ; INC A
    addr_A_8brni
    inc al
    flagsetnz8bnel
    addr_A_8bw
    endloop
SA1COp1Am16:; INC A
    addr_A_16brni
    inc ax
    flagsetnz16bnel
    addr_A_16bw
    endloop
SA1COpEEm8: ; INC a
    addr_a_8brni
    inc al
    flagsetnz8bnel
    addr_a_8bw
    endloop
SA1COpEEm16:; INC a
    addr_a_16brni
    inc ax
    flagsetnz16bnel
    addr_a_16bw
    endloop
SA1COpE6m8: ; INC d
    addr_d_8brni
    inc al
    flagsetnz8bnel
    addr_d_8bw
    endloop
SA1COpE6m16:; INC d
    addr_d_16brni
    inc ax
    flagsetnz16bnel
    addr_d_16bw
    endloop
SA1COpF6m8: ; INC d,x
    addr_dCx_8brni
    inc al
    flagsetnz8bnel
    addr_dCx_8bw
    endloop
SA1COpF6m16:; INC d,x
    addr_dCx_16brni
    inc ax
    flagsetnz16bnel
    addr_dCx_16bw
    endloop
SA1COpFEm8: ; INC a,x
    addr_aCx_8brni
    inc al
    flagsetnz8bnel
    addr_aCx_8bw
    endloop
SA1COpFEm16:; INC a,x
    addr_aCx_16brni
    inc ax
    flagsetnz16bnel
    addr_aCx_16bw
    endloop


SA1COpE8x8: ; INX i
    inc byte[SA1xx]
    mov al,[SA1xx]
    flagsetnz8b
    endloop
SA1COpE8x16:; INX i
    inc word[SA1xx]
    mov ax,[SA1xx]
    flagsetnz16b
    endloop
SA1COpC8x8: ; INY i
    inc byte[SA1xy]
    mov al,[SA1xy]
    flagsetnz8b
    endloop
SA1COpC8x16:; INY i
    inc word[SA1xy]
    mov ax,[SA1xy]
    flagsetnz16b
    endloop

SA1COpDC:   ; JML (a)
    JMLMacro
SA1COp4C:   ; JMP a
    JMPMacro4C
SA1COp6C:   ; JMP (a)
    JMPMacro6C
SA1COp7C:   ; JMP(a,x)
    JMPMacro7C
SA1COp5C:   ; JMP al
    JMPMacro5C

SA1COp22:   ; JSL al
   JSLMacro
SA1COp20:   ; JSR a
   JSRMacro20
SA1COpFC:   ; JSR(a,x)
   JSRMacroFC

SA1COpA1m8: ; LDA(d,x)
   addr_BdCxB_8br
   LDAMacro8b
SA1COpA1m16:; LDA(d,x)
   addr_BdCxB_16br
   LDAMacro16b
SA1COpA3m8: ; LDA d,s
   addr_dCs_8br
   LDAMacro8b
SA1COpA3m16:; LDA d,s
   addr_dCs_16br
   LDAMacro16b
SA1COpA5m8: ; LDA d
   addr_d_8br
   LDAMacro8b
SA1COpA5m16:; LDA d
   addr_d_16br
   LDAMacro16b
SA1COpA7m8: ; LDA [d]
   addr_LdL_8br
   LDAMacro8b
SA1COpA7m16:; LDA [d]
   addr_LdL_16br
   LDAMacro16b
SA1COpA9m8: ; LDA #
   addr_I_8br
   LDAMacro8b
SA1COpA9m16:; LDA #
   addr_I_16br
   LDAMacro16b
SA1COpADm8: ; LDA a
   addr_a_8br
   LDAMacro8b
SA1COpADm16:; LDA a
   addr_a_16br
   LDAMacro16b
SA1COpAFm8: ; LDA al
   addr_al_8br
   LDAMacro8b
SA1COpAFm16:; LDA al
   addr_al_16br
   LDAMacro16b
SA1COpB1m8: ; LDA(d),y
   addr_BdBCy_8br
   LDAMacro8b
SA1COpB1m16:; LDA(d),y
   addr_BdBCy_16br
   LDAMacro16b
SA1COpB2m8: ; LDA (d)
   addr_BdB_8br
   LDAMacro8b
SA1COpB2m16:; LDA (d)
   addr_BdB_16br
   LDAMacro16b
SA1COpB3m8: ; LDA(d,s),y
   addr_BdCsBCy_8br
   LDAMacro8b
SA1COpB3m16:; LDA(d,s),y
   addr_BdCsBCy_16br
   LDAMacro16b
SA1COpB5m8: ; LDA d,x
   addr_dCx_8br
   LDAMacro8b
SA1COpB5m16:; LDA d,x
   addr_dCx_16br
   LDAMacro16b
SA1COpB7m8: ; LDA [d],y
   addr_LdLCy_8br
   LDAMacro8b
SA1COpB7m16:; LDA [d],y
   addr_LdLCy_16br
   LDAMacro16b
SA1COpB9m8: ; LDA a,y
   addr_aCy_8br
   LDAMacro8b
SA1COpB9m16:; LDA a,y
   addr_aCy_16br
   LDAMacro16b
SA1COpBDm8: ; LDA a,x
   addr_aCx_8br
   LDAMacro8b
SA1COpBDm16:; LDA a,x
   addr_aCx_16br
   LDAMacro16b
SA1COpBFm8: ; LDA al,x
   addr_alCx_8br
   LDAMacro8b
SA1COpBFm16:; LDA al,x
   addr_alCx_16br
   LDAMacro16b

SA1COpA2x8: ; LDX #
   addr_I_8br
   LDXMacro8b
SA1COpA2x16:; LDX #
   addr_I_16br
   LDXMacro16b
SA1COpA6x8: ; LDX d
   addr_d_8br
   LDXMacro8b
SA1COpA6x16:; LDX d
   addr_d_16br
   LDXMacro16b
SA1COpAEx8: ; LDX a
   addr_a_8br
   LDXMacro8b
SA1COpAEx16:; LDX a
   addr_a_16br
   LDXMacro16b
SA1COpB6x8: ; LDX d,y
   addr_dCy_8br
   LDXMacro8b
SA1COpB6x16:; LDX d,y
   addr_dCy_16br
   LDXMacro16b
SA1COpBEx8: ; LDX a,y
   addr_aCy_8br
   LDXMacro8b
SA1COpBEx16:; LDX a,y
   addr_aCy_16br
   LDXMacro16b

SA1COpA0x8: ; LDY #
   addr_I_8br
   LDYMacro8b
SA1COpA0x16:; LDY #
   addr_I_16br
   LDYMacro16b
SA1COpA4x8: ; LDY d
   addr_d_8br
   LDYMacro8b
SA1COpA4x16:; LDY d
   addr_d_16br
   LDYMacro16b
SA1COpACx8: ; LDY a
   addr_a_8br
   LDYMacro8b
SA1COpACx16:; LDY a
   addr_a_16br
   LDYMacro16b
SA1COpB4x8: ; LDY d,x
   addr_dCx_8br
   LDYMacro8b
SA1COpB4x16:; LDY d,x
   addr_dCx_16br
   LDYMacro16b
SA1COpBCx8: ; LDY a,x
   addr_aCx_8br
   LDYMacro8b
SA1COpBCx16:; LDY a,x
   addr_aCx_16br
   LDYMacro16b

SA1COp46m8: ; LSR d
   addr_d_8brni
   LSRMacro8b
   addr_d_8bw
   endloop
SA1COp46m16:; LSR d
   addr_d_16brni
   LSRMacro16b
   addr_d_16bw
   endloop
SA1COp4Am8: ; LSR A
   addr_A_8brni
   LSRMacro8b
   addr_A_8bw
   endloop
SA1COp4Am16:; LSR A
   addr_A_16brni
   LSRMacro16b
   addr_A_16bw
   endloop
SA1COp4Em8: ; LSR a
   addr_a_8brni
   LSRMacro8b
   addr_a_8bw
   endloop
SA1COp4Em16:; LSR a
   addr_a_16brni
   LSRMacro16b
   addr_a_16bw
   endloop
SA1COp56m8: ; LSR d,x
   addr_dCx_8brni
   LSRMacro8b
   addr_dCx_8bw
   endloop
SA1COp56m16:; LSR d,x
   addr_dCx_16brni
   LSRMacro16b
   addr_dCx_16bw
   endloop
SA1COp5Em8: ; LSRa,x
   addr_aCx_8brni
   LSRMacro8b
   addr_aCx_8bw
   endloop
SA1COp5Em16:; LSRa,x
   addr_aCx_16brni
   LSRMacro16b
   addr_aCx_16bw
   endloop

SA1COp54:   ; MVN SA1xya
   MVNMacro
SA1COp44:   ; MVP SA1xya
   MVPMacro

SA1COpEA:   ; NOP i
   endloop

SA1COp01m8: ; ORA(d,x)
   addr_BdCxB_8br
   ORAMacro8b
SA1COp01m16:; ORA(d,x)
   addr_BdCxB_16br
   ORAMacro16b
SA1COp03m8: ; ORA d,s
   addr_dCs_8br
   ORAMacro8b
SA1COp03m16:; ORA d,s
   addr_dCs_16br
   ORAMacro16b
SA1COp05m8: ; ORA d
   addr_d_8br
   ORAMacro8b
SA1COp05m16:; ORA d
   addr_d_16br
   ORAMacro16b
SA1COp07m8: ; ORA [d]
   addr_LdL_8br
   ORAMacro8b
SA1COp07m16:; ORA [d]
   addr_LdL_16br
   ORAMacro16b
SA1COp09m8: ; ORA #
   addr_I_8br
   ORAMacro8b
SA1COp09m16:; ORA #
   addr_I_16br
   ORAMacro16b
SA1COp0Dm8: ; ORA a
   addr_a_8br
   ORAMacro8b
SA1COp0Dm16:; ORA a
   addr_a_16br
   ORAMacro16b
SA1COp0Fm8: ; ORA al
   addr_al_8br
   ORAMacro8b
SA1COp0Fm16:; ORA al
   addr_al_16br
   ORAMacro16b
SA1COp11m8: ; ORA(d),y
   addr_BdBCy_8br
   ORAMacro8b
SA1COp11m16:; ORA(d),y
   addr_BdBCy_16br
   ORAMacro16b
SA1COp12m8: ; ORA(d)
   addr_BdB_8br
   ORAMacro8b
SA1COp12m16:; ORA(d)
   addr_BdB_16br
   ORAMacro16b
SA1COp13m8: ; ORA(d,s),y
   addr_BdCsBCy_8br
   ORAMacro8b
SA1COp13m16:; ORA(d,s),y
   addr_BdCsBCy_16br
   ORAMacro16b
SA1COp15m8: ; ORA d,x
   addr_dCx_8br
   ORAMacro8b
SA1COp15m16:; ORA d,x
   addr_dCx_16br
   ORAMacro16b
SA1COp17m8: ; ORA [d],y
   addr_LdLCy_8br
   ORAMacro8b
SA1COp17m16:; ORA [d],y
   addr_LdLCy_16br
   ORAMacro16b
SA1COp19m8: ; ORA a,y
   addr_aCy_8br
   ORAMacro8b
SA1COp19m16:; ORA a,y
   addr_aCy_16br
   ORAMacro16b
SA1COp1Dm8: ; ORA a,x
   addr_aCx_8br
   ORAMacro8b
SA1COp1Dm16:; ORA a,x
   addr_aCx_16br
   ORAMacro16b
SA1COp1Fm8: ; ORA al,x
   addr_alCx_8br
   ORAMacro8b
SA1COp1Fm16:; ORA al,x
   addr_alCx_16br
   ORAMacro16b

SA1COpF4:   ; PEA s
   PEAMacro
SA1COpD4:   ; PEI s
   PEIMacro
SA1COp62:   ; PER s
   PERMacro

SA1COp48m8: ; PHA s
   PUSHMacro8b [SA1xa]
SA1COp48m16:; PHA s
   PUSHMacro16b [SA1xa], [SA1xa+1]
SA1COp8B:   ; PHB s
   PUSHMacro8b [SA1xdb]
SA1COp0B:   ; PHD s
   PUSHMacro16b [SA1xd], [SA1xd+1]
SA1COp4B:   ; PHK s
   PUSHMacro8b [SA1xpb]
SA1COp08:   ; PHP s
   PUSHMacro8bp
SA1COpDAx8: ; PHX s
   PUSHMacro8b [SA1xx]
SA1COpDAx16:; PHX s
   PUSHMacro16b [SA1xx], [SA1xx+1]
SA1COp5Ax8: ; PHY s
   PUSHMacro8b [SA1xy]
SA1COp5Ax16:; PHY s
   PUSHMacro16b [SA1xy], [SA1xy+1]

SA1COp68m8: ; PLA s
   POPMacro8b [SA1xa]
SA1COp68m16:; PLA s
   POPMacro16b [SA1xa], [SA1xa+1]
SA1COpAB:   ; PLB s
   POPMacro8b [SA1xdb]
SA1COp2B:   ; PLD s
   POPMacro16bd [SA1xd], [SA1xd+1]
SA1COp28:   ; PLP s
   POPMacroP
SA1COpFAx8: ; PLX s
   POPMacro8b [SA1xx]
SA1COpFAx16:; PLX s
   POPMacro16b [SA1xx], [SA1xx+1]
SA1COp7Ax8: ; PLY s
   POPMacro8b [SA1xy]
SA1COp7Ax16:; PLY s
   POPMacro16b [SA1xy], [SA1xy+1]

SA1COpC2:   ; REP #
   REPMacro

SA1COp26m8: ; ROL d
   addr_d_8brni
   ROLMacro8b
   addr_d_8bw
   endloop
SA1COp26m16:; ROL d
   addr_d_16brni
   ROLMacro16b
   addr_d_16bw
   endloop
SA1COp2Am8: ; ROL A
   addr_A_8brni
   ROLMacro8b
   addr_A_8bw
   endloop
SA1COp2Am16:; ROL A
   addr_A_16brni
   ROLMacro16b
   addr_A_16bw
   endloop
SA1COp2Em8: ; ROL a
   addr_a_8brni
   ROLMacro8b
   addr_a_8bw
   endloop
SA1COp2Em16:; ROL a
   addr_a_16brni
   ROLMacro16b
   addr_a_16bw
   endloop
SA1COp36m8: ; ROL d,x
   addr_dCx_8brni
   ROLMacro8b
   addr_dCx_8bw
   endloop
SA1COp36m16:; ROL d,x
   addr_dCx_16brni
   ROLMacro16b
   addr_dCx_16bw
   endloop
SA1COp3Em8: ; ROL a,x
   addr_aCx_8brni
   ROLMacro8b
   addr_aCx_8bw
   endloop
SA1COp3Em16:; ROL a,x
   addr_aCx_16brni
   ROLMacro16b
   addr_aCx_16bw
   endloop

SA1COp66m8: ; ROR d
   addr_d_8brni
   RORMacro8b
   addr_d_8bw
   endloop
SA1COp66m16:; ROR d
   addr_d_16brni
   RORMacro16b
   addr_d_16bw
   endloop
SA1COp6Am8: ; ROR A
   addr_A_8brni
   RORMacro8b
   addr_A_8bw
   endloop
SA1COp6Am16:; ROR A
   addr_A_16brni
   RORMacro16b
   addr_A_16bw
   endloop
SA1COp6Em8: ; ROR a
   addr_a_8brni
   RORMacro8b
   addr_a_8bw
   endloop
SA1COp6Em16:; ROR a
   addr_a_16brni
   RORMacro16b
   addr_a_16bw
   endloop
SA1COp76m8: ; ROR d,x
   addr_dCx_8brni
   RORMacro8b
   addr_dCx_8bw
   endloop
SA1COp76m16:; ROR d,x
   addr_dCx_16brni
   RORMacro16b
   addr_dCx_16bw
   endloop
SA1COp7Em8: ; ROR a,x
   addr_aCx_8brni
   RORMacro8b
   addr_aCx_8bw
   endloop
SA1COp7Em16:; ROR a,x
   addr_aCx_16brni
   RORMacro16b
   addr_aCx_16bw
   endloop

SA1COp40:   ; RTI s
   mov byte[intrset],2
   RTIMacro
SA1COp6B:   ; RTL s
   RTLMacro
SA1COp60:   ; RTS s
   RTSMacro

SA1COpE1m8nd:  ; SBC(d,x)
   addr_BdCxB_8br
   SBCMacro8bnd
SA1COpE1m16nd: ; SBC(d,x)
   addr_BdCxB_16br
   SBCMacro16bnd
SA1COpE1m8d:   ; SBC(d,x)
   addr_BdCxB_8br
   SBCMacro8bd
SA1COpE1m16d:  ; SBC(d,x)
   addr_BdCxB_16br
   SBCMacro16bd
SA1COpE3m8nd:  ; SBC d,s
   addr_dCs_8br
   SBCMacro8bnd
SA1COpE3m16nd: ; SBC d,s
   addr_dCs_16br
   SBCMacro16bnd
SA1COpE3m8d:   ; SBC d,s
   addr_dCs_8br
   SBCMacro8bd
SA1COpE3m16d:  ; SBC d,s
   addr_dCs_16br
   SBCMacro16bd
SA1COpE5m8nd:  ; SBC d
   addr_d_8br
   SBCMacro8bnd
SA1COpE5m16nd: ; SBC d
   addr_d_16br
   SBCMacro16bnd
SA1COpE5m8d:   ; SBC d
   addr_d_8br
   SBCMacro8bd
SA1COpE5m16d:  ; SBC d
   addr_d_16br
   SBCMacro16bd
SA1COpE7m8nd:  ; SBC [d]
   addr_LdL_8br
   SBCMacro8bnd
SA1COpE7m16nd: ; SBC [d]
   addr_LdL_16br
   SBCMacro16bnd
SA1COpE7m8d:   ; SBC [d]
   addr_LdL_8br
   SBCMacro8bd
SA1COpE7m16d:  ; SBC [d]
   addr_LdL_16br
   SBCMacro16bd
SA1COpE9m8nd:  ; SBC #
   addr_I_8br
   SBCMacro8bnd
SA1COpE9m16nd: ; SBC #
   addr_I_16br
   SBCMacro16bnd
SA1COpE9m8d:   ; SBC #
   addr_I_8br
   SBCMacro8bd
SA1COpE9m16d:  ; SBC #
   addr_I_16br
   SBCMacro16bd
SA1COpEDm8nd:  ; SBC a
   addr_a_8br
   SBCMacro8bnd
SA1COpEDm16nd: ; SBC a
   addr_a_16br
   SBCMacro16bnd
SA1COpEDm8d:   ; SBC a
   addr_a_8br
   SBCMacro8bd
SA1COpEDm16d:  ; SBC a
   addr_a_16br
   SBCMacro16bd
SA1COpEFm8nd:  ; SBC al
   addr_al_8br
   SBCMacro8bnd
SA1COpEFm16nd: ; SBC al
   addr_al_16br
   SBCMacro16bnd
SA1COpEFm8d:   ; SBC al
   addr_al_8br
   SBCMacro8bd
SA1COpEFm16d:  ; SBC al
   addr_al_16br
   SBCMacro16bd
SA1COpF1m8nd:  ; SBC(d),y
   addr_BdBCy_8br
   SBCMacro8bnd
SA1COpF1m16nd: ; SBC(d),y
   addr_BdBCy_16br
   SBCMacro16bnd
SA1COpF1m8d:   ; SBC(d),y
   addr_BdBCy_8br
   SBCMacro8bd
SA1COpF1m16d:  ; SBC(d),y
   addr_BdBCy_16br
   SBCMacro16bd
SA1COpF2m8nd:  ; SBC (d)
   addr_BdB_8br
   SBCMacro8bnd
SA1COpF2m16nd: ; SBC (d)
   addr_BdB_16br
   SBCMacro16bnd
SA1COpF2m8d:   ; SBC (d)
   addr_BdB_8br
   SBCMacro8bd
SA1COpF2m16d:  ; SBC (d)
   addr_BdB_16br
   SBCMacro16bd
SA1COpF3m8nd:  ; SBC(d,s),y
   addr_BdCsBCy_8br
   SBCMacro8bnd
SA1COpF3m16nd: ; SBC(d,s),y
   addr_BdCsBCy_16br
   SBCMacro16bnd
SA1COpF3m8d:   ; SBC(d,s),y
   addr_BdCsBCy_8br
   SBCMacro8bd
SA1COpF3m16d:  ; SBC(d,s),y
   addr_BdCsBCy_16br
   SBCMacro16bd
SA1COpF5m8nd:  ; SBC d,x
   addr_dCx_8br
   SBCMacro8bnd
SA1COpF5m16nd: ; SBC d,x
   addr_dCx_16br
   SBCMacro16bnd
SA1COpF5m8d:   ; SBC d,x
   addr_dCx_8br
   SBCMacro8bd
SA1COpF5m16d:  ; SBC d,x
   addr_dCx_16br
   SBCMacro16bd
SA1COpF7m8nd:  ; SBC [d],y
   addr_LdLCy_8br
   SBCMacro8bnd
SA1COpF7m16nd: ; SBC [d],y
   addr_LdLCy_16br
   SBCMacro16bnd
SA1COpF7m8d:   ; SBC [d],y
   addr_LdLCy_8br
   SBCMacro8bd
SA1COpF7m16d:  ; SBC [d],y
   addr_LdLCy_16br
   SBCMacro16bd
SA1COpF9m8nd:  ; SBC a,y
   addr_aCy_8br
   SBCMacro8bnd
SA1COpF9m16nd: ; SBC a,y
   addr_aCy_16br
   SBCMacro16bnd
SA1COpF9m8d:   ; SBC a,y
   addr_aCy_8br
   SBCMacro8bd
SA1COpF9m16d:  ; SBC a,y
   addr_aCy_16br
   SBCMacro16bd
SA1COpFDm8nd:  ; SBC a,x
   addr_aCx_8br
   SBCMacro8bnd
SA1COpFDm16nd: ; SBC a,x
   addr_aCx_16br
   SBCMacro16bnd
SA1COpFDm8d:   ; SBC a,x
   addr_aCx_8br
   SBCMacro8bd
SA1COpFDm16d:  ; SBC a,x
   addr_aCx_16br
   SBCMacro16bd
SA1COpFFm8nd:  ; SBC al,x
   addr_alCx_8br
   SBCMacro8bnd
SA1COpFFm16nd: ; SBC al,x
   addr_alCx_16br
   SBCMacro16bnd
SA1COpFFm8d:   ; SBC al,x
   addr_alCx_8br
   SBCMacro8bd
SA1COpFFm16d:  ; SBC al,x
   addr_alCx_16br
   SBCMacro16bd

SA1COp38:   ; SEC i
    mov dword[Sflagc],0FFh
    endloop
SA1COpF8:   ; SED i
    or dl,08h
    mov bl,dl
    mov edi,[SA1tablead+ebx*4]
    endloop
SA1COp78:   ; SEI i
    or dl,04h
    endloop

SA1COpE2:   ; SEP #
    SEPMacro

SA1COp81m8: ; STA(d,x)
   STAMacro8b
   addr_BdCxB_8bw
   endloop
SA1COp81m16:; STA(d,x)
   STAMacro16b
   addr_BdCxB_16bw
   endloop
SA1COp83m8: ; STA d,s
   STAMacro8b
   addr_dCs_8bw
   endloop
SA1COp83m16:; STA d,s
   STAMacro16b
   addr_dCs_16bw
   endloop
SA1COp85m8: ; STA d
   STAMacro8b
   addr_d_8bw
   endloop
SA1COp85m16:; STA d
   STAMacro16b
   addr_d_16bw
   endloop
SA1COp87m8: ; STA [d]
   STAMacro8b
   addr_LdL_8bw
   endloop
SA1COp87m16:; STA [d]
   STAMacro16b
   addr_LdL_16bw
   endloop
SA1COp8Dm8: ; STA a
   STAMacro8b
   addr_a_8bw
   endloop
SA1COp8Dm16:; STA a
   STAMacro16b
   addr_a_16bw
   endloop
SA1COp8Fm8: ; STA al
   STAMacro8b
   addr_al_8bw
   endloop
SA1COp8Fm16:; STA al
   STAMacro16b
   addr_al_16bw
   endloop
SA1COp91m8: ; STA(d),y
   STAMacro8b
   addr_BdBCy_8bw
   endloop
SA1COp91m16:; STA(d),y
   STAMacro16b
   addr_BdBCy_16bw
   endloop
SA1COp92m8: ; STA (d)
   STAMacro8b
   addr_BdB_8bw
   endloop
SA1COp92m16:; STA (d)
   STAMacro16b
   addr_BdB_16bw
   endloop
SA1COp93m8: ; STA(d,s),y
   STAMacro8b
   addr_BdCsBCy_8bw
   endloop
SA1COp93m16:; STA(d,s),y
   STAMacro16b
   addr_BdCsBCy_16bw
   endloop
SA1COp95m8: ; STA d,x
   STAMacro8b
   addr_dCx_8bw
   endloop
SA1COp95m16:; STA d,x
   STAMacro16b
   addr_dCx_16bw
   endloop
SA1COp97m8: ; STA [d],y
   STAMacro8b
   addr_LdLCy_8bw
   endloop
SA1COp97m16:; STA [d],y
   STAMacro16b
   addr_LdLCy_16bw
   endloop
SA1COp99m8: ; STA a,y
   STAMacro8b
   addr_aCy_8bw
   endloop
SA1COp99m16:; STA a,y
   STAMacro16b
   addr_aCy_16bw
   endloop
SA1COp9Dm8: ; STA a,x
   STAMacro8b
   addr_aCx_8bw
   endloop
SA1COp9Dm16:; STA a,x
   STAMacro16b
   addr_aCx_16bw
   endloop
SA1COp9Fm8: ; STA al,x
   STAMacro8b
   addr_alCx_8bw
   endloop
SA1COp9Fm16:; STA al,x
   STAMacro16b
   addr_alCx_16bw
   endloop

SA1COp86x8: ; STX d
   STXMacro8b
   addr_d_8bw
   endloop
SA1COp86x16:; STX d
   STXMacro16b
   addr_d_16bw
   endloop
SA1COp8Ex8: ; STX a
   STXMacro8b
   addr_a_8bw
   endloop
SA1COp8Ex16:; STX a
   STXMacro16b
   addr_a_16bw
   endloop
SA1COp96x8: ; STX d,y
   STXMacro8b
   addr_dCy_8bw
   endloop
SA1COp96x16:; STX d,y
   STXMacro16b
   addr_dCy_16bw
   endloop

SA1COp84x8: ; STY d
   STYMacro8b
   addr_d_8bw
   endloop
SA1COp84x16:; STY d
   STYMacro16b
   addr_d_16bw
   endloop
SA1COp8Cx8: ; STY a
   STYMacro8b
   addr_a_8bw
   endloop
SA1COp8Cx16:; STY a
   STYMacro16b
   addr_a_16bw
   endloop
SA1COp94x8: ; STYd,x
   STYMacro8b
   addr_dCx_8bw
   endloop
SA1COp94x16:; STYd,x
   STYMacro16b
   addr_dCx_16bw
   endloop

SA1COpDB:   ; STP i
    dec esi
    endloop

SA1COp64m8: ; STZ d
   xor al,al
   addr_d_8bw
   endloop
SA1COp64m16:; STZ d
   xor eax,eax
   addr_d_16bw
   endloop
SA1COp74m8: ; STZ d,x
   xor al,al
   addr_dCx_8bw
   endloop
SA1COp74m16:; STZ d,x
   xor eax,eax
   addr_dCx_16bw
   endloop
SA1COp9Cm8: ; STZ a
   xor al,al
   addr_a_8bw
   endloop
SA1COp9Cm16:; STZ a
   xor eax,eax
   addr_a_16bw
   endloop
SA1COp9Em8: ; STZ a,x
   xor al,al
   addr_aCx_8bw
   endloop
SA1COp9Em16:; STZ a,x
   xor eax,eax
   addr_aCx_16bw
   endloop

SA1COp14m8: ; TRB d
    addr_d_8brni
    mov cl,[SA1xa]
    test dword[Sflagnz],18000h
    jz .notflags
    or dword[Sflagnz],10000h
.notflags
    test cl,al
    jz .zero
    mov word[Sflagnz],1
    not cl
    and al,cl
    addr_d_8bw
    endloop
.zero
    mov word[Sflagnz],0
    not cl
    and al,cl
    addr_d_8bw
    endloop
SA1COp14m16:; TRB d
    addr_d_16brni
    mov cx,[SA1xa]
    test dword[Sflagnz],18000h
    jz .notflags
    or dword[Sflagnz],10000h
.notflags
    test cx,ax
    jz .zero
    mov word[Sflagnz],1
    not cx
    and ax,cx
    addr_d_16bw
    endloop
.zero
    mov word[Sflagnz],0
    not cx
    and ax,cx
    addr_d_16bw
    endloop
SA1COp1Cm8: ; TRB a
    addr_a_8brni
    mov cl,[SA1xa]
    test dword[Sflagnz],18000h
    jz .notflags
    or dword[Sflagnz],10000h
.notflags
    test cl,al
    jz .zero
    mov word[Sflagnz],1
    not cl
    and al,cl
    addr_a_8bw
    endloop
.zero
    mov word[Sflagnz],0
    not cl
    and al,cl
    addr_a_8bw
    endloop
SA1COp1Cm16:; TRB a
    addr_a_16brni
    mov cx,[SA1xa]
    test dword[Sflagnz],18000h
    jz .notflags
    or dword[Sflagnz],10000h
.notflags
    test cx,ax
    jz .zero
    mov word[Sflagnz],1
    not cx
    and ax,cx
    addr_a_16bw
    endloop
.zero
    mov word[Sflagnz],0
    not cx
    and ax,cx
    addr_a_16bw
    endloop

SA1COp04m8: ; TSB d
    addr_d_8brni
    mov cl,[SA1xa]
    test dword[Sflagnz],18000h
    jz .notflags
    or dword[Sflagnz],10000h
.notflags
    test al,cl
    jz .zero
    mov word[Sflagnz],1
    or al,cl
    addr_d_8bw
    endloop
.zero
    mov word[Sflagnz],0
    or al,cl
    addr_d_8bw
    endloop
SA1COp04m16:; TSB d
    addr_d_16brni
    mov cx,[SA1xa]
    test dword[Sflagnz],18000h
    jz .notflags
    or dword[Sflagnz],10000h
.notflags
    test ax,cx
    jz .zero
    mov word[Sflagnz],1
    or ax,cx
    addr_d_16bw
    endloop
.zero
    mov word[Sflagnz],0
    or ax,cx
    addr_d_16bw
    endloop
SA1COp0Cm8: ; TSB a
    addr_a_8brni
    mov cl,[SA1xa]
    test dword[Sflagnz],18000h
    jz .notflags
    or dword[Sflagnz],10000h
.notflags
    test al,cl
    jz .zero
    mov word[Sflagnz],1
    or al,cl
    addr_a_8bw
    endloop
.zero
    mov word[Sflagnz],0
    or al,cl
    addr_a_8bw
    endloop
SA1COp0Cm16:; TSB a
    addr_a_16brni
    mov cx,[SA1xa]
    test dword[Sflagnz],18000h
    jz .notflags
    or dword[Sflagnz],10000h
.notflags
    test ax,cx
    jz .zero
    mov word[Sflagnz],1
    or ax,cx
    addr_a_16bw
    endloop
.zero
    mov word[Sflagnz],0
    or ax,cx
    addr_a_16bw
    endloop

SA1COpAAx8: ; TAX i
    mov al,[SA1xa]
    mov [SA1xx],al
    flagsetnz8b
    endloop
SA1COpAAx16:; TAX i
    mov ax,[SA1xa]
    mov [SA1xx],ax
    flagsetnz16b
    endloop
SA1COpA8x8: ; TAY i
    mov al,[SA1xa]
    mov [SA1xy],al
    flagsetnz8b
    endloop
SA1COpA8x16:; TAY i
    mov ax,[SA1xa]
    mov [SA1xy],ax
    flagsetnz16b
    endloop
SA1COp5B:   ; TCD i
    mov ax,[SA1xa]
    mov [SA1xd],ax
    call SA1UpdateDPage
    flagsetnz16b
    endloop
SA1COp1B:   ; TCS i
    mov ax,[SA1xa]
    mov [SA1xs],ax
    endloop
SA1COp7B:   ; TDC i
    mov ax,[SA1xd]
    mov [SA1xa],ax
    flagsetnz16b
    endloop
SA1COp3B:   ; TSC i
    mov ax,[SA1xs]
    mov [SA1xa],ax
    flagsetnz16b
    endloop
SA1COpBAx8: ; TSX i
    mov al,[SA1xs]
    mov [SA1xx],al
    flagsetnz8b
    endloop
SA1COpBAx16:; TSX i
    mov ax,[SA1xs]
    mov [SA1xx],ax
    flagsetnz16b
    endloop
SA1COp8Am8: ; TSA1xa i
    mov al,[SA1xx]
    mov [SA1xa],al
    flagsetnz8b
    endloop
SA1COp8Am16:; TXA i
    mov ax,[SA1xx]
    mov [SA1xa],ax
    flagsetnz16b
    endloop
SA1COp9A:   ; TXS i
    mov ax,[SA1xx]
    mov [SA1xs],ax
    test byte[xe],1
    jnz .emul
    endloop
.emul
    mov byte[SA1xs+1],1
    endloop
SA1COp9Bx8: ; TSA1xy i
    mov al,[SA1xx]
    mov [SA1xy],al
    flagsetnz8b
    endloop
SA1COp9Bx16:; TSA1xy i
    mov ax,[SA1xx]
    mov [SA1xy],ax
    flagsetnz16b
    endloop
SA1COp98m8: ; TYA i
    mov al,[SA1xy]
    mov [SA1xa],al
    flagsetnz8b
    endloop
SA1COp98m16:; TYA i
    mov ax,[SA1xy]
    mov [SA1xa],ax
    flagsetnz16b
    endloop
SA1COpBBx8: ; TYX i
    mov al,[SA1xy]
    mov [SA1xx],al
    flagsetnz8b
    endloop
SA1COpBBx16:; TYX i
    mov ax,[SA1xy]
    mov [SA1xx],ax
    flagsetnz16b
    endloop

SA1COpCB:   ; WAI i
    WAIMacro

SA1COpEB:   ; XBA i
    mov ah,[SA1xa]
    mov al,[SA1xa+1]
    mov [SA1xa],ax
    flagsetnz8b
    endloop

SA1COpFB:   ; XCE i
    XCEMacro

SA1COp42:   ; WDM
    inc esi
    endloop


