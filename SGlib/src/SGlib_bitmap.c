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
  for(unsigned int tile=0; tile<((256 >> 3)*(192>>3)); tile++)
    VDPDataPort = tile;
}

#pragma save
#pragma disable_warning 85
#ifndef TARGET_CV
// profile: 393 to 491 cycles
void SG_putPixel_f (unsigned char color, unsigned int xy_coords) {
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
    di
    out (#_VDPControlPort),a     ; we want to read a byte from VRAM
    ld a,h
    or #0x20                     ; because CGTADDRESS = 0x2000
    out (#_VDPControlPort),a     ; 11
    ei                           ; 4
    nop                          ; 4
    nop                          ; 4
    nop                          ; 4 = 27 (VRAM SAFE)
    in a,(#_VDPDataPort)         ; read the byte at CGTADDRESS+offset

    ld e,a                       ; preserve A
    and #0x0F
    cp c
    jr z,p_bkgcol                ; when equal it means we want to put a pixel in background color

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
    di
    out (#_VDPControlPort),a     ; we want to write a byte to VRAM
    ld a,h
    or #0x60                     ; because because CGTADDRESS = 0x2000 and WRITE_VRAM = 0x4000
    out (#_VDPControlPort),a
    ei

    ld a,c
    out (#_VDPDataPort),a        ; 11 write new color attributes to VRAM

p_fgcol:
    ld c,#1                      ; 7

put_p:
    ld a,l                       ; 4
    nop                          ; 4
    di                           ; 4 = 30 (VRAM SAFE)
    out (#_VDPControlPort),a     ; we want to read a byte from VRAM
    ld a,h
    out (#_VDPControlPort),a
    ei

#ifndef TARGET_CV
    ld d,#0x00
#else
    ld d,#0x80
#endif
    ld e,b
    set 6,e                      ; DE = 0x0040+B (bitshift LUT in crt0 at $0040/$8040)
    ld a,(de)
    ld e,a                       ; preserve the calculated shifted bit

    in a,(#_VDPDataPort)         ; read the byte at PGTADDRESS+offset

    bit 0,c                      ; test C
    jr z,put_p_bkgcol

    or e                         ; turn on the requested bit
put_p_write:
    ld e,a                       ; preserve the new byte value

    ld a,l
    di
    out (#_VDPControlPort),a     ; we want to write a byte to VRAM
    ld a,h
    or #0x40                     ; because WRITE_VRAM = 0x4000
    out (#_VDPControlPort),a
    ei

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

  __endasm;
}
#else
#define SG_get_Tile_address(x,y)  ((unsigned int)(((y)>>3) << 8) + (((x) >> 3) << 3) + ((y) % 8))
void SG_putPixel (unsigned char x, unsigned char y, unsigned char color) {
  unsigned int address=SG_get_Tile_address(x, y);

  // check if color data update is needed
  SG_set_address_VRAM_read (CGTADDRESS + address);
  WAIT_VRAM;
  unsigned char color_data = VDPDataPort;
  if ((color_data & 0x0F) == color) {
    color=0;   // we want a pixel in the current background color
  } else if ((color_data >> 4) == color) {
    color=1;   // we want a pixel in the current foreground color
  } else {
    color_data = (color_data & 0x0F) | (color << 4);
    SG_set_address_VRAM (CGTADDRESS + address);
    VDPDataPort = color_data;
    color=1;    // we want a pixel in the new foreground color
  }

  unsigned char pattern_data=(0x80 >> (x & 0x07));
  // set pattern data
  SG_set_address_VRAM_read (PGTADDRESS + address);
  WAIT_VRAM;
  if (color) {
    pattern_data |= VDPDataPort;
  } else {
    pattern_data = VDPDataPort & (~pattern_data);
  }
  SG_set_address_VRAM (PGTADDRESS + address);
  VDPDataPort = pattern_data;
}
#endif
#pragma restore
