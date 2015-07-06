SMSlib
======

currently defined functions/macros:

```
/* basic VDP handling */
void SMS_init (void);                               /* you don't even need to call this if you're using devkitSMS crt0 */
void SMS_VDPturnOnFeature (unsigned int feature);   /* check feature list in SMSlib.h */
void SMS_VDPturnOffFeature (unsigned int feature);
SMS_displayOn();                                    /* macro - turns on screen */
SMS_displayOff();                                   /* macro - turns off screen */
void SMS_waitForVBlank (void);                      /* wait until next vBlank starts */
void SMS_setBGScrollX (int scrollX);                /* scroll the background horizontally */
void SMS_setBGScrollY (int scrollY);                /* scroll the background vertically */
void SMS_setBackdropColor (unsigned char entry);    /* set which sprite palette entry will be used for backdrop */
void SMS_useFirstHalfTilesforSprites (_Bool usefirsthalf);  /* use tiles 0-255 for sprites if true, 256-511 if false */

/* palettes functions: SMS only */
void SMS_setBGPaletteColor (unsigned char entry, unsigned char color);
void SMS_setSpritePaletteColor (unsigned char entry, unsigned char color);
void SMS_loadBGPalette (void *palette);
void SMS_loadSpritePalette (void *palette);

/* palettes functions: GG only */
void GG_setBGPaletteColor (unsigned char entry, unsigned int color);
void GG_setSpritePaletteColor (unsigned char entry, unsigned int color);
void GG_loadBGPalette (void *palette);
void GG_loadSpritePalette (void *palette);

/* functions to load tiles into VRAM */
void SMS_loadTiles (void *src, unsigned int Tilefrom, unsigned int len);
void SMS_loadPSGaidencompressedTiles (void *src, unsigned int Tilefrom);

/* functions for tilemap loading/handling */
void SMS_loadTileMap (unsigned char x, unsigned char y, void *src, unsigned int len);
void SMS_loadSTMcompressedTileMap (unsigned char x, unsigned char y, unsigned char *src);
void SMS_loadTileMapArea (unsigned char x, unsigned char y,  unsigned int *src, unsigned char width, unsigned char height);
void SMS_setTileatXY (unsigned char x, unsigned char y, unsigned int tile);
void SMS_setNextTileatXY (unsigned char x, unsigned char y);
void SMS_setTile (unsigned int tile);

/* sprite handling */
void SMS_initSprites (void);              /* we're going to start declaring sprites, in front-to-back order */
_Bool SMS_addSprite (unsigned char x, unsigned char y, unsigned char tile);  /* declare a sprite - returns false if no more sprites are available */
void SMS_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1);   /* set the sprite window. sprites completely outside the window will be clipped */ 
_Bool SMS_addSpriteClipping (int x, int y, unsigned char tile);  /* declare a sprite inside the window - returns false if sprite isn't added */
void SMS_finalizeSprites (void);          /* we're done declaring sprites */
void SMS_copySpritestoSAT (void);         /* copy sprites to Sprites Attribute Table (do that during vBlank) */

/* input handling */
unsigned int SMS_getKeysStatus (void);    /* the current status of the keys */
unsigned int SMS_getKeysPressed (void);   /* the keys that were up last frame and down now */
unsigned int SMS_getKeysHeld (void);      /* the keys that were down last frame and still down now */
unsigned int SMS_getKeysReleased (void);  /* the keys that were down last frame and up now */

/* MD pad handling (SMS only) */
unsigned int SMS_getMDKeysStatus (void);  /* the current status of the extended keys on a MD controller */
unsigned int SMS_getMDKeysPressed (void); /* the extended keys that were up last frame and down now on a MD controller */
unsigned int SMS_getMDKeysHeld (void);    /* the extended keys that were down last frame and still down now on a MD controller */
unsigned int SMS_getMDKeysReleased (void); /* the extended keys that were down last frame and up now on a MD controller */

/* pause handling (SMS only) */
_Bool SMS_queryPauseRequested (void);      /* the pause key has been pressed since previous check */
void SMS_resetPauseRequest (void);        /* reset/acknowledge pause requests */

/* line IRQ handling */
void SMS_setLineInterruptHandler (void (*theHandlerFunction)(void));  /* link your own handler to the line interrupt */
void SMS_setLineCounter (unsigned char count);                        /* choose on which line trigger the IRQ */
SMS_enableLineInterrupt()                                             /* macro - turns on line IRQ */
SMS_disableLineInterrupt()                                            /* macro - turns off line IRQ */

/* ROM banking */
SMS_mapROMBank(n);                        /* macro - maps bank n at address 0x8000 (slot 2) */

/* low level functions */
void SMS_VRAMmemcpy (unsigned int dst, void *src, unsigned int size);              /* memcpy to VRAM */
void SMS_VRAMmemcpy_brief (unsigned int dst, void *src, unsigned char size);       /* memcpy to VRAM (256 bytes max) */
void SMS_VRAMmemset (unsigned int dst, unsigned char value, unsigned int size);    /* memset to VRAM */

/* SEGA/SDCC headers */
SMS_EMBED_SEGA_ROM_HEADER(productCode,revision);                                   /* macro - embed SEGA header into ROM */
SMS_EMBED_SDSC_HEADER(verMaj,verMin,dateYear,dateMonth,dateDay,author,name,descr); /* macro - embed SDSC homebrew header into ROM */
```
