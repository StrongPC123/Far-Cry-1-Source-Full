#include "RenderPCH.h"
#include "DriverD3D9.h"

//=========================================================================================

#include "../CryFont/FBitmap.h"

bool CD3D9Renderer::FontUpdateTexture(int nTexId, int X, int Y, int USize, int VSize, byte *pData)
{
  WaitForDevice();

  STexPicD3D *tp = (STexPicD3D *)gRenDev->m_TexMan->m_Textures[nTexId-TX_FIRSTBIND];
  assert (tp && tp->m_Bind == nTexId);
  if (tp)
  {
    m_TexMan->UpdateTextureRegion(tp, pData, X, Y, USize, VSize);
    return true;
  }
  return false;
}

bool CD3D9Renderer::FontUploadTexture(class CFBitmap* pBmp, ETEX_Format eTF)
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
	sprintf(szName, "$AutoFont_%d", m_TexGenID++);

	int iFlags = FT_HASALPHA | FT_FONT | FT_NOSTREAM;
	STexPic *tp = m_TexMan->CreateTexture(szName, pBmp->GetWidth(), pBmp->GetHeight(), 1, iFlags, 0, (unsigned char *)pData, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF);

	SAFE_DELETE_ARRAY(pData);

	pBmp->SetRenderData((void *)tp);
	
	return true;
}

int CD3D9Renderer::FontCreateTexture(int Width, int Height, byte *pData, ETEX_Format eTF)
{
  if (!pData)
    return -1;

  char szName[128];
  sprintf(szName, "$AutoFont_%d", m_TexGenID++);

  int iFlags = FT_HASALPHA | FT_FONT | FT_NOSTREAM | FT_NOMIPS;
  STexPic *tp = m_TexMan->CreateTexture(szName, Width, Height, 1, iFlags, 0, pData, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF);

  return tp->GetTextureID();
}

void CD3D9Renderer::FontReleaseTexture(class CFBitmap *pBmp)
{
	if(!pBmp)
	{
		return;
	}
  
	STexPic *tp = (STexPic *)pBmp->GetRenderData();

	tp->Release(false);
}

void CD3D9Renderer::FontSetTexture(class CFBitmap* pBmp, int nFilterMode)
{
	if (pBmp)
	{
		STexPic *tp = (STexPic *)pBmp->GetRenderData();
		tp->Set();
	}
	LPDIRECT3DDEVICE9 dv = mfGetD3DDevice();
	int tmu = 0;
	switch(nFilterMode)
	{
		case FILTER_LINEAR:
		default:
      if (m_RP.m_TexStages[tmu].nMipFilter != D3DTEXF_NONE)
      {
        m_RP.m_TexStages[tmu].nMipFilter = D3DTEXF_NONE;
			  dv->SetSamplerState(tmu, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
      }
      if (m_RP.m_TexStages[tmu].MagFilter != D3DTEXF_LINEAR)
      {
        m_RP.m_TexStages[tmu].MagFilter = D3DTEXF_LINEAR;
        dv->SetSamplerState(tmu, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      }
      if (m_RP.m_TexStages[tmu].MinFilter != D3DTEXF_LINEAR)
      {
        m_RP.m_TexStages[tmu].MinFilter = D3DTEXF_LINEAR;
        dv->SetSamplerState(tmu, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      }
			break;
		case FILTER_BILINEAR:
      if (m_RP.m_TexStages[tmu].nMipFilter == D3DTEXF_POINT)
      {
        m_RP.m_TexStages[tmu].nMipFilter = D3DTEXF_POINT;
			  dv->SetSamplerState(tmu, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
      }
      if (m_RP.m_TexStages[tmu].MagFilter != D3DTEXF_LINEAR)
      {
        m_RP.m_TexStages[tmu].MagFilter = D3DTEXF_LINEAR;
        dv->SetSamplerState(tmu, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      }
      if (m_RP.m_TexStages[tmu].MinFilter != D3DTEXF_LINEAR)
      {
        m_RP.m_TexStages[tmu].MinFilter = D3DTEXF_LINEAR;
        dv->SetSamplerState(tmu, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      }
		  break;
		case FILTER_TRILINEAR:
      if (m_RP.m_TexStages[tmu].nMipFilter == D3DTEXF_LINEAR)
      {
        m_RP.m_TexStages[tmu].nMipFilter = D3DTEXF_LINEAR;
			  dv->SetSamplerState(tmu, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
      }
      if (m_RP.m_TexStages[tmu].MagFilter != D3DTEXF_LINEAR)
      {
        m_RP.m_TexStages[tmu].MagFilter = D3DTEXF_LINEAR;
        dv->SetSamplerState(tmu, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      }
      if (m_RP.m_TexStages[tmu].MinFilter != D3DTEXF_LINEAR)
      {
        m_RP.m_TexStages[tmu].MinFilter = D3DTEXF_LINEAR;
        dv->SetSamplerState(tmu, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      }
  		break;
	}
}

void CD3D9Renderer::FontSetTexture(int nTexId, int nFilterMode)
{
  if (nTexId <= 0 || nTexId > TX_LASTBIND)
    return;
  STexPicD3D *tp = (STexPicD3D *)gRenDev->m_TexMan->m_Textures[nTexId-TX_FIRSTBIND];
  assert (tp && tp->m_Bind == nTexId);

  if (CV_d3d9_forcesoftware)
    return;

  tp->Set();
  LPDIRECT3DDEVICE9 dv = mfGetD3DDevice();
  int tmu = 0;
  switch(nFilterMode)
  {
		case FILTER_LINEAR:
		default:
      if (m_RP.m_TexStages[tmu].nMipFilter != D3DTEXF_NONE)
      {
        m_RP.m_TexStages[tmu].nMipFilter = D3DTEXF_NONE;
			  dv->SetSamplerState(tmu, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
      }
      if (m_RP.m_TexStages[tmu].MagFilter != D3DTEXF_LINEAR)
      {
        m_RP.m_TexStages[tmu].MagFilter = D3DTEXF_LINEAR;
        dv->SetSamplerState(tmu, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      }
      if (m_RP.m_TexStages[tmu].MinFilter != D3DTEXF_LINEAR)
      {
        m_RP.m_TexStages[tmu].MinFilter = D3DTEXF_LINEAR;
        dv->SetSamplerState(tmu, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      }
			break;
		case FILTER_BILINEAR:
      if (m_RP.m_TexStages[tmu].nMipFilter == D3DTEXF_POINT)
      {
        m_RP.m_TexStages[tmu].nMipFilter = D3DTEXF_POINT;
			  dv->SetSamplerState(tmu, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
      }
      if (m_RP.m_TexStages[tmu].MagFilter != D3DTEXF_LINEAR)
      {
        m_RP.m_TexStages[tmu].MagFilter = D3DTEXF_LINEAR;
        dv->SetSamplerState(tmu, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      }
      if (m_RP.m_TexStages[tmu].MinFilter != D3DTEXF_LINEAR)
      {
        m_RP.m_TexStages[tmu].MinFilter = D3DTEXF_LINEAR;
        dv->SetSamplerState(tmu, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      }
		  break;
		case FILTER_TRILINEAR:
      if (m_RP.m_TexStages[tmu].nMipFilter == D3DTEXF_LINEAR)
      {
        m_RP.m_TexStages[tmu].nMipFilter = D3DTEXF_LINEAR;
			  dv->SetSamplerState(tmu, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
      }
      if (m_RP.m_TexStages[tmu].MagFilter != D3DTEXF_LINEAR)
      {
        m_RP.m_TexStages[tmu].MagFilter = D3DTEXF_LINEAR;
        dv->SetSamplerState(tmu, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      }
      if (m_RP.m_TexStages[tmu].MinFilter != D3DTEXF_LINEAR)
      {
        m_RP.m_TexStages[tmu].MinFilter = D3DTEXF_LINEAR;
        dv->SetSamplerState(tmu, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      }
  		break;
  }
}

void CD3D9Renderer::FontSetRenderingState(unsigned long nVPWidth, unsigned long nVPHeight)
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

void CD3D9Renderer::FontSetBlending(int blendSrc, int blendDest)
{
}

void CD3D9Renderer::FontRestoreRenderingState()
{
  D3DXMATRIX *m;

  m_matProj->Pop();
  m = m_matProj->GetTop();
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);

  EF_PopMatrix();
  
  FontSetState(true);
}

void CD3D9Renderer::FontSetState(bool bRestore)
{
  static DWORD polyMode;
  static D3DCOLORVALUE color;
  static bool bMatColor;
  static int State;
  
  if (CV_d3d9_forcesoftware)
    return;

	CD3D9TexMan::BindNULL(1);

  // grab the modes that we might need to change
  if(!bRestore)
  {
    D3DSetCull(eCULL_None);

    color = m_Material.Diffuse;
    bMatColor = m_bMatColor;
    State = m_CurState;
    polyMode = m_polygon_mode;

    EF_SetVertColor();
    
    if(polyMode == R_WIREFRAME_MODE)
      m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    m_RP.m_FlagsPerFlush = 0;
    EF_SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST | GS_ALPHATEST_GREATER0);

    EF_SetColorOp(eCO_REPLACE, eCO_MODULATE, eCA_Diffuse | (eCA_Diffuse<<3), DEF_TEXARG0);
		//m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		//m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
  }
  else
  {
    if (m_bMatColor)
      EF_SetGlobalColor(color.r,color.g,color.b,color.a);

    if(polyMode == R_WIREFRAME_MODE)
      m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

    //EF_SetState(State);
  }
}

