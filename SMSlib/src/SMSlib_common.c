/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#define HI(x)                 (((x)>>8))
#define LO(x)                 ((unsigned char)(x))

#define DISABLE_INTERRUPTS    __asm di __endasm
#define ENABLE_INTERRUPTS     __asm ei __endasm

#define WAIT_VRAM             __asm nop \
                                    nop \
                                    nop __endasm


// unsigned int PNTAddress=0x3800;
// unsigned int SATAddress=0x3F00;
// one day we might make these variables, but not for now
#define SMS_SATAddress    ((unsigned int)0x7F00)

#ifndef MAXSPRITES
#define MAXSPRITES            64
#endif

/* declare various I/O ports in SDCC z80 syntax */
/* define VDPControlPort */
__sfr __at 0xBF VDPControlPort;
/* define VDPStatusPort */
__sfr __at 0xBF VDPStatusPort;
/* define VDPDataPort */
__sfr __at 0xBE VDPDataPort;
/* define VDPVcounter */
__sfr __at 0x7E VDPVCounterPort;
/* define VDPHcounter */
__sfr __at 0x7F VDPHCounterPort;

inline void SMS_write_to_VDPRegister (unsigned char VDPReg, unsigned char value) {
  // INTERNAL FUNCTION
  unsigned char v=value;
  DISABLE_INTERRUPTS;
  VDPControlPort=v;
  VDPControlPort=VDPReg|0x80;
  ENABLE_INTERRUPTS;
}

/*
inline void SMS_set_address_CRAM (unsigned char address) {
  // INTERNAL FUNCTION
  unsigned char a=address;
  DISABLE_INTERRUPTS;
  VDPControlPort=a;
  VDPControlPort=0xC0;
  ENABLE_INTERRUPTS;
}

inline void SMS_set_address_VRAM (unsigned int address) {
  // INTERNAL FUNCTION
  unsigned int a=address;
  DISABLE_INTERRUPTS;
  VDPControlPort=LO(a);
  VDPControlPort=HI(a)|0x40;
  ENABLE_INTERRUPTS;
}
*/

inline void SMS_byte_to_VDP_data (unsigned char data) {
  // INTERNAL FUNCTION
  VDPDataPort=data;
}

inline void SMS_byte_array_to_VDP_data (const unsigned char *data, unsigned int size) {
  // INTERNAL FUNCTION
  do {
    VDPDataPort=*(data++);
  } while (--size);
}

inline void SMS_byte_brief_array_to_VDP_data (const unsigned char *data, unsigned char size) {
  // INTERNAL FUNCTION
  do {
    VDPDataPort=*(data++);
  } while (--size);
}

/*
inline void SMS_word_to_VDP_data (unsigned int data) {
  // INTERNAL FUNCTION
  VDPDataPort=LO(data);
  WAIT_VRAM;               // ensure we're not pushing data too fast
  VDPDataPort=HI(data);
}
*/

/*
inline void SMS_word_array_to_VDP_data (const unsigned int *data, unsigned int len) {
  INTERNAL FUNCTION
  while (len--!=0) {
    VDPDataPort=LO(*(data));
    VDPDataPort=HI(*(data));
    data++;
  }
}
*/
