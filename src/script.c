#include "script.h"
#include "tools.h"

static void testPlasma16(Screen *screen, int t)
{
	int x, y = screen->height;
	uint16 *dst16 = (uint16*)screen->data;
	const int width = screen->width;

	while(y!=0) {
		x = width;
		while(x!=0) {
			uint16 c = ((x*x)^(y*y)) + t;
			if (y==128) c = RGB2YUV(255,223,191);
			*dst16++ = c;
			--x;
		}
		--y;
	}
}

static void testPlasma8(Screen *screen, int t)
{
	int x, y = screen->height;
	uint8 *dst8 = (uint8*)screen->data;
	const int width = screen->width;

	while(y!=0) {
		x = width;
		while(x!=0) {
			uint8 c = (uint8)((x^y) + t);
			if (y==128) c = 127;
			*dst8++ = c;
			--x;
		}
		--y;
	}
}

void scriptInit(Screen *screen)
{
	if (screen->bpp==8) {
		int i;
		for(i = 0; i < 256; i++) {
			eris_tetsu_set_palette(i, RGB2YUV(i, i>>1, i>>2));
		}
	}
}

void scriptRun(Screen *screen, int t)
{
	if (screen->bpp==8) {
		testPlasma8(screen, t);
	} else {
		testPlasma16(screen, t);		
	}
}
