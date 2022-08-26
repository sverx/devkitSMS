#include "SMSlib.h"

void main(void)
{
  /* Clear VRAM */
  SMS_VRAMmemsetW(0, 0x0000, 16384);

  /* load a standard font character set into tiles 0-95,
   * set BG palette to B/W and turn on the screen */
  SMS_autoSetUpTextRenderer();

  /* Set the target of the next background write */
  SMS_setNextTileatXY(8,11);

  /* Write text to the background */
  SMS_printstring("Hello, World!");

  /* Turn on the display */
  SMS_displayOn();

  /* Do nothing */
  for(;;) { }
}

SMS_EMBED_SEGA_ROM_HEADER(9999,0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE(1,0,"raphnet","basic example","A simple example");
