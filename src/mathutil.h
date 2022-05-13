#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "sintab.h"

#define FP_CORE 16
#define FP_BASE 12
#define FP_BASE_TO_CORE (FP_CORE - FP_BASE)

#define NORMAL_SHIFT 8

#define FLOAT_TO_FIXED(f,b) ((int)((f) * (1 << b)))
#define INT_TO_FIXED(i,b) ((i) * (1 << b))
#define UINT_TO_FIXED(i,b) ((i) << b)
#define FIXED_TO_INT(x,b) ((x) >> b)
#define FIXED_TO_FLOAT(x,b) ((float)(x) / (1 << b))
#define FIXED_MUL(x,y,b) (((x) * (y)) >> b)
#define FIXED_DIV(x,y,b) (((x) << b) / (y))
#define FIXED_SQRT(x,b) (isqrt((x) << b))

#define VEC3D_TO_FIXED(v,b) v.x *= (1 << b); v.y *= (1 << b); v.z *= (1 << b);

#define CLAMP(value,min,max) if (value<min) value=min; if (value>max) value=max;
#define CLAMP_LEFT(value,min) if (value<min) value=min;
#define CLAMP_RIGHT(value,max) if (value>max) value=max;

#define PI 3.14159265359f
#define DEG256RAD ((2 * PI) / 256.0f)

#define SinF16(a) sineLUT[((a) & ((1 << 24) - 1)) >> (16 - (SINE_LUT_BITS - SINE_DEG_BITS))]
#define CosF16(a) SinF16((a) + (64 << 16))

#define MakeRGB15(r,g,b) (((uint32)(r)<<10)|((uint32)(g)<<5)|(uint32)(b))


typedef struct Vertex
{
	int x,y,z;
}Vertex;

typedef struct Vector3D
{
	int x,y,z;
}Vector3D;

typedef struct Point2D
{
    int x,y;
}Point2D;

typedef struct Point2Darray
{
	Point2D *points;
	int currentIndex;
	int pointsNum;
}Point2Darray;


extern int shr[257];

int isqrt(int x);

int getRand(int from, int to);
int getShr(int n);
void initMathUtil(void);

void setVector3D(Vector3D *v, int x, int y, int z);
void setVector3DfromVertices(Vector3D *v, Vertex *v0, Vertex *v1);
void calcVector3Dcross(Vector3D *vRes, Vector3D *v0, Vector3D *v1);
void normalizeVector3D(Vector3D *v);

Point2Darray *initPoint2Darray(int numPoints);
void addPoint2D(Point2Darray *ptArray, int x, int y);
void destroyPoint2Darray(Point2Darray *ptArray);

void MulManyVec3Mat33_F16(vec3f16* dest, vec3f16* src, mat33f16 mat, int32 count);

#endif
