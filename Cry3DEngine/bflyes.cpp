////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   bflyes.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: draw volume with bflyes right in front of the camera
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
/*
#include "bflyes.h"

Cry_Butterfly::Cry_Butterfly() 
{ 
  ZeroStruct(*this);

  m_vPos.x = rn()*BF_RANGE*2;
  m_vPos.y = rn()*BF_RANGE*2;

  m_fHigh = 1;

  m_fLifeSpeed = 1;// + rn()/2;
  m_fSize = 1;// + rn();
//    m_fLifeSpeed = 1.f/m_fSize/2;

  m_fSize *= 0.75f;
  
  {
    static int _X_ = 0;
    static int _Y_ = 0;
    _Y_++;
    if(_Y_>3)
    {
      _Y_=0;
      _X_=!_X_;
    }

    m_vCurWingPos.x = 0.25f*(_Y_);
    m_vCurWingPos.y = 0.25f*(2+_X_);
  }
}

bool Cry_Butterfly::IsPointInvalid(const Vec3d & pos)
{
//  if(pTerrain->GetZSafe(fastftol(pos.x),fastftol(pos.y)) < pTerrain->GetWaterLevel())
  //  return true;

  return false;
}

void Cry_Butterfly::Render(ITimer * pITimer, IRenderer * pIRenderer, const Vec3d & vCamPos, const bool & bEven, const Vec3d & vColor, struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pVertBufChunk, CTerrain * pTerrain )
{
  Vec3d start_pos = m_vPos;

  { // bf space loop
    if((m_vPos.x-vCamPos.x)>BF_RANGE)
    {
      while((m_vPos.x-vCamPos.x)>BF_RANGE)
        m_vPos.x-=BF_RANGEx2;

      if(IsPointInvalid(m_vPos))
        m_vPos.x+=BF_RANGEx2;
    }

    if((vCamPos.x-m_vPos.x)>BF_RANGE)
    {
      while((vCamPos.x-m_vPos.x)>BF_RANGE)
        m_vPos.x+=BF_RANGEx2;

      if(IsPointInvalid(m_vPos))
        m_vPos.x-=BF_RANGEx2;
    }

    if((m_vPos.y-vCamPos.y)>BF_RANGE)
    {
      while((m_vPos.y-vCamPos.y)>BF_RANGE)
        m_vPos.y-=BF_RANGEx2;

      if(IsPointInvalid(m_vPos))
        m_vPos.y+=BF_RANGEx2;
    }

    if((vCamPos.y-m_vPos.y)>BF_RANGE)
    {
      while((vCamPos.y-m_vPos.y)>BF_RANGE)
        m_vPos.y+=BF_RANGEx2;

      if(IsPointInvalid(m_vPos))
        m_vPos.y-=BF_RANGEx2;
    }
  }

  if(
		//m_vPos!=start_pos
		!IsEquivalent(m_vPos,start_pos,VEC_EPSILON)
		)
    m_fHigh = 0.5; // sit in the ea fix

////////////////////////////////////////////////////////////////////////////////////////////////////
// AI
////////////////////////////////////////////////////////////////////////////////////////////////////
  { 
    m_fAngle += m_fAngleDelta*fBFPerformanceFactor;    
    m_fHigh  += m_fHightDelta*fBFPerformanceFactor/15;
    
    if(bEven)
    {
      m_fAngleDelta += rn();
      Limit(&m_fAngleDelta,-3,3);
    }
    else
    {
      m_fHightDelta += rn();
      Limit(&m_fHightDelta,-1,1);
    }

    Limit(&m_fHigh, -0.25,5);
    
    bool water = pTerrain->GetZApr(m_vPos.x,m_vPos.y) < pTerrain->GetWaterLevel();
    if(water && m_fHigh<0.2f)
      m_fHigh=0.2f;

    if(m_fHigh>GROUND_LEVEL)
    { // fly
      if(m_fHigh<0.1f)
        m_fHigh=0.1f;

      float rad = (m_fAngle-90) * (gf_PI/180);

			Vec2 D; // the cosine and sine, y is guaranteed to follow x
			CosSin(rad, &D.x);

      m_vPos.x += D.x*fBFPerformanceFactor*0.05f*m_fLifeSpeed;
      m_vPos.y += D.y*fBFPerformanceFactor*0.05f*m_fLifeSpeed;
    
      m_vPos.z = pTerrain->GetZApr(m_vPos.x,m_vPos.y);
      if(m_vPos.z < pTerrain->GetWaterLevel())
        m_vPos.z = pTerrain->GetWaterLevel();
      m_vPos.z += m_fHigh;
    }
//    else
  //    pos.z += (pTerrain->GetZApr(pos.x,pos.y)+0.1f-pos.z)/100;
  }

  if(m_vPos.x>0 && m_vPos.y>0 && m_vPos.x<CTerrain::GetTerrainSize() && m_vPos.y<CTerrain::GetTerrainSize() &&
    m_vPos.z < (pTerrain->GetWaterLevel()+82) )// !IsPointInFuncZone(m_vPos.x,m_vPos.y))
  {
		struct {float c, s;}
			WingPos;

		CosSin(m_fWingPos, &WingPos.c);
		float c = WingPos.c;
		float s = WingPos.s;

    float wspeed = (1.1f*(float)fabs(c) + 0.25f)*(m_fLifeSpeed+m_fHightDelta*0.2f)/(1.f+(m_fHigh<GROUND_LEVEL)*7.f);

    if(m_bMoveDir)
      m_fWingPos += fBFPerformanceFactor*wspeed;
    else
      m_fWingPos -= fBFPerformanceFactor*wspeed;

    if(m_fWingPos> 1.2f) // up wind pos
      m_bMoveDir = 0;
    else if(m_fWingPos < ((m_fHigh<GROUND_LEVEL) ? 0.2f : -1.0f)) // down wind pos
      m_bMoveDir = 1; 

		float sh = 1;//0.5f + 0.5f*pTerrain->IsOnTheLight(fastftol(m_vPos.x),fastftol(m_vPos.y));

    uchar r = uchar(vColor[0]*sh);
    uchar g = uchar(vColor[1]*sh);
    uchar b = uchar(vColor[2]*sh);

  //  pIRenderer->RotateMatrix(-10,1,0,0);

    float R = 0.175f*m_fSize;

    float 
      x1=0      +m_vCurWingPos.x,
      x2=0.125f +m_vCurWingPos.x,
      x3=0.25f  +m_vCurWingPos.x;
    float 
      y1=1.f-(0      +m_vCurWingPos.y),
      y2=1.f-(0.125f +m_vCurWingPos.y),
      y3=1.f-(0.25f  +m_vCurWingPos.y);
    
    struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F data[6] = 
    {
      {-c*R, -R,  R*s  , r,g,b,255, x1,  y3},
      {-c*R,  R,  R*s  , r,g,b,255, x1,  y1},
      {   0, -R, -R*s/3, r,g,b,255, x2,  y3},
      {   0,  R, -R*s/3, r,g,b,255, x2,  y1},
      { c*R, -R,  R*s  , r,g,b,255, x3,  y3},
      { c*R,  R,  R*s  , r,g,b,255, x3,  y1}
    };

    if(m_fHigh<GROUND_LEVEL)
      data[2].z = data[3].z = 0;

    float rad = DEG2RAD(m_fAngle);
		struct {float c,s;} radCS;
		CosSin (rad, &radCS.c);
    float fCos = radCS.c;// (float)cos(rad);
    float fSin = radCS.s;// (float)sin(rad);

    struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pData = data;
    for(int i=0; i<6; i++)
    {
      float x = fCos*pData->x - fSin*pData->y;
      float y = fSin*pData->x + fCos*pData->y;

      pData->x = x;
      pData->y = y;

      pData->x+=m_vPos.x;
      pData->y+=m_vPos.y;
      pData->z+=m_vPos.z;

      pData++;
    }     

    memcpy(pVertBufChunk,data,sizeof(data));
  }
  else
  {
    memset(pVertBufChunk,0,sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F)*6);
  }
}

CBFManager::CBFManager()
{
  m_pBFArray = new Cry_Butterfly[MAX_BF_COUNT];
  m_Indixes.Reset();
  GetRenderer()->CreateIndexBuffer(&m_Indixes, NULL, MAX_BF_COUNT*12);
  GetRenderer()->UpdateIndexBuffer(&m_Indixes, NULL, 0, false);
  ushort *pInds = (ushort *)m_Indixes.m_VData;
  m_nTexID = 0;
  m_pVideoBuffer = 0;

  int i=0;
  for(int b=0; b<MAX_BF_COUNT; b++)
  {
    // 1st wing
    pInds[i++] = b*6;
    pInds[i++] = b*6+1;
    pInds[i++] = b*6+2;

    pInds[i++] = b*6+1;
    pInds[i++] = b*6+2;
    pInds[i++] = b*6+3;

    // 2nd
    pInds[i++] = b*6+2;
    pInds[i++] = b*6+3;
    pInds[i++] = b*6+4;

    pInds[i++] = b*6+3;
    pInds[i++] = b*6+4; 
    pInds[i++] = b*6+5;
//		assert(i<BF_COUNT*12);
  }
  GetRenderer()->UpdateIndexBuffer(&m_Indixes, NULL, 0, true);

  m_pVideoBuffer = GetRenderer()->CreateBuffer(MAX_BF_COUNT*6,VERTEX_FORMAT_P3F_COL4UB_TEX2F, "BFlyes");
  m_nCurrentObjectsCount = 0;
}

CBFManager::~CBFManager()
{
  delete [] m_pBFArray;
  GetRenderer()->ReleaseIndexBuffer(&m_Indixes);
  GetRenderer()->ReleaseBuffer(m_pVideoBuffer);
}

void CBFManager::Render(CTerrain * pTerrain)
{
  if(!GetCVars()->e_bflyes || m_nCurrentObjectsCount==0)
    return;

  IRenderer * rend = GetRenderer();
	rend->ResetToDefault();
  Vec3d vCamPos = GetViewCamera().GetPos();

  float terr_z = pTerrain->GetZSafe(vCamPos.x,vCamPos.y);  
  if(terr_z<pTerrain->GetWaterLevel())
    terr_z = pTerrain->GetWaterLevel();

  if(vCamPos.z > terr_z+48)
    return;  

  rend->SetMaterialColor(1,1,1,1);
  rend->SetColorOp(eCO_MODULATE, eCO_MODULATE, eCA_Texture|(eCA_Constant<<3), eCA_Texture|(eCA_Constant<<3));
  rend->SetState(GS_ALPHATEST_GEQUAL64 | GS_DEPTHWRITE);
  rend->SetCullMode(R_CULL_DISABLE);

  int tex_type;
  if(!m_nTexID)
    m_nTexID = rend->LoadTexture("Textures\\bfly.jpg", &tex_type);
//  else
  //  GetCVars()->e_bflyes = false;

  rend->SetTexture(m_nTexID);

  Matrix44 mat;
  rend->GetModelViewMatrix(mat.GetData());

	//CELL_CHANGED_BY_IVO
  //m_vForward(-mat.cell(2), -mat.cell(6), -mat.cell(10)); 
	m_vForward = -mat.GetColumn(2);

	m_vForward.Normalize();

  Vec3d vFocusPos = vCamPos + m_vForward*BF_CAMERA_SHIFT;

  Vec3d vColor = GetSystem()->GetI3DEngine()->GetWorldColor();//*
//    GetSystem()->GetI3DEngine()->GetWorldBrightnes();

  vColor.x*=255;
  vColor.y*=255;
  vColor.z*=255;

  // wait for fence
  GetRenderer()->UpdateBuffer(m_pVideoBuffer,0,0,false);

  for(int i=0; i<m_nCurrentObjectsCount && i<MAX_BF_COUNT; i++)
    m_pBFArray[i].Render(GetTimer(), GetRenderer(), vFocusPos, m_bEven, vColor, &((struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F*)m_pVideoBuffer->m_VS[VSF_GENERAL].m_VData)[i*6], pTerrain);

  m_bEven =! m_bEven;
 
  GetRenderer()->DrawBuffer(m_pVideoBuffer,&m_Indixes,m_nCurrentObjectsCount*12,0,R_PRIMV_TRIANGLES);

  rend->ResetToDefault();
}


void CBFManager::KillBF(const Vec3d & vExploPos, const float fRadius)
{
  for(int i=0; i<MAX_BF_COUNT; i++)
  {
    if(fabs(m_pBFArray[i].m_vPos.x - vExploPos.x)<fRadius)
    if(fabs(m_pBFArray[i].m_vPos.y - vExploPos.y)<fRadius)
    if(fabs(m_pBFArray[i].m_vPos.z - vExploPos.z)<fRadius)
    {
      m_pBFArray[i].m_vPos.x = m_pBFArray[i].m_vPos.x + m_vForward.x*BF_CAMERA_SHIFT;
      m_pBFArray[i].m_vPos.y = m_pBFArray[i].m_vPos.y + m_vForward.y*BF_CAMERA_SHIFT;
    }
  }
}*/