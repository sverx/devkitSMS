/* ****************************************************
   aPLib - decompression library for the SMS/GG/SG-1000
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ****************************************************

  Z80/SMS aPLib decompression library
  version 1.2
  1/12/2008
  Maxim (http://www.smspower.org/maxim)
  based on code by Dan Weiss (Dwedit) - see readme.txt
  *** C wrapper by sverx - 2015-12-28 ***
  
  Usage:
  -->  aPLib_depack (void *dest, void *src)
  to decompress from ROM to RAM, or
  -->  aPLib_depack_VRAM (unsigned int dest, void *src)
  to decompress from ROM to VRAM (on Sega 8-bit systems).

  RAM usage: 5 bytes.
  The amount of stack space it uses will depend on the data.
*/

#ifndef DEPACK_VRAM_ONLY
void aPLib_depack (void *dest, void *src);
#endif
void aPLib_depack_VRAM (unsigned int dest, void *src);
