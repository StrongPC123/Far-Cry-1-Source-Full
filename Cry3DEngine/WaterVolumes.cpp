////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   objmanWaterVolumes.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Loading water volumes, prepare water geometry
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "watervolumes.h"
#include "visareas.h"
#include "3dengine.h"

// Shader params ids.
enum {
	SHP_WATER_FLOW_POS
};

CWaterVolumeManager::CWaterVolumeManager( )
{
	// Add Water Flow Pos shader parameter.
	SShaderParam pr;
	pr.m_Type = eType_FLOAT;
	pr.m_Value.m_Float = 0;
	strcpy( pr.m_Name, "WaterFlowPos" );
	m_shaderParams.Reserve(1);
	m_shaderParams.AddElem(pr);
}

void CWaterVolumeManager::LoadWaterVolumesFromXML(XDOM::IXMLDOMDocumentPtr pDoc)
{
	// reset old data
	for(int i=0; i<m_lstWaterVolumes.Count(); i++)
	{
		m_lstWaterVolumes[i]->m_lstPoints.Reset();
		GetRenderer()->DeleteLeafBuffer(m_lstWaterVolumes[i]->m_pLeafBuffer);
		m_lstWaterVolumes[i]->m_pLeafBuffer=0;
		delete m_lstWaterVolumes[i];
	}
	m_lstWaterVolumes.Reset();

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
				XDOM::IXMLDOMNodePtr pType = pNode->getAttribute("Type");
				if (pType)
				{
					if (strstr(pType->getText(),"WaterVolume"))
					{
						CWaterVolume * pNewVolume = new CWaterVolume( GetRenderer() );
						m_lstWaterVolumes.Add(pNewVolume);
						m_lstWaterVolumes.Last()->m_vBoxMax=SetMinBB();
						m_lstWaterVolumes.Last()->m_vBoxMin=SetMaxBB();

						XDOM::IXMLDOMNodePtr pAttr0,pAttr1,pAttr2,pAttr3,pAttr4,pMatAttr;

						pAttr0 = pNode->getAttribute("Pos");
						pAttr1 = pNode->getAttribute("Width");
						pAttr2 = pNode->getAttribute("Height");
						pAttr3 = pNode->getAttribute("Name");
						pAttr4 = pNode->getAttribute("GroupId");
						pMatAttr = pNode->getAttribute("Material");

						if (pMatAttr)
						{
							IMatInfo *pMatInfo = GetSystem()->GetI3DEngine()->FindMaterial( pMatAttr->getText() );
							if (pMatInfo)
								pNewVolume->SetMaterial( pMatInfo );
						}
						
						// set shader
						XDOM::IXMLDOMNodePtr pAttr5 = pNode->getAttribute("WaterShader");
						if(pAttr5)
							pNewVolume->m_pShader = pAttr5->getText()[0] ? GetRenderer()->EF_LoadShader(pAttr5->getText(), eSH_World, EF_SYSTEM) : NULL;

						// set tesselation
						XDOM::IXMLDOMNodePtr pAttrTriMinSize = pNode->getAttribute("TriMinSize");
						XDOM::IXMLDOMNodePtr pAttrTriMaxSize = pNode->getAttribute("TriMaxSize");
						float fTriMaxSize = pAttrTriMaxSize ? (float)atof(pAttrTriMaxSize->getText()) : 8.f;
						if(pAttrTriMinSize)
							pNewVolume->SetTriSizeLimits((float)atof(pAttrTriMinSize->getText()), fTriMaxSize);

						// set flow speed
						XDOM::IXMLDOMNodePtr pAttr6 = pNode->getAttribute("WaterSpeed");
						if(pAttr6)
							pNewVolume->m_fFlowSpeed = (float)atof(pAttr6->getText());

						// set AffectToVolFog
						XDOM::IXMLDOMNodePtr pAffectToVolFog = pNode->getAttribute("AffectToVolFog");
						if(pAffectToVolFog)
							pNewVolume->m_bAffectToVolFog = atoi(pAffectToVolFog->getText())!=0;

						// load vertices
						if(pAttr0!=0 && pAttr1!=0 && pAttr2!=0 && pAttr3!=0 && pAttr4!=0)
						{
							pNewVolume->SetName(pAttr3->getText());

							pNewVolume->m_fHeight = (float)atof(pAttr2->getText());

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
											m_lstWaterVolumes.Last()->m_lstPoints.Add(vPos);
											m_lstWaterVolumes.Last()->m_vBoxMax.CheckMax(vPos);
											m_lstWaterVolumes.Last()->m_vBoxMin.CheckMin(vPos);
										}
									}

									if(	GetDistance(m_lstWaterVolumes.Last()->m_lstPoints.Last(), m_lstWaterVolumes.Last()->m_lstPoints[0])>0.1f )
										m_lstWaterVolumes.Last()->m_lstPoints.Add(Vec3d(m_lstWaterVolumes.Last()->m_lstPoints[0]));

									m_lstWaterVolumes.Last()->UpdateVisArea();
								}
							}
						}
					}
				}
			}
		}
	}
}

void MidVert(const struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F & v1, const struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F & v2, struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F & vRes)
{
  vRes.xyz = 0.5f*v1.xyz + 0.5f*v2.xyz;

  vRes.normal = 0.5f*v1.normal + 0.5f*v2.normal;

  vRes.color.bcolor[0] = uchar(0.5f*v1.color.bcolor[0] + 0.5f*v2.color.bcolor[0]);
  vRes.color.bcolor[1] = uchar(0.5f*v1.color.bcolor[1] + 0.5f*v2.color.bcolor[1]);
  vRes.color.bcolor[2] = uchar(0.5f*v1.color.bcolor[2] + 0.5f*v2.color.bcolor[2]);
  vRes.color.bcolor[3] = uchar(0.5f*v1.color.bcolor[3] + 0.5f*v2.color.bcolor[3]);

  vRes.st[0] = 0.5f*v1.st[0] + 0.5f*v2.st[0];
  vRes.st[1] = 0.5f*v1.st[1] + 0.5f*v2.st[1];
}

bool CWaterVolume::TesselateFace(list2<struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F> & lstVerts, list2<ushort> & lstIndices, int nFacePos, list2<Vec3d> & lstDirections)
{
  int n0 = lstIndices[nFacePos+0];
  int n1 = lstIndices[nFacePos+1];
  int n2 = lstIndices[nFacePos+2];

  struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F & v0 = lstVerts[n0];
  struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F & v1 = lstVerts[n1];
  struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F & v2 = lstVerts[n2];

  // get edge lengts
  float fDist01 = v0.xyz.GetDistance(v1.xyz);
  float fDist12 = v1.xyz.GetDistance(v2.xyz);
  float fDist20 = v2.xyz.GetDistance(v0.xyz);

  float fMaxDist = max(max(fDist01,fDist12), fDist20);
	float fCameraDist=200;
	
	if(fMaxDist == fDist01)
		fCameraDist = GetSquaredDistance(GetViewCamera().GetPos(), (v0.xyz+v1.xyz)*0.5f);
	else if(fMaxDist == fDist12)
		fCameraDist = GetSquaredDistance(GetViewCamera().GetPos(), (v1.xyz+v2.xyz)*0.5f);
	else if(fMaxDist == fDist20)
		fCameraDist = GetSquaredDistance(GetViewCamera().GetPos(), (v2.xyz+v0.xyz)*0.5f);
	else
		assert(0);

	if(fMaxDist<m_fTriMaxSize)
	{
		if(fMaxDist<fCameraDist/25 && m_fTriMinSize<8.f)
			return false;

		if(fMaxDist<m_fTriMinSize)
			return false;
	}

  // delete old face
//	lstIndices.Delete(nFacePos,3);
	lstIndices.DeleteFastUnsorted(nFacePos,3);

  int nNewIndex = lstVerts.Count();

  // tesselate longest
  if(fDist01>fDist12 && fDist01>fDist20)
  { // 01
		struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F v01;
    MidVert(v0, v1, v01);

    lstVerts.Add(v01);
    lstDirections.Add((lstDirections[n0]+lstDirections[n1]).GetNormalized());

    lstIndices.Add(n0);
    lstIndices.Add(nNewIndex);
    lstIndices.Add(n2);

    lstIndices.Add(n2);
    lstIndices.Add(nNewIndex);
    lstIndices.Add(n1);
  }
  else
  if(fDist12>fDist01 && fDist12>fDist20)
  { // 12
    struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F v12;
		MidVert(v1, v2, v12);

    lstVerts.Add(v12);
    lstDirections.Add((lstDirections[n1]+lstDirections[n2]).GetNormalized());

    lstIndices.Add(n1);
    lstIndices.Add(nNewIndex);
    lstIndices.Add(n0);

    lstIndices.Add(n0);
    lstIndices.Add(nNewIndex);
    lstIndices.Add(n2);
  }
  else
  if(fDist20>fDist12 && fDist20>fDist01)
  { // 20
    struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F v20;
		MidVert(v2, v0, v20);

    lstVerts.Add(v20);
    lstDirections.Add((lstDirections[n2]+lstDirections[n0]).GetNormalized());

    lstIndices.Add(n2);
    lstIndices.Add(nNewIndex);
    lstIndices.Add(n1);

    lstIndices.Add(n1);
    lstIndices.Add(nNewIndex);
    lstIndices.Add(n0);
  }

  return true;
}

void CWaterVolume::TesselateStrip(list2<struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F> & lstVerts, list2<ushort> & lstIndices, list2<Vec3d> & lstDirections)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	lstIndices.Clear();

  for(int i=0; i<lstVerts.Count()-2; i++)
  {
    if(i&1)
    {
      lstIndices.Add(i+2);
      lstIndices.Add(i+1);
      lstIndices.Add(i+0);
    }
    else
    {
      lstIndices.Add(i+0);
      lstIndices.Add(i+1);
      lstIndices.Add(i+2);
    }
  }
                    
  for(int i=0; i<lstIndices.Count() && i<10000; i+=3)
    if(TesselateFace(lstVerts, lstIndices, i, lstDirections))
      i-=3;
}

void CWaterVolumeManager::InitWaterVolumes()
{
  for(int i=0; i<m_lstWaterVolumes.Count(); i++)
    m_lstWaterVolumes[i]->CheckForUpdate(false);
}

void CWaterVolume::CheckForUpdate(bool bMakeLowestLod)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

	Vec3d vCenter = (m_vBoxMin + m_vBoxMax) * 0.5f;
	float fRadius = (m_vBoxMin - vCenter).Length();

	if( m_fTriMinSize != m_fPrevTriMinSize || m_fTriMaxSize != m_fPrevTriMaxSize ||
		((m_vCurrentCamPos.GetDistance(GetViewCamera().GetPos())>2 && (m_fTriMinSize<m_fTriMaxSize))))
	{ // force to rebuild water volume geometry
		m_vCurrentCamPos = GetViewCamera().GetPos();
		GetRenderer()->DeleteLeafBuffer(m_pLeafBuffer);
		m_pLeafBuffer=0;
	}

	if(!m_pLeafBuffer )
	if(	m_pShader )
	if(	m_lstPoints.Count() > 3 )
	if(	GetDistance(m_lstPoints.Last(),m_lstPoints[0]) < 1.f )
	{
		m_fPrevTriMaxSize = m_fTriMaxSize;
		m_fPrevTriMinSize = m_fTriMinSize;

		m_vCurrentCamPos = GetViewCamera().GetPos();
    // make verts strip list
		struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F tmp;
		list2<struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F> lstVertices; 
    m_lstDirections.Clear();

		Vec3d vNormal = (m_lstPoints[1]-m_lstPoints[0]).Cross(m_lstPoints[2]-m_lstPoints[0]);

		for(int p=0; p<m_lstPoints.Count()/2; p++)
		{
			int p2 = m_lstPoints.Count() - p - 2;

			if(p>=p2)
				break;

			// first
			tmp.xyz = m_lstPoints[p]+m_vWaterLevelOffset;
      
      tmp.normal = Vec3(0.f, 0.f, 1.f);

			tmp.st[0] = (float)p;
			tmp.st[1] = 0.f;
			tmp.color.dcolor = -1;

			lstVertices.Add(tmp);
			
      // calc water move direction in this point
      Vec3d vDir1 = (p+1>=p2) ? (m_lstPoints[p] - m_lstPoints[p-1]) : (m_lstPoints[p+1] - m_lstPoints[p]);
			m_lstDirections.Add(GetNormalized(vDir1));

			// second
			tmp.xyz = m_lstPoints[p2]+m_vWaterLevelOffset;
			tmp.st[0] = (float)p;
			tmp.st[1] = 1.f;

			lstVertices.Add(tmp);

      Vec3d vDir2 = (p+1>=p2) ? (m_lstPoints[p2] - m_lstPoints[p2+1]) : (m_lstPoints[p2-1] - m_lstPoints[p2]);
			m_lstDirections.Add(GetNormalized(vDir2));
		}

    // Tesselate to alllow nice reflections
		list2<ushort> lstIndices;
    TesselateStrip(lstVertices, lstIndices, m_lstDirections);

//      for(int i=0; i<lstVertices.Count(); i++)
//      GetRenderer()->DrawLabel(Vec3d(lstVertices[i].x,lstVertices[i].y,lstVertices[i].z),4,"%d", i);

		m_pLeafBuffer = GetRenderer()->CreateLeafBufferInitialized(
			lstVertices.GetElements(), lstVertices.Count(), VERTEX_FORMAT_P3F_N_COL4UB_TEX2F, 
			lstIndices.GetElements(), lstIndices.Count(), R_PRIMV_TRIANGLES,
			"WaterVolume", eBT_Static);

		m_pLeafBuffer->SetChunk(m_pShader,
			0,lstVertices.Count(), 0,lstIndices.Count());

		m_pLeafBuffer->m_pMats->Get(0)->m_vCenter = vCenter;
		m_pLeafBuffer->m_pMats->Get(0)->m_fRadius = fRadius;

		m_pLeafBuffer->m_vBoxMin = m_vBoxMin;
		m_pLeafBuffer->m_vBoxMax = m_vBoxMax;
	}
}

void CWaterVolumeManager::RenderWaterVolumes(bool bOutdoorVisible)
{
	if(!GetCVars()->e_water_volumes || m_nRenderStackLevel)
		return;

	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );
	for(int i=0; i<m_lstWaterVolumes.Count(); i++)
	{
		CWaterVolume * pWaterVolumes = m_lstWaterVolumes[i];

		// check vis
		if(!GetViewCamera().IsAABBVisibleFast( AABB(pWaterVolumes->m_vBoxMin,pWaterVolumes->m_vBoxMax)))
			continue;

    if(GetFrameID()%32==0) // hack
      pWaterVolumes->UpdateVisArea();

		if(m_lstWaterVolumes[i]->m_lstVisAreas.Count())
		{ // water in indoors
			if(!m_lstWaterVolumes[i]->IsWaterVolumeAreasVisible())
				continue; // water area not visible
		}
		else if(!bOutdoorVisible)
			continue; // water area (outdoor) not visible

    // create geometry if not ready
    m_lstWaterVolumes[i]->CheckForUpdate(false);

		if(!m_lstWaterVolumes[i]->m_pLeafBuffer)
			continue;

    m_lstWaterVolumes[i]->m_nLastRndFrame = GetFrameID();

		// draw debug
		if(GetCVars()->e_water_volumes==2)
		{
			int nPosStride=0;
			byte * pPos = (byte *)m_lstWaterVolumes[i]->m_pLeafBuffer->GetPosPtr(nPosStride);

			for(int p=0; p<m_lstWaterVolumes[i]->m_pLeafBuffer->m_SecVertCount; p++)
			{
				Vec3d vPos = *(Vec3d*)&pPos[p*nPosStride];
				GetRenderer()->SetMaterialColor(p==0, p!=0, 0.0f, 1);
				GetRenderer()->Draw3dBBox(vPos-Vec3d(0.2f,0.2f,0.2f), vPos+Vec3d(0.2f,0.2f,0.2f));
				GetRenderer()->DrawLabel(vPos,2,"%d",p);
				GetRenderer()->Draw3dBBox(vPos+Vec3d(0,0,0.1f), vPos+m_lstWaterVolumes[i]->m_lstDirections[p]+Vec3d(0,0,0.1f),DPRIM_LINE);
			}

			GetRenderer()->SetMaterialColor(1, 1, 0, 1);
			GetRenderer()->Draw3dBBox(pWaterVolumes->m_vBoxMin,pWaterVolumes->m_vBoxMax);
		}

		// draw volume geometry
		CCObject * pObject = GetRenderer()->EF_GetObject(true);

		// Assign water flow pos shader param to dynamic object.
		m_shaderParams[SHP_WATER_FLOW_POS].m_Value.m_Float = -m_lstWaterVolumes[i]->m_fFlowSpeed * GetCurTimeSec();
		pObject->m_ShaderParams = &m_shaderParams;

		pObject->m_Matrix.SetIdentity();
		
		// object should have some translation to simplify reflections processing in the renderer
		pObject->m_Matrix.SetTranslationOLD(Vec3d(0,0,0.025f));
		pObject->m_ObjFlags |= FOB_TRANS_TRANSLATE;

		uint nDynMask = 0;
		if(m_lstWaterVolumes[i]->m_lstVisAreas.Count())
		{
			nDynMask = (uint)-1;

			// remove sun
			for(int nId=0; nId<32; nId++) 
			{
				if(nDynMask & (1<<nId))
				{
					CDLight * pDLight = (CDLight*)GetRenderer()->EF_Query(EFQ_LightSource, nId);
					if(pDLight && pDLight->m_Flags & DLF_SUN)
					{
						nDynMask = nDynMask & ~(1<<nId);
						break;
					}
				}
			}

			float fRadius = (m_lstWaterVolumes[i]->m_vBoxMax-m_lstWaterVolumes[i]->m_vBoxMin).len()*0.5f;
			Vec3d vCenter = (m_lstWaterVolumes[i]->m_vBoxMin+m_lstWaterVolumes[i]->m_vBoxMax)*0.5f;
			((C3DEngine*)Get3DEngine())->CheckDistancesToLightSources(nDynMask,vCenter,fRadius);
		}
		else
			nDynMask=0;

    pObject->m_Color.a = 0.99f;
		m_lstWaterVolumes[i]->m_pLeafBuffer->AddRenderElements(pObject, nDynMask, -1, 0, 
      (GetViewCamera().GetPos().z>Get3DEngine()->GetWaterLevel()) ? eS_SeeThrough : eS_Water, 
      m_lstWaterVolumes[i]->m_pMaterial);

    m_lstWaterVolumes[i]->m_pLeafBuffer->m_vBoxMax = m_lstWaterVolumes[i]->m_pLeafBuffer->m_vBoxMin = GetViewCamera().GetPos();
	}

  // release old
	for(int i=0; i<m_lstWaterVolumes.Count(); i++)
	{
		CWaterVolume * pWaterVolumes = m_lstWaterVolumes[i];
    if(pWaterVolumes->m_pLeafBuffer && m_lstWaterVolumes[i]->m_nLastRndFrame < GetFrameID()-100)
    {
      m_lstWaterVolumes[i]->CheckForUpdate(true);
	//	  GetRenderer()->DeleteLeafBuffer(pWaterVolumes->m_pLeafBuffer);
	  //  pWaterVolumes->m_pLeafBuffer=0;
    }
  }
}

float CWaterVolumeManager::GetWaterVolumeLevelFor2DPoint(const Vec3d & vPos, Vec3d * pvFlowDir)
{
	float fResLevel = WATER_LEVEL_UNKNOWN;

	// check all volumes bboxes
	for(int i=0; i<m_lstWaterVolumes.Count(); i++)
	{ 
		CWaterVolume * pWaterVolumes = m_lstWaterVolumes[i];
		Vec3d vBoxMin2d = pWaterVolumes->m_vBoxMin;
		Vec3d vBoxMax2d = pWaterVolumes->m_vBoxMax;
		vBoxMin2d.z=WATER_LEVEL_UNKNOWN;
		vBoxMax2d.z=1024;

		if(!pWaterVolumes->m_pLeafBuffer)
			pWaterVolumes->CheckForUpdate(true);

		if(pWaterVolumes->m_pLeafBuffer && Overlap::Point_AABB(vPos,vBoxMin2d,vBoxMax2d))
		{ // if inside bbox
			int nInds = 0;
      ushort *pInds = pWaterVolumes->m_pLeafBuffer->GetIndices(&nInds);
			int nPosStride=0;
			const byte * pPos = pWaterVolumes->m_pLeafBuffer->GetPosPtr(nPosStride,0,true);
			for(int i=0; (i+2)<nInds; i+=3)
			{	// test all triangles of water surface strip
				Vec3d v0 = *(Vec3d*)&pPos[nPosStride*pInds[i+0]];
				Vec3d v1 = *(Vec3d*)&pPos[nPosStride*pInds[i+1]];
				Vec3d v2 = *(Vec3d*)&pPos[nPosStride*pInds[i+2]];
				v0.z = v1.z = v2.z = 0; // make triangle 2d
				Vec3d vPos2d(vPos.x,vPos.y,0);
				if(Overlap::PointInTriangle( vPos2d, v0,v1,v2,Vec3d(0,0,1.f)))
				{ // triangle found
					Plane plane;
					plane.CalcPlane( // calc plane using real vertices
						*(Vec3d*)&pPos[nPosStride*pInds[i+0]],
						*(Vec3d*)&pPos[nPosStride*pInds[i+1]],
						*(Vec3d*)&pPos[nPosStride*pInds[i+2]]);
					float fDist = plane.DistFromPlane(vPos);
					float fDot = plane.n.Dot(Vec3d(0,0,1.f));
					if(fDot>0) 
						fDist = -fDist;

					// check bottom bounds
					if(pWaterVolumes->m_fHeight && fDist > -pWaterVolumes->m_fHeight)
						continue;
						
					if(GetCVars()->e_water_volumes==2)
					{
						GetLog()->Log("CameraLevel=%.2f, WaterVolumeLevel=%.2f %d", 
							GetViewCamera().GetPos().z, fDist, GetFrameID());
						Vec3d vPos = vPos2d+Vec3d(0,0,fDist);
//						GetRenderer()->Draw3dBBox(vPos-Vec3d(0.05f,0.05f,0.05f),vPos+Vec3d(0.05f,0.05f,0.05f),DPRIM_SOLID_SPHERE);

					}

					if(pvFlowDir)
					{
						// todo: calculate the barycentric coordinates of the triangle
/*						float b0 =  (v1.x - v0.x) * (v2.y - v0.y) - (v2.x - v0.x) * (v1.y - v0.y);
						float b1 = ( v1.x * v2.y - v2.x * v1.y ) / b0 ;
						float b2 = ( v2.x * v0.y - v0.x * v2.y ) / b0 ;
						float b3 = ( v0.x * v1.y - v1.x * v0.y ) / b0 ;
						Vec3d vReconstructed = b1 * v0 + b2 * v1 + b3 * v2;
						vReconstructed=vReconstructed;*/

						float fDist0 = GetDistance(vPos2d,v0);
						float fDist1 = GetDistance(vPos2d,v1);
						float fDist2 = GetDistance(vPos2d,v2);
						float fSumm = fDist0 + fDist1 + fDist2;
						fDist0 /= fSumm;
						fDist1 /= fSumm;
						fDist2 /= fSumm;
						
						*pvFlowDir =	pWaterVolumes->m_lstDirections[pInds[i+0]]*(1.f-fDist0)+
													pWaterVolumes->m_lstDirections[pInds[i+1]]*(1.f-fDist1)+
													pWaterVolumes->m_lstDirections[pInds[i+2]]*(1.f-fDist2);

						if(GetCVars()->e_water_volumes==2)
							GetRenderer()->Draw3dBBox(vPos, vPos+*pvFlowDir, DPRIM_LINE);

            *pvFlowDir *= pWaterVolumes->m_fFlowSpeed;
					}

					if( (vPos.z+fDist) > fResLevel )
						fResLevel = (vPos.z+fDist);
				}
			}
		}
	}

	return fResLevel;
}

IWaterVolume * CWaterVolumeManager::CreateWaterVolume()
{
	CWaterVolume * pNewVolume = new CWaterVolume( GetRenderer() );
	m_lstWaterVolumes.Add(pNewVolume);
	return pNewVolume;
}

void CWaterVolumeManager::DeleteWaterVolume(IWaterVolume * pWaterVolume)
{
	GetRenderer()->DeleteLeafBuffer(((CWaterVolume*)pWaterVolume)->m_pLeafBuffer);
	((CWaterVolume*)pWaterVolume)->m_pLeafBuffer=0;
	delete pWaterVolume;
	m_lstWaterVolumes.Delete((CWaterVolume*)pWaterVolume);
}

void CWaterVolumeManager::UpdateWaterVolumeVisAreas()
{
	for(int i=0; i<m_lstWaterVolumes.Count(); i++)
		m_lstWaterVolumes[i]->UpdateVisArea();
}

void CWaterVolume::UpdatePoints(const Vec3d * pPoints, int nCount, float fHeight)
{
	m_lstPoints.PreAllocate(nCount+1,nCount);

	m_fHeight = fHeight;
	
	if(nCount)
	{
		memcpy(&m_lstPoints[0], pPoints, sizeof(Vec3d)*nCount);
		if(	GetDistance(m_lstPoints.Last(), m_lstPoints[0])>0.1f )
			m_lstPoints.Add(m_lstPoints[0]); // loop
	}

	// update bbox
	m_vBoxMax = SetMinBB();
	m_vBoxMin = SetMaxBB();

	for(int i=0; i<nCount; i++)
	{
		m_vBoxMax.CheckMax(pPoints[i]);
		m_vBoxMin.CheckMin(pPoints[i]);
	}

	// remake leaf buffer
	m_pRenderer->DeleteLeafBuffer(m_pLeafBuffer);
	m_pLeafBuffer=0;

	UpdateVisArea();
}

void CWaterVolume::SetShader(const char * szShaderName)
{
	if(szShaderName[0])
		m_pShader = m_pRenderer->EF_LoadShader(szShaderName,eSH_World, EF_SYSTEM);
	else
		m_pShader = NULL;//m_pRenderer->EF_LoadShader("default",eSH_World, EF_SYSTEM);

	// remake leaf buffer
	m_pRenderer->DeleteLeafBuffer(m_pLeafBuffer);
	m_pLeafBuffer=0;
}

CWaterVolumeManager::~CWaterVolumeManager()
{
	for(int i=0; i<m_lstWaterVolumes.Count(); i++)
	{ 
		CWaterVolume * pWaterVolume = m_lstWaterVolumes[i];
		GetRenderer()->DeleteLeafBuffer(pWaterVolume->m_pLeafBuffer);
		pWaterVolume->m_pLeafBuffer=0;
		delete pWaterVolume;
	}
  m_lstWaterVolumes.Reset();
}

void CWaterVolume::UpdateVisArea()
{
	m_lstVisAreas.Clear();

	if(!m_lstPoints.Count())
		return;
	
	// scan water volume bbox for vis areas
	const float dx = (m_vBoxMax.x - m_vBoxMin.x)/int(m_vBoxMax.x - m_vBoxMin.x) + 0.1f;
	const float dy = (m_vBoxMax.y - m_vBoxMin.y)/int(m_vBoxMax.y - m_vBoxMin.y) + 0.1f;
//	const float dz = (m_vBoxMax.z - m_vBoxMin.z)/int(m_vBoxMax.z - m_vBoxMin.z) + 0.1f;

	float z = (m_vBoxMin.z+m_vBoxMax.z)*0.5f;
	for(float x = m_vBoxMin.x; x<=m_vBoxMax.x; x+=dx*5)
		for(float y = m_vBoxMin.y; y<=m_vBoxMax.y; y+=dy*5)
//			for(float z = m_vBoxMin.z; z<=m_vBoxMax.z; z+=dz)
	{
		CVisArea * pVisArea = (CVisArea *)Get3DEngine()->GetVisAreaFromPos(Vec3d(x,y,z));
		if(pVisArea && m_lstVisAreas.Find(pVisArea)<0)
		{
			m_lstVisAreas.Add(pVisArea);
			if(m_bAffectToVolFog)
				UpdateVisAreaFogVolumeLevel(pVisArea);
		}
	}
}

void CWaterVolume::UpdateVisAreaFogVolumeLevel(CVisArea*pVisArea)
{
	CTerrain * pTerrain = ((C3DEngine*)Get3DEngine())->GetTerrain();
	int f;
	for(f=0; f<pTerrain->m_lstFogVolumes.Count(); f++)
	if(pTerrain->m_lstFogVolumes[f].nRendererVolumeID == pVisArea->m_nFogVolumeId)
	break;

	if(f<pTerrain->m_lstFogVolumes.Count())
	{
		VolumeInfo * pFogVolume = &pTerrain->m_lstFogVolumes[f];
		pFogVolume->vBoxMax.z = m_vBoxMax.z;
		GetRenderer()->EF_RegisterFogVolume(pFogVolume->fMaxViewDist,pFogVolume->vBoxMax.z,pFogVolume->vColor,pFogVolume->nRendererVolumeID, pFogVolume->m_bCaustics);
	}
}

//////////////////////////////////////////////////////////////////////////
void CWaterVolume::SetMaterial( IMatInfo *pMatInfo )
{
	m_pMaterial = pMatInfo;

	if(m_pMaterial)
		m_pMaterial->SetFlags(m_pMaterial->GetFlags()|MIF_WASUSED);
}

IMatInfo * CWaterVolume::GetMaterial()
{
  return m_pMaterial;
}

//////////////////////////////////////////////////////////////////////////
IWaterVolume * CWaterVolumeManager::FindWaterVolumeByName(const char * szName)
{
	for(int i=0; i<m_lstWaterVolumes.Count(); i++)
	{ 
		CWaterVolume * pWaterVolume = m_lstWaterVolumes[i];
		if(!stricmp(pWaterVolume->m_szName, szName))
			return pWaterVolume;
	}

	return 0;
}

void CWaterVolume::SetPositionOffset(const Vec3d & vNewOffset)
{
	m_vWaterLevelOffset = vNewOffset;

	m_vBoxMax = SetMinBB();
	m_vBoxMin = SetMaxBB();

	for(int p=0; p<m_lstPoints.Count(); p++)
	{
		Vec3d vPos = m_lstPoints[p] + m_vWaterLevelOffset;
		m_vBoxMax.CheckMax(vPos);
		m_vBoxMin.CheckMin(vPos);
	}

	m_vBoxMax.z += 0.01f;
	m_vBoxMin.z -= 0.01f;

	GetRenderer()->DeleteLeafBuffer(m_pLeafBuffer);
	m_pLeafBuffer=0;

	UpdateVisArea();
}

bool CWaterVolume::IsWaterVolumeAreasVisible()
{
	assert(m_lstVisAreas.Count());

	// water in indoors
	int v;
	for(v=0; v<m_lstVisAreas.Count(); v++)
		if(abs(((CVisArea*)m_lstVisAreas[v])->m_nRndFrameId - GetFrameID())<2)
			break; // water area is visible
	
	return v<m_lstVisAreas.Count();
}

void CWaterVolume::SetTriSizeLimits(float fTriMinSize, float fTriMaxSize)
{ 
	m_fTriMinSize = max(0.25f,min(fTriMinSize, 8));
	m_fTriMaxSize = max(0.25f,min(fTriMaxSize, 8));
	// remake leaf buffer
	m_pRenderer->DeleteLeafBuffer(m_pLeafBuffer);
	m_pLeafBuffer=0;
}