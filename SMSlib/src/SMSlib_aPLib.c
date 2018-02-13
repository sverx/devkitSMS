/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"

/*

; aPPack decompressor
; original source by dwedit
; very slightly adapted by utopian
; optimized by Metalbrain
; WLA-DX port by Maxim
; C wrapper by sverx

*/

#pragma save
#pragma disable_warning 85
void UNSAFE_SMS_loadaPLibcompressedTilesatAddr (void *src, unsigned int dst) {
__asm
  pop bc
  pop hl      ; pop src
  pop de      ; pop dst
  push de
  push hl
  push bc

  push ix     ; preserve ix

  ; Set the write address
  ld c, #0xbf
  di
  out (c), e
  out (c), d
  ei

  ; Set up bitstream with only the MSB set. This will trigger a bitstream read.
  ; The code shifts a left and then checks for zero every time a bit is consumed.
  ; If the remaining bits in a are zero, then the carry will always be 1.
  ; This then leads to code which reads another 8 bits in, and immediately rotates through c.
  ; This is guaranteed to insert a set bit at the LSB. Thus, it is possible to know when all the
  ; bits are consumed (a = 0 after shift) even if the bitstream itself is all zeroes.
  ld a, #0x80

  ; First byte must be a literal, so fall through into this handler
_emitRawByte:
  push af
    ld a,(hl)
    out (#0xbe),a
  pop af
  inc hl
  inc de
  ; Fall through

_mainLoop_noPair:
  .db 0xDD
  ld h, #1       ; ld ixh, #1     ; Set "no pair" flag

_mainLoop:
  ; Main loop
  ; a = bitstream, we shift it left into carry
  ; hl = next byte in the stream
  ; de = destination
  ; ixh = "pair" flag, 0 if can use last offset
  ; iy = last used offset

  ; Get next bit. See above about how this works...
  add a, a
  jr z, _getBitstream_bit1

  ; If we got a zero, emit a raw byte
  jr nc, _emitRawByte
_getBitstream_bit1_set:

  ; Else, look at the next bit. If we have run out of bits, again go to a bespoke handler
  add a, a
  jr z, _getBitstream_bit2
  jr nc, _emitBlock
_getBitstream_bit2_set:

  ; Next bit
  add a, a
  jr z, _getBitstream_bit3
  jr nc, _emitSmallBlock
_getBitstream_bit3_set:

_emitSingleByte:
  ; We read in a 4-bit offset.
  ; We do this by putting a bit in c and rotating bits out of a into it,
  ; until the first bit comes out. We also zero b here so bc comes out as the final result.
  ld bc, #0x10   ; (1<<4)

_b1:
  add a, a
  jr z, _getBitstream_fourBitNumber
_getBitstream_fourBitNumber_done:
  rl c
  jp nc, _b1

  jr nz, _emitSingleByte_offset

_emitSingleByte_zero:
  ; If the final result in c is zero, it's a literal zero. We zeroed b above.
  ex de, hl
  ld c, #0xbe
  out (c), b
  ex de, hl
  inc de
  jp _mainLoop_noPair

_getBitstream_bit1:
  ; Read eight more bits of the bitstream
  ld a,(hl)
  inc hl
  ; Check high bit - existing carry goes into LSB
  rla
  ; 1 = continue
  jr c, _getBitstream_bit1_set
  ; 0 = raw byte
  jp _emitRawByte

; we jr here and then jp back, because it is the fastest:
; - the jr condition is met 1/8 of the time; it costs 7 cycles when not met and 12 otherwise, so average 7.625
; - jp condition costs 10 cycles, so the averge cost is higher
; - a plain jp costs 10 cycles, 12 for jr
; - a conditional call costs 17/10 cycles, and ret is 10, so it is worse (and we always return to the same place)
_getBitstream_fourBitNumber:
  ld a, (hl)
  inc hl
  rla
  jp _getBitstream_fourBitNumber_done

_emitSingleByte_offset:
  ; Preserve the bitstream
  ex af, af'
    ; Get the dest into hl for maths
    ex de, hl
    push hl
      sbc hl, bc
      ld c,#0xbf
      out (c),l
      ld a,h
      xor #0x40
      out (c),a
      in a,(#0xbe)
    pop hl
    di
    out (c),l
    out (c),h
    ei
    out (#0xbe),a
    ex de, hl
  ex af, af'
  inc de
  jp _mainLoop_noPair

_getBitstream_bit3:
  ld a, (hl)
  inc hl
  rla
  jr c, _getBitstream_bit3_set
_emitSmallBlock:
  ld c,(hl) ; use 7 bit offset, length = 2 or 3
  inc hl
  ex af, af'
  rr c
  jp z, _leave   ; if a zero is found here, it's EOF
  ld a, #2
  ld b, #0
  adc a, b
  push hl
    push bc      ; ld iyh, b
    pop iy       ; ld iyl, c
    ld h, d
    ld l, e
    sbc hl, bc
    ld c, a
    ex af, af'
    call _ldir_vram_to_vram
  pop hl
  .db 0xDD
  ld h,b         ; ld ixh, b ; will be zero
  jp _mainLoop

_getBitstream_bit2: ; ap2:
  ; Get bitstream next byte
  ld a, (hl)
  inc hl
  rla
  jr c, _getBitstream_bit2_set
  ; fall through for zero

_emitBlock:
  ; Get the first part of the offset (usually the MSB)
  call _getVariableLengthNumber
  dec c
  ex af, af'     ; make a usable for maths
    ld a, c
    .db 0xDD
    sub a,h      ; sub a,ixh ; will be 1 if we should use the last offset, 0 otherwise (?)
    jr z, ap_r0_gamma ; If we hit zero here the the encoded MSB was 2 and ixh was 1.
    dec a ; Else we subtract another 1 so we are at n-3 (if the "r0" flag was set) or n-2 (otherwise)

    ; Shift into b and get a byte in c
    ld b, a
    ld c, (hl)
    inc hl

    ; Save offset
    push bc      ; ld iyh, b
    pop iy       ; ld iyl, c

    push bc
      ; Get the length
      call _getVariableLengthNumber_fromShadowA
    ; previous call restores us to normal af
    ; Swap the offset (on the stack) with the source pointer, to preserve the latter and do maths with the former
    ; This is cunning: it's almost equivalent to pop bc; push hl; ld h,b; ld l,c
    ex (sp), hl ;bc = len, hl=offs
    ; We need to amend the length
    ; Range       Amendment
    ; 0..127        +2
    ; 128..1279      0
    ; 1280..31999   +1
    ; 32000+        +2    <-- we don't bother with this
    push de
      ex de, hl ; de = length, hl = dest
      ex af, af' ; To scratch a
        ; First check for <128
        ld hl, #127
        sbc hl, de
        jr c, _f1
        inc bc
        inc bc
        jp _f2
_f1:
        ; Then for >=1280
        ld a, #4
        cp d
        jr nc,_f2
        inc bc
_f2:
/*
        ; First check for >= $0500 = 1280
        ; i.e. check d > 4
        ld a, 4
        cp d
        jr nc, +
        ; d > 4 so we are in the third category
        inc bc
        or a ; clear carry
+:      ; Next check for <=127 (will always fail for things that passed the first case)
        ld hl, 127
        sbc hl, de
        jr c, +
        inc bc
        inc bc
+:
*/
      pop hl     ; bc = len, de = offs, hl=junk
      push hl
        or a
        sbc hl, de
      ex af, af' ; To bitstream a
    pop de       ; hl=dest-offs, bc=len, de = dest
    call _ldir_vram_to_vram
  pop hl
  .db 0xDD
  ld h,b         ; ld ixh, b ; will be 0
  jp _mainLoop

ap_r0_gamma:
  call _getVariableLengthNumber_fromShadowA ;and a new gamma code for length
  push hl
    push de
      ex de, hl
      push iy    ; ld d, iyh
      pop de     ; ld e, iyl
      sbc hl, de
    pop de ;hl=dest-offs, bc=len, de = dest
    call _ldir_vram_to_vram
  pop hl
  .db 0xDD
  ld h,b         ; ld ixh, b ; will be 0
  jp _mainLoop


_getBitstream_variableLengthNumber_bit1:
  ld a,(hl)
  inc hl
  rla
  jp _getBitstream_variableLengthNumber_bit1_done
_getBitstream_variableLengthNumber_bit1flag:
  ld a, (hl)
  inc hl
  rla
  jp _getBitstream_variableLengthNumber_bit1flag_done
_getBitstream_variableLengthNumber_bit2:
  ld a, (hl)
  inc hl
  rla
  jp _getBitstream_variableLengthNumber_bit2_done
_getBitstream_variableLengthNumber_bit2flag:
  ld a, (hl)
  inc hl
  rla
  jp _getBitstream_variableLengthNumber_bit2flag_done
_getBitstream_variableLengthNumber_bit:
  ld a, (hl)
  inc hl
  rla
  jp _getBitstream_variableLengthNumber_bit_done


_getBitstream_variableLengthNumber_bitflag:
  ld a, (hl)
  inc hl
  rla
  ret nc
  jp _getVariableLengthNumberloop


_getVariableLengthNumber_fromShadowA:
  ; Variant of the below where we restore the bitstream a bofore we start
  ex af, af'

_getVariableLengthNumber:
  ; Reads a number encoded as all the bits of the number after the first 1 bit,
  ; separated by 1 bits and terminated by a 0. Returns the number in bc.
  ; It has a minimum value of 2, i.e. we always read at tleast two bits.
  ; The most common cases will be shorter numbers, so we have an unrolled loop for the first two bits.

  ; Accumulate into bc
  ld bc, #1

  ; Bit 1
  add a, a
  jr z, _getBitstream_variableLengthNumber_bit1
_getBitstream_variableLengthNumber_bit1_done:
  rl c
  add a, a
  jr z, _getBitstream_variableLengthNumber_bit1flag
_getBitstream_variableLengthNumber_bit1flag_done:
  ret nc

  ; Bit 2
  add a, a
  jr z, _getBitstream_variableLengthNumber_bit2
_getBitstream_variableLengthNumber_bit2_done:
  rl c
  add a, a
  jr z, _getBitstream_variableLengthNumber_bit2flag
_getBitstream_variableLengthNumber_bit2flag_done:
  ret nc

  ; Remaining bits
_getVariableLengthNumberloop:
  add a, a
  jr z, _getBitstream_variableLengthNumber_bit
_getBitstream_variableLengthNumber_bit_done:
  rl c
  rl b
  add a, a
  jr z, _getBitstream_variableLengthNumber_bitflag
_getBitstream_variableLengthNumber_bitflag_done:
  ret nc
  jp _getVariableLengthNumberloop

_ldir_vram_to_vram:
  ; Copy bc bytes from VRAM address hl to VRAM address de
  ; Both hl and de are "write" addresses ($4xxx)

  ex af, af'
  ; Make hl a read address
  res 6,h        ; ld a,h  ; xor #0x40 ; ld h,a
  ; Check if the count is below 256
  ld a,b
  or a
  jr z,_below256
  ; Else emit 256*b bytes
_b2:
  push bc
    ld c,#0xbf
    ld b,#0
    call _f3
  pop bc
  djnz _b2
  ; Then fall through for the rest
_below256:
  ; By emitting 256 at a time, we can use the out (c),r opcode
  ; for address setting, which then relieves pressure on a
  ; and saves some push/pops; and we can use djnz for the loop.
  ld b,c
  ld c,#0xbf
_f3:
_b3:
  di
  out (c),l
  out (c),h
  in a,(#0xbe)   ; saving one EI/DI around here
  out (c),e
  out (c),d
  ei
  out (#0xbe),a
  inc hl
  inc de
  djnz _b3
  ex af, af'
  ret
  
_leave:
  pop ix         ; restore ix
__endasm;
}
#pragma restore
