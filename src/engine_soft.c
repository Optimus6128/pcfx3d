#include "main.h"
#include "tools.h"

#include "engine_main.h"
#include "engine_mesh.h"
#include "engine_texture.h"
#include "engine_soft.h"

#include "mathutil.h"
#include "tools.h"

#define DIV_TAB_SIZE 4096
#define DIV_TAB_SHIFT 16


typedef struct Edge
{
	int x;
	int c;
	int u,v;
}Edge;

typedef struct RectArea
{
	int posX;
	int posY;
	int width;
	int height;
}RectArea;

typedef struct SoftBuffer
{
	int width;
	int height;
	int posX;
	int posY;
	uint8 *data;
}SoftBuffer;

static int renderSoftMethod = RENDER_SOFT_METHOD_GOURAUD;

SoftBuffer softBuffer;

static Edge *leftEdge;
static Edge *rightEdge;

static int32 divTab[DIV_TAB_SIZE];

static uint16 *lineColorShades[4] = { NULL, NULL, NULL, NULL };
static uint16 *gouraudColorShades;

static uint16 *activeGradient = NULL;
static Texture *activeTexture = NULL;

static int minX, maxX, minY, maxY;
static int softBufferMaxSize;

#define LN_BASE 8
#define LN_AND ((1 << LN_BASE) - 1)


static void(*fillEdges)(int y0, int y1);
static void(*prepareEdgeList) (ScreenElement *e0, ScreenElement *e1, Screen *screen);


static void bindGradient(uint16 *gradient)
{
	activeGradient = gradient;
}

static void bindTexture(Texture *texture)
{
	activeTexture = texture;
}

static void bindMeshPolyData(Mesh *ms, int numPoly)
{
	if (renderSoftMethod <= RENDER_SOFT_METHOD_GOURAUD) {
		bindGradient(lineColorShades[numPoly & 3]);
	} else {
		bindTexture(&ms->tex[ms->poly[numPoly].textureId]);
	}
}

static uint16 *crateColorShades(int r, int g, int b, int numShades) {
	uint16 *colorShades = (uint16*)malloc(sizeof(uint16) * numShades);

	setPalGradient(0, numShades-1, 0,0,0, r,g,b, colorShades);

	return colorShades;
}

static void initDivs()
{
    int i, ii;
    for (i=0; i<DIV_TAB_SIZE; ++i) {
        ii = i - DIV_TAB_SIZE / 2;
        if (ii==0) ++ii;

        divTab[i] = (1 << DIV_TAB_SHIFT) / ii;
    }
}

static void drawAntialiasedLine(ScreenElement *e1, ScreenElement *e2, Screen *screen)
{
	int x1 = e1->x;
	int y1 = e1->y;
	int x2 = e2->x;
	int y2 = e2->y;

	int dx, dy, l;
	int x00, y00;
	int vramofs;

	int x, y;
	int frac, shade;

	int temp;
    int chdx, chdy;

	uint16 *vram = (uint16*)softBuffer.data;
	const int screenWidth = softBuffer.width;
	const int screenHeight = softBuffer.height;

    // ==== Clipping ====

    int outcode1 = 0, outcode2 = 0;

    if (y1 < 1) outcode1 |= 0x0001;
        else if (y1 > screenHeight-2) outcode1 |= 0x0010;
    if (x1 < 1) outcode1 |= 0x0100;
        else if (x1 > screenWidth-2) outcode1 |= 0x1000;

    if (y2 < 1) outcode2 |= 0x0001;
        else if (y2 > screenHeight-2) outcode2 |= 0x0010;
    if (x2 < 1) outcode2 |= 0x0100;
        else if (x2 > screenWidth-2) outcode2 |= 0x1000;

    if ((outcode1 & outcode2)!=0) return;

    //if ((outcode1 | outcode2)!=0) return; // normally, should check for possible clip
	//I will do lame method now

    // ==================

	dx = x2 - x1;
	dy = y2 - y1;

	if (dx==0 && dy==0) return;

    chdx = dx;
	chdy = dy;
    if (dx<0) chdx = -dx;
    if (dy<0) chdy = -dy;

	if (chdy < chdx) {
		if (x1 > x2) {
			temp = x1; x1 = x2; x2 = temp;
			y1 = y2;
		}

		if (dx==0) return;
        l = (dy << LN_BASE) / dx;
        y00 = y1 << LN_BASE;
		for (x=x1; x<x2; x++) {
			const int yp = y00 >> LN_BASE;

			if (x >= 0 && x < screenWidth && yp >=0 && yp < screenHeight - 1) {
				vramofs = yp*screenWidth + x;
				frac = y00 & LN_AND;

				shade = (LN_AND - frac) >> 4;
				*(vram + vramofs) |= activeGradient[shade];

				shade = frac >> 4;
				*(vram + vramofs+screenWidth) |= activeGradient[shade];
			}
            y00+=l;
		}
	}
	else {
		if (y1 > y2) {
			temp = y1; y1 = y2; y2 = temp;
			x1 = x2;
		}

		if (dy==0) return;
        l = (dx << LN_BASE) / dy;
        x00 = x1 << LN_BASE;

		for (y=y1; y<y2; y++) {
			const int xp = x00 >> LN_BASE;

			if (y >= 0 && y < screenHeight && xp >=0 && xp < screenWidth - 1) {
				vramofs = y*screenWidth + xp;
				frac = x00 & LN_AND;

				shade = (LN_AND - frac) >> 4;
				*(vram + vramofs) |= activeGradient[shade];

				shade = frac >> 4;
				*(vram + vramofs + 1) |= activeGradient[shade];
			}
            x00+=l;
		}
	}
}

static void prepareEdgeListGouraud(ScreenElement *e0, ScreenElement *e1, Screen *screen)
{
	Edge *edgeListToWrite;
	ScreenElement *eTemp;
	const int screenHeight = screen->height;

	// Assumes CCW
	if (e0->y < e1->y) {
		edgeListToWrite = leftEdge;
	}
	else {
		edgeListToWrite = rightEdge;

		eTemp = e0;
		e0 = e1;
		e1 = eTemp;
	}

    {
        const int x0 = e0->x; int y0 = e0->y; int c0 = e0->c;
        const int x1 = e1->x; int y1 = e1->y; int c1 = e1->c;

        int dy = y1 - y0 + 1;
		const int repDiv = divTab[dy + DIV_TAB_SIZE / 2];
        const int dx = ((x1 - x0) * repDiv) >> (DIV_TAB_SHIFT - FP_BASE);
		const int dc = ((c1 - c0) * repDiv) >> (DIV_TAB_SHIFT - FP_BASE);

        int fx = INT_TO_FIXED(x0, FP_BASE);
		int fc = INT_TO_FIXED(c0, FP_BASE);

		if (y0 < 0) {
			fx += -y0 * dx;
			fc += -y0 * dc;
			dy += y0;
			y0 = 0;
		}
		if (y1 > screenHeight-1) {
			dy -= (y1 - screenHeight-1);
		}

        edgeListToWrite = &edgeListToWrite[y0];
        do {
			int x = FIXED_TO_INT(fx, FP_BASE);
			edgeListToWrite->x = x;
			edgeListToWrite->c = fc;
            ++edgeListToWrite;
            fx += dx;
			fc += dc;
		} while(--dy > 0);
    }
}

static void prepareEdgeListEnvmap(ScreenElement *e0, ScreenElement *e1, Screen *screen)
{
	Edge *edgeListToWrite;
	ScreenElement *eTemp;
	const int screenHeight = screen->height;

	// Assumes CCW
	if (e0->y < e1->y) {
		edgeListToWrite = leftEdge;
	}
	else {
		edgeListToWrite = rightEdge;

		eTemp = e0;
		e0 = e1;
		e1 = eTemp;
	}

    {
        const int x0 = e0->x; int y0 = e0->y; int u0 = e0->u; int v0 = e0->v;
        const int x1 = e1->x; int y1 = e1->y; int u1 = e1->u; int v1 = e1->v;

        int dy = y1 - y0 + 1;
		const int repDiv = divTab[dy + DIV_TAB_SIZE / 2];
        const int dx = ((x1 - x0) * repDiv) >> (DIV_TAB_SHIFT - FP_BASE);
		const int du = ((u1 - u0) * repDiv) >> (DIV_TAB_SHIFT - FP_BASE);
		const int dv = ((v1 - v0) * repDiv) >> (DIV_TAB_SHIFT - FP_BASE);

        int fx = INT_TO_FIXED(x0, FP_BASE);
		int fu = INT_TO_FIXED(u0, FP_BASE);
		int fv = INT_TO_FIXED(v0, FP_BASE);

		if (y0 < 0) {
			fx += -y0 * dx;
			fu += -y0 * du;
			fv += -y0 * dv;
			dy += y0;
			y0 = 0;
		}
		if (y1 > screenHeight-1) {
			dy -= (y1 - screenHeight-1);
		}

        edgeListToWrite = &edgeListToWrite[y0];
        do {
			int x = FIXED_TO_INT(fx, FP_BASE);
			edgeListToWrite->x = x;
			edgeListToWrite->u = fu;
			edgeListToWrite->v = fv;
            ++edgeListToWrite;
            fx += dx;
			fu += du;
			fv += dv;
		} while(--dy > 0);
    }
}

static void prepareEdgeListGouraudEnvmap(ScreenElement *e0, ScreenElement *e1, Screen *screen)
{
	Edge *edgeListToWrite;
	ScreenElement *eTemp;
	const int screenHeight = screen->height;

	// Assumes CCW
	if (e0->y < e1->y) {
		edgeListToWrite = leftEdge;
	}
	else {
		edgeListToWrite = rightEdge;

		eTemp = e0;
		e0 = e1;
		e1 = eTemp;
	}

    {
        const int x0 = e0->x; int y0 = e0->y; int c0 = e0->c; int u0 = e0->u; int v0 = e0->v;
        const int x1 = e1->x; int y1 = e1->y; int c1 = e1->c; int u1 = e1->u; int v1 = e1->v;

        int dy = y1 - y0 + 1;
		const int repDiv = divTab[dy + DIV_TAB_SIZE / 2];
        const int dx = ((x1 - x0) * repDiv) >> (DIV_TAB_SHIFT - FP_BASE);
		const int dc = ((c1 - c0) * repDiv) >> (DIV_TAB_SHIFT - FP_BASE);
		const int du = ((u1 - u0) * repDiv) >> (DIV_TAB_SHIFT - FP_BASE);
		const int dv = ((v1 - v0) * repDiv) >> (DIV_TAB_SHIFT - FP_BASE);

        int fx = INT_TO_FIXED(x0, FP_BASE);
		int fc = INT_TO_FIXED(c0, FP_BASE);
		int fu = INT_TO_FIXED(u0, FP_BASE);
		int fv = INT_TO_FIXED(v0, FP_BASE);

		if (y0 < 0) {
			fx += -y0 * dx;
			fc += -y0 * dc;
			fu += -y0 * du;
			fv += -y0 * dv;
			dy += y0;
			y0 = 0;
		}
		if (y1 > screenHeight-1) {
			dy -= (y1 - screenHeight-1);
		}

        edgeListToWrite = &edgeListToWrite[y0];
        do {
			int x = FIXED_TO_INT(fx, FP_BASE);
			edgeListToWrite->x = x;
			edgeListToWrite->c = fc;
			edgeListToWrite->u = fu;
			edgeListToWrite->v = fv;
            ++edgeListToWrite;
            fx += dx;
			fc += dc;
			fu += du;
			fv += dv;
		} while(--dy > 0);
    }
}

static void fillGouraudEdges8(int y0, int y1)
{
	const int stride = softBuffer.width;
	uint8 *dst8 = (uint8*)softBuffer.data + y0 * stride;

	int count = y1 - y0 + 1;
	Edge *le = &leftEdge[y0];
	Edge *re = &rightEdge[y0];
	do {
		const int xl = le->x;
		const int cl = le->c;
		const int cr = re->c;
		int length = re->x - xl;
		uint8 *dst = dst8 + xl;
		uint32 *dst32;

		const int repDiv = divTab[length + DIV_TAB_SIZE / 2];
		const int dc = ((cr - cl) * repDiv) >> DIV_TAB_SHIFT;
		int fc = cl;

		int xlp = xl & 3;
		if (xlp) {
			xlp = 4 - xlp;
			while (xlp-- > 0 && length-- > 0) {
				int c = FIXED_TO_INT(fc, FP_BASE);
				fc += dc;

				*dst++ = c;
			}
		}

		dst32 = (uint32*)dst;
		while(length >= 4) {
			int c0,c1,c2,c3;

			c0 = FIXED_TO_INT(fc, FP_BASE);
			fc += dc;
			c1 = FIXED_TO_INT(fc, FP_BASE);
			fc += dc;
			c2 = FIXED_TO_INT(fc, FP_BASE);
			fc += dc;
			c3 = FIXED_TO_INT(fc, FP_BASE);
			fc += dc;

			#ifdef BIG_ENDIAN
				*dst32++ = (c0 << 24) | (c1 << 16) | (c2 << 8) | c3;
			#else
				*dst32++ = (c3 << 24) | (c2 << 16) | (c1 << 8) | c0;
			#endif
			length-=4;
		};

		dst = (uint8*)dst32;
		while (length-- > 0) {
			int c = FIXED_TO_INT(fc, FP_BASE);
			fc += dc;

			*dst++ = c;
		}

		++le;
		++re;
		dst8 += stride;
	} while(--count > 0);
}

static void fillGouraudEdges16(int y0, int y1)
{
	const int stride = softBuffer.width;
	uint16 *dst16 = (uint16*)softBuffer.data + y0 * stride;

	int count = y1 - y0 + 1;
	Edge *le = &leftEdge[y0];
	Edge *re = &rightEdge[y0];
	do {
		const int xl = le->x;
		const int cl = le->c;
		const int cr = re->c;
		int length = re->x - xl;
		uint16 *dst = dst16 + xl;
		uint32 *dst32;

		const int repDiv = divTab[length + DIV_TAB_SIZE / 2];
		const int dc = ((cr - cl) * repDiv) >> DIV_TAB_SHIFT;
		int fc = cl;

		if (length>0){
			if (xl & 1) {
				int c = FIXED_TO_INT(fc, FP_BASE);
				fc += dc;

				*dst++ = activeGradient[c];
				length--;
			}

			dst32 = (uint32*)dst;
			while(length >= 2) {
				int c0, c1;

				c0 = FIXED_TO_INT(fc, FP_BASE);
				fc += dc;
				c1 = FIXED_TO_INT(fc, FP_BASE);
				fc += dc;

				#ifdef BIG_ENDIAN
					*dst32++ = (activeGradient[c0] << 16) | activeGradient[c1];
				#else
					*dst32++ = (activeGradient[c1] << 16) | activeGradient[c0];
				#endif
				length -= 2;
			};

			dst = (uint16*)dst32;
			if (length & 1) {
				int c = FIXED_TO_INT(fc, FP_BASE);
				fc += dc;

				*dst++ = activeGradient[c];
			}
		}

		++le;
		++re;
		dst16 += stride;
	} while(--count > 0);
}

static void fillEnvmapEdges8(int y0, int y1)
{
	const int stride = softBuffer.width;
	uint8 *dst8 = (uint8*)softBuffer.data + y0 * stride;

	int count = y1 - y0 + 1;
	Edge *le = &leftEdge[y0];
	Edge *re = &rightEdge[y0];

	const int texHeightShift = activeTexture->hShift;
	uint8* texData = (uint8*)activeTexture->bitmap;

	do {
		const int xl = le->x;
		const int ul = le->u;
		const int ur = re->u;
		const int vl = le->v;
		const int vr = re->v;
		int length = re->x - xl;

		uint8 *dst = dst8 + xl;
		uint32 *dst32;

		const int repDiv = divTab[length + DIV_TAB_SIZE / 2];
		const int du = ((ur - ul) * repDiv) >> DIV_TAB_SHIFT;
		const int dv = ((vr - vl) * repDiv) >> DIV_TAB_SHIFT;
		int fu = ul;
		int fv = vl;

		int xlp = xl & 3;
		if (xlp) {
			while (xlp++ < 4 && length-- > 0) {
				*dst++ = texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)];
				fu += du;
				fv += dv;
			}
		}

		dst32 = (uint32*)dst;
		while(length >= 4) {
			int c0,c1,c2,c3;

			c0 = texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)];
			fu += du;
			fv += dv;

			c1 = texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)];
			fu += du;
			fv += dv;

			c2 = texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)];
			fu += du;
			fv += dv;

			c3 = texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)];
			fu += du;
			fv += dv;

			#ifdef BIG_ENDIAN
				*dst32++ = (c0 << 24) | (c1 << 16) | (c2 << 8) | c3;
			#else
				*dst32++ = (c3 << 24) | (c2 << 16) | (c1 << 8) | c0;
			#endif
			length-=4;
		};

		dst = (uint8*)dst32;
		while (length-- > 0) {
			*dst++ = texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)];
			fu += du;
			fv += dv;
		}

		++le;
		++re;
		dst8 += stride;
	} while(--count > 0);
}

static void fillEnvmapEdges16(int y0, int y1)
{
	const int stride = softBuffer.width;
	uint16 *dst16 = (uint16*)softBuffer.data + y0 * stride;

	int count = y1 - y0 + 1;
	Edge *le = &leftEdge[y0];
	Edge *re = &rightEdge[y0];

	const int texHeightShift = activeTexture->hShift;
	uint16* texData = (uint16*)activeTexture->bitmap;

	do {
		const int xl = le->x;
		const int ul = le->u;
		const int ur = re->u;
		const int vl = le->v;
		const int vr = re->v;
		int length = re->x - xl;

		if (length>0){
			uint16 *dst = dst16 + xl;
			uint32 *dst32;

			const int repDiv = divTab[length + DIV_TAB_SIZE / 2];
			const int du = ((ur - ul) * repDiv) >> DIV_TAB_SHIFT;
			const int dv = ((vr - vl) * repDiv) >> DIV_TAB_SHIFT;
			int fu = ul;
			int fv = vl;
			
			if (xl & 1) {
				*dst++ = texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)];
				fu += du;
				fv += dv;

				length--;
			}

			dst32 = (uint32*)dst;
			while(length >= 2) {
				int c0, c1;

				c0 = texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)];
				fu += du;
				fv += dv;

				c1 = texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)];
				fu += du;
				fv += dv;

				#ifdef BIG_ENDIAN
					*dst32++ = (c0 << 16) | c1;
				#else
					*dst32++ = (c1 << 16) | c0;
				#endif
				length -= 2;
			};

			dst = (uint16*)dst32;
			if (length != 0) {
				*dst++ = texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)];
				fu += du;
				fv += dv;
			}
		}

		++le;
		++re;
		dst16 += stride;
	} while(--count > 0);
}

static void fillGouraudEnvmapEdges8(int y0, int y1)
{
	const int stride = softBuffer.width;
	uint8 *dst8 = (uint8*)softBuffer.data + y0 * stride;

	int count = y1 - y0 + 1;
	Edge *le = &leftEdge[y0];
	Edge *re = &rightEdge[y0];

	const int texHeightShift = activeTexture->hShift;
	uint8* texData = (uint8*)activeTexture->bitmap;

	do {
		const int xl = le->x;
		const int cl = le->c;
		const int cr = re->c;
		const int ul = le->u;
		const int ur = re->u;
		const int vl = le->v;
		const int vr = re->v;
		int length = re->x - xl;
		uint8 *dst = dst8 + xl;
		uint32 *dst32;

		const int repDiv = divTab[length + DIV_TAB_SIZE / 2];
		const int dc = ((cr - cl) * repDiv) >> DIV_TAB_SHIFT;
		const int du = ((ur - ul) * repDiv) >> DIV_TAB_SHIFT;
		const int dv = ((vr - vl) * repDiv) >> DIV_TAB_SHIFT;
		int fc = cl;
		int fu = ul;
		int fv = vl;

		int c;

		int xlp = xl & 3;
		if (xlp) {
			while (xlp++ < 4 && length-- > 0) {
				c = (texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)] * FIXED_TO_INT(fc, FP_BASE)) >> COLOR_ENVMAP_SHR;
				*dst++ = c;

				fc += dc;
				fu += du;
				fv += dv;
			}
		}

		dst32 = (uint32*)dst;
		while(length >= 4) {
			int c0,c1,c2,c3;

			c0 = (texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)] * FIXED_TO_INT(fc, FP_BASE)) >> COLOR_ENVMAP_SHR;
			fc += dc;
			fu += du;
			fv += dv;

			c1 = (texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)] * FIXED_TO_INT(fc, FP_BASE)) >> COLOR_ENVMAP_SHR;
			fc += dc;
			fu += du;
			fv += dv;

			c2 = (texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)] * FIXED_TO_INT(fc, FP_BASE)) >> COLOR_ENVMAP_SHR;
			fc += dc;
			fu += du;
			fv += dv;

			c3 = (texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)] * FIXED_TO_INT(fc, FP_BASE)) >> COLOR_ENVMAP_SHR;
			fc += dc;
			fu += du;
			fv += dv;

			#ifdef BIG_ENDIAN
				*dst32++ = ((c0 << 24) | (c1 << 16) | (c2 << 8) | c3);
			#else
				*dst32++ = ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0);
			#endif
			length-=4;
		};

		dst = (uint8*)dst32;
		while (length-- > 0) {
			c = (texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)] * FIXED_TO_INT(fc, FP_BASE)) >> COLOR_ENVMAP_SHR;
			*dst++ = c;

			fc += dc;
			fu += du;
			fv += dv;
		}

		++le;
		++re;
		dst8 += stride;
	} while(--count > 0);
}

static void fillGouraudEnvmapEdges16(int y0, int y1)
{
	const int stride = softBuffer.width;
	uint16 *dst16 = (uint16*)softBuffer.data + y0 * stride;

	int count = y1 - y0 + 1;
	Edge *le = &leftEdge[y0];
	Edge *re = &rightEdge[y0];

	const int texHeightShift = activeTexture->hShift;
	uint16* texData = (uint16*)activeTexture->bitmap;
	int Y;

	do {
		const int xl = le->x;
		const int cl = le->c;
		const int cr = re->c;
		const int ul = le->u;
		const int ur = re->u;
		const int vl = le->v;
		const int vr = re->v;
		int length = re->x - xl;

		if (length>0){
			uint16 *dst = dst16 + xl;
			uint32 *dst32;

			const int repDiv = divTab[length + DIV_TAB_SIZE / 2];
			const int dc = ((cr - cl) * repDiv) >> DIV_TAB_SHIFT;
			const int du = ((ur - ul) * repDiv) >> DIV_TAB_SHIFT;
			const int dv = ((vr - vl) * repDiv) >> DIV_TAB_SHIFT;
			int fc = cl;
			int fu = ul;
			int fv = vl;

			int c, cc;
			if (xl & 1) {
				c = FIXED_TO_INT(fc, FP_BASE);
				cc = texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)];
				//Y = ((cc >> 8) * c) >> COLOR_ENVMAP_SHR;
				Y = (cc * c) >> COLOR_ENVMAP_SHR;
				*dst++ = (cc & 0x00FF) | (Y & 0xFF00);
				fc += dc;
				fu += du;
				fv += dv;

				length--;
			}

			dst32 = (uint32*)dst;
			while(length >= 2) {
				int c0, c1;

				c = FIXED_TO_INT(fc, FP_BASE);
				cc = texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)];
				Y = (cc * c) >> COLOR_ENVMAP_SHR;
				c0 = (cc & 0x00FF) | (Y & 0xFF00);
				fc += dc;
				fu += du;
				fv += dv;

				c = FIXED_TO_INT(fc, FP_BASE);
				cc = texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)];
				Y = (cc * c) >> COLOR_ENVMAP_SHR;
				c1 = (cc & 0x00FF) | (Y & 0xFF00);
				fc += dc;
				fu += du;
				fv += dv;

				#ifdef BIG_ENDIAN
					*dst32++ = (c0 << 16) | c1;
				#else
					*dst32++ = (c1 << 16) | c0;
				#endif

				length -= 2;
			};

			dst = (uint16*)dst32;
			if (length & 1) {
				c = FIXED_TO_INT(fc, FP_BASE);
				cc = texData[(FIXED_TO_INT(fv, FP_BASE) << texHeightShift) + FIXED_TO_INT(fu, FP_BASE)];
				Y = (cc * c) >> COLOR_ENVMAP_SHR;
				*dst++ = (cc & 0x00FF) | (Y & 0xFF00);
				fc += dc;
				fu += du;
				fv += dv;
			}
		}

		++le;
		++re;
		dst16 += stride;
	} while(--count > 0);
}

static void drawTriangle(ScreenElement *e0, ScreenElement *e1, ScreenElement *e2, Screen *screen)
{
	int y0 = e0->y;
	int y1 = y0;

	prepareEdgeList(e0, e1, screen);
	prepareEdgeList(e1, e2, screen);
	prepareEdgeList(e2, e0, screen);

	if (e1->y < y0) y0 = e1->y; if (e1->y > y1) y1 = e1->y;
	if (e2->y < y0) y0 = e2->y; if (e2->y > y1) y1 = e2->y;

	if (y0 < 0) y0 = 0;
	if (y1 > screen->height-1) y1 = screen->height-1;

	fillEdges(y0, y1);
}
static void updateSoftBufferVariables(RectArea *bufferArea, Screen *screen)
{
	const int currentBufferSize = (bufferArea->width * bufferArea->height * screen->bpp) >> 3;
	uint32 *dst = (uint32*)softBuffer.data;

	softBuffer.width = bufferArea->width;
	softBuffer.height = bufferArea->height;
	softBuffer.posX = bufferArea->posX;
	softBuffer.posY = bufferArea->posY;

	if (currentBufferSize <= softBufferMaxSize) {
		int count = currentBufferSize >> 2;
		do {
			*dst++ = 0;
		}while(--count>0);
	}
}

static RectArea *createUnionOfLastTwoBuffersForArea(RectArea *currRectArea)
{
	static RectArea finalRectArea;
	static RectArea prevRectArea;
	static bool firstTime = true;

	const int currRightX = currRectArea->posX + currRectArea->width;
	const int currDownY = currRectArea->posY + currRectArea->height;

	int finalRightX, finalDownY;

	if (firstTime) {
		prevRectArea.posX = currRectArea->posX;
		prevRectArea.posY = currRectArea->posY;
		prevRectArea.width = currRectArea->width;
		prevRectArea.height = currRectArea->height;
		firstTime = false;
	}

	finalRightX = prevRectArea.posX + prevRectArea.width;
	finalDownY = prevRectArea.posY + prevRectArea.height;
	if (finalRightX < currRightX) finalRightX = currRightX;
	if (finalDownY < currDownY) finalDownY = currDownY;

	if (prevRectArea.posX < currRectArea->posX) finalRectArea.posX = prevRectArea.posX; else finalRectArea.posX = currRectArea->posX;
	if (prevRectArea.posY < currRectArea->posY) finalRectArea.posY = prevRectArea.posY; else finalRectArea.posY = currRectArea->posY;

	finalRectArea.width = finalRightX - finalRectArea.posX;
	finalRectArea.height = finalDownY - finalRectArea.posY;

	// Copy curr to prev for next frame
	prevRectArea.posX = currRectArea->posX;
	prevRectArea.posY = currRectArea->posY;
	prevRectArea.width = currRectArea->width;
	prevRectArea.height = currRectArea->height;

	return &finalRectArea;
}

static void prepareAndPositionSoftBuffer(Mesh *ms, ScreenElement *elements, Screen *screen)
{
	int i;
	RectArea currRectArea;
	RectArea *finalRectArea;
	const int count = ms->verticesNum;

	minX = maxX = elements[0].x;
	minY = maxY = elements[0].y;

	for (i=1; i<count; ++i) {
		const int x = elements[i].x;
		const int y = elements[i].y;
		if (x < minX) minX = x;
		if (x > maxX) maxX = x;
		if (y < minY) minY = y;
		if (y > maxY) maxY = y;
	}

	currRectArea.posX = minX;
	currRectArea.posY = minY;
	currRectArea.width = maxX - minX + 1;
	currRectArea.height = maxY - minY + 1;

	finalRectArea = createUnionOfLastTwoBuffersForArea(&currRectArea);
	
	finalRectArea->width = (finalRectArea->width + 3) & ~3;	// align 4 bytes width to avoid glitches or performance when memcpy lines to vram

	// Offset element positions to upper left min corner
	for (i=0; i<count; ++i) {
		elements[i].x -= finalRectArea->posX;
		elements[i].y -= finalRectArea->posY;
	}

	updateSoftBufferVariables(finalRectArea, screen);
}

static void prepareMeshSoftRender(Mesh *ms, ScreenElement *elements, Screen *screen)
{
	prepareAndPositionSoftBuffer(ms, elements, screen);

	switch(renderSoftMethod) {
		case RENDER_SOFT_METHOD_GOURAUD:
		{
			prepareEdgeList = prepareEdgeListGouraud;
			fillEdges = fillGouraudEdges16;
			if (ms->renderType & MESH_OPTION_RENDER_SOFT8) {
				fillEdges = fillGouraudEdges8;
			}
		}
		break;

		case RENDER_SOFT_METHOD_ENVMAP:
		{
			prepareEdgeList = prepareEdgeListEnvmap;
			fillEdges = fillEnvmapEdges16;
			if (ms->renderType & MESH_OPTION_RENDER_SOFT8) {
				fillEdges = fillEnvmapEdges8;
			}
		}
		break;

		case RENDER_SOFT_METHOD_GOURAUD_ENVMAP:
		{
			prepareEdgeList = prepareEdgeListGouraudEnvmap;
			fillEdges = fillGouraudEnvmapEdges16;
			if (ms->renderType & MESH_OPTION_RENDER_SOFT8) {
				fillEdges = fillGouraudEnvmapEdges8;
			}
		}
		break;

		default:
		break;
	}
}

static void renderMeshSoft(Mesh *ms, ScreenElement *elements, Screen *screen)
{
	ScreenElement *e0, *e1, *e2;
	int i,n;

	int *index = ms->index;

	prepareMeshSoftRender(ms, elements, screen);

	for (i=0; i<ms->polysNum; ++i) {
		e0 = &elements[*index++];
		e1 = &elements[*index++];
		e2 = &elements[*index++];

		n = (e0->x - e1->x) * (e2->y - e1->y) - (e2->x - e1->x) * (e0->y - e1->y);
		if (n > 0) {
			bindMeshPolyData(ms, i);
			drawTriangle(e0, e1, e2, screen);

			if (ms->poly[i].numPoints == 4) {	// if quad then render another triangle
				e1 = e2;
				e2 = &elements[*index];
				drawTriangle(e0, e1, e2, screen);
			}
		}
		if (ms->poly[i].numPoints == 4) ++index;
	}
}

static void renderSoftBufferToScreen8(Screen *screen)
{
	int x,y;
	const int srcWidth = softBuffer.width;
	const int srcHeight = softBuffer.height;
	const int screenWidth = screen->width;

	uint8 *src = (uint8*)softBuffer.data;
	uint8 *dst = (uint8*)screen->data + softBuffer.posY * screenWidth + softBuffer.posX;

	for (y=0; y<srcHeight; ++y) {
		for (x=0; x<srcWidth; ++x) {
			*(dst+x) = *(src+x);
		}
		src += srcWidth;
		dst += screenWidth;
	}
}

static void renderSoftBufferToScreen16(Screen *screen)
{
	int x,y;
	const int srcWidth = softBuffer.width;
	const int srcHeight = softBuffer.height;
	const int screenWidth = screen->width;

	uint16 *src = (uint16*)softBuffer.data;
	uint16 *dst = (uint16*)screen->data + softBuffer.posY * screenWidth + softBuffer.posX;

	for (y=0; y<srcHeight; ++y) {
		for (x=0; x<srcWidth; ++x) {
			*(dst+x) = *(src+x);
		}
		src += srcWidth;
		dst += screenWidth;
	}
}

static void renderMeshSoftWireframe(Mesh *ms, ScreenElement *elements, Screen *screen)
{
	ScreenElement *e0, *e1;

	int *lineIndex = ms->lineIndex;
	int i;

	prepareMeshSoftRender(ms, elements, screen);

	for (i=0; i<ms->linesNum; ++i) {
		e0 = &elements[*lineIndex++];
		e1 = &elements[*lineIndex++];

		bindMeshPolyData(ms, i);
		drawAntialiasedLine(e0, e1, screen);
	}
}

void renderTransformedMeshSoft(Mesh *ms, ScreenElement *elements, Screen *screen)
{
	if (renderSoftMethod == RENDER_SOFT_METHOD_WIREFRAME) {
		renderMeshSoftWireframe(ms, elements, screen);
	} else {
		renderMeshSoft(ms, elements, screen);
	}

	if (screen->bpp==8) {
		renderSoftBufferToScreen8(screen);
	} else if (screen->bpp==16) {
		renderSoftBufferToScreen16(screen);
	}
}

void setRenderSoftMethod(int method)
{
	renderSoftMethod = method;
}

void initEngineSoft(Screen *screen)
{
	initDivs();

	leftEdge = (Edge*)malloc(screen->height * sizeof(Edge));
	rightEdge = (Edge*)malloc(screen->height * sizeof(Edge));

	if (!lineColorShades[0]) lineColorShades[0] = crateColorShades(255,192,128, COLOR_GRADIENTS_SIZE);
	if (!lineColorShades[1]) lineColorShades[1] = crateColorShades(128,192,255, COLOR_GRADIENTS_SIZE);
	if (!lineColorShades[2]) lineColorShades[2] = crateColorShades(128,255,192, COLOR_GRADIENTS_SIZE);
	if (!lineColorShades[3]) lineColorShades[3] = crateColorShades(255,128,192, COLOR_GRADIENTS_SIZE);

	if (!gouraudColorShades) gouraudColorShades = crateColorShades(216,232,255, COLOR_GRADIENTS_SIZE);

	softBufferMaxSize = 4 * screen->width * screen->height;
	softBuffer.data = (uint8*)malloc(softBufferMaxSize);
}
