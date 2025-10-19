/* **************************************************
   PSGlib - C programming library for the SEGA PSG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#define PSG_STOPPED         0
#define PSG_PLAYING         1

#define SFX_CHANNEL2        #0x01
#define SFX_CHANNEL3        #0x02
#define SFX_CHANNELS2AND3   SFX_CHANNEL2|SFX_CHANNEL3
#define SFX_CHANNEL0        #0x04
#define SFX_CHANNEL1        #0x08

/* About multi bank support
 *
 * If PSGlib is compiled with PSGLIB_MULTIBANK, the PSGPlayX functions
 * also take a bank argument. This initial bank is remembered by
 * PSGlib and the bank will be switched automatically when calling
 * PSGFrame().
 *
 * PSGlib will automatically switch bank if the song data occupies
 * more than 16kB, but it is assumed that the data is not segmented.
 * (i.e. it must be consecutive in ROM)
 *
 * When PSGFrame() returns, the current bank in slot 2 may have
 * been changed, so keep this in mind and take the necessary
 * steps if necessary. (Set the bank again after the call, or
 * use SMS_saveROMBank/restoreROMBank)
 *
 * When NOT compiled with PSGLIB_MULTIBANK, you must
 * set the correct bank before calling PSGFrame manually and
 * the song data cannot exceed 16kB.
 */

#ifdef PSGLIB_MULTIBANK
void PSGPlay (void *song, unsigned char bank);
void PSGPlayLoops (void *song, unsigned char loops, unsigned char bank);
void PSGPlayNoRepeat (void *song, unsigned char bank);
#else
void PSGPlay (void *song);
void PSGPlayLoops (void *song, unsigned char loops);
void PSGPlayNoRepeat (void *song);
#endif

void PSGCancelLoop (void);
void PSGStop (void);
void PSGResume (void);
unsigned char PSGGetStatus (void);

#define PSGSetMusicVolumeChannelsAttenuation(chn0_attenuation,chn1_attenuation,chn2_attenuation,chn3_attenuation)    PSGSetMusicVolumeChannelsAttenuation_f(((((unsigned int)(chn3_attenuation)&0x0F)<<12)|(((unsigned int)(chn2_attenuation)&0x0F)<<8)|(((unsigned int)(chn1_attenuation)&0x0F)<<4)|((unsigned int)(chn0_attenuation)&0x0F)))
#define PSGSetMusicVolumeAttenuation(attenuation)                                                                    PSGSetMusicVolumeChannelsAttenuation((attenuation),(attenuation),(attenuation),(attenuation))
void PSGSetMusicVolumeChannelsAttenuation_f (unsigned int channels_attenuation);
void PSGSetSFXVolumeAttenuation (unsigned char attenuation);

void PSGSFXPlay (void *sfx, unsigned char channels);
void PSGSFXPlayLoop (void *sfx, unsigned char channels);
void PSGSFXCancelLoop (void);
void PSGSFXStop (void);
unsigned char PSGSFXGetStatus (void);

void PSGSilenceChannels (void);
void PSGRestoreVolumes (void);

void PSGFrame (void);
void PSGSFXFrame (void);
