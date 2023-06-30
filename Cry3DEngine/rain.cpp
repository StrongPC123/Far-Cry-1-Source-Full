////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   rain.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: draw rain volume
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "rain.h"
#include "partman.h"
#include "objman.h"

#define rn() ((((float)rand())/RAND_MAX)-0.5f)

CRainItem::CRainItem() 
{ 
  ZeroStruct(*this); 
  m_vPos.x = rn()*RAIN_RANGEx2;
  m_vPos.y = rn()*RAIN_RANGEx2;
  m_vPos.z = rn()*RAIN_RANGEx2;
  m_fSize=2.0f;
}

bool CRainItem::IsPointInvalid(const Vec3d & pos) 
{
  return 0;
}

void CRainItem::Process(Vec3d &right, Vec3d &up, Vec3d &front, const int & nTexID, const Vec3d & delta,
                        IRenderer * pIRenderer, ITimer * pITimer, const Vec3d & vFocusPos, 
												CPartManager * pPartManager, CTerrain * pTerrain,
												class CObjManager * pObjManager, const Vec3d & _vCamPos)
{
  { // space loop
		float fOldZ = m_vPos.z;

    if((m_vPos.x-vFocusPos.x)>RAIN_RANGE)
    {
      while((m_vPos.x-vFocusPos.x)>RAIN_RANGE)
        m_vPos.x-=RAIN_RANGEx2;

      if(IsPointInvalid(m_vPos))
        m_vPos.x+=RAIN_RANGEx2;
    }

    if((vFocusPos.x-m_vPos.x)>RAIN_RANGE)
    {
      while((vFocusPos.x-m_vPos.x)>RAIN_RANGE)
        m_vPos.x+=RAIN_RANGEx2;

      if(IsPointInvalid(m_vPos))
        m_vPos.x-=RAIN_RANGEx2;
    }

    if((m_vPos.y-vFocusPos.y)>RAIN_RANGE)
    {
      while((m_vPos.y-vFocusPos.y)>RAIN_RANGE)
        m_vPos.y-=RAIN_RANGEx2;

      if(IsPointInvalid(m_vPos))
        m_vPos.y+=RAIN_RANGEx2;
    }

    if((vFocusPos.y-m_vPos.y)>RAIN_RANGE)
    {
      while((vFocusPos.y-m_vPos.y)>RAIN_RANGE)
        m_vPos.y+=RAIN_RANGEx2;

      if(IsPointInvalid(m_vPos))
        m_vPos.y-=RAIN_RANGEx2;
    }

    if((m_vPos.z-vFocusPos.z)>RAIN_RANGE)
    {
      while((m_vPos.z-vFocusPos.z)>RAIN_RANGE)
        m_vPos.z-=RAIN_RANGEx2;

      if(IsPointInvalid(m_vPos))
        m_vPos.z+=(RAIN_RANGEx2-rnd());
    }

    if((vFocusPos.z-m_vPos.z)>RAIN_RANGE)
    {
      while((vFocusPos.z-m_vPos.z)>RAIN_RANGE)
        m_vPos.z+=(RAIN_RANGEx2-rnd());

      if(IsPointInvalid(m_vPos))
        m_vPos.z-=RAIN_RANGEx2;
    }

		if(m_vPos.z != fOldZ)
		{
			m_vPos.x += rn()*RAIN_RANGE;
			m_vPos.y += rn()*RAIN_RANGE;
		}
  }

  // draw
  pIRenderer->PushMatrix();
  pIRenderer->TranslateMatrix(m_vPos);
  pIRenderer->DrawQuad(right*m_fSize, up*m_fSize, Vec3d(0,0,0));
  pIRenderer->PopMatrix();

////////////////////////////////////////////////////////////////////////////////////////////////
// Move
////////////////////////////////////////////////////////////////////////////////////////////////

  // process movement and size
  m_vPos += delta*pITimer->GetFrameTime();

  if(0 && fabs((m_vPos.z + m_fSize) - pTerrain->GetZSafe(m_vPos.x,m_vPos.y))<0.1f && delta.z<0)
  {
    Vec3d vCollPos = m_vPos;
    vCollPos.z = pTerrain->GetZApr(m_vPos.x,m_vPos.y)+0.05f;

		float fDist = (float)max(
			fabs(vCollPos.x - _vCamPos.x),max(fabs(vCollPos.y - _vCamPos.y), fabs(vCollPos.z - _vCamPos.z)));

		fDist/=RAIN_RANGE;
		float fDistAlpha = 1.f-fDist;
		fDistAlpha*=2;
		if(fDistAlpha<0)
			fDistAlpha=0;
		if(fDistAlpha>1.f)
			fDistAlpha=1.f;

    ParticleParams params;
/*		params.fFocus = 1.f;
		params.fLifeTime = 0.25f;
		params.fSize = 0.5f;
		params.fSizeSpeed = 0;
		params.fSpeed = 0;
		params.nCount = 1;
		params.eBlendType = ParticleBlendType_ColorBased;
		params.nTexId = pIRenderer->LoadAnimatedTexture("ANIMATED\\raindrop\\rda%02d.tga",7);
		params.nTexAnimFramesCount = 7;
		float fAlpha = fDistAlpha*pObjManager->GetCVars()->e_rain_amount;
		params.vColorStart.Set(fAlpha,fAlpha,fAlpha);
		params.vColorEnd.Set(fAlpha,fAlpha,fAlpha);
		params.vDirection.Set(0,0,1);
		params.vPosition = vCollPos;
	*/																
		params.fFocus = 1.5f;
		params.fLifeTime = 0.5f;
		params.fSize = 0.02f;
		params.fSizeSpeed = 0;
		params.fSpeed = 1.f;
		params.vGravity(0,0,-5.f);
		params.nCount = 15;
		params.eBlendType = ParticleBlendType_ColorBased;
		params.nTexId = pIRenderer->LoadTexture("cloud");
		float fAlpha = fDistAlpha*pObjManager->GetCVars()->e_rain_amount;
		params.vColorStart(fAlpha,fAlpha,fAlpha);
		params.vColorEnd(fAlpha,fAlpha,fAlpha);
		params.vDirection(0,0,1);
		params.vPosition = vCollPos;
		params.fTailLenght = 0.25;
								 
		if(fDistAlpha>0)
			pPartManager->Spawn(params,RAIN_RANGE,pObjManager);

    m_vPos.x += rn()*RAIN_RANGE;
    m_vPos.y += rn()*RAIN_RANGE;
		m_vPos.z -= 0.2f;
  }
}

void CRainManager::Render(CTerrain * pTerrain, 
													const Vec3d & vColor, 
													CObjManager * pObjManager, 
													CPartManager * pPartManager,
													const Vec3d & vWindDir)
{
  if(!GetCVars()->e_rain_amount)
    return;

  // get orientation
  Matrix44 mat;
  GetRenderer()->GetModelViewMatrix(mat.GetData());
	Vec3d right,up,front;	

	//CELL_CHANGED_BY_IVO
	//right(mat.cell(0), mat.cell(4), mat.cell(8));
	//up   (mat.cell(1), mat.cell(5), mat.cell(9)); 
	//front(mat.cell(2), mat.cell(6), mat.cell(10)); 
	right = mat.GetColumn(0);
	up	  = mat.GetColumn(1); 
	front = mat.GetColumn(2);



  Vec3d vCamPos = GetViewCamera().GetPos();

 //CELL_CHANGED_BY_IVO
 //front(-mat.cell(2), -mat.cell(6), -mat.cell(10)); 
	front = -mat.GetColumn(2);

	front.Normalize();
  Vec3d vFocusPos = vCamPos + front*RAIN_RANGE;

  if(!m_nRainTexID)
    m_nRainTexID = GetRenderer()->LoadTexture("textures\\sprites\\rain.tga");

  GetRenderer()->SetTexture(m_nRainTexID);
  GetRenderer()->SetState(GS_BLSRC_ONE | GS_BLDST_ONE);
  GetRenderer()->SetColorOp(eCO_MODULATE, eCO_MODULATE, eCA_Texture|(eCA_Constant<<3), eCA_Texture|(eCA_Constant<<3));
  GetRenderer()->SetCullMode(R_CULL_DISABLE);
  GetRenderer()->SetMaterialColor( 1, 1, 1, GetCVars()->e_rain_amount );

	Vec3d	vUp = GetNormalized(-vWindDir);

  // draw
  for(int i=0; i<RAIN_COUNT; i++)
    m_arrItems[i].Process(right,vUp,front,m_nRainTexID,vWindDir,
    GetRenderer(),GetTimer(),vFocusPos,pPartManager,pTerrain,pObjManager,vCamPos);

	m_fDropTime += GetTimer()->GetFrameTime();

	const float fDropPeriod = max(0.002f, (1.f-GetCVars()->e_rain_amount)/50.f);
	
	while(m_fDropTime>fDropPeriod)
  {
		m_fDropTime -= fDropPeriod;

    Vec3d vCollPos( vFocusPos.x + rn()*RAIN_RANGE*2, vFocusPos.y + rn()*RAIN_RANGE*2, 0);
		vCollPos.z = pTerrain->GetZApr(vCollPos.x,vCollPos.y)+0.05f;

		float fDist = (float)max(
			fabs(vCollPos.x - vCamPos.x),max(fabs(vCollPos.y - vCamPos.y), fabs(vCollPos.z - vCamPos.z)));

		fDist/=RAIN_RANGE;
		float fDistAlpha = 1.f-fDist;
		fDistAlpha*=3;
		if(fDistAlpha<0)
			continue;
		if(fDistAlpha>1.f)
			fDistAlpha=1.f;
	
    ParticleParams params;
/*		params.fFocus = 1.f;
		params.fLifeTime = 0.25f;
		params.fSize = 0.5f;
		params.fSizeSpeed = 0;
		params.fSpeed = 0;
		params.nCount = 1;
		params.eBlendType = ParticleBlendType_ColorBased;
		params.nTexId = pIRenderer->LoadAnimatedTexture("ANIMATED\\raindrop\\rda%02d.tga",7);
		params.nTexAnimFramesCount = 7;
		float fAlpha = fDistAlpha*pObjManager->GetCVars()->e_rain_amount;
		params.vColorStart.Set(fAlpha,fAlpha,fAlpha);
		params.vColorEnd.Set(fAlpha,fAlpha,fAlpha);
		params.vDirection.Set(0,0,1);
		params.vPosition = vCollPos;
	*/																
		params.fFocus = 1.5f;
		params.fLifeTime = 0.5f;
		params.fSize = 0.02f;
		params.fSizeSpeed = 0;
		params.fSpeed = 1.f;
		params.vGravity(0,0,-5.f);
		params.nCount = 15;
		params.eBlendType = ParticleBlendType_ColorBased;
		params.nTexId = GetRenderer()->LoadTexture("cloud");
		float fAlpha = fDistAlpha*GetCVars()->e_rain_amount;
//		params.vColorStart.Set(fAlpha,fAlpha,fAlpha);
	//	params.vColorEnd.Set(fAlpha,fAlpha,fAlpha);
	params.vColorStart = Get3DEngine()->GetFogColor()*fAlpha;
	params.vColorEnd = Get3DEngine()->GetFogColor()*fAlpha;

		params.vDirection(0,0,1);
		params.vPosition = vCollPos;
//		params.fTailLenght = 1.f;
								 
		if(fAlpha>0.125)
			pPartManager->Spawn(params,RAIN_RANGE,pObjManager);
  }
}