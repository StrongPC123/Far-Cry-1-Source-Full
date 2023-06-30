////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjfar.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: making sprites for object
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"

void CStatObj::CreateModelFarImages(int nTexRes)
{    
	if(!nTexRes)
		nTexRes = FAR_TEX_SIZE;

	nTexRes /= (1<<GetCVars()->e_vegetation_sprites_texres);

  for(int i=0; i<FAR_TEX_COUNT; i++)
  {
    GetRenderer()->ResetToDefault();

    if(m_arrSpriteTexID[i])
    {
      GetRenderer()->RemoveTexture(m_arrSpriteTexID[i]);
      m_arrSpriteTexID[i]=0;
    }

		m_arrSpriteTexID[i] = GetRenderer()->MakeSprite(18.f, nTexRes,
			i*FAR_TEX_ANGLE+90.f, this, 0, m_arrSpriteTexID[i]);
  }

  GetRenderer()->ResetToDefault();
}

bool CStatObj::MakeObjectPicture(unsigned char * pRGBAData, int nWidth)
{
  int nTid = GetRenderer()->MakeSprite(m_vBoxMax.z, nWidth,45,this,pRGBAData,0);
  return nTid>0;
}
