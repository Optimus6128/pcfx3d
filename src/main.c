#include "main.h"
#include "script.h"

#include "system.h"
#include "input.h"
#include "tinyfont.h"
#include "tools.h"
#include "fastking.h"

#include "engine_main.h"
#include "engine_soft.h"

int nframe = 0;

Screen *screen;

int main()
{
	screen = initDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, 8);

	initTimer();
	initInput();
	initTinyFonts();

	initEngine(screen);

	scriptInit(screen);

	for(;;) {
		scriptRun(screen, nframe);

		drawNumber(8,16, getFps(), screen);

		writeDisplay(screen);

		++nframe;
	}

	return 0;
}
