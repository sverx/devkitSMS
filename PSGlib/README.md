PSGlib
======

crude list of currently defined functions/macros:

```
void PSGPlay (unsigned char *song);             /* this will make your PSG tune start */
void PSGCancelLoop (void);                      /* tell the library to stop the tune at next loop */
void PSGPlayNoRepeat (unsigned char *song);     /* this will make your PSG tune start and stop at loop */
void PSGStop (void);                            /* this will make your PSG tune stop */
unsigned char PSGGetStatus (void);              /* get the current status of the tune */

void PSGSFXPlay (unsigned char *sfx, unsigned char channels);         /* this will make your SFX start */
void PSGSFXPlayLoop (unsigned char *sfx, unsigned char channels);     /* this will make your looping SFX start */
void PSGSFXCancelLoop (void);                                         /* tell the library to stop the SFX at next loop */
void PSGSFXStop (void);                                               /* this will make your SFX stop */
unsigned char PSGSFXGetStatus (void);                                 /* get the current status of the SFX */

void PSGFrame (void);                           /* you should call this at a constant pace */
void PSGSFXFrame (void);                        /* you should call this too at a constant pace, if you use SFXs */
```
