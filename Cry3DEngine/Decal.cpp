////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   decals.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: draw, create decals on the world
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "DecalManager.h"
#include "3dengine.h"
#include "objman.h"

void CDecal::Process(bool & active, IRenderer * pIRenderer, const float fCurTime, C3DEngine * p3DEngine, IShader * pShader, CCamera* pCamera, float fSortOffset)
{
	// todo: take entity orientation into account
	m_nDynLMask = ((I3DEngine*)p3DEngine)->GetLightMaskFromPosition(m_pDecalOwner ? m_pDecalOwner->GetPos()+m_vPos : m_vPos, m_fSize);

	uint nDynLightMask = m_nDynLMask;

	// render StatObj decal
  if(m_pStatObj)
	{ 
		Render3DObject();
		return;
	}

	// Get decal alpha from life time
	float fLifeTime = m_fLifeEndTime - fCurTime;
  float fAlpha = fLifeTime*2;
  if(fAlpha > 1.f)
    fAlpha = 1.f;
	else if(fAlpha<0)
	{ // kill
    active=0;
		pIRenderer->DeleteLeafBuffer(m_pBigDecalLeafBuffer);
		m_pBigDecalLeafBuffer=0;
		return;
	}

	float fSizeK;
	if(m_fGrowTime)
		fSizeK = min(1.f, cry_sqrtf((fCurTime - m_fLifeBeginTime)/m_fGrowTime));
	else
		fSizeK = 1.f;

	// if there is owner - transform decal from owner space into world space
	if(m_pDecalOwner)
	{ 
		// if there is information about decal geometry - render complex decal and project texture on it
		if(m_pBigDecalLeafBuffer && m_pBigDecalLeafBuffer->m_Indices.m_nItems)
		{
			// setup transformation
			CCObject * pObj = pIRenderer->EF_GetObject(true);
			pObj->m_SortId = fSortOffset;

			Matrix44 objMat;
			if(!m_pDecalOwner->GetEntityStatObj(m_nDecalOwnerComponentId, &objMat))
			{
//				GetLog()->Log("Error: CDecal::Process: m_nDecalOwnerComponentId is out of range: %d", m_nDecalOwnerComponentId);
				assert(0);
				return;
			}

			pObj->m_Matrix = objMat;
      pObj->m_ObjFlags |= FOB_TRANS_MASK;

			p3DEngine->CheckDistancesToLightSources(nDynLightMask,m_pDecalOwner->GetPos(),m_pDecalOwner->GetRenderRadius(),m_pDecalOwner);

//			pIRenderer->DrawBall(objMat.GetTranslation(),1)	;

			pObj->m_DynLMMask = nDynLightMask;
			
			// somehow it's need's to be twice bigger to be same as simple decals
			float fSize2 = m_fSize*fSizeK*2.f;///m_pDecalOwner->GetScale();
			if(fSize2<0.05f)
				return;

			// todo: transform basis into world space ?

			// setup texgen
			// S component
			m_arrBigDecalCustomData[0] = m_vUp.x/fSize2;
			m_arrBigDecalCustomData[1] = m_vUp.y/fSize2;
			m_arrBigDecalCustomData[2] = m_vUp.z/fSize2;
																				
			float D0 = 
				m_arrBigDecalCustomData[0]*m_vPos.x + 
				m_arrBigDecalCustomData[1]*m_vPos.y + 
				m_arrBigDecalCustomData[2]*m_vPos.z;

			m_arrBigDecalCustomData[3] = -D0+0.5f;

			// T component
			m_arrBigDecalCustomData[4] = m_vRight.x/fSize2;
			m_arrBigDecalCustomData[5] = m_vRight.y/fSize2;
			m_arrBigDecalCustomData[6] = m_vRight.z/fSize2;

			float D1 = 
				m_arrBigDecalCustomData[4]*m_vPos.x + 
				m_arrBigDecalCustomData[5]*m_vPos.y + 
				m_arrBigDecalCustomData[6]*m_vPos.z;

			m_arrBigDecalCustomData[7] = -D1+0.5f;

			// pass attenuation info
			m_arrBigDecalCustomData[8] = m_vPos.x;
			m_arrBigDecalCustomData[9] = m_vPos.y;
			m_arrBigDecalCustomData[10]= m_vPos.z;
			m_arrBigDecalCustomData[11]= m_fSize;

			if(m_pDecalOwner->GetEntityRenderType() == eERType_Vegetation)
			{
				CObjManager * pObjManager = ((C3DEngine*)p3DEngine)->GetObjManager();
				CStatObjInst * pStatObjInst = (CStatObjInst *)m_pDecalOwner;
				CStatObj * pBody = pObjManager->m_lstStaticTypes[pStatObjInst->m_nObjectTypeID].GetStatObj();
				assert(pObjManager && pStatObjInst && pBody);

				if(pObjManager && pStatObjInst && pBody && pStatObjInst->m_fFinalBending)
				{
					pBody->SetupBending(pObj,pStatObjInst->m_fFinalBending);
				}
			}

			// draw complex decal using new indices and original object vertices
			m_pBigDecalLeafBuffer->SetRECustomData(m_arrBigDecalCustomData, 0, fAlpha);
			m_pBigDecalLeafBuffer->AddRenderElements(pObj);
		}
		else
		{ 
			// transform decal in software from owner space into world space and render as quad
			Matrix44 objMat;
			IStatObj * pEntObject = m_pDecalOwner->GetEntityStatObj(m_nDecalOwnerComponentId, &objMat);
			assert(pEntObject);

			Vec3d vPos	 = objMat.TransformPointOLD(m_vPos);
			Vec3d vRight = objMat.TransformVectorOLD(m_vRight*m_fSize/m_pDecalOwner->GetScale());
			Vec3d vUp    = objMat.TransformVectorOLD(m_vUp*m_fSize/m_pDecalOwner->GetScale());

			CDLight * pStrongestLightForTranspGeom = NULL;
			p3DEngine->CheckDistancesToLightSources(nDynLightMask,vPos,m_fSize,0,1,&pStrongestLightForTranspGeom,1);

			UCol uCol; 
			if(pStrongestLightForTranspGeom && pStrongestLightForTranspGeom->m_fRadius)
			{
				float fAtten = min(1.f,pStrongestLightForTranspGeom->m_fRadius/pStrongestLightForTranspGeom->m_Origin.GetDistance(m_vWSPos));
				float fDot = max(0,(pStrongestLightForTranspGeom->m_Origin-m_vWSPos).normalized().Dot(m_vFront));
				uCol.bcolor[0] = uchar(min(255.f,355.f*fDot*fAtten*pStrongestLightForTranspGeom->m_Color.r));
				uCol.bcolor[1] = uchar(min(255.f,355.f*fDot*fAtten*pStrongestLightForTranspGeom->m_Color.g));
				uCol.bcolor[2] = uchar(min(255.f,355.f*fDot*fAtten*pStrongestLightForTranspGeom->m_Color.b));
			}
			else
				uCol.dcolor = 0; 

			uCol.bcolor[3] = fastftol_positive(fAlpha*255);
			p3DEngine->GetObjManager()->AddPolygonToRenderer( m_nTexId, pShader, nDynLightMask, 
				vRight*fSizeK, vUp*fSizeK, uCol, ParticleBlendType_AlphaBased, m_vAmbient, vPos,
				0,0,0,0,fSortOffset,0,0,
				m_pDecalOwner->GetEntityRenderType() == eERType_Vegetation ? (CStatObjInst*)m_pDecalOwner : NULL);
		}
	}
	else if(!m_bOnTheGround)
	{	// draw small world space decal untransformed
		CDLight * pStrongestLightForTranspGeom = NULL;
		p3DEngine->CheckDistancesToLightSources(nDynLightMask,m_vPos,m_fSize,0,1,&pStrongestLightForTranspGeom,1);
		UCol uCol; 
		if(pStrongestLightForTranspGeom && pStrongestLightForTranspGeom->m_fRadius)
		{
			float fAtten = min(1.f,pStrongestLightForTranspGeom->m_fRadius/pStrongestLightForTranspGeom->m_Origin.GetDistance(m_vWSPos));
			float fDot = max(0,(pStrongestLightForTranspGeom->m_Origin-m_vWSPos).normalized().Dot(m_vFront));
			uCol.bcolor[0] = uchar(min(255.f,355.f*fDot*fAtten*pStrongestLightForTranspGeom->m_Color.r));
			uCol.bcolor[1] = uchar(min(255.f,355.f*fDot*fAtten*pStrongestLightForTranspGeom->m_Color.g));
			uCol.bcolor[2] = uchar(min(255.f,355.f*fDot*fAtten*pStrongestLightForTranspGeom->m_Color.b));
		}
		else
			uCol.dcolor = 0; 
		uCol.bcolor[3] = fastftol_positive(fAlpha*255);
		p3DEngine->GetObjManager()->AddPolygonToRenderer( m_nTexId, pShader, nDynLightMask, 
			m_vRight*m_fSize*fSizeK, m_vUp*m_fSize*fSizeK, uCol, ParticleBlendType_AlphaBased, m_vAmbient, m_vPos,
			0,0,0,0,fSortOffset);
	}

/*{
		Vec3d vPos = m_vPos + (m_pDecalOwner ? m_pDecalOwner->GetPos() : Vec3d(0,0,0));
		pIRenderer->Draw3dBBox(vPos,vPos+m_vFront);
	}*/

	// process life time and disable decal when needed
//  m_fLifeTime -= fFrameTime;
  if(m_fLifeEndTime<fCurTime)
	{
    active=0;
		pIRenderer->DeleteLeafBuffer(m_pBigDecalLeafBuffer);
		m_pBigDecalLeafBuffer=0;
	}
}

void CDecal::Render3DObject()
{
  // draw
  if(m_pStatObj)
  {
  	Vec3d vAngles = m_vFront;
    vAngles=ConvertVectorToCameraAngles(vAngles);
    vAngles.x+=90;

		//Matrix mat,mat1;
		//mat.Identity();
		//mat=GetRotationZYX44(-gf_DEGTORAD*vAngles)*mat; //NOTE: angles in radians and negated 
		//mat1.Identity();
		//mat1=GetTranslationMat(m_vPos)*mat1;
		//mat = mat * mat1;

		//OPTIMISED_BY_IVO  
		Matrix44 mat = Matrix34::CreateRotationXYZ( Deg2Rad(vAngles), m_vPos );
		mat	=	GetTransposed44(mat); //TODO: remove this after E3 and use Matrix34 instead of Matrix44

    SRendParams rParms;
    rParms.pMatrix = &mat;
    rParms.dwFObjFlags |= FOB_TRANS_MASK;
    m_pStatObj->Render(rParms,Vec3(zero),0);
  }
}

void CDecal::DrawBigDecalOnTerrain(C3DEngine * p3DEngine, IRenderer * pIRenderer, 	float fCurrTime)
{
	// Get decal alpha from life time
	float fLifeTime = m_fLifeEndTime - fCurrTime;
	float fAlpha = fLifeTime*2;
	if(fAlpha > 1.f)
		fAlpha = 1.f;
	else if(fAlpha<0)
		return;

	Vec3d vColor = p3DEngine->GetWorldColor();
	pIRenderer->SetMaterialColor(vColor.x,vColor.y,vColor.z,fAlpha);

  // calc area
  int x1=int(m_vPos.x-m_fSize*0.85)/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize();
  int y1=int(m_vPos.y-m_fSize*0.85)/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize();
  int x2=int(m_vPos.x+CTerrain::GetHeightMapUnitSize()+m_fSize*0.85)/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize();
  int y2=int(m_vPos.y+CTerrain::GetHeightMapUnitSize()+m_fSize*0.85)/CTerrain::GetHeightMapUnitSize()*CTerrain::GetHeightMapUnitSize();

	float fSizeK;
	if(m_fGrowTime)
		fSizeK = min(1.f, cry_sqrtf((fCurrTime - m_fLifeBeginTime)/m_fGrowTime));
	else
		fSizeK = 1.f;

	float fRadius = m_fSize*fSizeK*2.f;
	if(fRadius<0.05f)
		return;

  // limits
  if(x1<0) x1=0;
  if(y1<0) y1=0;
  if(x2>=CTerrain::GetTerrainSize()) x2=CTerrain::GetTerrainSize();
  if(y2>=CTerrain::GetTerrainSize()) y2=CTerrain::GetTerrainSize();

  // fill buffer and draw
  list2<struct_VERTEX_FORMAT_P3F_TEX2F> verts; 
  struct_VERTEX_FORMAT_P3F_TEX2F tmp;

	Vec3d vOffset(0,0,0);
	if(((C3DEngine*)p3DEngine)->GetObjManager())
		vOffset = (pIRenderer->GetCamera().GetPos()-m_vPos)*0.01f*((C3DEngine*)p3DEngine)->GetObjManager()->m_fZoomFactor;

  for(int x=x1; x<x2; x+=CTerrain::GetHeightMapUnitSize())
  {
    verts.Clear();
    for(int y=y1; y<=y2; y+=CTerrain::GetHeightMapUnitSize())
    {
      tmp.xyz.x = (float)x;
      tmp.xyz.y = (float)y;
      tmp.xyz.z = p3DEngine->GetTerrainZ(x,y)+0.07f;
      tmp.st[0] = (((float)x)-m_vPos.x)/fRadius+0.5f;
      tmp.st[1] = 1.f-((((float)y)-m_vPos.y)/fRadius+0.5f);

			tmp.xyz+=vOffset;
      verts.Add(tmp);

      tmp.xyz.x = (float)(x+CTerrain::GetHeightMapUnitSize());
      tmp.xyz.y = (float)y;
      tmp.xyz.z = p3DEngine->GetTerrainZ((x+CTerrain::GetHeightMapUnitSize()),y)+0.07f;
      tmp.st[0] = (((float)(x+CTerrain::GetHeightMapUnitSize()))-m_vPos.x)/fRadius+0.5f;
      tmp.st[1] = 1.f-((((float)y)-m_vPos.y)/fRadius+0.5f);

			tmp.xyz+=vOffset;
      verts.Add(tmp);
    }

		if(verts.Count())
		{
			pIRenderer->SetTexture(m_nTexId);
			pIRenderer->SetTexClampMode(true);
			pIRenderer->DrawTriStrip(&(CVertexBuffer (&verts[0].xyz.x,VERTEX_FORMAT_P3F_TEX2F)),verts.Count());
		}
  }
}
