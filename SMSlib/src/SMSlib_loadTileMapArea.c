/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#pragma save
#pragma disable_warning 85
// now even faster!
void SMS_loadTileMapAreaatAddr (unsigned int dst, const void *src, unsigned char width, unsigned char height) __naked __z88dk_callee __sdcccall(1) {
  // dst in hl
  // src in de
  // height and width onto the stack
  __asm
    pop bc             ; pop ret address
    pop iy             ; pop height (iyh) and width (iyl)
    push bc            ; push ret address

    set 6,h            ; set VRAM address for write
    .db 0xFD           ;   --- SDCC issues workaround
    ld a,l             ; ld a,iyl ; load width into A

1$:
    ld c,#_VDPControlPort
    di                 ; make it interrupt SAFE
    out (c),l
    out (c),h
    ei

    dec c              ; faster than ld c,#_VDPDataPort
    ex de,hl
    ld b,a             ; B=width

2$:
    outi                  ; 16
    inc b                 ;  4
    sub #0                ;  7 = 27 : SAFE on every SMS and GG
    outi         ; 16
    jr nz,2$     ; 12 = 28 : SAFE on every SMS and GG

    .db 0xFD           ;   --- SDCC issues workaround
    dec h              ; dec iyh ; decrement the height counter
    ret z              ; leave if zero

    ex de,hl
    ld c,#64
    add hl,bc          ; add offset to dest address (B is zero, C is 64)
    jp 1$
  __endasm;
}
#pragma restore
