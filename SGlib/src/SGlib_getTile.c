/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   code: na_th_an, sverx
   ************************************************** */

#include "SGlib.h"
#include "SGlib_common.h"

inline unsigned char  SG_byte_from_VDP_data (void) {
  /* INTERNAL FUNCTION */
  return(VDPDataPort);
}

void SG_getNextTileatXY (unsigned char x, unsigned char y) {
  SG_set_address_VRAM_read (PNTADDRESS + (y << 5) + x);
}

unsigned char SG_getTile (void) {
  return(SG_byte_from_VDP_data());
}
