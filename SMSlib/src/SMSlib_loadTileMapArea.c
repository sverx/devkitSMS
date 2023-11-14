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

    set 6,h            ; set VRAM address for write
    ld c,#64           ; preload VRAM offset between lines

1$:
    .db 0xFD           ;   --- SDCC issues workaround
    ld b,l             ; ld b,iyl ; load width into B

    ld a,l
    di                 ; make it interrupt SAFE
    out (#_VDPControlPort),a
    ld a,h
    out (#_VDPControlPort),a
    ei

2$:
    ld a,(de)                   ; 7
    out (#_VDPDataPort),a       ; 11 = 31
    inc de                                  ; 6
    ld a,(de)                               ; 7
    inc de                                  ; 6
    out (#_VDPDataPort),a                   ; 11 = 30
    djnz 2$                     ; 13

    .db 0xFD           ;   --- SDCC issues workaround
    dec h              ; dec iyh ; decrement the height counter
    ret z              ; leave if zero

    add hl,bc          ; add offset to dest address (B is zero, C is 64)
    jp 1$
  __endasm;
}
#pragma restore
