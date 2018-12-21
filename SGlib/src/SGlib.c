/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   code: na_th_an, sverx
   ************************************************** */

#include "SGlib.h"
#include <stdbool.h>

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
/* define IOPort (joypad) */
__sfr __at 0xDD IOPortH;

#define HI(x)				((x)>>8)
#define LO(x)				((x)&0xFF)

#ifndef MAXSPRITES
#define MAXSPRITES 			32
#endif

#define DISABLE_INTERRUPTS		__asm di __endasm
#define ENABLE_INTERRUPTS		__asm ei __endasm

#define WAIT_VRAM			__asm nop \
                                              nop \
					      nop __endasm

/*
                SG1000 VRAM memory map:

		$0000	+--------+
		        |   PG	 |	($1800 bytes, pattern generator table)
		$1800	+--------+
                        |   PN	 |	($0300 bytes, nametable)
		$1B00	+--------+
		        |   SA	 |	($0080 bytes, sprite attribute table)
		$1B80	+--------+
		        |	 |	($0480 bytes free)
		$2000	+--------+
		        |   CT	 |	($1800 bytes, colour table)
		$3800	+--------+
		        |   SG	 |	($0800 bytes, sprite generator table)
			+--------+
*/

#define PNTADDRESS			0x1800
#define SATADDRESS 			0x1B00
#define PGTADDRESS 			0x0000
#define CGTADDRESS			0x2000
#define SGTADDRESS			0x3800

/* the VDP registers initialization value */
const unsigned char VDPReg_init[8]={
                  0x02, // Mode2
                  0xa0,	// 16KB, screen off, VBlank IRQ, sprite 8x8, no zoom
                  0x06,	// PN bits 13-10 = 0 1 1 0	   (address = $1800)
                  0xff,	// CT bits 13-7  = 1 x x x x x x x (address = $2000)
                  0x03,	// PG bits 13-11 = 0 x x	   (address = $0000)
                  0x36,	// SA bits 13-7  = 0 1 1 0 1 1 0   (address = $1B00)
                  0x07,	// SG bits 13-11 = 1 1 1	   (address = $3800)
                  0x01	// text color (unused in Mode2) / backdrop (black)
};

/* the VDP registers #0 and #1 'shadow' RAM */
unsigned char	VDPReg[2]={0x02, 0xa0};

volatile bool	VDPBlank;		// used by INTerrupt
volatile bool 	PauseRequested; 	// used by NMI

#ifdef AUTODETECT_SPRITE_OVERFLOW
unsigned char spriteOverflowFlipflop=0;
unsigned char spriteOverflowCounter;
volatile bool VDPSpriteCollision=false;
volatile bool VDPSpriteOverflow=false;
#endif

volatile unsigned int KeysStatus, PreviousKeysStatus;

/* variables for sprite windowing and clipping */
unsigned int	spritesHeight=8, spritesWidth=8;
unsigned char	clipWin_x0, clipWin_y0, clipWin_x1, clipWin_y1;

#if MAXSPRITES==32
unsigned char	SpriteTable[MAXSPRITES*4];
#else
unsigned char	SpriteTable[(MAXSPRITES+1)*4];
#endif
unsigned char	SpriteNextFree;

#ifndef NESTED_DI_EI_SUPPORT
/* macro definitions (no nested DI/EI support) */
#define SG_write_to_VDPRegister(VDPReg,value)	{ DISABLE_INTERRUPTS; VDPControlPort=(value); VDPControlPort=(VDPReg)|0x80; ENABLE_INTERRUPTS; }
#define SG_set_address_VRAM(address) 		{ DISABLE_INTERRUPTS; VDPControlPort=LO(address); VDPControlPort=HI(address)|0x40; ENABLE_INTERRUPTS; }
#else
/* inline __critical functions (nested DI/EI supported!) */
inline void SG_write_to_VDPRegister (unsigned char VDPReg, unsigned char value) {
  /* INTERNAL FUNCTION */
  __critical {
    VDPControlPort=value;
    VDPControlPort=VDPReg|0x80;
  }
}

inline void SG_set_address_CRAM (unsigned char address) {
  /* INTERNAL FUNCTION */
  __critical {
    VDPControlPort=address;
    VDPControlPort=0xC0;
  }
}

inline void SG_set_address_VRAM (unsigned int address) {
  /* INTERNAL FUNCTION */
  __critical {
    VDPControlPort=LO(address);
    VDPControlPort=HI(address)|0x40;
  }
}
#endif

inline void SG_byte_to_VDP_data (unsigned char data) {
  /* INTERNAL FUNCTION */
  VDPDataPort = data;
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
  WAIT_VRAM;				/* ensure we're not pushing data too fast */
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
  SG_setClippingWindow (0, 0, 255, 191);
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

void SG_setTileatXY (unsigned char x, unsigned char y, unsigned char tile) {
  SG_set_address_VRAM (PNTADDRESS + (y << 5) + x);
  SG_byte_to_VDP_data (tile);
}

void SG_setNextTileatXY (unsigned char x, unsigned char y) {
  SG_set_address_VRAM (PNTADDRESS + (y << 5) + x);
}

void SG_setTile (unsigned char tile) {
  SG_byte_to_VDP_data (tile);
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

void SG_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1) {
  clipWin_x0=x0;
  clipWin_y0=y0;
  clipWin_x1=x1;
  clipWin_y1=y1;
}

_Bool SG_addSpriteClipping (int x, int y, unsigned char tile, unsigned char attr) {
  unsigned char idx;
  if (SpriteNextFree < MAXSPRITES) {
    if ((x > clipWin_x1) || (x < ((int) clipWin_x0 - spritesWidth)))
      return false;															// sprite clipped
    if ((y > clipWin_y1) || (y < ((int) clipWin_y0 - spritesHeight)))
      return false;															// sprite clipped
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
  return (KeysStatus&(~PreviousKeysStatus));
}

unsigned int SG_getKeysHeld (void) {
  return (KeysStatus&PreviousKeysStatus);
}

unsigned int SG_getKeysReleased (void) {
  return ((~KeysStatus)&PreviousKeysStatus);
}

_Bool SG_queryPauseRequested (void) {
  return(PauseRequested);
}

void SG_resetPauseRequest (void) {
  PauseRequested=false;
}

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
    WAIT_VRAM;													/* ensure we're not pushing data too fast */
    SG_byte_to_VDP_data(HI(value));
    size-=2;
  }
}

/* Interrupt Service Routines */
void SG_isr (void) __interrupt {
  volatile unsigned char VDPStatus=VDPStatusPort;  /* this also aknowledge interrupt at VDP */
#ifdef AUTODETECT_SPRITE_OVERFLOW
  VDPSpriteOverflow=(VDPStatus & 0x40);
  VDPSpriteCollision=(VDPStatus & 0x20);
#endif
  if (VDPStatus & 0x80) {
    VDPBlank=true;				 /* frame interrupt */
    /* read key input */
    PreviousKeysStatus=KeysStatus;
    KeysStatus=~(((IOPortH)<<8)|IOPortL);
  }
  /* Z80 disable the interrupts on ISR, so we should re-enable them explicitly */
  ENABLE_INTERRUPTS;
}

void SG_nmi_isr (void) __critical __interrupt {		/* this is for NMI */
  PauseRequested = true;
}
