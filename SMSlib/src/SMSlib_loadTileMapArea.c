/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#pragma save
#pragma disable_warning 85
void SMS_loadTileMapAreaatAddr (unsigned int dst, const void *src, unsigned char width, unsigned char height) __naked __z88dk_callee __sdcccall(1) {
  // dst in hl
  // src in de
  // height and width onto the stack
  __asm
    pop bc             ; pop ret address
    pop iy             ; pop height (iyh) and width (iyl)
    push bc            ; push ret address

    push ix            ; we need to preserve ix

                       ; now move height in ixl
    .db 0xFD           ;   --- SDCC bug workaround
    ld a,h             ; ld a,iyh
    .db 0xDD           ;   --- SDCC bug workaround
    ld l,a             ; ld ixl,a

    ld iyh,iyl         ; preserve width in iyh

    set 6,h            ; set VRAM address for write
    ld bc,#64          ; preload VRAM offset between lines

1$:
    ld a,l
    di                 ; make it interrupt SAFE
    out (#_VDPControlPort),a
    ld a,h
    out (#_VDPControlPort),a
    ei

2$:
    ld a,(de)                                           ; 7
    out (#_VDPDataPort),a
    inc de                    ; 6
    ld a,(de)                 ; 7
    inc de                    ; 6
    out (#_VDPDataPort),a     ; 11

    dec iyl            ; decrement the width counter    ; 8
    jr nz,2$           ; loop over until zero           ; 12

    dec ixl            ; decrement the height counter
    jr z,3$            ; leave if zero

    add hl,bc          ; add offset to dest address
    ld iyl,iyh         ; reload width
    jp 1$

3$:
    pop ix             ; restore original ix
    ret
  __endasm;
}
#pragma restore
