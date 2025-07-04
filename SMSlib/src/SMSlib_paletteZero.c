/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#pragma save
#pragma disable_warning 85
#ifndef TARGET_GG
void SMS_zeroBGPalette (void) {
  ASM_LD_DE_IMM(#SMS_CRAMAddress);
  ASM_DE_TO_VDP_CONTROL;
  ASM_LD_B_IMM(#16);
  __asm
    xor a
1$: out (_VDPDataPort),a       ; 11
    nop                        ; 4
    djnz 1$                    ; 13 = 28 *VRAM SAFE ON GG TOO*
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
    djnz 1$                    ; 13 = 28 *VRAM SAFE ON GG TOO*
  __endasm;
}
#else
void GG_zeroBGPalette (void) {
  ASM_LD_DE_IMM(#SMS_CRAMAddress);
  ASM_DE_TO_VDP_CONTROL;
  ASM_LD_B_IMM(#32);
  __asm
    xor a
1$: out (_VDPDataPort),a       ; 11
    nop                        ; 4
    djnz 1$                    ; 13 = 28 *VRAM SAFE ON GG TOO*
  __endasm;
}

void GG_zeroSpritePalette (void) {
  ASM_LD_DE_IMM(#SMS_CRAMAddress+0x20);
  ASM_DE_TO_VDP_CONTROL;
  ASM_LD_B_IMM(#32);
  __asm
    xor a
1$: out (_VDPDataPort),a       ; 11
    nop                        ; 4
    djnz 1$                    ; 13 = 28 *VRAM SAFE ON GG TOO*
  __endasm;
}
#endif
#pragma restore
