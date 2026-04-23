/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SGlib.h"
#include "SGlib_common.h"
#include <stdbool.h>

void SG_initBitmapMode (unsigned char foreground_color, unsigned char background_color) {
  SG_VRAMmemset (PGTADDRESS, 0x00, 256*3*8);
  SG_VRAMmemset (CGTADDRESS, ((foreground_color&0x0f)<<4)|(background_color&0x0f), 256*3*8);
  SG_setNextTileatXY(0,0);
  for(unsigned int tile=0; tile<((256>>3)*(192>>3)); tile++)
    VDPDataPort = tile;
}
