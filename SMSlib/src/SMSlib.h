/* **************************************************
   SMSlib - C programming library for the SMS
   (part of devkitSMS - github.com/sverx/devkitSMS)
   ************************************************** */

/* library initialization. you don't need to call this if you use devkitSMS */
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

/* group 1 */
#define VDPFEATURE_ZOOMSPRITES      0x0101
#define VDPFEATURE_USETALLSPRITES   0x0102
#define VDPFEATURE_240LINES         0x0108
#define VDPFEATURE_224LINES         0x0110

/* (it's possible to combine them if they belong to the same group) */
/* example: VDPFEATURE_ZOOMSPRITES|VDPFEATURE_USETALLSPRITES */

/* handy macros :) */
#define SMS_displayOn()   SMS_VDPturnOnFeature(0x0160)   /* turns on display and frame int */
#define SMS_displayOff()  SMS_VDPturnOffFeature(0x0160)  /* turns off display and frame int */

void SMS_setBGScrollX (int scrollX);
void SMS_setBGScrollY (int scrollY);
void SMS_setBackdropColor (unsigned char entry);
void SMS_useFirstHalfTilesforSprites (bool usefirsthalf);

/* macro for bankswitching */
volatile __at (0xffff) unsigned char bank_to_be_mapped_on_slot2;
#define SMS_mapROMBank(n)       bank_to_be_mapped_on_slot2=n

/* wait until next VBlank starts */
void SMS_waitForVBlank (void);

/* functions to set a color / load a palette */
void SMS_setBGPaletteColor (unsigned char entry, unsigned char color);
void SMS_setSpritePaletteColor (unsigned char entry, unsigned char color);
void SMS_loadBGPalette (const unsigned char *palette);
void SMS_loadSpritePalette (const unsigned char *palette);

/* functions to load tiles into VRAM */
void SMS_loadTiles (unsigned char *src, unsigned int Tilefrom, unsigned int len);

/* functions for the tilemap */
void SMS_loadTileMap (unsigned char x, unsigned char y, unsigned int *src, unsigned int len);
void SMS_loadTileMapArea (unsigned char x, unsigned char y,  unsigned int *src, unsigned char width, unsigned char height);
void SMS_setTileatXY (unsigned char x, unsigned char y, unsigned int tile);

/* handy defines for tilemaps entry */
#define TILE_FLIPPED_X            0x0200
#define TILE_FLIPPED_Y            0x0400
#define TILE_USE_SPRITE_PALETTE   0x0800
#define TILE_PRIORITY             0x1000

/* functions for sprites handling */
void SMS_initSprites (void);
bool SMS_addSprite (unsigned char x, int y, unsigned char tile); /* returns false if no more sprites are available */
/* STILL MISSING */
/* void SMS_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1); */
/* bool SMS_addSpriteClipping (int x, int y, unsigned char tile); */
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

/* pause handling */
bool SMS_queryPauseRequested (void);
void SMS_resetPauseRequest (void);

/* the Interrupt Service Routines (do not modify) */
void SMS_isr (void) __interrupt;
void SMS_nmi_isr (void) __critical __interrupt;

/* STILL MISSING
void SMS_VDPSetSATLocation (unsigned int location);
void SMS_VDPSetPNTLocation (unsigned int location);
void SMS_VDPSetSpritesLocation (unsigned int location);
*/
