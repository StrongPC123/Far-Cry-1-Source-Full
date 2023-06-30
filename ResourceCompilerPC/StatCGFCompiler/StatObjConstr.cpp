////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjconstr.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: loading
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "MeshIdx.h"
#include "../RenderDll/Common/shadow_renderer.h"
#include <irenderer.h>
#include <I3dIndoorEngine.h>
#include <CrySizer.h>

//#define USE_CCGF

float CStatObj::m_fStreamingTimePerFrame=0;

void CStatObj::Refresh(int nFlags)
{
	if(nFlags & FRO_GEOMETRY)
	{
		bool bSpritesWasCreated = IsSpritesCreated();
		ShutDown();
		Init();
		bool bRes = LoadObject(m_szFileName, m_szGeomName[0] ? m_szGeomName : 0, m_nStripify, m_bLoadAdditinalInfo, m_bKeepInLocalSpace);
		if(bRes && bSpritesWasCreated)
		{
			Vec3d vColor = Get3DEngine()->GetAmbientColorFromPosition(Vec3d(-1000,-1000,-1000));
			UpdateCustomLightingSpritesAndShadowMaps(vColor.x, m_nSpriteTexRes);
		}

		if(!bRes)
		{ // load default in case of error
			ShutDown();
			Init();
			LoadObject("Objects\\default.cgf", 0, m_nStripify, m_bLoadAdditinalInfo, m_bKeepInLocalSpace);
		}
	
		return;
	}

  if (nFlags & (FRO_TEXTURES | FRO_SHADERS))
  {
    CLeafBuffer *lb = m_pLeafBuffer;
    
    for (int i=0; i<lb->m_pMats->Count(); i++)
    {
      IShader *e = (*lb->m_pMats)[i].shaderItem.m_pShader;
      if (e && (*lb->m_pMats)[i].pRE && (*lb->m_pMats)[i].nNumIndices)
        e->Reload(nFlags);
    }
  }
}

bool CStatObj::LoadObject(const char * szFileName, 
                          const char*szGeomName, 
                          int nStripify,
                          bool bLoadAdditinalInfo,
													bool bKeepInLocalSpace,
													bool bLoadLater)
{ 
	if(!szFileName[0])
	{
		GetLog()->Log("Error: CStatObj::LoadObject: szFileName not specified");
		return 0;
	}

	m_nStripify          = nStripify;
	m_bLoadAdditinalInfo = bLoadAdditinalInfo;
	m_bKeepInLocalSpace  = bKeepInLocalSpace;
	m_bStreamable				 = bLoadLater;

	if(bLoadLater)
	{ // define fake bbox
		Init();

		m_vBoxMin = Vec3d(-1.f,-1.f,-1.f);
		m_vBoxMax = Vec3d( 1.f, 1.f, 1.f);
		m_vBoxCenter = Vec3d(0,0,0);
		m_fRadiusHors = m_fRadiusVert = 1.f;

		// remember names
		strcpy(m_szFileName,szFileName);
	  
		if(szGeomName)
			strcpy(m_szGeomName,szGeomName);
		else
			m_szGeomName[0]=0;

		strcpy(m_szFolderName,szFileName);
		while(m_szFolderName[0])
		{ // make folder name
			if(m_szFolderName[strlen(m_szFolderName)-1] == '\\' || m_szFolderName[strlen(m_szFolderName)-1] == '/')
			{ m_szFolderName[strlen(m_szFolderName)-1]=0; break; }
			m_szFolderName[strlen(m_szFolderName)-1]=0;
		}  

		m_nLoadedTrisCount = 0;

		return true;
	}

	FILE * f = 0;

#ifdef USE_CCGF

	char szCompiledFileNameFull[512];
	{
		char szCompiledFileName[512]="";
		strcpy(szCompiledFileName,szFileName);

		while(strstr(szCompiledFileName,"\\") || strstr(szCompiledFileName,"/"))
			strcpy(szCompiledFileName,szCompiledFileName+1);

		while(strstr(szCompiledFileName,"."))
			szCompiledFileName[strlen(szCompiledFileName)-1]=0;

		strcat( szCompiledFileName, "_" );
		strcat( szCompiledFileName, szGeomName ? szGeomName : "NoGeom" );
		strcat( szCompiledFileName, ".ccgf" );
	  
		snprintf(szCompiledFileNameFull, "CCGF\\%s", szCompiledFileName);
	}

	f = fopen(szCompiledFileNameFull, "rb");

#endif // USE_CCGF
                         
  if(!f || szGeomName)
  { // compile object and save to disk
		strcpy(m_szFileName,szFileName);
	  
		if(szGeomName)
			strcpy(m_szGeomName,szGeomName);
		else
			m_szGeomName[0]=0;

		strcpy(m_szFolderName,szFileName);
		while(m_szFolderName[0])
		{ // make folder name
			if(m_szFolderName[strlen(m_szFolderName)-1] == '\\' || m_szFolderName[strlen(m_szFolderName)-1] == '/')
			{ m_szFolderName[strlen(m_szFolderName)-1]=0; break; }
			m_szFolderName[strlen(m_szFolderName)-1]=0;
		}  

		m_nLoadedTrisCount = 0;
		m_pTriData = new CIndexedMesh( m_pSystem, szFileName, szGeomName, &m_nLoadedTrisCount, bLoadAdditinalInfo, bKeepInLocalSpace );
		if(!m_nLoadedTrisCount)
		{
			if(!szGeomName)
				return false;

			int i;
			for(i=0; i<m_pTriData->m_lstGeomNames.Count(); i++)
			if(strcmp(m_pTriData->m_lstGeomNames[i],szGeomName)==0)
				break;

			if(i>=m_pTriData->m_lstGeomNames.Count())
				return false;
		}

		m_vBoxMin = m_pTriData->m_vBoxMin;
		m_vBoxMax = m_pTriData->m_vBoxMax;
		m_vBoxCenter = (m_vBoxMax+m_vBoxMin)/2;

		// copy helpers
		m_lstHelpers.AddList(*m_pTriData->GetHelpers());

		// copy lsources
		for(int i=0; i<m_pTriData->GetLightSourcesList()->Count(); i++)
			m_lstLSources.Add(*m_pTriData->GetLightSourcesList()->GetAt(i));

		InitParams(m_pTriData->m_vBoxMax.z - m_pTriData->m_vBoxMin.z);

		Physicalize(); // can change some indices/faces

		// create vert buffers
		if(m_nLoadedTrisCount>30000)
			GetLog()->UpdateLoadingScreen("  Indexing huge vertex buffer ...");

		MakeBuffers(szGeomName!=0, nStripify, 0);

		for (int i=0; m_pLeafBuffer && m_pLeafBuffer->m_pMats && i<m_pLeafBuffer->m_pMats->Count(); i++)
			m_lstShaderTemplates.Add(-1);

		if(m_nLoadedTrisCount>30000)
			GetLog()->UpdateLoadingScreen(" Indexed OK");
	
#ifdef USE_CCGF

		delete m_pTriData;
		m_pTriData=0;

		CreateDirectory("CCGF", 0);

		// Save to file
    int nPos = 0;
		Serialize(nPos, 0, true, m_szFolderName);

    uchar * pData = new uchar[nPos];
		nPos=0;
    Serialize(nPos, pData, true, m_szFolderName);

    f = fopen(szCompiledFileNameFull,"wb");
		if(f)
			fwrite(pData,1,nPos,f);
    delete pData;

	}
	else
  { // load ready object from disk
		GetLog()->UpdateLoadingScreen("Loading compiled object: %s", szCompiledFileNameFull);

    fseek(f,0,SEEK_END);
    int nSize = ftell(f);
    fseek(f,0,SEEK_SET);

    uchar * pData = new uchar[nSize];
    int nReadedBytes = fread(pData,1,nSize,f);
		if(nReadedBytes != nSize)
			GetConsole()->Exit("Error: CStatObj::LoadObject: Error reading ccfg: %s", szCompiledFileNameFull);

		nSize=0;
		Serialize(nSize, pData, false, m_szFolderName);
		assert(nReadedBytes == nSize);

    delete pData;

#endif // USE_CCGF
  }

	if(f)
		fclose(f);

//	if(!szGeomName) // m_pTriData is needed only for indoors
	//	FreeTriData();

//	buildStencilShadowConnectivity (Get3DEngine()->GetNewStaticConnectivityBuilder(), 0, 0);

	return true;
}

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

void CStatObj::InitParams(float sizeZ)
{
  m_fObjectRadius = GetDistance(m_vBoxMin, m_vBoxMax)/2;

  // calc vert/horis radiuses
/*  float dxh = GetBoxMax().x - GetBoxMin().x;
  float dyh = GetBoxMax().y - GetBoxMin().y;
  m_fRadiusHors = sqrtf(dxh*dxh+dyh*dyh)/2;
  m_fRadiusVert = GetBoxMax().z/2;*/

  float dxh = (float)max( fabs(GetBoxMax().x), fabs(GetBoxMin().x));
  float dyh = (float)max( fabs(GetBoxMax().y), fabs(GetBoxMin().y));
  m_fRadiusHors = (float)sqrt(dxh*dxh+dyh*dyh);
  m_fRadiusVert = 0.01f + (GetBoxMax().z - GetBoxMin().z)*0.5f;
}

CStatObj::CStatObj(ISystem	* pSystem) 
{ 
  m_pSystem = pSystem;
  m_nUsers = 0; // referense counter

	m_nStripify=0;
	m_bLoadAdditinalInfo=false;
	m_bKeepInLocalSpace=false;
	m_bStreamable=false;
	m_nSpriteTexRes=0;

  ZeroStruct( m_szFolderName );
  ZeroStruct( m_szFileName );
  ZeroStruct( m_szGeomName );

	m_pStencilShadowConnectivity=0;

	m_nLastRendFrameId = 0;

	Init();
}

void CStatObj::Init() 
{
	m_pTriData = 0;
	m_nLoadedTrisCount = 0;
  m_fObjectRadius = 0;
	m_pSvObj=NULL;
	m_dwFlags=m_dwFlags2=0;

  ZeroStruct( m_arrSpriteTexID );

	m_vBoxMin.Set(0,0,0); 
	m_vBoxMax.Set(0,0,0); 
	m_vBoxCenter.Set(0,0,0);
	memset(m_arrPhysGeomInfo, 0, sizeof(m_arrPhysGeomInfo));

  m_pSMLSource = 0;

  m_pLeafBuffer = 0;//GetRenderer()->CreateLe afBuffer("StatObj");

	m_bDefaultObject=false;

  memset(m_arrpLowLODs,0,sizeof(m_arrpLowLODs));

  m_nLoadedLodsNum=1;
}

CStatObj::~CStatObj() 
{ 
	ShutDown();
}

void CStatObj::ShutDown() 
{
	if(!m_pSystem)
		return;

  if(m_pTriData)
    m_pTriData->FreeLMInfo();
  delete m_pTriData; 
	m_pTriData = 0;

  for(int n=0; n<2; n++)
  if(m_arrPhysGeomInfo[n])
    GetPhysicalWorld()->GetGeomManager()->UnregisterGeometry(m_arrPhysGeomInfo[n]);

  if(m_pSMLSource && m_pSMLSource->m_LightFrustums.Count() && m_pSMLSource->m_LightFrustums[0].pModelsList)
    delete m_pSMLSource->m_LightFrustums[0].pModelsList;
  delete m_pSMLSource;

	if(m_pLeafBuffer && !m_pLeafBuffer->m_bMaterialsWasCreatedInRenderer)
  {
    for (int i=0; i<(*m_pLeafBuffer->m_pMats).Count(); i++)
    {		
      if((*m_pLeafBuffer->m_pMats)[i].pRE)
        (*m_pLeafBuffer->m_pMats)[i].pRE->Release();
    }
		delete m_pLeafBuffer->m_pMats;
		m_pLeafBuffer->m_pMats=0;
  }

  GetRenderer()->DeleteLeafBuffer(m_pLeafBuffer);
	m_pLeafBuffer=0;

  for(int i=0; i<FAR_TEX_COUNT; i++)
		if(m_arrSpriteTexID[i])
		  GetRenderer()->RemoveTexture(m_arrSpriteTexID[i]);

	if (m_pSvObj)
	{
		m_pSvObj->Release();
		m_pSvObj=NULL;
	}
  
  for(int i=0; i<MAX_STATOBJ_LODS_NUM; i++)
    delete m_arrpLowLODs[i];

  m_ShaderParams.Free();
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

//float MakeBuffersTime = 0;

void CStatObj::MakeBuffers(bool make_tree, int nStripify, char * szCompiledFileName)
{
//  float fTimeStart = GetTimer()->GetAsyncCurTime();
/*
  FILE * f = fopen(szCompiledFileName,"rb");
                         
  if(f)
  {
    fseek(f,0,SEEK_END);
    int nSize = ftell(f);
    fseek(f,0,SEEK_SET);

    uchar * pData = new uchar[nSize];
    int nReadedBytes = fread(pData,1,nSize,f);
    m_pLeafBuffer->Serialize(nSize, pData, false, m_szFolderName, m_nEFT_Flags);

    delete pData;
  }
  else*/
  {
		assert(!m_pLeafBuffer);
	  m_pLeafBuffer = GetRenderer()->CreateLeafBuffer(eBT_Static,"StatObj");

    m_pLeafBuffer->m_pMats = new list2<CMatInfo>;
		m_pLeafBuffer->m_pMats->AddList(m_pTriData->m_lstMatTable);

    if(m_pTriData->m_nFaceCount)
		{
//      m_pLeafBuffer->CreateBuffer(m_pTriData, nStripify, true);
		m_pLeafBuffer->CreateBuffer(m_pTriData, STRIPTYPE_NONE, false ); // no sorting for lightmaps 
		}

        /*
    int nSize = 0;
    m_pLeafBuffer->Serialize(nSize, 0, true, m_szFolderName, m_nEFT_Flags);

    uchar * pData = new uchar[nSize];
    m_pLeafBuffer->Serialize(nSize, pData, true, m_szFolderName, m_nEFT_Flags);

    f = fopen(szCompiledFileName,"wb");
    fwrite(pData,1,nSize,f);
    delete pData;*/
  }

  //fclose(f);

//  MakeBuffersTime += (GetTimer()->GetAsyncCurTime() - fTimeStart);
}

//////////////////////////////////////////////////////////////////////////
void CStatObj::MakeLeafBuffer( CIndexedMesh *mesh,bool bStripify )
{
	m_pTriData = mesh;
	if (m_pLeafBuffer)
	{
		// Delete old leaf buffer.
		GetRenderer()->DeleteLeafBuffer(m_pLeafBuffer);
	}
	m_pLeafBuffer = GetRenderer()->CreateLeafBuffer(eBT_Static,"StatObj");

	m_pLeafBuffer->m_pMats = new list2<CMatInfo>;
	m_pLeafBuffer->m_pMats->AddList(m_pTriData->m_lstMatTable);

	if(m_pTriData->m_nFaceCount)
	{
//		m_pLeafBuffer->CreateBuffer(m_pTriData, (bStripify)?1:0, true );
		m_pLeafBuffer->CreateBuffer(m_pTriData, STRIPTYPE_NONE, false ); // no sorting for lightmaps 
	}
}

//////////////////////////////////////////////////////////////////////////
CStatObj::GetAllocatedBytes()
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
    return m_lstHelpers[i].tMat.GetTranslation();

  return Vec3d(0,0,0);
}

///////////////////////////////////////////////////////////////////////////////////////    
const Matrix * CStatObj::GetHelperMatrixByName(const char * szHelperName)
{
  for(int i=0; i<m_lstHelpers.Count(); i++)
  if(!strcmp(m_lstHelpers[i].sName,szHelperName))
    return &(m_lstHelpers[i].tMat);

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////    
const char *CStatObj::GetHelperById(int nId, Vec3d & vPos, Matrix * pMat, int * pnType)
{
  if ( nId >= m_lstHelpers.Count() || nId<0 )
    return (NULL);

	vPos   = m_lstHelpers[nId].tMat.GetTranslation();

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

void CStatObj::UpdateCustomLightingSpritesAndShadowMaps(float fStatObjAmbientLevel, int nTexRes)
{
	m_nSpriteTexRes = nTexRes;
  Vec3d vLight = m_pSystem->GetI3DEngine()->GetSunPosition();
  vLight.Normalize();
//  Vec3d vColor = m_pSystem->GetI3DEngine()->GetWorldColor();
  float fSize = m_vBoxMax.z - m_vBoxMin.z;

  // update lighting for full lod and lower lods
  m_pLeafBuffer->UpdateCustomLighting( vLight, fSize, fStatObjAmbientLevel );  
  int nLowestLod=0;
  for(int nLodLevel=1; nLodLevel<MAX_STATOBJ_LODS_NUM; nLodLevel++)
  if(m_arrpLowLODs[nLodLevel])
  {
    m_arrpLowLODs[nLodLevel]->GetLeafBuffer()->UpdateCustomLighting( vLight, fSize, fStatObjAmbientLevel );  
    nLowestLod = nLodLevel;
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

void CStatObj::LoadLowLODs(int nStripify,bool bLoadAdditinalInfo,bool bKeepInLocalSpace)
{
  if(m_szGeomName[0])
    return;

  m_nLoadedLodsNum = 1;

  for(int nLodLevel=1; nLodLevel<MAX_STATOBJ_LODS_NUM; nLodLevel++)
  {
    // make lod file name
    char sLodFileName[512];
    strncpy(sLodFileName, m_szFileName, sizeof(m_szFileName));
    sLodFileName[strlen(sLodFileName)-4]=0;
    strcat(sLodFileName,"_lod");
    char sLodNum[8];
    ltoa(nLodLevel,sLodNum,10);
    strcat(sLodFileName,sLodNum);
    strcat(sLodFileName,".cgf");

    // try to load
	  m_arrpLowLODs[nLodLevel] = new CStatObj(m_pSystem);
    bool bRes = fxopen(sLodFileName,"r") && 
			m_arrpLowLODs[nLodLevel]->LoadObject(sLodFileName, 0, nStripify, bLoadAdditinalInfo, bKeepInLocalSpace);

    if(!bRes || m_arrpLowLODs[nLodLevel]->m_nLoadedTrisCount > m_nLoadedTrisCount / 1.8f)
    {
      if(bRes)
        GetLog()->Log("Error: CStatObj::LoadLowLODs: Low lod model contains too many polygons (more than half of original model, loading skipped): %s", sLodFileName);

      delete m_arrpLowLODs[nLodLevel];
      m_arrpLowLODs[nLodLevel]=0;
      break;
    }
    m_nLoadedLodsNum++;
  }
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
/*
// SetHideability and SetBending will be removed from here
#include "objman.h"
#include "3dengine.h"

void CStatObj::SetHideability(int nHideability) 
{ 
	m_nHideability = nHideability; 

	list2<StatInstGroup> & TypeList = ((C3DEngine*)Get3DEngine())->GetObjManager()->m_lstStaticTypes;
	for(int i=0; i<TypeList.Count(); i++)
		if(TypeList[i].pStatObj == this)
			TypeList[i].bHideability = (nHideability!=0);
}

void CStatObj::SetBending(float fBending) 
{ 
	m_fBending = fBending; 

	list2<StatInstGroup> & TypeList = ((C3DEngine*)Get3DEngine())->GetObjManager()->m_lstStaticTypes;
	for(int i=0; i<TypeList.Count(); i++)
		if(TypeList[i].pStatObj == this)
			TypeList[i].fBending = fBending;
}
*/

void CStatObj::GetMemoryUsage(ICrySizer* pSizer)
{
	pSizer->AddObject(this,GetMemoryUsage());
}

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