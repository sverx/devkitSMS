/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"
#include <stdbool.h>

unsigned char *STM_src;
unsigned int STM_dest;
unsigned char STM_HH;
unsigned char STM_oldHH;
unsigned char STM_needRestore;
unsigned char STM_ttw;
unsigned char STM_width;

void SMS_loadSTMcompressedTileMapatAddr (unsigned int dst, const void *src) {
  STM_dest=dst;
  STM_src=(unsigned char *)(src);
  STM_width=*STM_src++;
  STM_ttw=STM_width;
  STM_HH=0;
  STM_oldHH=0;
  STM_needRestore=0;
  __asm
  ld hl,(#_STM_dest)
  call _SMS_crt0_RST08

_STM_loop:
  ld hl,(#_STM_src)
  ld a,(hl)
  inc hl
  ld (#_STM_src),hl
  rra
  jr nc,_STM_noRLE
  rra
  jr nc,_STM_RLE
                         ; *****************   RLE of successive words
  and #0x3F              ; keep only last 6 bits (counter)
  add a,#2
  ld b,a                 ; b = cnt
  ld a,(#_STM_HH)
  ld d,a                 ; d = high part of tile
  ld e,(hl)              ; e = low part of tile
  inc hl
  ld (#_STM_src),hl      ; src++
  ld hl,#_STM_ttw
  ld c,(hl)              ; c = ttw

_STM_inc_RLE_loop:
  ld a,e                    ; write tile to VRAM respecting access time costraints
  out (#0xBE),a             ;        11
  ld a,d                    ;         4
  nop                       ; *delay* 4
  nop                       ; *delay* 4
  dec c                     ; ttw--   4 = 27 (VRAM SAFE ON GG TOO)
  out (#0xBE),a

  call z,_STM_setAddr       ; ttw==0?

  dec b
  jr z,_STM_inc_RLE_done    ; cnt==0?
  inc de                    ; tile++
  jr _STM_inc_RLE_loop

_STM_inc_RLE_done:
  ld hl,#_STM_HH            ; save high part of tile
  ld (hl),d
  ld hl,#_STM_ttw           ; save ttw
  ld (hl),c

_STM_check_restore:
  ld a,(#_STM_needRestore)
  or a
  jr z,_STM_loop
  xor a
  ld (#_STM_needRestore),a
  ld a,(#_STM_oldHH)
  ld (#_STM_HH),a
  jr _STM_loop

_STM_noRLE:
  rra
  jr nc,_STM_RAW
                         ; *****************  special
  rra
  jr nc,_STM_no_temp_HH
  ld b,a
  ld a,(#_STM_HH)
  ld (#_STM_oldHH),a
  ld a,#1
  ld (#_STM_needRestore),a
  ld a,b
_STM_no_temp_HH:
  and #0x1F              ; keep only last 5 bits
  ld (#_STM_HH),a
  jr _STM_loop

_STM_RLE:                ; *****************   RLE
  and #0x3F              ; keep only last 6 bits (counter)
  add a,#2
  ld b,a                 ; b = cnt
  ld a,(#_STM_HH)
  ld d,a                 ; d = high part of tile
  ld e,(hl)              ; e = low part of tile
  inc hl
  ld (#_STM_src),hl      ; src++
  ld hl,#_STM_ttw
  ld c,(hl)              ; c = ttw

_STM_RLE_loop:
  ld a,e                    ; write tile to VRAM respecting access time costraints
  out (#0xBE),a             ;        11
  ld a,d                    ;         4
  nop                       ; *delay* 4
  nop                       ; *delay* 4
  dec c                     ; ttw--   4 = 27 (VRAM SAFE ON GG TOO)
  out (#0xBE),a

  call z,_STM_setAddr       ; ttw==0?

  dec b
  jr nz,_STM_RLE_loop       ; cnt==0?

  ld hl,#_STM_ttw           ; save ttw
  ld (hl),c

  jr _STM_check_restore

_STM_RAW:                ; *****************   RAW
  and #0x3F              ; keep only last 6 bits (counter)
  or a
  jr z,_STM_leave        ; if cnt is 0 - end of STM
  ld b,a                 ; b = cnt
  ld hl,#_STM_ttw
  ld c,(hl)              ; c = ttw
_STM_RAW_loop:
  ld hl,(#_STM_src)
  ld a,(hl)
  out (#0xBE),a          ; 11  low part of tile
  inc hl                 ; 6
  ld (#_STM_src),hl      ; 16  src++
  ld a,(#_STM_HH)        ; 13  high part of tile
  out (#0xBE),a          ; xx = VRAM safe

  dec c
  call z,_STM_setAddr

  dec b
  jr nz,_STM_RAW_loop

  ld hl,#_STM_ttw
  ld (hl),c

  jr _STM_check_restore


_STM_setAddr:
   ld hl,(#_STM_dest)
   ld a,b                         ; preserve b
   ld bc,#64
   add hl,bc
   ld b,a                         ; restore b
   ld (#_STM_dest),hl
   ld c,#0xBF                     ; set VDP Control Port
   di                             ; make it interrupt SAFE
   out (c),l
   out (c),h
   ei
   ld a,(#_STM_width)
   ld (#_STM_ttw),a
   ld c,a
   ret

_STM_leave:
  __endasm;
}
