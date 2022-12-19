/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

// #define TARGET_GG
/* to recompile the library for the GameGear */

// #define GG_SECOND_PAD_SUPPORT
/* to recompile with support for the external second pad on GameGear */

// #define MD_PAD_SUPPORT
/* to recompile with support for the Genesis/MegaDrive pad (SMS only) */

/* library initialization. you don't need to call this if you use devkitSMS crt0.rel */
void SMS_init (void);

/* ***************************************************************** */
/* VDP features handling                                             */
/* ***************************************************************** */

void SMS_VDPturnOnFeature (unsigned int feature) __z88dk_fastcall;
void SMS_VDPturnOffFeature (unsigned int feature)__z88dk_fastcall;
/* turns on/off a VDP feature */
/* feature can be one of the following: */

/* group 0 */
#define VDPFEATURE_EXTRAHEIGHT      0x0002     /* needed for 224-240 lines modes */
#define VDPFEATURE_SHIFTSPRITES     0x0008
#define VDPFEATURE_HIDEFIRSTCOL     0x0020
#define VDPFEATURE_LEFTCOLBLANK     0x0020     /* probably a better name */
#define VDPFEATURE_LOCKHSCROLL      0x0040
#define VDPFEATURE_LOCKVSCROLL      0x0080

/* group 1 */
#define VDPFEATURE_ZOOMSPRITES      0x0101     /* (use SMS_setSpriteMode instead) */
#define VDPFEATURE_USETALLSPRITES   0x0102     /* (use SMS_setSpriteMode instead) */
#define VDPFEATURE_240LINES         0x0108     /*  SMS II only! PAL only! */
#define VDPFEATURE_224LINES         0x0110     /*  SMS II only! */
#define VDPFEATURE_FRAMEIRQ         0x0120
#define VDPFEATURE_SHOWDISPLAY      0x0140

/* (it's possible to combine (OR) them if they belong to the same group) */
/* example: VDPFEATURE_LEFTCOLBLANK|VDPFEATURE_LOCKHSCROLL */

/* handy macros :) */
#define SMS_displayOn()   SMS_VDPturnOnFeature(VDPFEATURE_SHOWDISPLAY)   /* turns on display */
#define SMS_displayOff()  SMS_VDPturnOffFeature(VDPFEATURE_SHOWDISPLAY)  /* turns off display */

void SMS_setBGScrollX (unsigned char scrollX) __z88dk_fastcall;
void SMS_setBGScrollY (unsigned char scrollY) __z88dk_fastcall;
void SMS_setBackdropColor (unsigned char entry) __z88dk_fastcall;
void SMS_useFirstHalfTilesforSprites (_Bool usefirsthalf) __z88dk_fastcall;
void SMS_setSpriteMode (unsigned char mode) __z88dk_fastcall;
/* modes for SMS_setSpriteMode */
#define SPRITEMODE_NORMAL         0x00
#define SPRITEMODE_TALL           0x01
#define SPRITEMODE_ZOOMED         0x02
#define SPRITEMODE_TALL_ZOOMED    0x03

/* wait until next VBlank starts */
void SMS_waitForVBlank (void);

/* ***************************************************************** */
/* Cartridge mapper handling                                         */
/* ***************************************************************** */

/* macro for ROM bankswitching */
volatile __at (0xffff) unsigned char ROM_bank_to_be_mapped_on_slot2;
#define SMS_mapROMBank(n)       ROM_bank_to_be_mapped_on_slot2=(n)

/* macros to preserve and restore the currently mapped ROM bank */

/* Typical use: In functions using SMS_mapROMBank(), to make sure the mapped bank */
/* when entering the function is unchanged upon return. */
/* Use only one SMS_saveROMBank() before the first SMS_mapROMBank() in the function, */
/* and at least one SMS_restoreROMBank() per following return statement. */
/* SMS_restoreROMBank() may be used several times, for instance to access data in the original bank. */
#define SMS_saveROMBank()       unsigned char _saved_slot2_ROM_bank = ROM_bank_to_be_mapped_on_slot2
#define SMS_restoreROMBank()    SMS_mapROMBank(_saved_slot2_ROM_bank)

/* additional symbols to control other mapper slots - use with care! */
volatile __at (0xfffe) unsigned char ROM_bank_to_be_mapped_on_slot1;
volatile __at (0xfffd) unsigned char ROM_bank_to_be_mapped_on_slot0;

/* macro for SRAM access */
volatile __at (0xfffc) unsigned char SRAM_bank_to_be_mapped_on_slot2;
#define SMS_enableSRAM()        SRAM_bank_to_be_mapped_on_slot2=0x08
#define SMS_enableSRAMBank(n)   SRAM_bank_to_be_mapped_on_slot2=((((n)<<2)|0x08)&0x0C)
#define SMS_disableSRAM()       SRAM_bank_to_be_mapped_on_slot2=0x00

/* SRAM access is as easy as accessing an array of char */
__at (0x8000) unsigned char SMS_SRAM[];

/* ***************************************************************** */
/* Tiles / Background handling                                       */
/* ***************************************************************** */

void SMS_crt0_RST08(unsigned int addr) __z88dk_fastcall __preserves_regs(a,b,d,e,h,l,iyh,iyl);
void SMS_crt0_RST18(unsigned int tile) __z88dk_fastcall __preserves_regs(b,c,d,e,h,l,iyh,iyl);

/* function for setting tiles/moving 'cursor' */
#define SMS_setTile(tile)         SMS_crt0_RST18(tile)
#define SMS_setAddr(addr)         SMS_crt0_RST08(addr)

/* PNT define (has address and VDP flags) */
#define SMS_PNTAddress            0x7800
/* macro for turning x,y into VRAM addr */
#define XYtoADDR(x,y)             (SMS_PNTAddress|((((unsigned int)(y)<<5)+((unsigned char)(x)))<<1))
#define SMS_setNextTileatXY(x,y)  SMS_setAddr(XYtoADDR((x),(y)))
#define SMS_setNextTileatLoc(loc) SMS_setAddr(SMS_PNTAddress|((unsigned int)(loc)<<1))
#define SMS_setNextTileatAddr(a)  SMS_setAddr(a)
#define SMS_setTileatXY(x,y,tile) do{SMS_setAddr(XYtoADDR((x),(y)));SMS_setTile(tile);}while(0)

#define SMS_VDPVRAMWrite          0x4000
/* macro for turning tile numbers into VRAM addr for writing */
#define TILEtoADDR(tile)          (SMS_VDPVRAMWrite|((tile)*32))

/* handy defines for tilemaps entry */
#define TILE_FLIPPED_X            0x0200
#define TILE_FLIPPED_Y            0x0400
#define TILE_USE_SPRITE_PALETTE   0x0800
#define TILE_PRIORITY             0x1000

/* functions to load tiles into VRAM */
#define SMS_loadTiles(src,tilefrom,size)            SMS_VRAMmemcpy (TILEtoADDR(tilefrom),(src),(size))
void SMS_load1bppTiles (const void *src, unsigned int tilefrom, unsigned int size, unsigned char color0, unsigned char color1);

/* functions to load compressed tiles into VRAM */
#define SMS_loadSTC0compressedTiles(src,tilefrom) SMS_loadSTC0compressedTilesatAddr((src),TILEtoADDR(tilefrom))
void SMS_loadSTC0compressedTilesatAddr (const void *src, unsigned int dst) __naked __sdcccall(1);
#define SMS_loadPSGaidencompressedTiles(src,tilefrom) SMS_loadPSGaidencompressedTilesatAddr((src),TILEtoADDR(tilefrom))
void SMS_loadPSGaidencompressedTilesatAddr (const void *src, unsigned int dst) __naked __sdcccall(1);

/* UNSAFE functions to load compressed tiles into VRAM */
#define UNSAFE_SMS_loadZX7compressedTiles(src,tilefrom) UNSAFE_SMS_loadZX7compressedTilesatAddr((src),TILEtoADDR(tilefrom))
void UNSAFE_SMS_loadZX7compressedTilesatAddr (const void *src, unsigned int dst) __naked __sdcccall(1);
#define UNSAFE_SMS_loadaPLibcompressedTiles(src,tilefrom) UNSAFE_SMS_loadaPLibcompressedTilesatAddr((src),TILEtoADDR(tilefrom))
void UNSAFE_SMS_loadaPLibcompressedTilesatAddr (const void *src, unsigned int dst) __naked __sdcccall(1);

/* functions for the tilemap */
#define SMS_loadTileMap(x,y,src,size)               SMS_VRAMmemcpy (XYtoADDR((x),(y)),(src),(size))
void SMS_loadTileMapArea (unsigned char x, unsigned char y, const void *src, unsigned char width, unsigned char height);

void SMS_loadSTMcompressedTileMapatAddr (unsigned int dst, const void *src);
#define SMS_loadSTMcompressedTileMap(x,y,src)       SMS_loadSTMcompressedTileMapatAddr(XYtoADDR((x),(y)),(src))
#define SMS_loadSTMcompressedTileMapArea(x,y,src,w) SMS_loadSTMcompressedTileMapatAddr(XYtoADDR((x),(y)),(src))
// SMS_loadSTMcompressedTileMapArea *DEPRECATED* - will be dropped at some point in 2018

/* Functions for reading back tilemap and VRAM */
unsigned int SMS_getTile(void) __naked __z88dk_fastcall __preserves_regs(b,c,d,e,iyh,iyl);
void SMS_saveTileMapArea(unsigned char x, unsigned char y, void *dst, unsigned char width, unsigned char height);
void SMS_readVRAM(unsigned char *dst, unsigned int src, unsigned int size) __naked __z88dk_callee __preserves_regs(iyh,iyl) __sdcccall(1);

/* ***************************************************************** */
/* Sprites handling                                                  */
/* ***************************************************************** */

void SMS_initSprites (void);
#define SMS_addSprite(x,y,tile)                   SMS_addSprite_f((y),(((x)&0xff)<<8)|(((tile)&0xff)))
#define SMS_addTwoAdjoiningSprites(x,y,tile)      SMS_addTwoAdjoiningSprites_f((y),(((x)&0xff)<<8)|(((tile)&0xff)))
#define SMS_addThreeAdjoiningSprites(x,y,tile)    SMS_addThreeAdjoiningSprites_f((y),(((x)&0xff)<<8)|(((tile)&0xff)))
#ifdef NO_SPRITE_CHECKS
void SMS_addSprite_f (unsigned char y, unsigned int x_tile) __naked __preserves_regs(d,e,iyh,iyl) __sdcccall(1);               /* doesn't return anything */
void SMS_addTwoAdjoiningSprites_f (unsigned char y, unsigned int x_tile) __naked __preserves_regs(d,e,iyh,iyl) __sdcccall(1);  /* doesn't return anything */
#else
signed char SMS_addSprite_f (unsigned int y, unsigned int x_tile) __naked __preserves_regs(d,e,iyh,iyl) __sdcccall(1);         /* returns -1 if no more sprites are available, -2 if invalid Y coord */
void SMS_addTwoAdjoiningSprites_f (unsigned int y, unsigned int x_tile) __naked __preserves_regs(d,e,iyh,iyl) __sdcccall(1);   /* doesn't return anything */
#endif
void SMS_addThreeAdjoiningSprites_f (unsigned int y, unsigned int x_tile) __naked __preserves_regs(d,e,iyh,iyl) __sdcccall(1); /* doesn't return anything */
signed char SMS_reserveSprite (void);
void SMS_updateSpritePosition (signed char sprite, unsigned char x, unsigned char y);
void SMS_updateSpriteImage (signed char sprite, unsigned char tile);
void SMS_hideSprite (signed char sprite);
void SMS_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1);
signed char SMS_addSpriteClipping (int x, int y, unsigned char tile);   /* returns -1 if no more sprites are available or sprite clipped */
void SMS_finalizeSprites (void);     // *DEPRECATED* - will be dropped at some point in 2018
void SMS_copySpritestoSAT (void);

/* ***************************************************************** */
/* Colors / palettes handling                                        */
/* ***************************************************************** */

/* SMS_CRAMAddress define (has address and VDP flags) */
#define SMS_CRAMAddress                    0xC000

#ifdef TARGET_GG
/* GG functions to set a color / load a palette */
void GG_setBGPaletteColor (unsigned char entry, unsigned int color);
void GG_setSpritePaletteColor (unsigned char entry, unsigned int color);
void GG_loadBGPalette (const void *palette) __z88dk_fastcall;
void GG_loadSpritePalette (const void *palette) __z88dk_fastcall;
#define GG_setNextBGColoratIndex(i)       SMS_setAddr(SMS_CRAMAddress|((i)<<1))
#define GG_setNextSpriteColoratIndex(i)   SMS_setAddr(SMS_CRAMAddress|0x20|((i)<<1))
#define GG_setColor(color)       SMS_crt0_RST18(color)
/* GG macros for colors */
#define RGB(r,g,b)        ((r)|((g)<<4)|((b)<<8))
#define RGB8(r,g,b)       (((r)>>4)|(((g)>>4)<<4)|(((b)>>4)<<8))
#define RGBHTML(RGB24bit) (((RGB24bit)>>20)|((((RGB24bit)&0xFFFF)>>12)<<4)|((((RGB24bit)&0xFF)>>4)<<8))
#else
/* SMS functions to set a color / load a palette */
void SMS_setBGPaletteColor (unsigned char entry, unsigned char color);
void SMS_setSpritePaletteColor (unsigned char entry, unsigned char color);
void SMS_loadBGPalette (const void *palette) __z88dk_fastcall;
void SMS_loadSpritePalette (const void *palette) __z88dk_fastcall;
#define SMS_setNextBGColoratIndex(i)       SMS_setAddr(SMS_CRAMAddress|(i))
#define SMS_setNextSpriteColoratIndex(i)   SMS_setAddr(SMS_CRAMAddress|0x10|(i))
void SMS_setColor (unsigned char color) __z88dk_fastcall __preserves_regs(b,c,d,e,h,l,iyh,iyl);
/* SMS macros for colors */
#define RGB(r,g,b)        ((r)|((g)<<2)|((b)<<4))
#define RGB8(r,g,b)       (((r)>>6)|(((g)>>6)<<2)|(((b)>>6)<<4))
#define RGBHTML(RGB24bit) (((RGB24bit)>>22)|((((RGB24bit)&0xFFFF)>>14)<<2)|((((RGB24bit)&0xFF)>>6)<<4))
void SMS_loadBGPaletteHalfBrightness (const void *palette) __z88dk_fastcall;
void SMS_loadSpritePaletteHalfBrightness (const void *palette) __z88dk_fastcall;
void SMS_zeroBGPalette (void);
void SMS_zeroSpritePalette (void);
#endif

/* text renderer */
void SMS_configureTextRenderer (signed int ascii_to_tile_offset) __z88dk_fastcall;
void SMS_autoSetUpTextRenderer (void);
void SMS_putchar (unsigned char c);         /* faster than plain putchar() */
void SMS_print (const unsigned char *str);  /* faster than printf() for unformatted strings */
/* Macro to print a string at a given location */
#define SMS_printatXY(x,y,s) do { SMS_setNextTileatXY(x,y); SMS_print(s); } while(0)


/* decompress ZX7-compressed data to RAM */
void SMS_decompressZX7 (const void *src, void *dst) __naked __sdcccall(1);

/* ***************************************************************** */
/* Input handling (joypads)                                          */
/* ***************************************************************** */

/* functions to read joypad(s) */
unsigned int SMS_getKeysStatus (void);
unsigned int SMS_getKeysPressed (void);
unsigned int SMS_getKeysHeld (void);
unsigned int SMS_getKeysReleased (void);

/* handy defines for joypad(s) handling */
#ifndef CONTROLLER_PORTS
#define CONTROLLER_PORTS
#define PORT_A_KEY_UP           0x0001
#define PORT_A_KEY_DOWN         0x0002
#define PORT_A_KEY_LEFT         0x0004
#define PORT_A_KEY_RIGHT        0x0008
#define PORT_A_KEY_1            0x0010
#define PORT_A_KEY_2            0x0020
#define PORT_A_KEY_START        PORT_A_KEY_1    /* handy alias */

#define PORT_B_KEY_UP           0x0040
#define PORT_B_KEY_DOWN         0x0080
#define PORT_B_KEY_LEFT         0x0100
#define PORT_B_KEY_RIGHT        0x0200
#define PORT_B_KEY_1            0x0400
#define PORT_B_KEY_2            0x0800
#define PORT_B_KEY_START        PORT_B_KEY_1    /* handy alias */

#define RESET_KEY               0x1000          /* (absent on SMS II) */
#define CARTRIDGE_SLOT          0x2000          /* ??? */
#define PORT_A_TH               0x4000          /* for light gun */
#define PORT_B_TH               0x8000          /* for light gun */
#endif

#ifdef TARGET_GG
#define GG_KEY_START            0x8000          /* START key on GG */
#endif

#ifdef MD_PAD_SUPPORT
/* functions to read additional MD buttons */
unsigned int SMS_getMDKeysStatus (void);
unsigned int SMS_getMDKeysPressed (void);
unsigned int SMS_getMDKeysHeld (void);
unsigned int SMS_getMDKeysReleased (void);

/* handy defines for additional MD joypad(s) handling */
#define PORT_A_MD_KEY_Z         0x0001
#define PORT_A_MD_KEY_Y         0x0002
#define PORT_A_MD_KEY_X         0x0004
#define PORT_A_MD_KEY_MODE      0x0008
#define PORT_A_MD_KEY_A         0x0010
#define PORT_A_MD_KEY_START     0x0020
/* port B still missing */
#endif

#ifndef TARGET_GG
/* paddle controller handling (SMS only) */
#define PORT_A      0
#define PORT_B      1
_Bool SMS_detectPaddle (unsigned char port) __z88dk_fastcall __naked;
unsigned char SMS_readPaddle (unsigned char port) __z88dk_fastcall __naked;
#endif

#ifndef TARGET_GG
/* pause handling (SMS only) */
_Bool SMS_queryPauseRequested (void);
void SMS_resetPauseRequest (void);
#endif

#ifndef TARGET_GG
#ifdef  VDPTYPE_DETECTION
/* VDPType handling (SMS only) */
unsigned char SMS_VDPType (void);
/* WARNING: these constants may change value later, please use defines */
#define VDP_PAL                 0x80
#define VDP_NTSC                0x40
#endif
#endif

extern volatile unsigned char SMS_VDPFlags;
#define VDPFLAG_SPRITEOVERFLOW  0x40
#define VDPFLAG_SPRITECOLLISION 0x20

extern unsigned char SMS_Port3EBIOSvalue;

/* vertical interrupt hook */
#ifndef NO_FRAME_INT_HOOK
/* If non-NULL, the specified function will be called by SMS_isr after acknowledging */
/* the interrupt and reading controller status. */
void SMS_setFrameInterruptHandler (void (*theHandlerFunction)(void)) __z88dk_fastcall;
#endif

/* line interrupt */
void SMS_setLineInterruptHandler (void (*theHandlerFunction)(void)) __z88dk_fastcall;
void SMS_setLineCounter (unsigned char count) __z88dk_fastcall;
#define SMS_enableLineInterrupt()   SMS_VDPturnOnFeature(0x0010)   /* turns on line IRQ */
#define SMS_disableLineInterrupt()  SMS_VDPturnOffFeature(0x0010)  /* turns off line IRQ */

/* Vcount */
unsigned char SMS_getVCount (void);
/* Hcount */
unsigned char SMS_getHCount (void);

/* low level functions */
void SMS_VRAMmemcpy (unsigned int dst, const void *src, unsigned int size) __naked __z88dk_callee __preserves_regs(iyh,iyl) __sdcccall(1);
void SMS_VRAMmemcpy_brief (unsigned int dst, const void *src, unsigned char size) __naked __z88dk_callee __preserves_regs(iyh,iyl) __sdcccall(1);
#define SMS_VRAMmemset(dst,value,size)   SMS_VRAMmemset_f((value),(dst),(size))
void SMS_VRAMmemset_f (unsigned char value, unsigned int dst, unsigned int size) __naked __z88dk_callee __preserves_regs(a,h,l,iyh,iyl) __sdcccall(1);
void SMS_VRAMmemsetW (unsigned int dst, unsigned int value, unsigned int size) __naked __z88dk_callee __preserves_regs(d,e,iyh,iyl) __sdcccall(1);

/* VRAM unsafe functions. Fast, but dangerous! */
void UNSAFE_SMS_copySpritestoSAT (void);
void UNSAFE_SMS_VRAMmemcpy32 (unsigned int dst, const void *src);
void UNSAFE_SMS_VRAMmemcpy64 (unsigned int dst, const void *src);
void UNSAFE_SMS_VRAMmemcpy128 (unsigned int dst, const void *src);

/* handy macros for UNSAFE_SMS_VRAMmemcpy* */
#define UNSAFE_SMS_load1Tile(src,theTile)     UNSAFE_SMS_VRAMmemcpy32((theTile)*32,(src))
#define UNSAFE_SMS_load2Tiles(src,tilefrom)   UNSAFE_SMS_VRAMmemcpy64((tilefrom)*32,(src))
#define UNSAFE_SMS_load4Tiles(src,tilefrom)   UNSAFE_SMS_VRAMmemcpy128((tilefrom)*32,(src))

/* macros for SEGA and SDSC headers */
#define SMS_BYTE_TO_BCD(n) (((n)/10)*16+((n)%10))

#ifndef TARGET_GG
/* "SMS Export" (32KB) */
#define SMS_EMBED_SEGA_ROM_HEADER_REGION_CODE  0x4C
#else
/* "GG international" (32KB) */
#define SMS_EMBED_SEGA_ROM_HEADER_REGION_CODE  0x7C
#endif

#define SMS_EMBED_SEGA_ROM_HEADER(productCode,revision)                                        \
 const __at (0x7ff0) unsigned char __SMS__SEGA_signature[16]={'T','M','R',' ','S','E','G','A', \
                                                                          0xFF,0xFF,0xFF,0xFF, \
                  SMS_BYTE_TO_BCD((productCode)%100),SMS_BYTE_TO_BCD(((productCode)/100)%100), \
           (((productCode)/10000)<<4)|((revision)&0x0f),SMS_EMBED_SEGA_ROM_HEADER_REGION_CODE}

#define SMS_EMBED_SDSC_HEADER(verMaj,verMin,dateYear,dateMonth,dateDay,author,name,descr)      \
                          const __at (0x7fe0-sizeof(author)) char __SMS__SDSC_author[]=author; \
                 const __at (0x7fe0-sizeof(author)-sizeof(name)) char __SMS__SDSC_name[]=name; \
 const __at (0x7fe0-sizeof(author)-sizeof(name)-sizeof(descr)) char __SMS__SDSC_descr[]=descr; \
                          const __at (0x7fe0) char __SMS__SDSC_signature[16]={'S','D','S','C', \
                                              SMS_BYTE_TO_BCD(verMaj),SMS_BYTE_TO_BCD(verMin), \
                                          SMS_BYTE_TO_BCD(dateDay),SMS_BYTE_TO_BCD(dateMonth), \
                              SMS_BYTE_TO_BCD((dateYear)%100),SMS_BYTE_TO_BCD((dateYear)/100), \
                                                                  (0x7fe0-sizeof(author))%256, \
                                                                   (0x7fe0-sizeof(author))>>8, \
                                                     (0x7fe0-sizeof(author)-sizeof(name))%256, \
                                                      (0x7fe0-sizeof(author)-sizeof(name))>>8, \
                                       (0x7fe0-sizeof(author)-sizeof(name)-sizeof(descr))%256, \
                                        (0x7fe0-sizeof(author)-sizeof(name)-sizeof(descr))>>8}
/* pretty nice, isn't it? :) */

/* SEGA header for 16KB ROM */
#ifndef TARGET_GG
/* "SMS Export" (16KB) */
#define SMS_EMBED_SEGA_ROM_HEADER_16KB_REGION_CODE  0x4B
#else
/* "GG international" (16KB) */
#define SMS_EMBED_SEGA_ROM_HEADER_16KB_REGION_CODE  0x7B
#endif

#define SMS_EMBED_SEGA_ROM_HEADER_16KB(productCode,revision)                                   \
 const __at (0x3ff0) unsigned char __SMS__SEGA_signature[16]={'T','M','R',' ','S','E','G','A', \
                                                                          0xFF,0xFF,0xFF,0xFF, \
                  SMS_BYTE_TO_BCD((productCode)%100),SMS_BYTE_TO_BCD(((productCode)/100)%100), \
      (((productCode)/10000)<<4)|((revision)&0x0f),SMS_EMBED_SEGA_ROM_HEADER_16KB_REGION_CODE}

#define SMS_EMBED_SDSC_HEADER_16KB(verMaj,verMin,dateYear,dateMonth,dateDay,author,name,descr) \
                          const __at (0x3fe0-sizeof(author)) char __SMS__SDSC_author[]=author; \
                 const __at (0x3fe0-sizeof(author)-sizeof(name)) char __SMS__SDSC_name[]=name; \
 const __at (0x3fe0-sizeof(author)-sizeof(name)-sizeof(descr)) char __SMS__SDSC_descr[]=descr; \
                          const __at (0x3fe0) char __SMS__SDSC_signature[16]={'S','D','S','C', \
                                              SMS_BYTE_TO_BCD(verMaj),SMS_BYTE_TO_BCD(verMin), \
                                          SMS_BYTE_TO_BCD(dateDay),SMS_BYTE_TO_BCD(dateMonth), \
                              SMS_BYTE_TO_BCD((dateYear)%100),SMS_BYTE_TO_BCD((dateYear)/100), \
                                                                  (0x3fe0-sizeof(author))%256, \
                                                                   (0x3fe0-sizeof(author))>>8, \
                                                     (0x3fe0-sizeof(author)-sizeof(name))%256, \
                                                      (0x3fe0-sizeof(author)-sizeof(name))>>8, \
                                       (0x3fe0-sizeof(author)-sizeof(name)-sizeof(descr))%256, \
                                        (0x3fe0-sizeof(author)-sizeof(name)-sizeof(descr))>>8}

/* to set SDSC header date to 0000-00-00 so that ihx2sms updates that with compilation date */
#define SMS_EMBED_SDSC_HEADER_AUTO_DATE(verMaj,verMin,author,name,descr)                       \
                        SMS_EMBED_SDSC_HEADER((verMaj),(verMin),0,0,0,(author),(name),(descr))

#define SMS_EMBED_SDSC_HEADER_AUTO_DATE_16KB(verMaj,verMin,author,name,descr)                  \
                  SMS_EMBED_SDSC_HEADER_16KB((verMaj),(verMin),0,0,0,(author),(name),(descr))

/* the Interrupt Service Routines (do not modify) */
void SMS_isr (void) __naked;
void SMS_nmi_isr (void) __naked;

/* EOF */
