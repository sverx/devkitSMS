/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#pragma save
#pragma disable_warning 85
#ifndef TARGET_GG
void SMS_loadBGPaletteHalfBrightness (void *palette) __z88dk_fastcall {
  // *palette will be in HL
  ASM_LD_DE_IMM(#SMS_CRAMAddress);
  ASM_DE_TO_VDP_CONTROL;
  ASM_LD_B_IMM(#16);
  __asm
1$: ld a,(hl)                  ; 7
    rrca                       ; 4
    and #0x15                  ; 7
    out (_VDPDataPort),a       ; 11
    inc hl                     ; 6
    djnz 1$                    ; 13 = *VRAM SAFE*
  __endasm;
}

void SMS_zeroBGPalette (void) {
  ASM_LD_DE_IMM(#SMS_CRAMAddress);
  ASM_DE_TO_VDP_CONTROL;
  ASM_LD_B_IMM(#16);
  __asm
    xor a
1$: out (_VDPDataPort),a       ; 11
    nop                        ; 4
    djnz 1$                    ; 13 = 28 *VRAM SAFE*
  __endasm;
}

void SMS_loadSpritePaletteHalfBrightness (void *palette) __z88dk_fastcall {
  // *palette will be in HL
  ASM_LD_DE_IMM(#SMS_CRAMAddress+0x10);
  ASM_DE_TO_VDP_CONTROL;
  ASM_LD_B_IMM(#16);
  __asm
1$: ld a,(hl)                  ; 7
    rrca                       ; 4
    and #0x15                  ; 7
    out (_VDPDataPort),a       ; 11
    inc hl                     ; 6
    djnz 1$                    ; 13  *VRAM SAFE*
  __endasm;
}

void SMS_zeroSpritePalette (void) {
  ASM_LD_DE_IMM(#SMS_CRAMAddress+0x10);
  ASM_DE_TO_VDP_CONTROL;
  ASM_LD_B_IMM(#16);
  __asm
    xor a
1$: out (_VDPDataPort),a       ; 11
    nop                        ; 4
    djnz 1$                    ; 13 = 28 *VRAM SAFE*
  __endasm;
}
#endif
#pragma restore
