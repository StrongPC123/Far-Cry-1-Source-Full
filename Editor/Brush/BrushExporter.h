////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushexporter.h
//  Version:     v1.00
//  Created:     15/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __brushexporter_h__
#define __brushexporter_h__
#pragma once

#include "Util\ChunkFile.h"

// forward declarations.
class CChunkFile;
class CBrushObject;
class CPakFile;

#define BRUSH_FILE_TYPE 1
#define BRUSH_FILE_VERSION 3
#define BRUSH_FILE_SIGNATURE "CRY"

//////////////////////////////////////////////////////////////////////////
// Brush Export structures.
//////////////////////////////////////////////////////////////////////////
#pragma pack(push,1)

struct SExportedBrushHeader
{
	char signature[3];	// File signature.
	int filetype;				// File type.
	int	version;				// File version.
};

struct SExportedBrushGeom
{
	enum EFlags
	{
		SUPPORT_LIGHTMAP = 0x01,
		NO_PHYSICS = 0x02,
	};
	int size; // Size of this sructure.
	char filename[128];
	int flags; //! @see EFlags
	Vec3 m_minBBox;
	Vec3 m_maxBBox;
};

struct SExportedBrushMaterial
{
	int size;
	char material[64];
};

struct SExportedBrush
{
	int size; // Size of this sructure.
	int id;
	int geometry;
	int material;
	int flags;
	int mergeId;
	Matrix34 matrix;
	uchar ratioLod;
	uchar ratioViewDist;
	uchar reserved1;
	uchar reserved2;
};
#pragma pack(pop)
//////////////////////////////////////////////////////////////////////////

/** Export brushes from specified Indoor to .bld file.
*/
class CBrushExporter
{
public:
	void ExportBrushes( const CString &path,const CString &levelName,CPakFile &pakFile );

private:
	void ExportBrush( const CString &path,CBrushObject *brush );

	void SaveMesh();
	void SaveObject( CBrushObject *brush );
	int AddChunk( const CHUNK_HEADER &hdr,CMemFile &file );

	CChunkFile m_file;

	int nodeCount;

	CString m_levelName;

	//////////////////////////////////////////////////////////////////////////
	typedef std::map<CString,int,stl::less_stricmp<CString> > GeomIndexMap;
	GeomIndexMap m_geomIndexMap;
	std::map<CMaterial*,int> m_mtlMap;
	std::vector<SExportedBrushGeom> m_geoms;
	std::vector<SExportedBrush> m_brushes;
	std::vector<SExportedBrushMaterial> m_materials;
};

#endif // __brushexporter_h__
