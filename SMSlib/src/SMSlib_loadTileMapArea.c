/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

void SMS_loadTileMapArea (unsigned char x, unsigned char y, const void *src, unsigned char width, unsigned char height) {
  unsigned char sz = width * 2;
  const char *data = (const char *)src;
  unsigned int vdp_addr;

  vdp_addr = SMS_PNTAddress+(y*32+x)*2;

  do {
    SMS_setAddr(vdp_addr);

    x = sz;
    do {
      VDPDataPort=*(data++);
    } while(--x);

    vdp_addr += 64;
  } while(--height);
}
