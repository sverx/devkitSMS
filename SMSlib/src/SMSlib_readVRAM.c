/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */
#include "SMSlib.h"

void SMS_saveTileMapArea(unsigned char x, unsigned char y, void *dst, unsigned char width, unsigned char height) {
    unsigned char i,j;
    unsigned int *d = dst;

    for (i=0; i<height; i++) {
        SMS_setAddr((SMS_PNTAddress & ~0x4000) + (y + i) * 64 + x * 2);
        for (j=0; j<width; j++) {
            *d++ = SMS_getTile();
        }
    }
}

#pragma save
#pragma disable_warning 85

unsigned int SMS_getTile(void) __naked __z88dk_fastcall __preserves_regs(b,c,d,e,iyh,iyl) {
    __asm
    in a,(#0xBE)   ; 11
    ld l,a         ; 4
    inc hl         ; 6
    dec hl         ; 6 = 27 (safe on every Game Gear too)
    in a,(#0xBE)
    ld h,a
    ret
    __endasm;
}

#pragma restore
