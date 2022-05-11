#include "tools.h"

#include "engine_mesh.h"
#include "engine_texture.h"
#include "mathutil.h"

Mesh* initMesh(int verticesNum, int polysNum, int indicesNum, int linesNum, int renderType)
{
	Mesh *ms = (Mesh*)malloc(sizeof(Mesh));

	ms->verticesNum = verticesNum;
	ms->polysNum = polysNum;
	ms->indicesNum = indicesNum;
	ms->linesNum = linesNum;

	ms->vertex = (Vertex*)malloc(ms->verticesNum * sizeof(Vertex));
	ms->index = (int*)malloc(ms->indicesNum * sizeof(int));
	ms->poly = (PolyData*)malloc(polysNum * sizeof(PolyData));
	ms->lineIndex = (int*)malloc(linesNum * 2 * sizeof(int));
	ms->polyNormal = (Vector3D*)malloc(ms->polysNum * sizeof(Vector3D));

	ms->vertexNormal = (Vector3D*)malloc(ms->verticesNum * sizeof(Vector3D));
	ms->vertexCol = (int*)malloc(ms->verticesNum * sizeof(int));
	ms->vertexTC = (TexCoords*)malloc(ms->verticesNum * sizeof(TexCoords));

	ms->renderType = renderType;

	return ms;
}
