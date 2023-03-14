/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#pragma save
#pragma disable_warning 85
void SMS_loadTileMapColumnatAddr (unsigned int dst, const void *src, unsigned int height) __naked __z88dk_callee __sdcccall(1) {
  // dst in hl
  // src in de
  // height onto the stack
  __asm
    pop bc             ; pop ret address
    pop iy             ; pop height
    push bc            ; push ret address

    set 6,h            ; set VRAM address for write

1$:
    ld c,#_VDPControlPort
    di                 ; make it interrupt SAFE
    out (c),l
    out (c),h
    ei

    ld a,(de)
    out (#_VDPDataPort),a
    inc de                    ; 6
    ld a,(de)                 ; 7
    inc de                    ; 6
    out (#_VDPDataPort),a     ; 11

    dec iyl            ; decrement the counter
    ret z              ; return if zero

    ld bc,#64
    add hl,bc
    jp 1$
  __endasm;
}
#pragma restore
