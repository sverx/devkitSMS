/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include <stdbool.h>
#include "SMSlib.h"

/* define VDPControlPort (SDCC z80 syntax) */
__sfr __at 0xBF VDPControlPort;
/* define VDPStatusPort */
__sfr __at 0xBF VDPStatusPort;
/* define VDPDataPort */
__sfr __at 0xBE VDPDataPort;
/* define VDPVcounter */
__sfr __at 0x7E VDPVCounterPort;
/* define VDPHcounter */
__sfr __at 0x7F VDPHCounterPort;
/* define IOPort (joypad) */
__sfr __at 0xDC IOPortL;
#ifdef TARGET_GG
/* define GG IOPort (GG START key) */
__sfr __at 0x00 GGIOPort;
#else
/* define IOPort (joypad) */
__sfr __at 0xDE IOPortH;
#endif

#ifdef MD_PAD_SUPPORT
/* define IOPortCtrl (for accessing MD pad) */
__sfr __at 0x3F IOPortCtrl;
#define TH_HI 0xF5
#define TH_LO 0xD5
#endif

#define HI(x)                 ((x)>>8)
#define LO(x)                 ((x)&0xFF)

#ifndef MAXSPRITES
#define MAXSPRITES            64
#endif

#define DISABLE_INTERRUPTS    __asm di __endasm
#define ENABLE_INTERRUPTS     __asm ei __endasm

#define WAIT_VRAM             __asm nop \
                                    nop \
                                    nop __endasm

unsigned int PNTAddress=0x3800;
unsigned int SATAddress=0x3F00;

/* the VDP registers 'shadow' RAM */
unsigned char VDPReg[0x0B]= { 0x04, /* reg0: Mode 4 */
                              0x00, /* reg1: display and frame int OFF */
                              0xFF, /* reg2: PNT at 0x3800 */
                              0xFF, /* reg3: no effect (when in mode 4) */
                              0xFF, /* reg4: no effect (when in mode 4) */
                              0xFF, /* reg5: SAT at 0x3F00 */
                              0xFF, /* reg6: Sprite tiles at 0x2000 */
                              0x00, /* reg7: backdrop color (zero) */
                              0x00, /* reg8: scroll X (zero) */
                              0x00, /* reg9: scroll Y (zero) */
                              0xFF  /* regA: line interrupt (disabled) */
                             };

volatile bool VDPBlank=false;               /* used by INTerrupt */
#ifndef TARGET_GG
volatile bool PauseRequested=false;         /* used by NMI (SMS only) */
#endif
/*
volatile bool VDPSpriteOverflow=false;
volatile bool VDPSpriteCollision=false;
*/
volatile unsigned int KeysStatus=0,PreviousKeysStatus=0;
#ifdef MD_PAD_SUPPORT
volatile unsigned int MDKeysStatus=0,PreviousMDKeysStatus=0;
#endif

unsigned char clipWin_x0,clipWin_y0,clipWin_x1,clipWin_y1;

unsigned char SpriteTableY[MAXSPRITES];
unsigned char SpriteTableXN[MAXSPRITES*2];
unsigned char SpriteNextFree;

void (*SMS_theLineInterruptHandler)(void);

/* macro instead of inline __critical functions. I had no choice with current SDCC :| */
#define SMS_write_to_VDPRegister(VDPReg,value)    { DISABLE_INTERRUPTS; VDPControlPort=(value); VDPControlPort=(VDPReg)|0x80; ENABLE_INTERRUPTS; }
#define SMS_set_address_CRAM(address)             { DISABLE_INTERRUPTS; VDPControlPort=(address); VDPControlPort=0xC0; ENABLE_INTERRUPTS; }
#define SMS_set_address_VRAM(address)             { DISABLE_INTERRUPTS; VDPControlPort=LO(address); VDPControlPort=HI(address)|0x40; ENABLE_INTERRUPTS; }

inline void SMS_byte_to_VDP_data (unsigned char data) {
  /* INTERNAL FUNCTION */
  VDPDataPort=data;
}

inline void SMS_byte_array_to_VDP_data (const unsigned char *data, unsigned int size) {
  /* INTERNAL FUNCTION */
  while (size>0) {
    VDPDataPort=*(data++);
    size--;
  }
}

inline void SMS_byte_brief_array_to_VDP_data (const unsigned char *data, unsigned char size) {
  /* INTERNAL FUNCTION */
  do {
    VDPDataPort=*(data++);
  } while (--size!=0);
}

inline void SMS_word_to_VDP_data (unsigned int data) {
  /* INTERNAL FUNCTION */
  VDPDataPort=LO(data);
  WAIT_VRAM;               /* ensure we're not pushing data too fast */
  VDPDataPort=HI(data);
}

/*
inline void SMS_word_array_to_VDP_data (const unsigned int *data, unsigned int len) {
  INTERNAL FUNCTION
  while (len--!=0) {
    VDPDataPort=LO(*(data));
    VDPDataPort=HI(*(data));
    data++;
  }
}
*/

void SMS_init (void) {
  /* Initializes the lib */
  unsigned char i;
  /* VDP initialization */
  for (i=0;i<0x0B;i++)
    SMS_write_to_VDPRegister(i,VDPReg[i]);
#ifndef TARGET_GG
  /* init Pause (SMS only)*/
  SMS_resetPauseRequest();
#endif
  /* reset clipping window */
#ifdef TARGET_GG
  SMS_setClippingWindow(48,24,207,167);
#else
  SMS_setClippingWindow(0,0,255,191);
#endif
}

void SMS_VDPturnOnFeature (unsigned int feature) {
  /* turns on a VDP feature */
  VDPReg[HI(feature)]|=LO(feature);
  SMS_write_to_VDPRegister (HI(feature),VDPReg[HI(feature)]);
}

void SMS_VDPturnOffFeature (unsigned int feature) {
  /* turns off a VDP feature */
  VDPReg[HI(feature)]&=~LO(feature);
  SMS_write_to_VDPRegister (HI(feature),VDPReg[HI(feature)]);
}

void SMS_setBGScrollX (int scrollX) {
  VDPReg[0x08]=LO(scrollX);
  SMS_write_to_VDPRegister(0x08,VDPReg[0x08]);
}

void SMS_setBGScrollY (int scrollY) {
  VDPReg[0x09]=LO(scrollY);
  SMS_write_to_VDPRegister(0x09,VDPReg[0x09]);
}

void SMS_setBackdropColor (unsigned char entry) {
  VDPReg[0x07]=entry;
  SMS_write_to_VDPRegister(0x07,VDPReg[0x07]);
}

void SMS_useFirstHalfTilesforSprites (bool usefirsthalf) {
  VDPReg[0x06]=(usefirsthalf)?0xFB:0xFF;
  SMS_write_to_VDPRegister(0x06,VDPReg[0x06]);
}

#ifdef TARGET_GG
void GG_setBGPaletteColor (unsigned char entry, unsigned int color) {
  SMS_set_address_CRAM(entry*2);
  SMS_word_to_VDP_data(color);
}

void GG_setSpritePaletteColor (unsigned char entry, unsigned int color) {
  SMS_set_address_CRAM(entry*2+0x20);
  SMS_word_to_VDP_data(color);
}

void GG_loadBGPalette (void *palette) {
  SMS_set_address_CRAM(0x00);
  SMS_byte_brief_array_to_VDP_data(palette,32);
}

void GG_loadSpritePalette (void *palette) {
  SMS_set_address_CRAM(0x20);
  SMS_byte_brief_array_to_VDP_data(palette,32);
}
#else
void SMS_setBGPaletteColor (unsigned char entry, unsigned char color) {
  SMS_set_address_CRAM(entry);
  SMS_byte_to_VDP_data(color);
}

void SMS_setSpritePaletteColor (unsigned char entry, unsigned char color) {
  SMS_set_address_CRAM(entry+0x10);
  SMS_byte_to_VDP_data(color);
}

void SMS_loadBGPalette (void *palette) {
  SMS_set_address_CRAM(0x00);
  SMS_byte_brief_array_to_VDP_data(palette,16);
}

void SMS_loadSpritePalette (void *palette) {
  SMS_set_address_CRAM(0x10);
  SMS_byte_brief_array_to_VDP_data(palette,16);
}
#endif

void SMS_loadTiles (void *src, unsigned int Tilefrom, unsigned int size) {
  SMS_set_address_VRAM(Tilefrom*32);
  SMS_byte_array_to_VDP_data(src,size);
}

void SMS_setTileatXY (unsigned char x, unsigned char y, unsigned int tile) {
  SMS_set_address_VRAM(PNTAddress+(y*32+x)*2);
  SMS_word_to_VDP_data(tile);
}

void SMS_setNextTileatXY (unsigned char x, unsigned char y) {
  SMS_set_address_VRAM(PNTAddress+(y*32+x)*2);
}

void SMS_setTile (unsigned int tile) {
  SMS_word_to_VDP_data(tile);
}

void SMS_loadTileMap (unsigned char x, unsigned char y, void *src, unsigned int size) {
  SMS_set_address_VRAM(PNTAddress+(y*32+x)*2);
  SMS_byte_array_to_VDP_data(src,size);
}

void SMS_loadTileMapArea (unsigned char x, unsigned char y,  void *src, unsigned char width, unsigned char height) {
  unsigned char cur_y;
  for (cur_y=y;cur_y<y+height;cur_y++) {
    SMS_set_address_VRAM(PNTAddress+(cur_y*32+x)*2);
    SMS_byte_brief_array_to_VDP_data(src,width*2);
    src=(unsigned char*)src+width*2;
  }
}

void SMS_loadSTMcompressedTileMap (unsigned char x, unsigned char y, unsigned char *src) {
  unsigned int HH=0x0000;
  unsigned int oldHH=0x0000;
  unsigned char cur;
  unsigned char cnt;
  bool needRestore=false;

  SMS_set_address_VRAM(PNTAddress+(y*32+x)*2);
  while (true) {
    cur=*src++;
    if (cur & 0x01) {
      if (cur & 0x02) {
        /* RLE of successive words */
        HH=(HH & 0xFF00)|*src++;
        cnt=(cur>>2)+2;
        while (cnt>0) {
          SMS_word_to_VDP_data(HH);
          if (cnt>1)
            HH++;
          cnt--;
        }
      } else {
        /* RLE of same words */
        HH=(HH & 0xFF00)|*src++;
        cnt=(cur>>2)+2;
        while (cnt>0) {
          SMS_word_to_VDP_data(HH);
          cnt--;
        }
      }
    } else {
      if (cur & 0x02) {
        /* new HH */
        if (cur & 0x04) {
          /* temporary, thus save HH into oldHH */
          oldHH=HH;
          needRestore=true;
        }
        HH=((cur>>3)<<8);
        continue;         /* to skip the restore check at the end of the while block */
      } else {
        /* RAW */
        cnt=(cur>>2);
        if (cnt==0)
          break;          /* done, thus exit the while block */
        while (cnt>0) {
          SMS_byte_to_VDP_data(*src++);
          WAIT_VRAM;                      /* ensure we're not pushing data too fast */
          SMS_byte_to_VDP_data(HI(HH));
          cnt--;
        }
      }
    }
    if (needRestore) {
      HH=oldHH;
      needRestore=false;
    }
  } /* end while */
}

void SMS_initSprites (void) {
  SpriteNextFree=0;
}

bool SMS_addSprite (unsigned char x, unsigned char y, unsigned char tile) {
  if (SpriteNextFree<MAXSPRITES) {
    if ((y-1)!=0xD0) {                          // avoid placing sprites at this Y!
      SpriteTableY[SpriteNextFree]=y-1;
      SpriteTableXN[SpriteNextFree*2]=x;
      SpriteTableXN[SpriteNextFree*2+1]=tile;
      SpriteNextFree++;
    }
    return (true);
  } else
    return (false);
}

void SMS_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1) {
  clipWin_x0=x0;
  clipWin_y0=y0;
  clipWin_x1=x1;
  clipWin_y1=y1;
}

bool SMS_addSpriteClipping (int x, int y, unsigned char tile) {
  if (SpriteNextFree<MAXSPRITES) {
    if ((x>clipWin_x1) || (x<((int)clipWin_x0-8)))
      return (false);                               // sprite clipped
    if ((y>clipWin_y1) || (y<((int)clipWin_y0-8)))
      return (false);                               // sprite clipped
    if ((y-1)!=0xD0) {                              // avoid placing sprites at this Y!
      SpriteTableY[SpriteNextFree]=y-1;
      SpriteTableXN[SpriteNextFree*2]=x;
      SpriteTableXN[SpriteNextFree*2+1]=tile;
      SpriteNextFree++;
    }
    return (true);
  } else
    return (false);
}

void SMS_finalizeSprites (void) {
  if (SpriteNextFree<MAXSPRITES)
    SpriteTableY[SpriteNextFree]=0xD0;
}

void SMS_copySpritestoSAT (void) {
  SMS_set_address_VRAM(SATAddress);
  SMS_byte_brief_array_to_VDP_data(SpriteTableY,MAXSPRITES);
  SMS_set_address_VRAM(SATAddress+128);
  SMS_byte_brief_array_to_VDP_data(SpriteTableXN,MAXSPRITES*2);
}

void SMS_waitForVBlank (void) {
  VDPBlank=false;
  while (!VDPBlank);
}

unsigned int SMS_getKeysStatus (void) {
  return (KeysStatus);
}

unsigned int SMS_getKeysPressed (void) {
  return (KeysStatus&(~PreviousKeysStatus));
}

unsigned int SMS_getKeysHeld (void) {
  return (KeysStatus&PreviousKeysStatus);
}

unsigned int SMS_getKeysReleased (void) {
  return ((~KeysStatus)&PreviousKeysStatus);
}

#ifdef MD_PAD_SUPPORT
unsigned int SMS_getMDKeysStatus (void) {
  return (MDKeysStatus);
}

unsigned int SMS_getMDKeysPressed (void) {
  return (MDKeysStatus&(~PreviousMDKeysStatus));
}

unsigned int SMS_getMDKeysHeld (void) {
  return (MDKeysStatus&PreviousMDKeysStatus);
}

unsigned int SMS_getMDKeysReleased (void) {
  return ((~MDKeysStatus)&PreviousMDKeysStatus);
}
#endif

#ifndef TARGET_GG
bool SMS_queryPauseRequested (void) {
  return(PauseRequested);
}

void SMS_resetPauseRequest (void) {
  PauseRequested=false;
}
#endif

void SMS_setLineInterruptHandler (void (*theHandlerFunction)(void)) {
  SMS_theLineInterruptHandler=theHandlerFunction;
}

void SMS_setLineCounter (unsigned char count) {
  VDPReg[0x0A]=count;
  SMS_write_to_VDPRegister(0x0A,VDPReg[0x0A]);
}

/* Vcount */
unsigned char SMS_getVCount (void) {
  return(VDPVCounterPort);
}

/* Hcount */
unsigned char SMS_getHCount (void) {
  return(VDPHCounterPort);
}

/* low level functions, just to be used for dirty tricks ;) */
void SMS_VRAMmemcpy (unsigned int dst, void *src, unsigned int size) {
  SMS_set_address_VRAM(dst);
  SMS_byte_array_to_VDP_data(src,size);
}

void SMS_VRAMmemcpy_brief (unsigned int dst, void *src, unsigned char size) {
  SMS_set_address_VRAM(dst);
  SMS_byte_brief_array_to_VDP_data(src,size);
}

void SMS_VRAMmemset (unsigned int dst, unsigned char value, unsigned int size) {
  SMS_set_address_VRAM(dst);
  while (size>0) {
    SMS_byte_to_VDP_data(value);
    size--;
  }
}

void SMS_VRAMmemsetW (unsigned int dst, unsigned int value, unsigned int size) {
  SMS_set_address_VRAM(dst);
  while (size>0) {
    SMS_byte_to_VDP_data(LO(value));
    WAIT_VRAM;                          /* ensure we're not pushing data too fast */
    SMS_byte_to_VDP_data(HI(value));
    size-=2;
  }
}

/* VRAM unsafe functions. Fast, but dangerous! */
void UNSAFE_SMS_copySpritestoSAT (void) {
  SMS_set_address_VRAM(SATAddress);
  __asm
    ld c,#_VDPDataPort
    ld hl,#_SpriteTableY
    call _outi_block-MAXSPRITES*2
  __endasm;
  SMS_set_address_VRAM(SATAddress+128);
  __asm
    ld c,#_VDPDataPort
    ld hl,#_SpriteTableXN
    jp _outi_block-MAXSPRITES*4
  __endasm;
}

/* Interrupt Service Routines */
void SMS_isr (void) __interrupt {
  /*
  volatile unsigned char VDPStatus=VDPStatusPort;
  */

  if (VDPStatusPort & 0x80) {              /* this also aknowledge interrupt at VDP */
    VDPBlank=true;                         /* frame interrupt */
    /*
    VDPSpriteOverflow=(VDPStatus & 0x40);
    VDPSpriteCollision=(VDPStatus & 0x20);
    */

    /* read key input */
    PreviousKeysStatus=KeysStatus;

#ifdef MD_PAD_SUPPORT
    /* read MD controller (3 or 6 buttons) if detected */
    PreviousMDKeysStatus=MDKeysStatus;
    IOPortCtrl=TH_HI;
#endif

#ifdef TARGET_GG
    KeysStatus=~(((GGIOPort)<<8)|IOPortL);
#else
    KeysStatus=((~IOPortH)<<8)|(~IOPortL);
#endif

#ifdef MD_PAD_SUPPORT
    IOPortCtrl=TH_LO;
    MDKeysStatus=IOPortL;
    if (!(MDKeysStatus & 0x0C)) {           /* verify it's a MD pad */
      MDKeysStatus=(~MDKeysStatus)&0x30;    /* read MD_A & MD_START */
      IOPortCtrl=TH_HI;
      IOPortCtrl=TH_LO;
      if (!(IOPortL & 0x0F)) {              /* verify we're reading a 6 buttons pad */
        IOPortCtrl=TH_HI;
        MDKeysStatus|=(~IOPortL)&0x0F;      /* read MD_MODE, MD_X, MD_Y, MD_Z */
        IOPortCtrl=TH_LO;
      }
    } else
      MDKeysStatus=0;
#endif
  } else
    SMS_theLineInterruptHandler();          /* line interrupt */

  /* Z80 disable the interrupts on ISR, so we should re-enable them explicitly */
  ENABLE_INTERRUPTS;
}

void SMS_nmi_isr (void) __critical __interrupt {          /* this is for NMI */
#ifndef TARGET_GG
  PauseRequested=true;
#endif
}


/* *********** TEMPORARY CODE (needs fixing) ************************ */

/*
void SMS_VDPSetSATLocation (unsigned int location) {
  VDPReg[5]=(location>>7)|0b10000001;
  SMS_write_to_VDPRegister (5,VDPReg[5]);
}

void SMS_VDPSetPNTLocation (unsigned int location) {
  VDPReg[2]=(location>>10)|0b11110001;
  SMS_write_to_VDPRegister (2,VDPReg[2]);
}

void SMS_VDPSetSpritesLocation (unsigned int location) {
  VDPReg[6]=(location>>11)|0b11111011;
  SMS_write_to_VDPRegister (6,VDPReg[6]);
}
*/

/* *********** END ************************ */


/* inline critical function doesn't work correctly with SDCC 3.4.0 */
/*
inline void SMS_write_to_VDPRegister (unsigned char VDPReg, unsigned char value) {
   INTERNAL FUNCTION
  __critical {
    VDPControlPort=value;
    VDPControlPort=VDPReg|0x80;
  }
}

inline void SMS_set_address_CRAM (unsigned char address) {
   INTERNAL FUNCTION
  __critical {
    VDPControlPort=address;
    VDPControlPort=0xC0;
  }
}

inline void SMS_set_address_VRAM (unsigned int address) {
   INTERNAL FUNCTION
  __critical {
    VDPControlPort=LO(address);
    VDPControlPort=HI(address)|0x40;
  }
}
*/




/* *********** OLD CODE (that won't probably be used) ************************ */


/*
void SMS_VDPToggleFeature (unsigned int feature, bool on) {

  if (on)
    VDPReg[HI(feature)]|=LO(feature);
  else
    VDPReg[HI(feature)]&=~LO(feature); 
  SMS_write_to_VDPRegister (HI(feature),VDPReg[HI(feature)]);
}
*/
