/* **************************************************
   PSGlib - C programming library for the SEGA PSG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "PSGlib.h"
#include "PSGlib_extern.h"

void PSGSetMusicVolumeAttenuation (unsigned char attenuation) {
/* *********************************************************************
  sets the volume attenuation for the music (0-15)
*/
  PSGMusicVolumeAttenuation=attenuation;
  if (PSGMusicStatus) {
    if (!(PSGChannelSFX & SFX_CHANNEL0))
      PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|(((PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuation);
    if (!(PSGChannelSFX & SFX_CHANNEL1))
      PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|(((PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuation);
    if (!(PSGChannelSFX & SFX_CHANNEL2))
      PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|(((PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuation);
    if (!(PSGChannelSFX & SFX_CHANNEL3))
      PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|(((PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuation);
  }
}

#ifndef PSGLIB_NOSFXCODE
void PSGSetSFXVolumeAttenuation (unsigned char attenuation) {
/* *********************************************************************
  sets the volume attenuation for the SFXs (0-15)
*/
  PSGSFXVolumeAttenuation=attenuation;
  if (PSGMusicStatus) {
    if (PSGChannelSFX & SFX_CHANNEL0)
      PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|(((PSGSFXChan0Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan0Volume&0x0F)+PSGSFXVolumeAttenuation);
    if (PSGChannelSFX & SFX_CHANNEL1)
      PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|(((PSGSFXChan1Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan1Volume&0x0F)+PSGSFXVolumeAttenuation);
    if (PSGChannelSFX & SFX_CHANNEL2)
      PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|(((PSGSFXChan2Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan2Volume&0x0F)+PSGSFXVolumeAttenuation);
    if (PSGChannelSFX & SFX_CHANNEL3)
      PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|(((PSGSFXChan3Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan3Volume&0x0F)+PSGSFXVolumeAttenuation);
  }
}
#endif
