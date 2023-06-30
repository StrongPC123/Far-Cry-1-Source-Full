////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   BrushPoly.cpp
//  Version:     v1.00
//  Created:     8/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History: Based on Andrey's Indoor editor.
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BrushPoly.h"

#include "BrushPlane.h"

//////////////////////////////////////////////////////////////////////////
// SBrushPoly implementation.
//////////////////////////////////////////////////////////////////////////
#define POLY_DOT_EPSILON 0.001f

void SBrushPoly::ClipByPlane(SBrushPlane *p, float eps, SBrushPoly **front, SBrushPoly **back)
{
  float dists[64];
  int   sides[64];
  int   counts[3];
  float dot;
  int   i, j;
  SBrushVert *p1, *p2;
  Vec3d mid;
  SBrushPoly *f, *b;
  
  counts[0] = counts[1] = counts[2] = 0;

  for (i=0; i<m_Pts.size(); i++)
  {
    dot = (m_Pts[i].xyz | p->normal) - p->dist;
    dists[i] = dot;
    if (dot > eps)
      sides[i] = 0;
    else
    if (dot < -eps)
      sides[i] = 1;
    else
      sides[i] = 2;
    counts[sides[i]]++;
  }
  sides[i] = sides[0];
  dists[i] = dists[0];
  
  *front = *back = NULL;

  if (!counts[0])
  {
    *back = this;
    return;
  }
  if (!counts[1])
  {
    *front = this;
    return;
  }

  *front = f = new SBrushPoly;
  *back = b = new SBrushPoly;

	// Reserve space for at least 4 points.
	f->m_Pts.reserve(4);
	b->m_Pts.reserve(4);
    
  SBrushVert v;
  for (i=0; i<m_Pts.size(); i++)
  {
    p1 = &m_Pts[i];
    
    if (sides[i] == 2)
    {
      f->m_Pts.push_back(*p1);
      b->m_Pts.push_back(*p1);
      continue;
    }
  
    if (sides[i] == 0)
      f->m_Pts.push_back(*p1);
    if (sides[i] == 1)
      b->m_Pts.push_back(*p1);

    if (sides[i+1] == 2 || sides[i+1] == sides[i])
      continue;
      
    p2 = &m_Pts[(i+1)%m_Pts.size()];
    
    dot = dists[i] / (dists[i]-dists[i+1]);
    for (j=0; j<3; j++)
    {
      if (p->normal[j] == 1)
        mid[j] = p->dist;
      else
      if (p->normal[j] == -1)
        mid[j] = -p->dist;
      else
        mid[j] = p1->xyz[j] + dot*(p2->xyz[j]-p1->xyz[j]);
    }
    v.xyz = mid;
    f->m_Pts.push_back(v);
    b->m_Pts.push_back(v);
  }
}

//////////////////////////////////////////////////////////////////////////
SBrushPoly *SBrushPoly::ClipByPlane(SBrushPlane *split, bool keep)
{
  float dists[64];
  int   sides[64];
  int   counts[3];
  float dot;
  int   i, j;
  SBrushVert *p1, *p2;
  //SBrushPoly *newp;

  counts[0] = counts[1] = counts[2] = 0;

  for (i=0; i<m_Pts.size(); i++)
  {
    dot = (m_Pts[i].xyz | split->normal) - split->dist;
    dists[i] = dot;
    if (dot > POLY_DOT_EPSILON)
      sides[i] = 0;
    else
    if (dot < -POLY_DOT_EPSILON)
      sides[i] = 1;
    else
      sides[i] = 2;
    counts[sides[i]]++;
  }
  sides[i] = sides[0];
  dists[i] = dists[0];

  if (keep && !counts[0] && !counts[1])
    return this;

  if (!counts[0])
  {
    delete this;
    return NULL;
  }
  if (!counts[1])
    return this;

  //newp = new SBrushPoly;
  SBrushVert v;

	static std::vector<SBrushVert> newpoints;
	newpoints.clear();
	newpoints.reserve(16);

  for (i=0; i<m_Pts.size(); i++)
  {
    p1 = &m_Pts[i];

    if (sides[i] == 2)
    {
      //newp->m_Pts.push_back(*p1);
			newpoints.push_back(*p1);
      continue;
    }

    if (sides[i] == 0)
      //newp->m_Pts.push_back(*p1);
			newpoints.push_back(*p1);

    if (sides[i+1] == 2 || sides[i+1] == sides[i])
      continue;

    p2 = &m_Pts[(i+1)%m_Pts.size()];

    dot = dists[i] / (dists[i]-dists[i+1]);
    for (j=0; j<3 ; j++)
    {
      if (split->normal[j] == 1)
        v.xyz[j] = (float)split->dist;
      else
      if (split->normal[j] == -1)
        v.xyz[j] = -(float)split->dist;
      else
        v.xyz[j] = p1->xyz[j] + dot*(p2->xyz[j]-p1->xyz[j]);
    }
    for (j=0; j<2; j++)
    {
      if (split->normal[j] == 1)
        v.st[j] = (float)split->dist;
      else
      if (split->normal[j] == -1)
        v.st[j] = -(float)split->dist;
      else
        v.st[j] = p1->st[j] + dot*(p2->st[j]-p1->st[j]);
    }
    //newp->m_Pts.push_back(v);
		newpoints.push_back(v);
  }

	m_Pts = newpoints;

  //delete this;

  //return newp;
	return this;
}

//////////////////////////////////////////////////////////////////////////
void SBrushPoly::CalcBounds(Vec3d& mins, Vec3d& maxs)
{
  mins=SetMaxBB();
  maxs=SetMinBB();

  for (int i=0; i<m_Pts.size(); i++)
  {
    AddToBounds(m_Pts[i].xyz, mins, maxs);
  }
}

//////////////////////////////////////////////////////////////////////////
void SBrushPoly::MakePlane(SBrushPlane *pl)
{
  Vec3d v1 = m_Pts[1].xyz - m_Pts[0].xyz;
  Vec3d v2 = m_Pts[2].xyz - m_Pts[0].xyz;
  pl->normal = v2 ^ v1;
  pl->normal.Normalize();
  pl->dist = m_Pts[0].xyz | pl->normal;
}

//////////////////////////////////////////////////////////////////////////
float SBrushPoly::Area()
{
  int   i;
  Vec3d  d1, d2, cross;
  float total;
  
  total = 0;
  for (i=2; i<m_Pts.size(); i++)
  {
    d1 = m_Pts[i-1].xyz - m_Pts[0].xyz;
    d2 = m_Pts[i].xyz - m_Pts[0].xyz;
    cross = d1 ^ d2;
    total += 0.5 * cross.Length();
  }
  return total;
}