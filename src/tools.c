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
	CLAMP(r, 0, 255)
	CLAMP(g, 0, 255)
	CLAMP(b, 0, 255)

	pal[c] = RGB2YUV(r, g, b);
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

void myMemset(void *dst, uchar c, int count)
{
	const int count32 = count >> 2;
	int bytesLeft = count - (count32 << 2);
	int count32_unroll8 = count32 >> 3;
	int count32_left = count32 - (count32_unroll8 << 3);

	uint32 *dst32 = (uint32*)dst;
	uint8 *dst8;

	const uint32 c32 = (c << 24) | (c << 16) | (c << 8) | c;


	while(count32_unroll8-- > 0) {
		*dst32++ = c32;
		*dst32++ = c32;
		*dst32++ = c32;
		*dst32++ = c32;
		*dst32++ = c32;
		*dst32++ = c32;
		*dst32++ = c32;
		*dst32++ = c32;
	};
	while(count32_left-- > 0) {
		*dst32++ = c32;
	};

	dst8 = (uint8*)dst32;
	while(bytesLeft-- > 0) {
		*dst8++ = c;
	};
}

void myMemcpy(void *dst, void *src, int count)
{
	const int count32 = count >> 2;
	int bytesLeft = count - (count32 << 2);
	int count32_unroll4 = count32 >> 2;
	int count32_left = count32 - (count32_unroll4 << 2);

	uint32 *src32 = (uint32*)src;
	uint32 *dst32 = (uint32*)dst;
	uint8 *src8, *dst8;

	while(count32_unroll4-- > 0) {
		*dst32++ = *src32++;
		*dst32++ = *src32++;
		*dst32++ = *src32++;
		*dst32++ = *src32++;
	};
	while(count32_left-- > 0) {
		*dst32++ = *src32++;
	};

	src8 = (uint8*)src32;
	dst8 = (uint8*)dst32;
	while(bytesLeft-- > 0) {
		*dst8++ = *src8++;
	};
}
