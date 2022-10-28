#include "main.h"
#include "script.h"
#include "fx_script.h"

#include "system.h"
#include "input.h"
#include "tinyfont.h"
#include "tools.h"
#include "fastking.h"

#include "engine_main.h"
#include "engine_soft.h"

int nframe = 0;

Screen *screen;

static void main3D()
{
	initEngine(screen);

	scriptInit(screen);

	for(;;) {
		scriptRun(screen, nframe);

		drawNumberDirect(16,24, getFps(), screen);

		//writeDisplay(screen);

		++nframe;
	}
}


static void mainFX()
{
	fxScriptInit(screen);

	for(;;) {
		fxScriptRun(screen, nframe);

		drawNumberDirect(16,192, getFps(), screen);

		writeDisplay(screen);

		++nframe;
	}
}

int main()
{
	screen = initDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, 8);

	initTimer();
	initInput();
	initTinyFonts();

	//main3D();
	mainFX();

	return 0;
}
