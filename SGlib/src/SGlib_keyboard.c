/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "SGlib.h"
#include <stdbool.h>

#define PPI_TEST_VALUE_1   0x55
#define PPI_TEST_VALUE_2   0xAA

/* define PPI_A */
__sfr __at 0xDC SC_PPI_A;
/* define PPI_B */
__sfr __at 0xDD SC_PPI_B;
/* define PPI_C */
__sfr __at 0xDE SC_PPI_C;
/* define SC_PPI_CTRL */
__sfr __at 0xDF SC_PPI_CTRL;

volatile unsigned int KBDKeysStatus, KBDPreviousKeysStatus;

_Bool SG_detectKeyboard (void) {
  bool kbd_detected=false;

  SC_PPI_CTRL=0x92;              // Init PPI (if present). PortC = Ouput, PortA + PortB = Input
  SC_PPI_C=PPI_TEST_VALUE_1;     // write test value 1
  if (SC_PPI_C==PPI_TEST_VALUE_1) {
    SC_PPI_C=PPI_TEST_VALUE_2;   // write test value 2
    if (SC_PPI_C==PPI_TEST_VALUE_2) {
      kbd_detected=true;
    }
  }

  SC_PPI_C=0x07;                 // Set row 7 (joypad) as default
  return (kbd_detected);
}

void SG_scanKeyboardJoypad (void) {
  unsigned char tmp;
  unsigned int status=0;        // all keys not pressed unless some detection happens

  SC_PPI_C=0x06;                // we'll read row 6
  tmp=SC_PPI_A;                 // read kbd data
  if (!(tmp & 0x20))            // check bit 5 (right arrow key)
    status|=PORT_A_KEY_RIGHT;
  if (!(tmp & 0x40))            // check bit 6 (up arrow key)
    status|=PORT_A_KEY_UP;
  if (!(SC_PPI_B & 0x04))       // read kbd data (on PPI_B) and check bit 2 ('CTRL' key)
    status|=PORT_B_KEY_2;

  SC_PPI_C=0x05;                // we'll read row 5
  if (!(SC_PPI_A & 0x20))       // read kbd data and check bit 5 (left arrow key)
    status|=PORT_A_KEY_LEFT;
  if (!(SC_PPI_B & 0x08))       // read kbd data (on PPI_B) and check bit 3 ('FUNC' key)
    status|=PORT_B_KEY_1;

  SC_PPI_C=0x04;                // we'll read row 4
  if (!(SC_PPI_A & 0x20))       // read kbd data and check bit 5 (down arrow key)
    status|=PORT_A_KEY_DOWN;

  SC_PPI_C=0x03;                // we'll read row 3
  if (!(SC_PPI_A & 0x10))       // read kbd data and check bit 4 (Ins/Del key)
    status|=PORT_A_KEY_2;

  SC_PPI_C=0x02;                // we'll read row 2
  tmp=SC_PPI_A;                 // read kbd data
  if (!(tmp & 0x10))            // read kbd data and check bit 4 (Home/Clr key)
    status|=PORT_A_KEY_1;
  if (!(tmp & 0x04))            // read kbd data and check bit 2 ('D' key)
    status|=PORT_B_KEY_RIGHT;

  SC_PPI_C=0x01;                // we'll read row 1
  tmp=SC_PPI_A;                 // read kbd data
  if (!(tmp & 0x02))            // read kbd data and check bit 1 ('W' key)
    status|=PORT_B_KEY_UP;
  if (!(tmp & 0x04))            // read kbd data and check bit 2 ('S' key)
    status|=PORT_B_KEY_DOWN;

  SC_PPI_C=0x00;                // we'll read row 0
  if (!(SC_PPI_A & 0x04))       // read kbd data and check bit 2 ('A' key)
    status|=PORT_B_KEY_LEFT;

  SC_PPI_C=0x07;                // set row 7 (joypad) as default

  KBDPreviousKeysStatus=KBDKeysStatus;
  KBDKeysStatus=status;
}

unsigned int SG_getKeyboardJoypadStatus (void) {
  return (KBDKeysStatus);
}

unsigned int SG_getKeyboardJoypadPressed (void) {
  return (KBDKeysStatus & (~KBDPreviousKeysStatus));
}

unsigned int SG_getKeyboardJoypadHeld (void) {
  return (KBDKeysStatus & KBDPreviousKeysStatus);
}

unsigned int SG_getKeyboardJoypadReleased (void) {
  return ((~KBDKeysStatus) & KBDPreviousKeysStatus);
}

unsigned char SG_getKeycodes (unsigned int *keys, unsigned char max_keys) {
    unsigned char count=0;

    for(unsigned char keyb_row=0; keyb_row < 8; keyb_row++) {
        unsigned int keyb_stat, row_no;

        SC_PPI_C = keyb_row;
        row_no = keyb_row << 12;
        keyb_stat=(~((SC_PPI_B << 8) | SC_PPI_A)) & 0x0FFF;
        for(unsigned int bit_mask=0x800; keyb_stat; bit_mask >>= 1) {
            if (keyb_stat & bit_mask) {
                if (count < max_keys)
                        keys[count++] = row_no + bit_mask;
                else
                    return count;
                keyb_stat -= bit_mask;
            }
        }
    }
    return count;
}
