////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain_water_quad.cpp
//  Version:     v1.00
//  Created:     28/8/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Create and draw terrain water geometry (screen space grid, cycle buffers)
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "terrain_water.h"
#include "cbuffer.h"
#include "3dengine.h"
#include "objman.h"
#include "visareas.h"

// Shader params ids.
enum {
	SHP_REFLECT_AMMOUNT,
	SHP_BUMP_AMMOUNT_X,
	SHP_BUMP_AMMOUNT_Y
};

CWaterOcean::CWaterOcean(IShader * pTerrainWaterShader, int nBottomTexId, IShader * pSunRoadShader, float fWaterTranspRatio, float fWaterReflectRatio, float fWaterBumpAmountX, float fWaterBumpAmountY, float fWaterBorderTranspRatio)
{ 
  memset(m_pLeafBufferWaters,0,sizeof(m_pLeafBufferWaters));
  m_pLeafBufferBottom = m_pLeafBufferSunRoad = 0;
  
  m_pTerrainWaterShader       = pTerrainWaterShader;
  m_pSunRoadShader            = pSunRoadShader;    
  m_pTerrainWaterBottomShader = GetRenderer()->EF_LoadShader("TerrainWaterBottomSimple", eSH_World, EF_SYSTEM);

  m_fWaterTranspRatio   = fWaterTranspRatio;
  m_fWaterReflectRatio  = fWaterReflectRatio;
  m_fWaterBumpAmountX   = fWaterBumpAmountX;
  m_fWaterBumpAmountY   = fWaterBumpAmountY;
  m_fWaterBorderTranspRatio = fWaterBorderTranspRatio;
	m_nBottomTexId = nBottomTexId;
	m_fLastFov=0;

	//////////////////////////////////////////////////////////////////////////
	// Cache console variables.
	r_WaterRefractions =  GetConsole()->GetCVar("r_WaterRefractions");
	r_WaterReflections = GetConsole()->GetCVar("r_WaterReflections");
	r_Quality_BumpMapping = GetConsole()->GetCVar("r_Quality_BumpMapping");

  int nWRefl = 0;
  int nWRefr = 0;
  if(r_WaterReflections)
    nWRefl = r_WaterReflections->GetIVal();
  if(r_WaterRefractions)
    nWRefr = r_WaterRefractions->GetIVal();


  // default shaders
  if(!m_pTerrainWaterShader)
  {
    if(!nWRefl && !nWRefr)
      m_pTerrainWaterShader       = GetRenderer()->EF_LoadShader("TerrainWater_FP", eSH_World, EF_SYSTEM);
    else
      m_pTerrainWaterShader       = GetRenderer()->EF_LoadShader("terrainwater", eSH_World, EF_SYSTEM);
  }
  if(!m_pSunRoadShader)
    m_pSunRoadShader            = GetRenderer()->EF_LoadShader("BumpSunGlow", eSH_World, EF_SYSTEM);
//  if(!m_pTerrainWaterBottomShader)
  //  m_pTerrainWaterBottomShader = GetRenderer()->EF_LoadShader("terrainwaterbottom", eSH_World, 0);

	if( GetRenderer()->GetFeatures() & RFT_OCCLUSIONTEST )
		m_pShaderOcclusionQuery = GetRenderer()->EF_LoadShader("OcclusionTest", eSH_World, EF_SYSTEM);
	else
		m_pShaderOcclusionQuery = 0;

	memset(m_pREOcclusionQueries, 0, sizeof(m_pREOcclusionQueries));

  m_nFogVolumeId = -1;
  
	m_nLastVisibleFrameId = 0;

	//////////////////////////////////////////////////////////////////////////
	// Initialize shader params for ocean.
	//////////////////////////////////////////////////////////////////////////
	SShaderParam pr;
	pr.m_Type = eType_FLOAT;
	pr.m_Value.m_Float = 0;

	strcpy( pr.m_Name, "reflectamount" );
	m_shaderParams.AddElem(pr);
	strcpy( pr.m_Name, "bumpamountx" );
	m_shaderParams.AddElem(pr);
	strcpy( pr.m_Name, "bumpamounty" );
	m_shaderParams.AddElem(pr);
}

CWaterOcean::~CWaterOcean()
{
  for(int x=0; x<RECURSION_LEVELS_NUM; x++)
	{
		for(int y=0; y<CYCLE_BUFFERS_NUM; y++)
			GetRenderer()->DeleteLeafBuffer(m_pLeafBufferWaters[x][y]);

    for(int y=0; y<CYCLE_BUFFERS_NUM; y++)
      if(m_pREOcclusionQueries[x][y])
  			m_pREOcclusionQueries[x][y]->Release();
	}

  GetRenderer()->DeleteLeafBuffer(m_pLeafBufferBottom);
  GetRenderer()->DeleteLeafBuffer(m_pLeafBufferSunRoad);
}

// todo: use crymatrix functions instead
void CWaterOcean__MatMul4( float *product, const float *a, const float *b )
{
#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]
  int i;
  for (i=0; i<4; i++)
  {
    float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
    P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
    P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
    P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
    P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
  }
#undef A
#undef B
#undef P
}

void CWaterOcean__transform_point(float out[4], const float m[16], const float in[4])
{
#define M(row,col)  m[col*4+row]
  out[0] = M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
  out[1] = M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
  out[2] = M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
  out[3] = M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}
/*
void CWaterOcean::DrawWaterQuad(const int nRecursionLevel)
{	
  int nBufID = (GetFrameID() & 1);

  int nScreenY = GetRenderer()->GetHeight();
  int nScreenX = GetRenderer()->GetWidth();

	if(!nScreenY || !nScreenX)
		return;

  if (!m_pTerrainWaterShader)
    return;
  TArray <CRendElement *> *pREs = m_pTerrainWaterShader->GetREs();
  if (pREs && pREs->Num())
  {
    I3DEngine * p3DEngine = Get3DEngine();
    float fWaterLevel = p3DEngine->GetWaterLevel();

    CCObject * pObj = GetRenderer()->EF_GetObject(true);

    pObj->m_Trans.Clear();// = Vec3d(vCamPos.x,vCamPos.y,fWaterLevel);
    pObj->m_Angs.Clear();

    pObj->SetShaderFloat("ReflectAmount",m_fWaterReflectRatio);
    pObj->SetShaderFloat("BumpAmountX",m_fWaterBumpAmountX);
    pObj->SetShaderFloat("BumpAmountY",m_fWaterBumpAmountY);

    GetRenderer()->EF_AddEf(0, pREs->Get(0), m_pTerrainWaterShader, NULL, pObj, -1, NULL, (GetViewCamera().GetPos().z>fWaterLevel) ? eS_SeeThrough : eS_Water);
    return;
  }
  Verts_DWQ.Clear();
  Indices_DWQ.Clear();

  int nScreenXP = nScreenX/25;
  int nScreenYP = nScreenY/20;

	if(nScreenXP*25 != nScreenX)
		nScreenX += nScreenXP;

	if(nScreenYP*20 != nScreenY)
		nScreenY += nScreenYP;

	if(!nScreenYP || !nScreenXP)
		return;

  unsigned short nIdx = 0;
  int y_size = int(float(nScreenY)/(nScreenYP)+1.f);

  const Vec3d vCamPos = GetViewCamera().GetPos();

  I3DEngine * p3DEngine = Get3DEngine();
  float fWaterLevel = p3DEngine->GetWaterLevel();

  bool bWaterVisible = false;

  CCoverageBuffer * pCoverageBuffer = GetCVars()->e_cbuffer ? ((C3DEngine*)p3DEngine)->GetObjManager()->m_pCoverageBuffer : 0;

  // prepare inverted combined matrix
  int   matViewport[4] = {0,0,GetRenderer()->GetWidth(),GetRenderer()->GetHeight()};
  float matCombinedInv[16];
  {
    float matModel[16];
    float matProj[16];
    float matCombined[16];
    GetRenderer()->GetModelViewMatrix(matModel);
    GetRenderer()->GetProjectionMatrix(matProj);
    CWaterOcean__MatMul4(matCombined,matProj,matModel);
    invertMatrixf(matCombinedInv, matCombined);
  }

  struct_VERTEX_FORMAT_P3F_COL4UB vert;
  vert.r = (unsigned char)(255);
  vert.g = (unsigned char)(255);
  vert.b = (unsigned char)(255);

//  if(pCoverageBuffer)
  for(int x=0; x<=nScreenX/(nScreenXP)*(nScreenXP); x+=int(nScreenXP))
  for(int y=0; y<=nScreenY/(nScreenYP)*(nScreenYP); y+=int(nScreenYP))
  {
    Vec3d n, p;

    n.Set((float)x,(float)y,0);

    { // UnProject From The Screen( n[0], n[1], n[2], &p[0], &p[1], &p[2] );
      // matrix transformation
      float in[4], out[4];
      in[0] = (n.x - matViewport[0]) * 2 / matViewport[2] - 1.0f;
      in[1] = (n.y - matViewport[1]) * 2 / matViewport[3] - 1.0f;
      in[2] = 2.0f * n.z - 1.0f;
      in[3] = 1.0;

      CWaterOcean__transform_point(out, matCombinedInv, in);
      assert(out[3] != 0.0);

      p.x = out[0] / out[3];
      p.y = out[1] / out[3];
      p.z = out[2] / out[3];
    }

    float z1 = vCamPos.z - p.z;
    float t = 1000000;
    if(z1<0.0001f)
      z1=0.0001f;

    {
      float z2 = vCamPos.z - fWaterLevel;
      t = z2/z1;
      if( t > 300*(vCamPos.z - fWaterLevel) )
        t = 300*(vCamPos.z - fWaterLevel);
      p = p - vCamPos;
      p = vCamPos + p*t;
    }

    vert.x=p.x;
    vert.y=p.y;
    vert.z=(p3DEngine->GetTerrainElevation(vert.x,vert.y)+fWaterLevel*3)*0.25f;// affects on alpha

    if(vert.z<=fWaterLevel)
      if(!pCoverageBuffer || pCoverageBuffer->IsPixelVisible(x+(nScreenXP>>1),y+(nScreenYP>>1)))
        bWaterVisible=true;

    float fAlpha = (fWaterLevel) - vert.z;
    fAlpha = CLAMP(fAlpha*3, 0.0f, 1.0f);
    fAlpha *= m_fWaterTranspRatio;
    fAlpha += t*0.002f;
    fAlpha = CLAMP(fAlpha, 0.0f, 1.0f);
    vert.a = (unsigned char)(fAlpha*255);

  //  vert.z = (vert.z + fWaterLevel*9)*0.1f;
///    if( x==nScreenX/(nScreenXP)*(nScreenXP) || y==nScreenY/(nScreenYP)*(nScreenYP) || x==0 || y==0 )
      vert.z=fWaterLevel;

    Verts_DWQ.Add(vert);

    if(x<nScreenX/(nScreenXP)*(nScreenXP))
    if(y<nScreenY/(nScreenYP)*(nScreenYP))
    if(fabs(p.z-fWaterLevel)<1)
    {
      unsigned short nIdx2 = nIdx+y_size;

      unsigned short _nIdx = nIdx+1;
      unsigned short _nIdx2 = (nIdx+y_size)+1;

      Indices_DWQ.Add(_nIdx);
      Indices_DWQ.Add(nIdx);
      Indices_DWQ.Add(nIdx2);

      Indices_DWQ.Add(_nIdx);
      Indices_DWQ.Add(nIdx2);
      Indices_DWQ.Add(_nIdx2);
    }

    nIdx++;
  }

  if(Verts_DWQ.Count() && Indices_DWQ.Count() && bWaterVisible)
  {
    if(m_pLeafBufferWaters[nRecursionLevel][nBufID] && 
      m_pLeafBufferWaters[nRecursionLevel][nBufID]->m_pSecVertBuffer->m_vertexformat == VERTEX_FORMAT_P3F_COL4UB &&
      m_pLeafBufferWaters[nRecursionLevel][nBufID]->m_nPrimetiveType == R_PRIMV_TRIANGLES &&
      m_nScreenX == nScreenX && m_nScreenY == nScreenY &&
			m_pLeafBufferWaters[nRecursionLevel][nBufID]->m_SecVertCount >= Verts_DWQ.Count())
    {
      if(m_pLeafBufferWaters[nRecursionLevel][nBufID]->GetIndices().Count() != Indices_DWQ.Count())
        m_pLeafBufferWaters[nRecursionLevel][nBufID]->UpdateIndices(&Indices_DWQ);

      if(Verts_DWQ.Count())
        m_pLeafBufferWaters[nRecursionLevel][nBufID]->UpdateVertices(&Verts_DWQ[0],Verts_DWQ.Count());
    }
    else
    {
      if(m_pLeafBufferWaters[nRecursionLevel][nBufID])
        GetRenderer()->DeleteLeafBuffer(m_pLeafBufferWaters[nRecursionLevel][nBufID]);

      m_pLeafBufferWaters[nRecursionLevel][nBufID] = GetRenderer()->CreateLeafBufferInitialized(
        &Verts_DWQ[0], Verts_DWQ.Count(),VERTEX_FORMAT_P3F_COL4UB,
        &Indices_DWQ,R_PRIMV_TRIANGLES,"OutdoorWaterQuad");
      
      m_nScreenX = nScreenX;
      m_nScreenY = nScreenY;
    }

    m_pLeafBufferWaters[nRecursionLevel][nBufID]->SetChunk(m_pTerrainWaterShader, 0, Verts_DWQ.Count(),0, Indices_DWQ.Count());

    CCObject * pObj = GetRenderer()->EF_GetObject(true);

    pObj->m_Trans.Clear();// = Vec3d(vCamPos.x,vCamPos.y,fWaterLevel);
    pObj->m_Angs.Clear();

    pObj->SetShaderFloat("ReflectAmount",m_fWaterReflectRatio);
    pObj->SetShaderFloat("BumpAmountX",m_fWaterBumpAmountX);
    pObj->SetShaderFloat("BumpAmountY",m_fWaterBumpAmountY);

		if( GetCVars()->e_hw_occlusion_culling && (GetRenderer()->GetFeatures() & RFT_OCCLUSIONTEST) )
		{
			// make element if not ready
			if(!m_pREOcclusionQueries[nRecursionLevel][nBufID])
				m_pREOcclusionQueries[nRecursionLevel][nBufID] = 
				(CREOcclusionQuery *)GetRenderer()->EF_CreateRE(eDATA_OcclusionQuery);

			// get last test result
			bWaterVisible = m_pREOcclusionQueries[nRecursionLevel][nBufID]->m_nVisSamples>20;

			// request new test
			float fWaterPlaneSize = GetViewCamera().GetZMax()*2;
			m_pREOcclusionQueries[nRecursionLevel][nBufID]->m_vBoxMin.Set( vCamPos.x-fWaterPlaneSize, vCamPos.y-fWaterPlaneSize,fWaterLevel);
			m_pREOcclusionQueries[nRecursionLevel][nBufID]->m_vBoxMax.Set( vCamPos.x+fWaterPlaneSize, vCamPos.y+fWaterPlaneSize,fWaterLevel);
			GetRenderer()->EF_AddEf(0, m_pREOcclusionQueries[nRecursionLevel][nBufID], m_pShaderOcclusionQuery, 0);
		}

		if(bWaterVisible)
			m_pLeafBufferWaters[nRecursionLevel][nBufID]->AddRE(pObj,0,(GetViewCamera().GetPos().z>fWaterLevel) ? eS_SeeThrough : eS_Water);
  }
}	*/

int CWaterOcean::GetMemoryUsage() 
{
	int nSize=0;
	
	nSize += Indices_DWQ.GetMemoryUsage();
	nSize += Verts_DWQ.GetMemoryUsage();
	
	return nSize;
}

void CWaterOcean::Render(const int nRecursionLevel)
{
	if(nRecursionLevel>=RECURSION_LEVELS_NUM)
		return;

  Vec3d vCamPos = GetViewCamera().GetPos();

	if(GetCVars()->e_water_ocean_tesselation>2)
		GetCVars()->e_water_ocean_tesselation=2;
	const int nChunksNum = 16*(1+GetCVars()->e_water_ocean_tesselation);

	lstFirstIdxId.Clear();
  Verts_DWQ.Clear();
  Indices_DWQ.Clear();

	float fScale1 = 1.f;
	float fScale2 = 1.f;

	I3DEngine * p3DEngine = Get3DEngine();
  float fWaterLevel = p3DEngine->GetWaterLevel();

	if(vCamPos.z < fWaterLevel)
	{ // if camera is in indoors and lower than ocean level 
		// and exit portals are higher than ocean level - skip ocean rendering
		CVisArea * pVisArea = (CVisArea *)p3DEngine->GetVisAreaFromPos(vCamPos);
		if(pVisArea && !pVisArea->IsPortal())
		{
			int i;
			for(i=0; i<pVisArea->m_lstConnections.Count(); i++)
				if(pVisArea->m_lstConnections[i]->IsConnectedToOutdoor() && pVisArea->m_lstConnections[i]->m_vBoxMin.z<fWaterLevel)
					break; // there is portal making ocean visible

			if(i==pVisArea->m_lstConnections.Count())
				return; // ocean surface is not visible 
		}
	}

  int nBufID = (GetFrameID() & 1);

  CCObject * pObject = GetRenderer()->EF_GetObject(true);
  pObject->m_Matrix.SetIdentity();

  bool bWaterVisible = IsWaterVisible();
  // test visibility
  if( fabs(m_fLastFov - GetViewCamera().GetFov())<0.01f && GetCVars()->e_hw_occlusion_culling_water && (GetRenderer()->GetFeatures() & RFT_OCCLUSIONTEST) && (nRecursionLevel==0))
  {
    // make element if not ready
    if(!m_pREOcclusionQueries[nRecursionLevel][nBufID])
      m_pREOcclusionQueries[nRecursionLevel][nBufID] = 
      (CREOcclusionQuery *)GetRenderer()->EF_CreateRE(eDATA_OcclusionQuery);

    // get last test result
    if(abs(m_pREOcclusionQueries[nRecursionLevel][nBufID]->m_nCheckFrame - GetFrameID())<4)
    {
      if(m_pREOcclusionQueries[nRecursionLevel][nBufID]->m_nVisSamples>20)
      {
				m_nLastVisibleFrameId = GetFrameID();
        bWaterVisible = true;
      }
    }

    // request new test
    float fWaterPlaneSize = GetViewCamera().GetZMax()*2;
    m_pREOcclusionQueries[nRecursionLevel][nBufID]->m_vBoxMin( vCamPos.x-fWaterPlaneSize, vCamPos.y-fWaterPlaneSize,fWaterLevel+0.1f);
    m_pREOcclusionQueries[nRecursionLevel][nBufID]->m_vBoxMax( vCamPos.x+fWaterPlaneSize, vCamPos.y+fWaterPlaneSize,fWaterLevel+0.1f);
    GetRenderer()->EF_AddEf(0, m_pREOcclusionQueries[nRecursionLevel][nBufID], m_pShaderOcclusionQuery, NULL, NULL, 0, 0, eS_FogShader);
  }
  else
	{
		m_nLastVisibleFrameId = GetFrameID();
    bWaterVisible = true;
	}

	m_fLastFov = GetViewCamera().GetFov();

//	if(bWaterVisible)
//	m_nLastVisibleFrameId = GetFrameID();

  bool bGF2 = ((GetRenderer()->GetFeatures() & RFT_HW_MASK) == RFT_HW_GF2) != 0;

  float fA;
  // Calculate water geometry and update vertex buffers
  if(bWaterVisible)
  {
    struct_VERTEX_FORMAT_P3F_COL4UB tmp;

    bool bOnlyTransparency = true;

    if(r_WaterRefractions && r_WaterRefractions->GetIVal() != 0)
      bOnlyTransparency = false;
    if(r_WaterReflections && r_WaterReflections->GetIVal() != 0)
      bOnlyTransparency = false;
    if(r_Quality_BumpMapping && r_Quality_BumpMapping->GetIVal() == 0)
      bOnlyTransparency = true;

  //changed by ivo
	//we use the ambient color of the level to modulate the waterocean-color	
	 Vec3 ambient = Get3DEngine()->GetAmbientColorFromPosition(Vec3(0,0,0));

	 if (bGF2 || bOnlyTransparency)
    {
      tmp.color.bcolor[0] = (uint8)(ambient.z*255.0f);
      tmp.color.bcolor[1] = (uint8)(ambient.y*255.0f);
      tmp.color.bcolor[2] = (uint8)(ambient.x*255.0f);
      fA = 255;
    }
    else
    {
      tmp.color.bcolor[0] = 255;
      tmp.color.bcolor[1] = 255;
      tmp.color.bcolor[2] = 255;
      fA = 255;
    }

	const float fStep = 1.f/(1.f+GetCVars()->e_water_ocean_tesselation);

	float fFresnel = 0.003f/(vCamPos.z - fWaterLevel);

	for(float r1=0, r2=fStep; r2<=nChunksNum; r1+=fStep, r2+=fStep)
	{
		if(r1*fScale1>GetViewCamera().GetZMax())
			break;

		lstFirstIdxId.Add(Indices_DWQ.Count());

    float fTranspPlus1 = max(0, r1/nChunksNum-0.5f)*2;
    float fTranspPlus2 = max(0, r2/nChunksNum-0.5f)*2;

		int nRotStep = 15-GetCVars()->e_water_ocean_tesselation*5;
		for(int i=0; i<=360; i+=nRotStep)
		{
			float rad = (i) * (gf_PI/180);
      float fAlpha;

			// vert 1
			tmp.xyz.x = cry_sinf(rad)*(r2)*fScale2 + vCamPos.x;
			tmp.xyz.y = cry_cosf(rad)*(r2)*fScale2 + vCamPos.y;
			tmp.xyz.z = fWaterLevel;//max(0,fWaterLevel-r2*0.125f);

			fAlpha = fWaterLevel - p3DEngine->GetTerrainElevation(tmp.xyz.x,tmp.xyz.y);
			fAlpha = CLAMP(fAlpha*m_fWaterBorderTranspRatio, 0.0f, 1.0f);
			//fAlpha *= m_fWaterTranspRatio;
			fAlpha += r2*0.0125f*fScale2;
			fAlpha = CLAMP(fAlpha, 0.0f, min(1.f,m_fWaterTranspRatio+fTranspPlus2));
			fAlpha += r2*fScale2*fFresnel;
			tmp.color.bcolor[3] = (unsigned char)fastftol_positive(min(255,fAlpha*fA));

			Indices_DWQ.Add(Verts_DWQ.Count());
			Verts_DWQ.Add(tmp);

			// vert 2
			tmp.xyz.x = cry_sinf(rad)*(r1)*fScale1 + vCamPos.x;
			tmp.xyz.y = cry_cosf(rad)*(r1)*fScale1 + vCamPos.y;
			tmp.xyz.z = fWaterLevel;//max(0,fWaterLevel-r1*0.125f);

			fAlpha = fWaterLevel - p3DEngine->GetTerrainElevation(tmp.xyz.x,tmp.xyz.y);
			fAlpha = CLAMP(fAlpha*m_fWaterBorderTranspRatio, 0.0f, 1.0f);
//			fAlpha *= m_fWaterTranspRatio;
			fAlpha += r1*0.0125f*fScale1;
			fAlpha = CLAMP(fAlpha, 0.0f, min(1.f,m_fWaterTranspRatio+fTranspPlus1));
			fAlpha += r1*fScale1*fFresnel;
			tmp.color.bcolor[3] = (unsigned char)fastftol_positive(min(255,fAlpha*fA));

			Indices_DWQ.Add(Verts_DWQ.Count());
			Verts_DWQ.Add(tmp);
		}

		fScale1 = fScale2;
		fScale2 *= 1.4f;
	}

	lstFirstIdxId.Add(Indices_DWQ.Count());

  if(	m_pLeafBufferWaters[nRecursionLevel][nBufID] && 
			m_pLeafBufferWaters[nRecursionLevel][nBufID]->m_pSecVertBuffer->m_vertexformat == VERTEX_FORMAT_P3F_COL4UB &&
			m_pLeafBufferWaters[nRecursionLevel][nBufID]->m_nPrimetiveType == R_PRIMV_MULTI_STRIPS &&
			m_pLeafBufferWaters[nRecursionLevel][nBufID]->m_SecVertCount >= Verts_DWQ.Count() &&
			m_pLeafBufferWaters[nRecursionLevel][nBufID]->m_pMats->Count() == lstFirstIdxId.Count()-1)
  {
    if(m_pLeafBufferWaters[nRecursionLevel][nBufID]->m_Indices.m_nItems != Indices_DWQ.Count())
      m_pLeafBufferWaters[nRecursionLevel][nBufID]->UpdateSysIndices(Indices_DWQ.GetElements(), Indices_DWQ.Count());

    if(Verts_DWQ.Count())
      m_pLeafBufferWaters[nRecursionLevel][nBufID]->UpdateSysVertices(&Verts_DWQ[0],Verts_DWQ.Count());
  }
  else
  {
    if(m_pLeafBufferWaters[nRecursionLevel][nBufID])
      GetRenderer()->DeleteLeafBuffer(m_pLeafBufferWaters[nRecursionLevel][nBufID]);

		m_pLeafBufferWaters[nRecursionLevel][nBufID] = GetRenderer()->CreateLeafBufferInitialized(
			Verts_DWQ.GetElements(), Verts_DWQ.Count(), VERTEX_FORMAT_P3F_COL4UB, 
			Indices_DWQ.GetElements(), Indices_DWQ.Count(), R_PRIMV_MULTI_STRIPS,
			"OutdoorWaterCircle", eBT_Dynamic,nChunksNum, 0x1000);

		for(int i=0; i<nChunksNum && i<(lstFirstIdxId.Count()-1); i++)
		{
			m_pLeafBufferWaters[nRecursionLevel][nBufID]->SetChunk(m_pTerrainWaterShader,
				Indices_DWQ[lstFirstIdxId[i]],Verts_DWQ.Count()/(lstFirstIdxId.Count()-1),
				lstFirstIdxId[i],lstFirstIdxId[i+1]-lstFirstIdxId[i], i);
		}
  }

		//Timur, This is much faster then SetShaderFloat
		m_shaderParams[SHP_REFLECT_AMMOUNT].m_Value.m_Float = m_fWaterReflectRatio;
		m_shaderParams[SHP_BUMP_AMMOUNT_X].m_Value.m_Float = m_fWaterBumpAmountX;
		m_shaderParams[SHP_BUMP_AMMOUNT_Y].m_Value.m_Float = m_fWaterBumpAmountY;
		pObject->m_ShaderParams = &m_shaderParams;

    //pObject->SetShaderFloat("ReflectAmount",m_fWaterReflectRatio);
    //pObject->SetShaderFloat("BumpAmountX",m_fWaterBumpAmountX);
    //pObject->SetShaderFloat("BumpAmountY",m_fWaterBumpAmountY);

    // make distance to water level near to zero
    m_pLeafBufferWaters[nRecursionLevel][nBufID]->m_vBoxMin = vCamPos;
    m_pLeafBufferWaters[nRecursionLevel][nBufID]->m_vBoxMax = vCamPos;

    int nFogId = (GetViewCamera().GetPos().z<fWaterLevel && m_nFogVolumeId>0) ? m_nFogVolumeId : 0;

		if(!m_nRenderStackLevel)
		{
			pObject->m_nScissorX1 = GetViewCamera().m_ScissorInfo.x1;
			pObject->m_nScissorY1 = GetViewCamera().m_ScissorInfo.y1;
			pObject->m_nScissorX2 = GetViewCamera().m_ScissorInfo.x2;
			pObject->m_nScissorY2 = GetViewCamera().m_ScissorInfo.y2;
		}

		if(GetCVars()->e_water_ocean_sun_reflection)
		if(((C3DEngine*)Get3DEngine())->GetDynamicLightSources()->Count())
		{
			CDLight * pSun = ((C3DEngine*)Get3DEngine())->GetDynamicLightSources()->Get(0);
			pObject->m_DynLMMask = (pSun && pSun->m_Flags & DLF_SUN) ? (1<<pSun->m_Id) : 0;
		}

		m_pLeafBufferWaters[nRecursionLevel][nBufID]->AddRenderElements(pObject,0,-1,nFogId,(GetViewCamera().GetPos().z>fWaterLevel) ? eS_SeeThrough : eS_Water);
	}

	// draw bottom (use same water geometry)
	if(m_pTerrainWaterBottomShader)
	if((bWaterVisible || GetViewCamera().GetPos().z<fWaterLevel))
	{
/*		UCol col;
		col.dcolor = (DWORD)-1;
		float fMaxViewDist = Get3DEngine()->GetTerrainSize();
		((C3DEngine*)Get3DEngine())->GetObjManager()->AddPolygonToRenderer( 
				((C3DEngine*)Get3DEngine())->GetTerrain()->m_pLowLodCoverMapTex->GetTextureID(),
				m_pTerrainWaterBottomShader, 
				0, 
				Vec3d(fMaxViewDist*2,0,0), Vec3d(0,fMaxViewDist*2,0), 
				col,
				ParticleBlendType_None, 
				Vec3d(1.f,1.f,1.f), 
				Vec3d(fMaxViewDist/2,fMaxViewDist/2,0) );
	*/
/*		CCObject * pBottomObject = GetRenderer()->EF_GetObject(true);
    Matrix34 m = Matrix34::GetTransMat34(Vec3d(0,0,-1));
    pBottomObject->m_Matrix = Matrix33::GetScale33(Vec3d(1,1,0))* m;
    int nFogId = (m_nFogVolumeId>0) ? m_nFogVolumeId : 0;
		m_pLeafBufferWaters[nRecursionLevel][nBufID]->SetShader(m_pTerrainWaterBottomShader);
		m_pLeafBufferWaters[nRecursionLevel][nBufID]->AddRenderElements(pBottomObject,0,-1,nFogId);
		m_pLeafBufferWaters[nRecursionLevel][nBufID]->SetShader(m_pTerrainWaterShader);*/
		RenderBottom(nRecursionLevel);
	}
}

// render fog plane circle if camera is inside volume
void CWaterOcean::RenderBottom(int nRecursionLevel)
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_3DENGINE );
	if(nRecursionLevel)
		return;

	Vec3d vCamPos = GetViewCamera().GetPos();

	const int nChunksNum = 16;
//	float fScale = GetViewCamera().GetZMax()/nChunksNum;

//	GetRenderer()->DeleteLeafBuffer(m_pLeafBufferBottom);
	//m_pLeafBufferBottom=0;

	float fWaterLevel = Get3DEngine()->GetWaterLevel();
	float fBottomZOffset = 0.1f;

	if(!m_pLeafBufferBottom)
	{
		struct_VERTEX_FORMAT_P3F tmp;
		list2<struct_VERTEX_FORMAT_P3F> lstVertices; 
		list2<ushort> lstIndices;
		list2<ushort> lstFirstIdxId;

		for(float r1=0, r2=1.f; r2<=nChunksNum; r1+=1.f, r2+=1.f)
		{
			lstFirstIdxId.Add(lstIndices.Count());

			float fZ1 = (r1+1.f>nChunksNum) ? fWaterLevel : -fBottomZOffset;
			float fZ2 = (r2+1.f>nChunksNum) ? fWaterLevel : -fBottomZOffset;

			for(int i=0; i<=360; i+=30)
			{
				float rad = (i) * (gf_PI/180);

				tmp.xyz.x = cry_sinf(rad)*(r2);
				tmp.xyz.y = cry_cosf(rad)*(r2);
				tmp.xyz.z = fZ2;

				lstIndices.Add(lstVertices.Count());
				lstVertices.Add(tmp);

				tmp.xyz.x = cry_sinf(rad)*(r1);
				tmp.xyz.y = cry_cosf(rad)*(r1);
				tmp.xyz.z = fZ1;

				lstIndices.Add(lstVertices.Count());
				lstVertices.Add(tmp);
			}
		}

		lstFirstIdxId.Add(lstIndices.Count());

		if(m_pLeafBufferBottom)
			GetRenderer()->DeleteLeafBuffer(m_pLeafBufferBottom);

		m_pLeafBufferBottom = GetRenderer()->CreateLeafBufferInitialized(
			lstVertices.GetElements(), lstVertices.Count(), VERTEX_FORMAT_P3F, 
			lstIndices.GetElements(), lstIndices.Count(), R_PRIMV_MULTI_STRIPS,
			"OceanBottomCircle", eBT_Dynamic, nChunksNum, m_nBottomTexId);

		for(int i=0; i<nChunksNum; i++)
		{
			m_pLeafBufferBottom->SetChunk(m_pTerrainWaterBottomShader,
				lstIndices[lstFirstIdxId[i]],lstVertices.Count()/(lstFirstIdxId.Count()-1),
				lstFirstIdxId[i],lstFirstIdxId[i+1]-lstFirstIdxId[i], i);
		}
	}

	float fScale = GetViewCamera().GetZMax()/nChunksNum;
	CCObject * pObj = GetRenderer()->EF_GetObject(true);	
	pObj->m_Matrix.SetTranslationMat(Vec3d(vCamPos.x, vCamPos.y, 
		vCamPos.z>fWaterLevel ? -fBottomZOffset : 0.f));
	pObj->m_Matrix = Matrix33::CreateScale(Vec3d(fScale,fScale,1.f))*pObj->m_Matrix;
  pObj->m_ObjFlags |= FOB_TRANS_TRANSLATE | FOB_TRANS_SCALE;
	if(!m_nRenderStackLevel)
	{
		pObj->m_nScissorX1 = GetViewCamera().m_ScissorInfo.x1;
		pObj->m_nScissorY1 = GetViewCamera().m_ScissorInfo.y1;
		pObj->m_nScissorX2 = GetViewCamera().m_ScissorInfo.x2;
		pObj->m_nScissorY2 = GetViewCamera().m_ScissorInfo.y2;
	}

	int nFogId = (m_nFogVolumeId>0) ? m_nFogVolumeId : 0;
	m_pLeafBufferBottom->AddRenderElements(pObj, 0, -1, nFogId);
}

bool CWaterOcean::IsWaterVisible()
{
	const Vec3d & vCamPos = GetViewCamera().GetPos();
	return (vCamPos.z<Get3DEngine()->GetWaterLevel()+0.5f) || (GetFrameID() - m_nLastVisibleFrameId)<100;
}