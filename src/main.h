#ifndef MAIN_H
#define MAIN_H

#include <eris/v810.h>
#include <eris/king.h>
#include <eris/tetsu.h>
#include <eris/romfont.h>
#include <eris/timer.h>
#include <eris/cd.h>
#include <eris/low/pad.h>
#include <eris/low/scsi.h>

#include "types.h"

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 256

#define SCREEN_SIZE_IN_PIXELS (SCREEN_WIDTH * SCREEN_HEIGHT)
#define CLAMP(value,min,max) if (value<min) value=min; if (value>max) value=max;


#endif
