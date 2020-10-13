/* **************************************************
   MBMlib - C programming library for Moonblaster modules replay
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   code: 'Remco Schrijvers / MoonSoft', Kagesan, sverx
   ************************************************** */

#define MBMIVLIST_OFFSET           #0x00A3
#define MBMSTARTINSTR_OFFSET       #0x0101
#define MBMOPLDATA_OFFSET          #0x010A
#define MBMDRUMBLOCK_OFFSET        #0x0148
#define MBMDRUMVOL_OFFSET          #0x0157
#define MBMDRUMFREQ_OFFSET         #0x015A
#define MBMSTARTDETUNE_OFFSET      #0x016E

#define IX_PREFIX(...)    \
             .db 0xDD     \
             __VA_ARGS__

unsigned char const MBMFreqencyLUT[] = {
0xAD,0x00,0xB7,0x00,0xC2,0x00,0xCD,0x00,0xD9,0x00,0xE6,0x00,
0xF4,0x00,0x03,0x01,0x12,0x01,0x22,0x01,0x34,0x01,0x46,0x01,
0xAD,0x02,0xB7,0x02,0xC2,0x02,0xCD,0x02,0xD9,0x02,0xE6,0x02,
0xF4,0x02,0x03,0x03,0x12,0x03,0x22,0x03,0x34,0x03,0x46,0x03,
0xAD,0x04,0xB7,0x04,0xC2,0x04,0xCD,0x04,0xD9,0x04,0xE6,0x04,
0xF4,0x04,0x03,0x05,0x12,0x05,0x22,0x05,0x34,0x05,0x46,0x05,
0xAD,0x06,0xB7,0x06,0xC2,0x06,0xCD,0x06,0xD9,0x06,0xE6,0x06,
0xF4,0x06,0x03,0x07,0x12,0x07,0x22,0x07,0x34,0x07,0x46,0x07,
0xAD,0x08,0xB7,0x08,0xC2,0x08,0xCD,0x08,0xD9,0x08,0xE6,0x08,
0xF4,0x08,0x03,0x09,0x12,0x09,0x22,0x09,0x34,0x09,0x46,0x09,
0xAD,0x0A,0xB7,0x0A,0xC2,0x0A,0xCD,0x0A,0xD9,0x0A,0xE6,0x0A,
0xF4,0x0A,0x03,0x0B,0x12,0x0B,0x22,0x0B,0x34,0x0B,0x46,0x0B,
0xAD,0x0C,0xB7,0x0C,0xC2,0x0C,0xCD,0x0C,0xD9,0x0C,0xE6,0x0C,
0xF4,0x0C,0x03,0x0D,0x12,0x0D,0x22,0x0D,0x34,0x0D,0x46,0x0D,
0xAD,0x0E,0xB7,0x0E,0xC2,0x0E,0xCD,0x0E,0xD9,0x0E,0xE6,0x0E,
0xF4,0x0E,0x03,0x0F,0x12,0x0F,0x22,0x0F,0x34,0x0F,0x46,0x0F};

unsigned char const MBMDrumReg[] = {0x36,0x37,0x38,0x16,0x26,0x17,0x27,0x18,0x28};

signed int const MBMModVal[] = {1,2,2,-2,-2,-1,-2,-2,2,2};

unsigned char MBMStatus;           // status: 0 = not playing, 0xff = playing
void *MBMStart;                    // music data address
unsigned char MBMPosition;         // current position
unsigned char MBMStep;             // current step
unsigned char MBMStatusByte[3];    // status bytes
unsigned char MBMStepBuffer[13];   // step data to be played on the NEXT interrupt or has been played previously

unsigned char MBMFrequency;        // frequency
unsigned char MBMTransposeValue;   // transpose value
unsigned char MBMSpeed;            // speed value
unsigned char MBMSpeedCounter;     // speed counter
void *MBMPatternAddress;           // Pattern data start
void *MBMPatternPointer;           // current pattern line pointer
void *MBMPAT;                      // current pattern address table
unsigned char MBMFading;           // fading speed: if 0, no fading
unsigned char MBMFadeCounter;      // fade counter
unsigned char MBMChannelBuffer[9*14];   // channel buffer

void *MBMIVListAddr;               // instrument/volume list address
void *MBMDrumBlockAddr;            // drum set-up address

unsigned char MBMSongLength;       // song length
unsigned char MBMChannelInfo;      // number of channels: bit 6 (0-7) on is 6chn+drm, off is 9chn
unsigned char MBMLoopPoint;        // loop position

unsigned char MBMLastCustInstr;    // last programmed custom instrument

// SFX variables
unsigned char MBMSFXStatus;        // SFX status
void* MBMSFXStart;                 // ROM address of SFX
void* MBMSFXPointer;               // current address
void* MBMSFXLoopPoint;             // the pointer to the loop begin
unsigned char MBMSFXSkipFrames;    // wait counter for empty frames
unsigned char MBMSFXLoopFlag;      // looping? 1= yes, 0 = no
unsigned char MBMSFXDelayIVR;      // delay until instrument and volume are restored

void MBMPlay (void *module) __z88dk_fastcall __naked {

  MBMStart=module;

  __asm
    push ix                         ; we need to preserve IX for SDCC stack frame
    ld a, (_MBMStatus)              ; check if a song is already playing
    or a                            ; yes?
    call nz, _MBMStop               ; then stop it
    xor a
    ld (_MBMSpeedCounter), a        ; reset speed counter
    ld (_MBMFading), a              ; reset fading
    dec a
    ld (_MBMPosition), a            ; reset position
    ld (_MBMStatus), a              ; set status to "playing"
    ld (_MBMLastCustInstr),a        ; reset last custom instrument
    ld hl,(_MBMStart)
    ld de,MBMIVLIST_OFFSET
    add hl,de
    ld (_MBMIVListAddr),hl
    ld de,MBMDRUMBLOCK_OFFSET-MBMIVLIST_OFFSET
    add hl,de
    ld (_MBMDrumBlockAddr),hl
    ld hl, (_MBMStart)              ; transfer first part of header
    ld de, #_MBMSongLength
    ldi                             ; transfer song length [1 byte]
    ld bc, #2+144+16+32+10          ; skip song ID, msxa voice data, msxa instrument list, fm instrument / default volume list, channel chip set
    add hl, bc
    ld a,(hl)                       ;
    ld (_MBMSpeed),a                ; initialize tempo
    inc hl                          ;
    ldi                             ; transfer channel number (bit 5 on is 6 channels, off is 9 channels) [1 byte]
    ld bc, #41+9+9+48+6+8+15+3+20+9 ; skip song name, msxa start instruments, fm start instruments (9), fm opl data (48), fm opl prog nr (6), sample kit name (8), drum set-up (15), drum volumes (3), drum frequencies (20), start detune (9)
    add hl, bc
    ldi                             ; transfer loop position [1 byte]
    ld (_MBMPAT), hl                ; set pointer to start pattern of song
    ld a, (_MBMSongLength)          ; get song length
    inc a
    ld e, a
    ld d, #0
    add hl, de                      ; skip the song length to point to pattern address table
    ld (_MBMPatternAddress), hl     ; store the address of the pattern data start

; set start detune and instruments (reversed?) -->
    ld b, #9
    ld de, MBMSTARTINSTR_OFFSET+8
    ld iy, (_MBMStart)
    add iy,de
    ld ix,(_MBMDrumBlockAddr)
    ld de,MBMSTARTDETUNE_OFFSET-MBMDRUMBLOCK_OFFSET
    add ix,de
    ld hl, #_MBMChannelBuffer+112+3
    ld de, #-17                      ; skip through the channels reversed

_detuneLoop:
    ld (hl), #0                     ; reset frequency mode
    inc hl                          ; skip frequency register
    inc hl                          ; move to start detune
    ld a, 0 (ix)                    ; get start detune
    ld (hl), a                      ; set start detune
    inc hl                          ; move to start instrument
    ld a, 0 (iy)                    ; get start instrument
    ld (hl), a                      ; set start instrument
    inc ix
    dec iy
    add hl, de
    djnz _detuneLoop

; initialize frequency registers -->

    ld hl, #_MBMChannelBuffer+4
    ld a, #0x10                     ; first channel frequency register
    ld de, #14
    ld b, #9

_freqLoop:
    ld (hl), a
    inc a
    add hl, de
    djnz _freqLoop

; initialize volume registers -->

    ld hl, #_MBMChannelBuffer+7
    ld a, #0x30                     ; first channel volume register
    ld de, #14
    ld b, #9

_volLoop:
    ld (hl), a
    inc a
    add hl, de
    djnz _volLoop
    call InitInstr                  ; initialize instruments
    ld a, #15
    ld (_MBMStep), a
    ld a, #48
    ld (_MBMTransposeValue), a
    ld a, (_MBMSpeed)               ; skip leading silence
    sub #2                          ; (we have got to go back to the tick *before* the last one)
    ld (_MBMSpeedCounter), a
    pop ix                          ; restore SDCC stack frame pointer
    jp _MBMFrame                    ; refresh step buffer, end setup

; initialize instruments

InitInstr:
    ld hl,(_MBMStart)
    ld de,MBMSTARTINSTR_OFFSET
    add hl,de
    ld iy, #_MBMChannelBuffer
    ld b, #6
    ld a, (_MBMChannelInfo)         ; check if 6 or 9 channels
    bit 5, a
    push af
    jr nz, _skip9                   ; if 6 channels, continue
    ld b, #9                        ; else set 9 channels

_skip9:
    ld c, #48                       ; first register to write to

_setInstrLoop:
    push bc
    push hl
    ld a, (hl)
    ld hl, (_MBMIVListAddr)
    dec a
    add a, a
    ld c, a
    ld b, #0
    add hl, bc                      ; hl = adress of instrument/volume pair
    ld a, (hl)                      ; get start instrument
    cp #16                          ; check if hardware instrument or original
    call nc, InitOPL                ; if instr number >= 16, then it is a software instrument --> set it up
    rlca                            ; get instrument number into upper nibble
    rlca
    rlca
    rlca
    inc hl
    ld b, (hl)                      ; get start volume
    add a, b                        ; combine with instrument
    rlc b
    rlc b
    ld 13 (iy), b                   ; write FM volume
    ld bc, #14
    add iy, bc                      ; move to next channel
    pop hl
    pop bc
    call FMOut
    inc hl                          ; next start instrument
    inc c                           ; next register to write to
    djnz _setInstrLoop              ; repeat 6 or 9 times until setup is finished

; silence drums -->
    xor a
    ld c, #14
            ; 75
    ret nz  ; (always false, wait 5 cycles)
    nop     ;
    call FMOut
    pop af                          ; restore flags for test 9 or 6 fm channels
    ret z                           ; ret if 9 channels

; drums setup
    ld hl,(_MBMDrumBlockAddr)
    ld de,MBMDRUMVOL_OFFSET-MBMDRUMBLOCK_OFFSET
    add hl,de
    push hl
    ex de,hl
    ld hl, #_MBMDrumReg
    ld b, #9

_drumsetLoop:
    ld c, (hl)                      ; get register
    ld a, (de)                      ; get data
    call FMOut ;* 85
    inc hl
    inc de
    djnz _drumsetLoop
    pop hl
    ld a,(hl)
    and #0b00001111
    rlca
    rlca
    ld (_MBMChannelBuffer+(6*14)+13), a
    inc hl
    ld a,(hl)
    and #0b11110000
    rrca
    rrca
    ld (_MBMChannelBuffer+(7*14)+13), a
    inc hl
    ld a,(hl)
    and #0b00001111
    rlca
    rlca
    ld (_MBMChannelBuffer+(8*14)+13), a
    ret

InitOPL:
    push hl
    sub #16                         ; get number of original instrument
    ld hl,#_MBMLastCustInstr        ; avoid setting same instrument again
    cp (hl)
    call nz,MBMReprogramCustomInstr ; if not same instrument, reprogram chip
    pop hl                          ; revert to instr/vol pair list position
    xor a                           ; instrument number of original instrument is 0
    ret
  __endasm;
}

void MBMCancelLoop (void) __naked {
  __asm
    ld a, #0xFF
    ld (_MBMLoopPoint), a
    ret
  __endasm;
}

void MBMPlayNoRepeat (void *module) __z88dk_fastcall __naked {
  MBMPlay(module);
  MBMCancelLoop();
}

void MBMResume (void) __naked {
  __asm
    ld a, (_MBMStatus)                ; check if a song is already playing
    or a                              ; yes?
    ret nz                            ; then do nothing, return

    dec a
    ld (_MBMStatus), a                ; set status to "playing"
    ret
  __endasm;
}

void MBMStop (void) __naked {
  __asm
    ld a, (_MBMStatus)              ; check if a song is already playing
    or a                            ; no?
    ret z                           ; then just return

MBMSilenceChannels:
    push ix                         ; we need to preserve IX for SDCC stack frame

    ld de, #14
    ld iy, #_MBMChannelBuffer
    ld b, #9

    ld a, (_MBMSFXStatus)           ; check if sfx is playing
    IX_PREFIX(ld l, a)

_silenceLoop:
    call _MSXOut
    add iy, de                      ; move to next channel
    IX_PREFIX(ld l, #0)             ; reset sfx flag
    djnz _silenceLoop

    xor a
    ld (_MBMStatus), a              ; set status to 'not playing'
    pop ix                          ; restore SDCC stack frame pointer
    ret

_MSXOut:
    ld a, (_MBMChannelInfo)         ; check if 6 or 9 channels
    bit 5, a
    jr z, _skipThis
    ld a, b
    cp #4
    ret c

_skipThis:
    ld c, 4(iy)                     ; read frequency register
    xor a
    IX_PREFIX(dec l)                ; check if sfx is playing
    call nz, FMOut
    ld a, c                         ; set volume register
    add a, #0x10
    ld c, a
    xor a                           ; set key to off
    IX_PREFIX(inc l)                ; check if sfx is playing

    ret nz  ;* 71
    sub #0  ;* 7
    sub #0  ;* 7

    jp FMOut
  __endasm;
}

unsigned char MBMGetStatus (void) {
  return MBMStatus;
}

void MBMFadeOut (unsigned char fade_fraction) __z88dk_fastcall {
  MBMFading=fade_fraction;
}

void MBMFrame (void) __naked {
  __asm
    push ix                         ; we need to preserve IX for SDCC stack frame
    call _doMBMFrame
    pop ix                          ; restore SDCC stack frame pointer
    ret

_doMBMFrame:
    ld a, (_MBMStatus)              ; check if a song is already playing
    or a                            ; no?
    jp z, MBMSilenceChannels        ; then silence all channels

    ld a, (_MBMFading)              ; check for fade
    or a
    call nz, MBMDoFade              ; do fade if speed value is > 0
    call MBMDoPitchBend             ; pitch bend
    ld a, (_MBMSpeed)               ; handle speed
    ld hl, #_MBMSpeedCounter
    inc (hl)
    cp (hl)
    jp nz, _NoStep

; process music data
    ld (hl), #0                     ; reset speed counter
    ld iy, #_MBMChannelBuffer
    ld hl, #_MBMStepBuffer
    ld b, #9
    ld a, (_MBMSFXStatus)           ; check if sfx is playing
    ld d, a

_loopAllChans:
    push bc                         ; store channel counter
    ld a, (hl)                      ; get command of channel
    or a
    jp z, _noCommand                ; no command ?
    push hl                         ; store pointer to play buffer
    cp #97
    jp c, _OnEvent                  ; < 97 is a note
    jp z, _OffEvent                 ; 97 = silence
    cp #114
    jp c, _ChangeInstrEvent         ; change instrument
    cp #177
    jp c, _ChangeVolumeEvent        ; change volume
    cp #180
    jp c, _ContinueFrame            ; skip command, was -> chgste (stereo)
    cp #199
    jp c, _LinkEvent                ; link event        (180 -> L-9, 189 -> L+0, 198 -> L+9)
    cp #218
    jp c, _ChangePitchEvent         ; change pitch bend (199 -> P-9, 208 -> P+0, 217 -> P+9)
    cp #224
    jp c, _ChangeBrightnessNegEvent ; change brightness (negative)
    cp #231
    jp c, _ChangeDetuneEvent        ; change detune     (224 -> T-3, 227 -> T+0, 230 -> T+3)
    cp #237
    jp c, _ChangeBrightnessPosEvent ; change brightness (positive)
    cp #238
    jp c, _ChangeSustainEvent       ; change sustain
    jp _ChangeModulationEvent       ; change modulation

_ContinueFrame:
    pop hl

_noCommand:
    ld bc, #14
    add iy, bc                      ; point to next channel data
    inc hl                          ; next channel of step buffer
    pop bc                          ; restore channel counter
    ld d, #0                        ; reset sfx flag for all channels following channel 1
    djnz _loopAllChans              ; loop all 9 channels
    inc hl                          ; skip unused MSX-Audio channels
    inc hl
    call MBMPlayDrum

; process replayer command
    inc hl
    ld a, (hl)
    or a
    ret z                           ; no command
    cp #24
    jp c, MBMChangeTempo            ; command: change tempo
    jp z, MBMEndOfPattern           ; command: end of pattern
    cp #28
    jp c, MBMChangeDrumset          ; command: change drumset
    cp #31
    jp c, MBMChangeStatusByte       ; command: change status bytes
    jp MBMChangeTransposeValue      ; command: transpose

; check if there is need to prepare step buffer for next isr call

_NoStep:
    dec a
    cp (hl)
    ret nz
    ld a, (_MBMStep)
    inc a
    and #0b00001111                 ; wrap around 16 (0-15)
    ld (_MBMStep), a
    ld hl, (_MBMPatternPointer)     ; pattern pointer
    call z, MBMNextPattern          ; next pattern
    ld de, #_MBMStepBuffer          ; decrunch data
    ld c, #13

_loopData:
    ld a, (hl)
    cp #243
    jr nc, _decrunch                ; contains crunched data
    ld (de), a
    inc de
    dec c

_loopAllData:
    inc hl                          ; check if all 13 bytes are processed
    ld a, c
    or a
    jr nz, _loopData

; end of loop
    ld (_MBMPatternPointer), hl     ; store pattern line pointer
    jp _skipDecrunch

_decrunch:
    sub #242                        ; process crunched data
    ld b, a
    xor a

_loopDecrunch:
    ld (de), a
    inc de
    dec c
    djnz _loopDecrunch
    jr _loopAllData

_skipDecrunch:
    ld iy, #_MBMChannelBuffer
    ld hl, #_MBMStepBuffer
    ld b, #9

_loopAllChannels:
    push bc
    ld a, (hl)
    or a
    jr z, _gotoNext
    cp #97
    jr nc, _gotoNext
    push hl
    ld 3(iy), #0                    ; reset frequency mode
    ld c, a
    push bc
    push af
    call _FinishUp                  ; call nz, pacple
    pop af
    pop bc
    pop hl

_gotoNext:
    ld bc, #14                      ; move to next channel
    add iy, bc
    inc hl
    pop bc
    djnz _loopAllChannels
    ret

_FinishUp:
    ld a, (_MBMTransposeValue)      ; transpose
    add a, c
    cp #96+48+1
    jr c, _skipSub
    sub #96
    jr _skipAdd

_skipSub:
    cp #1+48
    jr nc, _skipAdd
    add a, #96

_skipAdd:
    sub #48
    ld 2(iy), a                    ; note link
    ld hl, #_MBMFreqencyLUT-1
    add a, a
    add a, l
    ld l, a
    jr nc, _skipInc
    inc h

_skipInc:
    ld a, (hl)
    ld 10(iy), a                    ; original frequency
    dec hl
    ld a, (hl)
    add a, 5(iy)                    ; tuning
    ld 11(iy), a                    ; original frequency
    ret

_OnEvent:                           ; on event
    call MBMKeyOn                   ; call nz, pacpl
    jp _ContinueFrame

_OffEvent:                          ; off event
    ld 3(iy), #0                    ; reset frequency mode
    call MBMKeyOff                  ; call nz, offpap
    jp _ContinueFrame

_ChangeSustainEvent:                ; change sustain
    ld 3(iy), #0                    ; reset frequency mode
    call MBMKeyOffSustained         ; call nz, suspap
    jp _ContinueFrame

_ChangeInstrEvent:                  ; change instrument
    ld 3(iy), #0                    ; reset frequency mode
    sub #97
    ld c, a
    call MBMChangeInstrument        ; call nz, chpaci
    jp _ContinueFrame

_ChangeVolumeEvent:                 ; change volume
    sub #114
    ld c, a
    call MBMChangeVolume            ; call nz, chpacv
    jp _ContinueFrame

_LinkEvent:                         ; link event
    sub #189
    ld c, a
    call MBMLinkNote                ; call nz, chlkpa
    jp _ContinueFrame

_ChangePitchEvent:                  ; change pitch bend
    sub #208
    ld 3(iy), #1                    ; set frequency mode: pitch
    ld 8(iy), a                     ; pitch value
    rlca
    jr c, _pitchNeg
    ld 9(iy), #0                    ; set pitch value 0 = positive
    jp _ContinueFrame

_pitchNeg:
    ld 9(iy), #0xFF                  ; set pitch value -1 = negativ
    call MBMChangePitch             ; call nz, chpidp
    jp _ContinueFrame

_ChangeBrightnessNegEvent:          ; change brightness (negative)
    sub #224
    jr _ChangeBrightness

_ChangeBrightnessPosEvent:          ; change brightness (positive)
    sub #230

_ChangeBrightness:
    push de
    ld c, a
    call MBMChangeBrightness        ; call nz, chpcbr
    pop de
    jp _ContinueFrame

_ChangeDetuneEvent:                 ; change detune
    sub #227
    ld 5(iy), a                    ; write detune
    jp _ContinueFrame

_ChangeModulationEvent:             ; change modulation
    ld 3(iy), #2                    ; set frequency mode: modulation
    jp _ContinueFrame

MBMNextPattern:                     ; next pattern
    ld a, (_MBMSongLength)
    ld b, a
    ld a, (_MBMPosition)
    cp b
    jr nz, _Next
    ld a, (_MBMLoopPoint)
    cp #255
    jr z, _skipLoopP
    dec a

_Next:
    inc a
    ld (_MBMPosition), a
    ld c, a
    ld b, #0
    ld hl, (_MBMPAT)
    add hl, bc
    ld a, (hl)
    dec a
    add a, a
    ld c, a
    ld hl, (_MBMPatternAddress)
    add hl, bc
    ld e, (hl)
    inc hl
    ld d, (hl)
    ex de, hl
    ld de, (_MBMStart)
    add hl, de
    ret

_skipLoopP:
    xor a
    ld (_MBMStatus), a
    dec a
    jr _Next

; note on event -->

MBMKeyOn:
    ld a, 11(iy)                    ; read original frequency
    ld c, 4(iy)                     ; get register
    bit 0, d                        ; check if sfx is playing
    call z, FMOut
    ld 0(iy), a                     ; write frequency
    ld a, c
    add a, #0x10
    ld c, a
    ld a, 10(iy)                    ; read original frequency
    bit 0, d                        ; check if sfx is playing
    call z, FMOut
    set 4, a
    ld 1(iy), a                     ; write  frequency
    bit 0, d                        ; check if sfx is playing
    ret nz ;* 79
    ret nz ; always false, here just to wait 5 more cycles
    jp FMOut

; note off event (sustained) -->

MBMKeyOffSustained:
    ld l, #0b00100000
    jr _skipKeyOff

; note off event -->

MBMKeyOff:
    ld l, #0
_skipKeyOff:
    ld c, 4(iy)                     ; get register
    ld a, 0(iy)                     ; read frequency
    bit 0, d                        ; check if sfx is playing
    call z, FMOut
    ld a, c
    add a, #0x10                    ; next register
    ld c, a
    ld a, 1(iy)                     ; read frequency
    and #0b11101111
    or l
    ld 1(iy), a                     ; write frequency
    bit 0, d                        ; check if sfx is playing
    jp z, FMOut
    ret

; change instrument -->

MBMChangeInstrument:
    ld a, c
    ld 6(iy), a                     ; write instrument
    dec a
    add a, a
    ld c, a
    ld b, #0
    ld hl,(_MBMIVListAddr)
    add hl, bc
    ld a, (hl)                      ; get instrument
    cp #16                          ; check if original instrument
    jp nc, MBMChangeOriginalInstr   ; if yes, set it up
    rlca                            ; get instrument number in upper nibble
    rlca
    rlca
    rlca

_ChangeInstrCont:
    inc hl
    ld c, (hl)                      ; get volume
    ld l, a
    add a, c                        ; combine instrument and volume
    push bc
    ld c, 7(iy)                     ; read volume register
    bit 0, d                        ; check if sfx is playing
    call z, FMOut
    pop bc
    rlc c
    rlc c
    ld b, 13(iy)                    ; get previous volume
    ld 13(iy), c                    ; store the new volume
    ld a, (_MBMFading)
    or a
    ret z                           ; return if fading is not active
    ld a, b
    cp c
    ret c                           ; return if old and new volume are smaller
    ld 13(iy), b
    srl b
    srl b
    ld a, l                         ; l contained the instrument value
    add a, b
    ld c, 7(iy)
    bit 0, d                        ; check if sfx is playing
    jp z, FMOut                     ; write the changed volume to the FM registers
    ret

MBMChangeOriginalInstr:
    exx
    sub #16
    ld hl,#_MBMLastCustInstr        ; avoid setting same instrument again
    cp (hl)
    call nz,MBMReprogramCustomInstr ; if not same instrument, reprogram chip
    exx
    xor a
    jp _ChangeInstrCont

; change volume -->

MBMChangeVolume:
    ld a, c
    push af
    srl a
    srl a
    ex af, af'                     ; '
    ld a, 6(iy)                    ; get instrument
    ld b, #0
    dec a
    add a, a
    ld c, a
    ld hl,(_MBMIVListAddr)
    add hl, bc
    ld a, (hl)
    cp #16
    jr c, _skipXor
    xor a

_skipXor:
    rlca
    rlca
    rlca
    rlca
    ld b, a
    ld c, 7(iy)
    ex af, af'                     ; '
    xor b
    bit 0, d                       ; check if sfx is playing
    call z, FMOut
    ld l, b
    pop bc
    ld c, 13(iy)                   ; read volume
    ld 13(iy), b                   ; write volume
    ld a, (_MBMFading)
    or a
    ret z

    ld a, c
    cp b
    ret c
    ld 13(iy), c
    ld a, c
    srl a
    srl a
    xor l
    ld c, 7(iy)
    bit 0, d                        ; check if sfx is playing
    jp z, FMOut
    ret

; note linking, legato -->

MBMLinkNote:
    ld a, 2(iy)
    add a, c
    ld 2(iy), a
    ld hl, #_MBMFreqencyLUT-1
    add a, a
    add a, l
    ld l, a
    jr nc, _skipInc2
    inc h

_skipInc2:
    ld a, (hl)
    ld (_MBMFrequency), a
    dec hl
    ld a, (hl)                      ; get frequency
    add a, 5(iy)                    ; add detune
    ld 0(iy), a                     ; write new frequency
    ld c, 4(iy)                     ; get register
    bit 0, d                        ; check if sfx is playing
    call z, FMOut
    ld a, c
    add a, #0x10                    ; next register
    ld c, a
    ld a, (_MBMFrequency)
    or #0b00010000
    ld 1(iy), a                     ; write new frequency
    ld 3(iy), #0                    ; reset frequency mode
    bit 0, d                        ; check if sfx is playing
    jp z, FMOut
    ret

; change brightness -->

MBMChangeBrightness:
    ld a, 6(iy)                    ; get instrument
    dec a
    add a, a
    ld e, a
    ld d, #0
    ld hl, (_MBMIVListAddr)
    add hl, de
    ld a, (hl)
    cp #16                          ; is instrument original?
    ret c                           ; no? then no brightness change possible

    ld a, 12(iy)
    and #0b11000000
    ld e, a
    ld a, 12(iy)
    and #0b00111111
    ld b, a
    ld a, c
    add a, b
    add a, e
    ld 12(iy), a
    ld c, #2
    jp FMOut

; change pitch bend -->

MBMChangePitch:
    ld a, 1(iy)
    bit 0, a
    ret nz

    dec a
    ld 1(iy), a
    ld a, 0(iy)
    add a, a
    ld 0(iy), a
    ret

; drums -->

MBMPlayDrum:
    ld a, (hl)
    and #0b00001111
    or a
    ret z

    dec a
    ld e, a
    ld d, #0
    push hl
    ld hl,(_MBMDrumBlockAddr)
    add hl, de
    ld a, (hl)
    ld c, a
    pop hl
    ld a, (_MBMChannelInfo)
    and #0b00100000
    ret z

    ld a, c
    and #0b00011111
    ld c, #0x0E
    call FMOut  ;* 47
    sub #0      ;*
    sub #0      ;*
    sub #0      ;*
    nop         ;*
    nop         ;*
    nop         ;*
    nop         ;*
    set 5, a
    jp FMOut

; command routines -->

MBMChangeTempo:                     ; change tempo
    ld b, a
    ld a, #25
    sub b
    ld (_MBMSpeed), a
    ret

MBMEndOfPattern:                    ; end of pattern
    ld a, #15
    ld (_MBMStep), a
    ret

MBMChangeDrumset:                   ; change drumset
    sub #25
    add a, a
    ld b, a
    add a, a
    add a, b
    ld e, a
    ld d, #0
    ld hl,(_MBMDrumBlockAddr)
    ld bc,#MBMDRUMFREQ_OFFSET-MBMDRUMBLOCK_OFFSET
    add hl,bc
    add hl,de
    ex de, hl
    ld hl, #_MBMDrumReg+3
    ld b, #6

_loopDrumset:
    ld c, (hl)
    ld a, (de)
    call FMOut
    inc hl
    inc de
    djnz _loopDrumset
    ret

; status bytes -->

MBMChangeStatusByte:
    ld c, a
    ld b, #0
    ld hl, #_MBMStatusByte-28
    add hl, bc
    ld (hl), #255
    ret

; change transpose -->

MBMChangeTransposeValue:
    sub #55-48
    ld (_MBMTransposeValue), a
    ret

; reprogram OPL custom instrument

MBMReprogramCustomInstr:
    ld (hl),a                       ; save instrument number to avoid setting same instrument again later
    add a,a
    add a,a
    add a,a
    add a, MBMOPLDATA_OFFSET & 0xFF
    ld c, a
    ld b, MBMOPLDATA_OFFSET >> 8
    ld hl,(_MBMStart)
    add hl, bc                      ; move to OPL data start address of original instrument
    push hl                         ; save start address
    inc hl
    inc hl
    ld a, (hl)                      ; get instrument
    ld 12(iy), a                    ; write instrument
    pop hl                          ; revert to OPL data start adress
    ld bc, #0x0800                  ; 8 registers, starting from $00
_loopProgram:
    ld a, (hl)                      ; get OPL data
    call FMOut                      ; cycles = 76
    nop                             ;
    nop                             ; +8 = 84
    inc c                           ; next register
    inc hl                          ; next data byte
    djnz _loopProgram               ; repeat 8 times
    ret

; 'every interrupt' routines -->

; music fade -->

MBMDoFade:                          ; decrease fade timer
    ld b, a
    ld a, (_MBMFadeCounter)
    inc a
    ld (_MBMFadeCounter), a
    cp b
    ret nz                          ; return until need to fade next step

    xor a
    ld (_MBMFadeCounter), a
    ld iy, #_MBMChannelBuffer
    ld b, #9
    ld hl, #0
    ld a, (_MBMSFXStatus)           ; check if sfx is playing
    IX_PREFIX(ld l, a)

_LoopThroughChannels:
    push bc
    ld a, b
    cp #3
    jr nc, _skipThis2
    ld a, (_MBMChannelInfo)
    bit 5, a                        ; check if 6 or 9 channels
    jp nz, _FadeDrums               ; 6ch mode

_skipThis2:
    push hl                         ; get instrument
    ld a, 6(iy)
    dec a
    add a, a
    ld c, a
    ld b, #0
    ld hl, (_MBMIVListAddr)
    add hl, bc
    ld a, (hl)
    cp #16                          ; check if original instrument
    jr c, _FadeChannel
    xor a

_FadeChannel:
    rlca                            ; get instrument number in upper nibble
    rlca
    rlca
    rlca
    ld b, a
    ld a, 13(iy)                    ; get volume
    add a, #4
    cp #64
    jr c, _skipResetVolume
    ld a, #63

_skipResetVolume:
    ld 13(iy), a                   ; write volume
    srl a
    srl a
    ld c, 7(iy)                    ; get volume register
    add a, b                       ; combine volume and instrument
    IX_PREFIX(dec l)               ; check if sfx is playing
    call nz, FMOut
    pop hl
    and #0b00001111
    xor #15
    ld e, a
    ld d, #0
    add hl, de
    ld bc, #14
    add iy, bc                      ; move to next channel
    pop bc
    IX_PREFIX(ld l, #0)             ; reset sfx flag
    dec b
    jp nz, _LoopThroughChannels
    ld a,h                          ; fading done?
    or l
    ret nz

    ld (_MBMStatus), a              ; set status to 'not playing'
    ret

_FadeDrums:                         ; fade drums
    push hl
    ld a, 13(iy)                    ; get volume
    srl a
    srl a
    jp _FadeChannel

; pitch bending -->

MBMDoPitchBend:
    ld iy, #_MBMChannelBuffer
    ld a, (_MBMSFXStatus)            ; check if sfx is playing
    IX_PREFIX(ld l, a)
    exx
    ld b, #9
    ld de, #14

_LoopPitchBend:
    exx
    ld a, 3(iy)                     ; get frequency mode
    ld h, a
    or a                            ; if not zero (0 = normal)
    call nz, _Pitch                 ; do pitch bend
    exx
    add iy, de                      ; move to next channel
    IX_PREFIX(ld l, #0)             ; reset sfx flag
    djnz _LoopPitchBend
    ret

_Pitch:
    dec h
    jr nz, _Modulate
    ld l, 8(iy)                     ; pitch mode
    ld h, 9(iy)                     ; pitch value
    ld c, 0(iy)                     ; frequency
    ld b, 1(iy)                     ; frequency
    bit 7, h
    jr nz, _skipPitchLoop
    add hl, bc
    ld a, b
    and #0b00000001
    ld b, a

_PitchLoop:
    ld 0(iy), l                     ; write frequency
    ld c, 4(iy)                     ; get frequency register
    ld a, l
    IX_PREFIX(dec l)
    call nz, FMOut
    ld a, c
    add a, #0x10                    ; next register
    ld c, a
    ld a, h
    or b
    ld 1(iy), a                     ; write frequency
    IX_PREFIX(inc l)
    jp z, FMOut
    ret

_skipPitchLoop:
    add hl, bc
    bit 0, h
    jr nz, _skipPitchLoop2
    dec h

_skipPitchLoop2:
    ld b, #0
    jp _PitchLoop

; modulate -->

_Modulate:
    ld a, h
    add a, #2
    cp #12
    jr nz, _skipReset
    ld a, #2

_skipReset:
    ld 3(iy), a                     ; set frequency mode: modulation
    ld a, h
    add a, a
    ld c, a
    ld b, #0
    ld hl, #_MBMModVal-2
    add hl, bc
    ld c, (hl)
    inc hl
    ld b, (hl)
    ld l, 0(iy)                     ; get frequency
    ld h, 1(iy)                     ; get frequency
    add hl, bc
    ld b, #0
    jp _PitchLoop

FMOut:
    ; C contains FM register #, A contains data
    ; to call here / to jp here:   17/10
    ex af, af'                 ; '   4
    ld a, c                    ;     4
    out (#0xF0), a             ;    11
    ex af, af'                 ; '   -
    out (#0xF1), a             ;     -
    ret                        ;    10
                               ; total =  46/39
  __endasm;
}

unsigned char SMS_GetFMAudioCapabilities (void) __naked {
  __asm

    ; first we need to perform region detection
    ; as devkitSMS currently does NOT support that :|

    ld a, #0b11110101               ; Output 1s on both TH lines
    out (#0x3f), a
    in a, (#0xdd)
    and #0b11000000                 ; See what the TH inputs are
    cp #0b11000000                  ; If the input does not match the output then it is a Japanese system
    jp nz, _IsJapanese

    ld a, #0b01010101               ; Output 0s on both TH lines
    out (#0x3f), a
    in a, (#0xdd)
    and #0b11000000                 ; See what the TH inputs are
    jp nz, _IsJapanese              ; If the input does not match the output then it is a Japanese system

    ld a, #0b11111111               ; Set everything back to being inputs
    out (#0x3f), a

    ld e, #1                        ; export = 1
    jr _getAudioCap

_IsJapanese:
    ld e, #0                        ; export = 0

_getAudioCap:
    ld a, (_SMS_Port3EBIOSvalue)
    or #0x04                        ; disable I/O chip
    out (#0x3E), a

    ld bc, #0                       ; reset counters

_next:
    ld a, b
    out (#0xF2), a                  ; output to the audio control port

    in a, (#0xF2)                   ; read back
    and #0b00000011                 ; mask to bits 0-1 only
    cp b                            ; check what is read is the same as what was written
    jr nz, _noinc

    inc c                           ; c = # of times the result is the same

_noinc:
    inc b                           ; increase counter
    bit 2, b                        ; repeated four times?
    jr z, _next                     ; no? then repeat again

    ld a, (_SMS_Port3EBIOSvalue)
    out (#0x3E), a                  ; turn I/O chip back on

    srl c                           ; 4 --> 2; 3, 2 --> 1; 0, 1 --> 0
    ld a, c
    bit 0, c                        ; check if PSG+FM (Japanese SMS) or PSG only
    jr z, _done                     ; yes? then transfer value directly

    add a,e                         ; else check region: if Region = 1 (Export)
                                    ; then 1 --> 2 (3rd party FM board);
                                    ; else 1 stays 1 unchanged (Mark III + FM unit)
_done:
    ld l, a                         ; return FM type
    ret
  __endasm;
}

#pragma save
#pragma disable_warning 85
void SMS_EnableAudio (unsigned char chips) __z88dk_fastcall __naked {
  __asm
  ld a, (_SMS_Port3EBIOSvalue)
  or #0x04                        ; disable I/O chip
  out (#0x3E), a

  ld a, l
  out (#0xF2), a                  ; output to the audio control port

  ld a, (_SMS_Port3EBIOSvalue)
  out (#0x3E), a                  ; turn I/O chip back on
  ret
  __endasm;
}
#pragma restore

// SFX

void MBMSFXPlay (void *sound_effect) __z88dk_fastcall __naked {

  MBMSFXStart=sound_effect;

  __asm
    xor a                           ; SFX is _NOT_ a looping one
    ld (_MBMSFXLoopFlag),a
    call MBMSFXSilence              ; stop any sfx that might be playing
    inc a
    ld (_MBMSFXStatus), a           ; set status to "playing"
    ld (_MBMSFXSkipFrames), a       ; next frame follows immediately
    ld hl,(_MBMSFXStart)            ; set pointer to start of sfx data
    ld (_MBMSFXPointer), hl
    ld (_MBMSFXLoopPoint), hl       ; loop pointer also points at the start for now
    xor a
    ld (_MBMSFXDelayIVR), a
    ret
  __endasm;
}

void MBMSFXPlayLoop (void *sound_effect) __z88dk_fastcall {
  MBMSFXPlay (sound_effect);
  MBMSFXLoopFlag=1;
}

void MBMSFXCancelLoop (void) {
  MBMSFXLoopFlag=0;
}

void MBMSFXStop (void) __naked {
  __asm
    ld a, #22                       ; set number of frames until I/V restore
    ld (_MBMSFXDelayIVR), a
    jp MBMSFXSilence                ; zero frequency

MBMSFXRestoreInstrVol:
    xor a
    ld (_MBMSFXStatus), a           ; reset status to "not playing"
    ld (_MBMSFXDelayIVR), a         ; reset delay timer
    ld a, (_MBMChannelBuffer+6)     ; get instrument number
    dec a
    ld b, #0
    add a, a
    ld c, a
    ld hl,(_MBMIVListAddr)
    add hl, bc
    ld a, (hl)                      ; get instrument from list
    cp #16
    jr c, _is_not_custom
    xor a
_is_not_custom:
    rlca                            ; move instrument into upper nibble
    rlca
    rlca
    rlca
    ld b, a
    ld a, (_MBMChannelBuffer+13)    ; get volume
    srl a
    srl a
    xor b                           ; combine volume with instrument
    ld c, #0x30
    jp FMOut                        ; restore instrument and volume, return

MBMSFXSilence:
    xor a
    ld c, #0x10                     ; set frequency to zero, key off
    call FMOut
    nop        ;*
    nop        ;*
    nop        ;*
    nop        ;*
    nop        ;*
    nop        ;*
    sub #0     ;*
    sub #0     ;*
    ld c, #0x20
    jp FMOut
  __endasm;
}

unsigned char MBMSFXGetStatus (void) {
  return MBMSFXStatus;
}

void MBMSFXFrame (void) __naked {
  __asm
    ld a, (_MBMSFXStatus)           ; check if sfx is playing
    or a                            ; no?
    ret z                           ; then just return

    ld a, (_MBMSFXDelayIVR)         ; check if inside delay period
    cp #1
    jp c, _proceed                  ; if not, proceed with sfx
    jp z, MBMSFXRestoreInstrVol     ; if end of delay, restore instrument/volume, end sfx
    dec a                           ; else reduce timer
    ld (_MBMSFXDelayIVR), a         ; store new value
    ret

_proceed:
    ld a, (_MBMSFXSkipFrames)       ; check if there are empty frames to skip
    dec a
    ld (_MBMSFXSkipFrames), a
    ret nz

    ld hl, (_MBMSFXPointer)
_sfx_loop:
    ld a, (hl)                      ; read command / register
    cp #16
    jp c, _EndFrame
    cp #255                         ; check if loop point
    jp nz, _loop_point
    inc hl
    ld (_MBMSFXLoopPoint), hl       ; set loop point
    jp _sfx_loop

_loop_point:
    ld c, a                         ; if a > 15, then it is a register number
    inc hl                          ; move to data byte
    ld a, (hl)                      ; read data byte
    call FMOut
    inc hl                          ; move to next command / register byte
    jp _sfx_loop                    ; repeat until frame ends

_EndFrame:
    or a                            ; check if end of sfx
    jp z, _EndSFX
    ld (_MBMSFXSkipFrames), a       ; store wait frames
    inc hl
    ld (_MBMSFXPointer), hl
    ret

_EndSFX:
    ld a, (_MBMSFXLoopFlag)         ; check if sfx is looping
    or a                            ; no?
    jp z, _MBMSFXStop               ; then stop sfx, restore instrument and volume
    ld a, #1
    ld (_MBMSFXSkipFrames), a       ; next frame follows immediately
    ld hl, (_MBMSFXLoopPoint)       ; set pointer back to loop point
    ld (_MBMSFXPointer), hl
    ret
  __endasm;
}
