;-------------------------------------------------------------------------------
; *** this is a modified version aimed to ColecoVision homebrew - sverx\2025 ***
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
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
;-------------------------------------------------------------------------------

  .module crt0
  .globl  _main

  .area _HEADER (ABS)
  .org 0x8000
  .db 0x55, 0xAA    ; cartridge header signature ("no title" version)
  .dw 0x0000        ; (unused LOCAL_SPR_TBL address)
  .dw 0x0000        ; (unused SPRITE_ORDER address)
  .dw 0x0000        ; (unused WORK_BUFFER address)
  .dw 0x0000        ; (unused CONTROLLER_MAP address)
  .dw 2$            ; address of program entry point

  .org 0x800c       ; RST $08 code (VDP_SEMAPHORE_OFF)
  jp 4$

  .org 0x800f       ; RST $10 code (VDP_SEMAPHORE_ON)
  push hl
    ld hl,#_CV_VDP_op_pending
    ld (hl),#1
  pop hl
  ret

  .org 0x8021
  jp _SG_nmi_isr    ; will call the vblank Interrupt Service Routine (NMI)
  
2$:
  di                ; disable interrupt
  im 1              ; interrupt mode 1 (this will not change)
  ld sp,#0x6400     ; set stack pointer at end of RAM
  xor a             ; clear RAM (to value 0x00)
  ld hl,#0x6000     ;   by setting value 0
  ld (hl),a         ;   to $6000 and
  ld de,#0x6001     ;   copying (LDIR) it to next byte
  ld bc,#0x0400-1   ;   for 1 KB minus 1 byte
  ldir              ;   do that

  ;; Initialise global variables
  call  gsinit
  call  _SG_init
  jr 3$

  .org   0x8040     ; bitshift LUT for putPixel
  .db 0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01

3$:
  ei                ; make sure interrupts are enabled before calling main
  call  _main

1$:
  halt              ; main has returned, stop here
  jr 1$

4$:                 ; VDP_SEMAPHORE_OFF code 
  push af
  push hl
    ld hl,#_CV_VDP_op_pending
    ld (hl),#0
    ld hl,#_CV_NMI_srv_pending
    bit 0,(hl)
    call nz,_SG_isr_process
  pop hl
  pop af
  ret

  .ascii 'devkitSMS'; this is shameless

  ;; Ordering of segments for the linker.
  .area _HOME
  .area _CODE
  .area _INITIALIZER
  .area   _GSINIT
  .area   _GSFINAL

  .area _DATA
  .area _INITIALIZED
  .area _BSEG
  .area   _BSS
  .area   _HEAP

  .area   _GSINIT
gsinit::
  ld  bc, #l__INITIALIZER
  ld  a, b
  or  a, c
  jr  Z, gsinit_next
  ld  de, #s__INITIALIZED
  ld  hl, #s__INITIALIZER
  ldir
gsinit_next:

  .area   _GSFINAL
  ret

