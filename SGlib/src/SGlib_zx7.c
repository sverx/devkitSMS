/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SGlib.h"

#pragma save
#pragma disable_warning 85
void SG_decompressZX7 (const void *src, void *dst) __naked {
/* **************************************************
  ZX7 decoder by Einar Saukas & Urusergi ("Turbo" version)
  C wrapper by sverx
***************************************************** */
  __asm
        ld      a, #0x80
dzx7t_copy_byte_loop:
        ldi                             ; copy literal byte
dzx7t_main_loop:
        add     a, a                    ; check next bit
        call    z, dzx7t_load_bits      ; no more bits left?
        jr      nc, dzx7t_copy_byte_loop ; next bit indicates either literal or sequence

; determine number of bits used for length (Elias gamma coding)
        push    de
        ld      bc, #1
        ld      d, b
dzx7t_len_size_loop:
        inc     d
        add     a, a                    ; check next bit
        call    z, dzx7t_load_bits      ; no more bits left?
        jr      nc, dzx7t_len_size_loop
        jp      dzx7t_len_value_start

; determine length
dzx7t_len_value_loop:
        add     a, a                    ; check next bit
        call    z, dzx7t_load_bits      ; no more bits left?
        rl      c
        rl      b
        jr      c, dzx7t_exit           ; check end marker
dzx7t_len_value_start:
        dec     d
        jr      nz, dzx7t_len_value_loop
        inc     bc                      ; adjust length

; determine offset
        ld      e, (hl)                 ; load offset flag (1 bit) + offset value (7 bits)
        inc     hl
        .db     0xcb, 0x33              ; opcode for undocumented instruction "SLL E" aka "SLS E"
        jr      nc, dzx7t_offset_end    ; if offset flag is set, load 4 extra bits
        add     a, a                    ; check next bit
        call    z, dzx7t_load_bits      ; no more bits left?
        rl      d                       ; insert first bit into D
        add     a, a                    ; check next bit
        call    z, dzx7t_load_bits      ; no more bits left?
        rl      d                       ; insert second bit into D
        add     a, a                    ; check next bit
        call    z, dzx7t_load_bits      ; no more bits left?
        rl      d                       ; insert third bit into D
        add     a, a                    ; check next bit
        call    z, dzx7t_load_bits      ; no more bits left?
        ccf
        jr      c, dzx7t_offset_end
        inc     d                       ; equivalent to adding 128 to DE
dzx7t_offset_end:
        rr      e                       ; insert inverted fourth bit into E

; copy previous sequence
        ex      (sp), hl                ; store source, restore destination
        push    hl                      ; store destination
        sbc     hl, de                  ; HL = destination - offset - 1
        pop     de                      ; DE = destination
        ldir
dzx7t_exit:
        pop     hl                      ; restore source address (compressed data)
        jp      nc, dzx7t_main_loop

dzx7t_load_bits:
        ld      a, (hl)                 ; load another group of 8 bits
        inc     hl
        rla
        ret                             ; because this function is naked
  __endasm;
}
#pragma restore
