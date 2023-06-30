////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushplane.h
//  Version:     v1.00
//  Created:     9/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History: Based on Andrey's Indoor editor.
//
////////////////////////////////////////////////////////////////////////////

#ifndef __brushplane_h__
#define __brushplane_h__

#if _MSC_VER > 1000
#pragma once
#endif

struct SBrushPoly;

struct SBrushPlane
{
  SBrushPlane()
  {
    type = 0;
		dist = 0;
		normal(0,0,0);
  }
	// Makes the palne given 3 points
	void Make( const Vec3d &p1, const Vec3d &p2, const Vec3d &p3)
  {
		normal = (p1 - p2)^(p3 - p2);
		normal.Normalize();
		dist = normal.Dot(p2);
	}

  void CalcTextureAxis(Vec3d& xv, Vec3d& yv, bool bTex);
  SBrushPoly *CreatePoly();
  int Equal(SBrushPlane *b, int flip);
  void Invert(SBrushPlane *p)
  {
    normal = Vec3d(0,0,0) - p->normal;
    dist = -p->dist;
  }
  _inline bool operator==(const SBrushPlane& p) const
  {
    if ( p.normal.x==normal.x && p.normal.y==normal.y && p.normal.z==normal.z && p.dist==dist)
      return true;
    return false;
  }
  _inline bool operator!=(const SBrushPlane& p) const
  {
    if ( p.normal.x==normal.x && p.normal.y==normal.y && p.normal.z==normal.z && p.dist==dist)
      return false;
    return true;
  }

  Vec3d normal;
  float dist;
  int   type;
};

#endif // __brushplane_h__
