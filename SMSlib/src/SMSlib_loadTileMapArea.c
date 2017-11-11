/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

void SMS_loadTileMapArea (unsigned char x, unsigned char y,  void *src, unsigned char width, unsigned char height) {
  unsigned char cur_y;
  for (cur_y=y;cur_y<y+height;cur_y++) {
    // SMS_set_address_VRAM(SMS_PNTAddress+(cur_y*32+x)*2);
    SMS_setAddr(SMS_PNTAddress+(cur_y*32+x)*2);
    SMS_byte_brief_array_to_VDP_data(src,width*2);
    src=(unsigned char*)src+width*2;
  }
}
