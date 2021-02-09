#include "SMSlib.h"

// All based on:
// https://www.smspower.org/Development/Paddle
//
// Also:
// https://www.smspower.org/Development/PeripheralPorts
//
#define DETECT_MIN	0x60
#define DETECT_MAX	0xA0

#define READ_TIMEOUT	32

__sfr __at 0xDC IOPortL;
__sfr __at 0xDD IOPortH;
__sfr __at 0x3F port3F;


void SMS_paddleInit(void)
{
	port3F = 0x55;
}

char SMS_detectPaddleA(void)
{
	unsigned char i,c;

	for (i=1, c=0; i; i++) {
		if (IOPortL & 0x20) {
			c++;
		}
	}
	if (c < DETECT_MIN) {
		return 0;
	}
	if (c > DETECT_MAX) {
		return 0;
	}

	return 1;
}

char SMS_detectPaddleB(void)
{
	unsigned char i,c;

	for (i=1, c=0; i; i++) {
		if (IOPortH & 0x08) {
			c++;
		}
	}
	if (c < DETECT_MIN) {
		return 0;
	}
	if (c > DETECT_MAX) {
		return 0;
	}

	return 1;

}

int SMS_paddleReadA(void)
{
	unsigned char val, tmp;
	unsigned int t = READ_TIMEOUT;

	/* First, synchronize by waiting until port A key 2 is high.
	 * Without this, the values occasionally glitches on real hardware.
	 *
	 * I think this happens when the low nibble gets read just
	 * when key 2 is just about to become high (or already rising).
	 *
	 * Since this solution works, I assume that the paddle updates
	 * the nibble first, and then sets key 2 to high. Or does
	 * both simultaneously.
	 */
	do {
		tmp = IOPortL;
		t--;
		if (!t) {
			return -1;
		}
	} while (!(tmp & 0x20));

	/* Now wait until key 2 is low, receive the low nibble. */
	do {
		tmp = IOPortL;
		t--;
		if (!t) {
			return -1;
		}
	} while (tmp & 0x20);

	val = tmp & 0xf;

	/* Now wait until key 2 is high to receive the high nibble. */
	do {
		tmp = IOPortL;
		t--;
		if (!t) {
			return -1;
		}
	} while (!(tmp & 0x20));

	val |= tmp << 4;

	return val;
}

int SMS_paddleReadB(void)
{
	unsigned char val, tmp;
	unsigned int t = READ_TIMEOUT;

	/* Sync */
	while (!(IOPortH & 0x08)) {
		t--;
		if (!t) {
			return -1;
		}
	}

	/* Wait for low nibble */
	while ((IOPortH & 0x08)) {
		t--;
		if (!t) {
			return -1;
		}
	}
	tmp = (IOPortL >> 6) | ((IOPortH & 3) << 2);
	val = tmp;

	/* Wait for high nibble */
	while (!(IOPortH & 0x08)) {
		t--;
		if (!t) {
			return -1;
		}
	}

	tmp = (IOPortL >> 6) | ((IOPortH & 3) << 2);
	val |= tmp << 4;

	return val;
}


