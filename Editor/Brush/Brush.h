////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brush.h
//  Version:     v1.00
//  Created:     9/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __brush_h__
#define __brush_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "BrushPlane.h"
#include "BrushPoly.h"
#include "BrushFace.h"

// forward declrations.
struct SBrushPlane;
struct SBrushPoly;

class CEdMesh;
class CBrushIndoor;

#define	DIST_EPSILON	0.01f

//////////////////////////////////////////////////////////////////////////
// Brush flags.
//////////////////////////////////////////////////////////////////////////
enum BrushFlags 
{
	BRF_HIDE = 0x01,
	BRF_FAILMODEL = 0x02,
	BRF_TEMP = 0x04,
	BRF_NODEL = 0x08,
	BRF_LIGHTVOLUME = 0x10,
	BRF_SHOW_AFTER = 0x20,
	BRF_RE_VALID = 0x40, //!< Set if render elements are valid.
};

//////////////////////////////////////////////////////////////////////////
/** Holds selected data from inside brush.
*/
struct SBrushSubSelection
{
	// Points contained in brush sub-selection.
	std::vector<Vec3*> points;

	//! Adds point to brush sub-selection.
	bool AddPoint( Vec3 *pnt );
	//! Clear all points in brush sub-selection.
	void Clear();
};

//////////////////////////////////////////////////////////////////////////
/** Indoor Brush
*/
struct SBrush : public CRefCountBase
{
public:
	//! Ctor.
	SBrush();
	//! Dtor (virtual)
  virtual ~SBrush();

	//! Assignment operator.
  SBrush& operator = (const SBrush& b);

	//! Create brush planes from bounding box.
  void Create( const Vec3 &mins,const Vec3& maxs,SMapTexInfo *ti );

	//! Create Solid brush out of planes.
	//! @return False if solid brush cannot be created.
  bool BuildSolid( bool bRemoveEmptyFaces=false,bool bBuildRE=false );
	
	//! Remove brush faces that are empty.
	void RemoveEmptyFaces();

  SBrush*	Clone( bool bBuildSolid );

	//! Calculates brush volume.
	//! @return Return volume of this brush.
  float GetVolume();
	
	//! Move brush by delta offset.
  void Move( Vec3d& delta );

  void ClipPrefabModels(bool bClip);

	//! Transform all planes of this brush by specified matrix.
	//! @param bBuild if true after transform face of brush will be build out of planes.
	void Transform( Matrix44 &tm,bool bBuild=true );

	//! Snap all plane points of brush to the specified grid.
  void SnapToGrid();
  
	//! Check if ray intersect brush, and return nearest face hit by the ray.
	//! @return Nearest face that was hit by ray.
	//! @param rayOrigin Source point of ray (in Brush Space).
	//! @param rayDir Normilized ray direction vector (in Brush Space).
	//! @param dist Returns distance from ray origin to the hit point on the brush.
  SBrushFace* Ray( Vec3d rayOrigin,Vec3d rayDir, float *dist );
  
	//! Select brush sides to SubSelection.
  void SelectSide( Vec3d Origin, Vec3d Dir,bool shear,SBrushSubSelection &subSelection );
	
	//! Select brush face to SubSelection.
	//! Called by select side.
	void SelectFace( SBrushFace *f,bool shear,SBrushSubSelection &subSelection );

	//! Create polygon for one of brush faces.
	SBrushPoly*	CreateFacePoly( SBrushFace *f );

	//! Detect on which side of plane this brush lies.
	//!	@return Side cane be SIDE_FRONT or SIDE_BACK.
  int OnSide (SBrushPlane *plane);

	//! Split brush by face and return front and back brushes resulting from split..
	//! @param front Returns splitted front part of brush.
	//! @param back Returns splitted back part of brush.
  void SplitByFace( SBrushFace *f, SBrush* &front, SBrush* &back );
	
	//! Split brush by plane and return front and back brushes resulting from split..
	//! @param front Returns splitted front part of brush.
	//! @param back Returns splitted back part of brush.
  void SplitByPlane (SBrushPlane *plane, SBrush* &front, SBrush* &back);

	//! Fit texture coordinates to match brush size.
  void FitTexture(int nX, int nY);
	
	//! Check if 3D point is inside volume of the brush.
	bool	IsInside(const Vec3d &vPoint);

	//! Check if volumes of 2 brushes intersect.
  bool IsIntersect(SBrush *b);

	//! Intersect 2 brushes, and returns list of all brush parts resulting from intersections.
	bool Intersect(SBrush *b, std::vector<SBrush*>& List);
  void SetShader(SMapTexInfo *ti);

  void Draw();
  bool DrawModel(bool b);
  void AddToList(CCObject *obj);
  bool AddToListModel(bool b);

	//! Return ammount of memory allocated for this brush.
	int GetMemorySize();

	//! Remove faces from brush.
	void ClearFaces();

	//! Serialize brush parameters to xml node.
	void Serialize( XmlNodeRef &xmlNode,bool bLoading );

	//! Set transformation matrix.
	void SetMatrix( const Matrix44 &tm );
	//! Get tranformation matrix.
	const Matrix44& GetMatrix() const { return m_matrix; };

	//! Render brush.
	void Render( IRenderer *renderer,const Vec3 &objectSpaceCamSrc );

	// Set prefab geometry used for brush.
	void SetPrefabGeom( CEdMesh *mesh );
	void ResetToPrefabSize();
	IStatObj* GetIndoorGeom();

	//! Assign indoor owning this brush.
	void SetIndoor( CBrushIndoor *indoor );

	//! Fill mesh with geometry from brush.
	//CIndexedMesh* CreateGeometry();

	//////////////////////////////////////////////////////////////////////////
	// Obsolete
	//////////////////////////////////////////////////////////////////////////
	void MakeSelFace( SBrushFace *f );

public:
	BBox m_bounds;
	std::vector<SBrushFace*> m_Faces;

	//! This brush transformation matrix.
	Matrix44 m_matrix;

  int m_flags;

	//! Original prefab geometry.
	TSmartPtr<CEdMesh> m_prefabGeom;
	//! Geometry used in indoor.
	IStatObj *m_indoorGeom;


private:
	//! Caluclate planes of brush faces.
	void MakeFacePlanes();
	//! Build render element for each face.
	bool BuildRenderElements();
};

#endif // __brush_h__
