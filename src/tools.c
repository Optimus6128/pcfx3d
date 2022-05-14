#include "tools.h"
#include "mathutil.h"

uint16 RGB2YUV(int r, int g, int b)
{
	uint16 yuv;

	int Y = (19595 * r + 38469 * g + 7471 * b) >> 16;
	int U = ((32243 * (b - Y)) >> 16) + 128;
	int V = ((57475 * (r - Y)) >> 16) + 128;

	if (Y <   0) Y =   0;
	if (Y > 255) Y = 255;
	if (U <   0) U =   0;
	if (U > 255) U = 255;
	if (V <   0) V =   0;
	if (V > 255) V = 255;

	yuv = (uint16)((Y<<8) | ((U>>4)<<4) | (V>>4));

	return yuv;
}

void setPal(int c, int r, int g, int b, uint16* pal)
{
	CLAMP(r, 0, 31)
	CLAMP(g, 0, 31)
	CLAMP(b, 0, 31)

	pal[c] = (r << 10) | (g << 5) | b;
}

void setPalGradient(int c0, int c1, int r0, int g0, int b0, int r1, int g1, int b1, uint16* pal)
{
	int i;
	const int dc = (c1 - c0);
	const int dr = ((r1 - r0) << 16) / dc;
	const int dg = ((g1 - g0) << 16) / dc;
	const int db = ((b1 - b0) << 16) / dc;

	r0 <<= 16;
	g0 <<= 16;
	b0 <<= 16;

	for (i = c0; i <= c1; i++)
	{
		setPal(i, r0>>16, g0>>16, b0>>16, pal);

		r0 += dr;
		g0 += dg;
		b0 += db;
	}
}

void setPalGradientFromPrevIndex(int c0, int c1, int r1, int g1, int b1, uint16* pal)
{
	int r0, g0, b0;

	c0--;
	r0 = (pal[c0] >> 10) & 31;
	g0 = (pal[c0] >> 5) & 31;
	b0 = pal[c0] & 31;

	setPalGradient(c0, c1, r0, g0, b0, r1, g1, b1, pal);
}
