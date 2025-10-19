/* **************************************************
   PSGlib - C programming library for the SEGA PSG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "PSGlib.h"
#include "PSGlib_extern.h"

void PSGSilenceChannels (void) {
/* *********************************************************************
  silence all the PSG channels
*/
  PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|0x0F;
  PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|0x0F;
  PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|0x0F;
  PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|0x0F;
}

void PSGRestoreVolumes (void) {
/* *********************************************************************
  restore the PSG channels volumes (if a tune or an SFX uses them!)
*/
  if (PSGChannel0SFX)
    PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|(((PSGSFXChan0Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan0Volume&0x0F)+PSGSFXVolumeAttenuation);
  else if (PSGMusicStatus)
    PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|(((PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuationChn[0]>15)?15:(PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuationChn[0]);
  if (PSGChannel1SFX)
    PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|(((PSGSFXChan1Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan1Volume&0x0F)+PSGSFXVolumeAttenuation);
  else if (PSGMusicStatus)
    PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|(((PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuationChn[1]>15)?15:(PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuationChn[1]);
  if (PSGChannel2SFX)
    PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|(((PSGSFXChan2Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan2Volume&0x0F)+PSGSFXVolumeAttenuation);
  else if (PSGMusicStatus)
    PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|(((PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuationChn[2]>15)?15:(PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuationChn[2]);
  if (PSGChannel3SFX)
    PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|(((PSGSFXChan3Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan3Volume&0x0F)+PSGSFXVolumeAttenuation);
  else if (PSGMusicStatus)
    PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|(((PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuationChn[3]>15)?15:(PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuationChn[3]);
}
