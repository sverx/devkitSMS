/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SGlib.h"

void SG_loadTileMapArea (unsigned char x, unsigned char y,  void *src, unsigned char width, unsigned char height) {
  unsigned char cur_y;
  for (cur_y=y; cur_y<(y+height); cur_y++) {
    SG_VRAMmemcpy(XYtoADDR(x,cur_y),src,width);
    src=(unsigned char *)src + width;
  }
}
