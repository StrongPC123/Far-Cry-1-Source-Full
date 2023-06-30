////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brush.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <ILMSerializationManager.h>
#include "StatObj.h"
#include "objman.h"
#include "visareas.h"
#include "terrain_sector.h"
#include "cbuffer.h"
#include "3DEngine.h"
#include "meshidx.h"
#include "watervolumes.h"
#include "LMCompStructures.h"
#include "brush.h"

//////////////////////////////////////////////////////////////////////////
// Brush Export structures.
//////////////////////////////////////////////////////////////////////////
#define BRUSH_FILE_TYPE 1
#define BRUSH_FILE_VERSION 3
#define BRUSH_FILE_SIGNATURE "CRY"

list2<IStatObj*> CBrush::m_lstBrushTypes;
list2<SExportedBrushMaterial> CBrush::m_lstSExportedBrushMaterials;

#pragma pack(push,1)
struct SExportedBrushHeader
{
	char signature[3];	// File signature.
	int filetype;				// File type.
	int	version;				// File version.
};

struct SExportedBrushGeom
{
  enum EFlags
  {
    SUPPORT_LIGHTMAP = 0x01,
    NO_PHYSICS = 0x02,
  };
  int size; // Size of this sructure.
  char filename[128];
  int flags; //! @see EFlags
  Vec3 m_minBBox;
  Vec3 m_maxBBox;
};

struct SExportedBrush
{
  int size; // Size of this sructure.
  int id;
  int geometry;
  int material;
  int flags;
  int mergeId;
  Matrix34 matrix;
  uchar ratioLod;
  uchar ratioViewDist;
  uchar reserved1;
  uchar reserved2;
};

#pragma pack(pop)
//////////////////////////////////////////////////////////////////////////

const char * CBrush::GetEntityClassName() const
{
	return "Brush";
}

const Vec3d & CBrush::GetPos(bool bWorldOnly) const
{
	assert(bWorldOnly);
	return m_vPos;
}

const Vec3d & CBrush::GetAngles(int realA) const
{
	assert(0);
	return m_vAngles;
}

float CBrush::GetScale() const
{
	return 1.f;
//	assert(0);
	//return m_fScale;
}

const char *CBrush::GetName() const
{
	return m_nObjectTypeID>=0 ? m_lstBrushTypes[m_nObjectTypeID]->GetFileName() : "StatObjNotSet";
}

void	CBrush::GetRenderBBox( Vec3d &mins,Vec3d &maxs )
{
	mins = m_vWSBoxMin;
	maxs = m_vWSBoxMax;
}

float CBrush::GetRenderRadius() const
{
	return m_fWSRadius;
}

bool CBrush::HasChanged()
{
	return false;
}

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4311 )
#endif

bool CBrush::DrawEntity(const struct SRendParams & _EntDrawParams)
{
	FUNCTION_PROFILER_FAST( GetSystem(),PROFILE_3DENGINE,m_bProfilerEnabled );

  assert(m_nObjectTypeID>=0 && m_nObjectTypeID<m_lstBrushTypes.Count());

	if(m_dwRndFlags & ERF_HIDDEN)
		return false;
  if (GetCVars()->e_brushes_onlymerged && !(m_dwRndFlags & ERF_MERGED_NEW))
    return false;
  //Matrix44 mat;
  //mat.SetIdentity();
  //IStatObj * pStatObj = GetEntityStatObj(0,&mat);
  //if (strcmp(pStatObj->GetFileName(), ""))
  //  return false;

//  if(!m_pStatObj || !((CStatObj*)m_pStatObj)->m_bStreamable)
  //  return false;
//  if(m_arrPhysGeomId[0]<0 && m_arrPhysGeomId[1]<0)
  //  return false;
        
//  if(!strstr(m_pStatObj->GetFileName(),"SWR_MP_PumpB.cgf"))
  //  return false;

  int nRecursionLevel = (int)GetRenderer()->EF_Query(EFQ_RecurseLevel) - 1;

  // some parameters will be modified
	SRendParams rParms = _EntDrawParams;

	int nLod;
	if (m_dwRndFlags & ERF_USELIGHTMAPS)
		nLod = 0; // disable since low lods has no lmaps calculated
	else // lod depends on distance and size
		nLod = max(0,(int)(rParms.fDistance*GetLodRatioNormilized()/(GetCVars()->e_obj_lod_ratio*GetRenderRadius())));


	// enable lightmaps if alowed
	if(m_dwRndFlags&ERF_USELIGHTMAPS && 
    HasLightmap(nLod) && 
    GetCVars()->e_light_maps &&
    _EntDrawParams.nShaderTemplate != EFT_INVLIGHT &&
//    !(_EntDrawParams.dwFlags & RPF_LIGHTPASS) &&
    !(_EntDrawParams.dwFObjFlags & FOB_FOGPASS))
	{
		rParms.pLightMapInfo = GetLightmap(nLod);
		rParms.pLMTCBuffer = GetLightmapTexCoord(nLod);
		
		if(GetCVars()->e_light_maps_occlusion)
		if(	m_arrOcclusionLightOwners[0] || m_arrOcclusionLightOwners[1] || 
				m_arrOcclusionLightOwners[2] || m_arrOcclusionLightOwners[3])
		{
			list2<CDLight> * pSources = ((C3DEngine*)m_p3DEngine)->GetDynamicLightSources();			

			*(DWORD*)rParms.arrOcclusionLightIds = 0;

			for(int i=0; i<pSources->Count(); i++)
			{
				CDLight * pDynLight = pSources->Get(i);
				if(pDynLight->m_Flags & DLF_SUN)
				{
					for(int k=0; m_arrOcclusionLightOwners[k] && k<4; k++)
					if((IEntityRender*)-1 == m_arrOcclusionLightOwners[k])
					{
						assert(k>=0 && k<4);
						rParms.arrOcclusionLightIds[k] = pDynLight->m_Id+1;
					}
				}
				else
				{
					for(int k=0; m_arrOcclusionLightOwners[k] && k<4; k++)
					if(pDynLight->m_pOwner == m_arrOcclusionLightOwners[k])
					{
						assert(k>=0 && k<4);
						rParms.arrOcclusionLightIds[k] = pDynLight->m_Id+1;
					}
				}
			}
		}		
	}
	else
	{
		rParms.pLightMapInfo = 0;
		rParms.pLMTCBuffer = 0;
	}

	// remember last rendered frames
  if(nRecursionLevel>=0 && nRecursionLevel<=1)
    SetDrawFrame( GetFrameID(), nRecursionLevel );

  if (m_nObjectTypeID<0 || !m_lstBrushTypes[m_nObjectTypeID])
    return false;

  Matrix44 tmpMatrix = m_Matrix;
	if(rParms.dwFObjFlags & FOB_RENDER_INTO_SHADOWMAP)
	{ // modify position
		tmpMatrix.SetTranslationOLD(rParms.vPos);
		rParms.pMatrix = &tmpMatrix;
	}
	else
		rParms.pMatrix = &m_Matrix;

	rParms.pMaterial = m_pMaterial;

	rParms.pCaller = this;

//  if(m_dwRndFlags & ERF_SELECTED)
  //  rParms.dwFObjFlags |= FOB_SELECTED;
	
	// render
	if (!rParms.pShadowVolumeLightSource)			
		m_lstBrushTypes[m_nObjectTypeID]->Render(rParms,Vec3(zero),nLod);
	else if(m_dwRndFlags&ERF_CASTSHADOWVOLUME)
		m_lstBrushTypes[m_nObjectTypeID]->RenderShadowVolumes(&rParms);
/*
	if (GetCVars()->e_brush_bboxes && !rParms.bRenderShadowVolumes)			
	{
		Vec3d mins,maxs;
		GetRenderBBox(mins,maxs);
		GetRenderer()->Draw3dBBox(mins,maxs);
	}
*/
	return true;
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

bool CBrush::IsStatic() const
{
	return true;
}

struct IStatObj * CBrush::GetEntityStatObj( unsigned int nSlot, Matrix44* pMatrix, bool bReturnOnlyVisible )
{
	if(nSlot != 0 || m_nObjectTypeID<0)
		return 0;

	if(pMatrix)
		*pMatrix = m_Matrix;

	return m_lstBrushTypes[m_nObjectTypeID];
}

struct ICryCharInstance* CBrush::GetEntityCharacter( unsigned int nSlot, Matrix44* pMatrix )
{
	return 0;
}

void CBrush::SetMatrix( Matrix44* pMatrix )
{
	Get3DEngine()->UnRegisterEntity(this);

	if(pMatrix)
	{
		if(!IsMatrixValid(*pMatrix))
		{
			Warning( 0,0,"Error: IEntityRender::SetMatrix: Invalid matrix passed from the editor - ignored, reset to identity: %s", GetName());
			m_Matrix.SetIdentity();
		}
    else
		  m_Matrix = *pMatrix;

		m_vPos = m_Matrix.GetTranslationOLD();
		CalcWholeBBox();
/*		if ((m_vWSBoxMin-m_vPos).Length() > (m_vWSBoxMax-m_vPos).Length())
			m_fWSRadius = (m_vWSBoxMin-m_vPos).Length();
		else
			m_fWSRadius = (m_vWSBoxMax-m_vPos).Length();*/

		m_fWSRadius = (m_vWSBoxMax-m_vWSBoxMin).Length()*0.5f;
	}

	if(!m_pEntityRenderState)
		m_pEntityRenderState = Get3DEngine()->MakeEntityRenderState();

	Get3DEngine()->RegisterEntity(this);

	Physicalize( );
}

void CBrush::CalcWholeBBox()
{
	m_vWSBoxMin = SetMaxBB();
	m_vWSBoxMax = SetMinBB();

	m_fWSRadius = 0;

	{
		if (m_nObjectTypeID<0 || !m_lstBrushTypes[m_nObjectTypeID])
			return;

		// get object space bbox
  	Vec3d mins,maxs;
		mins = m_lstBrushTypes[m_nObjectTypeID]->GetBoxMin();
		maxs = m_lstBrushTypes[m_nObjectTypeID]->GetBoxMax();

		/*
		// transform all 8 vertices into world space
		Vec3d verts[8] = 
		{ 
			m_Matrix.TransformPoint(Vec3d(mins.x,mins.y,mins.z)),
			m_Matrix.TransformPoint(Vec3d(mins.x,maxs.y,mins.z)),
			m_Matrix.TransformPoint(Vec3d(maxs.x,mins.y,mins.z)),
			m_Matrix.TransformPoint(Vec3d(maxs.x,maxs.y,mins.z)),
			m_Matrix.TransformPoint(Vec3d(mins.x,mins.y,maxs.z)),
			m_Matrix.TransformPoint(Vec3d(mins.x,maxs.y,maxs.z)),
			m_Matrix.TransformPoint(Vec3d(maxs.x,mins.y,maxs.z)),
			m_Matrix.TransformPoint(Vec3d(maxs.x,maxs.y,maxs.z))
		};

		// find new min/max values
		for(int i=0; i<8; i++)
		{
			m_vBoxMin.CheckMin(verts[i]);
			m_vBoxMax.CheckMax(verts[i]);
		}
		*/

		// Fast AABB transformation.
		Matrix44 &tm = m_Matrix;
		Vec3d m = tm.TransformPointOLD( mins );
		Vec3d vx = Vec3d(tm[0][0],tm[0][1],tm[0][2])*(maxs.x-mins.x);
		Vec3d vy = Vec3d(tm[1][0],tm[1][1],tm[1][2])*(maxs.y-mins.y);
		Vec3d vz = Vec3d(tm[2][0],tm[2][1],tm[2][2])*(maxs.z-mins.z);
		mins = m;
		maxs = m;
		if (vx.x < 0) mins.x += vx.x; else maxs.x += vx.x;
		if (vx.y < 0) mins.y += vx.y; else maxs.y += vx.y;
		if (vx.z < 0) mins.z += vx.z; else maxs.z += vx.z;

		if (vy.x < 0) mins.x += vy.x; else maxs.x += vy.x;
		if (vy.y < 0) mins.y += vy.y; else maxs.y += vy.y;
		if (vy.z < 0) mins.z += vy.z; else maxs.z += vy.z;

		if (vz.x < 0) mins.x += vz.x; else maxs.x += vz.x;
		if (vz.y < 0) mins.y += vz.y; else maxs.y += vz.y;
		if (vz.z < 0) mins.z += vz.z; else maxs.z += vz.z;

		m_vWSBoxMin.CheckMin( mins );
		m_vWSBoxMax.CheckMax( maxs );
	} 
}

CBrush::CBrush()
{
	m_vPos=m_vAngles=m_vWSBoxMin=m_vWSBoxMax=Vec3d(0,0,0);
	m_fScale=m_fWSRadius=0;
//	m_pStatObj=0;
	m_dwRndFlags=0;
	m_Matrix.SetIdentity();
	m_narrDrawFrames[0]=m_narrDrawFrames[1]=0;	
	m_pEntityRenderState=0;
	m_pPhysEnt=0;
	m_Matrix.SetIdentity();
//	m_pLMTCBuffer = NULL;
	m_pMaterial = 0;
  m_nEditorObjectId = 0;
//  m_arrPhysGeomId[0] = m_arrPhysGeomId[1] = -1;
  m_nObjectTypeID = -1;
  m_nMaterialId = -1;
	m_arrOcclusionLightOwners[0]=m_arrOcclusionLightOwners[1]=m_arrOcclusionLightOwners[2]=m_arrOcclusionLightOwners[3]=0;
}

void CBrush::DeleteLMTC()
{
  for(int nLod=0; nLod<MAX_BRUSH_LODS_NUM; nLod++)
  {
    if (m_arrLMData[nLod].m_pLMTCBuffer)
    {
      GetSystem()->GetIRenderer()->DeleteLeafBuffer(m_arrLMData[nLod].m_pLMTCBuffer);
      m_arrLMData[nLod].m_pLMTCBuffer = NULL;
    }
  }
}

CBrush::~CBrush()
{
	Dephysicalize( );
//	Get3DEngine()->UnRegisterEntity(this);
  Get3DEngine()->FreeEntityRenderState(this);

	//m_pLMData = NULL;
/*
	if(GetRndFlags()&ERF_MERGED_NEW && m_nObjectTypeID>=0)
	{
		CStatObj * pBody = (CStatObj*)m_lstBrushTypes[m_nObjectTypeID];
		assert(pBody->m_nUsers==1);
		delete pBody;
		m_lstBrushTypes[m_nObjectTypeID] = NULL;
		m_nObjectTypeID=-1;
	}
*/
  DeleteLMTC();
}

void CBrush::Physicalize(bool bInstant)
{
  assert(m_nObjectTypeID>=0);
	CStatObj * pBody = (CStatObj*)m_lstBrushTypes[m_nObjectTypeID];

	float fScaleX = m_Matrix.GetOrtX().len();
	float fScaleY = m_Matrix.GetOrtY().len();
	float fScaleZ = m_Matrix.GetOrtZ().len();

	if( fabs(fScaleX - fScaleY)>0.01f || fabs(fScaleX - fScaleZ)>0.01f || !pBody->IsPhysicsExist() )
	{ // scip non uniform scaled object and object without physics
		Dephysicalize();
		return;
	}

	if (!GetCVars()->e_on_demand_physics)
		bInstant = true;

	if (!bInstant)
	{
		pe_status_placeholder spc;
		if (m_pPhysEnt && m_pPhysEnt->GetStatus(&spc) && spc.pFullEntity)
			GetSystem()->GetIPhysicalWorld()->DestroyPhysicalEntity(spc.pFullEntity);

		pe_params_bbox pbb;
		pbb.BBox[0] = m_vWSBoxMin;
		pbb.BBox[1] = m_vWSBoxMax;
		pe_params_foreign_data pfd;
		pfd.pForeignData = this;
		pfd.iForeignData = 1;

		if (!m_pPhysEnt)
			m_pPhysEnt = GetSystem()->GetIPhysicalWorld()->CreatePhysicalPlaceholder(PE_STATIC,&pbb);
		else
			m_pPhysEnt->SetParams(&pbb);
		m_pPhysEnt->SetParams(&pfd);
		return;
	}

//  CStatObj * pBody = (CStatObj*)m_lstBrushTypes[m_nObjectTypeID];
  pBody->CheckLoaded();
//  if(!(pBody && (pBody->m_arrPhysGeomInfo[0] || pBody->m_arrPhysGeomInfo[1])))
  //  return;

	//Dephysicalize( );
	// create new
//	pe_params_pos par_pos;
	if (!m_pPhysEnt)
	{
		m_pPhysEnt = GetSystem()->GetIPhysicalWorld()->CreatePhysicalEntity(PE_STATIC,NULL,this,1);
		if(!m_pPhysEnt)
			return;
	}
	else if (!bInstant)
	{
		// this is on-demand creation, so entity pointer is automatically put into the placeholder
		GetSystem()->GetIPhysicalWorld()->CreatePhysicalEntity(PE_STATIC,5.0f);
	}

	pe_action_remove_all_parts remove_all;
	m_pPhysEnt->Action(&remove_all);

  pe_geomparams params;	  
	if(pBody->m_arrPhysGeomInfo[0])
  {
//    if(m_arrPhysGeomId[0]>=0)
  //    m_pPhysEnt->RemoveGeometry(m_arrPhysGeomId[0]);
    /*m_arrPhysGeomId[0] =*/ m_pPhysEnt->AddGeometry(pBody->m_arrPhysGeomInfo[0], &params);
  }
//  else
  //  m_arrPhysGeomId[0] = -1;
  
  params.flags = geom_colltype_ray;				          

  if(pBody->m_arrPhysGeomInfo[1])
  {
//    if(m_arrPhysGeomId[1]>=0)
  //    m_pPhysEnt->RemoveGeometry(m_arrPhysGeomId[1]);
	  /*m_arrPhysGeomId[1] =*/ m_pPhysEnt->AddGeometry(pBody->m_arrPhysGeomInfo[1], &params);
  }
//  else
  //  m_arrPhysGeomId[1] = -1;

  if(m_dwRndFlags & (ERF_HIDABLE|ERF_EXCLUDE_FROM_TRIANGULATION))
  {
    pe_params_foreign_data  foreignData;
    m_pPhysEnt->GetParams(&foreignData);
		if (m_dwRndFlags & ERF_HIDABLE)
			foreignData.iForeignFlags |= PFF_HIDABLE;
		//[PETAR] new flag to exclude from triangulation
		if (m_dwRndFlags & ERF_EXCLUDE_FROM_TRIANGULATION)
			foreignData.iForeignFlags |= PFF_EXCLUDE_FROM_STATIC;
    m_pPhysEnt->SetParams(&foreignData);
  }

	pe_params_flags par_flags;
	par_flags.flagsOR = pef_never_affect_triggers;
	m_pPhysEnt->SetParams(&par_flags);

  pe_params_pos par_pos;
	par_pos.pMtx4x4T = m_Matrix.GetData();
	m_pPhysEnt->SetParams(&par_pos);
}

void CBrush::Dephysicalize( )
{
	// delete old physics
	if(m_pPhysEnt)
    GetSystem()->GetIPhysicalWorld()->DestroyPhysicalEntity(m_pPhysEnt);
  m_pPhysEnt=0;
}

void CBrush::Dematerialize( )
{
  if(m_pMaterial)
    m_pMaterial = 0;
}

int __cdecl CBrush__Cmp_MergeID(const void* v1, const void* v2)
{
  CBrush ** pBr1 = (CBrush**)v1;
  CBrush ** pBr2 = (CBrush**)v2;

  // shader
  if(pBr1[0]->GetMergeId() > pBr2[0]->GetMergeId())
    return  1;
  else
  if(pBr1[0]->GetMergeId() < pBr2[0]->GetMergeId())
    return -1;

  return 0;
}

struct LMTexCoord
{
  float s,t;
};

int __cdecl CBrush__Cmp_MatChunks(const void* v1, const void* v2)
{
  CMatInfo * pMat1 = (CMatInfo*)v1;
  CMatInfo * pMat2 = (CMatInfo*)v2;

  // shader
  if(pMat1->shaderItem.m_pShader->GetTemplate(-1) > pMat2->shaderItem.m_pShader->GetTemplate(-1))
    return  1;
  else
  if(pMat1->shaderItem.m_pShader->GetTemplate(-1) < pMat2->shaderItem.m_pShader->GetTemplate(-1))
    return -1;

  // shader resources
  if(pMat1->shaderItem.m_pShaderResources > pMat2->shaderItem.m_pShaderResources)
    return  1;
  else
  if(pMat1->shaderItem.m_pShaderResources < pMat2->shaderItem.m_pShaderResources)
    return -1;

  // lm tex id
  if(pMat1->m_Id > pMat2->m_Id)
    return  1;
  else
  if(pMat1->m_Id < pMat2->m_Id)
    return -1;

  return 0;
}

struct SBrushM
{
  int Id;
  int nNumMats;
};
struct SMatGroup
{
  SShaderItem sh;
  int nLMId;
  int nNumVerts;
  TArray<SBrushM> Owners;
};

void CObjManager::MergeBrushes()
{
  if (!GetCVars()->e_brushes_merging || !m_lstBrushContainer.Count())
    return;

  int nEntityId=0;
  int nNextStartId=-1;
  list2<CBrush *> newBrushes;
  int nMergeId = -1;
  qsort(&m_lstBrushContainer[0], m_lstBrushContainer.Count(), sizeof(m_lstBrushContainer[0]), CBrush__Cmp_MergeID);

  list2<struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F> lstVerts;
  list2<LMTexCoord> lstLMTexCoords;
  list2<ushort> lstIndices;
  list2<CMatInfo> lstChunks;
  list2<CMatInfo> lstChunksMerged;
  list2<SPipTangents> lstTangBasises;
  int i, j;

  int nNumGroups = 0;
  TArray<SMatGroup *> *groupBrushes;
  for(i=0; i<m_lstBrushContainer.Count(); i++)
  {
    CBrush* pBrush	= m_lstBrushContainer[i];
    nMergeId = pBrush->GetMergeId();
    if (nMergeId <= 0)
      continue;
    //nMergeId = 1;
    //pBrush->SetMergeId(nMergeId);
    if (nMergeId > nNumGroups)
      nNumGroups = nMergeId;
  }
  if (!nNumGroups)
    return;
  nMergeId = -1;

  GetLog()->UpdateLoadingScreen("\003---Merge brushes ... ");

  FILE *fp = NULL;
  if (GetCVars()->e_brushes_merging_debug)
  {
    fp = fopen("Brushes.txt", "w");
    int nLastMergedId = -1;
    for(i=0; i<m_lstBrushContainer.Count(); i++)
    {
      CBrush* pBrush	= m_lstBrushContainer[i];
      nMergeId = pBrush->GetMergeId();
      if (nMergeId <= 0)
        continue;
      if (nLastMergedId != nMergeId)
      {
        nLastMergedId = nMergeId;
        fprintf(fp, "\n");
      }
      Matrix44 mat;
      mat.SetIdentity();
      IStatObj * pStatObj = pBrush->GetEntityStatObj(0,&mat);
      if(!pStatObj)
        continue;
      int nLMTexId = pBrush->GetLightmap(0) ? pBrush->GetLightmap(0)->GetColorLerpTex() : 0;
      int nLMHDRTexId = pBrush->GetLightmap(0) ? pBrush->GetLightmap(0)->GetHDRColorLerpTex() : 0;
      CLeafBuffer * pLB = pStatObj->GetLeafBuffer();
      fprintf(fp, "Brush: %d, MergeId: %d, LMId: %d, Mats: %d (%s)\n", i, nMergeId, nLMTexId, pLB->m_pMats->Count(), pStatObj->GetFileName());
    }
    fclose(fp);
    fp = fopen("BrushesMerged.txt", "w");
  }

  groupBrushes = new TArray<SMatGroup *>[nNumGroups];
  for(i=0; i<m_lstBrushContainer.Count(); i++)
  {
    CBrush* pBrush	= m_lstBrushContainer[i];
    nMergeId = pBrush->GetMergeId();
    if (nMergeId <= 0)
      continue;
    nMergeId--;
    if(pBrush->m_dwRndFlags & ERF_CASTSHADOWVOLUME || pBrush->m_dwRndFlags & ERF_CASTSHADOWMAPS)
      continue;
    Matrix44 mat;
    mat.SetIdentity();
    int nLMTexId = pBrush->GetLightmap(0) ? pBrush->GetLightmap(0)->GetColorLerpTex() : 0;
    int nHDRLMTexId = pBrush->GetLightmap(0) ? pBrush->GetLightmap(0)->GetHDRColorLerpTex() : 0;
    IStatObj * pStatObj = pBrush->GetEntityStatObj(0,&mat);
    if(!pStatObj)
      continue;
    CLeafBuffer * pLB = pStatObj->GetLeafBuffer();
    if(!pLB || !pLB->m_SecVertCount)
      continue;
    for(int m=0; m<pLB->m_pMats->Count(); m++)
    {
      CMatInfo newMatInfo = *pLB->m_pMats->Get(m);

      if(GetCVars()->e_materials)
      { // Override default material
        CMatInfo * pCustMat = (CMatInfo *)pBrush->GetMaterial();
        if (pCustMat)
        {
          int nMatId = newMatInfo.m_nCGFMaterialID;
          if(nMatId<0)
            continue;

          if (nMatId == 0)
            pCustMat = (CMatInfo*)pCustMat;
          else
          if (nMatId-1 < pCustMat->GetSubMtlCount())
            pCustMat = (CMatInfo*)pCustMat->GetSubMtl(nMatId-1);

          newMatInfo.shaderItem = pCustMat->shaderItem;
        }
      }
      SMatGroup *mg;
      for (j=0; j<groupBrushes[nMergeId].Num(); j++)
      {
        mg = groupBrushes[nMergeId][j];
        if (!mg)
          continue;
        IShader *shm = newMatInfo.shaderItem.m_pShader->GetTemplate(-1);
        if (shm == mg->sh.m_pShader &&
            newMatInfo.shaderItem.m_pShaderResources == mg->sh.m_pShaderResources &&
            nLMTexId == mg->nLMId)
          break;
      }
      if (j == groupBrushes[nMergeId].Num())
      {
        mg = new SMatGroup;
        mg->sh.m_pShader = newMatInfo.shaderItem.m_pShader->GetTemplate(-1);
        mg->sh.m_pShaderResources = newMatInfo.shaderItem.m_pShaderResources;
        mg->nLMId = nLMTexId;
        mg->nNumVerts = 0;
        groupBrushes[nMergeId].AddElem(mg);
      }
      for (j=0; j<mg->Owners.Num(); j++)
      {
        if (mg->Owners[j].Id == i)
          break;
      }
      if (j == mg->Owners.Num())
      {
        SBrushM bm;
        bm.Id = i;
        bm.nNumMats = pLB->m_pMats->Count();
        mg->Owners.AddElem(bm);
      }
      mg->nNumVerts += newMatInfo.nNumVerts;
    }
  }
  TArray<bool> Enabled;
  TArray<int> maxMergedMats;
  maxMergedMats.Reserve(m_lstBrushContainer.Count());
  Enabled.Reserve(m_lstBrushContainer.Count());
  for (i=0; i<nNumGroups; i++)
  {
    for (j=0; j<groupBrushes[i].Num(); j++)
    {
      SMatGroup *mg = groupBrushes[i][j];
      for (int m=0; m<mg->Owners.Num(); m++)
      {
        int nId = mg->Owners[m].Id;
        maxMergedMats[nId] = max(mg->Owners.Num(), maxMergedMats[nId]);
      }
    }
  }
  int nRejected = 0;
  int nRequested = 0;
  for(j=0; j<m_lstBrushContainer.Count(); j++)
  {
    CBrush* pBrush	= m_lstBrushContainer[j];
    if(pBrush->m_dwRndFlags & ERF_CASTSHADOWVOLUME || pBrush->m_dwRndFlags & ERF_CASTSHADOWMAPS)
      continue;
    nMergeId = pBrush->GetMergeId();
    if (nMergeId <= 0)
      continue;
    nRequested++;
    if (maxMergedMats[j] >= 3)
      Enabled[j] = true;
    else
    {
      nRejected++;
      Enabled[j] = false;
    }
  }

  while(nEntityId<m_lstBrushContainer.Count() || nNextStartId>=0)
  {
    Vec3d vBoxMax(-10000,-10000,-10000);
    Vec3d vBoxMin( 10000, 10000, 10000);

    int nLMTexId=-1;
    int nHDRLMTexId=-1;
    int nLMDirTexId=-1;
    int nCurVertsNum=0;

    if(nNextStartId>=0)
      nEntityId = nNextStartId;
    nNextStartId=-1;
    int nMergeID = -1;

    lstVerts.Clear();
    lstLMTexCoords.Clear();
    lstIndices.Clear();
    list2<ushort> lstIndicesSorted; lstIndicesSorted.Clear();
    lstChunks.Clear();
    lstChunksMerged.Clear();
    lstTangBasises.Clear();

    TArray<int> brMerged;
    brMerged.Free();
		int dwResRndFlags = ERF_RECVSHADOWMAPS;
    for( ;nEntityId<m_lstBrushContainer.Count(); nEntityId++)
    {
      CBrush* pBrush	= m_lstBrushContainer[nEntityId];
      if (pBrush->GetMergeId() <= 0)
        continue;
      if(	pBrush->m_dwRndFlags & ERF_MERGED ||
          pBrush->m_dwRndFlags & ERF_CASTSHADOWVOLUME ||
          pBrush->m_dwRndFlags & ERF_CASTSHADOWMAPS)
        continue;
      if (!Enabled[nEntityId])
        continue;
      if (nMergeID != pBrush->GetMergeId() && nCurVertsNum)
        break;
      nMergeID = pBrush->GetMergeId();
      if(nLMTexId<0)
      {
        nLMTexId = pBrush->GetLightmap(0) ? pBrush->GetLightmap(0)->GetColorLerpTex() : 0;
        nHDRLMTexId = pBrush->GetLightmap(0) ? pBrush->GetLightmap(0)->GetHDRColorLerpTex() : 0;
        nLMDirTexId = pBrush->GetLightmap(0) ? pBrush->GetLightmap(0)->GetDomDirectionTex() : 0;
      }
      else
      {
        if(nLMTexId != (pBrush->GetLightmap(0) ? pBrush->GetLightmap(0)->GetColorLerpTex() : 0) ||
           nHDRLMTexId != (pBrush->GetLightmap(0) ? pBrush->GetLightmap(0)->GetHDRColorLerpTex() : 0) ||
           nLMDirTexId != (pBrush->GetLightmap(0) ? pBrush->GetLightmap(0)->GetDomDirectionTex() : 0))
        {
          if(nNextStartId<0)
            nNextStartId = nEntityId;
          continue;
        }
      }
      Matrix44 mat;
      mat.SetIdentity();
      IStatObj * pStatObj = pBrush->GetEntityStatObj(0,&mat);
      if(!pStatObj)
        continue;
      EERType eType = pBrush->GetEntityRenderType();
      assert(eType == eERType_Brush);
      if (eType != eERType_Brush)
        continue;

      if(!CBrush::IsMatrixValid(mat))
        continue;

      CLeafBuffer * pLMLB = pBrush->GetLightmapTexCoord(0);

      CLeafBuffer * pLB = pStatObj->GetLeafBuffer();
      if(!pLB->m_SecVertCount)
        continue;

      if(nCurVertsNum + pLB->m_SecVertCount>65535)
        break;

      int nIndCount=0;
      pLB->GetIndices(&nIndCount);
      brMerged.AddElem(nEntityId);

      int nInitVertCout = lstVerts.Count();
      for(int m=0; m<pLB->m_pMats->Count(); m++)
      {
        CMatInfo newMatInfo = *pLB->m_pMats->Get(m);

        if(GetCVars()->e_materials)
        { // Override default material
          CMatInfo * pCustMat = (CMatInfo *)pBrush->GetMaterial();
          if (pCustMat)
          {
            int nMatId = newMatInfo.m_nCGFMaterialID;
            if(nMatId<0)
              continue;

            if (nMatId == 0)
              pCustMat = (CMatInfo*)pCustMat;
            else
            if (nMatId-1 < pCustMat->GetSubMtlCount())
              pCustMat = (CMatInfo*)pCustMat->GetSubMtl(nMatId-1);

            newMatInfo.shaderItem = pCustMat->shaderItem;
          }
        }

        // copy indices
        ushort *pSrcInds = pLB->GetIndices(0);
        for(int i=newMatInfo.nFirstIndexId; i<newMatInfo.nFirstIndexId+newMatInfo.nNumIndices; i++)
          lstIndices.Add(pSrcInds[i]+nInitVertCout);
        newMatInfo.nFirstIndexId = lstIndices.Count() - newMatInfo.nNumIndices;

        // copy verts
        int nPosStride=0;
        const byte * pPos = pLB->GetPosPtr(nPosStride,0,true);
        int nTexStride=0;
        const byte * pTex = pLB->GetUVPtr(nTexStride,0,true);
        int nNormStride=0;
        const byte * pNorm = pLB->GetNormalPtr(nNormStride,0,true);

        // get tangent basis
        int nTangStride=0;
        const byte * pTang = pLB->GetTangentPtr(nTangStride,0,true);
        int nTnormStride=0;
        const byte * pTNorm = pLB->GetTNormalPtr(nTnormStride,0,true);
        int nBNormStride=0;
        const byte * pBNorm = pLB->GetBinormalPtr(nBNormStride,0,true);
        int nColorStride=0;
        const byte * pColor = pLB->GetColorPtr(nColorStride,0,true);

        // get LM TexCoords
        int nLMStride=0;
        const byte * pLMTexCoords = pLMLB ? pLMLB->GetPosPtr(nLMStride,0,true) : 0;

        for(int v=newMatInfo.nFirstVertId; v<newMatInfo.nFirstVertId+newMatInfo.nNumVerts; v++)
        {
          struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F vert;

          // set pos
          Vec3d vPos = *(Vec3d*)&pPos[nPosStride*v];				
          vPos = mat.TransformPointOLD(vPos);
          vert.xyz = vPos;

          // set uv
          float * pUV = (float*)&pTex[nTexStride*v];				
          vert.st[0] = pUV[0];
          vert.st[1] = pUV[1];
          if (pNorm)
          {
            Vec3d vNrm = mat.TransformVectorOLD(*(Vec3d*)&pNorm[nNormStride*v]);
            vert.normal = vNrm;
          }
          else
          {
            vert.normal.Set(0,0,1);
          }

          vert.color.dcolor = *(DWORD*)&pColor[nColorStride*v+0];

          // calc bbox
          vBoxMin.CheckMin(vPos);
          vBoxMax.CheckMax(vPos);

          lstVerts.Add(vert);

          // add tbasis
          SPipTangents basis;
          basis.m_Tangent = mat.TransformVectorOLD(*(Vec3d*)&pTang[nTangStride*v]);
          basis.m_TNormal = mat.TransformVectorOLD(*(Vec3d*)&pTNorm[nTnormStride*v]);
          basis.m_Binormal= mat.TransformVectorOLD(*(Vec3d*)&pBNorm[nBNormStride*v]);
          lstTangBasises.Add(basis);

          // add LM texcoords
          LMTexCoord vLMTC;
          if(pLMTexCoords)
            vLMTC = *(LMTexCoord*)&pLMTexCoords[nLMStride*v];
          else
            vLMTC.s = vLMTC.t = 0;

          lstLMTexCoords.Add(vLMTC);
        }

        // set vert range
        newMatInfo.nFirstVertId = lstVerts.Count() - newMatInfo.nNumVerts;

        newMatInfo.pRE = 0;
        newMatInfo.m_Id = pBrush->GetLightmap(0) ? pBrush->GetLightmap(0)->GetColorLerpTex() : 0;

        if(newMatInfo.nNumIndices)
        {
          lstChunks.Add(newMatInfo);
        }
        else
          assert(!newMatInfo.nNumVerts);
      }
      nCurVertsNum += pLB->m_SecVertCount;

      pBrush->m_dwRndFlags |= ERF_MERGED;

			dwResRndFlags &= pBrush->m_dwRndFlags;
    }

    CBrush * pNewBrush = 0;
    if(!lstVerts.Count())
      continue;

    // sort
    if(lstChunks.Count())
      qsort(lstChunks.GetElements(), lstChunks.Count(), 
      sizeof(lstChunks[0]), CBrush__Cmp_MatChunks);

    // merge chunks
    for(int nChunk=0; nChunk<lstChunks.Count(); nChunk++)
    {
      if(!nChunk || CBrush__Cmp_MatChunks(&lstChunks[nChunk], &lstChunks[nChunk-1]))
      { // not equal materials - add new chunk
        lstChunksMerged.Add(lstChunks[nChunk]);
        lstChunksMerged.Last().nFirstIndexId = lstIndicesSorted.Count();
        lstChunksMerged.Last().nNumIndices = 0;

        lstChunksMerged.Last().nFirstVertId = 0;
        lstChunksMerged.Last().nNumVerts = lstVerts.Count();
      }

      // add indices
      for(int nId=lstChunks[nChunk].nFirstIndexId; nId<lstChunks[nChunk].nFirstIndexId+lstChunks[nChunk].nNumIndices; nId++)
        lstIndicesSorted.Add(lstIndices[nId]);

      // update start/stop pos
      lstChunksMerged.Last().nNumIndices += lstChunks[nChunk].nNumIndices;
    }
    lstChunks = lstChunksMerged;
    lstIndices = lstIndicesSorted;

    // make leaf buffer
    CLeafBuffer * pNewLB = GetRenderer()->CreateLeafBufferInitialized(
      lstVerts.GetElements(), lstVerts.Count(), VERTEX_FORMAT_P3F_N_COL4UB_TEX2F, 
      lstIndices.GetElements(), lstIndices.Count(), R_PRIMV_TRIANGLES,
      "MergedLB", eBT_Static, lstChunks.Count());

    pNewLB->UpdateTangBuffer(lstTangBasises.GetElements());

    for(int i=0; i<lstChunks.Count(); i++)
    {
      pNewLB->SetChunk(lstChunks[i].GetShaderItem().m_pShader,
        lstChunks[i].nFirstVertId, lstChunks[i].nNumVerts,
        lstChunks[i].nFirstIndexId, lstChunks[i].nNumIndices, i, true);

      assert(lstChunks[i].GetShaderItem().m_pShaderResources);
      assert(lstChunks[i].GetShaderItem().m_pShader);
      pNewLB->m_pMats->Get(i)->shaderItem = lstChunks[i].GetShaderItem();

      pNewLB->m_pMats->Get(i)->shaderItem.m_pShader->AddRef();
      pNewLB->m_pMats->Get(i)->shaderItem.m_pShaderResources->AddRef();
    }

    // make statobj
    CStatObj * pNewStatObj = MakeObject("NOFILE");
    pNewStatObj->m_nLoadedTrisCount = lstIndices.Count()/3;
    pNewStatObj->SetLeafBuffer(pNewLB);
    pNewStatObj->SetBBoxMin(vBoxMin);
    pNewStatObj->SetBBoxMax(vBoxMax);
		sprintf(pNewStatObj->m_szFileName,"nMergeID=%d",nMergeID);

    // make brush
    pNewBrush = new CBrush();
    if (fp)
    {
      fprintf(fp, "\n\nMerged Brush: %d, GroupID: %d\n", newBrushes.Count(), nMergeID);
      for (int i=0; i<brMerged.Num(); i++)
      {
        fprintf(fp, "Brush: %d:\n", brMerged[i]);
      }
    }

    //	if(m_pAreaBrush == (CBrush*)0x0e968358)
    //	int b=0;

    Matrix44 mat;
    mat.SetIdentity();
    pNewBrush->SetEntityStatObj(0,pNewStatObj,&mat);
    pNewBrush->m_vWSBoxMin = vBoxMin;
    pNewBrush->m_vWSBoxMax = vBoxMax;
    pNewBrush->m_fWSRadius = vBoxMin.GetDistance(vBoxMax)*0.5f;
    pNewBrush->m_dwRndFlags |= ERF_MERGED_NEW;
    pNewBrush->m_dwRndFlags |= ERF_NOTRANS_MASK;
		pNewBrush->m_dwRndFlags |= dwResRndFlags;

    // Make leafbuffer and fill it with texture coordinates
    if(nLMTexId && (nLMDirTexId || (GetCVars()->e_light_maps_quality==0)))
    {
      RenderLMData * pLMData = new RenderLMData(GetRenderer(), nLMTexId, nHDRLMTexId,  nLMDirTexId);
      pNewBrush->SetLightmap(pLMData, (float*)lstLMTexCoords.GetElements(), lstLMTexCoords.Count(), 0);
      pLMData->AddRef();
      pNewBrush->SetRndFlags(ERF_USELIGHTMAPS,true);
    }

    //Get3DEngine()->UnRegisterEntity(pNewBrush);
    //Get3DEngine()->RegisterEntity(pNewBrush);

    // find distance to the camera
    const Vec3d vCamPos = GetViewCamera().GetPos();
    Vec3d vCenter = (pNewBrush->m_vWSBoxMin+pNewBrush->m_vWSBoxMax)*0.5f;
    float fEntDistance = GetDist2D( vCamPos.x, vCamPos.y, vCenter.x, vCenter.y );

    assert(fEntDistance>=0);
    assert(_finite(fEntDistance));

    newBrushes.Add(pNewBrush);
  }
  int nOldBrushes = m_lstBrushContainer.Count();
  int nNewBrushes = newBrushes.Count();
  int nMergedBrushes = nNewBrushes;
  for (i=0; i<m_lstBrushContainer.Count(); i++)
  {
    CBrush *pBrush = m_lstBrushContainer[i];
    if (pBrush)
    {
      if (pBrush->m_dwRndFlags & ERF_MERGED)
      {
        pBrush->DeleteLMTC();
      }
      else
        nMergedBrushes++;
      newBrushes.Add(m_lstBrushContainer[i]);
    }
  }
  m_lstBrushContainer.Free();
  m_lstBrushContainer.AddList(newBrushes);

  for (i=0; i<nNumGroups; i++)
  {
    for (j=0; j<groupBrushes[i].Num(); j++)
    {
      SMatGroup *mg = groupBrushes[i][j];
      delete mg;
    }
  }
  SAFE_DELETE_ARRAY(groupBrushes);
  if (fp)
    fclose(fp);

  GetLog()->UpdateLoadingScreen("\003---%d groups, %d brushes requested, %d brushes rejected", nNumGroups, nRequested, nRejected);
  GetLog()->UpdateLoadingScreen("\003---%d additional brushes, %d new brushes: %d old brushes (%.3f perc)", nNewBrushes, nMergedBrushes, nOldBrushes, (float)nMergedBrushes/(float)nOldBrushes*100.0f);
}

void CObjManager::LoadBrushes()
{
	GetLog()->UpdateLoadingScreen("\003Loading brushes ... ");
  assert(!m_lstBrushContainer.Count());
	for(int i=0; i<m_lstBrushContainer.Count(); i++)
	{
		UnRegisterEntity(m_lstBrushContainer[i]);
		delete m_lstBrushContainer[i];
	}
	m_lstBrushContainer.Reset();

	SExportedBrushHeader header;
  /*list2<SExportedBrushMaterial> */CBrush::m_lstSExportedBrushMaterials.Reset();
	list2<SExportedBrushGeom> lstSExportedBrushGeom;
	list2<SExportedBrush> lstSExportedBrush;

	char szName[512]="";
	strncpy(szName, Get3DEngine()->GetLevelFilePath("brush.lst"), sizeof(szName));

  FILE * f = GetSystem()->GetIPak()->FOpen(szName, "rb");
  if(!f)
    return;

	GetSystem()->GetIPak()->FRead( &header,sizeof(header),1,f );
	if ((strncmp(header.signature,BRUSH_FILE_SIGNATURE,3) != 0) || (header.filetype != BRUSH_FILE_TYPE) ||
			(header.version != BRUSH_FILE_VERSION))
	{
		// Not Cry file.
		Warning(0,szName,"CObjManager::LoadBrushes: File %s is not a valid brush list file or wrong version, Reexport you level in Editor",szName );
		GetSystem()->GetIPak()->FClose(f);
		return;
	}

  int nCount = 0;
	GetSystem()->GetIPak()->FRead(&nCount, 4, 1, f);
	int numMaterials = nCount;
	if(nCount<0 || nCount>100000)
	{
		GetSystem()->Warning(VALIDATOR_MODULE_3DENGINE,VALIDATOR_ERROR,0,szName,
			"CObjManager::LoadBrushes: invalid brush.lst file, Reexport the level from Editor again");
		return;
	}

	if (numMaterials > 0)
	{
		CBrush::m_lstSExportedBrushMaterials.PreAllocate(nCount,nCount);
		GetSystem()->GetIPak()->FRead(&CBrush::m_lstSExportedBrushMaterials[0], sizeof(SExportedBrushMaterial), nCount, f);
	}

  GetSystem()->GetIPak()->FRead(&nCount, 4, 1, f);
	if(nCount<=0 || nCount>100000)
	{
		GetSystem()->Warning(VALIDATOR_MODULE_3DENGINE,VALIDATOR_ERROR,0,szName,
			"CObjManager::LoadBrushes: invalid brush.lst file, Reexport the level from Editor again");
		return;
	}
	lstSExportedBrushGeom.PreAllocate(nCount,nCount);
  GetSystem()->GetIPak()->FRead(&lstSExportedBrushGeom[0], sizeof(SExportedBrushGeom), nCount, f);
  
  nCount = 0;
	GetSystem()->GetIPak()->FRead(&nCount, 4, 1, f);
	if(nCount<=0 || nCount>100000)
	{
		GetSystem()->Warning(VALIDATOR_MODULE_3DENGINE,VALIDATOR_ERROR,0,szName,
			"CObjManager::LoadBrushes: invalid brush.lst file, Reexport the level from Editor again");
		return;
	}
	lstSExportedBrush.PreAllocate(nCount,nCount);
  GetSystem()->GetIPak()->FRead(&lstSExportedBrush[0], sizeof(SExportedBrush), nCount, f);

	GetSystem()->GetIPak()->FClose(f);

	std::vector<IEntityRender *> vGLMs;

  // load instances
	for(int i=0; i<lstSExportedBrush.Count(); i++)
	{
		Matrix44 mat44 = GetTransposed44(Matrix44(lstSExportedBrush[i].matrix));

		if(!CBrush::IsMatrixValid(mat44))
		{
			Warning( 0,szName,"CObjManager::LoadBrushes: Invalid transformation matrix specified for object: %s",
				lstSExportedBrushGeom[lstSExportedBrush[i].geometry].filename);
			continue;
		}

		if(lstSExportedBrush[i].geometry>=lstSExportedBrushGeom.Count() ||
			GetDistance(lstSExportedBrushGeom[lstSExportedBrush[i].geometry].m_minBBox, lstSExportedBrushGeom[lstSExportedBrush[i].geometry].m_maxBBox)>256)
		{
			GetSystem()->Warning(VALIDATOR_MODULE_3DENGINE,VALIDATOR_ERROR,0,szName,
				"CObjManager::LoadBrushes: invalid brush.lst file, Reexport the level from Editor again");
			continue;
		}

		CBrush * pEnt = (CBrush *)Get3DEngine()->CreateEntityRender();
		const char * szFileName = lstSExportedBrushGeom[lstSExportedBrush[i].geometry].filename;
		IStatObj * pStatObj = MakeObject(szFileName, NULL, 
			(lstSExportedBrush[i].flags&ERF_USELIGHTMAPS) ? evs_NoSharing : evs_ShareAndSortForCache, 
			true, false, GetCVars()->e_stream_cgf==1);

		// Find or allocate slot containing pointer to this StatObj
		int nBrushGeomId = CBrush::m_lstBrushTypes.Find(pStatObj);
		if(nBrushGeomId<0)
		{ // make new slot
			CBrush::m_lstBrushTypes.Add(pStatObj);
			nBrushGeomId = CBrush::m_lstBrushTypes.Count()-1;
		}

		assert(nBrushGeomId>=0);

    if(GetCVars()->e_stream_cgf)
    {
      pStatObj->SetBBoxMin(lstSExportedBrushGeom[lstSExportedBrush[i].geometry].m_minBBox);
		  pStatObj->SetBBoxMax(lstSExportedBrushGeom[lstSExportedBrush[i].geometry].m_maxBBox);
    }

    pEnt->SetStatObjGroupId(nBrushGeomId);
    pEnt->SetMergeId(lstSExportedBrush[i].mergeId);
    pEnt->SetMatrix( &mat44 );

		pEnt->SetRndFlags(lstSExportedBrush[i].flags & ~(ERF_SELECTED | ERF_HIDDEN));

		pEnt->SetEditorObjectId(lstSExportedBrush[i].id);

    pEnt->SetViewDistRatio(lstSExportedBrush[i].ratioViewDist);
    pEnt->SetLodRatio(lstSExportedBrush[i].ratioLod);

		if (pEnt->GetRndFlags() & ERF_USELIGHTMAPS)
			vGLMs.push_back(pEnt);

		// Find material.
		if (numMaterials > 0)
		{
			int mtlid = lstSExportedBrush[i].material;
			if (mtlid >= 0 && mtlid < numMaterials)
			{
				const char *sMtlName = CBrush::m_lstSExportedBrushMaterials[mtlid].material;
				IMatInfo *pMtl = Get3DEngine()->FindMaterial( sMtlName );
				if (pMtl)
				{
					pEnt->SetMaterial( pMtl );
					pEnt->SetMaterialId( mtlid );
				}
			}
		}
	}

  GetLog()->UpdateLoadingScreen("\003Brushes loaded: %d models, %d instances, %d custom materials", lstSExportedBrushGeom.Count(), lstSExportedBrush.Count(), CBrush::m_lstSExportedBrushMaterials.Count());

  if(GetCVars()->e_light_maps)
  {
	  ILMSerializationManager *pILMSerialization = Get3DEngine()->CreateLMSerializationManager();	
	  pILMSerialization->ApplyLightmapfile(Get3DEngine()->GetLevelFilePath(LM_EXPORT_FILE_NAME), vGLMs);
	  pILMSerialization->Release();
	  pILMSerialization = NULL;
  }
}

bool CBrush::IsEntityHasSomethingToRender()
{
	return m_nObjectTypeID>=0 && m_lstBrushTypes[m_nObjectTypeID] && !(m_dwRndFlags & ERF_HIDDEN);
}

bool CBrush::IsEntityAreasVisible()
{
	if(!GetEntityRS())
		return false;

	// test last render frame id
	if(m_pVisArea)
		if(abs(m_pVisArea->m_nRndFrameId - GetFrameID())<=2)
			return true;

	return false;
}

IPhysicalEntity* CBrush::GetPhysics() const
{
	return m_pPhysEnt;
}

void CBrush::SetPhysics( IPhysicalEntity* pPhys )
{
	m_pPhysEnt = pPhys;
}

bool CBrush::IsMatrixValid(const Matrix44 & mat)
{
	Vec3d vScaleTest = mat.TransformVectorOLD(Vec3d(0,0,1));
  float fDist = GetDistance(mat.GetTranslationOLD(),Vec3d(0,0,0));

	if( vScaleTest.Length()>100.f || vScaleTest.Length()<0.01f || fDist > 64000 ||
		!_finite(vScaleTest.x) || !_finite(vScaleTest.y) || !_finite(vScaleTest.z) )
		return false;

	return true;
}

void CBrush::SetMaterial( IMatInfo *pMatInfo )
{
  m_pMaterial = pMatInfo;

	if(m_pMaterial)
		m_pMaterial->SetFlags(m_pMaterial->GetFlags()|MIF_WASUSED);
}

IMatInfo* CBrush::GetMaterial() const
{
	return m_pMaterial;
}

void CBrush::CheckPhysicalized()
{
//  if(GetEntityStatObj(0))
  //  ((CStatObj*)GetEntityStatObj(0))->CheckLoaded(true);

  if(!m_pPhysEnt)
    Physicalize();
}

int CBrush::DestroyPhysicalEntityCallback(IPhysicalEntity *pent) 
{ 
//  assert(pent == m_pPhysEnt);

/*  for(int i=0; i<2; i++)
  {
    if(m_arrPhysGeomId[i]>=0)
      pent->RemoveGeometry(m_arrPhysGeomId[i]);
//    m_arrPhysGeomId[i] = -1;
  }
*/
  for(int i=0; i<2; i++)
  {
  //  if(m_arrPhysGeomId[i]>=0)
   //   m_pPhysEnt->RemoveGeometry(m_arrPhysGeomId[i]);
//    m_arrPhysGeomId[i] = -1;
  }

  return 1; 
}

float CBrush::GetMaxViewDist()
{
  return max(GetCVars()->e_obj_min_view_dist, 
		m_fWSRadius*GetCVars()->e_obj_view_dist_ratio*GetViewDistRatioNormilized());
}

void CBrush::Serialize(bool bSave, ICryPak * pPak, FILE * f)
{
#if 0

  if(bSave)
  {
    pPak->FWrite(this,sizeof(*this),1,f);

    if(0 && m_pLMTCBuffer)
    {
  /*    int nPos=0;
      m_pLMTCBuffer->Serialize(nPos,0,true,0,0);
      uchar * pLMLBuffer = new uchar[nPos];
      pPak->FWrite(&nPos,sizeof(nPos),1,f);
      nPos=0;
      m_pLMTCBuffer->Serialize(nPos,pLMLBuffer,true,0,0);*/

      // Make leafbuffer and fill it with texture coordinates
//      m_pLMTCBuffer = GetRenderer()->CreateLeafBufferInitialized(
  //      &vTexCoord3[0], pLeafBuffer->m_SecVertCount, VERTEX_FORMAT_P3F, 
    //    pLeafBuffer->GetIndices(0), pLeafBuffer->m_Indices.m_nItems, R_PRIMV_TRIANGLES, "LMapTexCoords", eBT_Static);

      pPak->FWrite(&m_pLMTCBuffer->m_SecVertCount,sizeof(m_pLMTCBuffer->m_SecVertCount),1,f);
      byte * pData = (byte *)m_pLMTCBuffer->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
      assert(m_VertexSize[m_pLMTCBuffer->m_pSecVertBuffer->m_vertexformat] == 12);
      int nSize = m_VertexSize[m_pLMTCBuffer->m_pSecVertBuffer->m_vertexformat]*m_pLMTCBuffer->m_SecVertCount;
      pPak->FWrite(&m_pLMTCBuffer->m_SecVertCount,sizeof(m_pLMTCBuffer->m_SecVertCount),1,f);
    }
    else
    {
      int nPos=0;
      pPak->FWrite(&nPos,sizeof(nPos),1,f);
    }
  }
  else
  {
    pPak->FRead(this,sizeof(*this),1,f);

    if(m_nMaterialId>=0)
    { // restore material
      const char * sMtlName = CBrush::m_lstSExportedBrushMaterials[m_nMaterialId].material;
      m_pMaterial = Get3DEngine()->FindMaterial( sMtlName );
    }
    else
      m_pMaterial = 0;

    int nSize=0;
    pPak->FRead(&nSize,sizeof(nSize),1,f);

    if(nSize)
    {
      assert(0);
      int nVertCount = 0;
      pPak->FRead(&nVertCount,sizeof(nVertCount),1,f);
      byte * pData = new byte[nVertCount*12];
      pPak->FRead(pData,nVertCount*12,1,f);

      // Make leafbuffer and fill it with texture coordinates
      m_pLMTCBuffer = GetRenderer()->CreateLeafBufferInitialized(
        pData, nVertCount, VERTEX_FORMAT_P3F, 
        0, 0, R_PRIMV_TRIANGLES, "LMapTexCoordsStreamed", eBT_Static);

      assert(m_VertexSize[m_pLMTCBuffer->m_pSecVertBuffer->m_vertexformat] == 12);
    }
    else
    {
      m_pLMTCBuffer = 0;
      m_pLMData = NULL;
    }
  }

#endif
}

int CBrush::GetMemoryUsage()
{
  return sizeof(*this) + m_pEntityRenderState ? sizeof(*m_pEntityRenderState) : 0;
}

void CBrush::SetEntityStatObj( unsigned int nSlot, IStatObj * pStatObj, Matrix44 * pMatrix )
{
	if(!pStatObj)
	{
		m_nObjectTypeID = -1;
		return;
	}

  int nFind = CBrush::m_lstBrushTypes.Find(pStatObj);
  if(nFind>=0)
  { // found
    m_nObjectTypeID = nFind;
  }
  else
  {
    m_nObjectTypeID = CBrush::m_lstBrushTypes.Count();
    CBrush::m_lstBrushTypes.Add(pStatObj);
  }

  if(pMatrix)
    SetMatrix(pMatrix);
}

void CBrush::PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime)
{
	if(!GetEntityStatObj(0))
		return;

	if(GetCVars()->e_stream_cgf)
		((CStatObj*)GetEntityStatObj(0))->StreamCCGF(false);

	if(!GetEntityStatObj(0)->GetLeafBuffer())
		return;

	float fDist = fPrevPortalDistance + m_vPos.GetDistance(vPrevPortalPos);

	float fMaxViewDist = GetMaxViewDist();
	if(fDist<fMaxViewDist && fDist<GetViewCamera().GetZMax())
		GetEntityStatObj(0)->PreloadResources(fDist,fTime,0);


	for(int nLod=0; nLod<MAX_BRUSH_LODS_NUM; nLod++)
#if !defined(LINUX64)
	if(m_arrLMData[nLod].m_pLMData != NULL && m_arrLMData[nLod].m_pLMData->GetColorLerpTex() && m_arrLMData[nLod].m_pLMData->GetDomDirectionTex())
#else
	if(m_arrLMData[nLod].m_pLMData != 0 && m_arrLMData[nLod].m_pLMData->GetColorLerpTex() && m_arrLMData[nLod].m_pLMData->GetDomDirectionTex())
#endif
	{
		{
			ITexPic * pTexPic = GetRenderer()->EF_GetTextureByID(m_arrLMData[nLod].m_pLMData->GetColorLerpTex());
			if(pTexPic)
				GetRenderer()->EF_PrecacheResource(pTexPic, 0, 1.f, 0);
		}
		{
			ITexPic * pTexPic = GetRenderer()->EF_GetTextureByID(m_arrLMData[nLod].m_pLMData->GetDomDirectionTex());
			if(pTexPic)
				GetRenderer()->EF_PrecacheResource(pTexPic, 0, 1.f, 0);
		}
	}
}