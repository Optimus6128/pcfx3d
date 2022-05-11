#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#include "engine_texture.h"
#include "mathutil.h"

#define MESH_OPTION_RENDER_SOFT8	(1 << 0)
#define MESH_OPTION_RENDER_SOFT16	(1 << 1)
#define MESH_OPTION_ENABLE_LIGHTING	(1 << 2)
#define MESH_OPTION_ENABLE_ENVMAP	(1 << 3)
#define MESH_OPTIONS_DEFAULT (MESH_OPTION_RENDER_SOFT16)

typedef struct PolyData
{
	ubyte numPoints;
	ubyte textureId;
	ubyte palId;
	ubyte dummy;	// dummy to keep it 32bit aligned (could use it for more poly info in the future)
}PolyData;

typedef struct TexCoords
{
	int u,v;
}TexCoords;

typedef struct Mesh
{
	Vertex *vertex;
	int verticesNum;

	int *index;
	int indicesNum;

	PolyData *poly;
	int polysNum;

	Vector3D *polyNormal;
	Vector3D *vertexNormal;

	int *lineIndex;
	int linesNum;

	int *vertexCol;
	TexCoords *vertexTC;

	Texture *tex;
	int texturesNum;

	int renderType;
}Mesh;


Mesh* initMesh(int verticesNum, int polysNum, int indicesNum, int linesNum, int renderType);

#endif
