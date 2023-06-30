////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjmanfar.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: draw far objects as sprites
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "objman.h"
#include "3dengine.h"

void CObjManager::RenderFarObjects()
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );

  if (m_REFarTreeSprites && GetCVars()->e_vegetation_sprites && m_lstFarObjects[m_nRenderStackLevel].Count())
  {
    CCObject * pObj = GetRenderer()->EF_GetObject(true, -1);
    pObj->m_Matrix.SetIdentity();
    GetRenderer()->EF_AddEf(0, m_REFarTreeSprites, m_p3DEngine->m_pSHFarTreeSprites, NULL, pObj, 0, NULL, 
      /*(GetViewCamera().GetPos().z<m_pTerrain->GetWaterLevel()) ? */eS_Trees /*: eS_Sprites*/
      );
  }
}
/*
static _inline int Compare(CStatObjInst *& p1, CStatObjInst *& p2)
{
  if(p1->m_fDistance > p2->m_fDistance)
    return 1;
  else
  if(p1->m_fDistance < p2->m_fDistance)
    return -1;
  
  return 0;
} */

void CObjManager::DrawFarObjects(float fMaxViewDist)
{
  if(!GetCVars()->e_vegetation_sprites)
    return;

//////////////////////////////////////////////////////////////////////////////////////
//  Draw all far
//////////////////////////////////////////////////////////////////////////////////////

  list2<CStatObjInst*> * pList = &m_lstFarObjects[m_nRenderStackLevel];
 
  if (pList->Count())
  {
    IRenderer * pRenderer = GetRenderer();

    int useBending;
    if (/*(GetRenderer()->GetFeatures() & RFT_HW_VS) &&*/ GetCVars()->e_vegetation_bending >= 2)
      useBending = 2;
    else
    if (GetCVars()->e_vegetation_bending == 1)
      useBending = 1;
    else
      useBending = 0;

    if(useBending == 2)
      GetRenderer()->DrawObjSprites(pList, fMaxViewDist, this);
    else
      DrawObjSpritesSorted(pList, fMaxViewDist, useBending);    
  }

  for(int s=0; s<m_lstDebugEntityList.Count(); s++)
  {
    IRenderer * pRenderer = GetRenderer();
    pRenderer->ResetToDefault();
    ShadowMapLightSource * & pLsource = m_lstDebugEntityList[s]->GetEntityRS()->pShadowMapInfo->pShadowMapFrustumContainer;
    
    GetRenderer()->PushMatrix();
    GetRenderer()->TranslateMatrix(m_lstDebugEntityList[s]->GetPos());
    pLsource->m_LightFrustums.Get(0)->DrawFrustum(pRenderer,m_lstDebugEntityList[s]->GetPos(),1.f);
    GetRenderer()->PopMatrix();

    Vec3d vMin,vMax;
    m_lstDebugEntityList[s]->GetRenderBBox(vMin,vMax);
    pRenderer->Draw3dBBox(vMin,vMax);

    pRenderer->DrawBall(m_lstDebugEntityList[s]->GetPos(),0.1f);

    pRenderer->ResetToDefault();
  }

  m_lstDebugEntityList.Clear();
}
  
void CObjManager::DrawObjSpritesSorted(list2<CStatObjInst*> *pList, float fMaxViewDist, int useBending)
{
#if 0
  static list2<CStatObjInst*>* arrSortedInstances[8192];
  { // fill hash table
    const Vec3d & vCamPos = GetViewCamera().GetPos();
    const float rad2deg = 180.0f/gf_PI;    
    const float far_tex_angle = (FAR_TEX_ANGLE>>1);

    for( int i=0; i<pList->Count(); i++ )
    { 
      CStatObjInst * o = pList->GetAt(i);
      CStatObj * pBody = m_lstStaticTypes[o->m_nObjectTypeID].GetStatObj();
      
      const float DX = o->m_vPos.x - vCamPos.x;
      const float DY = o->m_vPos.y - vCamPos.y;
      int angle(int(rad2deg*atan2( DX, DY )+far_tex_angle));

      while(angle<0) angle+=360;

      assert(angle>=0 && angle/FAR_TEX_ANGLE<FAR_TEX_COUNT);
      
      int tid = pBody->m_arrSpriteTexID[angle/FAR_TEX_ANGLE];

      if(tid>=4096 && tid<8192)
      {
        if(!arrSortedInstances[tid])
            arrSortedInstances[tid] = new list2<CStatObjInst*>;

        arrSortedInstances[tid]->Add(o);
      }
      else
      {
#if !defined(LINUX)
        Warning( 0,0,"Error: CObjManager::DrawObjSpritesSorted: Texture id is out of range: %d", tid);
#endif
        break;
      }
    }  
  }

  pList->Clear();

  // render sorted by texture
  IRenderer * pRenderer = GetRenderer();
  pRenderer->ResetToDefault();
  pRenderer->EnableBlend(false);
  pRenderer->EnableAlphaTest(true,0.5f);
  pRenderer->SetEnviMode(R_MODE_MODULATE);
  pRenderer->EnableDepthWrites(true);  
  pRenderer->SetCullMode(R_CULL_DISABLE);

	Vec3d vWorldCol = GetSystem()->GetI3DEngine()->GetWorldColor();
	pRenderer->SetMaterialColor(vWorldCol.x, vWorldCol.y, vWorldCol.z, 1.f);

  float max_view_dist = fMaxViewDist;//*0.8f;
  const Vec3d & vCamPos = GetViewCamera().GetPos();
  const float rad2deg = 180.0f/gf_PI;    
  const float far_tex_angle = (FAR_TEX_ANGLE>>1);

	static int nCurrBufId = 0;
//	nCurrBufId=0;

  for(int tid=4096; tid<8192; tid++)
  if(arrSortedInstances[tid] && arrSortedInstances[tid]->Count())
  {
#define MAX_BUFF_NUM 128
#define MAX_VERTS_NUM 1024

		static CVertexBuffer * arrVideoBuffers[MAX_BUFF_NUM];

		if(!arrVideoBuffers[0])
		for(int i=0; i<MAX_BUFF_NUM; i++)
			arrVideoBuffers[i] = pRenderer->CreateBuffer(MAX_VERTS_NUM,VERTEX_FORMAT_P3F_TEX2F,"CompiledTreeSprites",true);

		static SVertexStream Inds;
    if (!Inds.m_VData)
      pRenderer->CreateIndexBuffer(&Inds, NULL, MAX_VERTS_NUM*3/2);

    // Lock the index buffer
    pRenderer->UpdateIndexBuffer(&Inds, NULL, 0, false);
    ushort *pInds = (ushort *)Inds.m_VData;

		pRenderer->UpdateBuffer(arrVideoBuffers[nCurrBufId],0,0,true);

    for(int i=0; i<arrSortedInstances[tid]->Count(); i++)
    { 
      CStatObjInst * o = arrSortedInstances[tid]->GetAt(i);
      CStatObj * pBody = m_lstStaticTypes[o->m_nObjectTypeID].GetStatObj();
      
      float max_dist = o->m_fMaxDist;
      
      // note: move into sort by size
      if(max_dist>max_view_dist)
        max_dist=max_view_dist;

      const float alpha = min(1.f,(1.f-(o->m_fDistance*m_fZoomFactor)/(max_dist))*8.f);
      const float brigh = /*0.6666f + 0.3333f*o->m_bBright*/CHAR_TO_FLOAT*o->m_ucBright;

      const float DX = o->m_vPos.x - vCamPos.x;
      const float DY = o->m_vPos.y - vCamPos.y;

//      pRenderer->SetMaterialColor(vWorldColor.x*brigh, vWorldColor.y*brigh, vWorldColor.z*brigh, alpha);

      const float fSpriteScaleV = o->m_fScale*pBody->GetRadiusVert()*1.035f*alpha;
      const float fSpriteScaleH = o->m_fScale*pBody->GetRadiusHors()*m_fZoomFactor*1.050f*alpha;
      Vec3d vPos = o->m_vPos + pBody->GetCenter()*o->m_fScale;
/*
      float fBending;
      if (useBending)
        fBending = (o->m_fCurrentBending)*pBody->m_fBending;
      else
        fBending = 0;
	*/
			struct_VERTEX_FORMAT_P3F_TEX2F * pVerts = (struct_VERTEX_FORMAT_P3F_TEX2F*)(arrVideoBuffers[nCurrBufId])->m_VS[VSF_GENERAL].m_VData;

			assert(i*6 + 5 < (MAX_VERTS_NUM*3/2));
			assert(i*4 + 3 < MAX_VERTS_NUM);

			struct_VERTEX_FORMAT_P3F_TEX2F VertQuad[4];

      float dy = DX*fSpriteScaleH/o->m_fDistance;
      float dx = DY*fSpriteScaleH/o->m_fDistance;
      float dz = fSpriteScaleV;

			VertQuad[0].s = -1; 
			VertQuad[0].t =	0; 
			VertQuad[0].x = -dx+vPos.x; VertQuad[0].y =  dy+vPos.y; VertQuad[0].z = -dz+vPos.z;
			VertQuad[1].s = 0; 
			VertQuad[1].t = 0; 
			VertQuad[1].x =  dx+vPos.x; VertQuad[1].y = -dy+vPos.y; VertQuad[1].z = -dz+vPos.z;
			VertQuad[2].s = 0; 
			VertQuad[2].t = 1; 
			VertQuad[2].x =  dx+vPos.x; VertQuad[2].y = -dy+vPos.y; VertQuad[2].z =  dz+vPos.z;  
			VertQuad[3].s = -1; 
			VertQuad[3].t = 1; 
			VertQuad[3].x = -dx+vPos.x; VertQuad[3].y =  dy+vPos.y; VertQuad[3].z =  dz+vPos.z;

			memcpy(&pVerts[i*4], VertQuad, sizeof(VertQuad));
			pInds[i*6 + 0] = i*4 + 0;
			pInds[i*6 + 1] = i*4 + 1;
			pInds[i*6 + 2] = i*4 + 2;

			pInds[i*6 + 3] = i*4 + 0;
			pInds[i*6 + 4] = i*4 + 2;
			pInds[i*6 + 5] = i*4 + 3;
    }  

    // Unlock the index buffer
    pRenderer->UpdateIndexBuffer(&Inds, NULL, 0, true);
	  pRenderer->SetTexture(tid);
		pRenderer->DrawBuffer(arrVideoBuffers[nCurrBufId],&Inds,arrSortedInstances[tid]->Count()*6,0,R_PRIMV_TRIANGLES);

		nCurrBufId++;
		if(nCurrBufId>=MAX_BUFF_NUM)
			nCurrBufId=0;

    arrSortedInstances[tid]->Clear();
  }
#endif
}