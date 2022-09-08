/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"

#pragma save
#pragma disable_warning 85

void SMS_loadSTC0compressedTilesatAddr (const void *src, unsigned int dst) __naked __sdcccall(1) {
  __asm
  ld c,#0xbf                          ; VDP_CTRL_PORT
  di
  out (c),e
  out (c),d
  ei
  dec c                               ; same as ld c,VDP_DATA_PORT_ADDRESS
  ld de,#0x00FF                       ; D = 0x00, E = 0xFF

_stc0_decompress_outer_loop:
  ld b,#4
  ld a,(hl)
  inc hl

_stc0_decompress_inner_loop:
  rla
  jr c,_compressed_byte               ; if 1X found, write 0x00 or 0xFF
  rla
  jr nc,_same_byte_or_leave
  outi                                ; write raw byte
  jr nz,_stc0_decompress_inner_loop
  jp _stc0_decompress_outer_loop

_same_byte_or_leave:
  bit 2,b
  ret nz                              ; 00 found in first 2 bits -> end of data, leave
  dec hl
  outi                                ; write same raw byte again
  jr nz,_stc0_decompress_inner_loop
  jp _stc0_decompress_outer_loop

_compressed_byte:
  rla
  jr c,_compressed_FF
  out (c),d                           ; write 0x00
  djnz _stc0_decompress_inner_loop
  jp _stc0_decompress_outer_loop

_compressed_FF:
  out (c),e                           ; write 0xFF
  djnz _stc0_decompress_inner_loop
  jp _stc0_decompress_outer_loop
  __endasm;
}

#pragma restore
