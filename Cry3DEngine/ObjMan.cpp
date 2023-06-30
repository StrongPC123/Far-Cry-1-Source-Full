////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjman.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Loading trees, buildings, ragister/unregister entities for rendering
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "objman.h"
#include "visareas.h"
#include "terrain_sector.h"
#include "cbuffer.h"
#include "3DEngine.h"
#include "meshidx.h"
#include "watervolumes.h"
#include "brush.h"
#include "LMCompStructures.h"

double CObjManager::m_dMakeObjectTime = 0;
double CObjManager::m_dCIndexedMesh__LoadMaterial = 0;
double CObjManager::m_dUpdateCustomLightingSpritesAndShadowMaps = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Register / Unregister in sectors
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void CObjManager::RegisterEntity( IEntityRender * pEntityRS )
{
  if (!m_pTerrain)
    return;

  const char * szClass = pEntityRS->GetEntityClassName();
  const char * szName = pEntityRS->GetName();
  if(!szName[0] && !szClass[0])
    return; // do not register undefined objects

//  if(strstr(szName, "Merc"))
  //  int y=0;

//	if(strstr(szName, "Player"))
	//	int y=0;

  int nStatic = (pEntityRS->GetEntityRenderType() != eERType_Unknown);

  // if draw near - register only in sector (0,0)
  // if current sector will not be visible - weapon will be drawn from sector (0,0)
  ICryCharInstance * cmodel = pEntityRS->GetEntityCharacter(0);    
  if (cmodel && (cmodel->GetFlags() & CS_FLAG_DRAW_MODEL) && (cmodel->GetFlags() & CS_FLAG_DRAW_NEAR))
  {
    list2<IEntityRender*> * pList = &m_pTerrain->m_arrSecInfoTable[0][0]->m_lstEntities[nStatic];
    if(pList->Find(pEntityRS)<0)
      pList->Add(pEntityRS);
    pEntityRS->m_pSector = m_pTerrain->m_arrSecInfoTable[0][0];
		pEntityRS->m_pVisArea = NULL;
    return;
  }

  // find pos in sectors array
  Vec3d vBMin,vBMax;
  pEntityRS->GetRenderBBox(vBMin,vBMax);
  Vec3d vCenter = (vBMin+vBMax)*0.5f;
/*
	if(nStatic && vBMax.z<m_pTerrain->GetZSafe(vCenter.x,vCenter.y))
		Warning(0,pEntityRS->GetName(),"%s is placed under the ground and not inside vis area: "
		"pos=%.2f,%.2f,%.2f, "
		"terrain elevation is %.2f, "
		"object name is %s",
		(pEntityRS->GetEntityRenderType()==eERType_Vegetation) ? "Vegetation" : "Brush",
		pEntityRS->GetPos().x, pEntityRS->GetPos().y, pEntityRS->GetPos().z, 
		m_pTerrain->GetZSafe(vCenter.x,vCenter.y),
		pEntityRS->GetName());
*/
//	if(strstr(szName, "Player"))
	//	int y=0;

/*
	if(pEntityRS->GetRndFlags()&ERF_CASTSHADOWVOLUME)
	{	// adjust bbox by shadow
		Vec3d vShadowOffset = m_p3DEngine->GetSunPosition().Normalized()*pEntityRS->GetRadius();
		Vec3d vBoxMin2 = vBoxMin-vShadowOffset;
		Vec3d vBoxMax2 = vBoxMax-vShadowOffset;
		vBoxMin.CheckMin(vBoxMin2);
		vBoxMax.CheckMax(vBoxMax2);
	}
	*/
  // get 2d pos in sectors array
  int x = (int)(((vCenter.x)/CTerrain::GetSectorSize()));
  int y = (int)(((vCenter.y)/CTerrain::GetSectorSize()));

  // if outside of the map, or too big - register in sector (0,0)
  if( vCenter.x<0 || vCenter.y<0 ||
		x<0 || x>=CTerrain::GetSectorsTableSize() || y<0 || y>=CTerrain::GetSectorsTableSize() ||
		(vBMax.x - vBMin.x)>TERRAIN_SECTORS_MAX_OVERLAPPING*2 || (vBMax.y - vBMin.y)>TERRAIN_SECTORS_MAX_OVERLAPPING*2)
    x = y = 0;

  CSectorInfo * & pSector = pEntityRS->m_pSector;

  if(pSector)
		UnRegisterEntity( pEntityRS );

  pSector = m_pTerrain->m_arrSecInfoTable[x][y];

  // add if not added
  if(pSector->m_lstEntities[nStatic].Find(pEntityRS)<0)
    pSector->m_lstEntities[nStatic].Add(pEntityRS);

  if(nStatic && pSector)
  {
    pSector->m_vBoxMin.CheckMin(vBMin);
    pSector->m_vBoxMax.CheckMax(vBMax);
  }
} 

bool CObjManager::UnRegisterEntity( IEntityRender* pEntityRS )
{
  if(!m_pTerrain)
    return false;

#ifdef _DEBUG
	const char * szName = pEntityRS->GetName();
	if(strstr(szName, "Player"))
		int y=0;
#endif // _DEBUG

  bool bFound = false;

  int nStatic = (pEntityRS->GetEntityRenderType() != eERType_Unknown);

  // unregister objects outside of the map and 1 person weapon
  if(m_pTerrain->m_arrSecInfoTable[0][0])
    bFound |= m_pTerrain->m_arrSecInfoTable[0][0]->m_lstEntities[nStatic].Delete(pEntityRS);

  // unregister from sectors
  CSectorInfo * & pSector = pEntityRS->m_pSector;
  if(!pSector)
    return false;

  // delete if found
  bFound |= pSector->m_lstEntities[nStatic].Delete(pEntityRS);

	if(nStatic)
	{ // remove references to this entity
		pSector->m_lstStaticShadowMapCasters.Delete(pEntityRS);
		for(int i=0; i<pSector->m_lstStatEntInfoVegetNoCastersNoVolFog.Count(); i++)
		if(pSector->m_lstStatEntInfoVegetNoCastersNoVolFog[i].m_pEntityRender == pEntityRS)
		{
			pSector->m_lstStatEntInfoVegetNoCastersNoVolFog.Delete(i);
			i--;
		}
		for(int i=0; i<pSector->m_lstStatEntInfoOthers.Count(); i++)
		if(pSector->m_lstStatEntInfoOthers[i].m_pEntityRender == pEntityRS)
		{
			pSector->m_lstStatEntInfoOthers.Delete(i);
			i--;
		}
	}

  pSector=0;

  return bFound;
} 

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Load static objects
//////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CObjManager::LoadStaticObjectsFromXML()
{
	XDOM::IXMLDOMNodeListPtr pVegetTagList;
	XDOM::IXMLDOMNodePtr pVegetTag;

	XDOM::IXMLDOMDocumentPtr pDoc = GetSystem()->CreateXMLDocument();
	if(!pDoc->load(Get3DEngine()->GetLevelFilePath("LevelData.xml")))
		return false;

	pVegetTagList = pDoc->getElementsByTagName("Vegetation");
	if (pVegetTagList)
	{
		pVegetTagList->reset();
		pVegetTag = pVegetTagList->nextNode();
		if (pVegetTag) // [marco] added this check - it was crashing here sometimes
		{		
		XDOM::IXMLDOMNodeListPtr pVegetList;
		pVegetList = pVegetTag->getElementsByTagName("Object");
		if (pVegetList)
		{
			pVegetList->reset();
			XDOM::IXMLDOMNodePtr pVeget;
			int nGroupId=0;
			while (pVeget = pVegetList->nextNode())
			{
				XDOM::IXMLDOMNodePtr pAlphaBlend			= pVeget->getAttribute("AlphaBlend");
				XDOM::IXMLDOMNodePtr pBending					= pVeget->getAttribute("Bending");
				XDOM::IXMLDOMNodePtr pBrightness			= pVeget->getAttribute("Brightness");
				XDOM::IXMLDOMNodePtr pCastShadow			= pVeget->getAttribute("CastShadow");
				XDOM::IXMLDOMNodePtr pFileName				= pVeget->getAttribute("FileName");
				XDOM::IXMLDOMNodePtr pHideable				= pVeget->getAttribute("Hideable");
				XDOM::IXMLDOMNodePtr pPhysNonColl			= pVeget->getAttribute("PhysNonColl");
				XDOM::IXMLDOMNodePtr pIndex						= pVeget->getAttribute("Index");
				XDOM::IXMLDOMNodePtr pPrecalcShadow		= pVeget->getAttribute("PrecalcShadow");
				XDOM::IXMLDOMNodePtr pRecvShadow			= pVeget->getAttribute("RecvShadow");
				XDOM::IXMLDOMNodePtr pSpriteDistRatio	= pVeget->getAttribute("SpriteDistRatio");
				XDOM::IXMLDOMNodePtr pShadowDistRatio	= pVeget->getAttribute("ShadowDistRatio");
//				XDOM::IXMLDOMNodePtr pUseLigthBit			= pVeget->getAttribute("UseLigthBit");
//				XDOM::IXMLDOMNodePtr pAmbScale				= pVeget->getAttribute("AmbScale");
				XDOM::IXMLDOMNodePtr pSpriteTexRes		= pVeget->getAttribute("SpriteTexRes");
				XDOM::IXMLDOMNodePtr pMaxViewDistRatio= pVeget->getAttribute("MaxViewDistRatio");
				XDOM::IXMLDOMNodePtr pMaterialNode		= pVeget->getAttribute("Material");
				XDOM::IXMLDOMNodePtr pBackSideLevel		= pVeget->getAttribute("BackSideLevel");
        XDOM::IXMLDOMNodePtr pCalcLighting		= pVeget->getAttribute("CalcLighting");
        XDOM::IXMLDOMNodePtr pUseSprites		  = pVeget->getAttribute("UseSprites");
				XDOM::IXMLDOMNodePtr pFadeSize				= pVeget->getAttribute("FadeSize");
				XDOM::IXMLDOMNodePtr pUpdateShadowEveryFrame = pVeget->getAttribute("RealTimeShadow");

				{
					IStatInstGroup siGroup;
					if(pAlphaBlend)
						siGroup.bUseAlphaBlending			= atof(pAlphaBlend->getText()) != 0;
					if(pBending)
						siGroup.fBending							= (float)atof(pBending->getText());
					if(pBrightness)
						siGroup.fBrightness						= (float)atof(pBrightness->getText());
					if(pCastShadow)
						siGroup.bCastShadow						= atof(pCastShadow->getText()) != 0;
					if(pRecvShadow)
						siGroup.bRecvShadow						= atof(pRecvShadow->getText()) != 0;
					if(pFileName)
						siGroup.pStatObj							= MakeObject(pFileName->getText(), NULL, 
            evs_ShareAndSortForCache, true, false, false );

					if(siGroup.pStatObj)
						siGroup.pStatObj->CheckValidVegetation();

					if(siGroup.pStatObj && siGroup.pStatObj->GetLeafBuffer() && siGroup.pStatObj->GetLeafBuffer()->m_pMats && siGroup.pStatObj->GetLeafBuffer()->m_pMats->Count()>4)
						GetLog()->Log("Warning: Number of materials in distributed object is %d", 
						siGroup.pStatObj->GetLeafBuffer()->m_pMats->Count());

					if(pHideable)
						siGroup.bHideability					= atof(pHideable->getText()) != 0;
					if(pPhysNonColl)
						siGroup.bPhysNonColl					= atof(pPhysNonColl->getText()) != 0;
					if(pPrecalcShadow)
						siGroup.bPrecShadow						= atof(pPrecalcShadow->getText()) != 0;
					if(pSpriteDistRatio)
						siGroup.fSpriteDistRatio			= (float)atof(pSpriteDistRatio->getText());
					if(pShadowDistRatio)
						siGroup.fShadowDistRatio			= (float)atof(pShadowDistRatio->getText());
					if(pMaxViewDistRatio)
						siGroup.fMaxViewDistRatio			= (float)atof(pMaxViewDistRatio->getText());
					if(pSpriteTexRes)
						siGroup.nSpriteTexRes					= (int)atoi(pSpriteTexRes->getText());
					if(pMaterialNode)
						siGroup.pMaterial             = Get3DEngine()->FindMaterial( pMaterialNode->getText() );
          if(pBackSideLevel)
						siGroup.fBackSideLevel        = (float)atof(pBackSideLevel->getText());
					if(pCalcLighting)
						siGroup.bCalcLighting         = atof(pCalcLighting->getText()) != 0;
					if(pUseSprites)
						siGroup.bUseSprites           = atof(pUseSprites->getText()) != 0;
					if(pFadeSize)
						siGroup.bFadeSize							= atof(pFadeSize->getText()) != 0;
					if(pUpdateShadowEveryFrame)
						siGroup.bUpdateShadowEveryFrame = atof(pUpdateShadowEveryFrame->getText()) != 0;

					if(siGroup.pStatObj->GetLeafBuffer() && !((CStatObj*)siGroup.pStatObj)->IsSpritesCreated() && !GetSystem()->IsDedicated())
						((CStatObj*)siGroup.pStatObj)->UpdateCustomLightingSpritesAndShadowMaps(m_vOutdoorAmbientColor, siGroup.nSpriteTexRes, siGroup.fBackSideLevel, siGroup.bCalcLighting );

					((CStatObj*)siGroup.pStatObj)->FreeTriData(); // source geometry is needed only for stencil shadows

					Get3DEngine()->SetStatInstGroup(nGroupId, siGroup);
					nGroupId++;
        }
			}
    }
    }
  }
	return true;
}

IStatObj * CObjManager::GetStaticObjectByTypeID(int nTypeID)
{
  if(nTypeID>=0 && nTypeID<m_lstStaticTypes.Count())
    return m_lstStaticTypes[nTypeID].pStatObj;

  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Init / Release
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _XBOX
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#else
#include <xtl.h>
#endif

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

void CObjManager::LoadVegetationModels(const char *szMissionName,bool bEditorMode)
{  
  if(GetCVars()->e_vegetation)
  {
    int nCountStart = m_lstLoadedObjects.size();
    float time = GetCurAsyncTimeSec();
  	LoadStaticObjectsFromXML();
    UpdateLoadingScreen("%d of %d static objects loaded in %.2f seconds", 
      m_lstLoadedObjects.size()-nCountStart, m_lstLoadedObjects.size(), GetCurAsyncTimeSec()-time );
  }
}

void CObjManager::UnloadVegetations()
{
	// unload vegetation types
	for(int i=0; i<m_lstStaticTypes.Count(); i++)
		if( m_lstStaticTypes[i].GetStatObj() )
		{
			ReleaseObject( m_lstStaticTypes[i].GetStatObj() );
			memset(&m_lstStaticTypes[i], 0, sizeof(m_lstStaticTypes[i]));
		}
	m_lstStaticTypes.Clear();
}

void CObjManager::CheckObjectLeaks(bool bDeleteAll)
{
	// deleting leaked objects
	if(m_lstLoadedObjects.size()>1)
		GetLog()->Log("Warning: CObjManager::CheckObjectLeaks: %d object(s) found in memory", m_lstLoadedObjects.size());

	for (ObjectsMap::iterator it = m_lstLoadedObjects.begin(); it != m_lstLoadedObjects.end(); ++it)
	{
		if(!(*it)->IsDefaultObject())
		{
			if((*it)->m_szGeomName[0])
				GetLog()->Log("Warning: object not deleted: %s / %s", (*it)->m_szFileName, (*it)->m_szGeomName);
			else
				GetLog()->Log("Warning: object not deleted: %s", (*it)->m_szFileName);
		}

		if(bDeleteAll)
			delete (*it);
	}

	if(bDeleteAll)
		m_lstLoadedObjects.clear();
}

void CObjManager::UnloadObjects()
{
	UnloadVegetations();

  // delete leaked objects
	CheckObjectLeaks(true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create / delete object
//////////////////////////////////////////////////////////////////////////////////////////////////////////

CStatObj * CObjManager::MakeObject(const char * __szFileName, 
                                   const char * _szGeomName, 
                                   EVertsSharing eVertsSharing,
                                   bool bLoadAdditinalInfo,
																	 bool bKeepInLocalSpace,
																	 bool bLoadLater)
{	
	AUTO_PROFILE_SECTION(GetTimer(), CObjManager::m_dMakeObjectTime);

	if(!strcmp(__szFileName,"NOFILE"))
	{ // make ampty object to be filled from outside
		CStatObj * pObject = new CStatObj( );
		pObject->RegisterUser();
		m_lstLoadedObjects.insert(pObject);
		return pObject;
	}

	assert(__szFileName && __szFileName[0]);

	char szFileName[MAX_PATH_LENGTH];

  // Normilize file name
	char *pszDest=szFileName;
	const char *pszSource=__szFileName;
	while (*pszSource)
	{
		if (*pszSource=='/')
			*pszDest++='\\';
		else 
			*pszDest++=*pszSource;
		pszSource++;
	}
	*pszDest=0;

  if(strstr(szFileName,".ccgf"))
  {
    int nLen = strlen(szFileName);
    strncpy(&szFileName[nLen-4],&szFileName[nLen-3],4);
  }

	// Construct tmp object for search
	CStatObj tmp;
	strcpy(tmp.m_szFileName, szFileName);
	strcpy(tmp.m_szGeomName, _szGeomName ? _szGeomName : "");
	tmp.m_bKeepInLocalSpace	= bKeepInLocalSpace;
	tmp.m_bLoadAdditinalInfo= bLoadAdditinalInfo;
	tmp.m_eVertsSharing			=	eVertsSharing;
	//	tmp.m_bCalcLighting			== s2->m_bCalcLighting);
	//	tmp.m_bMakePhysics			== s2->m_bMakePhysics);

  // Try to find already loaded object
	if (!_szGeomName || !*_szGeomName || strcmp(_szGeomName,"cloth"))
	{	// [Anton] - always use new cgf for objects used for cloth simulation
		ObjectsMap::iterator it = m_lstLoadedObjects.find( &tmp );
		if (it != m_lstLoadedObjects.end())
		{
			assert(	stricmp((*it)->m_szFileName, szFileName)==0 && // compare file name
					(!_szGeomName || stricmp((*it)->m_szGeomName, _szGeomName)==0)); // compare geom name

			(*it)->RegisterUser();
			return (*it);
		}

		// if ccfg was requested - change extension to cgf
		tmp.m_szFileName[strlen(tmp.m_szFileName)-2]=0;
		strcat(tmp.m_szFileName,"cgf");

		// Try to find already loaded object
		it = m_lstLoadedObjects.find( &tmp );
		if (it != m_lstLoadedObjects.end())
		{
			assert(	stricmp((*it)->m_szFileName, tmp.m_szFileName)==0 && // compare file name
				(!_szGeomName || stricmp((*it)->m_szGeomName, _szGeomName)==0)); // compare geom name

			(*it)->RegisterUser();
			return (*it);
		}
	}

	// Load new CGF
	CStatObj * pObject = new CStatObj( );
  if(!pObject->Load(szFileName, _szGeomName, eVertsSharing, bLoadAdditinalInfo, bKeepInLocalSpace, bLoadLater))
	{ 
		// object not found
		// if geom name is specified - just return 0
    if(_szGeomName && _szGeomName[0]) 
		{
			delete pObject; 
      return 0;
		}

		if (!m_pDefaultCGF)
			GetConsole()->Exit ("Error: CObjManager::MakeObject: Default object not found");

		// return default object
		m_pDefaultCGF->RegisterUser();
		delete pObject; 
		return m_pDefaultCGF;
  }

  // now try to load lods
	pObject->LoadLowLODs(eVertsSharing,bLoadAdditinalInfo,bKeepInLocalSpace,bLoadLater);

//  if(!bLoadLater && bGenSpritesAndShadowMap)
  //  pObject->UpdateCustomLightingSpritesAndShadowMaps(m_vOutdoorAmbientColor, 0, fBackSideLevel, bCalcLighting );

	pObject->RegisterUser();
  m_lstLoadedObjects.insert(pObject);

	return pObject;
}

bool CObjManager::ReleaseObject(CStatObj * pObject)
{
//	ObjectsMap::iterator it = m_lstLoadedObjects.find( pObject );
	//if (it != m_lstLoadedObjects.end())

	for (ObjectsMap::iterator it = m_lstLoadedObjects.begin(); it != m_lstLoadedObjects.end(); ++it)
		if((CStatObj*)(*it) == pObject)
	{
		assert(pObject == (CStatObj*)(*it));
		
		CStatObj* p = (CStatObj*)(*it);

    pObject->UnregisterUser();

    if(pObject->m_nUsers<=0 && !m_bLockCGFResources)
    {
      GetLog()->Log("Object unloaded: %s  %s",pObject->m_szFileName, pObject->m_szGeomName);
			m_lstLoadedObjects.erase(it);

#ifdef _DEBUG
			// check that there is no other copies
//			ObjectsMap::iterator it_test = m_lstLoadedObjects.find( pObject );
	//		assert(it_test == m_lstLoadedObjects.end());
			for (ObjectsMap::iterator it2 = m_lstLoadedObjects.begin(); it2 != m_lstLoadedObjects.end(); ++it2)
				assert((CStatObj*)(*it2) != pObject);
#endif

			delete pObject;
    }

		return true;
	}

	return false; // not found
}

bool CObjManager::GetSectorBBox(list2<CStatObjInst> * stat_objects, Vec3d &sec_bbmin, Vec3d &sec_bbmax)
{
  sec_bbmin=SetMaxBB();
  sec_bbmax=SetMinBB();

  for( int i=0; i<stat_objects->Count(); i++ )
  {
    CStatObjInst * o = &((*stat_objects)[i]);

    if(o->m_nObjectTypeID>=m_lstStaticTypes.Count())
      continue; // NOTE
    if(!m_lstStaticTypes[o->m_nObjectTypeID].pStatObj)
      continue; // NOTE

    Vec3 ws_boxmin = m_lstStaticTypes[o->m_nObjectTypeID].GetStatObj()->m_vBoxMin*o->m_fScale;
    Vec3 ws_boxmax = m_lstStaticTypes[o->m_nObjectTypeID].GetStatObj()->m_vBoxMax*o->m_fScale;

    ws_boxmin += o->m_vPos;
    ws_boxmax += o->m_vPos;

    sec_bbmin.CheckMin(ws_boxmin);
    sec_bbmax.CheckMax(ws_boxmax);
  }

  return stat_objects->Count()>0;
}

CObjManager::CObjManager(C3DEngine * p3DEngine):
	m_pDefaultCGF (NULL)
{
  m_p3DEngine = p3DEngine;
  m_pTerrain=0;
  m_fZoomFactor=1;

  m_REFarTreeSprites = (CREFarTreeSprites*)GetRenderer()->EF_CreateRE(eDATA_FarTreeSprites);
  m_fWindForce = 0.15f;
  m_vOutdoorAmbientColor.Set(0,0,0);
  m_vSunColor.Set(0,0,0);
  
  m_pCoverageBuffer = new CCoverageBuffer(GetRenderer());

	m_fMaxViewDistanceScale=1.f;

	if( GetRenderer()->GetFeatures() & RFT_OCCLUSIONTEST )
		m_pShaderOcclusionQuery = GetRenderer()->EF_LoadShader("OcclusionTest", eSH_World);
	else
		m_pShaderOcclusionQuery = 0;

  m_pREClearStencil = (CREClearStencil*)GetRenderer()->EF_CreateRE(eDATA_ClearStencil);
	m_pCWaterVolumes = 0;

  CStatObjInst::m_pObjManager = this;
	
	// prepare default object
	m_pDefaultCGF = MakeObject("Objects\\default.cgf");
	m_pDefaultCGF->m_bDefaultObject = true;
	m_bLockCGFResources = false;
}

CObjManager::~CObjManager()
{
	// free default object
	ReleaseObject(m_pDefaultCGF);
	m_pDefaultCGF=0;

	// free brushes
  assert(!m_lstBrushContainer.Count());
	for(int i=0; i<m_lstBrushContainer.Count(); i++)
	{
    if(m_lstBrushContainer[i]->GetEntityStatObj(0))
      ReleaseObject((CStatObj*)m_lstBrushContainer[i]->GetEntityStatObj(0));
		delete m_lstBrushContainer[i];
	}
	m_lstBrushContainer.Reset();

  UnloadObjects();
	
	assert(m_lstLoadedObjects.size() == 0);

  m_REFarTreeSprites->Release();

  delete m_pCoverageBuffer;
  m_pCoverageBuffer=0;

	delete m_pCWaterVolumes;
	m_pCWaterVolumes=0;

	m_pREClearStencil->Release();
}

// update vertex lighting for satatic objects like trees
void CObjManager::UpdateCustomLighting(const Vec3d & vLight)
{
  GetLog()->UpdateLoadingScreen("Updating lighting on vegetations ");

  for(int i=0; i<m_lstStaticTypes.Count(); i++)
  {
    if(m_lstStaticTypes[i].GetStatObj())
    {
      m_lstStaticTypes[i].GetStatObj()->UpdateCustomLightingSpritesAndShadowMaps(m_vOutdoorAmbientColor, 
      m_lstStaticTypes[i].nSpriteTexRes, m_lstStaticTypes[i].fBackSideLevel, m_lstStaticTypes[i].bCalcLighting);

//      if(!(i%4))
      GetLog()->UpdateLoadingScreenPlus(".");
    }
  }

  GetLog()->UpdateLoadingScreenPlus(" done");
}

// mostly xy size
float CObjManager::GetXYRadius(int type)
{
  if((m_lstStaticTypes.Count()<=type || !m_lstStaticTypes[type].pStatObj))
    return 0;

  Vec3d vSize = m_lstStaticTypes[type].pStatObj->GetBoxMax() - m_lstStaticTypes[type].pStatObj->GetBoxMin();
  vSize.z *= 0.5;

  float fRadius = m_lstStaticTypes[type].pStatObj->GetRadius();
  float fXYRadius = vSize.Length()*0.5f;

  return fXYRadius;
}

bool CObjManager::GetStaticObjectBBox(int nType, Vec3d & vBoxMin, Vec3d & vBoxMax)
{
  if((m_lstStaticTypes.Count()<=nType || !m_lstStaticTypes[nType].pStatObj))
    return 0;

  vBoxMin = m_lstStaticTypes[nType].pStatObj->GetBoxMin();
  vBoxMax = m_lstStaticTypes[nType].pStatObj->GetBoxMax();
  
  return true;
}

void CObjManager::AddPolygonToRenderer( const int nTexBindId, 
                                        IShader * pShader, 
                                        const int nDynLMask,
                                        Vec3d right,
                                        Vec3d up,
                                        const UCol & ucResCol,
                                        const ParticleBlendType eBlendType,
                                        const Vec3d & vAmbientColor,
                                        Vec3d vPos,
																				const SColorVert * pTailVerts,
																				const int nTailVertsNum,
																				const byte * pTailIndices,
																				const int nTailIndicesNum,
																				const float fSortId,
                                        const int dwCCObjFlags,
                                        IMatInfo * pCustomMaterial,
																				CStatObjInst * pStatObjInst,
																				list2<struct ShadowMapLightSourceInstance> * pShadowMapCasters)
{
	if(pStatObjInst && pStatObjInst->m_fFinalBending)
	{ // transfer decal into object space
		Matrix44 objMat;
		IStatObj * pEntObject = pStatObjInst->GetEntityStatObj(0, &objMat);
		assert(pEntObject);
		if(pEntObject)
		{
			objMat.Invert44();
			vPos = objMat.TransformPointOLD(vPos);
			right = objMat.TransformVectorOLD(right);
			up = objMat.TransformVectorOLD(up);
		}
	}

	// set positions and tex coords
  SColorVert arrVerts[4];
  arrVerts[0].vert = (-right-up) + vPos;
  arrVerts[0].dTC[0] = 1; 
  arrVerts[0].dTC[1] = 0;
  arrVerts[0].color = ucResCol;
  arrVerts[1].vert = ( right-up) + vPos;
  arrVerts[1].dTC[0] = 1; 
  arrVerts[1].dTC[1] = 1;
  arrVerts[1].color = ucResCol;
  arrVerts[2].vert = ( right+up) + vPos;
  arrVerts[2].dTC[0] = 0; 
  arrVerts[2].dTC[1] = 1;
  arrVerts[2].color = ucResCol;
  arrVerts[3].vert = (-right+up) + vPos;
  arrVerts[3].dTC[0] = 0; 
  arrVerts[3].dTC[1] = 0;
  arrVerts[3].color = ucResCol;

  if(nTexBindId <= 0 || nTexBindId >= 16384)
  {
    Warning( 0,0,"CObjManager::AddPolygonToRenderer: texture id is out of range: %d", nTexBindId);
    return;
  }

	// calculate render state
	uint nRenderState=0;
	switch(eBlendType)
	{
		case ParticleBlendType_AlphaBased:
			nRenderState = GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_ALPHATEST_GREATER0;
			break;
		case ParticleBlendType_ColorBased:
			nRenderState = GS_BLSRC_ONE | GS_BLDST_ONEMINUSSRCCOL;
			break;
		case ParticleBlendType_Additive:
			nRenderState = GS_BLSRC_ONE | GS_BLDST_ONE;
			break;
	}

	// repeated objects are free imedeately in renderer
	CCObject * pOb = GetIdentityCCObject();		

	if(pShadowMapCasters && pShadowMapCasters->Count())
	{
		pOb->m_pShadowCasters = pShadowMapCasters;
		pOb->m_ObjFlags |= FOB_INSHADOW;
	}

	if(pStatObjInst && pStatObjInst->m_fFinalBending)
	{
		pStatObjInst->GetEntityStatObj(0,&pOb->m_Matrix);
		pOb->m_ObjFlags	|= FOB_TRANS_MASK;
		CStatObj * pBody = m_lstStaticTypes[pStatObjInst->m_nObjectTypeID].GetStatObj();
		assert(pBody);
		if(pStatObjInst && pBody && pStatObjInst->m_fFinalBending)
			pBody->SetupBending(pOb,pStatObjInst->m_fFinalBending);
	}

	pOb->m_DynLMMask = nDynLMask;
	assert(nTexBindId>0);
	pOb->m_NumCM = nTexBindId;	    
	pOb->m_AmbColor = vAmbientColor;
	pOb->m_RenderState = nRenderState;
	if(GetRenderer()->EF_GetHeatVision())
		pOb->m_ObjFlags |= FOB_HEATVISION;

  pOb->m_ObjFlags |= dwCCObjFlags;
	
	pOb->m_SortId = fSortId; // use m_SortId for sorting correction
	
  pOb = GetRenderer()->EF_AddSpriteToScene(pShader->GetID(), 4, arrVerts, pOb);

	if(pTailVerts && nTailVertsNum && pTailIndices && nTailIndicesNum)
	  GetRenderer()->EF_AddSpriteToScene(pShader->GetID(), nTailVertsNum, 
		(SColorVert*)pTailVerts, pOb, (byte*)pTailIndices, nTailIndicesNum);
}

int CObjManager::GetMemoryUsage(class ICrySizer * pSizer)
{
	int nSize = 0;
	
	nSize += lstEntList_MLSMCIA.GetMemoryUsage();
//	nSize += lstStatInstList_MLSMCIA.GetMemoryUsage();
	nSize += m_lstDebugEntityList.GetMemoryUsage();
	nSize += m_lstStatEntitiesShadowMaps.GetMemoryUsage();
	nSize += m_lstFarObjects[0].GetMemoryUsage();
	nSize += m_lstFarObjects[1].GetMemoryUsage();
	
  {
	  for(int i=0; i<MAX_LIGHTS_NUM; i++)
		  nSize += m_lstLightEntities[i].GetMemoryUsage();
  //	for(int i=0; i<MAX_LIGHTS_NUM; i++)
	  //	nSize += m_lstShadowEntities[i].GetMemoryUsage();
  }

	nSize += m_lstLoadedObjects.size()*sizeof(CStatObj*);
	for (ObjectsMap::iterator it = m_lstLoadedObjects.begin(); it != m_lstLoadedObjects.end(); ++it)
	{
		nSize += ((CStatObj*)(*it))->GetMemoryUsage();
		nSize += sizeof(CStatObj);
	}
	nSize += m_lstStaticTypes.GetMemoryUsage();
//	nSize += m_lstStatShadowsBuffer.GetMemoryUsage();
//	nSize += m_lstTmpEntities_MESF.GetMemoryUsage();
	nSize += m_lstTmpSectors_MELFP.GetMemoryUsage();
//	nSize += m_lstTmpStatInstList_MESF.GetMemoryUsage();
/*
  {
    SIZER_COMPONENT_NAME(pSizer, "Brushes");
    int nSizeBrushes=0;
    nSizeBrushes += m_lstBrushContainer.GetMemoryUsage();
    nSizeBrushes += m_lstBrushContainer.Count()*sizeof(CBrush);
    pSizer->AddObject(&m_lstBrushContainer, nSizeBrushes);
    {
      SIZER_COMPONENT_NAME(pSizer, "BrushRS");
      for(int i=0; i<m_lstBrushContainer.Count(); i++)
      {
        if(m_lstBrushContainer[i]->GetEntityRS())
          pSizer->AddObject(m_lstBrushContainer[i]->GetEntityRS(), sizeof(*m_lstBrushContainer[i]->GetEntityRS()));
      }
    }
  }

  {
    SIZER_COMPONENT_NAME(pSizer, "Veget");
    int nSizeVeg=0;
    nSizeVeg += m_lstVegetContainer.GetMemoryUsage();
    nSizeVeg += m_lstVegetContainer.Count()*sizeof(CStatObjInst);
    pSizer->AddObject(&m_lstVegetContainer, nSizeVeg);

    {
      SIZER_COMPONENT_NAME(pSizer, "VegetRS");
      for(int i=0; i<m_lstVegetContainer.Count(); i++)
      {
        if(m_lstVegetContainer[i]->GetEntityRS())
          pSizer->AddObject(m_lstVegetContainer[i]->GetEntityRS(), sizeof(*m_lstVegetContainer[i]->GetEntityRS()));
      }
    }
  }
*/
	nSize += sizeof(CDLight);
	
	return nSize;
}

void CObjManager::ReregisterEntitiesInArea(Vec3d vBoxMin, Vec3d vBoxMax)
{
	list2<IEntityRender*> lstEntitiesInArea;

  if(m_pTerrain)
    m_pTerrain->MoveAllEntitiesIntoList(&lstEntitiesInArea, vBoxMin, vBoxMax);
  
  GetVisAreaManager()->MoveAllEntitiesIntoList(&lstEntitiesInArea, vBoxMin, vBoxMax);
  
	int nChanged=0;
	for(int i=0; i<lstEntitiesInArea.Count(); i++)
	{
		IVisArea * pPrevArea = lstEntitiesInArea[i]->GetEntityVisArea();
		bool bFound = Get3DEngine()->UnRegisterEntity(lstEntitiesInArea[i]);
 
//    assert(!bFound);

/*    {
      Get3DEngine()->Un RegisterInAllSectors(lstEntitiesInArea[i]);

      bFound = Get3DEngine()->UnRegisterEntity(lstEntitiesInArea[i]);

      if(lstEntitiesInArea[i]->IsStatic())
      {
        CBrush * pEnt = (CBrush *)lstEntitiesInArea[i];
        Matrix mat;
        CStatObj * pStatObj = (CStatObj*)lstEntitiesInArea[i]->GetEntityStatObj(0,&mat);
        assert(CBrush::IsMatrixValid(mat));
      }
    }
    */

		Get3DEngine()->RegisterEntity(lstEntitiesInArea[i]);
		if(pPrevArea != lstEntitiesInArea[i]->GetEntityVisArea())
			nChanged++;
	}

	GetLog()->Log("  CObjManager::ReregisterEntitiesInArea: %d of %d objects updated", nChanged, lstEntitiesInArea.Count());
}
/*
int CObjManager::CountPhysGeomUsage(CStatObj * pStatObjToFind)
{
  int nRes=0;
	for(int i=0; i<m_lstBrushContainer.Count(); i++)
	{
		CBrush * pBrush = m_lstBrushContainer[i];
		IStatObj * pStatObj = pBrush->GetEntityStatObj(0);
//    assert(((CStatObj*)pStatObj)->m_bStreamable);
		if(pStatObjToFind == pStatObj)
    {
      if(pBrush->GetPhysGeomId(0)>=0 || pBrush->GetPhysGeomId(1)>=0)
        nRes++;
    }
	}

  return nRes;
}*/

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

void CObjManager::FreeNotUsedCGFs()
{
	assert(!m_bLockCGFResources);

	if (!m_bLockCGFResources)
	{
		//Timur, You MUST use next here, or with erase you invalidating
		ObjectsMap::iterator next;
		for (ObjectsMap::iterator it = m_lstLoadedObjects.begin(); it != m_lstLoadedObjects.end(); it = next)
		{
			next = it; next++;
			CStatObj* p = (CStatObj*)(*it);
			if(p->m_nUsers<=0)
			{
				GetLog()->Log("Object unloaded: %s  %s",p->m_szFileName, p->m_szGeomName);
				m_lstLoadedObjects.erase(it);

				delete p;
			}
		}
	}
}