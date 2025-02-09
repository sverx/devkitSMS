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
#define SG_SPRITEMODE_NORMAL           0x00
#define SG_SPRITEMODE_LARGE            0x01
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

/* functions to load tiles into VRAM */
void SG_loadTilePatterns (void *src, unsigned int tilefrom, unsigned int size);
void SG_loadTileColours (void *src, unsigned int tilefrom, unsigned int size);
void SG_loadSpritePatterns (void *src, unsigned int tilefrom, unsigned int size);

/* functions for the tilemap */
void SG_loadTileMap (unsigned char x, unsigned char y, void *src, unsigned int size);
void SG_loadTileMapArea (unsigned char x, unsigned char y, void *src, unsigned char width, unsigned char height);
void SG_setNextTileatXY (unsigned char x, unsigned char y);
void SG_setTile (unsigned char tile);
void SG_getNextTileatXY (unsigned char x, unsigned char y);
unsigned char SG_getTile (void);
#define SG_setTileatXY(x,y,tile) do{SG_setNextTileatXY((x),(y));SG_setTile(tile);}while(0)
#define SG_getTileatXY(x,y)      (SG_getNextTileatXY((x),(y)),SG_getTile())

/* functions for sprites handling */
void SG_initSprites (void);
_Bool SG_addSprite (unsigned char x, unsigned char y, unsigned char tile, unsigned char attr);  /* returns false if no more sprites are available */
void SG_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1);
_Bool SG_addSpriteClipping (int x, int y, unsigned char tile, unsigned char attr);              /* returns false if no more sprites are available or sprite clipped */
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
#define PORT_A_KEY_UP     0x0001
#define PORT_A_KEY_DOWN   0x0002
#define PORT_A_KEY_LEFT   0x0004
#define PORT_A_KEY_RIGHT  0x0008
#define PORT_A_KEY_1      0x0010
#define PORT_A_KEY_2      0x0020
#define PORT_A_KEY_START  PORT_A_KEY_1  /* handy alias */

#define PORT_B_KEY_UP     0x0040
#define PORT_B_KEY_DOWN   0x0080
#define PORT_B_KEY_LEFT   0x0100
#define PORT_B_KEY_RIGHT  0x0200
#define PORT_B_KEY_1      0x0400
#define PORT_B_KEY_2      0x0800
#define PORT_B_KEY_START  PORT_B_KEY_1  /* handy alias */

#define RESET_KEY         0x1000
#define CARTRIDGE_SLOT    0x2000        /* ??? */
#define PORT_A_TH         0x4000        /* for light gun */
#define PORT_B_TH         0x8000        /* for light gun */
#endif

_Bool SG_queryPauseRequested (void);    /* true if the pause key has been pressed */
void SG_resetPauseRequest (void);       /* reset/acknowledge pause requests */

_Bool SG_detectKeyboard (void);         /* true if an attached keyboard is detected */
void SG_scanKeyboardJoypad (void);      /* this scans the keyboard keys (emulating a joypad) */

/* functions to read the keyboard virtual joypad(s) */
unsigned int SG_getKeyboardJoypadStatus (void);
unsigned int SG_getKeyboardJoypadPressed (void);
unsigned int SG_getKeyboardJoypadHeld (void);
unsigned int SG_getKeyboardJoypadReleased (void);

/* read from keyboard max keys and return the keycodes and amount */
unsigned char SG_getKeycodes (unsigned int *keys, unsigned char max_keys);

/* functions to decompress data to VRAM */
#define SG_loadZX7compressedBGTiles(src,tilefrom)      SG_decompressZX7toVRAM((src),0x4000+((tilefrom)<<3))
#define SG_loadZX7compressedBGColors(src,tilefrom)     SG_decompressZX7toVRAM((src),0x6000+((tilefrom)<<3))
#define SG_loadZX7compressedSpritesTiles(src,tilefrom) SG_decompressZX7toVRAM((src),0x7800+((tilefrom)<<3))
#define SG_loadZX7compressedTilesatAddr(src,dst)       SG_decompressZX7toVRAM((src),(dst))
void SG_decompressZX7toVRAM (const void *src, unsigned int dst) __naked;

/* low level functions */
void SG_VRAMmemcpy (unsigned int dst, void *src, unsigned int size);
void SG_VRAMmemcpy_brief (unsigned int dst, void *src, unsigned char size);
void SG_VRAMmemset (unsigned int dst, unsigned char value, unsigned int size);
void SG_VRAMmemsetW (unsigned int dst, unsigned int value, unsigned int size);

/* vertical interrupt hook */
#ifndef NO_FRAME_INT_HOOK
/* If non-NULL, the specified function will be called by SG_isr after acknowledging */
/* the interrupt and reading controller status. */
void SG_setFrameInterruptHandler (void (*theHandlerFunction)(void)) __z88dk_fastcall;
#endif

/* the Interrupt Service Routines (do not modify) */
void SG_isr (void) __critical __interrupt(0);
void SG_nmi_isr (void) __critical __interrupt;
