/* **************************************************
   PSGlib - C programming library for the SEGA PSG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "PSGlib.h"
#include "PSGlib_extern.h"

void PSGSetMusicVolumeChannelsAttenuation_f (unsigned int channels_attenuation) {
/* *********************************************************************
  sets the volume attenuation for the music (0-15) for each channel separately
*/
  PSGMusicVolumeAttenuationChn[0]=(channels_attenuation&0x0F);
  PSGMusicVolumeAttenuationChn[1]=((channels_attenuation>>4)&0x0F);
  PSGMusicVolumeAttenuationChn[2]=((channels_attenuation>>8)&0x0F);
  PSGMusicVolumeAttenuationChn[3]=(channels_attenuation>>12);
  if (PSGMusicStatus) {
    if (!PSGChannel0SFX)
      PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|(((PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuationChn[0]>15)?15:(PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuationChn[0]);
    if (!PSGChannel1SFX)
      PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|(((PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuationChn[1]>15)?15:(PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuationChn[1]);
    if (!PSGChannel2SFX)
      PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|(((PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuationChn[2]>15)?15:(PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuationChn[2]);
    if (!PSGChannel3SFX)
      PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|(((PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuationChn[3]>15)?15:(PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuationChn[3]);
  }
}

#ifndef PSGLIB_NOSFXCODE
void PSGSetSFXVolumeAttenuation (unsigned char attenuation) {
/* *********************************************************************
  sets the volume attenuation for the SFXs (0-15)
*/
  PSGSFXVolumeAttenuation=(attenuation&0x0F);
  if (PSGMusicStatus) {
    if (PSGChannel0SFX)
      PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|(((PSGSFXChan0Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan0Volume&0x0F)+PSGSFXVolumeAttenuation);
    if (PSGChannel1SFX)
      PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|(((PSGSFXChan1Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan1Volume&0x0F)+PSGSFXVolumeAttenuation);
    if (PSGChannel2SFX)
      PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|(((PSGSFXChan2Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan2Volume&0x0F)+PSGSFXVolumeAttenuation);
    if (PSGChannel3SFX)
      PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|(((PSGSFXChan3Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan3Volume&0x0F)+PSGSFXVolumeAttenuation);
  }
}
#endif
