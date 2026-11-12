/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   code: na_th_an, sverx
   ************************************************** */

#include "SGlib.h"
#include "SGlib_common.h"
#include <stdbool.h>

/* the VDP registers initialization value */
const unsigned char VDPReg_init[8]={
  0x02, // 'Graphics II' video mode
  0xa0, // 16 KiB VRAM, screen off, VBlank IRQ active, 8×8 sprites (not zoomed)
  0x06, // PN bits 13-10 = 0 1 1 0         (address = $1800)
  0xff, // CT bits 13-7  = 1 x x x x x x x (address = $2000)
  0x03, // PG bits 13-11 = 0 x x           (address = $0000)
  0x36, // SA bits 13-7  = 0 1 1 0 1 1 0   (address = $1B00)
  0x07, // SG bits 13-11 = 1 1 1           (address = $3800)
  0x01  // black backdrop
};

/* the VDP registers #0 and #1 'shadow' RAM */
unsigned char VDPReg[2]={0x02, 0xa0};

volatile bool VDPBlank;   // used by INTerrupt

volatile unsigned int KeysStatus, PreviousKeysStatus;
#ifdef TARGET_CV
volatile unsigned char PadsStatus;
#endif

#ifndef TARGET_CV
volatile bool PauseRequested;   // used by NMI
#endif

/* ColecoVision VDP/NMI semaphore variables */
#ifdef TARGET_CV
_Bool CV_VDP_op_pending;
_Bool CV_NMI_srv_pending;
#endif

#ifndef NO_FRAME_INT_HOOK
/* If non-NULL, will be called by SG_isr after acknowledging */
/* the interrupt and reading controller status */
void (*SG_theFrameInterruptHandler)(void);
#endif

void SG_VDPturnOnFeature (unsigned int feature) {
  /* turns on a VDP feature */
  VDPReg[HI(feature)]|=LO(feature);
  SG_write_to_VDPRegister(HI(feature), VDPReg[HI(feature)]);
}

void SG_VDPturnOffFeature (unsigned int feature) {
  /* turns off a VDP feature */
  VDPReg[HI(feature)]&=~LO(feature);
  SG_write_to_VDPRegister(HI(feature), VDPReg[HI(feature)]);
}

void SG_init (void) {
  unsigned char i;
  for (i=0;i<8;i++)
    SG_write_to_VDPRegister (i, VDPReg_init[i]);
#ifdef TARGET_CV
  unsigned char VDPStatus=VDPStatusPort;          /* ensure there are no interrupts waiting aknowledgement in the VDP */
#endif
  SG_set_address_VRAM(SATADDRESS);                /* ensure the sprites table is empty */
  VDPDataPort=0xD0;
}

void SG_waitForVBlank (void) {
  VDPBlank=false;
  while (!VDPBlank);
}

#ifndef NO_FRAME_INT_HOOK
void SG_setFrameInterruptHandler (void (*theHandlerFunction)(void)) __z88dk_fastcall {
  SG_theFrameInterruptHandler=theHandlerFunction;
}
#endif

/* Interrupt Service Routines */
#ifndef TARGET_CV
void SG_isr (void) __critical __interrupt(0) {
  unsigned char VDPStatus=VDPStatusPort;  /* this aknowledges the VDP interrupt */
#ifdef AUTODETECT_SPRITE_OVERFLOW
  VDPSpriteOverflow=(VDPStatus & 0x40);
  VDPSpriteCollision=(VDPStatus & 0x20);
#endif
  if (VDPStatus & 0x80) {
    VDPBlank=true;         /* frame interrupt */
    /* read pad input */
    PreviousKeysStatus=KeysStatus;
    KeysStatus=~(((IOPortH)<<8)|IOPortL);
#ifndef NO_FRAME_INT_HOOK
    if (SG_theFrameInterruptHandler) {
      SG_theFrameInterruptHandler();
    }
#endif
  }
}

void SG_nmi_isr (void) __critical __interrupt {   /* this is for NMI */
  PauseRequested = true;
}
#else
void SG_isr_process (void) __naked {
  __asm
    push bc       ; AF and HL already saved
    push de
    push iy
  __endasm;
  unsigned char VDPStatus=VDPStatusPort;  /* this aknowledges the VDP interrupt */
#ifdef AUTODETECT_SPRITE_OVERFLOW
  VDPSpriteOverflow=(VDPStatus & 0x40);
  VDPSpriteCollision=(VDPStatus & 0x20);
#endif
  VDPBlank=true;         /* frame interrupt */
  /* read joy input */
  PreviousKeysStatus=KeysStatus;
  IOPortCTRLmode0=0xff;  // set the controllers in mode 0 (read joys directions)
  __asm
    ex (sp),hl    ; delay before reading the joy IO ports
    ex (sp),hl
  __endasm;
  unsigned int tempKeysStatus=~(((IOPortH|0x80)<<8)|(IOPortL|0x80));
  IOPortCTRLmode1=0xff;  // set the controllers in mode 1 (read numpad keys and right trigger)
  __asm
    ex (sp),hl    ; delay before reading the joy IO ports
    ex (sp),hl
  __endasm;
  PadsStatus=((IOPortH&0x0F)<<4)|(IOPortL&0x0F);
  KeysStatus=(((IOPortH&0x40)?0:0x8000)|((IOPortL&0x40)?0:0x80))|tempKeysStatus;
#ifndef NO_FRAME_INT_HOOK
  if (SG_theFrameInterruptHandler) {
    SG_theFrameInterruptHandler();
  }
#endif
  CV_NMI_srv_pending=false;
  __asm
    pop iy
    pop de
    pop bc
    ret           ; because I am naked
  __endasm;
}

void SG_nmi_isr (void) __naked {   /* this is for ColecoVision NMI */
  __asm
  push af
  push hl
    ld hl,#_CV_VDP_op_pending
    bit 0,(hl)
    jr z,1$                        ; VDP semaphore is off, we can serve NMI now
    ld hl,#_CV_NMI_srv_pending
    ld (hl),#1                     ; semaphore is on, defer NMI
    jr 2$
1$:
    call _SG_isr_process
2$:
  pop hl
  pop af
  retn
  __endasm;
}
#endif
