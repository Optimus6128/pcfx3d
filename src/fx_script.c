#include "fx_script.h"
#include "tools.h"
#include "input.h"

#include "mathutil.h"
#include "bitstest.h"
#include "tinyfont.h"

#include "tools.h"

#include "system.h"
#include "fx_julia.h"


void fxScriptInit(Screen *screen)
{
	if (screen->bpp==8) {
		int i;
		for(i = 0; i < 256; i++) {
			const int c = i;
			eris_tetsu_set_palette(i, RGB2YUV(c, c, c));
		}
	}
}

void fxScriptRun(Screen *screen, int t)
{
	fxJuliaRun(screen, t);
}
