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
/* define PPI_A */
__sfr __at 0xDC SC_PPI_A;
/* define PPI_B */
__sfr __at 0xDD SC_PPI_B;
/* define PPI_C */
__sfr __at 0xDE SC_PPI_C;
/* define SC_PPI_CTRL */
__sfr __at 0xDF SC_PPI_CTRL;

#define HI(x)       ((x)>>8)
#define LO(x)       ((x)&0xFF)

#ifndef MAXSPRITES
#define MAXSPRITES      32
#endif

#define DISABLE_INTERRUPTS    __asm di __endasm
#define ENABLE_INTERRUPTS     __asm ei __endasm

#define WAIT_VRAM             __asm nop \
                                    nop \
                                    nop __endasm

/*
                SG-1000 VRAM memory map:

    $0000 +--------+
          |   PG   |  ($1800 bytes, pattern generator table)
    $1800 +--------+
          |   PN   |  ($0300 bytes, nametable)
    $1B00 +--------+
          |   SA   |  ($0080 bytes, sprite attribute table)
    $1B80 +--------+
          |        |  ($0480 bytes free)
    $2000 +--------+
          |   CT   |  ($1800 bytes, colour table)
    $3800 +--------+
          |   SG   |  ($0800 bytes, sprite generator table)
          +--------+
*/

#define PNTADDRESS      0x1800
#define SATADDRESS      0x1B00
#define PGTADDRESS      0x0000
#define CGTADDRESS      0x2000
#define SGTADDRESS      0x3800

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

/* the VDP registers #0 and #1 'shadow' RAM */
unsigned char VDPReg[2]={0x02, 0xa0};

volatile bool VDPBlank;   // used by INTerrupt
volatile bool   PauseRequested;   // used by NMI

#ifdef AUTODETECT_SPRITE_OVERFLOW
unsigned char spriteOverflowFlipflop=0;
unsigned char spriteOverflowCounter;
volatile bool VDPSpriteCollision=false;
volatile bool VDPSpriteOverflow=false;
#endif

volatile unsigned int KeysStatus, PreviousKeysStatus;
volatile unsigned int KBDKeysStatus, KBDPreviousKeysStatus;

/* variables for sprite windowing and clipping */
unsigned int  spritesHeight=8, spritesWidth=8;
unsigned char clipWin_x0, clipWin_y0, clipWin_x1, clipWin_y1;

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

#ifndef NESTED_DI_EI_SUPPORT
/* macro definitions (no nested DI/EI support) */
#define SG_write_to_VDPRegister(VDPReg,value)  do{DISABLE_INTERRUPTS; VDPControlPort=(value);     VDPControlPort=(VDPReg)|0x80;     ENABLE_INTERRUPTS;}while(0)
#define SG_set_address_VRAM(address)           do{DISABLE_INTERRUPTS; VDPControlPort=LO(address); VDPControlPort=HI(address)|0x40;  ENABLE_INTERRUPTS;}while(0)
#define SG_set_address_VRAM_read(address)      do{DISABLE_INTERRUPTS; VDPControlPort=LO(address); VDPControlPort=HI(address)&~0x40; ENABLE_INTERRUPTS;}while(0)
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
      return false;                             // sprite clipped
    if ((y > clipWin_y1) || (y < ((int) clipWin_y0 - spritesHeight)))
      return false;                             // sprite clipped
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

_Bool SG_queryPauseRequested (void) {
  return (PauseRequested);
}

void SG_resetPauseRequest (void) {
  PauseRequested=false;
}

#define PPI_TEST_VALUE_1   0x55
#define PPI_TEST_VALUE_2   0xAA

_Bool SG_detectKeyboard (void) {
  bool kbd_detected=false;

  SC_PPI_CTRL=0x92;              // Init PPI (if present). PortC = Ouput, PortA + PortB = Input
  SC_PPI_C=PPI_TEST_VALUE_1;     // write test value 1
  if (SC_PPI_C==PPI_TEST_VALUE_1) {
    SC_PPI_C=PPI_TEST_VALUE_2;   // write test value 2
    if (SC_PPI_C==PPI_TEST_VALUE_2) {
      kbd_detected=true;
    }
  }

  SC_PPI_C=0x07;                 // Set row 7 (joypad) as default
  return (kbd_detected);
}

void SG_scanKeyboardJoypad (void) {
  unsigned char tmp;
  unsigned int status=0;        // all keys not pressed unless some detection happens

  SC_PPI_C=0x06;                // we'll read row 6
  tmp=SC_PPI_A;                 // read kbd data
  if (!(tmp & 0x20))            // check bit 5 (right arrow key)
    status|=PORT_A_KEY_RIGHT;
  if (!(tmp & 0x40))            // check bit 6 (up arrow key)
    status|=PORT_A_KEY_UP;
  if (!(SC_PPI_B & 0x04))       // read kbd data (on PPI_B) and check bit 2 ('CTRL' key)
    status|=PORT_B_KEY_2;

  SC_PPI_C=0x05;                // we'll read row 5
  if (!(SC_PPI_A & 0x20))       // read kbd data and check bit 5 (left arrow key)
    status|=PORT_A_KEY_LEFT;
  if (!(SC_PPI_B & 0x08))       // read kbd data (on PPI_B) and check bit 3 ('FUNC' key)
    status|=PORT_B_KEY_1;

  SC_PPI_C=0x04;                // we'll read row 4
  if (!(SC_PPI_A & 0x20))       // read kbd data and check bit 5 (down arrow key)
    status|=PORT_A_KEY_DOWN;

  SC_PPI_C=0x03;                // we'll read row 3
  if (!(SC_PPI_A & 0x10))       // read kbd data and check bit 4 (Ins/Del key)
    status|=PORT_A_KEY_2;

  SC_PPI_C=0x02;                // we'll read row 2
  tmp=SC_PPI_A;                 // read kbd data
  if (!(tmp & 0x10))            // read kbd data and check bit 4 (Home/Clr key)
    status|=PORT_A_KEY_1;
  if (!(tmp & 0x04))            // read kbd data and check bit 2 ('D' key)
    status|=PORT_B_KEY_RIGHT;

  SC_PPI_C=0x01;                // we'll read row 1
  tmp=SC_PPI_A;                 // read kbd data
  if (!(tmp & 0x02))            // read kbd data and check bit 1 ('W' key)
    status|=PORT_B_KEY_UP;
  if (!(tmp & 0x04))            // read kbd data and check bit 2 ('S' key)
    status|=PORT_B_KEY_DOWN;

  SC_PPI_C=0x00;                // we'll read row 0
  if (!(SC_PPI_A & 0x04))       // read kbd data and check bit 2 ('A' key)
    status|=PORT_B_KEY_LEFT;

  SC_PPI_C=0x07;                // set row 7 (joypad) as default

  KBDPreviousKeysStatus=KBDKeysStatus;
  KBDKeysStatus=status;
}

unsigned int SG_getKeyboardJoypadStatus (void) {
  return (KBDKeysStatus);
}

unsigned int SG_getKeyboardJoypadPressed (void) {
  return (KBDKeysStatus & (~KBDPreviousKeysStatus));
}

unsigned int SG_getKeyboardJoypadHeld (void) {
  return (KBDKeysStatus & KBDPreviousKeysStatus);
}

unsigned int SG_getKeyboardJoypadReleased (void) {
  return ((~KBDKeysStatus) & KBDPreviousKeysStatus);
}

unsigned char SG_getKeycodes (unsigned int *keys, unsigned char max_keys) {
    unsigned char count=0;

    for(unsigned char keyb_row=0; keyb_row < 8; keyb_row++) {
        unsigned int keyb_stat, row_no;

        SC_PPI_C = keyb_row;
        row_no = keyb_row << 12;
        keyb_stat=(~((SC_PPI_B << 8) | SC_PPI_A)) & 0x0FFF;
        for(unsigned int bit_mask=0x800; keyb_stat; bit_mask >>= 1) {
            if (keyb_stat & bit_mask) {
                if (count < max_keys)
                        keys[count++] = row_no + bit_mask;
                else
                    return count;
                keyb_stat -= bit_mask;
            }
        }
    }
    return count;
}

#pragma save
#pragma disable_warning 85
void SG_decompressZX7toVRAM (const void *src, unsigned int dst) __naked {
/* =====================================================================
* by Einar Saukas, Antonio Villena & Metalbrain
* modified for sms vram by aralbrec
* modified for asm by Maxim
* C wrapper/made interrupt safe/VDP timing safe by sverx
===================================================================== */
  __asm
  ld c,#0xbf     ; VDP control port
  set 6,d        ; set VRAM write bit
  di             ; set VRAM address
  out (c),e
  out (c),d
  ei
  res 6,d        ; remove VRAM write bit
  dec c          ; data port

  ld a,#0x80     ; Signal bit for flags byte (1<<7)

dzx7s_copy_byte_loop:
  outi           ; copy literal byte
  inc de

dzx7s_main_loop:

  call dzx7s_next_bit
  jr nc,dzx7s_copy_byte_loop     ; next bit indicates either literal or sequence

  ; determine number of bits used for length (Elias gamma coding)
  push de
  ld bc,#0
  ld d,b

dzx7s_len_size_loop:
  inc d
  call dzx7s_next_bit
  jr nc,dzx7s_len_size_loop

  ; determine length
dzx7s_len_value_loop:
  call nc,dzx7s_next_bit
  rl c
  rl b

  jp c,l_ret
  dec d
  jr nz,dzx7s_len_value_loop
  inc bc         ; adjust length

  ; determine offset
  ld e, (hl)     ; load offset flag (1 bit) + offset value (7 bits)
  inc hl
  sla e
  inc e
  jr nc,dzx7s_offset_end     ; if offset flag is set, load 4 extra bits
  ld d, #0x10                ; bit marker to load 4 bits

dzx7s_rld_next_bit:
  call dzx7s_next_bit
  rl d           ; insert next bit into D
  jr nc,dzx7s_rld_next_bit  ; repeat 4 times, until bit marker is out
  inc d          ; add 128 to DE
  srl d          ; retrieve fourth bit from D

dzx7s_offset_end:
  rr e           ; insert fourth bit into E

; copy previous sequence
  ex (sp),hl     ; store source, restore destination
  push hl        ; store destination
  sbc hl,de      ; HL = destination - offset - 1
  pop de         ; DE = destination
  push af

  ; ***********************
  set 6,d
  dec bc
  inc b
  inc c
  ld a,c
  ld c,#0xbf

dzx7s_outer_loop:
  push bc
  ld b,a

dzx7s_inner_loop:
  nop                     ; 4
  di                      ; 4 = 27 (safe on every Master System or Game Gear)
  out (c),l
  out (c),h
  ei             ; 4
  inc hl         ; 6
  xor a          ; 4
  ret nz         ; 5      (this ret will never happen, it is just to wait 5 cycles)
  nop            ; 4
  nop            ; 4 = 27 (safe on every Master System or Game Gear)
  in a,(#0xbe)

  nop            ; 4
  nop            ; 4
  nop            ; 4
  nop            ; 4
  nop            ; 4
  nop            ; 4
  di             ; 4 = 28 (safe on every Master System or Game Gear)
  out (c),e
  out (c),d
  ei
  out (#0xbe),a
  inc de                  ; 6
  djnz dzx7s_inner_loop   ; 13
  ld a,b
  pop bc
  djnz dzx7s_outer_loop
  ld c,b
  res 6,d
  ; ***********************

  pop af
  ld c,#0xbe
  pop hl         ; restore source address (compressed data)
  jr nc, dzx7s_main_loop

dzx7s_next_bit:
  add a,a        ; check next bit
  ret nz         ; no more bits left?
  ld a,(hl)      ; load another group of 8 bits
  inc hl
  rla
  ret

l_ret:
  pop hl
  ret
  __endasm;
}
#pragma restore

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
void SG_isr (void) __critical __interrupt(0) {
  volatile unsigned char VDPStatus=VDPStatusPort;  /* this also aknowledge interrupt at VDP */
#ifdef AUTODETECT_SPRITE_OVERFLOW
  VDPSpriteOverflow=(VDPStatus & 0x40);
  VDPSpriteCollision=(VDPStatus & 0x20);
#endif
  if (VDPStatus & 0x80) {
    VDPBlank=true;         /* frame interrupt */
    /* read key input */
    PreviousKeysStatus=KeysStatus;
    KeysStatus=~(((IOPortH)<<8)|IOPortL);
#ifndef NO_FRAME_INT_HOOK
    if (SG_theFrameInterruptHandler) {
      SG_theFrameInterruptHandler();
    }
#endif
  }
  /* Z80 disable the interrupts on ISR, so we should re-enable them explicitly */
  ENABLE_INTERRUPTS;
}

void SG_nmi_isr (void) __critical __interrupt {   /* this is for NMI */
  PauseRequested = true;
}
