/*=============================================================================
  PS2_System.cpp : HW depended PS2 functions and extensions handling.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;


#include "RenderPCH.h"
#include "NULL_Renderer.h"


bool CNULLRenderer::SetGammaDelta(const float fGamma)
{
  m_fDeltaGamma = fGamma;
  return true;
}

void CNULLRenderer::SetGamma(float fGamma)
{
}

void CNULLRenderer::MakeCurrent()
{
}

void CNULLRenderer::ShareResources( IRenderer *renderer )
{
}


int CNULLRenderer::EnumDisplayFormats(TArray<SDispFormat>& Formats, bool bReset)
{
  return 0;
}

bool CNULLRenderer::ChangeResolution(int nNewWidth, int nNewHeight, int nNewColDepth, int nNewRefreshHZ, bool bFullScreen)
{
  return false;
}

void CNULLRenderer::PS2SetDefaultState()
{
}

WIN_HWND CNULLRenderer::Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen,WIN_HINSTANCE hinst, WIN_HWND Glhwnd, WIN_HDC Glhdc, WIN_HGLRC hGLrc, bool bReInit)
{
  //=======================================
  // Add init code here
  //=======================================

  PS2SetDefaultState();

  SetPolygonMode(R_SOLID_MODE);

  SetGamma(CV_r_gamma+m_fDeltaGamma);

  m_width = width;
  m_height = height;
  
  if (bReInit)
  {
    iLog->Log("Reload textures\n");
    RefreshResources(0);
  }

  iLog->Log("Init Shaders\n");

  gRenDev->m_cEF.mfInit();
  EF_PipelineInit();

#if defined(LINUX)
	return (WIN_HWND)this;//it just get checked against NULL anyway
#else
  return (WIN_HWND)GetDesktopWindow();
#endif
}


bool CNULLRenderer::SetCurrentContext(WIN_HWND hWnd)
{
  return true;
}

bool CNULLRenderer::CreateContext(WIN_HWND hWnd, bool bAllowFSAA)
{
  return true;
}

bool CNULLRenderer::DeleteContext(WIN_HWND hWnd)
{
  return true;
}

void CNULLRenderer::RefreshResources(int nFlags)
{
  if (nFlags & FRO_TEXTURES)
    m_TexMan->ReloadAll(nFlags);
  if (nFlags & (FRO_SHADERS | FRO_SHADERTEXTURES))
    gRenDev->m_cEF.mfReloadAllShaders(nFlags);
}

void CNULLRenderer::ShutDown(bool bReInit)
{
  FreeResources(FRR_ALL);
  EF_PipelineShutdown();
  CName::mfExitSubsystem();
}


//=======================================================================

ILog     *iLog;
IConsole *iConsole;
ITimer   *iTimer;
ISystem  *iSystem;
//CVars    *cVars;
int *pTest_int;
//CryCharManager *pCharMan;
IPhysicalWorld *pIPhysicalWorld;

ISystem* GetISystem()
{
	return iSystem;
}

extern "C" DLL_EXPORT IRenderer* PackageRenderConstructor(int argc, char* argv[], SCryRenderInterface *sp);
DLL_EXPORT IRenderer* PackageRenderConstructor(int argc, char* argv[], SCryRenderInterface *sp)
{
  gbRgb = false;

  iConsole  = sp->ipConsole;
  iLog      = sp->ipLog;
  iTimer    = sp->ipTimer;
  iSystem   = sp->ipSystem;
//  cVars     = sp->ipVars;
  pTest_int = sp->ipTest_int;
	pIPhysicalWorld = sp->pIPhysicalWorld;
//  pCharMan = sp->ipCharMan;

#ifdef DEBUGALLOC
#undef new
#endif
  CRenderer *rd = (CRenderer *) (new CNULLRenderer());
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif

#ifdef LINUX
	srand( clock() );
#else
  srand( GetTickCount() );
#endif

  return rd;
}

void *gGet_D3DDevice()
{
  return NULL;
}
void *gGet_glReadPixels()
{
  return NULL;
}
