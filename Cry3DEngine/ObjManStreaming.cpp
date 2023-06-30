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

void CObjManager::PreloadNearObjects()
{
//  if(!GetCVars()->e_stream_cgf)
    return;

  CVisAreaManager * pVisAreaManager = GetVisAreaManager();
//  CVisArea * pVisArea = pVisAreaManager->m_pCurArea ? pVisAreaManager->m_pCurArea : pVisAreaManager->m_pCurPortal;
  if(CStatObj::m_fStreamingTimePerFrame<CGF_STREAMING_MAX_TIME_PER_FRAME)// && pVisArea)
  { // mark this and neighbor sectors
/*    pVisArea->MarkForStreaming();
    CVisArea * areaArray[8]={0,0,0,0,0,0,0,0};
    int nCount = pVisArea->GetVisAreaConnections((IVisArea **)&areaArray,8);
    for(int n=0; n<nCount; n++)
      areaArray[n]->MarkForStreaming();
  */
    // load marked objects
    for (ObjectsMap::iterator it = m_lstLoadedObjects.begin(); it != m_lstLoadedObjects.end(); ++it)
    {
      if((*it)->m_bUseStreaming && !(*it)->GetLeafBuffer() &&	(*it)->m_nMarkedForStreamingFrameId > GetFrameID()+10)
      {	// streaming
        CStatObj::m_fStreamingTimePerFrame -= GetTimer()->GetAsyncCurTime();
        bool bRes = (*it)->Load((*it)->m_szFileName, (*it)->m_szGeomName[0] ? (*it)->m_szGeomName : 0, 
          (*it)->m_eVertsSharing, (*it)->m_bLoadAdditinalInfo, (*it)->m_bKeepInLocalSpace, false);
        (*it)->m_bUseStreaming=true;
        CStatObj::m_fStreamingTimePerFrame += GetTimer()->GetAsyncCurTime();
      }

      if(CStatObj::m_fStreamingTimePerFrame>CGF_STREAMING_MAX_TIME_PER_FRAME)
        break;
    }
  }
}

void CObjManager::CheckUnload()
{
  if(GetCVars()->e_stream_cgf)
  for (ObjectsMap::iterator it = m_lstLoadedObjects.begin(); it != m_lstLoadedObjects.end(); ++it)
  {
    CStatObj * pStatObj = (*it);
    if(pStatObj->m_bUseStreaming && pStatObj->GetLeafBuffer() &&
      max(pStatObj->m_nLastRendFrameId, pStatObj->m_nMarkedForStreamingFrameId) < GetFrameID()-100)
    {
			int p;
      for(p=0; p<2; p++)
      { // check is phys geometry still in use
        phys_geometry * pPhysGeom = pStatObj->GetPhysGeom(p);
        if(pPhysGeom && pPhysGeom->nRefCount>1)
          break;
      }

      if(p==2)
      {
        pStatObj->ShutDown();
        pStatObj->Init();
        GetLog()->Log("Unloaded: %s", pStatObj->m_szFileName);
        break;
      }
    }
  }

  if(GetFrameID()%32==0)
  for (ObjectsMap::iterator it = m_lstLoadedObjects.begin(); it != m_lstLoadedObjects.end(); ++it)
  {
    if((*it)->GetLeafBuffer() && (*it)->GetShadowVolume())
      (*it)->GetShadowVolume()->CheckUnload();
  }
}

void CObjManager::GetObjectsStreamingStatus(int & nReady, int & nTotalInStreaming, int & nTotal)
{
  nReady=nTotalInStreaming=nTotal=0;
  for (ObjectsMap::iterator it = m_lstLoadedObjects.begin(); it != m_lstLoadedObjects.end(); ++it)
  {
    nTotal++;
    if((*it)->m_bUseStreaming)
    {
      nTotalInStreaming++;
      if((*it)->GetLeafBuffer())
      {
        CStatObj * p = (*it);
        nReady++;
      }
    }
    else
    {
      CStatObj * p = (*it);
      p=p;
      p->m_szFileName;
    }
  }
}

