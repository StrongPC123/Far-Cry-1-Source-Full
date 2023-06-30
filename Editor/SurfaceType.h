////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   SurfaceType.h
//  Version:     v1.00
//  Created:     19/11/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Surface type defenition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __SurfaceType_h__
#define __SurfaceType_h__

#if _MSC_VER > 1000
#pragma once
#endif

/** Defines axises of projection for detail texture.
*/
enum ESurfaceTypeProjectionAxis
{
	ESFT_X,
	ESFT_Y,
	ESFT_Z,
};

/** CSurfaceType describe parameters of terrain surface.
		Surface types are applied to the layers, total of 7 surface types are currently supported.
*/
class CSurfaceType 
{
public:
	CSurfaceType();
	~CSurfaceType();

	CSurfaceType( const CSurfaceType &st ) { *this = st; }

	void SetName( const CString &name ) { m_name = name; };
	const CString& GetName() const { return m_name; }

	void SetDetailTexture( const CString &tex ) { m_detailTexture = tex; };
	const CString& GetDetailTexture() const { return m_detailTexture; }

	void SetBumpmap( const CString &tex ) { m_bumpmap = tex; };
	const CString& GetBumpmap() const { return m_bumpmap; }

	void SetDetailTextureScale( const Vec3 &scale ) { m_detailScale[0] = scale.x; m_detailScale[1] = scale.y; }
	Vec3 GetDetailTextureScale() const { return Vec3(m_detailScale[0],m_detailScale[1],0); }

	void SetMaterial( const CString &mtl );
	const CString& GetMaterial() const { return m_material; }

	void	AddDetailObject( const CString &name ) { m_detailObjects.push_back(name); };
	int		GetDetailObjectCount() const { return m_detailObjects.size(); };
	void	RemoveDetailObject( int i ) { m_detailObjects.erase( m_detailObjects.begin()+i ); };
	const CString& GetDetailObject( int i ) const { return m_detailObjects[i]; };

	//! Set detail texture projection axis.
	void SetProjAxis( int axis ) { m_projAxis = axis; }

	//! Get detail texture projection axis.
	int GetProjAxis() const { return m_projAxis; }

	CSurfaceType& operator =( const CSurfaceType &st )
	{
		m_name = st.m_name;
		m_material = st.m_material;
		m_detailTexture = st.m_detailTexture;
		m_detailObjects = st.m_detailObjects;
		m_detailScale[0] = st.m_detailScale[0];
		m_detailScale[1] = st.m_detailScale[1];
		m_projAxis = st.m_projAxis;
		m_bumpmap = st.m_bumpmap;
    return *this;
	}

	void Serialize( class CXmlArchive &xmlAr );

private:
	//! Name of surface type.
	CString m_name;
	//! Detail texture applied to this surface type.
	CString m_detailTexture;
	//! Bump map for this surface type.
	CString m_bumpmap;
	//! Detail texture tiling.
	float m_detailScale[2];
	CString m_material;

	//! Array of detail objects used for this layer.
	std::vector<CString> m_detailObjects;

	//! Detail texture projection axis.
	int m_projAxis;
};


#endif // __SurfaceType_h__
