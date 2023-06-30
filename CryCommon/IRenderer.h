
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Renderer.h - API Indipendent
//
//	History:
//	-Jan 31,2001:Originally created by Marco Corbetta
//  -: Taken over by Andrey Khonich
//
//////////////////////////////////////////////////////////////////////

#ifndef _IRENDERER_H
#define _IRENDERER_H

#if defined(LINUX)
	#include "Splash.h"
#else
	enum eSplashType
	{
		EST_Water,
	};
#endif


typedef HRESULT (*MIPDXTcallback)(void * data, int miplevel, DWORD size, int width, int height, void * user_data);

// Global typedefs.
//////////////////////////////////////////////////////////////////////
typedef const char*			cstr;
#if !defined(LINUX)
typedef unsigned long       DWORD;
#endif //LINUX
#ifndef BOOL
typedef int                 BOOL;
#endif
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef int                 INT;
typedef unsigned int        UINT;

#ifndef uchar
typedef unsigned char		uchar;
typedef unsigned int		uint;
typedef unsigned short	ushort;
#endif


//forward declarations.
//////////////////////////////////////////////////////////////////////
typedef void*	WIN_HWND;
typedef void*	WIN_HINSTANCE;
typedef void*	WIN_HDC;
typedef void*	WIN_HGLRC;

class		CVertexBuffer;
class		CREOcLeaf;
//class		CImage;
struct  CStatObj;
class   CStatObjInst;
struct	ShadowMapFrustum;
class		CXFont;
struct	IStatObj;
class CObjManager;
struct	ShadowMapLightSource;
struct SPrimitiveGroup;
struct  ICryCharInstance;
class		CRendElement;
struct	ShadowMapLightSourceInstance;
class		CCObject;
class		CTexMan;
//class CVar;
struct	SVrect;
struct	SColorVert2D;
struct	SColorVert;
class		CFColor;
class		CShadowVolEdge;
class		CCamera;
class		CDLight;
struct	ILog;
struct	IConsole;
struct	ITimer;
struct	ISystem;
class		IPhysicalWorld;
class   ICrySizer;

//////////////////////////////////////////////////////////////////////
typedef unsigned char bvec4[4];
typedef float vec4_t[4];
typedef unsigned char byte;
typedef float vec2_t[2];

//////////////////////////////////////////////////////////////////////
//Vladimir's list
template	<class T> class list2;

//DOC-IGNORE-BEGIN
#include "ColorDefs.h"
#include "TArray.h"

#include <IFont.h>
//DOC-IGNORE-END

//////////////////////////////////////////////////////////////////////
#define R_CULL_DISABLE  0 
#define R_CULL_NONE     0 
#define R_CULL_FRONT    1
#define R_CULL_BACK     2

//////////////////////////////////////////////////////////////////////
#define R_TEXGEN_LINEAR	1

//////////////////////////////////////////////////////////////////////
#define R_FOGMODE_LINEAR	1
#define R_FOGMODE_EXP2		2

//////////////////////////////////////////////////////////////////////
#define R_DEFAULT_LODBIAS	0

//////////////////////////////////////////////////////////////////////
#define R_PRIMV_TRIANGLES		0
#define R_PRIMV_TRIANGLE_STRIP	1
#define R_PRIMV_QUADS	        2
#define R_PRIMV_TRIANGLE_FAN  3
#define R_PRIMV_MULTI_STRIPS	4
#define R_PRIMV_MULTI_GROUPS	5

//////////////////////////////////////////////////////////////////////
#define FILTER_NONE		   -1
#define FILTER_LINEAR		  0
#define FILTER_BILINEAR		1
#define FILTER_TRILINEAR	2

//////////////////////////////////////////////////////////////////////
#define R_SOLID_MODE		1
#define R_WIREFRAME_MODE	2

#define R_GL_RENDERER	0
#define R_DX8_RENDERER	1
#define R_DX9_RENDERER	2
#define R_NULL_RENDERER	3
#define R_CUBAGL_RENDERER	4

//////////////////////////////////////////////////////////////////////
// Render features

#define RFT_MULTITEXTURE 1
#define RFT_BUMP         2
#define RFT_OCCLUSIONQUERY 4
#define RFT_PALTEXTURE   8      // Support paletted textures
#define RFT_HWGAMMA      0x10
#define RFT_ALLOWRECTTEX  0x20  // Allow non-power-of-two textures
#define RFT_COMPRESSTEXTURE  0x40
#define RFT_ALLOWANISOTROPIC 0x100  // Allows anisotropic texture filtering
#define RFT_SUPPORTZBIAS     0x200
#define RFT_HW_ENVBUMPPROJECTED 0x400 // Allows projected environment maps with EMBM
#define RFT_ALLOWSECONDCOLOR 0x800
#define RFT_DETAILTEXTURE    0x1000
#define RFT_TEXGEN_REFLECTION 0x2000
#define RFT_TEXGEN_EMBOSS     0x4000
#define RFT_OCCLUSIONTEST     0x8000 // Support hardware occlusion test

#define RFT_HW_GF2        0x10000 // GF2 class hardware (ATI Radeon 7500 as well :) )
#define RFT_HW_GF3        0x20000 // NVidia GF3 class hardware (ATI Radeon 8500 as well :) )
#define RFT_HW_RADEON     0x30000 // ATI R300 class hardware
#define RFT_HW_CUBAGL	  0x40000 // Nintendo Game-Cube
#define RFT_HW_GFFX       0x50000 // Geforce FX class hardware
#define RFT_HW_NV4X       0x60000 // NV4X class hardware
#define RFT_HW_MASK       0x70000 // Graphics chip mask
#define RFT_HW_HDR        0x80000 // Hardware supports high dynamic range rendering

#define RFT_HW_VS         0x100000   // Vertex shaders 1.1
#define RFT_HW_RC         0x200000   // Register combiners (OpenGL only)
#define RFT_HW_TS         0x400000   // Texture shaders (OpenGL only)
#define RFT_HW_PS20       0x800000   // Pixel shaders 2.0
#define RFT_HW_PS30       0x1000000  // Pixel shaders 3.0

#define RFT_FOGVP         0x2000000  // fog should be calculted in vertex shader (all NVidia cards)
#define RFT_ZLOCKABLE     0x4000000  // depth buffer can be locked for read
#define RFT_SUPPORTFSAA   0x8000000  // FSAA is supported by hardware
#define RFT_DIRECTACCESSTOVIDEOMEMORY   0x10000000
#define RFT_RGBA          0x20000000 // RGBA order (otherwise BGRA)
#define RFT_DEPTHMAPS     0x40000000 // depth maps are supported
#define RFT_SHADOWMAP_SELFSHADOW  0x80000000 // depth correct shadow maps (via PS20 z-comparing)

//====================================================================
// PrecacheResources flags

#define FPR_NEEDLIGHT     1
#define FPR_2D            2
#define FPR_IMMEDIATELLY  4


//====================================================================
// Draw shaders flags (EF_EndEf3d)

#define SHDF_ALLOWHDR 1
#define SHDF_SORT     2

//////////////////////////////////////////////////////////////////////
// Texture flags

#define FT_PROJECTED   0x1
#define FT_NOMIPS      0x2
#define FT_HASALPHA    0x4
#define FT_NORESIZE    0x8
#define FT_HDR         0x10
#define FT_UPDATE      0x20
#define FT_ALLOCATED   0x40
#define FT_BUILD       0x80

#define FT_NODOWNLOAD  0x100
#define FT_CONV_GREY   0x200
#define FT_LM          0x400
#define FT_HASDSDT     0x800

#define FT_HASNORMALMAP 0x1000
#define FT_DYNAMIC    0x2000
#define FT_NOREMOVE  0x4000
#define FT_HASMIPS   0x8000
#define FT_PALETTED  0x10000
#define FT_NOTFOUND  0x20000
#define FT_FONT     0x40000
#define FT_SKY       0x80000
#define FT_SPEC_MASK 0x7f000
#define FT_CLAMP     0x100000
#define FT_NOSTREAM  0x200000
#define FT_DXT1      0x400000
#define FT_DXT3      0x800000
#define FT_DXT5      0x1000000
#define FT_DXT       0x1c00000
#define FT_3DC       0x2000000
#define FT_3DC_A     0x4000000
#define FT_ALLOW3DC  0x8000000

#define FT_BUMP_SHIFT 27
#define FT_BUMP_MASK      0xf0000000
#define FT_BUMP_DETALPHA  0x10000000
#define FT_BUMP_DETRED    0x20000000
#define FT_BUMP_DETBLUE   0x40000000
#define FT_BUMP_DETINTENS 0x80000000

//////////////////////////////////////////////////////////////////////
#define FT2_NODXT        1
#define FT2_RENDERTARGET 2
#define FT2_FORCECUBEMAP 4
#define FT2_WASLOADED    8
#define FT2_RELOAD       0x10
#define FT2_NEEDRESTORED 0x20
#define FT2_UCLAMP       0x40
#define FT2_VCLAMP       0x80
#define FT2_RECTANGLE    0x100
#define FT2_FORCEDXT     0x200

#define FT2_BUMPHIGHRES   0x400
#define FT2_BUMPLOWRES    0x800
#define FT2_PARTIALLYLOADED  0x1000
#define FT2_NEEDTORELOAD     0x2000
#define FT2_WASUNLOADED      0x4000
#define FT2_STREAMINGINPROGRESS  0x8000

#define FT2_FILTER_BILINEAR  0x10000
#define FT2_FILTER_TRILINEAR 0x20000
#define FT2_FILTER_ANISOTROPIC 0x40000
#define FT2_FILTER_NEAREST     0x80000
#define FT2_FILTER (FT2_FILTER_BILINEAR | FT2_FILTER_TRILINEAR | FT2_FILTER_ANISOTROPIC | FT2_FILTER_NEAREST)
#define FT2_VERSIONWASCHECKED  0x100000
#define FT2_BUMPCOMPRESED      0x200000
#define FT2_BUMPINVERTED       0x400000
#define FT2_STREAMFROMDDS      0x8000000
#define FT2_DISCARDINCACHE     0x1000000
#define FT2_NOANISO            0x2000000
#define FT2_CUBEASSINGLETEXTURE  0x4000000
#define FT2_FORCEMIPS2X2         0x8000000
#define FT2_DIFFUSETEXTURE       0x10000000
#define FT2_WASFOUND             0x20000000
#define FT2_REPLICATETOALLSIDES  0x40000000
#define FT2_CHECKFORALLSEQUENCES 0x80000000

//////////////////////////////////////////////////////////////////////

// Render State flags
#define GS_BLSRC_MASK              0xf
#define GS_BLSRC_ZERO              0x1
#define GS_BLSRC_ONE               0x2
#define GS_BLSRC_DSTCOL            0x3
#define GS_BLSRC_ONEMINUSDSTCOL    0x4
#define GS_BLSRC_SRCALPHA          0x5
#define GS_BLSRC_ONEMINUSSRCALPHA  0x6
#define GS_BLSRC_DSTALPHA          0x7
#define GS_BLSRC_ONEMINUSDSTALPHA  0x8
#define GS_BLSRC_ALPHASATURATE     0x9

#define GS_BLDST_MASK              0xf0
#define GS_BLDST_ZERO              0x10
#define GS_BLDST_ONE               0x20
#define GS_BLDST_SRCCOL            0x30
#define GS_BLDST_ONEMINUSSRCCOL    0x40
#define GS_BLDST_SRCALPHA          0x50
#define GS_BLDST_ONEMINUSSRCALPHA  0x60
#define GS_BLDST_DSTALPHA          0x70
#define GS_BLDST_ONEMINUSDSTALPHA  0x80

#define GS_BUMP                    0xa0
#define GS_ENV                     0xb0

#define GS_DXT1                    0xc0
#define GS_DXT3                    0xd0
#define GS_DXT5                    0xe0

#define GS_BLEND_MASK              0xff

#define GS_DEPTHWRITE               0x00000100

#define GS_MODULATE                0x00000200
#define GS_NOCOLMASK               0x00000400
#define GS_ADDITIONALSTATE         0x00000800

#define GS_POLYLINE                0x00001000
#define GS_TEXPARAM_CLAMP          0x00002000
#define GS_TEXPARAM_UCLAMP         0x00004000
#define GS_TEXPARAM_VCLAMP         0x00008000
#define GS_COLMASKONLYALPHA        0x00010000
#define GS_NODEPTHTEST             0x00020000
#define GS_COLMASKONLYRGB          0x00040000
#define GS_DEPTHFUNC_EQUAL         0x00100000
#define GS_DEPTHFUNC_GREAT         0x00200000
#define GS_STENCIL                 0x00400000
#define GS_TEXANIM                 0x00800000

#define GS_ALPHATEST_MASK          0xf0000000
#define GS_ALPHATEST_GREATER0      0x10000000
#define GS_ALPHATEST_LESS128       0x20000000
#define GS_ALPHATEST_GEQUAL128     0x40000000
#define GS_ALPHATEST_GEQUAL64      0x80000000

//////////////////////////////////////////////////////////////////////
// Texture object interface
struct ITexPic
{
  virtual void AddRef() = 0;
  virtual void Release(int bForce=false)=0;
  virtual const char *GetName()=0;
  virtual int GetWidth() = 0;
  virtual int GetHeight() = 0;
  virtual int GetOriginalWidth() = 0;
  virtual int GetOriginalHeight() = 0;
  virtual int GetTextureID() = 0;
  virtual int GetFlags() = 0;
  virtual int GetFlags2() = 0;
  virtual void SetClamp(bool bEnable) = 0;
  virtual bool IsTextureLoaded() = 0;
  virtual void PrecacheAsynchronously(float fDist, int Flags) = 0;
  virtual void Preload (int Flags)=0;
  virtual byte *GetData32()=0;
  virtual bool SetFilter(int nFilter)=0;
};

#define	FORMAT_8_BIT	 8
#define FORMAT_24_BIT	24
#define FORMAT_32_BIT	32

//////////////////////////////////////////////////////////////////////
// Import and Export interfaces passed to the renderer
struct SCryRenderInterface
{
  class CMalloc  *igcpMalloc;

  ILog     *ipLog;
  IConsole *ipConsole;
  ITimer   *ipTimer;
  ISystem  *ipSystem;
  int      *ipTest_int;
	IPhysicalWorld *pIPhysicalWorld;
};

//////////////////////////////////////////////////////////////////////
struct tLmInfo
{
	float						fS[3],fT[3];
	unsigned short	nTextureIdLM;     // general color light map
	unsigned short	nTextureIdLM_LD;  // lights direction texture for DOT3 LM
};

//////////////////////////////////////////////////////////////////////
struct CObjFace
{
  CObjFace() { memset(this,0,sizeof(CObjFace)); }
	~CObjFace()
	{
/*		if (m_lInfo)
		{
			delete m_lInfo;
			m_lInfo=NULL;
		}*/
	}
  
  unsigned short v[3];
  unsigned short t[3];
  unsigned short n[3];
	unsigned short b[3];
  unsigned short shader_id;
	
//	tLmInfo		*m_lInfo;

//  Vec3 m_vCenter;
	
	//! tell if this surface is lit by the light (for dynamic lights)
  bool	m_bLit;
	//! plane equation for this surface (for dynamic lights)
  Plane m_Plane;  
  Vec3 m_Vecs[3];

  uchar m_dwFlags;
  float m_fArea;	
};

#define VBF_DYNAMIC 1

struct SDispFormat
{
  int m_Width;
  int m_Height;
  int m_BPP;
};

struct SAAFormat
{
  char szDescr[64];
  int nSamples;
  int nQuality;
  int nAPIType;
};

// Stream ID's
#define VSF_GENERAL  0  // General vertex buffer
#define VSF_TANGENTS 1  // Tangents buffer

#define VSF_NUM      2  // Number of vertex streams

// Stream Masks (Used during updating)
#define VSM_GENERAL  (1<<VSM_GENERAL)
#define VSM_TANGENTS (1<<VSF_TANGENTS)

union UHWBuf
{
  void *m_pPtr;
  uint m_nID;
};

struct SVertexStream
{
  void *m_VData;      // pointer to buffer data
  UHWBuf m_VertBuf;   // HW buffer descriptor 
  int m_nItems;
  bool m_bLocked;     // Used in Direct3D only
  bool m_bDynamic;
  int m_nBufOffset;
  struct SVertPool *m_pPool;
  SVertexStream()
  {
    Reset();
    m_bDynamic = false;
    m_nBufOffset = 0;
    m_pPool = NULL;
  }

  void Reset()
  {
    m_VData = NULL;
    m_VertBuf.m_pPtr = NULL;
    m_nItems = NULL;
    m_bLocked = false;
  }
};

//////////////////////////////////////////////////////////////////////
// General VertexBuffer created by CreateVertexBuffer() function
class CVertexBuffer
{
public:	
  CVertexBuffer() 
  {
    for (int i=0; i<VSF_NUM; i++)
    {
      m_VS[i].Reset();
    }
    m_fence=0;
    m_bFenceSet=0;
    m_NumVerts = 0;
    m_vertexformat = 0;
  }
  
  CVertexBuffer(void* pData, int nVertexFormat, int nVertCount=0)
  {
    for (int i=0; i<VSF_NUM; i++)
    {
      m_VS[i].m_VData = NULL;
      m_VS[i].m_VertBuf.m_pPtr = NULL;
      m_VS[i].m_bLocked = false;
    }
    m_VS[VSF_GENERAL].m_VData = pData;
    m_vertexformat = nVertexFormat;
	  m_fence=0;
	  m_bFenceSet=0;
    m_NumVerts = nVertCount;
  }
  void *GetStream(int nStream, int *nOffs);

  SVertexStream m_VS[VSF_NUM]; // 4 vertex streams and one index stream

  uint m_bFenceSet : 1;
  uint m_bDynamic : 1;
	int		m_vertexformat;
	unsigned int m_fence;
  int   m_NumVerts;
//## MM unused?	void *pPS2Buffer;

  int Size(int Flags, int nVerts);
};

enum ERendStats
{
  eRS_VidBuffer,
  eRS_ShaderPipeline,
  eRS_CurTexturesInfo,
};

//////////////////////////////////////////////////////////////////////
/*struct IndexedVertexBuffer
{
  list2<unsigned short> indices;
  CVertexBuffer * pVertexBuffer;
  int strip_step;
  Vec3 vBoxMin,vBoxMax;
};*/

//////////////////////////////////////////////////////////////////////
//DOC-IGNORE-BEGIN
#include "IShader.h"
//DOC-IGNORE-END

enum EImFormat
{
  eIF_Unknown = 0,
  eIF_Pcx,
  eIF_Tga,
  eIF_Jpg,
  eIF_Gif,
  eIF_Tif,
  eIF_Bmp,
  eIF_Lbm,
  eIF_DXT1,
  eIF_DXT3,
  eIF_DXT5,
  eIF_DDS_LUMINANCE,
  eIF_DDS_RGB8,
  eIF_DDS_SIGNED_RGB8,
  eIF_DDS_SIGNED_HILO8,
  eIF_DDS_SIGNED_HILO16,
  eIF_DDS_RGBA8,
  eIF_DDS_DSDT,
  eIF_DDS_RGBA4,
};


// Flags passed in function FreeResources
#define FRR_SHADERS   1
#define FRR_SHADERTEXTURES 2
#define FRR_TEXTURES  4
#define FRR_SYSTEM    8
#define FRR_RESTORE   0x10
#define FRR_REINITHW  0x20
#define FRR_ALL      -1

// Refresh render resources flags
// Flags passed in function RefreshResources
#define FRO_SHADERS  1
#define FRO_SHADERTEXTURES  2
#define FRO_TEXTURES 4
#define FRO_GEOMETRY 8
#define FRO_FORCERELOAD 0x10


// LoadAnimatedTexture returns pointer to this structure
struct AnimTexInfo
{
  AnimTexInfo() { memset(this,0,sizeof(AnimTexInfo)); }
  char sName[256];
  int * pBindIds;
  int nFramesCount;
	int nRefCounter;
};

#ifdef PS2
typedef struct __HDC* HDC;
typedef struct __HGLRC* HGLRC;
#endif

/*
// The following may be (and sometimes is) used for testing. Please don't remove.
struct IRenderer;
class IRendererCallbackServer;

// Renderer callback client.
// Derive your class (a renderer client) from this one in order to register its instances
// with the renderer.
class IRendererCallbackClient
{
public:
	// within this function, you can render your stuff and it won't flicker in the viewport
	virtual void OnRenderer_BeforeEndFrame () {}
	// unregisters itsef from the server
	virtual void Renderer_SelfUnregister() {}
};

// renderer callback server.
// Derive your renderer (server) from this one in order for it to be able to serve
// renderer callback client requests
class IRendererCallbackServer
{
public:
	// registers the given client  - do nothing by default
	virtual void RegisterCallbackClient (IRendererCallbackClient* pClient) {}
	// unregisters the given client - do nothing by default
	virtual void UnregisterCallbackClient (IRendererCallbackClient* pClient) {}
};
*/

// Draw3dBBox PrimType params
#define DPRIM_WHIRE_BOX			0
#define DPRIM_LINE					1
#define DPRIM_SOLID_BOX			2
#define DPRIM_WHIRE_SPHERE	3
#define DPRIM_SOLID_SPHERE	4

enum EBufferType
{
	eBT_Static = 0,
	eBT_Dynamic,
};

//! Flags used in DrawText function.
//! @see SDrawTextInfo
enum EDrawTextFlags
{
	//! Text must be fixed pixel size.
	eDrawText_FixedSize = 0x01,
};

//////////////////////////////////////////////////////////////////////////
//! This structure used in DrawText method of renderer.
//! It provide all necesarry information of how to render text on screen.
//! @see IRenderer::Draw2dText
struct SDrawTextInfo
{
	//! One of EDrawTextFlags flags.
	//! @see EDrawTextFlags
	int			flags;
	//! Text color, (r,g,b,a) all members must be specified.
	float		color[4];
	float xscale;
	float yscale;
	CXFont*	xfont;

	SDrawTextInfo()
	{
		flags = 0;
		color[0] = color[1] = color[2] = color[3] = 1;
		xfont = 0;
	}
};

#define MAX_FRAME_ID_STEP_PER_FRAME 8

//////////////////////////////////////////////////////////////////////
struct IRenderer//: public IRendererCallbackServer
{
	//! Init the renderer, params are self-explanatory
  virtual WIN_HWND Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen,WIN_HINSTANCE hinst, WIN_HWND Glhwnd=0, WIN_HDC Glhdc=0, WIN_HGLRC hGLrc=0, bool bReInit=false)=0;

  virtual bool SetCurrentContext(WIN_HWND hWnd)=0;
  virtual bool CreateContext(WIN_HWND hWnd, bool bAllowFSAA=false)=0;
  virtual bool DeleteContext(WIN_HWND hWnd)=0;

  virtual int GetFeatures()=0;
  virtual int GetMaxTextureMemory()=0;

	//! Shut down the renderer
	virtual void	ShutDown(bool bReInit=false)=0;

	//! Return all supported by video card video formats (except low resolution formats)
	virtual int	EnumDisplayFormats(TArray<SDispFormat>& Formats, bool bReset)=0;

  //! Changes resolution of the window/device (doen't require to reload the level
  virtual bool	ChangeResolution(int nNewWidth, int nNewHeight, int nNewColDepth, int nNewRefreshHZ, bool bFullScreen)=0;

	//! Shut down the renderer
	virtual void	Release()=0;

	//! Free the allocated resources
  virtual void  FreeResources(int nFlags)=0;

  //! Refresh/Reload the allocated resources
  virtual void  RefreshResources(int nFlags)=0;

  //! Should be called before loading of the level
  virtual void	PreLoad (void)=0;

  //! Should be called after loading of the level
  virtual void	PostLoad (void)=0;

	//! Should be called at the beginning of every frame
	virtual void	BeginFrame	(void)=0;

	//! Should be called at the end of every frame
	virtual void	Update		(void)=0;	
	
	//! This renderer will share resources (textures) with specified renderer.
	//! Specified renderer must be of same type as this renderer.
	virtual void  ShareResources( IRenderer *renderer )=0;

  virtual	void	GetViewport(int *x, int *y, int *width, int *height)=0;
  virtual	void	SetViewport(int x=0, int y=0, int width=0, int height=0)=0;
  virtual	void	SetScissor(int x=0, int y=0, int width=0, int height=0)=0;

	//! Make this renderer current renderer.
	//! Only relevant for OpenGL ignored of DX, used by Editors.
	virtual void	MakeCurrent() = 0;

	//! Draw a triangle strip
	virtual void  DrawTriStrip(CVertexBuffer *src, int vert_num=4)=0;

  virtual void *GetDynVBPtr(int nVerts, int &nOffs, int Pool) = 0;
  virtual void DrawDynVB(int nOffs, int Pool, int nVerts) = 0;
  virtual void DrawDynVB(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pBuf, ushort *pInds, int nVerts, int nInds, int nPrimType) = 0;

	//! append fence to the end of rendering stream
	virtual void SetFenceCompleted(CVertexBuffer * buffer)=0;

	//! Create a vertex buffer
	virtual	CVertexBuffer	*CreateBuffer(int  vertexcount,int vertexformat, const char *szSource, bool bDynamic=false)=0;

	//! Release a vertex buffer
	virtual void	ReleaseBuffer(CVertexBuffer *bufptr)=0;

	//! Draw a vertex buffer
	virtual void	DrawBuffer(CVertexBuffer *src,SVertexStream *indicies,int numindices, int offsindex, int prmode,int vert_start=0,int vert_stop=0, CMatInfo *mi=NULL)=0;

	//! Update a vertex buffer
	virtual void	UpdateBuffer(CVertexBuffer *dest,const void *src,int vertexcount, bool bUnLock, int nOffs=0, int Type=0)=0;

  virtual void  CreateIndexBuffer(SVertexStream *dest,const void *src,int indexcount)=0;
  //! Update indicies 
  virtual void  UpdateIndexBuffer(SVertexStream *dest,const void *src, int indexcount, bool bUnLock=true)=0;
  virtual void  ReleaseIndexBuffer(SVertexStream *dest)=0;

	//! Check for an error in the current frame
	virtual	void	CheckError(const char *comment)=0;

	//! Draw a bbox specified by mins/maxs (debug puprposes)
	virtual	void	Draw3dBBox(const Vec3 &mins,const Vec3 &maxs, int nPrimType=DPRIM_WHIRE_BOX)=0;

	//! Draw a primitive specified by min/max vertex (for debug purposes)
	//! because of legacy code, the default implementation calls Draw3dBBox.
	//! in the newly changed renderer implementations, this will be the principal function and Draw3dBBox will eventually only draw 3dbboxes
	virtual	void	Draw3dPrim(const Vec3 &mins,const Vec3 &maxs, int nPrimType=DPRIM_WHIRE_BOX, const float* fRGBA = NULL)
	{
		// default implementaiton ignores color
		Draw3dBBox(mins, maxs,nPrimType);
	}

	//! Set the renderer camera
	virtual	void	SetCamera(const CCamera &cam)=0;

	//! Get the renderer camera
	virtual	const CCamera& GetCamera()=0;

	//! Set delta gamma
	virtual	bool	SetGammaDelta(const float fGamma)=0;

	//! Change display size
	virtual bool	ChangeDisplay(unsigned int width,unsigned int height,unsigned int cbpp)=0;

	//! Chenge viewport size
  virtual void  ChangeViewport(unsigned int x,unsigned int y,unsigned int width,unsigned int height)=0;

	//! Save source data to a Tga file (NOTE: Should not be here)	
	virtual	bool	SaveTga(unsigned char *sourcedata,int sourceformat,int w,int h,const char *filename,bool flip)=0;

	//! Set the current binded texture
	virtual	void	SetTexture(int tnum, ETexType Type=eTT_Base)=0;	

	//! Set the white texture
	virtual	void	SetWhiteTexture()=0;	

	//! Write a message on the screen
	virtual void	WriteXY(CXFont *currfont,int x,int y, float xscale,float yscale,float r,float g,float b,float a,const char *message, ...)=0;	
	//! Write a message on the screen with additional flags.
	//! for flags @see 
	virtual void	Draw2dText( float posX,float posY,const char *szText,SDrawTextInfo &info )=0;

	//! Draw a 2d image on the screen (Hud etc.)
  virtual void	Draw2dImage	(float xpos,float ypos,float w,float h,int texture_id,float s0=0,float t0=0,float s1=1,float t1=1,float angle=0,float r=1,float g=1,float b=1,float a=1,float z=1)=0;

	//! Draw a image using the current matrix
	virtual void DrawImage(float xpos,float ypos,float w,float h,int texture_id,float s0,float t0,float s1,float t1,float r,float g,float b,float a)=0;

	//! Set the polygon mode (wireframe, solid)
	virtual int	SetPolygonMode(int mode)=0;

	//! Get screen width
	virtual	int		GetWidth() = 0;

	//! Get screen height
	virtual	int		GetHeight() = 0;

	//! Memory status information
	virtual void GetMemoryUsage(ICrySizer* Sizer)=0;

	//! Get a screenshot and save to a file
	virtual void ScreenShot(const char *filename=NULL)=0;

	//! Get current bpp
  virtual int	GetColorBpp()=0;

	//! Get current z-buffer depth
  virtual int	GetDepthBpp()=0;

	//! Get current stencil bits
  virtual int	GetStencilBpp()=0;

  //! Project to screen
  virtual void ProjectToScreen( float ptx, float pty, float ptz, 
                                float *sx, float *sy, float *sz )=0;

	//! Unproject to screen
  virtual int UnProject(float sx, float sy, float sz, 
                float *px, float *py, float *pz,
                const float modelMatrix[16], 
                const float projMatrix[16], 
                const int    viewport[4])=0;

	//! Unproject from screen
  virtual int UnProjectFromScreen( float  sx, float  sy, float  sz, 
                           float *px, float *py, float *pz)=0;

  //! for editor
  virtual void  GetModelViewMatrix(float *mat)=0;

	//! for editor
  virtual void  GetModelViewMatrix(double *mat)=0;

	//! for editor
  virtual void  GetProjectionMatrix(double *mat)=0;

	//! for editor
  virtual void  GetProjectionMatrix(float *mat)=0;

	//! for editor
  virtual Vec3 GetUnProject(const Vec3 &WindowCoords,const CCamera &cam)=0;

  virtual void RenderToViewport(const CCamera &cam, float x, float y, float width, float height)=0;

  virtual void WriteDDS(byte *dat, int wdt, int hgt, int Size, const char *name, EImFormat eF, int NumMips)=0;
	virtual void WriteTGA(byte *dat, int wdt, int hgt, const char *name, int bits)=0;
	virtual void WriteJPG(byte *dat, int wdt, int hgt, char *name)=0;

	/////////////////////////////////////////////////////////////////////////////////
	//Replacement functions for Font

	virtual	bool FontUploadTexture(class CFBitmap*, ETEX_Format eTF=eTF_8888)=0;
	virtual	int  FontCreateTexture(int Width, int Height, byte *pData, ETEX_Format eTF=eTF_8888)=0;
  virtual	bool FontUpdateTexture(int nTexId, int X, int Y, int USize, int VSize, byte *pData)=0;
	virtual	void FontReleaseTexture(class CFBitmap *pBmp)=0;
	virtual void FontSetTexture(class CFBitmap*, int nFilterMode)=0;
  virtual void FontSetTexture(int nTexId, int nFilterMode)=0;
	virtual void FontSetRenderingState(unsigned long nVirtualScreenWidth, unsigned long nVirtualScreenHeight)=0;
	virtual void FontSetBlending(int src, int dst)=0;
	virtual void FontRestoreRenderingState()=0;

  /////////////////////////////////////////////////////////////////////////////////


  /////////////////////////////////////////////////////////////////////////////////
  // external interface for shaders
  /////////////////////////////////////////////////////////////////////////////////

  virtual bool EF_PrecacheResource(IShader *pSH, float fDist, float fTimeToReady, int Flags)=0;
  virtual bool EF_PrecacheResource(ITexPic *pTP, float fDist, float fTimeToReady, int Flags)=0;
  virtual bool EF_PrecacheResource(CLeafBuffer *pPB, float fDist, float fTimeToReady, int Flags)=0;
  virtual bool EF_PrecacheResource(CDLight *pLS, float fDist, float fTimeToReady, int Flags)=0;

  virtual void EF_EnableHeatVision(bool bEnable)=0;
  virtual bool EF_GetHeatVision()=0;

  virtual void EF_PolygonOffset(bool bEnable, float fFactor, float fUnits)=0;

  // Add 3D polygon to the list
  virtual void EF_AddPolyToScene3D(int Ef, int numPts, SColorVert *verts, CCObject *obj=NULL, int nFogID=0)=0;

  // Add Sprite to the list
  virtual CCObject *EF_AddSpriteToScene(int Ef, int numPts, SColorVert *verts, CCObject *obj, byte *inds=NULL, int ninds=0, int nFogID=0)=0;

  // Add 2D polygon to the list
  virtual void EF_AddPolyToScene2D(int Ef, int numPts, SColorVert2D *verts)=0;
  virtual void EF_AddPolyToScene2D(SShaderItem si, int nTempl, int numPts, SColorVert2D *verts)=0;

  /////////////////////////////////////////////////////////////////////////////////
  // Shaders/Shaders management /////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////
  // Load shader for name (name)
  virtual IShader *EF_LoadShader (const char *name, EShClass Class, int flags=0, uint64 nMaskGen=0)=0;
  // Load shader item for name (name)
  virtual SShaderItem EF_LoadShaderItem (const char *name, EShClass Class, bool bShare, const char *templName, int flags=0, SInputShaderResources *Res=NULL, uint64 nMaskGen=0)=0;
  // reload file
  virtual bool					EF_ReloadFile (const char *szFileName)=0;
  // Reinit all shader files (build hash tables)
  virtual void					EF_ReloadShaderFiles (int nCategory)=0;
  // Reload all texturer files
  virtual void					EF_ReloadTextures ()=0;
  // Create new shader as copy of (ef)
  virtual IShader			*EF_CopyShader(IShader *ef)=0;
  // Get texture object by ID
  virtual ITexPic *EF_GetTextureByID(int Id)=0;
  // Loading of the texture for name(nameTex)
  virtual ITexPic			*EF_LoadTexture(const char* nameTex, uint flags, uint flags2, byte eTT, float fAmount1=-1.0f, float fAmount2=-1.0f, int Id=-1, int BindId=0)=0;
  // Load lightmap for name (name)
  virtual int			    EF_LoadLightmap (const char *name)=0;
  virtual bool			    EF_ScanEnvironmentCM (const char *name, int size, Vec3& Pos)=0;
  // Function used for loading animated texture from specified folder
  virtual int						EF_ReadAllImgFiles(IShader *ef, SShaderTexUnit *tl, STexAnim *ta, char *name)=0;
  // Return texture procedure for name (name)
  virtual char					**EF_GetShadersForFile(const char *File, int num)=0;
  // Return Light. Material properties for Name (Str). Materials descriptions - in shader file LightMaterials.ses
  virtual SLightMaterial *EF_GetLightMaterial(char *Str)=0;
  // Register new user defined template
  virtual bool					EF_RegisterTemplate(int nTemplId, char *Name, bool bReplace)=0;

  // Create splash
  virtual void			EF_AddSplash (Vec3 Pos, eSplashType eST, float fForce, int Id=-1)=0;

  // Hide shader template (exclude from list)
  virtual bool					EF_HideTemplate(const char *name)=0;
  // UnHide shader template (include in list)
  virtual bool					EF_UnhideTemplate(const char *name)=0;
  // UnHide all shader templates (include in list)
  virtual bool					EF_UnhideAllTemplates()=0;

  virtual bool EF_SetLightHole(Vec3 vPos, Vec3 vNormal, int idTex, float fScale=1.0f, bool bAdditive=true)=0;

  // Create new RE (RenderElement) of type (edt)
  virtual CRendElement *EF_CreateRE (EDataType edt)=0;

  // Begin using shaders (return first index for allow recursions)
  virtual void EF_StartEf ()=0;

  // Get CCObject for RE transformation
  virtual CCObject *EF_GetObject (bool bTemp=false, int num=-1)=0;

  // Add shader to the list
  virtual void EF_AddEf (int NumFog, CRendElement *re, IShader *ef, SRenderShaderResources *sr,  CCObject *obj, int nTempl, IShader *efState=0, int nSort=0)=0;

  // Draw all shaded REs in the list
  virtual void EF_EndEf3D (int nFlags)=0;

  // Dynamic lights
  virtual bool EF_IsFakeDLight (CDLight *Source)=0;
  virtual void EF_ADDDlight(CDLight *Source)=0;
  virtual void EF_ClearLightsList()=0;
  virtual bool EF_UpdateDLight(CDLight *pDL)=0;

  /////////////////////////////////////////////////////////////////////////////////
  // 2d interface for the shaders
  /////////////////////////////////////////////////////////////////////////////////
  virtual void EF_EndEf2D(bool bSort)=0;

  // Add new shader to the list
  virtual bool EF_DrawEfForName(char *name, float x, float y, float width, float height, CFColor& col, int nTempl=-1)=0;
  virtual bool EF_DrawEfForNum(int num, float x, float y, float width, float height, CFColor& col, int nTempl=-1)=0;
  virtual bool EF_DrawEf(IShader *ef, float x, float y, float width, float height, CFColor& col, int nTempl=-1)=0;
  virtual bool EF_DrawEf(SShaderItem si, float x, float y, float width, float height, CFColor& col, int nTempl=-1)=0;

  // Add new shader as part of texture to the list
  virtual bool EF_DrawPartialEfForName(char *name, SVrect *vr, SVrect *pr, CFColor& col)=0;
  virtual bool EF_DrawPartialEfForNum(int num, SVrect *vr, SVrect *pr, CFColor& col)=0;
  virtual bool EF_DrawPartialEf(IShader *ef, SVrect *vr, SVrect *pr, CFColor& col, float iwdt=0, float ihgt=0)=0;

  // Return different common shader parameters (used in ShaderBrowser) CryIndEd.exe
  virtual void *EF_Query(int Query, int Param=0)=0;
  // Construct effector (optimize) after parsing
  virtual void EF_ConstructEf(IShader *Ef)=0;
  // Setting of the global world color (use in shaders pipeline)
  virtual void EF_SetWorldColor(float r, float g, float b, float a=1.0f)=0;
  //! Register fog volume for layered fog
  virtual int  EF_RegisterFogVolume(float fMaxFogDist, float fFogLayerZ, CFColor color, int nIndex=-1, bool bCaustics=false) = 0;

  // for stats
	virtual int  GetPolyCount()=0;
  virtual void GetPolyCount(int &nPolygons,int &nShadowVolPolys)=0;

  // 3d engine set this color to fog color
  virtual void SetClearColor(const Vec3 & vColor)=0;

  // create/delete LeafBuffer object
  virtual CLeafBuffer * CreateLeafBuffer(bool bDynamic, const char *szSource="Unknown", class CIndexedMesh * pIndexedMesh=0)=0;
  
  virtual CLeafBuffer * CreateLeafBufferInitialized(
    void * pVertBuffer, int nVertCount, int nVertFormat, 
    ushort* pIndices, int nIndices,
    int nPrimetiveType, const char *szSource, EBufferType eBufType = eBT_Dynamic,
    int nMatInfoCount=1, int nClientTextureBindID=0,
    bool (*PrepareBufferCallback)(CLeafBuffer *, bool)=NULL,
    void *CustomData=NULL,
    bool bOnlyVideoBuffer=false, bool bPrecache=true)=0;

  virtual void DeleteLeafBuffer(CLeafBuffer * pLBuffer)=0;
  virtual int GetFrameID(bool bIncludeRecursiveCalls=true)=0;

  virtual void MakeMatrix(const Vec3 & pos, const Vec3 & angles,const Vec3 & scale, Matrix44* mat)=0;

//////////////////////////////////////////////////////////////////////	
	/*!	Draw an image on the screen as a label text
			@param vPos:	3d position
			@param fSize: size of the image
			@nTextureId:	Texture Id dell'immagine
	*/
	virtual void DrawLabelImage(const Vec3 &vPos,float fSize,int nTextureId)=0;

  virtual void DrawLabel(Vec3 pos, float font_size, const char * label_text, ...)=0;
  virtual void DrawLabelEx(Vec3 pos, float font_size, float * pfColor, bool bFixedSize, bool bCenter, const char * label_text, ...)=0;
	virtual void Draw2dLabel( float x,float y, float font_size, float * pfColor, bool bCenter, const char * label_text, ...)=0;	

//////////////////////////////////////////////////////////////////////	

	virtual float ScaleCoordX(float value)=0;
	virtual float ScaleCoordY(float value)=0;

	virtual void	SetState(int State)=0;

	virtual void	SetCullMode	(int mode=R_CULL_BACK)=0;
	virtual bool	EnableFog	(bool enable)=0;
	virtual void	SetFog		(float density,float fogstart,float fogend,const float *color,int fogmode)=0;
	virtual void	EnableTexGen(bool enable)=0; 
	virtual void	SetTexgen	(float scaleX,float scaleY,float translateX=0,float translateY=0)=0;
  virtual void  SetTexgen3D(float x1, float y1, float z1, float x2, float y2, float z2)=0;
	virtual void	SetLodBias	(float value=R_DEFAULT_LODBIAS)=0;
  virtual void  SetColorOp(byte eCo, byte eAo, byte eCa, byte eAa)=0;

	//! NOTE: the following functions will be removed.
	virtual	void	EnableVSync(bool enable)=0;
	virtual void	PushMatrix()=0;
	virtual void	RotateMatrix(float a,float x,float y,float z)=0;
  virtual void	RotateMatrix(const Vec3 & angels)=0;
	virtual void	TranslateMatrix(float x,float y,float z)=0;
	virtual void	ScaleMatrix(float x,float y,float z)=0;
	virtual void	TranslateMatrix(const Vec3 &pos)=0;
  virtual void  MultMatrix(float * mat)=0;
	virtual	void	LoadMatrix(const Matrix44 *src=0)=0;
	virtual void	PopMatrix()=0;
	virtual	void	EnableTMU(bool enable)=0;
	virtual void	SelectTMU(int tnum)=0;	  	
	virtual	unsigned int DownLoadToVideoMemory(unsigned char *data,int w, int h, ETEX_Format eTFSrc, ETEX_Format eTFDst, int nummipmap, bool repeat=true, int filter=FILTER_BILINEAR, int Id=0, char *szCacheName=NULL, int flags=0)=0;
  virtual	void UpdateTextureInVideoMemory(uint tnum, unsigned char *newdata,int posx,int posy,int w,int h,ETEX_Format eTFSrc=eTF_0888)=0;	
	virtual unsigned int LoadTexture(const char * filename,int *tex_type=NULL,unsigned int def_tid=0,bool compresstodisk=true,bool bWarn=true)=0;
  virtual bool DXTCompress( byte *raw_data,int nWidth,int nHeight,ETEX_Format eTF, bool bUseHW, bool bGenMips, int nSrcBytesPerPix, MIPDXTcallback callback=0)=0;
  virtual bool DXTDecompress(byte *srcData,byte *dstData, int nWidth,int nHeight,ETEX_Format eSrcTF, bool bUseHW, int nDstBytesPerPix)=0;
	virtual	void	RemoveTexture(unsigned int TextureId)=0;
  virtual	void	RemoveTexture(ITexPic * pTexPic) = 0;	
  virtual void TextToScreen(float x, float y, const char * format, ...)=0;
  virtual void TextToScreenColor(int x, int y, float r, float g, float b, float a, const char * format, ...)=0;
  virtual void ResetToDefault()=0;
  virtual int  GenerateAlphaGlowTexture(float k)=0;
	virtual void SetMaterialColor(float r, float g, float b, float a)=0;	
	virtual int  LoadAnimatedTexture(const char * format,const int nCount)=0;  
	virtual void RemoveAnimatedTexture(AnimTexInfo * pInfo)=0;
  virtual AnimTexInfo * GetAnimTexInfoFromId(int nId)=0;
	virtual void	Draw2dLine	(float x1,float y1,float x2,float y2)=0;
//  virtual void DrawLine(float * p1, float * p2)=0;
  virtual void SetLineWidth(float fWidth)=0;
  virtual void DrawLine(const Vec3 & vPos1, const Vec3 & vPos2)=0;
  virtual void DrawLineColor(const Vec3 & vPos1, const CFColor & vColor1, const Vec3 & vPos2, const CFColor & vColor2)=0;
  virtual void Graph(byte *g, int x, int y, int wdt, int hgt, int nC, int type, char *text, CFColor& color, float fScale)=0;
  virtual void DrawBall(float x, float y, float z, float radius)=0;
  virtual void DrawBall(const Vec3 & pos, float radius )=0;
  virtual void DrawPoint(float x, float y, float z, float fSize = 0.0f)=0;
  virtual void FlushTextMessages()=0;  
  virtual void DrawObjSprites(list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan)=0;
  virtual void DrawQuad(const Vec3 &right, const Vec3 &up, const Vec3 &origin,int nFlipMode=0)=0;
  virtual void DrawQuad(float dy,float dx, float dz, float x, float y, float z)=0;
  virtual void ClearDepthBuffer()=0;  
  virtual void ClearColorBuffer(const Vec3 vColor)=0;  
  virtual void ReadFrameBuffer(unsigned char * pRGB, int nSizeX, int nSizeY, bool bBackBuffer, bool bRGBA, int nScaledX=-1, int nScaledY=-1)=0;  
  virtual void SetFogColor(float * color)=0;
  virtual void TransformTextureMatrix(float x, float y, float angle, float scale)=0;
  virtual void ResetTextureMatrix()=0;
  virtual char	GetType()=0;
  virtual char*	GetVertexProfile(bool bSupportedProfile)=0;
  virtual char*	GetPixelProfile(bool bSupportedProfile)=0;
  virtual void	SetType(char type)=0;	
  virtual unsigned int  MakeSprite(float object_scale, int tex_size, float angle, IStatObj * pStatObj, uchar * pTmpBuffer, uint def_tid)=0;
  virtual unsigned int  Make3DSprite(int nTexSize, float fAngleStep, IStatObj * pStatObj)=0;
  virtual ShadowMapFrustum * MakeShadowMapFrustum(ShadowMapFrustum * lof, ShadowMapLightSource * pLs, const Vec3 & obj_pos, list2<IStatObj*> * pStatObjects, int shadow_type)=0;
  virtual void Set2DMode(bool enable, int ortox, int ortoy)=0;
  virtual int ScreenToTexture()=0;  
  virtual void SetTexClampMode(bool clamp)=0;  
	virtual	void EnableSwapBuffers(bool bEnable) = 0;
  virtual WIN_HWND GetHWND() = 0;
	virtual void OnEntityDeleted(IEntityRender * pEntityRender)=0;
	virtual void SetGlobalShaderTemplateId(int nTemplateId) = 0;
	virtual int GetGlobalShaderTemplateId() = 0;

  //! Return all supported by video card video AA formats
  virtual int	EnumAAFormats(TArray<SAAFormat>& Formats, bool bReset)=0;
  virtual int  CreateRenderTarget (int nWidth, int nHeight, ETEX_Format eTF)=0;
  virtual bool DestroyRenderTarget (int nHandle)=0;
  virtual bool SetRenderTarget (int nHandle)=0;
  virtual float EF_GetWaterZElevation(float fX, float fY)=0;
};


// Query types for CryInd editor (used in EF_Query() function)
#define EFQ_NUMEFS    0
#define EFQ_LOADEDEFS 1
#define EFQ_NUMTEXTURES 2
#define EFQ_LOADEDTEXTURES 3
#define EFQ_NUMEFFILES0 6
#define EFQ_NUMEFFILES1 7
#define EFQ_EFFILENAMES0 12
#define EFQ_EFFILENAMES1 13
#define EFQ_VProgramms 16
#define EFQ_PShaders   17
#define EFQ_LightSource 18
#define EFQ_RecurseLevel 19
#define EFQ_Pointer2FrameID 20
#define EFQ_RegisteredTemplates 21
#define EFQ_NumRenderItems 22
#define EFQ_DeviceLost     23
#define EFQ_CubeColor      24
#define EFQ_D3DDevice      25
#define EFQ_glReadPixels   26

#define EFQ_Orients               33
#define EFQ_NumOrients            34
#define EFQ_SkyShader           35
#define EFQ_SunFlares             36
#define EFQ_CurSunFlare           37
#define EFQ_Materials             38
#define EFQ_LightMaterials        39

//////////////////////////////////////////////////////////////////////

#define STRIPTYPE_NONE           0
#define STRIPTYPE_ONLYLISTS      1
#define STRIPTYPE_SINGLESTRIP    2
#define STRIPTYPE_MULTIPLESTRIPS 3
#define STRIPTYPE_DEFAULT        4

/////////////////////////////////////////////////////////////////////

//DOC-IGNORE-BEGIN
#include "VertexFormats.h" 
#include "LeafBuffer.h" 
//DOC-IGNORE-END

// this structure used to pass render parameters to Render() functions of IStatObj and ICharInstance
struct SRendParams
{
  SRendParams()
  {
    memset(this, 0, sizeof(SRendParams));
    nShaderTemplate = -2;
    fScale = 1.f;
    vColor(1.f,1.f,1.f);
		fAlpha = 1.f;
		fSQDistance = -1.f;
  }

	SRendParams (const SRendParams& rThat)
	{
		memcpy (this, &rThat, sizeof(SRendParams));
	}

	//! position of render elements
  Vec3				vPos;
	//! scale of render elements
  float				fScale;
	//! angles of the object
  Vec3				vAngles;
	//! object transformations
  Matrix44		*pMatrix;
  //! custom offset for sorting by distance
  float				fCustomSortOffset;
	//! shader template to use
  int					nShaderTemplate;
	//! light mask to specifiy which light to use on the object
  unsigned int nDLightMask;
	//! strongest light affecting the object
	unsigned int nStrongestDLightMask;
	//! fog volume id
  int					nFogVolumeID;
	//! amount of bending animations for vegetations
  float				fBending;
	//! state shader
  IShader			*pStateShader;
	//! list of shadow map casters
  list2<ShadowMapLightSourceInstance> * pShadowMapCasters;
	//! object color
	Vec3     vColor;
	//! object alpha
  float     fAlpha;
	//! force a sort value for render elements
	int				nSortValue;
	//! Ambient color for the object
	Vec3			vAmbientColor;
	//! distance from camera
  float     fDistance;
	//! CCObject flags
  int		    dwFObjFlags;
	//! light source for shadow volume calculations
	CDLight		*pShadowVolumeLightSource;
  //! reference to entity, allows to improve handling of shadow volumes of IStatObj instances
  struct IEntityRender	* pCaller;
	//! Heat Amount for heat vision
	float			fHeatAmount;
	//! define size of shadow volume
	float			fShadowVolumeExtent;
	//! lightmap informaion
	struct RenderLMData * pLightMapInfo;
	struct CLeafBuffer * pLMTCBuffer; // Object instance specific tex LM texture coords;
	byte arrOcclusionLightIds[4];
	//! Override material.
	IMatInfo *pMaterial;
  //! Scissor settings for this object
//  int nScissorX1, nScissorY1, nScissorX2, nScissorY2;
	//! custom shader params
	TArray <struct SShaderParam> * pShaderParams;
	//! squared distance to the center of object
	float fSQDistance;
  //! CCObject custom data
  void * pCCObjCustomData;
};

//! holds shadow volume informations for static objects
//////////////////////////////////////////////////////////////////////
struct ItShadowVolume
{
	virtual void	Release()=0;	
	virtual Vec3	GetPos()=0;
	virtual void	SetPos(const Vec3 &vPos)=0;
	virtual	class CShadowVolObject	*GetShadowVolume()=0;
	virtual	void	SetShadowVolume(CShadowVolObject *psvObj)=0;

	//! /param lSource lightsource, worldspace and objectspace position is used
	//! /param inArea pointer to the area whre the object is in (could be 0 - but shadow extrusion is set to maximum)
  virtual	void	RebuildShadowVolumeBuffer( const CDLight &lSource, float fExtent )=0;

	//! return memory usage
	virtual	int GetMemoryUsage(){ return 0; }; //todo: implement

  //! this buffer will contain vertices after RebuildDynamicShadowVolumeBuffer() calll
  virtual	Vec3 * GetSysVertBufer() = 0;
  virtual	void CheckUnload() {};
};

#endif //rende