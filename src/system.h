#ifndef SYSTEM_H
#define SYSTEM_H

#include "types.h"

typedef struct Screen
{
	int width, height;
	int bpp;
	void *data;
} Screen;

extern int nframe;

Screen *initDisplay(int width, int height, int bpp);
void writeDisplay(Screen *screen);
void initTimer();
void vsync();
int getFps();

#endif
