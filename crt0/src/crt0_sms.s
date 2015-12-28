;--------------------------------------------------------------------------
; ***** this is a modified version aimed to SMS homebrew - sverx\2015 *****
;--------------------------------------------------------------------------
;--------------------------------------------------------------------------
;  crt0.s - Generic crt0.s for a Z80
;
;  Copyright (C) 2000, Michael Hope
;
;  This library is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundation; either version 2, or (at your option) any
;  later version.
;
;  This library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License 
;  along with this library; see the file COPYING. If not, write to the
;  Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
;   MA 02110-1301, USA.
;
;  As a special exception, if you link this library with other files,
;  some of which are compiled with SDCC, to produce an executable,
;  this library does not by itself cause the resulting executable to
;  be covered by the GNU General Public License. This exception does
;  not however invalidate any other reasons why the executable file
;   might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

	.module crt0
	.globl	_main

	.area	_HEADER (ABS)
	;; Reset vector
	.org 	0
	di				; disable interrupt
        im 1				; interrupt mode 1 (this won't change)
	jp	init

        .org    0x38                    ; handle IRQ
        jp _SMS_isr

        .org     0x66                   ; handle NMI
        jp _SMS_nmi_isr

	.org	 0x70
init:
        ld sp, #0xdff0			; set stack pointer at end of RAM
        ld de, #0xfffc			; initialize mappers
        xor a				
        ld (de),a			; [0xfffc]=$00
        ld b,#3
mapper_loop:
        inc de
        ld (de),a			; [0xfffd]=$00,[0xfffe]=$01,[0xffff]=$02
        inc a
        djnz mapper_loop
        
        xor a				; clear RAM (to value 0x00)
        ld hl,#0xc000			;   by setting value 0 to $c000 and
	ld (hl),a			;   to $c000 and
        ld de,#0xc001			;   copying (LDIR) it to next byte
        ld bc,#0x1ff0			;   for 8 KB minus 16 bytes
        ldir				;   do that

        ;; Initialise global variables
        call    gsinit
        call    _SMS_init
        ei				; re-enable interrupts before going to main()
	call	_main
	jp	_exit

        ; here's a block of 128 OUTI instructions, made for enabling
        ;    UNSAFE but FAST
        ; short data transfers to VRAM

_OUTI128::                              ; _OUTI128 label points to a block of 128 OUTI and a RET
        .rept	64
	outi
	.endm
_OUTI64::                               ; _OUTI64 label points to a block of 64 OUTI and a RET
        .rept	32
	outi
	.endm
_OUTI32::                               ; _OUTI32 label points to a block of 32 OUTI and a RET
        .rept	32
	outi
	.endm
_outi_block::				; _outi_block label points to END of OUTI block
	ret

	;; Ordering of segments for the linker.
	.area	_HOME
	.area	_CODE
	.area	_INITIALIZER
	.area   _GSINIT
	.area   _GSFINAL

	.area	_DATA
	.area	_INITIALIZED
	.area	_BSEG
	.area   _BSS
	.area   _HEAP

	.area   _CODE
__clock::
	ld	a,#2
	rst     0x08
	ret

_exit::
	;; Exit - special code to the emulator
	ld	a,#0
	rst     0x08
1$:
	halt
	jr	1$

	.area   _GSINIT
gsinit::
	ld	bc, #l__INITIALIZER
	ld	a, b
	or	a, c
	jr	Z, gsinit_next
	ld	de, #s__INITIALIZED
	ld	hl, #s__INITIALIZER
	ldir
gsinit_next:

	.area   _GSFINAL
	ret

