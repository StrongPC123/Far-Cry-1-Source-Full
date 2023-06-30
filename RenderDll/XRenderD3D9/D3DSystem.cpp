/*=============================================================================
  D3DSystem.cpp : D3D initialization / system functions.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honich Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "DriverD3D9.h"
#include <dxerr9.h>
#include "D3DCGVProgram.h"
#include "D3DCGPShader.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;


void CD3D9Renderer::DisplaySplash()
{
#ifdef GAME_IS_FARCRY
	HBITMAP hImage = (HBITMAP)LoadImage(GetModuleHandle(0), "fcsplash.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
#else
	HBITMAP hImage = (HBITMAP)LoadImage(GetModuleHandle(0), "splash.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
#endif

	if (hImage != INVALID_HANDLE_VALUE)
	{
		RECT rect;
		HDC hDC = GetDC(m_hWnd);
		HDC hDCBitmap = CreateCompatibleDC(hDC);
		BITMAP bm;

		GetClientRect(m_hWnd, &rect);
		GetObject(hImage, sizeof(bm), &bm);
		SelectObject(hDCBitmap, hImage);

		DWORD x = rect.left + (((rect.right-rect.left)-bm.bmWidth) >> 1);
		DWORD y = rect.top + (((rect.bottom-rect.top)-bm.bmHeight) >> 1);

		BitBlt(hDC, x, y, bm.bmWidth, bm.bmHeight, hDCBitmap, 0, 0, SRCCOPY);

		DeleteObject(hImage);
		DeleteDC(hDCBitmap);
	}
}

void CD3D9Renderer::UnSetRes()
{
  m_Features |= RFT_DIRECTACCESSTOVIDEOMEMORY | RFT_SUPPORTZBIAS | RFT_DETAILTEXTURE;

  m_bActive = FALSE;
  m_bReady  = FALSE;

  if( m_pd3dDevice )
  {
    Cleanup3DEnvironment();
    SAFE_RELEASE(m_pD3D);
  }
}

//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT CD3D9Renderer::FinalCleanup()
{
  UnSetRes();
  DeleteContext(m_hWnd);
  DestroyWindow();

  return S_OK;
}

void CD3D9Renderer::DestroyWindow(void)
{
  if (m_hWnd)
  {
    ::DestroyWindow(m_hWnd);
    m_hWnd = NULL;
  }
}

void CD3D9Renderer::RestoreGamma(void)
{
  if (!(GetFeatures() & RFT_HWGAMMA))
    return;

  if (CV_r_nohwgamma)
    return;

  m_fLastGamma = 1.0f;
  m_fLastBrightness = 0.5f;
  m_fLastContrast = 0.5f;

  HDC dc;

  //iLog->Log("...RestoreGamma");

  struct
  {
    ushort red[256];
    ushort green[256];
    ushort blue[256];
  } Ramp;

  for(int i=0; i<256; i++)
  {
    Ramp.red[i] = Ramp.green[i] = Ramp.blue[i] = i << 8;
  }

  m_hWndDesktop = GetDesktopWindow();

  dc = GetDC(m_hWndDesktop);
  SetDeviceGammaRamp(dc, &Ramp);
  ReleaseDC(m_hWndDesktop, dc);
}

void CD3D9Renderer::SetDeviceGamma(ushort *r, ushort *g, ushort *b)
{
  ushort gamma[3][256];
  int i;

  if (!(GetFeatures() & RFT_HWGAMMA))
    return;

  if (CV_r_nohwgamma)
    return;

  m_hWndDesktop = GetDesktopWindow();

  HDC dc = GetDC(m_hWndDesktop);

  if (!dc)
    return;

  for (i=0; i<256; i++)
  {
    gamma[0][i] = r[i];
    gamma[1][i] = g[i];
    gamma[2][i] = b[i];
  }
  //iLog->Log("...SetDeviceGamma");
  SetDeviceGammaRamp(dc, gamma);
  ReleaseDC(m_hWndDesktop, dc);
}

void CD3D9Renderer::SetGamma(float fGamma, float fBrightness, float fContrast, bool bForce)
{
  int i;

  fGamma = CLAMP(fGamma, 0.1f, 3.0f);
  if (!bForce && m_fLastGamma == fGamma && m_fLastBrightness == fBrightness && m_fLastContrast == fContrast)
    return;

  //int n;
  for ( i=0; i<256; i++ )
  {
    m_GammaTable[i] = CLAMP((int)((fContrast+0.5f)*cry_powf((float)i/255.f,1.0f/fGamma)*65535.f + (fBrightness-0.5f)*32768.f - fContrast*32768.f + 16384.f), 0, 65535);
  }

  m_fLastGamma = fGamma;
  m_fLastBrightness = fBrightness;
  m_fLastContrast = fContrast;

  //iLog->Log("...SetGamma %.3f", fGamma);

  SetDeviceGamma(m_GammaTable, m_GammaTable, m_GammaTable);
}

bool CD3D9Renderer::SetGammaDelta(const float fGamma)
{
  m_fDeltaGamma = fGamma;
  SetGamma(CV_r_gamma + fGamma, CV_r_brightness, CV_r_contrast, false);
  return true;
}


//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CD3D9Renderer::InvalidateDeviceObjects()
{
  int i = 0;
  int j;
  HRESULT hr;

  //iLog->Log("...InvalidateDeviceObjects");

  m_nFrameReset++;
  SAFE_RELEASE(m_pVB2D);
  SAFE_RELEASE(m_pIB);
  SAFE_RELEASE(m_pQuery);
  for (j=0; j<3; j++)
  {
    for (i=0; i<4; i++)
    {
      SAFE_RELEASE(m_pVB3DAr[j][i]);
    }
    m_pVB3D[j] = NULL;
  }
  SAFE_RELEASE(m_pSphere);

  EF_Invalidate();

  // Unload vertex/index buffers
  CLeafBuffer *pLB = CLeafBuffer::m_RootGlobal.m_NextGlobal;
  //iLog->Log("Start Unload");
  while (pLB != &CLeafBuffer::m_RootGlobal)
  {
    CLeafBuffer *Next = pLB->m_NextGlobal;
    if (pLB->m_bDynamic)
    {
      //if (pLB->m_sSource)
      //  iLog->Log("Unload '%s' LB", pLB->m_sSource);
      pLB->Unload();
    }
    pLB = Next;
  }
  //iLog->Log("End Unload\n\n");

  CRendElement *pRE = CRendElement::m_RootGlobal.m_NextGlobal;
  while (pRE != &CRendElement::m_RootGlobal)
  {
    pRE->mfReset();
    pRE = pRE->m_NextGlobal;
  }

  for (i=0; i<96; i++)
  {
    CCGVProgram_D3D::m_CurParams[i][0] = -99999.9f;
    CCGVProgram_D3D::m_CurParams[i][1] = -99999.9f;
    CCGVProgram_D3D::m_CurParams[i][2] = -99999.9f;
    CCGVProgram_D3D::m_CurParams[i][3] = -99999.9f;
  }
  for (i=0; i<32; i++)
  {
    CCGPShader_D3D::m_CurParams[i][0] = -99999.9f;
    CCGPShader_D3D::m_CurParams[i][1] = -99999.9f;
    CCGPShader_D3D::m_CurParams[i][2] = -99999.9f;
    CCGPShader_D3D::m_CurParams[i][3] = -99999.9f;
  }
  if (m_TexMan)
  {
    for (i=0; i<gRenDev->m_TexMan->m_Textures.Num(); i++)
    {
      STexPicD3D *tp = (STexPicD3D *)gRenDev->m_TexMan->m_Textures[i];
      if (!tp || !tp->m_RefTex.m_VidTex || tp->m_Bind == TX_FIRSTBIND)
        continue;
      IDirect3DTexture9 *pID3DTexture = NULL;
      IDirect3DCubeTexture9 *pID3DCubeTexture = NULL;
      LPDIRECT3DSURFACE9 pSurf = NULL;
      D3DSURFACE_DESC Desc;
      if (tp->m_eTT == eTT_Cubemap || tp->m_eTT == eTT_AutoCubemap)
      {
        pID3DCubeTexture = (IDirect3DCubeTexture9*)tp->m_RefTex.m_VidTex;
        hr = pID3DCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)0, 0, &pSurf);
      }
      else
      if (tp->m_eTT == eTT_Base || tp->m_eTT == eTT_Bumpmap || tp->m_eTT == eTT_Rectangle || tp->m_eTT == eTT_DSDTBump)
      {
        pID3DTexture = (IDirect3DTexture9*)tp->m_RefTex.m_VidTex;
        hr = pID3DTexture->GetSurfaceLevel(0, &pSurf);
      }
      if (!pSurf)
        continue;
      hr = pSurf->GetDesc(&Desc);
      SAFE_RELEASE(pSurf);
      if (Desc.Pool != D3DPOOL_DEFAULT)
        continue;

      tp->m_Flags2 |= FT2_NEEDRESTORED;
      SAFE_RELEASE(pID3DTexture);
      SAFE_RELEASE(pID3DCubeTexture);
      tp->m_RefTex.m_VidTex = NULL;
    }
    for (i=0; i<MAX_ENVLIGHTCUBEMAPS; i++)
    {
      SEnvTexture *cur = &gRenDev->m_TexMan->m_EnvLCMaps[i];
      for (j=0; j<6; j++)
      {
        if (cur->m_RenderTargets[j])
        {
          IDirect3DSurface9* pSurface = (IDirect3DSurface9* )cur->m_RenderTargets[j];
          cur->m_RenderTargets[j] = NULL;
          pSurface->Release();
        }
      }
    }
  }

  SAFE_RELEASE (m_pTempZBuffer);
  SAFE_RELEASE (m_pZBuffer);
  SAFE_RELEASE (m_pBackBuffer);
  m_pCurBackBuffer = NULL;
  m_pCurZBuffer = NULL;

  return S_OK;
}

HRESULT CD3D9Renderer::RestoreDeviceObjects()
{
  HRESULT hr;
  int i, j;

  //InitTexFillers();
  //Flush();

  //iLog->Log("...RestoreDeviceObjects");

  for (i=0; i<MAX_TMU; i++)
  {
    m_RP.m_TexStages[i].Flush();
  }

  SAFE_RELEASE (m_pTempZBuffer);
  SAFE_RELEASE(m_pZBuffer);
  SAFE_RELEASE(m_pBackBuffer);

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//if( m_FSAA == 2 )
	//{
	//	hr = m_pd3dDevice->CreateDepthStencilSurface( m_width, m_height, m_d3dpp.AutoDepthStencilFormat, ConvertFSAASamplesToType( m_FSAA_samples ), m_FSAA_quality, TRUE, &m_pZBuffer, 0 );
	//	if( SUCCEEDED( hr ) )
	//	{
	//		hr = m_pd3dDevice->SetDepthStencilSurface( m_pZBuffer );
	//		//iLog->Log( "HDR-FSAA: Created multi-sampled Z-Buffer for current FSAA settings (samples = %d / quality = %d).", m_FSAA_samples, m_FSAA_quality );
	//	}
	//	//else
	//	//	iLog->Log( "HDR-FSAA: Multi-sampled Z-Buffer creation failed render target creation failed!" );
	//}
	//else
	//	m_pd3dDevice->GetDepthStencilSurface( &m_pZBuffer );

 // m_pZBuffer->GetDesc( &m_d3dsdZBuffer );
 // m_pd3dDevice->CreateDepthStencilSurface(m_d3dsdZBuffer.Width, m_d3dsdZBuffer.Height, m_d3dsdZBuffer.Format, D3DMULTISAMPLE_NONE, 0, FALSE, &m_pTempZBuffer, NULL);
 // m_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer );
 // m_pBackBuffer->GetDesc(&m_d3dsdBackBuffer);
 // m_pCurBackBuffer = m_pBackBuffer;
 // m_pCurZBuffer = m_pZBuffer;

	//////////////////////////////////////////////////////////////////////////
	// new approach to re-use original non-aa z buffer for temp. render tasks
	//////////////////////////////////////////////////////////////////////////

	if( m_FSAA == 2 )
	{
		hr = m_pd3dDevice->CreateDepthStencilSurface( m_width, m_height, m_d3dpp.AutoDepthStencilFormat, ConvertFSAASamplesToType( m_FSAA_samples ), m_FSAA_quality, TRUE, &m_pZBuffer, 0 );
		if( SUCCEEDED( hr ) )
		{
			//iLog->Log( "HDR-FSAA: Created multi-sampled Z-Buffer for current FSAA settings (samples = %d / quality = %d).", m_FSAA_samples, m_FSAA_quality );			
			hr = m_pd3dDevice->GetDepthStencilSurface( &m_pTempZBuffer ); // re-use current non multi-sampled z buffer for temporary rendering tasks
			hr = m_pd3dDevice->SetDepthStencilSurface( m_pZBuffer );			
			m_pZBuffer->GetDesc( &m_d3dsdZBuffer );
		}
		else
		{
			//iLog->Log( "HDR-FSAA: Multi-sampled Z-Buffer creation failed render target creation failed! Resorting back to r_FSAA 0!" );
			m_FSAA = 0;
			CV_r_fsaa = 0;
		}
	}
	
	if( m_FSAA != 2 )
	{
		m_pd3dDevice->GetDepthStencilSurface( &m_pZBuffer );
		m_pZBuffer->GetDesc( &m_d3dsdZBuffer );
		m_pd3dDevice->CreateDepthStencilSurface(m_d3dsdZBuffer.Width, m_d3dsdZBuffer.Height, m_d3dsdZBuffer.Format, D3DMULTISAMPLE_NONE, 0, FALSE, &m_pTempZBuffer, NULL);
	}

	m_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer );
	m_pBackBuffer->GetDesc(&m_d3dsdBackBuffer);
	m_pCurBackBuffer = m_pBackBuffer;
	m_pCurZBuffer = m_pZBuffer;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

  SAFE_RELEASE(m_pVB2D);
  SAFE_RELEASE(m_pIB);
  SAFE_RELEASE(m_pQuery);
  for (j=0; j<3; j++)
  {
    for (i=0; i<4; i++)
    {
      SAFE_RELEASE(m_pVB3DAr[j][i]);
    }
    m_pVB3D[j] = NULL;
  }
  SAFE_RELEASE(m_pSphere);

  m_nVertsDMesh2D = 400;
  m_nIndsDMesh = CV_d3d9_rb_tris*3;
  m_nIOffsDMesh = m_nIndsDMesh;
  m_nOffsDMesh2D = m_nVertsDMesh2D;
  hr = m_pd3dDevice->CreateVertexBuffer(m_nVertsDMesh2D*sizeof(struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1, D3DPOOL_DEFAULT, &m_pVB2D, NULL);
  if (FAILED(hr))
    return hr;
  hr = m_pd3dDevice->CreateIndexBuffer(m_nIndsDMesh*sizeof(short), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, NULL);
  if (FAILED(hr))
    return hr;

  hr = m_pd3dDevice->CreateQuery(D3DQUERYTYPE_EVENT, &m_pQuery);
  if(hr != D3DERR_NOTAVAILABLE )
  {
    assert(m_pQuery);
    m_pQuery->Issue(D3DISSUE_END);
  }

  for (j=0; j<3; j++)
  {
    int nVertSize;
    int fvf;
    switch (j)
    {
      case 0:
      default:
        m_nVertsDMesh3D[j] = MAX_DYNVB3D_VERTS;
        nVertSize = sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F);
        fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
        break;
      case 1:
        m_nVertsDMesh3D[j] = 16384;
        nVertSize = sizeof(struct_VERTEX_FORMAT_P3F_TEX2F);
        fvf = D3DFVF_XYZ | D3DFVF_TEX1;
        break;
      case 2:
        m_nVertsDMesh3D[j] = 16384;
        nVertSize = sizeof(SPipTangents);
        fvf = 0;
        break;
    }
    m_nOffsDMesh3D[j] = m_nVertsDMesh3D[j];
    for (i=0; i<4; i++)
    {
      hr = m_pd3dDevice->CreateVertexBuffer(m_nVertsDMesh3D[j]*nVertSize, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, fvf, D3DPOOL_DEFAULT, &m_pVB3DAr[j][i], NULL);
      if (FAILED(hr))
        return hr;
    }
    m_pVB3D[j] = m_pVB3DAr[j][0];
  }
  hr = D3DXCreateSphere(m_pd3dDevice, 1, 16, 16, &m_pSphere, NULL);
  if (FAILED(hr))
    return hr;

  memset(&m_Material, 0, sizeof(m_Material));
  for (i=0; i<16; i++)
  {
    D3DLIGHT9 *l = &m_Lights[i];
    memset(l, 0, sizeof(D3DLIGHT9));
    l->Ambient.r = 0.0f;
    l->Ambient.g = 0.0f;
    l->Ambient.b = 0.0f;
    l->Ambient.a = 0.0f;
    l->Type = D3DLIGHT_DIRECTIONAL;
    l->Attenuation0 = 1.0f;
  }

  EF_Restore();

  for (i=0; i<gRenDev->m_TexMan->m_Textures.Num(); i++)
  {
    STexPicD3D *tp = (STexPicD3D *)gRenDev->m_TexMan->m_Textures[i];
    if (!tp || tp->m_Bind == TX_FIRSTBIND)
      continue;
    IDirect3DTexture9 *pID3DTexture = NULL;
    IDirect3DCubeTexture9 *pID3DCubeTexture = NULL;
    if (!(tp->m_Flags2 & FT2_NEEDRESTORED))
      continue;

    int D3DUsage = 0;
    if (tp->m_Flags2 & FT2_RENDERTARGET)
      D3DUsage |= D3DUSAGE_RENDERTARGET;
    if (tp->m_DstFormat == D3DFMT_D24S8 || tp->m_DstFormat == D3DFMT_D16)
    {
      D3DUsage |= D3DUSAGE_DEPTHSTENCIL;
      D3DUsage &= ~D3DUSAGE_RENDERTARGET;
    }
    if (tp->m_Flags & FT_DYNAMIC)
      D3DUsage |= D3DUSAGE_DYNAMIC;
    if (tp->m_eTT == eTT_Cubemap)
    {
      hr = m_pd3dDevice->CreateCubeTexture(tp->m_Width, 1, D3DUsage, (D3DFORMAT)tp->m_DstFormat, D3DPOOL_DEFAULT, &pID3DCubeTexture, NULL);
      tp->m_RefTex.m_VidTex = pID3DCubeTexture;

      for (int i=0; i<6; i++)
      {
        HRESULT hr = S_OK;
        PDIRECT3DSURFACE9 pSurface = NULL;
        hr = pID3DCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)i, 0, &pSurface);
        if (SUCCEEDED(hr))
          m_pd3dDevice->ColorFill(pSurface, NULL, D3DCOLOR_ARGB(0, 0, 0, 0));
        SAFE_RELEASE(pSurface);
      }
    }
    else
    {
      hr = m_pd3dDevice->CreateTexture(tp->m_Width, tp->m_Height, 1, D3DUsage, (D3DFORMAT)tp->m_DstFormat, D3DPOOL_DEFAULT, &pID3DTexture, NULL);
      tp->m_RefTex.m_VidTex = pID3DTexture;

      if (D3DUsage & D3DUSAGE_RENDERTARGET)
      {
        HRESULT hr = S_OK;
        PDIRECT3DSURFACE9 pSurface = NULL;
        hr = pID3DTexture->GetSurfaceLevel(0, &pSurface);
        if (SUCCEEDED(hr))
          m_pd3dDevice->ColorFill(pSurface, NULL, D3DCOLOR_ARGB(0, 0, 0, 0));
        SAFE_RELEASE(pSurface);
      }
    }
  }
  m_bDeviceLost = false;

  // Restore onlyVideo buffers
  CLeafBuffer *pLB = CLeafBuffer::m_RootGlobal.m_NextGlobal;
  while (pLB != &CLeafBuffer::m_RootGlobal)
  {
    CLeafBuffer *Next = pLB->m_NextGlobal;
    if (pLB->m_bDynamic)
    {
      if (pLB->m_bOnlyVideoBuffer)
      {
        pLB->CreateVidVertices(pLB->m_SecVertCount, pLB->m_nVertexFormat);
      }
    }
    pLB = Next;
  }

  m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff );
  // ATI instancing
  if (m_bDeviceSupportsInstancing == 2)
  {
    // Notify the driver that instancing support is expected
    D3DFORMAT instanceSupport = (D3DFORMAT)MAKEFOURCC('I', 'N', 'S', 'T');
    m_pd3dDevice->SetRenderState(D3DRS_POINTSIZE, instanceSupport);
  }
  m_pLastVDeclaration = NULL;

  return S_OK;
}


void  CD3D9Renderer::RefreshResources(int nFlags)
{
}


void CD3D9Renderer::ShutDown(bool bReInit)
{
  m_cEF.mfShutdown();
  if (bReInit)
    FreeResources(FRR_SHADERS | FRR_TEXTURES | FRR_REINITHW);
  else
    FreeResources(FRR_ALL);
  EF_PipelineShutdown();

  if (CV_r_printmemoryleaks)
  {
    FILE *fp = fxopen("LeafBufferLeaks.txt", "w");
    if (fp)
    {
      CLeafBuffer *pLB = CLeafBuffer::m_RootGlobal.m_NextGlobal;
      while (pLB != &CLeafBuffer::m_RootGlobal)
      {
        CLeafBuffer *Next = pLB->m_NextGlobal;
        float fSize = pLB->Size(0)/1024.0f/1024.0f;
        fprintf(fp, "*** LB %s: %0.3fMb\n", pLB->m_sSource, fSize);
        iLog->Log("WARNING: LB Leak %s: %0.3fMb", pLB->m_sSource, fSize);
        //DeleteLeafBuffer(pLB);
        pLB = Next;
      }

      CRendElement *pRE = CRendElement::m_RootGlobal.m_NextGlobal;
      while (pRE != &CRendElement::m_RootGlobal)
      {
        CRendElement *Next = pRE->m_NextGlobal;
        float fSize = pRE->Size()/1024.0f/1024.0f;
        fprintf(fp, "*** RE %s: %0.3fMb\n", pRE->mfTypeString(), fSize);
        iLog->Log("WARNING: RE Leak %s: %0.3fMb", pRE->mfTypeString(), fSize);
        //SAFE_RELEASE(pRE);
        pRE = Next;
      }

      fclose(fp);
    }
  }

#ifndef WIN64
  // NOTE: AMD64 port: find the 64-bit CG runtime
  if (m_CGContext)
  {
    cgDestroyContext(m_CGContext);
    cgD3D9SetDevice(NULL);
    m_CGContext = NULL;
  }
#endif
  FinalCleanup();
  CName::mfExitSubsystem();

  if (m_hLibHandle3DC)
  {
    ::FreeLibrary((HINSTANCE)m_hLibHandle3DC);
    m_hLibHandle3DC = NULL;
  }

	//////////////////////////////////////////////////////////////////////////
	// Clear globals.
	//////////////////////////////////////////////////////////////////////////
  if (!bReInit)
  {
	  iLog = NULL;
	  iConsole = NULL;
	  iTimer = NULL;
	  iSystem = NULL;
	  pIPhysicalWorld = NULL;
  }

  STexPic::m_Root.m_Next = &STexPic::m_Root;
  STexPic::m_Root.m_Prev = &STexPic::m_Root;

  CLeafBuffer::m_Root.m_Next = &CLeafBuffer::m_Root;
  CLeafBuffer::m_Root.m_Prev = &CLeafBuffer::m_Root;
  CLeafBuffer::m_RootGlobal.m_NextGlobal = &CLeafBuffer::m_RootGlobal;
  CLeafBuffer::m_RootGlobal.m_PrevGlobal = &CLeafBuffer::m_RootGlobal;
}

bool CD3D9Renderer::SetWindow(int width, int height, bool fullscreen, WIN_HWND hWnd)
{
  HWND temp = GetDesktopWindow();
  RECT trect;

  GetWindowRect(temp, &trect);

  m_deskwidth =trect.right-trect.left;
  m_deskheight=trect.bottom-trect.top;

  DWORD style, exstyle;
  int x, y, wdt, hgt;

  if (width < 640)
    width = 640;
  if (height < 480)
    height = 480;

  m_dwWindowStyle = WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_OVERLAPPED;

  if (fullscreen)
  {
#ifdef _DEBUG
    exstyle = 0;
#else
    exstyle = WS_EX_TOPMOST;
#endif
    style = WS_POPUP | WS_VISIBLE;
  }
  else
  {
    exstyle = WS_EX_APPWINDOW;
    style = m_dwWindowStyle;
  }

  x = (GetSystemMetrics(SM_CXFULLSCREEN)-width)/2;
  y = (GetSystemMetrics(SM_CYFULLSCREEN)-height)/2;
  wdt = GetSystemMetrics(SM_CXDLGFRAME)*2 + width;
  hgt = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXDLGFRAME)*2 + height;

  if (!hWnd)
  {
    m_hWnd = CreateWindowEx(exstyle,
                          "CryENGINE",
                          m_WinTitle,
                          style,
                          x,
                          y,
                          wdt,
                          hgt,
                          NULL,
                          NULL,
                          m_hInst,
                          NULL);
  }
  else
    m_hWnd = (HWND)hWnd;

  if (!m_hWnd)
    iConsole->Exit("Couldn't create window\n");

  if (!hWnd)
  {
    if (fullscreen)
    {
      // Hide the cursor
      SetCursor(NULL);
      ShowCursor(FALSE);
    }

  }

  return true;
}

#ifdef USE_3DC
#include "../Common/3Dc/CompressorLib.h"
void (*DeleteDataATI)(void *pData);
COMPRESSOR_ERROR (*CompressTextureATI)(DWORD width,
                                                 DWORD height,
                                                 UNCOMPRESSED_FORMAT sourceFormat,
                                                 COMPRESSED_FORMAT destinationFormat,
                                                 void    *inputData,
                                                 void    **dataOut,
                                                 DWORD   *outDataSize);
#endif

WIN_HWND CD3D9Renderer::Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen,WIN_HINSTANCE hinst, WIN_HWND Glhwnd, WIN_HDC Glhdc, WIN_HGLRC hGLrc, bool bReInit)
{
  if (m_IsDedicated)
  {
    m_MaxTextureSize = 256;
    return 0;
  }
  if (!iSystem || !iLog)
    return 0;

  /*float *f = (float *)0x402a92c0;
  FILE *fp = fopen("fl.txt", "w");
  for (int i=0; i<4; i++)
  {
    fprintf(fp, "{\n");
    for (int j=0; j<6; j++)
    {
      fprintf(fp, "  {%ff, %ff, %ff, %ff},\n", f[0], f[1], f[2], f[3]);
      f += 4;
    }
    fprintf(fp, "}\n");
  }
  fclose(fp);*/

  bool b = false;

  m_CVWidth = iConsole->GetCVar("r_Width");
  m_CVHeight = iConsole->GetCVar("r_Height");
  m_CVFullScreen = iConsole->GetCVar("r_FullScreen");
  m_CVColorBits = iConsole->GetCVar("r_ColorBits");

  iLog->Log ( "Direct3D9 driver is creating...\n");
  iLog->Log ( "\nCrytek Direct3D9 driver version %4.2f (%s <%s>).\n", VERSION_D3D, __DATE__, __TIME__);

  //strcpy(m_WinTitle, "- Far Cry -");
#ifdef GAME_IS_FARCRY
	sprintf(m_WinTitle,"- Far Cry - %s (%s)",__DATE__, __TIME__);
#else
	sprintf(m_WinTitle,"- CryEngine - %s (%s)",__DATE__, __TIME__);
#endif
  m_hInst = (HINSTANCE)hinst;

  if (Glhwnd)
    m_bEditor = true;

#ifdef USE_3DC
  m_hLibHandle3DC = ::LoadLibrary("CompressATI.dll");
  if (!m_hLibHandle3DC)
    m_hLibHandle3DC = ::LoadLibrary("CompressATI2.dll");
  if (m_hLibHandle3DC)
  {
    CompressTextureATI = (FnCompressTextureATI)GetProcAddress((HINSTANCE)m_hLibHandle3DC, "CompressTextureATI");
    DeleteDataATI = (FnDeleteDataATI)GetProcAddress((HINSTANCE)m_hLibHandle3DC, "DeleteDataATI");
  }
#endif

  // Save the new dimensions
  CRenderer::m_width  = width;
  CRenderer::m_height = height;
  CRenderer::m_cbpp   = cbpp;
  CRenderer::m_zbpp   = zbpp;
  CRenderer::m_sbpp   = sbits;
  m_bFullScreen       = fullscreen;
  while (true)
  {
    if (!SetWindow(width, height, fullscreen, Glhwnd))
    {
      ShutDown();
      return 0;
    }

    if (SetRes())
      break;
    ShutDown(true);
    if (b)
      return 0;
    m_bFullScreen ? m_bFullScreen = 0 : m_bFullScreen = 1;
    b = 1;
  }

  D3DAdapterInfo *pAI = m_D3DSettings.PAdapterInfo();
  D3DADAPTER_IDENTIFIER9 *ai = &pAI->AdapterIdentifier;
  D3DDeviceInfo *di = m_D3DSettings.PDeviceInfo();

  iLog->Log(" ****** D3D9 CryRender Stats ******");
  iLog->Log(" Driver description: %s", ai->Description);
  iLog->Log(" Full stats: %s", m_strDeviceStats);
  iLog->Log(" Hardware acceleration: %s", (di->DevType == D3DDEVTYPE_HAL) ? "Yes" : "No");
  //iLog->Log(" Full screen only: %s\n", (di->d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED) ? "No" : "Yes");
  if (CV_r_fsaa && (GetFeatures() & RFT_SUPPORTFSAA))
  {
    TArray<SAAFormat> Formats;
    int nNum = GetAAFormat(Formats, false);
    iLog->Log(" Full scene AA: Enabled: %s (%d Quality)\n", Formats[nNum].szDescr, Formats[nNum].nQuality);
    GetAAFormat(Formats, true);
  }
  else
    iLog->Log(" Full scene AA: Disabled\n");
  iLog->Log(" Projective EMBM: %s\n", (GetFeatures() & RFT_HW_ENVBUMPPROJECTED) ? "Enabled" : "Disabled");
  iLog->Log(" Detail textures: %s\n", (GetFeatures() & RFT_DETAILTEXTURE) ? "Enabled" : "Disabled");
  iLog->Log(" Z Buffer Locking: %s\n", (m_Features & RFT_ZLOCKABLE) ? "Enabled" : "Disabled");
  iLog->Log(" Multitexturing: %s (%d textures)\n", (m_Features & RFT_MULTITEXTURE) ? "Supported" : "Not supported", di->Caps.MaxSimultaneousTextures);
  iLog->Log(" Use bumpmapping : %s\n", (m_Features & RFT_BUMP) ? "Enabled (DOT3)" : "Disabled");
  iLog->Log(" Use paletted textures : %s\n", (m_Features & RFT_PALTEXTURE) ? "Enabled" : "Disabled");
  iLog->Log(" Current Resolution: %dx%dx%d %s\n", CRenderer::m_width, CRenderer::m_height, CRenderer::m_cbpp, m_bFullScreen ? "Full Screen" : "Windowed");
  iLog->Log(" Maximum Resolution: %dx%d\n", pAI->m_MaxWidth, pAI->m_MaxHeight);
  iLog->Log(" Maximum Texture size: %dx%d (Max Aspect: %d)\n", di->Caps.MaxTextureWidth, di->Caps.MaxTextureHeight, di->Caps.MaxTextureAspectRatio);
  iLog->Log(" Texture filtering type: %s\n", CV_d3d9_texturefilter->GetString());
  iLog->Log(" HDR Rendering: %s\n", m_nHDRType == 1 ? "FP16" : m_nHDRType == 2 ? "MRT" : "Disabled");
  iLog->Log(" MRT Rendering: %s\n", (m_bDeviceSupportsMRT) ? "Enabled" : "Disabled");
  iLog->Log(" Occlusion queries: %s\n", (m_Features & RFT_OCCLUSIONTEST) ? "Supported" : "Not supported");
  iLog->Log(" Geometry instancing: %s\n", (m_bDeviceSupportsInstancing) ? "Supported" : "Not supported");
  iLog->Log(" NormalMaps compression: %s\n", m_bDeviceSupportsComprNormalmaps==1 ? "3Dc" : m_bDeviceSupportsComprNormalmaps==2 ? "V8U8" : m_bDeviceSupportsComprNormalmaps==3 ? "CxV8U8" : "Not supported");
  iLog->Log(" Gamma control: %s\n", (m_Features & RFT_HWGAMMA) ? "Hardware" : "Software");
  iLog->Log(" Vertex Shaders version %d.%d\n", D3DSHADER_VERSION_MAJOR(di->Caps.VertexShaderVersion), D3DSHADER_VERSION_MINOR(di->Caps.VertexShaderVersion));
  iLog->Log(" Pixel Shaders version %d.%d\n", D3DSHADER_VERSION_MAJOR(di->Caps.PixelShaderVersion), D3DSHADER_VERSION_MINOR(di->Caps.PixelShaderVersion));
  int nGPU = m_Features & RFT_HW_MASK;
  if (nGPU == RFT_HW_NV4X)
    iLog->Log(" Use Hardware Shaders for NV4x GPU\n");
  else
  if (nGPU == RFT_HW_GFFX)
    iLog->Log(" Use Hardware Shaders for NV3x GPU\n");
  else
  if (nGPU == RFT_HW_GF2)
    iLog->Log(" Use Hardware Shaders for NV1x GPU\n");
  else
  if (nGPU == RFT_HW_GF3)
    iLog->Log(" Use Hardware Shaders for NV2x GPU\n");
  else
  if (nGPU == RFT_HW_RADEON)
  {
    if (m_bDeviceSupports_PS2X)
      iLog->Log(" Use Hardware Shaders for ATI R420 GPU\n");
    else
      iLog->Log(" Use Hardware Shaders for ATI R300 GPU\n");
  }
  else
    iLog->Log(" Hardware Shaders are not supported\n");
  if (D3DSHADER_VERSION_MAJOR(di->Caps.PixelShaderVersion) >= 3)
  {
    if (CV_r_shadowtype == 0)
      m_Features |= RFT_SHADOWMAP_SELFSHADOW;
    m_Features |= RFT_HW_PS30 | RFT_HW_PS20;
    m_bDeviceSupports_PS30 = true;
    m_bDeviceSupports_PS2X = false;
  }
  else
  if (D3DSHADER_VERSION_MAJOR(di->Caps.PixelShaderVersion) >= 2)
  {
    if (CV_r_shadowtype == 0)
      m_Features |= RFT_SHADOWMAP_SELFSHADOW;
    m_Features |= RFT_HW_PS20;
  }
#ifndef USE_HDR
  m_Features &= ~RFT_HW_HDR;
#endif

  // Disable per-pixel lighting if pixel-shaders aren't supported
  if (!(m_Features & (RFT_HW_PS20 | RFT_HW_TS | RFT_HW_RC)))
  {
    if (CV_r_Quality_BumpMapping)
      _SetVar("r_Quality_BumpMapping", 0);
    if (CV_r_checkSunVis >= 2)
      _SetVar("r_checkSunVis", 1);
  }

  // Allow pixel shaders 2.0 lighting on Radeon only cards with PS.2.0 support
  if (CV_r_Quality_BumpMapping == 3)
  {
    if (!(m_Features & RFT_HW_PS20) || (nGPU == RFT_HW_GFFX && !CV_d3d9_nv30_ps20))
      _SetVar("r_Quality_BumpMapping", 2);
  }

  // Shaders remapping for non-PS20 hardware
  if (!(m_Features & RFT_HW_PS20) || (nGPU == RFT_HW_GFFX && !CV_d3d9_nv30_ps20))
  {
    _SetVar("r_NoPS20", 1);
    if (CV_r_shadowblur == 3)
      _SetVar("r_ShadowBlur", 2);
  }

  // Disable trees per-pixel lighting from medium and low spec settings
  if (CV_r_Quality_BumpMapping < 2)
    _SetVar("r_Vegetation_PerpixelLight", 0);

  // Allow offset bump-mapping and parametric shaders system for very high spec settings only
  if (CV_r_Quality_BumpMapping < 3)
  {
    if (CV_r_usehwshaders == 2)
      _SetVar("r_UseHWShaders", 1);
    if (CV_r_offsetbump)
      _SetVar("r_OffsetBump", 0);
    m_bDeviceSupports_PS2X = false;
    m_bDeviceSupports_PS30 = false;
  }

  if (!m_bDeviceSupports_PS2X && CV_r_sm2xpath)
    _SetVar("r_SM2XPATH", 0);
  if (!CV_r_sm2xpath)
    _SetVar("r_NoPS2X", 1);
  m_nEnabled_PS2X = CV_r_nops2x ? 0 : 1;
  m_NoPS2X = CV_r_nops2x;
  m_sm2xpath = CV_r_sm2xpath;

  if (!m_bDeviceSupports_PS30 && CV_r_sm30path)
    _SetVar("r_SM30PATH", 0);
  if (!CV_r_sm30path)
    _SetVar("r_NoPS30", 1);
  m_nEnabled_PS30 = CV_r_nops30 ? 0 : 1;
  m_NoPS30 = CV_r_nops30;
  m_sm30path = CV_r_sm30path;

  if (m_bDeviceSupportsComprNormalmaps == 0)
    _SetVar("r_TexNormalMapCompressed", 0);

  if ((m_sm30path || m_sm2xpath) && m_bDeviceSupportsInstancing)
    _SetVar("r_GeomInstancing", 1);

  if (m_nHDRType == 2)
  {
    // HDRFake mode works only on NV40
    _SetVar("r_HDRFake", 0);
  }

  // DO NOT Use VB Pools with fixed pipeline (some driver bug)
  if (nGPU == RFT_HW_GF2 || CV_r_Quality_BumpMapping==0 || !(m_d3dCaps.DevCaps2 & D3DDEVCAPS2_STREAMOFFSET))
  {
    _SetVar("d3d9_VBPools", 0);
  }

  char *str;
  if (nGPU == RFT_HW_GF2)
    str = "Not using pixel shaders";
  else
  if (CV_r_nops20)
    str = "Replace PS.2.0 to PS.1.1";
  else
  if (CV_r_Quality_BumpMapping == 3)
  {
    if (m_bDeviceSupports_PS30)
      str = "PS.3.0, PS.2.0 and PS.1.1";
    else
    if (m_bDeviceSupports_PS2X)
    {
      if (nGPU == RFT_HW_GFFX)
        str = "PS.2.A, PS.2.0 and PS.1.1";
      else
        str = "PS.2.B, PS.2.0 and PS.1.1";
    }
    else
      str = "PS.2.0 and PS.1.1";
  }
  else
    str ="PS1.1 only";
  iLog->Log(" Pixel shaders usage: %s\n", str);

  if (nGPU == RFT_HW_GF2)
    str = "Not using vertex shaders";
  else
  if (CV_r_nops20)
    str = "Replace VS.2.0 to VS.1.1";
  else
  if (CV_r_Quality_BumpMapping == 3)
  {
    if (D3DSHADER_VERSION_MAJOR(di->Caps.VertexShaderVersion) >= 3)
      str = "VS.3.0, VS.2.0 and VS.1.1";
    else
      str = "VS.2.0 and VS.1.1";
  }
  else
    str ="VS1.1 only";
  iLog->Log(" Vertex shaders usage: %s\n", str);
  iLog->Log(" Shadow maps type: %s\n", (m_Features & RFT_DEPTHMAPS) ? "Depth maps" : (m_Features & RFT_SHADOWMAP_SELFSHADOW) ? "Mixed Depth/2D maps" : "2D shadow maps");
  iLog->Log(" Stencil shadows type: %s\n", m_d3dCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED ? "Two sided" : "Single sided");
  iLog->Log(" Lighting quality: %s\n", CV_r_Quality_BumpMapping==0 ? "Low" : CV_r_Quality_BumpMapping==1 ? "Medium" : CV_r_Quality_BumpMapping==2 ? "High" : "Highest");
  iLog->Log(" *****************************************\n\n");

  iLog->Log("Init Shaders\n");

  m_numtmus = di->Caps.MaxSimultaneousTextures;

#ifndef WIN64
	// NOTE: AMD64 port: find the 64-bit CG runtime
	if (!m_CGContext)
  {
    m_CGContext = cgCreateContext();
    assert(m_CGContext);
    cgD3D9SetDevice(mfGetD3DDevice());
#ifdef _DEBUG
    cgD3D9EnableDebugTracing(true);
#endif
  }
#endif

  gRenDev->m_cEF.mfInit();
  EF_Init();
  //ChangeResolution(1024, 768, 32, 75, false);

  m_bInitialized = true;

//  Cry_memcheck();

  // Success, return the window handle
  return (m_hWnd);
}

const char *CD3D9Renderer::D3DError( HRESULT h )
{
  const TCHAR* strHRESULT;
  strHRESULT = DXGetErrorString9(h);
  return strHRESULT;
}

bool CD3D9Renderer::Error(char *Msg, HRESULT h)
{
  const char *str = D3DError(h);
  iLog->Log("Error: %s (%s)", Msg, str);

  //UnSetRes();

  //if (Msg)
  //  iConsole->Exit("%s (%s)\n", Msg, str);
  //else
  //  iConsole->Exit("(%s)\n", str);

  return false;
}

//=============================================================================

//-----------------------------------------------------------------------------
// Name: FindBestWindowedMode()
// Desc: Sets up m_D3DSettings with best available windowed mode, subject to
//       the bRequireHAL and bRequireREF constraints.  Returns false if no such
//       mode can be found.
//-----------------------------------------------------------------------------
bool CD3D9Renderer::FindBestWindowedMode( bool bRequireHAL, bool bRequireREF )
{
  // Get display mode of primary adapter (which is assumed to be where the window
  // will appear)
  D3DDISPLAYMODE primaryDesktopDisplayMode;
  m_pD3D->GetAdapterDisplayMode(0, &primaryDesktopDisplayMode);

  D3DAdapterInfo* pBestAdapterInfo = NULL;
  D3DDeviceInfo* pBestDeviceInfo = NULL;
  D3DDeviceCombo* pBestDeviceCombo = NULL;

  for(int iai=0; iai<m_D3DEnum.m_pAdapterInfoList->Num(); iai++)
  {
    D3DAdapterInfo* pAdapterInfo = m_D3DEnum.m_pAdapterInfoList->Get(iai);
    for(int idi=0; idi<pAdapterInfo->pDeviceInfoList->Num(); idi++)
    {
        D3DDeviceInfo* pDeviceInfo = pAdapterInfo->pDeviceInfoList->Get(idi);
        if (bRequireHAL && pDeviceInfo->DevType != D3DDEVTYPE_HAL)
          continue;
        if (bRequireREF && pDeviceInfo->DevType != D3DDEVTYPE_REF)
          continue;
        for(int idc=0; idc<pDeviceInfo->pDeviceComboList->Num(); idc++)
        {
          D3DDeviceCombo* pDeviceCombo = pDeviceInfo->pDeviceComboList->Get(idc);
          bool bAdapterMatchesBB = (pDeviceCombo->BackBufferFormat == pDeviceCombo->AdapterFormat);
          if (!pDeviceCombo->IsWindowed)
            continue;
          if (pDeviceCombo->AdapterFormat != primaryDesktopDisplayMode.Format)
            continue;
          // If we haven't found a compatible DeviceCombo yet, or if this set
          // is better (because it's a HAL, and/or because formats match better),
          // save it
          if( pBestDeviceCombo == NULL || pBestDeviceCombo->DevType != D3DDEVTYPE_HAL && pDeviceCombo->DevType == D3DDEVTYPE_HAL || pDeviceCombo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesBB)
          {
            pBestAdapterInfo = pAdapterInfo;
            pBestDeviceInfo = pDeviceInfo;
            pBestDeviceCombo = pDeviceCombo;
            if( pDeviceCombo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesBB )
            {
              // This windowed device combo looks great -- take it
              goto EndWindowedDeviceComboSearch;
            }
            // Otherwise keep looking for a better windowed device combo
          }
      }
    }
  }
EndWindowedDeviceComboSearch:
  if (pBestDeviceCombo == NULL )
    return false;

  m_D3DSettings.pWindowed_AdapterInfo = pBestAdapterInfo;
  m_D3DSettings.pWindowed_DeviceInfo = pBestDeviceInfo;
  m_D3DSettings.pWindowed_DeviceCombo = pBestDeviceCombo;
  m_D3DSettings.IsWindowed = true;
  m_D3DSettings.Windowed_DisplayMode = primaryDesktopDisplayMode;
  m_D3DSettings.Windowed_Width = m_rcWindowClient.right - m_rcWindowClient.left;
  m_D3DSettings.Windowed_Height = m_rcWindowClient.bottom - m_rcWindowClient.top;
  if (m_D3DEnum.AppUsesDepthBuffer)
    m_D3DSettings.Windowed_DepthStencilBufferFormat = pBestDeviceCombo->pDepthStencilFormatList->Get(0);
  m_D3DSettings.Windowed_MultisampleType = pBestDeviceCombo->pMultiSampleTypeList->Get(0);
  m_D3DSettings.Windowed_MultisampleQuality = 0;
  m_D3DSettings.Windowed_VertexProcessingType = pBestDeviceCombo->pVertexProcessingTypeList->Get(0);
  m_D3DSettings.Windowed_PresentInterval = pBestDeviceCombo->pPresentIntervalList->Get(0);
  return true;
}




//-----------------------------------------------------------------------------
// Name: FindBestFullscreenMode()
// Desc: Sets up m_D3DSettings with best available fullscreen mode, subject to
//       the bRequireHAL and bRequireREF constraints.  Returns false if no such
//       mode can be found.
//-----------------------------------------------------------------------------
bool CD3D9Renderer::FindBestFullscreenMode( bool bRequireHAL, bool bRequireREF )
{
  // For fullscreen, default to first HAL DeviceCombo that supports the current desktop
  // display mode, or any display mode if HAL is not compatible with the desktop mode, or
  // non-HAL if no HAL is available
  D3DDISPLAYMODE adapterDesktopDisplayMode;
  D3DDISPLAYMODE bestAdapterDesktopDisplayMode;
  D3DDISPLAYMODE bestDisplayMode;
  bestAdapterDesktopDisplayMode.Width = 0;
  bestAdapterDesktopDisplayMode.Height = 0;
  bestAdapterDesktopDisplayMode.Format = D3DFMT_UNKNOWN;
  bestAdapterDesktopDisplayMode.RefreshRate = 0;

  D3DAdapterInfo* pBestAdapterInfo = NULL;
  D3DDeviceInfo* pBestDeviceInfo = NULL;
  D3DDeviceCombo* pBestDeviceCombo = NULL;

  for(int iai=0; iai<m_D3DEnum.m_pAdapterInfoList->Num(); iai++)
  {
    D3DAdapterInfo* pAdapterInfo = m_D3DEnum.m_pAdapterInfoList->Get(iai);
    m_pD3D->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &adapterDesktopDisplayMode );
    for(int idi=0; idi<pAdapterInfo->pDeviceInfoList->Num(); idi++)
    {
      D3DDeviceInfo* pDeviceInfo = pAdapterInfo->pDeviceInfoList->Get(idi);
      if (bRequireHAL && pDeviceInfo->DevType != D3DDEVTYPE_HAL)
        continue;
      if (bRequireREF && pDeviceInfo->DevType != D3DDEVTYPE_REF)
        continue;
      for(int idc=0; idc<pDeviceInfo->pDeviceComboList->Num(); idc++)
      {
        D3DDeviceCombo* pDeviceCombo = pDeviceInfo->pDeviceComboList->Get(idc);
        bool bAdapterMatchesBB = (pDeviceCombo->BackBufferFormat == pDeviceCombo->AdapterFormat);
        bool bAdapterMatchesDesktop = (pDeviceCombo->AdapterFormat == adapterDesktopDisplayMode.Format);
        if (pDeviceCombo->IsWindowed)
          continue;
        // If we haven't found a compatible set yet, or if this set
        // is better (because it's a HAL, and/or because formats match better),
        // save it
        if (pBestDeviceCombo == NULL ||
           (pBestDeviceCombo->DevType != D3DDEVTYPE_HAL && pDeviceInfo->DevType == D3DDEVTYPE_HAL) ||
           (pDeviceCombo->DevType == D3DDEVTYPE_HAL && pBestDeviceCombo->AdapterFormat != adapterDesktopDisplayMode.Format && bAdapterMatchesDesktop) ||
           pDeviceCombo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesDesktop && bAdapterMatchesBB )
        {
          bestAdapterDesktopDisplayMode = adapterDesktopDisplayMode;
          pBestAdapterInfo = pAdapterInfo;
          pBestDeviceInfo = pDeviceInfo;
          pBestDeviceCombo = pDeviceCombo;
          if (pDeviceInfo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesDesktop && bAdapterMatchesBB)
          {
            // This fullscreen device combo looks great -- take it
            goto EndFullscreenDeviceComboSearch;
          }
          // Otherwise keep looking for a better fullscreen device combo
        }
      }
    }
  }
EndFullscreenDeviceComboSearch:
  if (pBestDeviceCombo == NULL)
    return false;

  // Need to find a display mode on the best adapter that uses pBestDeviceCombo->AdapterFormat
  // and is as close to bestAdapterDesktopDisplayMode's res as possible
  bestDisplayMode.Width = 0;
  bestDisplayMode.Height = 0;
  bestDisplayMode.Format = D3DFMT_UNKNOWN;
  bestDisplayMode.RefreshRate = 0;
  for(int idm=0; idm<pBestAdapterInfo->pDisplayModeList->Num(); idm++)
  {
    D3DDISPLAYMODE* pdm = &pBestAdapterInfo->pDisplayModeList->Get(idm);
    if( pdm->Format != pBestDeviceCombo->AdapterFormat )
      continue;
    if( pdm->Width == bestAdapterDesktopDisplayMode.Width && pdm->Height == bestAdapterDesktopDisplayMode.Height && pdm->RefreshRate == bestAdapterDesktopDisplayMode.RefreshRate )
    {
      // found a perfect match, so stop
      bestDisplayMode = *pdm;
      break;
    }
    else
    if( pdm->Width == bestAdapterDesktopDisplayMode.Width && pdm->Height == bestAdapterDesktopDisplayMode.Height && pdm->RefreshRate > bestDisplayMode.RefreshRate )
    {
      // refresh rate doesn't match, but width/height match, so keep this
      // and keep looking
      bestDisplayMode = *pdm;
    }
    else if( pdm->Width == bestAdapterDesktopDisplayMode.Width )
    {
      // width matches, so keep this and keep looking
      bestDisplayMode = *pdm;
    }
    else
    if( bestDisplayMode.Width == 0 )
    {
      // we don't have anything better yet, so keep this and keep looking
      bestDisplayMode = *pdm;
    }
  }

  m_D3DSettings.pFullscreen_AdapterInfo = pBestAdapterInfo;
  m_D3DSettings.pFullscreen_DeviceInfo = pBestDeviceInfo;
  m_D3DSettings.pFullscreen_DeviceCombo = pBestDeviceCombo;
  m_D3DSettings.IsWindowed = false;
  m_D3DSettings.Fullscreen_DisplayMode = bestDisplayMode;
  if (m_D3DEnum.AppUsesDepthBuffer)
    m_D3DSettings.Fullscreen_DepthStencilBufferFormat = pBestDeviceCombo->pDepthStencilFormatList->Get(0);
  m_D3DSettings.Fullscreen_MultisampleType = pBestDeviceCombo->pMultiSampleTypeList->Get(0);
  m_D3DSettings.Fullscreen_MultisampleQuality = 0;
  m_D3DSettings.Fullscreen_VertexProcessingType = pBestDeviceCombo->pVertexProcessingTypeList->Get(0);
  m_D3DSettings.Fullscreen_PresentInterval = D3DPRESENT_INTERVAL_DEFAULT;
  return true;
}




//-----------------------------------------------------------------------------
// Name: ChooseInitialD3DSettings()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3D9Renderer::ChooseInitialD3DSettings()
{
  bool bFoundFullscreen = FindBestFullscreenMode( false, false );
  bool bFoundWindowed = FindBestWindowedMode( false, false );

  if( m_bFullScreen && bFoundFullscreen )
    m_D3DSettings.IsWindowed = false;
  if( !bFoundWindowed && bFoundFullscreen )
    m_D3DSettings.IsWindowed = false;

  if( !bFoundFullscreen && !bFoundWindowed )
    return D3DAPPERR_NOCOMPATIBLEDEVICES;

  return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CD3D9Renderer::ConfirmDevice()
// Desc: Called during device intialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CD3D9Renderer::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat )
{
  return S_OK;
}


//-----------------------------------------------------------------------------
// Name: ConfirmDeviceHelper()
// Desc: Static function used by D3DEnumeration
//-----------------------------------------------------------------------------
bool CD3D9Renderer::ConfirmDeviceHelper(D3DCAPS9* pCaps, VertexProcessingType vertexProcessingType, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat)
{
  DWORD dwBehavior;

  if (vertexProcessingType == SOFTWARE_VP)
    dwBehavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
  else
  if (vertexProcessingType == MIXED_VP)
    dwBehavior = D3DCREATE_MIXED_VERTEXPROCESSING;
  else
  if (vertexProcessingType == HARDWARE_VP)
    dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;
  else
  if (vertexProcessingType == PURE_HARDWARE_VP)
    dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
  else
    dwBehavior = 0; // TODO: throw exception

  return SUCCEEDED(gcpRendD3D->ConfirmDevice(pCaps, dwBehavior, adapterFormat, backBufferFormat));
}

static char *sD3DFMT( D3DFORMAT fmt )
{
  switch( fmt )
  {
  case D3DFMT_R8G8B8:
    return "D3DFMT_R8G8B8";
  case D3DFMT_A8R8G8B8:
    return "D3DFMT_A8R8G8B8";
  case D3DFMT_X8R8G8B8:
    return "D3DFMT_X8R8G8B8";
  case D3DFMT_R5G6B5:
    return "D3DFMT_R5G6B5";
  case D3DFMT_X1R5G5B5:
    return "D3DFMT_X1R5G5B5";
  case D3DFMT_A1R5G5B5:
    return "D3DFMT_A1R5G5B5";
  case D3DFMT_A4R4G4B4:
    return "D3DFMT_A4R4G4B4";
  case D3DFMT_R3G3B2:
    return "D3DFMT_R3G3B2";
  case D3DFMT_A8R3G3B2:
    return "D3DFMT_A8R3G3B2";
  case D3DFMT_X4R4G4B4:
    return "D3DFMT_X4R4G4B4";
  case D3DFMT_A2B10G10R10:
    return "D3DFMT_A2B10G10R10";
  case D3DFMT_A2R10G10B10:
    return "D3DFMT_A2R10G10B10";

  case D3DFMT_D24S8:
    return "D3DFMT_D24S8";
  case D3DFMT_D24X8:
    return "D3DFMT_D24X8";
  case D3DFMT_D16:
    return "D3DFMT_D16";
  case D3DFMT_D24X4S4:
    return "D3DFMT_D24X4S4";
  case D3DFMT_D32:
    return "D3DFMT_D32";
  case D3DFMT_D16_LOCKABLE:
    return "D3DFMT_D16_LOCKABLE";
  case D3DFMT_D15S1:
    return "D3DFMT_D15S1";
  case D3DFMT_D32F_LOCKABLE:
    return "D3DFMT_D32F_LOCKABLE";
  case D3DFMT_D24FS8:
    return "D3DFMT_D24FS8";
  default:
    return "Unknown";
  }
}

//-----------------------------------------------------------------------------
// Name: Initialize3DEnvironment()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3D9Renderer::Initialize3DEnvironment()
{
  HRESULT hr;

  D3DAdapterInfo* pAdapterInfo = m_D3DSettings.PAdapterInfo();
  D3DDeviceInfo* pDeviceInfo = m_D3DSettings.PDeviceInfo();
  D3DDISPLAYMODE ModeInfo = m_D3DSettings.DisplayMode();

  SetRendParms(&ModeInfo, pDeviceInfo);

  // Prepare window for possible windowed/fullscreen change
  AdjustWindowForChange();

  // Set up the presentation parameters
  BuildPresentParamsFromSettings();

  if( pDeviceInfo->Caps.PrimitiveMiscCaps & D3DPMISCCAPS_NULLREFERENCE )
  {
    // Warn user about null ref device that can't render anything
    iLog->Log("ERROR: Chosed NULL Ref Device that can't render anything");
    return E_FAIL;
  }

  DWORD behaviorFlags = D3DCREATE_FPU_PRESERVE;
  if (CV_d3d9_forcesoftware)
    behaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
  else
  {
    if (m_D3DSettings.GetVertexProcessingType() == SOFTWARE_VP)
      behaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    else
    if (m_D3DSettings.GetVertexProcessingType() == MIXED_VP)
      behaviorFlags |= D3DCREATE_MIXED_VERTEXPROCESSING;
    else
    if (m_D3DSettings.GetVertexProcessingType() == HARDWARE_VP)
      behaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else
    if (m_D3DSettings.GetVertexProcessingType() == PURE_HARDWARE_VP)
      behaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
  }

  // Create the device
  iLog->Log("Creating D3D device (Adapter format: %s, BackBuffer format: %s, Depth format: %s)", sD3DFMT(m_D3DSettings.AdapterFormat()), sD3DFMT(m_d3dpp.BackBufferFormat), sD3DFMT(m_d3dpp.AutoDepthStencilFormat));
  if (!CV_d3d9_nvperfhud)
    hr = m_pD3D->CreateDevice(m_D3DSettings.AdapterOrdinal(), pDeviceInfo->DevType, (HWND)m_CurrContext->m_hWnd, behaviorFlags, &m_d3dpp, &m_pd3dDevice);
  else
    hr = m_pD3D->CreateDevice(m_pD3D->GetAdapterCount()-1, D3DDEVTYPE_REF, (HWND)m_CurrContext->m_hWnd, behaviorFlags & ~(D3DCREATE_PUREDEVICE), &m_d3dpp, &m_pd3dDevice);

  if( SUCCEEDED(hr) )
  {
    // If we cannot use Queries Back Buffer should be lockable
    if (!(m_d3dpp.Flags & D3DPRESENTFLAG_LOCKABLE_BACKBUFFER))
    {
      hr = m_pd3dDevice->CreateQuery(D3DQUERYTYPE_EVENT, NULL);
      if(hr != D3DERR_NOTAVAILABLE)
        hr = m_pd3dDevice->CreateQuery (D3DQUERYTYPE_OCCLUSION, NULL);
      if(hr == D3DERR_NOTAVAILABLE)
      {
        SAFE_RELEASE(m_pd3dDevice);
        Sleep(1000);
        m_d3dpp.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

        // Create the device
        hr = m_pD3D->CreateDevice(m_D3DSettings.AdapterOrdinal(), pDeviceInfo->DevType, (HWND)m_CurrContext->m_hWnd, behaviorFlags, &m_d3dpp, &m_pd3dDevice);
        if (FAILED(hr))
        {
          SAFE_RELEASE(m_pd3dDevice);
          Sleep(1000);
          m_d3dpp.Flags &= ~D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
          hr = m_pD3D->CreateDevice(m_D3DSettings.AdapterOrdinal(), pDeviceInfo->DevType, (HWND)m_CurrContext->m_hWnd, behaviorFlags, &m_d3dpp, &m_pd3dDevice);
        }
      }
    }
    if( SUCCEEDED(hr) )
    {
      // When moving from fullscreen to windowed mode, it is important to
      // adjust the window size after recreating the device rather than
      // beforehand to ensure that you get the window size you want.  For
      // example, when switching from 640x480 fullscreen to windowed with
      // a 1000x600 window on a 1024x768 desktop, it is impossible to set
      // the window size to 1000x600 until after the display mode has
      // changed to 1024x768, because windows cannot be larger than the
      // desktop.
			ShowWindow(m_hWnd, SW_SHOW);
			UpdateWindow(m_hWnd);

			SetForegroundWindow(m_hWnd);
			SetFocus(m_hWnd);

      if(!m_bFullScreen )
      {
        SetWindowPos(m_hWnd, HWND_NOTOPMOST, m_rcWindowBounds.left, m_rcWindowBounds.top, (m_rcWindowBounds.right-m_rcWindowBounds.left), (m_rcWindowBounds.bottom - m_rcWindowBounds.top), SWP_SHOWWINDOW);
      }
      ChangeLog();
			DisplaySplash();

      // Store device Caps
      m_pd3dDevice->GetDeviceCaps(&m_d3dCaps);
      m_dwCreateFlags = behaviorFlags;

      // Store device description
      if(pDeviceInfo->DevType == D3DDEVTYPE_REF)
        lstrcpy(m_strDeviceStats, TEXT("REF"));
      else
      if(pDeviceInfo->DevType == D3DDEVTYPE_HAL)
        lstrcpy(m_strDeviceStats, TEXT("HAL"));
      else
      if(pDeviceInfo->DevType == D3DDEVTYPE_SW)
        lstrcpy( m_strDeviceStats, TEXT("SW") );

      if(behaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING && behaviorFlags & D3DCREATE_PUREDEVICE)
      {
        if(pDeviceInfo->DevType == D3DDEVTYPE_HAL)
          lstrcat( m_strDeviceStats, TEXT(" (pure hw vp)") );
        else
          lstrcat( m_strDeviceStats, TEXT(" (simulated pure hw vp)") );
      }
      else
      if(behaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
      {
        if(pDeviceInfo->DevType == D3DDEVTYPE_HAL)
          lstrcat( m_strDeviceStats, TEXT(" (hw vp)") );
        else
          lstrcat( m_strDeviceStats, TEXT(" (simulated hw vp)") );
      }
      else
      if(behaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING)
      {
        if( pDeviceInfo->DevType == D3DDEVTYPE_HAL )
          lstrcat( m_strDeviceStats, TEXT(" (mixed vp)") );
        else
          lstrcat( m_strDeviceStats, TEXT(" (simulated mixed vp)") );
      }
      else
      if(behaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING)
      {
        lstrcat( m_strDeviceStats, TEXT(" (sw vp)") );
      }

      if(pDeviceInfo->DevType == D3DDEVTYPE_HAL)
      {
        // Be sure not to overflow m_strDeviceStats when appending the adapter
        // description, since it can be long.  Note that the adapter description
        // is initially CHAR and must be converted to TCHAR.
        lstrcat( m_strDeviceStats, TEXT(": ") );
        const int cchDesc = sizeof(pAdapterInfo->AdapterIdentifier.Description);
        TCHAR szDescription[cchDesc];
        strncpy(szDescription, pAdapterInfo->AdapterIdentifier.Description, cchDesc);
        int maxAppend = sizeof(m_strDeviceStats) / sizeof(TCHAR) - lstrlen( m_strDeviceStats ) - 1;
        strncat(m_strDeviceStats, szDescription, maxAppend);
      }

      // Store render target surface desc
      CD3D9TexMan *pTM = (CD3D9TexMan *)m_TexMan;
      m_Features |= RFT_ZLOCKABLE;

      m_pd3dDevice->GetDepthStencilSurface( &m_pZBuffer );
      m_pZBuffer->GetDesc( &m_d3dsdZBuffer );
      m_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer );
      m_pBackBuffer->GetDesc(&m_d3dsdBackBuffer);

      // Initialize the app's device-dependent objects
      hr = InitDeviceObjects();
      if( FAILED(hr) )
      {
        DeleteDeviceObjects();
      }
      else
      {
        m_bDeviceObjectsInited = true;
        hr = RestoreDeviceObjects();
        if( FAILED(hr) )
        {
          InvalidateDeviceObjects();
        }
        else
        {
          m_bDeviceObjectsRestored = true;
          return S_OK;
        }
      }

      // Cleanup before we try again
      Cleanup3DEnvironment();
    }
  }

  return hr;
}

struct SMultiSample
{
  D3DMULTISAMPLE_TYPE Type;
  DWORD Quality;
};

int __cdecl MS_Cmp(const void* v1, const void* v2)
{
  SMultiSample *pMS0 = (SMultiSample *)v1;
  SMultiSample *pMS1 = (SMultiSample *)v2;

  if (pMS0->Type < pMS1->Type)
    return -1;
  if (pMS0->Type > pMS1->Type)
    return 1;

  if (pMS0->Quality < pMS1->Quality)
    return -1;
  if (pMS0->Quality > pMS1->Quality)
    return 1;

  return 0;
}

//! Return all supported by video card video AA formats
int CD3D9Renderer::EnumAAFormats(TArray<SAAFormat>& Formats, bool bReset)
{
  int i;
  if (bReset)
  {
    Formats.Free();
    return 0;
  }
  SAAFormat DF;

  D3DAdapterInfo *pAI = m_D3DSettings.PAdapterInfo();
  D3DDeviceCombo *pDev = m_D3DSettings.PDeviceCombo();
  TArray <SMultiSample> MSList;
  int nNONMaskable = 0;
  int nMaskable = 0;
  int nQuality = 0;
  for (i=0; i<pDev->pMultiSampleTypeList->Num(); i++)
  {
    if (pDev->pMultiSampleTypeList->Get(i) == D3DMULTISAMPLE_NONE)
      continue;
    SMultiSample MS;
    MS.Type = pDev->pMultiSampleTypeList->Get(i);
    if (MS.Type == D3DMULTISAMPLE_NONMASKABLE)
      nNONMaskable++;
    else
      nMaskable++;
    for (DWORD j=0; j<pDev->pMultiSampleQualityList->Get(i); j++)
    {
      MS.Quality = j;
      MSList.AddElem(MS);
      if (MS.Type == D3DMULTISAMPLE_NONMASKABLE)
        nQuality++;
    }
  }
  qsort(&MSList[0], MSList.Num(), sizeof(MSList[0]), MS_Cmp);
  DF.nAPIType = D3DMULTISAMPLE_NONMASKABLE;
  DF.nQuality = 0;
  DF.nSamples = 1;
  char str[64];
  for (i=0; i<MSList.Num(); i++)
  {
    if (MSList[i].Type == D3DMULTISAMPLE_NONMASKABLE)
    {
      if (nMaskable)
        continue;
      DF.nAPIType = D3DMULTISAMPLE_NONMASKABLE;
      DF.nQuality = MSList[i].Quality;
      DF.nSamples = 1;
      if (pAI->AdapterIdentifier.VendorId == 4318 && nNONMaskable == 1 && nQuality == 4)
      {
        switch (DF.nQuality)
        {
          case 0:
            strcpy(DF.szDescr, "2x");
            break;
          case 1:
            strcpy(DF.szDescr, "Quincunx");
            break;
          case 2:
            strcpy(DF.szDescr, "4x");
            break;
          case 3:
            strcpy(DF.szDescr, "4xS");
            break;
          default:
            sprintf(str, "AA Quality lev. %d", DF.nQuality);
            strcpy(DF.szDescr, str);
            break;

        }
      }
      else
      {
        sprintf(str, "AA Quality lev. %d", MSList[i].Quality);
        strcpy(DF.szDescr, str);
      }
      Formats.AddElem(DF);
    }
    else
    {
      DF.nAPIType = MSList[i].Type;
      DF.nQuality = MSList[i].Quality;
      DF.nSamples = MSList[i].Type;
      sprintf(str, "%dx Samples", MSList[i].Type);
      strcpy(DF.szDescr, str);
      Formats.AddElem(DF);
    }
  }
  return Formats.Num();
}

int CD3D9Renderer::GetAAFormat(TArray<SAAFormat>& Formats, bool bReset)
{
  int nNums = EnumAAFormats(Formats, bReset);
  if (bReset)
    return nNums;
  int i;

  if (!CV_r_fsaa)
    return -1;
  for (i=0; i<Formats.Num(); i++)
  {
    if (CV_r_fsaa_samples == Formats[i].nSamples && CV_r_fsaa_quality == Formats[i].nQuality)
      return i;
  }
  ICVar *pVar = iConsole->GetCVar("r_FSAA_samples");
  if (pVar)
    pVar->Set(Formats[0].nSamples);
  pVar = iConsole->GetCVar("r_FSAA_quality");
  if (pVar)
    pVar->Set(Formats[0].nQuality);
  return 0;
}


//-----------------------------------------------------------------------------
// Name: BuildPresentParamsFromSettings()
// Desc:
//-----------------------------------------------------------------------------
void CD3D9Renderer::BuildPresentParamsFromSettings()
{
  m_d3dpp.Windowed               = m_D3DSettings.IsWindowed;
  m_d3dpp.BackBufferCount        = CV_d3d9_triplebuffering ? 2 : 1;
  m_d3dpp.MultiSampleType        = m_D3DSettings.MultisampleType();
  m_d3dpp.MultiSampleQuality     = m_D3DSettings.MultisampleQuality();
  m_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
  m_d3dpp.EnableAutoDepthStencil = m_D3DEnum.AppUsesDepthBuffer;
  m_d3dpp.hDeviceWindow          = m_hWnd;
  if( m_D3DEnum.AppUsesDepthBuffer )
  {
    if (CV_d3d9_forcesoftware)
      m_d3dpp.Flags              = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    else
      m_d3dpp.Flags              = 0;
    m_d3dpp.AutoDepthStencilFormat = m_D3DSettings.DepthStencilBufferFormat();
  }
  else
  {
    m_d3dpp.Flags              = 0;
  }

  if(!m_bFullScreen)
  {
    if (m_bEditor)
    {
      m_d3dpp.BackBufferWidth  = m_deskwidth;
      m_d3dpp.BackBufferHeight = m_deskheight;
    }
    else
    {
      m_d3dpp.BackBufferWidth  = m_width;
      m_d3dpp.BackBufferHeight = m_height;
    }

    m_d3dpp.BackBufferFormat = m_D3DSettings.PDeviceCombo()->BackBufferFormat;
    m_d3dpp.FullScreen_RefreshRateInHz = 0;
    m_d3dpp.PresentationInterval = m_D3DSettings.PresentInterval();
  }
  else
  {
    m_d3dpp.BackBufferWidth  = m_D3DSettings.DisplayMode().Width;
    m_d3dpp.BackBufferHeight = m_D3DSettings.DisplayMode().Height;
    m_d3dpp.BackBufferFormat = m_D3DSettings.PDeviceCombo()->BackBufferFormat;
    m_d3dpp.FullScreen_RefreshRateInHz = m_D3DSettings.Fullscreen_DisplayMode.RefreshRate;
    m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; //m_D3DSettings.PresentInterval();
  }
  if (CV_r_fsaa)
  {
    TArray<SAAFormat> Formats;
    int nNum = GetAAFormat(Formats, false);
    D3DMULTISAMPLE_TYPE nType = (D3DMULTISAMPLE_TYPE)Formats[nNum].nAPIType;
    DWORD nQualityLevel = 0;
    HRESULT hr = m_pD3D->CheckDeviceMultiSampleType(m_D3DSettings.AdapterOrdinal(), m_D3DSettings.DevType(), m_d3dpp.BackBufferFormat, m_d3dpp.Windowed, nType, &nQualityLevel);

		if (!FAILED(hr) && nQualityLevel >= (DWORD)Formats[nNum].nQuality)
		{
      HRESULT hr = m_pD3D->CheckDeviceMultiSampleType(m_D3DSettings.AdapterOrdinal(), m_D3DSettings.DevType(), m_d3dpp.AutoDepthStencilFormat, m_d3dpp.Windowed, nType, &nQualityLevel);
      if (!FAILED(hr) && nQualityLevel >= (DWORD)Formats[nNum].nQuality)
      {
				if( CV_r_fsaa == 1 )
				{
					m_d3dpp.MultiSampleType = nType;
					m_d3dpp.MultiSampleQuality = Formats[nNum].nQuality;
					m_Features |= RFT_SUPPORTFSAA;
				}
				else
				{
					// Use special mode to enable FSAA in HDR which requires the d3d device to be setup w/o a FSAA backbuffer!
					m_d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
					m_d3dpp.MultiSampleQuality = 0;
				}
      }
    }
    m_FSAA = CV_r_fsaa;
    m_FSAA_samples = Formats[nNum].nSamples;
    m_FSAA_quality = Formats[nNum].nQuality;
    GetAAFormat(Formats, true);
  }
  else
	{
    m_FSAA = 0;
		m_d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
		m_d3dpp.MultiSampleQuality = 0;
	}
}



//-----------------------------------------------------------------------------
// Name: Cleanup3DEnvironment()
// Desc: Cleanup scene objects
//-----------------------------------------------------------------------------
void CD3D9Renderer::Cleanup3DEnvironment()
{
  if( m_pd3dDevice != NULL )
  {
    if( m_bDeviceObjectsRestored )
    {
      m_bDeviceObjectsRestored = false;
      InvalidateDeviceObjects();
    }
    if( m_bDeviceObjectsInited )
    {
      m_bDeviceObjectsInited = false;
      DeleteDeviceObjects();
    }
    RestoreGamma();

    if( m_pd3dDevice->Release() > 0 )
    {
      iLog->Log("ERROR: CD3D9Renderer::Cleanup3DEnvironment: Non zero reference counter after release of D3D Device");
      while( m_pd3dDevice->Release() ) {}
    }
    m_pd3dDevice = NULL;
  }
}


//-----------------------------------------------------------------------------
// Name: Reset3DEnvironment()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3D9Renderer::Reset3DEnvironment()
{
  HRESULT hr;

  // Release all vidmem objects
  if( m_bDeviceObjectsRestored )
  {
    m_bDeviceObjectsRestored = false;
    InvalidateDeviceObjects();
  }

  /*for (int i=0; i<gDVB.Num(); i++)
  {
    SDynVB *vb = &gDVB[i];
    if (vb->m_pRes->GetType() == D3DRTYPE_INDEXBUFFER)
    {
      D3DINDEXBUFFER_DESC desc;
      IDirect3DIndexBuffer9 *pIB = (IDirect3DIndexBuffer9 *)vb->m_pRes;
      pIB->GetDesc(&desc);
      assert(desc.Pool != D3DPOOL_DEFAULT);
    }
    else
    if (vb->m_pRes->GetType() == D3DRTYPE_VERTEXBUFFER)
    {
      D3DVERTEXBUFFER_DESC desc;
      IDirect3DVertexBuffer9 *pIB = (IDirect3DVertexBuffer9 *)vb->m_pRes;
      pIB->GetDesc(&desc);
      assert(desc.Pool != D3DPOOL_DEFAULT);
    }
    else
      assert(0);
  }*/

  // Reset the device
  if( FAILED( hr = m_pd3dDevice->Reset( &m_d3dpp ) ) )
    return hr;

  // Initialize the app's device-dependent objects
  hr = RestoreDeviceObjects();
  if( FAILED(hr) )
  {
    InvalidateDeviceObjects();
    return hr;
  }

  m_bDeviceObjectsRestored = true;
  m_bTemporaryDisabledSFX = false;

  return S_OK;
}

bool CD3D9Renderer::ChooseDevice(void)
{
  HRESULT hr;

  // Save window properties
  GetWindowRect( m_hWnd, &m_rcWindowBounds );
  GetClientRect( m_hWnd, &m_rcWindowClient );

  if(FAILED(hr = ChooseInitialD3DSettings()))
    return Error("Couldn't find any suitable device", hr);

  char drv[64];
  strcpy(drv, CV_d3d9_device->GetString());

  int numAdap = -1;
  bool bAuto = false;
  if (!strnicmp(drv, "auto", 4))
    bAuto = true;
  else
  if (!strnicmp(drv, "primary", 7))
    numAdap = 0;
  else
  if (!strnicmp(drv, "3dfx", 4) || !strnicmp(drv, "second", 6) || !strnicmp(drv, "Voodoo", 6))
    numAdap = 1;
  else
  if (drv[0] && isdigit(drv[0]))
    numAdap = atol(drv);
  else
  {
    iLog->Log ( "Unknown device name '%s' (use AutoDetect)\n", drv);
    iLog->Log ( "Only 'Auto', 'Primary', '3dfx' or digital device ID are supported\n");
    bAuto = true;
  }
  if (numAdap < 0)
    numAdap = 0;

  int nDev = -1;
  bool bAllowSoft = false;
  while (true)
  {
    if (bAuto)
    {
      for(int a=0; a<m_D3DEnum.m_pAdapterInfoList->Num(); a++)
      {
        if ((nDev=FindSuitableDevice(a, bAllowSoft)) >= 0)
        {
          numAdap = a;
          break;
        }
      }
    }
    else
    {
      nDev = FindSuitableDevice(numAdap, bAllowSoft);
    }

    if (nDev < 0)
    {
      if (!bAllowSoft)
      {
        if (CV_d3d9_allowsoftware)
        {
          bAllowSoft = true;
          continue;
        }
      }
      return Error("Can't find any suitable D3D device\n", 0);
    }
    break;
  }
  // The focus window can be a specified to be a different window than the
  // device window.  If not, use the device window as the focus window.
  if( m_CurrContext == NULL )
    CreateContext(m_hWnd);
  return true;
}

bool CD3D9Renderer::SetRes(void)
{
  UnSetRes();

  HRESULT hr;

  // Create the Direct3D object
  m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
  if( m_pD3D == NULL )
    return Error("DirectX9 not installed", 0);

  m_D3DEnum.SetD3D(m_pD3D);
  m_D3DEnum.ConfirmDeviceCallback = ConfirmDeviceHelper;

  // Build a list of Direct3D adapters, modes and devices. The
  // ConfirmDevice() callback is used to confirm that only devices that
  // meet the app's requirements are considered.
  if(FAILED(hr = m_D3DEnum.Enumerate()))
  {
    if (hr == D3DAPPERR_NOWINDOWABLEDEVICES)
    {
      if (!m_bFullScreen)
        return Error("Couldn't build list of D3D devices", hr);
    }
    else
      return Error("Couldn't build list of D3D devices", hr);
  }

  ChooseDevice();

  // Initialize the 3D environment for the app
  if( FAILED( hr = Initialize3DEnvironment() ) )
    return Error("Couldn't initialize 3D environment", hr);

  if (FAILED (hr = D3DXCreateMatrixStack(0, &m_matView)))
    return false;
  if (FAILED (hr = D3DXCreateMatrixStack(0, &m_matProj)))
    return false;
  D3DXMatrixIdentity(&m_TexMatrix[0]);
  D3DXMatrixIdentity(&m_TexMatrix[1]);
  D3DXMatrixIdentity(&m_TexMatrix[2]);
  D3DXMatrixIdentity(&m_TexMatrix[3]);

  RestoreDeviceObjects();

  return true;
}

//-----------------------------------------------------------------------------
// Name: CD3D9Renderer::mfAdjustWindowForChange()
// Desc: Prepare the window for a possible change between windowed mode and
//       fullscreen mode.  This function is virtual and thus can be overridden
//       to provide different behavior, such as switching to an entirely
//       different window for fullscreen mode (as in the MFC sample apps).
//-----------------------------------------------------------------------------
HRESULT CD3D9Renderer::AdjustWindowForChange()
{
  if (m_bEditor)
    return S_OK;
  if( !m_bFullScreen )
  {
    // Set windowed-mode style
    SetWindowLong( m_hWnd, GWL_STYLE, m_dwWindowStyle );
  }
  else
  {
    // Set fullscreen-mode style
    SetWindowLong( m_hWnd, GWL_STYLE, WS_POPUP|WS_SYSMENU|WS_VISIBLE );
  }

  return S_OK;
}

HRESULT CD3D9Renderer::InitDeviceObjects()
{
  int i;

  // Gamma correction support
  if (m_d3dCaps.Caps2 & D3DCAPS2_FULLSCREENGAMMA)
    m_Features |= RFT_HWGAMMA;
  if (m_d3dCaps.Caps2 & D3DCAPS2_CANCALIBRATEGAMMA)
    m_bGammaCalibrate = true;
  else
    m_bGammaCalibrate = false;

  // Device Id's
  D3DAdapterInfo *pAI = m_D3DSettings.PAdapterInfo();
  D3DADAPTER_IDENTIFIER9 *ai = &pAI->AdapterIdentifier;
  iLog->Log ( "D3D Adapter: Driver name: %s\n", ai->Driver);
  iLog->Log ( "D3D Adapter: Driver description: %s\n", ai->Description);
  iLog->Log ( "D3D Adapter: Driver version: %d.%02d.%02d.%04d\n", HIWORD( ai->DriverVersion.u.HighPart ), LOWORD( ai->DriverVersion.u.HighPart ), HIWORD(ai->DriverVersion.u.LowPart), LOWORD(ai->DriverVersion.u.LowPart));
  // Unique driver/device identifier:
  GUID *pGUID = &ai->DeviceIdentifier;
  iLog->Log ( "D3D Adapter: Driver GUID: %08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X\n", pGUID->Data1, pGUID->Data2, pGUID->Data3, pGUID->Data4[0], pGUID->Data4[1], pGUID->Data4[2], pGUID->Data4[3], pGUID->Data4[4], pGUID->Data4[5], pGUID->Data4[6], pGUID->Data4[7] );
  iLog->Log ( "D3D Adapter: VendorId = %i\n", ai->VendorId);
  iLog->Log ( "D3D Adapter: DeviceId = %i\n", ai->DeviceId);
  iLog->Log ( "D3D Adapter: SubSysId = %i\n", ai->SubSysId);
  iLog->Log ( "D3D Adapter: Revision = %i\n", ai->Revision);

  // Hardware-specific initialization and workarounds for driver bugs.
  {
    bool ConstrainAspect = 1;
    if( CV_d3d9_nodeviceid )
    {
      iLog->Log ("D3D Detected: -nodeviceid specified, 3D device identification skipped\n");
    }
    else
    if( ai->VendorId==4098 )
    {
      m_Features &= ~RFT_HW_MASK;
      m_Features |= RFT_HW_RADEON;
      iLog->Log ("D3D Detected: ATI video card\n");
      if( ai->DeviceId==18242 )
      {
        iLog->Log ("D3D Detected: ATI Rage Pro\n");
      }
      else
      if( ai->DeviceId==21062 )
      {
        iLog->Log ("D3D Detected: ATI Rage 128\n");
      }
      else
      if( ai->DeviceId==20812 )
      {
        iLog->Log ("D3D Detected: ATI Radeon 8500\n");
      }
      else
      if( ai->DeviceId==20036 )
      {
        iLog->Log ("D3D Detected: ATI Radeon 9700\n");
      }
      else
      if( ai->DeviceId==0x4a49 )
      {
        iLog->Log ("D3D Detected: ATI Radeon X800 Pro\n");
      }
      if( ai->DeviceId==0x4a49 || ai->DeviceId==0x4a4a || ai->DeviceId==0x4a4b || ai->DeviceId==0x4a4c || ai->DeviceId==0x4a50 || ai->DeviceId==0x4a69 || ai->DeviceId==0x4a6a || ai->DeviceId==0x4a6b || ai->DeviceId==0x4a6c || ai->DeviceId==0x4a70 ||
          ai->DeviceId==0x5b60 || ai->DeviceId==0x5b70 || ai->DeviceId==0x3e50 || ai->DeviceId==0x3e70 || ai->DeviceId==0x5549 || ai->DeviceId==0x5569 || ai->DeviceId==0x554b || ai->DeviceId==0x556b)
      {
        // Workaround for R42x to avoid terrain flickering
        _SetVar("d3d9_VBPools", 0);
      }
    }
    else
    if( ai->VendorId==4634 )
    {
      iLog->Log ("D3D Detected: 3dfx video card\n");
      if( ai->DeviceId==1 )
      {
        iLog->Log ("D3D Detected: 3dfx Voodoo\n");
      }
      else
      if( ai->DeviceId==2 )
      {
        iLog->Log ("D3D Detected: 3dfx Voodoo2\n");
      }
      else
      if( ai->DeviceId==3 )
      {
        iLog->Log ("D3D Detected: 3dfx Voodoo Banshee\n");
      }
      else
      if( ai->DeviceId==5 )
      {
        iLog->Log ("D3D Detected: 3dfx Voodoo3\n");
      }
    }
    else
    if( ai->VendorId==32902 )
    {
      iLog->Log ("D3D Detected: Intel video card\n");
      if( ai->DeviceId==30720 )
      {
        iLog->Log ("D3D Detected: Intel i740\n");
      }
      if( ai->DeviceId==7121 )
      {
        iLog->Log ("D3D Detected: Intel 810L\n");
      }
      if( ai->DeviceId==7123 )
      {
        iLog->Log ("D3D Detected: Intel 810 DC100\n");
      }
      if( ai->DeviceId==7125 )
      {
        iLog->Log ("D3D Detected: Intel 810E\n");
      }
    }
    else
    if( ai->VendorId==4818 )
    {
      iLog->Log ("D3D Detected: NVidia Riva video card\n");
      if( ai->DeviceId==0x18 || ai->DeviceId==0x19 )
        iLog->Log ("D3D Detected: Riva 128\n");
      else
        iLog->Log ("D3D Detected: Riva unknown\n");
    }
    else
    if( ai->VendorId==4318 )
    {
      m_Features |= RFT_FOGVP;
      iLog->Log ("D3D Detected: NVidia video card\n");
      if( ai->DeviceId==0x20 )
      {
        iLog->Log ("D3D Detected: Riva TNT\n");
        ConstrainAspect = 0;
      }
      if( ai->DeviceId==0x28 )
      {
        iLog->Log ("D3D Detected: Riva TNT2/TNT2 Pro\n");
        ConstrainAspect = 0;
      }
      if( ai->DeviceId==0x29 )
      {
        iLog->Log ("D3D Detected: Riva TNT2 Ultra\n");
        ConstrainAspect = 0;
      }
      if( ai->DeviceId==0x2c)
      {
        iLog->Log ("D3D Detected: Vanta/Vanta LT\n");
        ConstrainAspect = 0;
      }
      if( ai->DeviceId==0x2d)
      {
        iLog->Log ("D3D Detected: Riva TNT2 Model 64/Model 64 Pro\n");
        ConstrainAspect = 0;
      }
      if( ai->DeviceId==0x40 )
      {
        iLog->Log ("D3D Detected: Riva TNT2\n");
        ConstrainAspect = 0;
      }
      if( ai->DeviceId==0xa0)
      {
        iLog->Log ("D3D Detected: Aladdin TNT2\n");
        ConstrainAspect = 0;
      }
      if (ai->DeviceId == 0x100)
        iLog->Log ("D3D Detected: GeForce 256 SDR\n");
      if (ai->DeviceId == 0x101)
        iLog->Log ("D3D Detected: GeForce 256 DDR\n");
      if (ai->DeviceId == 0x103)
        iLog->Log ("D3D Detected: Quadro\n");
      if (ai->DeviceId == 0x110)
        iLog->Log ("D3D Detected: GeForce2 MX/MX 400\n");
      if (ai->DeviceId == 0x111)
        iLog->Log ("D3D Detected: GeForce2 MX 100/200\n");
      if (ai->DeviceId == 0x112)
        iLog->Log ("D3D Detected: GeForce2 Go\n");
      if (ai->DeviceId == 0x113)
        iLog->Log ("D3D Detected: Quadro2 MXR/EX/Go\n");
      if (ai->DeviceId == 0x150)
        iLog->Log ("D3D Detected: GeForce2 GTS/GeForce2 Pro\n");
      if (ai->DeviceId == 0x151)
        iLog->Log ("D3D Detected: GeForce2 Ti\n");
      if (ai->DeviceId == 0x152)
        iLog->Log ("D3D Detected: GeForce2 Ultra\n");
      if (ai->DeviceId == 0x153)
        iLog->Log ("D3D Detected: Quadro2 Pro\n");
      if (ai->DeviceId == 0x170)
        iLog->Log ("D3D Detected: GeForce4 MX 460\n");
      if (ai->DeviceId == 0x171)
        iLog->Log ("D3D Detected: GeForce4 MX 440\n");
      if (ai->DeviceId == 0x172)
        iLog->Log ("D3D Detected: GeForce4 MX 420\n");
      if (ai->DeviceId == 0x173)
        iLog->Log ("D3D Detected: GeForce4 MX 440-SE\n");
      if (ai->DeviceId == 0x174)
        iLog->Log ("D3D Detected: GeForce4 440 Go\n");
      if (ai->DeviceId == 0x175)
        iLog->Log ("D3D Detected: GeForce4 420 Go\n");
      if (ai->DeviceId == 0x176)
        iLog->Log ("D3D Detected: GeForce4 420 Go 32M\n");
      if (ai->DeviceId == 0x177)
        iLog->Log ("D3D Detected: GeForce4 460 Go\n");
      if (ai->DeviceId == 0x178)
        iLog->Log ("D3D Detected: Quadro4 500 XGL\n");
      if (ai->DeviceId == 0x179)
        iLog->Log ("D3D Detected: GeForce4 440 Go 64M\n");
      if (ai->DeviceId == 0x17a)
        iLog->Log ("D3D Detected: Quadro4 200 NVS\n");
      if (ai->DeviceId == 0x17c)
        iLog->Log ("D3D Detected: Quadro4 500 GoGL\n");
      if (ai->DeviceId == 0x17d)
        iLog->Log ("D3D Detected: GeForce4 410 Go 16M\n");
      if (ai->DeviceId == 0x181)
        iLog->Log ("D3D Detected: GeForce4 MX 440 with AGP8X\n");
      if (ai->DeviceId == 0x182)
        iLog->Log ("D3D Detected: GeForce4 MX 440-SE with AGP8X\n");
      if (ai->DeviceId == 0x183)
        iLog->Log ("D3D Detected: GeForce4 MX 420 with AGP8X\n");
      if (ai->DeviceId == 0x186)
        iLog->Log ("D3D Detected: GeForce4 448 Go\n");
      if (ai->DeviceId == 0x187)
        iLog->Log ("D3D Detected: GeForce4 488 Go\n");
      if (ai->DeviceId == 0x188)
        iLog->Log ("D3D Detected: Quadro4 580 XGL\n");
      if (ai->DeviceId == 0x18a)
        iLog->Log ("D3D Detected: Quadro NVS with AGP8X\n");
      if (ai->DeviceId == 0x18b)
        iLog->Log ("D3D Detected: Quadro4 380 XGL\n");
      if (ai->DeviceId == 0x1a0)
        iLog->Log ("D3D Detected: GeForce2 Integrated GPU\n");
      if (ai->DeviceId == 0x1f0)
        iLog->Log ("D3D Detected: GeForce4 MX Integrated GPU\n");

      if (ai->DeviceId == 0x200)
        iLog->Log ("D3D Detected: GeForce3\n");
      if (ai->DeviceId == 0x201)
        iLog->Log ("D3D Detected: GeForce3 Ti 200\n");
      if (ai->DeviceId == 0x202)
        iLog->Log ("D3D Detected: GeForce3 Ti 500\n");
      if (ai->DeviceId == 0x203)
        iLog->Log ("D3D Detected: Quadro DCC\n");
      if (ai->DeviceId == 0x250)
        iLog->Log ("D3D Detected: GeForce4 Ti 4600\n");
      if (ai->DeviceId == 0x251)
        iLog->Log ("D3D Detected: GeForce4 Ti 4400\n");
      if (ai->DeviceId == 0x251)
        iLog->Log ("D3D Detected: NV25\n");
      if (ai->DeviceId == 0x253)
        iLog->Log ("D3D Detected: GeForce4 Ti 4200\n");
      if (ai->DeviceId == 0x258)
        iLog->Log ("D3D Detected: Quadro4 900 XGL\n");
      if (ai->DeviceId == 0x259)
        iLog->Log ("D3D Detected: Quadro4 750 XGL\n");
      if (ai->DeviceId == 0x25b)
        iLog->Log ("D3D Detected: Quadro4 700 XGL\n");
      if (ai->DeviceId == 0x280)
        iLog->Log ("D3D Detected: GeForce4 TI4800\n");
      if (ai->DeviceId == 0x281)
        iLog->Log ("D3D Detected: GeForce4 TI4200 with AGP8X\n");
      if (ai->DeviceId == 0x282)
        iLog->Log ("D3D Detected: GeForce4 TI4800 SE\n");
      if (ai->DeviceId == 0x286)
        iLog->Log ("D3D Detected: GeForce4 4200 Go\n");
      if (ai->DeviceId == 0x288)
        iLog->Log ("D3D Detected: Quadro4 980 XGL\n");
      if (ai->DeviceId == 0x289)
        iLog->Log ("D3D Detected: Quadro4 780 XGL\n");
      if (ai->DeviceId == 0x28c)
        iLog->Log ("D3D Detected: Quadro4 700 GoGL\n");

      if (ai->DeviceId == 0x300)
        iLog->Log ("D3D Detected: GeForce FX 5800\n");
      if (ai->DeviceId == 0x301)
        iLog->Log ("D3D Detected: GeForce FX 5800 Ultra\n");
      if (ai->DeviceId == 0x302)
        iLog->Log ("D3D Detected: GeForce FX 5800\n");
      if (ai->DeviceId == 0x308)
        iLog->Log ("D3D Detected: QuadroFX 2000\n");
      if (ai->DeviceId == 0x309)
        iLog->Log ("D3D Detected: QuadroFX 1000\n");
      if (ai->DeviceId == 0x30a)
        iLog->Log ("D3D Detected: ICE FX 2000\n");
      if (ai->DeviceId == 0x311)
        iLog->Log ("D3D Detected: GeForce FX 5600 Ultra\n");
      if (ai->DeviceId == 0x312)
        iLog->Log ("D3D Detected: GeForce FX 5600\n");
      if (ai->DeviceId == 0x313)
        iLog->Log ("D3D Detected: NV31\n");
      if (ai->DeviceId == 0x314)
        iLog->Log ("D3D Detected: GeForce FX 5600SE\n");
      if (ai->DeviceId == 0x316)
        iLog->Log ("D3D Detected: NV31M\n");
      if (ai->DeviceId == 0x317)
        iLog->Log ("D3D Detected: NV31M Pro\n");
      if (ai->DeviceId == 0x31a)
        iLog->Log ("D3D Detected: GeForce FX Go5600\n");
      if (ai->DeviceId == 0x31b)
        iLog->Log ("D3D Detected: GeForce FX Go5650\n");
      if (ai->DeviceId == 0x31c)
        iLog->Log ("D3D Detected: Quadro FX Go700\n");
      if (ai->DeviceId == 0x31d)
        iLog->Log ("D3D Detected: NV31GLM\n");
      if (ai->DeviceId == 0x31e)
        iLog->Log ("D3D Detected: NV31GLM Pro\n");
      if (ai->DeviceId == 0x31f)
        iLog->Log ("D3D Detected: NV31GLM Pro\n");
      if (ai->DeviceId == 0x321)
        iLog->Log ("D3D Detected: GeForce FX 5200 Ultra\n");
      if (ai->DeviceId == 0x322)
        iLog->Log ("D3D Detected: GeForce FX 5200\n");
      if (ai->DeviceId == 0x323)
        iLog->Log ("D3D Detected: GeForce FX 5200SE\n");
      if (ai->DeviceId == 0x324)
        iLog->Log ("D3D Detected: GeForce FX Go5200\n");
      if (ai->DeviceId == 0x325)
        iLog->Log ("D3D Detected: GeForce FX Go5250\n");
      if (ai->DeviceId == 0x328)
        iLog->Log ("D3D Detected: GeForce FX Go5200 32M/64M\n");
      if (ai->DeviceId == 0x32a)
        iLog->Log ("D3D Detected: NV34GL\n");
      if (ai->DeviceId == 0x32b)
        iLog->Log ("D3D Detected: Quadro FX 500\n");
      if (ai->DeviceId == 0x32d)
        iLog->Log ("D3D Detected: GeForce FX Go5100\n");
      if (ai->DeviceId == 0x32f)
        iLog->Log ("D3D Detected: NV34GL\n");
      if (ai->DeviceId == 0x330)
        iLog->Log ("D3D Detected: GeForce FX 5900 Ultra\n");
      if (ai->DeviceId == 0x331)
        iLog->Log ("D3D Detected: GeForce FX 5900\n");
      if (ai->DeviceId == 0x332)
        iLog->Log ("D3D Detected: NV35\n");

      if (ai->DeviceId <= 0x40 && ai->DeviceId <= 0x4f)
        iLog->Log ("D3D Detected: NV4X\n");
    }
    else
    if( ai->VendorId==4818 )
    {
      iLog->Log ("D3D Detected: NVidia video card\n");
      if( ai->DeviceId==0x18 || ai->DeviceId==0x19)
      {
        iLog->Log ("D3D Detected: Riva 128\n");
        ConstrainAspect = 0;
      }
    }
    else
    if( ai->VendorId==4139 )
    {
      iLog->Log ("D3D Detected: Matrox video card\n");
      if( ai->DeviceId==1313 )
        iLog->Log ("D3D Detected: Matrox G200\n");
      else
      if( ai->DeviceId==1317 )
        iLog->Log ("D3D Detected: Matrox G400\n");
        //G400 lies about texture stages, last one is for bump only
    }
    else
    if( ai->VendorId==0x18ca )
    {
      m_Features &= ~RFT_HW_MASK;
      m_Features |= RFT_HW_RADEON;
      iLog->Log ("D3D Detected: XGI video card\n");
      if( ai->DeviceId==0x40 )
        iLog->Log ("D3D Detected: XGI Volary V8 DUO Ultra\n");
      else
        iLog->Log ("D3D Detected: XGI Unknown\n");
        //G400 lies about texture stages, last one is for bump only
    }
    else
    {
      iLog->Log ("D3D Detected: Generic 3D accelerator\n");
      // Hack for NVidia DualView
      if (!strnicmp(ai->Description, "NVIDIA", 6))
        m_Features |= RFT_FOGVP;
    }
  }

  if (m_d3dCaps.PS20Caps.NumInstructionSlots >= 256 && (D3DSHADER_VERSION_MAJOR(m_d3dCaps.PixelShaderVersion) >= 2))
    m_bDeviceSupports_PS2X = true;

  // Set viewport.
  SetViewport(0,0,CRenderer::m_width, CRenderer::m_height);

  // Check multitexture caps.
  iLog->Log ( "D3D Driver: MaxTextureBlendStages   = %i\n", m_d3dCaps.MaxTextureBlendStages);
  iLog->Log ( "D3D Driver: MaxSimultaneousTextures = %i\n", m_d3dCaps.MaxSimultaneousTextures);
  if( m_d3dCaps.MaxSimultaneousTextures>=2 )
    m_Features |= RFT_MULTITEXTURE;

  // Handle the texture formats we need.
  {
    // Zero them all.
    mFormatA8.Init();     // A
    mFormatA8L8.Init();   // A8L8
    mFormat8888.Init();   // BGRA
    mFormat4444.Init();   // BGRA
    mFormat0565.Init();   // BGR
    mFormat0555.Init();   // BGR
    mFormat1555.Init();   // BGRA
    mFormatV8U8.Init();   // Bump
    mFormatCxV8U8.Init();   // Bump
    mFormatU5V5L6.Init(); // Bump
    mFormat3Dc.Init();    // 3Dc
    mFormatDXT1.Init();   // DXT1
    mFormatDXT3.Init();   // DXT3
    mFormatDXT5.Init();   // DXT5
    mFormatPal8.Init();   // Paletted8
    mFormatDepth24.Init(); // Depth teture
    mFormatDepth16.Init(); // Depth teture

    // Find the needed texture formats.
    {
      mFirstPixelFormat = NULL;
      if(m_TextureBits >= 24)
      {
        if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8)))
          RecognizePixelFormat(mFormat8888, D3DFMT_A8R8G8B8, 32, "8888");
      }
      if( !mFirstPixelFormat )
      {
        m_TextureBits = 16;

        if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_A1R5G5B5)))
          RecognizePixelFormat(mFormat1555, D3DFMT_A1R5G5B5, 16, "1555");

        if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_A4R4G4B4)))
          RecognizePixelFormat(mFormat4444, D3DFMT_A4R4G4B4, 16, "4444");

        if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_R5G6B5)))
          RecognizePixelFormat(mFormat0565, D3DFMT_R5G6B5, 16, "0565");
        else
        if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_X1R5G5B5)))
          RecognizePixelFormat(mFormat0555, D3DFMT_X1R5G5B5, 16, "0555");
      }
      // Alpha formats
      if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_A8)))
        RecognizePixelFormat(mFormatA8, D3DFMT_A8, 8, "Alpha8");
      if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_A8L8)))
        RecognizePixelFormat(mFormatA8L8, D3DFMT_A8L8, 16, "Alpha8Lum8");

      if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_V16U16)))
        RecognizePixelFormat(mFormatU16V16, D3DFMT_V16U16, 32, "V16U16");

      // Depth formats
      if (CV_d3d9_forcesoftware)
        m_Features |= RFT_DEPTHMAPS;
      else
      if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_D24S8)))
      {
        m_Features |= RFT_DEPTHMAPS;
        RecognizePixelFormat(mFormatDepth24, D3DFMT_D24S8, 8, "Depth24");
      }
      else
      if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_D16)))
      {
        m_Features |= RFT_DEPTHMAPS;
        RecognizePixelFormat(mFormatDepth16, D3DFMT_D16, 8, "Depth16");
      }
			//// REMOVED DRIVER HACK -- Carsten
   //   // Workaround for NVidia drivers less than 61.00
   //   // Depth maps are working extremely bad on rel5X drivers
   //   if (LOWORD(ai->DriverVersion.u.LowPart) < 6100)
   //   {
   //     ICVar *pVar = iConsole->GetCVar("d3d9_NoDepthMaps");
   //     if (pVar)
   //       pVar->Set(1);
   //   }
      if (CV_r_shadowtype == 1 || CV_d3d9_nodepthmaps)
        m_Features &= ~RFT_DEPTHMAPS;

      // Bump formats
      if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_V8U8)))
      {
        RecognizePixelFormat(mFormatV8U8, D3DFMT_V8U8, 16, "BumpV8U8");
        m_bDeviceSupportsComprNormalmaps = 2;
      }
      if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_CxV8U8)))
      {
        RecognizePixelFormat(mFormatCxV8U8, D3DFMT_CxV8U8, 16, "BumpCxV8U8");
        //m_bDeviceSupportsComprNormalmaps = 3;
      }

      if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_Q8W8V8U8)))
        RecognizePixelFormat(mFormatQ8W8U8V8, D3DFMT_Q8W8V8U8, 32, "BumpQ8W8U8V8");
      if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_X8L8V8U8)))
        RecognizePixelFormat(mFormatX8L8U8V8, D3DFMT_X8L8V8U8, 32, "BumpX8L8U8V8");
      if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_L6V5U5)))
        RecognizePixelFormat(mFormatU5V5L6, D3DFMT_L6V5U5, 16, "BumpU5V5L6");
      if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, (D3DFORMAT)(MAKEFOURCC('A', 'T', 'I', '2')))))
      {
        RecognizePixelFormat(mFormat3Dc, (D3DFORMAT)(MAKEFOURCC('A', 'T', 'I', '2')), 16, "3Dc");
        m_bDeviceSupportsComprNormalmaps = 1;
      }

      // Paletted formats
      if (CV_d3d9_palettedtextures)
      {
        if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_P8)))
        {
          RecognizePixelFormat(mFormatPal8, D3DFMT_P8, 8, "Paletted8");
          m_Features |= RFT_PALTEXTURE;
        }
      }

      // Compressed formats
      if (CV_d3d9_compressedtextures)
      {
        if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_DXT1)))
        {
          RecognizePixelFormat(mFormatDXT1, D3DFMT_DXT1, 8, "DXT1");
          m_Features |= RFT_COMPRESSTEXTURE;
        }
        if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_DXT3)))
        {
          RecognizePixelFormat(mFormatDXT3, D3DFMT_DXT3, 8, "DXT3");
          m_Features |= RFT_COMPRESSTEXTURE;
        }
        if(SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), 0, D3DRTYPE_TEXTURE, D3DFMT_DXT5)))
        {
          RecognizePixelFormat(mFormatDXT5, D3DFMT_DXT5, 8, "DXT5");
          m_Features |= RFT_COMPRESSTEXTURE;
        }
      }
    }
  }

  // Verify mipmapping supported.
  if (!(m_d3dCaps.TextureCaps & D3DPTEXTURECAPS_MIPMAP))
  {
    iLog->Log ("D3D Driver: Mipmapping not available with this driver\n");
    //m_Features |= RFT_NOMIPS;
  }
  else
  {
    if( m_d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR )
      iLog->Log ("D3D Driver: Supports trilinear texture filtering\n");
  }

  if (!(m_d3dCaps.TextureCaps & D3DPTEXTURECAPS_NOPROJECTEDBUMPENV))
  {
    iLog->Log ("D3D Driver: Allowed projected textures with Env. bump mapping\n");
    m_Features |= RFT_HW_ENVBUMPPROJECTED;
  }
  else
    iLog->Log ("D3D Driver: projected textures with Env. bump mapping not allowed\n");

  if( m_d3dCaps.TextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE )
  {
    iLog->Log ("D3D Driver: Supports alpha palettes\n");
    m_bAllowAlphaPalettes = true;
  }
  if( m_d3dCaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY )
  {
    m_d3dCaps.MaxTextureAspectRatio = 1;
    iLog->Log ("D3D Driver: Requires square textures\n");
  }
  else
  if( !m_d3dCaps.MaxTextureAspectRatio )
    m_d3dCaps.MaxTextureAspectRatio = max(1,max(m_d3dCaps.MaxTextureWidth,m_d3dCaps.MaxTextureHeight));
  if( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_FOGRANGE )
    iLog->Log ("D3D Driver: Supports range-based fog\n");
  if( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_WFOG )
    iLog->Log ("D3D Driver: Supports eye-relative fog\n");
  if( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_ANISOTROPY )
    iLog->Log ("D3D Driver: Supports anisotropic filtering\n");
  if( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS )
    iLog->Log ("D3D Driver: Supports LOD biasing\n");
  if( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_DEPTHBIAS )
    iLog->Log ("D3D Driver: Supports Z biasing\n");
  else
    m_Features &= ~RFT_SUPPORTZBIAS;
  if( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR )
    iLog->Log ("D3D Driver: Device can perform hidden-surface removal (HSR)\n");
  if( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_SCISSORTEST )
    iLog->Log ("D3D Driver: Supports scissor test\n");
  if (m_d3dCaps.DevCaps2 & D3DDEVCAPS2_STREAMOFFSET)
    iLog->Log ("D3D Driver: Supports stream offset\n");

  if( !(m_d3dCaps.TextureCaps & D3DPTEXTURECAPS_POW2) )
    iLog->Log ("D3D Driver: Supports non-power-of-2 textures\n");
  if( m_d3dCaps.TextureOpCaps & D3DTEXOPCAPS_ADDSIGNED2X )
    iLog->Log ("D3D Driver: Supports D3DTOP_ADDSIGNED2X TextureOp\n");
  if( m_d3dCaps.TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP )
    iLog->Log ("D3D Driver: Supports D3DTOP_BUMPENVMAP TextureOp\n");
  if( m_d3dCaps.TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAPLUMINANCE )
    iLog->Log ("D3D Driver: Supports D3DTOP_BUMPENVMAPLUMINANCE TextureOp\n");
  if( m_d3dCaps.TextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3 )
    iLog->Log ("D3D Driver: Supports D3DTOP_DOTPRODUCT3 TextureOp\n");
  if( m_d3dCaps.TextureOpCaps & D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR )
    iLog->Log ("D3D Driver: Supports D3DTOP_MODULATEALPHA_ADDCOLOR TextureOp\n");
  if( m_d3dCaps.TextureOpCaps & D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA )
    iLog->Log ("D3D Driver: Supports D3DTOP_MODULATECOLOR_ADDALPHA TextureOp\n");
  if( m_d3dCaps.TextureOpCaps & D3DTEXOPCAPS_ADD )
    iLog->Log ("D3D Driver: Supports D3DTOP_ADD TextureOp\n");

  m_MaxAnisotropyLevel = 1;
  m_AnisotropyLevel = 1;
  // Check the device for supported filtering methods
  if( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_ANISOTROPY )
  {
    m_MaxAnisotropyLevel = m_AnisotropyLevel = m_d3dCaps.MaxAnisotropy;
    iLog->Log ("D3D Driver: Supports MaxAnisotropy level %d\n", m_AnisotropyLevel);
    m_Features |= RFT_ALLOWANISOTROPIC;
  }
  if( m_d3dCaps.StencilCaps & (D3DSTENCILCAPS_KEEP|D3DSTENCILCAPS_INCR|D3DSTENCILCAPS_DECR) )
  {
    iLog->Log ("D3D Driver: Supports Stencil shadows\n");
    if( m_d3dCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED)
      iLog->Log ("D3D Driver: Supports Two-Sided stencil\n");
  }
  else
    CRenderer::m_sbpp = 0;

  iLog->Log ("D3D Driver: Textures (%ix%i)-(%ix%i), Max aspect %i\n", mMinTextureWidth, mMinTextureHeight, m_d3dCaps.MaxTextureWidth, m_d3dCaps.MaxTextureHeight, m_d3dCaps.MaxTextureAspectRatio );

  // Depth buffering.
  m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
  /*if ( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_WBUFFER )
  {
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_USEW );
    m_bUseWBuffer = true;
    iLog->Log ("D3D Driver: Supports w-buffering\n");
  }
  else*/
    m_bUseWBuffer = false;

  m_Features &= ~RFT_BUMP;
  if( CV_d3d9_usebumpmap && (GetFeatures() & RFT_MULTITEXTURE))
  {
    if ((m_d3dCaps.TextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3) && (m_d3dCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP))
      m_Features |= RFT_BUMP;
  }

  // Init render states.
  {
    m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_COLORVERTEX, TRUE );
    D3DSetCull(eCULL_None);
    m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF, 127 );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
    if( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_DEPTHBIAS )
      m_pd3dDevice->SetRenderState( D3DRS_DEPTHBIAS, 0 );
    m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,   D3DFILL_SOLID );
    m_pd3dDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE);
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE);
    m_pd3dDevice->SetRenderState( D3DRS_FOGCOLOR, 0 );
    m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_NONE );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE, FALSE);
    m_pd3dDevice->SetRenderState( D3DRS_CLIPPING, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE,  FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
    //m_pd3dDevice->SetBackBufferScale(1.0f, 1.0f);
    m_pd3dDevice->SetPixelShader( NULL );
    m_pd3dDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_POINTSCALEENABLE,  FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND,      FALSE );
  	m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff );
    m_FS.m_bEnable = false;

    // Set default stencil states
    m_CurStencilState = 0;
    m_CurStencMask = 0xffffffff;
    m_CurStencRef = 0;
    m_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    if (m_d3dCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED)
    {
      m_pd3dDevice->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE);
      m_pd3dDevice->SetRenderState(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP);
      m_pd3dDevice->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
      m_pd3dDevice->SetRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_KEEP);
      m_pd3dDevice->SetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_ALWAYS);
    }
    m_pd3dDevice->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
    m_pd3dDevice->SetRenderState(D3DRS_STENCILREF, 0);

    m_pd3dDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
    m_pd3dDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
    m_pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
    m_pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
  }

  msCurState = GS_DEPTHWRITE | GS_NODEPTHTEST;
  for (i=0; i<MAX_TMU; i++)
  {
    m_eCurColorOp[i] = eCO_DISABLE;
    m_eCurColorArg[i] = 255;
    m_eCurAlphaArg[i] = 255;
  }

  // Init texture stage state.
  {
    //FLOAT LodBias=-0.5;
    //m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPMAPLODBIAS, *(DWORD*)&LodBias );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,   D3DTADDRESS_WRAP );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,   D3DTADDRESS_WRAP );
    EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
    m_RP.m_TexStages[0].MinFilter = D3DTEXF_LINEAR;
    m_RP.m_TexStages[0].MagFilter = D3DTEXF_LINEAR;
    m_RP.m_TexStages[0].nMipFilter = D3DTEXF_LINEAR;
    m_RP.m_TexStages[0].TCIndex = 0;
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );

    {
      if( GetFeatures() & RFT_MULTITEXTURE )
      {
        for( i=1; i<(int)m_d3dCaps.MaxSimultaneousTextures; i++ )
        {
          m_TexMan->m_CurStage = i;

          // Set stage i state.
          m_pd3dDevice->SetSamplerState( i, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP );
          m_pd3dDevice->SetSamplerState( i, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP );
          EF_SetColorOp(eCO_DISABLE, eCO_DISABLE, DEF_TEXARG1, DEF_TEXARG1);
          m_pd3dDevice->SetSamplerState( i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
          m_pd3dDevice->SetSamplerState( i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
          m_pd3dDevice->SetSamplerState( i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
          m_pd3dDevice->SetTextureStageState( i, D3DTSS_TEXCOORDINDEX, i );
          m_RP.m_TexStages[i].MinFilter = D3DTEXF_LINEAR;
          m_RP.m_TexStages[i].MagFilter = D3DTEXF_LINEAR;
          m_RP.m_TexStages[i].nMipFilter = D3DTEXF_LINEAR;
          m_RP.m_TexStages[0].TCIndex = i;
        }
      }
    }
  }

  // Check to see if device supports visibility query
  if (!CV_d3d9_occlusion_query || D3DERR_NOTAVAILABLE == m_pd3dDevice->CreateQuery (D3DQUERYTYPE_OCCLUSION, NULL))
    m_Features &= ~RFT_OCCLUSIONTEST;
  else
    m_Features |= RFT_OCCLUSIONTEST;

  m_TexMan->m_CurStage = 0;
  m_TexMan->SetFilter(CV_d3d9_texturefilter->GetString());

  // For safety, lots of drivers don't handle tiny texture sizes too tell.
  mMinTextureWidth       = 8;
  mMinTextureHeight      = 8;
  m_MaxTextureMemory = m_pd3dDevice->GetAvailableTextureMem();
  if (CRenderer::CV_r_texturesstreampoolsize <= 0)
    CRenderer::CV_r_texturesstreampoolsize = (int)(m_MaxTextureMemory/1024.0f/1024.0f*0.75f);

  m_MaxTextureSize = min(m_d3dCaps.MaxTextureHeight, m_d3dCaps.MaxTextureWidth); //min(wdt, hgt);

  m_Features &= ~RFT_DEPTHMAPS;
  if (m_d3dCaps.MaxSimultaneousTextures <= 2)
  {
    m_Features &= ~RFT_HW_MASK;
    m_Features |= RFT_HW_GF2;
  }
  else
  {
    if (m_d3dCaps.MaxSimultaneousTextures == 4 && D3DSHADER_VERSION_MAJOR(m_d3dCaps.PixelShaderVersion) == 1)
    {
      m_Features &= ~RFT_HW_MASK;
      m_Features |= RFT_HW_GF3;
      if (ai->VendorId==4318 && !CV_d3d9_nodepthmaps)
        m_Features |= RFT_DEPTHMAPS;
    }
    else
    if( ai->VendorId==4318 )
    {
      if (m_d3dCaps.MaxSimultaneousTextures >= 8 && (D3DSHADER_VERSION_MAJOR(m_d3dCaps.PixelShaderVersion) >= 3 || (D3DSHADER_VERSION_MAJOR(m_d3dCaps.PixelShaderVersion) >= 2 && D3DSHADER_VERSION_MINOR(m_d3dCaps.PixelShaderVersion) >= 1)))
      {
        m_Features &= ~RFT_HW_MASK;
        m_Features |= RFT_HW_NV4X;
      }
      else
      if (m_d3dCaps.MaxSimultaneousTextures >= 8 && D3DSHADER_VERSION_MAJOR(m_d3dCaps.PixelShaderVersion) >= 2)
      {
        m_Features &= ~RFT_HW_MASK;
        m_Features |= RFT_HW_GFFX;
      }
      else
      {
        m_Features &= ~RFT_HW_MASK;
        m_Features |= RFT_HW_GF3;
      }
      if (!CV_d3d9_nodepthmaps)
        m_Features |= RFT_DEPTHMAPS;
      if (m_bDeviceSupportsComprNormalmaps == 1)
      {
        iLog->Log ("Warning: Disabled 3DC for NVidia card\n");
        if (mFormatV8U8.BitsPerPixel)
          m_bDeviceSupportsComprNormalmaps = 2;
      }
    }
    else
    if (m_d3dCaps.MaxSimultaneousTextures >= 8 && D3DSHADER_VERSION_MAJOR(m_d3dCaps.PixelShaderVersion) >= 2)
    {
      m_Features &= ~RFT_HW_MASK;
      m_Features |= RFT_HW_RADEON;
    }
    else
    if (!(m_Features & RFT_HW_MASK))
      m_Features |= RFT_HW_GF3;
  }
  if (m_d3dCaps.MaxSimultaneousTextures >= 4 && D3DSHADER_VERSION_MAJOR(m_d3dCaps.PixelShaderVersion) >= 1)
    m_Features |= RFT_HW_TS | RFT_HW_VS;

  m_bDeviceSupportsInstancing = (D3DSHADER_VERSION_MAJOR(m_d3dCaps.VertexShaderVersion) >= 3);
  if (!m_bDeviceSupportsInstancing)
  {
    D3DFORMAT instanceSupport = (D3DFORMAT)MAKEFOURCC('I', 'N', 'S', 'T');
    if(SUCCEEDED(m_pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, instanceSupport)))
    {
      m_bDeviceSupportsInstancing = 2;
      // Notify the driver that instancing support is expected
      m_pd3dDevice->SetRenderState(D3DRS_POINTSIZE, instanceSupport);
    }
  }
  m_bDeviceSupportsMRT = (m_d3dCaps.NumSimultaneousRTs>=4) && SUCCEEDED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_G16R16F));

	//// REMOVED DRIVER HACK -- Carsten
	//// Workaround for NVidia drivers less than 66.00
 // // Don't use MRT on old drivers
 // if (LOWORD(ai->DriverVersion.u.LowPart) < 6600)
 // {
 //   m_bDeviceSupportsMRT = 0;
 // }

  m_bDeviceSupportsFP16Filter = true;
  if(FAILED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_FILTER, D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16F)))
    m_bDeviceSupportsFP16Filter = false;
  if(FAILED(m_pD3D->CheckDeviceFormat(pAI->AdapterOrdinal, m_d3dCaps.DeviceType, m_D3DSettings.AdapterFormat(), D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_FILTER, D3DRTYPE_TEXTURE, D3DFMT_G16R16F)))
    m_bDeviceSupportsFP16Filter = false;

  int nHDRSupported = 1;
  {
    // Check support for HDR rendering
    D3DDeviceCombo *pDev = m_D3DSettings.PDeviceCombo();
    m_HDR_FloatFormat_Scalar = D3DFMT_R16F;
    if (CV_r_Quality_BumpMapping < 3)
      nHDRSupported = 0;
    else
    if(FAILED(m_pD3D->CheckDeviceFormat(pDev->AdapterOrdinal, pDev->DevType, pDev->AdapterFormat, D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16F)))
      nHDRSupported = 0;
    else
    if(FAILED(m_pD3D->CheckDeviceFormat(pDev->AdapterOrdinal, pDev->DevType, pDev->AdapterFormat, D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_SURFACE, D3DFMT_A16B16G16R16F)))
      nHDRSupported = 0;
    else
    if(FAILED(m_pD3D->CheckDeviceFormat(pDev->AdapterOrdinal, pDev->DevType, pDev->AdapterFormat, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_R16F)))
    {
      m_HDR_FloatFormat_Scalar = D3DFMT_R32F;
      if(FAILED(m_pD3D->CheckDeviceFormat(pDev->AdapterOrdinal, pDev->DevType, pDev->AdapterFormat, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_R32F)))
        nHDRSupported = 0;
    }
    if (D3DSHADER_VERSION_MAJOR(m_d3dCaps.PixelShaderVersion) < 2)
      nHDRSupported = 0;
  }
  if (!nHDRSupported && m_d3dCaps.NumSimultaneousRTs>=2 && (m_d3dCaps.PrimitiveMiscCaps & D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING) &&
        D3DSHADER_VERSION_MAJOR(m_d3dCaps.PixelShaderVersion) >= 2 &&
        CV_r_Quality_BumpMapping >= 3)
      nHDRSupported = 2;
  if (nHDRSupported == 2 || CV_r_Quality_BumpMapping < 3)
    nHDRSupported = 0;
  if (!nHDRSupported)
  {
#ifdef USE_HDR
    _SetVar("r_HDRRendering", 0);
#endif
    m_Features &= ~RFT_HW_HDR;
  }
  else
    m_Features |= RFT_HW_HDR;
  m_nHDRType = nHDRSupported;

  float fColor[4];
  fColor[0] = fColor[1] = fColor[2] = fColor[3] = 0;
  EF_ClearBuffers(true, false, fColor);

  m_pd3dDevice->BeginScene();
  m_SceneRecurseCount++;

  return S_OK;
}

void CD3D9Renderer::RecognizePixelFormat(SPixFormat& Dest, D3DFORMAT FromD3D, INT InBitsPerPixel, const TCHAR* InDesc)
{
  Dest.Init();
  Dest.Format        = FromD3D;
  Dest.MaxWidth      = m_d3dCaps.MaxTextureWidth;
  Dest.MaxHeight     = m_d3dCaps.MaxTextureHeight;
  Dest.Desc          = InDesc;
  Dest.BitsPerPixel  = InBitsPerPixel;
  Dest.Next          = mFirstPixelFormat;
  mFirstPixelFormat   = &Dest;

  iLog->Log("  Using '%s' pixel texture format\n", InDesc);
}

