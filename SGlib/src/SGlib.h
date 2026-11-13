/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   code: na_th_an, sverx
   ************************************************** */

// #define TARGET_CV
/* to recompile the library for the ColecoVision */

/*  --------------------------------------------------------- *
        VRAM memory map

    $0000 +---------+
          |   PGT   |  ($1800 bytes, pattern generator table)
    $1800 +---------+
          |   PNT   |  ($0300 bytes, nametable)
    $1B00 +---------+
          |   SAT   |  ($0080 bytes, sprite attribute table)
    $1B80 +---------+
          |         |  ($0480 bytes free)
    $2000 +---------+
          |   CGT   |  ($1800 bytes, colour table)
    $3800 +---------+
          |   SGT   |  ($0800 bytes, sprite generator table)
          +---------+
 *  --------------------------------------------------------- */

#define PNTADDRESS      0x1800
#define SATADDRESS      0x1B00
#define PGTADDRESS      0x0000
#define CGTADDRESS      0x2000
#define SGTADDRESS      0x3800

/* library initialization. you don't need to call this if you use devkitSMS crt0_sg.rel or crt0_cv.rel */
void SG_init (void);

/* VDP operative mode handling functions */
void SG_VDPturnOnFeature (unsigned int feature);
void SG_VDPturnOffFeature (unsigned int feature);
/* turns on/off a VDP feature */
/* feature can be one of the following: */
#define SG_VDPFEATURE_SHOWDISPLAY      0x0140
#define SG_VDPFEATURE_FRAMEIRQ         0x0120
#define SG_VDPFEATURE_USELARGESPRITES  0x0102     /* don't use this directly, use SG_setSpriteMode */
#define SG_VDPFEATURE_ZOOMSPRITES      0x0101     /* don't use this directly, use SG_setSpriteMode */
/* (it's possible to combine (OR) them if they belong to the same group) */

/* handy macros :) */
#define SG_displayOn()                 SG_VDPturnOnFeature(SG_VDPFEATURE_SHOWDISPLAY)   /* turns on display */
#define SG_displayOff()                SG_VDPturnOffFeature(SG_VDPFEATURE_SHOWDISPLAY)  /* turns off display */

void SG_setSpriteMode (unsigned char mode);
#define SG_SPRITEMODE_NORMAL           0x00
#define SG_SPRITEMODE_LARGE            0x01
#define SG_SPRITEMODE_ZOOMED           0x02
#define SG_SPRITEMODE_LARGE_ZOOMED     0x03

#define SG_setBackdropColor(color)     SG_write_to_VDPRegister(0x07,(color));
#define SG_COLOR_TRANSPARENT           0x00
#define SG_COLOR_BLACK                 0x01
#define SG_COLOR_MEDIUM_GREEN          0x02
#define SG_COLOR_LIGHT_GREEN           0x03
#define SG_COLOR_DARK_BLUE             0x04
#define SG_COLOR_LIGHT_BLUE            0x05
#define SG_COLOR_DARK_RED              0x06
#define SG_COLOR_CYAN                  0x07
#define SG_COLOR_MEDIUM_RED            0x08
#define SG_COLOR_LIGHT_RED             0x09
#define SG_COLOR_DARK_YELLOW           0x0A
#define SG_COLOR_LIGHT_YELLOW          0x0B
#define SG_COLOR_DARK_GREEN            0x0C
#define SG_COLOR_MAGENTA               0x0D
#define SG_COLOR_GRAY                  0x0E
#define SG_COLOR_WHITE                 0x0F

#define BG_COL(col)          (col)
#define FG_COL(col)          ((col)<<4)

/* wait until next VBlank starts */
void SG_waitForVBlank (void);

#ifndef TARGET_CV
/* macro for ROM bankswitching */
volatile __at (0xffff) unsigned char ROM_bank_to_be_mapped_on_slot2;
#define SG_mapROMBank(n)       ROM_bank_to_be_mapped_on_slot2=(n)

/* macro to retrieve the currently mapped ROM bank */
#define SG_getROMBank()        (ROM_bank_to_be_mapped_on_slot2)

/* macros to preserve and restore the currently mapped ROM bank */
/* NOTE: they need to be used within the same scope (they use local variables) */
/* Typical use: In functions using SG_mapROMBank(), to make sure the mapped bank */
/* when entering the function is unchanged upon return. */
/* Use only one SG_saveROMBank() before the first SG_mapROMBank() in the function, */
/* and at least one SG_restoreROMBank() per following return statement. */
/* SG_restoreROMBank() may be used several times, for instance to access data in the original bank. */
#define SG_saveROMBank()       unsigned char _saved_slot2_ROM_bank = ROM_bank_to_be_mapped_on_slot2
#define SG_restoreROMBank()    SG_mapROMBank(_saved_slot2_ROM_bank)

/* additional symbols to control other mapper slots - use with care! */
volatile __at (0xfffe) unsigned char ROM_bank_to_be_mapped_on_slot1;
volatile __at (0xfffd) unsigned char ROM_bank_to_be_mapped_on_slot0;

/* macro for SRAM access */
volatile __at (0xfffc) unsigned char SRAM_bank_to_be_mapped_on_slot2;
#define SG_enableSRAM()        SRAM_bank_to_be_mapped_on_slot2=0x08
#define SG_enableSRAMBank(n)   SRAM_bank_to_be_mapped_on_slot2=((((n)<<2)|0x08)&0x0C)
#define SG_disableSRAM()       SRAM_bank_to_be_mapped_on_slot2=0x00

/* SRAM access is as easy as accessing an array of char */
__at (0x8000) unsigned char SG_SRAM[];
#else
/* ColecoVision MegaCart ROM bankswitching */
volatile __at (0xffff) unsigned char CV_MEGACART_BANKSWITCH[];
#define SG_mapROMBank(n)       CV_MEGACART_BANKSWITCH[-((n)&0x3F)]=(n)
#endif

/* macros to load patterns, their attributes (colours) and sprites into VRAM */
#define SG_loadTilePatterns(src,tilefrom,size)      SG_VRAMmemcpy((PGTADDRESS+((tilefrom)<<3)),(src),(size))
#define SG_loadTileColours(src,tilefrom,size)       SG_VRAMmemcpy((CGTADDRESS+((tilefrom)<<3)),(src),(size))
#define SG_loadSpritePatterns(src,tilefrom,size)    SG_VRAMmemcpy((SGTADDRESS+((tilefrom)<<3)),(src),(size))

#define XYtoADDR(x,y)      (PNTADDRESS|(((unsigned int)(y)<<5)+((unsigned char)(x))))

/* macro to load a tilemap */
#define SG_loadTileMap(x,y,src,size)                SG_VRAMmemcpy((XYtoADDR((x),(y))),(src),(size))

/* functions to load a part of the tilemap */
void SG_loadTileMapArea (unsigned char x, unsigned char y, void *src, unsigned char width, unsigned char height);

/* functions update the tilemap */
void SG_setNextTileatAddr (unsigned int addr) __naked __preserves_regs(a,b,d,e,l,iyh,iyl);
#define SG_setNextTileatXY(x,y)                     SG_setNextTileatAddr(PNTADDRESS+((y)<<5)+(x))
void SG_setTile (unsigned char tile) __naked __preserves_regs(a,b,c,d,e,h,l,iyh,iyl);
#define SG_setTileatAddr(addr,tile)                 do{SG_setNextTileatAddr(addr);SG_setTile(tile);}while(0)
#define SG_setTileatXY(x,y,tile)                    do{SG_setNextTileatXY((x),(y));SG_setTile(tile);}while(0)

void SG_getNextTileatAddr (unsigned int addr) __naked __preserves_regs(a,b,d,e,l,iyh,iyl);
#define SG_getNextTileatXY(x,y)                     SG_getNextTileatAddr(PNTADDRESS+((y)<<5)+(x))
unsigned char SG_getTile (void) __naked __preserves_regs(b,c,d,e,h,l,iyh,iyl);
#define SG_getTileatAddr(addr)                      (SG_getNextTileatAddr(addr),SG_getTile())
#define SG_getTileatXY(x,y)                         (SG_getNextTileatXY((x),(y)),SG_getTile())

/* functions for sprites handling */
void SG_initSprites (void);
#define SG_addSprite(x,y,tile,attr)                 SG_addSprite_f((((unsigned int)(x)&0xff)<<8)|(((y)&0xff)),(((unsigned int)(tile)&0xff)<<8)|(((attr)&0xff)))  /* returns false if no more sprites are available */
_Bool SG_addSprite_f (unsigned int x_y, unsigned int tilenum_attrib) __naked __preserves_regs(d,e,iyh,iyl);
void SG_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1);
_Bool SG_addSpriteClipping (int x, int y, unsigned char tile, unsigned char attr);              /* returns false if no more sprites are available or sprite clipped */
void SG_finalizeSprites (void);   /* now deprecated, you don't need to use it */
void SG_copySpritestoSAT (void) __naked __preserves_regs(d,e,iyh,iyl);

/* functions to read joypad(s) */
unsigned int SG_getKeysStatus (void);
unsigned int SG_getKeysPressed (void);
unsigned int SG_getKeysHeld (void);
unsigned int SG_getKeysReleased (void);

#ifdef TARGET_CV
/* functions to read ColecoVision numpad(s) */
unsigned char SG_readCVNumPad (unsigned char which);
#endif

/* handy defines for joypad(s) handling */
#ifndef CONTROLLER_PORTS
#define CONTROLLER_PORTS

#ifndef TARGET_CV
#define PORT_A_KEY_UP     0x0001
#define PORT_A_KEY_DOWN   0x0002
#define PORT_A_KEY_LEFT   0x0004
#define PORT_A_KEY_RIGHT  0x0008
#define PORT_A_KEY_1      0x0010
#define PORT_A_KEY_2      0x0020

#define PORT_B_KEY_UP     0x0040
#define PORT_B_KEY_DOWN   0x0080
#define PORT_B_KEY_LEFT   0x0100
#define PORT_B_KEY_RIGHT  0x0200
#define PORT_B_KEY_1      0x0400
#define PORT_B_KEY_2      0x0800

#define RESET_KEY         0x1000
#define CARTRIDGE_SLOT    0x2000        /* ??? */
#define PORT_A_TH         0x4000        /* for light gun */
#define PORT_B_TH         0x8000        /* for light gun */
#else
#define PORT_A_KEY_UP     0x0001
#define PORT_A_KEY_RIGHT  0x0002
#define PORT_A_KEY_DOWN   0x0004
#define PORT_A_KEY_LEFT   0x0008
#define PORT_A_KEY_1      0x0040
#define PORT_A_KEY_2      0x0080

#define PORT_A_SAC_YELLOW PORT_A_KEY_1  /* Super Action Controller yellow key */
#define PORT_A_SAC_RED    PORT_A_KEY_2  /* Super Action Controller red key */

#define PORT_B_KEY_UP     0x0100
#define PORT_B_KEY_RIGHT  0x0200
#define PORT_B_KEY_DOWN   0x0400
#define PORT_B_KEY_LEFT   0x0800
#define PORT_B_KEY_1      0x4000
#define PORT_B_KEY_2      0x8000

#define PORT_B_SAC_YELLOW PORT_B_KEY_1  /* Super Action Controller yellow key */
#define PORT_B_SAC_RED    PORT_B_KEY_2  /* Super Action Controller red key */

#define PORT_A_CV_NUMPAD  0
#define PORT_B_CV_NUMPAD  1

#define CV_NUMPAD_0       0x0A
#define CV_NUMPAD_1       0x0D
#define CV_NUMPAD_2       0x07
#define CV_NUMPAD_3       0x0C
#define CV_NUMPAD_4       0x02
#define CV_NUMPAD_5       0x03
#define CV_NUMPAD_6       0x0E
#define CV_NUMPAD_7       0x05
#define CV_NUMPAD_8       0x01
#define CV_NUMPAD_9       0x0B
#define CV_NUMPAD_STAR    0x09
#define CV_NUMPAD_HASH    0x06
#define CV_NUMPAD_NONE    0x0F

#define CV_SAC_PURPLE     0x08    /* Super Action Controller purple key */
#define CV_SAC_BLUE       0x04    /* Super Action Controller blue key */
#endif

#define PORT_A_KEY_START  PORT_A_KEY_1  /* handy alias */
#define PORT_B_KEY_START  PORT_B_KEY_1  /* handy alias */
#endif

#ifndef TARGET_CV
_Bool SG_queryPauseRequested (void);    /* true if the pause key has been pressed */
void SG_resetPauseRequest (void);       /* reset/acknowledge pause requests */
#endif

#ifndef TARGET_CV
_Bool SG_detectKeyboard (void);         /* true if an attached keyboard is detected */
void SG_scanKeyboardJoypad (void);      /* this scans the keyboard keys (emulating a joypad) */

/* functions to read the keyboard virtual joypad(s) */
unsigned int SG_getKeyboardJoypadStatus (void);
unsigned int SG_getKeyboardJoypadPressed (void);
unsigned int SG_getKeyboardJoypadHeld (void);
unsigned int SG_getKeyboardJoypadReleased (void);

/* read from keyboard max keys and return the keycodes and amount */
unsigned char SG_getKeycodes (unsigned int *keys, unsigned char max_keys);
#endif

/* decompress data to RAM */
void SG_decompressZX7 (const void *src, void *dst) __naked;

/* functions to decompress data to VRAM */
#define SG_loadZX7compressedBGTiles(src,tilefrom)      SG_decompressZX7toVRAM((src),PGTADDRESS+((tilefrom)<<3))
#define SG_loadZX7compressedBGColors(src,tilefrom)     SG_decompressZX7toVRAM((src),CGTADDRESS+((tilefrom)<<3))
#define SG_loadZX7compressedSpritesTiles(src,tilefrom) SG_decompressZX7toVRAM((src),SGTADDRESS+((tilefrom)<<3))
#define SG_loadZX7compressedTilesatAddr(src,dst)       SG_decompressZX7toVRAM((src),(dst))
void SG_decompressZX7toVRAM (const void *src, unsigned int dst) __naked;

/* old aPlib decompressor (works ONLY when screen is OFF) */
void UNSAFE_SG_decompressAPLIBtoVRAM (const void *src, unsigned int dst) __naked;

/* function to print messages to the debug console of emulators */
void SG_debugPrintf(const unsigned char *format, ...) __naked __preserves_regs(a,b,c,iyh,iyl);

/* Bitmap Mode functions */
void SG_initBitmapMode (unsigned char foreground_color, unsigned char background_color);
#define  SG_putPixel(x,y,color)          SG_putPixel_f((color),((x)<<8)|(y))
void SG_putPixel_f (unsigned char color, unsigned int xy_coords) __preserves_regs(iyh,iyl);
void SG_setPixel (unsigned char x, unsigned char y) __preserves_regs(iyh,iyl);

/* low level functions / macros */
void SG_VRAMmemcpy (unsigned int dst, const void *src, unsigned int size) __naked __z88dk_callee __preserves_regs(iyh,iyl);
#define SG_VRAMmemset(dst,value,size)    SG_VRAMmemset_f((value),(dst),(size))
//~ void SG_VRAMmemsetW (unsigned int dst, unsigned int value, unsigned int size);     /* removed because it was useless */
//~ void SG_VRAMmemcpy_brief (unsigned int dst, void *src, unsigned char size);        /* removed because it was useless */
void SG_VRAMmemset_f (unsigned char value, unsigned int dst, unsigned int size) __naked __z88dk_callee __preserves_regs(a,h,l,iyh,iyl);

/* vertical interrupt hook */
#ifndef NO_FRAME_INT_HOOK
/* If non-NULL, the specified function will be called by SG_isr after acknowledging */
/* the interrupt and reading controller status. */
void SG_setFrameInterruptHandler (void (*theHandlerFunction)(void)) __z88dk_fastcall;
#endif

/* the Interrupt Service Routines (do not modify) */
#ifndef TARGET_CV
void SG_isr (void) __critical __interrupt(0);
#endif

#ifndef TARGET_CV
void SG_nmi_isr (void) __critical __interrupt;
#else
void SG_nmi_isr (void) __naked;
#endif
