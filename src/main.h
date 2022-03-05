#ifndef MAIN_H
#define MAIN_H

#include "types.h"

#define _8BPP

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 256

#define SCREEN_SIZE_IN_PIXELS (SCREEN_WIDTH * SCREEN_HEIGHT)
#define CLAMP(value,min,max) if (value<min) value=min; if (value>max) value=max;


#endif
