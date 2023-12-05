/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

/*  metasprites definition are a sequence of n*3 char values:
 *  (signed) delta_x, (signed) delta_y, (unsigned) tile number
 *  terminated by a METASPRITE_END terminator value (-128)       */

/* pseudo:
 *
 *   preserve metasprite's origin point
 *   prepare destination addresses for sprites data
 *   read metasprite's delta X
 *   leave if METASPRITE_END
 *   calculate sprite's final X
 *   write final X
 *   calculate sprite's final Y
 *   write final Y
 *   write sprite's tile #
 *   loop
 */

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

#ifdef NO_SPRITE_CHECKS
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
      add hl,bc                  ; HL = &SpriteTableXN[SpriteNextFree]
    pop bc                       ; BC = &SpriteTableY[SpriteNextFree]
                                 ; DE = *metasprite

_metasprite_loop:
    ld a,(de)                    ; read delta_X
    cp #METASPRITE_END
    ret z                        ; leave when metasprite completed

    inc de

    or a                         ; test delta_X sign
    jp m,_negative_delta_X

    .db 0xFD                     ;   --- SDCC issues workaround
    add a,l                      ; add a,iyl  ; delta_X + origin_X
    jr c,_clipped_sprite         ; skip this sprite when clipped

_not_clipped_sprite:
    ld (hl),a                    ; write X

    ld a,(de)                    ; get delta_Y
    inc de

    .db 0xFD                     ;   --- SDCC issues workaround
    add a,h                      ; add a,iyh   ; delta_Y + origin_Y
    cp #191                      ; should this be clipped?
    jr c,_continue               ; not when <191               (0 to 190)

    cp #240                      ; should this be clipped?
    jr nc,_continue              ; not when >=240            (240 to 255)
                                 ; this works both with 8 and 16 pixels tall sprites,
                                 ; and makes sure that $D0 does not get into the Y table

    inc de                       ; skip tile#
    jp _metasprite_loop

_continue:
    ld (bc),a                    ; write Y
    inc bc

    ld a,(de)                    ; get tile#
    inc de

    inc hl
    ld (hl),a                    ; write tile#
    inc hl

    ld a,(#_SpriteNextFree)
    inc a
    ld (#_SpriteNextFree),a

#ifdef NO_SPRITE_CHECKS
    cp #MAXSPRITES               ; ensure that we are not using too many sprites
    ret nc
#endif

    jp _metasprite_loop

_negative_delta_X:
    .db 0xFD                     ;   --- SDCC issues workaround
    add a,l                      ; add a,iyl   ; delta_X + origin_X
    jr c,_not_clipped_sprite     ; draw this sprite when not clipped

_clipped_sprite:
    inc de                       ; skip delta_Y
    inc de                       ; skip tile#
    jp _metasprite_loop
   __endasm;
}
#pragma restore
