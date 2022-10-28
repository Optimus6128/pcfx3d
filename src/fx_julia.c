#include <math.h>
#include "mathutil.h"
#include "system.h"

#define FP_SHR 12
#define FP_MUL (1 << FP_SHR)
#define ESCAPE ((4 * FP_MUL) << FP_SHR)

#define FRAC_ITER_BLOCK \
						x1 = ((x0 * x0 - y0 * y0)>>FP_SHR) + xp; \
						y1 = ((x0 * y0)>>(FP_SHR-1)) + yp; \
						mj2 = x1 * x1 + y1 * y1; \
						x0 = x1; y0 = y1; c+=16;

#define FRAC_ITER_ESCAPE1 if (mj2 > ESCAPE) goto end1;
#define FRAC_ITER_ESCAPE2 if (mj2 > ESCAPE) goto end2;
#define FRAC_ITER_ESCAPE3 if (mj2 > ESCAPE) goto end3;
#define FRAC_ITER_ESCAPE4 if (mj2 > ESCAPE) goto end4;

#define FRAC_ITER_TIMES1_2 FRAC_ITER_BLOCK FRAC_ITER_ESCAPE1 FRAC_ITER_BLOCK FRAC_ITER_ESCAPE1
#define FRAC_ITER_TIMES1_4 FRAC_ITER_TIMES1_2 FRAC_ITER_TIMES1_2
#define FRAC_ITER_TIMES1_8 FRAC_ITER_TIMES1_4 FRAC_ITER_TIMES1_4
#define FRAC_ITER_TIMES1_16 FRAC_ITER_TIMES1_8 FRAC_ITER_TIMES1_8

#define FRAC_ITER_TIMES2_2 FRAC_ITER_BLOCK FRAC_ITER_ESCAPE2 FRAC_ITER_BLOCK FRAC_ITER_ESCAPE2
#define FRAC_ITER_TIMES2_4 FRAC_ITER_TIMES2_2 FRAC_ITER_TIMES2_2
#define FRAC_ITER_TIMES2_8 FRAC_ITER_TIMES2_4 FRAC_ITER_TIMES2_4
#define FRAC_ITER_TIMES2_16 FRAC_ITER_TIMES2_8 FRAC_ITER_TIMES2_8

#define FRAC_ITER_TIMES3_2 FRAC_ITER_BLOCK FRAC_ITER_ESCAPE3 FRAC_ITER_BLOCK FRAC_ITER_ESCAPE3
#define FRAC_ITER_TIMES3_4 FRAC_ITER_TIMES3_2 FRAC_ITER_TIMES3_2
#define FRAC_ITER_TIMES3_8 FRAC_ITER_TIMES3_4 FRAC_ITER_TIMES3_4
#define FRAC_ITER_TIMES3_16 FRAC_ITER_TIMES3_8 FRAC_ITER_TIMES3_8

#define FRAC_ITER_TIMES4_2 FRAC_ITER_BLOCK FRAC_ITER_ESCAPE4 FRAC_ITER_BLOCK FRAC_ITER_ESCAPE4
#define FRAC_ITER_TIMES4_4 FRAC_ITER_TIMES4_2 FRAC_ITER_TIMES4_2
#define FRAC_ITER_TIMES4_8 FRAC_ITER_TIMES4_4 FRAC_ITER_TIMES4_4
#define FRAC_ITER_TIMES4_16 FRAC_ITER_TIMES4_8 FRAC_ITER_TIMES4_8


void fxJuliaRun(Screen *screen, int t)
{
	int x, y;
	const int screenWidth = screen->width;
	const int screenHeight = screen->height;

	const int xp = SinF16(t<<19) >> (16 - FP_SHR);
	const int yp = CosF16(t<<19) >> (16 - FP_SHR);

	const int di = (int)(FP_MUL / 64);

	unsigned int *vram = (unsigned int*)screen->data;
	unsigned int *half = vram + (screenWidth/4) * screenHeight-1;

	int yk = -di * -screenHeight/2;
	int xl = di * -screenWidth/2 + (di / 2);

	for (y=0; y<screenHeight/2; y++)
	{
		int xk = xl;
		for (x=0; x<screenWidth/4; x++)
		{
			unsigned char c;
			int c0,c1,c2,c3;
			int x0,y0;
			int x1,y1,mj2;

			c = 240;
			x0 = xk; y0 = yk;
			FRAC_ITER_TIMES1_16
			end1:
			c0 = c;
			xk+=di;

			c = 240;
			x0 = xk; y0 = yk;
			FRAC_ITER_TIMES2_16
			end2:
			c1 = c;
			xk+=di;

			c = 240;
			x0 = xk; y0 = yk;
			FRAC_ITER_TIMES3_16
			end3:
			c2 = c;
			xk+=di;

			c = 240;
			x0 = xk; y0 = yk;
			FRAC_ITER_TIMES4_16
			end4:
			c3 = c;
			xk+=di;

			*(vram + x) = (c3 << 24) | (c2 << 16) | (c1 << 8) | c0;
			*(half - x) = (c0 << 24) | (c1 << 16) | (c2 << 8) | c3;
		}
		vram+= (screenWidth/4);
		half-= (screenWidth/4);
        yk-=di;
	}
}
