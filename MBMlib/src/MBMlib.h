/* **************************************************
   MBMlib - C programming library for Moonblaster modules replay
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   code: 'Remco Schrijvers / MoonSoft', Kagesan, sverx
   ************************************************** */

#define MBM_STOPPED         0
#define MBM_PLAYING         (~MBM_STOPPED)

void MBMPlay (void *module) __z88dk_fastcall __naked;
void MBMCancelLoop (void) __naked;
void MBMPlayNoRepeat (void *module) __z88dk_fastcall __naked;

void MBMStop (void) __naked;
void MBMResume (void) __naked;

unsigned char MBMGetStatus (void);

void MBMFadeOut (unsigned char fade_fraction) __z88dk_fastcall;

void MBMFrame (void) __naked;

/*  functions to detect and enable FM/PSG audio hardware */

#define SMS_AUDIO_NO_FM    0
#define SMS_AUDIO_FM_ONLY  1
#define SMS_AUDIO_FM_PSG   2

unsigned char SMS_GetFMAudioCapabilities (void) __naked;

#define SMS_ENABLE_AUDIO_FM_ONLY   0x01
#define SMS_ENABLE_AUDIO_FM_PSG    0x03
#define SMS_ENABLE_AUDIO_PSG_ONLY  0x00
#define SMS_ENABLE_AUDIO_NONE      0x02

void SMS_EnableAudio (unsigned char chips) __z88dk_fastcall __naked;

/* SFX */

#define MBMSFX_STOPPED         0
#define MBMSFX_PLAYING         1

void MBMSFXPlay (void *sound_effect) __z88dk_fastcall __naked;
void MBMSFXPlayLoop (void *sound_effect) __z88dk_fastcall;
void MBMSFXCancelLoop (void);

void MBMSFXStop (void) __naked;

unsigned char MBMSFXGetStatus (void);

void MBMSFXFrame (void) __naked;
