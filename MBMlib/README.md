MBMlib
======

A library to replay MoonBlaster modules on SEGA Master System / SEGA Mark III equipped with FM chip (either built in/add-on or mod board)

These are the currently defined functions for module replay:

```
void MBMPlay (void *module)              /* this will make your MoonBlaster module start and, if a loop has been defined in the module, loop infinitely */
void MBMCancelLoop (void)                /* tell the library to stop the looping tune at next loop point */
void MBMPlayNoRepeat (void *module)      /* this will make your MoonBlaster module start and stop at the end even if the module defined a loop */

void MBMStop (void)                      /* this will make the module stop (pause) */
void MBMResume (void)                    /* this will make your stopped (paused) tune resume playing */

unsigned char MBMGetStatus (void)        /* get the current status of the tune. possible values are: */

MBM_STOPPED                              /* (zero) no module currently playing */
MBM_PLAYING                              /* a module is playing */

void MBMFadeOut (unsigned char fade_fraction)   /* fade out the tune until the volume is zero and stop the tune */
                                                /* parameter is a fraction denominator so: */
                                                /* 0 is not valid, 1 will make the fade immediate and 255 is the slowest fade possible */

void MBMFrame (void)                     /* you should call this once per frame to replay the tune */
```

Then, there are functions for sound effect replay:

```
void MBMSFXPlay (void *sound_effect)     /* this will fire your sound effect */
void MBMSFXPlayLoop (void *sound_effect) /* this will start your sound effect in auto-retrigger mode */
void MBMSFXCancelLoop (void)             /* tells the library to cancel the auto-retrigging on the currently playing sound effect */

void MBMSFXStop (void)                   /* this stops the playing sound effect immediately */

unsigned char MBMSFXGetStatus (void)     /* get the current status of the sound effect. possible values are: */

MBMSFX_STOPPED                           /* (zero) no sound effect currently playing */
MBMSFX_PLAYING                           /* a sound effect is playing */

void MBMSFXFrame (void)                  /* you should call this once per frame to replay the sound effect */

```

Finally, the library also provides a function to detect the presence of the FM chip and a function to enable/disable audio chips:

```
unsigned char SMS_GetFMAudioCapabilities (void)    /* return if FM audio is available. possible values are: */

SMS_AUDIO_NO_FM                                    /* (zero) if no FM chip is available */
SMS_AUDIO_FM_ONLY                                  /* if FM chip is available but PSG audio won't be available when this is enabled */
SMS_AUDIO_FM_PSG                                   /* if FM chip is available and PSG audio can be enabled at the same time */

void SMS_EnableAudio (unsigned char chips)         /* use this to enable/disable audio chips. possible values are: */

SMS_ENABLE_AUDIO_FM_ONLY                           /* enable FM audio, disable PSG chip */
SMS_ENABLE_AUDIO_FM_PSG                            /* enable both audio chips, if possible */
SMS_ENABLE_AUDIO_PSG_ONLY                          /* disable FM chip, enable PSG audio */
SMS_ENABLE_AUDIO_NONE                              /* disable both audio chips, if possible */
```

