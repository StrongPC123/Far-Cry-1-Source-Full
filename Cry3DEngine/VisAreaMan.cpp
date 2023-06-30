////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjmandraw.cpp
//  Version:     v1.00
//  Created:     18/12/2002 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Visibility areas
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "objman.h"
#include "visareas.h"
#include "terrain_sector.h"
#include "3dengine.h"
#include "cbuffer.h"
#include "3dengine.h"
#include "IMarkers.h"

CVisAreaManager::CVisAreaManager()
{
	m_pCurPortal = m_pCurArea = 0;
  m_nLoadedSectors = 0;
}

CVisAreaManager::~CVisAreaManager()
{
	for(int i=0; i<m_lstVisAreas.Count(); i++)
		delete m_lstVisAreas[i];
	for(int i=0; i<m_lstPortals.Count(); i++)
		delete m_lstPortals[i];
	for(int i=0; i<m_lstOcclAreas.Count(); i++)
		delete m_lstOcclAreas[i];
}

bool CVisAreaManager::IsEntityVisible(IEntityRender * pEntityRS)
{
	if(!pEntityRS->GetEntityRS() || GetCVars()->e_portals==3)
		return true;

	if(!pEntityRS->GetEntityVisArea())
		return IsOutdoorAreasVisible();

	return true;
}

void CVisAreaManager::SetCurAreas(CObjManager * pObjManager)
{
	m_pCurArea = 0;
	m_pCurPortal = 0;

	if(!GetCVars()->e_portals)
		return;

	// find camera portal id
	for(int v=0; v<m_lstPortals.Count(); v++)
		if(m_lstPortals[v]->m_bActive &&  m_lstPortals[v]->IsPointInsideVisArea(GetViewCamera().GetOccPos()))
	{
		m_pCurPortal = m_lstPortals[v];
		break;
	}

	// if not inside any portal - try to find area
	if( !m_pCurPortal ) 
	{
		int nFoundAreasNum = 0;

		// find camera area
		for(int nVolumeId=0; nVolumeId<m_lstVisAreas.Count(); nVolumeId++)
		{
			if(m_lstVisAreas[nVolumeId]->IsPointInsideVisArea(GetViewCamera().GetOccPos()))
			{
				nFoundAreasNum++;
				m_pCurArea = m_lstVisAreas[nVolumeId];
			}
		}

		if(nFoundAreasNum>1) // if more than one area found - set cur area to undefined
		{ // todo: try to set joining portal as current
			m_pCurArea = 0;
		}
	}

	// camera is in outdoors 
	m_lstActiveEntransePortals.Clear();
	if(!m_pCurArea && !m_pCurPortal)
		MakeActiveEntransePortalsList(GetViewCamera(), m_lstActiveEntransePortals, 0, pObjManager);
	
/*
	if(m_pCurArea)
	{
		IVisArea * arrAreas[8];
		int nres = m_pCurArea->GetVisAreaConnections(arrAreas, 8);
		nres=nres;
	}
	DefineTrees();*/

	if(GetCVars()->e_portals == 4)
	{
		if(m_pCurPortal)
		{
			IVisArea * arrAreas[64];
			int nConnections = m_pCurPortal->GetVisAreaConnections(arrAreas,64);
			GetLog()->Log("CurPortal = %s, nConnections = %d", m_pCurPortal->m_sName, nConnections);
		}
		
		if(m_pCurArea)
		{
			IVisArea * arrAreas[64];
			int nConnections = m_pCurArea->GetVisAreaConnections(arrAreas,64);
			GetLog()->Log("CurArea = %s, nRes = %d", m_pCurArea->m_sName, nConnections);
		}
	}
}

bool CVisAreaManager::IsSkyVisible()
{
  return m_bSkyVisible;
}

bool CVisAreaManager::IsOutdoorAreasVisible()
{
	if(!m_pCurArea && !m_pCurPortal)
		return m_bOutdoorVisible=true; // camera not in the areas

	if(m_pCurPortal && m_pCurPortal->m_lstConnections.Count()==1)
		return m_bOutdoorVisible=true; // camera is in exit portal

	if(m_bOutdoorVisible)
		return true; // exit is visible

	// note: outdoor camera is no modified in this case
	return false;
}

void CVisAreaManager::LoadVisAreaBoxFromXML(XDOM::IXMLDOMDocumentPtr pDoc)
{
	{
		{ // reset only non shape volumes
			for(int i=0; i<m_lstVisAreas.Count(); i++)
			{
				if(m_lstVisAreas[i]->m_bLoadedAsAreaBox)
				{
					delete m_lstVisAreas[i];
					m_lstVisAreas.Delete(i);
					i--;
				}
			}

			for(int i=0; i<m_lstPortals.Count(); i++)
			{
				if(m_lstPortals[i]->m_bLoadedAsAreaBox)
				{
					delete m_lstPortals[i];
					m_lstPortals.Delete(i);
					i--;
				}
			}

      for(int i=0; i<m_lstOcclAreas.Count(); i++)
      {
        if(m_lstOcclAreas[i]->m_bLoadedAsAreaBox)
        {
          delete m_lstOcclAreas[i];
          m_lstOcclAreas.Delete(i);
          i--;
        }
      }
    }

		XDOM::IXMLDOMNodeListPtr pNodeTagList;
		XDOM::IXMLDOMNodePtr pNodeTag;
		pNodeTagList = pDoc->getElementsByTagName("Objects");
		if (pNodeTagList)
		{
			pNodeTagList->reset();
			pNodeTag = pNodeTagList->nextNode();
			XDOM::IXMLDOMNodeListPtr pNodeList;
			pNodeList = pNodeTag->getElementsByTagName("Object");
			if (pNodeList)
			{
				pNodeList->reset();
				XDOM::IXMLDOMNodePtr pNode;
				while (pNode = pNodeList->nextNode())
				{
					XDOM::IXMLDOMNodePtr pName = pNode->getAttribute("Type");
					if (pName)
					{
						if (strstr(pName->getText(),"AreaBox"))
						{
							XDOM::IXMLDOMNodePtr pAttr0,pAttr1,pAttr2,pAttr3,pAttr4,pAttr5;

							pAttr0 = pNode->getAttribute("Pos");
							pAttr1 = pNode->getAttribute("Width");
							pAttr2 = pNode->getAttribute("Length");
							pAttr3 = pNode->getAttribute("Height");
							pAttr4 = pNode->getAttribute("Name");

							if(pAttr0!=0 && pAttr1!=0 && pAttr2!=0 && pAttr3!=0 && pAttr4!=0)
							{
								CVisArea * pVolumeInfo = new CVisArea(true);

								Vec3d vPos = StringToVector(pAttr0->getText());

								Vec3d vSize((float)atof(pAttr1->getText()), (float)atof(pAttr2->getText()), (float)atof(pAttr3->getText()));
								pVolumeInfo->m_vBoxMax = vPos + Vec3d(vSize.x*0.5f, vSize.y*0.5f, vSize.z);
								pVolumeInfo->m_vBoxMin = vPos - Vec3d(vSize.x*0.5f, vSize.y*0.5f, 0);

								const Vec3d & min = pVolumeInfo->m_vBoxMin;
								const Vec3d & max = pVolumeInfo->m_vBoxMax;

								pVolumeInfo->m_lstShapePoints.Add(Vec3d(min.x,min.y,min.z));
								pVolumeInfo->m_lstShapePoints.Add(Vec3d(min.x,max.y,min.z));
								pVolumeInfo->m_lstShapePoints.Add(Vec3d(max.x,max.y,min.z));
								pVolumeInfo->m_lstShapePoints.Add(Vec3d(max.x,min.y,min.z));
								pVolumeInfo->m_fHeight = max.z-min.z;
								pVolumeInfo->m_vAmbColor = ((C3DEngine*)Get3DEngine())->GetObjManager()->m_vOutdoorAmbientColor;
								pVolumeInfo->m_vDynAmbColor = Vec3d(0,0,0);

								strncpy(pVolumeInfo->m_sName, pAttr4->getText(), sizeof(pVolumeInfo->m_sName));

								pVolumeInfo->UpdateGeometryBBox();

								// add volume to list
								strlwr(pVolumeInfo->m_sName);
								if(strstr(pVolumeInfo->m_sName, "portal"))
									m_lstPortals.Add(pVolumeInfo);
                else if(strstr(pVolumeInfo->m_sName, "visarea"))
                  m_lstVisAreas.Add(pVolumeInfo);
                else if(strstr(pVolumeInfo->m_sName, "occlarea"))
                  m_lstOcclAreas.Add(pVolumeInfo);
								else
									delete pVolumeInfo;
							}
						}
					}
				}
			}
		}
	}
	UpdateConnections();
}

void CVisAreaManager::SetAreaFogVolume(CTerrain * pTerrain, CVisArea * pVisArea)
{
	pVisArea->m_nFogVolumeId=0;
  if(pTerrain)
	for(int f=0; f<pTerrain->m_lstFogVolumes.Count(); f++)
	{
		const Vec3d & v1Min = pTerrain->m_lstFogVolumes[f].vBoxMin;
		const Vec3d & v1Max = pTerrain->m_lstFogVolumes[f].vBoxMax;
		const Vec3d & v2Min = pVisArea->m_vBoxMin;
		const Vec3d & v2Max = pVisArea->m_vBoxMax;

		if(v1Max.x>v2Min.x && v2Max.x>v1Min.x)
		if(v1Max.y>v2Min.y && v2Max.y>v1Min.y)
		if(v1Max.z>v2Min.z && v2Max.z>v1Min.z)
		if(!pTerrain->m_lstFogVolumes[f].bOcean)
		{
      Vec3d arrVerts3d[8] = 
      {
        Vec3d(v1Min.x,v1Min.y,v1Min.z),
        Vec3d(v1Min.x,v1Max.y,v1Min.z),
        Vec3d(v1Max.x,v1Min.y,v1Min.z),
        Vec3d(v1Max.x,v1Max.y,v1Min.z),
        Vec3d(v1Min.x,v1Min.y,v1Max.z),
        Vec3d(v1Min.x,v1Max.y,v1Max.z),
        Vec3d(v1Max.x,v1Min.y,v1Max.z),
        Vec3d(v1Max.x,v1Max.y,v1Max.z)
      };

      bool bIntersect = false;
      for(int i=0; i<8; i++)
        if(pVisArea->IsPointInsideVisArea(arrVerts3d[i]))
        {
          bIntersect = true;
          break;
        }

      if(!bIntersect)
        if(pVisArea->IsPointInsideVisArea((v1Min+v1Max)*0.5f))
          bIntersect = true;

      if(!bIntersect)
      {
        for(int i=0; i<pVisArea->m_lstShapePoints.Count(); i++)
          if(pTerrain->m_lstFogVolumes[f].InsideBBox(pVisArea->m_lstShapePoints[i]))
          {
            bIntersect = true;
            break;
          }
      }

			if(!bIntersect)
			{
				Vec3d vCenter = (pVisArea->m_vBoxMin+pVisArea->m_vBoxMax)*0.5f;
				if(pTerrain->m_lstFogVolumes[f].InsideBBox(vCenter))
					bIntersect = true;
			}

      if(bIntersect)
      {
			  int nFogId = pVisArea->m_nFogVolumeId = pTerrain->m_lstFogVolumes[f].nRendererVolumeID;
			  pTerrain->m_lstFogVolumes[f].bIndoorOnly = true;
			  pTerrain->UnregisterFogVolumeFromOutdoor(&pTerrain->m_lstFogVolumes[f]);
			  break;
      }
		}
	}
}

void CVisAreaManager::SetupFogVolumes(CTerrain * pTerrain)
{
  if(pTerrain)
	  for(int f=0; f<pTerrain->m_lstFogVolumes.Count(); f++)
		  pTerrain->m_lstFogVolumes[f].bIndoorOnly = false;

	// set fog volumes
	for(int p=0; p<m_lstPortals.Count(); p++)
		SetAreaFogVolume(pTerrain, m_lstPortals[p]);

	for(int v=0; v<m_lstVisAreas.Count(); v++)
		SetAreaFogVolume(pTerrain, m_lstVisAreas[v]);
}

void CVisAreaManager::PortalsDrawDebug()
{
	UpdateConnections();
/*
	if(m_pCurArea)
	{
		for(int p=0; p<m_pCurArea->m_lstConnections.Count(); p++)
		{
			CVisArea * pPortal = m_pCurArea->m_lstConnections[p];
			float fError = pPortal->IsPortalValid() ? 1.f : int(GetTimer()->GetCurrTime()*8)&1;
			GetRenderer()->SetMaterialColor(fError,fError*(pPortal->m_lstConnections.Count()<2),0,0.25f);
			GetRenderer()->Draw3dBBox(pPortal->m_vBoxMin, pPortal->m_vBoxMax, DPRIM_SOLID_BOX);
			GetRenderer()->DrawLabel((pPortal->m_vBoxMin+ pPortal->m_vBoxMax)*0.5f,
				2,pPortal->m_sName);
		}
	}
	else*/
	{
		// debug draw areas
		GetRenderer()->SetMaterialColor(0,1,0,0.25f);
		for(int v=0; v<m_lstVisAreas.Count(); v++)
		{
			GetRenderer()->Draw3dBBox(m_lstVisAreas[v]->m_vBoxMin, m_lstVisAreas[v]->m_vBoxMax, DPRIM_SOLID_BOX);
			GetRenderer()->DrawLabelEx((m_lstVisAreas[v]->m_vBoxMin+ m_lstVisAreas[v]->m_vBoxMax)*0.5f,
        1,(float*)&Vec3d(1,1,1),0,1,m_lstVisAreas[v]->m_sName);

			GetRenderer()->SetMaterialColor(0,1,0,0.25f);
			GetRenderer()->Draw3dBBox(m_lstVisAreas[v]->m_vGeomBoxMin, m_lstVisAreas[v]->m_vGeomBoxMax);
		}

		// debug draw portals
		for(int v=0; v<m_lstPortals.Count(); v++)
		{
			float fError = m_lstPortals[v]->IsPortalValid() ? 1.f : int(GetTimer()->GetCurrTime()*8)&1;
			GetRenderer()->SetMaterialColor(fError,fError*(m_lstPortals[v]->m_lstConnections.Count()<2),0,0.25f);
			GetRenderer()->Draw3dBBox(m_lstPortals[v]->m_vBoxMin, m_lstPortals[v]->m_vBoxMax, DPRIM_SOLID_BOX);

			GetRenderer()->DrawLabelEx((m_lstPortals[v]->m_vBoxMin+ m_lstPortals[v]->m_vBoxMax)*0.5f,
				1,(float*)&Vec3d(1,1,1),0,1,m_lstPortals[v]->m_sName);

			CVisArea * pPortal = m_lstPortals[v];
			Vec3d vCenter = (pPortal->m_vBoxMin+pPortal->m_vBoxMax)*0.5f;
			GetRenderer()->Draw3dBBox(vCenter-Vec3d(0.1f,0.1f,0.1f), vCenter+Vec3d(0.1f,0.1f,0.1f));
			for(int i=0; i<pPortal->m_lstConnections.Count() && i<2; i++)
				GetRenderer()->Draw3dBBox(vCenter, vCenter+pPortal->m_vConnNormals[i], DPRIM_LINE);

			GetRenderer()->SetMaterialColor(0,0,1,0.25f);
			GetRenderer()->Draw3dBBox(pPortal->m_vGeomBoxMin, pPortal->m_vGeomBoxMax);
		}

/*
		// debug draw area shape
		GetRenderer()->SetMaterialColor(0,0,1,0.25f);
		for(int v=0; v<m_lstVisAreas.Count(); v++)
		for(int p=0; p<m_lstVisAreas[v]->m_lstShapePoints.Count(); p++)
			GetRenderer()->DrawLabel(m_lstVisAreas[v]->m_lstShapePoints[p], 2,"%d", p);
		for(int v=0; v<m_lstPortals.Count(); v++)
		for(int p=0; p<m_lstPortals[v]->m_lstShapePoints.Count(); p++)
			GetRenderer()->DrawLabel(m_lstPortals[v]->m_lstShapePoints[p], 2,"%d", p);*/
	}
}

bool CVisAreaManager::SetEntityArea(IEntityRender* pEntityRS)
{
	assert(pEntityRS);

	CVisArea ** pVisArea = &pEntityRS->m_pVisArea;

	Vec3d vEntBMin, vEntBMax;
	pEntityRS->GetBBox(vEntBMin, vEntBMax);
	Vec3d vEntCenter = (vEntBMin+vEntBMax)*0.5f;

	if(pEntityRS->m_dwRndFlags&ERF_DONOTCHECKVIS)
	{ // if CS_FLAG_DRAW_NEAR is set (fps weapon) - use camera position because fps weapon bbox is not correct
		// temporary solution until weapon bbox will be fixed
		ICryCharInstance * pChar;
		if ((pChar = pEntityRS->GetEntityCharacter(1)) && (pChar->GetFlags() & CS_FLAG_DRAW_MODEL) && (pChar->GetFlags() & CS_FLAG_DRAW_NEAR))
			vEntCenter = GetViewCamera().GetPos();
		else if ((pChar = pEntityRS->GetEntityCharacter(0)) && (pChar->GetFlags() & CS_FLAG_DRAW_MODEL) && (pChar->GetFlags() & CS_FLAG_DRAW_NEAR))
			vEntCenter = GetViewCamera().GetPos();
	}

	if (*pVisArea)
	{ // detect if we are still in the same area
		if ((*pVisArea)->IsPointInsideVisArea(vEntCenter))
			return true;
		UnRegisterEntity(pEntityRS);
	}

	*pVisArea = 0;

  int nStatic = (pEntityRS->GetEntityRenderType() != eERType_Unknown);

	// find portal containing object center
	for(int v=0; v<m_lstPortals.Count(); v++)
	{
		if(m_lstPortals[v]->IsPointInsideVisArea(vEntCenter))
		{
			*pVisArea  = m_lstPortals[v];
			if(m_lstPortals[v]->m_lstEntities[nStatic].Find(pEntityRS)<0)
				m_lstPortals[v]->m_lstEntities[nStatic].Add(pEntityRS);	
			break;
		}
	}

	if(!(*pVisArea)) // if portal not found - find volume
	for(int v=0; v<m_lstVisAreas.Count(); v++)
	{
		if(m_lstVisAreas[v]->IsPointInsideVisArea(vEntCenter))
		{
			*pVisArea = m_lstVisAreas[v];			
			if(m_lstVisAreas[v]->m_lstEntities[nStatic].Find(pEntityRS)<0)
				m_lstVisAreas[v]->m_lstEntities[nStatic].Add(pEntityRS);
			break;
		}
	}

	/*if(0&&!(*pVisArea))
	{ // if still not found - reduce requirements and detect box intersection
		for(int v=0; v<m_lstPortals.Count(); v++)
		{
			const Vec3d & v2Min = m_lstPortals[v]->m_vBoxMin;
			const Vec3d & v2Max = m_lstPortals[v]->m_vBoxMax;

			if(vEntBMax.x>v2Min.x && v2Max.x>vEntBMin.x)
			if(vEntBMax.y>v2Min.y && v2Max.y>vEntBMin.y)
			if(vEntBMax.z>v2Min.z && v2Max.z>vEntBMin.z)
			{
				*pVisArea = m_lstPortals[v];
				if(m_lstPortals[v]->m_lstEntities[nStatic].Find(pEntityRS)<0)
					m_lstPortals[v]->m_lstEntities[nStatic].Add(pEntityRS);	
				break;
			}
		}
	}*/

	if(nStatic && *pVisArea) // update bbox of exit portal //			if((*pVisArea)->m_lstConnections.Count()==1)
		if((*pVisArea)->IsPortal())
			(*pVisArea)->UpdateGeometryBBox();

	return (*pVisArea) != 0;
}

bool CVisAreaManager::UnRegisterEntity(IEntityRender* pEntityRS)
{			
	assert(pEntityRS);

  bool bFound = false;

  int nStatic = (pEntityRS->GetEntityRenderType() != eERType_Unknown);

	// unregister in volumes
	if(pEntityRS->m_pVisArea)
		bFound |= pEntityRS->m_pVisArea->m_lstEntities[nStatic].Delete(pEntityRS);
	pEntityRS->m_pVisArea = 0;

/*
#ifdef _DEBUG
	{
		// find lost registrations
		for(int i=0; i<m_lstVisAreas.Count(); i++)
		{
			if(m_lstVisAreas[i]->m_lstEntities[nStatic].Find(pEntityRS)>=0)
			{
				m_lstVisAreas[i]->m_lstEntities[nStatic].Delete(pEntityRS);
				assert(0);
			}
		}
      
		for(int i=0; i<m_lstPortals.Count(); i++)
		{
			if(m_lstPortals[i]->m_lstEntities[nStatic].Find(pEntityRS)>=0)
			{
				m_lstPortals[i]->m_lstEntities[nStatic].Delete(pEntityRS);
				assert(0);
			}
		}   
	}
#endif
  */

  return bFound;
}

void CVisAreaManager::Render(CObjManager * pObjManager)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	if(!pObjManager->m_nRenderStackLevel)
  {
    m_bOutdoorVisible = false;
    m_bSkyVisible = false;
  }

	m_lstOutdoorPortalCameras.Clear();

	SetCurAreas(pObjManager);

  CCamera camRoot = GetViewCamera();
  camRoot.m_ScissorInfo.x1 = 0;
  camRoot.m_ScissorInfo.y1 = 0;
  camRoot.m_ScissorInfo.x2 = GetRenderer()->GetWidth(); // todo: use values from camera
  camRoot.m_ScissorInfo.y2 = GetRenderer()->GetHeight();

	if(GetCVars()->e_portals==3)
	{	// draw everything for debug
		for(int i=0; i<m_lstVisAreas.Count(); i++)
			if(camRoot.IsAABBVisibleFast( AABB(m_lstVisAreas[i]->m_vBoxMin,m_lstVisAreas[i]->m_vBoxMax) ))
				m_lstVisAreas[i]->DrawVolume(pObjManager, 0, camRoot, 0, m_pCurPortal, &m_bOutdoorVisible, &m_lstOutdoorPortalCameras, &m_bSkyVisible);

		for(int i=0; i<m_lstPortals.Count(); i++)
			if(camRoot.IsAABBVisibleFast( AABB(m_lstPortals[i]->m_vBoxMin,m_lstPortals[i]->m_vBoxMax) ))
				m_lstPortals[i]->DrawVolume(pObjManager, 0, camRoot, 0, m_pCurPortal, &m_bOutdoorVisible, &m_lstOutdoorPortalCameras, &m_bSkyVisible);
	}
	else
	{
		if(pObjManager->m_nRenderStackLevel)
		{ // use another starting point for reflections
			CVisArea * pVisArea = (CVisArea *)GetVisAreaFromPos(camRoot.GetOccPos());
			if(pVisArea)
				pVisArea->DrawVolume(pObjManager, 3, camRoot, 0, m_pCurPortal, &m_bOutdoorVisible, &m_lstOutdoorPortalCameras, &m_bSkyVisible);
		}
		else if(m_pCurArea) 
		{ // camera inside some sector
			m_pCurArea->DrawVolume(pObjManager, 6, camRoot, 0, m_pCurPortal, &m_bOutdoorVisible, &m_lstOutdoorPortalCameras, &m_bSkyVisible);
			for(int i=0; i<m_lstOutdoorPortalCameras.Count(); i++) // process all exit portals
			{ // for each portal build list of potentially visible entrances into other areas
				MakeActiveEntransePortalsList(m_lstOutdoorPortalCameras[i], m_lstActiveEntransePortals, (CVisArea*)m_lstOutdoorPortalCameras[i].m_pPortal, pObjManager);
				for(int i=0; i<m_lstActiveEntransePortals.Count(); i++) // entrance into another building is visible
					m_lstActiveEntransePortals[i]->DrawVolume(pObjManager, i==0 ? 5 : 1, 
					m_lstOutdoorPortalCameras.Count() ? m_lstOutdoorPortalCameras[0] : camRoot, 0, m_pCurPortal, 0, 0, 0);
			}

      // make one final camera
/*      while(m_lstOutdoorPortalCameras.Count()>1)
      {
        MergeCameras(m_lstOutdoorPortalCameras[0], m_lstOutdoorPortalCameras[1]);
        m_lstOutdoorPortalCameras.Delete(1);
      }*/

      if(m_lstOutdoorPortalCameras.Count()>1)
				m_lstOutdoorPortalCameras.Clear(); // use default camera if more than two exits found

			// reset scissor if skybox is visible also thru skyboxonly portal
			if(m_bSkyVisible && m_lstOutdoorPortalCameras.Count()==1)
				m_lstOutdoorPortalCameras[0].m_ScissorInfo.x1 = 
				m_lstOutdoorPortalCameras[0].m_ScissorInfo.x2 = 
				m_lstOutdoorPortalCameras[0].m_ScissorInfo.y1 = 
				m_lstOutdoorPortalCameras[0].m_ScissorInfo.y2 = 0;
		}
		else if(m_pCurPortal) 
		{	// camera inside some portal
			m_pCurPortal->DrawVolume(pObjManager, 5, camRoot, 0, m_pCurPortal, &m_bOutdoorVisible, &m_lstOutdoorPortalCameras, &m_bSkyVisible);

			if(m_pCurPortal->m_lstConnections.Count()==1 || m_lstOutdoorPortalCameras.Count())
			{ // if camera is in exit portal or exit is visible
				MakeActiveEntransePortalsList(m_lstOutdoorPortalCameras.Count() ? m_lstOutdoorPortalCameras[0] : camRoot, 
					m_lstActiveEntransePortals, 
					m_lstOutdoorPortalCameras.Count() ? (CVisArea*)m_lstOutdoorPortalCameras[0].m_pPortal : m_pCurPortal, pObjManager);
				for(int i=0; i<m_lstActiveEntransePortals.Count(); i++) // entrance into another building is visible
					m_lstActiveEntransePortals[i]->DrawVolume(pObjManager, i==0 ? 5 : 1, 
					m_lstOutdoorPortalCameras.Count() ? m_lstOutdoorPortalCameras[0] : camRoot, 0, m_pCurPortal, 0, 0, 0);
				m_lstOutdoorPortalCameras.Clear();
			}
		}
		else if(m_lstActiveEntransePortals.Count())
		{ // camera in outdoors - process visible entrance portals
			for(int i=0; i<m_lstActiveEntransePortals.Count(); i++)
				m_lstActiveEntransePortals[i]->DrawVolume(pObjManager, 5, camRoot, 0, m_lstActiveEntransePortals[i], &m_bOutdoorVisible, &m_lstOutdoorPortalCameras, &m_bSkyVisible);
			m_lstActiveEntransePortals.Clear();

			// do not recurse to another building since we already processed all potential entrances
			m_lstOutdoorPortalCameras.Clear(); // use default camera
			m_bOutdoorVisible=true;
		}
	}

	if(GetCVars()->e_portals==2)
		PortalsDrawDebug();
}

void CVisAreaManager::ActivatePortal(const Vec3d &vPos, bool bActivate, IEntityRender *pEntity)
{
	for(int v=0; v<m_lstPortals.Count(); v++)
	if(Overlap::Point_AABB(vPos, m_lstPortals[v]->m_vBoxMin-Vec3d(1,1,1), m_lstPortals[v]->m_vBoxMax+Vec3d(1,1,1)))
		m_lstPortals[v]->m_bActive = bActivate;
}
/*
bool CVisAreaManager::IsEntityInVisibleArea(IEntityRenderState * pRS)
{
	if( pRS && pRS->plstVisAreaId && pRS->plstVisAreaId->Count() )
	{
		list2<int> * pVisAreas = pRS->plstVisAreaId;
		for(int n=0; n<pVisAreas->Count(); n++)
			if( m_lstVisAreas[pVisAreas->GetAt(n)].m_nVisFrameId==GetFrameID() )
				break;

		if(n==pVisAreas->Count())
			return false; // no visible areas
	}
	else
		return false; // entity is not inside

	return true;
}	*/

bool CVisAreaManager::IsValidVisAreaPointer(CVisArea * pVisArea)
{
  if( m_lstVisAreas.Find(pVisArea)<0 &&
      m_lstPortals.Find(pVisArea)<0 && 
      m_lstOcclAreas.Find(pVisArea)<0 )
    return false;

  return true;
}

bool CVisAreaManager::DeleteVisArea(CVisArea * pVisArea)
{
  bool bFound = false;
	if(m_lstVisAreas.Delete(pVisArea) || m_lstPortals.Delete(pVisArea) || m_lstOcclAreas.Delete(pVisArea))
  {
    delete pVisArea;
    bFound = true;
  }
  m_pCurArea = 0;
  m_pCurPortal = 0;
  UpdateConnections();
  return bFound;
}

void CVisAreaManager::LoadVisAreaShapeFromXML(XDOM::IXMLDOMDocumentPtr pDoc)
{
	{ // reset only shape volumes
		for(int i=0; i<m_lstVisAreas.Count(); i++)
		{
			if(!m_lstVisAreas[i]->m_bLoadedAsAreaBox)
			{
				delete m_lstVisAreas[i];
				m_lstVisAreas.Delete(i);
				i--;
			}
		}

		for(int i=0; i<m_lstPortals.Count(); i++)
		{
			if(!m_lstPortals[i]->m_bLoadedAsAreaBox)
			{
				delete m_lstPortals[i];
				m_lstPortals.Delete(i);
				i--;
			}
		}
	}


	// fill list of volumes of shape points
	XDOM::IXMLDOMNodeListPtr pNodeTagList;
	XDOM::IXMLDOMNodePtr pNodeTag;
	pNodeTagList = pDoc->getElementsByTagName("Objects");
	if (pNodeTagList)
	{
		pNodeTagList->reset();
		pNodeTag = pNodeTagList->nextNode();
		XDOM::IXMLDOMNodeListPtr pNodeList;
		pNodeList = pNodeTag->getElementsByTagName("Object");
		if (pNodeList)
		{
			pNodeList->reset();
			XDOM::IXMLDOMNodePtr pNode;
			while (pNode = pNodeList->nextNode())
			{
				XDOM::IXMLDOMNodePtr pName = pNode->getAttribute("Type");
				if (pName)
				{
          const char * pType = pName->getText();
					if (strstr(pType,"OccluderArea") || strstr(pType,"VisArea") || strstr(pType,"Portal"))
					{
						CVisArea * pArea = new CVisArea(false);

						pArea->m_vBoxMax=SetMinBB();
						pArea->m_vBoxMin=SetMaxBB();

						// set shader
						XDOM::IXMLDOMNodePtr pAttr5 = pNode->getAttribute("Name");
						if(pAttr5)
							strcpy(pArea->m_sName, pAttr5->getText());

						// set height
						XDOM::IXMLDOMNodePtr pAttr6 = pNode->getAttribute("Height");
						if(pAttr6)
							pArea->m_fHeight = (float)atof(pAttr6->getText());

						// set ambient color
						XDOM::IXMLDOMNodePtr pAttr7 = pNode->getAttribute("AmbientColor");
						if(pAttr7)
							pArea->m_vAmbColor = StringToVector(pAttr7->getText());

						// set dyn ambient color
						XDOM::IXMLDOMNodePtr pDynAmbClr = pNode->getAttribute("DynAmbientColor");
						if(pDynAmbClr)
							pArea->m_vDynAmbColor = StringToVector(pDynAmbClr->getText());

            // set SkyOnly flag
            XDOM::IXMLDOMNodePtr pAttr8 = pNode->getAttribute("SkyOnly");
            if(pAttr8)
              pArea->m_bSkyOnly = atol(pAttr8->getText())!=0;

            // set AfectedByOutLights flag
            XDOM::IXMLDOMNodePtr pAttr9 = pNode->getAttribute("AffectedBySun");
            if(pAttr9)
              pArea->m_bAfectedByOutLights = atol(pAttr9->getText())!=0;

						// set ViewDistRatio
						XDOM::IXMLDOMNodePtr pAttr10 = pNode->getAttribute("ViewDistRatio");
						if(pAttr10)
							pArea->m_fViewDistRatio = (float)atof(pAttr10->getText());

						// set DoubleSide flag
						XDOM::IXMLDOMNodePtr pAttr11 = pNode->getAttribute("DoubleSide");
						if(pAttr11)
							pArea->m_bDoubleSide = atol(pAttr11->getText())!=0;

						// set UseDeepness flag
//						XDOM::IXMLDOMNodePtr pAttr12 = pNode->getAttribute("UseDeepness");
	//					if(pAttr12)
		//					pArea->m_bUseDeepness = atol(pAttr12->getText())!=0;

						// set UseInIndoors flag
						XDOM::IXMLDOMNodePtr pAttr13 = pNode->getAttribute("UseInIndoors");
						if(pAttr13)
							pArea->m_bUseInIndoors = atol(pAttr13->getText())!=0;

						// load vertices
						if(pAttr5!=0 && pAttr6!=0)
						{
							strlwr(pArea->m_sName);

							if(strstr(pType, "OccluderArea"))
								m_lstOcclAreas.Add(pArea);
							else if(strstr(pArea->m_sName,"portal") || strstr(pType,"Portal"))
								m_lstPortals.Add(pArea);
							else
								m_lstVisAreas.Add(pArea);

							XDOM::IXMLDOMNodeListPtr pNodeTagList;
							XDOM::IXMLDOMNodePtr pNodeTag;
							pNodeTagList = pNode->getElementsByTagName("Points");
							if (pNodeTagList)
							{
								pNodeTagList->reset();
								pNodeTag = pNodeTagList->nextNode();
								XDOM::IXMLDOMNodeListPtr pNodeList;
								pNodeList = pNodeTag->getElementsByTagName("Point");
								if (pNodeList)
								{
									pNodeList->reset();
									XDOM::IXMLDOMNodePtr pNode;
									while (pNode = pNodeList->nextNode())
									{
										XDOM::IXMLDOMNodePtr pPos = pNode->getAttribute("Pos");
										if (pPos)
										{
											Vec3d vPos = StringToVector(pPos->getText());
											pArea->m_lstShapePoints.Add(vPos);
											pArea->m_vBoxMax.CheckMax(vPos);
											pArea->m_vBoxMin.CheckMin(vPos);
											pArea->m_vBoxMax.CheckMax(vPos+Vec3d(0,0,pArea->m_fHeight));
											pArea->m_vBoxMin.CheckMin(vPos+Vec3d(0,0,pArea->m_fHeight));
										}
									}
								}
								pArea->UpdateGeometryBBox();
							}
						}
					}
				}
			}
		}
	}

	// load area boxes to support old way
//	LoadVisAreaBoxFromXML(pDoc);
}

void CVisAreaManager::UpdateVisArea(CVisArea * pArea, const Vec3d * pPoints, int nCount, const char * szName, float fHeight, const Vec3d & vAmbientColor, bool bAfectedByOutLights, bool bSkyOnly, CTerrain*pTerrain, const Vec3 & vDynAmbientColor, float fViewDistRatio, bool bDoubleSide, bool bUseDeepness, bool bUseInIndoors)
{ // on first update there will be nothing to delete, area will be added into list only in this function
	m_lstPortals.Delete(pArea);
	m_lstVisAreas.Delete(pArea);
	m_lstOcclAreas.Delete(pArea);

	pArea->Update(pPoints, nCount, szName, fHeight, vAmbientColor, bAfectedByOutLights, bSkyOnly, vDynAmbientColor, fViewDistRatio, bDoubleSide, bUseDeepness, bUseInIndoors);

	strlwr(pArea->m_sName);
	if(strstr(pArea->m_sName,"portal"))
	{
		if(pArea->m_lstConnections.Count()==1)
			pArea->UpdateGeometryBBox();

		m_lstPortals.Add(pArea);
	}
	else if(strstr(pArea->m_sName,"visarea"))
		m_lstVisAreas.Add(pArea);
  else if(strstr(pArea->m_sName, "occlarea"))
    m_lstOcclAreas.Add(pArea);

	UpdateConnections();

	SetAreaFogVolume(pTerrain, pArea);
}

void CVisAreaManager::UpdateConnections()
{
	// Reset connectivity
	for(int p=0; p<m_lstPortals.Count(); p++)
		m_lstPortals[p]->m_lstConnections.Clear();

	for(int v=0; v<m_lstVisAreas.Count(); v++)
		m_lstVisAreas[v]->m_lstConnections.Clear();

	// Init connectivity - check intersection of all areas and portals
	for(int p=0; p<m_lstPortals.Count(); p++)
	{
//    if(strstr(m_lstPortals[p]->m_sName,"l5"))
  //    int y=0;

		for(int v=0; v<m_lstVisAreas.Count(); v++)
		{
			if(m_lstVisAreas[v]->IsPortalIntersectAreaInValidWay(m_lstPortals[p]))
			{ // if bboxes intersect
				m_lstVisAreas[v]->m_lstConnections.Add(m_lstPortals[p]);
				m_lstPortals[p]->m_lstConnections.Add(m_lstVisAreas[v]);

				// set portal direction
				Vec3d vNormal = m_lstVisAreas[v]->GetConnectionNormal(m_lstPortals[p]);
				if(m_lstPortals[p]->m_lstConnections.Count()<=2)
					m_lstPortals[p]->m_vConnNormals[m_lstPortals[p]->m_lstConnections.Count()-1] = vNormal;
			}
		}
	}
}

void CVisAreaManager::MoveAllEntitiesIntoList(list2<IEntityRender*> * plstVisAreasEntities, 
                                              const Vec3d & vBoxMin, const Vec3d & vBoxMax)
{
	for(int p=0; p<m_lstPortals.Count(); p++)
	{
    if( m_lstPortals[p]->m_vBoxMin.x < vBoxMax.x && m_lstPortals[p]->m_vBoxMax.x > vBoxMin.x &&
        m_lstPortals[p]->m_vBoxMin.y < vBoxMax.y && m_lstPortals[p]->m_vBoxMax.y > vBoxMin.y )
    {
      plstVisAreasEntities->AddList(m_lstPortals[p]->m_lstEntities[DYNAMIC_ENTITIES]);
      plstVisAreasEntities->AddList(m_lstPortals[p]->m_lstEntities[STATIC_ENTITIES]);
      m_lstPortals[p]->m_lstEntities[DYNAMIC_ENTITIES].Clear();
      m_lstPortals[p]->m_lstEntities[STATIC_ENTITIES].Clear();
    }
	}

	for(int v=0; v<m_lstVisAreas.Count(); v++)
	{
    if( m_lstVisAreas[v]->m_vBoxMin.x < vBoxMax.x && m_lstVisAreas[v]->m_vBoxMax.x > vBoxMin.x &&
        m_lstVisAreas[v]->m_vBoxMin.y < vBoxMax.y && m_lstVisAreas[v]->m_vBoxMax.y > vBoxMin.y )
    {
      plstVisAreasEntities->AddList(m_lstVisAreas[v]->m_lstEntities[DYNAMIC_ENTITIES]);
      plstVisAreasEntities->AddList(m_lstVisAreas[v]->m_lstEntities[STATIC_ENTITIES]);
      m_lstVisAreas[v]->m_lstEntities[DYNAMIC_ENTITIES].Clear();
      m_lstVisAreas[v]->m_lstEntities[STATIC_ENTITIES].Clear();
    }
	}
}

IVisArea * CVisAreaManager::GetVisAreaFromPos(const Vec3d &vPos)
{
	// check areas
	for(int v=0; v<m_lstVisAreas.Count(); v++)
		if(m_lstVisAreas[v]->IsPointInsideVisArea(vPos))
			return m_lstVisAreas[v];

	// check portals
	for(int v=0; v<m_lstPortals.Count(); v++)
		if(m_lstPortals[v]->IsPointInsideVisArea(vPos))
			return m_lstPortals[v];

	return 0;
}

CVisArea * CVisAreaManager::CreateVisArea()
{
	CVisArea * p = new CVisArea(false);
	return p;
}
/*
void CVisAreaManager::DefineTrees()
{
	int nTreeId=0;
	for(int v=0; v<m_lstVisAreas.Count(); v++)
	{
		if(m_lstVisAreas[v]->m_nTreeId != nTreeId)
		{
			m_lstVisAreas[v]->SetTreeId(nTreeId);
			nTreeId++;
		}
	}
}
*/

bool CVisAreaManager::IsEntityVisAreaVisible(IEntityRender * pEnt, bool nCheckNeighbors)
{	
	if(pEnt->GetEntityVisArea())
	{
		if(pEnt->m_pVisArea)//->IsPortal())
		{ // check is lsource area was rendered in prev frame
			CVisArea * pVisArea = pEnt->m_pVisArea;
			int nRndFrameId = GetFrameID();
			if(abs(pVisArea->m_nRndFrameId - nRndFrameId)>2)
			{
				if(!nCheckNeighbors)
					return false; // this area is not visible

				// try neibhour areas
				bool bFound = false;
				if(pEnt->m_pVisArea->IsPortal())
				{
					CVisArea * pPort = pEnt->m_pVisArea;
					for(int n=0; n<pPort->m_lstConnections.Count(); n++)
					{ // loop other sectors
						CVisArea * pNeibArea = (CVisArea*)pPort->m_lstConnections[n];
						if(abs(pNeibArea->m_nRndFrameId - GetFrameID())<=2)
						{
							bFound=true;
							break;
						}
					}
				}
				else
				{
					for(int t=0; !bFound && t<pVisArea->m_lstConnections.Count(); t++)
					{ // loop portals
						CVisArea * pPort = (CVisArea*)pVisArea->m_lstConnections[t];
						if(abs(pPort->m_nRndFrameId - GetFrameID())<=2)
						{
							bFound=true;
							break;
						}

						for(int n=0; n<pPort->m_lstConnections.Count(); n++)
						{ // loop other sectors
							CVisArea * pNeibArea = (CVisArea*)pPort->m_lstConnections[n];
							if(abs(pNeibArea->m_nRndFrameId - GetFrameID())<=2)
							{
								bFound=true;
								break;
							}
						}
					}
				}
				
				if(!bFound)
					return false;

				return true;
			}
		}
		else
			return false; // not visible
	}
	else if(!IsOutdoorAreasVisible())
		return false;

	return true;
}	

int __cdecl CVisAreaManager__CmpDistToPortal(const void* v1, const void* v2)
{
	CVisArea * p1 = *((CVisArea **)v1);
  CVisArea * p2 = *((CVisArea **)v2);

  if(!p1 || !p2)
    return 0;

  if(p1->m_fDistance > p2->m_fDistance)
    return 1;
  else if(p1->m_fDistance < p2->m_fDistance)
    return -1;

  return 0;
}

void CVisAreaManager::MakeActiveEntransePortalsList(const CCamera & curCamera, list2<CVisArea *> & lstActiveEntransePortals, CVisArea * pThisPortal, CObjManager * pObjManager)
{
	lstActiveEntransePortals.Clear();
	for(int nPortalId=0; nPortalId<m_lstPortals.Count(); nPortalId++)
	{
		CVisArea * pPortal = m_lstPortals[nPortalId];
		if(pPortal->m_lstConnections.Count()==1 && pPortal != pThisPortal && 
      /*pPortal->IsActive() && */!pPortal->m_bSkyOnly)
		{
			if(curCamera.IsAABBVisibleFast( AABB(pPortal->m_vGeomBoxMin,pPortal->m_vGeomBoxMax) ))
			{
				Vec3d vNormal = pPortal->m_lstConnections[0]->GetConnectionNormal(pPortal);
				Vec3d vCenter = (pPortal->m_vBoxMin+pPortal->m_vBoxMax)*0.5f;
				if(vNormal.Dot(vCenter - curCamera.GetPos())<0)
					continue;
/*
				if(pCurPortal)
				{
					vNormal = pCurPortal->m_vConnNormals[0];
					if(vNormal.Dot(vCenter - curCamera.GetPos())<0)
						continue;
				}
	*/
				pPortal->m_fDistance = GetDistance( (pPortal->m_vBoxMin+pPortal->m_vBoxMax)*0.5f, curCamera.GetPos() );

				float fZoomFactor = 0.2f+0.8f*(RAD2DEG(curCamera.GetFov())/90.f);      
				float fRadius = (pPortal->m_vBoxMax-pPortal->m_vBoxMin).Length()*0.5f;
				if(pPortal->m_fDistance > fRadius*pPortal->m_fViewDistRatio/fZoomFactor)
					continue;

				// test occlusion by mountains
				if(pObjManager->IsBoxOccluded( pPortal->m_vGeomBoxMin, pPortal->m_vGeomBoxMax,pPortal->m_fDistance, &pPortal->m_OcclState))
					continue;

				// test occlusion by antiportals
				if(GetVisAreaManager()->IsOccludedByOcclVolumes(pPortal->m_vGeomBoxMin,pPortal->m_vGeomBoxMax))
					continue;

				lstActiveEntransePortals.Add(pPortal);

//				if(GetCVars()->e_portals==3)
	//				GetRenderer()->Draw3dBBox(pPortal->m_vGeomBoxMin, pPortal->m_vGeomBoxMax);
			}
		}
	}

	// sort by distance
	if(lstActiveEntransePortals.Count())
	{
		qsort(&lstActiveEntransePortals[0], lstActiveEntransePortals.Count(), 
			sizeof(lstActiveEntransePortals[0]), CVisAreaManager__CmpDistToPortal);
//		m_pCurPortal = lstActiveEntransePortals[0];
	}
}

void CVisAreaManager::MergeCameras(CCamera & cam1, const CCamera & cam2)
{	
	assert(0); // under development
/*	{
		float fDotLR1 =  cam1.GetFrustumPlane(FR_PLANE_LEFT )->n.Dot(cam1.GetFrustumPlane(FR_PLANE_RIGHT)->n);
		float fDotRL2 =  cam2.GetFrustumPlane(FR_PLANE_RIGHT)->n.Dot(cam2.GetFrustumPlane(FR_PLANE_LEFT)->n);
		int y=0;
	}
*/
  // left-right
  float fDotLR =  cam1.GetFrustumPlane(FR_PLANE_LEFT )->n.Dot(cam2.GetFrustumPlane(FR_PLANE_RIGHT)->n);
  float fDotRL =  cam1.GetFrustumPlane(FR_PLANE_RIGHT)->n.Dot(cam2.GetFrustumPlane(FR_PLANE_LEFT)->n);
  if(fabs(fDotLR)<fabs(fDotRL))
	{
    cam1.SetFrustumPlane(FR_PLANE_RIGHT, *cam2.GetFrustumPlane(FR_PLANE_RIGHT));
		cam1.SetFrustumVertex(2,cam2.GetFrustumVertex(2));
		cam1.SetFrustumVertex(3,cam2.GetFrustumVertex(3));
	}
  else
	{
    cam1.SetFrustumPlane(FR_PLANE_LEFT,  *cam2.GetFrustumPlane(FR_PLANE_LEFT));
		cam1.SetFrustumVertex(0,cam2.GetFrustumVertex(0));
		cam1.SetFrustumVertex(1,cam2.GetFrustumVertex(1));
	}

	/*
	{
		float fDotLR1 =  cam1.GetFrustumPlane(FR_PLANE_LEFT )->n.Dot(cam1.GetFrustumPlane(FR_PLANE_RIGHT)->n);
		float fDotRL2 =  cam2.GetFrustumPlane(FR_PLANE_RIGHT)->n.Dot(cam2.GetFrustumPlane(FR_PLANE_LEFT)->n);
		int y=0;
	}*/
/*
  // top-bottom
  float fDotTB =  cam1.GetFrustumPlane(FR_PLANE_TOP   )->n.Dot(cam2.GetFrustumPlane(FR_PLANE_BOTTOM)->n);
  float fDotBT =  cam1.GetFrustumPlane(FR_PLANE_BOTTOM)->n.Dot(cam2.GetFrustumPlane(FR_PLANE_TOP   )->n);
  
  if(fDotTB>fDotBT)
    cam1.SetFrustumPlane(FR_PLANE_BOTTOM, *cam2.GetFrustumPlane(FR_PLANE_BOTTOM));
  else
    cam1.SetFrustumPlane(FR_PLANE_TOP,    *cam2.GetFrustumPlane(FR_PLANE_TOP));
*/
  cam1.SetFrustumPlane(FR_PLANE_NEAR, *GetViewCamera().GetFrustumPlane(FR_PLANE_NEAR));
  cam1.SetFrustumPlane(FR_PLANE_FAR,  *GetViewCamera().GetFrustumPlane(FR_PLANE_FAR));

	cam1.SetFrustumPlane(FR_PLANE_TOP, *GetViewCamera().GetFrustumPlane(FR_PLANE_TOP));
	cam1.SetFrustumPlane(FR_PLANE_BOTTOM,  *GetViewCamera().GetFrustumPlane(FR_PLANE_BOTTOM));

/*
	if(GetCVars()->e_portals==4)
	{
		GetRenderer()->SetMaterialColor(1,1,1,1);
		GetRenderer()->Draw3dBBox(pVerts[0],pVerts[1],DPRIM_LINE);
		GetRenderer()->DrawLabel(pVerts[0],2,"0");
		GetRenderer()->Draw3dBBox(pVerts[1],pVerts[2],DPRIM_LINE);
		GetRenderer()->DrawLabel(pVerts[1],2,"1");
		GetRenderer()->Draw3dBBox(pVerts[2],pVerts[3],DPRIM_LINE);
		GetRenderer()->DrawLabel(pVerts[2],2,"2");
		GetRenderer()->Draw3dBBox(pVerts[3],pVerts[0],DPRIM_LINE);
		GetRenderer()->DrawLabel(pVerts[3],2,"3");
	}*/
}

void CVisAreaManager::DrawOcclusionAreasIntoCBuffer(CCoverageBuffer * pCBuffer)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );
	
	m_lstActiveOcclVolumes.Clear();
	m_lstIndoorActiveOcclVolumes.Clear();

	if(GetCVars()->e_occlusion_volumes)
  for(int i=0; i<m_lstOcclAreas.Count(); i++)
	{
		CVisArea * pArea = m_lstOcclAreas[i];
		if(GetViewCamera().IsAABBVisibleFast( AABB(pArea->m_vBoxMin,pArea->m_vBoxMax)))
		{
			Vec3d vPos = (pArea->m_vBoxMin+pArea->m_vBoxMax)*0.5f;
			float fRadius = (pArea->m_vBoxMin-pArea->m_vBoxMax).GetLength();
			float fDist = GetViewCamera().GetPos().GetDistance(vPos);
			float fZoomFactor = 0.2f+0.8f*(RAD2DEG(GetViewCamera().GetFov())/90.f);      
			if(fDist<fRadius*pArea->m_fViewDistRatio/fZoomFactor*0.125f && pArea->m_lstShapePoints.Count()>=2)
			{
				//				pArea->DrawAreaBoundsIntoCBuffer(pCBuffer);
				if(!pArea->m_pOcclCamera)
					pArea->m_pOcclCamera = new CCamera;
				*pArea->m_pOcclCamera = GetViewCamera();

				int nShift = pArea->m_lstShapePoints.Count()>2 &&
											(GetSquaredDistance(pArea->m_lstShapePoints[0], pArea->m_lstShapePoints[1])
											<GetSquaredDistance(pArea->m_lstShapePoints[1], pArea->m_lstShapePoints[2]));
				pArea->m_arrvActiveVerts[0] = pArea->m_lstShapePoints[0+nShift];
				pArea->m_arrvActiveVerts[1] = pArea->m_lstShapePoints[0+nShift]+Vec3d(0,0,pArea->m_fHeight);
				pArea->m_arrvActiveVerts[2] = pArea->m_lstShapePoints[1+nShift]+Vec3d(0,0,pArea->m_fHeight);
				pArea->m_arrvActiveVerts[3] = pArea->m_lstShapePoints[1+nShift];

				Plane plane;
				plane.CalcPlane(pArea->m_arrvActiveVerts[0], pArea->m_arrvActiveVerts[1], pArea->m_arrvActiveVerts[2]);

				if(plane.DistFromPlane(GetViewCamera().GetPos())<0)
				{
						pArea->m_arrvActiveVerts[3] = pArea->m_lstShapePoints[0+nShift];
						pArea->m_arrvActiveVerts[2] = pArea->m_lstShapePoints[0+nShift]+Vec3d(0,0,pArea->m_fHeight);
						pArea->m_arrvActiveVerts[1] = pArea->m_lstShapePoints[1+nShift]+Vec3d(0,0,pArea->m_fHeight);
						pArea->m_arrvActiveVerts[0] = pArea->m_lstShapePoints[1+nShift];
				}
				else if(!pArea->m_bDoubleSide)
						continue;

				GetRenderer()->SetMaterialColor(1,0,0,1);
				pArea->UpdatePortalCameraPlanes(*pArea->m_pOcclCamera, pArea->m_arrvActiveVerts, false);

				// make far plane never clip anything
				Plane newFarPlane;
				newFarPlane.CalcPlane(Vec3d(0,1,-1024), Vec3d(0,0,-1024), Vec3d(1,0,-1024));
				pArea->m_pOcclCamera->SetFrustumPlane(FR_PLANE_FAR, newFarPlane);

				m_lstActiveOcclVolumes.Add(pArea);
				pArea->m_fDistance = fDist;
			}
		}
	}

	if(m_lstActiveOcclVolumes.Count())
	{ // sort occluders by distance to the camera
		qsort(&m_lstActiveOcclVolumes[0], m_lstActiveOcclVolumes.Count(), 
			sizeof(m_lstActiveOcclVolumes[0]), CVisAreaManager__CmpDistToPortal);
	
		// remove occluded occluders
		for(int i=m_lstActiveOcclVolumes.Count()-1; i; i--)
		{
			CVisArea * pArea = m_lstActiveOcclVolumes[i];
			if(IsOccludedByOcclVolumes(pArea->m_vGeomBoxMin,pArea->m_vGeomBoxMax))
				m_lstActiveOcclVolumes.Delete(i);
		}

		// put indoor occluders into separate list
		for(int i=m_lstActiveOcclVolumes.Count()-1; i; i--)
		{
			CVisArea * pArea = m_lstActiveOcclVolumes[i];
			if(pArea->m_bUseInIndoors)
				m_lstIndoorActiveOcclVolumes.Add(pArea);
		}

		if(GetCVars()->e_portals==4)
		{	// show really active occluders
			for(int i=0; i<m_lstActiveOcclVolumes.Count(); i++)
			{
				CVisArea * pArea = m_lstActiveOcclVolumes[i];
				GetRenderer()->SetMaterialColor(0,1,0,1);
				GetRenderer()->Draw3dBBox(pArea->m_vGeomBoxMin,pArea->m_vGeomBoxMax);
			}
		}
	}
}

bool CVisAreaManager::IsOccludedByOcclVolumes(Vec3d vBoxMin, Vec3d vBoxMax, bool bCheckOnlyIndoorVolumes)
{
	list2<CVisArea*> & rList = bCheckOnlyIndoorVolumes ? m_lstIndoorActiveOcclVolumes : m_lstActiveOcclVolumes;

	for(int i=0; i<rList.Count(); i++)
	{
		bool bAllIn=0;
		if(rList[i]->m_pOcclCamera->IsAABBVisible_hierarchical(AABB(vBoxMin, vBoxMax), &bAllIn) && bAllIn)
			return true;
	}
	
	return false;
}

void CVisAreaManager::SortStaticInstancesBySize()
{
  // areas
  for(int v=0; v<m_lstVisAreas.Count(); v++)
    m_lstVisAreas[v]->SortStaticInstancesBySize();

  // portals
  for(int v=0; v<m_lstPortals.Count(); v++)
    m_lstPortals[v]->SortStaticInstancesBySize();
}

bool CVisAreaManager::PreloadResources()
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );
	bool bPreloadOutdoor = false;
	CVisArea * pVisArea = m_pCurArea ? m_pCurArea : m_pCurPortal;
	if(pVisArea)
		pVisArea->PreloadVisArea(GetCVars()->e_stream_preload_textures*2, &bPreloadOutdoor, 0, GetViewCamera().GetPos(), 0);
	else
		bPreloadOutdoor |= true;


	// areas
//	for(int v=0; v<m_lstVisAreas.Count(); v++)
	//	m_lstVisAreas[v]->PreloadResources();

	// portals
//	for(int v=0; v<m_lstPortals.Count(); v++)
	//	m_lstPortals[v]->PreloadResources();
/*
	CVisArea * pVisArea = m_pCurArea ? m_pCurArea : m_pCurPortal;
	if(pVisArea)
	{
		pVisArea->PreloadResources();
		CVisArea * areaArray[8]={0,0,0,0,0,0,0,0};
		int nCount = pVisArea->GetVisAreaConnections((IVisArea **)&areaArray,8);
		for(int n=0; n<nCount; n++)
			areaArray[n]->PreloadResources();
	}*/

//	GetLog()->Log("CVisAreaManager::PreloadResources: %d", (int)bPreloadOutdoor);

	return bPreloadOutdoor;
}

void CVisAreaManager::CheckUnload()
{
  m_nLoadedSectors=0;

  // areas
  for(int v=0; v<m_lstVisAreas.Count(); v++)
    m_nLoadedSectors += m_lstVisAreas[v]->CheckUnload();

  // portals
  for(int v=0; v<m_lstPortals.Count(); v++)
    m_nLoadedSectors += m_lstPortals[v]->CheckUnload();
}

void CVisAreaManager::GetStreamingStatus(int & nLoadedSectors, int & nTotalSectors)
{
  nLoadedSectors = m_nLoadedSectors;
  nTotalSectors = m_lstPortals.Count() + m_lstVisAreas.Count();
}

void CVisAreaManager::GetMemoryUsage(ICrySizer*pSizer)
{
  // areas
  for(int v=0; v<m_lstVisAreas.Count(); v++)
    m_lstVisAreas[v]->GetMemoryUsage(pSizer);

  // portals
  for(int v=0; v<m_lstPortals.Count(); v++)
    m_lstPortals[v]->GetMemoryUsage(pSizer);

  pSizer->AddObject(this,sizeof(*this));
}

bool CVisAreaManager::UnRegisterInAllSectors(IEntityRender * pEntityRS)
{
  bool bRes = 0;
  int nStatic = pEntityRS->IsStatic();
  // areas
  for(int v=0; v<m_lstVisAreas.Count(); v++)
    bRes |= m_lstVisAreas[v]->m_lstEntities[nStatic].Delete(pEntityRS);

  // portals
  for(int v=0; v<m_lstPortals.Count(); v++)
    bRes |= m_lstPortals[v]->m_lstEntities[nStatic].Delete(pEntityRS);

  pEntityRS->m_pVisArea=0;

  return bRes;
}




Vec3 CamAngles[6] =
{
		Vec3( 90, -90,  0),  //posx
		Vec3( 90,  90,  0 ),  //negx
		Vec3(180, 180,  0 ),  //posy
		Vec3(  0, 180,  0 ),  //negy
		Vec3( 90, 180,  0 ),  //posz
		Vec3( 90,   0,  0 )  //negz
};


void CVisAreaManager::Preceche(CObjManager * pObjManager)
{
	if(!GetCVars()->e_precache_level)
		return;

	//--------------------------------------------------------------------------------------
	//----                  PRE-FETCHING OF RENDER-DATA IN INDOORS                      ----
	//--------------------------------------------------------------------------------------

	GetRenderer()->EnableSwapBuffers(false);

	//loop over all sectors and place a light in the middle of the sector  
	for(int v=0; v<m_lstVisAreas.Count(); v++)
	{
		unsigned short * pPtr2FrameID = (unsigned short *)GetRenderer()->EF_Query(EFQ_Pointer2FrameID);
		if(pPtr2FrameID)
			(*pPtr2FrameID)++;

		m_nRenderFrameID = GetRenderer()->GetFrameID();


		CDLight DynLight;

		//place camera in the middle of a sector and render sector form 
		//different directions
		for(int i=0; i<6; i++) {

			GetRenderer()->BeginFrame();
	
			// Add sun lsource
			DynLight.m_Origin = (m_lstVisAreas[v]->m_vBoxMin + m_lstVisAreas[v]->m_vBoxMax)*0.5f;
			DynLight.m_fRadius  = 100;
			DynLight.m_Flags |= DLF_LM;
			Get3DEngine()->AddDynamicLightSource(DynLight,(IEntityRender*)-1);

			CCamera cam = GetViewCamera();

			cam.SetPos( DynLight.m_Origin );
			cam.SetAngle( CamAngles[i] );
 			cam.SetFov(1.7f);  //very wide-opend fov
			Get3DEngine()->SetCamera(cam);

			Get3DEngine()->Draw();
			GetRenderer()->Update();
		}

	}



//--------------------------------------------------------------------------------------
//----                  PRE-FETCHING OF RENDER-DATA IN OUTDOORS                     ----
//--------------------------------------------------------------------------------------
	
//we search all TagPoints with the label "Cam_PreFetch??"	in the level  
//and store the positions in an array  
	Vec3 CamOutdoorPositions[100]; //array for 100 cam-postionin outdoors ()
	int ValidPosition=0;
	char TagName[80];
	IGame *pGame = GetISystem()->GetIGame();
	if (pGame)
	{
		for (int cp=0; cp<99; cp++ )
		{
			sprintf(TagName, "Cam_PreFetch%02d", cp);
			ITagPoint *pTagPoint = pGame->GetTagPointManager()->GetTagPoint( TagName );
			if (pTagPoint) 
			{
				pTagPoint->GetPos( CamOutdoorPositions[ValidPosition] );
				ValidPosition++;
			}
		}
	}


//loop over all cam-position in the level and render this part of the level 
//from 6 different directions
	for(int p=0; p<ValidPosition; p++) //loop over outdoor-camera position
	{ 
		for(int i=0; i<6; i++) //loop over 6 camera orientations
		{

			GetRenderer()->BeginFrame();

			CCamera cam = GetViewCamera();
			cam.SetPos( CamOutdoorPositions[p] );
			cam.SetAngle( CamAngles[i] );
			cam.SetFov(1.7f);  //very wide-opend fov
			Get3DEngine()->SetCamera(cam);

			Get3DEngine()->Draw();
			GetRenderer()->Update();
		}
	}
	GetCVars()->e_sleep = 0;

	((C3DEngine*)Get3DEngine())->GetDynamicLightSources()->Clear();
	GetRenderer()->EF_ClearLightsList();
	((C3DEngine*)Get3DEngine())->UpdateLightSources();
	((C3DEngine*)Get3DEngine())->PrepareLightSourcesForRendering();    

	GetRenderer()->EnableSwapBuffers(true);
}

void CVisAreaManager::GetObjectsAround(Vec3d vExploPos, float fExploRadius, list2<IEntityRender*> * pEntList)
{
	CVisArea * pVisArea = (CVisArea *)GetVisAreaFromPos(vExploPos);

	// find static objects around
	for(int i=0; pVisArea && i<pVisArea->m_lstEntities[STATIC_ENTITIES].Count(); i++)
	{
		IEntityRender * pEntityRender =	pVisArea->m_lstEntities[STATIC_ENTITIES][i];
		Vec3d vEntBoxMin, vEntBoxMax;
		pEntityRender->GetBBox(vEntBoxMin, vEntBoxMax);
		if(Overlap::Sphere_AABB(Sphere(vExploPos,fExploRadius), AABB(vEntBoxMin,vEntBoxMax)))
			if(pEntList->Find(pEntityRender)<0)
				pEntList->Add(pEntityRender);
	}
}
