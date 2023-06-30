
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Renderer.h - API Indipendent
//
//	History:
//	-Jan 31,2001:Originally Created by Marco Corbetta
//	-: taken over by Andrey Khonich
//
//////////////////////////////////////////////////////////////////////

#ifndef _RENDERER_H
#define _RENDERER_H

#if _MSC_VER > 1000
# pragma once
#endif

#ifdef PS2
#include "File.h"
#endif

typedef void (PROCRENDEF)(SShaderPass *l, int nPrimType);

#define USE_HDR 1
#define USE_3DC 1
//#define USE_FX 1

#ifdef LINUX
#	define DLL_IMPORT	
#	define DLL_EXPORT 
#else
#	define DLL_IMPORT	__declspec(dllimport)	// Import function from DLL
#	define DLL_EXPORT __declspec(dllexport)	// Export function to DLL
#endif

struct ICVar;
struct ShadowMapFrustum;
struct IStatObj;
struct SShaderPass;

typedef int (*pDrawModelFunc)(void);

//=============================================================

struct SMoveClip;
struct STrace;

// flags for EF_Release function
#define EFRF_VSHADERS 1
#define EFRF_PSHADERS 2

struct SFogState
{
  int m_nFogMode;
  int m_nCurFogMode;
  bool m_bEnable;
  float m_FogDensity;
  float m_FogStart;
  float m_FogEnd;
  CFColor m_FogColor;
  UCol m_CurColor;

  bool operator != (SFogState fs)
  {
    if (m_nFogMode!=fs.m_nFogMode || m_FogDensity!=fs.m_FogDensity || m_FogStart!=fs.m_FogStart || m_FogEnd!=fs.m_FogEnd || m_FogColor!=fs.m_FogColor)
      return true;
    return false;
  }
};


//////////////////////////////////////////////////////////////////////
class CRenderer : public IRenderer
{
public:	
	DEFINE_ALIGNED_DATA( Matrix44, m_ViewMatrix, 16 );
	DEFINE_ALIGNED_DATA( Matrix44, m_CameraMatrix, 16 );
	DEFINE_ALIGNED_DATA( Matrix44, m_ProjMatrix, 16 );
	DEFINE_ALIGNED_DATA( Matrix44, m_CameraProjMatrix, 16 );
	DEFINE_ALIGNED_DATA( Matrix44, m_InvCameraProjMatrix, 16 );

  //_declspec(align(16)) Matrix44 m_ViewMatrix;
  //_declspec(align(16)) Matrix44 m_CameraMatrix;
  //_declspec(align(16)) Matrix44 m_ProjMatrix;
  //_declspec(align(16)) Matrix44 m_CameraProjMatrix;
  //_declspec(align(16)) Matrix44 m_CameraProjTransposedMatrix;
  //_declspec(align(16)) Matrix44 m_InvCameraProjMatrix;

  bool              m_bDeviceLost;

  // Shaders pipeline status
  //=============================================================================================================
  SRenderPipeline m_RP;
  //=============================================================================================================

  int              m_MaxVertBufferSize;
  int              m_CurVertBufferSize;

  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F m_DynVB[2048];
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *m_TempDynVB;

  bool m_bEditor; // Render instance created from editor
  bool m_bWasCleared;
  int  m_VSync;
  int  m_FSAA;
  int  m_FSAA_quality;
  int  m_FSAA_samples;
  int  m_NoPS30;
  int  m_NoPS2X;
  int  m_sm30path;
  int  m_sm2xpath;
	int		m_deskwidth,m_deskheight,m_deskbpp;//,m_deskfreq;
  float m_fLineWidth;
  int  m_nHDRType;
  bool  m_bDeviceSupports_PS2X;
  int  m_nEnabled_PS2X;
  bool  m_bDeviceSupports_PS30;
  int  m_nEnabled_PS30;

  ICVar *m_CVWidth;
  ICVar *m_CVHeight;
  ICVar *m_CVFullScreen;
  ICVar *m_CVColorBits;

  CXFont *m_pCurFont;
  CFColor m_CurFontColor;
  float m_fXFontScale, m_fYFontScale;
	CFColor m_FontColorTable[16];

  SFogState m_FS;
  SFogState m_FSStack[8];
  int m_nCurFSStackLevel;

  byte m_bDeviceSupportsComprNormalmaps; // 1: 3DC, 2: V8U8, 3: CxV8U8

  bool m_bHeatVision;
  // tiago added
  bool      m_bRefraction;

  float mMinDepth, mMaxDepth;
  DWORD m_Features;
  int m_MaxTextureSize;
  int m_MaxTextureMemory;
  int m_nShadowTexSize;
  bool m_IsDedicated;
  int m_CurState;
  int m_CurStencilState;
  uint m_CurStencMask;
  uint m_CurStencRef;
  int m_CurStencilSide;

  unsigned short m_GammaTable[256];
  float m_fLastGamma;
  float m_fLastBrightness;
  float m_fLastContrast;
  float m_fDeltaGamma;

  CRenderer();
	virtual ~CRenderer();

#ifndef PS2
  virtual WIN_HWND Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen,WIN_HINSTANCE hinst, WIN_HWND Glhwnd=0, WIN_HDC Glhdc=0, WIN_HGLRC hGLrc=0, bool bReInit=false)=0;
#else	//PS2
	virtual bool Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen, bool bReInit=false)=0;
#endif	//PS2
  virtual bool SetCurrentContext(WIN_HWND hWnd)=0;
  virtual bool CreateContext(WIN_HWND hWnd, bool bAllowFSAA=false)=0;
  virtual bool DeleteContext(WIN_HWND hWnd)=0;

  virtual int  CreateRenderTarget (int nWidth, int nHeight, ETEX_Format eTF=eTF_8888)=0;
  virtual bool DestroyRenderTarget (int nHandle)=0;
  virtual bool SetRenderTarget (int nHandle)=0;

  virtual int GetFeatures() {return m_Features;}
  virtual int GetMaxTextureMemory() {return m_MaxTextureMemory;}
	
	//! Return all supported by video card video formats (except low resolution formats)
	virtual int	EnumDisplayFormats(TArray<SDispFormat>& Formats, bool bReset)=0;

  //! Return all supported by video card video AA formats
  virtual int	EnumAAFormats(TArray<SAAFormat>& Formats, bool bReset)=0;

  //! Changes resolution of the window/device (doen't require to reload the level
  virtual bool	ChangeResolution(int nNewWidth, int nNewHeight, int nNewColDepth, int nNewRefreshHZ, bool bFullScreen)=0;
  virtual bool CheckDeviceLost() { return false; };

  virtual void	ShutDown(bool bReInit=false)=0;
	virtual void	Release();
  virtual void  FreeResources(int nFlags);
  virtual void  RefreshResources(int nFlags)=0;

	virtual void	BeginFrame	(void)=0;
	virtual void	Update		(void)=0;	

  virtual void	PreLoad (void);
  virtual void	PostLoad (void);
  virtual void	Reset (void) = 0;

  virtual void *GetDynVBPtr(int nVerts, int &nOffs, int Pool) = 0;
  virtual void DrawDynVB(int nOffs, int Pool, int nVerts) = 0;
  virtual void DrawDynVB(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pBuf, ushort *pInds, int nVerts, int nInds, int nPrimType) = 0;
	virtual	CVertexBuffer	*CreateBuffer(int  vertexcount,int vertexformat, const char *szSource, bool bDynamic=false)=0;
  virtual void  CreateBuffer(int size, int vertexformat, CVertexBuffer *buf, int Type, const char *szSource)=0;
	virtual void	ReleaseBuffer(CVertexBuffer *bufptr)=0;
	virtual void	DrawBuffer(CVertexBuffer *src,SVertexStream *indicies,int numindices, int offsindex, int prmode,int vert_start=0,int vert_stop=0, CMatInfo *mi=NULL)=0;
	virtual void	UpdateBuffer(CVertexBuffer *dest,const void *src,int vertexcount, bool bUnLock, int offs=0, int Type=0)=0;

  virtual void  CreateIndexBuffer(SVertexStream *dest,const void *src,int indexcount)=0;
  virtual void  UpdateIndexBuffer(SVertexStream *dest,const void *src,int indexcount, bool bUnLock=true)=0;
  virtual void  ReleaseIndexBuffer(SVertexStream *dest)=0;

	virtual	void	CheckError(const char *comment)=0;

	virtual	void	Draw3dBBox(const Vec3 &mins, const Vec3 &maxs, int nPrimType)=0;
  
	struct BBoxInfo 
	{ 
		BBoxInfo () { nPrimType=0; }
		Vec3 vMins, vMaxs; float fColor[4]; int nPrimType; 
	};
	std::vector<BBoxInfo> m_arrBoxesToDraw;

	virtual	void	SetCamera(const CCamera &cam)=0;
  virtual	void	SetViewport(int x=0, int y=0, int width=0, int height=0)=0;
  virtual	void	SetScissor(int x=0, int y=0, int width=0, int height=0)=0;
  virtual void  GetViewport(int *x, int *y, int *width, int *height);

  virtual void	SetState(int State) { EF_SetState(State); }
	virtual void	SetCullMode	(int mode=R_CULL_BACK)=0;
	virtual bool	EnableFog	(bool enable)=0;
	virtual void	SetFog		(float density,float fogstart,float fogend,const float *color,int fogmode)=0;
  virtual void  SetFogColor(float * color)=0;
	virtual void	EnableTexGen(bool enable)=0; 
	virtual void	SetTexgen	(float scaleX,float scaleY,float translateX=0,float translateY=0)=0;
  virtual void  SetTexgen3D(float x1, float y1, float z1, float x2, float y2, float z2)=0;
	virtual void	SetLodBias	(float value=R_DEFAULT_LODBIAS)=0;
	virtual	void	EnableVSync(bool enable)=0;

  virtual void  DrawTriStrip(CVertexBuffer *src, int vert_num=4)=0;
	
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
	
	virtual bool	ChangeDisplay(unsigned int width,unsigned int height,unsigned int cbpp)=0;
  virtual void  ChangeViewport(unsigned int x,unsigned int y,unsigned int width,unsigned int height)=0;

  virtual	bool	SaveTga(unsigned char *sourcedata,int sourceformat,int w,int h,const char *filename,bool flip);

	//download an image to video memory. 0 in case of failure
	virtual	unsigned int DownLoadToVideoMemory(unsigned char *data,int w, int h, ETEX_Format eTFSrc, ETEX_Format eTFDst, int nummipmap, bool repeat=true, int filter=FILTER_BILINEAR, int Id=0, char *szCacheName=NULL, int flags=0)=0;
  virtual	void UpdateTextureInVideoMemory(uint tnum, unsigned char *newdata,int posx,int posy,int w,int h,ETEX_Format eTF=eTF_0888)=0;
	
	virtual unsigned int LoadTexture(const char * filename,int *tex_type=NULL,unsigned int def_tid=0,bool compresstodisk=true,bool bWarn=true)=0;
  virtual bool DXTCompress( byte *raw_data,int nWidth,int nHeight,ETEX_Format eTF, bool bUseHW, bool bGenMips, int nSrcBytesPerPix, MIPDXTcallback callback=0);
  virtual bool DXTDecompress(byte *srcData,byte *dstData,int nWidth,int nHeight,ETEX_Format eSrcTF, bool bUseHW, int nDstBytesPerPix);
//virtual unsigned int MakeTexture(const char * filename,int *tex_type=NULL/*,unsigned int def_tid=0*/)=0;

	virtual	bool	SetGammaDelta(const float fGamma)=0;

	virtual	void	RemoveTexture(unsigned int TextureId)=0;
	virtual	void	RemoveTexture(ITexPic * pTexPic)=0;


	virtual	void	SetTexture(int tnum, ETexType Type=(ETexType)0)=0;	
  virtual	void	SetWhiteTexture();

  virtual void  PrintToScreen(float x, float y, float size, const char *buf);
	virtual void	WriteXY		(CXFont *currfont,int x,int y, float xscale,float yscale,float r,float g,float b,float a,const char *message, ...);	
	// Default implementation.
	// To be overrided in derived classes.
	virtual void	Draw2dText( float posX,float posY,const char *szText,SDrawTextInfo &info );
  virtual void TextToScreen(float x, float y, const char * format, ...);
  virtual void TextToScreenColor(int x, int y, float r, float g, float b, float a, const char * format, ...);

	/*!	Draw an image on the screen as a label text
			@param vPos:	3d position
			@param fSize: size of the image
			@nTextureId:	Texture Id dell'immagine
	*/
	virtual void DrawLabelImage(const Vec3 &vPos,float fSize,int nTextureId);

  virtual void DrawLabel(Vec3 pos, float font_size, const char * label_text, ...);
  virtual void DrawLabelEx(Vec3 pos, float font_size, float * pfColor, bool bFixedSize, bool bCenter, const char * label_text, ...);
	virtual void Draw2dLabel( float x,float y, float font_size, float * pfColor, bool bCenter, const char * label_text, ...);
	
  virtual void	Draw2dImage	(float xpos,float ypos,float w,float h,int texture_id,float s0=0,float t0=0,float s1=1,float t1=1,float angle=0,float r=1,float g=1,float b=1,float a=1,float z=1)=0;
  virtual void RenderToViewport(const CCamera &cam, float x, float y, float width, float height);

	virtual int	SetPolygonMode(int mode)=0;

  virtual void ResetToDefault()=0;

	inline float ScaleCoordX(float value)
	{
//		value=(int)((float)(value)*(float)(m_width)/800.0f);	
		value*=float(m_width)/800.0f;
		return (value);
	}
	
	inline float ScaleCoordY(float value)
	{
//		value=(int)((float)(value)*(float)(m_height)/600.0f);	
		value*=float(m_height)/600.0f;
		return (value);
	}

	virtual	int		GetWidth()	{ return (m_width); }
	virtual	int		GetHeight() { return (m_height); }

	virtual int		GetPolygonMode() { return(m_polygon_mode); }

	const CCamera& GetCamera(void) { return(m_cam);  }

	void		GetPolyCount(int &nPolygons,int &nShadowVolPolys) 
	{ 
		nPolygons=m_nPolygons; 
		nShadowVolPolys=m_nShadowVolumePolys;
	}
	
	int GetPolyCount() { return m_nPolygons; }
	
  virtual int  GenerateAlphaGlowTexture(float k)=0;

	virtual void SetMaterialColor(float r, float g, float b, float a)=0;
	
	virtual int LoadAnimatedTexture(const char * format,const int nCount)=0;
	virtual void WriteDDS(byte *dat, int wdt, int hgt, int Size, const char *name, EImFormat eF, int NumMips);
	virtual void WriteTGA(byte *dat, int wdt, int hgt, const char *name, int bits);
	virtual void WriteJPG(byte *dat, int wdt, int hgt, char *name);

	virtual char * GetStatusText(ERendStats type)=0;
	virtual void GetMemoryUsage(ICrySizer* Sizer)=0;

  virtual int GetFrameID(bool bIncludeRecursiveCalls=true)
  {
    if (bIncludeRecursiveCalls)
      return m_nFrameID;
    return m_nFrameUpdateID;
  }

  // Project/UnProject
  virtual void ProjectToScreen( float ptx, float pty, float ptz, 
                                float *sx, float *sy, float *sz )=0;
  virtual int UnProject(float sx, float sy, float sz, 
                float *px, float *py, float *pz,
                const float modelMatrix[16], 
                const float projMatrix[16], 
                const int    viewport[4])=0;
  virtual int UnProjectFromScreen( float  sx, float  sy, float  sz, 
                           float *px, float *py, float *pz)=0;

	virtual void	Draw2dLine	(float x1,float y1,float x2,float y2)=0;
//  virtual void DrawLine(float * p1, float * p2)=0;
  virtual void SetLineWidth(float fWidth) { m_fLineWidth = fWidth; }
  virtual void DrawLine(const Vec3 & vPos1, const Vec3 & vPos2)=0;
  virtual void DrawLineColor(const Vec3 & vPos1, const CFColor & vColor1, const Vec3 & vPos2, const CFColor & vColor2)=0;
  virtual void DrawBall(float x, float y, float z, float radius)=0;
  virtual void DrawBall(const Vec3 & pos, float radius )=0;
  virtual void DrawPoint(float x, float y, float z, float fSize = 0.0f)=0;

  virtual void FlushTextMessages();

  // Shadow Mapping
  virtual void PrepareDepthMap(ShadowMapFrustum * lof, bool make_new_tid=0)=0;
  virtual ShadowMapFrustum * MakeShadowMapFrustum(ShadowMapFrustum * lof, struct ShadowMapLightSource * pLs, const Vec3 & obj_pos, list2<IStatObj*> * pStatObjects, int shadow_type);
  virtual void SetupShadowOnlyPass(int Num, ShadowMapFrustum * pFrustum, Vec3 * vShadowTrans, const float fShadowScale, Vec3 vObjTrans, float fObjScale, const Vec3 vObjAngles, Matrix44 * pObjMat)=0;
	virtual void DrawAllShadowsOnTheScreen()=0;
	virtual void OnEntityDeleted(IEntityRender * pEntityRender)=0;

  virtual void SetClipPlane( int id, float * params )=0;
	virtual void EF_SetClipPlane (bool bEnable, float *pPlane, bool bRefract)=0;
	
	virtual void SetColorOp(byte eCo, byte eAo, byte eCa, byte eAa)=0;

  //for editor
  virtual void  GetModelViewMatrix(float *mat)=0;
  virtual void  GetModelViewMatrix(double *mat) {};
  virtual void  GetProjectionMatrix(double *mat) {};
  virtual void  GetProjectionMatrix(float *mat)=0;
  virtual Vec3 GetUnProject(const Vec3 &WindowCoords,const CCamera &cam) { return(Vec3(1,1,1)); };

  virtual void DrawObjSprites(list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan)=0;
  virtual void DrawQuad(const Vec3 &right, const Vec3 &up, const Vec3 &origin,int nFlipMode=0)=0;
  virtual void DrawQuad(float dy,float dx, float dz, float x, float y, float z)=0;

  virtual void ClearDepthBuffer()=0;
  virtual void ClearColorBuffer(const Vec3 vColor)=0;  
  virtual void ReadFrameBuffer(unsigned char * pRGB, int nSizeX, int nSizeY, bool bBackBuffer, bool bRGBA, int nScaledX=-1, int nScaledY=-1)=0;
  
  //misc
  virtual void TransformTextureMatrix(float x, float y, float angle, float scale)=0;
  virtual void ResetTextureMatrix()=0;
 
  virtual void ScreenShot(const char *filename=NULL)=0;

  virtual int	GetColorBpp()		{ return (m_cbpp); }
  virtual int	GetDepthBpp()		{ return (m_zbpp); }
  virtual int	GetStencilBpp()		{ return (m_sbpp); }
  virtual int	GetAlphaBpp()		{ return (m_abpp); }
  virtual char	GetType()			{ return (m_type); }
  virtual char*	GetVertexProfile(bool bSupportedProfile)=0;
  virtual char*	GetPixelProfile(bool bSupportedProfile)=0;
  virtual void	SetType(char type)	{ m_type=type; }	

  virtual unsigned int MakeSprite(float object_scale, int tex_size, float angle, IStatObj * pStatObj, uchar * pTmpBuffer, uint def_tid)=0;
  virtual unsigned int Make3DSprite(int nTexSize, float fAngleStep, IStatObj * pStatObj)=0;

  virtual void Set2DMode(bool enable, int ortox, int ortoy)=0;

  virtual int ScreenToTexture()=0;

  virtual void SetTexClampMode(bool clamp)=0;

  virtual void DrawPoints(Vec3 v[], int nump, CFColor& col, int flags)=0;
  virtual void DrawLines(Vec3 v[], int nump, CFColor& col, int flags, float fGround)=0;
  virtual void Graph(byte *g, int x, int y, int wdt, int hgt, int nC, int type, char *text, CFColor& color, float fScale);


  // Shaders/Shaders support
  // RE - RenderElement
  bool m_bTimeProfileUpdated;
  int m_PrevProfiler;
  int m_nCurSlotProfiler;

  FILE *m_LogFile;
  FILE *m_LogFileStr;
  _inline void Logv(int RecLevel, char *format, ...)
  {
    va_list argptr;
    
    if (m_LogFile)
    {
      for (int i=0; i<RecLevel; i++)
      {
        fprintf(m_LogFile, "  ");
      }
      va_start (argptr, format);
#ifndef PS2
      vfprintf (m_LogFile, format, argptr);
#endif      
      va_end (argptr);
    }
  }
  _inline void LogStrv(int RecLevel, char *format, ...)
  {
    va_list argptr;

    if (m_LogFileStr)
    {
      for (int i=0; i<RecLevel; i++)
      {
        fprintf(m_LogFileStr, "  ");
      }
      va_start (argptr, format);
#ifndef PS2
      vfprintf (m_LogFileStr, format, argptr);
#endif      
      va_end (argptr);
    }
  }
  _inline void Log(char *str)
  {
    if (m_LogFile)
    {
      fprintf (m_LogFile, str);
    }
  }
  

  void EF_TransformDLights();
  void EF_IdentityDLights();

  _inline void *EF_GetPointer(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags)
  {
    void *p;
    
    if (m_RP.m_pRE)
      p = m_RP.m_pRE->mfGetPointer(ePT, Stride, Type, Dst, Flags);
    else
      p = SRendItem::mfGetPointerCommon(ePT, Stride, Type, Dst, Flags);
    
    return p;
  }
  struct SSavedDLight
  {
    CDLight *m_pLight;
    CDLight m_Contents;
  };
  bool EF_IsOnlyLightPass(SShaderPassHW *slw)
  {
    if (slw && (slw->m_LightFlags & DLF_ACTIVE) && !(slw->m_LightFlags & DLF_HASAMBIENT))
      return true;
    return false;
  }
  bool EF_RejectLightPass(SShaderPassHW *slw)
  {
    if (!m_RP.m_pRE || m_RP.m_pRE->mfGetType() != eDATA_OcLeaf)
      return false;
    CREOcLeaf *re = (CREOcLeaf *)m_RP.m_pRE;
    if (CRenderer::CV_r_cullgeometryforlights && EF_IsOnlyLightPass(slw))
      m_RP.m_pCurLightIndices = re->mfGetIndiciesForLight(m_RP.m_pCurLight);
    else
      return false;
    return m_RP.m_pCurLightIndices ? false : true;
  }
  bool EF_BuildLightsList()
  {
    // Count light sources
    m_RP.m_NumActiveDLights = 0;
    int nR = SRendItem::m_RecurseLevel;
    if (m_RP.m_DynLMask)
    {
      for (int n=0; n<m_RP.m_DLights[nR].Num(); n++)
      {
        if (m_RP.m_DynLMask & (1<<n))
        {
          m_RP.m_pActiveDLights[m_RP.m_NumActiveDLights] = m_RP.m_DLights[nR][n];
          m_RP.m_NumActiveDLights++;
          if (m_RP.m_NumActiveDLights == 16)
            return false;
        }
      }
    }
    return true;
  }
  _inline bool EF_TryToMerge(int nNewObject, int nOldObject, CRendElement *pRE)
  {
#ifndef PIPE_USE_INSTANCING
    return false;
#else

    CREOcLeaf *re = (CREOcLeaf *)pRE;
    if ((m_RP.m_FlagsPerFlush & RBSI_MERGED) && (re->mfGetFlags() & FCEF_MERGABLE))
    {
      // Merging case
      CCObject *pObjN = m_RP.m_VisObjects[nNewObject];
      if (!pObjN->IsMergable())
        return false;
      CCObject *pObjO = m_RP.m_VisObjects[nOldObject];
      if (pObjN->m_nLMId != pObjO->m_nLMId)
        return false;
      if ((pObjN->m_ObjFlags & FOB_MASKCONDITIONS) != (pObjO->m_ObjFlags & FOB_MASKCONDITIONS))
        return false;
      if (pObjN->m_AmbColor != pObjO->m_AmbColor)
        return false;
      if (pObjN->m_Color != pObjO->m_Color)
        return false;
      if (m_RP.m_ClipPlaneEnabled)
      {
        if (pRE->mfCullByClipPlane(pObjN))
          return true;
      }
      re->mfFillRB(pObjN);
      return true;
    }
    if (!m_RP.m_pRE || m_RP.m_pRE != pRE || pRE->mfGetType() != eDATA_OcLeaf)
      return false;
    // Batching case
    CCObject *pObjN = m_RP.m_VisObjects[nNewObject];
    CCObject *pObjO = m_RP.m_VisObjects[nOldObject];
    if (pObjN->m_DynLMMask != pObjO->m_DynLMMask)
      return false;
    if (pObjN->m_nLMId != pObjO->m_nLMId)
      return false;
    if ((pObjN->m_ObjFlags & FOB_MASKCONDITIONS) != (pObjO->m_ObjFlags & FOB_MASKCONDITIONS))
      return false;
    if ((INT_PTR)pObjN->m_pShadowCasters | (INT_PTR)pObjO->m_pShadowCasters)
      return false;
    if (m_RP.m_ClipPlaneEnabled)
    {
      if (pRE->mfCullByClipPlane(pObjN))
        return true;
    }

    if (!m_RP.m_MergedObjects.Num())
      m_RP.m_MergedObjects.AddElem(pObjO);
    m_RP.m_MergedObjects.AddElem(pObjN);
    return true;
#endif
  }


  int EF_SelectHWTechnique(SShader *ef);

  _inline CFColor EF_GetCurrentAmbient(SLightMaterial *lm, int Flags)
  {
    CFColor colAmb;

    if (Flags & LMF_NOAMBIENT)
      colAmb = Col_Black;
    else
    if (Flags & LMF_ONLYMATERIALAMBIENT)
      colAmb = lm->Front.m_Ambient;
    else
      colAmb = m_RP.m_pCurObject->m_AmbColor;
    if (!(m_RP.m_pCurObject->m_ObjFlags & FOB_IGNOREMATERIALAMBIENT))
    {
      colAmb.r *= lm->Front.m_Ambient.r;
      colAmb.g *= lm->Front.m_Ambient.g;
      colAmb.b *= lm->Front.m_Ambient.b;
      colAmb.a *= lm->Front.m_Ambient.a;
    }
    float fScale = 1.0f;
    if (Flags & LMF_DIVIDEAMB4)
      fScale = 0.25f;
    if (Flags & LMF_DIVIDEAMB2)
      fScale = 0.5f;
    if (colAmb.a != 1.0f && (Flags & LMF_NOALPHA))
      colAmb.a = 1.0f;
    else
      colAmb.a = m_RP.m_fCurOpacity;
    colAmb.r = min(colAmb.r * fScale, 1.0f);
    colAmb.g = min(colAmb.g * fScale, 1.0f);
    colAmb.b = min(colAmb.b * fScale, 1.0f);

    return colAmb;
  }
  _inline CFColor EF_GetCurrentDiffuse(SLightMaterial *lm, int Flags)
  {
    CFColor colDif = lm->Front.m_Diffuse;

    float fScale = 1.0f;
    if (Flags & LMF_DIVIDEDIFF4)
      fScale = 0.25f;
    if (Flags & LMF_DIVIDEDIFF2)
      fScale = 0.5f;
    colDif.r = min(colDif.r * fScale, 1.0f);
    colDif.g = min(colDif.g * fScale, 1.0f);
    colDif.b = min(colDif.b * fScale, 1.0f);
    colDif.a = 1.0f;

    return colDif;
  }
  _inline void EF_ConstantLightMaterial(SLightMaterial *lm, int Flags)
  {
    CFColor colAmb = EF_GetCurrentAmbient(lm, Flags);
    UCol color;
    color.bcolor[0] = (byte)(colAmb.b * 255.0f);
    color.bcolor[1] = (byte)(colAmb.g * 255.0f);
    color.bcolor[2] = (byte)(colAmb.r * 255.0f);
    color.bcolor[3] = (byte)(colAmb.a * 255.0f);
    m_RP.m_NeedGlobalColor = color;
    m_RP.m_FlagsPerFlush |= RBSI_GLOBALRGB | RBSI_GLOBALALPHA;
  }


  _inline void EF_PushFog()
  {
    int nLevel = m_nCurFSStackLevel;
    if (nLevel >= 8)
      return;
    memcpy(&m_FSStack[nLevel], &m_FS, sizeof(SFogState));
    m_nCurFSStackLevel++;
  }
  _inline void EF_PopFog()
  {
    int nLevel = m_nCurFSStackLevel;
    if (nLevel <= 0)
      return;
    nLevel--;
    bool bFog = m_FS.m_bEnable;
    if (m_FS != m_FSStack[nLevel])
    {
      memcpy(&m_FS, &m_FSStack[nLevel], sizeof(SFogState));
      SetFog(m_FS.m_FogDensity, m_FS.m_FogStart, m_FS.m_FogEnd, &m_FS.m_FogColor[0], m_FS.m_nFogMode);
    }
    else
      m_FS.m_bEnable = m_FSStack[nLevel].m_bEnable;
    bool bNewFog = m_FS.m_bEnable;
    m_FS.m_bEnable = bFog;
    EnableFog(bNewFog);
    m_nCurFSStackLevel--;
  }
  void EF_InitFogVolumes();

  static void EF_DrawGeneralPasses(SShaderTechnique *hs, SShader *ef, bool bVolFog, int nStart, int nEnd);
  static void EF_DrawLightPasses(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd, EShaderPassType eShPass);
  static void EF_DrawShadowPasses(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd);

  virtual void EF_Release(int nFlags)=0;
  virtual void EF_EnableHeatVision(bool bEnable);
  virtual bool EF_GetHeatVision();
  virtual float EF_GetWaterZElevation(float fX, float fY);
  virtual void EF_PipelineShutdown() = 0;

  virtual void EF_PolygonOffset(bool bEnable, float fFactor, float fUnits)=0;

  virtual bool EF_PrecacheResource(IShader *pSH, float fDist, float fTimeToReady, int Flags);
  virtual bool EF_PrecacheResource(ITexPic *pTP, float fDist, float fTimeToReady, int Flags);
  virtual bool EF_PrecacheResource(CLeafBuffer *pPB, float fDist, float fTimeToReady, int Flags);
  virtual bool EF_PrecacheResource(CDLight *pLS, float fDist, float fTimeToReady, int Flags);
  
  // Register new user defined template
  virtual bool EF_RegisterTemplate(int nTemplId, char *Name, bool bReplace);

  // Create splash
  virtual void	EF_AddSplash (Vec3 Pos, eSplashType eST, float fForce, int Id=-1);
  void EF_UpdateSplashes(float fTime);

  virtual void EF_AddPolyToScene2D(int Ef, int numPts, SColorVert2D *verts);
  virtual void EF_AddPolyToScene2D(SShaderItem si, int nTempl, int numPts, SColorVert2D *verts);
  void EF_AddClientPolys2D(void);
  
  void EF_AddClientPolys3D(void);
  void EF_RemovePolysFromScene(void);

  virtual void EF_LightMaterial(SLightMaterial *lm, int Flags)=0;
  
  virtual void EF_CheckOverflow(int nVerts, int nTris, CRendElement *re)=0;
  virtual void EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, int nFog, CRendElement *re)=0;
  virtual void EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, CRendElement *re)=0;
  
  //==========================================================
  // external interface for shaders
  //==========================================================

  // Add 3D polygon to the list
  virtual void EF_AddPolyToScene3D(int Ef, int numPts, SColorVert *verts, CCObject *obj=NULL, int nFogID=0);
  virtual CCObject *EF_AddSpriteToScene(int Ef, int numPts, SColorVert *verts, CCObject *obj, byte *inds=NULL, int ninds=0, int nFogID=0);
  void EF_AddSprite(SShader *pSH, Vec3 vOrigin, float fRadius);

  // Shaders management
  virtual IShader *EF_LoadShader (const char *name, EShClass Class, int flags=0, uint64 nMaskGen=0);
  virtual SShaderItem EF_LoadShaderItem (const char *name, EShClass Class, bool bShare, const char *templName, int flags=0, SInputShaderResources *Res=NULL, uint64 nMaskGen=0);
  // reload file
  virtual bool EF_ReloadFile (const char *szFileName);
  virtual void EF_ReloadShaderFiles (int nCategory);
  virtual void EF_ReloadTextures ();
  virtual IShader *EF_CopyShader(IShader *ef);
  virtual int EF_LoadLightmap(const char* nameTex);
  virtual bool	EF_ScanEnvironmentCM (const char *name, int size, Vec3& Pos);
  virtual ITexPic *EF_GetTextureByID(int Id);
  virtual ITexPic *EF_LoadTexture(const char* nameTex, uint flags, uint flags2, byte eTT, float fAmount1=-1.0f, float fAmount2=-1.0f, int Id=-1, int BindId=0);
  virtual int EF_ReadAllImgFiles(IShader *ef, SShaderTexUnit *tl, STexAnim *ta, char *name);
  virtual char **EF_GetShadersForFile(const char *File, int num);
  virtual SLightMaterial *EF_GetLightMaterial(char *Str);
  virtual bool EF_SetLightHole(Vec3 vPos, Vec3 vNormal, int idTex, float fScale=1.0f, bool bAdditive=true)=0;

  virtual STexPic *EF_MakePhongTexture(int Exp)=0;

  // Create new RE of type (edt)
  virtual CRendElement *EF_CreateRE (EDataType edt);

  // Begin using shaders
  virtual void EF_StartEf ();

  // Get Object for RE transformation
  virtual CCObject *EF_GetObject (bool bTemp=false, int num=-1);

  // Add shader to the list (virtual)
  virtual void EF_AddEf (int NumFog, CRendElement *re, IShader *ef, SRenderShaderResources *sr,  CCObject *obj, int nTempl, IShader *efState=0, int nSort=0);

	// Add shader to the list 
	void EF_AddEf_NotVirtual (int NumFog, CRendElement *re, IShader *ef, SRenderShaderResources *sr, CCObject *obj, int nTempl, IShader *efState=0, int nSort=0);
  
  // Hide shader template (exclude from list)
  virtual bool					EF_HideTemplate(const char *name);
  // UnHide shader template (include in list)
  virtual bool					EF_UnhideTemplate(const char *name);
  // UnHide all shader templates (include in list)
  virtual bool					EF_UnhideAllTemplates();

  // Draw all shaded REs in the list
  virtual void EF_EndEf3D (int nFlags)=0;

  // 2d interface for shaders
  virtual void EF_EndEf2D(bool bSort)=0;

  // Dynamic lights
  virtual bool EF_IsFakeDLight (CDLight *Source);
  virtual void EF_ADDDlight(CDLight *Source);
  virtual void EF_ClearLightsList();
  virtual bool EF_UpdateDLight(CDLight *pDL);

  // Add new shader to the list
  virtual bool EF_DrawEfForName(char *name, float x, float y, float width, float height, CFColor& col, int nTempl=-1);
  virtual bool EF_DrawEfForNum(int num, float x, float y, float width, float height, CFColor& col, int nTempl=-1);
  virtual bool EF_DrawEf(IShader *ef, float x, float y, float width, float height, CFColor& col, int nTempl=-1);
  virtual bool EF_DrawEf(SShaderItem si, float x, float y, float width, float height, CFColor& col, int nTempl=-1);

  // Add new shader in part of texture to the list
  virtual bool EF_DrawPartialEfForName(char *name, SVrect *vr, SVrect *pr, CFColor& col);
  virtual bool EF_DrawPartialEfForNum(int num, SVrect *vr, SVrect *pr, CFColor& col);
  virtual bool EF_DrawPartialEf(IShader *ef, SVrect *vr, SVrect *pr, CFColor& col, float iwdt=0, float ihgt=0);

  virtual void *EF_Query(int Query, int Param=0);
  virtual void EF_ConstructEf(IShader *Ef);
  virtual void EF_SetWorldColor(float r, float g, float b, float a=1.0f);

  void EF_SetState(int st);
  void EF_SetStencilState(int st, uint nStencRef, uint nStencMask);

  virtual int  EF_RegisterFogVolume(float fMaxFogDist, float fFogLayerZ, CFColor color, int nIndex=-1, bool bCaustics=false) = 0;

/*	
// The following may be (and sometimes is) used for testing. Please don't remove.
#ifdef _DEBUG
	// registers the given client  - do nothing by default
	void RegisterCallbackClient (IRendererCallbackClient* pClient)
	{
		m_setRegisteredCallbacks.insert (pClient);
	}
	// unregisters the given client - do nothing by default
	void UnregisterCallbackClient (IRendererCallbackClient* pClient)
	{
		m_setRegisteredCallbacks.erase(pClient);
	}
	// unregisters all clients
	void UnregisterAllCallbackClients ()
	{
		while (!m_setRegisteredCallbacks.empty())
			(*m_setRegisteredCallbacks.begin())->Renderer_SelfUnregister();
	}
	// the set of registered callbacks
	std::set<IRendererCallbackClient*> m_setRegisteredCallbacks;
#endif
*/	
	// create/delete LeafBuffer object
  virtual CLeafBuffer * CreateLeafBuffer(bool bDynamic, const char *szSource, CIndexedMesh * pIndexedMesh=0);
  virtual CLeafBuffer * CreateLeafBufferInitialized(
    void * pVertBuffer, int nVertCount, int nVertFormat, 
    ushort* pIndices, int nIndices,
    int nPrimetiveType, const char *szSource, EBufferType eBufType = eBT_Dynamic,
    int nMatInfoCount=1, int nClientTextureBindID=0,    
    bool (*PrepareBufferCallback)(CLeafBuffer *, bool)=NULL,
    void *CustomData = NULL,
    bool bOnlyVideoBuffer=false, bool bPrecache=true);

  virtual void DeleteLeafBuffer(CLeafBuffer * pLBuffer);

  virtual int GetMaxActiveTexturesARB() { return 0;}

  //////////////////////////////////////////////////////////////////////
	// Replacement functions for the Font engine ( vlad: for font can be used old functions )
	virtual	bool FontUploadTexture(class CFBitmap*, ETEX_Format eTF=eTF_8888)=0;
	virtual	int  FontCreateTexture(int Width, int Height, byte *pData, ETEX_Format eTF=eTF_8888)=0;
  virtual	bool FontUpdateTexture(int nTexId, int X, int Y, int USize, int VSize, byte *pData)=0;
	virtual	void FontReleaseTexture(class CFBitmap *pBmp)=0;
	virtual void FontSetTexture(class CFBitmap*, int nFilterMode)=0;
  virtual void FontSetTexture(int nTexId, int nFilterMode)=0;
	virtual void FontSetRenderingState(unsigned long nVirtualScreenWidth, unsigned long nVirtualScreenHeight)=0;
	virtual void FontSetBlending(int src, int dst)=0;
	virtual void FontRestoreRenderingState()=0;

	virtual void SetGlobalShaderTemplateId(int nTemplateId) { m_nGlobalShaderTemplateId = nTemplateId; }
	virtual int GetGlobalShaderTemplateId() { return m_nGlobalShaderTemplateId; }

  //////////////////////////////////////////////////////////////////////
  // Used for pausing timer related stuff (eg: for texture animations, and shader 'time' parameter)
  void PauseTimer(bool bPause) {  m_bPauseTimer=bPause; }

protected:	
  
	int				m_nGlobalShaderTemplateId;
	char			m_type;
	int				m_width,m_height,m_cbpp,m_zbpp,m_sbpp,m_abpp,m_FullScreen;	
	int				m_polygon_mode;
  int       m_VX, m_VY, m_VWidth, m_VHeight;

	static	bool	m_showfps;
	CCamera			m_cam;        // current camera

  int m_nFrameID;

  TArray<CName> m_HidedShaderTemplates;

  struct text_info_struct 
	{ 
		char	mess[128];
		Vec3 pos; 
		float font_size; 
		float fColor[4]; 
		bool	bFixedSize,bCenter,b2D; 
		int		nTextureId;
	};

  list2<text_info_struct> m_listMessages;


public:  
  CCamera			m_prevCamera; // camera from previous frame
  int m_nFrameUpdateID;
  int m_nFrameLoad;
  bool m_bTemporaryDisabledSFX;


  Vec3 m_vClearColor;
  int				m_numtmus;
	int		m_nPolygons;
	int		m_nShadowVolumePolys;
  CTexMan *m_TexMan;
  CREScreenProcess *m_pREScreenProcess;
  
  // Used for pausing timer related stuff (eg: for texture animations, and shader 'time' parameter)
  bool m_bPauseTimer;
  float m_fPrevTime;

  // HDR rendering stuff
  int m_dwHDRCropWidth;
  int m_dwHDRCropHeight;

  //=====================================================================
  // Shaders interface
  CShader m_cEF;
  SShader *m_pDefaultShader;

  CFColor m_WorldColor;
  CFColor m_SavedWorldColor;
  int m_TexGenID;
  
  void SetClearColor(const Vec3 & vColor) { m_vClearColor = vColor; };

  //////////////////////////////////////////////////////////////////////
  // console variables
  //////////////////////////////////////////////////////////////////////

  static int CV_r_vsync;
  static int CV_r_stats;
  static int CV_r_log;
  static int CV_r_logTexStreaming;
  static int CV_r_logVBuffers;
  static int CV_r_syncvbuf;
  static int CV_r_flush;

#ifdef USE_HDR
  static int CV_r_hdrrendering;
  static int CV_r_hdrdebug;
#endif
  static int CV_r_hdrfake;
  static float CV_r_hdrlevel;
  static float CV_r_hdrbrightoffset;
  static float CV_r_hdrbrightthreshold;

  static int CV_r_geominstancing;

  static int CV_r_nobumpmap;
  static int CV_r_bumpselfshadow;
  static int CV_r_selfshadow;
  static int CV_r_shadowtype;
  static int CV_r_shadowblur;
  static int CV_r_debuglights;
//  static int CV_r_lightsscissor;
  static int CV_r_cullgeometryforlights;
  static ICVar *CV_r_showlight;

  static int CV_r_unifytangentnormals;
  static int CV_r_smoothtangents;
  static int CV_r_indexingWithTangents;

  static int CV_r_fullbrightness;

  // tiago added
  static int    CV_r_debugscreenfx;  
  static int    CV_r_flipscreen;
  
  static int    CV_r_dof;
  static float  CV_r_doffocaldist;
  static float  CV_r_doffocaldist_tag;
  static float  CV_r_doffocaldist_entity;
  static int    CV_r_doffocalsource;
  static float  CV_r_doffocalarea;
  static float  CV_r_doffocalareacurr;
  static float  CV_r_dofbluramount;

  static int    CV_r_resetscreenfx;
  static int    CV_r_rendermode;  
  static int    CV_r_glare;  
  static int    CV_r_glarequality;  
  static float  CV_r_glaretransition;  

  static float  CV_r_pp_contrast;  
  static float  CV_r_pp_brightness;  
  static float  CV_r_pp_gamma;  
  static float  CV_r_pp_saturation;  
  static float  CV_r_pp_sharpenamount;  
  static float  CV_r_pp_glareintensity;  
  static float  CV_r_pp_cmyk_c;  
  static float  CV_r_pp_cmyk_m;  
  static float  CV_r_pp_cmyk_y;  
  static float  CV_r_pp_cmyk_k;

  static int    CV_r_subsurface_type;  

  static float  CV_r_maxtexlod_bias;  

  static int    CV_r_heathaze;  
  static int    CV_r_cryvision;
  static int    CV_r_cryvisiontype;

  static int    CV_r_enhanceimage;
  static float  CV_r_enhanceimageamount;
  
  static float  CV_r_fadeamount;
  
  static int    CV_r_motionblur;  
  static int    CV_r_motionblurdisplace;    
  static float  CV_r_motionbluramount;  

  static int    CV_r_scopelens_fx;  
  static int    CV_r_disable_sfx;  


  static int CV_r_heatsize;
  static int CV_r_heatmapsave;
  static int CV_r_heatmapmips;
  static int CV_r_heattype;

  static int CV_r_flashbangsize;
  static int CV_r_screenrefract;

  static int CV_r_nightsize;
  static int CV_r_nightmapsave;
  static int CV_r_nighttype;

  static int CV_r_rainmapsize;

  static int CV_r_portals;
  static int CV_r_portalsrecursive;

  static int CV_r_stripmesh;

  static int CV_r_shownormals;
  static int CV_r_showlines;
  static float CV_r_normalslength;
  static int CV_r_showtangents;
  static int CV_r_showtimegraph;
  static int CV_r_showtextimegraph;
  static int CV_r_graphstyle;

  static int CV_r_hwlights;

  static int CV_r_specantialias;
  static float CV_r_shininess;
  static float CV_r_wavescale;

  static int CV_r_logusedtextures;
  static int CV_r_logusedshaders;
  static ICVar *CV_r_excludeshader;
  static ICVar *CV_r_showonlyshader;
  static int CV_r_logloadshaders;
  static int CV_r_profileshaders;
  static int CV_r_texprocedures;


#ifdef USE_3DC
  static int CV_r_texnormalmapcompressed;
#endif
  static int CV_r_texmaxsize;
  static int CV_r_texminsize;
  static int CV_r_texresolution;
  static int CV_r_texlmresolution;
  static int CV_r_texbumpquality;
  static int CV_r_texbumpresolution;
  static int CV_r_texquality;
  static int CV_r_texskyquality;
  static int CV_r_texskyresolution;
  static int CV_r_texforcesquare;
  static int CV_r_texquantizedither;
  static int CV_r_texnormalmaptype;
  static int CV_r_texgrayoverage;
  static int CV_r_texsimplemips;
  static int CV_r_texhwmipsgeneration;
  static int CV_r_texhwdxtcompression;

  static int CV_r_supportpalettedtextures;
  static int CV_r_supportcompressedtextures;
  
  static int CV_r_texturesstreampoolsize;
  static int CV_r_texturesstreaming;
  static int CV_r_texturesstreamingonlyvideo;
  static int CV_r_texturesstreamingsync;
  static int CV_r_texturespixelsize;
  static int CV_r_texturesbasemip;
  
  static int CV_r_envlightcmdebug;
  static int CV_r_envlighting;
  static int CV_r_envlightcmsize;
  static int CV_r_envcmwrite;
  static int CV_r_envcmresolution;
  static int CV_r_envtexresolution;
  static float CV_r_waterupdateDeltaAngle;
  static float CV_r_waterupdateFactor;
  static float CV_r_waterupdateDistance;
  static float CV_r_envlcmupdateinterval;
  static float CV_r_envcmupdateinterval;
  static float CV_r_envtexupdateinterval;
  static int CV_r_waterreflections;
  static int CV_r_waterrefractions;
  static int CV_r_waterbeachrefractions;
  static int CV_r_selfrefract;
  static int CV_r_texture_anisotropic_level;
  static int CV_r_oceanrendtype;
  static int CV_r_oceansectorsize;
  static int CV_r_oceanheightscale;
  static int CV_r_oceanloddist;
  static int CV_r_oceantexupdate;
  static int CV_r_oceanmaxsplashes;
  static float CV_r_oceansplashscale;
  static int CV_r_lightsourcesasheatsources;
  static ICVar *CV_r_glossdefault;
  static ICVar *CV_r_detaildefault;
  static ICVar *CV_r_opacitydefault;
  static ICVar *CV_r_shaderdefault;

  static int CV_r_reloadshaders;
  static int CV_r_decaltextures;

  static int CV_r_detailtextures;
  static float CV_r_detailscale;
  static float CV_r_detaildistance;
  static int CV_r_detailnumlayers;
  static int CV_r_usehwshaders;
  static int CV_r_nolightcalc;
  static int CV_r_nopreprocess;
  static int CV_r_noloadtextures;
  static int CV_r_texbindmode;
  static int CV_r_nodrawshaders;
  static int CV_r_nodrawnear;

  static int CV_r_nops20;
  static int CV_r_nops30;
  static int CV_r_nops2x;
  static int CV_r_sm30path;
  static int CV_r_sm2xpath;
  static int CV_r_offsetbump;
  static int CV_r_offsetbumpforce;
  static int CV_r_shaderssave;
  static int CV_r_shadersprecache;
  static int CV_r_precachemesh;

  static int CV_r_rb_merge;

  static int CV_r_fsaa;
  static int CV_r_fsaa_samples;
  static int CV_r_fsaa_quality;

  static int CV_r_flares;
  static int CV_r_procflares;
  static int CV_r_beams;
  static float CV_r_flaresize;

  static float CV_r_gamma;
  static float CV_r_contrast;
  static float CV_r_brightness;
  static int CV_r_nohwgamma;
  static int CV_r_noswgamma;

  static int CV_r_shaderdetailobjectsbending;
  static int CV_r_shaderterraindot3;
  static int CV_r_shaderterrainspecular;

  static int CV_r_checkSunVis;
  static float CV_r_embm;
  static int CV_r_sse;
  static int CV_r_coronas;
  static int CV_r_scissor;
  static float CV_r_coronafade;
  static float CV_r_coronacolorscale;
  static float CV_r_coronasizescale;

  static int CV_r_efmultitex;
	
  static int CV_r_noparticles;

  static int CV_r_cullbyclipplanes;
  static int CV_r_accurateparticles;

	static int CV_ind_VisualizeShadowVolumes;		
	static int CV_ind_ReverseShadowVolumes;			
	static int CV_ind_DrawBorderEdges;			

	static int CV_r_SunStyleCoronas;		

	static int CV_r_PolygonMode;		
	static int CV_r_GetScreenShot;	
  static int CV_r_VolumetricFog;
  static int CV_r_vpfog;
  static int CV_r_printmemoryleaks;
  static float CV_r_character_lod_bias;
  static int CV_r_character_nodeform;
  static int CV_r_character_debug;
  static int CV_r_character_noanim;
  static int CV_r_shadow_maps_debug;
  static int CV_r_draw_phys_only;
  static int CV_r_character_shadow_volume;
  static int CV_r_character_nophys;
	static int CV_r_ReplaceCubeMap;
  static int CV_r_VegetationSpritesAlphaBlend;
  static int CV_r_VegetationSpritesNoBend;
  static int CV_r_Vegetation_IgnoreVertColors;
  static int CV_r_Vegetation_PerpixelLight;

  static int CV_r_Quality_BumpMapping;
  static int CV_r_Quality_Reflection;

  static int CV_r_measureoverdraw;

	static int CV_r_DisplayInfo;

  static int CV_r_ShowVideoMemoryStats;

  virtual void MakeMatrix(const Vec3 & pos, const Vec3 & angles,const Vec3 & scale, Matrix44 * mat){assert(0);};

  list2<AnimTexInfo*> m_LoadedAnimatedTextures;
  AnimTexInfo * GetAnimTexInfoFromId(int nId);

#ifdef DEBUGALLOC
#undef new
#endif
	// Sergiy: it's enough to allocate 16 bytes more, even on 64-bit machine
	// - and we need to store only the offset, not the actual pointer
  void* operator new( size_t Size )
  {
    byte *ptr = (byte *)malloc(Size+16+4);
    memset(ptr, 0, Size+16+4);
    byte *bPtr = (byte *)ptr;
    byte *bPtrRes = (byte *)((INT_PTR)(bPtr+4+16) & ~0xf);
    ((byte**)bPtrRes)[-1] = ptr;

    return bPtrRes;
  }
  void operator delete( void *Ptr )
  {
    free(((byte**)Ptr)[-1]);
  }
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif

  virtual WIN_HWND GetHWND() = 0;

  void SetTextureAlphaChannelFromRGB(byte * pMemBuffer, int nTexSize);
	void RemoveAnimatedTexture(AnimTexInfo * pInfo);

	void EnableSwapBuffers(bool bEnable) { m_bSwapBuffers = bEnable; }
	bool m_bSwapBuffers;
};

//struct CVars;
class CryCharManager;
void *gGet_D3DDevice();
void *gGet_glReadPixels();

#include "CommonRender.h"


#define ZeroStruct( t ) { memset( &t,0,sizeof(t) ); }

#define SKY_BOX_SIZE 32.f

#endif //renderer