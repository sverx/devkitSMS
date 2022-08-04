/* **************************************************
   PSGlib - C programming library for the SEGA PSG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#define PSG_STOPPED         0
#define PSG_PLAYING         1

#define SFX_CHANNEL2        #0x01
#define SFX_CHANNEL3        #0x02
#define SFX_CHANNELS2AND3   SFX_CHANNEL2|SFX_CHANNEL3

/* About multi bank support
 *
 * If PSGlib is compiled with MULTIBANK, the current bank in slot 2
 * will be remembered by PSGlib and the bank will be switched
 * automatically when calling PSGFrame(), but you must still take care
 * of switching to the correct bank before calling PSGPlay, PSGPlayLoops
 * or PSGPlayNoRepeat.
 *
 * PSGlib will automatically switch banks if the song data occupies more
 * than 16kB, but it is assumed that the data is not segmented. (i.e.
 * consecutive in ROM)
 *
 * Make sure to save/restore the bank in slot 2 before calling
 * PSGFrame(), as bank 2 will change.
 *
 * (SMS_saveROMBank/restoreROMBank can help here!)
 */

void PSGPlay (void *song);
void PSGPlayLoops (void *song, unsigned char loops);
void PSGCancelLoop (void);
void PSGPlayNoRepeat (void *song);
void PSGStop (void);
void PSGResume (void);
unsigned char PSGGetStatus (void);
void PSGSetMusicVolumeAttenuation (unsigned char attenuation);

void PSGSFXPlay (void *sfx, unsigned char channels);
void PSGSFXPlayLoop (void *sfx, unsigned char channels);
void PSGSFXCancelLoop (void);
void PSGSFXStop (void);
unsigned char PSGSFXGetStatus (void);

void PSGSilenceChannels (void);
void PSGRestoreVolumes (void);

void PSGFrame (void);
void PSGSFXFrame (void);
