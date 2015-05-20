/* **************************************************
   PSGlib - C programming library for the SMS' PSG
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
unsigned char PSGMusicStatus=PSG_STOPPED;  // are we playing a background music?
void *PSGMusicStart;                       // the pointer to the beginning of music
void *PSGMusicPointer;                     // the pointer to the current
void *PSGMusicLoopPoint;                   // the pointer to the loop begin
unsigned char PSGMusicSkipFrames;          // the frames we need to skip
unsigned char PSGLoopFlag;                 // the tune should loop or not (flag)
unsigned char PSGMusicLastLatch;           // the last PSG music latch

//  decompression vars
unsigned char PSGMusicSubstringLen;        // lenght of the substring we are playing
void *PSGMusicSubstringRetAddr;            // return to this address when substring is over

// ******* SFX *************
// command buffering for channels 2-3
unsigned char PSGChan2Volume;              // the volume for channel 2
unsigned char PSGChan3Volume;              // the volume for channel 3
unsigned char PSGChan2LowTone;             // the low tone bits for channels 2
unsigned char PSGChan3LowTone;             //  the low tone bits for channels 3
unsigned char PSGChan2HighTone;            //  the high tone bits for channel 2

// flags for channels 2-3 access
unsigned char PSGChannel2SFX=0;            // !0 means channel 2 is allocated to SFX
unsigned char PSGChannel3SFX=0;            // !0 means channel 3 is allocated to SFX

// fundamental vars for SFX
unsigned char PSGSFXStatus=PSG_STOPPED;    // are we playing a SFX?
void *PSGSFXStart;                         // the pointer to the beginning of SFX
void *PSGSFXPointer;                       // the pointer to the current address
void *PSGSFXLoopPoint;                     // the pointer to the loop begin
unsigned char PSGSFXSkipFrames;            // the frames we need to skip
unsigned char PSGSFXLoopFlag;              // the SFX should loop or not (flag)

// decompression vars for SFX
unsigned char PSGSFXSubstringLen;          // lenght of the substring we are playing
void *PSGSFXSubstringRetAddr;              // return to this address when substring is over

void PSGStop (void) {
/* *********************************************************************
  stops the music (leaving the SFX on, if it's playing)
*/
  if (PSGMusicStatus) {
    PSGPort=PSGLatch|PSGChannel0|PSGVolumeData|0x0F;   // latch channel 0, volume=0xF (silent)
    PSGPort=PSGLatch|PSGChannel1|PSGVolumeData|0x0F;   // latch channel 1, volume=0xF (silent)
    if (!(PSGChannel2SFX))
      PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|0x0F;   // latch channel 2, volume=0xF (silent)
    if (!(PSGChannel3SFX))
      PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|0x0F;   // latch channel 3, volume=0xF (silent)
    PSGMusicStatus=PSG_STOPPED;
  }
}

void PSGPlay (void *song) {
/* *********************************************************************
  receives the address of the PSG to start playing (continuously)
*/
  PSGStop();
  PSGLoopFlag=1;
  PSGMusicStart=song;           // store the begin point of music
  PSGMusicPointer=song;         // set music pointer to begin of music
  PSGMusicLoopPoint=song;       // looppointer points to begin too

  PSGMusicSkipFrames=0;         // reset the skip frames
  PSGMusicSubstringLen=0;       // reset the substring len (for compression)
  PSGMusicLastLatch=PSGLatch|PSGChannel0|PSGVolumeData|0x0F;   // latch channel 0, volume=0xF (silent)
  PSGMusicStatus=PSG_PLAYING;
}

void PSGCancelLoop (void) {
/* *********************************************************************
  sets the currently looping music to no more loops after the current
*/
  PSGLoopFlag=0;
}

void PSGPlayNoRepeat (void *song) {
/* *********************************************************************
  receives the address of the PSG to start playing (once)
*/
  PSGPlay(song);
  PSGLoopFlag=0;
}

unsigned char PSGGetStatus (void) {
/* *********************************************************************
  returns the current status of music
*/
  return(PSGMusicStatus);
}

void PSGSFXStop (void) {
/* *********************************************************************
  stops the SFX (leaving the music on, if it's playing)
*/
  if (PSGSFXStatus) {
    if (PSGChannel2SFX) {
      if (PSGMusicStatus) {
        PSGPort=PSGLatch|PSGChannel2|(PSGChan2LowTone&0x0F);
        PSGPort=PSGChan2HighTone&0x3F;
        PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|(PSGChan2Volume&0x0F);
      } else {
        PSGPort=PSGLatch|PSGChannel2|PSGVolumeData|0x0F;
      }
      PSGChannel2SFX=PSG_STOPPED;
    }

    if (PSGChannel3SFX) {
      if (PSGMusicStatus) {
        PSGPort=PSGLatch|PSGChannel3|(PSGChan3LowTone&0x0F);
        PSGPort=PSGLatch|PSGChannel3|PSGVolumeData|(PSGChan3Volume&0x0F);
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
  ld a,(_PSGMusicStatus)          ; check if we have got to play a tune
  or a
  ret z

  ld a,(_PSGMusicSkipFrames)      ; check if we have got to skip frames
  or a
  jr nz,_skipFrame
  
  ld hl,(_PSGMusicPointer)        ; read current address

_intLoop:
  ld b,(hl)                      ; load PSG byte (in B)
  inc hl                         ; point to next byte
  ld a,(_PSGMusicSubstringLen)    ; read substring len
  or a
  jr z,_continue                 ; check if it is 0 (we are not in a substring)
  dec a                          ; decrease len
  ld (_PSGMusicSubstringLen),a    ; save len
  jr nz,_continue
  ld hl,(_PSGMusicSubstringRetAddr)  ; substring is over, retrieve return address

_continue:
  ld a,b                         ; copy PSG byte into A
  cp PSGLatch                    ; is it a latch?
  jr c,_noLatch                  ; if < $80 then it is NOT a latch
  ld (_PSGMusicLastLatch),a       ; it is a latch - save it
  
  ; we have got the latch PSG byte both in A and in B
  ; and we have to check if the value should pass to PSG or not
  bit 6,a                        ; test if the latch it is for channels 0-1 or for 2-3
  jr z,_send2PSG_A               ; jump if it is for channels 0-1
  bit 4,a                        ; test if it is a volume
  jr z,_low_part_Tone            ; jump if tone data (it is the low part)
  bit 5,a                        ; test if volume it is for channel 2 or 3
  jr z,_a1                       ; jump for channel 2
  ld (_PSGChan3Volume),a          ; save volume data
  ld a,(_PSGChannel3SFX)          ; channel 3 free?
  or a
  jr z,_send2PSG
  jp _intLoop
_a1:
  ld (_PSGChan2Volume),a          ; save volume data
  ld a,(_PSGChannel2SFX)          ; channel 2 free?
  or a
  jr z,_send2PSG
  jp _intLoop

_low_part_Tone:
  ; we have got the latch PSG byte both in A and in B
  ; and we have to check if the value should pass to PSG or not
  bit 5,a                        ; test if tone it is for channel 2 or 3
  jr z,_a2                     ; jump if channel 2
  ld (_PSGChan3LowTone),a         ; save tone LOW data
  ld a,(_PSGChannel3SFX)          ; channel 3 free?
  or a
  jr z,_send2PSG
  jp _intLoop

_a2:
  ld (_PSGChan2LowTone),a         ; save tone LOW data
  ld a,(_PSGChannel2SFX)          ; channel 2 free?
  or a
  jr z,_send2PSG
  jp _intLoop
  
_skipFrame:
  dec a
  ld (_PSGMusicSkipFrames),a
  ret

_noLatch:
  cp PSGData
  jr c,_command                  ; if < $40 then it is a command
  ; it is a data
  ld a,(_PSGMusicLastLatch)       ; retrieve last latch
  jp _output_NoLatch

_command:
  cp PSGWait
  jr z,_done                     ; no additional frames
  jr c,_otherCommands            ; other commands?
  and #0x07                      ; take only the last 3 bits for skip frames
  ld (_PSGMusicSkipFrames),a      ; we got additional frames
_done:
  ld (_PSGMusicPointer),hl        ; save current address
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

_send2PSG:
  ld a,b
_send2PSG_A:
  out (PSGDataPort),a              ; output the byte
  jp _intLoop

_output_NoLatch:
  ; we got the last latch in A and the PSG data in B
  ; and we have to check if the value should pass to PSG or not
  bit 6,a                        ; test if the latch it is for channels 0-1 or for 2-3
  jr z,_send2PSG                 ;    if it is for chn 0 or 1 then we have done!
  bit 4,a                        ; test if it is a volume
  jr z,_high_part_Tone           ; jump if tone data (it is the high part)
  bit 5,a                        ; test if volume it is for channel 2 or 3
  jr z,_a3
  ld a,b                         ; restore data in A
  ld (_PSGChan3Volume),a          ; save volume data
  ld a,(_PSGChannel3SFX)          ; channel 3 free?
  or a
  jr z,_send2PSG
  jp _intLoop
_a3:
  ld a,b                         ; restore data in A
  ld (_PSGChan2Volume),a          ; save volume data
  ld a,(_PSGChannel2SFX)          ; channel 2 free?
  or a
  jr z,_send2PSG
  jp _intLoop

_setLoopPoint:
  ld (_PSGMusicLoopPoint),hl
  jp _intLoop

_musicLoop:
  ld a,(_PSGLoopFlag)               ; looping requested?
  or a
  jp z,_PSGStop                    ; No:stop it!
  ld hl,(_PSGMusicLoopPoint)
  jp _intLoop

_substring:
  sub PSGSubString-4                  ; len is value - $08 + 4
  ld (_PSGMusicSubstringLen),a         ; save len
  ld c,(hl)                           ; load substring address (offset)
  inc hl
  ld b,(hl)
  inc hl
  ld (_PSGMusicSubstringRetAddr),hl    ; save return address
  ld hl,(_PSGMusicStart)
  add hl,bc                           ; make substring current
  jp _intLoop

_high_part_Tone:
  ; we got the last latch in A and the PSG data in B
  ; and we have to check if the value should pass to PSG or not
  bit 5,a                        ; test if tone it is for channel 2 or 3
  jr z,_a4                       ; jump if channel 2
  ld a,b                         ; PSG data in A
  and #0x07                      ; keep last 3 bits of PSG data only
  or PSGLatch|PSGChannel3        ; set it to latch for channel 3
  ld (_PSGChan3LowTone),a         ; save channel 3 tone LOW data (latched)
  ld a,(_PSGChannel3SFX)          ; channel 3 free?
  or a
  jr z,_send2PSG
  jp _intLoop
_a4:
  ld a,b                         ; PSG data in A
  ld (_PSGChan2HighTone),a        ; save channel 2 tone HIGH data
  ld a,(_PSGChannel2SFX)          ; channel 2 free?
  or a
  jr z,_send2PSG
  jp _intLoop
__endasm;
}

void PSGSFXFrame (void) {
/* ********************************************************************
   processes a SFX frame
*/
__asm
  ld a,(_PSGSFXStatus)            ; check if we have got to play SFX
  or a
  ret z

  ld a,(_PSGSFXSkipFrames)        ; check if we have got to skip frames
  or a
  jp nz,_skipFrame_SFX

  ld hl,(_PSGSFXPointer)          ; read current SFX address

_intLoop_SFX:
  ld b,(hl)                      ; load a byte in B, temporary
  inc hl                         ; point to next byte
  ld a,(_PSGSFXSubstringLen)      ; read substring len
  or a                           ; check if it is 0 (we are not in a substring)
  jr z,_continue_SFX
  dec a                          ; decrease len
  ld (_PSGSFXSubstringLen),a      ; save len
  jr nz,_continue_SFX
  ld hl,(_PSGSFXSubstringRetAddr) ; substring over, retrieve return address

_continue_SFX:
  ld a,b                         ; restore byte
  cp PSGData
  jp c,_command_SFX              ; if less than $40 then it is a command
  out (PSGDataPort),a            ; output the byte
  jp _intLoop_SFX

_skipFrame_SFX:
  dec a
  ld (_PSGSFXSkipFrames),a
  ret

_command_SFX:
  cp PSGWait
  jr z,_done_SFX                 ; no additional frames
  jr c,_otherCommands_SFX        ; other commands?
  and #0x07                      ; take only the last 3 bits for skip frames
  ld (_PSGSFXSkipFrames),a        ; we got additional frames to skip
_done_SFX:
  ld (_PSGSFXPointer),hl          ; save current address
  ret                            ; frame done

_otherCommands_SFX:
  cp PSGSubString
  jr nc,_substring_SFX
  cp PSGEnd
  jr z,_sfxLoop_SFX
  cp PSGLoop
  jr z,_setLoopPoint_SFX

  ; ***************************************************************************
  ; we should never get here!
  ; if we do, it means the PSG SFX file is probably corrupted, so we just RET
  ; ***************************************************************************

  ret

_setLoopPoint_SFX:
  ld (_PSGSFXLoopPoint),hl
  jp _intLoop_SFX

_sfxLoop_SFX:
  ld a,(_PSGSFXLoopFlag)               ; is it a looping SFX?
  or a
  jp z, _PSGSFXStop                  ; No:stop it!
  ld hl,(_PSGSFXLoopPoint)
  ld (_PSGSFXPointer),hl
  jp _intLoop_SFX

_substring_SFX:
  sub PSGSubString-4                  ; len is value - $08 + 4
  ld (_PSGSFXSubstringLen),a           ; save len
  ld c,(hl)                           ; load substring address (offset)
  inc hl
  ld b,(hl)
  inc hl
  ld (_PSGSFXSubstringRetAddr),hl    ; save return address
  ld hl,(_PSGSFXStart)
  add hl,bc                         ; make substring current
  jp _intLoop_SFX
__endasm;
}
