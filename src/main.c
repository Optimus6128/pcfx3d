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
	screen = initDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, 16);

	initTimer();
	initInput();
	initTinyFonts();

	initEngine(screen);

	scriptInit(screen);

	for(;;) {
		scriptRun(screen, nframe);

		drawNumberDirect(16,24, getFps(), screen);

		//writeDisplay(screen);

		++nframe;
	}

	return 0;
}
