////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   gameexporter.h
//  Version:     v1.00
//  Created:     30/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __gameexporter_h__
#define __gameexporter_h__
#pragma once

#include "Util\PakFile.h"

/*!	CGameExporter implements exporting of data fom Editor to Game format.
		It will produce LevelData.cpk file in current level folder, with nececcary exported files.
 */
class CGameExporter
{
public:
	CGameExporter( ISystem *system );
	virtual ~CGameExporter();

	// Export level info to game.
	virtual void Export( bool bSurfaceTexture,bool bReloadTerrain );
	virtual void ExportLevelData( const CString &path,bool bExportMission=true );
	virtual void ExportLevelInfo( const CString &path );

	void FlattenHeightmap( uint16 *pSaveHeightmapData,int width,int height );

	virtual void ExportMap(char *pszGamePath, bool bSurfaceTexture);
	virtual void ExportHeightMap(char *pszGamePath);
	virtual void ExportAnimations( const CString &path );
	virtual void ExportStaticObjects( const CString &path );
	virtual void ExportSurfaceTypes( const CString &iniFile );
	virtual void ExportMapIni( const CString &path );
	virtual void ExportMapInfo( XmlNodeRef &node );
	virtual bool ExportSurfaceTexture( const CString &path,uint *pSurface,int width,bool bHiQuality );
	virtual void ExportLowResSurfaceTexture( const char* pszGamePath,CImage &surfaceTexture );
	virtual void ExportAIGraph( const CString &path );
	
	virtual void ExportBrushes( const CString &path );
	virtual void ExportMusic( XmlNodeRef &levelDataNode,const CString &path );
	virtual void ExportMaterials( XmlNodeRef &levelDataNode,const CString &path );

private:
	void ExportTerrainBeaches( const CString &path );
	void DeleteOldFiles(  const CString &levelDir,bool bSurfaceTexture );
	void ForceDeleteFile( const char *filename );

	CString m_levelName;
	CString m_missionName;
	CString m_levelPath;

	ISystem *m_ISystem;
	I3DEngine *m_I3DEngine;
	IAISystem *m_IAISystem;
	IEntitySystem *m_IEntitySystem;
	IEditor *m_IEditor;

	CPakFile m_levelPakFile;
	bool m_bHiQualityCompression;
	int m_iExportTexWidth;
	int m_numExportedMaterials;
	//CPakFile m_texturePakFile;
};

#endif // __gameexporter_h__
