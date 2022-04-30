#include "script.h"

static void testPlasma(Screen *screen, int t)
{
	int x, y = screen->height;
	uint32 *dst32 = (uint32*)screen->data;
	const int width = screen->width;

	while(y!=0) {
		x = width >> 1;
		while(x!=0) {
			const uint32 c = ((x*x)^(y*y)) + t;
			*dst32++ = c;
			--x;
		}
		--y;
	}
}

void scriptInit(Screen *screen)
{
}

void scriptRun(Screen *screen, int t)
{
	testPlasma(screen, t);
}
