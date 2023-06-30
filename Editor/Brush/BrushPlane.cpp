////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushplane.cpp
//  Version:     v1.00
//  Created:     9/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BrushPlane.h"

#include "BrushPoly.h"

// Local variables.
namespace
{
	Vec3d s_baseAxis[] =
	{
		Vec3d(0,0,1), Vec3d(1,0,0), Vec3d(0,-1,0),  Vec3d(1,0,0), Vec3d(0,-1,0),    // floor
		Vec3d(0,0,-1), Vec3d(1,0,0), Vec3d(0,-1,0), Vec3d(1,0,0), Vec3d(0,-1,0),   // ceiling
		Vec3d(1,0,0), Vec3d(0,1,0), Vec3d(0,0,-1),  Vec3d(0,1,0), Vec3d(0,0,-1),   // west wall
		Vec3d(-1,0,0), Vec3d(0,1,0), Vec3d(0,0,-1), Vec3d(0,1,0), Vec3d(0,0,-1),   // east wall
		Vec3d(0,1,0), Vec3d(1,0,0), Vec3d(0,0,-1), Vec3d(1,0,0), Vec3d(0,0,-1),    // south wall
		Vec3d(0,-1,0), Vec3d(1,0,0), Vec3d(0,0,-1), Vec3d(1,0,0), Vec3d(0,0,-1)    // north wall
	};
}

#define PLANE_NORMAL_EPSILON 0.0001f
#define PLANE_DIST_EPSILON	0.001f

//////////////////////////////////////////////////////////////////////////
//
// SBrushPlane implementation
//
//////////////////////////////////////////////////////////////////////////
int SBrushPlane::Equal(SBrushPlane *b, int flip)
{
  Vec3d norm;
  float dst;

  if (flip)
  {
    norm[0] = -b->normal[0];
    norm[1] = -b->normal[1];
    norm[2] = -b->normal[2];
    dst = -b->dist;
  }
  else
  {
    norm[0] = b->normal[0];
    norm[1] = b->normal[1];
    norm[2] = b->normal[2];
    dst = b->dist;
  }
  if (fabs(norm[0]-normal[0]) < PLANE_NORMAL_EPSILON &&
			fabs(norm[1]-normal[1]) < PLANE_NORMAL_EPSILON && 
			fabs(norm[2]-normal[2]) < PLANE_NORMAL_EPSILON &&
			fabs(dst-dist) < PLANE_DIST_EPSILON)
    return true;
  return false;
}

//////////////////////////////////////////////////////////////////////////
void SBrushPlane::CalcTextureAxis(Vec3d& xv, Vec3d& yv, bool bTex)
{
  int   bestaxis;
  float dot,best;
  int   i;
  
  best = 0;
  bestaxis = 0;
  
  for (i=0 ; i<6 ; i++)
  {
    dot = normal | s_baseAxis[i*5];
    if (dot > best)
    {
      best = dot;
      bestaxis = i;
    }
  }

  if (bTex)
  {
    xv = s_baseAxis[bestaxis*5+1];
    yv = s_baseAxis[bestaxis*5+2];
  }
  else
  {
    xv = s_baseAxis[bestaxis*5+3];
    yv = s_baseAxis[bestaxis*5+4];
  }
}

//////////////////////////////////////////////////////////////////////////
SBrushPoly *SBrushPlane::CreatePoly()
{
  int   i, x;
  float max, v;
  Vec3d org, vright, vup;
  SBrushPoly *p;

  max = -99999;
  x = -1;
  for (i=0 ; i<3; i++)
  {
    v = (float)fabs(normal[i]);
    if (v > max)
    {
      x = i;
      max = v;
    }
  }
  if (x==-1)
  {
		CLogFile::WriteLine("Error: SBrushPlane::CreatePoly: no axis found");
    return NULL;
  }

  vup(0,0,0);
  if (x != 2)
    vup = Vec3d(0,0,1);
  else
    vup = Vec3d(1,0,0);

  v = vup | normal;
  vup += normal * -v;
  vup.Normalize();

  org = normal * dist;

  vright = vup ^ normal;

  vup *= 32768.0f;
  vright *= 32768.0f;

// project a really big axis aligned box onto the plane
  p = new SBrushPoly(4);

  p->m_Pts[0].xyz = org - vright;
  p->m_Pts[0].xyz += vup;

  p->m_Pts[1].xyz = org + vright;
  p->m_Pts[1].xyz += vup;

  p->m_Pts[2].xyz = org + vright;
  p->m_Pts[2].xyz -= vup;

  p->m_Pts[3].xyz = org - vright;
  p->m_Pts[3].xyz -= vup;

  return p;
}