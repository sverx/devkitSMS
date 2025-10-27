/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   code: na_th_an, sverx
   ************************************************** */

#include "SGlib.h"
#include "SGlib_common.h"
#include <stdbool.h>

/* the VDP registers initialization value */
const unsigned char VDPReg_init[8]={
  0x02, // Mode2
  0xa0, // 16KB, screen off, VBlank IRQ, sprite 8x8, no zoom
  0x06, // PN bits 13-10 = 0 1 1 0     (address = $1800)
  0xff, // CT bits 13-7  = 1 x x x x x x x (address = $2000)
  0x03, // PG bits 13-11 = 0 x x     (address = $0000)
  0x36, // SA bits 13-7  = 0 1 1 0 1 1 0   (address = $1B00)
  0x07, // SG bits 13-11 = 1 1 1     (address = $3800)
  0x01  // text color (unused in Mode2) / backdrop (black)
};

/* ColecoVision VDP/NMI semaphore variables */
#ifdef TARGET_CV
_Bool CV_VDP_op_pending;
_Bool CV_NMI_srv_pending;
#endif

/* the VDP registers #0 and #1 'shadow' RAM */
unsigned char VDPReg[2]={0x02, 0xa0};

volatile bool VDPBlank;   // used by INTerrupt

#ifndef TARGET_CV
volatile bool PauseRequested;   // used by NMI
#endif

#ifdef AUTODETECT_SPRITE_OVERFLOW
unsigned char spriteOverflowFlipflop=0;
unsigned char spriteOverflowCounter;
volatile bool VDPSpriteCollision=false;
volatile bool VDPSpriteOverflow=false;
#endif

volatile unsigned int KeysStatus, PreviousKeysStatus;

unsigned int  spritesHeight=8, spritesWidth=8;

#if MAXSPRITES==32
unsigned char SpriteTable[MAXSPRITES*4];
#else
unsigned char SpriteTable[(MAXSPRITES+1)*4];
#endif
unsigned char SpriteNextFree;

#ifndef NO_FRAME_INT_HOOK
/* If non-NULL, will be called by SG_isr after acknowledging */
/* the interrupt and reading controller status */
void (*SG_theFrameInterruptHandler)(void);
#endif

inline void SG_byte_to_VDP_data (unsigned char data) {
  /* INTERNAL FUNCTION */
  VDPDataPort = data;
}

inline unsigned char  SG_byte_from_VDP_data (void) {
  /* INTERNAL FUNCTION */
  return(VDPDataPort);
}

inline void SG_byte_array_to_VDP_data (const unsigned char *data, unsigned int size) {
  /* INTERNAL FUNCTION */
  do {
    VDPDataPort = *(data ++);
  } while (-- size);
}

inline void SG_byte_brief_array_to_VDP_data (const unsigned char *data, unsigned char size) {
  /* INTERNAL FUNCTION */
  do {
    VDPDataPort = *(data ++);
  } while (-- size);
}

inline void SG_word_to_VDP_data (unsigned int data) {
  /* INTERNAL FUNCTION */
  VDPDataPort = LO (data);
  WAIT_VRAM;        /* ensure we're not pushing data too fast */
  VDPDataPort = HI (data);
}

void SG_VDPturnOnFeature (unsigned int feature) {
  /* turns on a VDP feature */
  VDPReg[HI(feature)]|=LO(feature);
  SG_write_to_VDPRegister(HI(feature), VDPReg[HI(feature)]);
}

void SG_VDPturnOffFeature (unsigned int feature) {
  /* turns off a VDP feature */
  VDPReg[HI(feature)]&=~LO(feature);
  SG_write_to_VDPRegister(HI(feature), VDPReg[HI(feature)]);
}

void SG_init (void) {
  unsigned char i;
  for (i=0;i<8;i++)
    SG_write_to_VDPRegister (i, VDPReg_init[i]);
  SG_initSprites ();
  SG_finalizeSprites ();
  SG_copySpritestoSAT ();
}

void SG_setBackdropColor (unsigned char entry) {
  SG_write_to_VDPRegister (0x07, entry);
}

void SG_setSpriteMode (unsigned char mode) {
  if (mode & SG_SPRITEMODE_LARGE) {
    SG_VDPturnOnFeature(SG_VDPFEATURE_USELARGESPRITES);
    spritesHeight=16;
    spritesWidth=16;
  } else {
    SG_VDPturnOffFeature(SG_VDPFEATURE_USELARGESPRITES);
    spritesHeight=8;
    spritesWidth=8;
  }
  if (mode & SG_SPRITEMODE_ZOOMED) {
    SG_VDPturnOnFeature(SG_VDPFEATURE_ZOOMSPRITES);
    spritesWidth*=2;
    spritesHeight*=2;
  } else {
    SG_VDPturnOffFeature(SG_VDPFEATURE_ZOOMSPRITES);
  }
}

void SG_loadTilePatterns (void *src, unsigned int tilefrom, unsigned int size) {
  SG_set_address_VRAM (PGTADDRESS + (tilefrom << 3));
  SG_byte_array_to_VDP_data (src, size);
}

void SG_loadTileColours (void *src, unsigned int tilefrom, unsigned int size) {
  SG_set_address_VRAM (CGTADDRESS + (tilefrom << 3));
  SG_byte_array_to_VDP_data (src, size);
}

void SG_loadSpritePatterns (void *src, unsigned int tilefrom, unsigned int size) {
  SG_set_address_VRAM (SGTADDRESS + (tilefrom << 3));
  SG_byte_array_to_VDP_data (src, size);
}

void SG_setNextTileatXY (unsigned char x, unsigned char y) {
  SG_set_address_VRAM (PNTADDRESS + (y << 5) + x);
}

void SG_setTile (unsigned char tile) {
  SG_byte_to_VDP_data (tile);
}

void SG_getNextTileatXY (unsigned char x, unsigned char y) {
  SG_set_address_VRAM_read (PNTADDRESS + (y << 5) + x);
}

unsigned char SG_getTile (void) {
  return(SG_byte_from_VDP_data());
}

void SG_loadTileMap (unsigned char x, unsigned char y, void *src, unsigned int size) {
  SG_set_address_VRAM (PNTADDRESS + (y << 5) + x);
  SG_byte_array_to_VDP_data (src, size);
}

void SG_loadTileMapArea (unsigned char x, unsigned char y,  void *src, unsigned char width, unsigned char height) {
  unsigned char cur_y;
  for (cur_y=y; cur_y<(y+height); cur_y++) {
    SG_set_address_VRAM (PNTADDRESS+(cur_y<<5)+x);
    SG_byte_brief_array_to_VDP_data (src, width);
    src=(unsigned char *)src + width;
  }
}

void SG_initSprites (void) {
  SpriteNextFree=0;
#ifdef AUTODETECT_SPRITE_OVERFLOW
  spriteOverflowCounter=0;
  spriteOverflowFlipflop=1-spriteOverflowFlipflop;
#endif
}

_Bool SG_addSprite (unsigned char x, unsigned char y, unsigned char tile, unsigned char attr) {
  unsigned char idx;
  if (SpriteNextFree < MAXSPRITES) {
    if (y - 1 != 0xd0) {
#ifdef AUTODETECT_SPRITE_OVERFLOW
      spriteOverflowCounter ++;
      if (0 == VDPSpriteOverflow || (spriteOverflowCounter & 1) == spriteOverflowFlipflop)
#endif
      {
        idx = SpriteNextFree << 2;
        SpriteTable [idx ++] = y;
        SpriteTable [idx ++] = x;
        SpriteTable [idx ++] = tile;
        SpriteTable [idx] = attr;
        SpriteNextFree ++;
      }
    }
    return true;
  } else
    return false;
}

void SG_finalizeSprites (void) {
#if MAXSPRITES==32
  if (SpriteNextFree < MAXSPRITES)
#endif
    SpriteTable[SpriteNextFree<<2]=0xd0;
}

void SG_copySpritestoSAT (void) {
  SG_set_address_VRAM (SATADDRESS);
#if MAXSPRITES==32
  SG_byte_brief_array_to_VDP_data (SpriteTable, MAXSPRITES*4);
#else
  SG_byte_brief_array_to_VDP_data (SpriteTable, (MAXSPRITES+1)*4);
#endif
}

void SG_waitForVBlank (void) {
  VDPBlank=false;
  while (!VDPBlank);
}

unsigned int SG_getKeysStatus (void) {
  return (KeysStatus);
}

unsigned int SG_getKeysPressed (void) {
  return (KeysStatus & (~PreviousKeysStatus));
}

unsigned int SG_getKeysHeld (void) {
  return (KeysStatus & PreviousKeysStatus);
}

unsigned int SG_getKeysReleased (void) {
  return ((~KeysStatus) & PreviousKeysStatus);
}

#ifndef TARGET_CV
_Bool SG_queryPauseRequested (void) {
  return (PauseRequested);
}

void SG_resetPauseRequest (void) {
  PauseRequested=false;
}
#endif

/* low level functions, just to be used for dirty tricks ;) */
void SG_VRAMmemcpy (unsigned int dst, void *src, unsigned int size) {
  SG_set_address_VRAM(dst);
  SG_byte_array_to_VDP_data(src,size);
}

void SG_VRAMmemcpy_brief (unsigned int dst, void *src, unsigned char size) {
  SG_set_address_VRAM(dst);
  SG_byte_brief_array_to_VDP_data(src,size);
}

void SG_VRAMmemset (unsigned int dst, unsigned char value, unsigned int size) {
  SG_set_address_VRAM(dst);
  while (size>0) {
    SG_byte_to_VDP_data(value);
    size--;
  }
}

void SG_VRAMmemsetW (unsigned int dst, unsigned int value, unsigned int size) {
  SG_set_address_VRAM(dst);
  while (size>0) {
    SG_byte_to_VDP_data(LO(value));
    WAIT_VRAM;                          /* ensure we're not pushing data too fast */
    SG_byte_to_VDP_data(HI(value));
    size-=2;
  }
}

#ifndef NO_FRAME_INT_HOOK
void SG_setFrameInterruptHandler (void (*theHandlerFunction)(void)) __z88dk_fastcall {
  SG_theFrameInterruptHandler=theHandlerFunction;
}
#endif

/* Interrupt Service Routines */
#ifndef TARGET_CV
void SG_isr (void) __critical __interrupt(0) {
  volatile unsigned char VDPStatus=VDPStatusPort;  /* this also aknowledge interrupt at VDP */
#ifdef AUTODETECT_SPRITE_OVERFLOW
  VDPSpriteOverflow=(VDPStatus & 0x40);
  VDPSpriteCollision=(VDPStatus & 0x20);
#endif
  if (VDPStatus & 0x80) {
    VDPBlank=true;         /* frame interrupt */
    /* read pad input */
    PreviousKeysStatus=KeysStatus;
    KeysStatus=~(((IOPortH)<<8)|IOPortL);
#ifndef NO_FRAME_INT_HOOK
    if (SG_theFrameInterruptHandler) {
      SG_theFrameInterruptHandler();
    }
#endif
  }
}

void SG_nmi_isr (void) __critical __interrupt {   /* this is for NMI */
  PauseRequested = true;
}
#else
void SG_isr_process (void) {
  volatile unsigned char VDPStatus=VDPStatusPort;  /* this also aknowledge interrupt at VDP */
#ifdef AUTODETECT_SPRITE_OVERFLOW
  VDPSpriteOverflow=(VDPStatus & 0x40);
  VDPSpriteCollision=(VDPStatus & 0x20);
#endif
  VDPBlank=true;         /* frame interrupt */
  /* read joy input */
  PreviousKeysStatus=KeysStatus;
  IOPortCTRLmode0=0xff;
  unsigned int tempKeysStatus=~(((IOPortH|0x80)<<8)|(IOPortL|0x80));
  IOPortCTRLmode1=0xff;
  KeysStatus=(((IOPortH&0x40)?0:0x8000)|((IOPortL&0x40)?0:0x80))|tempKeysStatus;
#ifndef NO_FRAME_INT_HOOK
  if (SG_theFrameInterruptHandler) {
    SG_theFrameInterruptHandler();
  }
#endif
  CV_NMI_srv_pending=false;
}

void SG_isr (void) __critical __interrupt {   /* this is for ColecoVision NMI */
  if (!CV_VDP_op_pending)
    SG_isr_process();
  else
    CV_NMI_srv_pending=true;
}
#endif
