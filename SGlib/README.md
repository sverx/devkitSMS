SGlib
=====

currently defined functions/macros:

```
/* basic VDP handling */
void SG_init (void);                                /* you don't even need to call this if you're using devkitSMS crt0_sg */
void SG_VDPturnOnFeature (unsigned int feature);    /* check feature list in SGlib.h */
void SG_VDPturnOffFeature (unsigned int feature);
SG_displayOn();                                     /* macro - turns on screen */
SG_displayOff();                                    /* macro - turns off screen */
void SG_setSpriteMode (unsigned char mode);         /* check sprite modes in SGlib.h */
void SG_setBackdropColor (unsigned char entry);
void SG_waitForVBlank (void);                       /* wait until next vBlank starts */

/* functions to load tiles patterns/colours into VRAM */
void SG_loadTilePatterns (void *src, unsigned int tilefrom, unsigned int size);
void SG_loadTileColours (void *src, unsigned int tilefrom, unsigned int size);
void SG_loadSpritePatterns (void *src, unsigned int tilefrom, unsigned int size);

/* functions for tilemap loading/handling */
void SG_loadTileMap (unsigned char x, unsigned char y, void *src, unsigned int size);
void SG_loadTileMapArea (unsigned char x, unsigned char y, void *src, unsigned char width, unsigned char height);
void SG_setNextTileatXY (unsigned char x, unsigned char y);
void SG_setTileatXY (unsigned char x, unsigned char y, unsigned char tile);
void SG_setTile (unsigned char tile);

/* sprite handling */
void SG_initSprites (void);
_Bool SG_addSprite (unsigned char x, unsigned char y, unsigned char tile, unsigned char attr);	/* returns false if no more sprites are available */
void SG_setClippingWindow (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1);
_Bool SG_addSpriteClipping (int x, int y, unsigned char tile, unsigned char attr);		/* returns false if no more sprites are available or sprite clipped */
void SG_finalizeSprites (void);
void SG_copySpritestoSAT (void);

/* joypad handling */
unsigned int SG_getKeysStatus (void);     /* the current status of the keys */
unsigned int SG_getKeysPressed (void);    /* the keys that were up last frame and down now */
unsigned int SG_getKeysHeld (void);       /* the keys that were down last frame and still down now */
unsigned int SG_getKeysReleased (void);   /* the keys that were down last frame and up now */

/* keyboard detection and handling */
_Bool SG_detectKeyboard (void);         /* true if an attached keyboard is detected */
void SG_scanKeyboardJoypad (void);      /* this scans the keyboard keys (emulating a joypad) */

/* functions to read the keyboard virtual joypad(s) */
unsigned int SG_getKeyboardJoypadStatus (void);
unsigned int SG_getKeyboardJoypadPressed (void);
unsigned int SG_getKeyboardJoypadHeld (void);
unsigned int SG_getKeyboardJoypadReleased (void);

/* pause handling */
_Bool SG_queryPauseRequested (void);      /* the pause key has been pressed since previous check */
void SG_resetPauseRequest (void);         /* reset/acknowledge pause requests */

/* low level functions */
void SG_VRAMmemcpy (unsigned int dst, void *src, unsigned int size);              /* memcpy to VRAM */
void SG_VRAMmemcpy_brief (unsigned int dst, void *src, unsigned char size);       /* memcpy to VRAM (256 bytes max) */
void SG_VRAMmemset (unsigned int dst, unsigned char value, unsigned int size);    /* memset to VRAM */
```
