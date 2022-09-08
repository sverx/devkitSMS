#include "SMSlib.h"

void main(void)
{
  /* Clear VRAM */
  SMS_VRAMmemsetW(0, 0x0000, 16384);

  /* Set background color to blue */
#ifdef TARGET_GG
  GG_setBGPaletteColor(0, RGBHTML(0x0000FF));
#else
  SMS_setBGPaletteColor(0, RGBHTML(0x0000FF));
#endif

  /* Turn on the display */
  SMS_displayOn();

  /* Do nothing */
  for(;;) { }
}

SMS_EMBED_SEGA_ROM_HEADER(9999,0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE(1,0,"raphnet","first test","A first test. If you can compile this, you're good to go");
