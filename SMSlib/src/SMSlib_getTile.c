/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */
#include "SMSlib.h"
#include "SMSlib_common.c"

#pragma save
#pragma disable_warning 85
unsigned int SMS_getTile(void) __naked __z88dk_fastcall __preserves_regs(b,c,d,e,iyh,iyl) {
    __asm
    in a,(#_VDPDataPort) ; 11
    ld l,a               ; 4
    inc hl               ; 6
    dec hl               ; 6 = 27 (safe on every Game Gear too)
    in a,(#_VDPDataPort)
    ld h,a
    ret
    __endasm;
}
#pragma restore
