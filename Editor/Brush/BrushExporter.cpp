////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushexporter.cpp
//  Version:     v1.00
//  Created:     15/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BrushExporter.h"
#include "CryEditDoc.h"

#include "Objects\BrushObject.h"
#include "Objects\Group.h"
#include "Objects\ObjectManager.h"
#include "Material\Material.h"
#include "Brush.h"
#include "list2.h"
#include "Util\Pakfile.h"

#include <I3DEngine.h>

#define BRUSH_FILE "brush.lst"

//////////////////////////////////////////////////////////////////////////
void CBrushExporter::SaveMesh()
{
	/*
	std::vector<CBrushObject*> objects;
	m_indoor->GetObjects( objects );
	for (int i = 0; i < objects.size(); i++)
	{
		SaveObject( objects[i] );
	}
	*/
}

int CBrushExporter::AddChunk( const CHUNK_HEADER &hdr,CMemFile &file )
{
	int fsize = file.GetLength();
	return m_file.AddChunk( hdr,file.Detach(),fsize );
}

//////////////////////////////////////////////////////////////////////////
void CBrushExporter::SaveObject( CBrushObject *obj )
{
	SBrush *brush = obj->GetBrush();
	IStatObj *geom = 	brush->GetIndoorGeom();
	if (!geom)
		return;

	CLeafBuffer *buf = geom->GetLeafBuffer();
	int numVerts = buf->m_SecVertCount;
	int posStride,normalStride,uvStride;
	unsigned char *verts = buf->GetPosPtr( posStride );
	unsigned char *normals = buf->GetNormalPtr( normalStride );
	unsigned char *uvs = buf->GetUVPtr( uvStride );

  int numTris;
	ushort *pInds = buf->GetIndices(&numTris);
  numTris /= 3;

	
	MESH_CHUNK_DESC chunk;
	chunk.HasBoneInfo	= false;
	chunk.HasVertexCol = false;
	chunk.nFaces			= numTris;
	chunk.nTVerts			= numVerts;
	chunk.nVerts			= numVerts;
	chunk.VertAnimID	= 0;

	CMemFile memfile;
	CArchive ar( &memfile, CArchive::store );

	ar.Write( &chunk,sizeof(chunk) );

	std::vector<CryVertex> arrVerts;
	std::vector<CryFace> arrFaces;
	std::vector<CryUV> arrUVs;
	std::vector<CryTexFace> arrTexFaces;

	int i;

	// fill the vertices in
	arrVerts.resize(numVerts);
	arrUVs.resize(numVerts);
	for(i=0;i<numVerts;i++)
	{
		CryVertex& vert = arrVerts[i];
		vert.p = *((Vec3d*)(verts));
		vert.n = *((Vec3d*)(normals));
		arrUVs[i] = *((CryUV*)(uvs));

		normals += normalStride;
		verts += posStride;
		uvs += uvStride;
	}

	// fill faces.
	arrFaces.resize(numTris);
	arrTexFaces.resize(numTris);
	for (i = 0; i < numTris; i++) 
	{
		CryFace& mf = arrFaces[i];
		CryTexFace& texf = arrTexFaces[i];
		mf.v0	= pInds[i*3];
		mf.v1	= pInds[i*3+1];
		mf.v2	= pInds[i*3+2];
		mf.SmGroup = 0;
		mf.MatID = -1;

		texf.t0 = mf.v0;
		texf.t1 = mf.v1;
		texf.t2 = mf.v2;
	}

	// Save verts.
	ar.Write( &arrVerts[0], sizeof(arrVerts[0]) * arrVerts.size() );
	// Save faces.
	ar.Write( &arrFaces[0], sizeof(arrFaces[0]) * arrFaces.size() );

	// Save UVs
	ar.Write( &arrUVs[0], sizeof(arrUVs[0]) * arrUVs.size() );
	// Save tex faces.
	ar.Write( &arrTexFaces[0], sizeof(arrTexFaces[0]) * arrTexFaces.size() );
	ar.Close();
	
	int meshChunkId = AddChunk( chunk.chdr,memfile );

	//////////////////////////////////////////////////////////////////////////
	// Add node.
	//////////////////////////////////////////////////////////////////////////
	NODE_CHUNK_DESC ncd;
	ZeroStruct(ncd);
	ncd.MatID	= 0;
	ncd.nChildren	= 0;
	ncd.ObjectID	= meshChunkId;
	ncd.ParentID	= 0;

	ncd.IsGroupHead = false;
	ncd.IsGroupMember = false;
	ncd.rot.SetIdentity();
	ncd.tm.SetIdentity();

	strcpy( ncd.name,"$0_sector_outside" );
	m_file.AddChunk( ncd.chdr,&ncd,sizeof(ncd) );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CBrushExporter::ExportBrushes( const CString &path,const CString &levelName,CPakFile &pakFile )
{
	CLogFile::WriteLine( "Exporting Brushes...");

	int i;

	CString filename = Path::Make( path,BRUSH_FILE );
	// Export first brushes geometries.
	//if (!CFileUtil::OverwriteFile( filename ))
		//return;

	// Delete the old one.
	//DeleteFile( filename );
	CMemFile file;

	//////////////////////////////////////////////////////////////////////////
	// Clear export data.
	//////////////////////////////////////////////////////////////////////////
	std::vector<CBaseObject*> objects;
	GetIEditor()->GetObjectManager()->GetObjects( objects );

	m_levelName = levelName;

	for (i = 0; i < objects.size(); i++)
	{
		if (objects[i]->GetType() != OBJTYPE_BRUSH)
			continue;

		// Cast to brush.
		CBrushObject *obj = (CBrushObject*)objects[i];
		ExportBrush( path,obj );
	}

	if (m_geoms.empty() || m_brushes.empty())
	{
		// Nothing to export.
		pakFile.RemoveFile( filename );
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Export to file.
	//////////////////////////////////////////////////////////////////////////
	/*
	CFile file;
	if (!file.Open( filename,CFile::modeCreate|CFile::modeWrite ))
	{
		Error( "Unable to open indoor geometry brush.lst file %s for writing",(const char*)filename );
		return;
	}
	*/

	//////////////////////////////////////////////////////////////////////////
	// Write brush file header.
	//////////////////////////////////////////////////////////////////////////
	SExportedBrushHeader header;
	ZeroStruct( header );
	memcpy( header.signature,BRUSH_FILE_SIGNATURE,3 );
	header.filetype = BRUSH_FILE_TYPE;
	header.version = BRUSH_FILE_VERSION;
	file.Write( &header,sizeof(header) );

	//////////////////////////////////////////////////////////////////////////
	// Write materials.
	//////////////////////////////////////////////////////////////////////////
	int numMtls = m_materials.size();
	file.Write( &numMtls,sizeof(numMtls) );
	for (i = 0; i < numMtls; i++)
	{
		// write geometry description.
		file.Write( &m_materials[i],sizeof(m_materials[i]) );
	}

	//////////////////////////////////////////////////////////////////////////
	// Write geometries.
	//////////////////////////////////////////////////////////////////////////
	// Write number of brush geometries.
	int numGeoms = m_geoms.size();
	file.Write( &numGeoms,sizeof(numGeoms) );
	for (i = 0; i < m_geoms.size(); i++)
	{
		// write geometry description.
		file.Write( &m_geoms[i],sizeof(m_geoms[i]) );
	}

	//////////////////////////////////////////////////////////////////////////
	// Write brush instances.
	//////////////////////////////////////////////////////////////////////////
	// write number of brushes.
	int numBrushes = m_brushes.size();
	file.Write( &numBrushes,sizeof(numBrushes) );
	for (i = 0; i < m_brushes.size(); i++)
	{
		// write geometry description.
		file.Write( &m_brushes[i],sizeof(m_brushes[i]) );
	}

	pakFile.UpdateFile( filename,file );

	CLogFile::WriteString("Done.");
}

//////////////////////////////////////////////////////////////////////////
void CBrushExporter::ExportBrush( const CString &path,CBrushObject *brushObject )
{
	IStatObj *prefab = brushObject->GetPrefabGeom();
	if (!prefab)
	{
		return;
	}
	CString geomName = prefab->GetFileName();
	int geomIndex = stl::find_in_map( m_geomIndexMap,geomName,-1 );
	if (geomIndex < 0)
	{
		// add new geometry.
		SExportedBrushGeom geom;
		ZeroStruct( geom );
		geom.size = sizeof(geom);
		strcpy( geom.filename,geomName );
		geom.flags = 0;
		geom.m_minBBox = prefab->GetBoxMin();
		geom.m_maxBBox = prefab->GetBoxMax();
		m_geoms.push_back( geom );
		geomIndex = m_geoms.size()-1;
		m_geomIndexMap[geomName] = geomIndex;
	}
	CMaterial *pMtl =	brushObject->GetMaterial();
	int mtlIndex = -1;
	if (pMtl)
	{
		mtlIndex = stl::find_in_map( m_mtlMap,pMtl,-1 );
		if (mtlIndex < 0)
		{
			SExportedBrushMaterial mtl;
			mtl.size = sizeof(mtl);
			strncpy( mtl.material,pMtl->GetFullName(),sizeof(mtl.material) );
			m_materials.push_back(mtl);
			mtlIndex = m_materials.size()-1;
			m_mtlMap[pMtl] = mtlIndex;
		}
	}

	SExportedBrushGeom *pBrushGeom = &m_geoms[geomIndex];
	if (pBrushGeom)
	{
		if (brushObject->IsRecieveLightmap())
			pBrushGeom->flags |= SExportedBrushGeom::SUPPORT_LIGHTMAP;
		
		IStatObj *pBrushStatObj = brushObject->GetPrefabGeom();
		if (pBrushStatObj)
		{
			if (pBrushStatObj->GetPhysGeom(0) == NULL && pBrushStatObj->GetPhysGeom(1) == NULL)
			{
				pBrushGeom->flags |= SExportedBrushGeom::NO_PHYSICS;
			}
		}
	}

	SExportedBrush expbrush;
	expbrush.size = sizeof(expbrush);
	ZeroStruct( expbrush );
	CGroup *pGroup = brushObject->GetGroup();
	if (pGroup)
	{
		expbrush.mergeId = pGroup->GetGeomMergeId();
	}
	expbrush.id = brushObject->GetId().Data1;
	//HACK, Remove GetTranspose later, when Matrix44 fixed.
	expbrush.matrix = Matrix34( GetTransposed44(brushObject->GetBrushMatrix()) );
	expbrush.geometry = geomIndex;
	expbrush.flags = brushObject->GetRenderFlags();
	expbrush.material = mtlIndex;
	expbrush.ratioLod = brushObject->GetRatioLod();
	expbrush.ratioViewDist = brushObject->GetRatioViewDist();
	m_brushes.push_back( expbrush );
}
