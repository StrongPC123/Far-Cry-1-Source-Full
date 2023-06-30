#include "RenderPCH.h"
#include "DriverD3D9.h"

#define ecase(e) case e: return #e

static char sStr[1024];
#define edefault(e) default: sprintf(sStr, "0x%x", e); return sStr;

#define eflag(e)  \
  if (Flag & e)   \
  {               \
    if (sStr[0])  \
      strcat (sStr, " | "); \
    strcat(sStr, #e);       \
  }               \

static char *sHex (DWORD Value)
{
  sprintf(sStr, "0x%x", Value);
  return sStr;
}
static char *sInt (DWORD Value)
{
  sprintf(sStr, "%d", Value);
  return sStr;
}
static char *sDWFloat (DWORD Value)
{
  float fVal = *((float*)(&Value));
  sprintf(sStr, "%.3f", fVal);
  return sStr;
}

static char *sBool (DWORD Value)
{
  switch (Value)
  {
    ecase (FALSE);
    ecase (TRUE);
    edefault (Value);
  }
}

static char *sD3DZB (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DZB_FALSE);
    ecase (D3DZB_TRUE);
    ecase (D3DZB_USEW);
    edefault (Value);
  }
}

static char *sD3DFILL (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DFILL_POINT);
    ecase (D3DFILL_WIREFRAME);
    ecase (D3DFILL_SOLID);
    edefault (Value);
  }
}

static char *sD3DSHADE (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DSHADE_FLAT);
    ecase (D3DSHADE_GOURAUD);
    ecase (D3DSHADE_PHONG);
    edefault (Value);
  }
}

static char *sD3DBLEND (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DBLEND_ZERO);
    ecase (D3DBLEND_ONE);
    ecase (D3DBLEND_SRCCOLOR);
    ecase (D3DBLEND_INVSRCCOLOR);
    ecase (D3DBLEND_SRCALPHA);
    ecase (D3DBLEND_INVSRCALPHA);
    ecase (D3DBLEND_DESTALPHA);
    ecase (D3DBLEND_INVDESTALPHA);
    ecase (D3DBLEND_DESTCOLOR);
    ecase (D3DBLEND_INVDESTCOLOR);
    ecase (D3DBLEND_SRCALPHASAT);
    ecase (D3DBLEND_BOTHSRCALPHA);
    ecase (D3DBLEND_BOTHINVSRCALPHA);
    ecase (D3DBLEND_BLENDFACTOR);
    ecase (D3DBLEND_INVBLENDFACTOR);
    edefault (Value);
  }
}

static char *sD3DCULL (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DCULL_NONE);
    ecase (D3DCULL_CW);
    ecase (D3DCULL_CCW);
    edefault (Value);
  }
}

static char *sD3DCMP (DWORD Value)
{
  switch(Value)
  {
    ecase (D3DCMP_NEVER);
    ecase (D3DCMP_LESS);
    ecase (D3DCMP_EQUAL);
    ecase (D3DCMP_LESSEQUAL);
    ecase (D3DCMP_GREATER);
    ecase (D3DCMP_NOTEQUAL);
    ecase (D3DCMP_GREATEREQUAL);
    ecase (D3DCMP_ALWAYS);
    edefault (Value);
  }
}

static char *sD3DFOG (DWORD Value)
{
  switch(Value)
  {
    ecase (D3DFOG_NONE);
    ecase (D3DFOG_EXP);
    ecase (D3DFOG_EXP2);
    ecase (D3DFOG_LINEAR);
    edefault (Value);
  }
}

static char *sD3DSTENCILOP (DWORD Value)
{
  switch(Value)
  {
    ecase (D3DSTENCILOP_KEEP);
    ecase (D3DSTENCILOP_ZERO);
    ecase (D3DSTENCILOP_REPLACE);
    ecase (D3DSTENCILOP_INCRSAT);
    ecase (D3DSTENCILOP_DECRSAT);
    ecase (D3DSTENCILOP_INVERT);
    ecase (D3DSTENCILOP_INCR);
    ecase (D3DSTENCILOP_DECR);
    edefault (Value);
  }
}

static char *sD3DMCS (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DMCS_MATERIAL);
    ecase (D3DMCS_COLOR1);
    ecase (D3DMCS_COLOR2);
    edefault (Value);
  }
}

static char *sD3DVBF (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DVBF_DISABLE);
    ecase (D3DVBF_1WEIGHTS);
    ecase (D3DVBF_2WEIGHTS);
    ecase (D3DVBF_3WEIGHTS);
    ecase (D3DVBF_TWEENING);
    ecase (D3DVBF_0WEIGHTS);
    edefault (Value);
  }
}

static char *sD3DPATCHEDGE (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DPATCHEDGE_DISCRETE);
    ecase (D3DPATCHEDGE_CONTINUOUS);
    edefault (Value);
  }
}

static char *sD3DDMT (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DDMT_ENABLE);
    ecase (D3DDMT_DISABLE);
    edefault (Value);
  }
}

static char *sD3DBLENDOP (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DBLENDOP_ADD);
    ecase (D3DBLENDOP_SUBTRACT);
    ecase (D3DBLENDOP_REVSUBTRACT);
    ecase (D3DBLENDOP_MIN);
    ecase (D3DBLENDOP_MAX);
    edefault (Value);
  }
}

static char *sD3DDEGREE (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DDEGREE_LINEAR);
    ecase (D3DDEGREE_QUADRATIC);
    ecase (D3DDEGREE_CUBIC);
    ecase (D3DDEGREE_QUINTIC);
    edefault (Value);
  }
}

static char *sD3DTOP (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DTOP_DISABLE);
    ecase (D3DTOP_SELECTARG1);
    ecase (D3DTOP_SELECTARG2);
    ecase (D3DTOP_MODULATE);
    ecase (D3DTOP_MODULATE2X);
    ecase (D3DTOP_MODULATE4X);
    ecase (D3DTOP_ADD);
    ecase (D3DTOP_ADDSIGNED);
    ecase (D3DTOP_ADDSIGNED2X);
    ecase (D3DTOP_SUBTRACT);
    ecase (D3DTOP_ADDSMOOTH);
    ecase (D3DTOP_BLENDDIFFUSEALPHA);
    ecase (D3DTOP_BLENDTEXTUREALPHA);
    ecase (D3DTOP_BLENDFACTORALPHA);
    ecase (D3DTOP_BLENDTEXTUREALPHAPM);
    ecase (D3DTOP_BLENDCURRENTALPHA);
    ecase (D3DTOP_PREMODULATE);
    ecase (D3DTOP_MODULATEALPHA_ADDCOLOR);
    ecase (D3DTOP_MODULATECOLOR_ADDALPHA);
    ecase (D3DTOP_MODULATEINVALPHA_ADDCOLOR);
    ecase (D3DTOP_MODULATEINVCOLOR_ADDALPHA);
    ecase (D3DTOP_BUMPENVMAP);
    ecase (D3DTOP_BUMPENVMAPLUMINANCE);
    ecase (D3DTOP_DOTPRODUCT3);
    ecase (D3DTOP_MULTIPLYADD);
    ecase (D3DTOP_LERP);
    edefault (Value);
  }
}

static char *sD3DPT (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DPT_POINTLIST);
    ecase (D3DPT_LINELIST);
    ecase (D3DPT_LINESTRIP);
    ecase (D3DPT_TRIANGLELIST);
    ecase (D3DPT_TRIANGLESTRIP);
    ecase (D3DPT_TRIANGLEFAN);
    edefault (Value);
  }
}

static char *sD3DTA (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DTA_CURRENT);
    ecase (D3DTA_DIFFUSE);
    ecase (D3DTA_SELECTMASK);
    ecase (D3DTA_SPECULAR);
    ecase (D3DTA_TEMP);
    ecase (D3DTA_TEXTURE);
    ecase (D3DTA_TFACTOR);
    edefault (Value);
  }
}

static char *sTEXCOORDINDEX (DWORD Value)
{
  sprintf(sStr, "%d", Value & 0xf);
  if (Value & D3DTSS_TCI_CAMERASPACENORMAL)
    strcat (sStr, " | D3DTSS_TCI_CAMERASPACENORMAL");
  else
  if (Value & D3DTSS_TCI_CAMERASPACEPOSITION)
    strcat (sStr, " | D3DTSS_TCI_CAMERASPACEPOSITION");
  else
  if (Value & D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR)
    strcat (sStr, " | D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR");
  else
  if (Value & D3DTSS_TCI_SPHEREMAP)
    strcat (sStr, " | D3DTSS_TCI_SPHEREMAP");
  else
    strcat (sStr, " | D3DTSS_TCI_PASSTHRU");

  return sStr;
}

static char *sD3DTTFF (DWORD Value)
{
  switch(Value & 0xf)
  {
    case D3DTTFF_DISABLE:
      strcpy(sStr, "D3DTTFF_DISABLE");
  	  break;
    case D3DTTFF_COUNT1:
      strcpy(sStr, "D3DTTFF_COUNT1");
  	  break;
    case D3DTTFF_COUNT2:
      strcpy(sStr, "D3DTTFF_COUNT2");
      break;
    case D3DTTFF_COUNT3:
      strcpy(sStr, "D3DTTFF_COUNT3");
      break;
    case D3DTTFF_COUNT4:
      strcpy(sStr, "D3DTTFF_COUNT4");
      break;
    default:
      sprintf(sStr, "0x%x", Value & 0xf);
  }
  if (Value & D3DTTFF_PROJECTED)
    strcat (sStr, " | D3DTTFF_PROJECTED");

  return sStr;
}

static char *sD3DTADDRESS (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DTADDRESS_WRAP);
    ecase (D3DTADDRESS_MIRROR);
    ecase (D3DTADDRESS_CLAMP);
    ecase (D3DTADDRESS_BORDER);
    ecase (D3DTADDRESS_MIRRORONCE);
    edefault (Value);
  }
}

static char *sD3DTEXF (DWORD Value)
{
  switch (Value)
  {
    ecase (D3DTEXF_NONE);
    ecase (D3DTEXF_POINT);
    ecase (D3DTEXF_LINEAR);
    ecase (D3DTEXF_ANISOTROPIC);
    ecase (D3DTEXF_PYRAMIDALQUAD);
    ecase (D3DTEXF_GAUSSIANQUAD);
    edefault (Value);
  }
}

static char *sD3DTS (DWORD Value)
{
  switch(Value)
  {
    ecase (D3DTS_VIEW);
    ecase (D3DTS_PROJECTION);
    ecase (D3DTS_TEXTURE0);
    ecase (D3DTS_TEXTURE1);
    ecase (D3DTS_TEXTURE2);
    ecase (D3DTS_TEXTURE3);
    ecase (D3DTS_TEXTURE4);
    ecase (D3DTS_TEXTURE5);
    ecase (D3DTS_TEXTURE6);
    ecase (D3DTS_TEXTURE7);
    edefault (Value);
  }
}

static char *sD3DCLEARFLAGS (DWORD Value)
{
  if (!(Value & (D3DCLEAR_TARGET | D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER)))
    sprintf(sStr, "0x%x", Value);
  else
  {
    sStr[0] = 0;
    if (Value & D3DCLEAR_TARGET)
      strcat(sStr, "D3DCLEAR_TARGET");
    if (Value & D3DCLEAR_ZBUFFER)
    {
      if (sStr[0])
        strcat (sStr, " | ");
      strcat(sStr, "D3DCLEAR_ZBUFFER");
    }
    if (Value & D3DCLEAR_STENCIL)
    {
      if (sStr[0])
        strcat (sStr, " | ");
      strcat(sStr, "D3DCLEAR_STENCIL");
    }
  }

  return sStr;
}

static char *sFVF (DWORD Flag)
{
  sStr[0] = 0;
  eflag(D3DFVF_XYZ);
  eflag(D3DFVF_XYZRHW);
  eflag(D3DFVF_NORMAL);
  eflag(D3DFVF_PSIZE);
  eflag(D3DFVF_DIFFUSE);
  eflag(D3DFVF_SPECULAR);
  char ss[128];
  if(Flag & 0xf00)
  {
    sprintf(ss, "D3DFVF_TEX%d", (Flag >> 8)&0xf);
    if (sStr[0])
      strcat (sStr, " | ");
    strcat(sStr, ss);
  }

  return sStr;
}

//=========================================================================================

HRESULT CMyDirect3DDevice9::QueryInterface(REFIID riid, void** ppvObj)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::QueryInterface");
  return gcpRendD3D->m_pActuald3dDevice->QueryInterface(riid, ppvObj);
}

ULONG CMyDirect3DDevice9::AddRef()
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::AddRef");
  return gcpRendD3D->m_pActuald3dDevice->AddRef();
}
ULONG CMyDirect3DDevice9::Release()
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::Release");
  return gcpRendD3D->m_pActuald3dDevice->Release();
}

  /*** IDirect3DDevice9 methods ***/
HRESULT CMyDirect3DDevice9::TestCooperativeLevel()
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::TestCooperativeLevel");
  return gcpRendD3D->m_pActuald3dDevice->TestCooperativeLevel();
}


UINT CMyDirect3DDevice9::GetAvailableTextureMem()
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetAvailableTextureMem");
  return gcpRendD3D->m_pActuald3dDevice->GetAvailableTextureMem();
}


HRESULT CMyDirect3DDevice9::EvictManagedResources()
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::EvictManagedResources");
  return gcpRendD3D->m_pActuald3dDevice->EvictManagedResources();
}
HRESULT CMyDirect3DDevice9::GetDirect3D(IDirect3D9** ppD3D9)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetDirect3D");
  return gcpRendD3D->m_pActuald3dDevice->GetDirect3D(ppD3D9);
}
HRESULT CMyDirect3DDevice9::GetDeviceCaps(D3DCAPS9* pCaps)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetDeviceCaps");
  return gcpRendD3D->m_pActuald3dDevice->GetDeviceCaps(pCaps);
}
HRESULT CMyDirect3DDevice9::GetDisplayMode(UINT iSwapChain,D3DDISPLAYMODE* pMode)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetDisplayMode");
  return gcpRendD3D->m_pActuald3dDevice->GetDisplayMode(iSwapChain, pMode);
}
HRESULT CMyDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetCreationParameters");
  return gcpRendD3D->m_pActuald3dDevice->GetCreationParameters(pParameters);
}
HRESULT CMyDirect3DDevice9::SetCursorProperties(UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetCursorProperties");
  return gcpRendD3D->m_pActuald3dDevice->SetCursorProperties(XHotSpot,YHotSpot,pCursorBitmap);
}


void CMyDirect3DDevice9::SetCursorPosition(int X,int Y,DWORD Flags)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetCursorPosition");
  return gcpRendD3D->m_pActuald3dDevice->SetCursorPosition(X,Y,Flags);
}

BOOL CMyDirect3DDevice9::ShowCursor(BOOL bShow)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::ShowCursor");
  return gcpRendD3D->m_pActuald3dDevice->ShowCursor(bShow);
}


HRESULT CMyDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::CreateAdditionalSwapChain");
  return gcpRendD3D->m_pActuald3dDevice->CreateAdditionalSwapChain( pPresentationParameters, pSwapChain);
}
HRESULT CMyDirect3DDevice9::GetSwapChain(UINT iSwapChain,IDirect3DSwapChain9** pSwapChain)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetSwapChain");
  return gcpRendD3D->m_pActuald3dDevice->GetSwapChain(iSwapChain,pSwapChain);
}
UINT CMyDirect3DDevice9::GetNumberOfSwapChains()
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetNumberOfSwapChains");
  return gcpRendD3D->m_pActuald3dDevice->GetNumberOfSwapChains();
}    
HRESULT CMyDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::Reset");
  return gcpRendD3D->m_pActuald3dDevice->Reset(pPresentationParameters);
}
HRESULT CMyDirect3DDevice9::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::Present");
  return gcpRendD3D->m_pActuald3dDevice->Present(pSourceRect,pDestRect,hDestWindowOverride,pDirtyRegion);
}
HRESULT CMyDirect3DDevice9::GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetBackBuffer");
  return gcpRendD3D->m_pActuald3dDevice->GetBackBuffer(iSwapChain,iBackBuffer,Type,ppBackBuffer);
}
HRESULT CMyDirect3DDevice9::GetRasterStatus(UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetRasterStatus");
  return gcpRendD3D->m_pActuald3dDevice->GetRasterStatus(iSwapChain,pRasterStatus);
}
HRESULT CMyDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetDialogBoxMode");
  return gcpRendD3D->m_pActuald3dDevice->SetDialogBoxMode(bEnableDialogs);
}
void CMyDirect3DDevice9::SetGammaRamp(UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetGammaRamp");
  return gcpRendD3D->m_pActuald3dDevice->SetGammaRamp(iSwapChain,Flags,pRamp);
}

void CMyDirect3DDevice9::GetGammaRamp(UINT iSwapChain,D3DGAMMARAMP* pRamp)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetGammaRamp");
  return gcpRendD3D->m_pActuald3dDevice->GetGammaRamp(iSwapChain,pRamp);
}

HRESULT CMyDirect3DDevice9::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::CreateTexture");
  return gcpRendD3D->m_pActuald3dDevice->CreateTexture(Width,Height,Levels,Usage,Format,Pool,ppTexture,pSharedHandle);
}
HRESULT CMyDirect3DDevice9::CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::CreateVolumeTexture");
  return gcpRendD3D->m_pActuald3dDevice->CreateVolumeTexture(Width,Height,Depth,Levels,Usage,Format,Pool,ppVolumeTexture,pSharedHandle);
}
HRESULT CMyDirect3DDevice9::CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::CreateCubeTexture");
  return gcpRendD3D->m_pActuald3dDevice->CreateCubeTexture(EdgeLength,Levels,Usage,Format,Pool,ppCubeTexture,pSharedHandle);
}
HRESULT CMyDirect3DDevice9::CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::CreateVertexBuffer");
  return gcpRendD3D->m_pActuald3dDevice->CreateVertexBuffer(Length,Usage,FVF,Pool, ppVertexBuffer, pSharedHandle);
}
HRESULT CMyDirect3DDevice9::CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::CreateIndexBuffer");
  return gcpRendD3D->m_pActuald3dDevice->CreateIndexBuffer(Length,Usage,Format,Pool, ppIndexBuffer,pSharedHandle);
}
HRESULT CMyDirect3DDevice9::CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::CreateRenderTarget");
  return gcpRendD3D->m_pActuald3dDevice->CreateRenderTarget(Width,Height,Format,MultiSample,MultisampleQuality,Lockable, ppSurface,pSharedHandle);
}
HRESULT CMyDirect3DDevice9::CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::CreateDepthStencilSurface");
  return gcpRendD3D->m_pActuald3dDevice->CreateDepthStencilSurface(Width,Height,Format,MultiSample,MultisampleQuality,Discard, ppSurface, pSharedHandle);
}
HRESULT CMyDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::UpdateSurface");
  return gcpRendD3D->m_pActuald3dDevice->UpdateSurface(pSourceSurface,pSourceRect,pDestinationSurface, pDestPoint);
}
HRESULT CMyDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::UpdateTexture");
  return gcpRendD3D->m_pActuald3dDevice->UpdateTexture(pSourceTexture,pDestinationTexture);
}
HRESULT CMyDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetRenderTargetData");
  return gcpRendD3D->m_pActuald3dDevice->GetRenderTargetData(pRenderTarget,pDestSurface);
}
HRESULT CMyDirect3DDevice9::GetFrontBufferData(UINT iSwapChain,IDirect3DSurface9* pDestSurface)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetFrontBufferData");
  return gcpRendD3D->m_pActuald3dDevice->GetFrontBufferData(iSwapChain,pDestSurface);
}
HRESULT CMyDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::StretchRect");
  return gcpRendD3D->m_pActuald3dDevice->StretchRect(pSourceSurface,pSourceRect,pDestSurface,pDestRect,Filter);
}
HRESULT CMyDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::ColorFill");
  return gcpRendD3D->m_pActuald3dDevice->ColorFill(pSurface,pRect,color);
}
HRESULT CMyDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::CreateOffscreenPlainSurface");
  return gcpRendD3D->m_pActuald3dDevice->CreateOffscreenPlainSurface(Width,Height,Format,Pool, ppSurface, pSharedHandle);
}
HRESULT CMyDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetRenderTarget");
  return gcpRendD3D->m_pActuald3dDevice->SetRenderTarget(RenderTargetIndex,pRenderTarget);
}
HRESULT CMyDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetRenderTarget");
  return gcpRendD3D->m_pActuald3dDevice->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}
HRESULT CMyDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetDepthStencilSurface");
  return gcpRendD3D->m_pActuald3dDevice->SetDepthStencilSurface(pNewZStencil);
}
HRESULT CMyDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetDepthStencilSurface");
  return gcpRendD3D->m_pActuald3dDevice->GetDepthStencilSurface(ppZStencilSurface);
}
HRESULT CMyDirect3DDevice9::BeginScene()
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::BeginScene");
  return gcpRendD3D->m_pActuald3dDevice->BeginScene();
}
HRESULT CMyDirect3DDevice9::EndScene()
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::EndScene");
  return gcpRendD3D->m_pActuald3dDevice->EndScene();
}
HRESULT CMyDirect3DDevice9::Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%d, 0x%x, %s, 0x%x, %.3f, 0x%x)\n", "D3DDevice::Clear", Count, pRects, sD3DCLEARFLAGS(Flags), Color, Z, Stencil);
  return gcpRendD3D->m_pActuald3dDevice->Clear(Count,pRects,Flags,Color,Z,Stencil);
}
HRESULT CMyDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
  char *state = sD3DTS(State);
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%s, 0x%x)\n", "D3DDevice::SetTransform", state, pMatrix);
  return gcpRendD3D->m_pActuald3dDevice->SetTransform(State,pMatrix);
}
HRESULT CMyDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetTransform");
  return gcpRendD3D->m_pActuald3dDevice->GetTransform(State,pMatrix);
}
HRESULT CMyDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::MultiplyTransform");
  return gcpRendD3D->m_pActuald3dDevice->MultiplyTransform(State,pMatrix);
}
HRESULT CMyDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%d, %d, %d, %d, %.3f, %.3f)\n", "D3DDevice::SetViewport", pViewport->X, pViewport->Y, pViewport->Width, pViewport->Height, pViewport->MinZ, pViewport->MaxZ);
  return gcpRendD3D->m_pActuald3dDevice->SetViewport(pViewport);
}
HRESULT CMyDirect3DDevice9::GetViewport(D3DVIEWPORT9* pViewport)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetViewport");
  return gcpRendD3D->m_pActuald3dDevice->GetViewport(pViewport);
}
HRESULT CMyDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetMaterial");
  return gcpRendD3D->m_pActuald3dDevice->SetMaterial(pMaterial);
}
HRESULT CMyDirect3DDevice9::GetMaterial(D3DMATERIAL9* pMaterial)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetMaterial");
  return gcpRendD3D->m_pActuald3dDevice->GetMaterial(pMaterial);
}
HRESULT CMyDirect3DDevice9::SetLight(DWORD Index,CONST D3DLIGHT9* pLight)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%d, 0x%x)\n", "D3DDevice::SetLight", Index, pLight);
  return gcpRendD3D->m_pActuald3dDevice->SetLight(Index,pLight);
}
HRESULT CMyDirect3DDevice9::GetLight(DWORD Index,D3DLIGHT9* pLight)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetLight");
  return gcpRendD3D->m_pActuald3dDevice->GetLight(Index,pLight);
}
HRESULT CMyDirect3DDevice9::LightEnable(DWORD Index,BOOL Enable)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%d, %s)\n", "D3DDevice::LightEnable", Index, sBool(Enable));
  return gcpRendD3D->m_pActuald3dDevice->LightEnable(Index,Enable);
}
HRESULT CMyDirect3DDevice9::GetLightEnable(DWORD Index,BOOL* pEnable)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetLightEnable");
  return gcpRendD3D->m_pActuald3dDevice->GetLightEnable(Index,pEnable);
}
HRESULT CMyDirect3DDevice9::SetClipPlane(DWORD Index,CONST float* pPlane)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetClipPlane");
  return gcpRendD3D->m_pActuald3dDevice->SetClipPlane(Index,pPlane);
}
HRESULT CMyDirect3DDevice9::GetClipPlane(DWORD Index,float* pPlane)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetClipPlane");
  return gcpRendD3D->m_pActuald3dDevice->GetClipPlane(Index,pPlane);
}
HRESULT CMyDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State,DWORD Value)
{
#define ECASE(e) case e: state = #e
  char Str[1024];
#define EDEFAULT(e) default: sprintf(Str, "0x%x", e); state = Str;
  char *state;
  char *val = NULL;
  switch(State)
  {
    ECASE (D3DRS_ZENABLE);
    val = sD3DZB(Value);
    break;
    ECASE (D3DRS_FILLMODE);
    val = sD3DFILL(Value);
    break;
    ECASE (D3DRS_SHADEMODE);
    val = sD3DSHADE(Value);
    break;
    ECASE (D3DRS_ZWRITEENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_ALPHATESTENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_SRCBLEND);
    val = sD3DBLEND(Value);
    break;
    ECASE (D3DRS_DESTBLEND);
    val = sD3DBLEND(Value);
    break;
    ECASE (D3DRS_CULLMODE);
    val = sD3DCULL(Value);
    break;
    ECASE (D3DRS_ZFUNC);
    val = sD3DCMP(Value);
    break;
    ECASE (D3DRS_ALPHAREF);
    val = sInt(Value);
    break;
    ECASE (D3DRS_ALPHAFUNC);
    val = sD3DCMP(Value);
    break;
    ECASE (D3DRS_DITHERENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_ALPHABLENDENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_FOGENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_SPECULARENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_FOGCOLOR);
    val = sHex(Value);
    break;
    ECASE (D3DRS_FOGTABLEMODE);
    val = sD3DFOG(Value);
    break;
    ECASE (D3DRS_FOGSTART);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_FOGEND);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_FOGDENSITY);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_RANGEFOGENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_STENCILENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_STENCILFAIL);
    val = sD3DSTENCILOP(Value);
    break;
    ECASE (D3DRS_STENCILZFAIL);
    val = sD3DSTENCILOP(Value);
    break;
    ECASE (D3DRS_STENCILPASS);
    val = sD3DSTENCILOP(Value);
    break;
    ECASE (D3DRS_STENCILFUNC);
    val = sD3DCMP(Value);
    break;
    ECASE (D3DRS_STENCILREF);
    val = sInt(Value);
    break;
    ECASE (D3DRS_STENCILMASK);
    val = sHex(Value);
    break;
    ECASE (D3DRS_STENCILWRITEMASK);
    val = sHex(Value);
    break;
    ECASE (D3DRS_TEXTUREFACTOR);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP0);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP1);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP2);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP3);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP4);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP5);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP6);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP7);
    val = sHex(Value);
    break;
    ECASE (D3DRS_CLIPPING);
    val = sBool(Value);
    break;
    ECASE (D3DRS_LIGHTING);
    val = sBool(Value);
    break;
    ECASE (D3DRS_AMBIENT);
    val = sHex(Value);
    break;
    ECASE (D3DRS_FOGVERTEXMODE);
    val = sD3DFOG(Value);
    break;
    ECASE (D3DRS_COLORVERTEX);
    val = sBool(Value);
    break;
    ECASE (D3DRS_LOCALVIEWER);
    val = sBool(Value);
    break;
    ECASE (D3DRS_NORMALIZENORMALS);
    val = sBool(Value);
    break;
    ECASE (D3DRS_DIFFUSEMATERIALSOURCE);
    val = sD3DMCS(Value);
    break;
    ECASE (D3DRS_SPECULARMATERIALSOURCE);
    val = sD3DMCS(Value);
    break;
    ECASE (D3DRS_AMBIENTMATERIALSOURCE);
    val = sD3DMCS(Value);
    break;
    ECASE (D3DRS_EMISSIVEMATERIALSOURCE);
    val = sD3DMCS(Value);
    break;
    ECASE (D3DRS_VERTEXBLEND);
    val = sD3DVBF(Value);
    break;
    ECASE (D3DRS_CLIPPLANEENABLE);
    val = sHex(Value);
    break;
    ECASE (D3DRS_POINTSIZE);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_POINTSIZE_MIN);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_POINTSPRITEENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_POINTSCALEENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_POINTSCALE_A);
    break;
    ECASE (D3DRS_POINTSCALE_B);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_POINTSCALE_C);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_MULTISAMPLEANTIALIAS);
    val = sBool(Value);
    break;
    ECASE (D3DRS_MULTISAMPLEMASK);
    val = sHex(Value);
    break;
    ECASE (D3DRS_PATCHEDGESTYLE);
    val = sD3DPATCHEDGE(Value);
    break;
    ECASE (D3DRS_DEBUGMONITORTOKEN);
    val = sD3DDMT(Value);
    break;
    ECASE (D3DRS_POINTSIZE_MAX);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_INDEXEDVERTEXBLENDENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_COLORWRITEENABLE);
    val = sHex(Value);
    break;
    ECASE (D3DRS_TWEENFACTOR);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_BLENDOP);
    val = sD3DBLENDOP(Value);
    break;
    ECASE (D3DRS_POSITIONDEGREE);
    val = sD3DDEGREE(Value);
    break;
    ECASE (D3DRS_NORMALDEGREE);
    val = sD3DDEGREE(Value);
    break;
    ECASE (D3DRS_SCISSORTESTENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_SLOPESCALEDEPTHBIAS);
    val = sHex(Value);
    break;
    ECASE (D3DRS_ANTIALIASEDLINEENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_MINTESSELLATIONLEVEL);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_MAXTESSELLATIONLEVEL);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_ADAPTIVETESS_X);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_ADAPTIVETESS_Y);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_ADAPTIVETESS_Z);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_ADAPTIVETESS_W);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_ENABLEADAPTIVETESSELLATION);
    val = sBool(Value);
    break;
    ECASE (D3DRS_TWOSIDEDSTENCILMODE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_CCW_STENCILFAIL);
    val = sD3DSTENCILOP(Value);
    break;
    ECASE (D3DRS_CCW_STENCILZFAIL);
    val = sD3DSTENCILOP(Value);
    break;
    ECASE (D3DRS_CCW_STENCILPASS);
    val = sD3DSTENCILOP(Value);
    break;
    ECASE (D3DRS_CCW_STENCILFUNC);
    val = sD3DCMP(Value);
    break;
    ECASE (D3DRS_COLORWRITEENABLE1);
    val = sHex(Value);
    break;
    ECASE (D3DRS_COLORWRITEENABLE2);
    val = sHex(Value);
    break;
    ECASE (D3DRS_COLORWRITEENABLE3);
    val = sHex(Value);
    break;
    ECASE (D3DRS_BLENDFACTOR);
    val = sHex(Value);
    break;
    ECASE (D3DRS_SRGBWRITEENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_DEPTHBIAS);
    val = sDWFloat(Value);
    break;
    ECASE (D3DRS_WRAP8);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP9);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP10);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP11);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP12);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP13);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP14);
    val = sHex(Value);
    break;
    ECASE (D3DRS_WRAP15);
    val = sHex(Value);
    break;
    ECASE (D3DRS_SEPARATEALPHABLENDENABLE);
    val = sBool(Value);
    break;
    ECASE (D3DRS_SRCBLENDALPHA);
    val = sD3DBLEND(Value);
    break;
    ECASE (D3DRS_DESTBLENDALPHA);
    val = sD3DBLEND(Value);
    break;
    ECASE (D3DRS_BLENDOPALPHA);
    val = sD3DBLENDOP(Value);
    break;
    EDEFAULT (State);
  }
  if (!val)
    val = sHex(Value);
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%s, %s)\n", "D3DDevice::SetRenderState", state, val);
  return gcpRendD3D->m_pActuald3dDevice->SetRenderState(State,Value);
#undef ECASE
#undef EDEFAULT
}
HRESULT CMyDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetRenderState");
  return gcpRendD3D->m_pActuald3dDevice->GetRenderState(State,pValue);
}
HRESULT CMyDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::CreateStateBlock");
  return gcpRendD3D->m_pActuald3dDevice->CreateStateBlock(Type,ppSB);
}
HRESULT CMyDirect3DDevice9::BeginStateBlock()
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::BeginStateBlock");
  return gcpRendD3D->m_pActuald3dDevice->BeginStateBlock();
}
HRESULT CMyDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::EndStateBlock");
  return gcpRendD3D->m_pActuald3dDevice->EndStateBlock(ppSB);
}
HRESULT CMyDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetClipStatus");
  return gcpRendD3D->m_pActuald3dDevice->SetClipStatus(pClipStatus);
}
HRESULT CMyDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetClipStatus");
  return gcpRendD3D->m_pActuald3dDevice->GetClipStatus(pClipStatus);
}
HRESULT CMyDirect3DDevice9::GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetTexture");
  return gcpRendD3D->m_pActuald3dDevice->GetTexture(Stage, ppTexture);
}
HRESULT CMyDirect3DDevice9::SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%d, 0x%x)\n", "D3DDevice::SetTexture", Stage, pTexture);
  return gcpRendD3D->m_pActuald3dDevice->SetTexture(Stage,pTexture);
}
HRESULT CMyDirect3DDevice9::GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetTextureStageState");
  return gcpRendD3D->m_pActuald3dDevice->GetTextureStageState(Stage,Type,pValue);
}
HRESULT CMyDirect3DDevice9::SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
{
#define ECASE(e) case e: type = #e
  char Str[1024];
#define EDEFAULT(e) default: sprintf(Str, "0x%x", e); type = Str;
  char *type;
  char *val = NULL;
  switch (Type)
  {
    ECASE (D3DTSS_COLOROP);
    val = sD3DTOP(Value);
    break;
    ECASE (D3DTSS_COLORARG1);
    val = sD3DTA(Value);
    break;
    ECASE (D3DTSS_COLORARG2);
    val = sD3DTA(Value);
    break;
    ECASE (D3DTSS_ALPHAOP);
    val = sD3DTOP(Value);
    break;
    ECASE (D3DTSS_ALPHAARG1);
    val = sD3DTA(Value);
    break;
    ECASE (D3DTSS_ALPHAARG2);
    val = sD3DTA(Value);
    break;
    ECASE (D3DTSS_BUMPENVMAT00);
    val = sDWFloat(Value);
    break;
    ECASE (D3DTSS_BUMPENVMAT01);
    val = sDWFloat(Value);
    break;
    ECASE (D3DTSS_BUMPENVMAT10);
    val = sDWFloat(Value);
    break;
    ECASE (D3DTSS_BUMPENVMAT11);
    val = sDWFloat(Value);
    break;
    ECASE (D3DTSS_TEXCOORDINDEX);
    val = sTEXCOORDINDEX(Value);
    break;
    ECASE (D3DTSS_BUMPENVLSCALE);
    val = sDWFloat(Value);
    break;
    ECASE (D3DTSS_BUMPENVLOFFSET);
    val = sDWFloat(Value);
    break;
    ECASE (D3DTSS_TEXTURETRANSFORMFLAGS);
    val = sD3DTTFF(Value);
    break;
    ECASE (D3DTSS_COLORARG0);
    val = sD3DTA(Value);
    break;
    ECASE (D3DTSS_ALPHAARG0);
    val = sD3DTA(Value);
    break;
    ECASE (D3DTSS_RESULTARG);
    val = sD3DTA(Value);
    break;
    ECASE (D3DTSS_CONSTANT);
    val = sHex(Value);
    break;
    EDEFAULT (Type);
  }
  if (!val)
    val = sHex(Value);
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%d, %s, %s)\n", "D3DDevice::SetTextureStageState", Stage, type, val);
  return gcpRendD3D->m_pActuald3dDevice->SetTextureStageState(Stage,Type,Value);
#undef ECASE
#undef EDEFAULT
}
HRESULT CMyDirect3DDevice9::GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetSamplerState");
  return gcpRendD3D->m_pActuald3dDevice->GetSamplerState(Sampler,Type, pValue);
}
HRESULT CMyDirect3DDevice9::SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
{
#define ECASE(e) case e: type = #e
  char Str[1024];
#define EDEFAULT(e) default: sprintf(Str, "0x%x", e); type = Str;
  char *type;
  char *val = NULL;
  switch(Type)
  {
    ECASE (D3DSAMP_ADDRESSU);
    val = sD3DTADDRESS(Value);
    break;
    ECASE (D3DSAMP_ADDRESSV);
    val = sD3DTADDRESS(Value);
    break;
    ECASE (D3DSAMP_ADDRESSW);
    val = sD3DTADDRESS(Value);
    break;
    ECASE (D3DSAMP_BORDERCOLOR);
    val = sHex(Value);
    break;
    ECASE (D3DSAMP_MAGFILTER);
    val = sD3DTEXF(Value);
    break;
    ECASE (D3DSAMP_MINFILTER);
    val = sD3DTEXF(Value);
    break;
    ECASE (D3DSAMP_MIPFILTER);
    val = sD3DTEXF(Value);
    break;
    ECASE (D3DSAMP_MIPMAPLODBIAS);
    val = sDWFloat(Value);
    break;
    ECASE (D3DSAMP_MAXMIPLEVEL);
    val = sInt(Value);
    break;
    ECASE (D3DSAMP_MAXANISOTROPY);
    val = sInt(Value);
    break;
    ECASE (D3DSAMP_SRGBTEXTURE);
    val = sBool(Value);
    break;
    ECASE (D3DSAMP_ELEMENTINDEX);
    val = sInt(Value);
    break;
    ECASE (D3DSAMP_DMAPOFFSET);
    val = sInt(Value);
    break;
    EDEFAULT (Type);
  }
  if (!val)
    val = sHex(Value);
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%d, %s, %s)\n", "D3DDevice::SetSamplerState", Sampler, type, val);
  return gcpRendD3D->m_pActuald3dDevice->SetSamplerState(Sampler,Type,Value);
#undef ECASE
#undef EDEFAULT
}
HRESULT CMyDirect3DDevice9::ValidateDevice(DWORD* pNumPasses)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::ValidateDevice");
  return gcpRendD3D->m_pActuald3dDevice->ValidateDevice(pNumPasses);
}
HRESULT CMyDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetPaletteEntries");
  return gcpRendD3D->m_pActuald3dDevice->SetPaletteEntries(PaletteNumber,pEntries);
}
HRESULT CMyDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetPaletteEntries");
  return gcpRendD3D->m_pActuald3dDevice->GetPaletteEntries(PaletteNumber,pEntries);
}
HRESULT CMyDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetCurrentTexturePalette");
  return gcpRendD3D->m_pActuald3dDevice->SetCurrentTexturePalette(PaletteNumber);
}
HRESULT CMyDirect3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetCurrentTexturePalette");
  return gcpRendD3D->m_pActuald3dDevice->GetCurrentTexturePalette(PaletteNumber);
}
HRESULT CMyDirect3DDevice9::SetScissorRect(CONST RECT* pRect)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s(%d, %d, %d, %d)\n", "D3DDevice::SetScissorRect", pRect->left, pRect->right, pRect->top, pRect->bottom);
  return gcpRendD3D->m_pActuald3dDevice->SetScissorRect(pRect);
}
HRESULT CMyDirect3DDevice9::GetScissorRect(RECT* pRect)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetScissorRect");
  return gcpRendD3D->m_pActuald3dDevice->GetScissorRect(pRect);
}
HRESULT CMyDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetSoftwareVertexProcessing");
  return gcpRendD3D->m_pActuald3dDevice->SetSoftwareVertexProcessing(bSoftware);
}
BOOL CMyDirect3DDevice9::GetSoftwareVertexProcessing()
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetSoftwareVertexProcessing");
  return gcpRendD3D->m_pActuald3dDevice->GetSoftwareVertexProcessing();
}
    
HRESULT CMyDirect3DDevice9::SetNPatchMode(float nSegments)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetNPatchMode");
  return gcpRendD3D->m_pActuald3dDevice->SetNPatchMode(nSegments);
}
float CMyDirect3DDevice9::GetNPatchMode()
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetNPatchMode");
  return gcpRendD3D->m_pActuald3dDevice->GetNPatchMode();
}
HRESULT CMyDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s  (%s, %d, %d)\n", "D3DDevice::DrawPrimitive", sD3DPT(PrimitiveType), StartVertex, PrimitiveCount);
  return gcpRendD3D->m_pActuald3dDevice->DrawPrimitive(PrimitiveType,StartVertex,PrimitiveCount);
}
HRESULT CMyDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%s, %d, %d, %d, %d, %d)\n", "D3DDevice::DrawIndexedPrimitive", sD3DPT(PrimitiveType), BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
  return gcpRendD3D->m_pActuald3dDevice->DrawIndexedPrimitive(PrimitiveType,BaseVertexIndex,MinVertexIndex,NumVertices,startIndex,primCount);
}
HRESULT CMyDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
    gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%s, %d, 0x%x, %d)\n", "D3DDevice::DrawPrimitiveUP", sD3DPT(PrimitiveType), PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
  return gcpRendD3D->m_pActuald3dDevice->DrawPrimitiveUP(PrimitiveType,PrimitiveCount,pVertexStreamZeroData,VertexStreamZeroStride);
}
HRESULT CMyDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::DrawIndexedPrimitiveUP");
  return gcpRendD3D->m_pActuald3dDevice->DrawIndexedPrimitiveUP(PrimitiveType,MinVertexIndex,NumVertices,PrimitiveCount, pIndexData,IndexDataFormat,pVertexStreamZeroData,VertexStreamZeroStride);
}
HRESULT CMyDirect3DDevice9::ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::ProcessVertices");
  return gcpRendD3D->m_pActuald3dDevice->ProcessVertices(SrcStartIndex,DestIndex,VertexCount,pDestBuffer,pVertexDecl,Flags);
}
HRESULT CMyDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::CreateVertexDeclaration");
  return gcpRendD3D->m_pActuald3dDevice->CreateVertexDeclaration(pVertexElements,ppDecl);
}
HRESULT CMyDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (0x%x)\n", "D3DDevice::SetVertexDeclaration", pDecl);
  return gcpRendD3D->m_pActuald3dDevice->SetVertexDeclaration(pDecl);
}
HRESULT CMyDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetVertexDeclaration");
  return gcpRendD3D->m_pActuald3dDevice->GetVertexDeclaration( ppDecl);
}
HRESULT CMyDirect3DDevice9::SetFVF(DWORD FVF)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%s)\n", "D3DDevice::SetFVF", sFVF(FVF));
  return gcpRendD3D->m_pActuald3dDevice->SetFVF(FVF);
}
HRESULT CMyDirect3DDevice9::GetFVF(DWORD* pFVF)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetFVF");
  return gcpRendD3D->m_pActuald3dDevice->GetFVF( pFVF);
}
HRESULT CMyDirect3DDevice9::CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::CreateVertexShader");
  return gcpRendD3D->m_pActuald3dDevice->CreateVertexShader(pFunction, ppShader);
}
HRESULT CMyDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (0x%x)\n", "D3DDevice::SetVertexShader", pShader);
  return gcpRendD3D->m_pActuald3dDevice->SetVertexShader( pShader);
}
HRESULT CMyDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetVertexShader");
  return gcpRendD3D->m_pActuald3dDevice->GetVertexShader( ppShader);
}
HRESULT CMyDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%d", "D3DDevice::SetVertexShaderConstantF", StartRegister);
  const float *v = pConstantData;
  for (int i=0; i<(int)Vector4fCount; i++)
  {
    gcpRendD3D->Logv(0, ", [%.3f, %.3f, %.3f, %.3f]", v[0], v[1], v[2], v[3]);
    v += 4;
  }
  gcpRendD3D->Logv(0, ", %d)\n", Vector4fCount);
  return gcpRendD3D->m_pActuald3dDevice->SetVertexShaderConstantF(StartRegister,pConstantData,Vector4fCount);
}
HRESULT CMyDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetVertexShaderConstantF");
  return gcpRendD3D->m_pActuald3dDevice->GetVertexShaderConstantF(StartRegister,pConstantData,Vector4fCount);
}
HRESULT CMyDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetVertexShaderConstantI");
  return gcpRendD3D->m_pActuald3dDevice->SetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount);
}
HRESULT CMyDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetVertexShaderConstantI");
  return gcpRendD3D->m_pActuald3dDevice->GetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount);
}
HRESULT CMyDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetVertexShaderConstantB");
  return gcpRendD3D->m_pActuald3dDevice->SetVertexShaderConstantB(StartRegister,pConstantData,BoolCount);
}
HRESULT CMyDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetVertexShaderConstantB");
  return gcpRendD3D->m_pActuald3dDevice->GetVertexShaderConstantB(StartRegister,pConstantData,BoolCount);
}
HRESULT CMyDirect3DDevice9::SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%d, 0x%x, %d, %d)\n", "D3DDevice::SetStreamSource", StreamNumber, pStreamData, OffsetInBytes, Stride);
  return gcpRendD3D->m_pActuald3dDevice->SetStreamSource(StreamNumber,pStreamData,OffsetInBytes,Stride);
}
HRESULT CMyDirect3DDevice9::GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* OffsetInBytes,UINT* pStride)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetStreamSource");
  return gcpRendD3D->m_pActuald3dDevice->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride);
}
HRESULT CMyDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber,UINT Divider)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%d, 0x%x)\n", "D3DDevice::SetStreamSourceFreq", StreamNumber, Divider);
  return gcpRendD3D->m_pActuald3dDevice->SetStreamSourceFreq(StreamNumber,Divider);
}
HRESULT CMyDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber,UINT* Divider)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetStreamSourceFreq");
  return gcpRendD3D->m_pActuald3dDevice->GetStreamSourceFreq(StreamNumber,Divider);
}
HRESULT CMyDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (0x%x)\n", "D3DDevice::SetIndices", pIndexData);
  return gcpRendD3D->m_pActuald3dDevice->SetIndices( pIndexData);
}
HRESULT CMyDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetIndices");
  return gcpRendD3D->m_pActuald3dDevice->GetIndices( ppIndexData);
}
HRESULT CMyDirect3DDevice9::CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::CreatePixelShader");
  return gcpRendD3D->m_pActuald3dDevice->CreatePixelShader(pFunction,ppShader);
}
HRESULT CMyDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (0x%x)\n", "D3DDevice::SetPixelShader", pShader);
  return gcpRendD3D->m_pActuald3dDevice->SetPixelShader( pShader);
}
HRESULT CMyDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (0x%x)\n", "D3DDevice::GetPixelShader", ppShader);
  return gcpRendD3D->m_pActuald3dDevice->GetPixelShader( ppShader);
}
HRESULT CMyDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s (%d", "D3DDevice::SetPixelShaderConstantF", StartRegister);
  const float *v = pConstantData;
  for (int i=0; i<(int)Vector4fCount; i++)
  {
    gcpRendD3D->Logv(0, ", [%.3f, %.3f, %.3f, %.3f]", v[0], v[1], v[2], v[3]);
    v += 4;
  }
  gcpRendD3D->Logv(0, ", %d)\n", Vector4fCount);
  return gcpRendD3D->m_pActuald3dDevice->SetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount);
}
HRESULT CMyDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetPixelShaderConstantF");
  return gcpRendD3D->m_pActuald3dDevice->GetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount);
}
HRESULT CMyDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetPixelShaderConstantI");
  return gcpRendD3D->m_pActuald3dDevice->SetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount);
}
HRESULT CMyDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetPixelShaderConstantI");
  return gcpRendD3D->m_pActuald3dDevice->GetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount);
}
HRESULT CMyDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::SetPixelShaderConstantB");
  return gcpRendD3D->m_pActuald3dDevice->SetPixelShaderConstantB(StartRegister,pConstantData,BoolCount);
}
HRESULT CMyDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::GetPixelShaderConstantB");
  return gcpRendD3D->m_pActuald3dDevice->GetPixelShaderConstantB(StartRegister,pConstantData,BoolCount);
}
HRESULT CMyDirect3DDevice9::DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::DrawRectPatch");
  return gcpRendD3D->m_pActuald3dDevice->DrawRectPatch(Handle,pNumSegs,pRectPatchInfo);
}
HRESULT CMyDirect3DDevice9::DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::DrawTriPatch");
  return gcpRendD3D->m_pActuald3dDevice->DrawTriPatch(Handle,pNumSegs,pTriPatchInfo);
}
HRESULT CMyDirect3DDevice9::DeletePatch(UINT Handle)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::DeletePatch");
  return gcpRendD3D->m_pActuald3dDevice->DeletePatch(Handle);
}
HRESULT CMyDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
{
  gcpRendD3D->Logv(SRendItem::m_RecurseLevel, "%s\n", "D3DDevice::CreateQuery");
  return gcpRendD3D->m_pActuald3dDevice->CreateQuery(Type, ppQuery);
}

//==============================================================================================

void CD3D9Renderer::SetLogFuncs(bool set)
{
  static bool sSet = 0;

  if (set == sSet)
    return;

  sSet = set;

  if (set)
  {
    m_pActuald3dDevice = m_pd3dDevice;
    if (!m_pMyd3dDevice)
      m_pMyd3dDevice = new CMyDirect3DDevice9;
    m_pd3dDevice = m_pMyd3dDevice;
  }
  else
  {
    m_pd3dDevice = m_pActuald3dDevice;
  }
}
