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

void SMS_readVRAM(unsigned char *dst, unsigned int src, unsigned int size) __naked __z88dk_callee __preserves_regs(iyh,iyl) __sdcccall(1)
{
    // dst in hl
    // src in de
    // size onto the stack
__asm
    ld c, #0xBF     ; VDP control port

    ; Make sure this is a read (write bit is set if TILEtoADDR is used to compute src)
    ld a, d
    and a, #0x3f

    di
    out (c),e
    out (c),a
    ei

    pop de          ; pop ret address
    pop bc          ; pop size
    push de         ; push ret address

    dec bc
    inc b
    inc c

    ld a,b
    ld b,c

    ld c,#_VDPDataPort
1$:
    ini             ; 16
    jr  nz,1$       ; 12
    dec a
    jp nz, 1$
    ret

__endasm;
}
#pragma restore
