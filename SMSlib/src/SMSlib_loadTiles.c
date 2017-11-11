/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

void SMS_loadTiles (void *src, unsigned int tilefrom, unsigned int size) {
  // SMS_set_address_VRAM(tilefrom*32);
  SMS_setAddr(0x4000|(tilefrom*32));
  SMS_byte_array_to_VDP_data(src,size);
}
