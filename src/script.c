#include "script.h"
#include "tools.h"
#include "input.h"

#include "mathutil.h"

#include "engine_main.h"
#include "engine_soft.h"
#include "engine_mesh.h"
#include "engine_texture.h"

#include "procgen_mesh.h"
#include "procgen_texture.h"


static int rotX=0, rotY=0, rotZ=0;
static int zoom=256;

static const int rotVel = 2;
static const int zoomVel = 2;

static Mesh *softMesh8;
static Mesh *softMesh16;

static Texture *cloudTex8;
static Texture *cloudTex16;
static Object3D *softObj;

static int renderSoftMethodIndex = RENDER_SOFT_METHOD_GOURAUD;

 
static void effectMeshSoftInit(int bpp)
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

	cloudTex8 = initGenTexture(128, 128, 8, NULL, 1, TEXGEN_CLOUDS, false, NULL);
	cloudTex16 = initGenTexture(128, 128, 16, NULL, 1, TEXGEN_CLOUDS, false, NULL);
	params = makeMeshgenSquareColumnoidParams(size, ptArray->points, numPoints, true, true);

	softMesh8 = initGenMesh(meshType, params, MESH_OPTION_RENDER_SOFT8 | MESH_OPTION_ENABLE_LIGHTING | MESH_OPTION_ENABLE_ENVMAP, cloudTex8);
	softMesh16 = initGenMesh(meshType, params, MESH_OPTION_RENDER_SOFT16 | MESH_OPTION_ENABLE_LIGHTING | MESH_OPTION_ENABLE_ENVMAP, cloudTex16);

	if (bpp==8) {
		softObj = initObject3D(softMesh8);
		setObject3Dmesh(softObj, softMesh8);
	} else {
		softObj = initObject3D(softMesh16);
		setObject3Dmesh(softObj, softMesh16);
	}

	destroyPoint2Darray(ptArray);

	setRenderSoftMethod(renderSoftMethodIndex);
}

static void inputScript()
{
	//rotX+=1;
	//rotY+=2;
	//rotZ+=3;

	updateInput();

	if (isJoyButtonPressed(JOY_LEFT)) {
		rotY -= rotVel;
	}

	if (isJoyButtonPressed(JOY_RIGHT)) {
		rotY += rotVel;
	}

	if (isJoyButtonPressed(JOY_UP)) {
		rotX -= rotVel;
	}

	if (isJoyButtonPressed(JOY_DOWN)) {
		rotX += rotVel;
	}

	if (isJoyButtonPressed(JOY_A)) {
		rotZ += rotVel;
	}

	if (isJoyButtonPressed(JOY_B)) {
		rotZ -= rotVel;
	}

	if (isJoyButtonPressed(JOY_D)) {
		zoom += zoomVel;
	}

	if (isJoyButtonPressed(JOY_E)) {
		zoom -= zoomVel;
	}

	if (isJoyButtonPressedOnce(JOY_START)) {
		++renderSoftMethodIndex;
		if (renderSoftMethodIndex == RENDER_SOFT_METHOD_NUM) renderSoftMethodIndex = 0;

		setRenderSoftMethod(renderSoftMethodIndex);
	}
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
			const int c = (i & 31) << 3;
			eris_tetsu_set_palette(i, RGB2YUV(c, c, c));
		}
	}

	effectMeshSoftInit(screen->bpp);
}

void scriptRun(Screen *screen, int t)
{
	inputScript();

	fastClearScreen(screen);

	setObject3Dpos(softObj, 0, 0, zoom);
	setObject3Drot(softObj, rotX, rotY, rotZ);
	renderObject3D(softObj, screen);
}