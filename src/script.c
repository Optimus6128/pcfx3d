#include "script.h"
#include "tools.h"

#include "mathutil.h"

#include "engine_main.h"
#include "engine_soft.h"
#include "engine_mesh.h"
#include "engine_texture.h"

#include "procgen_mesh.h"
#include "procgen_texture.h"


static int rotX=0, rotY=0, rotZ=0;
static int zoom=256;

static Mesh *softMesh8;
static Mesh *softMesh16;

static Texture *cloudTex;
static Object3D *softObj;

static int renderSoftMethodIndex = RENDER_SOFT_METHOD_GOURAUD;

static void effectMeshSoftInit()
{
	int i;
	const int numPoints = 8;
	const int size = 64;
	MeshgenParams params;

	Point2Darray *ptArray = initPoint2Darray(numPoints);
	
	int meshType = MESH_SQUARE_COLUMNOID;

	for (i=0; i<numPoints; ++i) {
		const int y = (size/4) * (numPoints/2 - i);
		const int r = ((SinF16((i*20) << 16) * (size / 2)) >> 16) + size / 2;
		addPoint2D(ptArray, r,y);
	}

	cloudTex = initGenTexture(128, 128, 16, NULL, 1, TEXGEN_CLOUDS, false, NULL);
	params = makeMeshgenSquareColumnoidParams(size, ptArray->points, numPoints, true, true);

	softMesh8 = initGenMesh(meshType, params, MESH_OPTION_RENDER_SOFT8 | MESH_OPTION_ENABLE_LIGHTING | MESH_OPTION_ENABLE_ENVMAP, cloudTex);
	softMesh16 = initGenMesh(meshType, params, MESH_OPTION_RENDER_SOFT16 | MESH_OPTION_ENABLE_LIGHTING | MESH_OPTION_ENABLE_ENVMAP, cloudTex);

	softObj = initObject3D(softMesh8);

	setObject3Dmesh(softObj, softMesh8);

	destroyPoint2Darray(ptArray);

	setRenderSoftMethod(renderSoftMethodIndex);
}

static void inputScript()
{
	rotX+=1;
	rotY+=2;
	rotZ+=3;
}

static void fastClearScreen(Screen *screen)
{
	int i;

	uint32 *dst32 = (uint32*)screen->data;
	const int count = (screen->width * screen->height * (screen->bpp >> 3)) >> (2+3);
	for (i=0; i<count; ++i) {
		*dst32++ = 0;
		*dst32++ = 0;
		*dst32++ = 0;
		*dst32++ = 0;
		*dst32++ = 0;
		*dst32++ = 0;
		*dst32++ = 0;
		*dst32++ = 0;
	}
}

void scriptInit(Screen *screen)
{
	if (screen->bpp==8) {
		int i;
		for(i = 0; i < 256; i++) {
			eris_tetsu_set_palette(i, RGB2YUV(i, i, i));
		}
	}

	effectMeshSoftInit();
}

void scriptRun(Screen *screen, int t)
{
	inputScript();

	fastClearScreen(screen);

	setObject3Dpos(softObj, 0, 0, zoom);
	setObject3Drot(softObj, rotX, rotY, rotZ);
	renderObject3D(softObj, screen);
}
