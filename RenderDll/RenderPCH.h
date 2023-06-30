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

//////////////////////////////////////////////////////////////////////////
// THIS MUST BE AT THE VERY BEGINING OF STDAFX.H FILE.
// Disable STL threading support, (makes STL faster)
//////////////////////////////////////////////////////////////////////////
#define _NOTHREADS
#define _STLP_NO_THREADS
//////////////////////////////////////////////////////////////////////////

//! Include standart headers.

//#define PS2
//#define OPENGL

#if ((PROC_INTEL) && (!defined __GNUC__) && !defined(WIN64))
#define __HAS_SSE__ 1
#endif

#if __HAS_SSE__
#include <fvec.h>

#define CONST_INT32_PS(N, V3, V2, V1, V0) \
	const _MM_ALIGN16 int _##N[] = { V0, V1, V2, V3 }; /*little endian!*/ \
	const F32vec4 N = _mm_load_ps((float*)_##N);
#endif



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

#include <platform.h>

#ifdef LINUX
#include <asm/msr.h>
#endif

#if defined(_AMD64_) && !defined(LINUX)
#include <io.h>
#endif

#endif

// enable memory pool usage
#define USE_NEWPOOL
#include <CryMemoryManager.h>

#include "CrtOverrides.h"

void CRTFreeData(void *pData);
void CRTDeleteArray(void *pData);

//#if defined _DEBUG && defined OPENGL
//#define DEBUGALLOC
//#endif

/////////////////////////////////////////////////////////////////////////////
// STL //////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <list>
#include <map>
#ifdef WIN64
#define hash_map map
#else
#if defined(LINUX)
#include <ext/hash_map>
#else
#include <hash_map>
#endif
#endif
#include <set>
#include <string>
#include <algorithm>

#if !defined(LINUX)
#include <assert.h>
#endif

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
#include <IProcess.h>
#include <ITimer.h>
#include <ISystem.h>
#include <ILog.h>
#include <IConsole.h>
#include <IRenderer.h>
#include <IStreamEngine.h>
#include <CrySizer.h>

#include "Font.h"

#include <Cry_Math.h>
#include "Cry_Camera.h"
//#include "_Malloc.h"
#include "math.h"

#include <VertexFormats.h>

#include "Common/Shaders/Shader.h"
//#include "Common/XFile/File.h"
//#include "Common/Image.h"
#include "Common/Shaders/CShader.h"
#include "Common/EvalFuncs.h"
#include "Common/RenderPipeline.h"
#include "Common/Renderer.h"
#include "Common/Textures/TexMan.h"
#include "Common/Shaders/Parser.h"
#include "Common/SimpleFrameProfiler.h"

// per-frame profilers: collect the infromation for each frame for
// displaying statistics at the beginning of each frame
//#define PROFILER(ID,NAME) DECLARE_FRAME_PROFILER(ID,NAME)
//#include "Common/FrameProfilers-list.h"
//#undef PROFILER

#include "Common/3DUtils.h"

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
#include "Common/RendElements/CREScreenCommon.h"
#include "Common/RendElements/CREHDRProcess.h"

#if !defined(LINUX)
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
/*-----------------------------------------------------------------------------
	Vector transformations.
-----------------------------------------------------------------------------*/

extern int g_CpuFlags;
extern double g_SecondsPerCycle;

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
	Vec3d Temp = inp - Matr.GetTranslationOLD();

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
	out += m.GetTranslationOLD();
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

//////////////////////////////////////////////////////////////////////////
// Report warning to validator.
//////////////////////////////////////////////////////////////////////////
inline void Warning( int flags,const char *filename,const char *format,... )
{
	char buffer[MAX_WARNING_LENGTH];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	iSystem->Warning( VALIDATOR_MODULE_RENDERER,VALIDATOR_WARNING,flags,filename,"%s",buffer );
}

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

_inline void _SetVar(const char *szVarName, int nVal)
{
  ICVar *var = iConsole->GetCVar(szVarName);
  if (var)
    var->Set(nVal);
  else
  {
    assert(0);
  }
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

_inline void HeapCheck()
{
#if !defined(LINUX)
  int Result = _heapchk();
  assert(Result!=_HEAPBADBEGIN);
  assert(Result!=_HEAPBADNODE);
  assert(Result!=_HEAPBADPTR);
  assert(Result!=_HEAPEMPTY);
  assert(Result==_HEAPOK);
#endif
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

#ifdef WIN64
extern "C"
{
	uint64 __rdtsc();
}
#pragma intrinsic(__rdtsc)
#endif

inline DWORD sCycles()
{
  uint L;
#if defined(WIN32) 

#if defined(WIN64)
L = __rdtsc();
#else
  __asm
  {
    xor   eax,eax	          // Required so that VC++ realizes EAX is modified.
      _emit 0x0F		          // RDTSC  -  Pentium+ time stamp register to EDX:EAX.
      _emit 0x31		          // Use only 32 bits in EAX - even a Ghz cpu would have a 4+ sec period.
      mov   [L],eax           // Save low value.
      xor   edx,edx	          // Required so that VC++ realizes EDX is modified.
  }
#endif

#elif defined(LINUX)
	rdtscl( L );
#endif
  return L;
}

inline double sCycles2()
{
#if defined(WIN32) && !defined(WIN64)  
	uint L,H;
	__asm
	{
		xor   eax,eax	// Required so that VC++ realizes EAX is modified.
			xor   edx,edx	// Required so that VC++ realizes EDX is modified.
			_emit 0x0F		// RDTSC  -  Pentium+ time stamp register to EDX:EAX.
			_emit 0x31		// Use only 32 bits in EAX - even a Ghz cpu would have a 4+ sec period.
			mov   [L],eax   // Save low value.
			mov   [H],edx   // Save high value.
	}
	return ((double)L +  4294967296.0 * (double)H);
#else
	return __rdtsc();
#endif

}


//=========================================================================================

_inline void makeProjectionMatrix(float fovy, float aspect, float nearval, float farval, float *m)
{
  float left, right, bottom, top;

  top = nearval * cry_tanf(fovy * M_PI / 360.0f);
  bottom = -top;

  left = bottom * aspect;
  right = top * aspect;


  float x, y, a, b, c, d;
  if ((nearval<=0.0 || farval<=0.0) || (nearval == farval) || (left == right) || (top == bottom))
  {
    iLog->Log("Warning: makeProjectionMatrix: (near or far)\n");
    return;
  }
  x = (2.0f*nearval) / (right-left);
  y = (2.0f*nearval) / (top-bottom);
  a = (right+left) / (right-left);
  b = (top+bottom) / (top-bottom);
  c = -(farval+nearval) / ( farval-nearval);
  d = -(2.0f*farval*nearval) / (farval-nearval);
#define M(row,col)  m[col*4+row]
  M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
  M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
  M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
  M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M
}

//=========================================================================================

//
// Normal timing.
//
#define ticks(Timer)   {Timer -= sCycles2();}
#define unticks(Timer) {Timer += sCycles2()+34;}

//=============================================================================

// the int 3 call for 32-bit version for .l-generated files.
#ifdef WIN64
#define LEX_DBG_BREAK
#else
#define LEX_DBG_BREAK DEBUG_BREAK
#endif

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
