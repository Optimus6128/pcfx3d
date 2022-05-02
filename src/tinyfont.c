#include <stdio.h>
#include <stdlib.h>

#include "tinyfont.h"
#include "main.h"

/*
1110 0010 1110 1110 1010 1110 1110 1110 1110 1110
1010 0010 0010 0010 1010 1000 1000 0010 1010 1010
1010 0010 1110 0110 1110 1110 1110 0010 1110 1110
1010 0010 1000 0010 0010 0010 1010 0010 1010 0010
1110 0010 1110 1110 0010 1110 1110 0010 1110 1110
*/

static const uint8 miniDecimalData[] = { 0xE2, 0xEE, 0xAE, 0xEE, 0xEE,
0xA2, 0x22, 0xA8, 0x82, 0xAA,
0xA2, 0xE6, 0xEE, 0xE2, 0xEE,
0xA2, 0x82, 0x22, 0xA2, 0xA2,
0xE2, 0xEE, 0x2E, 0xE2, 0xEE };

static uint16 miniDecimalFonts[TINY_FONT_NUM_PIXELS];

void initTinyFonts()
{
	static uint8 miniDecimalPixels[TINY_FONT_NUM_PIXELS];
	int i,j,k=0;
	int x,y,n;

	for (i = 0; i < TINY_FONT_NUM_PIXELS / 8; i++) {
		uint8 d = miniDecimalData[i];
		for (j = 0; j < 8; j++) {
			uint8 c = (d & 0x80) >> 7;
			miniDecimalPixels[k++] = c;
			d <<= 1;
		}
	}

	i = 0;
	for (n = 0; n < TINY_FONTS_NUM; n++) {
		for (y = 0; y < TINY_FONT_HEIGHT; y++) {
			for (x = 0; x < TINY_FONT_WIDTH; x++) {
				miniDecimalFonts[i++] = miniDecimalPixels[n * TINY_FONT_WIDTH + x + y * TINY_FONT_WIDTH * TINY_FONTS_NUM] * 0x8080;
			}
		}
	}
}

static void drawFont16(int posX, int posY, uint8 decimal, Screen *screen)
{
	int x, y;
	const int width = screen->width;
    uint16 *fontData = &miniDecimalFonts[decimal * TINY_FONT_WIDTH * TINY_FONT_HEIGHT];
	uint16 *dst = (uint16*)screen->data + posY * width + posX;

    for (y = 0; y<TINY_FONT_HEIGHT; y++) {
		for (x = 0; x<TINY_FONT_WIDTH; ++x) {
			*(dst+x) = *fontData++;
		}
		dst += width;
    }
}

static void drawFont8(int posX, int posY, uint8 decimal, Screen *screen)
{
	int x, y;
	const int width = screen->width;
    uint16 *fontData = &miniDecimalFonts[decimal * TINY_FONT_WIDTH * TINY_FONT_HEIGHT];
	uint8 *dst = (uint8*)screen->data + posY * width + posX;

    for (y = 0; y<TINY_FONT_HEIGHT; y++) {
		for (x = 0; x<TINY_FONT_WIDTH; ++x) {
			*(dst+x) = (uint8)*fontData++;
		}
		dst += width;
    }
}

void drawNumber(int posX, int posY, int number, Screen *screen)
{
    static char buffer[10];
	int i = 0;
	sprintf(buffer, "%d", number);

	while(i < 10 && buffer[i] != 0) {
		const int px = posX + i * TINY_FONT_WIDTH; 
		const uint8 c = buffer[i++] - 48;
		if (screen->bpp == 8) {
			drawFont8(px, posY, c, screen);
		} else {
			drawFont16(px, posY, c, screen);
		}
    }
}
