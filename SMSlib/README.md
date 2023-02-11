SMSlib
======

Currently defined functions/macros:

 - Basic VDP handling
```
void SMS_init (void);                               /* you don't need to call this if you're using devkitSMS crt0 */
void SMS_VDPturnOnFeature (unsigned int feature);   /* check feature list in SMSlib.h */
void SMS_VDPturnOffFeature (unsigned int feature);
SMS_displayOn();                                    /* macro - turns on screen */
SMS_displayOff();                                   /* macro - turns off screen */
void SMS_waitForVBlank (void);                      /* wait until next vBlank starts */
void SMS_setBGScrollX (unsigned char scrollX);      /* scroll the background horizontally */
void SMS_setBGScrollY (unsigned char scrollY);      /* scroll the background vertically */
void SMS_setBackdropColor (unsigned char entry);    /* set which sprite palette entry will be used for backdrop */
void SMS_setSpriteMode (unsigned char mode);        /* check modes list in SMSlib.h */
void SMS_useFirstHalfTilesforSprites (_Bool usefirsthalf);  /* use tiles 0-255 for sprites if true, 256-511 if false */
```

 - Colors/palettes functions (Master System only)
```
void SMS_setBGPaletteColor (unsigned char entry, unsigned char color);
void SMS_setSpritePaletteColor (unsigned char entry, unsigned char color);
void SMS_loadBGPalette (void *palette);
void SMS_loadSpritePalette (void *palette);
SMS_setNextBGColoratIndex(i);                       /* macro - sets which BG color to set next */
SMS_setNextSpriteColoratIndex(i);                   /* macro - sets which sprite color to set next */
void SMS_setColor (unsigned char color);            /* set color */
void SMS_loadBGPaletteHalfBrightness (void *palette);
void SMS_loadSpritePaletteHalfBrightness (void *palette);
void SMS_zeroBGPalette (void);
void SMS_zeroSpritePalette (void);
RGB(r,g,b)                                          /* macro - compute SMS color (0-3)      */
RGB8(r,g,b)                                         /* macro - compute SMS color (0-255)    */
RGBHTML(RGB24bit);                                  /* macro - compute SMS color (0xRRGGBB) */
```

 - Colors/palettes functions (Game Gear only)
```
void GG_setBGPaletteColor (unsigned char entry, unsigned int color);
void GG_setSpritePaletteColor (unsigned char entry, unsigned int color);
void GG_loadBGPalette (void *palette);
void GG_loadSpritePalette (void *palette);
GG_setNextBGColoratIndex(i);                        /* macro - sets which BG color to set next */
GG_setNextSpriteColoratIndex(i);                    /* macro - sets which sprite color to set next */
void GG_setColor (unsigned char color);             /* set color */
RGB(r,g,b)                                          /* macro - compute GG color (0-15)     */
RGB8(r,g,b)                                         /* macro - compute GG color (0-255)    */
RGBHTML(RGB24bit);                                  /* macro - compute GG color (0xRRGGBB) */
```

 - Functions to load tiles into VRAM
```
void SMS_loadTiles (void *src, unsigned int Tilefrom, unsigned int size);
void SMS_loadSTC0compressedTiles (void *src, unsigned int Tilefrom);
void SMS_loadPSGaidencompressedTiles (void *src, unsigned int Tilefrom);

/* macros to load compressed tiles into VRAM */
SMS_loadSTC0compressedTiles(src,tilefrom)
SMS_loadZX7compressedTiles(src,tilefrom)

/* UNSAFE macro to load compressed tiles into VRAM (can be safely used only when screen is off) */
UNSAFE_SMS_loadaPLibcompressedTiles(src,tilefrom);
```

 - Functions to load a tilemap into VRAM
```
void SMS_loadTileMap (unsigned char x, unsigned char y, void *src, unsigned int size);
void SMS_loadTileMapArea (unsigned char x, unsigned char y,  unsigned int *src, unsigned char width, unsigned char height);

/* functions to load an STM compressed tilemap */
void SMS_loadSTMcompressedTileMap (unsigned char x, unsigned char y, unsigned char *src);
```

 - Functions/macros for tilemap handling
```
void SMS_setNextTileatXY (unsigned char x, unsigned char y);
void SMS_setTile (unsigned int tile);
SMS_setTileatXY (unsigned char x, unsigned char y, unsigned int tile);    /* macro - puts a tile at X/Y */
```

 - Functions to decompress compressed data (to RAM)
```
void SMS_decompressZX7 (const void *src, void *dst);
void SMS_decompressaPLib (const void *src, void *dst);
```

 - Functions for reading back tilemap and VRAM
```
SMS_readNextTilefromXY(x,y)
unsigned short SMS_getTile(void);
SMS_getTileatXY(x,y)                   /* macro - returns the tile at X/Y (unsigned int) */

void SMS_saveTileMapArea(unsigned char x, unsigned char y, void *dst, unsigned char width, unsigned char height);
void SMS_readVRAM(void *dst, unsigned int src, unsigned int size);
```

 - Sprite handling
```
void SMS_initSprites (void);                /* we're going to start declaring sprites, in front-to-back order */
signed char SMS_addSprite (unsigned char x, unsigned char y, unsigned char tile);  /* declare a sprite - returns handle or -1 if no more sprites are available */
void SMS_addTwoAdjoiningSprites (unsigned char x, unsigned char y, unsigned char tile);   /* doesn't return anything */
void SMS_addThreeAdjoiningSprites (unsigned char x, unsigned char y, unsigned char tile); /* doesn't return anything */
signed char SMS_reserveSprite (void);
void SMS_updateSpritePosition (signed char sprite, unsigned char x, unsigned char y);
void SMS_updateSpriteImage (signed char sprite, unsigned char tile);
void SMS_hideSprite (signed char sprite);
void SMS_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1); /* set the sprite window. sprites completely outside the window will be clipped */
signed char SMS_addSpriteClipping (int x, int y, unsigned char tile);   /* declare a sprite inside the window - returns handle or -1 if no more sprites are available or sprite clipped */
void SMS_copySpritestoSAT (void);           /* copy sprites to Sprites Attribute Table (do that during vBlank) */
```

 - Text renderer
```
void SMS_configureTextRenderer (signed int ascii_to_tile_offset);  /* set the value you should add to ASCII value to get the tile number */
void SMS_autoSetUpTextRenderer (void);                             /* load a standard font character set into tiles 0-95, set BG palette to B/W and turn on the screen */
void SMS_putchar (char c);                                         /* faster than plain putchar() */
void SMS_print(const char *str);                                   /* faster than printf() for unformatted strings */
SMS_printatXY(x,y,s);                                              /* macro - prints a string starting at X/Y */
```
 - Input handling
```
unsigned int SMS_getKeysStatus (void);    /* the current status of the keys */
unsigned int SMS_getKeysPressed (void);   /* the keys that were up last frame and down now */
unsigned int SMS_getKeysHeld (void);      /* the keys that were down last frame and still down now */
unsigned int SMS_getKeysReleased (void);  /* the keys that were down last frame and up now */

/* MD pad handling (SMS only) */
unsigned int SMS_getMDKeysStatus (void);  /* the current status of the extended keys on a MD controller */
unsigned int SMS_getMDKeysPressed (void); /* the extended keys that were up last frame and down now on a MD controller */
unsigned int SMS_getMDKeysHeld (void);    /* the extended keys that were down last frame and still down now on a MD controller */
unsigned int SMS_getMDKeysReleased (void); /* the extended keys that were down last frame and up now on a MD controller */

/* paddle controller handling */
_Bool SMS_detectPaddle (unsigned char port);
unsigned char SMS_readPaddle (unsigned char port);

/* pause handling (SMS only) */
_Bool SMS_queryPauseRequested (void);     /* the pause key has been pressed since previous check */
void SMS_resetPauseRequest (void);        /* reset/acknowledge pause requests */

_Bool SMS_detectPaddle (unsigned char port);                  /* detect paddle controller presence */
unsigned char SMS_readPaddle (unsigned char port);            /* read paddle controller position */
```
 - Line and Frame IRQ handling
```
/* line IRQ handling */
void SMS_setLineInterruptHandler (void (*theHandlerFunction)(void));  /* link your own handler to the line interrupt */
void SMS_setLineCounter (unsigned char count);                        /* choose on which line trigger the IRQ */
SMS_enableLineInterrupt()                                             /* macro - turns on line IRQ */
SMS_disableLineInterrupt()                                            /* macro - turns off line IRQ */

/* vertical interrupt hook */
/* If non-NULL, the specified function will be called by SMS_isr after acknowledging */
/* the interrupt and reading controller status. */
void SMS_setFrameInterruptHandler (void (*theHandlerFunction)(void)); /* link to your interrupt handler */
```

 - Other VDP handling
```
unsigned char SMS_getVCount (void);       /* Get Vertical count register */
unsigned char SMS_getHCount (void);       /* Get Horizontal count register */
unsigned char SMS_VDPType (void);         /* VDPType handling (SMS only) */
```

 - ROM/SRAM banking and access
```
SMS_mapROMBank(n);                        /* macro - maps bank n at address 0x8000 (slot 2) */

SMS_enableSRAM();                         /* macro - enable SRAM at address 0x8000 (slot 2) */
SMS_enableSRAMBank(n);                    /* macro - enable SRAM bank n (0 or 1) at address 0x8000 (slot 2) */
SMS_disableSRAM();                        /* macro - disable SRAM */
```

 - Low level VRAM functions
```
void SMS_VRAMmemcpy (unsigned int dst, void *src, unsigned int size);              /* memcpy to VRAM */
void SMS_VRAMmemcpy_brief (unsigned int dst, void *src, unsigned char size);       /* memcpy to VRAM (256 bytes max) */
void SMS_VRAMmemset (unsigned int dst, unsigned char value, unsigned int size);    /* memset (bytes) to VRAM */
void SMS_VRAMmemsetW (unsigned int dst, unsigned int value, unsigned int size);    /* memset (words) to VRAM */
```

  - VRAM unsafe functions. Fast, but dangerous, can be safely used only during VBlank or when screen is off
```
void UNSAFE_SMS_copySpritestoSAT (void);                         /* copy sprites to Sprites Attribute Table */
void UNSAFE_SMS_VRAMmemcpy32 (unsigned int dst, void *src);      /* copy 32 bytes to VRAM */
void UNSAFE_SMS_VRAMmemcpy64 (unsigned int dst, void *src);      /* copy 64 bytes to VRAM */
void UNSAFE_SMS_VRAMmemcpy128 (unsigned int dst, void *src);     /* copy 128 bytes to VRAM */

/* handy macros for UNSAFE_SMS_VRAMmemcpy* functions (can be safely used only during VBlank or when screen is off) */
UNSAFE_SMS_load1Tile(src,theTile)           /* copy ONE tile to VRAM */
UNSAFE_SMS_load2Tiles(src,tilefrom)         /* copy TWO tiles to VRAM */
UNSAFE_SMS_load4Tiles(src,tilefrom)         /* copy FOUR tiles to VRAM */
```

 - SEGA/SDSC headers
```
SMS_EMBED_SEGA_ROM_HEADER(productCode,revision);                                   /* macro - embed SEGA header into ROM */
SMS_EMBED_SDSC_HEADER(verMaj,verMin,dateYear,dateMonth,dateDay,author,name,descr); /* macro - embed SDSC homebrew header into ROM */
SMS_EMBED_SDSC_HEADER_AUTO_DATE(verMaj,verMin,author,name,descr);                  /* macro - embed auto timestamped SDSC homebrew header into ROM */

/* SEGA/SDSC headers when using banked code */
SMS_EMBED_SEGA_ROM_HEADER_16KB(productCode,revision);                                     /* macro - embed SEGA header into ROM - for banked code */
SMS_EMBED_SDSC_HEADER_16KB(verMaj,verMin,dateYear,dateMonth,dateDay,author,name,descr);   /* macro - embed SDSC homebrew header into ROM - for banked code */
SMS_EMBED_SDSC_HEADER_AUTO_DATE_16KB(verMaj,verMin,author,name,descr);                    /* macro - embed auto timestamped SDSC homebrew header into ROM - for banked code */
```

