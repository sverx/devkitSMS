/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SMSlib.h"
#include "SMSlib_common.c"
#include <stdbool.h>

/* declare various I/O ports in SDCC z80 syntax */
/* define IOPorts (for joypads) */
__sfr __at 0xDC IOPortL;
__sfr __at 0xDD IOPortH;

#ifdef TARGET_GG
/* define GG IOPort (GG START key) */
__sfr __at 0x00 GGIOPort;
#endif

#ifdef MD_PAD_SUPPORT
/* define IOPortCtrl (for accessing MD pad) */
__sfr __at 0x3F IOPortCtrl;
#define TH_HI                 0xF5
#define TH_LO                 0xD5
#endif

/* the VDP registers initialization value (ROM) */
const unsigned char VDPReg_init[11]={
                  0x04, /* reg0: Mode 4 */
                  0x20, /* reg1: display OFF - frame int (vblank) ON */
                  0xFF, /* reg2: PNT at 0x3800 */
                  0xFF, /* reg3: no effect (when in mode 4) */
                  0xFF, /* reg4: no effect (when in mode 4) */
                  0xFF, /* reg5: SAT at 0x3F00 */
                  0xFF, /* reg6: Sprite tiles at 0x2000 */
                  0x00, /* reg7: backdrop color (zero) */
                  0x00, /* reg8: scroll X (zero) */
                  0x00, /* reg9: scroll Y (zero) */
                  0xFF  /* regA: line interrupt count (offscreen) */
                                    };

/* the VDP registers #0 and #1 'shadow' (initialized RAM) */
unsigned char VDPReg[2]={0x04, 0x20};

volatile bool VDPBlank;               /* used by INTerrupt */
volatile unsigned char SMS_VDPFlags;  /* holds the sprite overflow and sprite collision flags */

#ifndef TARGET_GG
volatile bool PauseRequested;         /* used by NMI (SMS only) */
#ifdef  VDPTYPE_DETECTION
unsigned char VDPType;                /* used by NTSC/PAL and VDP type detection (SMS only) */
#endif
#endif

unsigned char SMS_Port3EBIOSvalue;    /* initialized by BIOS */

#ifndef NO_FRAME_INT_HOOK
/* If non-NULL, will be called by SMS_isr after acknowledging */
/* the interrupt and reading controller status */
void (*SMS_theFrameInterruptHandler)(void);
#endif

/* variables for pad handling */
volatile unsigned int KeysStatus,PreviousKeysStatus;
#ifdef MD_PAD_SUPPORT
volatile unsigned int MDKeysStatus,PreviousMDKeysStatus;
#endif

/* variables for sprite windowing and clipping */
unsigned char spritesHeight=8, spritesWidth=8, spritesTileOffset=1;

/* 'empty' line interrupt handler */
void (*SMS_theLineInterruptHandler)(void);

#ifndef TARGET_GG
#ifdef  VDPTYPE_DETECTION
inline void SMS_detect_VDP_type (void) {
  // INTERNAL FUNCTION
  unsigned char old_value,new_value;
  while (VDPVCounterPort!=0x80);      // wait for line 0x80
  new_value=VDPVCounterPort;
  do {
    old_value=new_value;              // wait until VCounter 'goes back'
    new_value=VDPVCounterPort;
  } while (old_value<=new_value);
  if (old_value>=0xE7)
    VDPType=VDP_PAL;                  // old value should be 0xF2
  else
    VDPType=VDP_NTSC;                 // old value should be 0xDA
}
#endif
#endif

void SMS_init (void) {
  // Initializes the lib
  unsigned char i;
  /* make sure the VDP is ready */
  while (VDPVCounterPort!=0xB0);
  /* set sprite palette color 0 to black */
#ifndef TARGET_GG
  SMS_setSpritePaletteColor(0, RGB(0,0,0));
#else
  GG_setSpritePaletteColor(0, RGB(0,0,0));
#endif
  /* VDP initialization */
  for (i=0;i<0x0B;i++)
    SMS_write_to_VDPRegister(i,VDPReg_init[i]);
  /* reset sprites */
  SMS_initSprites();
  // SMS_finalizeSprites();   // useless now!
  SMS_copySpritestoSAT();
#ifndef TARGET_GG
  /* init Pause (SMS only) */
  SMS_resetPauseRequest();
#ifdef  VDPTYPE_DETECTION
  /* PAL/NTSC detection (SMS only) */
  SMS_detect_VDP_type();
#endif
#endif
}

#ifndef TARGET_GG
#ifdef  VDPTYPE_DETECTION
unsigned char SMS_VDPType (void) {
  return VDPType;
}
#endif
#endif

void SMS_VDPturnOnFeature (unsigned int feature) __z88dk_fastcall {
  // turns on a VDP feature
  VDPReg[HI(feature)]|=LO(feature);
  SMS_write_to_VDPRegister (HI(feature),VDPReg[HI(feature)]);
}

void SMS_VDPturnOffFeature (unsigned int feature) __z88dk_fastcall {
  // turns off a VDP feature
  unsigned char val=~LO(feature);
  VDPReg[HI(feature)]&=val;
  SMS_write_to_VDPRegister (HI(feature),VDPReg[HI(feature)]);
}

void SMS_setBGScrollX (unsigned char scrollX) __z88dk_fastcall {
  SMS_write_to_VDPRegister(0x08,scrollX);
}

void SMS_setBGScrollY (unsigned char scrollY) __z88dk_fastcall {
  SMS_write_to_VDPRegister(0x09,scrollY);
}

void SMS_setBackdropColor (unsigned char entry) __z88dk_fastcall {
  SMS_write_to_VDPRegister(0x07,entry);
}

void SMS_useFirstHalfTilesforSprites (_Bool usefirsthalf) __z88dk_fastcall {
  SMS_write_to_VDPRegister(0x06,usefirsthalf?0xFB:0xFF);
}

void SMS_setSpriteMode (unsigned char mode) __z88dk_fastcall {
  if (mode & SPRITEMODE_TALL) {
    SMS_VDPturnOnFeature(VDPFEATURE_USETALLSPRITES);
    spritesHeight=16;
    spritesTileOffset=2;
  } else {
    SMS_VDPturnOffFeature(VDPFEATURE_USETALLSPRITES);
    spritesHeight=8;
    spritesTileOffset=1;
  }
  if (mode & SPRITEMODE_ZOOMED) {
    SMS_VDPturnOnFeature(VDPFEATURE_ZOOMSPRITES);
    spritesWidth=16;
    spritesHeight*=2;
  } else {
    SMS_VDPturnOffFeature(VDPFEATURE_ZOOMSPRITES);
    spritesWidth=8;
  }
}

#ifdef TARGET_GG
void GG_setBGPaletteColor (unsigned char entry, unsigned int color) {
  // SMS_set_address_CRAM(entry*2);
  SMS_setAddr(0xC000+(entry*2));
  // SMS_word_to_VDP_data(color);
  SMS_setTile(color);
}

void GG_setSpritePaletteColor (unsigned char entry, unsigned int color) {
  // SMS_set_address_CRAM((entry*2)|0x20);
  SMS_setAddr(0xC020+(entry*2));
  // SMS_word_to_VDP_data(color);
  SMS_setTile(color);
}
#else
void SMS_setBGPaletteColor (unsigned char entry, unsigned char color) {
  // SMS_set_address_CRAM(entry);
  SMS_setAddr(0xC000+entry);
  SMS_byte_to_VDP_data(color);
}

void SMS_setSpritePaletteColor (unsigned char entry, unsigned char color) {
  // SMS_set_address_CRAM(entry|0x10);
  SMS_setAddr(0xC010+entry);
  SMS_byte_to_VDP_data(color);
}
#endif

#pragma save
#pragma disable_warning 85
#ifdef TARGET_GG
void GG_loadBGPalette (const void *palette) __z88dk_fastcall {
  // *palette will be in HL
  ASM_LD_DE_IMM(#SMS_CRAMAddress);
  ASM_DE_TO_VDP_CONTROL;
  ASM_LD_B_IMM(#32);
  ASM_SHORT_XFER_TO_VDP_DATA;
}

void GG_loadSpritePalette (const void *palette) __z88dk_fastcall {
  // *palette will be in HL
  ASM_LD_DE_IMM(#SMS_CRAMAddress+0x20);
  ASM_DE_TO_VDP_CONTROL;
  ASM_LD_B_IMM(#32);
  ASM_SHORT_XFER_TO_VDP_DATA;
}
#else
void SMS_loadBGPalette (const void *palette) __z88dk_fastcall {
  // *palette will be in HL
  ASM_LD_DE_IMM(#SMS_CRAMAddress);
  ASM_DE_TO_VDP_CONTROL;
  ASM_LD_B_IMM(#16);
  ASM_SHORT_XFER_TO_VDP_DATA;
}

void SMS_loadSpritePalette (const void *palette) __z88dk_fastcall {
  // *palette will be in HL
  ASM_LD_DE_IMM(#SMS_CRAMAddress+0x10);
  ASM_DE_TO_VDP_CONTROL;
  ASM_LD_B_IMM(#16);
  ASM_SHORT_XFER_TO_VDP_DATA;
}

void SMS_setColor (unsigned char color) __z88dk_fastcall __preserves_regs(b,c,d,e,h,l,iyh,iyl) {
  // color will be in L
  ASM_L_TO_VDP_DATA;
}
#endif
#pragma restore

void SMS_waitForVBlank (void) {
  VDPBlank=false;
  while (!VDPBlank);
}

unsigned int SMS_getKeysStatus (void) {
  return (KeysStatus);
}

unsigned int SMS_getKeysPressed (void) {
  return (KeysStatus&(~PreviousKeysStatus));
}

unsigned int SMS_getKeysHeld (void) {
  return (KeysStatus&PreviousKeysStatus);
}

unsigned int SMS_getKeysReleased (void) {
  return ((~KeysStatus)&PreviousKeysStatus);
}

#ifdef MD_PAD_SUPPORT
unsigned int SMS_getMDKeysStatus (void) {
  return (MDKeysStatus);
}

unsigned int SMS_getMDKeysPressed (void) {
  return (MDKeysStatus&(~PreviousMDKeysStatus));
}

unsigned int SMS_getMDKeysHeld (void) {
  return (MDKeysStatus&PreviousMDKeysStatus);
}

unsigned int SMS_getMDKeysReleased (void) {
  return ((~MDKeysStatus)&PreviousMDKeysStatus);
}
#endif

#ifndef TARGET_GG
_Bool SMS_queryPauseRequested (void) {
  return(PauseRequested);
}

void SMS_resetPauseRequest (void) {
  PauseRequested=false;
}
#endif

#ifndef NO_FRAME_INT_HOOK
void SMS_setFrameInterruptHandler (void (*theHandlerFunction)(void)) __z88dk_fastcall {
  SMS_theFrameInterruptHandler=theHandlerFunction;
}
#endif

void SMS_setLineInterruptHandler (void (*theHandlerFunction)(void)) __z88dk_fastcall {
  SMS_theLineInterruptHandler=theHandlerFunction;
}

void SMS_setLineCounter (unsigned char count) __z88dk_fastcall {
  SMS_write_to_VDPRegister(0x0A,count);
}

/* Vcount */
unsigned char SMS_getVCount (void) {
  return(VDPVCounterPort);
}

/* Hcount */
unsigned char SMS_getHCount (void) {
  return(VDPHCounterPort);
}

/* Interrupt Service Routines */
#ifdef MD_PAD_SUPPORT
void SMS_isr (void) __naked {
  __asm
    push af
    push bc
    push de
    push hl
    push iy
    push ix
  __endasm;
  SMS_VDPFlags=VDPStatusPort;               /* read status port and write it to SMS_VDPFlags */
  if (SMS_VDPFlags & 0x80) {                /* this also aknowledge interrupt at VDP */
    VDPBlank=true;                          /* frame interrupt */
    PreviousKeysStatus=KeysStatus;
    PreviousMDKeysStatus=MDKeysStatus;
    KeysStatus=~(((IOPortH)<<8)|IOPortL);   /* TH status unimportant */
    IOPortCtrl=TH_LO;
    MDKeysStatus=IOPortL;
    if (!(MDKeysStatus & 0x0C)) {           /* verify it's a MD pad */
      MDKeysStatus=(~MDKeysStatus)&0x30;    /* read MD_A & MD_START */
      IOPortCtrl=TH_HI;
      IOPortCtrl=TH_LO;
      IOPortCtrl=TH_HI;
      IOPortCtrl=TH_LO;
      if (!(IOPortL & 0x0F)) {              /* verify we're reading a 6 buttons pad */
        IOPortCtrl=TH_HI;
        MDKeysStatus|=(~IOPortL)&0x0F;      /* read MD_MODE, MD_X, MD_Y, MD_Z */
        IOPortCtrl=TH_LO;
      }
    } else {
      MDKeysStatus=0;                       /* (because one might have detached his MD pad) */
    }
    IOPortCtrl=TH_HI;                       /* leave TH high */
#ifndef NO_FRAME_INT_HOOK
    if (SMS_theFrameInterruptHandler) {
        SMS_theFrameInterruptHandler();
    }
#endif
  } else
    SMS_theLineInterruptHandler();          /* line interrupt */
  __asm
    pop ix
    pop iy
    pop hl
    pop de
    pop bc
    pop af
    ei                                      /* Z80 disables interrupts on ISR so we should re-enable them explicitly. */
    reti                                    /* this is here because function is __naked */
  __endasm;
}
#else
void SMS_isr (void) __naked {
  __asm
    push af
    push hl
    in a,(_VDPStatusPort)                   /* also aknowledge interrupt at VDP */
    ld (_SMS_VDPFlags),a                    /* write flags to SMS_VDPFlags variable */
    rlca
    jr nc,1$
    ld hl,#_VDPBlank                        /* frame interrupt */
    ld (hl),#0x01
    ld hl,(_KeysStatus)
    ld (_PreviousKeysStatus),hl
    in a,(_IOPortL)                         /* read first pad */
    cpl
    ld hl,#_KeysStatus
    ld (hl),a                               /* save it in KeysStatus low byte */
    inc hl
#ifdef TARGET_GG
    in a,(_GGIOPort)                        /* read START button status */
    or #0x7F                                /* keep the msb only */
#ifdef GG_SECOND_PAD_SUPPORT
    ld (hl),a                               /* save it for later */
    in a,(_IOPortH)                         /* read second pad */
    or #0xF0                                /* keep the 4 lsb only */
    and (hl)                                /* combine the data */
#endif
#else
    in a,(_IOPortH)                         /* read second pad */
#endif
    cpl
    ld (hl),a                               /* save it in KeysStatus high byte */
#ifndef NO_FRAME_INT_HOOK
    ld hl,(_SMS_theFrameInterruptHandler)
    ld a,h
    or l
    jr z,2$                                 /* NULL? Do not call it */
    push bc
    push de
    push iy
    call ___sdcc_call_hl                    /* Call the function */
    pop iy
    pop de
    pop bc
#endif
    jr 2$
1$:                                         /* line interrupt */
    push bc
    push de
    push iy
    ld hl,(_SMS_theLineInterruptHandler)
    call ___sdcc_call_hl
    pop iy
    pop de
    pop bc
2$:
    pop hl
    pop af
    ei                                      /* Z80 disables interrupts on ISR so we should re-enable them explicitly. */
    reti                                    /* this is here because function is __naked */
  __endasm;
}
#endif

void SMS_nmi_isr (void) __naked {          /* this is for NMI */
  __asm
#ifndef TARGET_GG
    push hl
    ld hl,#_PauseRequested
    ld (hl),#0x01
    pop hl
#endif
    retn                                   /* this is here because function is __naked */
  __endasm;
}

/* EOF */
