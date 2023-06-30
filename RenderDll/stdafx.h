/*=============================================================================
	RenderPC.cpp: Cry Render support precompiled header generator.
	Copyright 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honitch Andrey

=============================================================================*/

#define CRY_API

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#endif //_DEBUG

//! Include standart headers.
#include <assert.h>

//#define PS2
//#define OPENGL


#ifdef _XBOX

//! Include standart headers.
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <io.h>
#include <memory.h>
#include <time.h>
#include <direct.h>
#include <search.h>
#include <stdarg.h>

typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;

#include <xtl.h>

#else

#include <windows.h>

#endif

#include <platform.h>

// enable memory pool usage
#define USE_NEWPOOL
#include <CryMemoryManager.h>

#include "CrtOverrides.h"

#if defined _DEBUG && defined OPENGL
#define DEBUGALLOC
#endif

/////////////////////////////////////////////////////////////////////////////
// STL //////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <list>
#include <map>
#include <hash_map>
#include <set>
#include <string>
#include <algorithm>

typedef const char*			cstr;

#define SIZEOF_ARRAY(arr) (sizeof(arr)/sizeof((arr)[0]))

// Include common headers.
//#include "Common\CryHelpers.h"

//typedef string String;

#ifdef DEBUGALLOC

#include <crtdbg.h>
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_CLIENTBLOCK

// memman
#define   calloc(s,t)       _calloc_dbg(s, t, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)

#endif


#include <list2.h>
#include <Names.h>

#define	MAX_TMU 8

//! Include main interfaces.
#include <ICryPak.h>
#include <IEntitySystem.h>
#include <IProcess.h>
#include <ITimer.h>
#include <ISystem.h>
#include <ILog.h>
#include <IPhysics.h>
#include <IConsole.h>
#include <IRenderer.h>
#include <IStreamEngine.h>
#include <CrySizer.h>

#include "Font.h"
#include "Except.h"

#include <Cry_Math.h>
#include "Cry_Camera.h"
//#include "_Malloc.h"
#include "math.h"
#include "Common/Mkl/Mkl.h"

#include <VertexFormats.h>
#include <CREPolyMesh.h>

#include "Common/Shaders/Shader.h"
//#include "Common/XFile/File.h"
//#include "Common/Image.h"
#include "Common/Shaders/CShader.h"
#include "Common/EvalFuncs.h"
#include "Common/RenderPipeline.h"
#include "Common/Renderer.h"
#include "Common/CPUDetect.h"
#include "Common/Textures/TexMan.h"
#include "Common/Shaders/Parser.h"
#include "Common/SimpleFrameProfiler.h"

// per-frame profilers: collect the infromation for each frame for
// displaying statistics at the beginning of each frame
#define PROFILER(ID,NAME) DECLARE_FRAME_PROFILER(ID,NAME)
#include "Common/FrameProfilers-list.h"
#undef PROFILER

// All handled render elements (except common ones included in "RendElement.h")
#include "Common/RendElements/CREBeam.h"
#include "Common/RendElements/CREPrefabGeom.h"
#include "Common/RendElements/CREClientPoly.h"
#include "Common/RendElements/CREClientPoly2D.h"
#include "Common/RendElements/CREParticleSpray.h"
#include "Common/RendElements/CREFlares.h"
#include "Common/RendElements/CREPolyBlend.h"
#include "Common/RendElements/CRESkyZone.h"
#include "Common/RendElements/CREOcean.h"
#include "Common/RendElements/CREGlare.h"
#include "Common/RendElements/CRETempMesh.h"


#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

/*-----------------------------------------------------------------------------
	Vector transformations.
-----------------------------------------------------------------------------*/

//
// Transformations in optimized assembler format.
// An adaption of Michael Abrash' optimal transformation code.
//
#if DO_ASM
_inline void ASMTransformPoint(const SCoord &Coords, const Vec3d& InVector, Vec3d& OutVector)
{
  // SCoords is a structure of 4 vectors: Origin, X, Y, Z
  //				 	  x  y  z
  // Vector  Origin;   0  4  8
  // Vector  XAxis;   12 16 20
  // Vector  YAxis;   24 28 32
  // Vector  ZAxis;   36 40 44
  //
  //	task:	VectorSubtract(InVector, Coords.org, Temp);
  //            Outvector[0] = DotProduct(Temp, Coords.rot[0]);
  //            Outvector[1] = DotProduct(Temp, Coords.rot[1]);
  //            Outvector[2] = DotProduct(Temp, Coords.rot[2]);

  //
  // About 33 cycles on a Pentium.
  //
  __asm
  {
    mov     esi,[InVector]
    mov     edx,[Coords]
    mov     edi,[OutVector]

    // get source
    fld     dword ptr [esi+0]
    fld     dword ptr [esi+4]
    fld     dword ptr [esi+8]    // z y x
    fxch    st(2)                // xyz

    // subtract origin
    fsub    dword ptr [edx + 0]  // xyz
    fxch    st(1)
    fsub    dword ptr [edx + 4]  // yxz
    fxch    st(2)
    fsub    dword ptr [edx + 8]  // zxy
    fxch    st(1)                // X Z Y

    // triplicate X for  transforming
    fld     st(0)                // X X   Z Y
    fmul    dword ptr [edx+12]   // Xx X Z Y
    fld     st(1)                // X Xx X  Z Y
    fmul    dword ptr [edx+24]   // Xy Xx X  Z Y
    fxch    st(2)
    fmul    dword ptr [edx+36]   // Xz Xx Xy  Z  Y
    fxch    st(4)                // Y  Xx Xy  Z  Xz

    fld     st(0)                // Y Y    Xx Xy Z Xz
    fmul    dword ptr [edx+16]
    fld     st(1)                // Y Yx Y    Xx Xy Z Xz
    fmul    dword ptr [edx+28]
    fxch    st(2)                // Y  Yx Yy   Xx Xy Z Xz
    fmul    dword ptr [edx+40]	 // Yz Yx Yy   Xx Xy Z Xz
    fxch    st(1)                // Yx Yz Yy   Xx Xy Z Xz

    faddp   st(3),st(0)	         // Yz Yy  XxYx   Xy Z  Xz
    faddp   st(5),st(0)          // Yy  XxYx   Xy Z  XzYz
    faddp   st(2),st(0)          // XxYx  XyYy Z  XzYz
    fxch    st(2)                // Z     XyYy XxYx XzYz

    fld     st(0)                //  Z  Z     XyYy XxYx XzYz
    fmul    dword ptr [edx+20]
    fld     st(1)                //  Z  Zx Z  XyYy XxYx XzYz
    fmul    dword ptr [edx+32]
    fxch    st(2)                //  Z  Zx Zy
    fmul    dword ptr [edx+44]	 //  Zz Zx Zy XyYy XxYx XzYz
    fxch    st(1)                //  Zx Zz Zy XyYy XxYx XzYz

    faddp   st(4),st(0)          //  Zz Zy XyYy  XxYxZx  XzYz
    faddp   st(4),st(0)	         //  Zy XyYy     XxYxZx  XzYzZz
    faddp   st(1),st(0)          //  XyYyZy      XxYxZx  XzYzZz

    fstp    dword ptr [edi+4]
    fstp    dword ptr [edi+0]
    fstp    dword ptr [edi+8]
  }
}
#endif

#if DO_ASM
_inline void ASMTransformVector(const SCoord &Coords, const Vec3d& InVector, Vec3d& OutVector)
{
  __asm
  {
    mov     esi,[InVector]
    mov     edx,[Coords]
    mov     edi,[OutVector]

    // get source
    fld     dword ptr [esi+0]
    fld     dword ptr [esi+4]
    fxch    st(1)
    fld     dword ptr [esi+8]      // z x y
    fxch    st(1)                  // x z y

    // triplicate X for  transforming
    fld     st(0)                  // X X   Z Y
    fmul    dword ptr [edx+12]     // Xx X Z Y
    fld     st(1)                  // X Xx X  Z Y
    fmul    dword ptr [edx+24]     // Xy Xx X  Z Y
    fxch    st(2)
    fmul    dword ptr [edx+36]     // Xz Xx Xy  Z  Y
    fxch    st(4)                  // Y  Xx Xy  Z  Xz

    fld     st(0)                  // Y Y    Xx Xy Z Xz
    fmul    dword ptr [edx+16]
    fld     st(1)                  // Y Yx Y    Xx Xy Z Xz
    fmul    dword ptr [edx+28]
    fxch    st(2)                  // Y  Yx Yy   Xx Xy Z Xz
    fmul    dword ptr [edx+40]     // Yz Yx Yy   Xx Xy Z Xz
    fxch    st(1)                  // Yx Yz Yy   Xx Xy Z Xz

    faddp   st(3),st(0)            // Yz Yy  XxYx   Xy Z  Xz
    faddp   st(5),st(0)            // Yy  XxYx   Xy Z  XzYz
    faddp   st(2),st(0)            // XxYx  XyYy Z  XzYz
    fxch    st(2)                  // Z     XyYy XxYx XzYz

    fld     st(0)                  //  Z  Z     XyYy XxYx XzYz
    fmul    dword ptr [edx+20]
    fld     st(1)                  //  Z  Zx Z  XyYy XxYx XzYz
    fmul    dword ptr [edx+32]
    fxch    st(2)                  //  Z  Zx Zy
    fmul    dword ptr [edx+44]	   //  Zz Zx Zy XyYy XxYx XzYz
    fxch    st(1)                  //  Zx Zz Zy XyYy XxYx XzYz

    faddp   st(4),st(0)            //  Zz Zy XyYy  XxYxZx  XzYz
    faddp   st(4),st(0)	           //  Zy XyYy     XxYxZx  XzYzZz
    faddp   st(1),st(0)            //  XyYyZy      XxYxZx  XzYzZz

    fstp    dword ptr [edi+4]
    fstp    dword ptr [edi+0]
    fstp    dword ptr [edi+8]
  }
}
#endif


//
// Transform a point by a coordinate system, moving
// it by the coordinate system's origin if nonzero.
//
_inline void TransformPoint( const SCoord &Coords, Vec3d& in, Vec3d& out)
{
#if !DO_ASM
  Vec3d Temp;

  Temp = in - Coords.m_Org;
  out[0] = Temp | Coords.m_Vecs[0];
  out[1] = Temp | Coords.m_Vecs[1];
  out[2] = Temp | Coords.m_Vecs[2];
#else
  ASMTransformPoint( Coords, in, out);
#endif
}

//we need a better function-name for this exotic operation
//there already is a "TransformPoint" in Cry_Matrix.h
_inline void TransformPoint( const Matrix44 &Matr, Vec3d& inp, Vec3d& outp)
{
  //T_CHANGED_BY_IVO
	//Vec3d Temp = inp - *(Vec3d *)&Matr.m_values[3][0];
	Vec3d Temp = inp - Matr.GetTranslation();

	//T_CHANGED_BY_IVO
	//outp.x = Temp | *(Vec3d *)&Matr.m_values[0][0];
  //outp.y = Temp | *(Vec3d *)&Matr.m_values[1][0];
  //outp.z = Temp | *(Vec3d *)&Matr.m_values[2][0];
	outp.x = Temp | Matr.GetOrtX();
	outp.y = Temp | Matr.GetOrtY();
	outp.z = Temp | Matr.GetOrtZ();

}


//
// Transform a directional vector by a coordinate system.
// Ignore's the coordinate system's origin.
//
_inline void TransformVector( SCoord Coords, Vec3d& in, Vec3d& out )
{
#if !DO_ASM
  Vec3d Temp;

  Temp = in;
  out[0] = Temp | Coords.m_Vecs[0];
  out[1] = Temp | Coords.m_Vecs[1];
  out[2] = Temp | Coords.m_Vecs[2];
#else
  ASMTransformVector( Coords, in, out);
#endif
}

_inline void TransformVec_ViewProj(Vec3d& v, Matrix44 viewmatr, Matrix44 projmatr, vec4_t vv, vec4_t pv)
{
  int i;

  for (i=0; i<4; i++)
  {
    vv[i] = viewmatr[0][i]*v[0] + viewmatr[1][i]*v[1] + viewmatr[2][i]*v[2] + viewmatr[3][i];
  }

  for (i=0; i<4; i++)
  {
    pv[i] = projmatr[0][i]*vv[0] + projmatr[1][i]*vv[1] + projmatr[2][i]*vv[2] + projmatr[3][i]*vv[3];
  }
}

_inline void ProjectPoint(vec4_t pv, Vec3d& v3d, vec2_t v2d)
{
  v3d[0] = pv[0] / pv[3];
  v3d[1] = pv[1] / pv[3];
  v3d[2] = (pv[2] + pv[3]) / (pv[2] + pv[3] + pv[3]);

  v2d[0] = (float)QRound((v3d[0] + 1) * gRenDev->GetWidth()  * 0.5f);
  v2d[1] = (float)QRound((v3d[1] + 1) * gRenDev->GetHeight() * 0.5f);

  v2d[0] = (float)QRound(v2d[0]);
  v2d[1] = (float)QRound(v2d[1]);
}

_inline void TransformVector(Vec3d& out, Vec3d& in, Matrix44& m)
{
	//T_CHANGED_BY_IVO
  //out.x = in.x * m.m_values[0][0] + in.y * m.m_values[1][0] + in.z * m.m_values[2][0];
  //out.y = in.x * m.m_values[0][1] + in.y * m.m_values[1][1] + in.z * m.m_values[2][1];
  //out.z = in.x * m.m_values[0][2] + in.y * m.m_values[1][2] + in.z * m.m_values[2][2];

	out.x = in.x * m(0,0) + in.y * m(1,0) + in.z * m(2,0);
	out.y = in.x * m(0,1) + in.y * m(1,1) + in.z * m(2,1);
	out.z = in.x * m(0,2) + in.y * m(1,2) + in.z * m(2,2);
}

_inline void TransformPosition(Vec3d& out, Vec3d& in, Matrix44& m)
{

	//T_CHANGED_BY_IVO
	//out.x = in.x * m.m_values[0][0] + in.y * m.m_values[1][0] + in.z * m.m_values[2][0] + m.m_values[3][0];
  //out.y = in.x * m.m_values[0][1] + in.y * m.m_values[1][1] + in.z * m.m_values[2][1] + m.m_values[3][1];
  //out.z = in.x * m.m_values[0][2] + in.y * m.m_values[1][2] + in.z * m.m_values[2][2] + m.m_values[3][2];
	TransformVector (out, in, m);
	out += m.GetTranslation();
}


inline Plane TransformPlaneByUsingAdjointT( const Matrix44& M, const Matrix44& TA, const Plane plSrc)
{
	//CHANGED_BY_IVO
	//Vec3d newNorm = TA.TransformVector(plSrc.n);
	Vec3d newNorm = GetTransposed44(TA)*(plSrc.n);

	newNorm.Normalize();

  if(M.Determinant() < 0.f) newNorm *= -1;

	Plane plane;
	plane.Set(newNorm, M.TransformPointOLD(plSrc.n * plSrc.d) | newNorm);

  return plane;
}

inline Matrix44 TransposeAdjoint(const Matrix44& M)
{
  Matrix44 ta;

  ta(0,0) = M(1,1) * M(2,2) - M(1,2) * M(2,1);
  ta(0,1) = M(1,2) * M(2,0) - M(1,0) * M(2,2);
  ta(0,2) = M(1,0) * M(2,1) - M(1,1) * M(2,0);
  ta(0,3) = 0.f;

  ta(1,0) = M(2,1) * M(0,2) - M(2,2) * M(0,1);
  ta(1,1) = M(2,2) * M(0,0) - M(2,0) * M(0,2);
  ta(1,2) = M(2,0) * M(0,1) - M(2,1) * M(0,0);
  ta(1,3) = 0.f;

  ta(2,0) = M(0,1) * M(1,2) - M(0,2) * M(1,1);
  ta(2,1) = M(0,2) * M(1,0) - M(0,0) * M(1,2);
  ta(2,2) = M(0,0) * M(1,1) - M(0,1) * M(1,0);
  ta(2,3) = 0.f;

  ta(3,0) = 0.f;
  ta(3,1) = 0.f;
  ta(3,2) = 0.f;
  ta(3,1) = 1.f;




  return ta;
}

inline Plane TransformPlane( const Matrix44& M, const Plane& plSrc)
{
  Matrix44 tmpTA = TransposeAdjoint(M);
  return TransformPlaneByUsingAdjointT(M, tmpTA, plSrc);
}

// Homogeneous plane transform.
inline Plane TransformPlane2(const Matrix44& m, const Plane& src )
{
  Plane plDst;

  float v0=src.n.x, v1=src.n.y, v2=src.n.z, v3=src.d;
  plDst.n.x = v0 * m[0][0] + v1 * m[0][1] + v2 * m[0][2] + v3 * m[0][3];
  plDst.n.y = v0 * m[1][0] + v1 * m[1][1] + v2 * m[1][2] + v3 * m[1][3];
  plDst.n.z = v0 * m[2][0] + v1 * m[2][1] + v2 * m[2][2] + v3 * m[2][3];

	plDst.d = v0 * m[3][0] + v1 * m[3][1] + v2 * m[3][2] + v3 * m[3][3];

  return plDst;
}
inline Plane TransformPlane2_NoTrans(const Matrix44& m, const Plane& src )
{
  Plane plDst;

  float v0=src.n.x, v1=src.n.y, v2=src.n.z;
  plDst.n.x = v0 * m[0][0] + v1 * m[0][1] + v2 * m[0][2];
  plDst.n.y = v0 * m[1][0] + v1 * m[1][1] + v2 * m[1][2];
  plDst.n.z = v0 * m[2][0] + v1 * m[2][1] + v2 * m[2][2];

  plDst.d = src.d;

  return plDst;
}

inline Plane TransformPlane2Transposed(const Matrix44& m, const Plane& src )
{
  Plane plDst;

  float v0=src.n.x, v1=src.n.y, v2=src.n.z, v3=src.d;
  plDst.n.x = v0 * m[0][0] + v1 * m[1][0] + v2 * m[2][0] + v3 * m[3][0];
  plDst.n.y = v0 * m[0][1] + v1 * m[1][1] + v2 * m[2][1] + v3 * m[3][1];
  plDst.n.z = v0 * m[0][2] + v1 * m[2][1] + v2 * m[2][2] + v3 * m[3][2];

  plDst.d   = v0 * m[0][3] + v1 * m[1][3] + v2 * m[2][3] + v3 * m[3][3];

  return plDst;
}

//===============================================================================================

_inline int CullBoxByPlane (float *Mins, float *Maxs, SPlane *p)
{
  float dist1, dist2;
  int sides;

// fast axial cases
  if (p->m_Type < 3)
  {
    return (p->m_Dist <= Mins[p->m_Type]) ? 1 : (p->m_Dist >= Maxs[p->m_Type]) ? 2 : 3;
  }

// general case
  switch (p->m_SignBits)
  {
    case 0:
      dist1 = p->m_Normal[0]*Maxs[0] + p->m_Normal[1]*Maxs[1] + p->m_Normal[2]*Maxs[2];
      dist2 = p->m_Normal[0]*Mins[0] + p->m_Normal[1]*Mins[1] + p->m_Normal[2]*Mins[2];
      break;

    case 1:
      dist1 = p->m_Normal[0]*Mins[0] + p->m_Normal[1]*Maxs[1] + p->m_Normal[2]*Maxs[2];
      dist2 = p->m_Normal[0]*Maxs[0] + p->m_Normal[1]*Mins[1] + p->m_Normal[2]*Mins[2];
      break;

    case 2:
      dist1 = p->m_Normal[0]*Maxs[0] + p->m_Normal[1]*Mins[1] + p->m_Normal[2]*Maxs[2];
      dist2 = p->m_Normal[0]*Mins[0] + p->m_Normal[1]*Maxs[1] + p->m_Normal[2]*Mins[2];
      break;

    case 3:
      dist1 = p->m_Normal[0]*Mins[0] + p->m_Normal[1]*Mins[1] + p->m_Normal[2]*Maxs[2];
      dist2 = p->m_Normal[0]*Maxs[0] + p->m_Normal[1]*Maxs[1] + p->m_Normal[2]*Mins[2];
      break;

    case 4:
      dist1 = p->m_Normal[0]*Maxs[0] + p->m_Normal[1]*Maxs[1] + p->m_Normal[2]*Mins[2];
      dist2 = p->m_Normal[0]*Mins[0] + p->m_Normal[1]*Mins[1] + p->m_Normal[2]*Maxs[2];
      break;

    case 5:
      dist1 = p->m_Normal[0]*Mins[0] + p->m_Normal[1]*Maxs[1] + p->m_Normal[2]*Mins[2];
      dist2 = p->m_Normal[0]*Maxs[0] + p->m_Normal[1]*Mins[1] + p->m_Normal[2]*Maxs[2];
      break;

    case 6:
      dist1 = p->m_Normal[0]*Maxs[0] + p->m_Normal[1]*Mins[1] + p->m_Normal[2]*Mins[2];
      dist2 = p->m_Normal[0]*Mins[0] + p->m_Normal[1]*Maxs[1] + p->m_Normal[2]*Maxs[2];
      break;

    case 7:
      dist1 = p->m_Normal[0]*Mins[0] + p->m_Normal[1]*Mins[1] + p->m_Normal[2]*Mins[2];
      dist2 = p->m_Normal[0]*Maxs[0] + p->m_Normal[1]*Maxs[1] + p->m_Normal[2]*Maxs[2];
      break;

    default:
      dist1 = dist2 = 0;  // shut up compiler
      ASSERT( 1 );
      break;
  }

  sides = 0;
  if (dist1 >= p->m_Dist)
    sides = 1;
  if (dist2 < p->m_Dist)
    sides |= 2;

  //ASSERT( sides != 0 );

  return sides;
}

//===============================================================================================

// Interfaces from the Game
extern ILog     *iLog;
extern IConsole *iConsole;
extern ITimer   *iTimer;
extern ISystem  *iSystem;
extern int *pTest_int;
extern IPhysicalWorld *pIPhysicalWorld;

#define MAX_PATH_LENGTH	512

inline void _text_to_log(char * format, ...)
{
  char buffer[MAX_PATH_LENGTH];
  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

	iLog->Log(buffer);
  if (gRenDev->CV_r_log == 3)
    gRenDev->Logv(SRendItem::m_RecurseLevel, buffer);
}

inline void _text_to_logPlus(char * format, ...)
{
  char buffer[MAX_PATH_LENGTH];
  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

	iLog->LogPlus(buffer);
  if (gRenDev->CV_r_log == 3)
    gRenDev->Logv(SRendItem::m_RecurseLevel, buffer);
}

inline void _UpdateLoadingScreen(const char * format, ...)
{
  if(format)
  {
    char buffer[MAX_PATH_LENGTH];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

	  iLog->Log(buffer);
    if (gRenDev->CV_r_log == 3)
      gRenDev->Logv(SRendItem::m_RecurseLevel, buffer);
  }

	//iConsole->Update();
	//gRenDev->BeginFrame();
	//iConsole->Draw();
	//gRenDev->Update();
}

inline void _UpdateLoadingScreenPlus(const char * format, ...)
{
  if(format)
  {
    char buffer[MAX_PATH_LENGTH];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

	  iLog->Log(buffer);
    if (gRenDev->CV_r_log == 3)
      gRenDev->Logv(SRendItem::m_RecurseLevel, buffer);
  }

	iConsole->Update();
	gRenDev->BeginFrame();
	iConsole->Draw();
	gRenDev->Update();
}

_inline char * Cry_strdup(const char * str)
{
  char *memory;

  if (!str)
    return(NULL);

  memory = (char *)malloc(strlen(str) + 1);
  if (memory)
    return(strcpy(memory,str));

  return(NULL);
}

const char* GetExtension (const char *in);
void StripExtension (const char *in, char *out);
void AddExtension (char *path, char *extension);
void ConvertDOSToUnixName( char *dst, const char *src );
void ConvertUnixToDosName( char *dst, const char *src );
void UsePath (char *name, char *path, char *dst);

#define Vector2Copy(a,b) {b[0]=a[0];b[1]=a[1];}

//==================================================================
// Profiling

inline DWORD sCycles()
{
  uint L;
#ifndef PS2
  __asm
  {
    xor   eax,eax	          // Required so that VC++ realizes EAX is modified.
      _emit 0x0F		          // RDTSC  -  Pentium+ time stamp register to EDX:EAX.
      _emit 0x31		          // Use only 32 bits in EAX - even a Ghz cpu would have a 4+ sec period.
      mov   [L],eax           // Save low value.
      xor   edx,edx	          // Required so that VC++ realizes EDX is modified.
  }
#else
  L = 0;
#endif
  return L;
}

inline double sCycles2()
{
  uint L,H;
#ifndef PS2
  __asm
  {
    xor   eax,eax	// Required so that VC++ realizes EAX is modified.
      xor   edx,edx	// Required so that VC++ realizes EDX is modified.
      _emit 0x0F		// RDTSC  -  Pentium+ time stamp register to EDX:EAX.
      _emit 0x31		// Use only 32 bits in EAX - even a Ghz cpu would have a 4+ sec period.
      mov   [L],eax   // Save low value.
      mov   [H],edx   // Save high value.
  }
#else
  L = H = 0;
#endif
  return ((DOUBLE)L +  4294967296.0 * (DOUBLE)H);
}

#define FP_BITS(fp) (*(DWORD *)&(fp))

_inline float C_sqrt_tab(float n)
{

  if (FP_BITS(n) == 0)
    return 0.0;                 // check for square root of 0

  FP_BITS(n) = gRenDev->fast_sqrt_table[(FP_BITS(n) >> 8) & 0xFFFF] | ((((FP_BITS(n) - 0x3F800000) >> 1) + 0x3F800000) & 0x7F800000);

  return n;
}

//=========================================================================================

//
// Memory copy.
//
#if DO_ASM
#define DEFINED_cryMemcpy
/******************************************************************************

 Copyright (c) 2001 Advanced Micro Devices, Inc.

 LIMITATION OF LIABILITY:  THE MATERIALS ARE PROVIDED *AS IS* WITHOUT ANY
 EXPRESS OR IMPLIED WARRANTY OF ANY KIND INCLUDING WARRANTIES OF MERCHANTABILITY,
 NONINFRINGEMENT OF THIRD-PARTY INTELLECTUAL PROPERTY, OR FITNESS FOR ANY
 PARTICULAR PURPOSE.  IN NO EVENT SHALL AMD OR ITS SUPPLIERS BE LIABLE FOR ANY
 DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF PROFITS,
 BUSINESS INTERRUPTION, LOSS OF INFORMATION) ARISING OUT OF THE USE OF OR
 INABILITY TO USE THE MATERIALS, EVEN IF AMD HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGES.  BECAUSE SOME JURISDICTIONS PROHIBIT THE EXCLUSION OR LIMITATION
 OF LIABILITY FOR CONSEQUENTIAL OR INCIDENTAL DAMAGES, THE ABOVE LIMITATION MAY
 NOT APPLY TO YOU.

 AMD does not assume any responsibility for any errors which may appear in the
 Materials nor any responsibility to support or update the Materials.  AMD retains
 the right to make changes to its test specifications at any time, without notice.

 NO SUPPORT OBLIGATION: AMD is not obligated to furnish, support, or make any
 further information, software, technical information, know-how, or show-how
 available to you.

 So that all may benefit from your experience, please report  any  problems
 or  suggestions about this software to 3dsdk.support@amd.com

 AMD Developer Technologies, M/S 585
 Advanced Micro Devices, Inc.
 5900 E. Ben White Blvd.
 Austin, TX 78741
 3dsdk.support@amd.com
******************************************************************************/

/*****************************************************************************
MEMCPY_AMD.CPP
******************************************************************************/

// Very optimized memcpy() routine for AMD Athlon and Duron family.
// This code uses any of FOUR different basic copy methods, depending
// on the transfer size.
// NOTE:  Since this code uses MOVNTQ (also known as "Non-Temporal MOV" or
// "Streaming Store"), and also uses the software prefetch instructions,
// be sure you're running on Athlon/Duron or other recent CPU before calling!

#define TINY_BLOCK_COPY 64       // upper limit for movsd type copy
// The smallest copy uses the X86 "movsd" instruction, in an optimized
// form which is an "unrolled loop".

#define IN_CACHE_COPY 64 * 1024  // upper limit for movq/movq copy w/SW prefetch
// Next is a copy that uses the MMX registers to copy 8 bytes at a time,
// also using the "unrolled loop" optimization.   This code uses
// the software prefetch instruction to get the data into the cache.

#define UNCACHED_COPY 197 * 1024 // upper limit for movq/movntq w/SW prefetch
// For larger blocks, which will spill beyond the cache, it's faster to
// use the Streaming Store instruction MOVNTQ.   This write instruction
// bypasses the cache and writes straight to main memory.  This code also
// uses the software prefetch instruction to pre-read the data.
// USE 64 * 1024 FOR THIS VALUE IF YOU'RE ALWAYS FILLING A "CLEAN CACHE"

#define BLOCK_PREFETCH_COPY  infinity // no limit for movq/movntq w/block prefetch
#define CACHEBLOCK 80h // number of 64-byte blocks (cache lines) for block prefetch
// For the largest size blocks, a special technique called Block Prefetch
// can be used to accelerate the read operations.   Block Prefetch reads
// one address per cache line, for a series of cache lines, in a short loop.
// This is faster than using software prefetch.  The technique is great for
// getting maximum read bandwidth, especially in DDR memory systems.

// Inline assembly syntax for use with Visual C++
inline void cryMemcpy( void* Dst, const void* Src, INT Count )
{
	if( gRenDev->m_Cpu->mCpu[0].mFeatures & CFI_MMX )
  {
	  __asm
	  {
		  mov		ecx, [Count]	; number of bytes to copy
		  mov		edi, [Dst]		; destination
		  mov		esi, [Src]		; source
		  mov		ebx, ecx		; keep a copy of count

		  cld
		  cmp		ecx, TINY_BLOCK_COPY
		  jb		$memcpy_ic_3	; tiny? skip mmx copy

		  cmp		ecx, 32*1024		; don't align between 32k-64k because
		  jbe		$memcpy_do_align	;  it appears to be slower
		  cmp		ecx, 64*1024
		  jbe		$memcpy_align_done
	  $memcpy_do_align:
		  mov		ecx, 8			; a trick that's faster than rep movsb...
		  sub		ecx, edi		; align destination to qword
		  and		ecx, 111b		; get the low bits
		  sub		ebx, ecx		; update copy count
		  neg		ecx				; set up to jump into the array
		  add		ecx, offset $memcpy_align_done
		  jmp		ecx				; jump to array of movsb's

	  align 4
		  movsb
		  movsb
		  movsb
		  movsb
		  movsb
		  movsb
		  movsb
		  movsb

	  $memcpy_align_done:			; destination is dword aligned
		  mov		ecx, ebx		; number of bytes left to copy
		  shr		ecx, 6			; get 64-byte block count
		  jz		$memcpy_ic_2	; finish the last few bytes

		  cmp		ecx, IN_CACHE_COPY/64	; too big 4 cache? use uncached copy
		  jae		$memcpy_uc_test

	  // This is small block copy that uses the MMX registers to copy 8 bytes
	  // at a time.  It uses the "unrolled loop" optimization, and also uses
	  // the software prefetch instruction to get the data into the cache.
	  align 16
	  $memcpy_ic_1:			; 64-byte block copies, in-cache copy

		  prefetchnta [esi + (200*64/34+192)]		; start reading ahead

		  movq	mm0, [esi+0]	; read 64 bits
		  movq	mm1, [esi+8]
		  movq	[edi+0], mm0	; write 64 bits
		  movq	[edi+8], mm1	;    note:  the normal movq writes the
		  movq	mm2, [esi+16]	;    data to cache; a cache line will be
		  movq	mm3, [esi+24]	;    allocated as needed, to store the data
		  movq	[edi+16], mm2
		  movq	[edi+24], mm3
		  movq	mm0, [esi+32]
		  movq	mm1, [esi+40]
		  movq	[edi+32], mm0
		  movq	[edi+40], mm1
		  movq	mm2, [esi+48]
		  movq	mm3, [esi+56]
		  movq	[edi+48], mm2
		  movq	[edi+56], mm3

		  add		esi, 64			; update source pointer
		  add		edi, 64			; update destination pointer
		  dec		ecx				; count down
		  jnz		$memcpy_ic_1	; last 64-byte block?

	  $memcpy_ic_2:
		  mov		ecx, ebx		; has valid low 6 bits of the byte count
	  $memcpy_ic_3:
		  shr		ecx, 2			; dword count
		  and		ecx, 1111b		; only look at the "remainder" bits
		  neg		ecx				; set up to jump into the array
		  add		ecx, offset $memcpy_last_few
		  jmp		ecx				; jump to array of movsd's

	  $memcpy_uc_test:
		  cmp		ecx, UNCACHED_COPY/64	; big enough? use block prefetch copy
		  jae		$memcpy_bp_1

	  $memcpy_64_test:
		  or		ecx, ecx		; tail end of block prefetch will jump here
		  jz		$memcpy_ic_2	; no more 64-byte blocks left

	  // For larger blocks, which will spill beyond the cache, it's faster to
	  // use the Streaming Store instruction MOVNTQ.   This write instruction
	  // bypasses the cache and writes straight to main memory.  This code also
	  // uses the software prefetch instruction to pre-read the data.
	  align 16
	  $memcpy_uc_1:				; 64-byte blocks, uncached copy

		  prefetchnta [esi + (200*64/34+192)]		; start reading ahead

		  movq	mm0,[esi+0]		; read 64 bits
		  add		edi,64			; update destination pointer
		  movq	mm1,[esi+8]
		  add		esi,64			; update source pointer
		  movq	mm2,[esi-48]
		  movntq	[edi-64], mm0	; write 64 bits, bypassing the cache
		  movq	mm0,[esi-40]	;    note: movntq also prevents the CPU
		  movntq	[edi-56], mm1	;    from READING the destination address
		  movq	mm1,[esi-32]	;    into the cache, only to be over-written
		  movntq	[edi-48], mm2	;    so that also helps performance
		  movq	mm2,[esi-24]
		  movntq	[edi-40], mm0
		  movq	mm0,[esi-16]
		  movntq	[edi-32], mm1
		  movq	mm1,[esi-8]
		  movntq	[edi-24], mm2
		  movntq	[edi-16], mm0
		  dec		ecx
		  movntq	[edi-8], mm1
		  jnz		$memcpy_uc_1	; last 64-byte block?

		  jmp		$memcpy_ic_2		; almost done

	  // For the largest size blocks, a special technique called Block Prefetch
	  // can be used to accelerate the read operations.   Block Prefetch reads
	  // one address per cache line, for a series of cache lines, in a short loop.
	  // This is faster than using software prefetch.  The technique is great for
	  // getting maximum read bandwidth, especially in DDR memory systems.
	  $memcpy_bp_1:			; large blocks, block prefetch copy

		  cmp		ecx, CACHEBLOCK			; big enough to run another prefetch loop?
		  jl		$memcpy_64_test			; no, back to regular uncached copy

		  mov		eax, CACHEBLOCK / 2		; block prefetch loop, unrolled 2X
		  add		esi, CACHEBLOCK * 64	; move to the top of the block
	  align 16
	  $memcpy_bp_2:
		  mov		edx, [esi-64]		; grab one address per cache line
		  mov		edx, [esi-128]		; grab one address per cache line
		  sub		esi, 128			; go reverse order to suppress HW prefetcher
		  dec		eax					; count down the cache lines
		  jnz		$memcpy_bp_2		; keep grabbing more lines into cache

		  mov		eax, CACHEBLOCK		; now that it's in cache, do the copy
	  align 16
	  $memcpy_bp_3:
		  movq	mm0, [esi   ]		; read 64 bits
		  movq	mm1, [esi+ 8]
		  movq	mm2, [esi+16]
		  movq	mm3, [esi+24]
		  movq	mm4, [esi+32]
		  movq	mm5, [esi+40]
		  movq	mm6, [esi+48]
		  movq	mm7, [esi+56]
		  add		esi, 64				; update source pointer
		  movntq	[edi   ], mm0		; write 64 bits, bypassing cache
		  movntq	[edi+ 8], mm1		;    note: movntq also prevents the CPU
		  movntq	[edi+16], mm2		;    from READING the destination address
		  movntq	[edi+24], mm3		;    into the cache, only to be over-written,
		  movntq	[edi+32], mm4		;    so that also helps performance
		  movntq	[edi+40], mm5
		  movntq	[edi+48], mm6
		  movntq	[edi+56], mm7
		  add		edi, 64				; update dest pointer

		  dec		eax					; count down

		  jnz		$memcpy_bp_3		; keep copying
		  sub		ecx, CACHEBLOCK		; update the 64-byte block count
		  jmp		$memcpy_bp_1		; keep processing chunks

	  // The smallest copy uses the X86 "movsd" instruction, in an optimized
	  // form which is an "unrolled loop".   Then it handles the last few bytes.
	  align 4
		  movsd
		  movsd			; perform last 1-15 dword copies
		  movsd
		  movsd
		  movsd
		  movsd
		  movsd
		  movsd
		  movsd
		  movsd			; perform last 1-7 dword copies
		  movsd
		  movsd
		  movsd
		  movsd
		  movsd
		  movsd

	  $memcpy_last_few:		; dword aligned from before movsd's
		  mov		ecx, ebx	; has valid low 2 bits of the byte count
		  and		ecx, 11b	; the last few cows must come home
		  jz		$memcpy_final	; no more, let's leave
		  rep		movsb		; the last 1, 2, or 3 bytes

	  $memcpy_final:
		  emms				; clean up the MMX state
		  sfence				; flush the write buffer
	  //	mov		eax, [dest]	; ret value = destination pointer
	  }
  }
	else
  {
	  __asm
	  {
		  mov		ecx, Count
		  mov		esi, Src
		  mov		edi, Dst
		  mov     ebx, ecx
		  shr     ecx, 2
		  and     ebx, 3
		  rep     movsd
		  mov     ecx, ebx
		  rep     movsb
	  }
  }
}
#else
inline void cryMemcpy( void* Dst, const void* Src, INT Count )
{
  memcpy(Dst, Src, Count);
}
#endif

//=========================================================================================

//
// Normal timing.
//
#define ticks(Timer)   {Timer -= sCycles2();}
#define unticks(Timer) {Timer += sCycles2()+34;}


//=============================================================================


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
