////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   edmesh.h
//  Version:     v1.00
//  Created:     13/2/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Editor structure that wraps access to IStatObj
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __edmesh_h__
#define __edmesh_h__
#pragma once

class CMaterial;

/** CEdMesh is an editor version of IStatObj,
		It wraps access to 3D engines IStatObj and provide access to custom mesh materials.
*/
class CRYEDIT_API CEdMesh : public CRefCountBase
{
	
public:
	~CEdMesh();

	// Return filename of mesh.
	const CString& GetFilename() const { return m_filename; };

	//! Bounding box of mesh. - m_pGeom must not be 0
	void GetBounds( BBox &box );

	//! Reload geometry of mesh.
	void ReloadGeometry();

	//! Access stored IStatObj.
	IStatObj* GetGeometry() const { return m_pGeom; }
	
	//! Returns true if filename and geomname refer to the same object as this one.
	bool IsSameObject( const char *filename );

	//! Render mesh.
	void Render( SRendParams &rp,int nLodLevel=0 );

	//! Make new CEdMesh, if same IStatObj loaded, and CEdMesh for this IStatObj is allocated.
	//! This instance of CEdMesh will be returned.
	static CEdMesh* LoadMesh( const char *filename,bool bStripify );
	//! Reload all geometries.
	static void ReloadAllGeometries();
	static void ReleaseAll();

	//! Assigns defaul material to the mesh.
	void SetMaterial( CMaterial *mtl );
	CMaterial* GetMaterial() const;

	//! Check if default object was loaded.
	bool IsDefaultObject();

private:
	//////////////////////////////////////////////////////////////////////////
	CEdMesh( IStatObj *pGeom );

	//! Register mesh materials with material manager.
	void RegisterMaterials();
	void UnregisterMaterials();

	//////////////////////////////////////////////////////////////////////////
	//! CGF filename.
	CString m_filename;
	IStatObj *m_pGeom;

	//! Material applied to this geometry.
    TSmartPtr<CMaterial> m_pMaterial;

	typedef std::map<CString,CEdMesh*,stl::less_stricmp<CString> > MeshMap;
	static MeshMap m_meshMap;
};

#endif // __edmesh_h__
