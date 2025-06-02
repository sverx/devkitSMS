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
    if (!PSGChannel0SFX) {
      PSGPort=PSGLatch|PSGChannel0|(PSGChan0LowTone&0x0F);       // restore channel 0 frequency
      PSGPort=PSGChan0HighTone&0x3F;
      PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|(((PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuationChn[0]>15)?15:(PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuationChn[0]); // restore channel 0 volume
    }
    if (!PSGChannel1SFX) {
      PSGPort=PSGLatch|PSGChannel1|(PSGChan1LowTone&0x0F);       // restore channel 1 frequency
      PSGPort=PSGChan1HighTone&0x3F;
      PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|(((PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuationChn[1]>15)?15:(PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuationChn[1]); // restore channel 1 volume
    }
    if (!PSGChannel2SFX) {
      PSGPort=PSGLatch|PSGChannel2|(PSGChan2LowTone&0x0F);       // restore channel 2 frequency
      PSGPort=PSGChan2HighTone&0x3F;
      PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|(((PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuationChn[2]>15)?15:(PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuationChn[2]); // restore channel 2 volume
    }
    if (!PSGChannel3SFX) {
      PSGPort=PSGLatch|PSGChannel3|(PSGChan3LowTone&0x0F);       // restore channel 3 frequency
      PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|(((PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuationChn[3]>15)?15:(PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuationChn[3]); // restore channel 3 volume
    }
    PSGMusicStatus=PSG_PLAYING;
  }
}
