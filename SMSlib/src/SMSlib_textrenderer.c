/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */
#include <stdio.h>
#include "SMSlib.h"
#include "SMSlib_common.c"

signed int SMS_TextRenderer_offset;

void SMS_configureTextRenderer (signed int ascii_to_tile_offset) __z88dk_fastcall {
  SMS_TextRenderer_offset=ascii_to_tile_offset;
}

#pragma save
#pragma disable_warning 59

int putchar (int c) {
  SMS_setTile(c+SMS_TextRenderer_offset);
}

#pragma restore
