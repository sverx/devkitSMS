/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   code: na_th_an, sverx
   ************************************************** */

/*  --------------------------------------------------------- *
        VRAM memory map

    $0000 +---------+
          |   PGT   |  ($1800 bytes, pattern generator table)
    $1800 +---------+
          |   PNT   |  ($0300 bytes, nametable)
    $1B00 +---------+
          |   SAT   |  ($0080 bytes, sprite attribute table)
    $1B80 +---------+
          |         |  ($0480 bytes free)
    $2000 +---------+
          |   CGT   |  ($1800 bytes, colour table)
    $3800 +---------+
          |   SGT   |  ($0800 bytes, sprite generator table)
          +---------+
 *  --------------------------------------------------------- */

#define PNTADDRESS      0x1800
#define SATADDRESS      0x1B00
#define PGTADDRESS      0x0000
#define CGTADDRESS      0x2000
#define SGTADDRESS      0x3800

/* define VDPControlPort (SDCC z80 syntax) */
__sfr __at 0xBF VDPControlPort;
/* define VDPStatusPort */
__sfr __at 0xBF VDPStatusPort;
/* define VDPDataPort */
__sfr __at 0xBE VDPDataPort;

#ifndef TARGET_CV
/* define IOPort (SG joypad) */
__sfr __at 0xDC IOPortL;
/* define IOPort (SG joypad) */
__sfr __at 0xDD IOPortH;
#else
/* define IOPort (CV joypad) */
__sfr __at 0xE0 IOPortL;
/* define IOPort (CV joypad) */
__sfr __at 0xE2 IOPortH;
/* define IOPort (CV joypad) */
__sfr __at 0xC0 IOPortCTRLmode0;
/* define IOPort (CV joypad) */
__sfr __at 0x80 IOPortCTRLmode1;
#endif

#define HI(x)       ((x)>>8)
#define LO(x)       ((x)&0xFF)

#ifndef MAXSPRITES
#define MAXSPRITES      32
#endif

#define WAIT_VRAM             __asm nop \
                                    nop \
                                    nop __endasm

#ifndef TARGET_CV
#define VDP_SEMAPHORE_ON      __asm di __endasm
#define VDP_SEMAPHORE_OFF     __asm ei __endasm
#else
#define VDP_SEMAPHORE_ON      __asm rst 0x10 __endasm
#define VDP_SEMAPHORE_OFF     __asm rst 0x08 __endasm
#endif

/* VDP address/registers operations macro definitions */
#define SG_write_to_VDPRegister(VDPReg,value)  do{VDP_SEMAPHORE_ON; VDPControlPort=(value);     VDPControlPort=(VDPReg)|0x80;     VDP_SEMAPHORE_OFF;}while(0)
#define SG_set_address_VRAM(address)           do{VDP_SEMAPHORE_ON; VDPControlPort=LO(address); VDPControlPort=HI(address)|0x40;  VDP_SEMAPHORE_OFF;}while(0)
#define SG_set_address_VRAM_read(address)      do{VDP_SEMAPHORE_ON; VDPControlPort=LO(address); VDPControlPort=HI(address)&~0x40; VDP_SEMAPHORE_OFF;}while(0)

#ifdef TARGET_CV
extern _Bool CV_VDP_op_pending;
extern _Bool CV_NMI_srv_pending;
#endif
