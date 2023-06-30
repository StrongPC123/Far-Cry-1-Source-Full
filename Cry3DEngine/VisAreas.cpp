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

void CVisArea::Update(const Vec3d * pPoints, int nCount, const char * szName, float fHeight, const Vec3d & vAmbientColor, bool bAfectedByOutLights, bool bSkyOnly, const Vec3d & vDynAmbientColor, float fViewDistRatio, bool bDoubleSide, bool bUseDeepness, bool bUseInIndoors)
{
	strncpy(m_sName, szName, sizeof(m_sName));
	strlwr(m_sName);

	m_fHeight = fHeight;
	m_vAmbColor = vAmbientColor;
	m_vDynAmbColor = vDynAmbientColor;
	m_bAfectedByOutLights = bAfectedByOutLights;
  m_bSkyOnly = bSkyOnly;
	m_fViewDistRatio = fViewDistRatio;
	m_bDoubleSide = bDoubleSide;
//	m_bUseDeepness = bUseDeepness;
	m_bUseInIndoors = bUseInIndoors;

	m_lstShapePoints.PreAllocate(nCount,nCount);
	
	if(nCount)
		memcpy(&m_lstShapePoints[0], pPoints, sizeof(Vec3d)*nCount);

	// update bbox
	m_vBoxMax=SetMinBB();
	m_vBoxMin=SetMaxBB();

	for(int i=0; i<nCount; i++)
	{
		m_vBoxMax.CheckMax(pPoints[i]);
		m_vBoxMin.CheckMin(pPoints[i]);

		m_vBoxMax.CheckMax(pPoints[i]+Vec3d(0,0,m_fHeight));
		m_vBoxMin.CheckMin(pPoints[i]+Vec3d(0,0,m_fHeight));
	}

	UpdateGeometryBBox();
}

#define PORTAL_GEOM_BBOX_EXTENT 3.f

void CVisArea::UpdateGeometryBBox()
{
	m_vGeomBoxMax = m_vBoxMax;
	m_vGeomBoxMin = m_vBoxMin;

	if(IsPortal())
	{ // fix for big objects passing portal
		m_vGeomBoxMax+=Vec3d(PORTAL_GEOM_BBOX_EXTENT,PORTAL_GEOM_BBOX_EXTENT,PORTAL_GEOM_BBOX_EXTENT);
		m_vGeomBoxMin-=Vec3d(PORTAL_GEOM_BBOX_EXTENT,PORTAL_GEOM_BBOX_EXTENT,PORTAL_GEOM_BBOX_EXTENT);
	}

	for(int i=0; i<m_lstEntities[STATIC_ENTITIES].Count(); i++)
	if(m_lstEntities[STATIC_ENTITIES][i]->IsStatic())
	{
		Vec3d vEntBoxMin,vEntBoxMax;
		m_lstEntities[STATIC_ENTITIES][i]->GetRenderBBox(vEntBoxMin,vEntBoxMax);
		m_vGeomBoxMin.CheckMin(vEntBoxMin);
		m_vGeomBoxMax.CheckMax(vEntBoxMax);
	}
}

void CVisArea::MarkForStreaming()
{
	for(int i=0; i<m_lstEntities[STATIC_ENTITIES].Count(); i++)
	{
		if(m_lstEntities[STATIC_ENTITIES][i]->GetEntityStatObj(0))
    {
      ((CStatObj*)m_lstEntities[STATIC_ENTITIES][i]->GetEntityStatObj(0))->m_nMarkedForStreamingFrameId = GetFrameID()+100;
//      m_lstEntities[STATIC_ENTITIES][i]->CheckPhysicalized();
    }
	}
}

CVisArea::CVisArea(bool bLoadedAsAreaBox) 
{
	m_vGeomBoxMin=m_vGeomBoxMax=m_vBoxMin=m_vBoxMax=Vec3d(0,0,0);
	m_sName[0]=0;
	m_nRndFrameId=-1;
	m_bActive=true;
	m_nFogVolumeId=0;
	m_fHeight=0;
	m_vAmbColor(0,0,0);
	m_vDynAmbColor(0,0,0);
	m_bLoadedAsAreaBox = bLoadedAsAreaBox;
	m_vConnNormals[0]=m_vConnNormals[1]=Vec3d(0,0,0);
	m_bAfectedByOutLights = false;
	m_fDistance=0;
  m_bSkyOnly=false;
	m_pOcclCamera=0;
	m_fViewDistRatio = 100.f;
	m_bDoubleSide = true;
//	m_bUseDeepness = false;
	m_bUseInIndoors = false;
	memset(m_arrvActiveVerts,0,sizeof(m_arrvActiveVerts));
}

CVisArea::~CVisArea()
{
	Unload();
	UnregisterDynamicEntities();
	delete m_pOcclCamera;
	m_pOcclCamera=0;

/*
  for(int nStatic=0; nStatic<2; nStatic++)
  for(int i=0; i<m_lstEntities[nStatic].Count(); i++)
    if(m_lstEntities[nStatic][i]->m_pVisArea==this)
      m_lstEntities[nStatic][i]->m_pVisArea=0;
			*/
}

bool InsidePolygon(Vec3 *polygon,int N,Vec3 p)
{
  int counter = 0;
  int i;
  double xinters;
  Vec3 p1,p2;

  p1 = polygon[0];
  for (i=1;i<=N;i++) {
    p2 = polygon[i % N];
    if (p.y > min(p1.y,p2.y)) {
      if (p.y <= max(p1.y,p2.y)) {
        if (p.x <= max(p1.x,p2.x)) {
          if (p1.y != p2.y) {
            xinters = (p.y-p1.y)*(p2.x-p1.x)/(p2.y-p1.y)+p1.x;
            if (p1.x == p2.x || p.x <= xinters)
              counter++;
          }
        }
      }
    }
    p1 = p2;
  }

  if (counter % 2 == 0)
    return(false);
  else
    return(true);
}

bool CVisArea::IsPointInsideVisArea(const Vec3d & vPos)
{
	if(Overlap::Point_AABB(vPos, m_vBoxMin, m_vBoxMax))
	if(InsidePolygon(&m_lstShapePoints[0], m_lstShapePoints.Count(), vPos))
		return true;

	return false;
}

bool CVisArea::FindVisArea(IVisArea * pAnotherArea, int nMaxReqursion, bool bSkipDisabledPortals)
{ // todo: avoid going into parent
	if(pAnotherArea == this)
		return true;

	if(nMaxReqursion>0)
	for(int p=0; p<m_lstConnections.Count(); p++)
		if(!bSkipDisabledPortals || m_lstConnections[p]->IsActive())
			if(m_lstConnections[p]->FindVisArea(pAnotherArea, nMaxReqursion-1, bSkipDisabledPortals))
				return true;

	return false;
}

bool CVisArea::PreloadVisArea(int nMaxReqursion, bool * pbOutdoorFound, CVisArea * pParentToAvoid, Vec3d vPrevPortalPos, float fPrevPortalDistance)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );
	if(IsPortal())
	{
		fPrevPortalDistance += vPrevPortalPos.GetDistance((m_vBoxMin+m_vBoxMax)*0.5f);
		vPrevPortalPos = (m_vBoxMin+m_vBoxMax)*0.5f;
	}

	PreloadResources(vPrevPortalPos, fPrevPortalDistance);

	if(IsConnectedToOutdoor())
		*pbOutdoorFound = true;

	if(nMaxReqursion>0)
		for(int p=0; p<m_lstConnections.Count(); p++)
			if(m_lstConnections[p] != pParentToAvoid)
				if(GetCurTimeSec()>(m_fPreloadStartTime+0.010f)||
					m_lstConnections[p]->PreloadVisArea(nMaxReqursion-1, pbOutdoorFound, this, vPrevPortalPos, fPrevPortalDistance))
					return true;

	return false;
}

int CVisArea::GetVisFrameId()
{
	return m_nRndFrameId;
}

bool CVisArea::IsConnectedToOutdoor()
{
	if(IsPortal()) // check if this portal has just one conection
		return m_lstConnections.Count()==1;

	// find portals with just one conection
	for(int p=0; p<m_lstConnections.Count(); p++)
	{
		CVisArea * pPortal = m_lstConnections[p];
		if(pPortal->m_lstConnections.Count()==1)
			return true;
	}

	return false;
}

bool Is2dLinesIntersect(float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4)
{
	float ua = ((x4-x3)*(y1-y3)-(y4-y3)*(x1-x3))/((y4-y3)*(x2-x1)-(x4-x3)*(y2-y1));
	float ub = ((x2-x1)*(y1-y3)-(y2-y1)*(x1-x3))/((y4-y3)*(x2-x1)-(x4-x3)*(y2-y1));
	return ua>0 && ua<1 && ub>0 && ub<1;
}

Vec3d CVisArea::GetConnectionNormal(CVisArea * pPortal)
{
//	if(strstr(pPortal->m_sName,"ab09_portal11"))
	//	int t=0;

	assert(m_lstShapePoints.Count()>=3);
	// find side of shape intersecting with portal
	int nIntersNum = 0;
	Vec3d arrNormals[2]={Vec3d(0,0,0),Vec3d(0,0,0)};
	for(int v=0; v<m_lstShapePoints.Count(); v++)
	{
		nIntersNum=0;
		arrNormals[0]=Vec3d(0,0,0);
		arrNormals[1]=Vec3d(0,0,0);
		for(int p=0; p<pPortal->m_lstShapePoints.Count(); p++)
		{
			const Vec3d & v0 = m_lstShapePoints[v];
			const Vec3d & v1 = m_lstShapePoints[(v+1)%m_lstShapePoints.Count()];
			const Vec3d & p0 = pPortal->m_lstShapePoints[p];
			const Vec3d & p1 = pPortal->m_lstShapePoints[(p+1)%pPortal->m_lstShapePoints.Count()];

			if(Is2dLinesIntersect(v0.x,v0.y,v1.x,v1.y,p0.x,p0.y,p1.x,p1.y))
			{
				Vec3d vNormal = GetNormalized(v0-v1).Cross(Vec3d(0,0,1.f));			
				if(nIntersNum<2)
					arrNormals[nIntersNum++] = (IsShapeClockwise()) ? -vNormal : vNormal;
			}
		}

		if(nIntersNum==2)
			break;
	}

	if(nIntersNum == 2 && 
		//IsEquivalent(arrNormals[0] == arrNormals[1])
		IsEquivalent(arrNormals[0],arrNormals[1],VEC_EPSILON)
		)
		return arrNormals[0];

	{
		int nBottomPoints=0;
		for(int p=0; p<pPortal->m_lstShapePoints.Count() && p<4; p++)
			if(IsPointInsideVisArea(pPortal->m_lstShapePoints[p]))
				nBottomPoints++;

		int nUpPoints=0;
		for(int p=0; p<pPortal->m_lstShapePoints.Count() && p<4; p++)
			if(IsPointInsideVisArea(pPortal->m_lstShapePoints[p]+Vec3d(0,0,pPortal->m_fHeight)))
				nUpPoints++;

		if(nBottomPoints==0 && nUpPoints==4)
			return Vec3d(0,0,1);

		if(nBottomPoints==4 && nUpPoints==0)
			return Vec3d(0,0,-1);
	}

	return Vec3d(0,0,0);
}

bool CVisArea::UpdatePortalCameraScissor(CCamera & cam, list2<Vec3d> * lstVerts, bool bMergeFrustums)
{
  IRenderer * pRend = GetRenderer();
  CVars * pCVars = GetCVars();
  
  Vec3d arrScreenSpacePos[8];
  memset(arrScreenSpacePos,0,sizeof(arrScreenSpacePos));
  for(int i=0; i<lstVerts->Count() && i<8; i++)
  { // result is in range from 0 to 100
    const Vec3d & v3d = lstVerts->GetAt(i);

    if(pCVars->e_portals == 4)
      pRend->Draw3dBBox(v3d-Vec3d(0.1f,0.1f,0.1f),v3d+Vec3d(0.1f,0.1f,0.1f));

    pRend->ProjectToScreen(v3d.x, v3d.y, v3d.z, 
      &arrScreenSpacePos[i].x, &arrScreenSpacePos[i].y, &arrScreenSpacePos[i].z);

    arrScreenSpacePos[i].x = arrScreenSpacePos[i].x*pRend->GetWidth()/100.f;
    arrScreenSpacePos[i].y = arrScreenSpacePos[i].y*pRend->GetHeight()/100.f;
  }

  // find 2d bounds in screen space
  Vec3d vMin2d = arrScreenSpacePos[0], vMax2d = arrScreenSpacePos[0];
  for(int i=0; i<lstVerts->Count() && i<8; i++)
  {
    if(arrScreenSpacePos[i].x < vMin2d.x)
      vMin2d.x = arrScreenSpacePos[i].x;
    if(arrScreenSpacePos[i].x > vMax2d.x)
      vMax2d.x = arrScreenSpacePos[i].x;
    if(arrScreenSpacePos[i].y < vMin2d.y)
      vMin2d.y = arrScreenSpacePos[i].y;
    if(arrScreenSpacePos[i].y > vMax2d.y)
      vMax2d.y = arrScreenSpacePos[i].y;
  }

  vMin2d.x = max(vMin2d.x,0);
  vMin2d.y = max(vMin2d.y,0);
  vMax2d.x = min(vMax2d.x,GetRenderer()->GetWidth());
  vMax2d.y = min(vMax2d.y,GetRenderer()->GetHeight());

  if(vMax2d.x <= vMin2d.x || vMax2d.y < vMin2d.y)
    return false;
          
  assert(vMin2d.x>=0 && vMin2d.x<=GetRenderer()->GetWidth());
  assert(vMin2d.y>=0 && vMin2d.y<=GetRenderer()->GetHeight());
  assert(vMax2d.x>=0 && vMax2d.x<=GetRenderer()->GetWidth());
  assert(vMax2d.y>=0 && vMax2d.y<=GetRenderer()->GetHeight());
  
  cam.m_ScissorInfo.x1 = ushort(vMin2d.x);
  cam.m_ScissorInfo.y1 = ushort(vMin2d.y);
  cam.m_ScissorInfo.x2 = ushort(vMax2d.x);
  cam.m_ScissorInfo.y2 = ushort(vMax2d.y);
    
  if(GetCVars()->e_scissor_debug)
  {
    float color[] = {1,0,0,1};
    pRend->Draw2dLabel(vMax2d.x, vMax2d.y, 2 , color, false, "br");
    pRend->Draw2dLabel(vMin2d.x, vMax2d.y, 2 , color, false, "bl");
    pRend->Draw2dLabel(vMax2d.x, vMin2d.y, 2 , color, false, "tr");
    pRend->Draw2dLabel(vMin2d.x, vMin2d.y, 2 , color, false, "tl");
  }

  return true;
}

void CVisArea::UpdatePortalCameraPlanes(CCamera & cam, Vec3d * pVerts, bool bMergeFrustums)
{ // todo: do also take into account GetViewCamera()
	Vec3d vCamPos = GetViewCamera().GetPos();
	Plane plane;

	plane.CalcPlane(pVerts[0],pVerts[1],pVerts[2]);
	cam.SetFrustumPlane(FR_PLANE_NEAR, plane);
	
	plane = *GetViewCamera().GetFrustumPlane(FR_PLANE_FAR);
	cam.SetFrustumPlane(FR_PLANE_FAR, plane);
		
	plane.CalcPlane(vCamPos,pVerts[2],pVerts[3]);	// update plane only if it reduces fov
	if(!bMergeFrustums || plane.n.Dot(cam.GetFrustumPlane(FR_PLANE_LEFT)->n)<
		cam.GetFrustumPlane(FR_PLANE_RIGHT)->n.Dot(cam.GetFrustumPlane(FR_PLANE_LEFT)->n))
		cam.SetFrustumPlane(FR_PLANE_RIGHT, plane);
		
	plane.CalcPlane(vCamPos,pVerts[0],pVerts[1]);	// update plane only if it reduces fov
	if(!bMergeFrustums || plane.n.Dot(cam.GetFrustumPlane(FR_PLANE_RIGHT)->n)<
		cam.GetFrustumPlane(FR_PLANE_LEFT)->n.Dot(cam.GetFrustumPlane(FR_PLANE_RIGHT)->n))
		cam.SetFrustumPlane(FR_PLANE_LEFT, plane);

	plane.CalcPlane(vCamPos,pVerts[3],pVerts[0]);	// update plane only if it reduces fov
	if(!bMergeFrustums || plane.n.Dot(cam.GetFrustumPlane(FR_PLANE_TOP)->n)<
		cam.GetFrustumPlane(FR_PLANE_BOTTOM)->n.Dot(cam.GetFrustumPlane(FR_PLANE_TOP)->n))
		cam.SetFrustumPlane(FR_PLANE_BOTTOM, plane);

	plane.CalcPlane(vCamPos,pVerts[1],pVerts[2]); // update plane only if it reduces fov
	if(!bMergeFrustums || plane.n.Dot(cam.GetFrustumPlane(FR_PLANE_BOTTOM)->n)<
		cam.GetFrustumPlane(FR_PLANE_TOP)->n.Dot(cam.GetFrustumPlane(FR_PLANE_BOTTOM)->n))
		cam.SetFrustumPlane(FR_PLANE_TOP, plane);

	Vec3d arrvPortVertsCamSpace[4];
	for(int i=0; i<4; i++)
		arrvPortVertsCamSpace[i] = pVerts[i]-cam.GetPos();
	cam.SetFrustumVertices(arrvPortVertsCamSpace);

	if(GetCVars()->e_portals==4)
	{
    float farrColor[4] = {1,1,1,1};
//		GetRenderer()->SetMaterialColor(1,1,1,1);
		GetRenderer()->Draw3dBBox(pVerts[0],pVerts[1],DPRIM_LINE);
		GetRenderer()->DrawLabelEx(pVerts[0],1,farrColor,false,true,"0");
		GetRenderer()->Draw3dBBox(pVerts[1],pVerts[2],DPRIM_LINE);
		GetRenderer()->DrawLabelEx(pVerts[1],1,farrColor,false,true,"1");
		GetRenderer()->Draw3dBBox(pVerts[2],pVerts[3],DPRIM_LINE);
		GetRenderer()->DrawLabelEx(pVerts[2],1,farrColor,false,true,"2");
		GetRenderer()->Draw3dBBox(pVerts[3],pVerts[0],DPRIM_LINE);
		GetRenderer()->DrawLabelEx(pVerts[3],1,farrColor,false,true,"3");
	}
}

int __cdecl CVisAreaManager__CmpDistToPortal(const void* v1, const void* v2);

void CVisArea::DrawVolume(CObjManager * pObjManager, int nReqursionLevel, 
													CCamera CurCamera, CVisArea * pParent, CVisArea * pCurPortal, 
													bool * pbOutdoorVisible, list2<CCamera> * plstOutPortCameras, bool * pbSkyVisible)
{
  IRenderer * pRenderer = GetRenderer();

	// mark as rendered
	if(!pObjManager->m_nRenderStackLevel)
		m_nRndFrameId = GetFrameID();

	// get area light mask
	Vec3d vCenter = (m_vBoxMin+m_vBoxMax)*0.5f;
	float fRadius = (m_vBoxMax-m_vBoxMin).Length()*0.5f;
	int nDLMask = Get3DEngine()->GetLightMaskFromPosition(vCenter, fRadius);

  // todo: prepare flag once
  bool bThisIsPortal = strstr(m_sName,"portal") != 0;

  // remove sun bit if it not allowed on exit portal geometry
  if(!bThisIsPortal || !m_bAfectedByOutLights || m_lstConnections.Count()!=1)
  for(int nId=0; nId<32; nId++)
  {
    if(nDLMask & (1<<nId))
    {
      CDLight * pDLight = (CDLight*)pRenderer->EF_Query(EFQ_LightSource, nId);
			if(pDLight && pDLight->m_Flags & DLF_SUN)
			{
				nDLMask = nDLMask & ~(1<<nId);
				break;
			}

			if(!pObjManager->m_nRenderStackLevel) // light scissor can not be shared between reqursion levels because same CDlight objects are used
      if(pDLight && pDLight->m_Flags & DLF_THIS_AREA_ONLY && pDLight->m_pOwner)
      if(pDLight->m_pOwner->GetEntityVisArea() == this)
      {
        if(!pDLight->m_sWidth || pDLight->m_sWidth == (CurCamera.m_ScissorInfo.x2-CurCamera.m_ScissorInfo.x1))
        { // first time - set
          pDLight->m_sX = CurCamera.m_ScissorInfo.x1;
          pDLight->m_sY = CurCamera.m_ScissorInfo.y1;
          pDLight->m_sWidth = CurCamera.m_ScissorInfo.x2-CurCamera.m_ScissorInfo.x1;
          pDLight->m_sHeight = CurCamera.m_ScissorInfo.y2-CurCamera.m_ScissorInfo.y1;
        }
        else
        { // not first time - merge
          int nMaxX = max(pDLight->m_sX + pDLight->m_sWidth,  CurCamera.m_ScissorInfo.x2);
          int nMaxY = max(pDLight->m_sY + pDLight->m_sHeight, CurCamera.m_ScissorInfo.y2);
          pDLight->m_sX = min(pDLight->m_sX, CurCamera.m_ScissorInfo.x1);
          pDLight->m_sY = min(pDLight->m_sY, CurCamera.m_ScissorInfo.y1);
          pDLight->m_sWidth  = nMaxX - pDLight->m_sX;
          pDLight->m_sHeight = nMaxY - pDLight->m_sY;
        }
      }
		}
	}

	// render area statics
	DrawEntities( m_nFogVolumeId, nDLMask, 0, CurCamera, 
		m_lstShapePoints.Count() ? &m_vAmbColor : 0, m_lstShapePoints.Count() ? &m_vDynAmbColor : 0,
		NULL, true, 0, pObjManager, 
		IsPointInsideVisArea(GetViewCamera().GetPos()), "", STATIC_ENTITIES);

	// render area entities
  DrawEntities( m_nFogVolumeId, nDLMask, 0, CurCamera, 
    m_lstShapePoints.Count() ? &m_vAmbColor : 0, m_lstShapePoints.Count() ? &m_vDynAmbColor : 0,
		NULL, true, 0, pObjManager, 
		IsPointInsideVisArea(GetViewCamera().GetPos()), "", DYNAMIC_ENTITIES);

	// limit recursion and portal activity
	if(!nReqursionLevel || !m_bActive)
		return;

	if(	bThisIsPortal && m_lstConnections.Count()==1 && // detect entrance
		 !IsPointInsideVisArea(GetViewCamera().GetPos()) && // detect camera in outdoors
		 !CurCamera.IsAABBVisibleFast( AABB(m_vGeomBoxMin,m_vGeomBoxMax) )) // if invisible 
		 return; // stop recursion

	bool bScisorValid = true;

	// prepare new camera for next areas
	if(bThisIsPortal && m_lstConnections.Count() && (this!=pCurPortal || !pCurPortal->IsPointInsideVisArea(CurCamera.GetPos())))
	{
		Vec3d vPortNorm = (!pParent || pParent == m_lstConnections[0] || m_lstConnections.Count()==1) ? 
			m_vConnNormals[0] : m_vConnNormals[1];

		// exit/entrance portal has only one normal in direction to outdoors, so flip it to the camera
		if(m_lstConnections.Count()==1 && !pParent)
			vPortNorm = -vPortNorm; 

		// back face check
		Vec3d vPortToCamDir = CurCamera.GetPos() - (m_vBoxMin+m_vBoxMax)*0.5f;
		if(vPortToCamDir.Dot(vPortNorm)<0)
			return;

		if(!m_bDoubleSide)
			if(vPortToCamDir.Dot(m_vConnNormals[0])<0)
				return;

		Vec3d arrPortVerts[4];
		Vec3d arrPortVertsOtherSide[4]; 
		bool barrPortVertsOtherSideValid = false;
		if(pParent && !IsEquivalent(vPortNorm,Vec3d(0,0,0),VEC_EPSILON) && vPortNorm.z)
		{ // up/down portal
			int nEven = IsShapeClockwise();
			if(vPortNorm.z>0)
				nEven=!nEven;
			for(int i=0; i<4; i++)
			{
				arrPortVerts[i] = m_lstShapePoints[nEven ? (3-i) : i]+Vec3d(0,0,m_fHeight)*(vPortNorm.z>0);
				arrPortVertsOtherSide[i] = m_lstShapePoints[nEven ? (3-i) : i]+Vec3d(0,0,m_fHeight)*(vPortNorm.z<0);
			}
			barrPortVertsOtherSideValid = true;
		}
		else if(!IsEquivalent(vPortNorm,Vec3d(0,0,0),VEC_EPSILON)	&& vPortNorm.z==0)
		{ // basic portal
			Vec3d arrInAreaPoint[2]={Vec3d(0,0,0),Vec3d(0,0,0)};
			int arrInAreaPointId[2]={-1,-1};
			int nInAreaPointCounter=0;
			// find 2 points of portal in this area (or in this outdoors)
			for(int i=0; i<m_lstShapePoints.Count() && nInAreaPointCounter<2; i++)
			{ 
				Vec3d vTestPoint = m_lstShapePoints[i]+Vec3d(0,0,m_fHeight*0.5f);
				CVisArea * pAnotherArea = m_lstConnections[0];
				if((pParent && (pParent->IsPointInsideVisArea(vTestPoint))) || 
					(!pParent && (!pAnotherArea->IsPointInsideVisArea(vTestPoint))) )
				{
					arrInAreaPointId[nInAreaPointCounter] = i;
					arrInAreaPoint[nInAreaPointCounter++] = m_lstShapePoints[i];
				}
			}

			if(nInAreaPointCounter==2)
			{ // success, take into account volume and portal shape versts order
				int nEven = IsShapeClockwise();
				if(arrInAreaPointId[1]-arrInAreaPointId[0] != 1)
					nEven = !nEven;

				arrPortVerts[0] = arrInAreaPoint[nEven];
				arrPortVerts[1] = arrInAreaPoint[nEven]+Vec3d(0,0,m_fHeight);
				arrPortVerts[2] = arrInAreaPoint[!nEven]+Vec3d(0,0,m_fHeight);
				arrPortVerts[3] = arrInAreaPoint[!nEven];

				nEven = !nEven;

				arrPortVertsOtherSide[0] = arrInAreaPoint[nEven];
				arrPortVertsOtherSide[1] = arrInAreaPoint[nEven]+Vec3d(0,0,m_fHeight);
				arrPortVertsOtherSide[2] = arrInAreaPoint[!nEven]+Vec3d(0,0,m_fHeight);
				arrPortVertsOtherSide[3] = arrInAreaPoint[!nEven];
				barrPortVertsOtherSideValid = true;
			}
			else
			{ // something wrong
				Warning(0,0,"CVisArea::DrawVolume: Invalid portal: %s", m_sName);
				return;
			}
		}
		else if(!pParent && vPortNorm.z==0 && m_lstConnections.Count()==1)
		{ // basic entrance portal
			Vec3d vBorder = GetNormalized(vPortNorm.Cross(Vec3d(0,0,1.f)))*fRadius;
			arrPortVerts[0] = vCenter - Vec3d(0,0,1.f)*fRadius - vBorder;
			arrPortVerts[1] = vCenter + Vec3d(0,0,1.f)*fRadius - vBorder;
			arrPortVerts[2] = vCenter + Vec3d(0,0,1.f)*fRadius + vBorder;
			arrPortVerts[3] = vCenter - Vec3d(0,0,1.f)*fRadius + vBorder;
		}
		else if(!pParent && vPortNorm.z!=0 && m_lstConnections.Count()==1)
		{ // up/down entrance portal
			Vec3d vBorder = GetNormalized(vPortNorm.Cross(Vec3d(0,1,0.f)))*fRadius;
			arrPortVerts[0] = vCenter - Vec3d(0,1,0.f)*fRadius + vBorder;
			arrPortVerts[1] = vCenter + Vec3d(0,1,0.f)*fRadius + vBorder;
			arrPortVerts[2] = vCenter + Vec3d(0,1,0.f)*fRadius - vBorder;
			arrPortVerts[3] = vCenter - Vec3d(0,1,0.f)*fRadius - vBorder;
		}
		else
		{ // something wrong or areabox portal - use simple solution
			if(
					//vPortNorm == Vec3d(0,0,0)
					IsEquivalent(vPortNorm,Vec3d(0,0,0),VEC_EPSILON)
				)
				vPortNorm = GetNormalized((vCenter - GetViewCamera().GetPos()));

			Vec3d vBorder = GetNormalized(vPortNorm.Cross(Vec3d(0,0,1.f)))*fRadius;
			arrPortVerts[0] = vCenter - Vec3d(0,0,1.f)*fRadius - vBorder;
			arrPortVerts[1] = vCenter + Vec3d(0,0,1.f)*fRadius - vBorder;
			arrPortVerts[2] = vCenter + Vec3d(0,0,1.f)*fRadius + vBorder;
			arrPortVerts[3] = vCenter - Vec3d(0,0,1.f)*fRadius + vBorder;
		}

		if(GetCVars()->e_portals==4) // make color reqursion dependent
			GetRenderer()->SetMaterialColor(1,1,pObjManager->m_nRenderStackLevel==0,1);

    CCamera camParent = CurCamera;
		UpdatePortalCameraPlanes(CurCamera, arrPortVerts, vPortNorm.z==0);
    
    static list2<Vec3d> lstPortVertsClipped; // Timur, keep this list static so it is not reallocated every time.
		lstPortVertsClipped.Clear();
    lstPortVertsClipped.AddList(arrPortVerts, 4);
    ClipPortalVerticesByCameraFrustum(&lstPortVertsClipped, camParent);

    bScisorValid = UpdatePortalCameraScissor(CurCamera, &lstPortVertsClipped, vPortNorm.z==0);

		if(bScisorValid && barrPortVertsOtherSideValid)
		{
			Vec3d vOtherSizeBoxMax = SetMinBB();
			Vec3d vOtherSizeBoxMin = SetMaxBB();
			for(int i=0; i<4; i++)
			{
				vOtherSizeBoxMin.CheckMin(arrPortVertsOtherSide[i]-Vec3d(0.01f,0.01f,0.01f));
				vOtherSizeBoxMax.CheckMax(arrPortVertsOtherSide[i]+Vec3d(0.01f,0.01f,0.01f));
			}

			bScisorValid = CurCamera.IsAABBVisible_exact(AABB(vOtherSizeBoxMin,vOtherSizeBoxMax));
		}

		if(bScisorValid && pParent && m_lstConnections.Count()==1)
		{ // set this camera for outdoor
      if(nReqursionLevel>=1)
      {
        if(!m_bSkyOnly)
        {
          if(plstOutPortCameras)
			    {
				    plstOutPortCameras->Add(CurCamera);
				    plstOutPortCameras->Last().m_pPortal = this;
			    }
			    if(pbOutdoorVisible)
				    *pbOutdoorVisible = true;
        }
				else if(pbSkyVisible)
					*pbSkyVisible = true;
      }

			return;
		}
	}

	// sort portals by distance
  if(!bThisIsPortal && m_lstConnections.Count())
  {
    for(int p=0; p<m_lstConnections.Count(); p++)
    {
      CVisArea * pNeibVolume = m_lstConnections[p];
      pNeibVolume->m_fDistance = CurCamera.GetPos().GetDistance((pNeibVolume->m_vBoxMin+pNeibVolume->m_vBoxMax)*0.5f);
    }

    qsort(&m_lstConnections[0], m_lstConnections.Count(), 
      sizeof(m_lstConnections[0]), CVisAreaManager__CmpDistToPortal);
  }

	float fZoomFactor = 0.2f+0.8f*(RAD2DEG(CurCamera.GetFov())/90.f);      

	// recurse to connetions
	for(int p=0; p<m_lstConnections.Count(); p++)
	{
		CVisArea * pNeibVolume = m_lstConnections[p];
		if(pNeibVolume != pParent)
		{
			if(!bThisIsPortal)
			{ // skip far portals
				float fRadius = (pNeibVolume->m_vBoxMax-pNeibVolume->m_vBoxMin).Length()*0.5f;
				if(pNeibVolume->m_fDistance*fZoomFactor > fRadius*pNeibVolume->m_fViewDistRatio)
					continue;
			}

			if((bScisorValid || m_lstConnections.Count()==1) && (bThisIsPortal || CurCamera.IsAABBVisibleFast( AABB(pNeibVolume->m_vGeomBoxMin,pNeibVolume->m_vGeomBoxMax) )))
				pNeibVolume->DrawVolume(pObjManager, nReqursionLevel-1, CurCamera, this, pCurPortal, pbOutdoorVisible, plstOutPortCameras, pbSkyVisible);
			else
				pNeibVolume->DrawVolume_NotThisAreaOnlyLights(pObjManager, nReqursionLevel-1, CurCamera, this, pCurPortal, pbOutdoorVisible, plstOutPortCameras, pbSkyVisible);
		}
	}
}

//! return list of visareas connected to specified visarea (can return portals and sectors)
int CVisArea::GetRealConnections(IVisArea ** pAreas, int nMaxConnNum, bool bSkipDisabledPortals)
{
  int nOut = 0;
  for(int nArea=0; nArea<m_lstConnections.Count(); nArea++)
  {
    if(nOut<nMaxConnNum)
      pAreas[nOut] = (IVisArea*)m_lstConnections[nArea];
    nOut++;
  }
  return nOut;
}

//! return list of sectors conected to specified sector or portal (returns sectors only)
// todo: change the way it returns data
int CVisArea::GetVisAreaConnections(IVisArea ** pAreas, int nMaxConnNum, bool bSkipDisabledPortals)
{
	int nOut = 0;
	if(IsPortal())
	{
/*		for(int nArea=0; nArea<m_lstConnections.Count(); nArea++)
		{
			if(nOut<nMaxConnNum)
				pAreas[nOut] = (IVisArea*)m_lstConnections[nArea];
			nOut++;
		}*/
    return min(nMaxConnNum,GetRealConnections(pAreas, nMaxConnNum, bSkipDisabledPortals));
	}
	else
	{
		for(int nPort=0; nPort<m_lstConnections.Count(); nPort++)
		{
			CVisArea * pPortal = m_lstConnections[nPort];
			assert(pPortal->IsPortal());
			for(int nArea=0; nArea<pPortal->m_lstConnections.Count(); nArea++)
			{
				if(pPortal->m_lstConnections[nArea]!=this)
          if(!bSkipDisabledPortals || pPortal->IsActive())
				{
					if(nOut<nMaxConnNum)
						pAreas[nOut] = (IVisArea*)pPortal->m_lstConnections[nArea];
					nOut++;
					break; // take first valid connection
				}
			}
		}
	}

	return min(nMaxConnNum,nOut);
}

bool CVisArea::IsPortalValid()
{
	if(m_lstConnections.Count()>2 || m_lstConnections.Count()==0)
		return false;

	for(int i=0; i<m_lstConnections.Count(); i++)
	if(IsEquivalent(m_vConnNormals[i],Vec3d(0,0,0),VEC_EPSILON))
		return false;

  if(m_lstConnections.Count()>1)
    if( m_vConnNormals[0].Dot(m_vConnNormals[1])>-0.99f )
      return false;

	return true;
}

bool CVisArea::IsPortalIntersectAreaInValidWay(CVisArea * pPortal)
{
	const Vec3d & v1Min = pPortal->m_vBoxMin;
	const Vec3d & v1Max = pPortal->m_vBoxMax;
	const Vec3d & v2Min = m_vBoxMin;
	const Vec3d & v2Max = m_vBoxMax;

	if(v1Max.x>v2Min.x && v2Max.x>v1Min.x)
	if(v1Max.y>v2Min.y && v2Max.y>v1Min.y)
	if(v1Max.z>v2Min.z && v2Max.z>v1Min.z)
	{
		// vertical portal
		for(int v=0; v<m_lstShapePoints.Count(); v++)
		{
			int nIntersNum=0;
			bool arrIntResult[4] = { 0,0,0,0 };
			for(int p=0; p<pPortal->m_lstShapePoints.Count() && p<4; p++)
			{
				const Vec3d & v0 = m_lstShapePoints[v];
				const Vec3d & v1 = m_lstShapePoints[(v+1)%m_lstShapePoints.Count()];
				const Vec3d & p0 = pPortal->m_lstShapePoints[p];
				const Vec3d & p1 = pPortal->m_lstShapePoints[(p+1)%pPortal->m_lstShapePoints.Count()];

				if(Is2dLinesIntersect(v0.x,v0.y,v1.x,v1.y,p0.x,p0.y,p1.x,p1.y))
				{
					nIntersNum++;
					arrIntResult[p] = true;
				}
			}
			if(nIntersNum==2 && arrIntResult[0]==arrIntResult[2] && arrIntResult[1]==arrIntResult[3])
				return true;
		}

		// horisontal portal
		{
			int nBottomPoints=0, nUpPoints=0;
			for(int p=0; p<pPortal->m_lstShapePoints.Count() && p<4; p++)
				if(IsPointInsideVisArea(pPortal->m_lstShapePoints[p]))
					nBottomPoints++;

			for(int p=0; p<pPortal->m_lstShapePoints.Count() && p<4; p++)
				if(IsPointInsideVisArea(pPortal->m_lstShapePoints[p]+Vec3d(0,0,pPortal->m_fHeight)))
					nUpPoints++;

			if(nBottomPoints==0 && nUpPoints==4)
				return true;

			if(nBottomPoints==4 && nUpPoints==0)
				return true;
		}
	}

	return false;
}

bool CVisArea::IsPortal()
{
	bool bThisIsPortal = strstr(m_sName,"portal") != 0;
	return bThisIsPortal;
}
/*
void CVisArea::SetTreeId(int nTreeId)
{
	if(m_nTreeId == nTreeId)
		return;

	m_nTreeId = nTreeId;

	for(int p=0; p<m_lstConnections.Count(); p++)
		m_lstConnections[p]->SetTreeId(nTreeId);
}
*/
bool CVisArea::IsShapeClockwise()
{
	float fClockWise = 
		(m_lstShapePoints[0].x-m_lstShapePoints[1].x)*(m_lstShapePoints[2].y-m_lstShapePoints[1].y)-
		(m_lstShapePoints[0].y-m_lstShapePoints[1].y)*(m_lstShapePoints[2].x-m_lstShapePoints[1].x);        

	return fClockWise>0;
}

void CVisArea::DrawAreaBoundsIntoCBuffer(CCoverageBuffer * pCBuffer)
{
  if(m_lstShapePoints.Count()!=4)
    return;
 
  Vec3d arrVerts[8];
  int arrIndices[24];

  int v=0;
  int i=0;
  for(int p=0; p<4 && p<m_lstShapePoints.Count(); p++)
  {
    arrVerts[v++] = m_lstShapePoints[p];
    arrVerts[v++] = m_lstShapePoints[p] + Vec3d(0,0,m_fHeight);

    arrIndices[i++] = (p*2+0)%8;
    arrIndices[i++] = (p*2+1)%8;
    arrIndices[i++] = (p*2+2)%8;
    arrIndices[i++] = (p*2+3)%8;
    arrIndices[i++] = (p*2+2)%8;
    arrIndices[i++] = (p*2+1)%8;
  }

  Matrix44 mat;
  mat.SetIdentity();
  pCBuffer->AddMesh(arrVerts,8,arrIndices,24,&mat);
}

void CVisArea::ClipPortalVerticesByCameraFrustum(list2<Vec3d> * pPolygon, const CCamera & cam)
{
  CCoverageBuffer::ClipPolygon(pPolygon, *cam.GetFrustumPlane(FR_PLANE_RIGHT));
  CCoverageBuffer::ClipPolygon(pPolygon, *cam.GetFrustumPlane(FR_PLANE_LEFT));
  CCoverageBuffer::ClipPolygon(pPolygon, *cam.GetFrustumPlane(FR_PLANE_TOP));
  CCoverageBuffer::ClipPolygon(pPolygon, *cam.GetFrustumPlane(FR_PLANE_BOTTOM));
}

void CVisArea::GetMemoryUsage(ICrySizer*pSizer)
{
  pSizer->AddContainer(m_lstEntities[STATIC_ENTITIES]);
  pSizer->AddContainer(m_lstEntities[DYNAMIC_ENTITIES]);

  int nSize=0;
  for(int nStatic=0; nStatic<2; nStatic++)
  for(int i=0; i<m_lstEntities[nStatic].Count(); i++)
    nSize += m_lstEntities[nStatic][i]->GetMemoryUsage();

  pSizer->AddObject(this,sizeof(*this)+nSize);
}

void CVisArea::DrawVolume_NotThisAreaOnlyLights(CObjManager * pObjManager, int nReqursionLevel, CCamera CurCamera, CVisArea * pParent, CVisArea * pCurPortal, bool * pbOutdoorVisible, list2<CCamera> * plstOutPortCameras, bool * pbSkyVisible)
{
	if(!pParent)
		return;

	IRenderer * pRenderer = GetRenderer();

	// mark as rendered
	m_nRndFrameId = GetFrameID();

	// render lsources
	{
		for( int i=0; i<m_lstEntities[DYNAMIC_ENTITIES].Count(); i++ )
		{
			IEntityRender * pEntityRender = m_lstEntities[DYNAMIC_ENTITIES].GetAt(i);
			CDLight	* pLight = pEntityRender->GetLight();
			if(pLight && !(pLight->m_Flags & DLF_THIS_AREA_ONLY))
			{ // process light sources
				Vec3d vBoxMin,vBoxMax;
				pEntityRender->GetRenderBBox(vBoxMin,vBoxMax);
				if( AABB(pParent->m_vBoxMin,pParent->m_vBoxMax).IsIntersectBox( AABB(vBoxMin,vBoxMax)))
					pObjManager->RenderObject( pEntityRender, 0, 0, false, CurCamera, 0, 0, 0, true, 
					pEntityRender->GetMaxViewDist());
			}
			else if(pEntityRender->GetRndFlags() & ERF_DONOTCHECKVIS)
			{ // process 1p weapon
				pEntityRender->m_fWSMaxViewDist = pEntityRender->GetMaxViewDist();				
				uint nDLightMask = ((C3DEngine*)Get3DEngine())->GetFullLightMask();
				pObjManager->RenderObject( pEntityRender, m_nFogVolumeId, 
					nDLightMask, false, CurCamera, &m_vAmbColor, &m_vDynAmbColor, 0, true,
					pEntityRender->m_fWSMaxViewDist);
			}
		}
	}

	// todo: prepare flag once
	bool bThisIsPortal = strstr(m_sName,"portal") != 0;

	if(bThisIsPortal && nReqursionLevel>0)
	{ // recurse to not rendered connetions
		for(int p=0; p<m_lstConnections.Count(); p++)
		{
			CVisArea * pNeibVolume = m_lstConnections[p];
			if(pNeibVolume != pParent)
			{
				pNeibVolume->DrawVolume_NotThisAreaOnlyLights(pObjManager, nReqursionLevel-1, CurCamera, this, pCurPortal, pbOutdoorVisible, plstOutPortCameras, pbSkyVisible);
				break;
			}
		}
	}
}