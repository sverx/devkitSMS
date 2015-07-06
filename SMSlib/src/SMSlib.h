/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

// #define TARGET_GG
/* uncomment previous line to compile for the GameGear */

// #define NO_MD_PAD_SUPPORT
/* uncomment previous line to remove support for the Genesis/MegaDrive pad (SMS only) */

#ifdef TARGET_GG
#define NO_MD_PAD_SUPPORT          /* no MD pad support on GG! */
#endif

/* library initialization. you don't need to call this if you use devkitSMS crt0.rel */
void SMS_init (void);

/* VDP operative mode handling functions */
void SMS_VDPturnOnFeature (unsigned int feature);
void SMS_VDPturnOffFeature (unsigned int feature);
/* turns on/off a VDP feature */
/* feature can be one of the following: */

/* group 0 */
#define VDPFEATURE_EXTRAHEIGHT      0x0002
#define VDPFEATURE_SHIFTSPRITES     0x0008
#define VDPFEATURE_HIDEFIRSTCOL     0x0020
#define VDPFEATURE_LOCKHSCROLL      0x0040
#define VDPFEATURE_LOCKVSCROLL      0x0080

/* group 1 */
#define VDPFEATURE_ZOOMSPRITES      0x0101
#define VDPFEATURE_USETALLSPRITES   0x0102
#define VDPFEATURE_240LINES         0x0108
#define VDPFEATURE_224LINES         0x0110
#define VDPFEATURE_FRAMEIRQ         0x0120
#define VDPFEATURE_SHOWDISPLAY      0x0140

/* (it's possible to combine (OR) them if they belong to the same group) */
/* example: VDPFEATURE_ZOOMSPRITES|VDPFEATURE_USETALLSPRITES */

/* handy macros :) */
#define SMS_displayOn()   SMS_VDPturnOnFeature(VDPFEATURE_SHOWDISPLAY)   /* turns on display */
#define SMS_displayOff()  SMS_VDPturnOffFeature(VDPFEATURE_SHOWDISPLAY)  /* turns off display */

void SMS_setBGScrollX (int scrollX);
void SMS_setBGScrollY (int scrollY);
void SMS_setBackdropColor (unsigned char entry);
void SMS_useFirstHalfTilesforSprites (_Bool usefirsthalf);

/* macro for bankswitching */
volatile __at (0xffff) unsigned char bank_to_be_mapped_on_slot2;
#define SMS_mapROMBank(n)       bank_to_be_mapped_on_slot2=n

/* wait until next VBlank starts */
void SMS_waitForVBlank (void);

#ifdef TARGET_GG
/* GG functions to set a color / load a palette */
void GG_setBGPaletteColor (unsigned char entry, unsigned int color);
void GG_setSpritePaletteColor (unsigned char entry, unsigned int color);
void GG_loadBGPalette (void *palette);
void GG_loadSpritePalette (void *palette);
#else
/* functions to set a color / load a palette */
void SMS_setBGPaletteColor (unsigned char entry, unsigned char color);
void SMS_setSpritePaletteColor (unsigned char entry, unsigned char color);
void SMS_loadBGPalette (void *palette);
void SMS_loadSpritePalette (void *palette);
#endif

/* functions to load tiles into VRAM */
void SMS_loadTiles (void *src, unsigned int tilefrom, unsigned int size);
void SMS_loadPSGaidencompressedTiles (void *src, unsigned int tilefrom);

/* functions for the tilemap */
void SMS_loadTileMap (unsigned char x, unsigned char y, void *src, unsigned int size);
void SMS_loadSTMcompressedTileMap (unsigned char x, unsigned char y, unsigned char *src);
void SMS_loadTileMapArea (unsigned char x, unsigned char y, void *src, unsigned char width, unsigned char height);
void SMS_setTileatXY (unsigned char x, unsigned char y, unsigned int tile);
void SMS_setNextTileatXY (unsigned char x, unsigned char y);
void SMS_setTile (unsigned int tile);

/* handy defines for tilemaps entry */
#define TILE_FLIPPED_X            0x0200
#define TILE_FLIPPED_Y            0x0400
#define TILE_USE_SPRITE_PALETTE   0x0800
#define TILE_PRIORITY             0x1000

/* functions for sprites handling */
void SMS_initSprites (void);
_Bool SMS_addSprite (unsigned char x, unsigned char y, unsigned char tile);  /* returns false if no more sprites are available */
void SMS_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1);
_Bool SMS_addSpriteClipping (int x, int y, unsigned char tile); /* returns false if no more sprites are available or sprite clipped */
void SMS_finalizeSprites (void);
void SMS_copySpritestoSAT (void);

/* functions to read joypad(s) */
unsigned int SMS_getKeysStatus (void);
unsigned int SMS_getKeysPressed (void);
unsigned int SMS_getKeysHeld (void);
unsigned int SMS_getKeysReleased (void);

/* handy defines for joypad(s) handling */
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

#define RESET_KEY_NOT           0x1000          /* 0 = pressed */
#define CARTRIDGE_SLOT          0x2000          /* ??? */
#define PORT_A_TH               0x4000          /* for light gun */
#define PORT_B_TH               0x8000          /* for light gun */

#ifdef TARGET_GG
#define GG_KEY_START            0x8000          /* START key on GG */
#endif

#ifndef NO_MD_PAD_SUPPORT
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
/* pause handling (SMS only) */
_Bool SMS_queryPauseRequested (void);
void SMS_resetPauseRequest (void);
#endif

/* line interrupt */
void SMS_setLineInterruptHandler (void (*theHandlerFunction)(void));
void SMS_setLineCounter (unsigned char count);
#define SMS_enableLineInterrupt()   SMS_VDPturnOnFeature(0x0010)   /* turns on line IRQ */
#define SMS_disableLineInterrupt()  SMS_VDPturnOffFeature(0x0010)  /* turns off line IRQ */

/* Vcount */
unsigned char SMS_getVCount (void);
/* Hcount */
unsigned char SMS_getHCount (void);

/* low level functions */
void SMS_VRAMmemcpy (unsigned int dst, void *src, unsigned int size);
void SMS_VRAMmemcpy_brief (unsigned int dst, void *src, unsigned char size);
void SMS_VRAMmemset (unsigned int dst, unsigned char value, unsigned int size);
void SMS_VRAMmemsetW (unsigned int dst, unsigned int value, unsigned int size);

/* VRAM unsafe functions. Fast, but dangerous! */
void UNSAFE_SMS_copySpritestoSAT (void);

/* macros for SEGA and SDSC headers */
#define SMS_BYTE_TO_BCD(n) (((n)/10)*16+((n)%10))

#define SMS_EMBED_SEGA_ROM_HEADER(productCode,revision) \
 const __at (0x7ff0) unsigned char __SMS__SEGA_signature[16]={'T','M','R',' ','S','E','G','A', \
                                                                          0xFF,0xFF,0xFF,0xFF, \
                  SMS_BYTE_TO_BCD((productCode)%100),SMS_BYTE_TO_BCD(((productCode)/100)%100), \
                                            (((productCode)/10000)<<4)|((revision)&0x0f),0x4C}

#define SMS_EMBED_SDSC_HEADER(verMaj,verMin,dateYear,dateMonth,dateDay,author,name,descr) \
                          const __at (0x7fe0-sizeof(author)) char __SMS__SDSC_author[]=author; \
                 const __at (0x7fe0-sizeof(author)-sizeof(name)) char __SMS__SDSC_name[]=name; \
 const __at (0x7fe0-sizeof(author)-sizeof(name)-sizeof(descr)) char __SMS__SDSC_descr[]=descr; \
                          const __at (0x7fe0) char __SMS__SDSC_signature[16]={'S','D','S','C', \
                                              SMS_BYTE_TO_BCD(verMaj),SMS_BYTE_TO_BCD(verMin), \
                                          SMS_BYTE_TO_BCD(dateDay),SMS_BYTE_TO_BCD(dateMonth), \
                              SMS_BYTE_TO_BCD((dateYear)%100),SMS_BYTE_TO_BCD((dateYear)/100), \
                                      (0x7fe0-sizeof(author))&0xff,(0x7fe0-sizeof(author))>>8, \
            (0x7fe0-sizeof(author)-sizeof(name))&0xff,(0x7fe0-sizeof(author)-sizeof(name))>>8, \
                                      (0x7fe0-sizeof(author)-sizeof(name)-sizeof(descr))&0xff, \
                                        (0x7fe0-sizeof(author)-sizeof(name)-sizeof(descr))>>8}
/* pretty nice, isn't it? :) */

/* the Interrupt Service Routines (do not modify) */
void SMS_isr (void) __interrupt;
void SMS_nmi_isr (void) __critical __interrupt;

/* STILL MISSING
void SMS_VDPSetSATLocation (unsigned int location);
void SMS_VDPSetPNTLocation (unsigned int location);
void SMS_VDPSetSpritesLocation (unsigned int location);
*/
