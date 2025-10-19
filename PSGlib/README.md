PSGlib
======

These are all the currently defined functions/macros:

```
void PSGPlay (void *song);                              /* this will make your PSG tune start and loop forever */
void PSGPlayLoops (void *song, unsigned char loops);    /* this will make your PSG tune start and loop the requested amount of times*/
void PSGPlayNoRepeat (void *song);                      /* this will make your PSG tune start and stop at loop */

void PSGCancelLoop (void);               /* tell the library to stop the tune at next loop */
void PSGStop (void);                     /* this will make your PSG tune stop (pause) */
void PSGResume (void);                   /* this will make your stopped (paused) PSG tune resume playing */
unsigned char PSGGetStatus (void);       /* get the current status of the tune */

PSGSetMusicVolumeChannelsAttenuation (chn0_attenuation,chn1_attenuation,chn2_attenuation,chn3_attenuation)   /* (macro) this will set the tune volume attenuation (0-15) separately for each channel */ 
PSGSetMusicVolumeAttenuation (attenuation)                                                                   /* (macro) this will set the tune volume attenuation (0-15) evenly for all the channels */
void PSGSetMusicVolumeChannelsAttenuation_f (unsigned int channels_attenuation);                             /* this is the underlying function for the above macros, and *NOT* meant to be called directly */

void PSGSFXPlay (unsigned char *sfx, unsigned char channels);         /* this will make your SFX start */
void PSGSFXPlayLoop (unsigned char *sfx, unsigned char channels);     /* this will make your looping SFX start */
void PSGSFXCancelLoop (void);                                         /* tell the library to stop the SFX at next loop */
void PSGSFXStop (void);                                               /* this will make your SFX stop */
unsigned char PSGSFXGetStatus (void);                                 /* get the current status of the SFX */

void PSGSetSFXVolumeAttenuation (unsigned char attenuation);    /* this will set the SFX volume attenuation (0-15) */

void PSGSilenceChannels (void);          /* this will silence all PSG channels */
void PSGRestoreVolumes (void);           /* this will restore PSG channels volume */

void PSGFrame (void);                    /* you should call this at a constant pace */
void PSGSFXFrame (void);                 /* you should call this too at a constant pace, if you use SFXs */
```

Multibank support
-----------------

If PSGlib is compiled with PSGLIB_MULTIBANK, the PSGPlay functions
also take a bank argument. This initial bank is remembered by
PSGlib and the bank will be switched automatically when calling
PSGFrame().

PSGlib will automatically switch bank if the song data occupies
more than 16kB, but it is assumed that the data is not segmented.
(i.e. it must be consecutive in ROM)

When PSGFrame() returns, the current bank in slot 2 may have
been changed, so keep this in mind and take the necessary
steps if necessary. (Set the bank again after the call, or
use SMS_saveROMBank/restoreROMBank)

When NOT compiled with PSGLIB_MULTIBANK, you must
set the correct bank before calling PSGFrame manually and
the song data cannot exceed 16kB.

```
/* Only if the library is compiled with PSGLIB_MULTIBANK.
 * Make sure to define PSGLIB_MULTIBANK before including
 * PSGlib.h
 */

void PSGPlay (void *song, unsigned char bank);                              /* this will make your PSG tune start and loop forever */
void PSGPlayLoops (void *song, unsigned char bank, unsigned char loops);    /* this will make your PSG tune start and loop the requested amount of times*/
void PSGPlayNoRepeat (void *song, unsigned char bank);                      /* this will make your PSG tune start and stop at loop */
```

