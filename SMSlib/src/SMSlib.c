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

#ifndef NO_MD_PAD_SUPPORT
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

/* the VDP registers initialization value */
const unsigned char VDPReg_init[11]={
                  0x04, /* reg0: Mode 4 */
                  0x20, /* reg1: display OFF - frame int (vblank) ON */
                  0xFF, /* reg2: PNT at 0x3800 */
                  0xFF, /* reg3: no effect (when in mode 4) */
                  0xFF, /* reg4: no effect (when in mode 4) */
                  0xFF, /* reg5: SAT at 0x3F00 */
                  0xFF, /* reg6: Sprite tiles at 0x2000 */
                  0x00, /* reg7: backdrop color (zero) */
                  0x00, /* reg8: scroll X (zero) */
                  0x00, /* reg9: scroll Y (zero) */
                  0xFF  /* regA: line interrupt count (offscreen) */
                                    };
                                    
/* the VDP registers #0 and #1 'shadow' RAM */
unsigned char VDPReg[2]={0x04, 0x20};

volatile _Bool VDPBlank;               /* used by INTerrupt */
#ifndef TARGET_GG
volatile _Bool PauseRequested;         /* used by NMI (SMS only) */
#endif
/*
volatile _Bool VDPSpriteOverflow=false;
volatile _Bool VDPSpriteCollision=false;
*/
volatile unsigned int KeysStatus,PreviousKeysStatus;
#ifndef NO_MD_PAD_SUPPORT
volatile unsigned int MDKeysStatus,PreviousMDKeysStatus;
#endif

unsigned char clipWin_x0,clipWin_y0,clipWin_x1,clipWin_y1;

#if MAXSPRITES==64
unsigned char SpriteTableY[MAXSPRITES];
#else
unsigned char SpriteTableY[MAXSPRITES+1];
#endif
unsigned char SpriteTableXN[MAXSPRITES*2];
unsigned char SpriteNextFree;

unsigned char decompBuffer[32];        /*  used by PSGaiden decompression routine */

void (*SMS_theLineInterruptHandler)(void);

#ifndef NESTED_DI_EI_SUPPORT
/* macro definitions (no nested DI/EI support) */
#define SMS_write_to_VDPRegister(VDPReg,value)    { DISABLE_INTERRUPTS; VDPControlPort=(value); VDPControlPort=(VDPReg)|0x80; ENABLE_INTERRUPTS; }
#define SMS_set_address_CRAM(address)             { DISABLE_INTERRUPTS; VDPControlPort=(address); VDPControlPort=0xC0; ENABLE_INTERRUPTS; }
#define SMS_set_address_VRAM(address)             { DISABLE_INTERRUPTS; VDPControlPort=LO(address); VDPControlPort=HI(address)|0x40; ENABLE_INTERRUPTS; }
#else
/* inline __critical functions (nested DI/EI supported!) */
inline void SMS_write_to_VDPRegister (unsigned char VDPReg, unsigned char value) {
  __critical {
    VDPControlPort=value;
    VDPControlPort=VDPReg|0x80;
  }    
}

inline void SMS_set_address_CRAM (unsigned char address) {
  __critical {
    VDPControlPort=address;
    VDPControlPort=0xC0;
  }
}

inline void SMS_set_address_VRAM (unsigned int address) {
  __critical {
    VDPControlPort=LO(address);
    VDPControlPort=HI(address)|0x40;
  }
}
#endif

inline void SMS_byte_to_VDP_data (unsigned char data) {
  /* INTERNAL FUNCTION */
  VDPDataPort=data;
}

inline void SMS_byte_array_to_VDP_data (const unsigned char *data, unsigned int size) {
  /* INTERNAL FUNCTION */
  do {
    VDPDataPort=*(data++);
  } while (--size);
}

inline void SMS_byte_brief_array_to_VDP_data (const unsigned char *data, unsigned char size) {
  /* INTERNAL FUNCTION */
  do {
    VDPDataPort=*(data++);
  } while (--size);
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
    SMS_write_to_VDPRegister(i,VDPReg_init[i]);
  /* reset sprites */
  SMS_initSprites();
  SMS_finalizeSprites();
  UNSAFE_SMS_copySpritestoSAT();
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
  SMS_write_to_VDPRegister(0x08,LO(scrollX));
}

void SMS_setBGScrollY (int scrollY) {
  SMS_write_to_VDPRegister(0x09,LO(scrollY));
}

void SMS_setBackdropColor (unsigned char entry) {
  SMS_write_to_VDPRegister(0x07,entry);
}

void SMS_useFirstHalfTilesforSprites (_Bool usefirsthalf) {
  SMS_write_to_VDPRegister(0x06,usefirsthalf?0xFB:0xFF);
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

void SMS_loadTiles (void *src, unsigned int tilefrom, unsigned int size) {
  SMS_set_address_VRAM(tilefrom*32);
  SMS_byte_array_to_VDP_data(src,size);
}

#pragma save
#pragma disable_warning 85
// SMS_loadPSGaidencompressedTiles would otherwise complain:
// warning 85: unreferenced function argument : 'src'
void SMS_loadPSGaidencompressedTiles (void *src, unsigned int tilefrom) {
/* ****************************************************************************
   Phantasy Star Gaiden Tile Decoder
   taken from http://www.smspower.org/Development/PhantasyStarGaidenTileDecoder
   (slightly modified and wrapped into a C function)
******************************************************************************* */
  SMS_set_address_VRAM(tilefrom*32);
  __asm

   pop bc                   ; move *src from stack into IX
   pop ix
   push ix
   push bc

   ld c,(ix)                ; bc = number of tiles
   inc ix
   ld b,(ix)
   inc ix

_DecompressTile:
   push bc                  ; save number of tiles
     ld b,#4                ; count 4 bitplanes
     ld de,#_decompBuffer   ; write to de
     ld c,(ix)              ; c = encoding information for 4 bitplanes
     inc ix

_DecompressBitplane:
     rlc c        ; %0x = all bits either 0 or 1
     jr nc,_AllTheSame
     rlc c        ; %11 = raw data
     jr c,_RawData

_Compressed:
     ld a,(ix)    ; get method byte
     inc ix

     ex de,hl     ; get bitplane, if it is referring to one
     ld d,a
     and #0x03
     add a,a      ; calculate address of that bitplane
     add a,a      ; = buffer + bitplane * 8
     add a,a
     ld e,a
     ld a,d       ; get method byte back
     ld d,#0x00
     ld iy,#_decompBuffer
     add iy,de    ; now iy points to the referred to bitplane
     ex de,hl

     ; now check the method byte
     cp #0x03     ; %000000pp
     jr c,_DuplicateBitplane
     cp #0x10
     jr c,_CommonValue
     cp #0x13     ; %000100pp
     jr c,_DuplicateBitplaneInvert
     cp #0x20
     jr c,_CommonValue
     cp #0x23     ; %001000pp
     jr c,_DuplicateBitplanePartial
     cp #0x40
     jr c,_CommonValue
     cp #0x43     ; %010000pp
     jr c,_DuplicateBitplanePartialInvert
     ; fall through

_CommonValue:
     ld h,a       ; h = bitmask
     ld l,(ix)    ; l = common value
     inc ix
     jr _OutputCommonValue

_RawData:
     ld h,#0x00   ; empty bitmask; no common value
     jr _OutputCommonValue

_AllTheSame:
     rlc c        ; get next bit into carry
     sbc a,a      ; will make $00 if carry = 0, $ff if it is 1
     ld l,a       ; that is the common value
     ld h,#0xff   ; full bitmask
     ; fall through

_OutputCommonValue:
     push bc
       ld b,#8      ; loop counter
_loop1:
       ld a,l       ; get common value
       rlc h        ; get bit out of bitmask
       jr c,_skip1  ; if 1, use the common value
       ld a,(ix)    ; else get it from (ix++)
       inc ix
_skip1:
       ld (de),a    ; write to dest
       inc de
       djnz _loop1  ; loop over 8 bytes
     pop bc
   jr _BitplaneDone

_DuplicateBitplane:
     ld hl,#0xff00  ; full copy bitmask, empty inversion bitmask
     jr _OutputDuplicate

_DuplicateBitplaneInvert:
     ld hl,#0xffff  ; full copy bitmask, full inversion bitmask
     jr _OutputDuplicate

_DuplicateBitplanePartial:
     ld h,(ix)    ; get copy bitmask
     ld l,#0x00   ; empty inversion bitmask
     inc ix
     jr _OutputDuplicate

_DuplicateBitplanePartialInvert:
     ld h,(ix)    ; get copy bitmask
     ld l,#0xff   ; full inversion bitmask
     inc ix
     ; fall through

_OutputDuplicate:
     push bc
       ld b,#8     ; loop counter
_loop2:
       ld a,(iy)   ; read byte to copy
       inc iy
       xor l       ; apply inversion mask
       rlc h       ; get bit out of bitmask
       jr c,_skip2 ; if 1, use the copied value
       ld a,(ix)   ; else get it from (ix++)
       inc ix
_skip2:
       ld (de),a   ; write to dest
       inc de
       djnz _loop2 ; loop over 8 bytes
     pop bc
     ; fall through

_BitplaneDone:
     dec b        ; decrement bitplane counter
     jp nz,_DecompressBitplane ; loop if not zero

_OutputTileToVRAM:
     ld de,#8              ; we are interleaving every 8th byte
     ld c,e                ; counter for the interleaving run
     ld hl,#_decompBuffer  ; point at data to write

_outLoop:
     ld b,#4               ; there are 4 bytes to interleave
     push hl
_inLoop:
       ld a,(hl)           ; read byte
       out (#0xbe),a       ; write to vram
       add hl,de           ; skip 8 bytes
       djnz _inLoop
     pop hl
     inc hl                ; next interleaving run
     dec c
     jr nz,_outLoop

   pop bc
   dec bc                  ; next tile
   ld a,b
   or c
   jp nz,_DecompressTile
  __endasm;
}
#pragma restore

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
  _Bool needRestore=false;

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

_Bool SMS_addSprite (unsigned char x, unsigned char y, unsigned char tile) {
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

_Bool SMS_addSpriteClipping (int x, int y, unsigned char tile) {
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
#if MAXSPRITES==64
  if (SpriteNextFree<MAXSPRITES)
#endif
    SpriteTableY[SpriteNextFree]=0xD0;
}

void SMS_copySpritestoSAT (void) {
  SMS_set_address_VRAM(SATAddress);
#if MAXSPRITES==64
  SMS_byte_brief_array_to_VDP_data(SpriteTableY,MAXSPRITES);
#else
  SMS_byte_brief_array_to_VDP_data(SpriteTableY,MAXSPRITES+1);
#endif
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

#ifndef NO_MD_PAD_SUPPORT
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
_Bool SMS_queryPauseRequested (void) {
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
  SMS_write_to_VDPRegister(0x0A,count);
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
#if MAXSPRITES==64
    call _outi_block-MAXSPRITES*2
#else
    call _outi_block-(MAXSPRITES+1)*2
#endif
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

#ifndef NO_MD_PAD_SUPPORT
    /* read MD controller (3 or 6 buttons) if detected */
    PreviousMDKeysStatus=MDKeysStatus;
    IOPortCtrl=TH_HI;
#endif

#ifdef TARGET_GG
    KeysStatus=~(((GGIOPort)<<8)|IOPortL);
#else
    // KeysStatus=((~IOPortH)<<8)|(~IOPortL);
    KeysStatus=~(((IOPortH)<<8)|IOPortL);
#endif

#ifndef NO_MD_PAD_SUPPORT
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

