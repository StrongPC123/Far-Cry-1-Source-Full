/*=============================================================================
  DriverD3D8.cpp : Direct3D Render interface implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#include "stdafx.h"
#include "DriverD3D8.h"
#include "D3DCGVProgram.h"
#include "D3DCGPShader.h"

#ifndef _XBOX
#include <windows.h>
#endif

#include "IStatObj.h"


CD3D8Renderer *gcpRendD3D;

HRESULT h;

int CD3D8Renderer::CV_d3d8_texture_filter_anisotropic;
int CD3D8Renderer::CV_d3d8_nodeviceid;
int CD3D8Renderer::CV_d3d8_palettedtextures;
int CD3D8Renderer::CV_d3d8_compressedtextures;
int CD3D8Renderer::CV_d3d8_usebumpmap;
int CD3D8Renderer::CV_d3d8_bumptype;
int CD3D8Renderer::CV_d3d8_forcesoftwarevp;
int CD3D8Renderer::CV_d3d8_texturebits;
int CD3D8Renderer::CV_d3d8_texmipfilter; // 0-point; 1-box; 2-linear; 3-triangle; 
int CD3D8Renderer::CV_d3d8_mipprocedures;
int CD3D8Renderer::CV_d3d8_squaretextures;
ICVar *CD3D8Renderer::CV_d3d8_device;
int CD3D8Renderer::CV_d3d8_allowsoftware;
float CD3D8Renderer::CV_d3d8_gamma;
int CD3D8Renderer::CV_d3d8_rb_verts;
int CD3D8Renderer::CV_d3d8_rb_tris;
int CD3D8Renderer::CV_d3d8_decaloffset;
float CD3D8Renderer::CV_d3d8_normalmapscale;
ICVar *CD3D8Renderer::CV_d3d8_texturefilter;

// Direct 3D console variables
CD3D8Renderer::CD3D8Renderer()
{
  m_bInitialized = false;
  gcpRendD3D = this;
  gRenDev = this;

  m_nFrameID = 0;

  m_TexMan = new CD3D8TexMan;
  m_TexMan->m_bRGBA = false;

  m_LogFile = NULL;

  RegisterVariables();

  m_dwNumAdapters     = 0;
  m_dwAdapter         = 0L;
  m_pD3D              = NULL;
  m_pd3dDevice        = NULL;
  m_hWnd              = NULL;
  m_bActive           = FALSE;
  m_bReady            = FALSE;
  m_dwCreateFlags     = 0L;
	m_pQuadVB       = NULL;
  m_pLineVB = 0;

  m_strDeviceStats[0] = 0;

  m_MinDepthBits    = 16;
  m_MinStencilBits  = 0;
  m_eCull = (ECull)-1;
  m_EnableLights = 0;
  m_Features = RFT_DIRECTACCESSTOVIDEOMEMORY | RFT_SUPPORTZBIAS;

  iConsole->Register("d3d8_Texture_Filter_Anisotropic", &CV_d3d8_texture_filter_anisotropic, 0);
  iConsole->Register("d3d8_NodeviceId", &CV_d3d8_nodeviceid, 0);
  iConsole->Register("d3d8_PalettedTextures", &CV_d3d8_palettedtextures, 1);
  iConsole->Register("d3d8_CompressedTextures", &CV_d3d8_compressedtextures, 1);
  iConsole->Register("d3d8_UseBumpmap", &CV_d3d8_usebumpmap, 1);
  iConsole->Register("d3d8_BumpType", &CV_d3d8_bumptype, 1);
  iConsole->Register("d3d8_ForceSoftwareVP", &CV_d3d8_forcesoftwarevp, 0);
  iConsole->Register("d3d8_TextureBits", &CV_d3d8_texturebits, 0);
  iConsole->Register("d3d8_MipProcedures", &CV_d3d8_mipprocedures, 0);
  iConsole->Register("d3d8_TexMipFilter", &CV_d3d8_texmipfilter, 2); // 0-point; 1-box; 2-linear; 3-triangle; 
  CV_d3d8_texturefilter = iConsole->CreateVariable("d3d8_TextureFilter", "TRILINEAR", NULL);
  iConsole->Register("d3d8_SquareTextures", &CV_d3d8_squaretextures, 0);
  CV_d3d8_device = iConsole->CreateVariable("d3d8_Device", "Auto", NULL);
  iConsole->Register("d3d8_AllowSoftware", &CV_d3d8_allowsoftware, 1L);
  iConsole->Register("d3d8_Gamma", &CV_d3d8_gamma, 1.2f);
  iConsole->Register("d3d8_rb_Verts", &CV_d3d8_rb_verts, 500);
  iConsole->Register("d3d8_rb_Tris", &CV_d3d8_rb_tris, 1000);
  iConsole->Register("d3d8_DecalOffset", &CV_d3d8_decaloffset, 15);
  iConsole->Register("d3d8_NormalMapScale", &CV_d3d8_normalmapscale, 0.15f);
}

CD3D8Renderer::~CD3D8Renderer()
{
  FreeResources(FRR_ALL);
  ShutDown();
}

void  CD3D8Renderer::ShareResources( IRenderer *renderer )
{
}

void	CD3D8Renderer::MakeCurrent()
{
}

bool CD3D8Renderer::SetCurrentContext(WIN_HWND hWnd)
{
  int i;

  for (i=0; i<m_RContexts.Num(); i++)
  {
    if (m_RContexts[i]->m_hWnd == hWnd)
      break;
  }
  if (i == m_RContexts.Num())
    return false;

  if (m_CurrContext == m_RContexts[i])
    return true;

  m_CurrContext = m_RContexts[i];

  CVProgram::m_LastVP = NULL;
  CPShader::m_CurRC = NULL;
  CPShader::m_CurPS = NULL;

  return true;
}

bool CD3D8Renderer::CreateContext(WIN_HWND hWnd, bool bAllowFSAA)
{
  int i;

  for (i=0; i<m_RContexts.Num(); i++)
  {
    if (m_RContexts[i]->m_hWnd == hWnd)
      break;
  }
  if (i != m_RContexts.Num())
    return true;
  SD3DContext *pContext = new SD3DContext;
  pContext->m_hWnd = hWnd;
  m_CurrContext = pContext;
  m_RContexts.AddElem(pContext);

  return true;
}

bool CD3D8Renderer::DeleteContext(WIN_HWND hWnd)
{
  int i;

  for (i=0; i<m_RContexts.Num(); i++)
  {
    if (m_RContexts[i]->m_hWnd == hWnd)
      break;
  }
  if (i == m_RContexts.Num())
    return false;
  if (m_CurrContext == m_RContexts[i])
  {
    for (int j=0; j<m_RContexts.Num(); j++)
    {
      if (m_RContexts[j]->m_hWnd != hWnd)
      {
        m_CurrContext = m_RContexts[j];
        break;
      }
    }
    if (j == m_RContexts.Num())
      m_CurrContext = NULL;
  }
  delete m_RContexts[i];
  m_RContexts.Remove(i, 1);

  return true;
}

//-----------------------------------------------------------------------------
// Name: CD3D8Renderer::DeleteDeviceObjects()
// Desc: Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.
//-----------------------------------------------------------------------------
HRESULT CD3D8Renderer::DeleteDeviceObjects()
{
  return S_OK;
}

//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT CD3D8Renderer::FinalCleanup()
{
  return S_OK;
}

void CD3D8Renderer::DestroyWindow(void)  
{
  guard(CD3D8Renderer::DestroyWindow);

#ifndef _XBOX  
  if (m_hWnd)
  {
    ::DestroyWindow(m_hWnd);
    m_hWnd = NULL;
  }
#endif

  unguard
}

void CD3D8Renderer::RestoreGamma(void)
{
  if (!m_pd3dDevice)
    return;

  if (m_bGammaCalibrate)
    m_pd3dDevice->SetGammaRamp(D3DSGR_CALIBRATE, &m_SystemGammaRamp);
  else
    m_pd3dDevice->SetGammaRamp(D3DSGR_NO_CALIBRATION, &m_SystemGammaRamp);
}

void CD3D8Renderer::SetGamma(float fGamma, float fBrightness, float fContrast)
{
  fGamma = Clamp(fGamma, 0.5f, 3.0f);
  if (m_fLastGamma == fGamma && m_fLastBrightness == fBrightness && m_fLastContrast == fContrast)
    return;

  float Gamma = CV_d3d8_gamma;
  D3DGAMMARAMP Ramp;
  int m;
  for( int x=0; x<256; x++ )
  {
    if ( Gamma == 1.0f )
      m = x;
    else
    {
      float f = (float)x;
      m = (int)(pow(f/255.0, 1.0/Gamma) * 255.0 + 0.5);
    }
    if ( m < 0 )
      m = 0;
    if ( m > 255 )
      m = 255;
    m <<= 8;

    Ramp.red[x] = Ramp.green[x] = Ramp.blue[x] = m;
  }
  if( m_Features & RFT_HWGAMMA )
  {
    if (m_bGammaCalibrate)
      m_pd3dDevice->SetGammaRamp(D3DSGR_CALIBRATE, &Ramp);
    else
      m_pd3dDevice->SetGammaRamp(D3DSGR_NO_CALIBRATION, &Ramp);
  }

  m_fLastGamma = fGamma;
  m_fLastBrightness = fBrightness;
  m_fLastContrast = fContrast;

  Gamma = CV_d3d8_gamma;
  for (int i=0; i<256; i++)
  {
    m_GammmaTable[i] = (byte)floor( pow(i / 255.f, 1.0f / Gamma) * 255.f + 0.5f);
  }
}

//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CD3D8Renderer::InvalidateDeviceObjects()
{
  return S_OK;
}

void CD3D8Renderer::UnSetRes()
{
  m_Features = RFT_DIRECTACCESSTOVIDEOMEMORY | RFT_SUPPORTZBIAS;

  m_bActive = FALSE;
  m_bReady  = FALSE;

  SAFE_RELEASE(m_pQuadVB);
  SAFE_RELEASE(m_pLineVB);

  if( m_pd3dDevice )
  {
    InvalidateDeviceObjects();
    DeleteDeviceObjects();

    m_pd3dDevice->Release();
    m_pD3D->Release();

    m_pd3dDevice = NULL;
    m_pD3D       = NULL;
  }

  FinalCleanup();
}

void CD3D8Renderer::ShutDown(bool bReInit)
{
  guardnw(CD3D8Renderer::ShutDown);

  UnSetRes();
  FreeResources(FRR_ALL);
  EF_PipelineShutdown();
  CName::mfExitSubsystem();

#ifndef _XBOX
  if (m_CGContext)
  {
    cgDestroyContext(m_CGContext);
    m_CGContext = NULL;
  }
#endif

  unguardnw;
}

void  CD3D8Renderer::RefreshResources(int nFlags)
{
}

void CD3D8Renderer::RegisterVariables()
{
}

void CD3D8Renderer::UnRegisterVariables()
{
}

int CD3D8Renderer::EnumDisplayFormats(TArray<SDispFormat>& Formats, bool bReset)
{
  Formats.Free();
  return 0;
}

bool CD3D8Renderer::SetWindow(int width, int height, bool fullscreen, WIN_HWND hWnd)
{
  guard(CD3D8Renderer::mfSetWindow);

#ifndef _XBOX  
  
  RECT rc;
  DWORD style, exstyle;
  int x, y, wdt, hgt;
  
  if (width < 640)
    width = 640;
  if (height < 480)
    height = 480;

  m_dwWindowStyle = WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

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
  rc.left = 0;
  rc.right = width;
  rc.top = 0;
  rc.bottom = height;

  AdjustWindowRect(&rc, style, false);
  wdt = rc.right - rc.left;
  hgt = rc.bottom - rc.top;
  x = y = 0;

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
  m_width = width;
  m_height = height;    

  if (!m_hWnd)
    iConsole->Exit("Couldn't create window\n");

  if (fullscreen)
  {
    // Hide the cursor
    SetCursor(NULL);
    ShowCursor(FALSE);
  }
  else
  if (!m_FullScreen)
    SetWindowPos(m_hWnd, HWND_NOTOPMOST, (GetSystemMetrics(SM_CXFULLSCREEN)-width)/2, (GetSystemMetrics(SM_CYFULLSCREEN)-height)/2, GetSystemMetrics(SM_CXDLGFRAME)*2 + width, GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXDLGFRAME)*2 + height, SWP_SHOWWINDOW);

  if (!hWnd)
  {
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    SetForegroundWindow(m_hWnd);
    SetFocus(m_hWnd);
  }

#endif
  
  unguard

  return true;
}

#ifndef PS2
WIN_HWND CD3D8Renderer::Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen,WIN_HINSTANCE hinst, WIN_HWND Glhwnd, WIN_HDC Glhdc, WIN_HGLRC hGLrc, bool bReInit)
{
  guard(CD3D8Renderer::Init);

  if (m_IsDedicated)
  {
    m_MaxTextureSize = 256;
    return 0;
  }

  bool b = false;

#ifdef _XBOX
  fullscreen = true;
#endif
  
  iLog->Log ( "Direct3D8 driver is opening...\n");
  iLog->Log ( "\nCrytek Direct3D8 driver version %4.2f (%s <%s>).\n", VERSION_D3D, __DATE__, __TIME__);

  //strcpy(m_WinTitle, "- Far Cry -");
	sprintf(m_WinTitle,"- Far Cry - %s (%s)",__DATE__, __TIME__);
  m_hInst = (HINSTANCE)hinst;

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
    ShutDown();
    if (b)
      return 0;
    m_bFullScreen ? m_bFullScreen = 0 : m_bFullScreen = 1;
    b = 1;
  }

  D3DADAPTER_IDENTIFIER8 *ai = &m_Adapters[m_dwAdapter].d3dAdapterIdentifier;
  SD3DDeviceInfo *di = &m_Adapters[m_dwAdapter].devices[m_Adapters[m_dwAdapter].dwCurrentDevice];

  iLog->Log("\n ****** D3D8 Render Stats ******\n");
  iLog->Log(" Driver description: %s\n", ai->Description);
  iLog->Log(" Full stats: %s\n", m_strDeviceStats);
  iLog->Log(" Hardware acceleration: %s\n", (di->DeviceType == D3DDEVTYPE_HAL) ? "Yes" : "No");
  iLog->Log(" Full screen only: %s\n", (di->d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED) ? "No" : "Yes");
  iLog->Log(" Detail textures: %s\n", (GetFeatures() & RFT_DETAILTEXTURE) ? "Yes" : "No");
  iLog->Log(" Z Buffer Locking: %s\n", (GetFeatures() & RFT_ZLOCKABLE) ? "Yes" : "No");
  iLog->Log(" Use multitexture mode: %s (%d texture(s))\n", (GetFeatures() & RFT_MULTITEXTURE) ? "Yes" : "No", di->d3dCaps.MaxSimultaneousTextures);
  iLog->Log(" Use bumpmapping : %s\n", (GetFeatures() & RFT_BUMP) ? ((GetFeatures() & RFT_BUMP_DOT3) ? "Yes (DOT3)" : "Yes (EMBM)") : "No");
  iLog->Log(" Use paletted textures : %s\n", (GetFeatures() & RFT_PALTEXTURE) ? "Yes" : "No");
  iLog->Log(" Current Resolution: %dx%dx%d %s\n", CRenderer::m_width, CRenderer::m_height, CRenderer::m_cbpp, m_bFullScreen ? "Full Screen" : "Windowed");
  iLog->Log(" Maximum Resolution: %dx%d\n", m_Adapters[m_dwAdapter].mMaxWidth, m_Adapters[m_dwAdapter].mMaxHeight);
  iLog->Log(" Maximum Texture size: %dx%d (Max Aspect: %d)\n", di->d3dCaps.MaxTextureWidth, di->d3dCaps.MaxTextureHeight, di->d3dCaps.MaxTextureAspectRatio);
  iLog->Log(" Texture filtering type: %s\n", CV_d3d8_texturefilter->GetString());
  iLog->Log(" Use %d bits textures\n", m_TextureBits);
  iLog->Log(" Gamma control: %s\n", (GetFeatures() & RFT_HWGAMMA) ? "Hardware" : "Software");
#ifdef _XBOX
  iLog->Log(" Use Hardware Shaders for XBox\n");
#else
  if (GetFeatures() & RFT_HW_GF2)
    iLog->Log(" Use Hardware Shaders for NV1x GPU\n");
  else
  if (GetFeatures() & RFT_HW_GF3)
    iLog->Log(" Use Hardware Shaders for NV2x GPU\n");
  else
  if (GetFeatures() & RFT_HW_RADEON8500)
    iLog->Log(" Use Hardware Shaders for ATI R300 GPU\n");
  else
    iLog->Log(" Hardware Shaders not supported\n");
#endif
  iLog->Log(" *****************************************\n\n");

  iLog->Log("Init Shaders\n");

  m_numtmus = di->d3dCaps.MaxSimultaneousTextures;
  
  gcEf.mfInit();
  EF_Init();

  m_bInitialized = true;

//  Cry_memcheck();

  // Success, return the window handle
#ifndef _XBOX
  return (m_hWnd);
#else
  return (WIN_HWND)1;
#endif

  unguard;
}
#else
bool CD3D8Renderer::Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen)
{
  return false;
}
#endif

static char sErr[256];

char *CD3D8Renderer::D3DError( HRESULT h )
{
  D3DXGetErrorString(h, sErr, 256);

  return sErr;
}

bool CD3D8Renderer::Error(char *Msg, HRESULT h)
{
  guard(CD3D8Renderer::mfError);

  char *str = D3DError(h);
  iLog->Log("Error: %s (%s)", Msg, str);

  //UnSetRes();

  //if (Msg)
  //  iConsole->Exit("%s (%s)\n", Msg, str);
  //else
  //  iConsole->Exit("(%s)\n", str);

  return false;

  unguard;
}

//-----------------------------------------------------------------------------
// Name: CD3D8Renderer::mfConfirmDevice()
// Desc: Called during device intialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CD3D8Renderer::ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior, D3DFORMAT Format )
{
  return S_OK;
}


// Name: SortModesCallback()
// Desc: Callback function for sorting display modes (used by BuildDevicesList).
//-----------------------------------------------------------------------------
int SortModesCallback( const VOID* arg1, const VOID* arg2 )
{
  D3DDISPLAYMODE* p1 = (D3DDISPLAYMODE*)arg1;
  D3DDISPLAYMODE* p2 = (D3DDISPLAYMODE*)arg2;

  if( p1->Format > p2->Format )   return -1;
  if( p1->Format < p2->Format )   return +1;
  if( p1->Width  < p2->Width )    return -1;
  if( p1->Width  > p2->Width )    return +1;
  if( p1->Height < p2->Height )   return -1;
  if( p1->Height > p2->Height )   return +1;

  return 0;
}


//-----------------------------------------------------------------------------
// Name: CD3D8Renderer::mfFindDepthStencilFormat()
// Desc: Finds a depth/stencil format for the given device that is compatible
//       with the render target format and meets the needs of the app.
//-----------------------------------------------------------------------------
bool CD3D8Renderer::FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType, D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat )
{
  if( m_MinDepthBits <= 16 && m_MinStencilBits == 0 )
  {
    if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType, TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16_LOCKABLE ) ) )
    {
      if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType, TargetFormat, TargetFormat, D3DFMT_D16_LOCKABLE ) ) )
      {
        *pDepthStencilFormat = D3DFMT_D16_LOCKABLE;
        return TRUE;
      }
    }
    if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType, TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16 ) ) )
    {
      if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType, TargetFormat, TargetFormat, D3DFMT_D16 ) ) )
      {
        *pDepthStencilFormat = D3DFMT_D16;
        return TRUE;
      }
    }
  }

  if( m_MinDepthBits <= 15 && m_MinStencilBits <= 1 )
  {
#ifndef _XBOX
    if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType, TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D15S1 ) ) )
    {
      if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType, TargetFormat, TargetFormat, D3DFMT_D15S1 ) ) )
      {
        *pDepthStencilFormat = D3DFMT_D15S1;
        return TRUE;
      }
    }
#endif
  }

  if( m_MinDepthBits <= 24 && m_MinStencilBits == 0 )
  {
#ifndef _XBOX
    if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType, TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8 ) ) )
    {
      if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType, TargetFormat, TargetFormat, D3DFMT_D24X8 ) ) )
      {
        *pDepthStencilFormat = D3DFMT_D24X8;
        return TRUE;
      }
    }
#endif
  }

  if( m_MinDepthBits <= 24 && m_MinStencilBits <= 8 )
  {
    if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType, TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8 ) ) )
    {
      if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType, TargetFormat, TargetFormat, D3DFMT_D24S8 ) ) )
      {
        *pDepthStencilFormat = D3DFMT_D24S8;
        return TRUE;
      }
    }
  }

  if( m_MinDepthBits <= 24 && m_MinStencilBits <= 4 )
  {
#ifndef _XBOX
    if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType, TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X4S4 ) ) )
    {
      if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType, TargetFormat, TargetFormat, D3DFMT_D24X4S4 ) ) )
      {
        *pDepthStencilFormat = D3DFMT_D24X4S4;
        return TRUE;
      }
    }
#endif
  }

  if( m_MinDepthBits <= 32 && m_MinStencilBits == 0 )
  {
#ifndef _XBOX
    if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType, TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32 ) ) )
    {
      if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType, TargetFormat, TargetFormat, D3DFMT_D32 ) ) )
      {
        *pDepthStencilFormat = D3DFMT_D32;
        return TRUE;
      }
    }
#endif
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
// Name: CD3D8Renderer::mfBuildDevicesList()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3D8Renderer::BuildDevicesList()
{
  const DWORD dwNumDeviceTypes = 2;
  const TCHAR* strDeviceDescs[] = {"HAL", "REF"};
  const D3DDEVTYPE DeviceTypes[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF };

  m_bHALExists = FALSE;
  m_bHALIsWindowedCompatible = FALSE;
  m_bHALIsDesktopCompatible = FALSE;
  m_bHALIsSampleCompatible = FALSE;
  m_dwNumAdapters = 0;


  // Loop through all the adapters on the system (usually, there's just one
  // unless more than one graphics card is present).
  for( UINT iAdapter = 0; iAdapter < m_pD3D->GetAdapterCount(); iAdapter++ )
  {
    // Fill in adapter info
    SD3DAdapterInfo* pAdapter  = &m_Adapters[m_dwNumAdapters];
    m_pD3D->GetAdapterIdentifier( iAdapter, 0, &pAdapter->d3dAdapterIdentifier );
    m_pD3D->GetAdapterDisplayMode( iAdapter, &pAdapter->d3ddmDesktop );
    pAdapter->dwNumDevices    = 0;
    pAdapter->dwCurrentDevice = 0;
    pAdapter->mMaxWidth = 0;
    pAdapter->mMaxHeight = 0;

    // Enumerate all display modes on this adapter
    D3DDISPLAYMODE modes[100];
    D3DFORMAT      formats[20];
    DWORD dwNumFormats      = 0;
    DWORD dwNumModes        = 0;
    DWORD dwNumAdapterModes = m_pD3D->GetAdapterModeCount( iAdapter );

    // Add the adapter's current desktop format to the list of formats
    formats[dwNumFormats++] = pAdapter->d3ddmDesktop.Format;

    for( UINT iMode = 0; iMode < dwNumAdapterModes; iMode++ )
    {
      // Get the display mode attributes
      D3DDISPLAYMODE DisplayMode;
      m_pD3D->EnumAdapterModes( iAdapter, iMode, &DisplayMode );

      // Check if the mode already exists (to filter out refresh rates)
      for( DWORD m=0L; m<dwNumModes; m++ )
      {
        if( ( modes[m].Width  == DisplayMode.Width  ) &&
            ( modes[m].Height == DisplayMode.Height ) &&
            ( modes[m].Format == DisplayMode.Format ) )
            break;
      }

      // If we found a new mode, add it to the list of modes
      if( m == dwNumModes )
      {
        modes[dwNumModes].Width       = DisplayMode.Width;
        modes[dwNumModes].Height      = DisplayMode.Height;
        modes[dwNumModes].Format      = DisplayMode.Format;
        modes[dwNumModes].RefreshRate = 0;
        if ((int)modes[dwNumModes].Width > pAdapter->mMaxWidth)
          pAdapter->mMaxWidth = modes[dwNumModes].Width;
        if ((int)modes[dwNumModes].Height > pAdapter->mMaxHeight)
          pAdapter->mMaxHeight = modes[dwNumModes].Height;

        dwNumModes++;

        // Check if the mode's format already exists
        for( DWORD f=0; f<dwNumFormats; f++ )
        {
          if( DisplayMode.Format == formats[f] )
            break;
        }

        // If the format is new, add it to the list
        if( f== dwNumFormats )
          formats[dwNumFormats++] = DisplayMode.Format;
      }
    }

    // Sort the list of display modes (by format, then width, then height)
    qsort( modes, dwNumModes, sizeof(D3DDISPLAYMODE), SortModesCallback );

    // Add devices to adapter
    for( UINT iDevice = 0; iDevice < dwNumDeviceTypes; iDevice++ )
    {
      // Fill in device info
      SD3DDeviceInfo* pDevice;
      pDevice                 = &pAdapter->devices[pAdapter->dwNumDevices];
      pDevice->DeviceType     = DeviceTypes[iDevice];
      m_pD3D->GetDeviceCaps( iAdapter, DeviceTypes[iDevice], &pDevice->d3dCaps );
      pDevice->strDesc        = strDeviceDescs[iDevice];
      pDevice->dwNumModes     = 0;
      pDevice->dwCurrentMode  = 0;
      pDevice->bCanDoWindowed = FALSE;
      pDevice->bWindowed      = !m_bFullScreen;
      pDevice->MultiSampleType = D3DMULTISAMPLE_NONE;

      // Examine each format supported by the adapter to see if it will
      // work with this device and meets the needs of the application.
      BOOL  bFormatConfirmed[20];
      DWORD dwBehavior[20];
      D3DFORMAT fmtDepthStencil[20];

      for( DWORD f=0; f<dwNumFormats; f++ )
      {
        bFormatConfirmed[f] = FALSE;
        fmtDepthStencil[f] = D3DFMT_UNKNOWN;

        // Skip formats that cannot be used as render targets on this device
        if( FAILED( m_pD3D->CheckDeviceType( iAdapter, pDevice->DeviceType, formats[f], formats[f], FALSE ) ) )
          continue;

        if( pDevice->DeviceType == D3DDEVTYPE_HAL )
        {
          // This system has a HAL device
          m_bHALExists = TRUE;

          if( pDevice->d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED )
          {
            // HAL can run in a window for some mode
            m_bHALIsWindowedCompatible = TRUE;

            if( f == 0 )
            {
              // HAL can run in a window for the current desktop mode
              m_bHALIsDesktopCompatible = TRUE;
            }
          }
        }

        // Confirm the device/format for HW vertex processing
        if( pDevice->d3dCaps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT )
        {
          if( pDevice->d3dCaps.DevCaps&D3DDEVCAPS_PUREDEVICE )
          {
            dwBehavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;

            if( SUCCEEDED( ConfirmDevice( &pDevice->d3dCaps, dwBehavior[f], formats[f] ) ) )
              bFormatConfirmed[f] = TRUE;
          }

          if ( FALSE == bFormatConfirmed[f] )
          {
            dwBehavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING;

            if( SUCCEEDED( ConfirmDevice( &pDevice->d3dCaps, dwBehavior[f], formats[f] ) ) )
              bFormatConfirmed[f] = TRUE;
          }

          if ( FALSE == bFormatConfirmed[f] )
          {
            dwBehavior[f] = D3DCREATE_MIXED_VERTEXPROCESSING;

            if( SUCCEEDED( ConfirmDevice( &pDevice->d3dCaps, dwBehavior[f], formats[f] ) ) )
              bFormatConfirmed[f] = TRUE;
          }
        }

        // Confirm the device/format for SW vertex processing
        if( FALSE == bFormatConfirmed[f] )
        {
          dwBehavior[f] = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

          if( SUCCEEDED( ConfirmDevice( &pDevice->d3dCaps, dwBehavior[f], formats[f] ) ) )
            bFormatConfirmed[f] = TRUE;
        }

        // Find a suitable depth/stencil buffer format for this device/format
        if( bFormatConfirmed[f] )
        {
          m_MinDepthBits = Clamp(CRenderer::m_zbpp, 15, 32);
          m_MinStencilBits = Clamp(CRenderer::m_sbpp, 0, 8);

          if( !FindDepthStencilFormat( iAdapter, pDevice->DeviceType, formats[f], &fmtDepthStencil[f] ) )
          {
            m_MinStencilBits = 0;
            if( FindDepthStencilFormat( iAdapter, pDevice->DeviceType, formats[f], &fmtDepthStencil[f] ) )
              goto ok;
            for (m_MinDepthBits=32; m_MinDepthBits>=15; m_MinDepthBits-=8)
            {
              for (m_MinStencilBits=8; m_MinStencilBits>=0; m_MinStencilBits-=4)
              {
                if( FindDepthStencilFormat( iAdapter, pDevice->DeviceType, formats[f], &fmtDepthStencil[f] ) )
                  goto ok;
              }
            }
            bFormatConfirmed[f] = FALSE;
          }
        }
ok:
        int nn = 0;
      }

      // Add all enumerated display modes with confirmed formats to the
      // device's list of valid modes
      for( DWORD m=0L; m<dwNumModes; m++ )
      {
        for( DWORD f=0; f<dwNumFormats; f++ )
        {
          if( modes[m].Format == formats[f] )
          {
            if( bFormatConfirmed[f] == TRUE )
            {
              // Add this mode to the device's list of valid modes
              pDevice->modes[pDevice->dwNumModes].Width      = modes[m].Width;
              pDevice->modes[pDevice->dwNumModes].Height     = modes[m].Height;
              pDevice->modes[pDevice->dwNumModes].Format     = modes[m].Format;
              pDevice->modes[pDevice->dwNumModes].dwBehavior = dwBehavior[f];
              pDevice->modes[pDevice->dwNumModes].DepthStencilFormat = fmtDepthStencil[f];
              pDevice->dwNumModes++;

              if( pDevice->DeviceType == D3DDEVTYPE_HAL )
                m_bHALIsSampleCompatible = TRUE;
            }
          }
        }
      }

      // Check if the device is compatible with the desktop display mode
      // (which was added initially as formats[0])
      if( bFormatConfirmed[0] && (pDevice->d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED) )
      {
        pDevice->bCanDoWindowed = TRUE;
      }
      else
      if( bFormatConfirmed[0] && !(pDevice->d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED) )
        pDevice->bWindowed = FALSE;

      // If valid modes were found, keep this device
      if( pDevice->dwNumModes > 0 )
        pAdapter->dwNumDevices++;
    }

    // If valid devices were found, keep this adapter
    if( pAdapter->dwNumDevices > 0 )
      m_dwNumAdapters++;
  }

  // Return an error if no compatible devices were found
  if( 0L == m_dwNumAdapters )
    return D3DAPPERR_NOCOMPATIBLEDEVICES;

  return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CD3D8Renderer::mfAdjustWindowForChange()
// Desc: Prepare the window for a possible change between windowed mode and
//       fullscreen mode.  This function is virtual and thus can be overridden
//       to provide different behavior, such as switching to an entirely
//       different window for fullscreen mode (as in the MFC sample apps).
//-----------------------------------------------------------------------------
HRESULT CD3D8Renderer::AdjustWindowForChange()
{

#ifndef _XBOX

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
#endif
  
  return S_OK;
}

HRESULT CD3D8Renderer::InitDeviceObjects()
{
  // Gamma correction support
  if (m_d3dCaps.Caps2 & D3DCAPS2_FULLSCREENGAMMA)
    m_Features |= RFT_HWGAMMA;
  if (m_d3dCaps.Caps2 & D3DCAPS2_CANCALIBRATEGAMMA)
    m_bGammaCalibrate = true;
  else
    m_bGammaCalibrate = false;

  // Device Id's
  D3DADAPTER_IDENTIFIER8 *ai = &m_Adapters[m_dwAdapter].d3dAdapterIdentifier;
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
  guard(DeviceIdentification)
  {
    bool ConstrainAspect = 1;
    if( CV_d3d8_nodeviceid )
    {
      iLog->Log ("D3D Detected: -nodeviceid specified, 3D device identification skipped\n");
    }
    else
    if( ai->VendorId==4098 )
    {
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
    		m_Features |= RFT_HW_RADEON8500;
      }
      else
      if( ai->DeviceId==20036 )
      {
        iLog->Log ("D3D Detected: ATI Radeon 9700\n");
        m_Features |= RFT_HW_RADEON8500;
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
    if( ai->VendorId==4318 )
    {
      iLog->Log ("D3D Detected: NVidia video card\n");
      if( ai->DeviceId==0x20 )
      {
        iLog->Log ("D3D Detected: Riva TNT\n");
        ConstrainAspect = 0;
      }
      if( ai->DeviceId==0x40 )
      {
        iLog->Log ("D3D Detected: Riva TNT2\n");
        ConstrainAspect = 0;
      }
      if( ai->DeviceId==0x29)
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
      if( ai->DeviceId==0xa0)
      {
        iLog->Log ("D3D Detected: Aladdin TNT2\n");
        ConstrainAspect = 0;
      }
      if (ai->DeviceId == 0x100)
      {
        iLog->Log ("D3D Detected: GeForce SDR\n");
      }
      if (ai->DeviceId == 0x101)
      {
        iLog->Log ("D3D Detected: GeForce DDR\n");
      }
      if (ai->DeviceId == 0x103)
      {
        iLog->Log ("D3D Detected: Quadro\n");
      }
      if (ai->DeviceId == 0x110)
      {
        iLog->Log ("D3D Detected: GeForce2 MX/MX 400\n");
      }
      if (ai->DeviceId == 0x111)
      {
        iLog->Log ("D3D Detected: GeForce2 MX 100/MX 200\n");
      }
      if (ai->DeviceId == 0x112)
      {
        iLog->Log ("D3D Detected: GeForce2 Go\n");
      }
      if (ai->DeviceId == 0x113)
      {
        iLog->Log ("D3D Detected: Quadro2 MXR/EX/Go\n");
      }
      if (ai->DeviceId == 0x150)
      {
        iLog->Log ("D3D Detected: GeForce2 GTS\n");
      }
      if (ai->DeviceId == 0x151)
      {
        iLog->Log ("D3D Detected: GeForce2 Ti\n");
      }
      if (ai->DeviceId == 0x152)
      {
        iLog->Log ("D3D Detected: GeForce2 Ultra\n");
      }
      if (ai->DeviceId == 0x153)
      {
        iLog->Log ("D3D Detected: Quadro2 Pro\n");
      }
      if (ai->DeviceId == 0x170)
      {
        iLog->Log ("D3D Detected: GeForce4 MX 460\n");
      }
      if (ai->DeviceId == 0x171)
      {
        iLog->Log ("D3D Detected: GeForce4 MX 440\n");
      }
      if (ai->DeviceId == 0x172)
      {
        iLog->Log ("D3D Detected: GeForce4 MX 420\n");
      }
      if (ai->DeviceId == 0x174)
      {
        iLog->Log ("D3D Detected: GeForce4 440 Go\n");
      }
      if (ai->DeviceId == 0x175)
      {
        iLog->Log ("D3D Detected: GeForce4 420 Go\n");
      }
      if (ai->DeviceId == 0x176)
      {
        iLog->Log ("D3D Detected: GeForce4 420 Go 32M\n");
      }
      if (ai->DeviceId == 0x177)
      {
        iLog->Log ("D3D Detected: GeForce4 440 Go 64M\n");
      }
      if (ai->DeviceId == 0x178)
      {
        iLog->Log ("D3D Detected: Quadro4 500 XGL\n");
      }
      if (ai->DeviceId == 0x179)
      {
        iLog->Log ("D3D Detected: Quadro4 550 XGL\n");
      }
      if (ai->DeviceId == 0x17a)
      {
        iLog->Log ("D3D Detected: Quadro4 200 NVS\n");
      }
      if (ai->DeviceId == 0x1a0)
      {
        iLog->Log ("D3D Detected: GeForce2 Integrated GPU\n");
      }
      if (ai->DeviceId == 0x200)
      {
        iLog->Log ("D3D Detected: GeForce3\n");
      }
      if (ai->DeviceId == 0x201)
      {
        iLog->Log ("D3D Detected: GeForce3 Ti 200\n");
      }
      if (ai->DeviceId == 0x202)
      {
        iLog->Log ("D3D Detected: GeForce3 Ti 500\n");
      }
      if (ai->DeviceId == 0x203)
      {
        iLog->Log ("D3D Detected: Quadro DCC\n");
      }
      if (ai->DeviceId == 0x250)
      {
        iLog->Log ("D3D Detected: GeForce4 Ti 4600\n");
      }
      if (ai->DeviceId == 0x251)
      {
        iLog->Log ("D3D Detected: GeForce4 Ti 4400\n");
      }
      if (ai->DeviceId == 0x253)
      {
        iLog->Log ("D3D Detected: GeForce4 Ti 4200\n");
      }
      if (ai->DeviceId == 0x258)
      {
        iLog->Log ("D3D Detected: Quadro4 900 XGL\n");
      }
      if (ai->DeviceId == 0x259)
      {
        iLog->Log ("D3D Detected: Quadro4 750 XGL\n");
      }
      if (ai->DeviceId == 0x25b)
      {
        iLog->Log ("D3D Detected: Quadro4 700 XGL\n");
      }
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
    {
      iLog->Log ("D3D Detected: Generic 3D accelerator\n");
    }
  }
  unguardnw


  // Set viewport.
  guardnw(SetViewport);
  SetViewport(0,0,CRenderer::m_width, CRenderer::m_height);
  unguardnw;

  // Check multitexture caps.
  iLog->Log ( "D3D Driver: MaxTextureBlendStages   = %i\n", m_d3dCaps.MaxTextureBlendStages);
  iLog->Log ( "D3D Driver: MaxSimultaneousTextures = %i\n", m_d3dCaps.MaxSimultaneousTextures);
  if( m_d3dCaps.MaxSimultaneousTextures>=2 )
    m_Features |= RFT_MULTITEXTURE;

  // Handle the texture formats we need.
  guardnw(InitTextureFormats);
  {
    // Zero them all.
    mFormat8000.Init();   // A
    mFormat8888.Init();   // BGRA
    mFormat4444.Init();   // BGRA
    mFormat0565.Init();   // BGR
    mFormat0555.Init();   // BGR
    mFormat1555.Init();   // BGRA
    mFormatU8V8.Init();   // Bump
    mFormatU5V5L6.Init(); // Bump
    mFormatDXT1.Init();   // DXT1
    mFormatDXT3.Init();   // DXT3
    mFormatDXT5.Init();   // DXT5
    mFormatPal8.Init();   // Paletted8

    // Find the needed texture formats.
    guardnw(FindTextureFormats);
    {
      mFirstPixelFormat = NULL;
      if(m_TextureBits >= 24)
      {
        if(SUCCEEDED(m_pD3D->CheckDeviceFormat( m_dwAdapter, m_d3dCaps.DeviceType, m_d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8)))
          RecognizePixelFormat(mFormat8888, D3DFMT_A8R8G8B8, 32, "8888");
      }
      if( !mFirstPixelFormat )
      {
        m_TextureBits = 16;

        if(SUCCEEDED(m_pD3D->CheckDeviceFormat( m_dwAdapter, m_d3dCaps.DeviceType, m_d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A1R5G5B5)))
          RecognizePixelFormat(mFormat1555, D3DFMT_A1R5G5B5, 16, "1555");

        if(SUCCEEDED(m_pD3D->CheckDeviceFormat( m_dwAdapter, m_d3dCaps.DeviceType, m_d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A4R4G4B4)))
          RecognizePixelFormat(mFormat4444, D3DFMT_A4R4G4B4, 16, "4444");

        if(SUCCEEDED(m_pD3D->CheckDeviceFormat( m_dwAdapter, m_d3dCaps.DeviceType, m_d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_R5G6B5)))
          RecognizePixelFormat(mFormat0565, D3DFMT_R5G6B5, 16, "0565");
        else
        if(SUCCEEDED(m_pD3D->CheckDeviceFormat( m_dwAdapter, m_d3dCaps.DeviceType, m_d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_X1R5G5B5)))
          RecognizePixelFormat(mFormat0555, D3DFMT_X1R5G5B5, 16, "0555");
      }
      // Alpha formats
      if(SUCCEEDED(m_pD3D->CheckDeviceFormat( m_dwAdapter, m_d3dCaps.DeviceType, m_d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A8)))
        RecognizePixelFormat(mFormat8000, D3DFMT_A8, 8, "Alpha8");

      // Bump formats
      if(SUCCEEDED(m_pD3D->CheckDeviceFormat( m_dwAdapter, m_d3dCaps.DeviceType, m_d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_V8U8)))
        RecognizePixelFormat(mFormatU8V8, D3DFMT_V8U8, 16, "BumpU8V8");
      if(SUCCEEDED(m_pD3D->CheckDeviceFormat( m_dwAdapter, m_d3dCaps.DeviceType, m_d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_Q8W8V8U8)))
        RecognizePixelFormat(mFormatU8V8, D3DFMT_Q8W8V8U8, 32, "BumpQ8W8U8V8");
      if(SUCCEEDED(m_pD3D->CheckDeviceFormat( m_dwAdapter, m_d3dCaps.DeviceType, m_d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_X8L8V8U8)))
        RecognizePixelFormat(mFormatU8V8, D3DFMT_X8L8V8U8, 32, "BumpX8L8U8V8");
      if(SUCCEEDED(m_pD3D->CheckDeviceFormat( m_dwAdapter, m_d3dCaps.DeviceType, m_d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_L6V5U5)))
        RecognizePixelFormat(mFormatU5V5L6, D3DFMT_L6V5U5, 16, "BumpU5V5L6");

      // Paletted formats
      if (CV_d3d8_palettedtextures)
      {
        if(SUCCEEDED(m_pD3D->CheckDeviceFormat( m_dwAdapter, m_d3dCaps.DeviceType, m_d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_P8)))
        {
          RecognizePixelFormat(mFormatPal8, D3DFMT_P8, 8, "Paletted8");
          m_Features |= RFT_PALTEXTURE;
        }
      }

      // Compressed formats
      if (CV_d3d8_compressedtextures)
      {
        if(SUCCEEDED(m_pD3D->CheckDeviceFormat( m_dwAdapter, m_d3dCaps.DeviceType, m_d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT1)))
        {
          RecognizePixelFormat(mFormatDXT1, D3DFMT_DXT1, 8, "DXT1");
          m_Features |= RFT_COMPRESSTEXTURE;
        }
        if(SUCCEEDED(m_pD3D->CheckDeviceFormat( m_dwAdapter, m_d3dCaps.DeviceType, m_d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT3)))
        {
          RecognizePixelFormat(mFormatDXT3, D3DFMT_DXT3, 8, "DXT3");
          m_Features |= RFT_COMPRESSTEXTURE;
        }
        if(SUCCEEDED(m_pD3D->CheckDeviceFormat( m_dwAdapter, m_d3dCaps.DeviceType, m_d3dpp.BackBufferFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT5)))
        {
          RecognizePixelFormat(mFormatDXT5, D3DFMT_DXT5, 8, "DXT5");
          m_Features |= RFT_COMPRESSTEXTURE;
        }
      }
    }
    unguardnw;
  }
  unguardnw;

  // Verify mipmapping supported.
  if (!(m_d3dCaps.TextureCaps & D3DPTEXTURECAPS_MIPMAP))
  {
    iLog->Log ("D3D Driver: Mipmapping not available with this driver\n");
    m_Features |= RFT_NOMIPS;
  }
  else
  {
    if( m_d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR )
      iLog->Log ("D3D Driver: Supports trilinear texture filtering\n");
  }
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
    m_d3dCaps.MaxTextureAspectRatio = Max<int>(1,Max<int>(m_d3dCaps.MaxTextureWidth,m_d3dCaps.MaxTextureHeight));
  if( !(m_d3dCaps.TextureCaps & D3DPTEXTURECAPS_POW2) )
    iLog->Log ("D3D Driver: Supports non-power-of-2 textures\n");
  if( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS )
    iLog->Log ("D3D Driver: Supports LOD biasing\n");
  if( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_ZBIAS )
    iLog->Log ("D3D Driver: Supports Z biasing\n");
  else
    m_Features &= ~RFT_SUPPORTZBIAS;
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
  if( m_d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFAFLATCUBIC )
  {
    m_bDeviceDoesFlatCubic = TRUE;
    iLog->Log ("D3D Driver: Supports FlatCubic Filter\n"); 
  }
  if( m_d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC )
  {
    m_bDeviceDoesGaussianCubic = TRUE;
    iLog->Log ("D3D Driver: Supports GaussianCubic Filter\n"); 
  }
  if( m_d3dCaps.StencilCaps & (D3DSTENCILCAPS_KEEP|D3DSTENCILCAPS_INCR|D3DSTENCILCAPS_DECR) )
    iLog->Log ("D3D Driver: Supports Stencil shadows\n"); 
  else
    CRenderer::m_sbpp = 0;

  iLog->Log ("D3D Driver: Textures (%ix%i)-(%ix%i), Max aspect %i\n", mMinTextureWidth, mMinTextureHeight, m_d3dCaps.MaxTextureWidth, m_d3dCaps.MaxTextureHeight, m_d3dCaps.MaxTextureAspectRatio );

  // Depth buffering.
  m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
  if ( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_WBUFFER )
  {
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_USEW );
    m_bUseWBuffer = true;
    iLog->Log ("D3D Driver: Supports w-buffering\n");
  }
  else
    m_bUseWBuffer = false;

  m_Features &= ~RFT_BUMP;
  if( CV_d3d8_usebumpmap && (GetFeatures() & RFT_MULTITEXTURE))
  {
    int type = CV_d3d8_bumptype;
    if (type == 0 && (m_d3dCaps.TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP) && m_d3dCaps.MaxTextureBlendStages>=3)
      m_Features |= RFT_BUMP_EMBM;
    else
    if (type == 1 && (m_d3dCaps.TextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3) && (m_d3dCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP) && mFormat8888.BitsPerPixel)
      m_Features |= RFT_BUMP_DOT3;
  }

  // Init render states.
  guardnw(InitRenderState);
  {
    m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_COLORVERTEX, TRUE );    
    D3DSetCull(eCULL_None);
    m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF, 127 );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
    if( m_d3dCaps.RasterCaps & D3DPRASTERCAPS_ZBIAS )
      m_pd3dDevice->SetRenderState( D3DRS_ZBIAS, 0 );
    m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,   D3DFILL_SOLID );
    m_pd3dDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE);
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE);
    m_pd3dDevice->SetRenderState( D3DRS_FOGCOLOR, 0 );        
    m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_LINEAR );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE, FALSE);
#ifndef _XBOX
    m_pd3dDevice->SetRenderState( D3DRS_CLIPPING, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE,  FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
#else
    //m_pd3dDevice->SetBackBufferScale(1.0f, 1.0f);
    m_pd3dDevice->SetPixelShader( NULL );
    m_pd3dDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_POINTSCALEENABLE,  FALSE );
#endif
    m_pd3dDevice->SetRenderState( D3DRS_VERTEXBLEND,      FALSE );
  }
  unguardnw;

  msCurState = GS_DEPTHWRITE | GS_NODEPTHTEST;
  for (int i=0; i<MAX_TMU; i++)
  {
    eCurColorOp[i] = eCO_DISABLE;
  }

  // Init texture stage state.
  guardnw(InitTextureStageState);
  {
    FLOAT LodBias=-0.5;
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPMAPLODBIAS, *(DWORD*)&LodBias );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,   D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,   D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    EF_SetColorOp(eCO_MODULATE);
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR );
    mStages[0].MinFilter = D3DTEXF_LINEAR;
    mStages[0].MagFilter = D3DTEXF_LINEAR;
    mStages[0].UseMips = 1;
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
#ifdef _XBOX
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_RESULTARG, D3DTA_CURRENT);
#endif


    guardnw( TextureStagesSetup )
    {
      // Set stage 0 state.
      m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
      
      if( GetFeatures() & RFT_MULTITEXTURE )
      {
        for( i=1; i<(int)m_d3dCaps.MaxSimultaneousTextures; i++ )
        {
          m_TexMan->m_CurStage = i;
          
          // Set stage i state.
          m_pd3dDevice->SetTextureStageState( i, D3DTSS_ADDRESSU,  D3DTADDRESS_WRAP );
          m_pd3dDevice->SetTextureStageState( i, D3DTSS_ADDRESSV,  D3DTADDRESS_WRAP );
          m_pd3dDevice->SetTextureStageState( i, D3DTSS_COLORARG1, D3DTA_TEXTURE );
          m_pd3dDevice->SetTextureStageState( i, D3DTSS_COLORARG2, D3DTA_CURRENT );
          EF_SetColorOp(eCO_DISABLE);
          m_pd3dDevice->SetTextureStageState( i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
          m_pd3dDevice->SetTextureStageState( i, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
          m_pd3dDevice->SetTextureStageState( i, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
          m_pd3dDevice->SetTextureStageState( i, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
          m_pd3dDevice->SetTextureStageState( i, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
          m_pd3dDevice->SetTextureStageState( i, D3DTSS_MIPFILTER, D3DTEXF_LINEAR );
          m_pd3dDevice->SetTextureStageState( i, D3DTSS_TEXCOORDINDEX, i );
          mStages[i].MinFilter = D3DTEXF_LINEAR;
          mStages[i].MagFilter = D3DTEXF_LINEAR;
          mStages[i].UseMips = 1;
#ifdef _XBOX
          m_pd3dDevice->SetTextureStageState(0, D3DTSS_RESULTARG, D3DTA_CURRENT);
#endif
        }
      }
    }
    unguardnw;
  }
  unguardnw;

  m_TexMan->m_CurStage = 0;
  m_TexMan->SetFilter(CV_d3d8_texturefilter->GetString());


  // For safety, lots of drivers don't handle tiny texture sizes too tell.
  mMinTextureWidth       = 8;
  mMinTextureHeight      = 8;
  m_d3dCaps.MaxTextureAspectRatio = Min<int>( m_d3dCaps.MaxTextureAspectRatio, 2 );//8 is safe, this is just more efficient

  m_MaxTextureSize = Min(m_d3dCaps.MaxTextureHeight, m_d3dCaps.MaxTextureWidth); //Min(wdt, hgt);

  return S_OK;
}

void CD3D8Renderer::RecognizePixelFormat(SPixFormat& Dest, D3DFORMAT FromD3D, INT InBitsPerPixel, const TCHAR* InDesc)
{
  guard(CD3D8Renderer::RecognizePixelFormat);

  Dest.Init();
  Dest.Format        = FromD3D;
  Dest.MaxWidth      = m_d3dCaps.MaxTextureWidth;
  Dest.MaxHeight     = m_d3dCaps.MaxTextureHeight;
  Dest.Desc          = InDesc;
  Dest.BitsPerPixel  = InBitsPerPixel;
  Dest.Next          = mFirstPixelFormat;
  mFirstPixelFormat   = &Dest;

  iLog->Log("  Using '%s' pixel texture format\n", InDesc);

  unguard;
}

HRESULT CD3D8Renderer::RestoreDeviceObjects()
{
  //InitTexFillers();
  //Flush();

  for (int i=0; i<MAX_TMU; i++)
  {
    mStages[i].Flush();
  }
  
  return S_OK;
}

void CD3D8Renderer::SetRendParms(SD3DModeInfo *pModeInfo, SD3DDeviceInfo *pDeviceInfo)
{
  m_bFullScreen = !pDeviceInfo->bWindowed;
  m_AlphaDepth = 0;
  if (m_bFullScreen)
  {
    CRenderer::m_width = pModeInfo->Width;
    CRenderer::m_height = pModeInfo->Height;
  }
  else
  {
    pModeInfo->Width = CRenderer::m_width;
    pModeInfo->Height = CRenderer::m_height;
    if (m_cbpp == 32)
      pModeInfo->Format = D3DFMT_A8R8G8B8;
    else
    if (m_cbpp == 24)
      pModeInfo->Format = D3DFMT_X8R8G8B8;
    else
      pModeInfo->Format = D3DFMT_R5G6B5;
  }
  m_AlphaDepth = 0;
  switch (pModeInfo->Format)
  {
    case D3DFMT_X8R8G8B8:
      CRenderer::m_cbpp = 24;
      break;

    case D3DFMT_A8R8G8B8:
      m_AlphaDepth = 8;
      CRenderer::m_cbpp = 32;
      break;

    case D3DFMT_A1R5G5B5:
      m_AlphaDepth = 1;
      CRenderer::m_cbpp = 16;
      break;
    case D3DFMT_A4R4G4B4:
      m_AlphaDepth = 4;
      CRenderer::m_cbpp = 16;
      break;
    case D3DFMT_R5G6B5:
    case D3DFMT_X1R5G5B5:
      CRenderer::m_cbpp = 16;
      break;
      
#ifdef _XBOX
    case D3DFMT_LIN_X8R8G8B8:
      CRenderer::m_cbpp = 24;
      break;
    case D3DFMT_LIN_X1R5G5B5:
    case D3DFMT_LIN_R5G6B5:
      CRenderer::m_cbpp = 16;
      break;
    case D3DFMT_LIN_A8R8G8B8:
      CRenderer::m_cbpp = 32;
      break;
      
#endif
      
  }
  CRenderer::m_sbpp = 0;
  CRenderer::m_zbpp = 0;
  switch (pModeInfo->DepthStencilFormat)
  {
    case D3DFMT_D16:
      CRenderer::m_zbpp = 16;
      break;

#ifndef _XBOX      
    case D3DFMT_D32:
      CRenderer::m_zbpp = 32;
      break;

    case D3DFMT_D15S1:
      CRenderer::m_sbpp = 1;
      CRenderer::m_zbpp = 16;
      break;
#endif      

    case D3DFMT_D24S8:
      CRenderer::m_sbpp = 8;
      CRenderer::m_zbpp = 24;
      break;

#ifndef _XBOX      
    case D3DFMT_D24X8:
      CRenderer::m_zbpp = 24;
      break;

    case D3DFMT_D24X4S4:
      CRenderer::m_zbpp = 24;
      CRenderer::m_sbpp = 4;
      break;

#endif
  }
}

//-----------------------------------------------------------------------------
// Name: CD3D8Renderer::mfInitialize3DEnvironment()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3D8Renderer::Initialize3DEnvironment()
{
  HRESULT hr;

  SD3DAdapterInfo* pAdapterInfo = &m_Adapters[m_dwAdapter];
  SD3DDeviceInfo*  pDeviceInfo  = &pAdapterInfo->devices[pAdapterInfo->dwCurrentDevice];
  SD3DModeInfo*    pModeInfo    = &pDeviceInfo->modes[pDeviceInfo->dwCurrentMode];

  SetRendParms(pModeInfo, pDeviceInfo);

  // Prepare window for possible windowed/fullscreen change
  AdjustWindowForChange();

  // Set up the presentation parameters
  ZeroMemory( &m_d3dpp, sizeof(m_d3dpp) );
  m_d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
  m_d3dpp.Windowed               = pDeviceInfo->bWindowed;
  m_d3dpp.BackBufferCount        = 1;
  m_d3dpp.MultiSampleType        = pDeviceInfo->MultiSampleType;
  // Anti-aliasing
  //m_d3dpp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_LINEAR;
#ifdef _XBOX
  m_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
#else
  m_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
#endif
  m_d3dpp.EnableAutoDepthStencil = TRUE;
  m_d3dpp.AutoDepthStencilFormat = pModeInfo->DepthStencilFormat;
  m_d3dpp.hDeviceWindow          = (HWND)m_hWnd;
  if( !m_bFullScreen )
  {
    m_d3dpp.BackBufferWidth  = m_width; // m_rcWindowClient.right - m_rcWindowClient.left;
    m_d3dpp.BackBufferHeight = m_height; //m_rcWindowClient.bottom - m_rcWindowClient.top;
    m_d3dpp.BackBufferFormat = pAdapterInfo->d3ddmDesktop.Format;
  }
  else
  {
#ifndef _DEBUG
    //m_d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;//D3DPRESENT_RATE_UNLIMITED;
    //m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
#endif
    m_d3dpp.BackBufferWidth  = pModeInfo->Width;
    m_d3dpp.BackBufferHeight = pModeInfo->Height;
    m_d3dpp.BackBufferFormat = pModeInfo->Format;
  }

  pModeInfo->dwBehavior &=~(D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_HARDWARE_VERTEXPROCESSING);
  if((pModeInfo->dwBehavior & D3DCREATE_PUREDEVICE) == 0)
  {
    D3DCAPS8 caps;
    // Query caps
    hr = m_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, pDeviceInfo->DeviceType, &caps);
    if((m_d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
    {
      if(D3DSHADER_VERSION_MAJOR(caps.VertexShaderVersion) < 1)
      {
        pModeInfo->dwBehavior |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        m_bUseSWVP = true;
      }
      else
      {
        if (CV_d3d8_forcesoftwarevp)
        {
          m_bUseSWVP = true;
          pModeInfo->dwBehavior |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }
        else
        {
          m_bUseSWVP = false;
          pModeInfo->dwBehavior |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
        }
      }
    }
    else
    {
      pModeInfo->dwBehavior |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
      m_bUseSWVP = true;
    }
  }
  else
  {
    pModeInfo->dwBehavior |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
  }

#ifdef _XBOX
  pModeInfo->dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
#endif

  // Allow unlimited frame rate
  if (m_bFullScreen)
    m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

  // Create the device
  hr = m_pD3D->CreateDevice( m_dwAdapter, pDeviceInfo->DeviceType,
                             (HWND)m_CurrContext->m_hWnd, pModeInfo->dwBehavior, &m_d3dpp,
                             &m_pd3dDevice );
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
#ifndef _XBOX      
    m_pd3dDevice->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, m_bUseSWVP);
    if( !m_bFullScreen )
    {
      SetWindowPos( m_hWnd, HWND_NOTOPMOST,
                    m_rcWindowBounds.left, m_rcWindowBounds.top,
                    ( m_rcWindowBounds.right - m_rcWindowBounds.left ),
                    ( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
                    SWP_SHOWWINDOW );
    }
#endif
    // Store device Caps
    m_pd3dDevice->GetDeviceCaps( &m_d3dCaps );
    m_dwCreateFlags = pModeInfo->dwBehavior;

    if (m_d3dCaps.MaxSimultaneousTextures == 2 && D3DSHADER_VERSION_MAJOR(m_d3dCaps.VertexShaderVersion) == 1 && D3DSHADER_VERSION_MINOR(m_d3dCaps.VertexShaderVersion) == 1)
      m_Features |= RFT_HW_GF2;
    if (m_d3dCaps.MaxSimultaneousTextures == 4 && D3DSHADER_VERSION_MAJOR(m_d3dCaps.PixelShaderVersion) == 1 && D3DSHADER_VERSION_MINOR(m_d3dCaps.PixelShaderVersion) == 1 && m_d3dCaps.MaxTextureBlendStages == 8)
      m_Features |= RFT_HW_GF3 | RFT_HW_TS | RFT_HW_VS;
    else
    if (m_d3dCaps.MaxSimultaneousTextures == 4 && D3DSHADER_VERSION_MAJOR(m_d3dCaps.PixelShaderVersion) == 1 && D3DSHADER_VERSION_MINOR(m_d3dCaps.PixelShaderVersion) >= 1 && m_d3dCaps.MaxTextureBlendStages >= 8)
       m_Features |= RFT_HW_GF3 | RFT_HW_TS | RFT_HW_VS;
    else
    if (m_d3dCaps.MaxSimultaneousTextures > 4 && D3DSHADER_VERSION_MAJOR(m_d3dCaps.PixelShaderVersion) == 1 && D3DSHADER_VERSION_MINOR(m_d3dCaps.PixelShaderVersion) >= 3 && m_d3dCaps.MaxTextureBlendStages >= 8)
      m_Features |= RFT_HW_RADEON8500 | RFT_HW_TS | RFT_HW_VS;
#ifdef _XBOX
     m_Features |= RFT_HW_GF3 | RFT_HW_TS | RFT_HW_VS;
#endif

    // Store device description
    if( pDeviceInfo->DeviceType == D3DDEVTYPE_REF )
      lstrcpy( m_strDeviceStats, TEXT("REF") );
    else
    if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
      lstrcpy( m_strDeviceStats, TEXT("HAL") );
    else
    if( pDeviceInfo->DeviceType == D3DDEVTYPE_SW )
      lstrcpy( m_strDeviceStats, TEXT("SW") );

    if( (pModeInfo->dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING) && (pModeInfo->dwBehavior & D3DCREATE_PUREDEVICE) )
    {
      if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
        lstrcat( m_strDeviceStats, TEXT(" (pure hw vp)") );
      else
        lstrcat( m_strDeviceStats, TEXT(" (simulated pure hw vp)") );
    }
    else
    if( pModeInfo->dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING )
    {
      if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
        lstrcat( m_strDeviceStats, TEXT(" (hw vp)") );
      else
        lstrcat( m_strDeviceStats, TEXT(" (simulated hw vp)") );
    }
    else
    if( pModeInfo->dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING )
    {
      if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
        lstrcat( m_strDeviceStats, TEXT(" (mixed vp)") );
      else
        lstrcat( m_strDeviceStats, TEXT(" (simulated mixed vp)") );
    }
    else
    if( pModeInfo->dwBehavior & D3DCREATE_SOFTWARE_VERTEXPROCESSING )
    {
      lstrcat( m_strDeviceStats, TEXT(" (sw vp)") );
    }

    if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
    {
      lstrcat( m_strDeviceStats, TEXT(": ") );
      lstrcat( m_strDeviceStats, pAdapterInfo->d3dAdapterIdentifier.Description );
    }

    // Store render target surface desc
    m_pd3dDevice->GetDepthStencilSurface( &m_pZBuffer );
    m_pZBuffer->GetDesc( &m_d3dsdZBuffer );
    CD3D8TexMan *pTM = (CD3D8TexMan *)m_TexMan;
    m_pd3dDevice->CreateDepthStencilSurface(512, 512, m_d3dsdZBuffer.Format, D3DMULTISAMPLE_NONE, &pTM->m_pZSurf);
    if (m_d3dsdZBuffer.Format == D3DFMT_D16_LOCKABLE)
      m_Features |= RFT_ZLOCKABLE;
    
#ifdef _XBOX
    m_Features |= RFT_ZLOCKABLE;
#endif

    m_pd3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer );

    // Initialize the app's device-dependent objects
    hr = InitDeviceObjects();
    if( SUCCEEDED(hr) )
    {
      hr = RestoreDeviceObjects();
      if( SUCCEEDED(hr) )
      {
        m_bActive = TRUE;
        return S_OK;
      }
    }
  }

  return hr;
}

static int sCDFromFormat(D3DFORMAT fmt)
{
  switch (fmt)
  {
    case D3DFMT_X8R8G8B8:
      return 24;
    case D3DFMT_A8R8G8B8:
      return 32;
    case D3DFMT_R5G6B5:
      return 16;
#ifdef _XBOX
    case D3DFMT_LIN_X8R8G8B8:
      return 24;
    case D3DFMT_LIN_X1R5G5B5:
    case D3DFMT_LIN_R5G6B5:
      return 16;
    case D3DFMT_LIN_A8R8G8B8:
      return 32;
#endif
  }
  return -1;
}

bool CD3D8Renderer::IsSuitableDevice(int a, bool bAllowSoft)
{
  if (a > m_dwNumAdapters)
    a = m_dwNumAdapters-1;
  if (a < 0)
    a = 0;
  for( DWORD d=0; d < m_Adapters[a].dwNumDevices; d++ )
  {
    SD3DDeviceInfo* pDevice;
    SD3DAdapterInfo* pAdapter = &m_Adapters[a];
    pDevice = &pAdapter->devices[d];
    if (pDevice->DeviceType != D3DDEVTYPE_HAL && !bAllowSoft)
      continue;
    if (!m_bFullScreen && !pDevice->bCanDoWindowed)
      continue;
    if (m_bFullScreen)
    {
      int BestError = 999999;
      int Best;
      int BestCD;
      for (DWORD m=0; m<pDevice->dwNumModes; m++)
      {
        SD3DModeInfo *mi = &pDevice->modes[m];
        int thisCD = sCDFromFormat(mi->Format);
        if (thisCD == -1)
          continue;
        int ThisError = Abs((int)mi->Width-(int)CRenderer::m_width) + Abs((int)mi->Height-(int)CRenderer::m_height) + Abs((int)thisCD-(int)CRenderer::m_cbpp);
        if (ThisError < BestError)
        {
          Best = m;
          BestCD = thisCD;
          BestError = ThisError;
        }
      }
      if( BestError == 999999 )
        continue;
      m_Adapters[a].dwCurrentDevice = d;
      m_dwAdapter = a;
      pDevice->dwCurrentMode = Best;

      CRenderer::m_cbpp    = BestCD;
      SD3DModeInfo *mi = &pDevice->modes[Best];
      CRenderer::m_width  = mi->Width;
      CRenderer::m_height = mi->Height;
      iLog->Log ("Best-match display mode: %ix%ix%i (Error=%i)\n",CRenderer::m_width,CRenderer::m_height,CRenderer::m_cbpp,BestError);

      if (CV_d3d8_texturebits)
        m_TextureBits = CV_d3d8_texturebits;
      else
        m_TextureBits = CRenderer::m_cbpp;
    }
    else
    {
      if (CV_d3d8_texturebits)
        m_TextureBits = CV_d3d8_texturebits;
      else
        m_TextureBits = CRenderer::m_cbpp;
    }
    break;
  }
  if (d == m_Adapters[a].dwNumDevices)
    return false;

  m_dwAdapter = a;
  m_Adapters[a].dwCurrentDevice = d;
  return true;
}

bool CD3D8Renderer::SetRes(void)  
{
  HRESULT hr;

  guardnw(CD3D8Renderer::SetRes);

  UnSetRes();

  // Create the Direct3D object
  m_pD3D = Direct3DCreate8( D3D_SDK_VERSION );
  if( m_pD3D == NULL )
    return Error("Direct3D8 not installed", 0);

  // Build a list of Direct3D adapters, modes and devices. The
  // ConfirmDevice() callback is used to confirm that only devices that
  // meet the app's requirements are considered.
  if( FAILED( hr = BuildDevicesList() ) )
  {
    if (h == D3DAPPERR_NOWINDOWABLEDEVICES)
    {
      if (!m_bFullScreen)
        return Error("Couldn't build list of D3D devices", h);
    }
    else
      return Error("Couldn't build list of D3D devices", h);
  }

  char drv[64];
  strcpy(drv, CV_d3d8_device->GetString());

  int numAdap;
  bool bAuto = false;
  int numDev = 0;
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
    iLog->Log ( "Only 'Auto', 'Primary', '3dfx' or digital number of device are supported\n");
    bAuto = true;
  }
  if (numDev < 0)
    numDev = 0;

  m_dwAdapter = -1;
  bool bAllowSoft = false;
trysoft:
  if (bAuto)
  {
    for( int a=0; a<m_dwNumAdapters; a++ )
    {
      if (IsSuitableDevice(a, bAllowSoft))
        break;
    }
  }
  else
  {
    IsSuitableDevice(numDev, bAllowSoft);
  }

  if (m_dwAdapter < 0)
  {
    if (!bAllowSoft)
    {
      if (CV_d3d8_allowsoftware)
      {
        bAllowSoft = true;
        goto trysoft;
      }
    }
    return Error("Can't find any suitable D3D device\n", 0);
  }

  int a = m_dwAdapter;
  int d = m_Adapters[a].dwCurrentDevice;

  // Display a warning message
  if( !m_bHALExists )
    iLog->Log ( "No hardware-accelerated Direct3D devices were found.\n");
  else
  if( !m_bHALIsSampleCompatible )
    iLog->Log ( "FarCry requires functionality that is not available on your Direct3D hardware accelerator.\n");
  else
  if( !m_bHALIsWindowedCompatible )
    iLog->Log ( "Your Direct3D hardware accelerator cannot render into a window.\n");
  else
  if( !m_bHALIsDesktopCompatible )
    iLog->Log ( "Your Direct3D hardware accelerator cannot render into a window with the current desktop display settings.\n");

  // The focus window can be a specified to be a different window than the
  // device window.  If not, use the device window as the focus window.
  if( m_CurrContext == NULL )
    CreateContext(m_hWnd);

#ifndef _XBOX      
  // Save window properties
  GetWindowRect( m_hWnd, &m_rcWindowBounds );
  GetClientRect( m_hWnd, &m_rcWindowClient );
#endif

  // Initialize the 3D environment for the app
  if( FAILED( hr = Initialize3DEnvironment() ) )
    return Error("Couldn't initialize 3D environment", h);

  if (FAILED (hr = D3DXCreateMatrixStack(0, &m_matView)))
    return false;
  if (FAILED (hr = D3DXCreateMatrixStack(0, &m_matProj)))
    return false;
  D3DXMatrixIdentity(&m_TexMatrix[0]);
  D3DXMatrixIdentity(&m_TexMatrix[1]);
  D3DXMatrixIdentity(&m_TexMatrix[2]);
  D3DXMatrixIdentity(&m_TexMatrix[3]);

  hr = m_pd3dDevice->CreateVertexBuffer(300 * STRIDE_TR_D_1T, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFVF_TRVERTEX_D_1T, D3DPOOL_DEFAULT, &m_pQuadVB);
  if (FAILED(hr))
    return false;
  
  hr = m_pd3dDevice->CreateVertexBuffer(8 * STRIDE_D, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFVF_VERTEX_D, D3DPOOL_DEFAULT, &m_pLineVB);
  if (FAILED(hr))
    return false;

  memset(&m_Material, 0, sizeof(m_Material));
  for (int i=0; i<16; i++)
  {
    D3DLIGHT8 *l = &m_Lights[i];
    memset(l, 0, sizeof(D3DLIGHT8));
    l->Type = D3DLIGHT_DIRECTIONAL;
    l->Attenuation0 = 1.0f;
  }

  return true;

  unguardnw;
} 

bool CD3D8Renderer::ChangeDisplay(unsigned int width,unsigned int height,unsigned int cbpp)
{
  return false;
}

void CD3D8Renderer::ChangeViewport(unsigned int x,unsigned int y,unsigned int width,unsigned int height)
{
  assert(m_CurrContext);
  m_CurrContext->m_X = x;
  m_CurrContext->m_Y = y;
  m_CurrContext->m_Width = width;
  m_CurrContext->m_Height = height;
  m_width = width;
  m_height = height;
  m_VWidth = width;
  m_VHeight = height;
  if (m_pd3dDevice)
  {
    m_d3dpp.BackBufferWidth = width;
    m_d3dpp.BackBufferHeight = height;
    m_Viewport.X = x;
    m_Viewport.Y = y;
    m_Viewport.Width = width;
    m_Viewport.Height = height;
    m_pd3dDevice->SetViewport(&m_Viewport);

    //m_pd3dDevice->Reset(&m_d3dpp);
  }
}

float glLodCorrection;

void CD3D8Renderer::BeginFrame()
{
  //////////////////////////////////////////////////////////////////////
  // Set up everything so we can start rendering
  //////////////////////////////////////////////////////////////////////

  HRESULT hReturn;

  ASSERT(m_pd3dDevice);

  if (m_SceneRecurseCount++)
    return;

  if (CV_r_vsync != m_VSync)
  {
    m_VSync = CV_r_vsync;
		EnableVSync(m_VSync?true:false);
  }
  if (CV_r_reloadshaders)
  {
    gcEf.mfReloadAllShaders(CV_r_reloadshaders);
    CV_r_reloadshaders = 0;
  }

  //////////////////////////////////////////////////////////////////////
  // Build the matrices
  //////////////////////////////////////////////////////////////////////
  
  // Set the world matrix to an indetity matrix
  //D3DXMatrixIdentity(&matWorld);
  m_matView->LoadIdentity();
  m_pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)m_matView->GetTop());
  
  if (CV_r_gamma+m_fDeltaGamma != m_fLastGamma || CV_r_brightness != m_fLastBrightness || CV_r_contrast != m_fLastContrast)
    SetGamma(CV_r_gamma+m_fDeltaGamma, CV_r_brightness, CV_r_contrast);

  gcEf.mfBeginFrame();

  if (CV_r_PolygonMode!=m_polygon_mode)
    SetPolygonMode(CV_r_PolygonMode);	

  m_bWasCleared = false;
//  EF_ClearBuffer(false, NULL);

  //////////////////////////////////////////////////////////////////////
  // Begin the scene
  //////////////////////////////////////////////////////////////////////

  hReturn = m_pd3dDevice->BeginScene();

  if (FAILED(hReturn))
    return;

  { // Calculate lod correction if more than 100 000 poly used
    float k = ((float)m_nPolygons-100000.f)/10000.f;
    
    float d = 0.01f;
    if(fabs(k)<1)
      d/=10;

    if(k<-0.1f)
      glLodCorrection-=d;
    
    if(k> 0.1f)
      glLodCorrection+=d;

    if(glLodCorrection<0)
      glLodCorrection=0;
  }

  SetMaterialColor(1,1,1,1);

  if (strcmp(CV_d3d8_texturefilter->GetString(), m_TexMan->m_CurTexFilter) || CV_r_texture_anisotropic_level != m_TexMan->m_CurAnisotropic)
    m_TexMan->SetFilter(CV_d3d8_texturefilter->GetString());

  if (CV_r_log && !m_LogFile)
  {
    m_LogFile = fxopen ("Direct3DLog.txt", "w");
    if (m_LogFile)
    {      
      iLog->Log("Direct3D log file '%s' opened\n", "Direct3DLog.txt");
      char time[128];
      char date[128];
      
      _strtime( time );
      _strdate( date );
      
      fprintf(m_LogFile, "\n==========================================\n");
      fprintf(m_LogFile, "Direct3D Log file opened: %s (%s)\n", date, time);
      fprintf(m_LogFile, "==========================================\n");
    }
  }
  else
  if (!CV_r_log && m_LogFile)
  {
    char time[128];
    char date[128];
    _strtime( time );
    _strdate( date );
    
    fprintf(m_LogFile, "\n==========================================\n");
    fprintf(m_LogFile, "Direct3D Log file closed: %s (%s)\n", date, time);
    fprintf(m_LogFile, "==========================================\n");
    
    fclose(m_LogFile);
    m_LogFile = NULL;
    iLog->Log("Direct3D log file '%s' closed\n", "Direct3DLog.txt");
  }

  ResetToDefault();

  m_MatDepth = 0;
  m_nPolygons = 0;
  m_nFrameID++;

  TArray<CRETempMesh *> *tm = &m_RP.m_TempMeshes[m_nFrameID & 1];
  for (int i=0; i<tm->Num(); i++)
  {
    CRETempMesh *re = tm->Get(i);
    if (!re)
      continue;
    if (re->m_VBuffer)
    {
      ReleaseBuffer(re->m_VBuffer);
      re->m_VBuffer = NULL;
    }
  }
  tm->SetUse(0);
  m_RP.m_CurTempMeshes = tm;
}

void CD3D8Renderer::Update()
{
  //////////////////////////////////////////////////////////////////////
  // End the scene and update
  //////////////////////////////////////////////////////////////////////

  // Check for the presence of a D3D device
  ASSERT(m_pd3dDevice);

  if (!m_SceneRecurseCount)
  {
    iLog->Log("EndScene without BeginScene\n");
    return;
  }
  m_SceneRecurseCount--;

  if (m_SceneRecurseCount)
    return;


  if (CV_r_measureoverdraw)
  {
    /*byte *buf = new byte [m_width*m_height];
    glReadPixels(0, 0, m_width, m_height, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, buf);

    int size = m_width * m_height;
    int acc = 0;
    for (int i=0; i<size; i++)
    {
      acc += buf[i];
    }
    delete [] buf;
    m_RP.m_PS.m_fOverdraw += (float)acc; */
  }

  if (iConsole && iConsole->GetFont())
  { 
    CRenderer *crend=gRenDev;
    CXFont    *cf=iConsole->GetFont();
    if (CV_r_measureoverdraw)
      crend->WriteXY(cf,550,75, 0.5f,1,1,1,1,1,"Overdraw=%0.3f",m_RP.m_PS.m_fOverdraw / (float)(m_width * m_height));
    switch (CV_r_stats)
    {
      case 1:
        crend->WriteXY(cf,550,40, 0.5f,1,1,1,1,1, "Shaders=%d",m_RP.m_PS.m_NumRendShaders);
        crend->WriteXY(cf,550,55, 0.5f,1,1,1,1,1, "VPrograms=%d",m_RP.m_PS.m_NumVShaders);
        crend->WriteXY(cf,550,70, 0.5f,1,1,1,1,1, "RCombiners=%d",m_RP.m_PS.m_NumRCombiners);
        crend->WriteXY(cf,550,85, 0.5f,1,1,1,1,1, "PShaders=%d",m_RP.m_PS.m_NumPShaders);
        crend->WriteXY(cf,550,100, 0.5f,1,1,1,1,1, "CGVShaders=%d",m_RP.m_PS.m_NumCGVShaders);
        crend->WriteXY(cf,550,115, 0.5f,1,1,1,1,1, "CGPShaders=%d",m_RP.m_PS.m_NumCGPShaders);
        crend->WriteXY(cf,550,140, 0.5f,1,1,1,1,1,"Textures=%d",m_RP.m_PS.m_NumTextures);
        crend->WriteXY(cf,550,155, 0.5f,1,1,1,1,1,"TexturesSize=%.03f Kb",m_RP.m_PS.m_TexturesSize/1024.0f);
        break;

      case 2:
        {
          crend->WriteXY(cf,550,140, 0.5f,1,1,1,1,1,"FFTTables=%0.3f",CREOcean::m_RS.m_StatsTimeFFTTable);
          crend->WriteXY(cf,550,155, 0.5f,1,1,1,1,1,"FFT=%0.3f",CREOcean::m_RS.m_StatsTimeFFT);
          crend->WriteXY(cf,550,170, 0.5f,1,1,1,1,1,"UpdateVerts=%0.3f",CREOcean::m_RS.m_StatsTimeUpdateVerts);
          crend->WriteXY(cf,550,185, 0.5f,1,1,1,1,1,"BumpTexUpdate=%0.3f",CREOcean::m_RS.m_StatsTimeTexUpdate);
          crend->WriteXY(cf,550,200, 0.5f,1,1,1,1,1,"RendOcean=%0.3f",CREOcean::m_RS.m_StatsTimeRendOcean);
          crend->WriteXY(cf,550,215, 0.5f,1,1,1,1,1,"NumRendSectors=%d",CREOcean::m_RS.m_StatsNumRendOceanSectors);
          crend->WriteXY(cf,550,230, 0.5f,1,1,1,1,1,"NumAllocatedSectors=%d",CREOcean::m_RS.m_StatsAllocatedSectors);
        }
        break;

      case 3:
        {
          CLeafBuffer *pLB = CLeafBuffer::m_Root.m_Prev;
          int nMem = 0;
          while (pLB != &CLeafBuffer::m_Root)
          {
            nMem += pLB->GetAllocatedBytes(true);
            pLB = pLB->m_Prev;
          }
          crend->WriteXY(cf,550,140, 0.5f,1,1,1,1,1,"LeafBuffer Vert. size=%0.3fMb",(float)nMem/1024.0f/1024.0f);
        }
        break;

      case 4:
        {
          int i;

          int nSize = 0;
          int n = 0;
          for (i=0; i<SShader::m_Shaders_known.GetSize(); i++)
          {
            SShader *pSH = SShader::m_Shaders_known[i];
            if (!pSH)
              continue;
            nSize += pSH->Size(0);
            n++;
          }
          crend->WriteXY(cf,550,160, 0.5f,1,1,1,1,1,"Shaders: %d, size=%0.3fMb",n , (float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          for (i=0; i<gcEf.m_ShaderResources.Num(); i++)
          {
            SRenderShaderResources *pSR = gcEf.m_ShaderResources[i];
            if (!pSR)
              continue;
            nSize += pSR->Size();
            n++;
          }
          crend->WriteXY(cf,550,175, 0.5f,1,1,1,1,1,"Shader Res: %d, size=%0.3fMb",n , (float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          for (i=0; i<SParamComp::m_ParamComps.Num(); i++)
          {
            nSize += SParamComp::m_ParamComps[i]->Size();
            n++;
          }
          crend->WriteXY(cf,550,190, 0.5f,1,1,1,1,1,"Shader Comps: %d,  size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          for (i=0; i<CVProgram::m_VPrograms.Num(); i++)
          {
            nSize += CVProgram::m_VPrograms[i]->Size();
            n++;
          }
          crend->WriteXY(cf,550,205, 0.5f,1,1,1,1,1,"VProgramms: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          for (i=0; i<CPShader::m_PShaders.Num(); i++)
          {
            nSize += CPShader::m_PShaders[i]->Size();
            n++;
          }
          crend->WriteXY(cf,550,220, 0.5f,1,1,1,1,1,"PShaders: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          for (i=0; i<CLightStyle::m_LStyles.Num(); i++)
          {
            nSize += CLightStyle::m_LStyles[i]->Size();
            n++;
          }
          crend->WriteXY(cf,550,235, 0.5f,1,1,1,1,1,"LStyles: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          for (i=0; i<SLightMaterial::known_materials.Num(); i++)
          {
            nSize += SLightMaterial::known_materials[i]->Size();
            n++;
          }
          crend->WriteXY(cf,550,250, 0.5f,1,1,1,1,1,"LMaterials: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          CLeafBuffer *pLB = CLeafBuffer::m_RootGlobal.m_NextGlobal;
          while (pLB != &CLeafBuffer::m_RootGlobal)
          {
            n++;
            nSize += pLB->Size(0);
            pLB = pLB->m_NextGlobal;
          }
          crend->WriteXY(cf,550,280, 0.5f,1,1,1,1,1,"Mesh: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          CRendElement *pRE = CRendElement::m_RootGlobal.m_NextGlobal;
          while (pRE != &CRendElement::m_RootGlobal)
          {
            n++;
            nSize += pRE->Size();
            pRE = pRE->m_NextGlobal;
          }
          crend->WriteXY(cf,550,295, 0.5f,1,1,1,1,1,"Rend. Elements: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          for (i=0; i<CTexMan::m_Textures.Num(); i++)
          {
            STexPic *tp = CTexMan::m_Textures[i];
            if (!tp)
              continue;
            n++;
            nSize += tp->Size(0);
          }
          crend->WriteXY(cf,550,310, 0.5f,1,1,1,1,1,"Textures: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);
        }
        break;
    }
    //crend->WriteXY(cf,55,200, 0.5f,1,1,1,1,1,"Time ftol=%0.4f, Time fistp=%0.4f, Time QRound=%0.4f", timeFtoL, timeFtoI, timeQRound);
  }

  m_TexMan->Update();    

  if (CV_r_profileshaders)
    EF_PrintProfileInfo();

  // draw debug bboxes and lines

	std :: vector<BBoxInfo>::iterator itBBox = m_arrBoxesToDraw.begin(), itBBoxEnd = m_arrBoxesToDraw.end();
  for(; itBBox != itBBoxEnd; ++itBBox)
  {
		BBoxInfo& rBBox = *itBBox;
    SetMaterialColor( rBBox.fColor[0], rBBox.fColor[1],
                      rBBox.fColor[2], rBBox.fColor[3] );

		// set blend for transparent objects
		if(rBBox.fColor[3]!=1.f)
		{
			///glEnable(GL_BLEND);
      EnableBlend(true);
			///glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      SetBlendMode(R_BLEND_MODE__SRC_ALPHA__ONE_MINUS_SRC_ALPHA);
			///glDepthMask(GL_FALSE);
      EnableDepthWrites(false);
		}

		switch(rBBox.nPrimType)
		{
		case DPRIM_LINE:
			///glDisable(GL_TEXTURE_2D);
      m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
      m_pd3dDevice->SetTexture( 0, 0 );

			///glBegin(GL_LINE_LOOP);
			///glVertex3f(rBBox.vMins.x,rBBox.vMins.y,rBBox.vMins.z);
			///glVertex3f(rBBox.vMaxs.x,rBBox.vMaxs.y,rBBox.vMaxs.z);
			///glEnd();

      SPipeVertex_D * pVerts;
#ifdef _XBOX
      HRESULT hr = m_pLineVB->Lock(0, 0, (BYTE **) &pVerts, 0);
#else
      HRESULT hr = m_pLineVB->Lock(0, 0, (BYTE **) &pVerts, D3DLOCK_DISCARD);
#endif

      DWORD dwCol = (DWORD(rBBox.fColor[0] * 255) << 24) |
        (DWORD(rBBox.fColor[1] * 255) << 16) |
        (DWORD(rBBox.fColor[2] * 255) << 8) |
        (DWORD(rBBox.fColor[3] * 255));
      if(hr==S_OK)
      {
        pVerts[0].xyz.x = rBBox.vMins.x;
        pVerts[0].xyz.y = rBBox.vMins.y;
        pVerts[0].xyz.z = rBBox.vMins.z;
        pVerts[0].color.dcolor = dwCol;

        pVerts[1].xyz.x = rBBox.vMaxs.x;
        pVerts[1].xyz.y = rBBox.vMaxs.y;
        pVerts[1].xyz.z = rBBox.vMaxs.z;
        pVerts[1].color.dcolor = dwCol;

        m_pLineVB->Unlock();

        m_pd3dDevice->SetVertexShader( D3DFVF_VERTEX_D );
        m_pd3dDevice->SetStreamSource(0, m_pLineVB, STRIDE_D);
        hr = m_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, 1);
      }

			
      
      ///glEnable(GL_TEXTURE_2D);
      m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
			break;

/*
		case DPRIM_WHIRE_BOX:
			Flush3dBBox(rBBox.vMins, rBBox.vMaxs, false);
			break;

		case DPRIM_SOLID_BOX:
			Flush3dBBox(rBBox.vMins, rBBox.vMaxs, true);
			break;

		case DPRIM_SOLID_SPHERE:
			{
				Vec3d vPos = (rBBox.vMins+rBBox.vMaxs)*0.5f;
				float fRadius = (rBBox.vMins-rBBox.vMaxs).Length();
				DrawBall(vPos, fRadius);
			}
			break;
*/
		}

		if(rBBox.fColor[3]!=1.f)
		{
			///glDisable(GL_BLEND);
      EnableBlend(false);
			///glDepthMask(GL_TRUE);
      EnableDepthWrites(true);
		}
	}

  m_arrBoxesToDraw.clear();

  // print shadow volume stats
  ICVar *pVar = iConsole->GetCVar("e_stencil_shadows");
  if(pVar && pVar->GetIVal()==3)
    CRETriMeshShadow::PrintStats();

  // End the scene
  HRESULT hReturn = m_pd3dDevice->EndScene();

  if (FAILED(hReturn))
  {
    return;
  }

  // Flip the back buffer to the front
  m_pd3dDevice->Present(NULL, NULL, NULL, NULL);

  if (CV_r_GetScreenShot)
  {
    ScreenShot();
    CV_r_GetScreenShot = 0;
  }
}

void CD3D8Renderer::Print(CXFont *currfont,float x, float y, const char *buf,float xscale,float yscale,float r,float g,float b,float a)
{
  // Check for the presence of a D3D device
  ASSERT(m_pd3dDevice);

  if (iConsole && iConsole->GetFont())
  { 
    CXFont    *cf=iConsole->GetFont();
    IFFont *pFont = iSystem->GetICryFont()->GetFont("Default");
    if (pFont)
    {
      pFont->SetColor(cry::color4f(r,g,b,a));
      pFont->SetEffect("buttonfocus");
      pFont->SetSameSize(true);
      pFont->SetCharWidthScale(2.0f/3.0f);
      pFont->SetSize(vector2f(12,16));
      pFont->DrawString(x, y, buf);
    }
  }
}

void CD3D8Renderer::PrintToScreen(float x, float y, float size, const char *buf)
{
  Print(iConsole->GetFont(),0,0,buf,size*0.5f/8, size*1.f/8, 1,1,1,1);
}

void CD3D8Renderer::WriteXY(CXFont *currfont, int x, int y, float xscale, float yscale, float r, float g, float b, float a, const char *message, ...)
{
  //////////////////////////////////////////////////////////////////////
  // Write a string to the screen
  //////////////////////////////////////////////////////////////////////

  va_list ArgList;
  char szStringBuffer[4096];

  // Check for the presence of a D3D device
  ASSERT(m_pd3dDevice);
  
  // Format the string
  va_start(ArgList, message);
  vsprintf(szStringBuffer, message, ArgList);
  va_end(ArgList);

  Print(currfont,(float)(x),(float)(y),szStringBuffer,xscale,yscale,r,g,b,a);
}

void	CD3D8Renderer::Draw2dImage(float xpos,float ypos,float w,float h,int textureid,float s0,float t0,float s1,float t1,float angle,float r,float g,float b,float a,float z)
{
  //////////////////////////////////////////////////////////////////////
  // Draw a textured quad, texture is assumed to be in video memory
  //////////////////////////////////////////////////////////////////////

  // Check for the presence of a D3D device
  assert(m_pd3dDevice);

  assert(m_SceneRecurseCount);

  DWORD col = D3DRGBA(r,g,b,a);
  xpos = (float)(int)ScaleCoordX(xpos); w = (float)(int)ScaleCoordX(w);
  ypos = (float)(int)ScaleCoordY(ypos); h = (float)(int)ScaleCoordY(h);
  
  VOID *pVertices;
  HRESULT hReturn;

  float fx = xpos;
  float fy = ypos;
  float fw = w;
  float fh = h;


  //if (!pID3DTexture)
  //  D3DXCreateTextureFromFile(m_pd3dDevice, "d:\\Textures\\Console\\DefaultConsole.tga", &pID3DTexture);
  
  SetCullMode(R_CULL_DISABLE);
  EnableDepthTest(0);
  EnableTMU(true);

  // Lock the entire buffer and obtain a pointer to the location where we have to
  // write the vertex data. Note that we specify zero here to lock the entire 
  // buffer.

#ifdef _XBOX
  HRESULT hr = m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, 0);
#else
  HRESULT hr = m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, D3DLOCK_DISCARD);
#endif

#ifdef _XBOX
  SPipeTRVertex_D_1T *vQuad = (SPipeTRVertex_D_1T *)pVertices;
#else
  SPipeTRVertex_D_1T *vQuad = (SPipeTRVertex_D_1T *)pVertices;
#endif

  // Now that we have write access to the buffer, we can copy our vertices
  // into it

  // Define the quad
  vQuad[0].dvSX = fx;
  vQuad[0].dvSY = fy;
  vQuad[0].dvSZ = 1.0f;
  vQuad[0].dvRHW = 1.0f;
  vQuad[0].dcColor = col;
  vQuad[0].dvTU[0] = s0;
  vQuad[0].dvTU[1] = 1.0f-t0;
  
  vQuad[1].dvSX = fx + fw;
  vQuad[1].dvSY = fy;
  vQuad[1].dvSZ = 1.0f;
  vQuad[1].dvRHW = 1.0f;
  vQuad[1].dcColor = col;
  vQuad[1].dvTU[0] = s1;
  vQuad[1].dvTU[1] = 1.0f-t0;
  
  vQuad[2].dvSX = fx + fw;
  vQuad[2].dvSY = fy + fh;
  vQuad[2].dvSZ = 1.0f;
  vQuad[2].dvRHW = 1.0f;
  vQuad[2].dcColor = col;
  vQuad[2].dvTU[0] = s1;
  vQuad[2].dvTU[1] = 1.0f-t1;
  
  vQuad[3].dvSX = fx;
  vQuad[3].dvSY = fy + fh;
  vQuad[3].dvSZ = 1.0f;
  vQuad[3].dvRHW = 1.0f;
  vQuad[3].dcColor = col;
  vQuad[3].dvTU[0] = s0;
  vQuad[3].dvTU[1] = 1.0f-t1;

  // We are finished with accessing the vertex buffer
  m_pQuadVB->Unlock();

  // Activate the image texture
  SetTexture(textureid);
  //if (pID3DTexture)
  //  m_pd3dDevice->SetTexture(0, pID3DTexture);
  
  // Bind our vertex as the first data stream of our device
#ifdef _XBOX
  m_pd3dDevice->SetStreamSource(0, m_pQuadVB, STRIDE_TR_D_1T);
  // Set the vertex shader to the FVF fixed function shader
  m_pd3dDevice->SetVertexShader(D3DFVF_TRVERTEX_D_1T);
#else
  m_pd3dDevice->SetStreamSource(0, m_pQuadVB, STRIDE_TR_D_1T);
  // Set the vertex shader to the FVF fixed function shader
  m_pd3dDevice->SetVertexShader(D3DFVF_TRVERTEX_D_1T);
#endif

  // Render the two triangles from the data stream
  hReturn = m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

  if (FAILED(hReturn))
  {
    ASSERT(hReturn);
    return;
  }

  EnableDepthTest(1);
}

void	CD3D8Renderer::DrawImage(float xpos,float ypos,float w,float h,int texture_id,float s0,float t0,float s1,float t1,float r,float g,float b,float a)
{
  //////////////////////////////////////////////////////////////////////
  // Draw a textured quad, texture is assumed to be in video memory
  //////////////////////////////////////////////////////////////////////

  // Check for the presence of a D3D device
  assert(m_pd3dDevice);

  assert(m_SceneRecurseCount);

  xpos = (float)(int)ScaleCoordX(xpos); w = (float)(int)ScaleCoordX(w);
  ypos = (float)(int)ScaleCoordY(ypos); h = (float)(int)ScaleCoordY(h);
  
  VOID *pVertices;
  HRESULT hReturn;

  float fx = xpos;
  float fy = ypos;
  float fw = w;
  float fh = h;


  //if (!pID3DTexture)
  //  D3DXCreateTextureFromFile(m_pd3dDevice, "d:\\Textures\\Console\\DefaultConsole.tga", &pID3DTexture);
  
  SetCullMode(R_CULL_DISABLE);
  EnableDepthTest(0);
  EnableTMU(true);

  // Lock the entire buffer and obtain a pointer to the location where we have to
  // write the vertex data. Note that we specify zero here to lock the entire 
  // buffer.
#ifdef _XBOX
  HRESULT hr = m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, 0);
#else
  HRESULT hr = m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, D3DLOCK_DISCARD);
#endif

#ifdef _XBOX
  SPipeTRVertex_D_1T *vQuad = (SPipeTRVertex_D_1T *)pVertices;
#else
  SPipeTRVertex_D_1T *vQuad = (SPipeTRVertex_D_1T *)pVertices;
#endif

	if (a<0)	
	{
		// always blend over the screen
    EnableBlend(true);
    m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
    m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
		a = 1.0f;
	}
  DWORD col = D3DRGBA(r,g,b,a);

  // Now that we have write access to the buffer, we can copy our vertices
  // into it

  // Define the quad
  vQuad[0].dvSX = xpos;
  vQuad[0].dvSY = ypos;
  vQuad[0].dvSZ = 1.0f;
  vQuad[0].dvRHW = 1.0f;
  vQuad[0].dcColor = col;
  vQuad[0].dvTU[0] = s0;
  vQuad[0].dvTU[1] = 1.0f-t0;
  
  vQuad[1].dvSX = xpos + w;
  vQuad[1].dvSY = ypos;
  vQuad[1].dvSZ = 1.0f;
  vQuad[1].dvRHW = 1.0f;
  vQuad[1].dcColor = col;
  vQuad[1].dvTU[0] = s1;
  vQuad[1].dvTU[1] = 1.0f-t0;
  
  vQuad[2].dvSX = xpos + w;
  vQuad[2].dvSY = ypos + h;
  vQuad[2].dvSZ = 1.0f;
  vQuad[2].dvRHW = 1.0f;
  vQuad[2].dcColor = col;
  vQuad[2].dvTU[0] = s1;
  vQuad[2].dvTU[1] = 1.0f-t1;
  
  vQuad[3].dvSX = xpos;
  vQuad[3].dvSY = ypos + h;
  vQuad[3].dvSZ = 1.0f;
  vQuad[3].dvRHW = 1.0f;
  vQuad[3].dcColor = col;
  vQuad[3].dvTU[0] = s0;
  vQuad[3].dvTU[1] = 1.0f-t1;

  // We are finished with accessing the vertex buffer
  m_pQuadVB->Unlock();

  // Activate the image texture
  SetTexture(texture_id);
  //if (pID3DTexture)
  //  m_pd3dDevice->SetTexture(0, pID3DTexture);
  
  // Bind our vertex as the first data stream of our device
#ifdef _XBOX
  m_pd3dDevice->SetStreamSource(0, m_pQuadVB, STRIDE_TR_D_1T);
  // Set the vertex shader to the FVF fixed function shader
  m_pd3dDevice->SetVertexShader(D3DFVF_TRVERTEX_D_1T);
#else
  m_pd3dDevice->SetStreamSource(0, m_pQuadVB, STRIDE_TR_D_1T);
  // Set the vertex shader to the FVF fixed function shader
  m_pd3dDevice->SetVertexShader(D3DFVF_TRVERTEX_D_1T);
#endif

  // Render the two triangles from the data stream
  hReturn = m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

  if (FAILED(hReturn))
  {
    ASSERT(hReturn);
    return;
  }

  EnableDepthTest(1);
}

void CD3D8Renderer::Draw2dLine(float x1, float y1, float x2, float y2)
{
  SetCullMode(R_CULL_DISABLE);
  EnableDepthTest(0);
  EnableTMU(false);

  float x1pos=(float)(int)ScaleCoordX(x1);
  float y1pos=(float)(int)ScaleCoordY(y1);
  float x2pos=(float)(int)ScaleCoordX(x2);
  float y2pos=(float)(int)ScaleCoordY(y2);

  VOID *pVertices;

  // Lock the entire buffer and obtain a pointer to the location where we have to
  // write the vertex data. Note that we specify zero here to lock the entire 
  // buffer.
#ifdef _XBOX
  HRESULT hr = m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, 0);
#else
  HRESULT hr = m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, D3DLOCK_DISCARD);
#endif

  SPipeTRVertex_D_1T *vQuad = (SPipeTRVertex_D_1T *)pVertices;

  // Define the line
  vQuad[0].dvSX = x1pos;
  vQuad[0].dvSY = y1pos;
  vQuad[0].dvSZ = 1.0f;
  vQuad[0].dvRHW = 1.0f;
  vQuad[0].dcColor = 0;

  vQuad[1].dvSX = x2pos;
  vQuad[1].dvSY = y2pos;
  vQuad[1].dvSZ = 1.0f;
  vQuad[1].dvRHW = 1.0f;
  vQuad[1].dcColor = 0;

  // We are finished with accessing the vertex buffer
  m_pQuadVB->Unlock();

  // Bind our vertex as the first data stream of our device
  m_pd3dDevice->SetStreamSource(0, m_pQuadVB, STRIDE_TR_D_1T);
  // Set the vertex shader to the FVF fixed function shader
  m_pd3dDevice->SetVertexShader(D3DFVF_TRVERTEX_D_1T);

  // Render the two triangles from the data stream
  hr = m_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, 1);

  if (FAILED(hr))
  {
    ASSERT(hr);
    return;
  }
  EnableTMU(true);
}

void CD3D8Renderer::DrawPoints(Vec3d v[], int nump, CFColor& col, int flags)
{
}

void CD3D8Renderer::DrawLines(Vec3d v[], int nump, CFColor& col, int flags)
{
}

void CD3D8Renderer::DrawLine(const Vec3d & vPos1, const Vec3d & vPos2)
{
  float fX1 = (float)(int)ScaleCoordX(vPos1.x);
  float fY1 = (float)(int)ScaleCoordY(vPos1.y);
  float fX2 = (float)(int)ScaleCoordX(vPos2.x);
  float fY2 = (float)(int)ScaleCoordY(vPos2.y);

  SetCullMode(R_CULL_DISABLE);
  EnableDepthTest(0);
  EnableTMU(false);

  VOID *pVertices;

  // Lock the entire buffer and obtain a pointer to the location where we have to
  // write the vertex data. Note that we specify zero here to lock the entire 
  // buffer.
#ifdef _XBOX
  HRESULT hr = m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, 0);
#else
  HRESULT hr = m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, D3DLOCK_DISCARD);
#endif

  SPipeTRVertex_D_1T *vQuad = (SPipeTRVertex_D_1T *)pVertices;

  // Define the line
  vQuad[0].dvSX = fX1;
  vQuad[0].dvSY = fY1;
  vQuad[0].dvSZ = vPos1.z;
  vQuad[0].dvRHW = 1.0f;
  vQuad[0].dcColor = 0xffffffff;

  vQuad[1].dvSX = fX2;
  vQuad[1].dvSY = fY2;
  vQuad[1].dvSZ = vPos2.z;
  vQuad[1].dvRHW = 1.0f;
  vQuad[1].dcColor = 0xffffffff;

  // We are finished with accessing the vertex buffer
  m_pQuadVB->Unlock();

  // Bind our vertex as the first data stream of our device
  m_pd3dDevice->SetStreamSource(0, m_pQuadVB, STRIDE_TR_D_1T);
  // Set the vertex shader to the FVF fixed function shader
  m_pd3dDevice->SetVertexShader(D3DFVF_TRVERTEX_D_1T);

  // Render the two triangles from the data stream

  m_pd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_SELECTARG1);
  m_pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TFACTOR );

  hr = m_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, 1);

  m_pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE );

  if (FAILED(hr))
  {
    ASSERT(hr);
    return;
  }

  EnableTMU(true);
}

void CD3D8Renderer::DrawLineColor(const Vec3d & vPos1, const CFColor & vColor1, const Vec3d & vPos2, const CFColor & vColor2)
{
  float fX1 = (float)(int)ScaleCoordX(vPos1.x);
  float fY1 = (float)(int)ScaleCoordY(vPos1.y);
  float fX2 = (float)(int)ScaleCoordX(vPos2.x);
  float fY2 = (float)(int)ScaleCoordY(vPos2.y);

  EnableTMU(false);

  VOID *pVertices;

  // Lock the entire buffer and obtain a pointer to the location where we have to
  // write the vertex data. Note that we specify zero here to lock the entire 
  // buffer.
#ifdef _XBOX
  HRESULT hr = m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, 0);
#else
  HRESULT hr = m_pQuadVB->Lock(0, 0, (BYTE **) &pVertices, D3DLOCK_DISCARD);
#endif

  SPipeTRVertex_D_1T *vQuad = (SPipeTRVertex_D_1T *)pVertices;

  DWORD col1 = D3DRGBA(vColor1[0],vColor1[1],vColor1[2],1.0f);
  DWORD col2 = D3DRGBA(vColor2[0],vColor2[1],vColor2[2],1.0f);

  // Define the line
  vQuad[0].dvSX = fX1;
  vQuad[0].dvSY = fY1;
  vQuad[0].dvSZ = vPos1.z;
  vQuad[0].dvRHW = 1.0f;
  vQuad[0].dcColor = col1;

  vQuad[1].dvSX = fX2;
  vQuad[1].dvSY = fY2;
  vQuad[1].dvSZ = vPos2.z;
  vQuad[1].dvRHW = 1.0f;
  vQuad[1].dcColor = col2;

  // We are finished with accessing the vertex buffer
  m_pQuadVB->Unlock();

  // Bind our vertex as the first data stream of our device
  m_pd3dDevice->SetStreamSource(0, m_pQuadVB, STRIDE_TR_D_1T);
  // Set the vertex shader to the FVF fixed function shader
  m_pd3dDevice->SetVertexShader(D3DFVF_TRVERTEX_D_1T);

  // Render the two triangles from the data stream
  hr = m_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, 1);

  if (FAILED(hr))
  {
    ASSERT(hr);
    return;
  }

  EnableTMU(true);
}

//*********************************************************************

void CD3D8Renderer::GetModelViewMatrix(float * mat)
{
  memcpy(mat, (D3DXMATRIX *)m_matView->GetTop(), 4*4*sizeof(float));
}

void CD3D8Renderer::GetProjectionMatrix(float * mat)
{
  memcpy(mat, (D3DXMATRIX *)m_matProj->GetTop(), 4*4*sizeof(float));
}

///////////////////////////////////////////
void CD3D8Renderer::PushMatrix()
{
  ASSERT(m_pd3dDevice);

  EF_PushMatrix();
}

///////////////////////////////////////////
void CD3D8Renderer::PopMatrix()
{
  ASSERT(m_pd3dDevice);

  EF_PopMatrix();
}

///////////////////////////////////////////
void CD3D8Renderer::RotateMatrix(float a,float x,float y,float z)
{
  ASSERT(m_pd3dDevice);

  D3DXVECTOR3 v;
  v[0] = x;
  v[1] = y;
  v[2] = z;
  m_matView->RotateAxisLocal(&v, a * PI/180.0f);
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop());
  m_bInvertedMatrix = false;
}

void CD3D8Renderer::LoadMatrix(const Matrix *src)
{
  if(src)
	  m_matView->LoadMatrix((D3DXMATRIX *)src);
  else
    m_matView->LoadIdentity();
  m_bInvertedMatrix = false;
}

void CD3D8Renderer::RotateMatrix(const Vec3d & angles)
{
  D3DXMATRIX mat;
  D3DXMATRIX *m = m_matView->GetTop();
  D3DXMatrixRotationZ(&mat, angles.z * PI/180);    
  m_matView->MultMatrixLocal(&mat);   
  D3DXMatrixRotationY(&mat, angles.y * PI/180);    
  m_matView->MultMatrixLocal(&mat);   
  D3DXMatrixRotationX(&mat, angles.x * PI/180);    
  m_matView->MultMatrixLocal(&mat);   
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop());
  m_bInvertedMatrix = false;
}

void CD3D8Renderer::MultMatrix(float * mat)
{
  m_matView->MultMatrixLocal((D3DXMATRIX *)mat);   
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop()); 
  m_bInvertedMatrix = false;
}

///////////////////////////////////////////
void CD3D8Renderer::ScaleMatrix(float x,float y,float z)
{
  ASSERT(m_pd3dDevice);

  m_matView->ScaleLocal(x, y, z);   
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop()); 
  m_bInvertedMatrix = false;
}

///////////////////////////////////////////
void CD3D8Renderer::TranslateMatrix(float x,float y,float z)
{
  ASSERT(m_pd3dDevice);

  m_matView->TranslateLocal(x, y, z);   
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop()); 
  m_bInvertedMatrix = false;
}

void CD3D8Renderer::TranslateMatrix(const Vec3d &pos)
{
  ASSERT(m_pd3dDevice);

  float x = pos[0];
  float y = pos[1];
  float z = pos[2];
  m_matView->TranslateLocal(x,y,z);   
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop()); 
  m_bInvertedMatrix = false;
}

void CD3D8Renderer::SetPerspective(const CCamera &cam)
{
  D3DXMATRIX *m = m_matProj->GetTop();
  D3DXMatrixPerspectiveFovRH(m, cam.GetFov()*cam.GetProjRatio(), 1.0f/cam.GetProjRatio(), cam.GetZMin(), cam.GetZMax());
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
}

void CD3D8Renderer::SetCamera(const CCamera &cam)
{
  ASSERT(m_pd3dDevice);

  Matrix mat = cam.GetMatrix();
  D3DXMATRIX *m = m_matView->GetTop();
  m_matView->LoadMatrix((D3DXMATRIX *)&mat);
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
  D3DXMatrixInverse(&m_matViewInv, NULL, m);
  m = m_matProj->GetTop();
	D3DXMatrixPerspectiveFovRH(m, cam.GetFov()*cam.GetProjRatio(), 1.0f/cam.GetProjRatio(), cam.GetZMin(), cam.GetZMax());
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
  m_bInvertedMatrix = false;

	m_cam = cam;
}

void CD3D8Renderer::SetViewport(int x, int y, int width, int height)
{
  if (!x && !y && !width && !height)
  {
    m_Viewport.X = m_VX;
    m_Viewport.Y = m_VY;
    m_Viewport.Width = m_VWidth;
    m_Viewport.Height = m_VHeight;
    m_Viewport.MinZ       = 0.0;
    m_Viewport.MaxZ       = 1.0;

    m_pd3dDevice->SetViewport(&m_Viewport);
    return;
  }

  m_VX = x;
  m_VY = y;
  m_VWidth = width;
  m_VHeight = height;

  m_Viewport.X = m_VX;
  m_Viewport.Y = m_VY;
  m_Viewport.Width = m_VWidth;
  m_Viewport.Height = m_VHeight;
  m_Viewport.MinZ       = 0.0;
  m_Viewport.MaxZ       = 1.0;
  
  m_pd3dDevice->SetViewport(&m_Viewport);
}

void CD3D8Renderer::SetScissor(int x, int y, int width, int height)
{
  /*D3DCLIPSTATUS8 Clip;
  Clip.ClipUnion = D3DCS_BOTTOM | D3DCS_TOP | D3DCS_LEFT | D3DCS_RIGHT;
  Clip.ClipIntersection = Clip.ClipUnion;
  if (FAILED(h=m_pd3dDevice->SetClipStatus(&Clip)))
  {
    Error("SetClipStatus failed\n",h);
    return;
  }*/
}

void CD3D8Renderer::Draw3dBBox(const Vec3d &mins,const Vec3d &maxs, int nPrimType)
{
	Draw3dPrim (mins, maxs, nPrimType);
}

void CD3D8Renderer::Draw3dPrim(const Vec3d &mins,const Vec3d &maxs, int nPrimType, const float* fRGBA)
{
	BBoxInfo info;
	info.vMins = mins;
	info.vMaxs = maxs;
	info.nPrimType = nPrimType;
	//glGetFloatv(GL_CURRENT_COLOR, (float*)info.fColor);
	if (fRGBA)
	{
		for (int i = 0;i < 4; ++i)
			info.fColor[i] = fRGBA[i];
	}
	else
	{
		for (int i = 0;i < 4; ++i)
			info.fColor[i] = 1.0f;
	}
	m_arrBoxesToDraw.push_back(info);
}

/*
void CD3D8Renderer::Draw3dBBox(const Vec3d &mins,const Vec3d &maxs, int nPrimType)
{
  //////////////////////////////////////////////////////////////////////
  // Draw an AABB in wireframe mode
  //
  // TODO: Box creation
  //////////////////////////////////////////////////////////////////////

	//assert(!bDrawLine); // draw line is not supported in dx

  D3DXMATRIX *m = m_matView->GetTop();

  ID3DXMesh *pID3DBoxMesh = NULL;

  ASSERT(m_pd3dDevice);

  // Create the box
  D3DXCreateBox(m_pd3dDevice, 10, 10, 10, &pID3DBoxMesh, NULL);

  // Switch into wireframe mode
  //m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

  // Render it
  pID3DBoxMesh->DrawSubset(0);

  // Switch out of wireframe mode
  //m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    
  // Release the mesh
  DX_RELEASE(pID3DBoxMesh)
}
*/

void CD3D8Renderer::SetCullMode(int mode)
{
  //////////////////////////////////////////////////////////////////////
  // Change the culling mode
  //////////////////////////////////////////////////////////////////////

  ASSERT(m_pd3dDevice);

  switch (mode)
  {
    case R_CULL_DISABLE:
      m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
      break;
    case R_CULL_BACK:
      m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
      break;
    case R_CULL_FRONT:
      m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
      break;
  }
}

void CD3D8Renderer::EnableBlend(bool enable)
{
  //////////////////////////////////////////////////////////////////////
  // Enable or disable blending
  //////////////////////////////////////////////////////////////////////

  ASSERT(m_pd3dDevice);

  m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, enable);
}

void CD3D8Renderer::SetBlendMode(int mode)
{
  //////////////////////////////////////////////////////////////////////
  // Set D3D Blending mode
  //////////////////////////////////////////////////////////////////////

  ASSERT(m_pd3dDevice);

  int src, dst;

  switch (mode)
  {
    case R_BLEND_MODE__ZERO__SRC_COLOR:
      src = D3DBLEND_ZERO;
      dst = D3DBLEND_SRCCOLOR;
      break;
    case R_BLEND_MODE__SRC_COLOR__ZERO:
      src = D3DBLEND_SRCCOLOR;
      dst = D3DBLEND_ZERO;
      break;
    case R_BLEND_MODE__SRC_COLOR__ONE_MINUS_SRC_COLOR:
      src = D3DBLEND_SRCCOLOR;
      dst = D3DBLEND_INVSRCCOLOR;
      break;
    case R_BLEND_MODE__SRC_ALPHA__ONE_MINUS_SRC_ALPHA:
      src = D3DBLEND_SRCALPHA;
      dst = D3DBLEND_INVSRCALPHA;
      break;
    case R_BLEND_MODE__SRC_ALPHA__ONE:
      src = D3DBLEND_SRCALPHA;
      dst = D3DBLEND_ONE;
      break;
    case R_BLEND_MODE__ONE__ONE:
      src = D3DBLEND_ONE;
      dst = D3DBLEND_ONE;
      break;
    case R_BLEND_MODE__DST_COLOR__SRC_COLOR:
      src = D3DBLEND_DESTCOLOR;
      dst = D3DBLEND_SRCCOLOR;
      break;
    case R_BLEND_MODE__ZERO__ONE_MINUS_SRC_COLOR:
      src = D3DBLEND_ZERO;
      dst = D3DBLEND_INVSRCCOLOR;
      break;
    case R_BLEND_MODE__ONE__ONE_MINUS_SRC_COLOR:
      src = D3DBLEND_ONE;
      dst = D3DBLEND_INVSRCCOLOR;
      break;
    case R_BLEND_MODE__ONE__ONE_MINUS_SRC_ALPHA:
      src = D3DBLEND_ONE;
      dst = D3DBLEND_INVSRCALPHA;
      break;
    case R_BLEND_MODE__DST_ALPHA__ONE_MINUS_DST_ALPHA:
      src = D3DBLEND_DESTALPHA;
      dst = D3DBLEND_INVDESTALPHA;
      break;
    case R_BLEND_MODE__ONE__ZERO:
      src = D3DBLEND_ONE;
      dst = D3DBLEND_INVDESTALPHA;
      break;
    case R_BLEND_MODE__ZERO__ZERO:
      src = D3DBLEND_ZERO;
      dst = D3DBLEND_ZERO;
      break;
    case R_BLEND_MODE__ADD_SIGNED:
      src = D3DBLEND_DESTCOLOR;
      dst = D3DBLEND_SRCCOLOR;
      EF_SetColorOp(eCO_BLENDDIFFUSEALPHA);
      break;
    default:
      iLog->Log("Unknown blend mode %d\n", mode);
      return;
  }

  m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,  src);
  m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, dst);
}

void CD3D8Renderer::SetFog(float density, float fogstart, float fogend, const float *color, int fogmode)
{
  //////////////////////////////////////////////////////////////////////
  // Configure the fog settings
  //////////////////////////////////////////////////////////////////////

  D3DCOLOR dwColor;

  ASSERT(m_pd3dDevice);

#ifndef _XBOX
  m_pd3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE,	D3DFOG_NONE);
#endif

  m_FS.m_FogDensity = density;
  m_FS.m_FogStart = fogstart;
  m_FS.m_FogEnd = fogend;
  m_FS.m_nFogMode = fogmode;
  CFColor col = CFColor(color[0], color[1], color[2], 1.0f);
  if (m_bHeatVision)
    col = CFColor(0.0f, 0.0f, 0.0f, 1.0f);
  m_FS.m_FogColor = col;
    
  // Fog color
  dwColor = D3DRGBA(color[0],color[1],color[2],color[3]);
  m_pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, dwColor);

  //fogstart /= m_cam.GetZMax();
  //fogend /= m_cam.GetZMax();

  // Fog start and end
  m_pd3dDevice->SetRenderState(D3DRS_FOGSTART, *((LPDWORD) (&fogstart)));
  m_pd3dDevice->SetRenderState(D3DRS_FOGEND, *((LPDWORD) (&fogend)));

  // Fog density
  m_pd3dDevice->SetRenderState(D3DRS_FOGDENSITY, *((LPDWORD) (&density)));
  
  // Fog mode
  switch (fogmode)
  {
    case R_FOGMODE_LINEAR:
      m_pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
      break;

    case R_FOGMODE_EXP2:
      m_pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP2);
      break;

    default:
      // Invalid mode
      // Disable
      m_pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
      break;
  }
}

void CD3D8Renderer::SetFogColor(float * color)
{
  m_FS.m_FogColor = CFColor(color);
  D3DCOLOR dwColor;
  
  // Fog color
  dwColor = D3DRGBA(color[0],color[1],color[2],color[3]);
  m_pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, dwColor);
}

bool CD3D8Renderer::EnableFog(bool enable)
{
  //////////////////////////////////////////////////////////////////////
  // Enable or disable fog
  //////////////////////////////////////////////////////////////////////

  ASSERT(m_pd3dDevice);

  bool bPrevFog = m_FS.m_bEnable; // remember fog value
  m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, enable);

  m_FS.m_bEnable = enable;
  return bPrevFog;
}

///////////////////////////////////////////
void CD3D8Renderer::EnableTexGen(bool enable)
{
  if (enable)
    m_pd3dDevice->SetTextureStageState( m_TexMan->m_CurStage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | m_TexMan->m_CurStage);
  else
  {
    D3DXMATRIX mat;
    D3DXMatrixIdentity(&mat);
    m_pd3dDevice->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+m_TexMan->m_CurStage), &mat );
    m_pd3dDevice->SetTextureStageState( m_TexMan->m_CurStage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | m_TexMan->m_CurStage);
    m_pd3dDevice->SetTextureStageState( m_TexMan->m_CurStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
  }
}

void CD3D8Renderer::SetTexgen3D(float x1, float y1, float z1, float x2, float y2, float z2)
{
  ASSERT(m_pd3dDevice);

  D3DXMATRIX mat, *mi;

  mat._11 = x1;   mat._12 = x2; mat._13 = 0.0f; mat._14 = 0.0f; 
  mat._21 = y1;   mat._22 = y2; mat._23 = 0.0f; mat._24 = 0.0f; 
  mat._31 = z1;   mat._32 = z2; mat._33 = 0.0f; mat._34 = 0.0f; 
  mat._41 = 0;    mat._42 = 0;  mat._43 = 0.0f; mat._44 = 1.0f; 

  mi = EF_InverseMatrix();
  D3DXMatrixMultiply(&mat, mi, &mat);

  m_pd3dDevice->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+m_TexMan->m_CurStage), &mat );
  m_pd3dDevice->SetTextureStageState( m_TexMan->m_CurStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
  m_pd3dDevice->SetTextureStageState( m_TexMan->m_CurStage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | m_TexMan->m_CurStage);
}

void CD3D8Renderer::SetTexgen(float scaleX, float scaleY,float translateX,float translateY)
{        
  //////////////////////////////////////////////////////////////////////
  //
  //////////////////////////////////////////////////////////////////////
  ASSERT(m_pd3dDevice);

  D3DXMATRIX mat, *mi;

  mat._11 = 0.0f; mat._12 = scaleX; mat._13 = 0.0f; mat._14 = 0.0f; 
  mat._21 = scaleY;   mat._22 = 0.0f; mat._23 = 0.0f; mat._24 = 0.0f; 
  mat._31 = 0.0f;   mat._32 = 0.0f; mat._33 = 1.0f; mat._34 = 0.0f; 
  mat._41 = translateY; mat._42 = translateX; mat._43 = 0.0f; mat._44 = 1.0f; 

  mi = EF_InverseMatrix();
  D3DXMatrixMultiply(&mat, mi, &mat);

  m_pd3dDevice->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+m_TexMan->m_CurStage), &mat );
  m_pd3dDevice->SetTextureStageState( m_TexMan->m_CurStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
  m_pd3dDevice->SetTextureStageState( m_TexMan->m_CurStage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | m_TexMan->m_CurStage);
}

void CD3D8Renderer::SetSphericalTexgen()
{
  assert(0);
}

void CD3D8Renderer::SetLodBias(float value)
{
  //////////////////////////////////////////////////////////////////////
  // Set the mip-map LOD bias
  //////////////////////////////////////////////////////////////////////

  ASSERT(m_pd3dDevice);

  value = -value;
  m_pd3dDevice->SetTextureStageState(m_TexMan->m_CurStage, D3DTSS_MIPMAPLODBIAS, *((LPDWORD)(&value)));
}

void CD3D8Renderer::SelectTMU(int tnum)
{
  m_TexMan->m_CurStage = tnum;
}

void CD3D8Renderer::EnableTMU(bool enable)
{
  ASSERT(m_pd3dDevice);

  EF_SetColorOp((enable ? eCO_MODULATE : eCO_DISABLE));
  if (!enable)
  {
    m_pd3dDevice->SetTexture(m_TexMan->m_CurStage, NULL);
    mStages[m_TexMan->m_CurStage].Texture = NULL;
  }
}

void CD3D8Renderer::SetEnviMode(int mode)
{
  //////////////////////////////////////////////////////////////////////
  // Change the textrue environment operation
  //////////////////////////////////////////////////////////////////////

  ASSERT(m_pd3dDevice);

  switch (mode)
  {
  case R_MODE_DECAL:
    EF_SetColorOp(eCO_DECAL);
    break;

  case R_MODE_MODULATE:
    EF_SetColorOp(eCO_MODULATE);
    break;

  case R_MODE_BLEND:
    EF_SetColorOp(eCO_BLENDTEXTUREALPHA);
    break;

  case R_MODE_ADD_SIGNED:
    EF_SetColorOp(eCO_ADDSIGNED);
    break;

  default:
    // Invalid mode
    iLog->Log("Unknown ColorOp %d\n", mode);
    break;
  }
}

///////////////////////////////////////////

void CD3D8Renderer::CreateIndexBuffer(SVertexStream *dest,const void *src,int indexcount)
{
  if (dest->m_VertBuf.m_pPtr)
  {
    IDirect3DIndexBuffer8 *pIndBuf = (IDirect3DIndexBuffer8 *)dest->m_VertBuf.m_pPtr;
    SAFE_RELEASE(pIndBuf);
  }
  dest->m_nItems = 0;
  if (indexcount)
  {
    IDirect3DIndexBuffer8 *ibuf=NULL;
    int size = indexcount*sizeof(ushort);
    int flags = D3DUSAGE_WRITEONLY;
    D3DPOOL Pool = D3DPOOL_MANAGED;
    if (dest->m_bDynamic)
    {
      flags |= D3DUSAGE_DYNAMIC;
      Pool = D3DPOOL_DEFAULT;
    }
    HRESULT hReturn = m_pd3dDevice->CreateIndexBuffer(size, flags, D3DFMT_INDEX16, Pool, &ibuf);

    if (FAILED(hReturn))
    {
      iLog->Log("Failed to create index buffer\n");
      return;
    }
    dest->m_VertBuf.m_pPtr = ibuf;
    dest->m_nItems = indexcount;
  }
  if (src && indexcount)
    UpdateIndexBuffer(dest, src, indexcount, true);
}
void CD3D8Renderer::UpdateIndexBuffer(SVertexStream *dest,const void *src,int indexcount, bool bUnLock)
{
  HRESULT hReturn;
  IDirect3DIndexBuffer8 *ibuf;
  if (src && indexcount)
  {
    if (dest->m_nItems < indexcount)
    {
      if (dest->m_nItems)
        ReleaseIndexBuffer(dest);
      CreateIndexBuffer(dest, NULL, indexcount);
    }
    ushort *dst;
    ibuf = (IDirect3DIndexBuffer8 *)dest->m_VertBuf.m_pPtr;
    hReturn = ibuf->Lock(0, 0, (BYTE **) &dst, 0);
    int size = indexcount*sizeof(ushort);
    cryMemcpy(dst, src, size);
    dest->m_VData = dst;
    if (bUnLock)
    {
      hReturn = ibuf->Unlock();
      dest->m_bLocked = false;
    }
    else
      dest->m_bLocked = true;
  }
  else
  if( dest->m_VertBuf.m_pPtr )
  {
    if (bUnLock && dest->m_bLocked)
    {
      ibuf = (IDirect3DIndexBuffer8 *)dest->m_VertBuf.m_pPtr;
      hReturn = ibuf->Unlock();
      dest->m_bLocked = false;
    }
    else
    if (!bUnLock && !dest->m_bLocked)
    {
      ibuf = (IDirect3DIndexBuffer8 *)dest->m_VertBuf.m_pPtr;
      ushort *dst;
      hReturn = ibuf->Lock(0, 0, (BYTE **) &dst, 0);
      dest->m_bLocked = true;
      dest->m_VData = dst;
    }
  }
}
void CD3D8Renderer::ReleaseIndexBuffer(SVertexStream *dest)
{
  IDirect3DIndexBuffer8 *ibuf = (IDirect3DIndexBuffer8 *)dest->m_VertBuf.m_pPtr;
  SAFE_RELEASE(ibuf);
  dest->Reset();
}


void CD3D8Renderer::CreateBuffer(int size, int vertexformat, CVertexBuffer *buf, int Type, const char *szSource)
{
  IDirect3DVertexBuffer8 *vptr = NULL;
  int fvf = m_RP.m_D3DFixedPipeline[Type][vertexformat+16].m_Handle;

  int Flags = D3DUSAGE_WRITEONLY;
  D3DPOOL Pool = D3DPOOL_MANAGED;
  if (buf->m_bDynamic)
  {
    Flags |= D3DUSAGE_DYNAMIC;
    Pool = D3DPOOL_DEFAULT;
  }

  HRESULT hReturn = m_pd3dDevice->CreateVertexBuffer(size, Flags, fvf, Pool, &vptr);

  if (FAILED(hReturn))
    return;

  void *dst;
  buf->m_VS[Type].m_VertBuf.m_pPtr = vptr;
  hReturn = vptr->Lock(0, 0, (BYTE **) &dst, 0);
  buf->m_VS[Type].m_VData = dst;
  hReturn = vptr->Unlock();

  m_CurVertBufferSize += size;
}

CVertexBuffer *CD3D8Renderer::CreateBuffer(int vertexcount,int vertexformat, const char *szSource, bool bDynamic)
{
  IDirect3DVertexBuffer8 *vptr = NULL;
  int fvf = m_RP.m_D3DFixedPipeline[0][vertexformat+16].m_Handle;

  int Flags = D3DUSAGE_WRITEONLY;
  D3DPOOL Pool = D3DPOOL_MANAGED;
  if (bDynamic)
  {
    Flags |= D3DUSAGE_DYNAMIC;
    Pool = D3DPOOL_DEFAULT;
  }
  int size = m_VertexSize[vertexformat]*vertexcount;
  if (size+m_CurVertBufferSize > m_MaxVertBufferSize)
  {
    CLeafBuffer *pLB = CLeafBuffer::m_Root.m_Prev;
    while (size+m_CurVertBufferSize > m_MaxVertBufferSize)
    {
      if (pLB == &CLeafBuffer::m_Root)
        iConsole->Exit("Error: Pipeline buffer overflow. Current geometry is too-oo-oo big (%s)", gRenDev->GetStatusText(eRS_VidBuffer));
      
      CLeafBuffer *Next = pLB->m_Prev;
      pLB->Unload();
      pLB = Next;
    }
  }

  HRESULT hReturn = m_pd3dDevice->CreateVertexBuffer(m_VertexSize[vertexformat]*vertexcount, Flags, fvf, Pool, &vptr);

  if (FAILED(hReturn))
    return (NULL);

  m_CurVertBufferSize += m_VertexSize[vertexformat]*vertexcount;

  CVertexBuffer *newbuf = new CVertexBuffer;
  newbuf->m_bDynamic = bDynamic;
  newbuf->m_VS[VSF_GENERAL].m_bLocked = false;
  newbuf->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr = vptr;
  newbuf->m_fence=0;
  newbuf->m_NumVerts = vertexcount;
  newbuf->m_vertexformat = vertexformat;

  return(newbuf);
}

#include "../Common/NvTriStrip/NVTriStrip.h"

///////////////////////////////////////////
void CD3D8Renderer::DrawBuffer(CVertexBuffer *src,SVertexStream *indicies,int numindices,int offsindex, int prmode,int vert_start,int vert_stop, CMatInfo *mi)
{
  int size = numindices * sizeof(short);

  if (src->m_VS[VSF_GENERAL].m_bLocked)
  {
    IDirect3DVertexBuffer8 *tvert =  (IDirect3DVertexBuffer8 *)src->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr;
    tvert->Unlock();
    src->m_VS[VSF_GENERAL].m_bLocked = false;
  }
  IDirect3DIndexBuffer8 *ibuf = (IDirect3DIndexBuffer8 *)indicies->m_VertBuf.m_pPtr;
  assert(ibuf);
  h = m_pd3dDevice->SetVertexShader(m_RP.m_D3DFixedPipeline[0][src->m_vertexformat+0x10].m_Handle);
  h = m_pd3dDevice->SetStreamSource( 0, (IDirect3DVertexBuffer8 *)src->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr, m_VertexSize[src->m_vertexformat]);
  h = m_pd3dDevice->SetIndices(ibuf, 0);

  int NumVerts = src->m_NumVerts;
  if (vert_stop)
    NumVerts = vert_stop;

  switch(prmode)
  {
    case R_PRIMV_TRIANGLES:
      h = m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,NumVerts,offsindex,numindices/3);
      m_nPolygons+=numindices/3;
      break;

    case R_PRIMV_TRIANGLE_STRIP:
      h = m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,0,NumVerts,offsindex,numindices-2);
      m_nPolygons+=numindices-2;
      break;

    case R_PRIMV_MULTI_GROUPS:
      {
        if (mi)
        {
          int offs = mi->nFirstIndexId;
          int nGroups = mi->m_dwNumSections;
          SPrimitiveGroup *gr = mi->m_pPrimitiveGroups;
          if (gr)
          {
            for (int i=0; i<nGroups; i++)
            {
              SPrimitiveGroup *g = &gr[i];
              switch (g->type)
              {
                case PT_STRIP:
                  if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, mi->nNumVerts, g->offsIndex+offs, g->numIndices - 2)))
                  {
                    Error("CD3D8Renderer::DrawBuffer: DrawIndexedPrimitive error", h);
                    return;
                  }
                  m_nPolygons += (g->numIndices - 2);
                  break;
                  
                case PT_LIST:
                  if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, mi->nNumVerts, g->offsIndex+offs, g->numIndices / 3)))
                  {
                    Error("CD3D8Renderer::DrawBuffer: DrawIndexedPrimitive error", h);
                    return;
                  }
                  m_nPolygons += (g->numIndices / 3);
                  break;
                  
                case PT_FAN:
                  if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, mi->nNumVerts, g->offsIndex+offs, g->numIndices - 2)))
                  {
                    Error("CD3D8Renderer::DrawBuffer: DrawIndexedPrimitive error", h);
                    return;
                  }
                  m_nPolygons += (g->numIndices - 2);
                  break;
              }
            }
          }
        }
      }
      break;
  }
}

///////////////////////////////////////////
void CD3D8Renderer::UpdateBuffer(CVertexBuffer *dest, const void *src, int vertexcount, bool bUnLock, int offs, int Type)
{
  VOID *pVertices;

  HRESULT hr = 0;
  IDirect3DVertexBuffer8 *tvert;
  int size;
  if (!src)
  {
    if (!Type)
    {
      tvert=(IDirect3DVertexBuffer8 *)dest->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr;
      size = m_VertexSize[dest->m_vertexformat];
      if (bUnLock)
      {
        // video buffer update
        if (dest->m_VS[VSF_GENERAL].m_bLocked)
        {
          dest->m_VS[VSF_GENERAL].m_bLocked = false;
          hr = tvert->Unlock();
        }
      }
      else
      {
        // video buffer update
        if (!dest->m_VS[VSF_GENERAL].m_bLocked)
        {
          dest->m_VS[VSF_GENERAL].m_bLocked = true;
#ifdef _XBOX
          hr = tvert->Lock(0, 0, (BYTE **) &pVertices, 0);
#else
          hr=tvert->Lock(0, 0, (BYTE **) &pVertices, dest->m_bDynamic ? D3DLOCK_DISCARD : 0);
#endif
          dest->m_VS[VSF_GENERAL].m_VData = pVertices;
        }
      }
    }
    else
    {
      for (int i=0; i<VSF_NUM-1; i++)
      {
        tvert = (IDirect3DVertexBuffer8 *)dest->m_VS[i].m_VertBuf.m_pPtr;
        if (!tvert)
          continue;
        if (!((1<<i) & Type))
          continue;
        if (bUnLock)
        {
          if (dest->m_VS[i].m_bLocked)
          {
            dest->m_VS[i].m_bLocked = false;
            hr = tvert->Unlock();
          }
        }
        else
        {
          if (!dest->m_VS[i].m_bLocked)
          {
            dest->m_VS[i].m_bLocked = true;
#ifndef _XBOX
            hr=tvert->Lock(0, 0, (BYTE **) &pVertices, dest->m_bDynamic ? D3DLOCK_DISCARD : 0);
#else
            hr=tvert->Lock(0, 0, (BYTE **) &pVertices, 0);
#endif
            dest->m_VS[i].m_VData = pVertices;
          }
        }
      }
    }
    return;
  }

  if (Type == VSF_GENERAL)
  {
    tvert = (IDirect3DVertexBuffer8 *)dest->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr;
    size = m_VertexSize[dest->m_vertexformat];
  }
  else
  if (Type == VSF_TANGENTS)
  {
    tvert = (IDirect3DVertexBuffer8 *)dest->m_VS[VSF_TANGENTS].m_VertBuf.m_pPtr;
    size = sizeof(SPipTangents);
  }
  else
  if (Type == VSF_LMTC)
  {
    tvert = (IDirect3DVertexBuffer8 *)dest->m_VS[VSF_LMTC].m_VertBuf.m_pPtr;
    size = sizeof(SMRendTexVert);
  }

  if (!tvert)  // system buffer update
  {
    memcpy(dest->m_VS[VSF_GENERAL].m_VData, src, m_VertexSize[dest->m_vertexformat]*vertexcount);
    dest->m_VS[Type].m_bLocked = 0;
    return;
  }

  // video buffer update
  if (!dest->m_VS[Type].m_bLocked)
  {
    dest->m_VS[Type].m_bLocked = true;
#ifndef _XBOX
    hr=tvert->Lock(0, 0, (BYTE **) &pVertices, dest->m_bDynamic ? D3DLOCK_DISCARD : 0);
#else
    hr=tvert->Lock(0, 0, (BYTE **) &pVertices, 0);
#endif
    dest->m_VS[Type].m_VData = pVertices;
  }

  if (SUCCEEDED(hr) && src)
  {
    memcpy(dest->m_VS[Type].m_VData, src, size*vertexcount);
    tvert->Unlock();
    dest->m_VS[Type].m_bLocked = false;
  }
  else
  if (dest->m_VS[Type].m_bLocked && bUnLock)
  {
    tvert->Unlock();
    dest->m_VS[Type].m_bLocked = false;
  }
}

void CD3D8Renderer::UnlockBuffer(CVertexBuffer *buf, int Type)
{
  if (!buf->m_VS[Type].m_bLocked)
    return;

  IDirect3DVertexBuffer8 *tvert = (IDirect3DVertexBuffer8 *)buf->m_VS[Type].m_VertBuf.m_pPtr;

  tvert->Unlock();
  buf->m_VS[Type].m_bLocked = false;
}

///////////////////////////////////////////
void CD3D8Renderer::ReleaseBuffer(CVertexBuffer *bufptr)
{
  if (bufptr)
  {
    m_CurVertBufferSize -= m_VertexSize[bufptr->m_vertexformat]*bufptr->m_NumVerts;
    if (bufptr->m_VS[VSF_TANGENTS].m_VertBuf.m_pPtr)
      m_CurVertBufferSize -= sizeof(SPipTangents)*bufptr->m_NumVerts;
    if (bufptr->m_VS[VSF_LMTC].m_VertBuf.m_pPtr)
      m_CurVertBufferSize -= sizeof(SMRendTexVert)*bufptr->m_NumVerts;

    if (bufptr->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr)
    {
      IDirect3DVertexBuffer8 *vtemp = (IDirect3DVertexBuffer8 *)bufptr->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr;
      SAFE_RELEASE(vtemp);
      bufptr->m_VS[VSF_GENERAL].m_VertBuf.m_pPtr = NULL;

      vtemp = (IDirect3DVertexBuffer8 *)bufptr->m_VS[VSF_TANGENTS].m_VertBuf.m_pPtr;
      SAFE_RELEASE(vtemp);
      bufptr->m_VS[VSF_TANGENTS].m_VertBuf.m_pPtr = NULL;

      vtemp = (IDirect3DVertexBuffer8 *)bufptr->m_VS[VSF_LMTC].m_VertBuf.m_pPtr;
      SAFE_RELEASE(vtemp);
      bufptr->m_VS[VSF_LMTC].m_VertBuf.m_pPtr = NULL;
    }
     
    delete bufptr;
  }
}

///////////////////////////////////////////
void CD3D8Renderer::CheckError(const char *comment)
{

}

///////////////////////////////////////////
int CD3D8Renderer::SetPolygonMode(int mode)
{
  int prev_mode = m_polygon_mode;
	m_polygon_mode = mode;
  if(m_polygon_mode == R_WIREFRAME_MODE)
    m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
  else
    m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
  return prev_mode;
}

///////////////////////////////////////////
void CD3D8Renderer::EnableVSync(bool enable)
{
}

void CD3D8Renderer::EnableDepthTest(bool enable)
{
  ASSERT(m_pd3dDevice);

  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, enable ? TRUE : FALSE);
}

void CD3D8Renderer::EnableDepthWrites(bool enable)
{
  ASSERT(m_pd3dDevice);

  m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, enable ? TRUE : FALSE);
}

///////////////////////////////////////////

///////////////////////////////////////////
void CD3D8Renderer::EnableAlphaTest(bool enable,float alphavalue)
{
  if (enable)
  {
    m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, (unsigned long)(256.0f*alphavalue));
    m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
  }
  else
    m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
}

void CD3D8Renderer::DrawQuadInFogVolume(float dy,float dx, float dz, float x, float y, float z, float fFogLevel, float fFogViewDist)
{
}

#include "I3DEngine.h"
#include "../Cry3DEngine/Terrain.h"
#include "../Cry3DEngine/StatObj.h"
#include "../Cry3DEngine/ObjMan.h"

#ifdef WIN32

// duplicated definition (first one is in 3dengine)
ISystem * Cry3DEngineBase::m_pSys=0;
IRenderer * Cry3DEngineBase::m_pRenderer=0;
ITimer * Cry3DEngineBase::m_pTimer=0;
ILog * Cry3DEngineBase::m_pLog=0;
IPhysicalWorld * Cry3DEngineBase::m_pPhysicalWorld=0;
IConsole * Cry3DEngineBase::m_pConsole=0;
I3DEngine * Cry3DEngineBase::m_p3DEngine=0;
CVars * Cry3DEngineBase::m_pCVars=0;
ICryPak * Cry3DEngineBase::m_pCryPak=0;

#endif // WIN32
/*
static _inline int Compare(CStatObjInst *& p1, CStatObjInst *& p2)
{
  if(p1->m_fDistance > p2->m_fDistance)
    return 1;
  else
  if(p1->m_fDistance < p2->m_fDistance)
    return -1;
  
  return 0;
}
  */
void CD3D8Renderer::DrawObjSprites (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan)
{
  HRESULT hr;

  ResetToDefault();
  EnableBlend(true);
  EnableAlphaTest(true,0.125f);
  SetEnviMode(R_MODE_MODULATE);
  EnableDepthWrites(true);  
  SetCullMode(R_CULL_DISABLE);

  // Sorting far objects front to back since we use alphablending
//  ::Sort(&(*pList)[0], pList->Count());
  //pList->SortByDistanceMember_(true);

  float max_view_dist = fMaxViewDist*0.8f;
  const Vec3d & vCamPos = m_RP.m_ViewOrg;
  const float rad2deg = 180.0f/PI;    
  const float far_tex_angle = (FAR_TEX_ANGLE*0.5f);

  CD3D8TexMan::BindNULL(1);
  m_TexMan->SetTexture(TX_FIRSTBIND, eTT_Base);

  float v[4];
  v[3] = 1.0f;
  CCGVProgram_D3D *pVP = NULL;
  CCGVProgram_D3D *pVP_FV = NULL;
  // If device supports vertex shaders use advanced bending for sprites
  if (!m_RP.m_VPPlantBendingSpr && (GetFeatures() & RFT_HW_VS))
  {
    //pVP = (CVProgram_D3D *)CVProgram::mfForName("VProgSimple_Plant_Bended_Sprite", false);
    pVP = (CCGVProgram_D3D *)CVProgram::mfForName("CGVProgSimple_Plant_Bended_Sprite", true);
    pVP_FV = (CCGVProgram_D3D *)CVProgram::mfForName("CGVProgSimple_Plant_Bended_Sprite_FV", true);
    m_RP.m_VPPlantBendingSpr = pVP;
    m_RP.m_VPPlantBendingSpr_FV = pVP_FV;
  }
  else
  {
    pVP = (CCGVProgram_D3D *)m_RP.m_VPPlantBendingSpr;
    pVP_FV = (CCGVProgram_D3D *)m_RP.m_VPPlantBendingSpr_FV;
  }
  //pVP = NULL;
  //m_pd3dDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

  SCGBind *pBindBend = NULL;
  SCGBind *pBindPos = NULL;
  int nf = VERTEX_FORMAT_P3F_COL4UB_TEX2F;
  m_RP.m_CurD3DVFormat = nf + 0x10;
  m_RP.m_FlagsModificators &= ~7;

  SMFog *fb = NULL;

  SCGBind *pBindTG00, *pBindTG01, *pBindTG10, *pBindTG11;

  SCGBind *pBindBend_FV = NULL;
  SCGBind *pBindPos_FV = NULL;
  CFColor FogColor;
  Plane plane00, plane01, plane10, plane11;

  SCGBind *pCurBindBend = NULL;
  SCGBind *pCurBindPos = NULL;

  CCGVProgram_D3D *lastvpD3D = NULL;

  SWaveForm2 wfMain;
  if (pVP)
  {
    wfMain.m_eWFType = eWF_Sin;
    wfMain.m_Amp = 0.002f;
    wfMain.m_Level = 0;
    pVP->mfSet(true, NULL);

    lastvpD3D = pVP;
    pBindBend = pVP->mfGetParameterBind("Bend");
    pBindPos = pVP->mfGetParameterBind("ObjPos");

    pCurBindBend = pBindBend;
    pCurBindPos = pBindPos;
  }

  //struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad = NULL;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F vQuad[6];
  //int nFirst = 0;
  //int nLast = 0;
  //LPDIRECT3DVERTEXBuffer9 pVB = m_RP.mVBSprites.VBPtr_D_T2F->GetInterface();
  //pVB->Lock(0, 0, (BYTE **)&vQuad, D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
  //hr = m_pd3dDevice->SetStreamSource(0, pVB, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));

  Vec3d vWorldColor = pObjMan->GetSystem()->GetI3DEngine()->GetWorldColor();
  int prev_tid=-1;

  if (m_bHeatVision)
  {
    float param[4];
    if (!m_RP.m_RCSprites_Heat)
      m_RP.m_RCSprites_Heat = CPShader::mfForName("RCHeat_TreesSprites", false);
    if (m_RP.m_RCSprites_Heat)
    {
      m_RP.m_RCSprites_Heat->mfSet(true);
      param[0] = param[1] = 0;
      param[2] = 1.0f;
      param[3] = 1.0f;
      CPShader_D3D::mfSetFloat4f(0, param);
    }
  }

  for( int i=pList->Count()-1; i>=0; i-- )
  { 
    CStatObjInst * o = pList->GetAt(i);
    CStatObj * pStatObj = pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].GetStatObj();
    
    float fMaxDist = o->m_fMaxViewDist;//o->m_fMaxDist*pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].fMaxViewDistRatio; //o->scale*pStatObj->GetRadius()*80;
    
    // note: move into sort by size
    if(fMaxDist > max_view_dist)
      fMaxDist = max_view_dist;

    assert(SRendItem::m_RecurseLevel>=1 && SRendItem::m_RecurseLevel-1<=2);
    float fDistance = (*(((*(IEntityRender*)(&*o))).m_pEntityRenderState)).arrfDistance[SRendItem::m_RecurseLevel-1];

    float alpha = (1.f-(fDistance*pObjMan->m_fZoomFactor)/(fMaxDist))*8.f;
    if (alpha <= 0)
      continue;
    if(alpha>1.f)
      alpha=1.f;
/*
    Vec3d vBright;		
    if(o->m_bBright)
      vBright.Set(1.f,1.f,1.f);// = pObjMan->m_fOutdoorAmbientLevel + (1.f - pObjMan->m_fOutdoorAmbientLevel);
    else
      vBright = pObjMan->m_vOutdoorAmbientColor * pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].fAmbScale;
  */
    Vec3d vBright = Vec3d(0.01f,0.01f,0.01f)*o->m_ucBright;

    // use brightness from vegetation group settings
    vBright *= pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].fBrightness;

    float DX = o->m_vPos.x - vCamPos.x;
    float DY = o->m_vPos.y - vCamPos.y;
    int angle = FtoI(rad2deg*atan2f( DX, DY )+far_tex_angle);

    while(angle<0) angle+=360;

    assert(angle>=0 && angle/FAR_TEX_ANGLE<FAR_TEX_COUNT);
    
    int tid = pStatObj->m_arrSpriteTexID[(int)(angle/FAR_TEX_ANGLE)];
    if(prev_tid != tid)
    {
      m_TexMan->SetTexture(tid, eTT_Base);
      prev_tid = tid;
    }

    float r = vWorldColor.x*vBright.x;
    float g = vWorldColor.y*vBright.y;
    float b = vWorldColor.z*vBright.z;
    DWORD cCol = D3DRGBA(r,g,b,alpha);

    float fSpriteScaleV = o->m_fScale*pStatObj->GetRadiusVert()*1.04f*alpha;
    float fSpriteScaleH = o->m_fScale*pStatObj->GetRadiusHors()*pObjMan->m_fZoomFactor*1.050f*alpha;
    Vec3d vPos = o->m_vPos + pStatObj->GetCenter()*o->m_fScale;

    if(o->GetEntityRS()->nFogVolumeID>0 && CV_r_VolumetricFog)
    {
      // setup Volume Fog Texgen planes
      if (!pBindBend_FV)
      {
        if (lastvpD3D)
        {
          lastvpD3D->mfSet(false, NULL);
          lastvpD3D = NULL;
        }
        pVP_FV->mfSet(true, NULL);
        pBindBend_FV = pVP_FV->mfGetParameterBind("Bend");
        pBindPos_FV = pVP_FV->mfGetParameterBind("ObjPos");

        if (!m_RP.m_RCSprites_FV)
          m_RP.m_RCSprites_FV = CPShader::mfForName("RCTreeSprites_FV", false);

        fb = &m_RP.m_FogVolumes[o->GetEntityRS()->nFogVolumeID];
        if (fb->m_FogInfo.m_WaveFogGen.m_eWFType)
        {
          float f = SEvalFuncs::EvalWaveForm(&fb->m_FogInfo.m_WaveFogGen);

          fb->m_fMaxDist = f;
        }
        float intens = fb->m_fMaxDist;
        if (intens <= 0)
          intens = 1.0f;
        intens = -0.25f / intens;
        plane00.n.x = intens*m_CameraMatrix(0,2);
        plane00.n.y = intens*m_CameraMatrix(1,2);
        plane00.n.z = intens*m_CameraMatrix(2,2);
        plane00.d   = intens*m_CameraMatrix(3,2);
        plane00.d += 0.5f;
        pBindTG00 = pVP_FV->mfGetParameterBind("TexGen00");

        plane01.n.x = plane01.n.y = plane01.n.z = 0;
        plane01.d = 0.49f;
        pBindTG01 = pVP_FV->mfGetParameterBind("TexGen01");

        plane11.n.x = 0;
        plane11.n.y = 0;
        plane11.n.z = 0;
        plane11.d     = fb->m_Normal.Dot(m_RP.m_ViewOrg) - fb->m_Dist;
        float fSmooth;
        FogColor = fb->m_Color;
        if (plane11.d < -0.5f)
          fSmooth = 1.0f;
        else
          fSmooth = 0.1f;
        plane11.d *= fSmooth;
        plane11.d += 0.5f;
        pBindTG10 = pVP_FV->mfGetParameterBind("TexGen10");

        plane10.n.x = fb->m_Normal.x * fSmooth;
        plane10.n.y = fb->m_Normal.y * fSmooth;
        plane10.n.z = fb->m_Normal.z * fSmooth;
        plane10.d     = -(fb->m_Dist) * fSmooth;
        plane10.d += 0.5f;
        pBindTG11 = pVP_FV->mfGetParameterBind("TexGen11");
      }
      if (lastvpD3D != pVP_FV)
      {
        pCurBindBend = pBindBend_FV;
        pCurBindPos = pBindPos_FV;
        if (lastvpD3D)
          lastvpD3D->mfSet(false, NULL);
        lastvpD3D = pVP_FV;
        if (pVP_FV)
        {
          pVP_FV->mfSet(true, NULL);

          if (pBindTG00)
            pVP_FV->mfParameter4f(pBindTG00, &plane00.n.x);
          if (pBindTG01)
            pVP_FV->mfParameter4f(pBindTG01, &plane01.n.x);
          if (pBindTG10)
            pVP_FV->mfParameter4f(pBindTG10, &plane11.n.x);
          if (pBindTG11)
            pVP_FV->mfParameter4f(pBindTG11, &plane10.n.x);
        }

        EF_SelectTMU(1);
        CTexMan::m_Text_Fog->Set();
        EF_SelectTMU(2);
        CTexMan::m_Text_Fog_Enter->Set();
        EF_SelectTMU(0);

        float param[4];
        if (m_RP.m_RCSprites_FV)
        {
          m_RP.m_RCSprites_FV->mfSet(true);
          param[0] = m_WorldColor[0]; param[1] = m_WorldColor[1]; param[2] = m_WorldColor[2]; param[3] = m_WorldColor[3];
          CPShader_D3D::mfSetFloat4f(0, param);
          param[0] = FogColor[0]; param[1] = FogColor[1]; param[2] = FogColor[2]; param[3] = FogColor[3];
          CPShader_D3D::mfSetFloat4f(1, param);
        }
        EF_PushFog();
        EnableFog(false);
      }
    }
    else
    {
      if (lastvpD3D != pVP)
      {
        pCurBindBend = pBindBend;
        pCurBindPos = pBindPos;

        if (lastvpD3D)
          lastvpD3D->mfSet(false, NULL);

        if (lastvpD3D == pVP_FV)
        {
          EF_PopFog();
          if (m_RP.m_RCSprites_FV)
            m_RP.m_RCSprites_FV->mfSet(false);
        }
        lastvpD3D = pVP;
        if (pVP)
          pVP->mfSet(true, NULL);

        EF_SelectTMU(1);
        EnableTMU(false);
        EF_SelectTMU(2);
        EnableTMU(false);
        EF_SelectTMU(0);
      }
    }
    float dy = DX*fSpriteScaleH/fDistance;
    float dx = DY*fSpriteScaleH/fDistance;
    if (pVP)
    {
			float fGroupBending = pObjMan->m_lstStaticTypes[o->m_nObjectTypeID].fBending;

      float fIrv = 1.0f / pStatObj->GetRadiusVert();
      float fIScale = 1.0f / o->m_fScale;
      wfMain.m_Freq = fIrv/8.0f+0.2f;
      wfMain.m_Phase = vPos.x/8.0f;
      v[2] = (o->m_fCurrentBending + pObjMan->m_fWindForce*fGroupBending*6.0f)*fIrv;  // Bending factor
      v[0] = SEvalFuncs::EvalWaveForm(&wfMain) * fIScale;       // x amount
      wfMain.m_Freq = fIrv/7.0f+0.2f;
      wfMain.m_Phase = vPos.y/8.0f;
      v[1] = SEvalFuncs::EvalWaveForm(&wfMain) * fIScale;       // y amount
      //pVP->mfParameter(20, v);
      if (pCurBindBend)
        pVP->mfParameter4f(pCurBindBend, v);

      v[0] = o->m_vPos.x;
      v[1] = o->m_vPos.y;
      v[2] = o->m_vPos.z;      
      //pVP->mfParameter(21, v);  // Object position
      if (pCurBindPos)
        pVP->mfParameter4f(pCurBindPos, v);
    }

    // Define the tris
    vQuad[0].x = dx+vPos.x;
    vQuad[0].y = -dy+vPos.y;
    vQuad[0].z = -fSpriteScaleV+vPos.z;
    *(DWORD *)(&vQuad[0].r) = cCol;
    vQuad[0].s = 0;
    vQuad[0].t = 0;

    vQuad[1].x = -dx+vPos.x;
    vQuad[1].y = dy+vPos.y;
    vQuad[1].z = -fSpriteScaleV+vPos.z;
    *(DWORD *)(&vQuad[1].r) = cCol;
    vQuad[1].s = -1;
    vQuad[1].t = 0;

    vQuad[2].x = dx+vPos.x;
    vQuad[2].y = -dy+vPos.y;
    vQuad[2].z = vPos.z;
    *(DWORD *)(&vQuad[2].r) = cCol;
    vQuad[2].s = 0;
    vQuad[2].t = 0.5f;

    vQuad[3].x = -dx+vPos.x;
    vQuad[3].y = dy+vPos.y;
    vQuad[3].z = vPos.z;
    *(DWORD *)(&vQuad[3].r) = cCol;
    vQuad[3].s = -1;
    vQuad[3].t = 0.5f;

    vQuad[4].x = dx+vPos.x;
    vQuad[4].y = -dy+vPos.y;
    vQuad[4].z = fSpriteScaleV+vPos.z;
    *(DWORD *)(&vQuad[4].r) = cCol;
    vQuad[4].s = 0;
    vQuad[4].t = 1.0f;

    vQuad[5].x = -dx+vPos.x;
    vQuad[5].y = dy+vPos.y;
    vQuad[5].z = fSpriteScaleV+vPos.z;
    *(DWORD *)(&vQuad[5].r) = cCol;
    vQuad[5].s = -1.0f;
    vQuad[5].t = 1.0f;
    //nLast += 6;

    //pVB->Unlock();
    hr = m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 4, vQuad, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    //hr = m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, nFirst, nLast-nFirst-2);
    //nFirst = nLast;

    /*if (nLast + 6 >= m_RP.MaxVerts)
    {
      nFirst = nLast = 0;
      pVB->Lock(0, 6*sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F), (BYTE **)vQuad, D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
    }
    else
      pVB->Lock(nFirst*sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F), 6*sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F), (BYTE **)vQuad, D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK);*/

    m_nPolygons += 4;
  }
  if (pVP)
    pVP->mfSet(false, NULL);

  if (m_bHeatVision)
  {
    if (m_RP.m_RCSprites_Heat)
      m_RP.m_RCSprites_Heat->mfSet(false);
  }

  //pVB->Unlock();

  m_TexMan->SetTexture(0, eTT_Base);
}

void CD3D8Renderer::DrawQuad(const Vec3d &right, const Vec3d &up, const Vec3d &origin,int nFlipMode/*=0*/)
{
  SPipeVertex_D_1T_2F Verts[4];

	Vec3d curr;
  curr=origin+(-right-up);
  Verts[0].xyz[0] = curr[0]; Verts[0].xyz[1] = curr[1]; Verts[0].xyz[2] = curr[2];
  Verts[0].st[0] = -1; Verts[0].st[1] = 0;

	curr=origin+(right-up);
  Verts[1].xyz[0] = curr[0]; Verts[1].xyz[1] = curr[1]; Verts[1].xyz[2] = curr[2];
  Verts[1].st[0] = 0; Verts[1].st[1] = 0;

	curr=origin+(right+up);
  Verts[2].xyz[0] = curr[0]; Verts[2].xyz[1] = curr[1]; Verts[2].xyz[2] = curr[2];
  Verts[2].st[0] = 0; Verts[2].st[1] = 1;

	curr=origin+(-right+up);
  Verts[3].xyz[0] = curr[0]; Verts[3].xyz[1] = curr[1]; Verts[3].xyz[2] = curr[2];
  Verts[3].st[0] = -1; Verts[3].st[1] = 1;

  m_pd3dDevice->SetVertexShader( D3DFVF_VERTEX_D_1T_2F );
  h = m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Verts, STRIDE_D_1T_2F);
}

void CD3D8Renderer::DrawQuad(float dy,float dx, float dz, float x, float y, float z)
{
  SPipeVertex_D_1T_2F Verts[4];

  Verts[0].xyz[0] = -dx+x; Verts[0].xyz[1] = dy+y; Verts[0].xyz[2] = -dz+z;
  Verts[0].st[0] = 0; Verts[0].st[1] = 0;

  Verts[1].xyz[0] = dx+x; Verts[1].xyz[1] = -dy+y; Verts[1].xyz[2] = -dz+z;
  Verts[1].st[0] = 1; Verts[1].st[1] = 0;

  Verts[2].xyz[0] = dx+x; Verts[2].xyz[1] = -dy+y; Verts[2].xyz[2] = dz+z;
  Verts[2].st[0] = 1; Verts[2].st[1] = 1;

  Verts[3].xyz[0] = -dx+x; Verts[3].xyz[1] = dy+y; Verts[3].xyz[2] = dz+z;
  Verts[3].st[0] = 0; Verts[3].st[1] = 1;

  m_pd3dDevice->SetVertexShader( D3DFVF_VERTEX_D_1T_2F );
  h = m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Verts, STRIDE_D_1T_2F);
}

void CD3D8Renderer::DrawPoint(float x, float y, float z, float fSize)
{
  SPipeVertex_D_1T_2F Verts[1];

  SetCullMode(R_CULL_DISABLE);
  EnableDepthTest(0);
  EnableTMU(false);

  m_pd3dDevice->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&fSize));

  Verts[0].xyz[0] = x; Verts[0].xyz[1] = y; Verts[0].xyz[2] = z;
  Verts[0].color.dcolor = 0xffffffff;

  m_pd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_SELECTARG1);
  m_pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TFACTOR );
  m_pd3dDevice->SetVertexShader( D3DFVF_VERTEX_D_1T_2F );
  h = m_pd3dDevice->DrawPrimitiveUP(D3DPT_POINTLIST, 1, Verts, STRIDE_D_1T_2F);

  m_pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE );

  EnableTMU(true);
}

///////////////////////////////////////////
void CD3D8Renderer::DrawTriStrip(CVertexBuffer *src, int vert_num)
{
  switch (src->m_vertexformat)
  {
    case VERTEX_FORMAT_P3F_TEX2F:
      {
        struct_VERTEX_FORMAT_P3F_TEX2F *dt = (struct_VERTEX_FORMAT_P3F_TEX2F *)src->m_VS[VSF_GENERAL].m_VData;
        int n = src->m_vertexformat + 16;
        m_pd3dDevice->SetVertexShader( m_RP.m_D3DFixedPipeline[0][n].m_Handle );
        h = m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, vert_num-2, dt, sizeof(struct_VERTEX_FORMAT_P3F_TEX2F));
      }
      break;
    case VERTEX_FORMAT_P3F_COL4UB:
      {				
        struct_VERTEX_FORMAT_P3F_COL4UB *dt = (struct_VERTEX_FORMAT_P3F_COL4UB *)src->m_VS[VSF_GENERAL].m_VData;
        EF_SetVertColor();
        
        int n = src->m_vertexformat + 16;
        m_pd3dDevice->SetVertexShader( m_RP.m_D3DFixedPipeline[0][n].m_Handle );
        h = m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, vert_num-2, dt, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB));
      }
      break;
    case VERTEX_FORMAT_P3F_COL4UB_TEX2F:
      {				
        struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *dt = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)src->m_VS[VSF_GENERAL].m_VData;
        EF_SetVertColor();
        
        int n = src->m_vertexformat + 16;
        m_pd3dDevice->SetVertexShader( m_RP.m_D3DFixedPipeline[0][n].m_Handle );
        h = m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, vert_num-2, dt, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
      }
      break;
    case VERTEX_FORMAT_TRP3F_COL4UB_TEX2F:
      {				
        struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F *dt = (struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F *)src->m_VS[VSF_GENERAL].m_VData;
        EF_SetVertColor();
        
        int n = src->m_vertexformat + 16;
        m_pd3dDevice->SetVertexShader( m_RP.m_D3DFixedPipeline[0][n].m_Handle );
        h = m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, vert_num-2, dt, sizeof(struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F));
      }
      break;

    default:
      assert(0);
      break;
  }
	
	m_nPolygons += vert_num-2;
}

///////////////////////////////////////////
void CD3D8Renderer::ResetToDefault()
{
  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, ".... ResetToDefault ....\n");

  EnableDepthTest(true);
  EnableAlphaTest(false);
  SetCullMode(R_CULL_BACK);
  EnableBlend(false);
  SetBlendMode(R_BLEND_MODE__SRC_ALPHA__ONE_MINUS_SRC_ALPHA);	

  for (int i=0; i<m_numtmus; i++)
  {
    EF_SelectTMU(i);
    eCurColorOp[i] = -1;
    if (!i)
    {
      EnableTMU(true);
    }
    else
    {
      EnableTMU(false);
    }
    EnableTexGen(false);		
  }
  EF_SelectTMU(0);
  CTexMan::m_nCurStages = 1;

	SetMaterialColor(1,1,1,1);
  EnableLighting(false);
  SetDepthFunc(R_LEQUAL);
  EnableDepthWrites(true);

  m_pd3dDevice->SetRenderState(D3DRS_ZBIAS, 0);
#ifdef _XBOX
  m_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);
#else
  m_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
#endif

  m_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);

  EF_SetVertColor();

  mCurState = GS_DEPTHWRITE;
  m_eCull = (ECull)-1;

  if (m_LogFile && CV_r_log == 3)
    Logv(SRendItem::m_RecurseLevel, ".... End ResetToDefault ....\n");
}


//////////////////////////////////////////
int CD3D8Renderer::GenerateAlphaGlowTexture(float k)
{   
//  float k = 6;
  const int tex_size = 256;
  byte *data = new byte [tex_size*tex_size];

  memset(data, 255, tex_size*tex_size);

  for(int x=0; x<tex_size; x++)
  for(int y=0; y<tex_size; y++)
  {
    int _x = x-tex_size/2;
    int _y = y-tex_size/2;

    float val = k*2.f*((float)tex_size/2 - (float)(sqrt(double(_x*_x+_y*_y))));
    val = Clamp(val, 0.0f, 255.0f);

    data[x*256+y] = (int)(val);
  }

  int nRes = DownLoadToVideoMemory((unsigned char*)data,tex_size,tex_size,eTF_8000,eTF_8000,true,true,FILTER_LINEAR);
  delete [] data;

  return nRes;
}

uint CD3D8Renderer::Make3DSprite(int nTexSize, float fAngleStep, IStatObj * pStatObj)
{
  return 0;
}


uint CD3D8Renderer::MakeSprite(float object_scale, int tex_size, float angle, IStatObj * pStatObj, uchar * _pTmpBuffer, uint def_tid)
{
  D3DCOLOR cColor = D3DRGBA(0.3f,0.3f,0.3f,0.0f);
  HRESULT h;

  // calc vertical/horisontal radiuses
  float dxh = (float)max( fabs(pStatObj->GetBoxMax().x), fabs(pStatObj->GetBoxMin().x));
  float dyh = (float)max( fabs(pStatObj->GetBoxMax().y), fabs(pStatObj->GetBoxMin().y));
  float fRadiusHors = (float)sqrt(dxh*dxh + dyh*dyh);//max(dxh,dyh);
  float fRadiusVert = (pStatObj->GetBoxMax().z-pStatObj->GetBoxMin().z)/2;
  
  float fDrawDist = fRadiusVert*25.f*8.f;

  EF_PushFog();
  EnableFog(false);

  D3DXMATRIX *m = m_matProj->GetTop();
  D3DXMatrixPerspectiveFovRH(m, 0.58f*(gf_PI/180.0f), fRadiusHors/fRadiusVert, fDrawDist-fRadiusHors*2, fDrawDist+fRadiusHors*2);
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
  m_bInvertedMatrix = false;

  if (!m_SceneRecurseCount)
    m_pd3dDevice->BeginScene();
  m_SceneRecurseCount++;

  m_Viewport.X = 0;
  m_Viewport.Y = 0;
  m_Viewport.Width = tex_size;
  m_Viewport.Height = tex_size;
  m_Viewport.MinZ = 0.0f;
  m_Viewport.MaxZ = 1.0f;
  m_pd3dDevice->SetViewport(&m_Viewport);

  Vec3d vCenter = (pStatObj->GetBoxMax()+pStatObj->GetBoxMin())*0.5f;

  D3DXVECTOR3 Eye = D3DXVECTOR3(0,0,0);
  D3DXVECTOR3 At = D3DXVECTOR3(-1,0,0);
  D3DXVECTOR3 Up = D3DXVECTOR3(0,0,1);

  m = m_matView->GetTop();  
  D3DXMatrixLookAtRH(m, &Eye, &At, &Up);
  m_matView->TranslateLocal(-fDrawDist,0,0);
  D3DXVECTOR3 Axis = D3DXVECTOR3(0,0,1);
  m_matView->RotateAxisLocal(&Axis, angle*(gf_PI/180.0f));
  m_matView->TranslateLocal(-vCenter.x,-vCenter.y,-vCenter.z);
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
  
  byte *data = new byte[tex_size*tex_size*4];
  char name[128];
  sprintf(name, "$AutoSprites_%d", m_TexGenID++);
  int flags = FT_NOMIPS | FT_HASALPHA;
  STexPic *tp = m_TexMan->CreateTexture(name, tex_size, tex_size, 1, flags, FT2_NODXT, data, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);
  STexPicD3D *t = (STexPicD3D *)tp;
  LPDIRECT3DTEXTURE8 pID3DTexture = NULL;
  LPDIRECT3DTEXTURE8 pID3DTargetTexture = NULL;
  delete [] data;
  if (!t->m_RefTex)
    return 0;
  pID3DTexture = (LPDIRECT3DTEXTURE8)t->m_RefTex->m_VidTex;
  if (!pID3DTexture)
    return 0;

#ifndef _XBOX  
  if( FAILED( h = m_pd3dDevice->CreateTexture( tex_size, tex_size, 1, D3DUSAGE_RENDERTARGET,  D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pID3DTargetTexture )))
    return 0;
#else
  if( FAILED( h = m_pd3dDevice->CreateTexture( tex_size, tex_size, 1, D3DUSAGE_RENDERTARGET,  D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pID3DTargetTexture )))
    return 0;
#endif

  LPDIRECT3DSURFACE8 pSrcSurf;
  h = pID3DTargetTexture->GetSurfaceLevel(0, &pSrcSurf);
  m_pd3dDevice->SetRenderTarget( pSrcSurf, m_pZBuffer );
  pSrcSurf->Release();

  // render object
  if (m_sbpp)
    m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, cColor, 1.0f, 0);
  else
    m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, cColor, 1.0f, 0);
  
  EF_SetWorldColor(0.5f,0.5f,0.5f);
  
  EF_StartEf();  
  SRendParams rParms;
  rParms.nShaderTemplate = -1;
  pStatObj->Render(rParms);
  EF_EndEf3D(true);

  m_pd3dDevice->SetRenderTarget( m_pBackBuffer, m_pZBuffer );
  m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, cColor, 1.0f, 0);

  // Copy data
  {
    LPDIRECT3DTEXTURE8 pID3DSrcTexture;
    LPDIRECT3DSURFACE8 pDstSurf, pSrcSurf;
    D3DLOCKED_RECT d3dlrSrc, d3dlrDst;
    h = pID3DTargetTexture->GetSurfaceLevel(0, &pSrcSurf);
    h = D3DXCreateTexture(m_pd3dDevice, tex_size, tex_size, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pID3DSrcTexture );
    h = pID3DSrcTexture->GetSurfaceLevel(0, &pDstSurf);
    h = m_pd3dDevice->CopyRects(pSrcSurf, NULL, 0, pDstSurf, NULL);
    h = pID3DSrcTexture->LockRect(0, &d3dlrSrc, NULL, 0);
    h = pID3DTexture->LockRect(0, &d3dlrDst, NULL, 0);
    byte *ds = new byte [tex_size*tex_size*4];
#ifndef _XBOX
    // Copy data to the texture 
    memcpy(ds, d3dlrSrc.pBits, tex_size*tex_size*4);
#else
    XGUnswizzleRect( d3dlrSrc.pBits, tex_size, tex_size, NULL, ds, d3dlrSrc.Pitch, NULL, 4 );
#endif
    
    SetTextureAlphaChannelFromRGB(ds, tex_size);

#ifndef _XBOX
    byte *dst = (byte *)d3dlrDst.pBits;
#else
    byte *dst = new byte [tex_size*tex_size*4];
#endif
    for (int i=0; i<tex_size; i++)
    {
      int ni0 = (tex_size-i-1)*tex_size*4;
      int ni1 = (i * tex_size)*4;
      cryMemcpy(&dst[ni0], &ds[ni1], tex_size*4);
    }
    //cryMemcpy(dst, ds, tex_size*tex_size*4);
	  /*char buff[32]="";
	  static int imid=0;
	  sprintf(buff, "sprite%d_D3D.tga", imid);
	  imid++;
    ::WriteTGA((byte *)dst, tex_size, tex_size, buff, 32);*/

    delete [] ds;

#ifdef _XBOX
    XGSwizzleRect( dst, 0, NULL, d3dlrDst.pBits, tex_size, tex_size, NULL, sizeof(DWORD) );
    delete [] dst;
#endif

    h = pID3DTexture->UnlockRect(0);
    pID3DSrcTexture->UnlockRect(0);
    SAFE_RELEASE (pSrcSurf);
    SAFE_RELEASE (pDstSurf);
    SAFE_RELEASE (pID3DSrcTexture);
    SAFE_RELEASE (pID3DTargetTexture);
  }

  m_matProj->LoadIdentity();
  m = m_matProj->GetTop();
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
  
  m_matView->LoadIdentity();
  m = m_matView->GetTop();
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m);

  SetViewport();  
  EF_PopFog();

  m_SceneRecurseCount--;
  if (!m_SceneRecurseCount)
    m_pd3dDevice->EndScene();
  
  return tp->m_Bind;
}

///////////////////////////////////////////
void CD3D8Renderer::SetMaterialColor(float r, float g, float b, float a)
{
  EF_SetGlobalColor(r, g, b, a);
}

///////////////////////////////////////////
int CD3D8Renderer::LoadAnimatedTexture(const char * szFileNameFormat,const int nCount)
{
  if(nCount<1)
    return 0;

  for(int t=0; t<m_LoadedAnimatedTextures.Count(); t++)
  {
    if(strcmp(m_LoadedAnimatedTextures[t]->sName, szFileNameFormat) == 0)
    if(m_LoadedAnimatedTextures[t]->nFramesCount == nCount)
      return t+1;
  }

  {
    char szFileName[256];
    sprintf(szFileName,szFileNameFormat,0);
    iLog->LogToFile("Loading animated texture %s", szFileName);
  }

  // load new textures
  AnimTexInfo * pInfo = new AnimTexInfo();
  pInfo->pBindIds = new int[nCount];
  memset(pInfo->pBindIds,0,nCount*sizeof(int));
  pInfo->nFramesCount = nCount;
  strncpy(pInfo->sName, szFileNameFormat, sizeof(pInfo->sName));
  
  for (int i=0; i<nCount; i++) 
  {
    char filename[80]="";
    sprintf(filename, szFileNameFormat, i);
    pInfo->pBindIds[i] = LoadTexture( filename );
  }

  iLog->LogToFilePlus(" (%d)", nCount);

  m_LoadedAnimatedTextures.Add(pInfo);

  return (int)m_LoadedAnimatedTextures.Count();
}

///////////////////////////////////////////
char * CD3D8Renderer::GetStatusText(ERendStats type)
{
  return "No status yet";
}

void sLogTexture (char *name, int Size);

///////////////////////////////////////////
void CD3D8Renderer::EnableLight     (int id, bool enable)
{
  m_pd3dDevice->LightEnable(id, enable);
}
void CD3D8Renderer::SetLightPos     (int id, Vec3d &pos, bool bDirectional)
{
  D3DLIGHT8 *Light = &m_Lights[id];;
  Light->Position.x = pos[0];
  Light->Position.y = pos[1];
  Light->Position.z = pos[2];
  Vec3d n = pos;
  n.Normalize();
  Light->Direction.x = n[0];
  Light->Direction.y = n[1];
  Light->Direction.z = n[2];
  m_pd3dDevice->SetLight(id, Light);
}
void CD3D8Renderer::SetLightDiffuse (int id, Vec3d &color)
{
  D3DLIGHT8 *Light = &m_Lights[id];;
  Light->Diffuse.r = color[0];
  Light->Diffuse.g = color[1];
  Light->Diffuse.b = color[2];
  Light->Diffuse.a = 1.0f;
  m_pd3dDevice->SetLight(id, Light);
}
void CD3D8Renderer::SetLightAmbient (int id, Vec3d &color)
{
  D3DLIGHT8 *Light = &m_Lights[id];;
  Light->Ambient.r = color[0];
  Light->Ambient.g = color[1];
  Light->Ambient.b = color[2];
  Light->Ambient.a = 1.0f;
  m_pd3dDevice->SetLight(id, Light);
}
void CD3D8Renderer::SetLightSpecular(int id, Vec3d &color)
{
  D3DLIGHT8 *Light = &m_Lights[id];;
  Light->Specular.r = color[0];
  Light->Specular.g = color[1];
  Light->Specular.b = color[2];
  Light->Specular.a = 1.0f;
  m_pd3dDevice->SetLight(id, Light);
}
void CD3D8Renderer::SetLightAttenuat(int id, float att)
{
  D3DLIGHT8 *Light = &m_Lights[id];;
  Light->Attenuation0 = att;
  m_pd3dDevice->SetLight(id, Light);
}
void CD3D8Renderer::EnableLighting  (bool enable)
{
  m_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, enable);
  m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, enable);
  m_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0xffffffff);
}
void CD3D8Renderer::SetMaterialDiffuse (Vec3d &color)
{
  m_Material.Diffuse.r = color[0];
  m_Material.Diffuse.g = color[1];
  m_Material.Diffuse.b = color[2];
  m_Material.Diffuse.a = 1.0f;

  m_pd3dDevice->SetMaterial( &m_Material );
}
void CD3D8Renderer::SetMaterialAmbient (Vec3d &color)
{
  m_Material.Ambient.r = color[0];
  m_Material.Ambient.g = color[1];
  m_Material.Ambient.b = color[2];
  m_Material.Ambient.a = 1.0f;

  m_pd3dDevice->SetMaterial( &m_Material );
}
void CD3D8Renderer::SetMaterialSpecular(Vec3d &color)
{
  m_Material.Specular.r = color[0];
  m_Material.Specular.g = color[1];
  m_Material.Specular.b = color[2];
  m_Material.Specular.a = 1.0f;

  m_pd3dDevice->SetMaterial( &m_Material );
}

void CD3D8Renderer::ProjectToScreen(float ptx, float pty, float ptz,float *sx, float *sy, float *sz )
{
  D3DXVECTOR3 vOut, vIn;
  vIn.x = ptx;
  vIn.y = pty;
  vIn.z = ptz;
  D3DXMATRIX mIdent;
  D3DXMatrixIdentity(&mIdent);
  D3DXVec3Project(&vOut, &vIn, &m_Viewport, m_matProj->GetTop(), m_matView->GetTop(), &mIdent);
  *sx = vOut.x;
  *sy = vOut.y;
  *sz = vOut.z;
}

void CD3D8Renderer::DrawBall(float x, float y, float z, float radius)
{
  LPD3DXMESH pMesh;

  HRESULT hr = D3DXCreateSphere(m_pd3dDevice, radius, 16, 16, &pMesh, NULL);
  if (FAILED(hr))
    return;
  EF_PushMatrix();
  EnableTMU(false);
  TranslateMatrix(x, y, z);

  pMesh->DrawSubset(0);
  SAFE_RELEASE(pMesh);

  EnableTMU(true);
  EF_PopMatrix();

}

void CD3D8Renderer::DrawBall(const Vec3d & pos, float radius)
{
  assert(0);
}

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]

static void matmul4( float *product, const float *a, const float *b )
{
  int i;
  for (i=0; i<4; i++)
  {
    float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
    P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
    P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
    P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
    P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
  }
}

#undef A
#undef B
#undef P

/*
* Transform a point (column vector) by a 4x4 matrix.  I.e.  out = m * in
* Input:  m - the 4x4 matrix
*         in - the 4x1 vector
* Output:  out - the resulting 4x1 vector.
*/
static void transform_point(float out[4], const float m[16], const float in[4])
{
#define M(row,col)  m[col*4+row]
  out[0] = M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
  out[1] = M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
  out[2] = M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
  out[3] = M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

static int sUnProject(float winx, float winy, float winz, const float model[16], const float proj[16], const int viewport[4], float * objx, float * objy, float * objz)
{
  /* matrice de transformation */
  float m[16], A[16];
  float in[4], out[4];
  /* transformation coordonnees normalisees entre -1 et 1 */
  in[0] = (winx - viewport[0]) * 2 / viewport[2] - 1.0f;
  in[1] = (winy - viewport[1]) * 2 / viewport[3] - 1.0f;
  in[2] = 2.0f * winz - 1.0f;
  in[3] = 1.0;
  /* calcul transformation inverse */
  matmul4(A, proj, model);
  QQinvertMatrixf(m, A);
  /* d'ou les coordonnees objets */
  transform_point(out, m, in);
  if (out[3] == 0.0)
    return false;
  *objx = out[0] / out[3];
  *objy = out[1] / out[3];
  *objz = out[2] / out[3];
  return true;
}

int CD3D8Renderer::UnProject(float sx, float sy, float sz, float *px, float *py, float *pz, const float modelMatrix[16], const float projMatrix[16], const int viewport[4])
{
  return sUnProject(sx, sy, sz, modelMatrix, projMatrix, viewport, px, py, pz);
}

int CD3D8Renderer::UnProjectFromScreen( float sx, float sy, float sz, float *px, float *py, float *pz)
{
  float modelMatrix[16];
  float projMatrix[16];
  int viewport[4];

  GetModelViewMatrix(modelMatrix);
  GetProjectionMatrix(projMatrix);
  GetViewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);
  return sUnProject(sx, sy, sz, modelMatrix, projMatrix, viewport, px, py, pz);
}

void CD3D8Renderer::SetColorMask(unsigned char r,unsigned char g,unsigned char b,unsigned char a)
{
  UINT flag = 0;
  if(r)
    flag |= D3DCOLORWRITEENABLE_RED;
  if(g)
    flag |= D3DCOLORWRITEENABLE_GREEN;
  if(b)
    flag |= D3DCOLORWRITEENABLE_BLUE;
  if(a)
    flag |= D3DCOLORWRITEENABLE_ALPHA;
  m_pd3dDevice->SetRenderState (D3DRS_COLORWRITEENABLE, flag);
}

//////////////////////////////////////////////////////////////////////
void CD3D8Renderer::ClearDepthBuffer()
{
  m_bWasCleared = true;
  m_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);
}

void CD3D8Renderer::ClearColorBuffer(const Vec3d vColor)
{
  m_bWasCleared = true;
  m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DRGBA(vColor[0], vColor[1], vColor[2], 1.0f), 1.0f, 0);
}

void CD3D8Renderer::ReadFrameBuffer(unsigned char * pRGB, int nSizeX, int nSizeY, bool bBackBuffer, bool bRGBA)
{
	assert(0);
}

void CD3D8Renderer::SetDepthFunc(int mode)
{
  switch (mode)
  {
    case R_NEVER:
      m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_NEVER);
      break;
    case R_LESS:
      m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
      break;
    case R_EQUAL:
      m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
      break;
    case R_LEQUAL:
      m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
      break;
    case R_GREATER:
      m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATER);
      break;
    case R_NOTEQUAL:
      m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_NOTEQUAL);
      break;
    case R_GEQUAL:
      m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
      break;
    case R_ALWAYS:
      m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
      break;
    default:
      iLog->Log("Unknown DepthFunc 0x%x\n", mode);
  }
}

void CD3D8Renderer::EnableStencilTest(bool enable)
{
  mfGetD3DDevice()->SetRenderState(D3DRS_STENCILENABLE, enable);
}

void CD3D8Renderer::SetStencilMask(unsigned char value)
{
  assert(0);
}

void CD3D8Renderer::SetStencilFunc(int func,int ref,int mask)
{
  assert(0);
}

void CD3D8Renderer::SetStencilOp(int fail,int zfail,int pass)
{
  assert(0);  
}

void CD3D8Renderer::DrawTransparentQuad2D(float color)
{
  assert(0);  
}

void CD3D8Renderer::EnableAALines(bool bEnable)
{
  assert(0);
}

void CD3D8Renderer::DrawCircle(float fX, float fY, float fZ, float fRadius)
{
  assert(0);
}

void CD3D8Renderer::PrepareDepthMap(ShadowMapFrustum * lof, bool make_new_tid)
{
  //assert(0);
}

//======================================================================

void CD3D8Renderer::ConfigCombinersForShadowPass()
{
  assert(0);
}

void CD3D8Renderer::ConfigCombinersForHardwareShadowPass(int withTexture, float * lightDimColor)
{
  assert(0);
}

void CD3D8Renderer::Set2DMode(bool enable, int ortox, int ortoy)
{
  D3DXMATRIX *m;
  if(enable)
  {
    m_matProj->Push();
    m = m_matProj->GetTop();
	  D3DXMatrixOrthoOffCenterLH(m, 0.0, (float)ortox, (float)ortoy, 0.0, -9999.0, 9999.0);
    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
    EF_PushMatrix();
    m = m_matView->GetTop();
    m_matView->LoadIdentity();
    m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
  }
  else
  {
    m_matProj->Pop();
    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m_matProj->GetTop());
    EF_PopMatrix();
  }
}

unsigned int CD3D8Renderer::MakeTexture(const char * _filename,int *_tex_type/*,unsigned int def_tid*/)
{
  return LoadTexture(_filename,_tex_type);
}

void CD3D8Renderer::SetTexClampMode(bool clamp)
{
  if (m_TexMan->m_LastTex)  
  {
    STexPicD3D *ti = (STexPicD3D *)m_TexMan->m_LastTex;
    if (ti->m_RefTex)
      ti->m_RefTex->bRepeats = !clamp;
  }
  mStages[m_TexMan->m_CurStage].Repeat = !clamp;
  m_pd3dDevice->SetTextureStageState(m_TexMan->m_CurStage, D3DTSS_ADDRESSU, clamp ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
  m_pd3dDevice->SetTextureStageState(m_TexMan->m_CurStage, D3DTSS_ADDRESSV, clamp ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
}

void CD3D8Renderer::TransformTextureMatrix(float x, float y, float angle, float scale)
{
  D3DXMATRIX *m = &m_TexMatrix[m_TexMan->m_CurStage];

  D3DXMATRIX ma;
  D3DXMatrixTranslation(&ma, x, y, 0);
  D3DXMatrixMultiply(m, &ma, m);
  D3DXMatrixTranslation(&ma, 0.5, 0.5, 0);
  D3DXMatrixMultiply(m, &ma, m);
  D3DXMatrixRotationZ(&ma, angle);
  D3DXMatrixMultiply(m, &ma, m);
  D3DXMatrixScaling(&ma, scale, scale, scale);
  D3DXMatrixMultiply(m, &ma, m);
  D3DXMatrixTranslation(&ma, -0.5, -0.5, 0);
  D3DXMatrixMultiply(m, &ma, m);

  m_pd3dDevice->SetTransform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+m_TexMan->m_CurStage), m);
}

void CD3D8Renderer::ResetTextureMatrix()
{
  D3DXMATRIX *mt = &m_TexMatrix[m_TexMan->m_CurStage];
  D3DXMatrixIdentity(mt);
  m_pd3dDevice->SetTransform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+m_TexMan->m_CurStage), mt);
}

void CD3D8Renderer::RemoveTexture(ITexPic * pTexPic)
{
  STexPic * pSTexPic = (STexPic *)pTexPic;
  pSTexPic->Release(false);
}


void CD3D8Renderer::RemoveTexture(unsigned int TextureId)
{
  CD3D8TexMan *tm = (CD3D8TexMan *)m_TexMan;
  TTextureMapItor it = tm->m_RefTexs.find(TextureId);
  SRefTex *rt;
  if (it == tm->m_RefTexs.end())
    return;
  rt = it->second;
  if (rt->m_SrcTex)
    rt->m_SrcTex->Release(false);
  else
    m_TexMan->RemoveFromHash(TextureId, NULL);
}

unsigned int CD3D8Renderer::LoadTexture(const char * _filename,int *tex_type,unsigned int def_tid,bool compresstodisk,bool bWarn)
{
  if (def_tid == 0)
    def_tid = -1;
  ITexPic * pPic = EF_LoadTexture((char*)_filename,FT_NOREMOVE, 0, eTT_Base, -1, -1, def_tid);
  return pPic->IsTextureLoaded() ? pPic->GetTextureID() : 0;
}

unsigned int CD3D8Renderer::DownLoadToVideoMemory(unsigned char *data,int w, int h, ETEX_Format eTFSrc, ETEX_Format eTFDst, int nummipmap, bool repeat, int filter, int Id)
{
  char name[128];
  sprintf(name, "$AutoDownload_%d", m_TexGenID++);
  int flags = nummipmap ? 0 : FT_NOMIPS;
  int DXTSize = 0;
  int blockSize = 0;
  if (eTFSrc == eTF_DXT1)
  {
    flags |= FT_DXT1;
    blockSize = 8;
  }
  else
  if (eTFSrc == eTF_DXT3)
  {
    flags |= FT_DXT3;
    blockSize = 16;
  }
  else
  if (eTFSrc == eTF_DXT5)
  {
    flags |= FT_DXT5;
    blockSize = 16;
  }
  if (blockSize)
  {
    if (!nummipmap)
      nummipmap = 1;
    int wdt = w;
    int hgt = h;
    for (int i=0; i<nummipmap; i++)
    {
      DXTSize += ((wdt+3)/4)*((hgt+3)/4)*blockSize;
      wdt >>= 1;
      hgt >>= 1;
      if (!wdt)
        wdt = 1;
      if (!hgt)
        hgt = 1;
    }
  }
  if (!repeat)
    flags |= FT_CLAMP;
  
  STexPic *tp = m_TexMan->CreateTexture(name, w, h, 1, flags, FT2_NODXT, data, eTT_Base, -1.0f, -1.0f, DXTSize, NULL, 0, eTFSrc);

  return (tp->m_Bind);  
}

void CD3D8Renderer::SetTexture(int tnum, ETexType Type)
{
  m_TexMan->SetTexture(tnum, Type);
}

bool CD3D8Renderer::EF_SetLightHole(Vec3d vPos, Vec3d vNormal, int idTex, float fScale, bool bAdditive)
{
  return false;
}

bool CD3D8Renderer::SetGammaDelta(const float fGamma)
{
  return true;
}

void CD3D8Renderer::GetMemoryUsage(ICrySizer* Sizer)
{
}


void CD3D8Renderer::DrawQuad(float x0, float y0, float x1, float y1, const CFColor & color, float ftx0,  float fty0, float ftx1, float fty1)
{
  LPDIRECT3DDEVICE8 dv = mfGetD3DDevice();
  DWORD col = (DWORD(color.a * 255) << 24) | (DWORD(color.r * 255) << 16) |
              (DWORD(color.g * 255)<<8) | DWORD(color.b * 255);

  SPipeTRVertex_D_1T *vQuad;

#ifdef _XBOX
  HRESULT hr = m_pQuadVB->Lock(0, 0, (BYTE **) &vQuad, 0);
#else
  HRESULT hr = m_pQuadVB->Lock(0, 0, (BYTE **) &vQuad, D3DLOCK_DISCARD);
#endif

  // Define the quad
  vQuad[0].dvSX = x0;
  vQuad[0].dvSY = y0;
  vQuad[0].dvSZ = 1.0f;
  vQuad[0].dvRHW = 1.0f;
  vQuad[0].dcColor = col;
  vQuad[0].dvTU[0] = ftx0;
  vQuad[0].dvTU[1] = fty0;
  
  vQuad[1].dvSX = x1;
  vQuad[1].dvSY = y0;
  vQuad[1].dvSZ = 1.0f;
  vQuad[1].dvRHW = 1.0f;
  vQuad[1].dcColor = col;
  vQuad[1].dvTU[0] = ftx1;
  vQuad[1].dvTU[1] = fty0;
  
  vQuad[2].dvSX = x1;
  vQuad[2].dvSY = y1;
  vQuad[2].dvSZ = 1.0f;
  vQuad[2].dvRHW = 1.0f;
  vQuad[2].dcColor = col;
  vQuad[2].dvTU[0] = ftx1;
  vQuad[2].dvTU[1] = fty1;
  
  vQuad[3].dvSX = x0;
  vQuad[3].dvSY = y1;
  vQuad[3].dvSZ = 1.0f;
  vQuad[3].dvRHW = 1.0f;
  vQuad[3].dcColor = col;
  vQuad[3].dvTU[0] = ftx0;
  vQuad[3].dvTU[1] = fty1;

  m_pQuadVB->Unlock();

  dv->SetStreamSource(0, m_pQuadVB, sizeof(SPipeTRVertex_D_1T));
  dv->SetVertexShader((D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1));
  dv->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

  m_nPolygons += 2;
}

void CD3D8Renderer::DrawQuad3D(const Vec3d & v0, const Vec3d & v1, const Vec3d & v2, const Vec3d & v3,
                               const CFColor & color, float ftx0,  float fty0,  float ftx1,  float fty1,
                               float ftx2,  float fty2,  float ftx3,  float fty3)
{
  LPDIRECT3DDEVICE8 dv = mfGetD3DDevice();
  DWORD col = (DWORD(color.a * 255) << 24) | (DWORD(color.r * 255) << 16) |
              (DWORD(color.g * 255)<<8) | DWORD(color.b * 255);

  SPipeVertex_D_1T *vQuad;

#ifdef _XBOX
  HRESULT hr = m_pQuadVB->Lock(0, 0, (BYTE **) &vQuad, 0);
#else
  HRESULT hr = m_pQuadVB->Lock(0, 0, (BYTE **) &vQuad, D3DLOCK_DISCARD);
#endif

  // Define the quad
  vQuad[0].xyz.x = v0.x;
  vQuad[0].xyz.y = v0.y;
  vQuad[0].xyz.z = v0.z;
  vQuad[0].color.dcolor = col;
  vQuad[0].st[0] = ftx0;
  vQuad[0].st[1] = fty0;
  vQuad[0].st[2] = ftx2;
  vQuad[0].st[3] = fty2;

  vQuad[1].xyz.x = v1.x;
  vQuad[1].xyz.y = v1.y;
  vQuad[1].xyz.z = v1.z;
  vQuad[1].color.dcolor = col;
  vQuad[1].st[0] = ftx1;
  vQuad[1].st[1] = fty0;
  vQuad[1].st[2] = ftx3;
  vQuad[1].st[3] = fty2;

  vQuad[2].xyz.x = v2.x;
  vQuad[2].xyz.y = v2.y;
  vQuad[2].xyz.z = v2.z;
  vQuad[2].color.dcolor = col;
  vQuad[2].st[0] = ftx1;
  vQuad[2].st[1] = fty1;
  vQuad[2].st[2] = ftx3;
  vQuad[2].st[3] = fty3;

  vQuad[3].xyz.x = v3.x;
  vQuad[3].xyz.y = v3.y;
  vQuad[3].xyz.z = v3.z;
  vQuad[3].color.dcolor = col;
  vQuad[3].st[0] = ftx0;
  vQuad[3].st[1] = fty1;
  vQuad[3].st[2] = ftx2;
  vQuad[3].st[3] = fty3;

  m_pQuadVB->Unlock();

  dv->SetStreamSource(0, m_pQuadVB, STRIDE_D_1T);
  dv->SetVertexShader((D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2));
  dv->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

  m_nPolygons += 2;
}

char*	CD3D8Renderer::GetVertexProfile()
{
}

char*	CD3D8Renderer::GetPixelProfile()
{
}

//====================================================================

ILog     *iLog;
IConsole *iConsole;
ITimer   *iTimer;
ISystem  *iSystem;
int *pTest_int;
IPhysicalWorld *pIPhysicalWorld;

extern "C" DLL_EXPORT IRenderer* PackageRenderConstructor(int argc, char* argv[], SCryRenderInterface *sp);
DLL_EXPORT IRenderer* PackageRenderConstructor(int argc, char* argv[], SCryRenderInterface *sp)
{
  gbRgb = false;

  iConsole = sp->ipConsole;
  iLog = sp->ipLog;
  iTimer = sp->ipTimer;
  iSystem   = sp->ipSystem;
  pTest_int = sp->ipTest_int;
  pIPhysicalWorld = sp->pIPhysicalWorld;

#ifdef DEBUGALLOC
#undef new
#endif
  gRenDev = (CRenderer *) (new CD3D8Renderer());
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif

  srand( GetTickCount() );

  return gRenDev;
}


//=========================================================================================

