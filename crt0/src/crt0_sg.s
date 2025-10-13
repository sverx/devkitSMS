;--------------------------------------------------------------------------
; *** this is a modified version aimed to SG-1000 homebrew - sverx\2025 ***
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
  .globl  _main

  .area _HEADER (ABS)
  ;; Reset vector
  .org  0
  di                ; disable interrupt
  im 1              ; interrupt mode 1 (this will not change)
  ld sp,#0xc3f0     ; set stack pointer at end of RAM
  xor a             ; clear RAM (to value 0x00)
  ld hl,#0xc000     ;   by setting value 0
  ld (hl),a         ;   to $c000 and
  ld de,#0xc001     ;   copying (LDIR) it to next byte
  ld bc,#0x0400-17  ;   for 1 KB minus 17 bytes
  ldir              ;   do that

  ;; ensure this runs fine on SC-3000 too
  ld a,#0x92
  out (0xDF),a      ; Config PPI (no effect on SG-1000)
  ld a,#7
  out (0xDE),a      ; Select ROW 7 (row 7 of PPI is joypad = default - no effect on SG-1000)

  ;; Initialise global variables
  call  gsinit
  call  _SG_init
  ei                ; make sure interrupts are enabled before calling main
  call  _main
1$:
  halt              ; main has returned, stop here
  jr 1$

;--------------------------------------------------------------------------
get_bank::          ; get current code bank num into A
  ld a,(#0xfffe)    ; (read current page from mapper)
  ret
set_bank::          ; set current code bank num to A
  ld (#0xfffe),a    ; (restore caller page)
  ret

;--------------------------------------------------------------------------
  .org   0x38       ; handle IRQ
  jp _SG_isr

;--------------------------------------------------------------------------
  .org   0x40       ; bitshift LUT for putPixel
  .db 0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01

  .org   0x55
  .ascii 'devkitSMS'; this is shameless

;--------------------------------------------------------------------------
  .org   0x66       ; handle NMI
  jp _SG_nmi_isr

  .rept 128         ; this is a block of 128 OUTI
    outi            ; made for enabling UNSAFE but FAST
  .endm             ; short data transfers to VRAM
_outi_block::       ; _outi_block label points to END of block
  ret

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
