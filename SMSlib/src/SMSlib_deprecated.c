/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )

                       NOTE:
   this is the source file containing deprecated functions
   I suggest you stop using them, as they'll be dropped
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

extern unsigned char SpriteTableY[MAXSPRITES];
extern unsigned char SpriteTableXN[MAXSPRITES*2];
extern unsigned char SpriteNextFree;

void SMS_finalizeSprites (void) {
// *DEPRECATED* - will be dropped at some point in 2018
}
