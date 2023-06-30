////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   edmesh.cpp
//  Version:     v1.00
//  Created:     13/2/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Implementation of CEdMesh class.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "EdMesh.h"
#include <I3Dengine.h>

#include "Material\Material.h"
#include "Material\MaterialManager.h"

//////////////////////////////////////////////////////////////////////////
// Static member of CEdMesh.
//////////////////////////////////////////////////////////////////////////
CEdMesh::MeshMap CEdMesh::m_meshMap;

//////////////////////////////////////////////////////////////////////////
// CEdMesh implementation.
//////////////////////////////////////////////////////////////////////////
CEdMesh* CEdMesh::LoadMesh( const char *filename,bool bStripify )
{
	if (strlen(filename) == 0)
		return 0;

	// If object created see if its not yet registered.
	CEdMesh *pMesh = stl::find_in_map( m_meshMap,filename,(CEdMesh*)0 );
	if (pMesh)
	{
		// Found, return it.
		return pMesh;
	}
	
	EVertsSharing sflags = evs_NoSharing;
	if (bStripify)
		sflags = evs_ShareAndSortForCache;
	// Make new.
	IStatObj *pGeom = GetIEditor()->Get3DEngine()->MakeObject( filename,NULL,sflags );
	if (!pGeom)
		return 0;

	// Not found, Make new.
	pMesh = new CEdMesh( pGeom );
	pMesh->m_filename = filename;
	m_meshMap[filename] = pMesh;
	pMesh->RegisterMaterials();
	return pMesh;
}

//////////////////////////////////////////////////////////////////////////
void CEdMesh::ReloadAllGeometries()
{
	for (MeshMap::iterator it = m_meshMap.begin(); it != m_meshMap.end(); ++it)
	{
		CEdMesh *pMesh = it->second;
		if (pMesh)
			pMesh->ReloadGeometry();
	}
}

void CEdMesh::ReleaseAll()
{
	m_meshMap.clear();
}

//////////////////////////////////////////////////////////////////////////
CEdMesh::CEdMesh( IStatObj *pGeom )
{
	assert( pGeom );
	m_pGeom = pGeom;
}

//////////////////////////////////////////////////////////////////////////
CEdMesh::~CEdMesh()
{
	GetIEditor()->Get3DEngine()->ReleaseObject( m_pGeom );
	// Remove this object from map.
	m_meshMap.erase(m_filename);
}

//////////////////////////////////////////////////////////////////////////
void CEdMesh::RegisterMaterials()
{
	/*
	CMaterialManager *pMtlMan = GetIEditor()->GetMaterialManager();
	CMaterial *pMtl = pMtlMan->CreateMaterial( "Level" );
	pMtl->SetName( CString("CGF.") + m_pGeom->GetFileName() );

	m_materials.push_back(pMtl);
	*/
}

//////////////////////////////////////////////////////////////////////////
void CEdMesh::UnregisterMaterials()
{
}

void CEdMesh::ReloadGeometry()
{
	// Reload mesh.
	m_pGeom->Refresh( FRO_GEOMETRY );
}

//////////////////////////////////////////////////////////////////////////
bool CEdMesh::IsSameObject( const char *filename )
{
	return stricmp(m_filename,filename) == 0;
}

//////////////////////////////////////////////////////////////////////////
void CEdMesh::GetBounds( BBox &box )
{
	assert(m_pGeom);

	box.min = m_pGeom->GetBoxMin();
	box.max = m_pGeom->GetBoxMax();
}

//////////////////////////////////////////////////////////////////////////
void CEdMesh::Render( SRendParams &rp,int nLodLevel )
{
	m_pGeom->Render( rp,Vec3(zero),nLodLevel );
}

//////////////////////////////////////////////////////////////////////////
void CEdMesh::SetMaterial( CMaterial *mtl )
{
	m_pMaterial = mtl;
};

//////////////////////////////////////////////////////////////////////////
CMaterial* CEdMesh::GetMaterial() const
{
	return m_pMaterial;
};

//////////////////////////////////////////////////////////////////////////
bool CEdMesh::IsDefaultObject()
{
	return m_pGeom->IsDefaultObject();
}