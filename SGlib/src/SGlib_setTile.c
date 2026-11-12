/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   code: na_th_an, sverx
   ************************************************** */

#include "SGlib.h"
#include "SGlib_common.h"

void SG_setNextTileatXY (unsigned char x, unsigned char y) {
  SG_set_address_VRAM (PNTADDRESS + (y << 5) + x);
}

void SG_setTile (unsigned char tile) {
  VDPDataPort = tile;
}
