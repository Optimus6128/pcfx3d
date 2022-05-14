#include "tools.h"
#include "main.h"
#include "mathutil.h"

#include "engine_main.h"
#include "engine_mesh.h"
#include "engine_texture.h"
#include "engine_main.h"
#include "engine_soft.h"


static Vertex screenVertices[MAX_VERTEX_ELEMENTS_NUM];
static ScreenElement screenElements[MAX_VERTEX_ELEMENTS_NUM];
static Vector3D rotatedNormals[MAX_VERTEX_ELEMENTS_NUM];

static int icos[256], isin[256];
static uint32 recZ[NUM_REC_Z];


void createRotationMatrixValues(int rotX, int rotY, int rotZ, int *rotVecs)
{
	const int cosxr = icos[rotX & 255];
	const int cosyr = icos[rotY & 255];
	const int coszr = icos[rotZ & 255];
	const int sinxr = isin[rotX & 255];
	const int sinyr = isin[rotY & 255];
	const int sinzr = isin[rotZ & 255];

	*rotVecs++ = (FIXED_MUL(cosyr, coszr, FP_BASE)) << FP_BASE_TO_CORE;
	*rotVecs++ = (FIXED_MUL(FIXED_MUL(sinxr, sinyr, FP_BASE), coszr, FP_BASE) - FIXED_MUL(cosxr, sinzr, FP_BASE)) << FP_BASE_TO_CORE;
	*rotVecs++ = (FIXED_MUL(FIXED_MUL(cosxr, sinyr, FP_BASE), coszr, FP_BASE) + FIXED_MUL(sinxr, sinzr, FP_BASE)) << FP_BASE_TO_CORE;
	*rotVecs++ = (FIXED_MUL(cosyr, sinzr, FP_BASE)) << FP_BASE_TO_CORE;
	*rotVecs++ = (FIXED_MUL(cosxr, coszr, FP_BASE) + FIXED_MUL(FIXED_MUL(sinxr, sinyr, FP_BASE), sinzr, FP_BASE)) << FP_BASE_TO_CORE;
	*rotVecs++ = (FIXED_MUL(-sinxr, coszr, FP_BASE) + FIXED_MUL(FIXED_MUL(cosxr, sinyr, FP_BASE), sinzr, FP_BASE)) << FP_BASE_TO_CORE;
	*rotVecs++ = (-sinyr) << FP_BASE_TO_CORE;
	*rotVecs++ = (FIXED_MUL(sinxr, cosyr, FP_BASE)) << FP_BASE_TO_CORE;
	*rotVecs = (FIXED_MUL(cosxr, cosyr, FP_BASE)) << FP_BASE_TO_CORE;
}

static void translateAndProjectVertices(Object3D *obj, Screen *screen)
{
	int i;

	const int posX = obj->posX;
	const int posY = obj->posY;
	const int posZ = obj->posZ;

	const int lvNum = obj->mesh->verticesNum;

	const int offsetX = screen->width >> 1;
	const int offsetY = screen->height >> 1;

	for (i=0; i<lvNum; i++)
	{
		const int vz = screenVertices[i].z + posZ;
		if (vz > 0) {
			const int recDivZ = recZ[vz];
			screenElements[i].x = offsetX + ((((screenVertices[i].x + posX) << PROJ_SHR) * recDivZ) >> REC_FPSHR);
			screenElements[i].y = offsetY - ((((screenVertices[i].y + posY) << PROJ_SHR) * recDivZ) >> REC_FPSHR);
			screenElements[i].z = vz;
		}
	}
}

static void rotateVerticesAndNormals(Object3D *obj, bool rotatePolyNormals, bool rotateVertexNormals)
{
	mat33f16 rotMat;

	createRotationMatrixValues(obj->rotX, obj->rotY, obj->rotZ, (int*)rotMat);

	MulManyVec3Mat33_F16((vec3f16*)screenVertices, (vec3f16*)obj->mesh->vertex, rotMat, obj->mesh->verticesNum);

	if (obj->mesh->renderType & MESH_OPTION_ENABLE_LIGHTING) {
		// in the future, we might not need to rotate the normals but rather the light against the normals
		// so we won't need the normals container, as we will test light vector against original mesh normals
		if (rotatePolyNormals) {
			MulManyVec3Mat33_F16((vec3f16*)rotatedNormals, (vec3f16*)obj->mesh->polyNormal, rotMat, obj->mesh->polysNum); 
		}
		if (rotateVertexNormals) {
			MulManyVec3Mat33_F16((vec3f16*)rotatedNormals, (vec3f16*)obj->mesh->vertexNormal, rotMat, obj->mesh->verticesNum);
		}
	}
}

static void calculateVertexLighting(Object3D *obj)
{
	int i, c;
	const int verticesNum = obj->mesh->verticesNum;

	for (i=0; i<verticesNum; ++i) {
		int normZ = -rotatedNormals[i].z;
		CLAMP(normZ,0,((1<<NORMAL_SHIFT)-1))
		c = normZ >> (NORMAL_SHIFT-COLOR_GRADIENTS_SHR);
		CLAMP(c,1,COLOR_GRADIENTS_SIZE-2)
		screenElements[i].c = c;
	}
}

static void calculateVertexEnvmapTC(Object3D *obj)
{
	int i;
	const int verticesNum = obj->mesh->verticesNum;
	Texture *tex = &obj->mesh->tex[0];

	const int texWidthHalf = tex->width >> 1;
	const int texHeightHalf = tex->height >> 1;
	const int wShiftHalf = NORMAL_SHIFT - tex->wShift + 1;
	const int hShiftHalf = NORMAL_SHIFT - tex->hShift + 1;

	for (i=0; i<verticesNum; ++i) {
		int normZ = rotatedNormals[i].z;
		if (normZ != 0) {
			int normX = (rotatedNormals[i].x>>wShiftHalf) + texWidthHalf;
			int normY = (rotatedNormals[i].y>>hShiftHalf) + texHeightHalf;

			screenElements[i].u = normX;
			screenElements[i].v = normY;
		}
	}
}

void renderObject3D(Object3D *obj, Screen *screen)
{
	rotateVerticesAndNormals(obj, false, true);
	translateAndProjectVertices(obj, screen);

	if (obj->mesh->renderType & MESH_OPTION_ENABLE_LIGHTING) {
		calculateVertexLighting(obj);
	}
	if (obj->mesh->renderType & MESH_OPTION_ENABLE_ENVMAP) {
		calculateVertexEnvmapTC(obj);
	}
	renderTransformedMeshSoft(obj->mesh, screenElements, screen);
}

Object3D* initObject3D(Mesh *ms)
{
	Object3D *obj = (Object3D*)malloc(sizeof(Object3D));

	obj->mesh = ms;
	
	obj->posX = obj->posY = obj->posZ = 0;
	obj->rotX = obj->rotY = obj->rotZ = 0;

	return obj;
}

void setObject3Dpos(Object3D *obj, int px, int py, int pz)
{
	obj->posX = px;
	obj->posY = py;
	obj->posZ = pz;
}

void setObject3Drot(Object3D *obj, int rx, int ry, int rz)
{
	obj->rotX = rx;
	obj->rotY = ry;
	obj->rotZ = rz;
}

void setObject3Dmesh(Object3D *obj, Mesh *ms)
{
	obj->mesh = ms;
}

void initEngine(Screen *screen)
{
	uint32 i;
	for(i=0; i<256; i++)
	{
		isin[i] = SinF16(i << 16) >> 4;
		icos[i] = CosF16(i << 16) >> 4;
	}

	for (i=1; i<NUM_REC_Z; ++i) {
		recZ[i] = (1 << REC_FPSHR) / i;
	}

	initEngineSoft(screen);
}
