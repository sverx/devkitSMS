;--------------------------------------------------------------------------
; *** this is a modified version aimed to SG-1000 homebrew - sverx\2015 ***
; *** additional changes targeting BASIC on the SC-3000 - JoppyFurr\2024 ***
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

; Memory layout:
;  0x8000: RAM at addresses lower than the load-address:
;    -> Use 0x8000--0x95ff, 5.5 KiB, as work-ram
;    -> Use 0x9600--0x9700, 257 bytes, as the interrupt vector table.
;  0x9800: Destination address where BASIC loads our code to.
;    -> Continues to 0xc7ff for BASIC IIIa (12 KiB)
;    -> Continues to 0xffff for BASIC IIIb (26 KiB)
;  0x9898: The interrupt handler, pointed to by the interrupt vector table.
;
; Note: The NMI (reset button) is not available to the user application.

  .module crt0
  .globl  _main

  .area _HEADER (ABS)
  ;; Destination address used by the BASIC `LOAD` command
  .org  0x9800
init:
  di                ; disable interrupt
  im 2              ; interrupt mode 2 (this will not change)

  ld sp,#0x95f0     ; set stack pointer at end of our 5.5k block of RAM
  xor a             ; clear RAM (to value 0x00)
  ld hl,#0x8000     ;   by setting value 0
  ld (hl),a         ;   to $c000 and
  ld de,#0x8001     ;   copying (LDIR) it to next byte
  ld bc,#0x1600-17  ;   for 5.5 KB minus 17 bytes
  ldir              ;   do that

  ;; ensure this runs fine on SC-3000 too
  ld a,#0x92
  out (0xDF),a      ; Config PPI (no effect on SG-1000)
  ld a,#7
  out (0xDE),a      ; Select ROW 7 (row 7 of PPI is joypad = default - no effect on SG-1000)

  ;; Set up for interrupt-mode 2
  ;; Register I controls the vector MSB. The vector LSB is whatever happens to be on the bus.
  ;; So, a 257-entry table with a single repeated byte (0x98) will give us a reliable 0x9898 call.
  ld a,#0x96        ; Vector table at 0x9600, between work-ram and user code.
  ld i,a

  ld a,#0x98
  ld hl,#0x9600
  ld (hl),a         ; Initialize first byte of vector table at 0x9600 to value 0x98
  ld de,#0x9601     ; Prepare copy to the next byte
  ld bc,#0x0100     ; Copy for 256 bytes, giving 257 bytes total.
  ldir

  ;; Initialise global variables
  call    gsinit
  call    _SG_init
  ei                ; re-enable interrupts before going to main()
  call  _main
1$:
  halt
  jr 1$

  .org    0x9898    ; handle IRQ
  jp _SG_isr

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
  .area   _CODE
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

