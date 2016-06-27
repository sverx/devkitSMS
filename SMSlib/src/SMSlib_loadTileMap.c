/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

void SMS_loadTileMap (unsigned char x, unsigned char y, void *src, unsigned int size) {
  // SMS_set_address_VRAM(SMS_PNTAddress+(y*32+x)*2);
  SMS_setAddr(SMS_PNTAddress+(y*32+x)*2);
  SMS_byte_array_to_VDP_data(src,size);
}
