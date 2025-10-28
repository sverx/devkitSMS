/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SGlib.h"
#include "SGlib_common.h"
#include <stdbool.h>

void SG_initBitmapMode (unsigned char foreground_color, unsigned char background_color) {
  SG_VRAMmemset (PGTADDRESS, 0x00, 256*3*8);
  SG_VRAMmemset (CGTADDRESS, ((foreground_color&0x0f)<<4)|(background_color&0x0f), 256*3*8);
  SG_setNextTileatXY(0,0);
  for(unsigned int tile=0; tile<((256>>3)*(192>>3)); tile++)
    VDPDataPort = tile;
}

#pragma save
#pragma disable_warning 85
// profile: 393 to 491 cycles (SG-1000)
void SG_putPixel_f (unsigned char color, unsigned int xy_coords) __preserves_regs(iyh,iyl) {
  __asm

    /* in:  A color
            D x coord
            E y coord   */

    and #0x0F
    ld c,a        ; preserve color in C (0-15)

    ld a,d
    and #0xF8     ; (x) & 0xF8
    ld l,a

    ld a,d
    and #0x07
    ld b,a        ; (x) & 0x07

    ld a,e
    and #0x7      ; (y) % 8
    add l
    ld l,a        ; ((x) & 0xF8) + ((y) % 8))

    ld a,e
    rrca
    rrca
    rrca
    and #0x1F     ; (y) >> 3
    ld h,a

    ; HL now holds the calculated offset
    ; B holds how many times we have to right shift the bit (0-7)
    ; C holds the requested pixel color

    ld a,l
#ifndef TARGET_CV
    di
#else
    push hl
      ld hl,#_CV_VDP_op_pending
      ld (hl),#1
    pop hl
#endif
    out (#_VDPControlPort),a     ; we want to read a byte from VRAM
    ld a,h
    or #0x20                     ; because CGTADDRESS = 0x2000
    out (#_VDPControlPort),a     ; 11
#ifndef TARGET_CV
    ei                           ; 4
    nop                          ; 4
    nop                          ; 4
    nop                          ; 4 = 27 (VRAM SAFE)
#else
    call putpixel_cv_ei
#endif
    in a,(#_VDPDataPort)         ; read the byte at CGTADDRESS+offset

    ld e,a                       ; preserve A
    and #0x0F
    cp c
#ifndef TARGET_CV
    jr z,p_bkgcol                ; when equal it means we want to put a pixel in background color
#else
    jp z,p_bkgcol                ; when equal it means we want to put a pixel in background color
#endif

    ld a,e                       ; restore A
    rrca
    rrca
    rrca
    rrca
    and #0x0F
    cp c
    jr z,p_fgcol                 ; when equal it means we want to put a pixel in the current foreground color

    ld a,c                       ; calculate new foreground color (color << 4)
    add a,a
    add a,a
    add a,a
    add a,a
    ld c,a

    ld a,e
    and #0x0F
    or c
    ld c,a                       ; C now holds the new foreground color with the old background color

    ld a,l
#ifndef TARGET_CV
    di
#else
    push hl
      ld hl,#_CV_VDP_op_pending
      ld (hl),#1
    pop hl
#endif
    out (#_VDPControlPort),a     ; we want to write a byte to VRAM
    ld a,h
    or #0x60                     ; because because CGTADDRESS = 0x2000 and WRITE_VRAM = 0x4000
    out (#_VDPControlPort),a
#ifndef TARGET_CV
    ei
#else
    call putpixel_cv_ei
#endif
    ld a,c
    out (#_VDPDataPort),a        ; 11 write new color attributes to VRAM

p_fgcol:
    ld c,#1                      ; 7

put_p:
    ld a,l                       ; 4
#ifndef TARGET_CV
    nop                          ; 4
    di                           ; 4 = 30 (VRAM SAFE)
#else
    push hl
      ld hl,#_CV_VDP_op_pending
      ld (hl),#1
    pop hl
#endif
    out (#_VDPControlPort),a     ; we want to read a byte from VRAM
    ld a,h
    out (#_VDPControlPort),a
#ifndef TARGET_CV
    ei
#else
    call putpixel_cv_ei
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

    bit 0,c                      ; test C
    jr z,put_p_bkgcol

    or e                         ; turn on the requested bit
put_p_write:
    ld e,a                       ; preserve the new byte value

    ld a,l
#ifndef TARGET_CV
    di
#else
    push hl
      ld hl,#_CV_VDP_op_pending
      ld (hl),#1
    pop hl
#endif
    out (#_VDPControlPort),a     ; we want to write a byte to VRAM
    ld a,h
    or #0x40                     ; because WRITE_VRAM = 0x4000
    out (#_VDPControlPort),a
#ifndef TARGET_CV
    ei
#else
    call putpixel_cv_ei
#endif
    ld a,e                       ; restore the new byte value
    out (#_VDPDataPort),a        ; write updated pattern to VRAM
    ret                          ; process complete!

put_p_bkgcol:
    ld d,a                       ; preserve the read value
    ld a,e
    cpl                          ; complement the pattern
    ld e,a
    ld a,d
    and e                        ; turn off the requested bit
    jp put_p_write

p_bkgcol:
    ld c,#0
    jp put_p

#ifdef TARGET_CV
putpixel_cv_ei:
    push hl
      ld hl,#_CV_VDP_op_pending
      ld (hl),#0
      ld hl,#_CV_NMI_srv_pending
      bit 0,(hl)
      jr z, skip_call
      push iy
      push de
      push bc
      push af
        call _SG_isr_process
      pop af
      pop bc
      pop de
      pop iy
skip_call:
    pop hl
    ret
#endif
  __endasm;
}
#pragma restore
