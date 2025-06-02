/* **************************************************
   PSGlib - C programming library for the SEGA PSG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

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
extern unsigned char PSGMusicStatus;              // are we playing a background music?
extern void *PSGMusicStart;                       // the pointer to the beginning of music
extern void *PSGMusicPointer;                     // the pointer to the current
extern void *PSGMusicLoopPoint;                   // the pointer to the loop begin
#ifdef PSGLIB_MULTIBANK
extern unsigned char PSGMusicStartBank;
extern unsigned char PSGMusicPointerBank;
extern unsigned char PSGMusicLoopPointBank;
#endif
extern unsigned char PSGMusicSkipFrames;          // the frames we need to skip
extern unsigned char PSGLoopFlag;                 // the tune should loop infinitely or not (flag)
extern unsigned char PSGLoopCounter;              // how many times the tune should loop
extern unsigned char PSGMusicLastLatch;           // the last PSG music latch
extern unsigned char PSGMusicVolumeAttenuationChn[4];   // the volume attenuation applied to the tune (0-15) per each channel

//  decompression vars
extern unsigned char PSGMusicSubstringLen;        // lenght of the substring we are playing
extern void *PSGMusicSubstringRetAddr;            // return to this address when substring is over
#ifdef PSGLIB_MULTIBANK
extern unsigned char PSGMusicSubstringRetBank;
#endif

// volume/frequence buffering
extern unsigned char PSGChan0Volume;              // the volume for channel 0
extern unsigned char PSGChan1Volume;              // the volume for channel 1
extern unsigned char PSGChan2Volume;              // the volume for channel 2
extern unsigned char PSGChan3Volume;              // the volume for channel 3
extern unsigned char PSGChan0LowTone;             // the low tone bits for channel 0
extern unsigned char PSGChan0HighTone;            // the high tone bits for channel 0
extern unsigned char PSGChan1LowTone;             // the low tone bits for channel 1
extern unsigned char PSGChan1HighTone;            // the high tone bits for channel 1
extern unsigned char PSGChan2LowTone;             // the low tone bits for channel 2
extern unsigned char PSGChan2HighTone;            // the high tone bits for channel 2
extern unsigned char PSGChan3LowTone;             // the tone bits for channel 3

// flags for channels SFX access
extern unsigned char PSGChannel0SFX;              // !0 means channel 0 is allocated to SFX
extern unsigned char PSGChannel1SFX;              // !0 means channel 1 is allocated to SFX
extern unsigned char PSGChannel2SFX;              // !0 means channel 2 is allocated to SFX
extern unsigned char PSGChannel3SFX;              // !0 means channel 3 is allocated to SFX

// volume buffering for SFX
extern unsigned char PSGSFXChan0Volume;           // the volume for SFX channel 0
extern unsigned char PSGSFXChan1Volume;           // the volume for SFX channel 1
extern unsigned char PSGSFXChan2Volume;           // the volume for SFX channel 2
extern unsigned char PSGSFXChan3Volume;           // the volume for SFX channel 3

// fundamental vars for SFX
extern unsigned char PSGSFXStatus;                // are we playing a SFX?
extern void *PSGSFXStart;                         // the pointer to the beginning of SFX
extern void *PSGSFXPointer;                       // the pointer to the current address
extern void *PSGSFXLoopPoint;                     // the pointer to the loop begin
extern unsigned char PSGSFXSkipFrames;            // the frames we need to skip
extern unsigned char PSGSFXLoopFlag;              // the SFX should loop or not (flag)
extern unsigned char PSGSFXVolumeAttenuation;     // the volume attenuation applied to the SFX (0-15)

// decompression vars for SFX
extern unsigned char PSGSFXSubstringLen;          // lenght of the substring we are playing
extern void *PSGSFXSubstringRetAddr;              // return to this address when substring is over
