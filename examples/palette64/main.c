#include "../SMSlib/SMSlib.h"

unsigned char color;

/* declare the function we'll use as a handler for the line IRQ */
void lineIRQhandler (void) {

	/* update the preselected color */
	SMS_setColor(color);

  /* prepare for next color update */
	color++;
	SMS_setNextSpriteColoratIndex(0);
}

void main (void) {

  /* configure the line IRQ handler */
	SMS_setLineInterruptHandler(&lineIRQhandler);

  /* configure the line counter to trigger the IRQ after line 2 and every 3rd line after that */
	SMS_setLineCounter(2);

  /* enable the line interrupt */
	SMS_enableLineInterrupt();

  /* we won't even turn the display on, as we're using the backdrop/border color */
	//SMS_displayOn();

	for (;;) {
		SMS_waitForVBlank();

		/* set sprite color #0 to black (the sprite color entry #0 is the default pick for the border and backdrop color) */
		color = 0;
		SMS_setNextSpriteColoratIndex(0);
		SMS_setColor(color);

		/* prepare for next color update */
		color++;
		SMS_setNextSpriteColoratIndex(0);   /* note: this needs to be the last command to the VDP, so that next command is the first issued in the line IRQ handler */
	}
}

SMS_EMBED_SEGA_ROM_HEADER(9999,0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE(1, 0, "slogra & sverx", "palette64", "a simple devkitSMS example");
