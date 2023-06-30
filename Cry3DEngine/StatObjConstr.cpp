////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjconstr.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: creation
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "MeshIdx.h"
#include "../RenderDll/Common/shadow_renderer.h"
#include <irenderer.h>
#include <CrySizer.h>
#include "objman.h"

float CStatObj::m_fStreamingTimePerFrame=0;

void CStatObj::FreeTriData()
{
	delete m_pTriData;
	m_pTriData=0;
}

const char * CStatObj::GetScriptMaterialName(int Id)
{
  CLeafBuffer *lb = m_pLeafBuffer;
  if (Id < 0)
  {
    for (int i=0; i<lb->m_pMats->Count(); i++)
    {
      if ((*lb->m_pMats)[i].sScriptMaterial[0])
        return (*lb->m_pMats)[i].sScriptMaterial;
    }
    return NULL;
  }
  else
  if (Id < lb->m_pMats->Count() && (*lb->m_pMats)[Id].sScriptMaterial[0])
    return (*lb->m_pMats)[Id].sScriptMaterial;

  return NULL;
}

void CStatObj::CalcRadiuses()
{
	m_vBoxCenter = (m_vBoxMax+m_vBoxMin)*0.5f;
  m_fObjectRadius = GetDistance(m_vBoxMin, m_vBoxMax)*0.5f;
  float dxh = (float)max( fabs(GetBoxMax().x), fabs(GetBoxMin().x));
  float dyh = (float)max( fabs(GetBoxMax().y), fabs(GetBoxMin().y));
  m_fRadiusHors = (float)cry_sqrtf(dxh*dxh+dyh*dyh);
  m_fRadiusVert = (GetBoxMax().z - GetBoxMin().z)*0.5f;// never change this
}

CStatObj::CStatObj( ) 
{ 
  m_nUsers = 0; // referense counter
	m_bOpenEdgesTested = false;

	m_eVertsSharing=evs_NoSharing;
	m_bLoadAdditinalInfo=false;
	m_bKeepInLocalSpace=false;
	m_bUseStreaming=false;
  m_bMakePhysics=false;
	m_nSpriteTexRes=0;

  ZeroStruct( m_szFolderName );
  ZeroStruct( m_szFileName );
  ZeroStruct( m_szGeomName );

	m_pStencilShadowConnectivity=0;

	m_nLastRendFrameId = 0;
  m_nMarkedForStreamingFrameId = 0;
  m_fBackSideLevel = 1.f;
  m_bCalcLighting = false;

  memset(m_arrpLowLODs,0,sizeof(m_arrpLowLODs));
  m_nLoadedLodsNum=1;

	m_bPhysicsExistInCompiledFile=false;

	Init();
}

void CStatObj::Init() 
{
	m_pTriData = 0;
	m_nLoadedTrisCount = 0;
  m_fObjectRadius = 0;
	m_fRadiusHors = 0;
	m_fRadiusVert = 0;
	m_pSvObj=NULL;
	m_dwFlags=m_dwFlags2=0;

  ZeroStruct( m_arrSpriteTexID );

	m_vBoxMin.Set(0,0,0); 
	m_vBoxMax.Set(0,0,0); 
	m_vBoxCenter.Set(0,0,0);
	memset(m_arrPhysGeomInfo, 0, sizeof(m_arrPhysGeomInfo));

  m_pSMLSource = 0;

  m_pLeafBuffer = 0;

	m_bDefaultObject=false;

  for(int i=0; i<MAX_STATOBJ_LODS_NUM; i++)
    if(m_arrpLowLODs[i])
      m_arrpLowLODs[i]->Init();

	m_eCCGFStreamingStatus = ecss_NotLoaded;
	m_pReadStream=0;
	m_bCompilingNotAllowed=0;
}

CStatObj::~CStatObj() 
{ 
	ShutDown();

  for(int i=0; i<MAX_STATOBJ_LODS_NUM; i++)
    if(m_arrpLowLODs[i])
      delete m_arrpLowLODs[i];
}

void CStatObj::ShutDown() 
{
//	assert (IsHeapValid());
	m_pReadStream=0;

  if(m_pTriData)
    m_pTriData->FreeLMInfo();

//	assert (IsHeapValid());

  delete m_pTriData; 
	m_pTriData = 0;

//	assert (IsHeapValid());

  for(int n=0; n<2; n++)
  if(m_arrPhysGeomInfo[n])
    GetPhysicalWorld()->GetGeomManager()->UnregisterGeometry(m_arrPhysGeomInfo[n]);

//	assert (IsHeapValid());

  if(m_pSMLSource && m_pSMLSource->m_LightFrustums.Count() && m_pSMLSource->m_LightFrustums[0].pModelsList)
    delete m_pSMLSource->m_LightFrustums[0].pModelsList;
  delete m_pSMLSource;

//	assert (IsHeapValid());

	if(m_pLeafBuffer && !m_pLeafBuffer->m_bMaterialsWasCreatedInRenderer)
  {
    for (int i=0; i<(*m_pLeafBuffer->m_pMats).Count(); i++)
    {		
      CMatInfo *mi = m_pLeafBuffer->m_pMats->Get(i);
      if(mi->pRE)
        mi->pRE->Release();
      if (mi->shaderItem.m_pShader)
        mi->shaderItem.m_pShader->Release();
      if (mi->shaderItem.m_pShaderResources)
        mi->shaderItem.m_pShaderResources->Release();
    }
		delete m_pLeafBuffer->m_pMats;
		m_pLeafBuffer->m_pMats=0;
  }

//	assert (IsHeapValid());

  GetRenderer()->DeleteLeafBuffer(m_pLeafBuffer);
	m_pLeafBuffer=0;

//	assert (IsHeapValid());

  for(int i=0; i<FAR_TEX_COUNT; i++)
		if(m_arrSpriteTexID[i])
		  GetRenderer()->RemoveTexture(m_arrSpriteTexID[i]);

	if (m_pSvObj)
	{
//		assert (IsHeapValid());
		m_pSvObj->Release();
		m_pSvObj=NULL;
	}
  
  for(int i=0; i<MAX_STATOBJ_LODS_NUM; i++)
  {
    if(m_arrpLowLODs[i])
      m_arrpLowLODs[i]->ShutDown();
  }

  m_ShaderParams.Free();

	// free light source smart pointers
	for(int i=0; i<m_lstLSources.Count(); i++)
	{ 
		m_lstLSources[i].m_pLightImage = NULL;
		m_lstLSources[i].m_pShader = NULL;
	}
}

/*
void CStatObj::BuildOcTree()
{
  if(!m_pTriData->m_nFaceCount || m_pTriData->m_nFaceCount<=2)
    return;

  CBox parent_box( m_pTriData->m_vBoxMin, m_pTriData->m_vBoxMax );
  parent_box.max += 0.01f;
  parent_box.min +=-0.01f;

  CObjFace ** allFaces = new CObjFace *[m_pTriData->m_nFaceCount];

  for(int f=0; f<m_pTriData->m_nFaceCount; f++)
  {
    m_pTriData->m_pFaces[f].m_vCenter = 
     (Vec3d(&m_pTriData->m_pVerts[m_pTriData->m_pFaces[f].v[0]].x) + 
      Vec3d(&m_pTriData->m_pVerts[m_pTriData->m_pFaces[f].v[1]].x) + 
      Vec3d(&m_pTriData->m_pVerts[m_pTriData->m_pFaces[f].v[2]].x))/3.f;

    allFaces[f] = &m_pTriData->m_pFaces[f];
  }

  const int max_tris_in_leaf = 2000;
  const float leaf_min_size  = stricmp(m_szGeomName,"sector_0") ? 16.f : 32.f;

  text_to_log("  Generating octree ... ");
  m_pOcTree = new octree_node( &parent_box, allFaces, triData->m_nFaceCount, triData, leaf_min_size, max_tris_in_leaf, 2);
  text_to_log_plus("%d leafs created", octree_node::static_current_leaf_id);
  m_pOcTree->update_bbox(triData);

  delete [] allFaces;
}*/

//float M akeBuffersTime = 0;

void CStatObj::MakeLeafBuffer(bool bSortAndShareVerts)
{
	if(GetSystem()->IsDedicated())
		return;

	assert(!m_pLeafBuffer);
	m_pLeafBuffer = GetRenderer()->CreateLeafBuffer(eBT_Static,"StatObj");

  m_pLeafBuffer->m_pMats = new list2<CMatInfo>;
	m_pLeafBuffer->m_pMats->AddList(m_pTriData->m_lstMatTable);

  if(bSortAndShareVerts)
    bSortAndShareVerts=bSortAndShareVerts;

  if(m_pTriData->m_nFaceCount)
		m_pLeafBuffer->CreateBuffer(m_pTriData, bSortAndShareVerts, true, //false,);
				m_pTriData->m_lstGeomNames.Count()>0 && strstr(m_pTriData->m_lstGeomNames[0],"cloth")!=0);

	// remove materials without geometry
	if(m_bIgnoreFakeMaterialsInCGF)
	if(m_pLeafBuffer && !m_pLeafBuffer->m_bMaterialsWasCreatedInRenderer)
	{
		for (int i=0; i<(*m_pLeafBuffer->m_pMats).Count(); i++)
		{		
			CMatInfo *mi = m_pLeafBuffer->m_pMats->Get(i);
			if(!mi->pRE || !mi->nNumIndices || !mi->nNumVerts)
			{
				if (mi->shaderItem.m_pShader)
					mi->shaderItem.m_pShader->Release();
				if (mi->shaderItem.m_pShaderResources)
					mi->shaderItem.m_pShaderResources->Release();
				m_pLeafBuffer->m_pMats->Delete(i);
				
				if(i<m_lstShaderTemplates.Num())
					m_lstShaderTemplates.DelElem(i);
				i--;
			}
			else
				mi->pRE->m_pChunk = mi;
		}
	}

	int nDeleted = m_pTriData->m_lstMatTable.Count() - m_pLeafBuffer->m_pMats->Count();
	if(nDeleted)
		CryLogComment("  %d not used material slots removed", nDeleted);
}

//////////////////////////////////////////////////////////////////////////
int CStatObj::GetAllocatedBytes()
{
  int size = sizeof(*this) + m_pTriData ? m_pTriData->GetAllocatedBytes() : 0;

//  for(int i=0; i<MAX_TREE_LEAFS_NUM; i++)
  {
    size += m_pLeafBuffer->GetAllocatedBytes(false);
  }

  return size;
} 

///////////////////////////////////////////////////////////////////////////////////////    
Vec3d CStatObj::GetHelperPos(const char * szHelperName)
{
  for(int i=0; i<m_lstHelpers.Count(); i++)
  if(!strcmp(m_lstHelpers[i].sName,szHelperName))
    return m_lstHelpers[i].tMat.GetTranslationOLD();

  return Vec3d(0,0,0);
}

///////////////////////////////////////////////////////////////////////////////////////    
const Matrix44 * CStatObj::GetHelperMatrixByName(const char * szHelperName)
{
  for(int i=0; i<m_lstHelpers.Count(); i++)
  if(!strcmp(m_lstHelpers[i].sName,szHelperName))
    return &(m_lstHelpers[i].tMat);

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////    
const char *CStatObj::GetHelperById(int nId, Vec3d & vPos, Matrix44* pMat, int * pnType)
{
  if ( nId >= m_lstHelpers.Count() || nId<0 )
    return (NULL);

	vPos   = m_lstHelpers[nId].tMat.GetTranslationOLD();

  if(pnType)
		*pnType = m_lstHelpers[nId].nType;

	if(pMat)
		*pMat   = m_lstHelpers[nId].tMat;

  return (m_lstHelpers[nId].sName);
}

/*
bool CStatObj::GetHelper(int id, char * szHelperName, int nMaxHelperNameSize, Vec3d * pPos, Vec3d * pRot)
{
  if(id<0 || id>=triData->m_Helpers.Count())
    return false;

  strncpy(szHelperName, triData->m_Helpers[id].name, nMaxHelperNameSize);
  *pPos = triData->m_Helpers[id].pos;
  *pRot = triData->m_Helpers[id].rot;
  return true;
} */

void CStatObj::UpdateCustomLightingSpritesAndShadowMaps(Vec3d vStatObjAmbientColor, int nTexRes, float fBackSideLevel, bool bCalcLighting)
{
	AUTO_PROFILE_SECTION(GetTimer(), CObjManager::m_dUpdateCustomLightingSpritesAndShadowMaps);

	m_nSpriteTexRes = nTexRes;
  m_fBackSideLevel = fBackSideLevel;
  m_bCalcLighting = bCalcLighting;
  Vec3d vLight = GetSystem()->GetI3DEngine()->GetSunPosition();
  vLight.Normalize();
//  Vec3d vColor = GetSystem()->GetI3DEngine()->GetWorldColor();
  float fSize = m_vBoxMax.z - m_vBoxMin.z;

  // update lighting for full lod and lower lods
	if(m_pLeafBuffer)
		m_pLeafBuffer->UpdateCustomLighting( m_fBackSideLevel, vStatObjAmbientColor, vLight, bCalcLighting );  

  int nLowestLod=0;
  for(int nLodLevel=1; nLodLevel<MAX_STATOBJ_LODS_NUM; nLodLevel++)
  if(m_arrpLowLODs[nLodLevel])
  {
    if(m_arrpLowLODs[nLodLevel]->GetLeafBuffer())
		{
			m_arrpLowLODs[nLodLevel]->GetLeafBuffer()->UpdateCustomLighting( m_fBackSideLevel, vStatObjAmbientColor, vLight, bCalcLighting );
			nLowestLod = nLodLevel;
		}
  }

  // make sprites
  if(nLowestLod)
  {
    // clear sprites in full lod
    for(int i=0; i<FAR_TEX_COUNT; i++)
      if(m_arrSpriteTexID[i])
    {
      GetRenderer()->RemoveTexture(m_arrSpriteTexID[i]);
      m_arrSpriteTexID[i]=0;
    }

    // make new sprites in low lod
    m_arrpLowLODs[nLowestLod]->CreateModelFarImages(nTexRes); // use lowest lod if present

    // move sprite id from low inro into full lod
    memcpy(m_arrSpriteTexID, m_arrpLowLODs[nLowestLod]->m_arrSpriteTexID, sizeof(m_arrSpriteTexID));
    memset(m_arrpLowLODs[nLowestLod]->m_arrSpriteTexID, 0, sizeof(m_arrpLowLODs[nLowestLod]->m_arrSpriteTexID));
		m_nSpriteTexRes = m_arrpLowLODs[nLowestLod]->m_nSpriteTexRes;
  }
  else
    CreateModelFarImages(nTexRes);


  MakeShadowMaps(vLight);

//  if(m_pTriData && !m_pTriData->m_lstLSources.Count())
	//	FreeTriData();
}

/*
const char * CStatObj::GetPhysMaterialName(int nMatID)
{
  if(m_pLeafBuffer && m_pLeafBuffer->m_pMats && nMatID < m_pLeafBuffer->m_pMats->Count())
    return m_pLeafBuffer->m_pMats->Get(nMatID)->szPhysMat;

  return 0;
}

bool CStatObj::SetPhysMaterialName(int nMatID, const char * szPhysMatName)
{
  if(m_pLeafBuffer && m_pLeafBuffer->m_pMats && nMatID < m_pLeafBuffer->m_pMats->Count())
  {
    strncpy(m_pLeafBuffer->m_pMats->Get(nMatID)->szPhysMat, szPhysMatName, sizeof(m_pLeafBuffer->m_pMats->Get(nMatID)->szPhysMat));
    return true;
  }

  return false;
}
*/

void CStatObj::RegisterUser()
{
  m_nUsers++;
}

void CStatObj::UnregisterUser()
{
  m_nUsers--;
}

float CStatObj::GetDistFromPoint(const Vec3d & vPoint)
{
  float fMinDist = 4096;
  for(int v=0; v<m_pTriData->m_nVertCount; v++)
  {
    float fDist = GetDistance(m_pTriData->m_pVerts[v],vPoint);
    if(fDist < fMinDist)
      fMinDist = fDist;
  }

  return fMinDist;
}

bool CStatObj::IsSameObject(const char * szFileName, const char * szGeomName)
{
	// cmp object names
	if (szGeomName)
	{
		// [Anton] - always use new cgf for objects used for cloth simulation
		if (*szGeomName && stricmp(szGeomName,"cloth") == 0)
			return false;

		if(stricmp(szGeomName,m_szGeomName)!=0)
			return false;
	}

  // Normilize file name
	char szFileNameNorm[MAX_PATH_LENGTH]="";
	char *pszDest = szFileNameNorm;
	const char *pszSource = szFileName;
	while (*pszSource)
	{
		if (*pszSource=='/')
			*pszDest++='\\';
		else 
			*pszDest++=*pszSource;
		pszSource++;
	}
	*pszDest=0;

	// cmp file names
	if(stricmp(szFileNameNorm,m_szFileName)!=0)
		return false;

	return true;
}

void CStatObj::GetMemoryUsage(ICrySizer* pSizer)
{
	pSizer->AddObject(this,GetMemoryUsage());
}

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

int CStatObj::GetMemoryUsage()
{
	int nSize=0;
	
	for(int i=0; i<MAX_STATOBJ_LODS_NUM; i++)
		if(m_arrpLowLODs[i])
			nSize += m_arrpLowLODs[i]->GetMemoryUsage();

	nSize += m_lstHelpers.GetMemoryUsage();
	nSize += m_lstLSources.GetMemoryUsage();
	nSize += m_lstOcclVolInds.GetMemoryUsage();
	nSize += m_lstOcclVolVerts.GetMemoryUsage();
	nSize += m_lstShaderTemplates.GetMemoryUsage();
	nSize += m_pSMLSource ? sizeof(*m_pSMLSource) : 0;
	nSize += m_pSvObj ? m_pSvObj->GetMemoryUsage() : 0;
	nSize += m_pTriData ? m_pTriData->GetMemoryUsage() : 0;
	nSize += m_ShaderParams.GetMemoryUsage() + m_ShaderParams.Num()*sizeof(SShaderParam);
	
	return nSize;
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

bool CStatObj::CheckValidVegetation()
{
  if(m_pLeafBuffer && m_pLeafBuffer->m_pMats)
  for(int i=0; i<m_pLeafBuffer->m_pMats->Count(); i++)
  {
    CMatInfo * pMatInfo = m_pLeafBuffer->m_pMats->Get(i);
    IShader * pShader = pMatInfo->shaderItem.m_pShader;
    if(!pShader || !pMatInfo->nNumIndices)
      continue;

    pShader = pShader->GetTemplate(0);

    const char * pTemplateName = pShader->GetName();
    char buff[64];
    strncpy(buff, pTemplateName, sizeof(buff)); buff[sizeof(buff)-1]=0;
    strlwr(buff);
    if( !strstr(buff,"templplants") && 
        !strstr(buff,"nodraw") &&
				!strstr(buff,"templdecal_vcolors") &&
				!strstr(buff,"templdecalalphatest_vcolors"))
    {
#if !defined(LINUX)//don't worry, we won't render anything under linux
			Warning( 0,m_szFileName,"CStatObj::CheckValidVegetation: Shader template is undefined or can not be used for vegetations: %s [%s]", 
        pTemplateName, m_szFileName);
//      return false;
#endif
    }
  }

  return true;
}

bool CStatObj::EnableLightamapSupport()
{
	assert(m_eVertsSharing == evs_NoSharing);

  if(m_eVertsSharing == evs_NoSharing)
    return true;
             /*
  GetLog()->Log("Activating lightamap support for %s", m_szFileName);

  m_eVertsSharing = evs_NoSharing;

  Refresh(FRO_GEOMETRY);
         */
  return false;//!m_bDefaultObject;
}

IStatObj * CStatObj::GetLodObject(int nLodLevel)
{
	if(nLodLevel<1)
		return this;

	if(nLodLevel<3)
		return (IStatObj *)m_arrpLowLODs[nLodLevel];

	return 0;
}

bool CStatObj::IsPhysicsExist()
{
	return m_bPhysicsExistInCompiledFile || GetPhysGeom(0) || GetPhysGeom(1) || GetPhysGeom(2);
}

int CStatObj::GetRenderTrisCount() 
{ 
	int nCount=0;
	if(GetLeafBuffer())
		GetLeafBuffer()->GetIndices(&nCount); 

	return nCount/3;
}

bool CStatObj::IsSphereOverlap(const Sphere& sSphere)
{
	if(m_pLeafBuffer && Overlap::Sphere_AABB(sSphere,AABB(m_vBoxMin,m_vBoxMax)))
	{ // if inside bbox
		int nInds = 0, nPosStride=0;
		ushort *pInds = m_pLeafBuffer->GetIndices(&nInds);
		const byte * pPos = m_pLeafBuffer->GetPosPtr(nPosStride,0,true);
		
		if(pInds && pPos)
		for(int i=0; (i+2)<nInds; i+=3)
		{	// test all triangles of water surface strip
			Vec3d v0 = *(Vec3d*)&pPos[nPosStride*pInds[i+0]];
			Vec3d v1 = *(Vec3d*)&pPos[nPosStride*pInds[i+1]];
			Vec3d v2 = *(Vec3d*)&pPos[nPosStride*pInds[i+2]];
			Vec3d vBoxMin = v0; vBoxMin.CheckMin(v1); vBoxMin.CheckMin(v2);
			Vec3d vBoxMax = v0; vBoxMax.CheckMax(v1); vBoxMax.CheckMax(v2);

			if(	Overlap::Sphere_AABB(sSphere,AABB(vBoxMin,vBoxMax)))
				return true;
		}
	}
	
	return false;
}
