/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

void SMS_load1bppTiles (void *src, unsigned int tilefrom, unsigned int size, unsigned char color0, unsigned char color1) {
  unsigned char *s=(unsigned char *)src;
  unsigned char mask;
  SMS_setAddr(0x4000|(tilefrom*32));
  do {
    for (mask=0x01;mask<0x10;mask<<=1) {
      if ((color0^color1)&mask)
        if (color1&mask)
          SMS_byte_to_VDP_data(*s);
        else
          SMS_byte_to_VDP_data(~*s);
      else
        if (color1&mask)
          SMS_byte_to_VDP_data(0xff);
        else
          SMS_byte_to_VDP_data(0x00);
    }
    s++;
  } while (--size);
}
