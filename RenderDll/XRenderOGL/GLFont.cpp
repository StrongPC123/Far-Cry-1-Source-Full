/*=============================================================================
  GLFont.cpp : OpenGL specific font functions.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey
	* Modified by Márcio Martins

=============================================================================*/

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;


#include "RenderPCH.h"
#include "GL_Renderer.h"

#include "..\..\CryFont\FBitmap.h"

int CGLRenderer::FontCreateTexture(int Width, int Height, byte *pData, ETEX_Format eTF)
{
  if (!pData)
    return -1;

  char szName[128];
  sprintf(szName, "$AutoFont_%d", m_TexGenID++);

  int iFlags = FT_HASALPHA | FT_NOREMOVE | FT_FONT | FT_NOSTREAM | FT_NOMIPS;
  STexPic *tp = m_TexMan->CreateTexture(szName, Width, Height, 1, iFlags, 0, pData, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF);

  return tp->GetTextureID();
}

bool CGLRenderer::FontUpdateTexture(int nTexId, int X, int Y, int USize, int VSize, byte *pData)
{
  STexPic *tp = gRenDev->m_TexMan->m_Textures[nTexId-TX_FIRSTBIND];
  assert (tp && tp->m_Bind == nTexId);
  if (tp)
  {
    m_TexMan->UpdateTextureRegion(tp, pData, X, Y, USize, VSize);
    return true;
  }
  return false;
}

bool CGLRenderer::FontUploadTexture(class CFBitmap* pBmp, ETEX_Format eTF) 
{
	if(!pBmp)
	{
		return false;
	}
  
	unsigned int *pData = new unsigned int[pBmp->GetWidth() * pBmp->GetHeight()];

	if (!pData)
	{
		return false;
	}

	pBmp->Get32Bpp(&pData);

	char szName[128];
	sprintf(szName, "$Auto_%d", m_TexGenID++);
    
	int iFlags = FT_HASALPHA | FT_NOREMOVE | FT_FONT | FT_NOMIPS;
	STexPic *tp = m_TexMan->CreateTexture(szName, pBmp->GetWidth(), pBmp->GetHeight(), 1, iFlags, 0, (unsigned char *)pData, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF);
  //int size = pBmp->GetWidth() * pBmp->GetHeight() * 4;
  //tp->m_pData32 = new byte[size];
  //memcpy(tp->m_pData32, pBmp->GetData(), size);

	SAFE_DELETE_ARRAY(pData);

	pBmp->SetRenderData((void *)tp);

	return true;
}

void CGLRenderer::FontReleaseTexture(class CFBitmap *pBmp)
{
  if(!pBmp)
    return;
  
  STexPic *tp = (STexPic *)pBmp->GetRenderData();
  tp->Release(false);
}

void CGLRenderer::FontSetTexture(class CFBitmap* pBmp, int nFilterMode)
{
  // NOTE: we don't have mips for font texture
  if (pBmp)
  {
    STexPic *tp = (STexPic *)pBmp->GetRenderData();
    tp->Set();
  }
  switch(nFilterMode)
  {
    case FILTER_LINEAR:
    default:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);      
  	  break;
    case FILTER_BILINEAR:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);      
  	  break;
    case FILTER_TRILINEAR:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);      
      break;
  }
  EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
}

void CGLRenderer::FontSetTexture(int nTexId, int nFilterMode)
{
  // NOTE: we don't have mips for font texture
  STexPic *tp = gRenDev->m_TexMan->m_Textures[nTexId-TX_FIRSTBIND];
  assert (tp && tp->m_Bind == nTexId);
  tp->Set();
  //tp->SaveTGA("Font.tga", 0);
  switch(nFilterMode)
  {
    case FILTER_LINEAR:
    default:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);      
  	  break;
    case FILTER_BILINEAR:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);      
  	  break;
    case FILTER_TRILINEAR:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);      
      break;
  }
  //EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
}

void CGLRenderer::FontSetRenderingState(unsigned long nVPWidth, unsigned long nVPHeight)
{
  GLSetCull(eCULL_None);
  EnableTMU(true);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, m_width, m_height, 0.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();  
  m_RP.m_FlagsPerFlush = 0;

  EF_SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST | GS_ALPHATEST_GREATER0);

  EF_SetColorOp(eCO_REPLACE, eCO_MODULATE, eCA_Diffuse | (eCA_Diffuse<<3), DEF_TEXARG0);
}

// match table with the blending modes
static int nBlendMatchTable[] =
{
	GL_ZERO,
	GL_ONE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
};

void CGLRenderer::FontSetBlending(int blendSrc, int blendDest)
{
}

void CGLRenderer::FontRestoreRenderingState()
{
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  
  EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  
  SetPolygonMode(m_polygon_mode);
}

