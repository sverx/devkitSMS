/* **************************************************
   PSGlib - C programming library for the SEGA PSG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "PSGlib.h"
#include "PSGlib_extern.h"

void PSGResume (void) {
/* *********************************************************************
  resume the previously playing music (also using current attenuation)
*/
  if (!PSGMusicStatus) {
    if (!(PSGChannelSFX & SFX_CHANNEL0)) {
      PSGPort=PSGLatch|PSGChannel0|(PSGChan0LowTone&0x0F);       // restore channel 0 frequency
      PSGPort=PSGChan0HighTone&0x3F;
      PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|(((PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuation); // restore channel 0 volume
    }
    if (!(PSGChannelSFX & SFX_CHANNEL1)) {
      PSGPort=PSGLatch|PSGChannel1|(PSGChan1LowTone&0x0F);       // restore channel 1 frequency
      PSGPort=PSGChan1HighTone&0x3F;
      PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|(((PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuation); // restore channel 1 volume
    }
    if (!(PSGChannelSFX & SFX_CHANNEL2)) {
      PSGPort=PSGLatch|PSGChannel2|(PSGChan2LowTone&0x0F);       // restore channel 2 frequency
      PSGPort=PSGChan2HighTone&0x3F;
      PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|(((PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuation); // restore channel 2 volume
    }
    if (!(PSGChannelSFX & SFX_CHANNEL3)) {
      PSGPort=PSGLatch|PSGChannel3|(PSGChan3LowTone&0x0F);       // restore channel 3 frequency
      PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|(((PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuation); // restore channel 3 volume
    }
    PSGMusicStatus=PSG_PLAYING;
  }
}
