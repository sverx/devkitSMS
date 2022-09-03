/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */
#include <stdio.h>
#include "SMSlib.h"
#include "SMSlib_common.c"

extern signed int SMS_TextRenderer_offset;

void SMS_putchar (unsigned char c) {
  SMS_setTile(c+SMS_TextRenderer_offset);
}

void SMS_print (const unsigned char *str) {
  // If using SMS_TextRenderer_offset directly, SDCC
  // ends up loading it from memory inside the loop.
  //
  // The register keyword does not make a difference, but
  // it does capture the intention...
  register int off = SMS_TextRenderer_offset;

  while (*str) {
    SMS_setTile(*str + off);
    str++;
  }
}

