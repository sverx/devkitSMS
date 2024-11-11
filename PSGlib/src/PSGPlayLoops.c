/* **************************************************
   PSGlib - C programming library for the SEGA PSG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

#include "PSGlib.h"
#include "PSGlib_extern.h"

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
