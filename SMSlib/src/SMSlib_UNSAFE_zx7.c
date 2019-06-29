/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"

#pragma save
#pragma disable_warning 85
void UNSAFE_SMS_loadZX7compressedTilesatAddr (void *src, unsigned int dst) {
/* **************************************************
; by Einar Saukas, Antonio Villena & Metalbrain
; modified for sms vram by aralbrec
; modified for asm by Maxim
; C wrapper/made interrupt safe by sverx
; ========================================================================
; *** This version only supports match lengths up to 255. This enables ***
; *** it to be smaller and faster, but it is not 100% compatible.      ***
   ************************************************** */
  __asm
  
  pop bc
  pop hl         ; move *src from stack into hl
  pop de         ; move dst from stack into de
  push de
  push hl
  push bc

  ; Set VRAM address
  ld c, #0xbf
  di
  out (c),e
  out (c),d
  ei
  dec c          ; data port

  ld a, #0x80    ; Signal bit for flags byte (1<<7)

  ; This is a trick whereby we can cycle a flags byte in a through the carry flag,
  ; and a will never be zero until it is time to read a new flags byte because we
  ; shift a bit to the right of the data to keep it non-zero until they are all
  ; consumed.
  ; Each time we want a bit, we do:
  ; add a, a ; get MSB into carry
  ; jr z, _nextFlagsByte ; get new flags byte if necessary (and shift it into carry)
  ; <use the bit in carry>

_b0:             ; First byte is always literal
  outi           ; increments hl
  inc de

_main_loop:      ; Main loop
  add a, a
  call z, _nextFlagsByte
  jr nc, _b0     ; next bit indicates either literal or sequence

  ; 0 bit = literal byte
  ; 1 bit = sequence:
  ; - length is encoded using a variable number of flags bits, encoding
  ;   the number of bits and the value
  ;   length 2 is encoded as %0
  ;                           ^--- indicates 0 bit number, value 0
  ;   length 3 is encoded as %101
  ;                           ^^-- indicates 1 bit number, value 1
  ;   length 4 is encoded as %11010
  ;                           ^^^- indicates 2 bit number, value 2
  ;   ...etc
  ; - offsets are encoded as either 7 or 11 bits plus a flag:
  ;   - 0oooooooo for offset ooooooo
  ;   - 1oooooooo plus bitstream bits abcd for offset abcdooooooo

  push de

    ; determine number of bits used for length (Elias gamma coding)
    ld b, #1     ; length result
    ld d, #0     ; d = 0

_b1:             ; Count how many 0 bits we have in the flags sequence
    inc d
    add a, a
    call z, _nextFlagsByte
    jr nc, _b1
    jp _f0

    ; determine length
_b2:
    add a, a
    call z, _nextFlagsByte
    rl b
    jp c, _done  ; check end marker
_f0:
    dec d
    jr nz, _b2
    inc b        ; adjust length

    ; determine offset
    ld e, (hl)   ; load offset flag (1 bit) + offset value (7 bits)
    inc hl
    
    sla e        ; these two instructions instead of
    inc e        ; 'sll e' undocumented instruction

    jr nc, _f1   ; if offset flag is set, load 4 extra bits

    add a, a
    call z, _nextFlagsByte
    rl d

    add a, a
    call z, _nextFlagsByte
    rl d

    add a, a
    call z, _nextFlagsByte
    rl d

    add a, a
    call z, _nextFlagsByte
    ccf
    jr c, _f1
    inc d

_f1:
    rr e         ; insert inverted fourth bit into E

    ; copy previous sequence
    ex (sp), hl  ; store source, restore destination
    push hl      ; store destination
      sbc hl, de ; HL = destination - offset - 1
    pop de       ; DE = destination

    ; ldir vram -> vram
    push af      ; need to preserve carry
      ; Make hl a read address
      res 6, h
      inc c      ; ld c, $bf
_b3:
      di
      out (c),l
      out (c),h
      ; saving one ei here
      in a,(#0xbe)
      ; saving one di here
      out (c),e
      out (c),d
      ei
      out (#0xbe),a
      inc hl
      inc de
      djnz _b3
    pop af

    dec c        ; restore VRAM write port

  pop hl         ; restore source address (compressed data)
  jp nc, _main_loop

_nextFlagsByte:
  ld a, (hl)     ; Else load the next byte
  inc hl
  rla            ; And push that into the carry bit
  ret

_done:
  pop hl
  __endasm;
}
#pragma restore
