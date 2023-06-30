#include "stdafx.h"
#include "DriverD3D8.h"

//=========================================================================================

#include "../CryFont/FBitmap.h"

bool CD3D8Renderer::FontUploadTexture(class CFBitmap* pBmp) 
{
  if(!pBmp)
    return false;

  char name[128];
  sprintf(name, "$AutoDownload_%d", m_TexGenID++);

  int flags = FT_NOMIPS | FT_HASALPHA | FT_NOREMOVE;
  STexPic *tp = m_TexMan->CreateTexture(name, pBmp->GetWidth(), pBmp->GetHeight(), 1, flags, 0, (byte *)pBmp->GetData(), eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);

  pBmp->m_pIRenderData = (void*)tp;
  return true;
}
void CD3D8Renderer::FontReleaseTexture(class CFBitmap *pBmp)
{
  if(!pBmp)
    return;
  
  STexPic *tp = (STexPic *)pBmp->m_pIRenderData;
  tp->Release(false);
}

void CD3D8Renderer::FontSetTexture(class CFBitmap* pBmp)
{
  STexPic *tp = (STexPic *)pBmp->m_pIRenderData;
  tp->Set();
  EF_SetColorOp(eCO_MODULATE);
}

void CD3D8Renderer::FontSetRenderingState(unsigned long nVPWidth, unsigned long nVPHeight)
{
  // setup various d3d things that we need
  FontSetState(false);

  D3DXMATRIX *m;
  m_matProj->Push();
  m_matProj->LoadIdentity();
  m = m_matProj->GetTop();
  
  D3DXMatrixOrthoOffCenterRH(m, 0.0f, (float)m_Viewport.Width, (float)m_Viewport.Height, 0.0f, -1.0f, 1.0f);
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);

  EF_PushMatrix();
  m_matView->LoadIdentity();
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop());
}

// match table with the blending modes
static int nBlendMatchTable[] =
{
	D3DBLEND_ZERO,
	D3DBLEND_ONE,
	D3DBLEND_SRCCOLOR,
	D3DBLEND_INVSRCCOLOR,
	D3DBLEND_SRCALPHA,
	D3DBLEND_INVSRCALPHA,
	D3DBLEND_DESTALPHA,
	D3DBLEND_INVDESTALPHA,
	D3DBLEND_DESTCOLOR,
	D3DBLEND_INVDESTCOLOR,
};

void CD3D8Renderer::FontSetBlending(int blendSrc, int blendDest)
{
  m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,  nBlendMatchTable[blendSrc]);
  m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, nBlendMatchTable[blendDest]);
  mCurState &= ~GS_BLEND_MASK;
  mCurState |= GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;
}

void CD3D8Renderer::FontRestoreRenderingState()
{
  D3DXMATRIX *m;

  m_matProj->Pop();
  m = m_matProj->GetTop();
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);

  EF_PopMatrix();
  
  FontSetState(true);
}

void CD3D8Renderer::FontSetState(bool bRestore)
{
  static DWORD polyMode;
  static D3DCOLORVALUE color;
  static bool bMatColor;
  static int State;
  
  // grab the modes that we might need to change
  if(!bRestore)
  {
    D3DSetCull(eCULL_None);

    color = m_Material.Diffuse;
    bMatColor = m_bMatColor;
    State = mCurState;
    polyMode = m_polygon_mode;

    EF_SetVertColor();
    
    if(polyMode == R_WIREFRAME_MODE)
      m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST | GS_ALPHATEST_GEQUAL128);
    
    EF_SetColorOp(eCO_MODULATE);
  }
  else
  {
    if (m_bMatColor)
      EF_SetGlobalColor(color.r,color.g,color.b,color.a);

    if(polyMode == R_WIREFRAME_MODE)
      m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

    SetState(State);
  }
  m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
  m_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

void CD3D8Renderer::DrawString(int x, int y,bool bIgnoreColor, const char *message, ...)
{
  CXFont *pFont = m_pCurFont;
  float xscale = m_fXFontScale;
  float yscale = m_fYFontScale;
  if (!pFont)
  {
    if (iConsole && iConsole->GetFont())
      pFont = iConsole->GetFont();
  }
  if (!pFont)
    return;

  float xsize = pFont->m_charsize*xscale*(float)(m_width)/800.0f;
  float ysize = pFont->m_charsize*yscale*(float)(m_height)/600.0f;

  va_list   arglist;
  char    buf[1024];

  va_start(arglist, message);
  vsprintf(buf, message, arglist);
  va_end(arglist);

	FontSetRenderingState(m_width, m_height);
  m_TexMan->SetTexture(pFont->m_image->GetTextureID(), eTT_Base);

  float fX, fY, fBaseX;

  fX = fBaseX = (float)x;
  fY = (float)y;
	
  bool bRGB = ((GetFeatures() & RFT_RGBA) != 0) ? true : false;

  CFColor fCol = m_CurFontColor;
  if (!bRGB)
    COLCONV(fCol);

  char *p = buf;
  int nChar;

  //glBegin(GL_QUADS); 

  while(*p)
	{
		nChar = *p;
		if (nChar=='$')
		{
			if (*(p+1))
			{
				p++;
				if ((*p)!='$')
				{
					char sColor[2];
					sColor[0] =(*p);
					sColor[1] = '\0';
					int nColor = atoi(sColor);
					if (nColor<0 || nColor>=FONTCOLORS)
						nColor = 1;
					if (!bIgnoreColor)		// only change color for first pass !
          {
            fCol = m_FontColorTable[nColor];
            if (!bRGB)
              COLCONV(fCol);
          }
					nChar = 255;
				}
			}
		}
		if(nChar < 255)
    {
		  if(nChar == '\n' || nChar == '\r')
		  {
			  fY += ysize;
			  fX = fBaseX;
		  }
		  else
      if(nChar == '\t')
		  {
			  fX += xsize*4;
		  }
		  else
      if (nChar > 32 && nChar < 256)
  	  {										
        float xot = (float)(nChar & 15)*pFont->m_char_inc;     
        float yot = 1.f-(-(float)(*buf >> 4)*pFont->m_char_inc);     

        /*glColor4fv (&fCol[0]);
        glTexCoord2f(xot, yot);
        glVertex2f (fX, fY);

        glColor4fv (&fCol[0]);
        glTexCoord2f(xot+pFont->m_char_inc, yot);
        glVertex2f (fX+xsize, fY);

        glColor4fv (&fCol[0]);
        glTexCoord2f(xot+pFont->m_char_inc, yot+pFont->m_char_inc);
        glVertex2f (fX+xsize, fY+ysize);

        glColor4fv (&fCol[0]);
        glTexCoord2f(xot, yot+pFont->m_char_inc);
        glVertex2f (fX, fY+ysize);*/
      }
    }
    fX += xsize;
    p++;
  }

  FontRestoreRenderingState();
}
