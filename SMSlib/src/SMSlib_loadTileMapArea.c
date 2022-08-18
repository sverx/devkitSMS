/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

void SMS_loadTileMapArea (unsigned char x, unsigned char y, const void *src, unsigned char width, unsigned char height) {
  unsigned char sz = width * 2;
  const char *data = (const char *)src;
  unsigned char skip = 64 - width * 2;
  unsigned int vdp_addr;

  vdp_addr = SMS_PNTAddress+(y*32+x)*2;

  y = height;
  do {
    SMS_setAddr(vdp_addr);
	x = sz;
	do {
	    VDPDataPort=*(data++);
	} while(--x);

	data += skip;
	vdp_addr += 64;
  } while(--y);
}
