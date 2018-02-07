/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

unsigned char decompBuffer[32];        /*  decompression buffer */

#pragma save
#pragma disable_warning 85
// SMS_loadPSGaidencompressedTiles would otherwise complain:
// warning 85: unreferenced function argument : 'src'
void SMS_loadPSGaidencompressedTilesatAddr (void *src, unsigned int dst) {
/* ***********************************************************************************
   Phantasy Star Gaiden Tile Decoder
   taken from http://www.smspower.org/Development/PhantasyStarGaidenTileDecoder
   (slightly modified and wrapped into a C function - and it MUST preserve ix register )
************************************************************************************** */
  SMS_setAddr(dst);
__asm

   pop bc                   ; move *src from stack into hl
   pop hl
   push hl
   push bc

   ld c,(hl)                ; bc = number of tiles
   inc hl
   ld b,(hl)
   inc hl
   
   push ix                  ; preserve ix

   push hl                  ; ld ix,hl
   pop ix

_DecompressTile:
   push bc                  ; save number of tiles
     ld b,#4                ; count 4 bitplanes
     ld de,#_decompBuffer   ; write to de
     ld c,(ix)              ; c = encoding information for 4 bitplanes
     inc ix

_DecompressBitplane:
     rlc c        ; %0x = all bits either 0 or 1
     jr nc,_AllTheSame
     rlc c        ; %11 = raw data
     jr c,_RawData

_Compressed:
     ld a,(ix)    ; get method byte
     inc ix

     ex de,hl     ; get bitplane, if it is referring to one
     ld d,a
     and #0x03
     add a,a      ; calculate address of that bitplane
     add a,a      ; = buffer + bitplane * 8
     add a,a
     ld e,a
     ld a,d       ; get method byte back
     ld d,#0x00
     ld iy,#_decompBuffer
     add iy,de    ; now iy points to the referred to bitplane
     ex de,hl

     ; now check the method byte
     cp #0x03     ; %000000pp
     jr c,_DuplicateBitplane
     cp #0x10
     jr c,_CommonValue
     cp #0x13     ; %000100pp
     jr c,_DuplicateBitplaneInvert
     cp #0x20
     jr c,_CommonValue
     cp #0x23     ; %001000pp
     jr c,_DuplicateBitplanePartial
     cp #0x40
     jr c,_CommonValue
     cp #0x43     ; %010000pp
     jr c,_DuplicateBitplanePartialInvert
     ; fall through

_CommonValue:
     ld h,a       ; h = bitmask
     ld l,(ix)    ; l = common value
     inc ix
     jr _OutputCommonValue

_RawData:
     ld h,#0x00   ; empty bitmask; no common value
     jr _OutputCommonValue

_AllTheSame:
     rlc c        ; get next bit into carry
     sbc a,a      ; will make $00 if carry = 0, $ff if it is 1
     ld l,a       ; that is the common value
     ld h,#0xff   ; full bitmask
     ; fall through

_OutputCommonValue:
     push bc
       ld b,#8      ; loop counter
_loop1:
       ld a,l       ; get common value
       rlc h        ; get bit out of bitmask
       jr c,_skip1  ; if 1, use the common value
       ld a,(ix)    ; else get it from (ix++)
       inc ix
_skip1:
       ld (de),a    ; write to dest
       inc de
       djnz _loop1  ; loop over 8 bytes
     pop bc
   jr _BitplaneDone

_DuplicateBitplane:
     ld hl,#0xff00  ; full copy bitmask, empty inversion bitmask
     jr _OutputDuplicate

_DuplicateBitplaneInvert:
     ld hl,#0xffff  ; full copy bitmask, full inversion bitmask
     jr _OutputDuplicate

_DuplicateBitplanePartial:
     ld h,(ix)    ; get copy bitmask
     ld l,#0x00   ; empty inversion bitmask
     inc ix
     jr _OutputDuplicate

_DuplicateBitplanePartialInvert:
     ld h,(ix)    ; get copy bitmask
     ld l,#0xff   ; full inversion bitmask
     inc ix
     ; fall through

_OutputDuplicate:
     push bc
       ld b,#8     ; loop counter
_loop2:
       ld a,(iy)   ; read byte to copy
       inc iy
       xor l       ; apply inversion mask
       rlc h       ; get bit out of bitmask
       jr c,_skip2 ; if 1, use the copied value
       ld a,(ix)   ; else get it from (ix++)
       inc ix
_skip2:
       ld (de),a   ; write to dest
       inc de
       djnz _loop2 ; loop over 8 bytes
     pop bc
     ; fall through

_BitplaneDone:
     dec b        ; decrement bitplane counter
     jp nz,_DecompressBitplane ; loop if not zero

_OutputTileToVRAM:
     ld de,#8              ; we are interleaving every 8th byte
     ld c,e                ; counter for the interleaving run
     ld hl,#_decompBuffer  ; point at data to write

_outLoop:
     ld b,#4               ; there are 4 bytes to interleave
     push hl
_inLoop:
       ld a,(hl)           ; read byte
       out (#0xbe),a       ; write to vram
       add hl,de           ; skip 8 bytes
       djnz _inLoop
     pop hl
     inc hl                ; next interleaving run
     dec c
     jr nz,_outLoop

   pop bc
   dec bc                  ; next tile
   ld a,b
   or c
   jp nz,_DecompressTile
   pop ix                  ; restore preserved ix
__endasm;
}
#pragma restore
