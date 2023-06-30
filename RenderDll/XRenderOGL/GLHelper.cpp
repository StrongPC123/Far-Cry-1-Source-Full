/*=============================================================================
  GLHelper.cpp : OpenGL some useful functions implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#include "RenderPCH.h"
#include <Cry_Math.h>
#include "GL_Renderer.h"



static float sIdentity[4][4] = 
{
  {1,0,0,0},
  {0,1,0,0},
  {0,0,1,0},
  {0,0,0,1}
};


/*
* Transform a point (column vector) by a 4x4 matrix.  I.e.  out = m * in
* Input:  m - the 4x4 matrix
*         in - the 4x1 vector
* Output:  out - the resulting 4x1 vector.
*/
static void transform_point(float out[4], const float m[16], const float in[4])
{
#define M(row,col)  m[col*4+row]
  out[0] = M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
  out[1] = M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
  out[2] = M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
  out[3] = M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

/* projection du point (objx,objy,obz) sur l'ecran (winx,winy,winz) */
int SGLFuncs::gluProject(float objx, float objy, float objz, const float model[16], const float proj[16], const int viewport[4], float * winx, float * winy, float * winz)
{          
  /* matrice de transformation */
  float in[4], out[4];
  /* initilise la matrice et le vecteur a transformer */
  in[0] = objx;
  in[1] = objy;
  in[2] = objz;
  in[3] = 1.0;
  transform_point(out, model, in);
  transform_point(in, proj, out);
  /* d'ou le resultat normalise entre -1 et 1 */
  if (in[3] == 0.0)
    return FALSE;
  in[0] /= in[3];
  in[1] /= in[3];
  in[2] /= in[3];
  /* en coordonnees ecran */
  *winx = viewport[0] + (1 + in[0]) * viewport[2] / 2.0f;
  *winy = viewport[1] + (1 + in[1]) * viewport[3] / 2.0f;
  /* entre 0 et 1 suivant z */
  *winz = (1 + in[2]) / 2;
  return TRUE;
}

int SGLFuncs::gluUnProject(float winx, float winy, float winz, const float model[16], const float proj[16], const int viewport[4], float * objx, float * objy, float * objz)
{
  /* matrice de transformation */
  float m[16], A[16];
  float in[4], out[4];
  /* transformation coordonnees normalisees entre -1 et 1 */
  in[0] = (winx - viewport[0]) * 2 / viewport[2] - 1.0f;
  in[1] = (winy - viewport[1]) * 2 / viewport[3] - 1.0f;
  in[2] = 2.0f * winz - 1.0f;
  in[3] = 1.0;
  /* calcul transformation inverse */
  mathMatrixMultiply(A, (float *)proj, (float *)model, g_CpuFlags);
  mathMatrixInverse(m, A, g_CpuFlags);
  /* d'ou les coordonnees objets */
  transform_point(out, m, in);
  if (out[3] == 0.0)
    return false;
  *objx = out[0] / out[3];
  *objy = out[1] / out[3];
  *objz = out[2] / out[3];
  return true;
}

void SGLFuncs::gluLookAt(float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz, float *m)
{
  float x[3], y[3], z[3];
  float mag;
  /* Make rotation matrix */
  /* Z vector */
  z[0] = eyex - centerx;
  z[1] = eyey - centery;
  z[2] = eyez - centerz;
  mag = cry_sqrtf(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
  if (mag)
  {			/* mpichler, 19950515 */
    z[0] /= mag;
    z[1] /= mag;
    z[2] /= mag;
  }
  /* Y vector */
  y[0] = upx;
  y[1] = upy;
  y[2] = upz;
  /* X vector = Y cross Z */
  x[0] = y[1] * z[2] - y[2] * z[1];
  x[1] = -y[0] * z[2] + y[2] * z[0];
  x[2] = y[0] * z[1] - y[1] * z[0];
  /* Recompute Y = Z cross X */
  y[0] = z[1] * x[2] - z[2] * x[1];
  y[1] = -z[0] * x[2] + z[2] * x[0];
  y[2] = z[0] * x[1] - z[1] * x[0];
  /* mpichler, 19950515 */
  /* cross product gives area of parallelogram, which is < 1.0 for
  * non-perpendicular unit-length vectors; so normalize x, y here
  */
  mag = cry_sqrtf(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
  if (mag)
  {
    x[0] /= mag;
    x[1] /= mag;
    x[2] /= mag;
  }
  mag = cry_sqrtf(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
  if (mag)
  {
    y[0] /= mag;
    y[1] /= mag;
    y[2] /= mag;
  }
#define M(row,col)  m[col*4+row]
  M(0, 0) = x[0];
  M(0, 1) = x[1];
  M(0, 2) = x[2];
  M(0, 3) = 0;
  M(1, 0) = y[0];
  M(1, 1) = y[1];
  M(1, 2) = y[2];
  M(1, 3) = 0;
  M(2, 0) = z[0];
  M(2, 1) = z[1];
  M(2, 2) = z[2];
  M(2, 3) = 0;
  M(3, 0) = 0;
  M(3, 1) = 0;
  M(3, 2) = 0;
  M(3, 3) = 1;
#undef M
  glTranslate(m, -eyex, -eyey, -eyez);
}

void SGLFuncs::glTranslate(float *m, float x, float y, float z)
{
  m[12] = m[0] * x + m[4] * y + m[8]  * z + m[12];
  m[13] = m[1] * x + m[5] * y + m[9]  * z + m[13];
  m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
  m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];  
}

void SGLFuncs::glRotate(float *m, float angle, float x, float y, float z)
{
  float mr[16];

  /* This function contributed by Erich Boleyn (erich@uruk.org) */
  float mag, s, c;
  float xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;
  angle = ( angle * PI ) / 180.0f;
  s = cry_sinf( angle );
  c = cry_cosf( angle );
  mag = cry_sqrtf( x*x + y*y + z*z );
  if (mag <= 1.0e-4)
  {
    /* generate an identity matrix and return */
    memcpy(m, sIdentity, sizeof(float)*16);
  }
  else
  {
    x /= mag;
    y /= mag;
    z /= mag;
  #define M(row,col)  mr[col*4+row]
    xx = x * x;
    yy = y * y;
    zz = z * z;
    xy = x * y;
    yz = y * z;
    zx = z * x;
    xs = x * s;
    ys = y * s;
    zs = z * s;
    one_c = 1.0F - c;
    M(0,0) = (one_c * xx) + c;
    M(0,1) = (one_c * xy) - zs;
    M(0,2) = (one_c * zx) + ys;
    M(0,3) = 0.0F;
    M(1,0) = (one_c * xy) + zs;
    M(1,1) = (one_c * yy) + c;
    M(1,2) = (one_c * yz) - xs;
    M(1,3) = 0.0F;
    M(2,0) = (one_c * zx) - ys;
    M(2,1) = (one_c * yz) + xs;
    M(2,2) = (one_c * zz) + c;
    M(2,3) = 0.0F;
    M(3,0) = 0.0F;
    M(3,1) = 0.0F;
    M(3,2) = 0.0F;
    M(3,3) = 1.0F;
  }
  mathMatrixMultiply(m, m, mr, g_CpuFlags);
}

