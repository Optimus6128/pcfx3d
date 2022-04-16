#include "main.h"

#include "system.h"
#include "tinyfont.h"
#include "tools.h"

int nframe = 0;

Screen *screen;

static void testPlasma(int t)
{
	int x;
	int y = 200; //SCREEN_HEIGHT;
	unsigned int *dst32 = (unsigned int*)framebuffer;

	while(y!=0) {
		x = SCREEN_WIDTH >> 1;
		while(x!=0) {
			const unsigned int c = (x*x)^(y*y) + t;
			*dst32++ = c;
			--x;
		}
		--y;
	}
}

int main()
{
	screen = initDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, 16);

	initTimer();
	initTinyFonts();

	for(;;) {
		testPlasma(nframe++);

		eris_king_set_kram_write(SCREEN_SIZE_IN_PIXELS * 4, 1);

		king_kram_write_buffer(framebuffer, SCREEN_SIZE_IN_PIXELS);

		drawNumber(16,216, getFps());
	}

	return 0;
}
