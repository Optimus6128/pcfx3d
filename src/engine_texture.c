#include "engine_texture.h"

#include "mathutil.h"

void setupTexture(int width, int height, int bpp, int type, ubyte *bmp, uint16 *pal, ubyte numPals, Texture *tex)
{
	// Can't have palettized texture if bpp over 8
	if (bpp > 8 || numPals == 0) {
		type &= ~TEXTURE_TYPE_PALLETIZED;
	}

	tex->width = width;
	tex->height = height;
	tex->wShift = getShr(width);
	tex->hShift = getShr(height);
	tex->bpp = bpp;
	tex->type = type;

	if (type & TEXTURE_TYPE_PALLETIZED) {
		if (!pal) {
			int palBits = bpp;
			if (palBits > 5) palBits = 5;	// Can't have more than 5bits (32 colors palette), even in 6bpp or 8bpp modes
			tex->pal = (uint16*)malloc(sizeof(uint16) * (1 << palBits) * numPals);
		} else {
			tex->pal = pal;
		}
	}

	if (!bmp) {
		const int size = (width * height * bpp) / 8;
		tex->bitmap = (ubyte*)malloc(size);
	} else {
		tex->bitmap = bmp;
	}
}

Texture* initTextures(int width, int height, int bpp, int type, ubyte *bmp, uint16 *pal, ubyte numPals, ubyte numTextures)
{
	int i;
	Texture *texs = (Texture*)malloc(numTextures * sizeof(Texture));

	for (i=0; i<numTextures; ++i) {
		setupTexture(width, height, bpp, type, bmp, pal, numPals, &texs[i]);
	}

	return texs;
}

Texture *initTexture(int width, int height, int bpp, int type, ubyte *bmp, uint16 *pal, ubyte numPals)
{
	return initTextures(width, height, bpp, type, bmp, pal, numPals, 1);
}
