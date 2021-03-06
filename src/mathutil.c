#include <stdlib.h>

#include "mathutil.h"


int isqrt(int x) {
    int q = 1, r = 0;
    while (q <= x) {
        q <<= 2;
    }
    while (q > 1) {
        int t;
        q >>= 2;
        t = x - r - q;
        r >>= 1;
        if (t >= 0) {
            x = t;
            r += q;
        }
    }
    return r;
} 

int getRand(int from, int to)
{
	int rnd;
	if (from > to) return 0;
	if (from==to) return to;
	rnd = from + (rand() % (to - from));
	return rnd;
}

int getShr(int n)
{
	int b = -1;
	do{
		b++;
	}while((n>>=1)!=0);
	return b;
}

void setVector3D(Vector3D *v, int x, int y, int z)
{
	v->x = x;
	v->y = y;
	v->z = z;
}

void setVector3DfromVertices(Vector3D *v, Vertex *v0, Vertex *v1)
{
	v->x = v1->x - v0->x;
	v->y = v1->y - v0->y;
	v->z = v1->z - v0->z;
}

void calcVector3Dcross(Vector3D *vRes, Vector3D *v0, Vector3D *v1)
{
	vRes->x = v0->y * v1->z - v0->z * v1->y;
	vRes->y = v0->z * v1->x - v0->x * v1->z;
	vRes->z = v0->x * v1->y - v0->y * v1->x;
}

void normalizeVector3D(Vector3D *v)
{
	int length = isqrt(v->x * v->x + v->y * v->y + v->z * v->z);

	if (length != 0) {
		v->x = (v->x << NORMAL_SHIFT) / length;
		v->y = (v->y << NORMAL_SHIFT) / length;
		v->z = (v->z << NORMAL_SHIFT) / length;
	}
}

Point2Darray *initPoint2Darray(int numPoints)
{
	Point2Darray *ptArray = (Point2Darray*)malloc(sizeof(Point2Darray));

	ptArray->pointsNum = numPoints;
	ptArray->currentIndex = 0;
	ptArray->points = (Point2D*)malloc(sizeof(Point2D) * numPoints);

	return ptArray;
}

void addPoint2D(Point2Darray *ptArray, int x, int y)
{
	if (ptArray->currentIndex < ptArray->pointsNum) {
		Point2D *newPoint = &ptArray->points[ptArray->currentIndex++];
		newPoint->x = x;
		newPoint->y = y;
	}
}

void destroyPoint2Darray(Point2Darray *ptArray)
{
	free(ptArray->points);
	free(ptArray);
}

void MulManyVec3Mat33_F16(vec3f16* dest, vec3f16* src, mat33f16 mat, int32 count)
{
	int i;
	frac16 *m = (frac16*)mat;
	frac16 *s = (frac16*)src;
	frac16 *d = (frac16*)dest;

	for (i = 0; i < count; ++i)
	{
		const int x = *s++;
		const int y = *s++;
		const int z = *s++;

		*d++ = FIXED_TO_INT(x * m[0] + y * m[3] + z * m[6], FP_CORE);
		*d++ = FIXED_TO_INT(x * m[1] + y * m[4] + z * m[7], FP_CORE);
		*d++ = FIXED_TO_INT(x * m[2] + y * m[5] + z * m[8], FP_CORE);
	}
}
