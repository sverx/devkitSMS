/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"

unsigned char stc4_buffer[4];

#pragma save
#pragma disable_warning 85
void SMS_loadSTC4compressedTilesatAddr (const void *src, unsigned int dst) __naked __sdcccall(1) {
  __asm
  ld c,#0xbf                          ; VDP_CTRL_PORT
  di
  out (c),e                           ; set VRAM destination address
  out (c),d
  ei

_stc4_decompress_outer_loop:
  ld a,(hl)
  cp #0x20                            ; if value less than 0x20 it is a rerun or an end-of-data marker
  jr c,_reruns_or_leave

  ld b,#4
  ld de,#_stc4_buffer

_stc4_decompress_inner_loop:
  rla
  jr c,_compressed_00_or_FF           ; if 1X found, write $00 or $FF

  rla
  jr nc,_same_or_diff                 ; if 0b00 found it is same or diff

  ld c,a                              ; preserve A
  inc hl
  ld a,(hl)                           ; load uncompressed byte

_stc4_write_byte:
  out (#0xbe),a                       ; write byte to VRAM
  ld (de),a                           ; and to buffer too
  inc de                              ; advance buffer pointer
  ld a,c                              ; restore A
  djnz _stc4_decompress_inner_loop    ; we have got more to process in this byte

  inc hl                              ; move to next byte
  jp _stc4_decompress_outer_loop

_compressed_00_or_FF:
  rla
  ld c,a                              ; preserve A
  sbc a                               ; this turns the CF into $00 or $FF
  jp _stc4_write_byte

_same_or_diff:
  bit 2,b
  jr nz,_diff                         ; if byte is $00nnnnnn then it means it is a diff, not a same byte in the group

_same_byte:
  ld c,a                              ; preserve A
  ld a,(hl)                           ; we won't [inc hl] here because we're loading the same value as before so we are already on that
  jp _stc4_write_byte

; ************************************
_diff:
  rla                                 ; skip D5
  ld c,a                              ; save D4 in C MSB
  rla                                 ; skip D4

_diff_loop:
  rla
  .db 0xFD                            ;   --- SDCC issues workaround
  ld l,a                              ; ld iyl,a  (preserve A)
  jr c,_raw_value_follows             ; when 1 a data byte will follow

  ld a,(de)
  bit 7,c
  jr z,_write_diff_byte

  cpl                                 ; invert data if D4 was set
  ld (de),a                           ; and save it back into the buffer

_write_diff_byte:
  out (#0xbe),a                       ; write byte from buffer to VRAM
  .db 0xFD                            ;   --- SDCC issues workaround
  ld a,l                              ; ld a,iyl (restore A)
  inc de                              ; advance buffer pointer
  djnz _diff_loop                     ; until we are done with the 4 bits

  inc hl                              ; move to next byte
  jp _stc4_decompress_outer_loop      ; loop over

_raw_value_follows:
  inc hl
  ld a,(hl)
  ld (de),a                           ; save it into the buffer
  jp _write_diff_byte

; ************************************
_reruns_or_leave:
  and #0x1F                           ; keep the lowest 5 bits only
  ret z                               ; if value is zero, the EOD marker has been found, so leave

  ld b,a                              ; save reruns counter in B

_transfer_whole_buffer_B_times:
  ld de,#_stc4_buffer
  ld a,(de)                           ; 7
  out (#0xbe),a                       ; 11
  nop                                 ; 4
  inc de                              ; 6  = 28

  ld a,(de)                           ; 7
  out (#0xbe),a                       ; 11
  nop                                 ; 4
  inc de                              ; 6  = 28

  ld a,(de)                           ; 7
  out (#0xbe),a                       ; 11
  nop                                 ; 4
  inc de                              ; 6  = 28

  ld a,(de)                           ; 7
  out (#0xbe),a                       ; 11
  djnz _transfer_whole_buffer_B_times ; 13 = 31

  inc hl                              ; move to next byte
  jp _stc4_decompress_outer_loop      ; loop over

  __endasm;
}
#pragma restore
