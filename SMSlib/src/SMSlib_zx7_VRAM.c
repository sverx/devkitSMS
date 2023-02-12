/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"

#pragma save
#pragma disable_warning 85
void SMS_loadZX7compressedTilesatAddr (const void *src, unsigned int dst) __naked __sdcccall(1) {
/* =====================================================================
* by Einar Saukas, Antonio Villena & Metalbrain
* modified for sms vram by aralbrec
* modified for asm by Maxim
* C wrapper/made interrupt safe/VDP timing safe by sverx
===================================================================== */
  __asm
  ld c,#0xbf     ; VDP control port
  set 6,d        ; set VRAM write bit
  di             ; set VRAM address
  out (c),e
  out (c),d
  ei
  res 6,d        ; remove VRAM write bit
  dec c          ; data port

  ld a,#0x80     ; Signal bit for flags byte (1<<7)

dzx7s_copy_byte_loop:
  outi           ; copy literal byte
  inc de

dzx7s_main_loop:

  call dzx7s_next_bit
  jr nc,dzx7s_copy_byte_loop     ; next bit indicates either literal or sequence

  ; determine number of bits used for length (Elias gamma coding)
  push de
  ld bc,#0
  ld d,b

dzx7s_len_size_loop:
  inc d
  call dzx7s_next_bit
  jr nc,dzx7s_len_size_loop

  ; determine length
dzx7s_len_value_loop:
  call nc,dzx7s_next_bit
  rl c
  rl b

  jp c,l_ret
  dec d
  jr nz,dzx7s_len_value_loop
  inc bc         ; adjust length

  ; determine offset
  ld e, (hl)     ; load offset flag (1 bit) + offset value (7 bits)
  inc hl
  sla e
  inc e
  jr nc,dzx7s_offset_end     ; if offset flag is set, load 4 extra bits
  ld d, #0x10                ; bit marker to load 4 bits

dzx7s_rld_next_bit:
  call dzx7s_next_bit
  rl d           ; insert next bit into D
  jr nc,dzx7s_rld_next_bit  ; repeat 4 times, until bit marker is out
  inc d          ; add 128 to DE
  srl d          ; retrieve fourth bit from D

dzx7s_offset_end:
  rr e           ; insert fourth bit into E

; copy previous sequence
  ex (sp),hl     ; store source, restore destination
  push hl        ; store destination
  sbc hl,de      ; HL = destination - offset - 1
  pop de         ; DE = destination
  push af

  ; ***********************
  set 6,d
  dec bc
  inc b
  inc c
  ld a,c
  ld c,#0xbf

dzx7s_outer_loop:
  push bc
  ld b,a

dzx7s_inner_loop:
  nop                     ; 4
  di                      ; 4 = 27 (safe on every Master System or Game Gear)
  out (c),l
  out (c),h
  ei             ; 4
  inc hl         ; 6
  xor a          ; 4
  ret nz         ; 5      (this ret will never happen, it is just to wait 5 cycles)
  nop            ; 4
  nop            ; 4 = 27 (safe on every Master System or Game Gear)
  in a,(#0xbe)

  nop            ; 4
  nop            ; 4
  nop            ; 4
  nop            ; 4
  nop            ; 4
  nop            ; 4
  di             ; 4 = 28 (safe on every Master System or Game Gear)
  out (c),e
  out (c),d
  ei
  out (#0xbe),a
  inc de                  ; 6
  djnz dzx7s_inner_loop   ; 13
  ld a,b
  pop bc
  djnz dzx7s_outer_loop
  ld c,b
  res 6,d
  ; ***********************

  pop af
  ld c,#0xbe
  pop hl         ; restore source address (compressed data)
  jr nc, dzx7s_main_loop

dzx7s_next_bit:
  add a,a        ; check next bit
  ret nz         ; no more bits left?
  ld a,(hl)      ; load another group of 8 bits
  inc hl
  rla
  ret

l_ret:
  pop hl
  ret
  __endasm;
}
#pragma restore
