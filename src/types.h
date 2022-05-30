#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

typedef signed char	int8;
typedef signed short int16;
typedef signed long	int32;

typedef unsigned char	uchar;
typedef uchar	ubyte;
typedef uchar	uint8;

typedef unsigned short ushort;
typedef ushort uint16;

typedef unsigned long ulong;
typedef ulong uint32;

/*typedef	uchar Boolean;
typedef Boolean	bool;

#define TRUE	((Boolean)1)
#define FALSE	((Boolean)0)

#define false	FALSE
#define true	TRUE*/

typedef int32 frac16, frac30, frac14;
typedef uint32 ufrac16, ufrac30, ufrac14;

typedef frac16 vec3f16[3], vec4f16[4], mat33f16[3][3], mat44f16[4][4], mat34f16[4][3];
typedef frac30 vec3f30[3], vec4f30[4], mat33f30[3][3], mat44f30[4][4], mat34f30[4][3];

#endif
