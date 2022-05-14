#ifndef TOOLS_H
#define TOOLS_H

#include "types.h"

uint16 RGB2YUV(int r, int g, int b);

void setPal(int c, int r, int g, int b, uint16* pal);
void setPalGradient(int c0, int c1, int r0, int g0, int b0, int r1, int g1, int b1, uint16* pal);

#endif
