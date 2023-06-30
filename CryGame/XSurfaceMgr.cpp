
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// XSurfaceMgr.cpp: implementation of the CXSurfaceMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XSurfaceMgr.h"
#include <ICryPak.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CXSurfaceMgr::CXSurfaceMgr()
{
	m_mat_default_id = -1;
	
	m_pScriptSystem=NULL;
	m_pObjectMaterials=NULL;
	m_pMaterialScriptObject=NULL;
	Reset();
}

//////////////////////////////////////////////////////////////////////////
CXSurfaceMgr::~CXSurfaceMgr()
{
	if(m_pMaterialScriptObject)
		m_pMaterialScriptObject->Release();
	if(m_pObjectMaterials)
		m_pObjectMaterials->Release();
}

//////////////////////////////////////////////////////////////////////////
void CXSurfaceMgr::Init(IScriptSystem *pScriptSystem,I3DEngine *p3DEngine,IPhysicalWorld *pPhysicalWorld)
{
	m_pScriptSystem=pScriptSystem;
	m_p3DEngine=p3DEngine;
	m_pPhysicalWorld=pPhysicalWorld;

	m_p3DEngine->SetPhysMaterialEnumerator(this);
	m_pMaterialScriptObject=m_pScriptSystem->CreateEmptyObject();
	m_pObjectMaterials=m_pScriptSystem->CreateGlobalObject("Materials");	
}

//////////////////////////////////////////////////////////////////////////
void CXSurfaceMgr::OnElementFound(const char *sName,ScriptVarType type)
{
	if(type==svtObject)
	{
		AddMaterial(sName);
	}
}

//////////////////////////////////////////////////////////////////////////
int CXSurfaceMgr::GetSurfaceIDByMaterialName(const char *sMaterialName)
{
	if (!sMaterialName || strlen(sMaterialName) == 0)
	{
		return GetDefaultMaterial();
	}
	MaterialsMapItor itor = m_mapMaterials.find( sMaterialName );
	if (itor != m_mapMaterials.end())
	{
		return itor->second;
	}
	// Material not found... try to load it.
	return LoadMaterial(sMaterialName); // [Anton]
}

//////////////////////////////////////////////////////////////////////////
int CXSurfaceMgr::EnumPhysMaterial(const char * szPhysMatName)
{
	int nSurfaceID=GetSurfaceIDByMaterialName(szPhysMatName);

	return nSurfaceID;
}

//////////////////////////////////////////////////////////////////////////
bool CXSurfaceMgr::IsCollidable(int nMatId)
{
	PhysicalSurfacesPropsMapItor It=m_mapMaterialProps.find(nMatId);
	if (It==m_mapMaterialProps.end())
		return false;
	SMatProps &Props=It->second;
	return !Props.bNoCollide;
}

#if !defined(PS2) && !defined(LINUX)
#include <io.h>
#endif
//////////////////////////////////////////////////////////////////////////
int CXSurfaceMgr::LoadMaterial(const string &sMaterialName,bool bLoadAlsoIfDuplicate,int nForceID)
{ 
	int matid = m_mat_default_id;
	MaterialsNamesMapItor itor=m_mapMaterialsNames.find(sMaterialName);
	if(itor!=m_mapMaterialsNames.end())
	{
		//check if the material is already loaded
		if(!bLoadAlsoIfDuplicate)
		{
			MaterialsMapItor mit = m_mapMaterials.find( sMaterialName );
			if (mit != m_mapMaterials.end())
			{
				// This material is already loaded.
				return mit->second;
			}
		}
		MatDesc &mdesc = itor->second;
		const char *sScriptFilename = mdesc.sScriptFilename.c_str();
		if (nForceID < 0)
			nForceID = mdesc.surfaceId;
		else
			mdesc.surfaceId = nForceID;
		
		matid = nForceID;
		if (!mdesc.sScriptFilename.empty())
		{		
			//add the material
			if(m_pScriptSystem->ExecuteFile( sScriptFilename,true,false ))
			{
				return AddMaterial(sMaterialName,nForceID);
			}
			else
			{
				if (m_mapMaterialsNames.empty())
					GameWarning("Attempting to load material %s before material list is loaded (script %s)", sMaterialName.c_str(),sScriptFilename);
				else
					GameWarning( "Physical material %s failed to load from script %s",sMaterialName.c_str(),sScriptFilename );
			}
		}
	}
	else
	{
		GameWarning( "Unknown physical material %s",sMaterialName.c_str() );
	}
	return matid;
}

//////////////////////////////////////////////////////////////////////////
int CXSurfaceMgr::GetDefaultMaterial()
{
	if (m_mat_default_id < 0)
	{
		m_mat_default_id = LoadMaterial("mat_default",false,255);
		// If failed to load default.
		if (m_mat_default_id < 0)
			m_mat_default_id = 0;
	}
	return m_mat_default_id;
}

void CXSurfaceMgr::ReloadMaterials()
{
	m_mapMaterialsNames.clear();
	MaterialsNamesMapItor itor=m_mapMaterialsNames.begin();
	while(itor!=m_mapMaterialsNames.end())
	{
		MatDesc &mdesc = itor->second;
		if(m_pScriptSystem->ExecuteFile(mdesc.sScriptFilename.c_str(),true,true))
		{
			ReloadMaterialPhysics(itor->first.c_str());
		}
		++itor;
	}
}
//////////////////////////////////////////////////////////////////////////
bool CXSurfaceMgr::LoadMaterials( const string &sFolder,bool bReload,bool bAddMaterials )
{
	_SmartScriptObject pDummy(m_pScriptSystem);

	if (!bAddMaterials)
	{
		m_mapMaterialsNames.clear();
		m_pObjectMaterials->Clear();
	}

	// Reserve first 100 ids for terrain.
	int surfaceId = 102; // 100,101 reserved for default and water.

	//_bstr_t sSearchPattern=sPath+_T("\\")+_T("*.*");
	struct _finddata_t c_file;
	intptr_t hFile;
	string sSearchPattern = sFolder+"\\*.*";
	char fName[_MAX_PATH];
#ifndef _XBOX
	strcpy(fName, sSearchPattern.c_str());
#else
	_ConvertNameForXBox(fName, sSearchPattern.c_str());
#endif
	ICryPak *pIPak = GetISystem()->GetIPak();

	if( (hFile = pIPak->FindFirst( fName, &c_file )) == -1L )
	{
		//m_pLog->LogToConsole( "No *.lua files in current directory!\n" );
		return true;
	}
	else 
	{
		do{

			if(!((!strncmp(c_file.name,".",1)) &&  (c_file.attrib &_A_SUBDIR)))
			{
				if(c_file.attrib & _A_SUBDIR)
				{
					LoadMaterials(sFolder+"/"+string(c_file.name));
				}
				else{

					//Check for the extension .lua
					if(strlen(c_file.name)>=4)
					{
						if(!stricmp(&c_file.name[strlen(c_file.name)-4],".lua"))
						{
							char sMaterialName[256];
							strncpy(sMaterialName,c_file.name,strlen(c_file.name)-4);
							sMaterialName[strlen(c_file.name)-4]='\0';
							string sFilePath = sFolder+"/"+string(c_file.name);
							//m_pLog->Log( "Loading %s ",sFile.c_str());
							//TRACE("Loading MatName %s [%s]",sMaterialName,sFilePath.c_str());

							MatDesc mdesc;
							mdesc.sScriptFilename = sFilePath;
							mdesc.surfaceId = surfaceId++;
							m_mapMaterialsNames[sMaterialName] = mdesc;

							//create the material field to allow the editor to display the materials into the materials combobox
							m_pObjectMaterials->SetValue(sMaterialName,pDummy);

							CryLogComment( "Loading Phys Material: %d, %s (%s)",mdesc.surfaceId,sMaterialName,sFilePath.c_str(),mdesc.sScriptFilename.c_str() );

							/*if(!bReload)
							{
							m_pScriptSystem->ExecuteFile(sFilePath.c_str());
							}
							else
							{
							m_pScriptSystem->ReloadScript(sFilePath.c_str());
							}*/
						}
					}
				}
			}

		}while(pIPak->FindNext( hFile, &c_file ) == 0);

		pIPak->FindClose( hFile );

	}


	LoadDefaults();

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Load default materials.
//////////////////////////////////////////////////////////////////////////
void CXSurfaceMgr::LoadDefaults()
{
	m_mat_default_id = LoadMaterial("mat_default",false,100);
	LoadMaterial("mat_water",false,101);
}

//////////////////////////////////////////////////////////////////////////
void CXSurfaceMgr::InitPhisicalSurfaces() {}

//////////////////////////////////////////////////////////////////////////
int CXSurfaceMgr::AddMaterial( const string &sMaterial,int nForceID)
{
  _SmartScriptObject pScriptObject(m_pScriptSystem,true);
	int nID=m_nLastFreeSurfaceID;
	
	if(nForceID>-1)
		nID=nForceID;
	 else
		m_nLastFreeSurfaceID++;

	m_mapPhysSurfaces[nID] = sMaterial;
	m_mapMaterials[sMaterial] = nID;
	m_mapMaterialProps[nID] = SMatProps();

	if(m_pObjectMaterials->GetValue(sMaterial.c_str(),*pScriptObject))
		SetMaterialGameplayPhysic( nID, pScriptObject);

	return nID;
}

//////////////////////////////////////////////////////////////////////////
void CXSurfaceMgr::ReloadMaterialPhysics(const char *sMaterialName)
{
	LoadMaterial(sMaterialName);

	MaterialsMapItor itor = m_mapMaterials.find(sMaterialName);

	if(itor==m_mapMaterials.end())
		return;

	int nID = itor->second;

	_SmartScriptObject pScriptObject(m_pScriptSystem,true);

	if(m_pObjectMaterials->GetValue(sMaterialName,*pScriptObject))
		SetMaterialGameplayPhysic( nID, pScriptObject);
}

//////////////////////////////////////////////////////////////////////////
void CXSurfaceMgr::SetMaterialGameplayPhysic( int nId, _SmartScriptObject &table )
{
	_SmartScriptObject pGameplayPhysic(m_pScriptSystem,true);
	float fBouncyness=0.0f;
	float fFriction=1.0f;
	int		iPiercingResistence=sf_max_pierceable;	// physics taces range 0-15
	PhysicalSurfacesPropsMapItor It=m_mapMaterialProps.find(nId);
	ASSERT(It!=m_mapMaterialProps.end());
	SMatProps &MatProps=It->second;

	if(table->GetValue("gameplay_physic",*pGameplayPhysic))
	{
		pGameplayPhysic->GetValue("friction",fFriction);
		pGameplayPhysic->GetValue("bouncyness",fBouncyness);

		if(	pGameplayPhysic->GetValue("piercing_resistence",iPiercingResistence) )
		{
			if(iPiercingResistence>sf_max_pierceable)
				iPiercingResistence = sf_max_pierceable;
			iPiercingResistence = sf_max_pierceable - iPiercingResistence;
		}
		pGameplayPhysic->GetValue("no_collide", MatProps.bNoCollide);

		// piercable surfaces that still need to be processed normaly (e.g. mat_arm) need the following flag
		int bImportant;

		if (pGameplayPhysic->GetValue("important",bImportant) && bImportant)
			iPiercingResistence |= sf_important;
	}
	m_pPhysicalWorld->SetSurfaceParameters(nId,fBouncyness,fFriction,iPiercingResistence);
}

//////////////////////////////////////////////////////////////////////////
void CXSurfaceMgr::SetTerrainSurface( const string &sMaterial,int nSurfaceID)
{
	//TRACE("----Terrain Surface [%d] = \"%s\"",nSurfaceID,sMaterial.c_str());
//	m_vTerrainSurfaces[nSurfaceID]=sMaterial;
	//load the material
	//NOTE :The a new surface id is generated also if the same material
	//is altready mapped to a surface id,this to map correctly the terrain surface id(that we cannot modify)
	//to the correct material.The result is that more than 1 surface id can be mapped
	//on a certain material.
	LoadMaterial( sMaterial,true,nSurfaceID);
}

//////////////////////////////////////////////////////////////////////////
void CXSurfaceMgr::Reset()
{
	m_nLastFreeSurfaceID=10;
//	m_vTerrainSurfaces.clear();
//	m_vTerrainSurfaces.resize(10);
	if(m_pScriptSystem && m_pObjectMaterials)
	{
		// Creates a new "Materials" table.
		SAFE_RELEASE( m_pObjectMaterials );
		m_pObjectMaterials = m_pScriptSystem->CreateGlobalObject("Materials");
	}
	m_mapPhysSurfaces.clear();
	m_mapMaterials.clear();
	m_mapMaterialProps.clear();
	m_mat_default_id = -1;
#if !defined(LINUX)
	LoadDefaults();
#endif
}


//////////////////////////////////////////////////////////////////////////
IScriptObject * CXSurfaceMgr::GetMaterialByName( const char *sMaterialName )
{
	if(!m_pObjectMaterials)
		return NULL;
	if(!m_pObjectMaterials->GetValue(sMaterialName,m_pMaterialScriptObject))
	{
		return NULL;
	}
	else
	{ 
		return m_pMaterialScriptObject;
	}
}

//////////////////////////////////////////////////////////////////////////
IScriptObject * CXSurfaceMgr::GetMaterialBySurfaceID(int nSurfaceID)
{
	PhysicalSurfecesMapItor itor;
	itor=m_mapPhysSurfaces.find(nSurfaceID);
	const char *sMaterialName = "mat_default";
	if(itor!=m_mapPhysSurfaces.end())
	{
		sMaterialName = itor->second.c_str();
	}
	return GetMaterialByName(sMaterialName);	
}

//////////////////////////////////////////////////////////////////////////
bool CXSurfaceMgr::IsMaterialBurnable(int nSurfaceID)
{
//  _SmartScriptObject pScriptObject(m_pScriptSystem,true);
IScriptObject *pScriptObject = GetMaterialBySurfaceID(nSurfaceID);
bool bBurnable=false;
	if(pScriptObject)
	{
		_SmartScriptObject pGameplayPhysic(m_pScriptSystem,true);
		if(pScriptObject->GetValue("gameplay_physic",*pGameplayPhysic))
		{
			pGameplayPhysic->GetValue("burnable",bBurnable);
		}
	}
	return bBurnable;
}

//////////////////////////////////////////////////////////////////////////
bool CXSurfaceMgr::GetMaterialParticlesTbl(int nSurfaceID, const char* tblName, ParticleParams &sParamOut, 
																					 IGame* pGame, ISystem* pSystem)
{
	return (false);
}

//////////////////////////////////////////////////////////////////////////
int	CXSurfaceMgr::GetMaterialCount()
{
	return m_mapMaterialsNames.size();
}

//////////////////////////////////////////////////////////////////////////
const char* CXSurfaceMgr::GetMaterialNameByIndex( int index )
{
	MaterialsNamesMap::iterator it = m_mapMaterialsNames.begin();
	std::advance( it,index );
	if (it != m_mapMaterialsNames.end())
		return it->first.c_str();
	return "";
}

