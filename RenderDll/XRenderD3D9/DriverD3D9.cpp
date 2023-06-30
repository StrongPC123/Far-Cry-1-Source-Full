/*=============================================================================
  DriverD3D9.cpp : Direct3D Render interface implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "DriverD3D9.h"
#include "D3DCGVProgram.h"
#include "D3DCGPShader.h"

#include <windows.h>

#include "IStatObj.h"

CD3D9Renderer *gcpRendD3D;

int CD3D9Renderer::CV_d3d9_texture_filter_anisotropic;
int CD3D9Renderer::CV_d3d9_nodeviceid;
int CD3D9Renderer::CV_d3d9_nvperfhud;
int CD3D9Renderer::CV_d3d9_palettedtextures;
int CD3D9Renderer::CV_d3d9_vbpools;
int CD3D9Renderer::CV_d3d9_vbpoolsize;
int CD3D9Renderer::CV_d3d9_nv30_ps20;
int CD3D9Renderer::CV_d3d9_occlusion_query;
int CD3D9Renderer::CV_d3d9_compressedtextures;
int CD3D9Renderer::CV_d3d9_usebumpmap;
int CD3D9Renderer::CV_d3d9_clipplanes;
int CD3D9Renderer::CV_d3d9_triplebuffering;
int CD3D9Renderer::CV_d3d9_resetdeviceafterloading;
int CD3D9Renderer::CV_d3d9_savedepthmaps;
int CD3D9Renderer::CV_d3d9_forcesoftware;
int CD3D9Renderer::CV_d3d9_texturebits;
int CD3D9Renderer::CV_d3d9_texmipfilter; // 0-point; 1-box; 2-linear; 3-triangle; 
int CD3D9Renderer::CV_d3d9_mipprocedures;
int CD3D9Renderer::CV_d3d9_squaretextures;
ICVar *CD3D9Renderer::CV_d3d9_device;
int CD3D9Renderer::CV_d3d9_allowsoftware;
float CD3D9Renderer::CV_d3d9_pip_buff_size;
int CD3D9Renderer::CV_d3d9_rb_verts;
int CD3D9Renderer::CV_d3d9_rb_tris;
int CD3D9Renderer::CV_d3d9_decaloffset;
int CD3D9Renderer::CV_d3d9_nodepthmaps;
float CD3D9Renderer::CV_d3d9_normalmapscale;
ICVar *CD3D9Renderer::CV_d3d9_texturefilter;

char *resourceName[] = {
    "UNKNOWN",
    "Surfaces",
    "Volumes",
    "Textures",
    "Volume Textures",
    "Cube Textures",
    "Vertex Buffers",
    "Index Buffers"
};

// Direct 3D console variables
CD3D9Renderer::CD3D9Renderer()
{
  m_bInitialized = false;
  gcpRendD3D = this;
  gRenDev = this;

  m_TexMan = new CD3D9TexMan;
  m_TexMan->m_bRGBA = false;

  m_LogFile = NULL;

  RegisterVariables();

  m_pD3D              = NULL;
  m_pd3dDevice        = NULL;
  m_hWnd              = NULL;
  m_bActive           = FALSE;
  m_bReady            = FALSE;
  m_dwCreateFlags     = 0L;
	m_pVB2D       = NULL;
  m_pVB3D[0]       = NULL;
  m_pVB3D[1]       = NULL;
  m_pVB3D[2]       = NULL;
  m_fLineWidth = 1.0f;

  m_strDeviceStats[0] = 0;

  m_MinDepthBits    = 16;
  m_MinStencilBits  = 0;
  m_eCull = (ECull)-1;
  m_Features = RFT_DIRECTACCESSTOVIDEOMEMORY | RFT_SUPPORTZBIAS;

  iConsole->Register("d3d9_Texture_Filter_Anisotropic", &CV_d3d9_texture_filter_anisotropic, 0);
  iConsole->Register("d3d9_NodeviceId", &CV_d3d9_nodeviceid, 0);
  iConsole->Register("d3d9_NVPerfHUD", &CV_d3d9_nvperfhud, 0);
  iConsole->Register("d3d9_PalettedTextures", &CV_d3d9_palettedtextures, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("d3d9_VBPools", &CV_d3d9_vbpools, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("d3d9_VBPoolSize", &CV_d3d9_vbpoolsize, 256*1024);
  iConsole->Register("d3d9_NV30_PS20", &CV_d3d9_nv30_ps20, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("d3d9_Occlusion_Query", &CV_d3d9_occlusion_query, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("d3d9_CompressedTextures", &CV_d3d9_compressedtextures, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("d3d9_UseBumpmap", &CV_d3d9_usebumpmap, 1);
  iConsole->Register("d3d9_ClipPlanes", &CV_d3d9_clipplanes, 1);
  iConsole->Register("d3d9_TripleBuffering", &CV_d3d9_triplebuffering, 0, VF_REQUIRE_APP_RESTART);
  iConsole->Register("d3d9_ResetDeviceAfterLoading", &CV_d3d9_resetdeviceafterloading, 1);
  iConsole->Register("d3d9_SaveDepthmaps", &CV_d3d9_savedepthmaps, 0);
  iConsole->Register("d3d9_ForceSoftware", &CV_d3d9_forcesoftware, 0, VF_REQUIRE_APP_RESTART);
  iConsole->Register("d3d9_TextureBits", &CV_d3d9_texturebits, 0, VF_REQUIRE_APP_RESTART);
  iConsole->Register("d3d9_MipProcedures", &CV_d3d9_mipprocedures, 0, VF_REQUIRE_APP_RESTART);
  iConsole->Register("d3d9_TexMipFilter", &CV_d3d9_texmipfilter, 1, VF_REQUIRE_LEVEL_RELOAD); // 0-point; 1-linear; 
  CV_d3d9_texturefilter = iConsole->CreateVariable("d3d9_TextureFilter", "TRILINEAR", VF_DUMPTODISK,
    "Specifies D3D specific texture filtering type.\n"
    "Usage: d3d9_TexMipFilter [TRILINEAR/BILINEAR/LINEAR/NEAREST]\n");
  iConsole->Register("d3d9_SquareTextures", &CV_d3d9_squaretextures, 0);
  CV_d3d9_device = iConsole->CreateVariable("d3d9_Device", "Auto", 0,
    "Specifies D3D specific device name.\n"
    "Usage: d3d9_Device [Auto/Second/Primary]\n");
  iConsole->Register("d3d9_AllowSoftware", &CV_d3d9_allowsoftware, 1, VF_REQUIRE_APP_RESTART);
  iConsole->Register("d3d9_pip_buff_size", &CV_d3d9_pip_buff_size, 50, VF_REQUIRE_APP_RESTART);
  iConsole->Register("d3d9_rb_Verts", &CV_d3d9_rb_verts, 2048, VF_REQUIRE_APP_RESTART);
  iConsole->Register("d3d9_rb_Tris", &CV_d3d9_rb_tris, 4096, VF_REQUIRE_APP_RESTART);
  iConsole->Register("d3d9_DecalOffset", &CV_d3d9_decaloffset, 15);
  iConsole->Register("d3d9_NoDepthMaps", &CV_d3d9_nodepthmaps, 0, VF_REQUIRE_APP_RESTART);
  iConsole->Register("d3d9_NormalMapScale", &CV_d3d9_normalmapscale, 0.15f, VF_REQUIRE_LEVEL_RELOAD);
}

CD3D9Renderer::~CD3D9Renderer()
{
  //FreeResources(FRR_ALL);
  ShutDown();
}

void  CD3D9Renderer::ShareResources( IRenderer *renderer )
{
}

void	CD3D9Renderer::MakeCurrent()
{
  if (m_CurrContext == m_RContexts[0])
    return;

  m_CurrContext = m_RContexts[0];

  CVProgram::m_LastVP = NULL;
  CPShader::m_CurRC = NULL;
}

bool CD3D9Renderer::SetCurrentContext(WIN_HWND hWnd)
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

  return true;
}

bool CD3D9Renderer::CreateContext(WIN_HWND hWnd, bool bAllowFSAA)
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
  pContext->m_hWnd = (HWND)hWnd;
  pContext->m_X = 0;
  pContext->m_Y = 0;
  pContext->m_Width = m_width;
  pContext->m_Height = m_height;
  m_CurrContext = pContext;
  m_RContexts.AddElem(pContext);

  return true;
}

bool CD3D9Renderer::DeleteContext(WIN_HWND hWnd)
{
  int i, j;

  for (i=0; i<m_RContexts.Num(); i++)
  {
    if (m_RContexts[i]->m_hWnd == hWnd)
      break;
  }
  if (i == m_RContexts.Num())
    return false;
  if (m_CurrContext == m_RContexts[i])
  {
    for (j=0; j<m_RContexts.Num(); j++)
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
// Name: CD3D9Renderer::DeleteDeviceObjects()
// Desc: Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.
//-----------------------------------------------------------------------------
HRESULT CD3D9Renderer::DeleteDeviceObjects()
{
  return S_OK;
}

void CD3D9Renderer::RegisterVariables()
{
}

void CD3D9Renderer::UnRegisterVariables()
{
}

void CD3D9Renderer::WaitForDevice()
{
  if (m_bEditor)
    return;

  int *nLost = (int *)EF_Query(EFQ_DeviceLost, 0);
  if (!*nLost)
    return;

  if (m_hWnd)
  {
    MSG msg;
    // Don't make any steps while 3D device is lost
    while (true)
    {
      while (PeekMessage(&msg, (HWND)m_hWnd, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      nLost = (int *)EF_Query(EFQ_DeviceLost, 0);
      if (!*nLost)
        break;
    }
  }
}

void CD3D9Renderer::Reset (void)
{
  if (!CV_d3d9_resetdeviceafterloading)
    return;
  HRESULT hReturn;
  m_bDeviceLost = true;
  //iLog->Log("...Reset");
  if (m_bFullScreen)
    RestoreGamma();
  if(FAILED(hReturn = Reset3DEnvironment()))
    return;
  m_bDeviceLost = false;
  if (m_bFullScreen)
    SetGamma(CV_r_gamma+m_fDeltaGamma, CV_r_brightness, CV_r_contrast, false);  
  hReturn = m_pd3dDevice->BeginScene();
}

bool CD3D9Renderer::ChangeResolution(int nNewWidth, int nNewHeight, int nNewColDepth, int nNewRefreshHZ, bool bFullScreen)
{
  HRESULT hReturn;

  iLog->Log("Change resolution: %dx%dx%d (%s)", nNewWidth, nNewHeight, nNewColDepth, bFullScreen ? "Fullscreen" : "Windowed");

  int nPrevWidth = CRenderer::m_width;
  int nPrevHeight = CRenderer::m_height;
  int nPrevColorDepth = CRenderer::m_cbpp;
  bool bPrevFullScreen = m_bFullScreen;
  if (nNewWidth < 512)
    nNewWidth = 512;
  if (nNewHeight < 300)
    nNewHeight = 300;
  if (nNewColDepth < 24)
    nNewColDepth = 16;
  else
    nNewColDepth = 32;
  if (!bFullScreen)
  {
    if (nNewWidth > m_deskwidth-16)
      nNewWidth = m_deskwidth-16;
    if (nNewHeight > m_deskheight-32)
      nNewHeight = m_deskheight-32;
  }

  // Save the new dimensions
  CRenderer::m_width  = nNewWidth;
  CRenderer::m_height = nNewHeight;
  CRenderer::m_cbpp   = nNewColDepth;
  m_bFullScreen       = bFullScreen;
  if (bFullScreen && nNewColDepth == 16)
  {
    CRenderer::m_zbpp = 16;
    CRenderer::m_sbpp = 0;
  }

  DeleteContext(m_hWnd);

  ChooseDevice();

  D3DAdapterInfo* pAdapterInfo = m_D3DSettings.PAdapterInfo();
  D3DDeviceInfo* pDeviceInfo = m_D3DSettings.PDeviceInfo();
  D3DDISPLAYMODE ModeInfo = m_D3DSettings.DisplayMode();

  // Prepare window for possible windowed/fullscreen change
  AdjustWindowForChange();

  SetRendParms(&ModeInfo, pDeviceInfo);

  // Set up the presentation parameters
  int nFlags = m_d3dpp.Flags;
  BuildPresentParamsFromSettings();
  m_d3dpp.Flags = nFlags;

  if (m_bFullScreen)
    RestoreGamma();

  m_bDeviceLost = true;
  if(FAILED(hReturn = Reset3DEnvironment()))
  {
    if (m_nRecurs)
      return false;
    m_nRecurs = 1;
    ChangeResolution(nPrevWidth, nPrevHeight, nPrevColorDepth, 0, bPrevFullScreen);
    return false;
  }
  if (!bFullScreen)
  {
    int x = (m_deskwidth-CRenderer::m_width)/2;
    int y = (m_deskheight-CRenderer::m_height)/2;
    int wdt = GetSystemMetrics(SM_CXDLGFRAME)*2 + CRenderer::m_width;
    int hgt = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXDLGFRAME)*2 + CRenderer::m_height;
    SetWindowPos(m_hWnd, HWND_NOTOPMOST, x, y, wdt, hgt, SWP_SHOWWINDOW);
  }
  // Save window properties
  GetWindowRect( m_hWnd, &m_rcWindowBounds );
  GetClientRect( m_hWnd, &m_rcWindowClient );

  hReturn = m_pd3dDevice->BeginScene();
  ICryFont *pCryFont = iSystem->GetICryFont();
  if (pCryFont)
  {
    IFFont *pFont = pCryFont->GetFont("Default");
  }
  if (m_CVWidth)
    m_CVWidth->Set(CRenderer::m_width);
  if (m_CVHeight)
    m_CVHeight->Set(CRenderer::m_height);
  if (m_CVFullScreen)
    m_CVFullScreen->Set(m_bFullScreen);
  if (m_CVColorBits)
    m_CVColorBits->Set(CRenderer::m_cbpp);
  ChangeViewport(0, 0, CRenderer::m_width, CRenderer::m_height);

  m_bDeviceLost = false;

  if (m_bFullScreen)
    SetGamma(CV_r_gamma+m_fDeltaGamma, CV_r_brightness, CV_r_contrast, true);  

  return true;
}

int CD3D9Renderer::EnumDisplayFormats(TArray<SDispFormat>& Formats, bool bReset)
{
  int i;

  if (bReset)
  {
    Formats.Free();
    return 0;
  }
  SDispFormat DF;

  D3DAdapterInfo *pAI = m_D3DSettings.PAdapterInfo();
  for (i=0; i<pAI->pDisplayModeList->Num(); i++)
  {
    D3DDISPLAYMODE *pDM = &pAI->pDisplayModeList->Get(i);
    DF.m_Width = pDM->Width;
    DF.m_Height = pDM->Height;
    DF.m_BPP = ColorBits(pDM->Format);
    if (DF.m_BPP == 24)
      DF.m_BPP = 32;
    //DF.m_RefreshRate = pDM->RefreshRate;
    Formats.AddElem(DF);
  }
  return Formats.Num();
}

/*bool CD3D9Renderer::IsSuitableDevice(int a, bool bAllowSoft)
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

      if (CV_d3d9_texturebits)
        m_TextureBits = CV_d3d9_texturebits;
      else
        m_TextureBits = CRenderer::m_cbpp;
    }
    else
    {
      if (CV_d3d9_texturebits)
        m_TextureBits = CV_d3d9_texturebits;
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
}*/

bool CD3D9Renderer::ChangeDisplay(unsigned int width,unsigned int height,unsigned int cbpp)
{
  return false;
}

void CD3D9Renderer::ChangeViewport(unsigned int x,unsigned int y,unsigned int width,unsigned int height)
{
  if (m_bDeviceLost)
    return;
  assert(m_CurrContext);
  m_CurrContext->m_X = x;
  m_CurrContext->m_Y = y;
  m_CurrContext->m_Width = width;
  m_CurrContext->m_Height = height;
  m_width = width;
  m_height = height;
  m_VWidth = width;
  m_VHeight = height;
  m_VX = x;
  m_VY = y;
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

void CD3D9Renderer::ChangeLog()
{
  if (CV_r_log && !m_LogFile)
  {
    if (CV_r_log == 3)
      SetLogFuncs(true);
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
    SetLogFuncs(false);

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

  if (CV_r_logTexStreaming && !m_LogFileStr)
  {
    m_LogFileStr = fxopen ("Direct3DLogStreaming.txt", "w");
    if (m_LogFileStr)
    {      
      iLog->Log("Direct3D texture streaming log file '%s' opened\n", "Direct3DLogStreaming.txt");
      char time[128];
      char date[128];
      
      _strtime( time );
      _strdate( date );
      
      fprintf(m_LogFileStr, "\n==========================================\n");
      fprintf(m_LogFileStr, "Direct3D Textures streaming Log file opened: %s (%s)\n", date, time);
      fprintf(m_LogFileStr, "==========================================\n");
    }
  }
  else
  if (!CV_r_logTexStreaming && m_LogFileStr)
  {
    char time[128];
    char date[128];
    _strtime( time );
    _strdate( date );
    
    fprintf(m_LogFileStr, "\n==========================================\n");
    fprintf(m_LogFileStr, "Direct3D Textures streaming Log file closed: %s (%s)\n", date, time);
    fprintf(m_LogFileStr, "==========================================\n");
    
    fclose(m_LogFileStr);
    m_LogFileStr = NULL;
    iLog->Log("Direct3D texture streaming log file '%s' closed\n", "Direct3DLogStreaming.txt");
  }
}

void CD3D9Renderer::BeginFrame()
{
  //////////////////////////////////////////////////////////////////////
  // Set up everything so we can start rendering
  //////////////////////////////////////////////////////////////////////

  assert(m_pd3dDevice);

	g_bProfilerEnabled = iSystem->GetIProfileSystem()->IsProfiling();
  
  PROFILE_FRAME(Screen_Begin);

  //////////////////////////////////////////////////////////////////////
  // Build the matrices
  //////////////////////////////////////////////////////////////////////
  
  m_matView->LoadIdentity();
  m_pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)m_matView->GetTop());
  
  if (!m_bDeviceLost)
  {
    if (CV_r_gamma+m_fDeltaGamma != m_fLastGamma || CV_r_brightness != m_fLastBrightness || CV_r_contrast != m_fLastContrast)
      SetGamma(CV_r_gamma+m_fDeltaGamma, CV_r_brightness, CV_r_contrast, false);  
  }
  if (CV_r_fsaa != m_FSAA || CV_r_fsaa_samples != m_FSAA_samples || CV_r_fsaa_quality != m_FSAA_quality)
  {
    if (m_bEditor && (m_Features & RFT_SUPPORTFSAA))
    {
      m_FSAA = CV_r_fsaa;
      if (!m_FSAA)
        m_pd3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
      else
        m_pd3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
    }
    else
    if (!m_bEditor)
    {
      if (CV_r_fsaa)
      {
        TArray<SAAFormat> Formats;
        int nNum = GetAAFormat(Formats, false);
        iLog->Log(" Full scene AA: Enabled: %s (%d Quality)\n", Formats[nNum].szDescr, Formats[nNum].nQuality);
        bool bChanged = false;
        if (Formats[nNum].nQuality != m_FSAA_quality || Formats[nNum].nSamples != m_FSAA_samples)
        {
          bChanged = true;
          ICVar *pVar = iConsole->GetCVar("r_FSAA_quality");
          if (pVar)
            pVar->Set(Formats[nNum].nQuality);
          pVar = iConsole->GetCVar("r_FSAA_samples");
          if (pVar)
            pVar->Set(Formats[nNum].nSamples);
        }
        else
        if (m_FSAA != CV_r_fsaa)
          bChanged = true;
        GetAAFormat(Formats, true);
        if (bChanged)
          ChangeResolution(m_CVWidth->GetIVal(), m_CVHeight->GetIVal(), m_CVColorBits->GetIVal(), 75, m_CVFullScreen->GetIVal()!=0);
      }
      else
      if (CV_r_fsaa != m_FSAA)
      {
        ChangeResolution(m_CVWidth->GetIVal(), m_CVHeight->GetIVal(), m_CVColorBits->GetIVal(), 75, m_CVFullScreen->GetIVal()!=0);
        iLog->Log(" Full scene AA: Disabled\n");
      }
    }
    m_FSAA = CV_r_fsaa;
    m_FSAA_quality = CV_r_fsaa_quality;
    m_FSAA_samples = CV_r_fsaa_samples;
  }
  bool bChanged2X = false;
  bool bChanged30 = false;
  ICVar *var;
  if (CV_r_sm30path != m_sm30path)
  {
    bChanged30 = true;
    m_sm30path = CV_r_sm30path;
    if (m_sm30path >= 0 && !m_bDeviceSupports_PS30)
    {
      iLog->Log("Device doesn't support pixel shaders 3.0 (or it's disabled)");
      var = iConsole->GetCVar("r_SM30PATH");
      if (var)
        var->Set(0);
      m_sm30path = 0;
    }
    if (m_sm30path >= 0)
    {
      var = iConsole->GetCVar("r_NoPS30");
      if (var)
        var->Set(m_sm30path ? 0 : 1);
    }
  }
  if (CV_r_sm2xpath != m_sm2xpath)
  {
    bChanged2X = true;
    m_sm2xpath = CV_r_sm2xpath;
    if (m_sm2xpath >= 0 && !m_bDeviceSupports_PS2X)
    {
      int nGPU = m_Features & RFT_HW_MASK;
      if (nGPU == RFT_HW_GFFX)
        iLog->Log("Device doesn't support pixel shaders 2.0a (or it's disabled)");
      else
        iLog->Log("Device doesn't support pixel shaders 2.0b (or it's disabled)");
      var = iConsole->GetCVar("r_SM2XPATH");
      if (var)
        var->Set(0);
      m_sm2xpath = 0;
    }
    if (m_sm2xpath >= 0)
    {
      var = iConsole->GetCVar("r_NoPS2X");
      if (var)
        var->Set(m_sm2xpath ? 0 : 1);
    }
  }
  if (!m_bDeviceSupportsInstancing)
  {
    if (CV_r_geominstancing)
    {
      iLog->Log("Device doesn't support HW geometry instancing (or it's disabled)");
      var = iConsole->GetCVar("r_GeomInstancing");
      if (var)
        var->Set(0);
    }
  }
  else
  {
    if (bChanged2X && m_sm2xpath >= 0)
    {
      var = iConsole->GetCVar("r_GeomInstancing");
      if (var)
        var->Set(m_sm2xpath ? 1 : 0);
    }
    else
    if (bChanged30 && m_sm30path >= 0)
    {
      var = iConsole->GetCVar("r_GeomInstancing");
      if (var)
        var->Set(m_sm30path ? 1 : 0);
    }
  }

  if (CV_r_reloadshaders)
  {
    m_cEF.mfReloadAllShaders(CV_r_reloadshaders);
    CV_r_reloadshaders = 0;
  }
  if (CV_r_nops30 != m_NoPS30)
  {
    m_NoPS30 = CV_r_nops30;
    if (m_bDeviceSupports_PS30)
    {
      if (m_NoPS30)
      {
        m_nEnabled_PS30 = 0;
        m_Features &= ~RFT_HW_PS30;
      }
      else
      {
        m_nEnabled_PS30 = 1;
        m_Features |= RFT_HW_PS30;
      }
      m_cEF.mfReloadAllShaders(FRO_SHADERS | FRO_FORCERELOAD);
      CV_r_reloadshaders = 0;
    }
    else
    {
      iLog->Log("Device doesn't support pixel shaders 3.0 (or it's disabled)");
      ICVar *var = iConsole->GetCVar("r_SM30PATH");
      if (var)
        var->Set(0);
      m_sm30path = 0;
    }
  }
  if (CV_r_nops2x != m_NoPS2X)
  {
    m_NoPS2X = CV_r_nops2x;
    if (m_bDeviceSupports_PS2X)
    {
      if (m_NoPS2X)
        m_nEnabled_PS2X = 0;
      else
        m_nEnabled_PS2X = 1;
      m_cEF.mfReloadAllShaders(FRO_SHADERS | FRO_FORCERELOAD);
      CV_r_reloadshaders = 0;
    }
    else
    {
      if ((GetFeatures() & RFT_HW_MASK) == RFT_HW_GFFX)
        iLog->Log("Device doesn't support pixel shaders 2.0a (or it's disabled)");
      else
        iLog->Log("Device doesn't support pixel shaders 2.0b (or it's disabled)");
      ICVar *var = iConsole->GetCVar("r_SM2XPATH");
      if (var)
        var->Set(0);
      m_sm2xpath = 0;
    }
  }

  if (CV_r_vsync != m_VSync)
  {
    m_VSync = CV_r_vsync;
		EnableVSync(m_VSync?true:false);
  }
  if (!m_bEditor)
  {
    if (m_CVWidth && m_CVHeight && m_CVFullScreen && m_CVColorBits)
    {
      if (m_CVWidth->GetIVal() != CRenderer::m_width || m_CVHeight->GetIVal() != CRenderer::m_height || m_CVFullScreen->GetIVal() != (int)m_bFullScreen || m_CVColorBits->GetIVal() != CRenderer::m_cbpp)
        ChangeResolution(m_CVWidth->GetIVal(), m_CVHeight->GetIVal(), m_CVColorBits->GetIVal(), 75, m_CVFullScreen->GetIVal()!=0);
    }
  }

  gRenDev->m_cEF.mfBeginFrame();

  if (CV_r_PolygonMode!=m_polygon_mode)
    SetPolygonMode(CV_r_PolygonMode);	

  m_bWasCleared = false;
//  EF_ClearBuffer(false, NULL);

  //////////////////////////////////////////////////////////////////////
  // Begin the scene
  //////////////////////////////////////////////////////////////////////

  SetMaterialColor(1,1,1,1);

  if (strcmp(CV_d3d9_texturefilter->GetString(), m_TexMan->m_CurTexFilter) || CV_r_texture_anisotropic_level != m_TexMan->m_CurAnisotropic)
    m_TexMan->SetFilter(CV_d3d9_texturefilter->GetString());

  ChangeLog ();

  ResetToDefault();

  if (CRenderer::CV_r_logTexStreaming)
  {
    LogStrv(0, "******************************* EndFrame ********************************\n");
    LogStrv(0, "Loaded: %.3f Kb, UpLoaded: %.3f Kb, UploadTime: %.3fMs\n\n", gRenDev->m_TexMan->m_LoadBytes/1024.0f, gRenDev->m_TexMan->m_UpLoadBytes/1024.0f, m_RP.m_PS.m_fTexUploadTime);
  }

  if (gRenDev->m_TexMan->m_LoadBytes > 3.0f*1024.0f*1024.0f)
    gRenDev->m_TexMan->m_fStreamDistFactor = min(2048.0f, gRenDev->m_TexMan->m_fStreamDistFactor*1.2f);
  else
    gRenDev->m_TexMan->m_fStreamDistFactor = max(1.0f, gRenDev->m_TexMan->m_fStreamDistFactor/1.2f);
  gRenDev->m_TexMan->m_UpLoadBytes = 0;
  gRenDev->m_TexMan->m_LoadBytes = 0;

  m_MatDepth = 0;
  m_nPolygons = 0;
  m_nFrameID++;
  m_nFrameUpdateID++;
  m_nShadowVolumePolys=0;
  m_RP.m_RealTime = iTimer->GetCurrTime();
  m_RP.m_PersFlags &= ~RBPF_HDR;

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
    ReleaseIndexBuffer(&re->m_Inds);
  }
  tm->SetUse(0);
  m_RP.m_CurTempMeshes = tm;

  if (CRenderer::CV_r_log)
    Logv(0, "******************************* BeginFrame %d ********************************\n", m_nFrameID);
 
  if (CRenderer::CV_r_logTexStreaming)
    LogStrv(0, "******************************* BeginFrame %d ********************************\n", m_nFrameID);

  FlushHardware();
  CheckDeviceLost();
}

bool CD3D9Renderer::CheckDeviceLost()
{
  HRESULT hReturn;

  // Test the cooperative level to see if it's okay to render
  if (FAILED(hReturn = m_pd3dDevice->TestCooperativeLevel()))
  {
    // If the device was lost, do not render until we get it back
    if (D3DERR_DEVICELOST == hReturn)
    {
      RestoreGamma();
      m_bDeviceLost = true;
      return true;
    }
    // Check if the device needs to be reset.
    if(D3DERR_DEVICENOTRESET == hReturn)
    {
      m_bDeviceLost = true;
      if(FAILED(hReturn = Reset3DEnvironment()))
        return true;
      m_bDeviceLost = false;
      SetGamma(CV_r_gamma+m_fDeltaGamma, CV_r_brightness, CV_r_contrast, true);  
      hReturn = m_pd3dDevice->BeginScene();
    }
  }
  return false;
}

void CD3D9Renderer::FlushHardware()
{
  if (m_bDeviceLost)
    return;

  HRESULT hr;
  if (CV_r_flush)
  {
    if (m_pQuery)
    {
      BOOL bQuery = false;
      double time = sCycles2();
      bool bInfinite = false;
      do
      {
        double dif = sCycles2()+34-time;
        if (dif*1000.0*g_SecondsPerCycle > 5000)
        {
          // 5 seconds in the loop
          bInfinite = true;
          break;
        }
        hr = m_pQuery->GetData((void *)&bQuery, sizeof(BOOL), D3DGETDATA_FLUSH);      	
      } while(hr == S_FALSE);

      if (bInfinite)
        iLog->Log("Error: Seems like infinite loop in GPU sync query");

      m_pQuery->Issue(D3DISSUE_END);
    }
    else
    {
      IDirect3DSurface9 * pTar = mfGetBackSurface();
      if (pTar)
      {
        D3DLOCKED_RECT lockedRect;
        RECT sourceRect;
        sourceRect.bottom = 1;
        sourceRect.top = 0;
        sourceRect.left = 0;
        sourceRect.right = 4;
        hr = pTar->LockRect(&lockedRect,&sourceRect,D3DLOCK_READONLY);
        if (!FAILED(hr))
        {
          volatile unsigned long a;
          memcpy((void *)&a,(unsigned char*)lockedRect.pBits,sizeof(a));
          hr = pTar->UnlockRect();
        }
      }
    }
  }
}

void CD3D9Renderer::Update()
{
  //////////////////////////////////////////////////////////////////////
  // End the scene and update
  //////////////////////////////////////////////////////////////////////

  // Check for the presence of a D3D device
  assert(m_pd3dDevice);

  if (!m_SceneRecurseCount)
  {
    iLog->Log("EndScene without BeginScene\n");
    return;
  }

  int i;

  {
    //IDirect3DSurface9 * pTar = gcpRendD3D->mfGetBackSurface();
    //D3DLOCKED_RECT rc;
    //HRESULT ddrval = pTar->LockRect( &rc, NULL, D3DLOCK_READONLY );
    //ddrval = pTar->UnlockRect();
  }

  PROFILE_FRAME(Screen_Update);
  HRESULT hReturn;

  m_prevCamera = m_cam;
  if (CV_r_showtimegraph)
  {
    static byte *fg;
    static float fPrevTime = iTimer->GetCurrTime();
    static int sPrevWidth = 0;
    static int sPrevHeight = 0;
    static int nC;

    float fCurTime = iTimer->GetCurrTime();
    float frametime = fCurTime - fPrevTime;
    fPrevTime = fCurTime;
    int wdt = m_width;
    int hgt = m_height;

    if (sPrevHeight != hgt || sPrevWidth != wdt)
    {
      if (fg)
      {
        delete [] fg;
        fg = NULL;
      }
      sPrevWidth = wdt;
      sPrevHeight = hgt;
    }

    if (!fg)
    {
      fg = new byte[wdt];
      memset(fg, -1, wdt);
    }

    int type = CV_r_showtimegraph;
    float f;
    float fScale;
    if (type > 2)
    {
      type = 2;
      fScale = (float)CV_r_showtimegraph / 1000.0f;
    }
    else
      fScale = 0.1f;
    f = frametime / fScale;
    f = 255.0f - CLAMP(f*255.0f, 0, 255.0f);
    if (fg)
    {
      fg[nC] = (byte)f;
      Graph(fg, 0, hgt-280, wdt, 256, nC, type, "Frame Time", Col_Green, fScale);
    }
    nC++;
    if (nC == wdt)
      nC = 0;
  }
  else
  if (CV_r_showtextimegraph)
  {
    static byte *fgUpl;
    static byte *fgStreamSync;
    static byte *fgTimeUpl;
    static byte *fgDistFact;
    static byte *fgCustMip;
    static byte *fgTotalMem;
    static byte *fgCurMem;

    static float fScaleUpl = 1;  // in Mb
    static float fScaleStreamSync = 1;  // in Mb
    static float fScaleTimeUpl = 75;  // in Ms
    static float fScaleDistFact = 200;  // in Meters
    static float fScaleCustMip = 4;        // in Mips levels
    static FLOAT fScaleTotalMem = 0;  // in Mb
    static float fScaleCurMem = 80;   // in Mb

    static CFColor ColUpl = Col_White;
    static CFColor ColStreamSync = Col_Cyan;
    static CFColor ColTimeUpl = Col_SeaGreen;
    static CFColor ColDistFact = Col_Orchid;
    static CFColor ColCustMip = Col_Orange;
    static CFColor ColTotalMem = Col_Red;
    static CFColor ColCurMem = Col_Yellow;

    static int sMask = -1;
    if (GetAsyncKeyState('1') & 0x1)
      sMask ^= 1;
    if (GetAsyncKeyState('2') & 0x1)
      sMask ^= 2;
    if (GetAsyncKeyState('3') & 0x1)
      sMask ^= 4;
    if (GetAsyncKeyState('4') & 0x1)
      sMask ^= 8;
    if (GetAsyncKeyState('5') & 0x1)
      sMask ^= 16;
    if (GetAsyncKeyState('6') & 0x1)
      sMask ^= 32;
    if (GetAsyncKeyState('7') & 0x1)
      sMask ^= 64;

    if (!fScaleTotalMem)
      fScaleTotalMem = (float)CRenderer::CV_r_texturesstreampoolsize;

    static float fPrevTime = iTimer->GetCurrTime();
    static int sPrevWidth = 0;
    static int sPrevHeight = 0;
    static int nC;

    int wdt = m_width;
    int hgt = m_height;
    int type = 3;

    if (sPrevHeight != hgt || sPrevWidth != wdt)
    {
      SAFE_DELETE_ARRAY(fgUpl);
      SAFE_DELETE_ARRAY(fgStreamSync);
      SAFE_DELETE_ARRAY(fgTimeUpl);
      SAFE_DELETE_ARRAY(fgDistFact);
      SAFE_DELETE_ARRAY(fgCustMip);
      SAFE_DELETE_ARRAY(fgTotalMem);
      SAFE_DELETE_ARRAY(fgCurMem);
      sPrevWidth = wdt;
      sPrevHeight = hgt;
    }

    if (!fgUpl)
    {
      fgUpl = new byte[wdt];
      memset(fgUpl, -1, wdt);
      fgStreamSync = new byte[wdt];
      memset(fgStreamSync, -1, wdt);
      fgTimeUpl = new byte[wdt];
      memset(fgTimeUpl, -1, wdt);
      fgDistFact = new byte[wdt];
      memset(fgDistFact, -1, wdt);
      fgCustMip = new byte[wdt];
      memset(fgCustMip, -1, wdt);
      fgTotalMem = new byte[wdt];
      memset(fgTotalMem, -1, wdt);
      fgCurMem = new byte[wdt];
      memset(fgCurMem, -1, wdt);
    }
    CXFont  *cf = iConsole->GetFont();

    Set2DMode(true, m_width, m_height);
    CFColor col = Col_White;
    int num = gRenDev->m_TexMan->m_Text_White->GetTextureID();
    DrawImage((float)nC, (float)(hgt-280), 1, 256, num, 0, 0, 1, 1, col.r, col.g, col.b, col.a);
    Set2DMode(false, m_width, m_height);

    float f;
    if (sMask & 1)
    {
      f = (gRenDev->m_TexMan->m_UpLoadBytes/1024.0f/1024.0f) / fScaleUpl;
      f = 255.0f - CLAMP(f*255.0f, 0, 255.0f);
      fgUpl[nC] = (byte)f;
      Graph(fgUpl, 0, hgt-280, wdt, 256, nC, type, NULL, ColUpl, fScaleUpl);
      col = ColUpl;
      WriteXY(cf,4,hgt-280, 0.5f,1,col.r,col.g,col.b,1, "UploadMB (%d-%d)", (int)(gRenDev->m_TexMan->m_UpLoadBytes/1024.0f/1024.0f), (int)fScaleUpl);
    }

    if (sMask & 2)
    {
      f = m_RP.m_PS.m_fTexUploadTime / fScaleTimeUpl;
      f = 255.0f - CLAMP(f*255.0f, 0, 255.0f);
      fgTimeUpl[nC] = (byte)f;
      Graph(fgTimeUpl, 0, hgt-280, wdt, 256, nC, type, NULL, ColTimeUpl, fScaleTimeUpl);
      col = ColTimeUpl;
      WriteXY(cf,4,hgt-280+16, 0.5f,1,col.r,col.g,col.b,1, "Upload Time (%.3fMs - %.3fMs)", m_RP.m_PS.m_fTexUploadTime, fScaleTimeUpl);
    }

    if (sMask & 4)
    {
      f = (gRenDev->m_TexMan->m_LoadBytes/1024.0f/1024.0f) / fScaleStreamSync;
      f = 255.0f - CLAMP(f*255.0f, 0, 255.0f);
      fgStreamSync[nC] = (byte)f;
      Graph(fgStreamSync, 0, hgt-280, wdt, 256, nC, type, NULL, ColStreamSync, fScaleStreamSync);
      col = ColStreamSync;
      WriteXY(cf,4,hgt-280+16*2, 0.5f,1,col.r,col.g,col.b,1, "StreamMB (%d-%d)", (int)(gRenDev->m_TexMan->m_LoadBytes/1024.0f/1024.0f), (int)fScaleStreamSync);
    }

    if (sMask & 8)
    {
      f = gRenDev->m_TexMan->m_fStreamDistFactor / fScaleDistFact;
      f = 255.0f - CLAMP(f*255.0f, 0, 255.0f);
      fgDistFact[nC] = (byte)f;
      Graph(fgDistFact, 0, hgt-280, wdt, 256, nC, type, NULL, ColDistFact, fScaleDistFact);
      col = ColDistFact;
      WriteXY(cf,4,hgt-280+16*3, 0.5f,1,col.r,col.g,col.b,1, "Dist Factor (Upload) (%.3f-%d)", gRenDev->m_TexMan->m_fStreamDistFactor, (int)fScaleDistFact);
    }

    if (sMask & 16)
    {
      f = m_TexMan->m_nCustomMip / fScaleCustMip;
      f = 255.0f - CLAMP(f*255.0f, 0, 255.0f);
      fgCustMip[nC] = (byte)f;
      Graph(fgCustMip, 0, hgt-280, wdt, 256, nC, type, NULL, ColCustMip, fScaleCustMip);
      col = ColCustMip;
      WriteXY(cf,4,hgt-280+16*4, 0.5f,1,col.r,col.g,col.b,1, "Custom Mip (Thrash) (%d-%d)", m_TexMan->m_nCustomMip, (int)fScaleCustMip);
    }

    if (sMask & 32)
    {
      f = (gRenDev->m_TexMan->m_StatsCurTexMem/1024.0f/1024.0f) / fScaleTotalMem;
      f = 255.0f - CLAMP(f*255.0f, 0, 255.0f);
      fgTotalMem[nC] = (byte)f;
      Graph(fgTotalMem, 0, hgt-280, wdt, 256, nC, type, NULL, ColTotalMem, fScaleTotalMem);
      col = ColTotalMem;
      WriteXY(cf,4,hgt-280+16*5, 0.5f,1,col.r,col.g,col.b,1, "Cur Pool Size (Mb) (%d-%d)", (int)(gRenDev->m_TexMan->m_StatsCurTexMem/1024.0f/1024.0f), (int)fScaleTotalMem);
    }
    if (sMask & 64)
    {
      f = (m_RP.m_PS.m_TexturesSize/1024.0f/1024.0f) / fScaleCurMem;
      f = 255.0f - CLAMP(f*255.0f, 0, 255.0f);
      fgCurMem[nC] = (byte)f;
      Graph(fgCurMem, 0, hgt-280, wdt, 256, nC, type, NULL, ColCurMem, fScaleCurMem);
      col = ColCurMem;
      WriteXY(cf,4,hgt-280+16*6, 0.5f,1,col.r,col.g,col.b,1, "Cur Scene Size (Mb) (%d-%d)", (int)(m_RP.m_PS.m_TexturesSize/1024.0f/1024.0f), (int)fScaleCurMem);
    }

    nC++;
    if (nC == wdt)
      nC = 0;
  }
  if (CV_r_envlightcmdebug)
  {
    SEnvTexture *cm = NULL;
    Vec3d Pos = m_cam.GetPos();
    cm = m_cEF.mfFindSuitableEnvLCMap(Pos, true, 0, 0);
    if (cm)
    {
      EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
      EF_SetState(GS_NODEPTHTEST);
      gRenDev->m_TexMan->m_Text_White->Set();
      DrawQuad(0,0,64,64,CFColor(cm->m_EnvColors[0].bcolor), 1.0f);
      DrawQuad(64,0,128,64,CFColor(cm->m_EnvColors[2].bcolor), 1.0f);
      DrawQuad(128,0,192,64,CFColor(cm->m_EnvColors[4].bcolor), 1.0f);
      DrawQuad(0,64,64,128,CFColor(cm->m_EnvColors[1].bcolor), 1.0f);
      DrawQuad(64,64,128,128,CFColor(cm->m_EnvColors[3].bcolor), 1.0f);
      DrawQuad(128,64,192,128,CFColor(cm->m_EnvColors[5].bcolor), 1.0f);
      PrintToScreen(5,5,4,"Pos X");
      PrintToScreen(5+64,5,4,"Pos Y");
      PrintToScreen(5+128,5,4,"Pos Z");
      PrintToScreen(5,5+64,4,"Neg X");
      PrintToScreen(5+64,5+64,4,"Neg Y");
      PrintToScreen(5+128,5+64,4,"Neg Z");
    }
  }

  double time = 0;
  ticks(time);

  if (CV_r_measureoverdraw)
  {
    gRenDev->Set2DMode(true, 800, 600);
    int nOffs;
    struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);
    vQuad[0].xyz.x = 0;
    vQuad[0].xyz.y = 0;
    vQuad[0].xyz.z = 1;
    vQuad[0].color.dcolor = -1;
    vQuad[0].st[0] = 0;
    vQuad[0].st[1] = 0;

    vQuad[1].xyz.x = 800;
    vQuad[1].xyz.y = 0;
    vQuad[1].xyz.z = 1;
    vQuad[1].color.dcolor = -1;
    vQuad[1].st[0] = 1;
    vQuad[1].st[1] = 0;

    vQuad[2].xyz.x = 800;
    vQuad[2].xyz.y = 600;
    vQuad[2].xyz.z = 1;
    vQuad[2].color.dcolor = -1;
    vQuad[2].st[0] = 1;
    vQuad[2].st[1] = 1;

    vQuad[3].xyz.x = 0;
    vQuad[3].xyz.y = 600;
    vQuad[3].xyz.z = 1;
    vQuad[3].color.dcolor = -1;
    vQuad[3].st[0] = 0;
    vQuad[3].st[1] = 1;

    m_RP.m_PersFlags &= ~RBPF_MEASUREOVERDRAW;
    SetCullMode(R_CULL_DISABLE);
    EF_SetState(GS_NODEPTHTEST);
    EnableTMU(true);
    gRenDev->m_TexMan->m_Text_White->Set();
    UnlockVB3D();
    // Bind our vertex as the first data stream of our device
    m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);
    EF_SetGlobalColor(0,0,0,0);
    // Render the two triangles from the data stream
    HRESULT hr = m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

    EF_SetStencilState(STENCOP_FAIL(FSS_STENCOP_KEEP) |
                       STENCOP_ZFAIL(FSS_STENCOP_KEEP) |
                       STENCOP_PASS(FSS_STENCOP_KEEP) |
                       STENC_FUNC(FSS_STENCFUNC_LEQUAL),
                       0, -1);

    EF_SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST | GS_STENCIL);

    float fColor = 0.015625f;
    EF_SetGlobalColor(fColor,fColor,fColor,fColor);
    float fStencil = 1;
    float fStencInc = 1;
    for (int i=0; i<64; i++)
    {
      m_pd3dDevice->SetRenderState(D3DRS_STENCILREF, (int)fStencil);

      // Render the two triangles from the data stream
      hr = m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);
      fStencil += fStencInc;
    }
    m_pd3dDevice->SetRenderState(D3DRS_STENCILREF, 0);
  }

  if (iConsole)
  { 
    CRenderer *crend=gRenDev;
    CXFont    *cf=iConsole->GetFont();
    /*if (CV_r_sm30path > 0)
    {
      CFColor col = Col_Yellow;
#ifdef USE_HDR
      if (CV_r_hdrrendering)
        crend->Draw2dLabel(300,10, 2.0f, &col[0], true, "SM30 HDR Path Beta");
      else
#endif
        crend->Draw2dLabel(300,10, 2.0f, &col[0], true, "SM30 Path Beta");
    }
    else
    if (CV_r_sm2xpath > 0)
    {
      CFColor col = Col_Yellow;
#ifdef USE_HDR
      if (CV_r_hdrrendering)
      {
        if ((gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_GFFX)
          crend->Draw2dLabel(300,10, 2.0f, &col[0], true, "SM20b HDR Path Beta");
        else
          crend->Draw2dLabel(300,10, 2.0f, &col[0], true, "SM20a HDR Path Beta");
      }
      else
#endif
      {
        if ((gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_GFFX)
          crend->Draw2dLabel(300,10, 2.0f, &col[0], true, "SM20a Path Beta");
        else
          crend->Draw2dLabel(300,10, 2.0f, &col[0], true, "SM20b Path Beta");
      }
    }*/

    switch (CV_r_stats)
    {
      case 1:
        crend->WriteXY(cf,10,270, 0.5f,1,1,1,1,1, "Unique Render Items=%d (%d)",m_RP.m_PS.m_NumRendItems, m_RP.m_PS.m_NumRendBatches);
        crend->WriteXY(cf,10,335, 0.5f,1,1,1,1,1, "Unique CVShaders=%d",m_RP.m_PS.m_NumVShaders);
        crend->WriteXY(cf,10,350, 0.5f,1,1,1,1,1, "Unique CPShaders=%d",m_RP.m_PS.m_NumPShaders);
        crend->WriteXY(cf,10,365, 0.5f,1,1,1,1,1,"Unique Textures=%d",m_RP.m_PS.m_NumTextures);
        crend->WriteXY(cf,10,385, 0.5f,1,1,1,1,1,"TexturesSize=%.03f Kb",m_RP.m_PS.m_TexturesSize/1024.0f);
        crend->WriteXY(cf,10,400, 0.5f,1,1,1,1,1,"Mesh update=%.03f Kb",m_RP.m_PS.m_MeshUpdateBytes/1024.0f);
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
          for (i=0; i<SShader::m_ShaderResources_known.Num(); i++)
          {
            SRenderShaderResources *pSR = SShader::m_ShaderResources_known[i];
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
            if (CVProgram::m_VPrograms[i])
            {
              nSize += CVProgram::m_VPrograms[i]->Size();
              n++;
            }
          }
          crend->WriteXY(cf,550,205, 0.5f,1,1,1,1,1,"VProgramms: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          for (i=0; i<CPShader::m_PShaders.Num(); i++)
          {
            if (CPShader::m_PShaders[i])
            {
              nSize += CPShader::m_PShaders[i]->Size();
              n++;
            }
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
          int nSizeVid = 0;
          int nSizeInds = 0;
          n = 0;
          int nVB = 0;
          int nVI = 0;
          CLeafBuffer *pLB = CLeafBuffer::m_RootGlobal.m_NextGlobal;
          while (pLB != &CLeafBuffer::m_RootGlobal)
          {
            n++;
            nSize += pLB->Size(0);
            if (pLB->m_pVertexBuffer)
            {
              nSizeVid += pLB->m_pVertexBuffer->Size(0, pLB->m_SecVertCount);
              nVB++;
            }
            if (pLB->m_Indices.m_VertBuf.m_pPtr)
            {
              nSizeInds += pLB->m_Indices.m_nItems*2;
              nVI++;
            }
            pLB = pLB->m_NextGlobal;
          }
          crend->WriteXY(cf,550,280, 0.5f,1,1,1,1,1,"Mesh (LB): %d, SysSize=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);
          crend->WriteXY(cf,550,295, 0.5f,1,1,1,1,1,"VidBuf: %d, Size: %0.3fMb", nVB, (float)nSizeVid/1024.0f/1024.0f);
          crend->WriteXY(cf,550,310, 0.5f,1,1,1,1,1,"VidInds: %d, Size: %0.3fMb", nVI, (float)nSizeInds/1024.0f/1024.0f);

          nSize = 0;
          n = 0;
          CRendElement *pRE = CRendElement::m_RootGlobal.m_NextGlobal;
          while (pRE != &CRendElement::m_RootGlobal)
          {
            n++;
            nSize += pRE->Size();
            pRE = pRE->m_NextGlobal;
          }
          crend->WriteXY(cf,550,325, 0.5f,1,1,1,1,1,"Rend. Elements: %d, size=%0.3fMb",n,(float)nSize/1024.0f/1024.0f);

          nSize = 0;
          int nObjSize = 0;
          n = 0;
          for (i=0; i<gRenDev->m_TexMan->m_Textures.Num(); i++)
          {
            STexPic *tp = gRenDev->m_TexMan->m_Textures[i];
            if (!tp || tp->m_Bind == TX_FIRSTBIND)
              continue;
            n++;
            nObjSize += tp->Size(0);
            nSize += tp->m_Size;
          }
          crend->WriteXY(cf,550,350, 0.5f,1,1,1,1,1,"Textures: %d, ObjSize=%0.3fMb",n,(float)nObjSize/1024.0f/1024.0f);
          crend->WriteXY(cf,550,365, 0.5f,1,1,1,1,1,"TexturesDataSize=%.03f Mb",nSize/1024.0f/1024.0f);
          crend->WriteXY(cf,550,380, 0.5f,1,1,1,1,1,"CurTexturesDataSize=%.03f Mb",m_RP.m_PS.m_TexturesSize/1024.0f/1024.0f);
          crend->WriteXY(cf,550,405, 0.5f,1,1,1,1,1,"Mesh update=%.03f Kb",m_RP.m_PS.m_MeshUpdateBytes/1024.0f);
        }
        break;
    }
  }

  m_TexMan->Update();    

  if (CV_r_profileshaders)
    EF_PrintProfileInfo();

  EF_SetState(GS_DEPTHWRITE);

  // draw debug bboxes and lines
  std :: vector<BBoxInfo>::iterator itBBox = m_arrBoxesToDraw.begin(), itBBoxEnd = m_arrBoxesToDraw.end();
  for( ; itBBox != itBBoxEnd; ++itBBox)
  {
    BBoxInfo& rBBox = *itBBox;
    SetMaterialColor( rBBox.fColor[0], rBBox.fColor[1],
      rBBox.fColor[2], rBBox.fColor[3] );

    // set blend for transparent objects
    if(rBBox.fColor[3]!=1.f)
    {
      m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
      m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
      m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    }

    switch(rBBox.nPrimType)
    {
    case DPRIM_LINE:
      DrawLine(rBBox.vMins, rBBox.vMaxs);
      break;

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
    }

    if(rBBox.fColor[3]!=1.f)
    {
      m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
      m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    }
  }

  EF_SetState(GS_NODEPTHTEST);

  m_arrBoxesToDraw.clear();

	{ // print shadow volume stats
		static ICVar *pVar = iConsole->GetCVar("e_stencil_shadows");
		if(pVar && pVar->GetIVal()==3)
			CRETriMeshShadow::PrintStats();
	}

	{ // print shadow maps on the screen
		static ICVar *pVar = iConsole->GetCVar("e_shadow_maps_debug");
		if(pVar && pVar->GetIVal()==2)
			DrawAllShadowsOnTheScreen();
	}

#ifdef USE_HDR
  if (CV_r_hdrdebug)
    HDR_DrawDebug();
#endif

  /*{
    Vec3 vPos = Vec3(-50, 865, 0);
    vPos.z = CREOcean::m_pStaticOcean->GetWaterZElevation(vPos.x, vPos.y);
    float fRadius = 1;
    EF_SetState(GS_DEPTHWRITE);
    D3DSetCull(eCULL_None);
    DrawBall(vPos, fRadius);
  }*/
  {
    /*EF_SetState(GS_NODEPTHTEST);
    int iTmpX, iTmpY, iTempWidth, iTempHeight;
    GetViewport(&iTmpX, &iTmpY, &iTempWidth, &iTempHeight);   
    EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
    Set2DMode(true, 1, 1);

    SetViewport(10, 400, 100, 100);   
    DrawImage(0, 0, 1, 1, 0x12bf, 0, 1, 1, 0, 1,1,1,1);

    SetViewport(120, 400, 100, 100);   
    DrawImage(0, 0, 1, 1, 0x12cf, 0, 1, 1, 0, 1,1,1,1);
*/
    /*{
      CFurNormalMap *fnm = (CFurNormalMap *)m_TexMan->m_Text_FurNormalMap->m_pFuncMap;
      SetViewport(10, 400, 100, 100);   
      DrawImage(0, 0, 1, 1, fnm->m_pTexOffset0->m_Bind, 0, 1, 1, 0, 1,1,1,1);

      SetViewport(120, 400, 100, 100);   
      DrawImage(0, 0, 1, 1, fnm->m_pTexOffset1->m_Bind, 0, 1, 1, 0, 1,1,1,1);

      SetViewport(230, 400, 100, 100);   
      DrawImage(0, 0, 1, 1, fnm->m_pTex->m_Bind, 0, 1, 1, 0, 1,1,1,1);

      SetViewport(340, 400, 100, 100);   
      DrawImage(0, 0, 1, 1, fnm->m_pTexClamp->m_Bind, 0, 1, 1, 0, 1,1,1,1);

      SetViewport(450, 400, 100, 100);   
      DrawImage(0, 0, 1, 1, fnm->m_pTexNormalize->m_Bind, 0, 1, 1, 0, 1,1,1,1);
    }*/
    /*SetViewport(10, 400, 100, 100);   
    DrawImage(0, 0, 1, 1, m_TexMan->m_Text_ScreenMap->m_Bind, 0, 1, 1, 0, 1,1,1,1);

    SetViewport(120, 400, 100, 100);   
    DrawImage(0, 0, 1, 1, m_TexMan->m_Text_ScreenLowMap->m_Bind, 0, 1, 1, 0, 1,1,1,1);

    SetViewport(230, 400, 100, 100);   
    DrawImage(0, 0, 1, 1, m_TexMan->m_Text_ScreenAvg1x1->m_Bind, 0, 1, 1, 0, 1,1,1,1);

    SetViewport(340, 400, 100, 100);   
    DrawImage(0, 0, 1, 1, m_TexMan->m_Text_ScreenCurrLuminosityMap->m_Bind, 0, 1, 1, 0, 1,1,1,1);

    SetViewport(450, 400, 100, 100);   
    DrawImage(0, 0, 1, 1, m_TexMan->m_Text_Glare->m_Bind, 0, 1, 1, 0, 1,1,1,1);

    SetViewport(560, 400, 100, 100);   
    DrawImage(0, 0, 1, 1, m_TexMan->m_Text_PrevScreenMap->m_Bind, 0, 1, 1, 0, 1,1,1,1);*/

    //Set2DMode(false, 1, 1);
    //SetViewport(iTmpX, iTmpY, iTempWidth, iTempHeight);   
  }

  if (CRenderer::CV_r_log)
    Logv(0, "******************************* EndFrame ********************************\n");

  // End the scene
  m_pd3dDevice->EndScene();
  m_SceneRecurseCount--;

  if (CV_r_ShowVideoMemoryStats)
  {
    HRESULT hr;
    CV_r_ShowVideoMemoryStats = FALSE;
    IDirect3DQuery9 *resourceQuery;
    BOOL timeOut;

    D3DDEVINFO_RESOURCEMANAGER resourceStats;
    hr = m_pd3dDevice->CreateQuery(D3DQUERYTYPE_RESOURCEMANAGER, &resourceQuery);
    if (hr == D3D_OK)
    {
      resourceQuery->Issue(D3DISSUE_END);
    	float timeoutTime = iTimer->GetCurrTime() + 2.0f;
      timeOut = FALSE;
      // D3DGETDATA_FLUSH
      while (resourceQuery->GetData((void *)&resourceStats, sizeof(D3DDEVINFO_RESOURCEMANAGER), 0) != S_OK)
      {
        if (iTimer->GetCurrTime() < timeoutTime)
        {
          timeOut = TRUE;
          break;
        }
      }
      resourceQuery->Release();

      // make sure succeeded
      if (!timeOut)
      {
        for (i=0; i<D3DRTYPECOUNT; i++)
        {
          iLog->Log("%s - Thrashing = %s, Approx Bytes Downloaded %d, Number Evictions %d",
              resourceName[i], resourceStats.stats[i].bThrashing ? "TRUE" : "FALSE", 
              resourceStats.stats[i].ApproxBytesDownloaded, resourceStats.stats[i].NumEvicts);
              iLog->Log("    Number Video Creates %d, Last Priority %d",
                resourceStats.stats[i].NumVidCreates, resourceStats.stats[i].LastPri);
              iLog->Log("    Number set to Device %d, Number used In Vid mem %d",
                resourceStats.stats[i].NumUsed, resourceStats.stats[i].NumUsedInVidMem);

          iLog->Log("%s - %d object(s) with %d bytes in video memory.\n", 
            resourceName[i], resourceStats.stats[i].WorkingSet, resourceStats.stats[i].WorkingSetBytes);
          iLog->Log("%s - %d object(s) with %d bytes in managed memory.\n", 
            resourceName[i], resourceStats.stats[i].TotalManaged, resourceStats.stats[i].TotalBytes);
        }
      }
    }
  }


  // Flip the back buffer to the front
  if(m_bSwapBuffers)
	{
		if (!m_bEditor)
			hReturn = m_pd3dDevice->Present(NULL, NULL, NULL, NULL);
		else
		{
			RECT ClientRect;
			ClientRect.top		= 0;
			ClientRect.left		= 0;
			ClientRect.right	= m_CurrContext->m_Width;
			ClientRect.bottom	= m_CurrContext->m_Height;
			hReturn = m_pd3dDevice->Present(&ClientRect,&ClientRect,m_CurrContext->m_hWnd,NULL);
		}
	}


  hReturn = m_pd3dDevice->BeginScene();

	CheckDeviceLost();

  if (CV_r_GetScreenShot)
  {
    ScreenShot();
    CV_r_GetScreenShot = 0;
  }

  m_SceneRecurseCount++;
}



void CD3D9Renderer::ScreenShot(const char *filename)
{
  char scname[512];
  int i;

  FILE *fp;

  if (!filename)
  {           
    for (i=0 ; i<10000; i++)
    { 
			#ifdef WIN64
				sprintf(scname,"FarCry%04d.tga",i);
			#else
				sprintf(scname,"FarCry%04d.jpg",i);
			#endif

      fp = fxopen(scname,"rb");
      if (!fp)
        break; // file doesn't exist

      fclose(fp);
    }

    if (i==10000) 
    {     
      iLog->Log("Cannot save ScreenShot: Too many JPG files\n"); 
      return;
    }   
  }
  else 
    strcpy(scname,filename);

  iLog->Log("ScreenShot %s\n",scname);

  LPDIRECT3DSURFACE9 pSysDeskSurf;
  D3DLOCKED_RECT d3dlrSys;
  int wdt = m_deskwidth;
  int hgt = m_deskheight;
  if (m_bFullScreen)
  {
    wdt = m_width;
    hgt = m_height;
  }
  HRESULT h = m_pd3dDevice->CreateOffscreenPlainSurface(wdt, hgt, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSysDeskSurf, NULL);
  if (FAILED(h))
    return;
  h = m_pd3dDevice->GetFrontBufferData(0, pSysDeskSurf);
  if (FAILED(h))
    return;
  POINT WndP;
  WndP.x = 0;
  WndP.y = 0;
  ClientToScreen(m_hWnd, &WndP);  
  h = pSysDeskSurf->LockRect(&d3dlrSys, NULL, D3DLOCK_READONLY);
  if (FAILED(h))
    return;
  byte *src = (byte *)d3dlrSys.pBits;
  int height = m_height;
  int width = m_width;
  if (WndP.y < 0)
  {
    height += WndP.y;
    WndP.y = 0;
  }
  if (WndP.x < 0)
  {
    width += WndP.x;
    WndP.x = 0;
  }
  if (width+WndP.x >= wdt)
    width = wdt-WndP.x;
  if (height+WndP.y >= hgt)
    height = hgt-WndP.y;

  //iLog->Log("wdt: %d, hgt: %d, width: %d, height: %d, WndP.x: %d, WndP.y: %d", wdt, hgt, width, height, WndP.x, WndP.y);

  unsigned char *pic=new unsigned char [width*height*4];
  byte *dst = pic;
  src += WndP.y*d3dlrSys.Pitch;
  for (i=0; i<height; i++)
  {
    for (int j=0; j<width; j++)
    {
      *(uint *)&dst[j*4] = *(uint *)&src[(WndP.x+j)*4];
			Exchange(dst[j*4+0], dst[j*4+2]);
    }
    dst += width*4;
    src += d3dlrSys.Pitch;
  }
  pSysDeskSurf->UnlockRect();
  SAFE_RELEASE(pSysDeskSurf);

	#ifdef WIN64
	  //TODO: we need a lib for AMD64 to create JPEG-files
		
		//for TGA we need to flip picture before we store it
		uint32* fb1=(uint32*)pic;
		uint32* fb2=(uint32*)malloc(height*width*32);
		uint32* buffer=fb2;
		for(uint32 y=height; y>0; y--) {
			uint32* fbuf=fb1+((y-1)*width);
			for(uint32 x=0; x<width; x++) {	fb2[x]=fbuf[x];	}
			fb2+=width;
		}
		WriteTGA((uint8*)buffer, width, height, scname, 32);
		free(buffer);
	#else
		::WriteJPG(pic, width, height, scname);
	#endif

  delete [] pic;    
}

int CD3D9Renderer::CreateRenderTarget (int nWidth, int nHeight, ETEX_Format eTF)
{
  if (!m_RTargets.Num())
    m_RTargets.AddIndex(1);
  int n = m_RTargets.Num();
  for (int i=1; i<m_RTargets.Num(); i++)
  {
    if (!m_RTargets[i].m_pRT && m_RTargets[i].m_pZB)
    {
      n = i;
      break;
    }
  }
  if (n == m_RTargets.Num())
    m_RTargets.AddIndex(1);
  SD3DRenderTarget rt;
  D3DFORMAT fmt;
  switch (eTF)
  {
    case eTF_8888:
    default:
      fmt = D3DFMT_A8R8G8B8;
      break;

    case eTF_0888:
      fmt = D3DFMT_X8R8G8B8;
      break;

    case eTF_4444:
      fmt = D3DFMT_A4R4G4B4;
      break;

    case eTF_0565:
      fmt = D3DFMT_R5G6B5;
      break;
  }
  HRESULT hr = m_pd3dDevice->CreateRenderTarget(nWidth, nHeight, fmt, D3DMULTISAMPLE_NONE, 0, TRUE, &rt.m_pRT, NULL);
  if ( FAILED(hr) )
    return -1;
  hr = m_pd3dDevice->CreateDepthStencilSurface(nWidth, nHeight, m_d3dsdZBuffer.Format, D3DMULTISAMPLE_NONE, 0, FALSE, &rt.m_pZB, NULL);
  if ( FAILED(hr) )
  {
    SAFE_RELEASE(rt.m_pRT);
    return -1;
  }
  m_RTargets[n] = rt;

  return n;
}

bool CD3D9Renderer::DestroyRenderTarget (int nHandle)
{
  if (nHandle <= m_RTargets.Num())
    return false;
  SD3DRenderTarget *rt = &m_RTargets[nHandle];
  SAFE_RELEASE(rt->m_pRT);
  SAFE_RELEASE(rt->m_pZB);

  return true;
}

bool CD3D9Renderer::SetRenderTarget (int nHandle)
{
  if (nHandle == 0)
  {
    EF_RestoreRenderTarget();
    return true;
  }

  if (nHandle <= m_RTargets.Num())
    return false;
  SD3DRenderTarget *rt = &m_RTargets[nHandle];

  HRESULT hr = S_OK;
  if (rt->m_pRT != m_pCurBackBuffer)
  {
    hr = m_pd3dDevice->SetRenderTarget(0, rt->m_pRT);
    m_pCurBackBuffer = rt->m_pRT;
  }
  if (FAILED(hr))
    return false;
  if (m_pCurZBuffer != rt->m_pZB)
  {
    hr = m_pd3dDevice->SetDepthStencilSurface(rt->m_pZB);
    m_pCurZBuffer = rt->m_pZB;
  }
  if (FAILED(hr))
    return false;
  return true;
}

void CD3D9Renderer::ReadFrameBuffer(unsigned char * pRGB, int nSizeX, int nSizeY, bool bBackBuffer, bool bRGBA, int nScaledX, int nScaledY)
{
  int i;

  LPDIRECT3DSURFACE9 pSysSurf = NULL;
	LPDIRECT3DSURFACE9 pTmpSurface = NULL;
  D3DLOCKED_RECT d3dlrSys;
	D3DSURFACE_DESC desc;
  HRESULT hr;
  if (bBackBuffer)
  {
    if (!m_pCurBackBuffer)
      return;
	  hr = m_pCurBackBuffer->GetDesc(&desc);
	  if (FAILED(hr))
      return;
    POINT WndP;
    WndP.x = 0;
    WndP.y = 0;
    //if (m_bEditor)
    //  ClientToScreen(m_hWnd, &WndP);  
    RECT srcRect, dstRect;
    srcRect.left = WndP.x;
    srcRect.right = nSizeX;
    srcRect.top = WndP.y;
    srcRect.bottom = nSizeY;
    if (nScaledX <= 0)
    {
      nScaledX = nSizeX;
      nScaledY = nSizeY;
    }
    dstRect.left = 0;
    dstRect.right = nScaledX;
    dstRect.top = 0;
    dstRect.bottom = nScaledY;
		hr = m_pd3dDevice->CreateRenderTarget(nScaledX, nScaledY, desc.Format, D3DMULTISAMPLE_NONE, 0, TRUE, &pTmpSurface, NULL );
		if ( FAILED(hr) )
      return;

		hr = m_pd3dDevice->StretchRect(m_pCurBackBuffer, &srcRect, pTmpSurface, &dstRect, D3DTEXF_NONE);
		if ( FAILED(hr) )
			return;
    if (desc.Format != D3DFMT_X8R8G8B8 && desc.Format != D3DFMT_A8R8G8B8)
    {
  	  // Create a buffer the same size and format
	    hr = m_pd3dDevice->CreateOffscreenPlainSurface(nScaledX, nScaledY, desc.Format, D3DPOOL_SYSTEMMEM, &pSysSurf, NULL);
      D3DXLoadSurfaceFromSurface(pSysSurf, NULL, NULL, pTmpSurface, NULL, NULL, D3DX_FILTER_NONE, 0);
      hr = pSysSurf->LockRect(&d3dlrSys, NULL, 0);
    }
    else
      hr = pTmpSurface->LockRect(&d3dlrSys, NULL, 0);
    byte *src = (byte *)d3dlrSys.pBits;
    byte *dst = pRGB;
    if (bRGBA)
    {
      for (i=0; i<nScaledY; i++)
      {
        int ni0 = (nScaledY-i-1)*nSizeX*4;
        int ni1 = i * d3dlrSys.Pitch;
        for (int j=0; j<nScaledX; j++)
        {
          dst[ni0+j*4+0] = src[ni1+j*4+2];
          dst[ni0+j*4+1] = src[ni1+j*4+1];
          dst[ni0+j*4+2] = src[ni1+j*4+0];
          dst[ni0+j*4+3] = 255;
        }
      }
    }
    else
    {
      for (i=0; i<nScaledY; i++)
      {
        int ni0 = (nScaledY-i-1)*nScaledX*3;
        int ni1 = i * d3dlrSys.Pitch;
        for (int j=0; j<nScaledX; j++)
        {
          dst[ni0+j*3+0] = src[ni1+j*4+0];
          dst[ni0+j*3+1] = src[ni1+j*4+1];
          dst[ni0+j*3+2] = src[ni1+j*4+2];
        }
      }
    }
    if (pSysSurf)
      pSysSurf->UnlockRect();
    else
    if (pTmpSurface)
      pTmpSurface->UnlockRect();
  }
  else
  {
    hr = m_pd3dDevice->CreateOffscreenPlainSurface(m_deskwidth, m_deskheight, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSysSurf, NULL);
    hr = m_pd3dDevice->GetFrontBufferData(0, pSysSurf);
    POINT WndP;
    WndP.x = 0;
    WndP.y = 0;
    ClientToScreen(m_hWnd, &WndP);  
    hr = pSysSurf->LockRect(&d3dlrSys, NULL, 0);
    byte *src = (byte *)d3dlrSys.pBits;
    byte *dst = pRGB;
    for (i=0; i<nSizeY; i++)
    {
      int ni0 = (nSizeY-i-1)*nSizeX*4;
      for (int j=0; j<nSizeX; j++)
      {
        *(uint *)&dst[ni0+j*4] = *(uint *)&src[WndP.y*d3dlrSys.Pitch+(WndP.x+j)*4];
        Exchange(dst[ni0+j*4+0], dst[ni0+j*4+2]);
      }
      src += d3dlrSys.Pitch;
    }
    pSysSurf->UnlockRect();
  }
  SAFE_RELEASE(pTmpSurface);
  SAFE_RELEASE(pSysSurf);
}


int CD3D9Renderer::ScreenToTexture()
{
  return 0;
}

void	CD3D9Renderer::Draw2dImage(float xpos,float ypos,float w,float h,int textureid,float s0,float t0,float s1,float t1,float angle,float r,float g,float b,float a,float z)
{
  if (textureid > TX_FIRSTBIND && unsigned(textureid - TX_FIRSTBIND) >= m_TexMan->m_Textures.size())
  {
    iLog->LogError ("\001CD3D9Renderer::Draw2dImage(x=%g,y=%g,w=%g,h=%g,texid=%d): invalid texid, should be between %u and %u",
      xpos, ypos, w,h, textureid,	 TX_FIRSTBIND, m_TexMan->m_Textures.size() + TX_FIRSTBIND);
    return;
  }

  if (CV_d3d9_forcesoftware)
    return;

  if (m_bDeviceLost)
    return;

  //////////////////////////////////////////////////////////////////////
  // Draw a textured quad, texture is assumed to be in video memory
  //////////////////////////////////////////////////////////////////////

  // Check for the presence of a D3D device
  assert(m_pd3dDevice);

  if (!m_SceneRecurseCount)
  {
    iLog->Log("ERROR: CD3D9Renderer::Draw2dImage before BeginScene");
    return;
  }

  PROFILE_FRAME(Draw_2DImage);

  DWORD col = D3DRGBA(r,g,b,a);

  bool bSaveZTest = ((m_CurState & GS_NODEPTHTEST) == 0);
  SetCullMode(R_CULL_DISABLE);

  HRESULT hReturn;

  xpos = (float)ScaleCoordX(xpos); w = (float)ScaleCoordX(w);
  ypos = (float)ScaleCoordY(ypos); h = (float)ScaleCoordY(h);
  float fx = xpos-0.5f;
  float fy = ypos-0.5f;
  float fw = w;
  float fh = h;

  EF_SelectTMU(0);

  if(textureid>0)
    SetTexture(textureid);
  else
    EnableTMU(false);

  EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);

  m_matProj->Push();
  D3DXMATRIX *m = m_matProj->GetTop();
  D3DXMatrixOrthoOffCenterLH(m, 0.0f, (float)m_width, (float)m_height, 0.0f, -1e30f, 1e30f);
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
  EF_PushMatrix();
  m = m_matView->GetTop();
  m_matView->LoadIdentity();
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m);

  // Lock the entire buffer and obtain a pointer to the location where we have to
  // write the vertex data. Note that we specify zero here to lock the entire 
  // buffer.

  int nOffs;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);
  if (!vQuad)
    return;

  // Now that we have write access to the buffer, we can copy our vertices
  // into it

  if (angle!=0)
  {
    float xsub=(float)(xpos+w/2.0f);
    float ysub=(float)(ypos+h/2.0f);

    float x,y,x1,y1;
    float mcos=cry_cosf(DEG2RAD(angle));
    float msin=cry_sinf(DEG2RAD(angle));

    x=xpos-xsub;
    y=ypos-ysub;
    x1=x*mcos-y*msin;
    y1=x*msin+y*mcos;
    x1+=xsub;y1+=ysub;

    // Define the quad
    vQuad[0].xyz.x = x1;
    vQuad[0].xyz.y = y1;

    x=xpos+w-xsub;
    y=ypos-ysub;
    x1=x*mcos-y*msin;
    y1=x*msin+y*mcos;
    x1+=xsub;y1+=ysub; 

    vQuad[1].xyz.x = x1;//fx + fw;
    vQuad[1].xyz.y = y1;// fy;

    x=xpos+w-xsub;
    y=ypos+h-ysub;
    x1=x*mcos-y*msin;
    y1=x*msin+y*mcos;
    x1+=xsub;y1+=ysub; 

    vQuad[2].xyz.x = x1;//fx + fw;
    vQuad[2].xyz.y = y1;//fy + fh;

    x=xpos-xsub;
    y=ypos+h-ysub;
    x1=x*mcos-y*msin;
    y1=x*msin+y*mcos;
    x1+=xsub;y1+=ysub; 

    vQuad[3].xyz.x = x1;//fx;
    vQuad[3].xyz.y = y1;//fy + fh;
  }
  else 
  {
    // Define the quad
    vQuad[0].xyz.x = fx;
    vQuad[0].xyz.y = fy;
    vQuad[1].xyz.x = fx + fw;
    vQuad[1].xyz.y = fy;
    vQuad[2].xyz.x = fx + fw;
    vQuad[2].xyz.y = fy + fh;
    vQuad[3].xyz.x = fx;
    vQuad[3].xyz.y = fy + fh;
  }

  // set uv's
  vQuad[0].st[0] = s0;
  vQuad[0].st[1] = 1.0f-t0;
  vQuad[1].st[0] = s1;
  vQuad[1].st[1] = 1.0f-t0;
  vQuad[2].st[0] = s1;
  vQuad[2].st[1] = 1.0f-t1;
  vQuad[3].st[0] = s0;
  vQuad[3].st[1] = 1.0f-t1;

  // set data
	for(int i=0;i<4;i++)
	{
	  vQuad[i].color.dcolor = col;
		vQuad[i].xyz.z = z;
	}

  // We are finished with accessing the vertex buffer
  UnlockVB3D();

  EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);
  // Bind our vertex as the first data stream of our device
  m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));

  // Render the two triangles from the data stream
  hReturn = m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

  EF_PopMatrix();
  m_matProj->Pop();
  m = m_matProj->GetTop();
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
  if (FAILED(hReturn))
  {
    assert(hReturn);
    return;
  }
}

void	CD3D9Renderer::DrawImage(float xpos,float ypos,float w,float h,int texture_id,float s0,float t0,float s1,float t1,float r,float g,float b,float a)
{
  if (m_bDeviceLost)
    return;

  //////////////////////////////////////////////////////////////////////
  // Draw a textured quad, texture is assumed to be in video memory
  //////////////////////////////////////////////////////////////////////

  // Check for the presence of a D3D device
  assert(m_pd3dDevice);

  if (!m_SceneRecurseCount)
  {
    iLog->LogError("CD3D9Renderer::DrawImage before BeginScene");
    return;
  }

  PROFILE_FRAME(Draw_2DImage);

  HRESULT hReturn;

  float fx = xpos;
  float fy = ypos;
  float fw = w;
  float fh = h;


  //if (!pID3DTexture)
  //  D3DXCreateTextureFromFile(m_pd3dDevice, "d:\\Textures\\Console\\DefaultConsole.tga", &pID3DTexture);

  SetCullMode(R_CULL_DISABLE);
  EnableTMU(true);

  // Lock the entire buffer and obtain a pointer to the location where we have to
  // write the vertex data. Note that we specify zero here to lock the entire 
  // buffer.
  int nOffs;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);

  DWORD col = D3DRGBA(r,g,b,a);

  // Now that we have write access to the buffer, we can copy our vertices
  // into it

  // Define the quad
  vQuad[0].xyz.x = xpos;
  vQuad[0].xyz.y = ypos;
  vQuad[0].xyz.z = 1.0f;
  vQuad[0].color.dcolor = col;
  vQuad[0].st[0] = s0;
  vQuad[0].st[1] = 1.0f-t0;

  vQuad[1].xyz.x = xpos + w;
  vQuad[1].xyz.y = ypos;
  vQuad[1].xyz.z = 1.0f;
  vQuad[1].color.dcolor = col;
  vQuad[1].st[0] = s1;
  vQuad[1].st[1] = 1.0f-t0;

  vQuad[2].xyz.x = xpos + w;
  vQuad[2].xyz.y = ypos + h;
  vQuad[2].xyz.z = 1.0f;
  vQuad[2].color.dcolor = col;
  vQuad[2].st[0] = s1;
  vQuad[2].st[1] = 1.0f-t1;

  vQuad[3].xyz.x = xpos;
  vQuad[3].xyz.y = ypos + h;
  vQuad[3].xyz.z = 1.0f;
  vQuad[3].color.dcolor = col;
  vQuad[3].st[0] = s0;
  vQuad[3].st[1] = 1.0f-t1;

  // We are finished with accessing the vertex buffer
  UnlockVB3D();

  // Activate the image texture
  SetTexture(texture_id);
  //if (pID3DTexture)
  //  m_pd3dDevice->SetTexture(0, pID3DTexture);

  // Bind our vertex as the first data stream of our device
  m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);

  // Render the two triangles from the data stream
  hReturn = m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

  if (FAILED(hReturn))
  {
    assert(hReturn);
    return;
  }
}

void CD3D9Renderer::Draw2dLine(float x1, float y1, float x2, float y2)
{
  if (m_bDeviceLost)
    return;

  SetCullMode(R_CULL_DISABLE);
  EnableTMU(true);
  gRenDev->m_TexMan->m_Text_White->Set();

  float x1pos=(float)(int)ScaleCoordX(x1);
  float y1pos=(float)(int)ScaleCoordY(y1);
  float x2pos=(float)(int)ScaleCoordX(x2);
  float y2pos=(float)(int)ScaleCoordY(y2);

  // Lock the entire buffer and obtain a pointer to the location where we have to
  // write the vertex data. Note that we specify zero here to lock the entire 
  // buffer.
  int nOffs;
  struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F *vQuad = GetVBPtr2D(2, nOffs);

  // Define the line
  vQuad[0].x = x1pos;
  vQuad[0].y = y1pos;
  vQuad[0].z = 1.0f;
  vQuad[0].rhw = 1.0f;
  vQuad[0].color.dcolor = 0;

  vQuad[1].x = x2pos;
  vQuad[1].y = y2pos;
  vQuad[1].z = 1.0f;
  vQuad[1].rhw = 1.0f;
  vQuad[1].color.dcolor = 0;

  // We are finished with accessing the vertex buffer
  UnlockVB2D();

  // Bind our vertex as the first data stream of our device
  m_pd3dDevice->SetStreamSource(0, m_pVB2D, 0, sizeof(struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F));
  EF_SetVertexDeclaration(0, VERTEX_FORMAT_TRP3F_COL4UB_TEX2F);

  // Render the two triangles from the data stream
  HRESULT hr = m_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, nOffs, 1);

  if (FAILED(hr))
  {
    assert(hr);
    return;
  }
}

void CD3D9Renderer::DrawPoints(Vec3d v[], int nump, CFColor& col, int flags)
{
  if (m_bDeviceLost)
    return;
}

void CD3D9Renderer::DrawLines(Vec3d v[], int nump, CFColor& col, int flags, float fGround)
{
  if (m_bDeviceLost)
    return;
  if (nump <= 1)
    return;

  int i;
  int st;

  EF_SetVertColor();
  EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  st = GS_NODEPTHTEST;
  if (flags & 1)
    st |= GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;
  else
    st = flags | GS_NODEPTHTEST;
  EF_SetState(st);
  gRenDev->m_TexMan->m_Text_White->Set();

  DWORD c = D3DRGBA(col.r, col.g, col.b, col.a);
  int nOffs;

  if (fGround >= 0)
  {
    struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(nump*2, nOffs);

    for (i=0; i<nump; i++)
    {
      vQuad[i*2+0].xyz.x = v[i][0]; vQuad[i*2+0].xyz.y = fGround; vQuad[i*2+0].xyz.z = 0;
      vQuad[i*2+0].color.dcolor = c;
      vQuad[i*2+0].st[0] = 0; vQuad[i*2+0].st[1] = 0;
      vQuad[i*2+1].xyz = v[i];
      vQuad[i*2+1].color.dcolor = c;
      vQuad[i*2+1].st[0] = 0; vQuad[i*2+1].st[1] = 0;
    }
    // We are finished with accessing the vertex buffer
    UnlockVB3D();

    // Bind our vertex as the first data stream of our device
    m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);

    HRESULT hr = m_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, nOffs, nump);
    if (FAILED(hr))
    {
      assert(hr);
      return;
    }
  }
  else
  {
    struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(nump, nOffs);

    for (i=0; i<nump; i++)
    {
      vQuad[i].xyz = v[i];
      vQuad[i].color.dcolor = c;
      vQuad[i].st[0] = 0; vQuad[i].st[1] = 0;
    }
    // We are finished with accessing the vertex buffer
    UnlockVB3D();

    // Bind our vertex as the first data stream of our device
    m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);

    HRESULT hr = m_pd3dDevice->DrawPrimitive(D3DPT_LINESTRIP, nOffs, nump-1);
    if (FAILED(hr))
    {
      assert(hr);
      return;
    }
  }
}

void CD3D9Renderer::DrawLine(const Vec3d & vPos1, const Vec3d & vPos2)
{
  if (m_bDeviceLost)
    return;

  if (!m_SceneRecurseCount)
  {
    iLog->Log("ERROR: CD3D9Renderer::DrawLine before BeginScene");
    return;
  }

  SetCullMode(R_CULL_DISABLE);
  EnableTMU(true);
  gRenDev->m_TexMan->m_Text_White->Set();

  // Lock the entire buffer and obtain a pointer to the location where we have to
  // write the vertex data. Note that we specify zero here to lock the entire 
  // buffer.
  int nOffs;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(2, nOffs);

  // Define the line
  vQuad[0].xyz = vPos1;
  vQuad[0].color.dcolor = 0xffffffff;

  vQuad[1].xyz = vPos2;
  vQuad[1].color.dcolor = 0xffffffff;

  // We are finished with accessing the vertex buffer
  UnlockVB3D();

  // Bind our vertex as the first data stream of our device
  m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);

  HRESULT hr = m_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, nOffs, 1);

  if (FAILED(hr))
  {
    assert(hr);
    return;
  }

}

void CD3D9Renderer::DrawLineColor(const Vec3d & vPos1, const CFColor & vColor1, const Vec3d & vPos2, const CFColor & vColor2)
{
  if (m_bDeviceLost)
    return;

  if (!m_SceneRecurseCount)
  {
    iLog->Log("ERROR: CD3D9Renderer::DrawLine before BeginScene");
    return;
  }

  HRESULT hr;

  //hr = D3DXCreateLine(m_pd3dDevice, &m_pLine);

  EnableTMU(true);
  gRenDev->m_TexMan->m_Text_White->Set();
  EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);

  // tiago - fixed, it wasn't passing alpha parameter...
  DWORD col1 = D3DRGBA(vColor1[0],vColor1[1],vColor1[2], vColor1[3]);
  DWORD col2 = D3DRGBA(vColor2[0],vColor2[1],vColor2[2], vColor1[3]);

  int nOffs;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(2, nOffs);
  if (!vQuad)
    return;

  // Define the line
  vQuad[0].xyz = vPos1;
  vQuad[0].color.dcolor = col1;

  vQuad[1].xyz = vPos2;
  vQuad[1].color.dcolor = col2;

  // We are finished with accessing the vertex buffer
  UnlockVB3D();

  // Bind our vertex as the first data stream of our device
  m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);

  hr = m_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, nOffs, 1);

  if (FAILED(hr))
  {
    assert(hr);
    return;
  }
}

//*********************************************************************

void CD3D9Renderer::GetModelViewMatrix(float * mat)
{
  memcpy(mat, (D3DXMATRIX *)m_matView->GetTop(), 4*4*sizeof(float));
}

void CD3D9Renderer::GetProjectionMatrix(float * mat)
{
  memcpy(mat, (D3DXMATRIX *)m_matProj->GetTop(), 4*4*sizeof(float));
}

///////////////////////////////////////////
void CD3D9Renderer::PushMatrix()
{
  assert(m_pd3dDevice);

  EF_PushMatrix();
}

///////////////////////////////////////////
void CD3D9Renderer::PopMatrix()
{
  assert(m_pd3dDevice);

  EF_PopMatrix();
}

///////////////////////////////////////////
void CD3D9Renderer::RotateMatrix(float a,float x,float y,float z)
{
  assert(m_pd3dDevice);

  D3DXVECTOR3 v;
  v[0] = x;
  v[1] = y;
  v[2] = z;
  m_matView->RotateAxisLocal(&v, a * PI/180.0f);
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop());
  m_bInvertedMatrix = false;
}

void CD3D9Renderer::LoadMatrix(const Matrix44 *src)
{
  if(src)
    m_matView->LoadMatrix((D3DXMATRIX *)src);
  else
    m_matView->LoadIdentity();
  m_bInvertedMatrix = false;
}

void CD3D9Renderer::RotateMatrix(const Vec3d & angles)
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

void CD3D9Renderer::MultMatrix(float * mat)
{
  m_matView->MultMatrixLocal((D3DXMATRIX *)mat);   
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop()); 
  m_bInvertedMatrix = false;
}

///////////////////////////////////////////
void CD3D9Renderer::ScaleMatrix(float x,float y,float z)
{
  assert(m_pd3dDevice);

  m_matView->ScaleLocal(x, y, z);   
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop()); 
  m_bInvertedMatrix = false;
}

///////////////////////////////////////////
void CD3D9Renderer::TranslateMatrix(float x,float y,float z)
{
  assert(m_pd3dDevice);

  m_matView->TranslateLocal(x, y, z);   
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop()); 
  m_bInvertedMatrix = false;
}

void CD3D9Renderer::TranslateMatrix(const Vec3d &pos)
{
  assert(m_pd3dDevice);

  float x = pos[0];
  float y = pos[1];
  float z = pos[2];
  m_matView->TranslateLocal(x,y,z);   
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop()); 
  m_bInvertedMatrix = false;
}

void CD3D9Renderer::SetPerspective(const CCamera &cam)
{
  D3DXMATRIX *m = m_matProj->GetTop();
  D3DXMatrixPerspectiveFovRH(m, cam.GetFov()*cam.GetProjRatio(), 1.0f/cam.GetProjRatio(), cam.GetZMin(), cam.GetZMax());
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
}





//-----------------------------------------------------------------------------
// coded by ivo:
// calculate parameter for an off-center projection matrix.
// the projection matrix itself is calculated by D3D9.
//-----------------------------------------------------------------------------
D3DXMATRIX OffCenterProjection(const CCamera& cam, const Vec3& nv, unsigned short max, unsigned short win_width,  unsigned short win_height) {

  //get the size of near plane 
  float l=+nv.x;
  float r=-nv.x;
  float b=-nv.z;
  float t=+nv.z;

  //---------------------------------------------------

  float max_x=(float)max;
  float max_z=(float)max;

  float win_x=(float)win_width;
  float win_z=(float)win_height;

  if ((win_x<max_x) && (win_z<max_z) ) {
    //calculate parameters for off-center projection-matrix
    float ext_x=-nv.x*2;
    float ext_z=+nv.z*2;
    l=+nv.x+(ext_x/max_x)*win_x;
    r=+nv.x+(ext_x/max_x)*(win_x+1);
    t=+nv.z-(ext_z/max_z)*win_z;
    b=+nv.z-(ext_z/max_z)*(win_z+1);
  }

  D3DXMATRIX m; 
  D3DXMatrixPerspectiveOffCenterRH(&m, l,r,b,t, cam.GetZMin(), cam.GetZMax());
  return m;
}



void CD3D9Renderer::SetCamera(const CCamera &cam)
{
  assert(m_pd3dDevice);

  Matrix44 mat = cam.GetVCMatrixD3D9();
  D3DXMATRIX *m = m_matView->GetTop();
  m_matView->LoadMatrix((D3DXMATRIX *)&mat);
  if (m_RP.m_PersFlags & RBPF_DRAWMIRROR)
    m_matView->Scale(1,-1,1);
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
  D3DXMatrixInverse(&m_matViewInv, NULL, m);
  m = m_matProj->GetTop();
  m_eCull = (ECull)-1;

  float fov=cam.GetFov()*cam.GetProjRatio();
  D3DXMatrixPerspectiveFovRH(m, fov, 1.0f/cam.GetProjRatio(), cam.GetZMin(), cam.GetZMax());

  //IVO: code to check, if off-center projection works
  if (0) 
  {
    unsigned short win_width	=0xffff; 
    unsigned short win_height	=0xffff;
    //DEBUG_STUFF: Vladimir please remove this
    if ((GetAsyncKeyState('I') & 0x8000))	{ win_width=0; win_height=0; }
    if ((GetAsyncKeyState('O') & 0x8000))	{ win_width=1; win_height=0; }
    if ((GetAsyncKeyState('K') & 0x8000))	{ win_width=0; win_height=1; }
    if ((GetAsyncKeyState('L') & 0x8000))	{ win_width=1; win_height=1; }


    //get edge information of frustum
    Vec3 edge_plt=cam.GetEdgeP();
    Vec3 edge_nlt=cam.GetEdgeN();;

    //adjust distance of projection-plane to camera (width & height is fixed)
    edge_plt.y	= cry_cosf(fov*0.5f) / cry_sinf(fov*0.5f) * edge_plt.z;

    //recalculate size of near-plane (distance is fixed)
    edge_nlt.x	= (edge_nlt.y/edge_plt.y)*edge_plt.x;   
    edge_nlt.z	= (edge_nlt.y/edge_plt.y)*edge_plt.z;  

    //overwrite the original projection matrix
    //parametes:  max:xz   win:xz
    //if values in win are bigger/equal to max, then functions returns no part-screen mat 
    *m=OffCenterProjection(cam, edge_nlt, 0x02, win_width, win_height ); 
  }

  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
  m_bInvertedMatrix = false;

  EF_SetCameraInfo();

  m_cam = cam;
}

void CD3D9Renderer::SetViewport(int x, int y, int width, int height)
{
  //SAFE_RELEASE(m_pLine);

  if (!x && !y && !width && !height)
  {
    m_Viewport.X = m_VX;
    m_Viewport.Y = m_VY;
    m_Viewport.Width = m_VWidth;
    m_Viewport.Height = m_VHeight;
    m_Viewport.MinZ       = 0.0;
    m_Viewport.MaxZ       = 1.0;

    m_pd3dDevice->SetViewport(&m_Viewport);
    //HRESULT hr = D3DXCreateLine(m_pd3dDevice, &m_pLine);
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
  //HRESULT hr = D3DXCreateLine(m_pd3dDevice, &m_pLine);
}

void CD3D9Renderer::SetScissor(int x, int y, int width, int height)
{
  if (!x && !y && !width && !height)
    EF_Scissor(false, x, y, width, height);
  else
    EF_Scissor(true, x, y, width, height);
}

void CD3D9Renderer::Flush3dBBox(const Vec3d &mins,const Vec3d &maxs,const bool bSolid)
{
  SetCullMode(R_CULL_DISABLE);
  EnableTMU(true);
  gRenDev->m_TexMan->m_Text_White->Set();

  HRESULT hr;

  int nOffs;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad;
  if(bSolid)
  {
    EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);

    vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);
    hr = m_pd3dDevice->SetStreamSource( 0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    vQuad[0].xyz.x = maxs.x; vQuad[0].xyz.y = mins.y; vQuad[0].xyz.z = mins.z; //3
    vQuad[1].xyz.x = maxs.x; vQuad[1].xyz.y = mins.y; vQuad[1].xyz.z = maxs.z; //2
    vQuad[2].xyz.x = mins.x; vQuad[2].xyz.y = mins.y; vQuad[2].xyz.z = maxs.z; //1
    vQuad[3].xyz.x = mins.x; vQuad[3].xyz.y = mins.y; vQuad[3].xyz.z = mins.z; //0
    UnlockVB3D();
    m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

    vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);
    hr = m_pd3dDevice->SetStreamSource( 0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    vQuad[0].xyz.x = mins.x; vQuad[0].xyz.y = mins.y; vQuad[0].xyz.z = mins.z; //0
    vQuad[1].xyz.x = mins.x; vQuad[1].xyz.y = mins.y; vQuad[1].xyz.z = maxs.z; //1
    vQuad[2].xyz.x = mins.x; vQuad[2].xyz.y = maxs.y; vQuad[2].xyz.z = maxs.z; //6
    vQuad[3].xyz.x = mins.x; vQuad[3].xyz.y = maxs.y; vQuad[3].xyz.z = mins.z; //4
    UnlockVB3D();
    m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

    vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);
    hr = m_pd3dDevice->SetStreamSource( 0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    vQuad[0].xyz.x = mins.x; vQuad[0].xyz.y = maxs.y; vQuad[0].xyz.z = mins.z; //4
    vQuad[1].xyz.x = mins.x; vQuad[1].xyz.y = maxs.y; vQuad[1].xyz.z = maxs.z; //6
    vQuad[2].xyz.x = maxs.x; vQuad[2].xyz.y = maxs.y; vQuad[2].xyz.z = maxs.z; //7
    vQuad[3].xyz.x = maxs.x; vQuad[3].xyz.y = maxs.y; vQuad[3].xyz.z = mins.z; //5
    UnlockVB3D();
    m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

    vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);
    hr = m_pd3dDevice->SetStreamSource( 0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    vQuad[0].xyz.x = maxs.x; vQuad[0].xyz.y = maxs.y; vQuad[0].xyz.z = mins.z; //54
    vQuad[1].xyz.x = maxs.x; vQuad[1].xyz.y = maxs.y; vQuad[1].xyz.z = maxs.z; //73
    vQuad[2].xyz.x = maxs.x; vQuad[2].xyz.y = mins.y; vQuad[2].xyz.z = maxs.z; //22
    vQuad[3].xyz.x = maxs.x; vQuad[3].xyz.y = mins.y; vQuad[3].xyz.z = mins.z; //31
    UnlockVB3D();
    m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

    // top
    vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);
    hr = m_pd3dDevice->SetStreamSource( 0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    vQuad[0].xyz.x = maxs.x; vQuad[0].xyz.z = maxs.z; vQuad[0].xyz.y = mins.y; //3
    vQuad[1].xyz.x = maxs.x; vQuad[1].xyz.z = maxs.z; vQuad[1].xyz.y = maxs.y; //2
    vQuad[2].xyz.x = mins.x; vQuad[2].xyz.z = maxs.z; vQuad[2].xyz.y = maxs.y; //1
    vQuad[3].xyz.x = mins.x; vQuad[3].xyz.z = maxs.z; vQuad[3].xyz.y = mins.y; //0
    UnlockVB3D();
    m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

    // bottom
    vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);
    hr = m_pd3dDevice->SetStreamSource( 0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    vQuad[0].xyz.x = maxs.x; vQuad[0].xyz.z = mins.z; vQuad[0].xyz.y = mins.y; //3
    vQuad[1].xyz.x = maxs.x; vQuad[1].xyz.z = mins.z; vQuad[1].xyz.y = maxs.y; //2
    vQuad[2].xyz.x = mins.x; vQuad[2].xyz.z = mins.z; vQuad[2].xyz.y = maxs.y; //1
    vQuad[3].xyz.x = mins.x; vQuad[3].xyz.z = mins.z; vQuad[3].xyz.y = mins.y; //0
    UnlockVB3D();
    m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);
  }
  else
  {
    EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);

    vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);
    hr = m_pd3dDevice->SetStreamSource( 0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    vQuad[0].xyz.x = maxs.x; vQuad[0].xyz.y = mins.y; vQuad[0].xyz.z = mins.z; //3
    vQuad[1].xyz.x = maxs.x; vQuad[1].xyz.y = mins.y; vQuad[1].xyz.z = maxs.z; //2
    vQuad[2].xyz.x = mins.x; vQuad[2].xyz.y = mins.y; vQuad[2].xyz.z = maxs.z; //1
    vQuad[3].xyz.x = mins.x; vQuad[3].xyz.y = mins.y; vQuad[3].xyz.z = mins.z; //0
    UnlockVB3D();
    m_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, nOffs, 2);

    vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);
    hr = m_pd3dDevice->SetStreamSource( 0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    vQuad[0].xyz.x = mins.x; vQuad[0].xyz.y = mins.y; vQuad[0].xyz.z = mins.z; //0
    vQuad[1].xyz.x = mins.x; vQuad[1].xyz.y = mins.y; vQuad[1].xyz.z = maxs.z; //1
    vQuad[2].xyz.x = mins.x; vQuad[2].xyz.y = maxs.y; vQuad[2].xyz.z = maxs.z; //6
    vQuad[3].xyz.x = mins.x; vQuad[3].xyz.y = maxs.y; vQuad[3].xyz.z = mins.z; //4
    UnlockVB3D();
    m_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, nOffs, 2);

    vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);
    hr = m_pd3dDevice->SetStreamSource( 0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    vQuad[0].xyz.x = mins.x; vQuad[0].xyz.y = maxs.y; vQuad[0].xyz.z = mins.z; //4
    vQuad[1].xyz.x = mins.x; vQuad[1].xyz.y = maxs.y; vQuad[1].xyz.z = maxs.z; //6
    vQuad[2].xyz.x = maxs.x; vQuad[2].xyz.y = maxs.y; vQuad[2].xyz.z = maxs.z; //7
    vQuad[3].xyz.x = maxs.x; vQuad[3].xyz.y = maxs.y; vQuad[3].xyz.z = mins.z; //5
    UnlockVB3D();
    m_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, nOffs, 2);

    vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);
    hr = m_pd3dDevice->SetStreamSource( 0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    vQuad[0].xyz.x = maxs.x; vQuad[0].xyz.y = mins.y; vQuad[0].xyz.z = mins.z; //54
    vQuad[1].xyz.x = maxs.x; vQuad[1].xyz.y = mins.y; vQuad[1].xyz.z = maxs.z; //73
    vQuad[2].xyz.x = maxs.x; vQuad[2].xyz.y = maxs.y; vQuad[2].xyz.z = maxs.z; //22
    vQuad[3].xyz.x = maxs.x; vQuad[3].xyz.y = maxs.y; vQuad[3].xyz.z = mins.z; //31
    UnlockVB3D();
    m_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, nOffs, 2);

    // top
    vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(5, nOffs);
    hr = m_pd3dDevice->SetStreamSource( 0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    vQuad[0].xyz.x = maxs.x; vQuad[0].xyz.z = maxs.z; vQuad[0].xyz.y = mins.y; //3
    vQuad[1].xyz.x = maxs.x; vQuad[1].xyz.z = maxs.z; vQuad[1].xyz.y = maxs.y; //2
    vQuad[2].xyz.x = mins.x; vQuad[2].xyz.z = maxs.z; vQuad[2].xyz.y = maxs.y; //1
    vQuad[3].xyz.x = mins.x; vQuad[3].xyz.z = maxs.z; vQuad[3].xyz.y = mins.y; //0
    vQuad[4].xyz.x = maxs.x; vQuad[4].xyz.z = maxs.z; vQuad[4].xyz.y = mins.y; //3
    UnlockVB3D();
    m_pd3dDevice->DrawPrimitive(D3DPT_LINESTRIP, nOffs, 4);

    // bottom
    vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(5, nOffs);
    hr = m_pd3dDevice->SetStreamSource( 0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    vQuad[0].xyz.x = maxs.x; vQuad[0].xyz.z = mins.z; vQuad[0].xyz.y = mins.y; //3
    vQuad[1].xyz.x = maxs.x; vQuad[1].xyz.z = mins.z; vQuad[1].xyz.y = maxs.y; //2
    vQuad[2].xyz.x = mins.x; vQuad[2].xyz.z = mins.z; vQuad[2].xyz.y = maxs.y; //1
    vQuad[3].xyz.x = mins.x; vQuad[3].xyz.z = mins.z; vQuad[3].xyz.y = mins.y; //0
    vQuad[4].xyz.x = maxs.x; vQuad[4].xyz.z = mins.z; vQuad[4].xyz.y = mins.y; //3
    UnlockVB3D();
    m_pd3dDevice->DrawPrimitive(D3DPT_LINESTRIP, nOffs, 4);
  }
}

void CD3D9Renderer::Draw3dBBox(const Vec3d &mins,const Vec3d &maxs, int nPrimType)
{
  Draw3dPrim(mins, maxs, nPrimType);
}

void CD3D9Renderer::Draw3dPrim(const Vec3d &mins,const Vec3d &maxs, int nPrimType, const float* pRGBA)
{
  BBoxInfo info;
  info.vMins = mins;
  info.vMaxs = maxs;
  info.nPrimType = nPrimType;
  if (pRGBA)
  {
    for (int i = 0; i < 4; ++i)
      info.fColor[i] = pRGBA[i];
  }
  else
  {
    info.fColor[0] = m_RP.m_CurGlobalColor.bcolor[0] / 255.0f;
    info.fColor[1] = m_RP.m_CurGlobalColor.bcolor[1] / 255.0f;
    info.fColor[2] = m_RP.m_CurGlobalColor.bcolor[2] / 255.0f;
    info.fColor[3] = m_RP.m_CurGlobalColor.bcolor[3] / 255.0f;
  }
  m_arrBoxesToDraw.push_back(info);
}


void CD3D9Renderer::SetCullMode(int mode)
{
  //////////////////////////////////////////////////////////////////////
  // Change the culling mode
  //////////////////////////////////////////////////////////////////////

  assert(m_pd3dDevice);

  switch (mode)
  {
    case R_CULL_DISABLE:
      D3DSetCull(eCULL_None);
      break;
    case R_CULL_BACK:
      D3DSetCull(eCULL_Back);
      break;
    case R_CULL_FRONT:
      D3DSetCull(eCULL_Front);
      break;
  }
}

void CD3D9Renderer::SetFog(float density, float fogstart, float fogend, const float *color, int fogmode)
{
  //////////////////////////////////////////////////////////////////////
  // Configure the fog settings
  //////////////////////////////////////////////////////////////////////

  D3DCOLOR dwColor;

  assert(m_pd3dDevice);

  m_pd3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE,	D3DFOG_NONE);

  m_FS.m_FogDensity = density;
  m_FS.m_FogStart = fogstart;
  m_FS.m_FogEnd = fogend;
  m_FS.m_nFogMode = fogmode;
  m_FS.m_nCurFogMode = fogmode;
  CFColor col = CFColor(color[0], color[1], color[2], 1.0f);
  if (m_bHeatVision)
    col = CFColor(0.0f, 0.0f, 0.0f, 1.0f);
  m_FS.m_FogColor = col;

  // Fog color
  dwColor = D3DRGBA(col[0],col[1],col[2],col[3]);
  EF_SetFogColor(dwColor, true, false);

  // Fog start and end
  m_pd3dDevice->SetRenderState(D3DRS_FOGSTART, *((LPDWORD)&fogstart));
  m_pd3dDevice->SetRenderState(D3DRS_FOGEND, *((LPDWORD) &fogend));


  // Fog mode
  switch (fogmode)
  {
    case R_FOGMODE_LINEAR:
      m_pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
      density = 1;
      m_pd3dDevice->SetRenderState(D3DRS_FOGDENSITY, *((LPDWORD) (&density)));
      break;

    case R_FOGMODE_EXP2:
      m_pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP2);
      m_pd3dDevice->SetRenderState(D3DRS_FOGDENSITY, *((LPDWORD) (&density)));
      break;

    default:
      // Invalid mode
      // Disable
      m_pd3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
      break;
  }
}

void CD3D9Renderer::SetFogColor(float * color)
{
  m_FS.m_FogColor = CFColor(color);
  D3DCOLOR dwColor;

  // Fog color
  dwColor = D3DRGBA(color[0],color[1],color[2],color[3]);
  EF_SetFogColor(dwColor, true, false);
}

bool CD3D9Renderer::EnableFog(bool enable)
{
  //////////////////////////////////////////////////////////////////////
  // Enable or disable fog
  //////////////////////////////////////////////////////////////////////

  assert(m_pd3dDevice);

  bool bPrevFog = m_FS.m_bEnable; // remember fog value

  if (bPrevFog != enable)
    m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, enable);

  m_FS.m_bEnable = enable;

  return bPrevFog;
}

///////////////////////////////////////////
void CD3D9Renderer::EnableTexGen(bool enable)
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
  m_RP.m_TexStages[m_TexMan->m_CurStage].TCIndex = m_TexMan->m_CurStage;
}

void CD3D9Renderer::SetTexgen3D(float x1, float y1, float z1, float x2, float y2, float z2)
{
  assert(m_pd3dDevice);

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

  m_RP.m_TexStages[m_TexMan->m_CurStage].TCIndex = m_TexMan->m_CurStage;
}

void CD3D9Renderer::SetTexgen(float scaleX, float scaleY,float translateX,float translateY)
{        
  //////////////////////////////////////////////////////////////////////
  //
  //////////////////////////////////////////////////////////////////////
  assert(m_pd3dDevice);

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
  m_RP.m_TexStages[m_TexMan->m_CurStage].TCIndex = m_TexMan->m_CurStage;
}

void CD3D9Renderer::SetLodBias(float value)
{
  //////////////////////////////////////////////////////////////////////
  // Set the mip-map LOD bias
  //////////////////////////////////////////////////////////////////////

  assert(m_pd3dDevice);

  value = -value;
  m_pd3dDevice->SetSamplerState(m_TexMan->m_CurStage, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&value)));
}

void CD3D9Renderer::SelectTMU(int tnum)
{
  m_TexMan->m_CurStage = tnum;
}

void CD3D9Renderer::EnableTMU(bool enable)
{
  assert(m_pd3dDevice);

  byte eCO = (enable ? eCO_MODULATE : eCO_DISABLE);
  EF_SetColorOp(eCO, eCO, 255, 255);
  if (!enable)
  {
    m_pd3dDevice->SetTexture(m_TexMan->m_CurStage, NULL);
    m_RP.m_TexStages[m_TexMan->m_CurStage].Texture = NULL;
  }
}

///////////////////////////////////////////
void CD3D9Renderer::CheckError(const char *comment)
{

}

///////////////////////////////////////////
int CD3D9Renderer::SetPolygonMode(int mode)
{
  int prev_mode = m_polygon_mode;
  m_polygon_mode = mode;

  if (CV_d3d9_forcesoftware)
    return prev_mode;

  if(m_polygon_mode == R_WIREFRAME_MODE)
    m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
  else
    m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
  return prev_mode;
}

///////////////////////////////////////////
void CD3D9Renderer::EnableVSync(bool enable)
{
}

void CD3D9Renderer::DrawQuad(const Vec3d &right, const Vec3d &up, const Vec3d &origin,int nFlipMode/*=0*/)
{
  PROFILE_FRAME(Draw_2DImage);

  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F Verts[4];

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

  int nOffs;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);
  memcpy(vQuad, Verts, 4*sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));

  UnlockVB3D();

  m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);
  m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

  m_nPolygons += 2;
}

void CD3D9Renderer::DrawQuad(float dy,float dx, float dz, float x, float y, float z)
{
  PROFILE_FRAME(Draw_2DImage);

  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F Verts[4];

  Verts[0].xyz[0] = -dx+x; Verts[0].xyz[1] = dy+y; Verts[0].xyz[2] = -dz+z;
  Verts[0].st[0] = 0; Verts[0].st[1] = 0;

  Verts[1].xyz[0] = dx+x; Verts[1].xyz[1] = -dy+y; Verts[1].xyz[2] = -dz+z;
  Verts[1].st[0] = 1; Verts[1].st[1] = 0;

  Verts[2].xyz[0] = dx+x; Verts[2].xyz[1] = -dy+y; Verts[2].xyz[2] = dz+z;
  Verts[2].st[0] = 1; Verts[2].st[1] = 1;

  Verts[3].xyz[0] = -dx+x; Verts[3].xyz[1] = dy+y; Verts[3].xyz[2] = dz+z;
  Verts[3].st[0] = 0; Verts[3].st[1] = 1;

  int nOffs;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);
  memcpy(vQuad, Verts, 4*sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));

  UnlockVB3D();

  m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);
  m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

  m_nPolygons += 2;
}

void CD3D9Renderer::DrawQuad(float x0, float y0, float x1, float y1, const CFColor & color, float z)
{
  PROFILE_FRAME(Draw_2DImage);

  LPDIRECT3DDEVICE9 dv = mfGetD3DDevice();
  DWORD col = D3DRGBA(color.r, color.g, color.b, color.a);

  int nOffs;
  struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F *vQuad = GetVBPtr2D(4, nOffs);

  float ftx0 = 0;
  float fty0 = 0;
  float ftx1 = 1;
  float fty1 = 1;

  // Define the quad
  vQuad[0].x = x0;
  vQuad[0].y = y0;
  vQuad[0].z = z;
  vQuad[0].rhw = 1.0f;
  vQuad[0].color.dcolor = col;
  vQuad[0].st[0] = ftx0;
  vQuad[0].st[1] = fty0;

  vQuad[1].x = x1;
  vQuad[1].y = y0;
  vQuad[1].z = z;
  vQuad[1].rhw = 1.0f;
  vQuad[1].color.dcolor = col;
  vQuad[1].st[0] = ftx1;
  vQuad[1].st[1] = fty0;

  vQuad[2].x = x1;
  vQuad[2].y = y1;
  vQuad[2].z = z;
  vQuad[2].rhw = 1.0f;
  vQuad[2].color.dcolor = col;
  vQuad[2].st[0] = ftx1;
  vQuad[2].st[1] = fty1;

  vQuad[3].x = x0;
  vQuad[3].y = y1;
  vQuad[3].z = z;
  vQuad[3].rhw = 1.0f;
  vQuad[3].color.dcolor = col;
  vQuad[3].st[0] = ftx0;
  vQuad[3].st[1] = fty1;

  UnlockVB2D();

  dv->SetStreamSource(0, m_pVB2D, 0, sizeof(struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F));
  EF_SetVertexDeclaration(0, VERTEX_FORMAT_TRP3F_COL4UB_TEX2F);
  dv->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

  m_nPolygons += 2;
}

void CD3D9Renderer::DrawQuad3D(const Vec3d & v0, const Vec3d & v1, const Vec3d & v2, const Vec3d & v3,
                               const CFColor & color, float ftx0,  float fty0,  float ftx1,  float fty1)
{
  LPDIRECT3DDEVICE9 dv = mfGetD3DDevice();
  DWORD col = D3DRGBA(color.r, color.g, color.b, color.a);

  int nOffs;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *vQuad = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(4, nOffs);

  // Define the quad
  vQuad[0].xyz.x = v0.x;
  vQuad[0].xyz.y = v0.y;
  vQuad[0].xyz.z = v0.z;
  vQuad[0].color.dcolor = col;
  vQuad[0].st[0] = ftx0;
  vQuad[0].st[1] = fty0;
  //vQuad[0].st[2] = ftx0;
  //vQuad[0].st[3] = fty0;

  vQuad[1].xyz.x = v1.x;
  vQuad[1].xyz.y = v1.y;
  vQuad[1].xyz.z = v1.z;
  vQuad[1].color.dcolor = col;
  vQuad[1].st[0] = ftx1;
  vQuad[1].st[1] = fty0;
  //vQuad[1].st[2] = ftx1;
  //vQuad[1].st[3] = fty0;

  vQuad[2].xyz.x = v2.x;
  vQuad[2].xyz.y = v2.y;
  vQuad[2].xyz.z = v2.z;
  vQuad[2].color.dcolor = col;
  vQuad[2].st[0] = ftx1;
  vQuad[2].st[1] = fty1;
  //vQuad[2].st[2] = ftx1;
  //vQuad[2].st[3] = fty1;

  vQuad[3].xyz.x = v3.x;
  vQuad[3].xyz.y = v3.y;
  vQuad[3].xyz.z = v3.z;
  vQuad[3].color.dcolor = col;
  vQuad[3].st[0] = ftx0;
  vQuad[3].st[1] = fty1;
  //vQuad[3].st[2] = ftx0;
  //vQuad[3].st[3] = fty1;

  UnlockVB3D();

  dv->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);
  dv->DrawPrimitive(D3DPT_TRIANGLEFAN, nOffs, 2);

  m_nPolygons += 2;
}

void CD3D9Renderer::DrawPoint(float x, float y, float z, float fSize)
{
  if (!m_SceneRecurseCount)
  {
    iLog->Log("ERROR: CD3D9Renderer::DrawPoint before BeginScene");
    return;
  }

  m_pd3dDevice->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&fSize));

  int nOffs;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *Verts = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(1, nOffs);
  Verts[0].xyz[0] = x; Verts[0].xyz[1] = y; Verts[0].xyz[2] = z;

  UnlockVB3D();

  m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
  EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);
  m_pd3dDevice->DrawPrimitive(D3DPT_POINTLIST, nOffs, 1);

  m_nPolygons += 1;
}

///////////////////////////////////////////
void CD3D9Renderer::DrawTriStrip(CVertexBuffer *src, int vert_num)
{
  if (!m_SceneRecurseCount)
  {
    iLog->Log("ERROR: CD3D9Renderer::DrawTriStrip before BeginScene");
    return;
  }
  HRESULT h;

  EF_SetVertexDeclaration(0, src->m_vertexformat);

  switch (src->m_vertexformat)
  {
  case VERTEX_FORMAT_P3F_TEX2F:
    {
      struct_VERTEX_FORMAT_P3F_TEX2F *dt = (struct_VERTEX_FORMAT_P3F_TEX2F *)src->m_VS[VSF_GENERAL].m_VData;
      h = m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, vert_num-2, dt, sizeof(struct_VERTEX_FORMAT_P3F_TEX2F));
    }
    break;
  case VERTEX_FORMAT_P3F_COL4UB:
    {				
      struct_VERTEX_FORMAT_P3F_COL4UB *dt = (struct_VERTEX_FORMAT_P3F_COL4UB *)src->m_VS[VSF_GENERAL].m_VData;
      EF_SetVertColor();
      h = m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, vert_num-2, dt, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB));
    }
    break;
  case VERTEX_FORMAT_P3F_COL4UB_TEX2F:
    {				
      struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *dt = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)src->m_VS[VSF_GENERAL].m_VData;
      EF_SetVertColor();

      int nOffs;
      struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *Verts = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)GetVBPtr3D(vert_num, nOffs);
      cryMemcpy(Verts, dt, vert_num*sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));

      m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
      m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, nOffs, vert_num-2);
    }
    break;
  case VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F:
    {				
      struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F *dt = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F *)src->m_VS[VSF_GENERAL].m_VData;
      EF_SetVertColor();
      h = m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, vert_num-2, dt, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F));
    }
    break;
  case VERTEX_FORMAT_TRP3F_COL4UB_TEX2F:
    {				
      struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F *dt = (struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F *)src->m_VS[VSF_GENERAL].m_VData;
      EF_SetVertColor();

      int nOffs;
      struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F *Verts = (struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F *)GetVBPtr2D(vert_num, nOffs);
      cryMemcpy(Verts, dt, vert_num*sizeof(struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F));

      m_pd3dDevice->SetStreamSource(0, m_pVB3D[0], 0, sizeof(struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F));
      m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, nOffs, vert_num-2);
    }
    break;

  default:
    assert(0);
    break;
  }

  m_nPolygons += vert_num-2;
}

///////////////////////////////////////////
void CD3D9Renderer::ResetToDefault()
{
  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, ".... ResetToDefault ....\n");

  EF_Scissor(false, 0, 0, 0, 0);
  if (m_RP.m_ClipPlaneWasOverrided == 2)
  {
    m_pd3dDevice->SetClipPlane(0, &m_RP.m_CurClipPlane.m_Normal[0]);
    m_RP.m_ClipPlaneWasOverrided = 0;
  }

  for (int i=0; i<m_numtmus; i++)
  {
    EF_SelectTMU(i);
    m_eCurColorOp[i] = -1;
    m_eCurAlphaOp[i] = -1;
    m_eCurColorArg[i] = -1;
    m_eCurAlphaArg[i] = -1;
    m_pd3dDevice->SetTextureStageState( i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    m_RP.m_TexStages[i].Projected = false;
    m_RP.m_TexStages[i].TCIndex = i;
    m_pd3dDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, i);
    
    // tiago: added
    SetLodBias(CRenderer::CV_r_maxtexlod_bias);  

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
  gRenDev->m_TexMan->m_Text_White->Set();
  gRenDev->m_TexMan->m_nCurStages = 1;

  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
  m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
  m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
  m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
  m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
  m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
  m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
  m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);    

  if (m_d3dCaps.RasterCaps & (D3DPRASTERCAPS_DEPTHBIAS | D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS))
    m_pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, 0);
  m_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
  m_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
  EF_SetVertColor();
  m_RP.m_FlagsModificators = 0;

  m_RP.m_PersFlags &= ~(RBPF_VSNEEDSET | RBPF_PS1NEEDSET);
  m_RP.m_PersFlags &= ~RBPF_WASWORLDSPACE;
  m_RP.m_CurrentVLights = 0;
  EF_CommitShadersState();
  EF_CommitVLightsState();

  m_CurState = GS_DEPTHWRITE;
  m_eCull = (ECull)-1;
  D3DSetCull(eCULL_Back);

  m_FS.m_nCurFogMode = m_FS.m_nFogMode;
  m_pd3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);

  if (m_LogFile && CV_r_log == 3)
    Logv(SRendItem::m_RecurseLevel, ".... End ResetToDefault ....\n");

  CPShader::m_CurRC = NULL;
  CVProgram::m_LastVP = NULL;
  CPShader::m_LastVP = NULL;
}


//////////////////////////////////////////
int CD3D9Renderer::GenerateAlphaGlowTexture(float k)
{   
  //  float k = 6;
  const int tex_size = 256;
  static byte data[tex_size][tex_size];

  memset(&data[0][0], 255, tex_size*tex_size);

  for(int x=0; x<tex_size; x++)
    for(int y=0; y<tex_size; y++)
    {
      float _x = (float)(x-tex_size/2);
      float _y = (float)(y-tex_size/2);

      float val = (float)k*2.f*((float)tex_size/2 - (float)(cry_sqrtf(_x*_x+_y*_y)));
      val = CLAMP(val, 0.0f, 255.0f);

      data[x][y] = (int)(val);
    }

    return DownLoadToVideoMemory((unsigned char*)data,tex_size,tex_size,eTF_8000,eTF_8000,true,true,FILTER_LINEAR, 0, "$AlphaGlow");
}

///////////////////////////////////////////
void CD3D9Renderer::SetMaterialColor(float r, float g, float b, float a)
{
  EF_SetGlobalColor(r, g, b, a);
}

///////////////////////////////////////////
int CD3D9Renderer::LoadAnimatedTexture(const char * szFileNameFormat,const int nCount)
{
  if(nCount<1)
    return 0;

  for(int t=0; t<m_LoadedAnimatedTextures.Count(); t++)
  {
    if(strcmp(m_LoadedAnimatedTextures[t]->sName, szFileNameFormat) == 0)
      if(m_LoadedAnimatedTextures[t]->nFramesCount == nCount)
			{
				m_LoadedAnimatedTextures[t]->nRefCounter++;
        return t+1;
			}
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
  pInfo->nFramesCount = 0;
  strncpy(pInfo->sName, szFileNameFormat, sizeof(pInfo->sName));

  if (strstr(szFileNameFormat,"%") != 0)
  {
    // Formatted filename.
    for (int i=0; i<nCount; i++) 
    {
      char filename[80]="";
      sprintf(filename, szFileNameFormat, i);
      pInfo->pBindIds[i] = LoadTexture( filename );
      if (!pInfo->pBindIds[i])
        break;
      pInfo->nFramesCount++;
    }
  }
	else
	{
		char szFileName[_MAX_PATH];
		char szFileFormat[_MAX_PATH];
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		_splitpath( szFileNameFormat,drive,dir,fname,ext );

		int numDigits = 0;
		int namelen = strlen(fname);
		int i = namelen-1;
		// Find out number of digits in the end of the filename.
		while (i >= 0 && isdigit(fname[i]))
		{
			numDigits++;
			i--;
		}

		int firstFrame = 0;
		if (numDigits > 0)
		{
			const char *sNum = fname + namelen - numDigits;
			firstFrame = atoi(sNum);

			fname[namelen-numDigits] = 0;
			// Make format string.
			sprintf( fname+strlen(fname),"%%.%dd",numDigits );
		}
		_makepath( szFileFormat,drive,dir,fname,ext );
		for (int i=0; i<nCount; i++) 
		{
			sprintf( szFileName,szFileFormat,firstFrame + i );
			pInfo->pBindIds[i] = LoadTexture( szFileName );
			if (!pInfo->pBindIds[i])
				break;
			pInfo->nFramesCount++;
		}
	}

  iLog->LogToFilePlus(" (%d)", nCount);

	pInfo->nRefCounter=1;

  m_LoadedAnimatedTextures.Add(pInfo);

  return (int)m_LoadedAnimatedTextures.Count();
}

///////////////////////////////////////////
char * CD3D9Renderer::GetStatusText(ERendStats type)
{
  return "No status yet";
}

void sLogTexture (char *name, int Size);

void CD3D9Renderer::ProjectToScreen(float ptx, float pty, float ptz,float *sx, float *sy, float *sz )
{
  D3DXVECTOR3 vOut, vIn;
  vIn.x = ptx;
  vIn.y = pty;
  vIn.z = ptz;
  D3DXMATRIX mIdent;
  D3DXMatrixIdentity(&mIdent);
  D3DXVec3Project(&vOut, &vIn, &m_Viewport, m_matProj->GetTop(), m_matView->GetTop(), &mIdent);
  *sx = vOut.x * 100/m_width;
  *sy = vOut.y * 100/m_height;
  *sz = vOut.z;
}

void CD3D9Renderer::DrawBall(float x, float y, float z, float radius)
{
  if (m_bDeviceLost)
    return;

  assert (m_pSphere);
  EF_PushMatrix();
  EnableTMU(true);
  gRenDev->m_TexMan->m_Text_White->Set();
  TranslateMatrix(x, y, z);
  ScaleMatrix(radius, radius, radius);

  m_pSphere->DrawSubset(0);
  m_pLastVDeclaration = NULL;

  EF_PopMatrix();
}

void CD3D9Renderer::DrawBall(const Vec3d & pos, float radius)
{
  if (m_bDeviceLost)
    return;

  assert (m_pSphere);

  EF_PushMatrix();
  EnableTMU(true);
  gRenDev->m_TexMan->m_Text_White->Set();
  TranslateMatrix(pos.x, pos.y, pos.z);
  ScaleMatrix(radius, radius, radius);

  m_pSphere->DrawSubset(0);
  m_pLastVDeclaration = NULL;

  EF_PopMatrix();
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
  in[2] = winz;//2.0f * winz - 1.0f;
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

int CD3D9Renderer::UnProject(float sx, float sy, float sz, float *px, float *py, float *pz, const float modelMatrix[16], const float projMatrix[16], const int viewport[4])
{
  return sUnProject(sx, sy, sz, modelMatrix, projMatrix, viewport, px, py, pz);
}

int CD3D9Renderer::UnProjectFromScreen( float sx, float sy, float sz, float *px, float *py, float *pz)
{
  float modelMatrix[16];
  float projMatrix[16];
  int viewport[4];

  GetModelViewMatrix(modelMatrix);
  GetProjectionMatrix(projMatrix);
  GetViewport(&viewport[0], &viewport[1], &viewport[2], &viewport[3]);
  return sUnProject(sx, sy, sz, modelMatrix, projMatrix, viewport, px, py, pz);
}

//////////////////////////////////////////////////////////////////////
void CD3D9Renderer::ClearDepthBuffer()
{
  m_bWasCleared = true;
  m_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);
}

void CD3D9Renderer::ClearColorBuffer(const Vec3d vColor)
{
  m_bWasCleared = true;
  m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DRGBA(vColor[0], vColor[1], vColor[2], 1.0f), 1.0f, 0);
}

void CD3D9Renderer::EnableAALines(bool bEnable)
{
  assert(0);
}

void CD3D9Renderer::Set2DMode(bool enable, int ortox, int ortoy)
{
  D3DXMATRIX *m;
  if(enable)
  {
    m_matProj->Push();
    m = m_matProj->GetTop();
    D3DXMatrixOrthoOffCenterLH(m, 0.0, (float)ortox, (float)ortoy, 0.0, -1e30f, 1e30f);
    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m);
    EF_PushMatrix();
    m = m_matView->GetTop();
    m_matView->LoadIdentity();
    m_pd3dDevice->SetTransform(D3DTS_VIEW, m);
  }
  else
  {
    HRESULT hr = m_matProj->Pop();
    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, m_matProj->GetTop());
    EF_PopMatrix();
  }
  EF_SetCameraInfo();
}

unsigned int CD3D9Renderer::MakeTexture(const char * _filename,int *_tex_type/*,unsigned int def_tid*/)
{
  return LoadTexture(_filename,_tex_type);
}

void CD3D9Renderer::SetTexClampMode(bool clamp)
{
  byte bRepeat = !clamp;
  if (m_TexMan->m_LastTex)  
  {
    STexPicD3D *ti = (STexPicD3D *)m_TexMan->m_LastTex;
    ti->m_RefTex.bRepeats = bRepeat;
  }
  if (bRepeat != m_RP.m_TexStages[m_TexMan->m_CurStage].Repeat)
  {
    m_RP.m_TexStages[m_TexMan->m_CurStage].Repeat = bRepeat;
    m_pd3dDevice->SetSamplerState(m_TexMan->m_CurStage, D3DSAMP_ADDRESSU, clamp ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
    m_pd3dDevice->SetSamplerState(m_TexMan->m_CurStage, D3DSAMP_ADDRESSV, clamp ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
  }
}

void CD3D9Renderer::TransformTextureMatrix(float x, float y, float angle, float scale)
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
  m->_31 = m->_41;
  m->_32 = m->_42;

  m_pd3dDevice->SetTransform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+m_TexMan->m_CurStage), m);
  m_pd3dDevice->SetTextureStageState(m_TexMan->m_CurStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
}

void CD3D9Renderer::ResetTextureMatrix()
{
  D3DXMATRIX *mt = &m_TexMatrix[m_TexMan->m_CurStage];
  D3DXMatrixIdentity(mt);
  m_pd3dDevice->SetTransform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+m_TexMan->m_CurStage), mt);
  m_pd3dDevice->SetTextureStageState(m_TexMan->m_CurStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
}

void CD3D9Renderer::RemoveTexture(ITexPic * pTexPic)
{
  STexPic * pSTexPic = (STexPic *)pTexPic;
  pSTexPic->Release(false);
}


void CD3D9Renderer::RemoveTexture(unsigned int TextureId)
{
  CD3D9TexMan *tm = (CD3D9TexMan *)m_TexMan;
  TTextureMapItor it = tm->m_RefTexs.find(TextureId);
  STexPic *t;
  if (it == tm->m_RefTexs.end())
    return;
  t = it->second;
  if (t)
    t->Release(false);
  else
    m_TexMan->RemoveFromHash(TextureId, NULL);

  // free shadow maps slot
  for(int i=0; i<MAX_DYNAMIC_SHADOW_MAPS_COUNT; i++)
  {
    if(m_ShadowTexIDBuffer[i].nTexId0 == TextureId)
    {
      iLog->Log("Warning: CD3D9Renderer::RemoveTexture: shadowmap tex slot freed");
      m_ShadowTexIDBuffer[i].nTexId0 = 0;
    }
  }
}

unsigned int CD3D9Renderer::LoadTexture(const char * _filename,int *tex_type,unsigned int def_tid,bool compresstodisk,bool bWarn)
{
  if (def_tid == 0)
    def_tid = -1;
  ITexPic * pPic = EF_LoadTexture((char*)_filename,0,0,eTT_Base, -1, -1, def_tid);
  return pPic->IsTextureLoaded() ? pPic->GetTextureID() : 0;
}

void CD3D9Renderer::UpdateTextureInVideoMemory(uint tnum, unsigned char *newdata,int posx,int posy,int w,int h,ETEX_Format eTFSrc)
{
  if (m_bDeviceLost)
    return;

  SetTexture(tnum);

  D3DFORMAT srcformat = D3DFMT_UNKNOWN;
  if (eTFSrc == eTF_DXT1)
    srcformat = D3DFMT_DXT1;
  else
  if (eTFSrc == eTF_0888)
    srcformat = D3DFMT_X8R8G8B8;
  else
  if (eTFSrc==eTF_8888)
    srcformat = D3DFMT_A8R8G8B8;
  else
  if (eTFSrc==eTF_4444)
    srcformat = D3DFMT_A4R4G4B4;

  if(srcformat==D3DFMT_UNKNOWN)
    assert(0);

  if (tnum > TX_FIRSTBIND && srcformat != D3DFMT_UNKNOWN)
  {
    STexPicD3D *tp = (STexPicD3D *)(m_TexMan->m_Textures[tnum-TX_FIRSTBIND]);
    if (!tp)
      return;
    IDirect3DTexture9 * tex = (IDirect3DTexture9 *) tp->m_RefTex.m_VidTex;

    IDirect3DSurface9 * pSurf;
    tex->GetSurfaceLevel(0, &pSurf);

    if(pSurf)
    {
      RECT rc={posx,posy,posx+w,posy+h};
      D3DSURFACE_DESC desc;
      pSurf->GetDesc(&desc);
      if((eTFSrc==eTF_8888 || eTFSrc==eTF_0888) && (desc.Format==D3DFMT_A8R8G8B8 || desc.Format==D3DFMT_X8R8G8B8))
      {
        D3DLOCKED_RECT lr;
        if(pSurf->LockRect(&lr, &rc, 0)==D3D_OK)
        {
          byte * p = (byte*) lr.pBits;
          for(int y=0; y<h; y++)
          {
            cryMemcpy(&p[y*lr.Pitch], &newdata[y*w*4], w*4);
          }
          pSurf->UnlockRect();
        }
      }
      else
      {
        int size = CD3D9TexMan::TexSize(w, h, srcformat);
        int nPitch;
        if (eTFSrc != eTF_DXT1 && eTFSrc != eTF_DXT3 && eTFSrc != eTF_DXT5)
          nPitch = size / h;
        else
        {
          int blockSize = (eTFSrc == eTF_DXT1) ? 8 : 16;
          nPitch = (w+3)/4 * blockSize;
        }
        
        if((eTFSrc==eTF_8888 || eTFSrc==eTF_0888) && (desc.Format==D3DFMT_R5G6B5))
        {
          D3DLOCKED_RECT lr;
          if(pSurf->LockRect(&lr, &rc, 0)==D3D_OK)
          {
            byte *pBits = (byte *)lr.pBits;
            for (int i=0; i<h; i++)
            {
              uint *src = (uint *)&newdata[i*w*4];
              ushort *dst = (ushort *)&pBits[i*lr.Pitch];
              for (int j=0; j<w; j++)
              {
                uint argb = *src++;
                *dst++ =  ((argb >> 8) & 0xF800) |
                          ((argb >> 5) & 0x07E0) |
                          ((argb >> 3) & 0x001F);
              }
            }
            pSurf->UnlockRect();
          }
        }
        else
        {
          HRESULT hr = D3DXLoadSurfaceFromMemory(pSurf, NULL, &rc, newdata, srcformat, nPitch, NULL, &rc, D3DX_FILTER_NONE, 0);
        }
        SAFE_RELEASE(pSurf);
      }
    }
  }
}

unsigned int CD3D9Renderer::DownLoadToVideoMemory(unsigned char *data,int w, int h, ETEX_Format eTFSrc, ETEX_Format eTFDst, int nummipmap, bool repeat, int filter, int Id, char *szCacheName, int flags)
{
  char name[128];
  if (!szCacheName)
    sprintf(name, "$AutoDownload_%d", m_TexGenID++);
  else
  {
    // WORKAROUND: NVidia driver bug during playing of video file
    // Solution: Never remove video texture
    /*if (!strcmp(szCacheName, "$VideoPanel"))
    {
      if (m_TexMan->IsTextureLoaded(szCacheName))
      {
        STexPic *tp = m_TexMan->GetByName(szCacheName);
        return tp->GetTextureID();
      }
    }*/
    strcpy(name, szCacheName);
  }
  int flags2 = FT2_NODXT;
  if (!nummipmap)
  {
    if (filter != FILTER_BILINEAR && filter != FILTER_TRILINEAR)
      flags |= FT_NOMIPS;
  }
  int DXTSize = 0;
  int blockSize = 0;
  if (eTFDst == eTF_DXT1)
  {
    flags |= FT_DXT1;
    blockSize = 8;
  }
  else
  if (eTFDst == eTF_DXT3)
  {
    flags |= FT_DXT3;
    blockSize = 16;
  }
  else
  if (eTFDst == eTF_DXT5)
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
  if (eTFSrc == eTF_RGBA)
  {
    for (int i=0; i<h; i++)
    {
      byte *dst = &data[i*w*4];
      for (int j=0; j<w; j++)
      {
        Exchange(dst[j*4], dst[j*4+2]);
      }
    }
  }

  if (!repeat)
    flags |= FT_CLAMP;
  if (eTFDst == eTF_RGBA)
    flags |= FT_HASALPHA;
  if (!szCacheName)
    flags |= FT_NOSTREAM;
  else
    flags2 |= FT2_DISCARDINCACHE;
  flags2 |= FT2_NOANISO;

  STexPic *tp;

  int tnum = Id;

  unsigned char * pData = 0;

  if(eTFSrc == eTFDst)
    pData = data;
  else
    pData = new unsigned char[w * h * 4];

  if (tnum > TX_FIRSTBIND)
  {
    tp = (STexPicD3D *)(m_TexMan->m_Textures[tnum-TX_FIRSTBIND]);
    tp = m_TexMan->CreateTexture(name, w, h, 1, flags, flags2, pData , eTT_Base, -1.0f, -1.0f, DXTSize, tp, 0, eTFDst);
  }
  else
  {
    tp = m_TexMan->CreateTexture(name, w, h, 1, flags, flags2, pData , eTT_Base, -1.0f, -1.0f, DXTSize, NULL, 0, eTFDst);
  }

  if(data && eTFSrc != eTFDst && !m_bDeviceLost)
    UpdateTextureInVideoMemory( tp->m_Bind, data, 0 , 0, w, h, eTFSrc);

  if(pData && eTFSrc != eTFDst)
    delete[] pData;
  //tp->SaveJPG("bug.jpg", false);

  return (tp->m_Bind);  
}

void CD3D9Renderer::SetTexture(int tnum, ETexType Type)
{
  m_TexMan->SetTexture(tnum, Type);
}

bool CD3D9Renderer::EF_SetLightHole(Vec3d vPos, Vec3d vNormal, int idTex, float fScale, bool bAdditive)
{
  return false;
}

char*	CD3D9Renderer::GetVertexProfile(bool bSupportedProfile)
{
  CGprofile pr = CG_PROFILE_VS_1_1;

  if (bSupportedProfile)
  {
#ifndef WIN64
    pr = cgD3D9GetLatestVertexProfile();
#else
    if (GetFeatures() & RFT_HW_PS20)
      pr = CG_PROFILE_VS_2_0;
#endif
  }

  switch(pr)
  {
    case CG_PROFILE_VS_1_1:
      return "PROFILE_VS_1_1";
    case CG_PROFILE_VS_2_0:
    case CG_PROFILE_VS_2_X:
      return "PROFILE_VS_2_0";
    default:
      return "Unknown";
  }
}

char*	CD3D9Renderer::GetPixelProfile(bool bSupportedProfile)
{
  CGprofile pr = CG_PROFILE_PS_1_1;

  if (bSupportedProfile)
  {
#ifndef WIN64
		pr = 	cgD3D9GetLatestPixelProfile();
    if (pr == CG_PROFILE_PS_1_2 || pr == CG_PROFILE_PS_1_3)
      pr = CG_PROFILE_PS_1_1;
#else
    if (GetFeatures() & RFT_HW_PS20)
    {
      if ((GetFeatures() & RFT_HW_MASK) == RFT_HW_GFFX)
        pr = CG_PROFILE_PS_2_X;
      else
        pr = CG_PROFILE_PS_2_0;
    }
#endif
  }

  switch(pr)
  {
    case CG_PROFILE_PS_1_1:
    case CG_PROFILE_PS_1_2:
    case CG_PROFILE_PS_1_3:
      return "PROFILE_PS_1_1";
    case CG_PROFILE_PS_2_0:
    case CG_PROFILE_PS_2_X:
      return "PROFILE_PS_2_0";
    default:
      return "Unknown";
  }
}

void CD3D9Renderer::EF_PolygonOffset(bool bEnable, float fFactor, float fUnits)
{
  if (bEnable)
  {
    float fOffs = -(float)fFactor;
    m_pd3dDevice->SetRenderState( D3DRS_DEPTHBIAS, *(DWORD*)&fOffs );
  }
  else
  {
    m_pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, 0);
  }
}

void CD3D9Renderer::GetMemoryUsage(ICrySizer* Sizer)
{
  int i, nSize;

  assert (Sizer);

  //SIZER_COMPONENT_NAME(Sizer, "GLRenderer");
  {
    SIZER_COMPONENT_NAME(Sizer, "Renderer static");
    for (i=0; i<NUMRI_LISTS; i++)
    {
      Sizer->AddObject(&SRendItem::m_RendItems[i], SRendItem::m_RendItems[i].GetMemoryUsage());
    }
  }
  {
    SIZER_COMPONENT_NAME(Sizer, "Renderer dynamic");

    Sizer->Add(*this);
    Sizer->AddObject(&CCObject::m_Waves, CCObject::m_Waves.GetMemoryUsage());
    Sizer->AddObject(m_RP.m_VisObjects, MAX_REND_OBJECTS*sizeof(CCObject *));
    Sizer->AddObject(&m_RP.m_TempObjects, m_RP.m_TempObjects.GetMemoryUsage());
    Sizer->AddObject(&m_RP.m_Objects, m_RP.m_Objects.GetMemoryUsage());
    Sizer->AddObject(&m_RP.m_ObjectsPool, m_RP.m_nNumObjectsInPool * sizeof(CCObject));
    Sizer->AddObject(&m_RP.m_DLights, m_RP.m_DLights[1].GetMemoryUsage());
    Sizer->AddObject(&m_RP.m_Splashes, m_RP.m_Splashes.GetMemoryUsage());
    nSize = 0;
    for (i=0; i<4; i++)
    {
      nSize += CREClientPoly::mPolysStorage[i].GetMemoryUsage();
    }
    nSize += CREClientPoly2D::mPolysStorage.GetMemoryUsage();
    Sizer->AddObject(&CREClientPoly2D::mPolysStorage, nSize);
    Sizer->AddObject(&m_RP.m_Sprites, m_RP.m_Sprites.GetMemoryUsage());
    nSize = 0;
    for (i=0; i<2; i++)
    {
      nSize += m_RP.m_TempMeshes[i].GetMemoryUsage();
    }
    Sizer->AddObject(&m_RP.m_TempMeshes, nSize);
    Sizer->AddObject(&gCurLightStyles, gCurLightStyles.GetMemoryUsage());
    Sizer->AddObject(&CName::mDuplicate, CName::Size());
  }


  {
    SIZER_COMPONENT_NAME(Sizer, "Shaders");
    nSize = SShader::m_Shaders_known.GetMemoryUsage();
    Sizer->AddObject(&SShader::m_Shaders_known, nSize);
    for (i=0; i<SShader::m_Shaders_known.Num(); i++)
    {
      SShader *pSH = SShader::m_Shaders_known[i];
      if (!pSH)
        continue;
      nSize = pSH->Size(0);
      Sizer->AddObject(pSH, nSize);
    }
    {
      SIZER_COMPONENT_NAME(Sizer, "Shader manager");
      Sizer->AddObject(&m_cEF, m_cEF.Size());
    }
    {
      SIZER_COMPONENT_NAME(Sizer, "Shader resources");
      nSize = SShader::m_ShaderResources_known.GetMemoryUsage();
      Sizer->AddObject(&SShader::m_ShaderResources_known, nSize);
      for (i=0; i<SShader::m_ShaderResources_known.Num(); i++)
      {
        SRenderShaderResources *pSHR = SShader::m_ShaderResources_known[i];
        if (!pSHR)
          continue;
        nSize = pSHR->Size();
        Sizer->AddObject(pSHR, nSize);
      }
    }
    {
      SIZER_COMPONENT_NAME(Sizer, "Shader shared components");

      for (i=0; i<SParamComp::m_ParamComps.Num(); i++)
      {
        nSize = SParamComp::m_ParamComps[i]->Size();
        Sizer->AddObject(SParamComp::m_ParamComps[i], nSize);
      }

      for (i=0; i<CVProgram::m_VPrograms.Num(); i++)
      {
        if (CVProgram::m_VPrograms[i])
        {
          nSize = CVProgram::m_VPrograms[i]->Size();
          Sizer->AddObject(CVProgram::m_VPrograms[i], nSize);
        }
      }
      // Shared vshaders scripts
      nSize = CCGVProgram_D3D::m_CGScripts.GetMemoryUsage();
      for (i=0; i<CCGVProgram_D3D::m_CGScripts.Num(); i++)
      {
        if (CCGVProgram_D3D::m_CGScripts[i])
          nSize += CCGVProgram_D3D::m_CGScripts[i]->Size(true);
      }
      Sizer->AddObject(&CCGVProgram_D3D::m_CGScripts, nSize);

      for (i=0; i<CPShader::m_PShaders.Num(); i++)
      {
        if (CPShader::m_PShaders[i])
        {
          nSize = CPShader::m_PShaders[i]->Size();
          Sizer->AddObject(CPShader::m_PShaders[i], nSize);
        }
      }

      for (i=0; i<CLightStyle::m_LStyles.Num(); i++)
      {
        nSize = CLightStyle::m_LStyles[i]->Size();
        Sizer->AddObject(CLightStyle::m_LStyles[i], nSize);
      }

      for (i=0; i<SLightMaterial::known_materials.Num(); i++)
      {
        if (SLightMaterial::known_materials[i])
        {
          nSize = SLightMaterial::known_materials[i]->Size();
          Sizer->AddObject(SLightMaterial::known_materials[i], nSize);
        }
      }
    }
  }
  {
    SIZER_COMPONENT_NAME(Sizer, "Mesh");

    CLeafBuffer *pLB = CLeafBuffer::m_RootGlobal.m_NextGlobal;
    int nNums = 0;
    int nSizeTotal = 0;
    while (pLB != &CLeafBuffer::m_RootGlobal)
    {
      nSize = pLB->Size(0);
      Sizer->AddObject(pLB, nSize);
      pLB = pLB->m_NextGlobal;
      nSizeTotal += nSize;
      nNums++;
    }
    {
      SIZER_COMPONENT_NAME(Sizer, "API Mesh size");

      nNums = 0;
      nSize = 0;
      CLeafBuffer *pLB = CLeafBuffer::m_Root.m_Next;
      while (pLB != &CLeafBuffer::m_Root)
      {
        nSize += pLB->Size(1);
        pLB = pLB->m_Next;
        nNums++;
      }
      Sizer->AddObject(&CLeafBuffer::m_Root.m_Next, nSize);
    }
    {
      SIZER_COMPONENT_NAME(Sizer, "API Mesh indices size");

      nNums = 0;
      nSize = 0;
      CLeafBuffer *pLB = CLeafBuffer::m_Root.m_Next;
      while (pLB != &CLeafBuffer::m_Root)
      {
        nSize += pLB->Size(2);
        pLB = pLB->m_Next;
        nNums++;
      }
      Sizer->AddObject(&CLeafBuffer::m_Root.m_Prev, nSize);
    }
  }
  {
    SIZER_COMPONENT_NAME(Sizer, "Render elements");

    CRendElement *pRE = CRendElement::m_RootGlobal.m_NextGlobal;
    while (pRE != &CRendElement::m_RootGlobal)
    {
      nSize = pRE->Size();
      Sizer->AddObject(pRE, nSize);
      pRE = pRE->m_NextGlobal;
    }
  }
  {
    SIZER_COMPONENT_NAME(Sizer, "Texture Objects");

    for (i=0; i<gRenDev->m_TexMan->m_Textures.Num(); i++)
    {
      STexPic *tp = gRenDev->m_TexMan->m_Textures[i];
      if (!tp)
        continue;
      nSize = tp->Size(0);
      Sizer->AddObject(tp, nSize);
    }
    Sizer->AddObject(&gRenDev->m_TexMan->m_Textures[0], gRenDev->m_TexMan->m_Textures.GetMemoryUsage());
    Sizer->AddObject(&gRenDev->m_TexMan->m_FreeSlots[0], gRenDev->m_TexMan->m_FreeSlots.GetMemoryUsage());
    Sizer->AddObject(&gRenDev->m_TexMan->m_TexData[0], 256*256*4);
    Sizer->AddObject(&gRenDev->m_TexMan->m_EnvCMaps[0], sizeof(SEnvTexture)*MAX_ENVCUBEMAPS);
    Sizer->AddObject(&gRenDev->m_TexMan->m_EnvTexts[0], sizeof(SEnvTexture)*MAX_ENVTEXTURES);
    Sizer->AddObject(&gRenDev->m_TexMan->m_CustomCMaps[0], sizeof(SEnvTexture)*16);
    Sizer->AddObject(&gRenDev->m_TexMan->m_CustomTextures[0], sizeof(SEnvTexture)*16);
    Sizer->AddObject(&gRenDev->m_TexMan->m_Templates[0], sizeof(STexPic)*EFTT_MAX);
  }
  {
    SIZER_COMPONENT_NAME(Sizer, "API Texture memory");

    nSize = 0;
    for (i=0; i<gRenDev->m_TexMan->m_Textures.Num(); i++)
    {
      STexPic *tp = gRenDev->m_TexMan->m_Textures[i];
      if (!tp || !tp->m_bBusy)
        continue;
      nSize += tp->m_Size;
    }
    Sizer->AddObject(&gRenDev->m_TexMan->m_Textures[0]->m_Size, nSize);
  }
}

//====================================================================

ILog     *iLog;
IConsole *iConsole;
ITimer   *iTimer;
ISystem  *iSystem;
int *pTest_int;
IPhysicalWorld *pIPhysicalWorld;

ISystem *GetISystem()
{
  return iSystem;
}

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
  gRenDev = (CRenderer *) (new CD3D9Renderer());
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif

  srand( GetTickCount() );

  g_SecondsPerCycle = iSystem->GetSecondsPerCycle();
  g_CpuFlags = iSystem->GetCPUFlags();

  return gRenDev;
}

void *gGet_D3DDevice()
{
  return (void *)gcpRendD3D->m_pd3dDevice;
}
void *gGet_glReadPixels()
{
  return NULL;
}

//=========================================================================================

//=========================================================================================
