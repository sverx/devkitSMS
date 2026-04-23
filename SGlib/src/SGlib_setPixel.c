/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SGlib.h"
#include "SGlib_common.h"
#include <stdbool.h>

#pragma save
#pragma disable_warning 85
// profile: 227 cycles (SG-1000)
void SG_setPixel (unsigned char x, unsigned char y) __preserves_regs(iyh,iyl) {
  __asm

    /* in:  A x coord
            L y coord   */

    ld e,a
    and #0x07
    ld b,a        ; (x) & 0x07

    ld a,e
    and #0xF8     ; (x) & 0xF8
    ld h,a

    ld a,l
    and #0x7      ; (y) % 8
    add h         ; ((y) % 8) + ((x) & 0xF8)
    ld e,a

    ld a,l
    rrca
    rrca
    rrca
    and #0x1F     ; (y) >> 3
    ld d,a

    ex de,hl

    ; HL now holds the calculated offset
    ; B holds how many times we have to right shift the bit (0-7)

    ld a,l
#ifndef TARGET_CV
    di
#else
    rst 0x10
#endif
    out (#_VDPControlPort),a     ; we want to read a byte from VRAM
    ld a,h
    out (#_VDPControlPort),a
#ifndef TARGET_CV
    ei
#else
    rst 0x08
#endif

#ifndef TARGET_CV
    ld d,#0x00                   ; bitshift LUT in crt0 at $0040 (SG-1000)
#else
    ld d,#0x80                   ; bitshift LUT in crt0 at $8040 (ColecoVision)
#endif
    ld e,b
    set 6,e                      ; DE = LUT+B
    ld a,(de)
    ld e,a                       ; preserve the calculated shifted bit

    in a,(#_VDPDataPort)         ; read the byte at PGTADDRESS+offset

    or e                         ; turn on the requested bit
    ld e,a                       ; preserve the new byte value

    ld a,l
#ifndef TARGET_CV
    di
#else
    rst 0x10
#endif
    out (#_VDPControlPort),a     ; we want to write a byte to VRAM
    ld a,h
    or #0x40                     ; because WRITE_VRAM = 0x4000
    out (#_VDPControlPort),a
#ifndef TARGET_CV
    ei
#else
    rst 0x08
#endif
    ld a,e                       ; restore the new byte value
    out (#_VDPDataPort),a        ; write updated pattern to VRAM
    ret                          ; process complete!
  __endasm;
}
#pragma restore
