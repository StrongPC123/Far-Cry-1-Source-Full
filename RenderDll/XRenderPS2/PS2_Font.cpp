/*=============================================================================
  PS2_Font.cpp : OpenGL specific font functions.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;


#include "stdafx.h"
#include "PS2_Renderer.h"

#include "CryFont/FBitmap.h"

bool CPS2Renderer::FontUploadTexture(class CFBitmap* pBmp) 
{
  if(!pBmp)
    return false;
  
  char name[128];
  sprintf(name, "$Auto_%d", m_TexGenID++);

  int flags = FT_NOMIPS | FT_HASALPHA | FT_NOREMOVE;
  STexPic *tp = m_TexMan->UploadTexture(name, pBmp->GetWidth(), pBmp->GetHeight(), flags, FT2_FONT, (byte *)pBmp->GetData(), eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);
  int size = pBmp->GetWidth() * pBmp->GetHeight() * 4;
  tp->m_pData32 = new byte[size];
  memcpy(tp->m_pData32, pBmp->GetData(), size);

  pBmp->m_pIRenderData = (void*)tp;

  return true;
}
void CPS2Renderer::FontReleaseTexture(class CFBitmap *pBmp)
{
  if(!pBmp)
    return;
  
  STexPic *tp = (STexPic *)pBmp->m_pIRenderData;
  tp->Release(false);
}

void CPS2Renderer::FontSetTexture(class CFBitmap* pBmp)
{
  if (pBmp)
  {
    STexPic *tp = (STexPic *)pBmp->m_pIRenderData;
    tp->Set();
  }
  EF_SetColorOp(eCO_MODULATE);
}

void CPS2Renderer::FontSetRenderingState(unsigned long nVPWidth, unsigned long nVPHeight)
{
}

void CPS2Renderer::FontSetBlending(int blendSrc, int blendDest)
{
}

void CPS2Renderer::FontRestoreRenderingState()
{
}

void CPS2Renderer::FontSetState(bool bRestore)
{
}

void CPS2Renderer::PrintToScreen(float x, float y, float size, const char *buf)
{
}
