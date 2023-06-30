////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushpoly.h
//  Version:     v1.00
//  Created:     9/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History: Based on Andrey's Indoor editor.
//
////////////////////////////////////////////////////////////////////////////

#ifndef __brushpoly_h__
#define __brushpoly_h__

#if _MSC_VER > 1000
#pragma once
#endif

struct SBrushPlane;

/** Brush vertex used in SBrushPoly structure.
*/
struct SBrushVert
{
  Vec3d xyz;
  float st[2];
};

/** Brush polygon.
*/
struct SBrushPoly
{
  SBrushPoly(int nv) { m_Pts.resize(nv); }
  SBrushPoly() {}
  SBrushPoly& operator = (const SBrushPoly& src) { m_Pts = src.m_Pts; return *this; }

  SBrushPoly *ClipByPlane(SBrushPlane *p, bool keep);
  void ClipByPlane(SBrushPlane *p, float eps, SBrushPoly **front, SBrushPoly **back);
  void CalcBounds(Vec3d& mins, Vec3d& maxs);
  void MakePlane(SBrushPlane *pl);
  float Area();

	std::vector<SBrushVert> m_Pts;
};

#endif // __brushpoly_h__
