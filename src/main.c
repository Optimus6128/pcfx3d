#include "main.h"
#include "script.h"

#include "system.h"
#include "tinyfont.h"
#include "tools.h"
#include "fastking.h"

int nframe = 0;

Screen *screen;

int main()
{
	screen = initDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, 16);

	initTimer();
	initTinyFonts();

	scriptInit(screen);

	for(;;) {
		scriptRun(screen, nframe);

		eris_king_set_kram_write(SCREEN_SIZE_IN_PIXELS * 4, 1);

		king_kram_write_buffer(screen->data, SCREEN_SIZE_IN_PIXELS);

		drawNumber(16,216, getFps());

		++nframe;
	}

	return 0;
}
