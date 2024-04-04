/* **************************************************
   PSGlib - C programming library for the SEGA PSG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "PSGlib.h"

#define PSGDataPort         #0x7f

#define PSGLatch            #0x80
#define PSGData             #0x40

#define PSGChannel0         #0b00000000
#define PSGChannel1         #0b00100000
#define PSGChannel2         #0b01000000
#define PSGChannel3         #0b01100000
#define PSGVolumeData       #0b00010000

#define PSGWait             #0x38
#define PSGSubString        #0x08
#define PSGLoop             #0x01
#define PSGEnd              #0x00

/* define PSGPort (SDCC z80 syntax) */
__sfr __at 0x7F PSGPort;

// fundamental vars
unsigned char PSGMusicStatus;              // are we playing a background music?
void *PSGMusicStart;                       // the pointer to the beginning of music
void *PSGMusicPointer;                     // the pointer to the current
void *PSGMusicLoopPoint;                   // the pointer to the loop begin
#ifdef PSGLIB_MULTIBANK
unsigned char PSGMusicStartBank;
unsigned char PSGMusicPointerBank;
unsigned char PSGMusicLoopPointBank;
#endif
unsigned char PSGMusicSkipFrames;          // the frames we need to skip
unsigned char PSGLoopFlag;                 // the tune should loop infinitely or not (flag)
unsigned char PSGLoopCounter;              // how many times the tune should loop
unsigned char PSGMusicLastLatch;           // the last PSG music latch
unsigned char PSGMusicVolumeAttenuation;   // the volume attenuation applied to the tune (0-15)

//  decompression vars
unsigned char PSGMusicSubstringLen;        // lenght of the substring we are playing
void *PSGMusicSubstringRetAddr;            // return to this address when substring is over
#ifdef PSGLIB_MULTIBANK
unsigned char PSGMusicSubstringRetBank;
#endif

// volume/frequence buffering
unsigned char PSGChan0Volume;              // the volume for channel 0
unsigned char PSGChan1Volume;              // the volume for channel 1
unsigned char PSGChan2Volume;              // the volume for channel 2
unsigned char PSGChan3Volume;              // the volume for channel 3
unsigned char PSGChan0LowTone;             // the low tone bits for channel 0
unsigned char PSGChan0HighTone;            // the high tone bits for channel 0
unsigned char PSGChan1LowTone;             // the low tone bits for channel 1
unsigned char PSGChan1HighTone;            // the high tone bits for channel 1
unsigned char PSGChan2LowTone;             // the low tone bits for channel 2
unsigned char PSGChan2HighTone;            // the high tone bits for channel 2
unsigned char PSGChan3LowTone;             // the tone bits for channel 3

// flags for channels SFX access
unsigned char PSGChannel0SFX;              // !0 means channel 0 is allocated to SFX
unsigned char PSGChannel1SFX;              // !0 means channel 1 is allocated to SFX
unsigned char PSGChannel2SFX;              // !0 means channel 2 is allocated to SFX
unsigned char PSGChannel3SFX;              // !0 means channel 3 is allocated to SFX

// volume buffering for SFX
unsigned char PSGSFXChan0Volume;           // the volume for SFX channel 0
unsigned char PSGSFXChan1Volume;           // the volume for SFX channel 1
unsigned char PSGSFXChan2Volume;           // the volume for SFX channel 2
unsigned char PSGSFXChan3Volume;           // the volume for SFX channel 3

// fundamental vars for SFX
unsigned char PSGSFXStatus;                // are we playing a SFX?
void *PSGSFXStart;                         // the pointer to the beginning of SFX
void *PSGSFXPointer;                       // the pointer to the current address
void *PSGSFXLoopPoint;                     // the pointer to the loop begin
unsigned char PSGSFXSkipFrames;            // the frames we need to skip
unsigned char PSGSFXLoopFlag;              // the SFX should loop or not (flag)
unsigned char PSGSFXVolumeAttenuation;     // the volume attenuation applied to the SFX (0-15)

// decompression vars for SFX
unsigned char PSGSFXSubstringLen;          // lenght of the substring we are playing
void *PSGSFXSubstringRetAddr;              // return to this address when substring is over

void PSGStop (void) {
/* *********************************************************************
  stops the music (leaving the SFX on, if it's playing)
*/
  if (PSGMusicStatus) {
    if (!PSGChannel0SFX)
      PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|0x0F;   // latch channel 0, volume=0xF (silent)
    if (!PSGChannel1SFX)
      PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|0x0F;   // latch channel 1, volume=0xF (silent)
    if (!PSGChannel2SFX)
      PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|0x0F;   // latch channel 2, volume=0xF (silent)
    if (!PSGChannel3SFX)
      PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|0x0F;   // latch channel 3, volume=0xF (silent)
    PSGMusicStatus=PSG_STOPPED;
  }
}

void PSGResume (void) {
/* *********************************************************************
  resume the previously playing music (also using current attenuation)
*/
  if (!PSGMusicStatus) {
    if (!PSGChannel0SFX) {
      PSGPort=PSGLatch|PSGChannel0|(PSGChan0LowTone&0x0F);       // restore channel 0 frequency
      PSGPort=PSGChan0HighTone&0x3F;
      PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|(((PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuation); // restore channel 0 volume
    }
    if (!PSGChannel1SFX) {
      PSGPort=PSGLatch|PSGChannel1|(PSGChan1LowTone&0x0F);       // restore channel 1 frequency
      PSGPort=PSGChan1HighTone&0x3F;
      PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|(((PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuation); // restore channel 1 volume
    }
    if (!PSGChannel2SFX) {
      PSGPort=PSGLatch|PSGChannel2|(PSGChan2LowTone&0x0F);       // restore channel 2 frequency
      PSGPort=PSGChan2HighTone&0x3F;
      PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|(((PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuation); // restore channel 2 volume
    }
    if (!PSGChannel3SFX) {
      PSGPort=PSGLatch|PSGChannel3|(PSGChan3LowTone&0x0F);       // restore channel 3 frequency
      PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|(((PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuation); // restore channel 3 volume
    }
    PSGMusicStatus=PSG_PLAYING;
  }
}

#ifdef PSGLIB_MULTIBANK
void PSGPlay (void *song, unsigned char bank) {
#else
void PSGPlay (void *song) {
#endif
/* *********************************************************************
  receives the address of the PSG to start playing (continuously)
*/
  PSGStop();
  PSGLoopFlag=1;
  PSGMusicStart=song;           // store the begin point of music
  PSGMusicPointer=song;         // set music pointer to begin of music
  PSGMusicLoopPoint=song;       // loop pointer points to begin of music too

  PSGMusicSkipFrames=0;         // reset the skip frames
  PSGMusicSubstringLen=0;       // reset the substring len (for compression)
  PSGMusicLastLatch=PSGLatch|PSGChannel0|PSGVolumeData|0x0F;   // latch channel 0, volume=0xF (silent)
  PSGChan0Volume=0x0F;          // ensure unused channels remain silent
  PSGChan1Volume=0x0F;
  PSGChan2Volume=0x0F;
  PSGChan3Volume=0x0F;
  PSGMusicStatus=PSG_PLAYING;
#ifdef PSGLIB_MULTIBANK
  PSGMusicStartBank = bank;
  PSGMusicPointerBank = bank;
  PSGMusicLoopPointBank = bank;
#endif
}

#ifdef PSGLIB_MULTIBANK
void PSGPlayLoops (void *song, unsigned char bank, unsigned char loops) {
#else
void PSGPlayLoops (void *song, unsigned char loops) {
#endif
/* *********************************************************************
  receives the address of the PSG to start playing (continuously) and
  the number of loops (going back to loop point) requested
*/
#ifdef PSGLIB_MULTIBANK
  PSGPlay(song, bank);
#else
  PSGPlay(song);
#endif
  PSGLoopFlag=0;
  PSGLoopCounter=loops;
}

void PSGCancelLoop (void) {
/* *********************************************************************
  sets the currently looping music to no more loops after the current
*/
  PSGLoopFlag=0;
  PSGLoopCounter=0;
}

#ifdef PSGLIB_MULTIBANK
void PSGPlayNoRepeat (void *song, unsigned char bank) {
#else
void PSGPlayNoRepeat (void *song) {
#endif
/* *********************************************************************
  receives the address of the PSG to start playing (once)
*/
#ifdef PSGLIB_MULTIBANK
  PSGPlay(song, bank);
#else
  PSGPlay(song);
#endif
  PSGLoopFlag=0;
  PSGLoopCounter=0;
}

unsigned char PSGGetStatus (void) {
/* *********************************************************************
  returns the current status of music
*/
  return(PSGMusicStatus);
}

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
    PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|(((PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuation);
  if (PSGChannel1SFX)
    PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|(((PSGSFXChan1Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan1Volume&0x0F)+PSGSFXVolumeAttenuation);
  else if (PSGMusicStatus)
    PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|(((PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuation);
  if (PSGChannel2SFX)
    PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|(((PSGSFXChan2Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan2Volume&0x0F)+PSGSFXVolumeAttenuation);
  else if (PSGMusicStatus)
    PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|(((PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuation);
  if (PSGChannel3SFX)
    PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|(((PSGSFXChan3Volume&0x0F)+PSGSFXVolumeAttenuation>15)?15:(PSGSFXChan3Volume&0x0F)+PSGSFXVolumeAttenuation);
  else if (PSGMusicStatus)
    PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|(((PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuation);
}

void PSGSetMusicVolumeAttenuation (unsigned char attenuation) {
/* *********************************************************************
  sets the volume attenuation for the music (0-15)
*/
  PSGMusicVolumeAttenuation=attenuation;
  if (PSGMusicStatus) {
    if (!PSGChannel0SFX)
      PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|(((PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuation);
    if (!PSGChannel1SFX)
      PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|(((PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuation);
    if (!PSGChannel2SFX)
      PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|(((PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuation);
    if (!PSGChannel3SFX)
      PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|(((PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuation);
  }
}

void PSGSetSFXVolumeAttenuation (unsigned char attenuation) {
/* *********************************************************************
  sets the volume attenuation for the SFXs (0-15)
*/
  PSGSFXVolumeAttenuation=attenuation;
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

void PSGSFXStop (void) {
/* *********************************************************************
  stops the SFX (leaving the music on, if it's playing)
*/
  if (PSGSFXStatus) {
    if (PSGChannel0SFX) {
      if (PSGMusicStatus) {
        PSGPort=PSGLatch|PSGChannel0|(PSGChan0LowTone&0x0F);
        PSGPort=PSGChan0HighTone&0x3F;
        PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|(((PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan0Volume&0x0F)+PSGMusicVolumeAttenuation);
      } else {
        PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|0x0F;
      }
      PSGChannel0SFX=PSG_STOPPED;
    }

    if (PSGChannel1SFX) {
      if (PSGMusicStatus) {
        PSGPort=PSGLatch|PSGChannel1|(PSGChan1LowTone&0x0F);
        PSGPort=PSGChan1HighTone&0x3F;
        PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|(((PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan1Volume&0x0F)+PSGMusicVolumeAttenuation);
      } else {
        PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|0x0F;
      }
      PSGChannel1SFX=PSG_STOPPED;
    }

    if (PSGChannel2SFX) {
      if (PSGMusicStatus) {
        PSGPort=PSGLatch|PSGChannel2|(PSGChan2LowTone&0x0F);
        PSGPort=PSGChan2HighTone&0x3F;
        PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|(((PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan2Volume&0x0F)+PSGMusicVolumeAttenuation);
      } else {
        PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|0x0F;
      }
      PSGChannel2SFX=PSG_STOPPED;
    }

    if (PSGChannel3SFX) {
      if (PSGMusicStatus) {
        PSGPort=PSGLatch|PSGChannel3|(PSGChan3LowTone&0x0F);
        PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|(((PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuation>15)?15:(PSGChan3Volume&0x0F)+PSGMusicVolumeAttenuation);
      } else {
        PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|0x0F;
      }
      PSGChannel3SFX=PSG_STOPPED;
    }
    PSGSFXStatus=PSG_STOPPED;
  }
}

void PSGSFXPlay (void *sfx, unsigned char channels) {
/* *********************************************************************
  receives the address of the SFX to start and the mask that indicates
                                     which channel(s) the SFX will use
*/
  PSGSFXStop();
  PSGSFXLoopFlag=0;
  PSGSFXStart=sfx;              // store begin of SFX
  PSGSFXPointer=sfx;            // set the pointer to begin of SFX
  PSGSFXLoopPoint=sfx;          // looppointer points to begin too
  PSGSFXSkipFrames=0;           // reset the skip frames
  PSGSFXSubstringLen=0;         // reset the substring len
  PSGChannel0SFX=(channels&SFX_CHANNEL0)?PSG_PLAYING:PSG_STOPPED;
  PSGChannel1SFX=(channels&SFX_CHANNEL1)?PSG_PLAYING:PSG_STOPPED;
  PSGChannel2SFX=(channels&SFX_CHANNEL2)?PSG_PLAYING:PSG_STOPPED;
  PSGChannel3SFX=(channels&SFX_CHANNEL3)?PSG_PLAYING:PSG_STOPPED;
  PSGSFXStatus=PSG_PLAYING;
}

void PSGSFXCancelLoop (void) {
/* *********************************************************************
  sets the currently looping SFX to no more loops after the current
*/
  PSGSFXLoopFlag=0;
}

unsigned char PSGSFXGetStatus (void) {
/* *********************************************************************
  returns the current SFX status
*/
  return(PSGSFXStatus);
}

void PSGSFXPlayLoop (void *sfx, unsigned char channels) {
/* *********************************************************************
  receives the address of the SFX to start continuously and the mask
                    that indicates which channel(s) the SFX will use
*/
  PSGSFXPlay(sfx, channels);
  PSGSFXLoopFlag=1;
}

void PSGFrame (void) {
/* *********************************************************************
  processes a music frame
*/
__asm
  ld a,(_PSGMusicStatus)         ; check if we have got to play a tune
  or a
  ret z

  ld a,(_PSGMusicSkipFrames)     ; check if we have got to skip frames
  or a
  jp nz,_skipFrame

  ld hl,(_PSGMusicPointer)       ; read current address

_intLoop:
#ifdef PSGLIB_MULTIBANK
  call _PSG_ReadByte_B           ; load PSG byte (in B)
#else
  ld b,(hl)                      ; load PSG byte (in B)
  inc hl                         ; point to next byte
#endif

  ld a,(_PSGMusicSubstringLen)   ; read substring len
  or a
  jr z,_continue                 ; check if it is 0 (we are not in a substring)
  dec a                          ; decrease len
  ld (_PSGMusicSubstringLen),a   ; save len
  jr nz,_continue
  ld hl,(_PSGMusicSubstringRetAddr)  ; substring is over, retrieve return address
#ifdef PSGLIB_MULTIBANK
  ld a,(_PSGMusicSubstringRetBank)   ; also retreive the return bank
  ld (_PSGMusicPointerBank), a
#endif

_continue:
  ld a,b                         ; copy PSG byte into A
  cp PSGLatch                    ; is it a latch?
  jr c,_noLatch                  ; if < $80 then it is NOT a latch
  ld (_PSGMusicLastLatch),a      ; it is a latch - save it in "LastLatch"

  ; we have got the latch PSG byte both in A and in B
  ; and we have to check if the value should pass to PSG or not
  bit 4,a                        ; test if it is a volume
  jr nz,_latch_Volume            ; jump if volume data
  bit 6,a                        ; test if the latch it is for channels 0-1 or for 2-3
  jp nz,_latch_chn23             ; jump if it is for channels 2-3

  ; we have got the latch (tone, chn 0 or 1) PSG byte both in A and in B
  ; and we have to check if the value should be passed to PSG or not
  bit 5,a                        ; test if tone it is for channel 0 or 1
  jr z,_ifchn0                   ; jump if channel 0
  ld (_PSGChan1LowTone),a        ; save tone LOW data
  ld a,(_PSGChannel1SFX)         ; channel 1 available for music?
  or a
  jr z,_send2PSG_B               ; then send data
  jp _intLoop
_ifchn0:
  ld (_PSGChan0LowTone),a        ; save tone LOW data
  ld a,(_PSGChannel0SFX)         ; channel 0 available for music?
  or a
  jr z,_send2PSG_B               ; then send data
  jp _intLoop

_latch_chn23:
  ; we have got the latch (tone, chn 2 or 3) PSG byte both in A and in B
  ; and we have to check if the value should be passed to PSG or not
  bit 5,a                        ; test if tone it is for channel 2 or 3
  jr z,_ifchn2                   ; jump if channel 2
  ld (_PSGChan3LowTone),a        ; save tone LOW data
  ld a,(_PSGChannel3SFX)         ; channel 3 available for music?
  or a
  jp nz,_intLoop
  ld a,(_PSGChan3LowTone)
  and #3                         ; test if channel 3 is set to use the frequency of channel 2
  cp #3
  jr nz,_send2PSG_B              ; if channel 3 does not use frequency of channel 2 jump
  ld a,(_PSGSFXStatus)           ; test if an SFX is playing
  or a
  jr z,_send2PSG_B               ; if no SFX is playing jump
  ld (_PSGChannel3SFX),a         ; otherwise mark channel 3 as occupied
  ld a,PSGLatch|PSGChannel3|PSGVolumeData|#0x0F   ; and silence channel 3
  out (PSGDataPort),a
  jp _intLoop
_ifchn2:
  ld (_PSGChan2LowTone),a        ; save tone LOW data
  ld a,(_PSGChannel2SFX)         ; channel 2 available for music?
  or a
  jr z,_send2PSG_B
  jp _intLoop

_noLatch:
  cp PSGData
  jr c,_command                  ; if < $40 then it is a command
  ; it is a data
  ld a,(_PSGMusicLastLatch)      ; retrieve last latch
  jp _output_NoLatch

_latch_Volume:
  bit 6,a                        ; test if the latch it is for channels 0-1 or for 2-3
  jr nz,_latch_Volume_23         ; volume is for channel 2 or 3
  bit 5,a                        ; test if volume it is for channel 0 or 1
  jr z,_ifchn0v                  ; jump for channel 0
  ld (_PSGChan1Volume),a         ; save volume data
  ld a,(_PSGChannel1SFX)         ; channel 1 available for music?
  or a
  jr z,_sendVolume2PSG_B
  jp _intLoop

_ifchn0v:
  ld (_PSGChan0Volume),a         ; save volume data
  ld a,(_PSGChannel0SFX)         ; channel 0 available for music?
  or a
  jr z,_sendVolume2PSG_B
  jp _intLoop

_send2PSG_B:
  ld a,b
_send2PSG_A:
  out (PSGDataPort),a              ; output the byte
  jp _intLoop

_latch_Volume_23:
  bit 5,a                        ; test if volume it is for channel 2 or 3
  jr z,_chn2                     ; jump for channel 2
  ld (_PSGChan3Volume),a         ; save volume data
  ld a,(_PSGChannel3SFX)         ; channel 3 available for music?
  or a
  jr z,_sendVolume2PSG_B
  jp _intLoop
_chn2:
  ld (_PSGChan2Volume),a         ; save volume data
  ld a,(_PSGChannel2SFX)         ; channel 2 available for music?
  or a
  jr z,_sendVolume2PSG_B
  jp _intLoop

_skipFrame:
  dec a
  ld (_PSGMusicSkipFrames),a
  ret

_command:
  cp PSGWait
  jr z,_done                     ; no additional frames
  jr c,_otherCommands            ; other commands?
  and #0x07                      ; take only the last 3 bits for skip frames
  ld (_PSGMusicSkipFrames),a     ; we got additional frames
_done:
  ld (_PSGMusicPointer),hl       ; save current address
  ret                            ; frame done

_otherCommands:
  cp PSGSubString
  jr nc,_substring
  cp PSGEnd
  jr z,_musicLoop
  cp PSGLoop
  jr z,_setLoopPoint

  ; ***************************************************************************
  ; we should never get here!
  ; if we do, it means the PSG file is probably corrupted, so we just RET
  ; ***************************************************************************

  ret

_sendVolume2PSG_B:
  ld a,b
_sendVolume2PSG_A:
  ld c,a                           ; save the PSG command byte
  and #0x0F                        ; keep lower nibble
  ld b,a                           ; save value
  ld a,(_PSGMusicVolumeAttenuation) ; load volume attenuation
  add a,b                          ; add value
  cp #0x0F                         ; check overflow
  jr c,_no_overflow                ; if it is <=15 then ok
  ld a,#0x0F                       ; else, reset to 15
_no_overflow:
  ld b,a                           ; save new attenuated volume value
  ld a,c                           ; retrieve PSG command
  and #0xF0                        ; keep upper nibble
  or b                             ; set attenuated volume
  out (PSGDataPort),a              ; output the byte
  jp _intLoop

_output_NoLatch:
  ; we got the last latch in A and the PSG data in B
  ; and we have to check if the value should pass to PSG or not
  ; note that non-latch commands can be only contain frequencies (no volumes)
  ; for channels 0,1,2 only (no noise)
  bit 6,a                        ; test if the latch it is for channels 0-1 or for chn 2
  jr nz,_high_part_Tone_ch2      ; it is tone data for channel 2

  bit 5,a                        ; test if latch is for channel 0 or 1
  jr nz,_high_part_Tone_ch1      ; it is tone data for channel 1

  ; we got the last latch in A and the PSG data in B
  ; and we have to check if the value should pass to PSG or not
  ld a,b                         ; move PSG data in A
  ld (_PSGChan0HighTone),a       ; save channel 0 tone HIGH data
  ld a,(_PSGChannel0SFX)         ; channel 0 available for music?
  or a
  jr z,_send2PSG_B
  jp _intLoop

_high_part_Tone_ch1:
  ; we got the last latch in A and the PSG data in B
  ; and we have to check if the value should pass to PSG or not
  ld a,b                         ; move PSG data in A
  ld (_PSGChan1HighTone),a       ; save channel 1 tone HIGH data
  ld a,(_PSGChannel1SFX)         ; channel 1 available for music?
  or a
  jr z,_send2PSG_B
  jp _intLoop

_high_part_Tone_ch2:
  ; we got the last latch in A and the PSG data in B
  ; and we have to check if the value should pass to PSG or not
  ; PSG data can only be for channel 2, here
  ld a,b                         ; move PSG data in A
  ld (_PSGChan2HighTone),a       ; save channel 2 tone HIGH data
  ld a,(_PSGChannel2SFX)         ; channel 2 available for music
  or a
  jp z,_send2PSG_B               ; TOO FAR for JR here :|
  jp _intLoop

_setLoopPoint:
  ld (_PSGMusicLoopPoint),hl
#ifdef PSGLIB_MULTIBANK
  ld a, (_PSGMusicPointerBank)
  ld (_PSGMusicLoopPointBank), a
#endif

  jp _intLoop

_musicLoop:
  ld hl,(_PSGMusicLoopPoint)
#ifdef PSGLIB_MULTIBANK
  ld a, (_PSGMusicLoopPointBank)
  ld (_PSGMusicPointerBank), a
#endif
  ld a,(_PSGLoopFlag)              ; infinite loop requested?
  or a
  jp nz,_intLoop                   ; Yes: do loop
  ld a,(_PSGLoopCounter)           ; one more loop requested?
  or a
  jp z,_PSGStop                    ; No: stop the music! (tail call optimization)
  dec a
  ld (_PSGLoopCounter),a           ; decrement loop counter
  jp _intLoop                      ; do loop

_substring:
  sub PSGSubString-4                  ; len is value - $08 + 4
  ld (_PSGMusicSubstringLen),a        ; save len
#ifdef PSGLIB_MULTIBANK
  call _PSG_ReadByte_C                ; load substring address (offset)
  call _PSG_ReadByte_B
#else
  ld c,(hl)                           ; load substring address (offset)
  inc hl
  ld b,(hl)
  inc hl
#endif
  ld (_PSGMusicSubstringRetAddr),hl   ; save return address
#ifdef PSGLIB_MULTIBANK
  ld a,(_PSGMusicPointerBank)
  ld (_PSGMusicSubstringRetBank),a    ; save return bank
#endif
  ld hl,(_PSGMusicStart)
  add hl,bc                           ; make substring current
#ifdef PSGLIB_MULTIBANK
  ; Restrict to slot 2 (bit 15 set, bit 14 cleared)
  set 7, h
  res 6, h

  ld a, b                             ; Bits 14-15 are the bank offset
  rlc a
  rlc a
  and a, #0x03
  ld b, a

  ld a, (_PSGMusicStartBank)         ; compute current bank
  add a, b
  ld (_PSGMusicPointerBank), a       ; save new bank
#endif
  jp _intLoop

#ifdef PSGLIB_MULTIBANK
 // Reads a byte from HL, increment HL and take care of
 // bank overflow.
 //
 // Returns read value in B.
 // Updates HL, _PSGMusicPointerBank and current slot 2 bank.
 // Trashes A
_PSG_ReadByte_B:
  ld a,(_PSGMusicPointerBank)    ; Switch to current bank
  ld (0xFFFF), a
  ld b,(hl)                      ; load PSG byte (in B)
  inc hl                         ; point to next byte
  bit 6,h
  ret z
  res 6,h                        ; Reset the bit (back to slot 2)
  inc a                          ; And advance to next bank
  ld (_PSGMusicPointerBank), a   ; Save new bank
  ret

 // Same as above, but to C
_PSG_ReadByte_C:
  ld a,(_PSGMusicPointerBank)    ; Switch to current bank
  ld (0xFFFF), a
  ld c,(hl)                      ; load PSG byte (in C)
  inc hl                         ; point to next byte
  bit 6,h
  ret z
  res 6,h                        ; Reset the bit (back to slot 2)
  inc a                          ; And advance to next bank
  ld (_PSGMusicPointerBank), a   ; Save new bank
  ret
#endif
__endasm;
}

void PSGSFXFrame (void) {
/* ********************************************************************
   processes a SFX frame
*/
__asm
  ld a,(_PSGSFXStatus)           ; check if we have got to play SFX
  or a
  ret z

  ld a,(_PSGSFXSkipFrames)       ; check if we have got to skip frames
  or a
  jp nz,_skipSFXFrame

  ld hl,(_PSGSFXPointer)         ; read current SFX address

_intSFXLoop:
  ld b,(hl)                      ; load a byte in B, temporary
  inc hl                         ; point to next byte
  ld a,(_PSGSFXSubstringLen)     ; read substring len
  or a                           ; check if it is 0 (we are not in a substring)
  jr z,_SFXcontinue
  dec a                          ; decrease len
  ld (_PSGSFXSubstringLen),a     ; save len
  jr nz,_SFXcontinue
  ld hl,(_PSGSFXSubstringRetAddr) ; substring over, retrieve return address

_SFXcontinue:
  ld a,b                         ; restore byte
  cp PSGData
  jp c,_SFXcommand               ; if less than $40 then it is a command
  bit 7,a                        ; check if it is a latch
  jr z,_SFXoutbyte               ; if not, output it directly
  bit 4,a                        ; check if it is a volume byte
  jr z,_SFXoutbyte               ; if not, output it directly

  bit 6,a                        ; check if it is volume for channels 0-1 or for 2-3
  jr nz,_SFXchn23

  bit 5,a                        ; check if it is volume for channel 0 or channel 1
  jr nz,_SFXvolumechn1
  ld (_PSGSFXChan0Volume),a
  jp _SFXoutvolume

_SFXvolumechn1:
  ld (_PSGSFXChan1Volume),a
  jp _SFXoutvolume

_SFXchn23:
  bit 5,a                        ; check if it is volume for channel 2 or channel 3
  jr nz,_SFXvolumechn3
  ld (_PSGSFXChan2Volume),a
  jp _SFXoutvolume

_SFXvolumechn3:
  ld (_PSGSFXChan3Volume),a

_SFXoutvolume:
  and #0x0F                       ; keep lower nibble
  ld c,a                          ; save value
  ld a,(_PSGSFXVolumeAttenuation) ; load volume attenuation
  add a,c                         ; add value
  cp #0x0F                        ; check overflow
  jr c,_no_overflow_SFX           ; if it is <=15 then ok
  ld a,#0x0F                      ; else, reset to 15
_no_overflow_SFX:
  ld c,a                          ; save new attenuated volume value
  ld a,b                          ; retrieve original PSG command
  and #0xF0                       ; keep upper nibble (channel+volume bits)
  or c                            ; set attenuated SFX volume
_SFXoutbyte:
  out (PSGDataPort),a            ; output the byte
  jp _intSFXLoop

_skipSFXFrame:
  dec a
  ld (_PSGSFXSkipFrames),a
  ret

_SFXcommand:
  cp PSGWait
  jr z,_SFXdone                  ; no additional frames
  jr c,_SFXotherCommands         ; other commands?
  and #0x07                      ; take only the last 3 bits for skip frames
  ld (_PSGSFXSkipFrames),a       ; we got additional frames to skip
_SFXdone:
  ld (_PSGSFXPointer),hl         ; save current address
  ret                            ; frame done

_SFXotherCommands:
  cp PSGSubString
  jr nc,_SFXsubstring
  cp PSGEnd
  jr z,_sfxLoop
  cp PSGLoop
  jr z,_SFXsetLoopPoint

  ; ***************************************************************************
  ; we should never get here!
  ; if we do, it means the PSG SFX file is probably corrupted, so we just RET
  ; ***************************************************************************

  ret

_SFXsetLoopPoint:
  ld (_PSGSFXLoopPoint),hl
  jp _intSFXLoop

_sfxLoop:
  ld a,(_PSGSFXLoopFlag)              ; is it a looping SFX?
  or a
  jp z,_PSGSFXStop                    ; No:stop it! (tail call optimization)
  ld hl,(_PSGSFXLoopPoint)
  ld (_PSGSFXPointer),hl
  jp _intSFXLoop

_SFXsubstring:
  sub PSGSubString-4                  ; len is value - $08 + 4
  ld (_PSGSFXSubstringLen),a          ; save len
  ld c,(hl)                           ; load substring address (offset)
  inc hl
  ld b,(hl)
  inc hl
  ld (_PSGSFXSubstringRetAddr),hl     ; save return address
  ld hl,(_PSGSFXStart)
  add hl,bc                           ; make substring current
  jp _intSFXLoop
__endasm;
}
