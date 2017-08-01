/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"

extern unsigned char spritesHeight, spritesWidth;
#ifdef TARGET_GG
unsigned char clipWin_x0=48,clipWin_y0=24,clipWin_x1=207,clipWin_y1=167;
#else
unsigned char clipWin_x0,clipWin_y0,clipWin_x1=255,clipWin_y1=191;
#endif

#if MAXSPRITES==64
extern unsigned char SpriteTableY[MAXSPRITES];
#else
extern unsigned char SpriteTableY[MAXSPRITES+1];
#endif
extern unsigned char SpriteTableXN[MAXSPRITES*2];
extern unsigned char SpriteNextFree;

void SMS_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1) {
  clipWin_x0=x0;
  clipWin_y0=y0;
  clipWin_x1=x1;
  clipWin_y1=y1;
}

signed char SMS_addSpriteClipping (int x, int y, unsigned char tile) {
  unsigned char *stXN;
  if ( (SpriteNextFree>=MAXSPRITES) ||                                    // no sprite left?
       (x>clipWin_x1) || (x<((int)(clipWin_x0)-(int)(spritesWidth))) ||   // clipped by x?
       (x<0) ||                                                           // x negative?
       (y>clipWin_y1) || (y<((int)(clipWin_y0)-(int)(spritesHeight))) ||  // clipped by y?
       ((y-1)==0xD0) )                                                    // y-1 is 0xD1?
    return (-1);                                                          // sprite clipped!
  SpriteTableY[SpriteNextFree]=y-1;
  stXN=&SpriteTableXN[SpriteNextFree*2];
  *stXN++=x;
  *stXN=tile;
  return(SpriteNextFree++);
}

/*
// old code
signed char SMS_addSpriteClipping (int x, int y, unsigned char tile) {
  unsigned char *stXN;
  if (SpriteNextFree<MAXSPRITES) {
    if ((x>clipWin_x1) || (x<0) || (x<((int)(clipWin_x0)-(int)(spritesWidth))))
      return (-1);                                  // sprite clipped
    if ((y>clipWin_y1) || (y<((int)(clipWin_y0)-(int)(spritesHeight))))
      return (-1)   ;                               // sprite clipped
    if ((y-1)!=0xD0) {                              // avoid placing sprites at this Y!
      SpriteTableY[SpriteNextFree]=y-1;
      stXN=&SpriteTableXN[SpriteNextFree*2];
      *stXN++=x;
      *stXN=tile;
      // older code was:
      // SpriteTableXN[SpriteNextFree*2]=x;
      // SpriteTableXN[SpriteNextFree*2+1]=tile;
      return(SpriteNextFree++);
    } else
      return (-1);
  }
  return (-1);
}
*/