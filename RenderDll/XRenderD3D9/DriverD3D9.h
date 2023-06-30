/*=============================================================================
  DriverD3D9.h : Direct3D8 Render interface declarations.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#ifndef DRIVERD3D9_H
#define DRIVERD3D9_H

#if _MSC_VER > 1000
# pragma once 
#endif

#ifndef PS2

/*
===========================================
The DXRenderer interface Class
===========================================
*/
#if defined(WIN64)
#include <d3dx9.h>
#include <dxerr9.h>
#elif defined(_XBOX)
#include <xtl.h>
#include <xgraphics.h>
#else
// Base class
#include <d3dx9.h>
#include <dxerr9.h>
#endif

//=======================================================================

#if defined(WIN32) && !defined(WIN64)
#include "cg\cgD3D9.h"
#endif

// DRIVERD3D.H
// CRender3D Direct3D rasterizer class.

#define VERSION_D3D 1.2

#define DECLARE_INITED(typ,var) typ var; memset(&var,0,sizeof(var)); var.dwSize=sizeof(var);
#define SAFETRY(cmd) {try{cmd;}catch(...){ShError("Exception in '%s'\n", #cmd);}}
#define DX_RELEASE(x) { if(x) { (x)->Release(); (x) = NULL; } }

struct SPixFormat;
class CRender3DD3D;
struct SDeviceInfo;

//=======================================================================

#include "DynamicVB.h"
#include "DynamicIB.h"
#include "StaticVB.h"
#include "StaticIB.h"
#include "D3DTexture.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"

class CMyDirect3DDevice9 : public IDirect3DDevice9
{
  /*** IUnknown methods ***/
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
  STDMETHOD_(ULONG,AddRef)(THIS);
  STDMETHOD_(ULONG,Release)(THIS);

  /*** IDirect3DDevice9 methods ***/
  STDMETHOD(TestCooperativeLevel)(THIS);
  STDMETHOD_(UINT, GetAvailableTextureMem)(THIS);
  STDMETHOD(EvictManagedResources)(THIS);
  STDMETHOD(GetDirect3D)(THIS_ IDirect3D9** ppD3D9);
  STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps);
  STDMETHOD(GetDisplayMode)(THIS_ UINT iSwapChain,D3DDISPLAYMODE* pMode);
  STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters);
  STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap);
  STDMETHOD_(void, SetCursorPosition)(THIS_ int X,int Y,DWORD Flags);
  STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow);
  STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain);
  STDMETHOD(GetSwapChain)(THIS_ UINT iSwapChain,IDirect3DSwapChain9** pSwapChain);
  STDMETHOD_(UINT, GetNumberOfSwapChains)(THIS);
  STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);
  STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion);
  STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer);
  STDMETHOD(GetRasterStatus)(THIS_ UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus);
  STDMETHOD(SetDialogBoxMode)(THIS_ BOOL bEnableDialogs);
  STDMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp);
  STDMETHOD_(void, GetGammaRamp)(THIS_ UINT iSwapChain,D3DGAMMARAMP* pRamp);
  STDMETHOD(CreateTexture)(THIS_ UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle);
  STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle);
  STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle);
  STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle);
  STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle);
  STDMETHOD(CreateRenderTarget)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
  STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
  STDMETHOD(UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint);
  STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture);
  STDMETHOD(GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface);
  STDMETHOD(GetFrontBufferData)(THIS_ UINT iSwapChain,IDirect3DSurface9* pDestSurface);
  STDMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter);
  STDMETHOD(ColorFill)(THIS_ IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color);
  STDMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
  STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget);
  STDMETHOD(GetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget);
  STDMETHOD(SetDepthStencilSurface)(THIS_ IDirect3DSurface9* pNewZStencil);
  STDMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface9** ppZStencilSurface);
  STDMETHOD(BeginScene)(THIS);
  STDMETHOD(EndScene)(THIS);
  STDMETHOD(Clear)(THIS_ DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil);
  STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix);
  STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix);
  STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE,CONST D3DMATRIX*);
  STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport);
  STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT9* pViewport);
  STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial);
  STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL9* pMaterial);
  STDMETHOD(SetLight)(THIS_ DWORD Index,CONST D3DLIGHT9*);
  STDMETHOD(GetLight)(THIS_ DWORD Index,D3DLIGHT9*);
  STDMETHOD(LightEnable)(THIS_ DWORD Index,BOOL Enable);
  STDMETHOD(GetLightEnable)(THIS_ DWORD Index,BOOL* pEnable);
  STDMETHOD(SetClipPlane)(THIS_ DWORD Index,CONST float* pPlane);
  STDMETHOD(GetClipPlane)(THIS_ DWORD Index,float* pPlane);
  STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD Value);
  STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD* pValue);
  STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB);
  STDMETHOD(BeginStateBlock)(THIS);
  STDMETHOD(EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB);
  STDMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS9* pClipStatus);
  STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus);
  STDMETHOD(GetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9** ppTexture);
  STDMETHOD(SetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9* pTexture);
  STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue);
  STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value);
  STDMETHOD(GetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue);
  STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value);
  STDMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses);
  STDMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber,CONST PALETTEENTRY* pEntries);
  STDMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber,PALETTEENTRY* pEntries);
  STDMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber);
  STDMETHOD(GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber);
  STDMETHOD(SetScissorRect)(THIS_ CONST RECT* pRect);
  STDMETHOD(GetScissorRect)(THIS_ RECT* pRect);
  STDMETHOD(SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware);
  STDMETHOD_(BOOL, GetSoftwareVertexProcessing)(THIS);
  STDMETHOD(SetNPatchMode)(THIS_ float nSegments);
  STDMETHOD_(float, GetNPatchMode)(THIS);
  STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount);
  STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount);
  STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
  STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
  STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags);
  STDMETHOD(CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl);
  STDMETHOD(SetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9* pDecl);
  STDMETHOD(GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl);
  STDMETHOD(SetFVF)(THIS_ DWORD FVF);
  STDMETHOD(GetFVF)(THIS_ DWORD* pFVF);
  STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader);
  STDMETHOD(SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader);
  STDMETHOD(GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader);
  STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount);
  STDMETHOD(GetVertexShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount);
  STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
  STDMETHOD(GetVertexShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount);
  STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
  STDMETHOD(GetVertexShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount);
  STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride);
  STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* OffsetInBytes,UINT* pStride);
  STDMETHOD(SetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT Divider);
  STDMETHOD(GetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT* Divider);
  STDMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer9* pIndexData);
  STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData);
  STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader);
  STDMETHOD(SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader);
  STDMETHOD(GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader);
  STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount);
  STDMETHOD(GetPixelShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount);
  STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
  STDMETHOD(GetPixelShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount);
  STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
  STDMETHOD(GetPixelShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount);
  STDMETHOD(DrawRectPatch)(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo);
  STDMETHOD(DrawTriPatch)(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo);
  STDMETHOD(DeletePatch)(THIS_ UINT Handle);
  STDMETHOD(CreateQuery)(THIS_ D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery);
};

inline D3DMULTISAMPLE_TYPE ConvertFSAASamplesToType( int numSamples )
{
	static D3DMULTISAMPLE_TYPE s_MapNumSamplesToFSAAType[] = {	
		D3DMULTISAMPLE_NONE            ,//=  0,
		D3DMULTISAMPLE_NONMASKABLE     ,//=  1,
		D3DMULTISAMPLE_2_SAMPLES       ,//=  2,
		D3DMULTISAMPLE_3_SAMPLES       ,//=  3,
		D3DMULTISAMPLE_4_SAMPLES       ,//=  4,
		D3DMULTISAMPLE_5_SAMPLES       ,//=  5,
		D3DMULTISAMPLE_6_SAMPLES       ,//=  6,
		D3DMULTISAMPLE_7_SAMPLES       ,//=  7,
		D3DMULTISAMPLE_8_SAMPLES       ,//=  8,
		D3DMULTISAMPLE_9_SAMPLES       ,//=  9,
		D3DMULTISAMPLE_10_SAMPLES      ,//= 10,
		D3DMULTISAMPLE_11_SAMPLES      ,//= 11,
		D3DMULTISAMPLE_12_SAMPLES      ,//= 12,
		D3DMULTISAMPLE_13_SAMPLES      ,//= 13,
		D3DMULTISAMPLE_14_SAMPLES      ,//= 14,
		D3DMULTISAMPLE_15_SAMPLES      ,//= 15,
		D3DMULTISAMPLE_16_SAMPLES      ,//= 16, 
	};

	if( numSamples < 0 || numSamples >= sizeof( s_MapNumSamplesToFSAAType ) / sizeof( s_MapNumSamplesToFSAAType[ 0 ] ) )
		numSamples = 0;

	return( s_MapNumSamplesToFSAAType[ numSamples ] );
}

#define D3DRGBA(r, g, b, a) \
    (   (((long)((a) * 255)) << 24) | (((long)((r) * 255)) << 16) \
    |   (((long)((g) * 255)) << 8) | (long)((b) * 255) \
    )

_inline DWORD FLOATtoDWORD( float f )
{
  union FLOATDWORD
  {
    float f;
    DWORD dw;
  };

  FLOATDWORD val;
  val.f = f;
  return val.dw;
}

//=======================================================================

typedef std::map<int,STexPic*> TTextureMap;
typedef TTextureMap::iterator TTextureMapItor;

struct STexPicD3D : public STexPic
{
  STexPicD3D() : STexPic()
  {
    iConsole->Register("r_LogTextureUsage", &CV_r_LogTextureUsage, 0);
  }

  virtual void SaveTGA(const char *name, bool bMips);
  virtual void SaveJPG(const char *name, bool bMips);
  
  virtual void Release(bool bForce);

  virtual void Set(int nTexSlot=-1);
  
  virtual void SetClamp(bool bEnable);
  virtual void BuildMips();
  virtual bool UploadMips(int nStartMip, int nEndMip);
  virtual void ReleaseDriverTexture();
  virtual byte *GetData32();
  virtual void Preload (int Flags);
  virtual int DstFormatFromTexFormat(ETEX_Format eTF);
  virtual int TexSize(int Width, int Height, int DstFormat);

  static FILE *m_TexUseLogFile;
  static int CV_r_LogTextureUsage;

};

//=====================================================

struct SD3DStats
{
};

struct SPixFormat
{
  // Pixel format info.
  D3DFORMAT       Format;   // Pixel format from Direct3D.
  SPixFormat*     Next;     // Next in linked list of all compatible pixel formats.
  const TCHAR*    Desc;     // Stat: Human readable name for stats.
  int         BitsPerPixel; // Total bits per pixel.
  DWORD       MaxWidth;
  DWORD       MaxHeight;

  // Multi-frame stats.
  int         Binned;     // Stat: How many textures of this format are available in bins.
  int         BinnedRAM;    // Stat: How much RAM is used by total textures of this format in the cache.

  // Per-frame stats.
  int         Active;     // Stat: How many textures of this format are active.
  int         ActiveRAM;    // Stat: How much RAM is used by active textures of this format per frame.
  int         Sets;     // Stat: Number of SetTexture was called this frame on textures of this format.
  int         Uploads;    // Stat: Number of texture Blts this frame.
  int         UploadCycles; // Stat: Cycles spent Blting.
  void Init()
  {
    Next = NULL;
    BitsPerPixel = Binned = BinnedRAM = 0;
  }
  void InitStats()
  {
    Sets = Uploads = UploadCycles = Active = ActiveRAM = 0;
  }
};

struct STexFiller;

#define BUFFERED_VERTS 256

struct SD3DContext
{
  HWND m_hWnd;
  int m_X;
  int m_Y;
  int m_Width;
  int m_Height;
};

#define D3DAPPERR_NODIRECT3D          0x82000001
#define D3DAPPERR_NOWINDOW            0x82000002
#define D3DAPPERR_NOCOMPATIBLEDEVICES 0x82000003
#define D3DAPPERR_NOWINDOWABLEDEVICES 0x82000004
#define D3DAPPERR_NOHARDWAREDEVICE    0x82000005
#define D3DAPPERR_HALNOTCOMPATIBLE    0x82000006
#define D3DAPPERR_NOWINDOWEDHAL       0x82000007
#define D3DAPPERR_NODESKTOPHAL        0x82000008
#define D3DAPPERR_NOHALTHISMODE       0x82000009
#define D3DAPPERR_NONZEROREFCOUNT     0x8200000a
#define D3DAPPERR_MEDIANOTFOUND       0x8200000b
#define D3DAPPERR_RESIZEFAILED        0x8200000c


struct alloc_info_struct { int ptr; int bytes_num; bool busy; const char *szSource; };
struct SVertPool
{
  int m_nBufSize;
  IDirect3DVertexBuffer9 *m_pVB;
  list2<alloc_info_struct> m_alloc_info;
};
typedef TDList<SVertPool> TVertPool;

struct SD3DRenderTarget
{
  LPDIRECT3DSURFACE9 m_pRT;
  LPDIRECT3DSURFACE9 m_pZB;
};

//======================================================================
/// Direct3D Render driver class

class CD3D9Renderer : public CRenderer
{
  friend class CD3D9TexMan;
  
public:
  CD3D9Renderer();
  ~CD3D9Renderer();

protected:

// Windows context
  char      m_WinTitle[80];
  HINSTANCE m_hInst;            
  HWND      m_hWnd;              // The main app window
  HWND      m_hWndDesktop;       // The desktop window

#ifdef USE_3DC
  HANDLE    m_hLibHandle3DC;
#endif

  LPDIRECT3D9       m_pD3D;              // The main D3D object
  CD3DEnumeration   m_D3DEnum;
  CD3DSettings      m_D3DSettings;
  bool              m_bDeviceObjectsInited;
  bool              m_bDeviceObjectsRestored;

  // From D3D.
  // Main objects used for creating and rendering the 3D scene
  D3DPRESENT_PARAMETERS m_d3dpp;         // Parameters for CreateDevice/Reset
  D3DSURFACE_DESC   m_d3dsdZBuffer;      // Surface desc of the Zbuffer
  LPDIRECT3DSURFACE9 m_pBackBuffer;
  LPDIRECT3DSURFACE9 m_pZBuffer;
  LPDIRECT3DSURFACE9 m_pTempZBuffer;
  LPDIRECT3DSURFACE9 m_pHDRZBuffer;
  LPDIRECT3DSURFACE9 m_pCurBackBuffer;
  LPDIRECT3DSURFACE9 m_pCurZBuffer;
  DWORD             m_dwCreateFlags;     // Indicate sw or hw vertex processing
  DWORD             m_dwWindowStyle;     // Saved window style for mode switches
  RECT              m_rcWindowBounds;    // Saved window bounds for mode switches
  RECT              m_rcWindowClient;    // Saved client area size for mode switches
  TCHAR             m_strDeviceStats[90];// String to hold D3D device stats
  D3DVIEWPORT9      m_Viewport;
  
  int               m_MinDepthBits;    // Minimum number of bits needed in depth buffer
  int               m_MinStencilBits;  // Minimum number of bits needed in stencil buffer

  bool             m_bActive;
  bool             m_bReady;
  bool             m_bGammaCalibrate;
  bool             m_bUseSWVP;

  bool             m_bAllowAlphaPalettes;

  bool             m_bUseWBuffer;

  int              mMinTextureWidth, mMinTextureHeight;
  int              m_SceneRecurseCount;

  D3DGAMMARAMP     m_SystemGammaRamp;

  bool m_bHALExists, m_bHALIsSampleCompatible, m_bHALIsWindowedCompatible, m_bHALIsDesktopCompatible;
  int m_CurPal;
  // Direct3D-specific render options.

  int m_MaxAnisotropyLevel;
  int m_AnisotropyLevel;

  SD3DStats         mStats;

  D3DMATERIAL9 m_Material;
  D3DLIGHT9 m_Lights[16];

  float m_CurStereoSeparation;

//==================================================================

public:
  D3DCAPS9          m_d3dCaps;           // Caps for the device
  int m_TextureBits;
  D3DSURFACE_DESC   m_d3dsdBackBuffer;      // Surface desc of the Zbuffer
  IDirect3DQuery9  *m_pQuery; 
  IDirect3DVertexBuffer9 *m_pVB2D;
  IDirect3DVertexBuffer9 *m_pVB3DAr[3][4];
  IDirect3DVertexBuffer9 *m_pVB3D[3];
  IDirect3DIndexBuffer9 *m_pIB;
  int m_nCur3DBuf[3];
  int m_nVertsDMesh3D[3];
  int m_nOffsDMesh3D[3];

  int m_nVertsDMesh2D;
  int m_nOffsDMesh2D;
  int m_nIndsDMesh;
  int m_nIOffsDMesh;
  LPD3DXMESH m_pSphere;
  LPDIRECT3DDEVICE9 m_pd3dDevice;        // The D3D rendering device
  CMyDirect3DDevice9 *m_pMyd3dDevice;        // The D3D rendering device
  LPDIRECT3DDEVICE9 m_pActuald3dDevice;        // The D3D rendering device
  int m_nFrameReset;

  bool m_bHackEMBM;
  byte m_bDeviceSupportsInstancing;
  bool m_bDeviceSupportsMRT;
  bool m_bDeviceSupportsFP16Filter;

  IDirect3DSurface9* m_pHDRTargetSurf;
  IDirect3DSurface9* m_pHDRTargetSurf_K;
  D3DFORMAT m_HDR_FloatFormat_Scalar;

#define MAX_DYNVB3D_VERTS 4096

  struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F *GetVBPtr2D(int nVerts, int &nOffs)
  {
    if (!m_pVB2D)
      return NULL;
    HRESULT hr;
    struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F *pVertices = NULL;
    if (nVerts > m_nVertsDMesh2D)
    {
      assert(false);
      return NULL;
    }
    if (nVerts+m_nOffsDMesh2D > m_nVertsDMesh2D)
    {
      hr = m_pVB2D->Lock(0, nVerts*sizeof(struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F), (void **) &pVertices, D3DLOCK_DISCARD);
      nOffs = 0;
      m_nOffsDMesh2D = nVerts;
    }
    else
    {
      hr = m_pVB2D->Lock(m_nOffsDMesh2D*sizeof(struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F), nVerts*sizeof(struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F), (void **) &pVertices, D3DLOCK_NOOVERWRITE);
      nOffs = m_nOffsDMesh2D;
      m_nOffsDMesh2D += nVerts;
    }
    return pVertices;
  }
  void UnlockVB2D()
  {
    HRESULT hr = m_pVB2D->Unlock();
  }

  ushort *GetIBPtr(int nInds, int &nOffs)
  {
    if (!m_pIB)
      return NULL;
    HRESULT hr;
    ushort *pInds = NULL;
    if (nInds > m_nIndsDMesh)
    {
      assert(0);
      return NULL;
    }
    if (nInds+m_nIOffsDMesh > m_nIndsDMesh)
    {
      hr = m_pIB->Lock(0, nInds*sizeof(short), (void **) &pInds, D3DLOCK_DISCARD);
      nOffs = 0;
      m_nIOffsDMesh = nInds;
    }
    else
    {
      hr = m_pIB->Lock(m_nIOffsDMesh*sizeof(short), nInds*sizeof(short), (void **) &pInds, D3DLOCK_NOOVERWRITE);
      nOffs = m_nIOffsDMesh;
      m_nIOffsDMesh += nInds;
    }
    return pInds;
  }
  void UnlockIB()
  {
    HRESULT hr = m_pIB->Unlock();
  }

  void *GetVBPtr3D(int nVerts, int &nOffs, int Pool=0)
  {
    HRESULT hr;
    void *pVertices = NULL;
    if (nVerts > m_nVertsDMesh3D[Pool])
    {
      assert(0);
      return NULL;
    }
    if (!m_pVB3DAr[Pool][0])
      return NULL;

    int nVertSize;
    switch (Pool)
    {
      case 0:
      default:
        nVertSize = sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F);
    	  break;
      case 1:
        nVertSize = sizeof(struct_VERTEX_FORMAT_P3F_TEX2F);
    	  break;
      case 2:
        nVertSize = sizeof(SPipTangents);
        break;
    }
    if (nVerts+m_nOffsDMesh3D[Pool] > m_nVertsDMesh3D[Pool])
    {
      m_nCur3DBuf[Pool] = (m_nCur3DBuf[Pool]+1)&3;
      m_pVB3D[Pool] = m_pVB3DAr[Pool][m_nCur3DBuf[Pool]];
      hr = m_pVB3D[Pool]->Lock(0, nVerts*nVertSize, &pVertices, D3DLOCK_DISCARD);
      nOffs = 0;
      m_nOffsDMesh3D[Pool] = nVerts;
    }
    else
    {
      hr = m_pVB3D[Pool]->Lock(m_nOffsDMesh3D[Pool]*nVertSize, nVerts*nVertSize, &pVertices, D3DLOCK_NOOVERWRITE);
      nOffs = m_nOffsDMesh3D[Pool];
      m_nOffsDMesh3D[Pool] += nVerts;
    }
    return pVertices;
  }
  void UnlockVB3D(int Pool=0)
  {
    HRESULT hr = m_pVB3D[Pool]->Unlock();
  }

//============================================================================================

  TVertPool *sVertPools;

  void AllocVBInPool(int nSize, int nVFormat, SVertexStream *pVB);
  bool AllocateVBChunk(int size, TVertPool *Ptr, SVertexStream *pVB, const char *szSource);
  bool ReleaseVBChunk(TVertPool *Ptr, SVertexStream *pVB);

#if !defined(_XBOX) && !defined(WIN64)
  CGcontext m_CGContext;
#endif

  // Pixel formats from D3D.
  SPixFormat        mFormatA8;      //8 bit alpha
  SPixFormat        mFormatA8L8;    //16
  SPixFormat        mFormat8888;    //32 bit
  SPixFormat        mFormat1555;    //16 bit
  SPixFormat        mFormat4444;    //16 bit
  SPixFormat        mFormat0555;    //16 bit
  SPixFormat        mFormat0565;    //16 bit
  SPixFormat        mFormatV8U8;    //16 bit
  SPixFormat        mFormatCxV8U8;    //16 bit
  SPixFormat        mFormatQ8W8U8V8;  //32 bit
  SPixFormat        mFormatX8L8U8V8;  //32 bit
  SPixFormat        mFormatU5V5L6;  //16 bit
  SPixFormat        mFormat3Dc;     //
  SPixFormat        mFormatDXT1;    //Compressed RGB
  SPixFormat        mFormatDXT3;    //Compressed RGBA
  SPixFormat        mFormatDXT5;    //Compressed RGBA
  SPixFormat        mFormatPal8;    //Paletted 8 bit
  SPixFormat        mFormatDepth24; //Depth texture
  SPixFormat        mFormatDepth16; //Depth texture
  SPixFormat        mFormatU16V16;  //16 bit per component

  SPixFormat*       mFirstPixelFormat;

  int mZBias;

  byte m_GammmaTable[256];

  bool m_bEnableLights;

public:

  LPDIRECT3DDEVICE9  mfGetD3DDevice() { return m_pd3dDevice; }
  LPDIRECT3D9        mfGetD3D() { return m_pD3D; }
  D3DSURFACE_DESC*   mfGetZSurfaceDesc() { return &m_d3dsdZBuffer; }
  LPDIRECT3DSURFACE9 mfGetZSurface() { return m_pZBuffer; }
  LPDIRECT3DSURFACE9 mfGetBackSurface() { return m_pBackBuffer; }
  D3DCAPS9           *mfGetD3DCaps()  { return &m_d3dCaps; }

  int GetAnisotropicLevel()
  {
    if (GetFeatures() & RFT_ALLOWANISOTROPIC)
      return CV_r_texture_anisotropic_level;
    return 0;
  }

  void SetDefaultTexParams(bool bUseMips, bool bRepeat, bool bLoad);

public:

#define MAX_DYNAMIC_SHADOW_MAPS_COUNT 64
  struct ShadowMapTexInfo
  {
    ShadowMapTexInfo() { nTexId0=nTexId1=0; pOwner=0; pOwnerGroup=0; nLastFrameID=-1; nTexSize=0; dwFlags=0; }
    unsigned int nTexId0;
    unsigned int nTexId1;
    IEntityRender * pOwner;
		IStatObj * pOwnerGroup;
		int dwFlags;
		int nLastFrameID;
    int nTexSize;
  };
  ShadowMapTexInfo m_ShadowTexIDBuffer[MAX_DYNAMIC_SHADOW_MAPS_COUNT];
  TArray<ShadowMapTexInfo> m_TempShadowTextures;

  void BlurImage(int nSizeX, int nSizeY, int nType, ShadowMapTexInfo *st, int nTexDst);
  unsigned int GenShadowTexture(int nSize, bool bProjected);
  void PrepareDepthMap(ShadowMapFrustum * lof, bool make_new_tid);
  void ConfigShadowTexgen(int Num, int rangeMap, ShadowMapFrustum * pFrustum, float * pLightFrustumMatrix, float * pLightViewMatrix, float *ModelVPMatrix);
  virtual void SetupShadowOnlyPass(int Num, ShadowMapFrustum * pFrustum, Vec3 * vShadowTrans, const float fShadowScale, Vec3 vObjTrans=Vec3(0,0,0), float fObjScale=1.f, const Vec3 vObjAngles=Vec3(0,0,0), Matrix44 * pObjMat=0);
  int MakeShadowIdentityTexture();
	void DrawAllShadowsOnTheScreen();
	void OnEntityDeleted(IEntityRender * pEntityRender);

  //=============================================================

  byte m_eCurColorOp[MAX_TMU];
  byte m_eCurAlphaOp[MAX_TMU];
  byte m_eCurColorArg[MAX_TMU];
  byte m_eCurAlphaArg[MAX_TMU];
  int msCurState;

  void D3DSetCull(ECull eCull);
  const char *D3DError( HRESULT h );
  bool Error(char *Msg, HRESULT h);

  void SetDeviceGamma(ushort *r, ushort *g, ushort *b);

private:

  void RegisterVariables();
  void UnRegisterVariables();

  static bool ConfirmDeviceHelper(D3DCAPS9* pCaps, VertexProcessingType vertexProcessingType, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat);
  bool FindBestWindowedMode(bool bRequireHAL, bool bRequireREF);
  bool FindBestFullscreenMode(bool bRequireHAL, bool bRequireREF);
  HRESULT ChooseInitialD3DSettings();
  bool SetWindow(int width, int height, bool fullscreen, WIN_HWND hWnd);
  bool SetRes();
  bool ChooseDevice();
	void DisplaySplash(); //!< Load a bitmap from a file, blit it to the windowdc and free it
  void UnSetRes();
  HRESULT DeleteDeviceObjects();
  HRESULT InvalidateDeviceObjects();
  HRESULT FinalCleanup();

  HRESULT ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat );
  HRESULT AdjustWindowForChange();
  HRESULT InitDeviceObjects();
  void RecognizePixelFormat(SPixFormat& Dest, D3DFORMAT FromD3D, INT InBitsPerPixel, const TCHAR* InDesc);
  HRESULT RestoreDeviceObjects();
  void SetRendParms(D3DDISPLAYMODE *pModeInfo, D3DDeviceInfo *pDeviceInfo);
  void BuildPresentParamsFromSettings();
  void Cleanup3DEnvironment();
  HRESULT Reset3DEnvironment();
  HRESULT Initialize3DEnvironment();
  int FindSuitableDevice(int a, bool bAllowSoft);
  D3DFORMAT FindBestDepthFormat(D3DDeviceCombo* pBestDeviceCombo);
  void DestroyWindow(void);
  void RestoreGamma(void);
  void SetGamma(float fGamma, float fBrigtness, float fContrast, bool bForce);
  virtual char*	GetVertexProfile(bool bSupportedProfile);
  virtual char*	GetPixelProfile(bool bSupportedProfile);

  struct texture_info
  {
    texture_info() { ZeroStruct(*this); }
    char filename[256];
    int  bind_id;
    int  low_tid;
    int  type;
    int  last_time_used;
  };
  list2<texture_info> m_texture_registry;
  int FindTextureInRegistry(const char * filename, int * tex_type);
  int RegisterTextureInRegistry(const char * filename, int tex_type, int tid, int low_tid);
  unsigned int MakeTextureREAL(const char * filename,int *tex_type, unsigned int load_low_res);
  unsigned int CheckTexturePlus(const char * filename, const char * postfix);


public:  
  LPD3DXMATRIXSTACK m_matView;  
  LPD3DXMATRIXSTACK m_matProj;  
  D3DXMATRIX m_matViewInv;
  D3DXMATRIX m_TexMatrix[4];
  int m_MatDepth;
  

  static int CV_d3d9_texture_filter_anisotropic;
  static int CV_d3d9_nodeviceid;
  static int CV_d3d9_nvperfhud;
  static int CV_d3d9_palettedtextures;
  static int CV_d3d9_vbpools;
  static int CV_d3d9_vbpoolsize;
  static int CV_d3d9_psforce11;
  static int CV_d3d9_vsforce11;
  static int CV_d3d9_nv30_ps20;
  static int CV_d3d9_occlusion_query;
  static int CV_d3d9_compressedtextures;
  static int CV_d3d9_usebumpmap;
  static int CV_d3d9_bumptype;
  static int CV_d3d9_forcesoftware;
  static int CV_d3d9_texturebits;
  static int CV_d3d9_texmipfilter;
  static ICVar *CV_d3d9_texturefilter;
  static int CV_d3d9_squaretextures;
  static int CV_d3d9_mipprocedures;
  static ICVar *CV_d3d9_device;
  static int CV_d3d9_allowsoftware;
  static float CV_d3d9_pip_buff_size;
  static int CV_d3d9_rb_verts;
  static int CV_d3d9_rb_tris;
  static int CV_d3d9_decaloffset;
  static int CV_d3d9_nodepthmaps;
  static float CV_d3d9_normalmapscale;
  static int CV_d3d9_clipplanes;
  static int CV_d3d9_triplebuffering;
  static int CV_d3d9_resetdeviceafterloading;
  static int CV_d3d9_savedepthmaps;
  

//============================================================
// Renderer interface

  bool m_bInitialized;
	string m_Description;
  bool m_bFullScreen;

  TArray<SD3DContext *> m_RContexts;
  SD3DContext *m_CurrContext;
  TArray<SD3DRenderTarget> m_RTargets;
  int m_nRecurs;

public:
#ifndef PS2	
  virtual WIN_HWND Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen,WIN_HINSTANCE hinst, WIN_HWND Glhwnd=0, WIN_HDC Glhdc=0, WIN_HGLRC hGLrc=0, bool bReInit=false);
#else //PS2
  virtual bool Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen, bool bReInit=false);
#endif  //endif	
  virtual bool SetCurrentContext(WIN_HWND hWnd);
  virtual bool CreateContext(WIN_HWND hWnd, bool bAllowFSAA=false);
  virtual bool DeleteContext(WIN_HWND hWnd);

  virtual int  CreateRenderTarget (int nWidth, int nHeight, ETEX_Format eTF=eTF_8888);
  virtual bool DestroyRenderTarget (int nHandle);
  virtual bool SetRenderTarget (int nHandle);

  virtual void  ShareResources( IRenderer *renderer );
  virtual void	MakeCurrent();

	virtual bool ChangeDisplay(unsigned int width,unsigned int height,unsigned int cbpp);
  virtual void ChangeViewport(unsigned int x,unsigned int y,unsigned int width,unsigned int height);
	virtual int	 EnumDisplayFormats(TArray<SDispFormat>& Formats, bool bReset);

  //! Return all supported by video card video AA formats
  virtual int	EnumAAFormats(TArray<SAAFormat>& Formats, bool bReset);

  //! Changes resolution of the window/device (doen't require to reload the level
  virtual bool	ChangeResolution(int nNewWidth, int nNewHeight, int nNewColDepth, int nNewRefreshHZ, bool bFullScreen);
  virtual void	Reset(void);
  virtual void  WaitForDevice();

  virtual void  RefreshResources(int nFlags);
  virtual void BeginFrame();
  virtual void ShutDown(bool bReInit=false);
  virtual void Update(void);  
  virtual void GetMemoryUsage(ICrySizer* Sizer);
  virtual void Draw2dImage(float xpos,float ypos,float w,float h,int textureid,float s0=0,float t0=0,float s1=1,float t1=1,float angle=0,float r=1,float g=1,float b=1,float a=1,float z=1);
	//! Draw a image using the current matrix
	virtual void DrawImage(float xpos,float ypos,float w,float h,int texture_id,float s0,float t0,float s1,float t1,float r,float g,float b,float a);
	virtual void SetCullMode(int mode=R_CULL_BACK);
    
  virtual bool EnableFog(bool enable);
  virtual void SetFog(float density, float fogstart, float fogend, const float *color, int fogmode);

  //virtual CImage *TryLoadImage(const char *szFilename) { return (NULL); }

  virtual void SetLodBias(float value);
  virtual void SelectTMU(int tnum);
  virtual void EnableTMU(bool enable);
  virtual void SetTexture(int tnum, ETexType Type=eTT_Base);
  virtual unsigned int DownLoadToVideoMemory(unsigned char *data,int w, int h, ETEX_Format eTFSrc, ETEX_Format eTFDst, int nummipmap, bool repeat=true, int filter=FILTER_BILINEAR, int Id=0, char *szCacheName=NULL, int flags=0);
  virtual	void UpdateTextureInVideoMemory(uint tnum, unsigned char *newdata,int posx,int posy,int w,int h,ETEX_Format eTF=eTF_0888);
  virtual void RemoveTexture(unsigned int TextureId);
  virtual void RemoveTexture(ITexPic * pTexPic);
  virtual unsigned int MakeTexture(const char * filename,int *tex_type=NULL/*,unsigned int def_tid=0*/);
  virtual unsigned int LoadTexture(const char * filename,int *tex_type=NULL,unsigned int def_tid=0,bool compresstodisk=true,bool bWarn=true);

  virtual void SetCamera(const CCamera &cam);
  virtual	void SetViewport(int x=0, int y=0, int width=0, int height=0);
  virtual	void SetScissor(int x=0, int y=0, int width=0, int height=0);
  virtual void Draw3dBBox(const Vec3 &mins,const Vec3 &maxs, int nPrimType);
	virtual void Draw3dPrim(const Vec3 &mins,const Vec3 &maxs, int nPrimType, const float* pColor = NULL);
  virtual void Flush3dBBox(const Vec3 &mins,const Vec3 &maxs,const bool bSolid);
  virtual void EnableTexGen(bool enable);
  virtual void SetTexgen(float scaleX, float scaleY,float translateX,float translateY);
  virtual void SetTexgen3D(float x1, float y1, float z1, float x2, float y2, float z2);

  virtual void *GetDynVBPtr(int nVerts, int &nOffs, int Pool);
  virtual void DrawDynVB(int nOffs, int Pool, int nVerts);
  virtual void DrawDynVB(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pBuf, ushort *pInds, int nVerts, int nInds, int nPrimType);
	virtual CVertexBuffer	*CreateBuffer(int  buffersize,int vertexformat, const char *szSource, bool bDynamic=false);
  virtual void CreateBuffer(int size, int vertexformat, CVertexBuffer *buf, int Type, const char *szSource);
	virtual void	DrawBuffer(CVertexBuffer *src,SVertexStream *indicies,int numindices, int offsindex, int prmode,int vert_start=0,int vert_stop=0, CMatInfo *mi=NULL);
  void UnlockBuffer(CVertexBuffer *dest, int Type);
  virtual void UpdateBuffer(CVertexBuffer *dest,const void *src,int vertexcount, bool bUnLock, int offs=0, int Type=0);
  virtual void  CreateIndexBuffer(SVertexStream *dest,const void *src,int indexcount);
  virtual void  UpdateIndexBuffer(SVertexStream *dest,const void *src,int indexcount, bool bUnLock=true);
  virtual void  ReleaseIndexBuffer(SVertexStream *dest);
  virtual void ReleaseBuffer(CVertexBuffer *bufptr);
  virtual void CheckError(const char *comment);
  virtual int SetPolygonMode(int mode);

  virtual void PushMatrix();
  virtual void PopMatrix();
  virtual void RotateMatrix(float a,float x,float y,float z);
  virtual void RotateMatrix(const Vec3 & angels);
  virtual void ScaleMatrix(float x,float y,float z);
  virtual void TranslateMatrix(float x,float y,float z);
  virtual void TranslateMatrix(const Vec3 &pos);

  virtual void EnableVSync(bool enable);
  virtual void DrawTriStrip(CVertexBuffer *src, int vert_num);
  virtual void ResetToDefault();
  virtual void LoadMatrix(const Matrix44 *src);
  virtual void MultMatrix(float * mat);
  virtual int GenerateAlphaGlowTexture(float k);

  virtual void SetMaterialColor(float r, float g, float b, float a);
  virtual int LoadAnimatedTexture(const char * format,const int nCount);
  virtual char * GetStatusText(ERendStats type);
  //virtual void Project3DSprite(const Vec3 &origin,CImage *image);

  virtual void ProjectToScreen(float ptx, float pty, float ptz,float *sx, float *sy, float *sz );
  virtual void Draw2dLine(float x1, float y1, float x2, float y2);
  virtual void DrawPoints(Vec3 v[], int nump, CFColor& col, int flags);
  virtual void DrawLines(Vec3 v[], int nump, CFColor& col, int flags, float fGround);
	virtual void DrawLine(const Vec3 & vPos1, const Vec3 & vPos2);
  virtual void DrawLineColor(const Vec3 & vPos1, const CFColor & vColor1, const Vec3 & vPos2, const CFColor & vColor2);
  virtual void DrawBall(float x, float y, float z, float radius);
  virtual void DrawBall(const Vec3 & pos, float radius );
  virtual void DrawPoint(float x, float y, float z, float fSize);
  virtual int UnProject(float sx, float sy, float sz, float *px, float *py, float *pz, const float modelMatrix[16], const float projMatrix[16], const int    viewport[4]);
  virtual int UnProjectFromScreen( float  sx, float  sy, float  sz, float *px, float *py, float *pz);

  virtual void SetClipPlane( int id, float * params ){ EF_SetClipPlane(params ? true : false, params, false); }

  virtual void GetModelViewMatrix(float * mat);
  virtual void GetProjectionMatrix(float * mat);
  virtual void DrawObjSprites (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan);
  void DrawObjSprites_NoBend (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan);
  void DrawObjSprites_NoBend_Merge (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan);
  void ObjSpritesFlush (TArray<struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F>& Verts, IDirect3DVertexBuffer9 *&pCurVB);
  virtual void DrawQuad(const Vec3 &right, const Vec3 &up, const Vec3 &origin,int nFlipMode=0);
  virtual void DrawQuad(float dy,float dx, float dz, float x, float y, float z);
  void    DrawQuad(float x0, float y0, float x1, float y1, const CFColor & color, float z = 1.0f);
  void DrawQuad3D(const Vec3 & v0, const Vec3 & v1, const Vec3 & v2, const Vec3 & v3, const CFColor & color, 
    float ftx0 = 0,  float fty0 = 0,  float ftx1 = 1,  float fty1 = 1);

 //fog	
  void SetFogColor(float * color);
  virtual void TransformTextureMatrix(float x, float y, float angle, float scale);
  virtual void ResetTextureMatrix();

  virtual void SetPerspective(const CCamera &cam);

  virtual void ClearDepthBuffer();
  virtual void ClearColorBuffer(const Vec3 vColor);  
  virtual void ReadFrameBuffer(unsigned char * pRGB, int nSizeX, int nSizeY, bool bBackBuffer, bool bRGBA, int nScaledX=-1, int nScaledY=-1);  

  //misc	
  virtual void ScreenShot(const char *filename=NULL);  

  virtual uint MakeSprite(float object_scale, int tex_size, float angle, IStatObj * pStatObj, uchar * _pTmpBuffer, uint def_tid);
  virtual uint Make3DSprite(int nTexSize, float fAngleStep, IStatObj * pStatObj);

  virtual void UnloadOldTextures(){};

  virtual void Set2DMode(bool enable, int ortox, int ortoy);

  virtual int ScreenToTexture();

  virtual void SetFenceCompleted(CVertexBuffer * buffer) { };
  virtual void SetTexClampMode(bool clamp);

  virtual void EnableAALines(bool bEnable);

	virtual	bool	SetGammaDelta(const float fGamma);

  //////////////////////////////////////////////////////////////////////
  // Replacement functions for the Font engine
  virtual	bool FontUploadTexture(class CFBitmap*, ETEX_Format eTF=eTF_8888);
	virtual	int  FontCreateTexture(int Width, int Height, byte *pData, ETEX_Format eTF=eTF_8888);
  virtual	bool FontUpdateTexture(int nTexId, int X, int Y, int USize, int VSize, byte *pData);
  virtual	void FontReleaseTexture(class CFBitmap *pBmp);
  virtual void FontSetTexture(class CFBitmap*, int nFilterMode);
  virtual void FontSetTexture(int nTexId, int nFilterMode);
  virtual void FontSetRenderingState(unsigned long nVirtualScreenWidth, unsigned long nVirtualScreenHeight);
  virtual void FontSetBlending(int src, int dst);
  virtual void FontRestoreRenderingState();
  
  void FontSetState(bool bRestore);
  //////////////////////////////////////////////////////////////////////

  // Shaders pipeline
  virtual void EF_Release(int nFlags);
  virtual void EF_PipelineShutdown();
  void EF_ClearBuffers(bool bForce, bool bOnlyDepth, float *Colors);

  void EF_Invalidate();
  void EF_Restore();

  virtual void EF_LightMaterial(SLightMaterial *lm, int Flags);

  void ChangeLog();
  void SetLogFuncs(bool set);
  void FlushHardware();
  virtual bool CheckDeviceLost();

  byte *m_SysArray;
  ECull m_eCull;
  D3DXMATRIX *m_CurOpMatrix;
  bool m_bInvertedMatrix;
  D3DXMATRIX m_InvertedMatrix;
  bool m_bMatColor;

  _inline void EF_PushMatrix()
  {
    m_matView->Push();
    m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop()); 
    m_MatDepth++;
  }
  _inline void EF_PopMatrix()
  {
    m_matView->Pop();
    m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop()); 
    m_MatDepth--;
    m_bInvertedMatrix = false;
  }
  _inline D3DXMATRIX *EF_InverseMatrix()
  {
    if (m_bInvertedMatrix)
      return &m_InvertedMatrix;
    
    D3DXMATRIX *mi;
    if (m_MatDepth)
    {
      D3DXMATRIX *m = m_matView->GetTop();
      D3DXMatrixInverse(&m_InvertedMatrix, NULL, m);
      mi = &m_InvertedMatrix;
    }
    else
      mi = &m_matViewInv;
    m_bInvertedMatrix = true;

    return mi;
  }
  _inline void EF_SetGlobalColor(UCol& color)
  {
    EF_SelectTMU(0);
    EF_SetColorOp(255, 255, eCA_Texture | (eCA_Constant<<3), eCA_Texture | (eCA_Constant<<3));
    Exchange(color.bcolor[0], color.bcolor[2]);
    if (m_RP.m_CurGlobalColor.dcolor != color.dcolor)
    {
      m_RP.m_CurGlobalColor = color;
      m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, m_RP.m_CurGlobalColor.dcolor );
    }
  }

  _inline void EF_SetGlobalColor(float r, float g, float b, float a)
  {
    EF_SelectTMU(0);
    EF_SetColorOp(255, 255, eCA_Texture | (eCA_Constant<<3), eCA_Texture | (eCA_Constant<<3));
    UCol color;
    color.bcolor[0] = (byte)(b * 255.0f);
    color.bcolor[1] = (byte)(g * 255.0f);
    color.bcolor[2] = (byte)(r * 255.0f);
    color.bcolor[3] = (byte)(a * 255.0f);
    if (m_RP.m_CurGlobalColor.dcolor != color.dcolor)
    {
      m_RP.m_CurGlobalColor = color;
      m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, m_RP.m_CurGlobalColor.dcolor );
    }
  }
  _inline void EF_SetVertColor()
  {
    EF_SelectTMU(0);
    EF_SetColorOp(255, 255, eCA_Texture | (eCA_Diffuse<<3), eCA_Texture | (eCA_Diffuse<<3));
  }
  _inline void EF_SetArrayPointers(TArray<SArrayPointer *>& Pointers, int Id)
  {
    for (int i=0; i<Pointers.Num(); i++)
    {
      SArrayPointer *ap = Pointers[i];
      ap->mfSet(Id);
    }
  }
  _inline void EF_SelectTMU(int Stage)
  {
    CTexMan::m_CurStage = Stage;
  }
  _inline void EF_CommitTexTransforms(bool bEnable)
  {
    int i;
    int fl = m_RP.m_FlagsModificators;
    D3DXMATRIX mat, *mi;
    if (fl & RBMF_TCG)
    {
      for (i=0; i<4; i++)
      {
        if (fl & (RBMF_TCGOL0<<i))
        {
          if (bEnable)
          {
            SEfResTexture *pRT = gRenDev->m_RP.m_ShaderTexResources[i];
            D3DXMATRIX ma;
            D3DXMatrixTranspose(&ma, (D3DXMATRIX *)pRT->m_TexModificator.m_TexGenMatrix.GetData());
            mi = EF_InverseMatrix();
            D3DXMatrixMultiply(&mat, mi, &ma);
            m_pd3dDevice->SetTransform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+i), &mat);
            m_pd3dDevice->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT4);
            m_pd3dDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | i);
          }
          else
          {
            D3DXMatrixIdentity(&mat);
            m_pd3dDevice->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+i), &mat );
            m_pd3dDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | i);
          }
        }
        else
        if (fl & (RBMF_TCGRM0<<i))
        {
          if (bEnable)
            m_pd3dDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR | i);
          else
            m_pd3dDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | i);
        }
        else
        if (fl & (RBMF_TCGNM0<<i))
        {
          if (bEnable)
            m_pd3dDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL | i);
          else
            m_pd3dDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | i);
        }
        else
        if (fl & (RBMF_TCGSM0<<i))
        {
          iLog->Log("Warning: CD3D9Renderer::EF_CommitTexTransforms: Sphere Map texture gen mode isn't supported in D3D9 renderer yet (for fixed pipeline)");
        }
      }
    }
    if (fl & RBMF_TCM)
    {
      for (i=0; i<4; i++)
      {
        if (fl & (RBMF_TCM0<<i))
        {
          if (bEnable)
          {
            SEfResTexture *pRT = m_RP.m_ShaderTexResources[i];
            assert(m_RP.m_TexStages[i].Texture);
            if (m_RP.m_TexStages[i].Texture->m_eTT == eTT_Cubemap)
              m_pd3dDevice->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
            else
              m_pd3dDevice->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
            m_pd3dDevice->SetTransform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+i), (D3DMATRIX *)pRT->m_TexModificator.m_TexMatrix.GetData());
          }
          else
            m_pd3dDevice->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        }
      }
    }
    if (!bEnable)
      m_RP.m_FlagsModificators &= ~(RBMF_TCM | RBMF_TCG);
  }
  _inline void EF_CommitVLightsState()
  {
    uint i;
    if (m_RP.m_CurrentVLights != m_RP.m_EnabledVLights)
    {
      uint xorVL = (m_RP.m_CurrentVLights ^ m_RP.m_EnabledVLights) & 0xff;
      if (xorVL)
      {
        for (i=0; i<8; i++)
        {
          if (xorVL & (1<<i))
          {
            m_pd3dDevice->LightEnable(i, (m_RP.m_CurrentVLights & (1<<i)) != 0);
            if ((uint)(1<<(i+1)) > xorVL)
              break;
          }
        }
      }
      if (!m_RP.m_CurrentVLights && m_RP.m_EnabledVLights)
      {
        m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
        m_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
      }
      else
      if (m_RP.m_CurrentVLights && !m_RP.m_EnabledVLights)
      {
        m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
        m_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
      }
      m_RP.m_EnabledVLights = m_RP.m_CurrentVLights;
    }
    if (m_RP.m_EnabledVLights)
    {
      if ((m_RP.m_CurrentVLightFlags ^ m_RP.m_EnabledVLightFlags) & (LMF_NOADDSPECULAR | LMF_NOSPECULAR))
      {
        if (m_RP.m_CurrentVLightFlags & (LMF_NOADDSPECULAR | LMF_NOSPECULAR))
          m_pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
        else
          m_pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
      }
      m_RP.m_EnabledVLightFlags = m_RP.m_CurrentVLightFlags;
    }
    else
    if (!(m_RP.m_EnabledVLightFlags & (LMF_NOADDSPECULAR | LMF_NOSPECULAR)))
    {
      m_RP.m_EnabledVLightFlags |= (LMF_NOADDSPECULAR | LMF_NOSPECULAR);
      m_pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    }
  }
  _inline void EF_CommitShadersState()
  {
    if (!(m_RP.m_PersFlags & RBPF_VSNEEDSET) && CVProgram::m_LastVP)
    {
      if (m_RP.m_ClipPlaneWasOverrided == 2)
      {
        m_pd3dDevice->SetClipPlane(0, &m_RP.m_CurClipPlane.m_Normal[0]);
        m_RP.m_ClipPlaneWasOverrided = 0;
      }
      m_pd3dDevice->SetVertexShader(NULL);
      CVProgram::m_LastVP = 0;
      if (m_FS.m_bEnable && ((m_Features & RFT_HW_MASK) != RFT_HW_RADEON))
      {
        if (m_FS.m_nCurFogMode!=m_FS.m_nFogMode)
        {
          m_FS.m_nCurFogMode = m_FS.m_nFogMode;
          m_pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
        }
      }
    }
    else
    if ((m_RP.m_PersFlags & RBPF_VSNEEDSET) && ((m_Features & RFT_HW_MASK) != RFT_HW_RADEON))
    {
      if (m_FS.m_bEnable)
      {
        if (m_FS.m_nCurFogMode != 0)
        {
          m_FS.m_nCurFogMode = 0;
          m_pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
        }
      }
    }
    if (!(m_RP.m_PersFlags & RBPF_PS1NEEDSET) && CPShader::m_LastVP)
    {
      m_pd3dDevice->SetPixelShader(NULL);
      CPShader::m_LastVP = 0;
    }
    if (!CVProgram::m_LastVP)
    {
      if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
        EF_CommitTexTransforms(true);
    }
  }
  _inline void EF_CommitTexStageState()
  {
    if (m_RP.m_CurGlobalColor.dcolor != m_RP.m_NeedGlobalColor.dcolor)
    {
      m_RP.m_CurGlobalColor = m_RP.m_NeedGlobalColor;
      m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, m_RP.m_CurGlobalColor.dcolor );
    }
    int i;
    for (i=0; i<CTexMan::m_nCurStages; i++)
    {
      EF_SelectTMU(i);
      if (!i)
      {
        byte nCA = m_RP.m_TexStages[i].m_CA;
        byte nAA = m_RP.m_TexStages[i].m_AA;
        if (m_RP.m_FlagsPerFlush & RBSI_GLOBALRGB)
          nCA = ((m_RP.m_TexStages[i].m_CA & ~(7<<3)) | (eCA_Constant << 3));
        if (m_RP.m_FlagsPerFlush & RBSI_GLOBALALPHA)
          nAA = ((m_RP.m_TexStages[i].m_AA & ~(7<<3)) | (eCA_Constant << 3));
        EF_SetColorOp(m_RP.m_TexStages[i].m_CO, m_RP.m_TexStages[i].m_AO, nCA, nAA);
      }
      else
        EF_SetColorOp(m_RP.m_TexStages[i].m_CO, m_RP.m_TexStages[i].m_AO, m_RP.m_TexStages[i].m_CA, m_RP.m_TexStages[i].m_AA);
    }
  }
  _inline void EF_SetVertexStreams(TArray<SArrayPointer *>& Pointers, int Id)
  {
    for (int i=0; i<Pointers.Num(); i++)
    {
      SArrayPointer *ap = Pointers[i];
      ap->mfSet(Id);
    }
  }
  bool m_bTempHDRWasSet;
  _inline HRESULT EF_SetRenderTarget(LPDIRECT3DSURFACE9 pTargSurf, bool bSetZBuf)
  {
    HRESULT hr = 0;
    if (m_RP.m_PersFlags & RBPF_HDR)
      m_bTempHDRWasSet = true;
    if (pTargSurf != m_pCurBackBuffer)
    {
      hr = m_pd3dDevice->SetRenderTarget(0, pTargSurf);
      m_pCurBackBuffer = pTargSurf;
    }
    if (!FAILED(hr) && bSetZBuf && m_pCurZBuffer != m_pTempZBuffer)
    {
      hr = m_pd3dDevice->SetDepthStencilSurface(m_pTempZBuffer);
      m_pCurZBuffer = m_pTempZBuffer;
    }
    return hr;
  }
  _inline HRESULT EF_RestoreRenderTarget(LPDIRECT3DSURFACE9 pTargSurf = NULL)
  {
    if (!pTargSurf)
    {
      if (m_bTempHDRWasSet)
      {
        m_bTempHDRWasSet = false;
        assert(m_pHDRTargetSurf);
        pTargSurf = m_pHDRTargetSurf;
      }
      else
        pTargSurf = m_pBackBuffer;
    }
    HRESULT hr = 0;
    if (pTargSurf != m_pCurBackBuffer)
    {
      hr = m_pd3dDevice->SetRenderTarget(0, pTargSurf);
      m_pCurBackBuffer = pTargSurf;
    }
    if (!FAILED(hr) && m_pCurZBuffer != m_pZBuffer)
    {
      hr = m_pd3dDevice->SetDepthStencilSurface(m_pZBuffer);
      m_pCurZBuffer = m_pZBuffer;
    }
    return hr;
  }

  LPDIRECT3DVERTEXDECLARATION9 m_pLastVDeclaration;

  _inline HRESULT EF_SetVertexDeclaration(int StreamMask, int nVFormat)
  {
    HRESULT hr;

    assert (nVFormat>=0 && nVFormat<VERTEX_FORMAT_NUMS);

    if (!m_RP.m_D3DFixedPipeline[StreamMask][nVFormat].m_pDeclaration)
    {
      if(FAILED(hr = m_pd3dDevice->CreateVertexDeclaration(&m_RP.m_D3DFixedPipeline[StreamMask][nVFormat].m_Declaration[0], &m_RP.m_D3DFixedPipeline[StreamMask][nVFormat].m_pDeclaration)))
        return hr;
    }
    if (m_pLastVDeclaration != m_RP.m_D3DFixedPipeline[StreamMask][nVFormat].m_pDeclaration)
    {
      m_pLastVDeclaration = m_RP.m_D3DFixedPipeline[StreamMask][nVFormat].m_pDeclaration;
      return m_pd3dDevice->SetVertexDeclaration(m_RP.m_D3DFixedPipeline[StreamMask][nVFormat].m_pDeclaration);
    }
    return S_OK;
  }
  int GetAAFormat(TArray<SAAFormat>& Formats, bool bReset);

  void EF_SetColorOp(byte eCo, byte eAo, byte eCa, byte eAa);

  // Clip Planes support
  void EF_SetClipPlane (bool bEnable, float *pPlane, bool bRefract);

  void EF_HDRPostProcessing();

  void EF_Init();
  void EF_PreRender(int Stage);
  void EF_PostRender();
  void EF_RenderPipeLine(void (*RenderFunc)());
  void EF_InitRandTables();
  void EF_InitWaveTables();
  void EF_InitEvalFuncs(int num);
  void EF_InitD3DFixedPipeline();
  bool EF_PreDraw(SShaderPass *sl, bool bSetVertexDecl=true);
  void EF_SetCameraInfo();
  void EF_SetObjectTransform(CCObject *obj, SShader *pSH, int nTransFlags);
  bool EF_ObjectChange(SShader *Shader, SRenderShaderResources *pRes, int nObject, CRendElement *pRE);

  void EF_UpdateTextures(SShaderPass *Layer);
  void EF_UpdateTextures(SShaderPassHW *Layer);
  
  void EF_EvalNormalsRB(SShader *ef);
  void EF_Eval_DeformVerts(TArray<SDeform>* Defs);
  void EF_Eval_RGBAGen(SShaderPass *sfm);
  void EF_Eval_TexGen(SShaderPass *sfm);

  void EF_ApplyMatrixOps(TArray<SMatrixTransform>* MatrixOps, bool bEnable);
  
  bool EF_SetLights(int Flags);
  void EF_SetHWLight(int Num, vec4_t Pos, CFColor& Diffuse, CFColor& Specular, float ca, float la, float qa, float fRange);

  void EF_DrawDebugLights();
  void EF_DrawDebugTools();

  static void EF_DrawWire();
  static void EF_DrawNormals();
  static void EF_DrawTangents();
  static void EF_Flush();

  int m_sPrevX, m_sPrevY, m_sPrevWdt, m_sPrevHgt;
  bool m_bsPrev;
  _inline void EF_Scissor(bool bEnable, int sX, int sY, int sWdt, int sHgt)
  {
    if (!CV_r_scissor)
      return;
    RECT scRect;
    if (bEnable)
    {
      if (sX != m_sPrevX || sY != m_sPrevY || sWdt != m_sPrevWdt || sHgt != m_sPrevHgt)
      {
        m_sPrevX = sX;
        m_sPrevY = sY;
        m_sPrevWdt = sWdt;
        m_sPrevHgt = sHgt;
        scRect.left = sX;
        scRect.right = sX + sWdt;
        scRect.top = sY;
        scRect.bottom = sY + sHgt;
        m_pd3dDevice->SetScissorRect(&scRect);
      }
      if (bEnable != m_bsPrev)
      {
        m_bsPrev = bEnable;
        m_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
      }
    }
    else
    {
      if (bEnable != m_bsPrev)
      {
        m_bsPrev = bEnable;
        m_sPrevWdt = 0;
        m_sPrevHgt = 0;
        m_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
      }
    }
  }

  _inline void EF_SetFogColor(DWORD dwColor, bool bHDRIncr, bool bFogVP)
  {
    if (dwColor != m_FS.m_CurColor.dcolor)
    {
      m_FS.m_CurColor.dcolor = dwColor;
      m_pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, dwColor);
      if ((m_RP.m_PersFlags & RBPF_HDR) || bFogVP)
      {
        int n = 31; //PSCONST_HDR_FOGCOLOR;
        //if (m_CurParams[n][0] != v[0] || m_CurParams[n][1] != v[1] || m_CurParams[n][2] != v[2] || m_CurParams[n][3] != v[3])
        {
          CFColor col = CFColor((uint)dwColor);
          Exchange(col.r, col.b);
          if (!bHDRIncr)
          {
            col.r = col.r / 4;
            col.g = col.g / 4;
            col.b = col.b / 4;
          }
          m_pd3dDevice->SetPixelShaderConstantF(n, &col[0], 1);
        }
      }
    }
  }
  _inline int EF_FogCorrection(bool bFogDisable, bool bFogVP)
  {
    int bFogOverride = 0;
    if (bFogDisable || bFogVP)
    {
      bFogDisable = true;
      if (m_FS.m_bEnable)
        m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, false);
    }
    switch (m_CurState & GS_BLEND_MASK)
    {
      case GS_BLSRC_ONE | GS_BLDST_ONE:
        bFogOverride = 1;
        EF_SetFogColor(0, true, bFogVP);
        break;
      case GS_BLSRC_DSTALPHA | GS_BLDST_ONE:
        bFogOverride = 1;
        EF_SetFogColor(0, true, bFogVP);
        break;
      case GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL:
        bFogOverride = 1;
        EF_SetFogColor(0x808080, false, bFogVP);
        break;
      case GS_BLSRC_ONE | GS_BLDST_ONEMINUSSRCALPHA:
        bFogOverride = 1;
        EF_SetFogColor(0, true, bFogVP);
        break;
      case GS_BLSRC_ONE | GS_BLDST_ONEMINUSSRCCOL:
        bFogOverride = 1;
        EF_SetFogColor(0, true, bFogVP);
        break;
      case GS_BLSRC_ZERO | GS_BLDST_ONEMINUSSRCCOL:
        bFogOverride = 1;
        EF_SetFogColor(0, true, bFogVP);
        break;
      case GS_BLSRC_SRCALPHA | GS_BLDST_ONE:
        bFogOverride = 1;
        EF_SetFogColor(0, true, bFogVP);
        break;
      case GS_BLSRC_ZERO | GS_BLDST_ONE:
        bFogOverride = 1;
        EF_SetFogColor(0, true, bFogVP);
        break;
      case GS_BLSRC_DSTCOL | GS_BLDST_ZERO:
        bFogOverride = 1;
        EF_SetFogColor(0xffffffff, false, bFogVP);
        break;
    }
    if (bFogDisable)
      bFogOverride = 2;
    if (bFogVP)
      bFogOverride |= 4;
    return bFogOverride;
  }
  _inline void EF_FogRestore(int bFogOverrided)
  {
    if (bFogOverrided)
    {
      if ((bFogOverrided & 3) == 2)
      {
        if (m_FS.m_bEnable)
          m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, true);
      }      
      EF_SetFogColor(D3DRGBA(m_FS.m_FogColor.r, m_FS.m_FogColor.g, m_FS.m_FogColor.b, m_FS.m_FogColor.a), true, (bFogOverrided & 4) != 0);
    }
  }

  _inline void EF_Draw(SShader *sh, SShaderPass *sl)
  {
    int bFogOverrided = 0;

    // Unlock all VB (if needed) and set current streams
    EF_PreDraw(sl);

    {
      //PROFILE_FRAME_TOTAL(Draw_EFIndexMesh);

      if (m_FS.m_bEnable)
        bFogOverrided = EF_FogCorrection(false, false);

      if (m_RP.m_pRE)
        m_RP.m_pRE->mfDraw(sh, sl);
      else
        EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);

      EF_FogRestore(bFogOverrided);
      if (!m_RP.m_LastVP)
      {
        if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
          EF_CommitTexTransforms(false);
      }
    }
  }
  EShaderPassType m_SHPTable[eSHP_MAX];
  _inline bool EF_DrawPasses(EShaderPassType eShPass, SShaderTechnique *hs, SShader *ef, int nStart, int &nEnd)
  {
    bool bLights = false;
    switch(eShPass)
    {
      case eSHP_General:
        EF_DrawGeneralPasses(hs, ef, false, nStart, nEnd, false);
        break;
      case eSHP_DiffuseLight:
      case eSHP_SpecularLight:
      case eSHP_Light:
        EF_DrawLightPasses(hs, ef, nStart, nEnd, false);
        bLights = true;
        break;
      case eSHP_MultiLights:
        EF_DrawLightPasses_PS30(hs, ef, nStart, nEnd, false);
        bLights = true;
        break;
      case eSHP_MultiShadows:
        nEnd = EF_DrawMultiShadowPasses(hs, ef, nStart);
        bLights = true;
        break;
      case eSHP_Shadow:
        EF_DrawShadowPasses(hs, ef, nStart, nEnd, false);
        break;
      case eSHP_Fur:
      case eSHP_SimulatedFur:
        EF_DrawFurPasses(hs, ef, nStart, nEnd, eShPass);
        break;
      default:
        assert(false);
    }
    return bLights;
  }
  void EF_DrawIndexedMesh (int nPrimType);

  void EF_DrawInstances(SShader *ef, SShaderPassHW *slw, int nCurInst, int nLastInst, int nUsage, byte bUsage[], int StreamMask);
  void EF_DrawGeometryInstancing_VS30(SShader *ef, SShaderPassHW *slw, CVProgram *curVP);
  void EF_DrawGeneralPasses(SShaderTechnique *hs, SShader *ef, bool bVolFog, int nStart, int nEnd, bool bDstAlpha);
  void EF_DrawLightPasses(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd, bool bDstAlpha);
  void EF_DrawLightPasses_PS30(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd, bool bDstAlpha);
  void EF_DrawShadowPasses(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd, bool bDstAlpha);
  int  EF_DrawMultiShadowPasses(SShaderTechnique *hs, SShader *ef, int nStart);
  void EF_DrawFurPasses(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd, EShaderPassType eShPass);
  void EF_DrawSubsurfacePasses(SShaderTechnique *hs, SShader *ef);
  void EF_DrawLightShadowMask(int nLight);
  void EF_DrawFogOverlayPasses();
  void EF_DrawDetailOverlayPasses();

  void EF_FlushRefractedObjects(SShader *pSHRefr[], CRendElement *pRERefr[], CCObject *pObjRefr[], int nObjs, int nFlags, int DLDFlags);

  void EF_FlushHW();
  void EF_SetStateShaderState();
  void EF_ResetStateShaderState();
  bool EF_SetResourcesState(bool bSet);
  void EF_FlushShader();
  int  EF_Preprocess(SRendItemPre *ri, int nums, int nume);
  void EF_DrawREPreprocess(SRendItemPreprocess *ris, int Nums);
  void EF_PipeLine(int nums, int nume, int nList, void (*RenderFunc)());
  
  STWarpZone *EF_SetWarpZone(SWarpSurf *sf, int *NumWarps, STWarpZone Warps[]);
  void EF_UpdateWarpZone(STWarpZone *wp, SWarpSurf *srf);
  bool EF_CalcWarpCamera(STWarpZone *wp, int nObject, CCamera& prevCam, CCamera& newCam);
  bool EF_RenderWarpZone(STWarpZone *wp);

  void EF_PrintProfileInfo();

  virtual void EF_CheckOverflow(int nVerts, int nTris, CRendElement *re);
  virtual void EF_EndEf3D (int nFlags);
  virtual void EF_EndEf2D(bool bSort);  // 2d only
  virtual void EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, int nFog, CRendElement *re);
  virtual void EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, CRendElement *re);
  virtual bool EF_SetLightHole(Vec3 vPos, Vec3 vNormal, int idTex, float fScale=1.0f, bool bAdditive=true);
  
  virtual int  EF_RegisterFogVolume(float fMaxFogDist, float fFogLayerZ, CFColor color, int nIndex=-1, bool bCaustics=false);
  virtual void SetTexGen(byte eTC) {};

  virtual STexPic *EF_MakeSpecularTexture(float fExp);
  virtual STexPic *EF_MakePhongTexture(int Exp);
  virtual void EF_PolygonOffset(bool bEnable, float fFactor, float fUnits);

  virtual WIN_HWND GetHWND() { return  m_hWnd; }
    
	virtual void SetColorOp(byte eCo, byte eAo, byte eCa, byte eAa);
};

extern CD3D9Renderer *gcpRendD3D;

//=========================================================================================

//////////////////////////////////////////////////////////////////////////////
// CFnMap8: 2D procedural texture
//////////////////////////////////////////////////////////////////////////////
class CFnMap9 : public CBaseMap
{
private:
  static VOID WINAPI Fill2DWrapper(D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord, const D3DXVECTOR2* pTexelSize, LPVOID pData);

protected:
  D3DFORMAT m_Format;

  virtual D3DXCOLOR Function(const D3DXVECTOR2* pTexCoord, const D3DXVECTOR2* pTexelSize) = 0;

public:
  CFnMap9()
  {
    m_Format = D3DFMT_A8R8G8B8;
  }
  HRESULT Initialize();
};

extern UINT ColorChannelBits( D3DFORMAT fmt );
extern UINT ColorBits( D3DFORMAT fmt );

//-----------------------------------------------------------------------------
// Ghost map
//-----------------------------------------------------------------------------
class CGhostMap : public CFnMap9
{
public:
  CGhostMap(DWORD size, STexPic *pTP)
  {
    m_dwWidth = size;
    m_pTex = pTP;
  }
  D3DXCOLOR Function(const D3DXVECTOR2* p, const D3DXVECTOR2* s)
  {
    // input is p->x = (Distance^2/Attenuation^2)
    FLOAT fx = (float)1-p->x;
    FLOAT fi = min(1.0f, max(0.0f, min(fx*fx+.1f, 1-fx*fx*fx) ) );
    return D3DXCOLOR(fi, fi, fi, fi);
  }
};

//-----------------------------------------------------------------------------
// Fur map
//-----------------------------------------------------------------------------
struct SFurLayers
{
  TArray <STexPic *> m_Layers;
};

class CFurMap : public CFnMap9
{
  TArray<SFurLayers *> m_Inst;
  int m_nCurInst;

public:
  CFurMap(STexPic *pTP)
  {
    m_pTex = pTP;
  }
  HRESULT Initialize(int seed, int size, int num, SFurLayers *fl);
  virtual D3DXCOLOR Function(const D3DXVECTOR2* pTexCoord, const D3DXVECTOR2* pTexelSize)
  {
    return D3DXCOLOR();
  }
  void Update(int nLayers);
  void Bind (float layer, int nTMU);
};

struct SDynFurInstance
{
  bool m_bPrepared;
  int m_nFrame;
  uint m_nUsedFrame;

  STexPic *m_pTexOffset0;
  STexPic *m_pTexOffset1;
  STexPic *m_pTexNormal;

  Vec3 m_Trans;
  Vec3 m_Accel;
  Vec3 m_PrevVel;
  Vec3 m_OmegaAccel;
  Vec3 m_PrevOmega;
  SDynFurInstance()
  {
    m_pTexOffset0 = NULL;
    m_pTexOffset1 = NULL;
    m_pTexNormal = NULL;

    m_bPrepared = false;
    m_Accel = Vec3(0,0,0);
    m_PrevVel = Vec3(0,0,0);
    m_OmegaAccel = Vec3(0,0,0);
    m_PrevOmega = Vec3(0,0,0);
  }
};

class CFurNormalMap : public CFnMap9
{
public:
  int m_nCurInst;
  TArray<SDynFurInstance> m_Inst;

  STexPic *m_pTexClamp;
  STexPic *m_pTexNormalize;

public:
  CFurNormalMap(STexPic *pTP, int Width, int Height)
  {
    m_pTexClamp = NULL;
    m_pTexNormalize = NULL;

    m_nCurInst = -1;
    m_dwWidth = Width;
    m_dwHeight = Height;
  }
  HRESULT Initialize();
  virtual D3DXCOLOR Function(const D3DXVECTOR2* pTexCoord, const D3DXVECTOR2* pTexelSize)
  {
    return D3DXCOLOR();
  }
  void Update(EShaderPassType eShPass, float dt, SShaderPassHW *slw, bool bUseSimulation);
  void Bind(int nTMU);
};

void ClearRenderTarget(LPDIRECT3DDEVICE9 plD3DDevice, STexPic *&pTex, uchar r, uchar g, uchar b, uchar a);

// round up sigh
#define BILERP_PROTECTION	2
// attenuation texture function width ranges from 0-1 with linear interpolation between sample
#define ATTENUATION_WIDTH			512
// space for 512 different attuentation functions
#define NUM_ATTENUATION_FUNCTIONS	8

/* Some attenuation shapes
*/
enum ATTENUATION_FUNCTION
{
	AF_LINEAR,
	AF_SQUARED,
	AF_SHAPE1,
	AF_SHAPE2,
};

class CD3D9TexMan : public CTexMan
{
protected:
  virtual STexPic *CreateTexture(const char *name, int wdt, int hgt, int depth, uint flags, uint flags2, byte *dst, ETexType eTT, float fAmount1=-1.0f, float fAmount2=-1.0f, int DXTSize=0, STexPic *ti=NULL, int bind=0, ETEX_Format eTF=eTF_8888, const char *szSourceName=NULL);
  virtual STexPic *CopyTexture(const char *name, STexPic *ti, int CubeSide=-1);

public:  
  CD3D9TexMan() : CTexMan()
  {
#ifndef _XBOX  
    m_CurPal = 1;
#endif
    m_pCurCubeTexture = NULL;
  }
  STexPic *CD3D9TexMan::CreateTexture(int nWidth, int nHeight, D3DFORMAT d3dFMT, int d3dUsage, bool bMips, const char *szName);

  void D3DCompressTexture(int tgt, STexPicD3D *ti, int CubeSide);
  LPDIRECT3DTEXTURE9 CD3D9TexMan::D3DCreateSrcTexture(STexPicD3D *ti, byte *src, D3DFORMAT srcFormat, int SizeSrc, int DXTSize);
  void D3DCreateVideoTexture(int tgt, byte *src, int wdt, int hgt, int depth, D3DFORMAT SrcFormat, D3DFORMAT DstFormat, STexPicD3D *ti, bool bMips, int CubeSide, PALETTEENTRY *pe, int DXTSize);
  void BuildMipsSub(byte* src, int wdt, int hgt);
  void ClearBuffer(int Width, int Height, bool bEnd, STexPic *pImage, int Side);
  void DrawCubeSide(const float *angle, Vec3& Pos, int tex_size, int side, int RendFlags, float fFarDist);

  float CalcFogVal(float fi, float fj);
  void GenerateNoiseVolumeMap();
  void GenerateDepthLookup();
  void GenerateAttenMap();
  void GenerateFlareMap();
  void GenerateFogMaps();
  void GenerateGhostMap();
  void GenerateFurNormalMap();
  void GenerateFurLightMap();
  void GenerateFurMap();
  void GenerateHDRMaps();
  void DestroyHDRMaps();

  static void CalcMipsAndSize(STexPic *ti);

  static BindNULL(int From)
  {
    int n = CTexMan::m_nCurStages;
    CTexMan::m_nCurStages = From;
    for (; From<n; From++)
    {
      HRESULT hr = gcpRendD3D->mfGetD3DDevice()->SetTexture(From, NULL);
      gcpRendD3D->EF_SelectTMU(From);
      gcpRendD3D->EF_SetColorOp(eCO_DISABLE, eCO_DISABLE, 255, 255);
      gcpRendD3D->m_RP.m_TexStages[From].Texture = NULL;
    }
  }

  virtual ~CD3D9TexMan();

  virtual STexPic *GetByID(int Id);
  virtual STexPic *AddToHash(int Id, STexPic *ti);
  virtual void RemoveFromHash(int Id, STexPic *ti);
  virtual void SetTexture(int Id, ETexType eTT);

  virtual byte *GenerateDXT_HW(STexPic *ti, EImFormat eF, byte *dst, int *numMips, int *DXTSize, bool bMips);
  virtual bool SetFilter(char *filt);
  virtual void UpdateTextureData(STexPic *pic, byte *data, int USize, int VSize, bool bProc, int State, bool bPal);
  virtual void UpdateTextureRegion(STexPic *pic, byte *data, int X, int Y, int USize, int VSize);
  virtual STexPic *CreateTexture();
  virtual bool ScanEnvironmentCM (const char *name, int size, Vec3& Pos);
  virtual void GetAverageColor(SEnvTexture *cm, int nSide);
  virtual void ScanEnvironmentCube(SEnvTexture *cm, int RendFlags, int Size, bool bLightCube);
  virtual void ScanEnvironmentTexture(SEnvTexture *cm, SShader *pSH, SRenderShaderResources *pRes, int RendFlags, bool bUseExistingREs);
  virtual void EndCubeSide(CCObject *obj, bool bNeedClear);
  virtual void StartCubeSide(CCObject *obj);
  virtual void DrawToTexture(Plane& Pl, STexPic *Tex, int RendFlags);
  virtual void DrawToTextureForDof(int Id);
  virtual void DrawToTextureForGlare(int Id);
  virtual void DrawToTextureForRainMap(int Id);
  virtual void StartHeatMap(int Id);
  virtual void EndHeatMap();
  virtual void StartRefractMap(int Id);
  virtual void EndRefractMap();
  virtual void StartNightMap(int Id);
  virtual void EndNightMap();
  virtual void StartScreenMap(int Id);
  virtual void EndScreenMap();
    
  // tiago: added
  virtual void StartScreenTexMap(int Id);
  virtual void EndScreenTexMap();
  virtual bool PreloadScreenFxMaps(void);

  virtual void DrawFlashBangMap(int Id, int RendFlags, CREFlashBang *pRE);
  virtual void Update();
  virtual void GenerateFuncTextures();

  IDirect3DSurface9* m_pZSurf;
#ifdef USE_HDR	
	IDirect3DSurface9* m_HDR_RT_FSAA;
#endif
  CCamera m_PrevCamera;
  int m_TempX, m_TempY, m_TempWidth, m_TempHeight;

  static int TexSize(int wdt, int hgt, int mode);

#ifndef _XBOX  
  int m_CurPal;
#endif

  static int m_Format;
  static int m_FirstBind;

  TTextureMap m_RefTexs;
};

void HDR_DrawDebug();

#ifdef _DEBUG
struct SDynVB
{
  IDirect3DResource9 *m_pRes;
  SVertexStream *m_pStr;
  CVertexBuffer *m_pBuf;
  char m_szDescr[256];
};
struct SDynTX
{
  IDirect3DBaseTexture9 *m_pRes;
  STexPic *m_pTP;
  char m_szDescr[256];
};

extern TArray<SDynVB> gDVB;
extern TArray<SDynTX> gDTX;

_inline void sAddVB(IDirect3DResource9 *pRes, SVertexStream *pStr, CVertexBuffer *pBuf, const char *szDesc)
{
  return;
  if (!pStr->m_bDynamic)
    return;

  int i;
  for (i=0; i<gDVB.Num(); i++)
  {
    if (gDVB[i].m_pRes == pRes)
      assert(0);
  }
  SDynVB VB;
  VB.m_pRes = pRes;
  VB.m_pBuf = pBuf;
  VB.m_pStr = pStr;
  if (szDesc)
    strcpy(VB.m_szDescr, szDesc);
  else
    VB.m_szDescr[0] = 0;
  gDVB.AddElem(VB);
}
_inline void sRemoveVB(IDirect3DResource9 *pRes, SVertexStream *pStr)
{
  return;
  if (!pStr->m_bDynamic)
    return;

  int i;
  for (i=0; i<gDVB.Num(); i++)
  {
    if (gDVB[i].m_pRes == pRes)
    {
      gDVB.Remove(i);
      return;
    }
  }
  assert(0);
}

/*_inline void sAddTX(STexPic *tp, const char *szDesc)
{
  IDirect3DBaseTexture9 *pRes = (IDirect3DBaseTexture9 *)tp->m_RefTex->m_VidTex;
  int i;
  for (i=0; i<gDTX.Num(); i++)
  {
    if (gDTX[i].m_pRes == pRes)
      assert(0);
  }
  SDynTX TX;
  TX.m_pRes = pRes;
  if (szDesc)
    strcpy(TX.m_szDescr, szDesc);
  else
    TX.m_szDescr[0] = 0;
  TX.m_pTP = tp;
  gDTX.AddElem(TX);
}
_inline void sRemoveTX(STexPic *tp)
{
  IDirect3DTexture9 *pID3DTexture = NULL;
  IDirect3DCubeTexture9 *pID3DCubeTexture = NULL;
  LPDIRECT3DSURFACE9 pSurf = NULL;
  D3DSURFACE_DESC Desc;
  HRESULT hr;
  if (tp->m_eTT == eTT_Cubemap)
  {
    pID3DCubeTexture = (IDirect3DCubeTexture9*)tp->m_RefTex->m_VidTex;
    hr = pID3DCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)0, 0, &pSurf);
  }
  else
  if (tp->m_eTT == eTT_Base || tp->m_eTT == eTT_Bumpmap)
  {
    pID3DTexture = (IDirect3DTexture9*)tp->m_RefTex->m_VidTex;
    hr = pID3DTexture->GetSurfaceLevel(0, &pSurf);
  }
  if (!pSurf)
    return;
  hr = pSurf->GetDesc(&Desc);
  SAFE_RELEASE(pSurf);
  if (Desc.Pool != D3DPOOL_DEFAULT)
    return;

  IDirect3DBaseTexture9 *pRes = (IDirect3DBaseTexture9 *)pID3DTexture;
  if (!pRes)
    pRes = (IDirect3DBaseTexture9 *)pID3DCubeTexture;
  int i;
  for (i=0; i<gDTX.Num(); i++)
  {
    if (gDTX[i].m_pRes == pRes)
    {
      gDTX.Remove(i);
      return;
    }
  }
  assert(0);
}*/
#endif

#endif //PS2

#endif // DRIVERD3D9_H
