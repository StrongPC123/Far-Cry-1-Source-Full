/*=============================================================================
  DriverD3D8.h : Direct3D8 Render interface declarations.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#ifndef DRIVERD3D8_H
#define DRIVERD3D8_H

#if _MSC_VER > 1000
# pragma once 
#endif

#ifndef PS2

/*
===========================================
The DXRenderer interface Class
===========================================
*/

#ifndef _XBOX
// Base class
#include <d3dx8.h>
#else
#include <xtl.h>
#include <xgraphics.h>
#endif

extern SPipeVertex_D_1T gVerts[];

//=======================================================================

#ifndef _XBOX
#include "cg\cgD3D8.h"
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

extern HRESULT h;

#include "StaticVB.h"
#include "StaticIB.h"
#include "D3DTexture.h"
#include "D3DPShaders.h"

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

typedef std::map<int,SRefTex*> TTextureMap;
typedef TTextureMap::iterator TTextureMapItor;

#define TEXTGT_2D 0
#define TEXTGT_CUBEMAP 1

struct STexPicD3D : public STexPic
{
  STexPicD3D() : STexPic()
  {
    m_RefTex = NULL;
  }

  virtual void SaveTGA(const char *name, bool bMips);
  virtual void SaveJPG(const char *name, bool bMips);
  
  virtual void Release(bool bForce);

  virtual void Set();
  
  virtual void SetClamp(bool bEnable);

  virtual void BuildMips(TArray<SMipmap *>* Mips);
  virtual void DownloadMips(TArray<SMipmap *>* Mips, int nStartMip, int nEndMip, int SizeFirst);
  virtual void ReleaseHW();
};

//=====================================================

struct STexStageInfo
{
  DWORD      TextureCacheID;
  bool       UseMips;
  bool       Repeat;
  int        MagFilter;
  int        MinFilter;
  int        Anisotropic;
#ifndef _XBOX
  int        Palette;
#else
  D3DPalette* pPalette;
#endif
  STexPic    *Texture;
  STexStageInfo() {}
  void Flush()
  {
    TextureCacheID  = 0;
    UseMips = (bool)-1;
    Repeat = (bool)-1;
    Anisotropic = 255;
    Texture = NULL;
#ifndef _XBOX
    Palette = -1;
#else
    pPalette = NULL;
#endif
  }
};


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

struct SDeviceInfo
{
  GUID       Guid;
	//[Alberto] I changed the decalration from String to string because String 
	//was removed
	string     Description;
  string     Name;
  SDeviceInfo( GUID InGuid, const TCHAR* InDescription, const TCHAR* InName ) : Guid(InGuid), Description(InDescription), Name(InName) {}
};

struct SStencil_D3D : public SStencil
{
  int Func;
  int FuncRef;
  int FuncMask;

  int OpFail;
  int OpZFail;
  int OpZPass;
  
  virtual void mfSet();
  virtual void mfReset();
};


#define BUFFERED_VERTS 256


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


//-----------------------------------------------------------------------------
// Name: struct D3DModeInfo
// Desc: Structure for holding information about a display mode
//-----------------------------------------------------------------------------
struct SD3DModeInfo
{
  DWORD      Width;      // Screen width in this mode
  DWORD      Height;     // Screen height in this mode
  D3DFORMAT  Format;     // Pixel format in this mode
  DWORD      dwBehavior; // Hardware / Software / Mixed vertex processing
  D3DFORMAT  DepthStencilFormat; // Which depth/stencil format to use with this mode
};




//-----------------------------------------------------------------------------
// Name: struct D3DDeviceInfo
// Desc: Structure for holding information about a Direct3D device, including
//       a list of modes compatible with this device
//-----------------------------------------------------------------------------
struct SD3DDeviceInfo
{
  // Device data
  D3DDEVTYPE   DeviceType;      // Reference, HAL, etc.
  D3DCAPS8     d3dCaps;         // Capabilities of this device
  const TCHAR* strDesc;         // Name of this device
  BOOL         bCanDoWindowed;  // Whether this device can work in windowed mode

  // Modes for this device
  DWORD        dwNumModes;
  SD3DModeInfo  modes[150];

  // Current state
  DWORD        dwCurrentMode;
  BOOL         bWindowed;
  D3DMULTISAMPLE_TYPE MultiSampleType;
};

//-----------------------------------------------------------------------------
// Name: struct D3DAdapterInfo
// Desc: Structure for holding information about an adapter, including a list
//       of devices available on this adapter
//-----------------------------------------------------------------------------
struct SD3DAdapterInfo
{
  // Adapter data
  D3DADAPTER_IDENTIFIER8 d3dAdapterIdentifier;
  D3DDISPLAYMODE d3ddmDesktop;      // Desktop display mode for this adapter

  int mMaxWidth, mMaxHeight;

  // Devices for this adapter
  DWORD          dwNumDevices;
  SD3DDeviceInfo  devices[5];

  // Current state
  DWORD          dwCurrentDevice;
};

struct SD3DContext
{
  WIN_HWND m_hWnd;
  int m_X;
  int m_Y;
  int m_Width;
  int m_Height;
};

//======================================================================
/// Direct3D Render driver class

class CD3D8Renderer : public CRenderer
{
  friend class CD3D8TexMan;
  
public:
  CD3D8Renderer();
  ~CD3D8Renderer();

protected:

// Windows context
  char      m_WinTitle[80];
  HINSTANCE m_hInst;            
  HWND      m_hWnd;              // The main app window

  SD3DAdapterInfo   m_Adapters[10];
  int               m_dwAdapter;
  int               m_dwNumAdapters;

  // From D3D.
  // Main objects used for creating and rendering the 3D scene
  D3DPRESENT_PARAMETERS m_d3dpp;         // Parameters for CreateDevice/Reset
  LPDIRECT3D8       m_pD3D;              // The main D3D object
  D3DCAPS8          m_d3dCaps;           // Caps for the device
  D3DSURFACE_DESC   m_d3dsdZBuffer;      // Surface desc of the Zbuffer
  LPDIRECT3DSURFACE8 m_pZBuffer;
  LPDIRECT3DSURFACE8 m_pBackBuffer;
  DWORD             m_dwCreateFlags;     // Indicate sw or hw vertex processing
  DWORD             m_dwWindowStyle;     // Saved window style for mode switches
  RECT              m_rcWindowBounds;    // Saved window bounds for mode switches
  RECT              m_rcWindowClient;    // Saved client area size for mode switches
  TCHAR             m_strDeviceStats[90];// String to hold D3D device stats
  D3DVIEWPORT8      m_Viewport;
  
  int               m_MinDepthBits;    // Minimum number of bits needed in depth buffer
  int               m_MinStencilBits;  // Minimum number of bits needed in stencil buffer

  bool             m_bActive;
  bool             m_bReady;
  bool             m_bGammaCalibrate;
  bool             m_bUseSWVP;

  bool             m_bAllowAlphaPalettes;

  bool             m_bDeviceDoesFlatCubic;
  bool             m_bDeviceDoesGaussianCubic;

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

  D3DMATERIAL8 m_Material;
  D3DLIGHT8 m_Lights[16];

  float m_CurStereoSeparation;

//==================================================================

public:
  STexStageInfo     mStages[MAX_TMU];
  int m_TextureBits;
  IDirect3DVertexBuffer8 *m_pQuadVB;
  IDirect3DVertexBuffer8 *m_pLineVB;
  LPDIRECT3DDEVICE8 m_pd3dDevice;        // The D3D rendering device

#ifndef _XBOX
  CGcontext m_CGContext;
#endif

  // Pixel formats from D3D.
  SPixFormat        mFormat8000;    //8 bit alpha
  SPixFormat        mFormat8888;    //32 bit
  SPixFormat        mFormat1555;    //16 bit
  SPixFormat        mFormat4444;    //16 bit
  SPixFormat        mFormat0555;    //16 bit
  SPixFormat        mFormat0565;    //16 bit
  SPixFormat        mFormatU8V8;    //16 bit
  SPixFormat        mFormatQ8W8U8V8;  //32 bit
  SPixFormat        mFormatX8L8U8V8;  //32 bit
  SPixFormat        mFormatU5V5L6;  //16 bit
  SPixFormat        mFormatDXT1;    //Compressed RGB
  SPixFormat        mFormatDXT3;    //Compressed RGBA
  SPixFormat        mFormatDXT5;    //Compressed RGBA
  SPixFormat        mFormatPal8;    //Paletted 8 bit

  SPixFormat*       mFirstPixelFormat;

  int mZBias;

  byte m_GammmaTable[256];

  bool m_bEnableLights;

public:

  LPDIRECT3DDEVICE8  mfGetD3DDevice() { return m_pd3dDevice; }
  LPDIRECT3D8        mfGetD3D() { return m_pD3D; }
  D3DSURFACE_DESC*   mfGetZSurfaceDesc() { return &m_d3dsdZBuffer; }
  LPDIRECT3DSURFACE8 mfGetZSurface() { return m_pZBuffer; }
  LPDIRECT3DSURFACE8 mfGetBackSurface() { return m_pBackBuffer; }
  D3DCAPS8           *mfGetD3DCaps()  { return &m_d3dCaps; }

  int GetAnisotropicLevel()
  {
    if (GetFeatures() & RFT_ALLOWANISOTROPIC)
      return CV_r_texture_anisotropic_level;
    return 0;
  }

  void SetDefaultTexParams(bool bUseMips, bool bRepeat, bool bLoad);

public:

  //=============================================================

  byte eCurColorOp[MAX_TMU];
  int msCurState;

  void D3DSetCull(ECull eCull);
  char *D3DError( HRESULT h );
  void DrawQuad(float x0, float y0, float x1, float y1, const CFColor & color, float ftx0 = 0,  float fty0 = 0, float ftx1 = 1, float fty1 = 1);
  void DrawQuad3D(const Vec3d & v0, const Vec3d & v1, const Vec3d & v2, const Vec3d & v3, const CFColor & color, 
                  float ftx0 = 0,  float fty0 = 0,  float ftx1 = 1,  float fty1 = 1,
                  float ftx2 = 0,  float fty2 = 0,  float ftx3 = 1,  float fty3 = 1);


private:

  void RegisterVariables();
  void UnRegisterVariables();

  bool Error(char *Msg, HRESULT h);

  bool SetWindow(int width, int height, bool fullscreen, WIN_HWND hWnd);
  bool SetRes();
  void UnSetRes();
  HRESULT DeleteDeviceObjects();
  HRESULT InvalidateDeviceObjects();
  HRESULT FinalCleanup();

  HRESULT ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior, D3DFORMAT Format );
  bool FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType, D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat );
  HRESULT BuildDevicesList();
  HRESULT AdjustWindowForChange();
  HRESULT InitDeviceObjects();
  void RecognizePixelFormat(SPixFormat& Dest, D3DFORMAT FromD3D, INT InBitsPerPixel, const TCHAR* InDesc);
  HRESULT RestoreDeviceObjects();
  void SetRendParms(SD3DModeInfo *pModeInfo, SD3DDeviceInfo *pDeviceInfo);
  HRESULT Initialize3DEnvironment();
  bool IsSuitableDevice(int a, bool bAllowSoft);
  void DestroyWindow(void);
  void RestoreGamma(void);
  void SetGamma(float fGamma, float fBrigtness, float fContrast);

  virtual char*	GetVertexProfile();
  virtual char*	GetPixelProfile();

  void PrepareOutSpaceTextures(CREOutSpace * pRE);
  void ScanOutSpaceCube(uint & nTexID, const Vec3d & vPos, int * pResult);
  void DrawOutSpaceSide( const float *angle, const Vec3d& Pos, int tex_size, int offsetX, int offsetY);

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
  

  static int CV_d3d8_texture_filter_anisotropic;
  static int CV_d3d8_nodeviceid;
  static int CV_d3d8_palettedtextures;
  static int CV_d3d8_compressedtextures;
  static int CV_d3d8_usebumpmap;
  static int CV_d3d8_bumptype;
  static int CV_d3d8_forcesoftwarevp;
  static int CV_d3d8_texturebits;
  static int CV_d3d8_texmipfilter;
  static ICVar *CV_d3d8_texturefilter;
  static int CV_d3d8_squaretextures;
  static int CV_d3d8_mipprocedures;
  static ICVar *CV_d3d8_device;
  static int CV_d3d8_allowsoftware;
  static float CV_d3d8_gamma;
  static int CV_d3d8_rb_verts;
  static int CV_d3d8_rb_tris;
  static int CV_d3d8_decaloffset;
  static float CV_d3d8_normalmapscale;
  

//============================================================
// Renderer interface

  bool m_bInitialized;
	string m_Description;
  bool m_bFullScreen;
  int m_AlphaDepth;

  TArray<SD3DContext *> m_RContexts;
  SD3DContext *m_CurrContext;

public:
#ifndef PS2	
  virtual WIN_HWND Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen,WIN_HINSTANCE hinst, WIN_HWND Glhwnd=0, WIN_HDC Glhdc=0, WIN_HGLRC hGLrc=0, bool bReInit=false);
#else //PS2
  virtual bool Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen, bool bReInit=false);
#endif  //endif	
  virtual bool SetCurrentContext(WIN_HWND hWnd);
  virtual bool CreateContext(WIN_HWND hWnd, bool bAllowFSAA=false);
  virtual bool DeleteContext(WIN_HWND hWnd);

  virtual void  ShareResources( IRenderer *renderer );
  virtual void	MakeCurrent();

	virtual bool ChangeDisplay(unsigned int width,unsigned int height,unsigned int cbpp);
  virtual void ChangeViewport(unsigned int x,unsigned int y,unsigned int width,unsigned int height);
	virtual int	EnumDisplayFormats(TArray<SDispFormat>& Formats, bool bReset);

  virtual void  RefreshResources(int nFlags);
  virtual void BeginFrame();
  virtual void ShutDown(bool bReInit=false);
  virtual void Update(void);  
  virtual void GetMemoryUsage(ICrySizer* Sizer);
  virtual void WriteXY    (CXFont *currfont,int x,int y, float xscale,float yscale,float r,float g,float b,float a,const char *message, ...); 
  virtual void Draw2dImage(float xpos,float ypos,float w,float h,int textureid,float s0=0,float t0=0,float s1=1,float t1=1,float angle=0,float r=1,float g=1,float b=1,float a=1,float z=1);
	//! Draw a image using the current matrix
	virtual void DrawImage(float xpos,float ypos,float w,float h,int texture_id,float s0,float t0,float s1,float t1,float r,float g,float b,float a);
  virtual void Print(CXFont *currfont,float x, float y, const char *buf,float xscale,float yscale,float r,float g,float b,float a);
	virtual void SetCullMode(int mode=R_CULL_BACK);
  
  virtual void EnableBlend(bool enable);
  virtual void SetBlendMode(int mode);
  
  virtual bool EnableFog(bool enable);
  virtual void SetFog(float density, float fogstart, float fogend, const float *color, int fogmode);

  //virtual CImage *TryLoadImage(const char *szFilename) { return (NULL); }

  virtual void SetLodBias(float value);
  virtual void SelectTMU(int tnum);
  virtual void EnableTMU(bool enable);
  virtual void SetTexture(int tnum, ETexType Type=eTT_Base);
  virtual void SetEnviMode(int mode);
  virtual unsigned int DownLoadToVideoMemory(unsigned char *data,int w, int h, ETEX_Format eTFSrc, ETEX_Format eTFDst, int nummipmap, bool repeat=true, int filter=FILTER_BILINEAR, int Id=0);
  virtual	void UpdateTextureInVideoMemory(uint tnum, unsigned char *newdata,int posx,int posy,int w,int h,ETEX_Format eTF=eTF_0888){};
  virtual void RemoveTexture(unsigned int TextureId);
  virtual void RemoveTexture(ITexPic * pTexPic);
  virtual unsigned int MakeTexture(const char * filename,int *tex_type=NULL/*,unsigned int def_tid=0*/);
  virtual unsigned int LoadTexture(const char * filename,int *tex_type=NULL,unsigned int def_tid=0,bool compresstodisk=true,bool bWarn=true);

	virtual void SetCamera(const CCamera &cam);
  virtual	void SetViewport(int x=0, int y=0, int width=0, int height=0);
  virtual	void SetScissor(int x=0, int y=0, int width=0, int height=0);
  virtual void Draw3dBBox(const Vec3d &mins,const Vec3d &maxs, int nPrimType);
	virtual	void Draw3dPrim(const Vec3d &mins,const Vec3d &maxs, int nPrimType=DPRIM_WHIRE_BOX, const float* fRGBA = NULL);
  virtual void EnableTexGen(bool enable);
  virtual void SetTexgen(float scaleX, float scaleY,float translateX,float translateY);
  virtual void SetTexgen3D(float x1, float y1, float z1, float x2, float y2, float z2);
  virtual void  SetSphericalTexgen();

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
  virtual void RotateMatrix(const Vec3d & angels);
  virtual void ScaleMatrix(float x,float y,float z);
  virtual void TranslateMatrix(float x,float y,float z);
  virtual void TranslateMatrix(const Vec3d &pos);

  virtual void EnableVSync(bool enable);
  virtual void EnableDepthTest(bool enable);
  virtual void EnableDepthWrites(bool enable);
	virtual void	EnableAlphaTest(bool enable,float alphavalue=0.5f);
  virtual void DrawTriStrip(CVertexBuffer *src, int vert_num);
  virtual void ResetToDefault();
  virtual void LoadMatrix(const Matrix *src);
  virtual void MultMatrix(float * mat);
  virtual int GenerateAlphaGlowTexture(float k);

  virtual void SetMaterialColor(float r, float g, float b, float a);
  virtual void SetColorMask(unsigned char r,unsigned char g,unsigned char b,unsigned char a);
  virtual int LoadAnimatedTexture(const char * format,const int nCount);
  virtual char * GetStatusText(ERendStats type);
  virtual char * GetTexturesStatusText();
  //virtual void Project3DSprite(const Vec3d &origin,CImage *image);

  virtual void EnableLight (int id, bool enable);
  virtual void SetLightPos (int id, Vec3d &pos, bool bDirectional=0);
  virtual void SetLightDiffuse (int id, Vec3d &color);
  virtual void SetLightAmbient (int id, Vec3d &color);
  virtual void SetLightSpecular(int id, Vec3d &color);
  virtual void SetLightAttenuat(int id, float att);
  virtual void EnableLighting  (bool enable);

  virtual void SetMaterialDiffuse (Vec3d &color);
  virtual void SetMaterialAmbient (Vec3d &color);
  virtual void SetMaterialSpecular(Vec3d &color);
  virtual void ProjectToScreen(float ptx, float pty, float ptz,float *sx, float *sy, float *sz );
  virtual void Draw2dLine(float x1, float y1, float x2, float y2);
  virtual void DrawPoints(Vec3d v[], int nump, CFColor& col, int flags);
  virtual void DrawLines(Vec3d v[], int nump, CFColor& col, int flags);
	virtual void DrawLine(const Vec3d & vPos1, const Vec3d & vPos2);
  virtual void DrawLineColor(const Vec3d & vPos1, const CFColor & vColor1, const Vec3d & vPos2, const CFColor & vColor2);
  virtual void DrawBall(float x, float y, float z, float radius);
  virtual void DrawBall(const Vec3d & pos, float radius );
  virtual void DrawPoint(float x, float y, float z, float fSize);
  virtual int UnProject(float sx, float sy, float sz, float *px, float *py, float *pz, const float modelMatrix[16], const float projMatrix[16], const int    viewport[4]);
  virtual int UnProjectFromScreen( float  sx, float  sy, float  sz, float *px, float *py, float *pz);

  virtual void PrepareDepthMap(ShadowMapFrustum * lof, bool make_new_tid);
  virtual void ConfigCombinersForShadowPass();
  virtual void ConfigCombinersForHardwareShadowPass(int withTexture, float * lightDimColor);
  virtual void ConfigShadowTexgen(int Num, int rangeMap, ShadowMapFrustum * pFrustum, float * pLightFrustumMatrix, float * pLightViewMatrix);
  virtual void SetupShadowOnlyPass(int Num, ShadowMapFrustum * pFrustum, Vec3d * vShadowTrans, const float fShadowScale, Vec3d vObjTrans=Vec3d(0,0,0), float fObjScale=1.f, const Vec3d vObjAngles=Vec3d(0,0,0), Matrix44 * pObjMat=0);
//  virtual void DrawShadowGrid(const Vec3d & pos, const Vec3d & Scale, ShadowMapFrustum*lf, bool translate_projection, float alpha, IndexedVertexBuffer* pVertexBuffer, float anim_angle);
  int MakeShadowIdentityTexture();

  virtual void SetClipPlane( int id, float * params ){};

  virtual void EnableBumpCombinersAdditive(){}
  virtual void EnableCombiners(bool enable){};
  virtual int GenBumpTexUnsignedNorMap(char * bump_filename){return 0;};
  virtual void GetModelViewMatrix(float * mat);
  virtual void GetProjectionMatrix(float * mat);
  virtual void DrawObjSprites (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan);
  virtual void DrawQuad(const Vec3d &right, const Vec3d &up, const Vec3d &origin,int nFlipMode=0);
  virtual void DrawQuad(float dy,float dx, float dz, float x, float y, float z);
  virtual void DrawQuadInFogVolume(float dy,float dx, float dz, float x, float y, float z, float fFogLevel, float fFogViewDist);
  virtual void GenTexID(int num, unsigned int * ids){};

 //fog	
  void EnableFogCoordExt(bool enable){};
  void SetFogColor(float * color);
  virtual void TransformTextureMatrix(float x, float y, float angle, float scale);
  virtual void ResetTextureMatrix();

  virtual void SetPerspective(const CCamera &cam);

  virtual void ClearDepthBuffer();
  virtual void ClearColorBuffer(const Vec3d vColor);  
  virtual void ReadFrameBuffer(unsigned char * pRGB, int nSizeX, int nSizeY, bool bBackBuffer, bool bRGBA);  

  //misc	
  virtual void ScreenShot(const char *filename=NULL) {};  

  virtual uint MakeSprite(float object_scale, int tex_size, float angle, IStatObj * pStatObj, uchar * _pTmpBuffer, uint def_tid);
  virtual uint  Make3DSprite(int nTexSize, float fAngleStep, IStatObj * pStatObj);
  virtual void EnablePolygonOffset(bool enable, float factor/*1.1f*/, float units/*4.0f*/){};

  virtual void UnloadOldTextures(){};

  virtual void Set2DMode(bool enable, int ortox, int ortoy);
  virtual void PrintToScreen(float x, float y, float size, const char *buf);

  virtual int ScreenToTexture(){ return 0; };

  virtual void SetFenceCompleted(CVertexBuffer * buffer){};
  virtual void SetTexClampMode(bool clamp);

  virtual void SetDepthFunc(int mode);

  virtual void EnableStencilTest(bool enable);
  virtual void SetStencilMask(unsigned char value);
  virtual void SetStencilFunc(int func,int ref,int mask);
  virtual void SetStencilOp(int fail,int zfail,int pass);
  virtual void DrawTransparentQuad2D(float color);
  virtual void EnableAALines(bool bEnable);
  virtual void DrawCircle(float fX, float fY, float fZ, float fRadius);

	virtual	bool	SetGammaDelta(const float fGamma);

  //////////////////////////////////////////////////////////////////////
  // Replacement functions for the Font engine
  virtual	bool FontUploadTexture(class CFBitmap*);
  virtual	void FontReleaseTexture(class CFBitmap *pBmp);
  virtual void FontSetTexture(class CFBitmap*);
  virtual void FontSetRenderingState(unsigned long nVirtualScreenWidth, unsigned long nVirtualScreenHeight);
  virtual void FontSetBlending(int src, int dst);
  virtual void FontRestoreRenderingState();
  
  virtual void FontSetState(bool bRestore);
  virtual void DrawString(int x, int y,bool bIgnoreColor,const char *message, ...);
  //////////////////////////////////////////////////////////////////////

  // Shaders pipeline
  virtual void EF_Release(int nFlags);
  virtual void EF_PipelineShutdown();
  void EF_ClearBuffer(bool bForce, float *Colors);

  virtual void EF_LightMaterial(SLightMaterial *lm, bool bEnable, int Flags);

  byte *m_SysArray;
  ECull m_eCull;
  int m_EnableLights;
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
    if (!m_bMatColor)
    {
      m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2,	D3DTA_TFACTOR	);
      m_bMatColor = true;
    }
    m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR,  color.dcolor );
  }

  _inline void EF_SetGlobalColor(float r, float g, float b, float a)
  {
    if (!m_bMatColor)
    {
      m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2,	D3DTA_TFACTOR	);
      m_bMatColor = true;
    }
    UCol color;
    color.bcolor[0] = (byte)(b * 255.0f);
    color.bcolor[1] = (byte)(g * 255.0f);
    color.bcolor[2] = (byte)(r * 255.0f);
    color.bcolor[3] = (byte)(a * 255.0f);
    m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR,  color.dcolor );
  }
  _inline void EF_SetVertColor()
  {
    if (m_bMatColor)
    {
      m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE	);
      m_bMatColor = false;
    }
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
  void EF_SetColorOp(byte co);

  // Clip Planes support
  void EF_SetClipPlane (bool bEnable, float *pPlane, bool bRefract);

  void EF_Init();
  void EF_PreRender();
  void EF_PostRender();
  void EF_RenderPipeLine(void (*RenderFunc)());
  void EF_InitRandTables();
  void EF_InitWaveTables();
  void EF_InitEvalFuncs(int num);
  void EF_InitD3DFixedPipeline();
  bool EF_PreDraw();
  void EF_SetCameraInfo();
  void EF_SetObjectTransform(CCObject *obj);
  bool EF_ObjectChange(SShader *Shader, int nObject, CRendElement *pRE);

  void EF_UpdateTextures(SShaderPass *Layer);
  void EF_UpdateTextures(SShaderPassHW *Layer);
  
  void EF_DrawFogOverlay();
  void EF_DrawDetailOverlay();
  void EF_DrawDecalOverlay();

  void EF_EvalNormalsRB(SShader *ef);
  void EF_Eval_DeformVerts(TArray<SDeform>* Defs);
  void EF_Eval_RGBAGen(SShaderPass *sfm);
  void EF_Eval_TexGen(SShaderPass *sfm);

  void EF_ApplyMatrixOps(TArray<SMatrixTransform>* MatrixOps, bool bEnable);
  
  bool EF_SetLights(int Flags, bool Enable);
  void EF_SetHWLight(int Num, vec4_t Pos, CFColor& Diffuse, CFColor& Specular, float ca, float la, float qa, float fRange);

  void EF_DrawDebugLights();
  void EF_DrawDebugTools();

  static void EF_DrawWire();
  static void EF_DrawNormals();
  static void EF_DrawTangents();
  static void EF_Flush();

  _inline void EF_Draw(SShader *sh, SShaderPass *sl)
  {
    if (!CV_r_nodrawshaders)
    {
      if (m_RP.m_pRE)
        m_RP.m_pRE->mfDraw(sh, sl);
      else
        EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
    }
  }
  void EF_DrawIndexedMesh (int nPrimType);
  void EF_DrawLayersHW(SShaderTechnique *hs, SShader *ef);
  void EF_DrawLightsHW(SShaderTechnique *hs, SShader *ef, int Stage);
  void EF_FlushHW();
  void EF_SetStateShaderState();
  void EF_ResetStateShaderState();
  bool EF_SetResourcesState(bool bSet);
  void EF_FlushShader();
  int  EF_Preprocess(SRendItemPre *ri, int nums, int nume);
  void EF_DrawREPreprocess(SRendItemPreprocess *ris, int Nums);
  void EF_PipeLine(int nums, int nume, int nList, int nSortType, void (*RenderFunc)());
  
  STWarpZone *EF_SetWarpZone(SWarpSurf *sf, int *NumWarps, STWarpZone Warps[]);
  void EF_UpdateWarpZone(STWarpZone *wp, SWarpSurf *srf);
  bool EF_CalcWarpCamera(STWarpZone *wp, int nObject, CCamera& prevCam, CCamera& newCam);
  bool EF_RenderWarpZone(STWarpZone *wp);

  void EF_PrintProfileInfo();

  virtual void EF_CheckOverflow(int nVerts, int nTris, CRendElement *re);
  virtual void EF_EndEf3D (bool bSort);
  virtual void EF_EndEf2D(bool bSort);  // 2d only
  virtual void EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, int nFog, CRendElement *re);
  virtual void EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, CRendElement *re);
  virtual bool EF_SetLightHole(Vec3d vPos, Vec3d vNormal, int idTex, float fScale=1.0f, bool bAdditive=true);
  
  virtual int  EF_RegisterFogVolume(float fMaxFogDist, float fFogLayerZ, CFColor color);
  virtual void SetTexGen(byte eTC) {};
  virtual void SetState(int st);

  virtual STexPic *EF_MakePhongTexture(int Exp);

  void EF_InitFogVolumes();

  virtual WIN_HWND GetHWND() { return 0; }
    
//  virtual void ClearAlphaBuffer(float fAlphaValue) {}
};

extern CD3D8Renderer *gcpRendD3D;

class CD3D8TexMan : public CTexMan
{
protected:
  virtual STexPic *CreateTexture(const char *name, int wdt, int hgt, int depth, uint flags, uint flags2, byte *dst, ETexType eTT, float fAmount1=-1.0f, float fAmount2=-1.0f, int DXTSize=0, STexPic *ti=NULL, int bind=0, ETEX_Format eTF=eTF_8888, const char *szSourceName=NULL);
  virtual STexPic *CopyTexture(const char *name, STexPic *ti, int CubeSide=-1);

public:  
  CD3D8TexMan() : CTexMan()
  {
#ifndef _XBOX  
    m_CurPal = 1;
#endif
    m_pCurCubeTexture = NULL;
  }
  void D3DCompressTexture(int tgt, STexPicD3D *ti, int CubeSide);
  void D3DCreateVideoTexture(int tgt, byte *src, int wdt, int hgt, D3DFORMAT SrcFormat, D3DFORMAT DstFormat, STexPicD3D *ti, bool bMips, int CubeSide, PALETTEENTRY *pe, int DXTSize);
  void BuildMipsSub(byte* src, int wdt, int hgt);
  void ClearBuffer(int Width, int Height, bool bEnd, STexPic *pImage, int Side);

  void DrawCubeSide( const float *angle, Vec3d& Pos, int tex_size, int side, int RendFlags);
  void CreateBufRegion(int Width, int Height);

  void AmplifyGlare(SByteColor *glarepixels, int width, int height);
  void SmoothGlare(SByteColor *src, int src_w, int src_h, SLongColor *dst);
  void BlurGlare(SByteColor *src, int src_w, int src_h, SByteColor *dst, SLongColor *p, int boxw, int boxh);

  float CalcFogVal(float fi, float fj);
  void GenerateFlareMap();
  void GenerateFogMaps();

  static void CalcMipsAndSize(STexPic *ti);

  static BindNULL(int From)
  {
    int n = CTexMan::m_nCurStages;
    CTexMan::m_nCurStages = From;
    for (; From<n; From++)
    {
      HRESULT hr = gcpRendD3D->mfGetD3DDevice()->SetTexture(From, NULL);
      gcpRendD3D->EF_SelectTMU(From);
      gcpRendD3D->EF_SetColorOp(eCO_DISABLE);
      gcpRendD3D->mStages[From].Texture = NULL;
    }
  }

  virtual ~CD3D8TexMan();

  virtual STexPic *GetByID(int Id);
  virtual SRefTex *AddToHash(int Id, STexPic *ti);
  virtual void RemoveFromHash(int Id, STexPic *ti);
  virtual void SetTexture(int Id, ETexType eTT);

  virtual byte *GenerateDXT_HW(STexPic *ti, EImFormat eF, byte *dst, int *numMips, int *DXTSize, bool bMips);
  virtual bool SetFilter(char *filt);
  virtual void UpdateTextureData(STexPic *pic, byte *data, int USize, int VSize, bool bProc, int State, bool bPal);
  virtual STexPic *CreateTexture();
  virtual bool ScanEnvironmentCM (const char *name, int size, Vec3d& Pos);
  virtual void ScanEnvironmentCube(SEnvTexture *cm, int RendFlags);
  virtual void ScanEnvironmentTexture(SEnvTexture *cm, int RendFlags);
  virtual void EndCubeSide(CCObject *obj, bool bNeedClear);
  virtual void StartCubeSide(CCObject *obj);
  virtual void DrawToTexture(Plane& Pl, STexPic *Tex, int RendFlags);
  virtual void DrawToTextureForGlare(int Id);
  virtual void DrawToTextureForRainMap(int Id);
  virtual void StartHeatMap(int Id);
  virtual void EndHeatMap();
  virtual void StartNightMap(int Id);
  virtual void EndNightMap();
  virtual void StartMotionMap(int Id, CCObject *pObject);
  virtual void EndMotionMap(void);
  virtual void StartScreenMap(int Id);
  virtual void EndScreenMap();

  // tiago: added
  virtual void StartScreenTexMap(int Id);
  virtual void EndScreenTexMap();

  virtual void DrawFlashBangMap(int Id, int RendFlags, CREFlashBang *pRE);
  virtual void Update();
  virtual void GenerateFuncTextures();

  IDirect3DSurface8* m_pZSurf;
  LPDIRECT3DCUBETEXTURE8 m_pCurCubeTexture;
  STexPicD3D *m_CurCubeFaces[6];
  CCamera m_PrevCamera;
  int m_TempX, m_TempY, m_TempWidth, m_TempHeight;

  static int TexSize(int wdt, int hgt, int mode);

#ifndef _XBOX  
  int m_CurPal;
#endif

  static int m_Format;
  static int m_FirstBind;

  static TTextureMap m_RefTexs;
};


#endif //PS2

#endif // DRIVERD3D8_H
