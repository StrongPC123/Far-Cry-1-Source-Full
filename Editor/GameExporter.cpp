#include "StdAfx.h"
#include "gameexporter.h"
#include "gameengine.h"
#include "PluginManager.h"
#include "texturecompression.h"
#include "DimensionsDialog.h"
#include "TerrainTexture.h"
#include "TerrainLighting.h"
#include "SurfaceType.h"
#include "EquipPackLib.h"
#include "TopRendererWnd.h"
#include "ViewManager.h"
#include "VegetationObject.h"
#include "CryEditDoc.h"
#include "Mission.h"
#include "VegetationMap.h"

#include "Util\FileUtil.h"
#include "Objects\ObjectManager.h"
#include "Objects\BaseObject.h"
#include "Brush\BrushExporter.h"

//#include "Anim\IMovieSystem.h"
#include "AnimationSerializer.h"
#include "Material\MaterialManager.h"
#include "Material\MaterialLibrary.h"
#include "Particles\ParticleManager.h"
#include "Music\MusicManager.h"
#include "BaseLibrary.h"

#include "ParticleExporter.h"

#include "Heightmap.h"

#include "TerrainTexGen.h"
#include "TerrainBeachGen.h"

#include <IAgent.h>
#include <IAISystem.h>
#include <I3dengine.h>
#include <IGame.h>
#include <ICryPak.h>
#include <I3DEngine.h>

//////////////////////////////////////////////////////////////////////////
#define VEGETATION_FILE "objects.lst"
#define MUSIC_LEVEL_LIBRARY_FILE "Music.xml"
#define MATERIAL_LEVEL_LIBRARY_FILE "Materials.xml"

#define TERRAIN_BEACHES_FILENAME "beach7.tmp"


//////////////////////////////////////////////////////////////////////////
CGameExporter::CGameExporter( ISystem *system )
{
	m_IEditor = GetIEditor();
	m_ISystem = system;
	m_I3DEngine = m_ISystem->GetI3DEngine();
	m_IAISystem = m_ISystem->GetAISystem();
	m_IEntitySystem = m_ISystem->GetIEntitySystem();

	m_iExportTexWidth = 0;
	m_bHiQualityCompression = true;
}

CGameExporter::~CGameExporter(void)
{
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::Export( bool bSurfaceTexture,bool bReloadTerrain )
{
	CString szLevelPath = Path::GetPath( m_IEditor->GetDocument()->GetPathName() );
	szLevelPath = GetIEditor()->GetRelativePath(szLevelPath);
	if (szLevelPath.IsEmpty())
	{
		// Bad path.
		Warning( "Export Failed!\r\nLevel folder must be relative to root folder of the game.",MB_OK|MB_ICONERROR );
		return;
	}
	m_levelName = Path::RemoveBackslash(szLevelPath);
	m_levelPath = Path::RemoveBackslash(szLevelPath);
	
	if (bSurfaceTexture)
	{
		CDimensionsDialog cDialog;

		// 4096x4096 is default
		cDialog.SetDimensions(4096);

		// Query the size of the preview
		if (cDialog.DoModal() == IDCANCEL)
			return;

		m_bHiQualityCompression = cDialog.GetCompressionQuality();

		m_iExportTexWidth = cDialog.GetDimensions();
	}

	m_missionName = m_IEditor->GetGameEngine()->GetMissionName();

	// Make sure the path is ready for adding a filename
	char pszGamePath[_MAX_PATH];
	CString strTempCopyStr = m_IEditor->GetDocument()->GetPathName();
	strcpy(pszGamePath, strTempCopyStr.GetBuffer(1));
	PathRemoveFileSpec(pszGamePath);
	PathAddBackslash(pszGamePath);

	CString pakFilename = CString(pszGamePath) + "Level.pak";

	//////////////////////////////////////////////////////////////////////////
	// Make sure 3D engine closes texture handle.
	GetIEditor()->Get3DEngine()->CloseTerrainTextureFile();

	// Close this pak file.
	if (!m_ISystem->GetIPak()->ClosePack( pakFilename ))
	{
		Warning( "Export Failed!\r\nCannot Close Pak File %s",(const char*)pakFilename );
	}

	//////////////////////////////////////////////////////////////////////////
	if (!CFileUtil::OverwriteFile(pakFilename))
		return;
	
	// Delete old pak file.
	//DeleteFile( pakFilename );
	
	if (!m_levelPakFile.Open( pakFilename ))
	{
		Warning( "Export Failed!\r\nCannot Open pak file %s for writing.",(const char*)pakFilename );
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Start Export.
	//////////////////////////////////////////////////////////////////////////
	// Reset all animations before exporting.
	GetIEditor()->GetAnimation()->ResetAnimations(false);

	////////////////////////////////////////////////////////////////////////
	// Export all data to the game
	////////////////////////////////////////////////////////////////////////

	ExportMap(pszGamePath, bSurfaceTexture);
	
	////////////////////////////////////////////////////////////////////////
	// Export the cloud layer
	////////////////////////////////////////////////////////////////////////

	//m_IEditor->SetStatusText("Exporting cloud layer...");
	//CLogFile::WriteLine("Exportin Cloud layer...");

	////////////////////////////////////////////////////////////////////////
	// Export the heightmap, store shadow informations in it
	////////////////////////////////////////////////////////////////////////

	ExportHeightMap(pszGamePath);

	////////////////////////////////////////////////////////////////////////
	// Export static objects
	////////////////////////////////////////////////////////////////////////
	ExportStaticObjects( pszGamePath );
	HEAP_CHECK

	////////////////////////////////////////////////////////////////////////
	// Exporting map setttings
	////////////////////////////////////////////////////////////////////////
	//ExportMapIni( pszGamePath );

	//////////////////////////////////////////////////////////////////////////
	// Export Particles.
	{
		CParticlesExporter partExporter;
		partExporter.ExportParticles( pszGamePath,m_levelPath,m_levelPakFile );
	}
	//////////////////////////////////////////////////////////////////////////
		
	//////////////////////////////////////////////////////////////////////////
	//! Export Level data.
	CLogFile::WriteLine("Exporting LevelData.xml");
	ExportLevelData( pszGamePath );
	CLogFile::WriteLine("Exporting LevelData.xml done.");
	HEAP_CHECK

	ExportLevelInfo( pszGamePath );

	////////////////////////////////////////////////////////////////////////
	// Export the data of all plugin
	////////////////////////////////////////////////////////////////////////

	CLogFile::WriteLine("Exporting plugin data...");

	if (!m_IEditor->GetPluginManager()->CallExport(pszGamePath))
	{
		CLogFile::WriteLine("Error while exporting plugin data !");
		ASSERT(false);
		AfxMessageBox("Error while exporting plugin data !");
	}

	ExportAIGraph(pszGamePath);

	//////////////////////////////////////////////////////////////////////////
	// Start Movie System animations.
	//////////////////////////////////////////////////////////////////////////
	ExportAnimations(pszGamePath);

	//////////////////////////////////////////////////////////////////////////
	// Export Brushes.
	//////////////////////////////////////////////////////////////////////////
	ExportBrushes( pszGamePath );
	
	//////////////////////////////////////////////////////////////////////////
	// End Exporting Game data.
	//////////////////////////////////////////////////////////////////////////

	// Close all packs.
	m_levelPakFile.Close();
//	m_texturePakFile.Close();

	////////////////////////////////////////////////////////////////////////
	// Reload the level in the engine
	////////////////////////////////////////////////////////////////////////
	if (bReloadTerrain)
	{
		GetIEditor()->SetStatusText( _T("Reloading Level...") );
		m_IEditor->GetGameEngine()->LoadLevel( szLevelPath, m_missionName,false,false );
	}

	// Force hide local player.
#ifndef _ISNOTFARCRY
		GetIXGame( GetIEditor()->GetGame() )->HideLocalPlayer( true, true );
#endif

	GetIEditor()->SetStatusText( _T("Ready") );
	// Reopen this pak file.
	if (!m_ISystem->GetIPak()->OpenPack( pakFilename ))
	{
		Warning( "Export Failed!\r\nCannot Open Pak File %s",(const char*)pakFilename );
		return;
	}

	CLogFile::WriteLine("Exporting was successful.");
}

void CGameExporter::ExportMap(char *pszGamePath, bool bSurfaceTexture)
{
	m_IEditor->ShowConsole( true );

	char szBuffer[512 + _MAX_PATH];

	// Settings
	int iExportTexWidth       = m_iExportTexWidth; // Size of the exported surface texture
	const int iMapPreviewWidth      = 512;  // Size of the map preview
	bool bHiQualityCompression = m_bHiQualityCompression;

	// No need to generate texture if there are no layers or the caller does
	// not demand the texture to be generated
	if (m_IEditor->GetDocument()->GetLayerCount() == 0 || !bSurfaceTexture)
		return;

	/*
	if (!GetPathName().IsEmpty())
	{
		OnSaveDocument( GetPathName() );
	}
	*/

	// Check if the .cry file is in the correct folder
	strcpy(szBuffer, pszGamePath);
	szBuffer[strlen(GetIEditor()->GetMasterCDFolder())] = '\0';
	if (stricmp(szBuffer,GetIEditor()->GetMasterCDFolder()) != 0)
	{
		AfxMessageBox("Can't export because the .cry file is not in the Levels folder inside" \
			" the Master CD folder, or editor executable is not in the Master CD folder. Always create" \
			" your projects with the 'Create / open level...' command. Do not manually move" \
			" the .cry file or the editor executable.");
		return;
	}

	CWaitCursor wait;

	CLogFile::FormatLine("Exporting data to game (%s)...", pszGamePath);

	////////////////////////////////////////////////////////////////////////
	// Create the necessary subdirectories
	////////////////////////////////////////////////////////////////////////

	//sprintf(szFileOutputPath, "%sterrain", pszGamePath);
	//CreateDirectory( szFileOutputPath,0 );

	////////////////////////////////////////////////////////////////////////
	// Export the map preview
	////////////////////////////////////////////////////////////////////////
	//Timur[2/6/2003] Commented out, Not used now.
	/* 
	{
		m_IEditor->SetStatusText("Exporting map preview...");
		CLogFile::FormatLine("Map preview (%ix%i)...", iMapPreviewWidth, iMapPreviewWidth);

		CImage preview;

		preview.Allocate( iMapPreviewWidth,iMapPreviewWidth );

		CTerrainTexGen texGen( iMapPreviewWidth );
		if (!texGen.GenerateSurfaceTexture( ETTG_QUIET|ETTG_KEEP_LAYERMASKS|ETTG_LIGHTING ))
		{
			Error( "Failed to generate preview texture.");
			return;
		}
		// Construct the filename of the map preview file
		sprintf(szFileOutputPath, "%s\\terrain\\map_preview.jpg", pszGamePath);	

		// Save the surface texture preview as JPEG file
		if (!CImageUtil::SaveImage( szFileOutputPath, preview ))
		{
			Error("Can't export / save surface texture for map preview");
			return;
		}
	}
	*/

	////////////////////////////////////////////////////////////////////////
	// Export the surface texture
	////////////////////////////////////////////////////////////////////////

	CLogFile::WriteLine("Exporting Surface texture.");

	CHeightmap &HeightMap=m_IEditor->GetDocument()->GetHeightmap();

	m_IEditor->SetStatusText("Exporting surface texture (Generating)...");

	// Check dimensions
	if (HeightMap.GetWidth() != HeightMap.GetHeight() || HeightMap.GetWidth() % 128)
	{
		ASSERT(HeightMap.GetWidth() % 128 == 0);
		AfxMessageBox("Can't export a heightmap with dimensions that can't be" \
			" evenly divided by 128 or that are not square !");
		CLogFile::WriteLine("Can't export a heightmap");
		return;
	}

	/*
	// Make Texture Pack.
	CString pakFilename = CString(pszGamePath) + "LevelTexture.pak";
	// Close this pak file.
	if (!m_ISystem->GetIPak()->ClosePack( pakFilename ))
	{
		Error( "Cannot Close Pak File %s",(const char*)pakFilename );
	}
	*/

	//////////////////////////////////////////////////////////////////////////
	//if (!CFileUtil::OverwriteFile(pakFilename))
		//return;

	uint t0 = GetTickCount();
	// Time how long does it take to generate Surface texture.
	{
		// Allocate memory for the lighting bit array
		CBitArray lightBits;
		lightBits.reserve( iExportTexWidth*iExportTexWidth );
		lightBits.clear_num( iExportTexWidth*iExportTexWidth );

		CTerrainTexGen texGen( iExportTexWidth );
		texGen.SetLightingBits( &lightBits );

		// Allocate the memory for the texture
		CImage surfaceTexture;
		surfaceTexture.Allocate( iExportTexWidth,iExportTexWidth );
		if (!surfaceTexture.GetData())
		{
			Error( "Surface Texture Memory Allocation Failed.\r\nChoose smaller texture size." );
			return;
		}
		if (!texGen.GenerateSurfaceTexture(ETTG_QUIET|ETTG_KEEP_LAYERMASKS|ETTG_LIGHTING|ETTG_STATOBJ_SHADOWS|ETTG_STATOBJ_PAINTBRIGHTNESS|ETTG_ABGR,surfaceTexture ))
			return;
		uint t1 = GetTickCount();
		CLogFile::FormatLine( "Surface Texture Generated in %u seconds",(t1-t0)/1000 );

		// Delete old pak file.
		//DeleteFile( pakFilename );
		//m_texturePakFile.Open( pakFilename,false );

		ExportLowResSurfaceTexture( pszGamePath,surfaceTexture );

		HeightMap.SetLightingBit( lightBits,iExportTexWidth,iExportTexWidth );
		HeightMap.CalcSurfaceTypes();

		if (!ExportSurfaceTexture( pszGamePath,surfaceTexture.GetData(),surfaceTexture.GetWidth(),bHiQualityCompression ))
			return;
	}

	/*
	CLogFile::WriteLine("Compressing surface texture...");
	// Call the compression function
	sprintf(szFileOutputPath, "%sTerrain\\", pszGamePath);
	cCompression.CompressCTU( szFileOutputPath,bHiQualityCompression );
	*/

	//int texSize = m_I3DEngine->GetTerrainTextureDim();
	//if (texSize > 0)
	HeightMap.SetSurfaceTextureSize( iExportTexWidth,iExportTexWidth );

	int t2 = GetTickCount();
	CLogFile::FormatLine( "Surface Texture Exported in %u seconds.",(t2-t0)/1000 );
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::ExportHeightMap(char *pszGamePath)
{
	char szFileOutputPath[_MAX_PATH];
	t_hmap *pHeightmapBits = NULL;
	t_hmap *pHeightmapBitsStart = NULL;
	t_hmap *pHeightmapBitsEnd = NULL;
	uint16 *pSaveHeightmapData = NULL;
	uint16 *pSaveHeightmapDataStart = NULL;
	uint16 *pSaveHeightmapDataEnd = NULL;
	CHeightmap &HeightMap=m_IEditor->GetDocument()->GetHeightmap();

	m_IEditor->SetStatusText("Exporting heightmap...");
	CLogFile::WriteLine("Heightmap...");

	m_IEditor->SetStatusText("Calculating Surface Types...");
	CLogFile::WriteLine("Calculating Surface Types...");

	//HeightMap.CalcSurfaceTypes();
	m_IEditor->SetStatusText("Exporting heightmap...");
	HEAP_CHECK

	// Allocate memory for the 16 bit version that will be saved
	pSaveHeightmapData = new uint16 [HeightMap.GetWidth() * HeightMap.GetHeight()];
	ASSERT(pSaveHeightmapData);
	pSaveHeightmapDataStart = pSaveHeightmapData;
	pSaveHeightmapDataEnd = &pSaveHeightmapData[HeightMap.GetWidth() *	HeightMap.GetHeight()];

	// Set the loop pointers
	pHeightmapBitsStart = HeightMap.GetData();
	pHeightmapBits = pHeightmapBitsStart;
	pHeightmapBitsEnd = &pHeightmapBitsStart[HeightMap.GetWidth() * HeightMap.GetHeight()];

	// Convert the heightmap data to 16 bit and store it in the 16 bit array
	while (pHeightmapBits != pHeightmapBitsEnd)
	{
		// Must multiply to 256.
		*pSaveHeightmapData++ = ftoi((*pHeightmapBits)*256.0f);
		pHeightmapBits++;
	}

	// Restore the pointers
	pHeightmapBits = pHeightmapBitsStart;
	pSaveHeightmapData = pSaveHeightmapDataStart;

/*	gameExporter.*/FlattenHeightmap( pSaveHeightmapData,HeightMap.GetWidth(),HeightMap.GetHeight() );
	HEAP_CHECK

	// Construct the filename of the heightmap file
	sprintf(szFileOutputPath, "%sTerrain\\LAND_MAP.H16", pszGamePath);

	// Open the heightmap file
	//if (!CFileUtil::OverwriteFile( szFileOutputPath ))
		//return;

	// Save heightmap file.
	CMemFile hmapFile;
	hmapFile.Write( pSaveHeightmapDataStart,2*HeightMap.GetWidth()*HeightMap.GetHeight() );
	m_levelPakFile.UpdateFile( szFileOutputPath,hmapFile );


	// Free the memory of the 16 bit version of the heightmap data
	if (pSaveHeightmapDataStart)
	{
		delete [] pSaveHeightmapDataStart;
		pSaveHeightmapDataStart = NULL;
	}

	ExportTerrainBeaches( pszGamePath );
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::ExportAnimations( const CString &path )
{
	GetIEditor()->SetStatusText( _T("Exporting Animation Sequences...") );
	CLogFile::WriteLine("Export animation sequences...");
	CAnimationSerializer animSaver;
	animSaver.SaveAllSequences( path,m_levelPakFile );
	CLogFile::WriteString("Done.");

	/*
	char szPath[_MAX_PATH];

	strcpy( szPath,m_levelPath );
	PathAddBackslash(szPath);
	strcat( szPath,"Animation\\" );

	ISequenceIt *It=m_IEditor->GetMovieSystem()->GetSequences();
	IAnimSequence *seq=It->first();
	while (seq)
	{
		char szFile[_MAX_PATH];
		_makepath( szFile,0,szPath,seq->GetName(),"seq" );
		CAnimationSerializer animSerializer;
		animSerializer.SaveSequence( seq,szFile,false );
		seq=It->next();
	}
	It->Release();
	*/
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::ExportStaticObjects( const CString &exportPath )
{
	int iCount,i;
	Vec3 p;

	CHeightmap *pHeightMap = m_IEditor->GetHeightmap();
	int worldSize = pHeightMap->GetUnitSize() * pHeightMap->GetWidth();

	GetIEditor()->SetStatusText( _T("Exporting Vegetation...") );
	CLogFile::WriteLine("Exporting Vegetation...");

	// Assign indices to vegetation objects.
	CVegetationMap *pVegetationMap = m_IEditor->GetVegetationMap();
	for (i = 0; i < pVegetationMap->GetObjectCount(); i++)
	{
		pVegetationMap->GetObject(i)->SetIndex(i);
	}


	std::vector<CVegetationInstance*> vegInstances;
	pVegetationMap->GetAllInstances( vegInstances );


	// Generate a list of static objects
	std::vector<StatObjInstance> statObjects;
	statObjects.resize( vegInstances.size() );
	if (statObjects.size() > 0)
	{
		memset( &statObjects[0],0,sizeof(StatObjInstance)*statObjects.size() );
	}

	for (i = 0; i < statObjects.size(); i++)
	{
		CVegetationInstance *vegobj = vegInstances[i];
		CVegetationObject *so = vegobj->object;

		StatObjInstance &inst = statObjects[i];
		// Swap x/y.
		inst.SetXYZ( vegobj->pos.x,vegobj->pos.y,vegobj->pos.z,worldSize );
		inst.SetID( so->GetIndex() );
		inst.SetScale( vegobj->scale );
		inst.SetBrightness( vegobj->brightness );
	}

/*
	// Construct the filename of the static object index file
	sprintf(szFileOutputPath, "%sobjects.idx", (const char*)exportPath );

	// Open the index file
	//if (!CFileUtil::OverwriteFile( szFileOutputPath ))
		//return;
	//FILE *hFile = fopen(szFileOutputPath, "w"); 

	CMemFile objectsidxFile;

	typedef StdMap<CString,int> StatObjFileMap;
	StatObjFileMap statObjFileMap;

	const char *strFormat = "%s Angles=(%g,%g,%g) Bending=%g Hide=%d\n";
	float fZero = 0;
	int iZero = 0;
	// Write the objects into the index file
	for (i=0; i < pVegetationMap->GetObjectCount(); i++)
	{
		CVegetationObject *obj = pVegetationMap->GetObject(i);
		CString objName = obj->GetFileName();
		float fBending = obj->GetBending();
		const char *objFileName = obj->GetFileName();
		int hidable = obj->IsHideable();
		char str[1024];
		sprintf( str,strFormat,objFileName,fZero,fZero,fZero,fBending,hidable );
		
		objectsidxFile.Write( str,strlen(str) );

		statObjFileMap.Insert( str,i );
	}
	// Update file in pak.
	m_levelPakFile.UpdateFile( szFileOutputPath,objectsidxFile );
*/

	// Write the objects into the index file
	int iStaticId = pVegetationMap->GetObjectCount();

	CString filename = Path::Make( exportPath,VEGETATION_FILE );
	if (statObjects.size() != 0)
	{
		CLogFile::FormatLine("Exporting %i static object instances...", statObjects.size() );

		CMemFile objectlstFile;

		// Write the size of one static object
		iCount =  sizeof(statObjects[0]);
		objectlstFile.Write( &iCount,sizeof(iCount) );

		// Save every static object
		for (int i = 0; i < statObjects.size(); i++)
		{
			// Write the structure to the file
			objectlstFile.Write( &statObjects[i], sizeof(statObjects[i]) );
		}
		// Update file in pak.
		m_levelPakFile.UpdateFile( filename,objectlstFile );
	}
	else
	{
		m_levelPakFile.RemoveFile( filename );
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::ExportSurfaceTypes( const CString &iniFile )
{
	/*
	std::vector<CString> detailObjects;
	int i;

	// Fill detail objects.
	for (i = 0; i < m_IEditor->GetDocument()->GetSurfaceTypeCount(); i++)
	{
		CSurfaceType *sf = m_IEditor->GetDocument()->GetSurfaceType(i);
		for (int j = 0; j < sf->GetDetailObjectCount(); j++)
		{
			if (std::find(detailObjects.begin(),detailObjects.end(),sf->GetDetailObject(j)) == detailObjects.end())
			{
				detailObjects.push_back( sf->GetDetailObject(j) );
			}
		}
	}

	// Write detail objects.
	char str[256];
	char str1[256];
	for (i = 0; i < detailObjects.size(); i++)
	{
		sprintf( str,"DetailObject%d_FileName",i );
		WritePrivateProfileString("DetailObjects", str, detailObjects[i], iniFile );
		sprintf( str,"DetailObject%d_Scale",i );
		WritePrivateProfileString("DetailObjects", str, "0.5",iniFile );
	}

	// Write surface types defenitions.
	for (i = 0; i < m_IEditor->GetDocument()->GetSurfaceTypeCount(); i++)
	{
		CSurfaceType *sf = m_IEditor->GetDocument()->GetSurfaceType(i);
		sprintf( str,"Surface%d_Texture",i );
		WritePrivateProfileString("SurfaceDefinition", str, sf->GetDetailTexture(), iniFile );
		sprintf( str,"Surface%d_DetailObjects",i );
		strcpy( str1,"" );
		for (int j = 0; j < sf->GetDetailObjectCount(); j++)
		{
			int id = std::find(detailObjects.begin(),detailObjects.end(),sf->GetDetailObject(j)) - detailObjects.begin();
			char str2[64];
			strcat( str1,itoa( id,str2,10 ) );
			strcat( str1,"," );
		}
		// remove last comma.
		int len1 = strlen(str1);
		if (len1 > 0)
		{
			str1[len1-1] = 0;
		}
		WritePrivateProfileString("SurfaceDefinition", str, str1, iniFile );
	}
	*/
}

void CGameExporter::ExportLevelData( const CString &path,bool bExportMission )
{
	GetIEditor()->SetStatusText( _T("Exporting LevelData.xml...") );

	int i;
	XmlNodeRef root = new CXmlNode( "LevelData" );
	root->setAttr( "SandboxVersion",(const char*)GetIEditor()->GetFileVersion().ToFullString() );

	ExportMapInfo( root );

	//////////////////////////////////////////////////////////////////////////
	/// Export vegetation objects.
	XmlNodeRef vegetationNode = root->newChild( "Vegetation" );
	CVegetationMap *pVegetationMap = m_IEditor->GetVegetationMap();
	for (i = 0; i < pVegetationMap->GetObjectCount(); i++)
	{
		XmlNodeRef node = vegetationNode->newChild( "Object" );
		pVegetationMap->GetObject(i)->Serialize( node,false );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Export materials.
	ExportMaterials( root,path );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Export particle manager.
	GetIEditor()->GetParticleManager()->Export( root );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Export Dymaic Music info.
	ExportMusic( root,path );
	//////////////////////////////////////////////////////////////////////////
	

	//XmlNodeRef objectsRoot = root->newChild( "Objects" );
	//m_IEditor->GetObjectManager()->Export( path,objectsRoot,true );

	// Save contents of current mission.	
	//m_IEditor->GetDocument()->GetCurrentMission()->SyncContent( false );

	CCryEditDoc *pDocument = m_IEditor->GetDocument();
	CMission *pCurrentMission = 0;
	
	if (bExportMission)
	{
		pCurrentMission = pDocument->GetCurrentMission();
		// Save contents of current mission.	
	}


	//////////////////////////////////////////////////////////////////////////
	// Export missions tag.
	//////////////////////////////////////////////////////////////////////////
	XmlNodeRef missionsNode = root->newChild("Missions");
	CString missionFileName;
	CString currentMissionFileName;
	for (i = 0; i < pDocument->GetMissionCount(); i++)
	{
		CMission *pMission = pDocument->GetMission(i);

		CString name = pMission->GetName();
		name.Replace( ' ','_' );
		missionFileName.Format( "Mission_%s.xml",(const char*)name );

		XmlNodeRef missionDescNode = missionsNode->newChild("Mission");
		missionDescNode->setAttr( "Name",pMission->GetName() );
		missionDescNode->setAttr( "File",missionFileName );
		missionDescNode->setAttr( "CGFCount",m_I3DEngine->GetLoadedObjectCount() );

		int nProgressBarRange = m_numExportedMaterials/10 + m_I3DEngine->GetLoadedObjectCount();
		missionDescNode->setAttr( "ProgressBarRange",nProgressBarRange );

		if (pMission == pCurrentMission)
		{
			currentMissionFileName = missionFileName;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Save Level Data XML
	//////////////////////////////////////////////////////////////////////////
	CString levelDataFile = path + "LevelData.xml";
	//if (!CFileUtil::OverwriteFile( levelDataFile ))
		//return;
	XmlString xmlData = root->getXML();
	
	CMemFile file;
	file.Write( xmlData.c_str(),xmlData.length() );
	m_levelPakFile.UpdateFile( levelDataFile,file );

	if (bExportMission)
	{
		//////////////////////////////////////////////////////////////////////////
		// Export current mission file.
		//////////////////////////////////////////////////////////////////////////
		XmlNodeRef missionNode = root->createNode("Mission");
		m_IEditor->GetEquipPackLib()->Serialize(missionNode, false);
		pCurrentMission->Export( missionNode );

		missionNode->setAttr( "CGFCount",m_I3DEngine->GetLoadedObjectCount() );

		//if (!CFileUtil::OverwriteFile( path+currentMissionFileName ))
//			return;
		
		xmlData = missionNode->getXML();
		CMemFile file;
		file.Write( xmlData.c_str(),xmlData.length() );
		m_levelPakFile.UpdateFile( path+currentMissionFileName,file );
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::ExportLevelInfo( const CString &path )
{
	//////////////////////////////////////////////////////////////////////////
	// Export short level info xml.
	//////////////////////////////////////////////////////////////////////////
	XmlNodeRef root = new CXmlNode( "LevelInfo" );
	root->setAttr( "SandboxVersion",(const char*)GetIEditor()->GetFileVersion().ToFullString() );

	CString levelName = GetIEditor()->GetGameEngine()->GetLevelName();
	root->setAttr( "Name",levelName );
	root->setAttr( "HeightmapSize",GetIEditor()->GetHeightmap()->GetWidth() );
	
	// Save all missions in this level.
	XmlNodeRef missionsNode = root->newChild( "Missions" );
	int numMissions = GetIEditor()->GetDocument()->GetMissionCount();
	for (int i = 0; i < numMissions; i++)
	{
		CMission *pMission = GetIEditor()->GetDocument()->GetMission(i);
		XmlNodeRef missionNode = missionsNode->newChild( "Mission" );
		missionNode->setAttr( "Name",pMission->GetName() );
		missionNode->setAttr( "Description",pMission->GetDescription() );
	}


	//////////////////////////////////////////////////////////////////////////
	// Save LevelInfo file.
	//////////////////////////////////////////////////////////////////////////
	CString filename = path + "LevelInfo.xml";
	XmlString xmlData = root->getXML();

	CMemFile file;
	file.Write( xmlData.c_str(),xmlData.length() );
	m_levelPakFile.UpdateFile( filename,file );
}

void CGameExporter::ExportMapIni( const CString &path )
{
	/*
	m_IEditor->SetStatusText("Exporting map settings...");
	CLogFile::WriteLine("Map settings...");

	// Construct the filename of the map INI file
	char szFileOutputPath[1024];
	char szBuffer[1024];
	sprintf(szFileOutputPath, "%smap.ini", path);

	// Delete any old INI file
	if (PathFileExists(szFileOutputPath))
	{
		if (!m_IEditor->GetDocument()->FileDelete(szFileOutputPath))
		{
			AfxMessageBox("Can't rebuilt map.ini file !");
			CLogFile::WriteLine("Can't rebuilt map.ini file !");
		}
	}
	
	// Write the version of the file format
	VERIFY(WritePrivateProfileString("File", "MapFormatVersion", "1.0", szFileOutputPath));

	// Write the creation time of the file
	_strdate(szBuffer);
	VERIFY(WritePrivateProfileString("File", "MapCreationDate", szBuffer, szFileOutputPath));

	// Write the name of the map
	strcpy(szBuffer, LPCTSTR(m_IEditor->GetDocument()->GetTitle()));
	PathRemoveExtension(szBuffer);
	VERIFY(WritePrivateProfileString("Map", "MapName", szBuffer, szFileOutputPath));

	CHeightmap &HeightMap=m_IEditor->GetDocument()->GetHeightmap();
	// Write the size of the heightmap
	ASSERT(HeightMap.GetHeight() == HeightMap.GetWidth());
	sprintf(szBuffer, "%i", HeightMap.GetWidth());
	VERIFY(WritePrivateProfileString("Map", "HeightmapSize", szBuffer, szFileOutputPath));
	
	// Write the water level
	sprintf(szBuffer, "%i", (int)HeightMap.GetWaterLevel() );
	VERIFY(WritePrivateProfileString("Map", "MapWaterLevel", szBuffer, szFileOutputPath));

	// Write the water color.
	sprintf(szBuffer, "%x", m_IEditor->GetDocument()->GetWaterColor());
	VERIFY(WritePrivateProfileString("Map", "MapWaterColor", szBuffer, szFileOutputPath));

	//////////////////////////////////////////////////////////////////////////
	CLogFile::WriteLine("...Success");

	ExportSurfaceTypes( szFileOutputPath );
	*/
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::ExportMapInfo( XmlNodeRef &node )
{
	XmlNodeRef info = node->newChild( "LevelInfo" );

	// Write the creation time of the file
	char szBuffer[1024];
	_strdate(szBuffer);
	info->setAttr( "CreationDate",szBuffer );
	
	strcpy(szBuffer, LPCTSTR(m_IEditor->GetDocument()->GetTitle()));
	PathRemoveExtension(szBuffer);
	info->setAttr( "Name",szBuffer );

	CHeightmap *heightmap = m_IEditor->GetHeightmap();
	if (heightmap)
	{
		info->setAttr( "HeightmapSize",heightmap->GetWidth() );
		info->setAttr( "HeightmapUnitSize",heightmap->GetUnitSize() );
		info->setAttr( "WaterLevel",heightmap->GetWaterLevel() );
	}

	// Serialize surface types.
	CXmlArchive xmlAr;
	xmlAr.bLoading = false;
	xmlAr.root = node;
	m_IEditor->GetDocument()->SerializeSurfaceTypes( xmlAr );
}

//////////////////////////////////////////////////////////////////////////
bool CGameExporter::ExportSurfaceTexture( const CString &path,uint *pSurface,int iExportTexWidth,bool bHiQuality )
{
	unsigned int iTexSX, iTexSY;
	unsigned int iCurTexelX, iCurTexelY;
	uint dwBlendColor,dwClr1, dwClr2;
	uint iStartPosX, iStartPosY;

	CHeightmap *heightmap = m_IEditor->GetHeightmap();

	heightmap->SetSurfaceTextureSize(iExportTexWidth,iExportTexWidth);

	SSectorInfo sectorInfo;
	heightmap->GetSectorsInfo( sectorInfo );
	int lSectorDimensions = sectorInfo.sectorTexSize;

	m_IEditor->SetStatusText("Exporting surface texture (Saving)...");
	CLogFile::WriteLine("Saving surface texture...");

	// Fix seams on texture
	for (iCurTexelY=1; iCurTexelY<iExportTexWidth; iCurTexelY++)
	{
		for (iCurTexelX=1; iCurTexelX<iExportTexWidth; iCurTexelX++)
		{
				// Horizontal seam ?
			if (iCurTexelX % lSectorDimensions == 0)
			{
				// Get the two neighbor colors from the texture
				dwClr1 = pSurface[iCurTexelX + iCurTexelY * iExportTexWidth];
				dwClr2 = pSurface[iCurTexelX - 1 + iCurTexelY * iExportTexWidth];

				// Blend the two colors
				dwBlendColor = ((127 * (dwClr1 & 0x000000FF)        +  (dwClr2 & 0x000000FF)        * 127) >> 8)      |
					(((127 * (dwClr1 & 0x0000FF00) >>  8) + ((dwClr2 & 0x0000FF00) >>  8) * 127) >> 8) << 8 |
					(((127 * (dwClr1 & 0x00FF0000) >> 16) + ((dwClr2 & 0x00FF0000) >> 16) * 127) >> 8) << 16;

				// Write the back
				pSurface[iCurTexelX + iCurTexelY * iExportTexWidth] = 
					pSurface[iCurTexelX - 1 + iCurTexelY * iExportTexWidth] = dwBlendColor;
			}

			// Vertical seam ?
			if (iCurTexelY % lSectorDimensions == 0)
			{
				// Get the two neighbor colors from the texture
				dwClr1 = pSurface[iCurTexelX + iCurTexelY * iExportTexWidth];
				dwClr2 = pSurface[iCurTexelX + iCurTexelY * iExportTexWidth - iExportTexWidth];

				// Blend the two colors
				dwBlendColor = ((127 * (dwClr1 & 0x000000FF)        +  (dwClr2 & 0x000000FF)        * 127) >> 8)      |
					(((127 * (dwClr1 & 0x0000FF00) >>  8) + ((dwClr2 & 0x0000FF00) >>  8) * 127) >> 8) << 8 |
					(((127 * (dwClr1 & 0x00FF0000) >> 16) + ((dwClr2 & 0x00FF0000) >> 16) * 127) >> 8) << 16;

				// Write the back
				pSurface[iCurTexelX + iCurTexelY * iExportTexWidth] = 
					pSurface[iCurTexelX + iCurTexelY * iExportTexWidth - iExportTexWidth] = dwBlendColor;
			}
		}
	}

	CTextureCompression texcomp;
	CImage sector;
	sector.Allocate( lSectorDimensions,lSectorDimensions );

	CMemFile ctcFile;
	ctcFile.Write( &lSectorDimensions,4 );

	CWaitProgress progress( "Compressing Surface Texture" );
	int counter = 0;

	int numSectors = iExportTexWidth/lSectorDimensions;

	CString ddsFilename;

	// Write the texture to the file
	for (iTexSY=0; iTexSY<iExportTexWidth / lSectorDimensions; iTexSY++)
	{
		// Calculate the start position in the array
		iStartPosY = iTexSY * lSectorDimensions;

		for (iTexSX=0; iTexSX<iExportTexWidth / lSectorDimensions; iTexSX++)
		{
			// Calculate the start position in the array
			iStartPosX = iTexSX * lSectorDimensions;

			// Write the sector dimensions
			//file.Write( &lSectorDimensions,4 );

			// Write sector.
			for (iCurTexelY=0; iCurTexelY<lSectorDimensions; iCurTexelY++)
			{
				for (iCurTexelX=0; iCurTexelX<lSectorDimensions; iCurTexelX++)
				{
					// Write the texel to the file
					sector.ValueAt(iCurTexelX,iCurTexelY) = pSurface[(iStartPosX + iCurTexelX)+(iStartPosY + iCurTexelY)*iExportTexWidth];
				}
			}

			/*
			// Save sector.
			CMemFile ddsFile;
			ddsFilename.Format( "%sTerrain\\Texture_%d.dds",(const char*)path,counter );
			texcomp.CompressDDS( ddsFile,sector,bHiQuality );
			m_texturePakFile.UpdateFile( ddsFilename,ddsFile );
			*/

			texcomp.CompressDXT1( ctcFile,sector,bHiQuality );

			counter++;
			if (!progress.Step( (counter*100)/(numSectors*numSectors) ))
				return false;

			/*
			// Write the texture data to the file
			for (iCurTexelY=0; iCurTexelY<lSectorDimensions; iCurTexelY++)
			{
				for (iCurTexelX=0; iCurTexelX<lSectorDimensions; iCurTexelX++)
				{
					// Write the texel to the file
					file.Write( &pSurface[(iStartPosX + iCurTexelX) + (iStartPosY + iCurTexelY)*iExportTexWidth],3 );
				}
			}
			*/
		}
	}

	CString ctcFilename;
	ctcFilename.Format( "%sTerrain\\cover.ctc",(const char*)path );
	//m_texturePakFile.UpdateFile( ctcFilename,ctcFile );
	m_levelPakFile.UpdateFile( ctcFilename,ctcFile,false ); // Do not compress this file.
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::ExportAIGraph( const CString &path )
{ 
	GetIEditor()->SetStatusText( _T("Exporting AI Graph...") );

	char szLevel[1024];
	char szMission[1024];
	char fileName[1024];
	char fileHideName[1024];
	strcpy( szLevel,path );
	strcpy( szMission, m_IEditor->GetDocument()->GetCurrentMission()->GetName() );
	PathRemoveBackslash(szLevel);
	sprintf( fileName,"%s\\net%s.bai",szLevel,szMission );
	sprintf( fileHideName,"%s\\hide%s.bai",szLevel,szMission );
	
  if (m_IEditor->GetSystem()->GetAISystem()->GetNodeGraph())
	{
	  CLogFile::FormatLine( "Exporting AI Graph to %s",fileName );
	  m_IEditor->GetSystem()->GetAISystem()->GetNodeGraph()->WriteToFile( fileName );
	  CLogFile::FormatLine( "Exporting Hide Graph to %s",fileHideName );
	  m_IEditor->GetSystem()->GetAISystem()->GetHideGraph()->WriteToFile( fileHideName);

		// Read theose files back and put them to Pak file.
		CFile file;
		if (file.Open( fileName,CFile::modeRead ))
		{
			CMemoryBlock mem;
			mem.Allocate( file.GetLength() );
			file.Read( mem.GetBuffer(),file.GetLength() );
			file.Close();
			m_levelPakFile.UpdateFile( fileName,mem );
			DeleteFile( fileName );
		}
		if (file.Open( fileHideName,CFile::modeRead ))
		{
			CMemoryBlock mem;
			mem.Allocate( file.GetLength() );
			file.Read( mem.GetBuffer(),file.GetLength() );
			file.Close();
			m_levelPakFile.UpdateFile( fileHideName,mem );
			DeleteFile( fileHideName );
		}
	} 
	CLogFile::WriteLine( "Exporting AI Graph done." );

	//m_IEditor->GetSystem()->GetAISystem()->GetNodeGraph()->ReadFromFile( fileName );
	//m_IEditor->GetSystem()->GetAISystem()->GetNodeGraph()->RemoveIndoorNodes();
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::FlattenHeightmap( uint16 *pSaveHeightmapData,int width,int height )
{
	float fDist,fMaxDist;
	int iX, iY;
	float fAttenuation;
	int i,j;
	int iStartPosX, iStartPosY;
	Vec3 op;

	std::vector<CBaseObject*> objects;
	m_IEditor->GetObjectManager()->GetObjects( objects );

		// Mark holes and used flag.
	for (j=0; j<height; j++)
	{
		for (i=0; i<width; i++)
		{
			// Clear inuse and hole bit.
			pSaveHeightmapData[i + j * width] &= ~TERRAIN_BITMASK;
		}
	}

	CHeightmap *pHeightmap = m_IEditor->GetHeightmap();
	int unitSize = pHeightmap->GetUnitSize();

	// Encoding reserved bit and flatten the ground in the marked area
	// of map objects
	for (i=0; i < objects.size(); i++)
	{  
		CBaseObject *obj = objects[i];

		int iEmptyRadius = ftoi(obj->GetArea()/2.0f);

		// Skip if the object has no empty radius
		if (iEmptyRadius == 0)
			continue;

		// Swap X/Y
		op = obj->GetWorldPos();
		float fZ = m_IEditor->GetTerrainElevation(op.x,op.y);
		int objX = ftoi( op.y/unitSize );
		int objY = ftoi( op.x/unitSize );

		//m_vegetationMap->ClearObjects( CPoint(objX*UNIT_SIZE,objY*UNIT_SIZE),2*iEmptyRadius );

		if (!obj->CheckFlags(OBJFLAG_FLATTEN))
			continue;
 
		// Calculate the maximum distance to be able to calculate attenuation
		// inside the loop
		fMaxDist = (float) sqrtf(iEmptyRadius*iEmptyRadius + iEmptyRadius*iEmptyRadius);

		// Mark the area for this object
		for (iStartPosX= -(iEmptyRadius + 15); iStartPosX<iEmptyRadius+15; iStartPosX++)
		{
			for (iStartPosY= -(iEmptyRadius + 15); iStartPosY<iEmptyRadius + 15; iStartPosY++)
			{
				// Calculate current position
				iX = objX + iStartPosX;
				iY = objY + iStartPosY;

				// Skip invalid positions
				if (iX < 0 || iY < 0 || iX >= width || iY >= height)
				{
					continue;
				}

				// Calclate the attenuation for the current distance
				fDist = sqrtf((float) (abs(iStartPosX) * abs(iStartPosX) + abs(iStartPosY) * abs(iStartPosY)));
				fDist = __max(0.0f, fDist - 15);
				fAttenuation = 1.0f - __min(1.0f, fDist / fMaxDist);
				
				// Make the area around the building flat
				uint16 *src = &pSaveHeightmapData[iX+iY*width];
				unsigned int h = ftoi(fAttenuation * (fZ * 256.0f) + (1.0f - fAttenuation) * (*src));
				
				*src &= TERRAIN_BITMASK; // Leave only bitsflags.
				*src |= h & ~(TERRAIN_BITMASK); // Combine height width flags.

				if (fAttenuation < 0.3)
				 continue;

				// Set the third bit
				pHeightmap->InfoAt(iX,iY) &= ~HEIGHTMAP_INFO_SFTYPE_MASK;
			}
		}
	}

	// Mark holes and used flag.
	for (j=0; j<height; j++)
	{
		for (i=0; i<width; i++)
		{
			unsigned char hinfo = pHeightmap->InfoAt(i,j);
			uint sfType = ((hinfo&HEIGHTMAP_INFO_SFTYPE_MASK) >> HEIGHTMAP_INFO_SFTYPE_SHIFT);
			pSaveHeightmapData[i + j*width] |= sfType;
			if (hinfo&HEIGHTMAP_INFO_HOLE)
			{
				pSaveHeightmapData[i + j*width] |= SURFACE_TYPE_MASK;
			}
			if (hinfo&HEIGHTMAP_INFO_LIGHT)
			{
				pSaveHeightmapData[i + j*width] |= TERRAIN_LIGHT_BIT;
			}

			//pSaveHeightmapData[i + j*width] |= LIGHT_BIT;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::ExportLowResSurfaceTexture( const char* pszGamePath,CImage &surfaceTexture )
{
	char szLowLodFilePath[256];
	sprintf(szLowLodFilePath, "%sTerrain\\cover_low.dds", pszGamePath);

	if (!CFileUtil::OverwriteFile( szLowLodFilePath ))
		return;

	// Saving low-res version for terrain rendering optimizations
	uint *pSurface = surfaceTexture.GetData();
	uint iExportTexWidth = surfaceTexture.GetWidth();

	SSectorInfo info;
	GetIEditor()->GetHeightmap()->GetSectorsInfo(info);
	int nTerrainSize = info.numSectors*info.sectorSize;
	int k = 2*(iExportTexWidth/nTerrainSize);
	if(k<4)
		k=4;

	int nTexSize = surfaceTexture.GetWidth()/k;
	CLogFile::FormatLine("Saving lowres version of surface texture %dx%d...",nTexSize,nTexSize );

	CImage lowres;
	lowres.Allocate(nTexSize,nTexSize);
	uchar *pData = (uchar*)lowres.GetData();
	if (!pData)
		return;
	for(int x=0; x<nTexSize; x++)
	{
		for(int y=0; y<nTexSize; y++)
		{
			pData[(y*nTexSize+x)*4+0] = (pSurface[(x*k) + (y*k) * iExportTexWidth] & 0x000000FF)      ;
			pData[(y*nTexSize+x)*4+1] = (pSurface[(x*k) + (y*k) * iExportTexWidth] & 0x0000FF00) >>  8;
			pData[(y*nTexSize+x)*4+2] = (pSurface[(x*k) + (y*k) * iExportTexWidth] & 0x00FF0000) >> 16;
			pData[(y*nTexSize+x)*4+3] = 255;
		}
	}

	CMemFile ddsFile;
	CTextureCompression texcomp;
	texcomp.CompressDDS( ddsFile,lowres,true );
	//m_texturePakFile.UpdateFile( szLowLodFilePath,ddsFile );
	m_levelPakFile.UpdateFile( szLowLodFilePath,ddsFile );
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::ExportBrushes( const CString &path )
{
	GetIEditor()->SetStatusText( _T("Exporting Brushes...") );

	CBrushExporter brushExport;
	brushExport.ExportBrushes( path,m_levelPath,m_levelPakFile );
}

void CGameExporter::ForceDeleteFile( const char *filename )
{
	SetFileAttributes( filename,FILE_ATTRIBUTE_NORMAL );
	DeleteFile( filename );
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::DeleteOldFiles(  const CString &levelDir,bool bSurfaceTexture )
{
	ForceDeleteFile( levelDir + "brush.lst" );
	ForceDeleteFile( levelDir + "particles.lst" );
	ForceDeleteFile( levelDir + "LevelData.xml" );
	ForceDeleteFile( levelDir + "MovieData.xml" );
	ForceDeleteFile( levelDir + "objects.lst" );
	ForceDeleteFile( levelDir + "objects.idx" );
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::ExportTerrainBeaches( const CString &path )
{
	/*
	CMemFile file;
	CTerrainBeachGenerator terrGen( GetIEditor()->GetHeightmap() );
	terrGen.Generate( file );
	m_levelPakFile.UpdateFile( path + "Terrain\\beach7.tmp",file );
	*/
	
	////////////////////////////////////////////////////////////////////////
	// Recalculate shore geometry and save it to disk
	////////////////////////////////////////////////////////////////////////
	m_I3DEngine->RecompileBeaches();
	// Read file back and put to pack file and delete from disk.
	CString filename = Path::AddBackslash(path) + TERRAIN_BEACHES_FILENAME;
	CFile file;
	if (file.Open( filename,CFile::modeRead ))
	{
		CMemoryBlock mem;
		mem.Allocate( file.GetLength() );
		file.Read( mem.GetBuffer(),file.GetLength() );

		m_levelPakFile.UpdateFile( filename,mem );
		file.Close();
		DeleteFile( filename );
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::ExportMusic( XmlNodeRef &levelDataNode,const CString &path )
{
	// Export music manager.
	CMusicManager *pMusicManager = GetIEditor()->GetMusicManager();
	pMusicManager->Export( levelDataNode );

	CString musicPath = Path::AddBackslash(path) + "Music";
	CString filename = Path::Make( musicPath,MUSIC_LEVEL_LIBRARY_FILE );

	bool bEmptyLevelLib = true;
	XmlNodeRef nodeLib = new CXmlNode( "MusicThemeLibrary" );
	// Export Music local level library.
	for (int i = 0; i < pMusicManager->GetLibraryCount(); i++)
	{
		IDataBaseLibrary *pLib = pMusicManager->GetLibrary(i);
		if (pLib->IsLevelLibrary())
		{
			if (pLib->GetItemCount() > 0)
			{
				bEmptyLevelLib = false;
				// Export this library.
				pLib->Serialize( nodeLib,false );
			}
		}
		else
		{
			if (pLib->GetItemCount() > 0)
			{
				// Export this library to pak file.
				XmlNodeRef nodeLib = new CXmlNode( "MusicThemeLibrary" );
				pLib->Serialize( nodeLib,false );
				CMemFile file;
				XmlString xmlData = nodeLib->getXML();
				file.Write( xmlData.c_str(),xmlData.length() );
				CString filename = Path::Make( musicPath,Path::GetFile(pLib->GetFilename()) );
				m_levelPakFile.UpdateFile( filename,file );
			}
		}
	}
	if (!bEmptyLevelLib)
	{
		XmlString xmlData = nodeLib->getXML();

		CMemFile file;
		file.Write( xmlData.c_str(),xmlData.length() );
		m_levelPakFile.UpdateFile( filename,file );
	}
	else
	{
		m_levelPakFile.RemoveFile( filename );
	}
}

//////////////////////////////////////////////////////////////////////////
void CGameExporter::ExportMaterials( XmlNodeRef &levelDataNode,const CString &path )
{
	//////////////////////////////////////////////////////////////////////////
	// Export materials manager.
	CMaterialManager *pManager = GetIEditor()->GetMaterialManager();
	pManager->Export( levelDataNode );

	CString filename = Path::Make( path,MATERIAL_LEVEL_LIBRARY_FILE );

	bool bHaveItems = true;

	int numMtls = 0;

	XmlNodeRef nodeMaterials = new CXmlNode( "MaterialsLibrary" );
	// Export Materials local level library.
	for (int i = 0; i < pManager->GetLibraryCount(); i++)
	{
		XmlNodeRef nodeLib = nodeMaterials->newChild( "Library" );
		CMaterialLibrary *pLib = (CMaterialLibrary*)pManager->GetLibrary(i);
		if (pLib->GetItemCount() > 0)
		{
			bHaveItems = false;
			// Export this library.
			numMtls += pManager->ExportLib( pLib,nodeLib );
		}
	}
	if (!bHaveItems)
	{
		XmlString xmlData = nodeMaterials->getXML();

		CMemFile file;
		file.Write( xmlData.c_str(),xmlData.length() );
		m_levelPakFile.UpdateFile( filename,file );
	}
	else
	{
		m_levelPakFile.RemoveFile( filename );
	}
	m_numExportedMaterials = numMtls;
}