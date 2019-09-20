;/*
;	$Workfile:   C0startup.asm  $
;	$Revision: 1.11 $
;	$Date: 2011/12/15 01:43:01 $
;*/
;//******************************************************************************
;// COPYRIGHT (C) STMicroelectronics 2011.
;//
;// All rights reserved. This document contains proprietary and
;// confidential information of the STMicroelectronics Group. This
;// document is not to be copied in whole or part. STMicroelectronics
;// assumes no responsibility for the consequences of use of such
;// information nor for any infringement of patents or other rights of
;// third parties which may result from its use. No license is granted by
;// implication or otherwise under any patent or patent rights of
;// STMicroelectronics.
;// STMicroelectronics
;// products are not authorized for use as critical components in life
;// support devices or systems without express written approval of
;// STMicroelectronics.
;//==============================================================================
;//
;// MODULE:      C0Startup.asm
;//
;// USAGE:       This module contains system data initialization
;//
;//******************************************************************************
.186							; Required for target independence

;//******************************************************************************
;//  C H I P - S P E C I F I C     D E F I N I T I O N S
;//******************************************************************************
;Sequoia specific
PCB_BASE		EQU	0FA00h
PCB_BASE_DEF		EQU	0FF00h
PCB_UMCS_0_WAIT_STATES	EQU	0FE38h			;0 wait states number for upper memory block 0xFE000

INTERRUPT_VECTOR_START	EQU	0h
INTERRUPT_VECTOR_END	EQU	60h

STACK_SIZE EQU 2048 ; 2K stack size

STACK_TOP                EQU 0fc00h	;0fff0h 
STACK_SEG		 EQU 0f000h	

;//******************************************************************************
;//  G E N E R I C    C H I P     D E F I N I T I O N S
;//******************************************************************************
PCB_RELOCATION_DEF	EQU	(PCB_BASE_DEF + 0FEh)	;Relocation register on the base of default PCB_BASE
PCB_UMCS_DEF		EQU	(PCB_BASE_DEF + 0A0h)	;upper memory on the base of default PCB_BASE
PCB_IMASK_DEF		EQU	(PCB_BASE_DEF + 028h)	;interrupt mask
PCB_IN_MEMORY		EQU	1000h	;BIT12

;//******************************************************************************
;//  S E G M E N T S    D E F I N I T I O N S
;//******************************************************************************
_ALIGN_			equ	<word>

;
; Segment and group declarations.  The order of these declarations is
; used to control the order of segments in the .ROM file since the
; Paradigm linker copies segments in the same order they
; are encountered in the object files.
;
; Make sure that this startup module is specified first when linking
; the application.
;

% _TEXT segment para public use16 'CODE'	;Default code
_TEXT ends

; These are segment definitions for the classes ROMDATA and ENDROMDATA.  Class
; ROMDATA never contains anything (it is filled in by a Paradigm LOCATE DUP
; directive).  Class ENDROMDATA is used to mark the end of class ROMDATA and
; prevent a segment alias (since class ROMDATA will have zero length).
% _RD segment para public use16 'ROMDATA'
public			_romdata_start				; Mark the start of class ROMDATA
_romdata_start	label 	byte
_RD ends


% _ERD segment para public use16 'ENDROMDATA'
public			_romdata_end				; Mark the end of class ROMDATA
_romdata_end	label 	byte
db				16 dup (?)			; Force the next segment to a new paragraph
_ERD ends

% _INTERRUPT_VECTOR segment _ALIGN_ public use16 'INT_TABLE'
	db INTERRUPT_VECTOR_END dup (?)		; Size of Interrupt Vector
	_interrupt_vector_end label byte
_INTERRUPT_VECTOR ends
DGROUP group _INTERRUPT_VECTOR

% _DATA segment _ALIGN_ public use16 'DATA'		; Initialized data
public _initialized_data_start 
_initialized_data_start label byte
_DATA ends
DGROUP group _DATA

% _BSS segment _ALIGN_ public use16 'BSS'			; Uninitialized data
public _uninitialized_data_start
_uninitialized_data_start label byte
_BSS ends
DGROUP group _BSS

% _NVRAM segment _ALIGN_ public use16 'NVRAM'	; Dummy NVRAM para to make linker happy
;   	dw ?
_NVRAM ends
DGROUP group _NVRAM

% _STACK segment _ALIGN_ stack use16 'STACK'       ; Dummy STACK para to make linker happy
    public    _lpm_used_sdram_end
    _lpm_used_sdram_end label byte
    dw    ?
_STACK ends
DGROUP group _STACK

extrn _Lpm_Main:far

;//******************************************************************************
;//  C O D E
;//******************************************************************************

_TEXT segment
	assume cs:_TEXT
    	assume ds:DGROUP                ;00000h	
_main proc far
	public _main

	mov    ax, STACK_SEG
	mov    ss, ax 
    	mov    ax, DGROUP
    	mov    sp, STACK_TOP     ; SP = SS + C000h = C000h
    	mov    ds, ax            ; DS=SS=ES=00000h (ax=0)
    	mov    es, ax            ; DS=SS=ES=00000h (ax=0)
   	assume    ds:DGROUP, ss:STACK_SEG, es:DGROUP

	;----------------------------------------------------------------
	;Disable interrupts and force the forward direction
	cli
	cld

	;-----------------------------------------------------------------
	;Set UMCS base address to _TEXT, 0 Wait states, and external READY also used
	mov dx, PCB_UMCS_DEF
	mov ax, PCB_UMCS_0_WAIT_STATES   
	out dx, ax
	
	;-----------------------------------------------------------------------
	; Relocate the T186 Peripheral Register Block to new memory space 0FA00h
	mov dx, PCB_RELOCATION_DEF
	mov ax, HIGH PCB_BASE + PCB_IN_MEMORY
	out dx, ax

	;-----------------------------------------------------------------
	;Zero out the interrupt vector table, total 0x80 bytes
	mov	ax, 0						; ax = 0x0000
	mov	di, offset DGROUP:_INTERRUPT_VECTOR
	mov	cx, offset DGROUP:_interrupt_vector_end
	sub	cx, di						; cx = counter (tells REP how many time to execute the instruction)
	shr	cx, 1						; counter (words) = counter (bytes) / 2
    	rep    stosw                        ; Store ax in word es:di						; Store ax in word es:di

    	mov    ax, _RD
    	sub    ax, _TEXT
    	shl    ax, 4
    	mov    si, ax
    	mov    ax, _TEXT
    	mov    ds, ax
    	assume    ds:_TEXT

    	mov    di, offset DGROUP:_initialized_data_start
    	mov    cx, offset DGROUP:_uninitialized_data_start
    	sub    cx, di
    	shr    cx, 1
    	rep    movsw

    	;-----------------------------------------------------------------
    	;Zero out the uninitialized data area (_uninitialized_data_start - irom used sdram end)
    	mov    ax, DGROUP
    	mov    ds, ax
    	assume    ds:DGROUP
    	xor    ax, ax ;ax = 0x0000
    	mov    di, offset DGROUP:_uninitialized_data_start
    	mov    cx, offset DGROUP:_lpm_used_sdram_end	
    	sub    cx, di ;cx = counter (tells REP how many time to execute the instruction)
    	shr    cx, 1 ;counter (words) = counter (bytes) / 2
    	rep    stosw ;Store ax in word es:di    

@@1:
	jmp _Lpm_Main
_main endp
_TEXT ends

_START segment para public use16 'START'			; Restart data
	assume cs:_START ;for correct long jump and because CS actually = 0FFFFh
    org	00h ;reset vector for 20 bit mode
    jmp    _main
    
_init proc
	jmp _main
	
_init endp
_START ends

end _init	; Startup code entry point

;//*********************************  END  **************************************
