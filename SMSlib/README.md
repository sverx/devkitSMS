SMSlib
======

crude list of currently defined functions/macros:

```
void SMS_init (void);                     /* you don't even need to call this if you're using devkitSMS crt0 */
void SMS_VDPturnOnFeature (unsigned int feature);
void SMS_VDPturnOffFeature (unsigned int feature);
SMS_displayOn()                           /* macro - turns on screen and vblank IRQ */
SMS_displayOff()                          /* macro - turns off screen and vblank IRQ */
void SMS_waitForVBlank (void);            /* wait until next vBlank starts */
void SMS_setBGPaletteColor (unsigned char entry, unsigned char color);
void SMS_setSpritePaletteColor (unsigned char entry, unsigned char color);
void SMS_loadBGPalette (const unsigned char *palette);
void SMS_loadSpritePalette (const unsigned char *palette);
void SMS_loadTiles (unsigned char *src, unsigned int Tilefrom, unsigned int len);
void SMS_loadTileMap (unsigned char x, unsigned char y, unsigned int *src, unsigned int len);
void SMS_loadTileMapArea (unsigned char x, unsigned char y,  unsigned int *src, unsigned char width, unsigned char height);
void SMS_setTileatXY (unsigned char x, unsigned char y, unsigned int tile);
void SMS_initSprites (void);              /* we're going to start declaring sprites, in front-to-back order */
bool SMS_addSprite (unsigned char x, int y, unsigned char tile); /* returns false if no more sprites are available */
void SMS_finalizeSprites (void);          /* we're done declaring sprites */
void SMS_copySpritestoSAT (void);         /* copy sprites to Sprites Attribute Table (do that during vBlank) */
unsigned int SMS_getKeysStatus (void);    /* the current status of the keys */
unsigned int SMS_getKeysPressed (void);   /* the keys that were up last frame and down now */
unsigned int SMS_getKeysHeld (void);      /* the keys that were down last frame and still down now */
unsigned int SMS_getKeysReleased (void);  /* the keys that were down last frame and up now */
bool SMS_queryPauseRequested (void);      /* the pause key has been pressed since previous check */
void SMS_resetPauseRequest (void);        /* reset/acknowledge pause requests */
SMS_mapROMBank(n)                         /* macro - bank to map at 0x8000 */
```
