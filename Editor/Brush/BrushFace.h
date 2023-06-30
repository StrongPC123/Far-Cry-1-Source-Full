////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushface.h
//  Version:     v1.00
//  Created:     9/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History: Based on Andrey's Indoor editor.
//
////////////////////////////////////////////////////////////////////////////

#ifndef __brushface_h__
#define __brushface_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "BrushPlane.h"

// forward declarations.
class CREPrefabGeom;
class CRendElement;


struct SBrush;
struct SBrushPlane;
struct SBrushPoly;
struct SBrushVert;
class CBrushMtl;

//////////////////////////////////////////////////////////////////////////
/** Texture info of face.
*/
struct SMapTexInfo
{
  char name[64];
  float shift[2];
  float rotate;
  float scale[2];
  int   contents;
  int   flags;
  int   value;

  SMapTexInfo()
  {
		name[0] = 0;
		shift[0] = shift[1] = 0;
    scale[0] = scale[1] = 0.5f;
		rotate = 0;
		contents = 0;
		flags = 0;
		value = 0;
  }
  void SetName(const char *p)
  {
    strcpy(name, p);
  }
};

/** Prefab geometry used instead of texture.
*/
struct SPrefabItem
{
  SPrefabItem()
  {
    m_Geom = NULL;
    m_Model = NULL;
  }
  CREPrefabGeom *m_Geom;
  Matrix44 m_Matrix;
  //CComModel *m_Model;
	IStatObj *m_Model;
};

/** Single Face of brush.
*/
struct SBrushFace
{
  SBrushFace()
  {
    m_Poly = 0;
		m_mtl = 0;
		m_RE = 0;
		m_Prefabs = 0;
    m_color = 0xFFFFFFFF;
		m_fArea = 0;
		ZeroStruct(m_PlanePts);
		m_vCenter(0,0,0);
  }
  ~SBrushFace();
  SBrushFace& operator = (const SBrushFace& f);

	//check if the face intersect the brush
	bool	IsIntersecting(SBrush *Vol);
	//check if the face intersect a plane
	bool	IsIntersecting(const SBrushPlane &Plane);

	float CalcArea();
	void	CalcCenter();

  void ClipPrefab(SPrefabItem *pi);
  void DeletePrefabs();
  void CalcTexCoords( SBrushVert &v );
  void TextureVectors(Vec3d& tVecx, Vec3d& tVecy, float& tShiftx, float& tShifty);
  void BuildPrefabs();
	//! Build Render element for this face using specified world matrix.
  void BuildRE( const Matrix44 &worldTM );
  void BuildPrefabs_r(SBrushPoly *p, Vec3d& Area, Vec3d& Scale);
  void FitTexture(int nX, int nY);

  bool ClipLine(Vec3d& p1, Vec3d& p2); 

  void Draw();

	//! Make plane of brush face.
	void MakePlane();

private:
	void ReleaseRE();

public:
	SBrushPlane	m_Plane;
	SBrushPoly*	m_Poly;
  Vec3d				m_PlanePts[3];
  SMapTexInfo	m_TexInfo;

	//! Color of face.
  COLORREF		m_color;

	//! Material used for this brush.
	CBrushMtl*	m_mtl;
	//! Shader used for this face.
  //IShader* m_shader;
	//! Render element created for this face.
  CRendElement*	m_RE;

	//! List of prefabs.
	SPrefabItem *m_Prefabs;

	//! Stores face center.
	Vec3d	m_vCenter;
	//! Area of face.
	float	m_fArea;
	
	//int		m_dwFlags;
  ///bool m_bTesselated;
};

#endif // __brushface_h__