SGlib
=====

currently defined functions/macros:

```
/* basic VDP handling */
void SG_init (void)                                /* you don't even need to call this if you're using devkitSMS crt0_sg */
void SG_VDPturnOnFeature (unsigned int feature)    /* check feature list in SGlib.h */
void SG_VDPturnOffFeature (unsigned int feature)
SG_displayOn()                                     /* macro - turns on screen */
SG_displayOff()                                    /* macro - turns off screen */
void SG_setSpriteMode (unsigned char mode)         /* check sprite modes in SGlib.h */
void SG_setBackdropColor (unsigned char entry)
void SG_waitForVBlank (void)                       /* wait until next vBlank starts */

/* bankswitching */
SG_mapROMBank(n)                                   /* macro - maps ROM bank n */
SG_getROMBank()                                    /* macro - retrieve currently mapped ROM bank */
SG_saveROMBank()                                   /* macro - save the currently mapped ROM bank in a local variable */
SG_restoreROMBank()                                /* macro - restore the previously saved ROM bank */
SG_enableSRAM()                                    /* macro - enable cartridge SRAM */
SG_enableSRAMBank(n)                               /* macro - enable cartridge SRAM bank n */ 
SG_disableSRAM()                                   /* macro - disable cartridge SRAM and go back to ROM mapping */

/* functions to load tiles patterns/colours into VRAM */
void SG_loadTilePatterns (void *src, unsigned int tilefrom, unsigned int size)
void SG_loadTileColours (void *src, unsigned int tilefrom, unsigned int size)
void SG_loadSpritePatterns (void *src, unsigned int tilefrom, unsigned int size)

/* functions for tilemap loading/handling */
void SG_loadTileMap (unsigned char x, unsigned char y, void *src, unsigned int size)
void SG_loadTileMapArea (unsigned char x, unsigned char y, void *src, unsigned char width, unsigned char height)
void SG_setNextTileatXY (unsigned char x, unsigned char y)
void SG_setTile (unsigned char tile)
SG_setTileatXY(x,y,tile)
void SG_getNextTileatXY (unsigned char x, unsigned char y)
unsigned char SG_getTile (void)
SG_getTileatXY(x,y)

/* sprite handling */
void SG_initSprites (void)
_Bool SG_addSprite (unsigned char x, unsigned char y, unsigned char tile, unsigned char attr)	/* returns false if no more sprites are available */
void SG_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1)
_Bool SG_addSpriteClipping (int x, int y, unsigned char tile, unsigned char attr)		        /* returns false if no more sprites are available or sprite clipped */
void SG_finalizeSprites (void)
void SG_copySpritestoSAT (void)

/* joypad handling */
unsigned int SG_getKeysStatus (void)     /* the current status of the keys */
unsigned int SG_getKeysPressed (void)    /* the keys that were up last frame and down now */
unsigned int SG_getKeysHeld (void)       /* the keys that were down last frame and still down now */
unsigned int SG_getKeysReleased (void)   /* the keys that were down last frame and up now */

/* functions to read the keyboard virtual joypad(s) */
unsigned int SG_getKeyboardJoypadStatus (void)
unsigned int SG_getKeyboardJoypadPressed (void)
unsigned int SG_getKeyboardJoypadHeld (void)
unsigned int SG_getKeyboardJoypadReleased (void)

/* read from keyboard max keys and return the keycodes and amount */
unsigned char SG_getKeycodes (unsigned int *keys, unsigned char max_keys)

/* pause handling */
_Bool SG_queryPauseRequested (void)      /* the pause key has been pressed since previous check */
void SG_resetPauseRequest (void)         /* reset/acknowledge pause requests */

/* decompress data to RAM */
void SG_decompressZX7 (const void *src, void *dst)                    /* function that does the decompression to RAM */

/* functions and macros to decompress data to VRAM */
void SG_decompressZX7toVRAM (const void *src, unsigned int dst)       /* function that does the decompression directly to VRAM */
SG_loadZX7compressedBGTiles(src,tilefrom)                             /* handy macro to decompress tiles to background tiles */
SG_loadZX7compressedBGColors(src,tilefrom)                            /* handy macro to decompress tiles to background attributes */
SG_loadZX7compressedSpritesTiles(src,tilefrom)                        /* handy macro to decompress tiles to sprites */
SG_loadZX7compressedTilesatAddr(src,dst)                              /* handy macro to decompress tiles to any location in VRAM */

/* Bitmap Mode functions and macros */
void SG_initBitmapMode (unsigned char foreground_color, unsigned char background_color)   /* initialize BMP mode with specified foreground and background colors */
SG_putPixel(x,y,color)                                                                    /* macro to put a pixel on the bitmap with the specified color */

/* low level functions */
void SG_VRAMmemcpy (unsigned int dst, void *src, unsigned int size)              /* memcpy to VRAM */
void SG_VRAMmemcpy_brief (unsigned int dst, void *src, unsigned char size)       /* memcpy to VRAM (256 bytes max) */
void SG_VRAMmemset (unsigned int dst, unsigned char value, unsigned int size)    /* memset to VRAM */

/* print messages to the debug console of emulators */
void SG_debugPrintf(const unsigned char *format, ...) __naked __preserves_regs(a,b,c,iyh,iyl);
```

