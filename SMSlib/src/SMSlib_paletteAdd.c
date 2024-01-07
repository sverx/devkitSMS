/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#ifdef TARGET_GG
#define COLOR_ADD(c, r) (((c & 0x00f) + (r & 0x00f) >= 0x00f ? 0x00f : (c & 0x00f) + (r & 0x00f)) | ((c & 0x0f0) + (r & 0x0f0) >= 0x0f0 ? 0x0f0 : (c & 0x0f0) + (r & 0x0f0) | ((c & 0xf00) + (r & 0xf00) >= 0xf00 ? 0xf00 : (c & 0xf00) + (r & 0xf00)))
void GG_loadBGPaletteafterColorAddition(const void *palette, const unsigned int addition_color)
{
  unsigned char i;
  GG_setNextBGColoratIndex(0);
  for (i = 0; i < 16; i++)
    GG_setColor(COLOR_ADD(((unsigned int *)(palette))[i], addition_color));
}

void GG_loadSpritePaletteafterColorAddition(const void *palette, const unsigned int addition_color)
{
  unsigned char i;
  GG_setNextSpriteColoratIndex(0);
  for (i = 0; i < 16; i++)
    GG_setColor(COLOR_ADD(((unsigned int *)(palette))[i], addition_color));
}
#else
#define COLOR_ADD(c, r) (((c & 0x03) + (r & 0x03) >= 0x03 ? 0x03 : (c & 0x03) + (r & 0x03)) | ((c & 0x0c) + (r & 0x0c) >= 0x0c ? 0x0c : (c & 0x0c) + (r & 0x0c)) | ((c & 0x30) + (r & 0x30) >= 0x30 ? 0x30 : (c & 0x30) + (r & 0x30)))

void SMS_loadBGPaletteafterColorAddition(const void *palette, const unsigned char addition_color)
{
  unsigned char i;
  SMS_setNextBGColoratIndex(0);
  for (i = 0; i < 16; i++)
    SMS_setColor(COLOR_ADD(((unsigned char *)(palette))[i], addition_color));
}

void SMS_loadSpritePaletteafterColorAddition(const void *palette, const unsigned char addition_color)
{
  unsigned char i;
  SMS_setNextSpriteColoratIndex(0);
  for (i = 0; i < 16; i++)
    SMS_setColor(COLOR_ADD(((unsigned char *)(palette))[i], addition_color));
}
#endif
