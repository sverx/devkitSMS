/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */
#include "SMSlib.h"

void SMS_saveTileMapArea(unsigned char x, unsigned char y, void *dst, unsigned char width, unsigned char height)
{
    unsigned char i,j;
    unsigned short *d = dst;

    for (i=0; i<height; i++) {
        SMS_setAddr((SMS_PNTAddress & ~0x4000) + (y + i) * 64 + x * 2);
        for (j=0; j<width; j++) {
            *d++ = SMS_getTile();
        }
    }
}

#pragma save
#pragma disable_warning 85

unsigned short SMS_getTile(void) __z88dk_fastcall __naked
{
    __asm
    in a, (#0xBE)  ; 11
    ld l, a        ; 4
    inc hl         ; 6
    dec hl         ; 6
    in a, (#0xBE)  ; 11
    ld h, a        ; 4
    ret
    __endasm;
}

#pragma restore
