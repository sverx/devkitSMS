/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

signed int SMS_TextRenderer_offset;

void SMS_configureTextRenderer (signed int ascii_to_tile_offset) {
  SMS_TextRenderer_offset=ascii_to_tile_offset;
}

void putchar (char c) {
  SMS_setTile(c+SMS_TextRenderer_offset);
}
