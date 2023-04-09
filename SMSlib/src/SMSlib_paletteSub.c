/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#ifdef TARGET_GG
#define COLOR_SUBTRACT(c,r)     (((((r)&0xF)>=((c)&0xF))?0:(((c)&0xF)-((r)&0xF)))|((((r)&0xF0)>=((c)&0xF0))?0:(((c)&0xF0)-((r)&0xF0)))|((((r)&0xF00)>=((c)&0xF00))?0:(((c)&0xF00)-((r)&0xF00))))
void GG_loadBGPaletteafterColorSubtraction (const void *palette, const unsigned int subtraction_color) {
  unsigned char i;
  GG_setNextBGColoratIndex(0);
  for (i=0;i<16;i++)
    GG_setColor(COLOR_SUBTRACT(((unsigned int *)(palette))[i],subtraction_color));
}

void GG_loadSpritePaletteafterColorSubtraction (const void *palette, const unsigned int subtraction_color) {
  unsigned char i;
  GG_setNextSpriteColoratIndex(0);
  for (i=0;i<16;i++)
    GG_setColor(COLOR_SUBTRACT(((unsigned int *)(palette))[i],subtraction_color));
}
#else
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
