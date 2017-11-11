/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   code: na_th_an, sverx
   ************************************************** */

/* library initialization. you don't need to call this if you use devkitSMS crt0_sg.rel */
void SG_init (void);

/* VDP operative mode handling functions */
void SG_VDPturnOnFeature (unsigned int feature);
void SG_VDPturnOffFeature (unsigned int feature);
/* turns on/off a VDP feature */
/* feature can be one of the following: */
#define SG_VDPFEATURE_SHOWDISPLAY      0x0140
#define SG_VDPFEATURE_FRAMEIRQ         0x0120
#define SG_VDPFEATURE_USELARGESPRITES  0x0102
#define SG_VDPFEATURE_ZOOMSPRITES      0x0101
/* (it's possible to combine (OR) them if they belong to the same group) */

/* handy macros :) */
#define SG_displayOn()   SG_VDPturnOnFeature(SG_VDPFEATURE_SHOWDISPLAY)   /* turns on display */
#define SG_displayOff()  SG_VDPturnOffFeature(SG_VDPFEATURE_SHOWDISPLAY)  /* turns off display */

void SG_setSpriteMode (unsigned char mode);
#define SG_SPRITEMODE_NORMAL	       0x00
#define SG_SPRITEMODE_LARGE 	       0x01
#define SG_SPRITEMODE_ZOOMED           0x02
#define SG_SPRITEMODE_LARGE_ZOOMED     0x03

void SG_setBackdropColor (unsigned char entry);
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

/* wait until next VBlank starts */
void SG_waitForVBlank (void);

/* functions to load tiles into VRAM */
void SG_loadTilePatterns (void *src, unsigned int tilefrom, unsigned int size);
void SG_loadTileColours (void *src, unsigned int tilefrom, unsigned int size);
void SG_loadSpritePatterns (void *src, unsigned int tilefrom, unsigned int size);

/* functions for the tilemap */
void SG_loadTileMap (unsigned char x, unsigned char y, void *src, unsigned int size);
void SG_loadTileMapArea (unsigned char x, unsigned char y, void *src, unsigned char width, unsigned char height);
void SG_setNextTileatXY (unsigned char x, unsigned char y);
void SG_setTileatXY (unsigned char x, unsigned char y, unsigned char tile);
void SG_setTile (unsigned char tile);

/* functions for sprites handling */
void SG_initSprites (void);
_Bool SG_addSprite (unsigned char x, unsigned char y, unsigned char tile, unsigned char attr);	/* returns false if no more sprites are available */
void SG_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1);
_Bool SG_addSpriteClipping (int x, int y, unsigned char tile, unsigned char attr);				/* returns false if no more sprites are available or sprite clipped */
void SG_finalizeSprites (void);
void SG_copySpritestoSAT (void);

/* functions to read joypad(s) */
unsigned int SG_getKeysStatus (void);
unsigned int SG_getKeysPressed (void);
unsigned int SG_getKeysHeld (void);
unsigned int SG_getKeysReleased (void);

/* handy defines for joypad(s) handling */
#ifndef CONTROLLER_PORTS
#define CONTROLLER_PORTS
#define PORT_A_KEY_UP		0x0001
#define PORT_A_KEY_DOWN 	0x0002
#define PORT_A_KEY_LEFT 	0x0004
#define PORT_A_KEY_RIGHT	0x0008
#define PORT_A_KEY_1		0x0010
#define PORT_A_KEY_2		0x0020
#define PORT_A_KEY_START	PORT_A_KEY_1  /* handy alias */

#define PORT_B_KEY_UP		0x0040
#define PORT_B_KEY_DOWN 	0x0080
#define PORT_B_KEY_LEFT 	0x0100
#define PORT_B_KEY_RIGHT	0x0200
#define PORT_B_KEY_1		0x0400
#define PORT_B_KEY_2		0x0800
#define PORT_B_KEY_START	PORT_B_KEY_1  /* handy alias */

#define RESET_KEY_NOT		0x1000        /* 0 = pressed */
#define CARTRIDGE_SLOT		0x2000        /* ??? */
#define PORT_A_TH		0x4000        /* for light gun */
#define PORT_B_TH		0x8000        /* for light gun */
#endif

_Bool SG_queryPauseRequested (void);          /* true if the pause key has been pressed */
void SG_resetPauseRequest (void);             /* reset/acknowledge pause requests */

/* low level functions */
void SG_VRAMmemcpy (unsigned int dst, void *src, unsigned int size);
void SG_VRAMmemcpy_brief (unsigned int dst, void *src, unsigned char size);
void SG_VRAMmemset (unsigned int dst, unsigned char value, unsigned int size);
void SG_VRAMmemsetW (unsigned int dst, unsigned int value, unsigned int size);

/* the Interrupt Service Routines (do not modify) */
void SG_isr (void) __interrupt;
void SG_nmi_isr (void) __critical __interrupt;
