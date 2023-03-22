/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#ifndef TARGET_GG
#define COLOR_SUBTRACT(c,r)     (((((r)&0x3)>=((c)&0x3))?0:(((c)&0x3)-((r)&0x3)))|((((r)&0xC)>=((c)&0xC))?0:(((c)&0xC)-((r)&0xC)))|((((r)&0x30)>=((c)&0x30))?0:(((c)&0x30)-((r)&0x30))))

void SMS_loadBGPaletteafterColorSubtraction (const void *palette, const unsigned char subtraction_color) {
  unsigned char i;
  SMS_setNextBGColoratIndex(0);
  for (i=0;i<16;i++)
    SMS_setColor(COLOR_SUBTRACT(((unsigned char *)(palette))[i],subtraction_color));
}

void SMS_loadSpritePaletteafterColorSubtraction (const void *palette, const unsigned char subtraction_color) {
  unsigned char i;
  SMS_setNextSpriteColoratIndex(0);
  for (i=0;i<16;i++)
    SMS_setColor(COLOR_SUBTRACT(((unsigned char *)(palette))[i],subtraction_color));
}
#endif
