//==============================================================================
//
//  3D math file
//
//==============================================================================

#ifndef __3DUTILS_H__
#define __3DUTILS_H__

#include <math.h>

typedef struct {
    float    translate[3];
    float    scale[3];
    float    rotation[3];     /* Euler angles    */
    float    quaternion[4];   /* quaternion      */
    float    rotMatrix[3][3]; /* rotation matrix */
} DECOMP_MAT;


#define UTL_ROT_XYZ          0
#define UTL_ROT_XZY          1
#define UTL_ROT_YXZ          2
#define UTL_ROT_YZX          3
#define UTL_ROT_ZXY          4
#define UTL_ROT_ZYX          5

#define XROT                 'x'
#define YROT                 'y'
#define ZROT                 'z'

void utlMtx2Euler(int ord, float m[3][3], float rot[3]);
int DtMatrixGetTransforms(float *matrix, float *translate, float *scale, float *quaternion, float *rotation);

void init_math(void);

//==========================================================================

#define FP_BITS(fp) (*(DWORD *)&(fp))
extern unsigned int gFastSqrtTable[0x10000];  // declare table of square roots 
extern float gSinTable[1024];

_inline float crySqrtf(float n)
{

  if (FP_BITS(n) == 0)
    return 0.0;                 // check for square root of 0

  FP_BITS(n) = gFastSqrtTable[(FP_BITS(n) >> 8) & 0xFFFF] | ((((FP_BITS(n) - 0x3F800000) >> 1) + 0x3F800000) & 0x7F800000);

  return n;
}


#endif
