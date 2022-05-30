#include "script.h"
#include "tools.h"
#include "input.h"

#include "mathutil.h"
#include "bitstest.h"
#include "tinyfont.h"

#include "engine_main.h"
#include "engine_soft.h"
#include "engine_mesh.h"
#include "engine_texture.h"

#include "procgen_mesh.h"
#include "procgen_texture.h"

#include "tools.h"


static int rotX=0, rotY=0, rotZ=0;
static int zoom=256;

static const int rotVel = 2;
static const int zoomVel = 8;

static Texture *cloudTex8;
static Texture *cloudTex16;
static Object3D *softCubeObj, *softColumnoidObj;
static Object3D *softObj;

static int renderSoftMethodIndex = RENDER_SOFT_METHOD_GOURAUD;

static bool autoRot = true;
static bool renderStuff = true;


static Object3D *initMeshObject(int meshgenId, const MeshgenParams params, int optionsFlags, Texture *tex)
{
	Object3D *meshObj;

	Mesh *softMesh = initGenMesh(meshgenId, params, optionsFlags, tex);
	meshObj = initObject3D(softMesh);
	setObject3Dmesh(meshObj, softMesh);

	return meshObj;
}

static MeshgenParams initMeshObjectParams(int meshgenId)
{
	MeshgenParams params;

	switch(meshgenId) {
		case MESH_CUBE:
		{
			params = makeDefaultMeshgenParams(96);
		}
		break;

		case MESH_SQUARE_COLUMNOID:
		{
			int i;
			const int numPoints = 8;
			const int size = 64;
			Point2Darray *ptArray = initPoint2Darray(numPoints);

			for (i=0; i<numPoints; ++i) {
				const int y = (size/4) * (numPoints/2 - i);
				const int r = ((SinF16((i*20) << 16) * (size / 2)) >> 16) + size / 2;
				addPoint2D(ptArray, r,y);
			}
			params = makeMeshgenSquareColumnoidParams(size, ptArray->points, numPoints, true, true);

			// destroyPoint2Darray(ptArray); //why it crashes now?
		}
		break;
	}

	return params;
}
 
static void effectMeshSoftInit(int bpp)
{
	const int lightingOptions = MESH_OPTION_ENABLE_LIGHTING | MESH_OPTION_ENABLE_ENVMAP;
	MeshgenParams paramsCube = initMeshObjectParams(MESH_CUBE);
	MeshgenParams paramsColumnoid = initMeshObjectParams(MESH_SQUARE_COLUMNOID);

	const int texWidth = 64;
	const int texHeight = 64;

	if (bpp==8) {
		const int optionsFlags = MESH_OPTION_RENDER_SOFT8 | lightingOptions;
		cloudTex8 = initGenTexture(texWidth, texHeight, 8, NULL, 1, TEXGEN_CLOUDS, false, NULL);
		softCubeObj = initMeshObject(MESH_CUBE, paramsCube, optionsFlags, cloudTex8);
		softColumnoidObj = initMeshObject(MESH_SQUARE_COLUMNOID, paramsColumnoid, optionsFlags, cloudTex8);
	} else {
		const int optionsFlags = MESH_OPTION_RENDER_SOFT16 | lightingOptions;
		cloudTex16 = initGenTexture(texWidth, texHeight, 16, NULL, 1, TEXGEN_CLOUDS, false, NULL);
		softCubeObj = initMeshObject(MESH_CUBE, paramsCube, optionsFlags, cloudTex16);
		softColumnoidObj = initMeshObject(MESH_SQUARE_COLUMNOID, paramsColumnoid, optionsFlags, cloudTex16);

	}
	setRenderSoftMethod(renderSoftMethodIndex);

	softObj = softCubeObj;
}

static void inputScript()
{
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
		zoom += zoomVel;
	}
	if (isJoyButtonPressed(JOY_B)) {
		zoom -= zoomVel;
	}
	if (isJoyButtonPressedOnce(JOY_C)) {
		autoRot = !autoRot;
	}

	if (isJoyButtonPressedOnce(JOY_D)) {
		renderStuff = !renderStuff;
	}
	if (isJoyButtonPressedOnce(JOY_E)) {
		/*rotZ -= rotVel;*/
		displayMethod--;
		if (displayMethod<0) displayMethod = 4;
	}
	if (isJoyButtonPressedOnce(JOY_F)) {
		/*rotZ += rotVel;*/
		displayMethod++;
		if (displayMethod==5) displayMethod = 0;
	}

	if (isJoyButtonPressedOnce(JOY_SELECT)) {
		if (softObj==softCubeObj) {
			softObj = softColumnoidObj;
		} else {
			softObj = softCubeObj;
		}
	}
	
	if (isJoyButtonPressedOnce(JOY_START)) {
		++renderSoftMethodIndex;
		if (renderSoftMethodIndex == RENDER_SOFT_METHOD_NUM) renderSoftMethodIndex = 0;

		setRenderSoftMethod(renderSoftMethodIndex);
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
	updateInput();
	inputScript();

	if (screen->bpp != 16 && renderSoftMethodIndex==RENDER_SOFT_METHOD_WIREFRAME) {
		renderSoftMethodIndex = RENDER_SOFT_METHOD_GOURAUD;
		setRenderSoftMethod(renderSoftMethodIndex);
	}

	if (autoRot) {
		rotX+=1;
		rotY+=2;
		rotZ+=3;
	}

	if (renderStuff) {
		setObject3Dpos(softObj, 0, 0, zoom);
		setObject3Drot(softObj, rotX, rotY, rotZ);
		renderObject3D(softObj, screen);
	}

	if (screen->bpp > 8) drawNumber(224,24, displayMethod, screen);
}
