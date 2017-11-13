/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )

                       NOTE:
   this is the source file containing deprecated functions
   I suggest you stop using them, as they'll be dropped
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

#if MAXSPRITES==64
extern unsigned char SpriteTableY[MAXSPRITES];
#else
extern unsigned char SpriteTableY[MAXSPRITES+1];
#endif
extern unsigned char SpriteTableXN[MAXSPRITES*2];
extern unsigned char SpriteNextFree;

void SMS_finalizeSprites (void) {
// *DEPRECATED* - will be dropped at some point in 2018
#if MAXSPRITES==64
  if (SpriteNextFree<MAXSPRITES)
#endif
    SpriteTableY[SpriteNextFree]=0xD0;
}
