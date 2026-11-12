/* **************************************************
   SGlib - C programming library for the SEGA SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   code: na_th_an, sverx
   ************************************************** */

#include "SGlib.h"
#include <stdbool.h>

extern volatile unsigned int KeysStatus, PreviousKeysStatus;

unsigned int SG_getKeysStatus (void) {
  return (KeysStatus);
}

unsigned int SG_getKeysPressed (void) {
  return (KeysStatus & (~PreviousKeysStatus));
}

unsigned int SG_getKeysHeld (void) {
  return (KeysStatus & PreviousKeysStatus);
}

unsigned int SG_getKeysReleased (void) {
  return ((~KeysStatus) & PreviousKeysStatus);
}

#ifndef TARGET_CV
extern volatile bool PauseRequested;

_Bool SG_queryPauseRequested (void) {
  return (PauseRequested);
}

void SG_resetPauseRequest (void) {
  PauseRequested=false;
}
#endif

#ifdef TARGET_CV
extern volatile unsigned char PadsStatus;

unsigned char SG_readCVNumPad (unsigned char which) {
  if (which==PORT_A_CV_NUMPAD)
    return (PadsStatus & 0x0F);
  else
    return (PadsStatus >> 4);
}
#endif
