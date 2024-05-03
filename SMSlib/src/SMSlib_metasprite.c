/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

/*  metasprites definition are a sequence of n*3 char values:
 *  (signed) delta_x, (signed) delta_y, (unsigned) tile number
 *  terminated by a METASPRITE_END terminator value (-128)
 *  if METASPRITE_DELTA_TILES is defined, also tile number is
 *  signed and is relative to MetaSpriteBaseTile variable */

/* pseudo:
 *
 *   preserve metasprite's origin point
 *   prepare destination addresses for sprites data
 *   read metasprite's delta X
 *   leave if METASPRITE_END
 *   calculate sprite's final X
 *   write final X
 *   read metasprite's delta Y
 *   calculate sprite's final Y
 *   write final Y
 *   read metasprite's tile #
 *   write sprite's tile #
 *   loop
 */

#ifdef METASPRITE_DELTA_TILES
  unsigned char MetaSpriteBaseTile;
#endif

#pragma save
#pragma disable_warning 85
void SMS_addMetaSprite_f (unsigned int origin_yx, void *metasprite) __naked __sdcccall(1) {
  // origin_Y passed in H
  // origin_X passed in L
  // *metasprite passed in DE
  __asm
    dec h                        ; origin_Y = origin_Y-1
    push hl
    pop iy                       ; origin X in iyl, origin Y in iyh

    ld a,(#_SpriteNextFree)

#ifndef NO_SPRITE_CHECKS
    cp #MAXSPRITES               ; ensure that we are not using too many sprites
    ret nc
#endif

    ld c,a
    ld b,#0
    ld hl,#_SpriteTableY
    add hl,bc

    push hl
      add a,a
      ld c,a
      ld hl,#_SpriteTableXN
      add hl,bc
    pop bc                       ; BC = &SpriteTableY[SpriteNextFree]
    ex de,hl                     ; DE = &SpriteTableXN[SpriteNextFree]
                                 ; HL = *metasprite

metasprite_loop:
    ld a,(hl)                    ; read delta_X
    cp #METASPRITE_END
    ret z                        ; leave when metasprite completed

    inc hl

    or a                         ; test delta_X sign
    jp m, negative_delta_X

    .db 0xFD                     ;   --- SDCC issues workaround
    add a,l                      ; add a,iyl  ; delta_X + origin_X
    jr c, h_clipped              ; skip this sprite when clipped

not_h_clipped:
    ld (de),a                    ; write X

    ld a,(hl)                    ; get delta_Y
    inc hl

    .db 0xFD                     ;   --- SDCC issues workaround
    add a,h                      ; add a,iyh   ; delta_Y + origin_Y
    cp #191                      ; should this be clipped?
    jr nc, check_v_clipped       ; surely not when 0<=y<=190 (that is row 1 to row 191 on screen)

not_v_clipped:
    ld (bc),a                    ; write Y
    inc bc

#ifdef METASPRITE_DELTA_TILES
    ld a,(#_MetaSpriteBaseTile)  ; get tile # base
    add a,(hl)                   ; add tile # delta
#else
    ld a,(hl)                    ; get tile#
#endif

    inc hl
    inc de
    ld (de),a                    ; write tile#
    inc de

    ld a,(#_SpriteNextFree)
    inc a
    ld (#_SpriteNextFree),a

#ifndef NO_SPRITE_CHECKS
    cp #MAXSPRITES               ; ensure that we are not using too many sprites
    ret nc
#endif

    jp metasprite_loop

check_v_clipped:
#ifdef NO_SPRITE_ZOOM
    cp #241                      ; should this be clipped? yes when y<=240 (above row 240 on screen)
#else
    cp #225                      ; should this be clipped? yes when y<=224 (above row 224 on screen)
#endif
    jr c, v_clipped              ; this works both with 8 and 16 pixels tall sprites (and zoomed ones)
                                 ; and makes sure that $D0 does not get into the Y table
    jp not_v_clipped

negative_delta_X:
    .db 0xFD                     ;   --- SDCC issues workaround
    add a,l                      ; add a,iyl   ; delta_X + origin_X
    jp c, not_h_clipped          ; draw this sprite when not clipped

h_clipped:
    inc hl                       ; skip delta_Y
v_clipped:
    inc hl                       ; skip tile#
    jp metasprite_loop
   __endasm;
}
#pragma restore
