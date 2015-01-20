SMSlib
======

list of defined functions:

void SMS_init (void);
void SMS_VDPturnOnFeature (unsigned int feature);
void SMS_VDPturnOffFeature (unsigned int feature);
void SMS_waitForVBlank (void);
void SMS_setBGPaletteColor (unsigned char entry, unsigned char color);
void SMS_setSpritePaletteColor (unsigned char entry, unsigned char color);
void SMS_loadBGPalette (const unsigned char *palette);
void SMS_loadSpritePalette (const unsigned char *palette);
void SMS_loadTiles (unsigned char *src, unsigned int Tilefrom, unsigned int len);
void SMS_loadTileMap (unsigned char x, unsigned char y, unsigned int *src, unsigned int len);
void SMS_loadTileMapArea (unsigned char x, unsigned char y,  unsigned int *src, unsigned char width, unsigned char height);
void SMS_setTileatXY (unsigned char x, unsigned char y, unsigned int tile);
void SMS_initSprites (void);
bool SMS_addSprite (unsigned char x, int y, unsigned char tile); /* returns false if no more sprites are available */
void SMS_finalizeSprites (void);
void SMS_copySpritestoSAT (void);
unsigned int SMS_getKeysStatus (void);
unsigned int SMS_getKeysPressed (void);
unsigned int SMS_getKeysHeld (void);
unsigned int SMS_getKeysReleased (void);
bool SMS_queryPauseRequested (void);
void SMS_resetPauseRequest (void);
