
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:GL_Renderer.h - GL API 
//
//	History:
//	-Jan 31,2001:Originally created by Marco Corbetta
//	-: taken over by Andrey Khonich
//
//////////////////////////////////////////////////////////////////////

#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#if _MSC_VER > 1000
# pragma once
#endif

/*
===========================================
The GLRenderer interface Class
===========================================
*/


//#include "list2.h"
#include "Mygl.h"
#include "Myglu.h"

// GL functions declare.
#define GL_EXT(name) extern byte SUPPORTS##name;
#define GL_PROC(ext,ret,func,parms) extern ret (__stdcall *func)parms;
#include "GLFuncs.h"
#undef GL_EXT
#undef GL_PROC

#include "cg\cgGL.h"

class PBuffer;
class CPBuffer;

typedef std::map<int,STexPic*> TTextureMap;
typedef TTextureMap::iterator TTextureMapItor;

struct SBufRegion
{
  HANDLE m_BRHandle;
  int m_Width;
  int m_Height;
};

struct SRendContext
{
	HDC		m_hDC;
	HGLRC	m_hRC;
	HWND	m_Glhwnd;
  bool  m_bFSAAWasSet;
  int m_PixFormat;
};

struct STextureTarget
{
  CPBuffer *m_pBuffer;
  int m_Bind;
  int m_Width;
  int m_Height;
  int m_DrawCount;
};

struct SGLTexUnit
{
  int m_Bind;
  int m_Target;
  float m_fTexFilterLodBias;
  UCol m_Color;
};

//////////////////////////////////////////////////////////////////////
class CGLRenderer : public CRenderer
{
  friend class CGLTexMan;
  friend class CPBuffer;
  friend class CREOcean;

  char m_LibName[64];
  const unsigned char *m_VendorName;
  const unsigned char *m_RendererName;
  const unsigned char *m_VersionName;
  const unsigned char *m_ExtensionsName;

  HANDLE m_hLibHandle;          
  HANDLE m_hLibHandleGDI;          
  HINSTANCE m_hInst;

  int m_MaxClipPlanes;
  int m_MaxLightSources;
  int m_MaxAnisotropicLevel;

	GLboolean m_bFontRestoreZTest;

  bool m_bDynVBFenceSet;
  uint m_DynVBFence;
  uint m_DynVBId;
  int m_nVertsDMesh3D;
  int m_nOffsDMesh3D;

public:	
  CGcontext m_CGContext;
  int m_MaxActiveTexturesARBFixed;
  int m_MaxActiveTexturesARB_VP;

	CGLRenderer();
	~CGLRenderer();
	
#ifndef PS2	
  virtual WIN_HWND Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen,WIN_HINSTANCE hinst, WIN_HWND Glhwnd=0, WIN_HDC Glhdc=0, WIN_HGLRC hGLrc=0, bool bReInit=false);
#else //PS2
  virtual bool Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen, bool bReInit=false);
#endif  //endif	
  virtual bool SetCurrentContext(WIN_HWND hWnd);
  virtual bool CreateContext(WIN_HWND hWnd, bool bAllowFSAA=false);
  virtual bool DeleteContext(WIN_HWND hWnd);

  virtual int  CreateRenderTarget (int nWidth, int nHeight, ETEX_Format eTF=eTF_8888) {return 0;}
  virtual bool DestroyRenderTarget (int nHandle) {return true;}
  virtual bool SetRenderTarget (int nHandle) {return true;}

	virtual void  ShareResources( IRenderer *renderer );

	virtual void	MakeCurrent();

	void	ShutDown(bool bReInit=false);

	virtual	bool	SetGammaDelta(const float fGamma);

	bool  LoadLibrary();
	void	FreeLibrary();
	void  FindProcs(bool bExt);
  void  FindProc( void*& ProcAddress, char* Name, char* SupportName, byte& Supports, bool AllowExt );
  bool  FindExt( const char* Name );

	virtual int	EnumDisplayFormats(TArray<SDispFormat>& Formats, bool bReset);

  //! Return all supported by video card video AA formats
  virtual int	EnumAAFormats(TArray<SAAFormat>& Formats, bool bReset);

  //! Changes resolution of the window/device (doen't require to reload the level
  virtual bool	ChangeResolution(int nNewWidth, int nNewHeight, int nNewColDepth, int nNewRefreshHZ, bool bFullScreen);
  virtual void	Reset (void) {};

  HWND  SetMode(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen,HINSTANCE hinst, HWND Glhwnd);
  bool  CheckOGLExtensions();
  void  GLSetDefaultState();

  void  ChangeLog();
	void	BeginFrame(void);
	void	Update(void);	
  virtual void GetMemoryUsage(ICrySizer* Sizer);

  virtual void  SetGamma(float fGamma, float fBrigtness, float fContrast);
  void  RestoreDeviceGamma(void);
  void  CheckGammaSupport(void);
  void  SetDeviceGamma(ushort *r, ushort *g, ushort *b);
  virtual void  RefreshResources(int nFlags);

  virtual char*	GetVertexProfile(bool bSupportedProfile);
  virtual char*	GetPixelProfile(bool bSupportedProfile);

  virtual void *GetDynVBPtr(int nVerts, int &nOffs, int Pool);
  virtual void DrawDynVB(int nOffs, int Pool, int nVerts);
  virtual void DrawDynVB(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pBuf, ushort *pInds, int nVerts, int nInds, int nPrimType);
	virtual CVertexBuffer	*CreateBuffer(int  buffersize,int vertexformat, const char *szSource, bool bDynamic=false);
	virtual void  CreateBuffer(int size, int vertexformat, CVertexBuffer *buf, int Type, const char *szSource);
	virtual void	ReleaseBuffer(CVertexBuffer *bufptr);
	virtual void	DrawBuffer(CVertexBuffer *src,SVertexStream *indicies,int numindices, int offsindex, int prmode,int vert_start=0,int vert_stop=0, CMatInfo *mi=NULL);
	virtual void	UpdateBuffer(CVertexBuffer *dest,const void *src,int size, bool bUnlock, int offs=0, int Type=0);
  
  virtual void  CreateIndexBuffer(SVertexStream *dest,const void *src,int indexcount);
  virtual void  UpdateIndexBuffer(SVertexStream *dest,const void *src,int indexcount, bool bUnLock=true);
  virtual void  ReleaseIndexBuffer(SVertexStream *dest);

	void	CheckError(const char *comment);
  const char* ErrorString( GLenum errorCode );

	void	SetCullMode	(int mode=R_CULL_BACK);
	bool	EnableFog	(bool enable);
	void	SetFog(float density,float fogstart,float fogend,const float *color,int fogmode);
	void	EnableTexGen(bool enable); 
	void	SetTexgen(float scaleX,float scaleY,float translateX,float translateY);
  void  SetTexgen3D(float x1, float y1, float z1, float x2, float y2, float z2);
	void	SetLodBias(float value);
	void	EnableVSync(bool enable);
  void  EnableAALines(bool bEnable);

	void	DrawTriStrip(CVertexBuffer *src, int vert_num);

	void	PushMatrix();
	void	RotateMatrix(float a,float x,float y,float z);
  void  RotateMatrix(const Vec3d & angels);
	void	TranslateMatrix(float x,float y,float z);
	void	ScaleMatrix(float x,float y,float z);
	void	TranslateMatrix(const Vec3d &pos);
  void  MultMatrix(float * mat);
	void	PopMatrix();
	void	LoadMatrix(const Matrix44 *src);

	void	EnableTMU(bool enable);	
	void	SelectTMU(int tnum);

	int	SetPolygonMode(int mode);

	void	Draw3dBBox(const Vec3d &mins,const Vec3d &maxs, int nPrimType);
	void	Draw3dPrim(const Vec3d &mins,const Vec3d &maxs, int nPrimType, const float pColor[4] = NULL);
  void  Flush3dBBox(const Vec3d &mins,const Vec3d &maxs,const bool bSolid);

	void	SetCamera(const CCamera &cam);
  void	SetViewport(int x=0, int y=0, int width=0, int height=0);
  void	SetScissor(int x=0, int y=0, int width=0, int height=0);

	virtual bool	ChangeDisplay(unsigned int width,unsigned int height,unsigned int cbpp);
  virtual void  ChangeViewport(unsigned int x,unsigned int y,unsigned int width,unsigned int height);

	virtual unsigned int DownLoadToVideoMemory(unsigned char *data,int w, int h, ETEX_Format eTFSrc, ETEX_Format eTFDst, int nummipmap, bool repeat=true, int filter=FILTER_BILINEAR, int Id=0, char *szCacheName=NULL, int flags=0);
  virtual void UpdateTextureInVideoMemory(uint tid, unsigned char *data,int posx,int posy,int w,int h,ETEX_Format eTF=eTF_0888);

//  CImage *TryLoadImage(const char *szFilename);
//	void	RemoveTexture(CImage *image);
	void	RemoveTexture(unsigned int TextureId);
  void  RemoveTexture(ITexPic * pTexPic);

	unsigned int LoadTexture(const char * filename,int *tex_type=NULL,unsigned int def_tid=0,bool compresstodisk=true,bool bWarn=true);

	virtual void	SetTexture(int tnum, ETexType eTT=eTT_Base);	
		
//	void	Draw2dImage(int xpos,int ypos,int w,int h,CImage *image,float s0=0,float t0=0,float s1=1,float t1=1,float angle=0,float r=1,float g=1,float b=1,float a=1);
  virtual void	Draw2dImage(float xpos,float ypos,float w,float h,int textureid,float s0=0,float t0=0,float s1=1,float t1=1,float angle=0,float r=1,float g=1,float b=1,float a=1,float z=1);
//	void	DrawImage(CImage *image,int xpos=0,int ypos=0);
	virtual void DrawImage(float xpos,float ypos,float w,float h,int texture_id,float s0,float t0,float s1,float t1,float r,float g,float b,float a);

  virtual void ResetToDefault();
  int  GenerateAlphaGlowTexture(float k);
	void SetMaterialColor(float r, float g, float b, float a);
	int	LoadAnimatedTexture(const char * format,const int nCount);
//  int Load3DTexture(const char * format,const int nCount);
	char * GetStatusText(ERendStats type);
  
  void ProjectToScreen(float ptx, float pty, float ptz, float *sx, float *sy, float *sz );
  int   UnProject(float sx, float sy, float sz, 
                float *px, float *py, float *pz,
                const float modelMatrix[16], 
                const float projMatrix[16], 
                const int    viewport[4]);
  int UnProjectFromScreen( float  sx, float  sy, float  sz, 
                           float *px, float *py, float *pz);

	void Draw2dLine	(float x1,float y1,float x2,float y2);
  void SetLineWidth(float fWidth) { glLineWidth(fWidth); m_fLineWidth = fWidth; }; // Put this in .cpp file.
  virtual void DrawLine(const Vec3d & vPos1, const Vec3d & vPos2);
  virtual void DrawLineColor(const Vec3d & vPos1, const CFColor & vColor1, const Vec3d & vPos2, const CFColor & vColor2);
  void DrawBall(float x, float y, float z, float radius);
  void DrawBall(const Vec3d & pos, float radius );
  virtual void DrawPoint(float x, float y, float z, float fSize = 0.0f);

  virtual void SetPerspective(const CCamera &cam);

  virtual void SetClipPlane( int id, float * params );

  //for editor 
  virtual void  GetModelViewMatrix(float *mat);
  virtual void  GetModelViewMatrix(double *mat);
  virtual void  GetProjectionMatrix(double *mat);
  virtual void  GetProjectionMatrix(float *mat);
  virtual Vec3d GetUnProject(const Vec3d &WindowCoords,const CCamera &cam);

  virtual void DrawObjSprites (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan);
  void DrawObjSprites_NoBend (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan);
  void DrawObjSprites_NoBend_Merge (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan);
  void DrawQuad(const Vec3d &right, const Vec3d &up, const Vec3d &origin,int nFlipMode=0);
  void DrawQuad(float dy,float dx, float dz, float x, float y, float z);
  void DrawQuad(float x0, float y0, float x1, float y1, const CFColor & color, float z);

  void ClearDepthBuffer();
  void ClearColorBuffer(const Vec3d vColor);
  void ReadFrameBuffer(unsigned char * pRGB, int nSizeX, int nSizeY, bool bBackBuffer, bool bRGBA, int nScaledX=-1, int nScaledY=-1);  

	void DrawTransparentQuad2D(float color);	

	//fog	
  void SetFogColor(float * color);
  void TransformTextureMatrix(float x, float y, float angle, float scale);
  void ResetTextureMatrix();

	//misc
	void ScreenShot(const char *filename=NULL);

  virtual uint MakeSprite(float object_scale, int tex_size, float angle, IStatObj * pStatObj, uchar * pTmpBuffer, uint def_tid);
  virtual uint Make3DSprite(int nTexSize, float fAngleStep, IStatObj * pStatObj);
  int GetPipWaterLevel();

  void UnloadOldTextures();

  void Set2DMode(bool enable, int ortox, int ortoy);
//  void TextToScreen(float x, float y, const char * format, ...);
  void PrintToScreen(float x, float y, float size, const char *buf);

  int ScreenToTexture();

  void SetFenceCompleted(CVertexBuffer * buffer);

  void SetTexClampMode(bool clamp);

  //void ClearStencilBuffer(int val);

//  int GetMaxActiveTexturesARB() { return m_MaxActiveTexturesARB; };

	virtual bool FontUploadTexture(class CFBitmap*, ETEX_Format eTF=eTF_8888);
	virtual	int  FontCreateTexture(int Width, int Height, byte *pData, ETEX_Format eTF=eTF_8888);
  virtual	bool FontUpdateTexture(int nTexId, int X, int Y, int USize, int VSize, byte *pData);
	virtual void FontReleaseTexture(class CFBitmap *pBmp);
	void FontSetTexture(class CFBitmap*, int nFilterMode);
  virtual void FontSetTexture(int nTexId, int nFilterMode);
	void FontSetRenderingState(unsigned long nVirtualScreenWidth, unsigned long nVirtualScreenHeight);
	void FontSetBlending(int src, int dst);
	void FontRestoreRenderingState();
  
private:

	bool	SetupPixelFormat(unsigned char colorbits,unsigned char zbits,unsigned char sbits,SRendContext *rc);	
	void	Print(CXFont *currfont,float x, float y, const char *buf,float xscale,float yscale,float r,float g,float b,float a=1.f);
  void	CheckGLError(const char *comment) { CheckError(comment); };
  int   _wglExtensionSupported(const char *extension);
  bool CreateRContext(SRendContext *rc, WIN_HDC Glhdc, WIN_HGLRC hGLrc, int cbpp, int zbpp, int sbits, bool bAllowFSAA);

#ifdef WIN32
  TArray<SRendContext *> m_RContexts;
  SRendContext *m_CurrContext;

	int			m_numvidmodes;
	DEVMODE		*m_vidmodes;			
#endif	

	bool	m_lod_biasSupported; 

	// VAR stuff
	void	InitVAR();
	void	ShutDownVAR();
	void * AllocateVarShunk(int bytes_count, const char *szSource);
	BOOL	ReleaseVarShunk(void * p);
	BOOL	IsVarPresent();
	void  GenerateVBLog(const char *szName);

	int		m_pip_buffer_size /*m_pip_water_level*/, m_var_valid;
	unsigned char * m_AGPbuf;
  struct alloc_info_struct { void * ptr; int bytes_num; bool busy; const char *szSource; };
  list2<alloc_info_struct> m_alloc_info;

  int m_nTempTextWidth,m_nTempTextHeight;


#define MAX_DYNAMIC_SHADOW_MAPS_COUNT 128
  struct ShadowMapTexInfo
  {
    ShadowMapTexInfo() { nTexId=0; pOwner=0; nLastFrameID=-1; nTexSize=0; dwFlags=0; }
    unsigned int nTexId;
    unsigned int nTexIdTemp;
    IEntityRender * pOwner;
    IStatObj * pOwnerGroup;
		int dwFlags;
    int nLastFrameID;
    int nTexSize;
  } m_ShadowTexIDBuffer[MAX_DYNAMIC_SHADOW_MAPS_COUNT];
  TArray<ShadowMapTexInfo> m_TempShadowTextures;

  // Shadows maps
  void PrepareDepthMap(ShadowMapFrustum * lof, bool make_new_tid=0);
  void ConfigShadowTexgen(int Num, int rangeMap, ShadowMapFrustum * pFrustum, float * pLightFrustumMatrix, float * pLightViewMatrix);
  void SetupShadowOnlyPass(int Num, ShadowMapFrustum * pFrustum, Vec3d * vShadowTrans, const float fShadowScale, Vec3d vObjTrans=Vec3d(0,0,0), float fObjScale=1.f, const Vec3d vObjAngles=Vec3d(0,0,0), Matrix44 * pObjMat=0);
  void BlurImage(int nSizeX, int nSizeY, int nType, int nTexId, int nTexIdTemp);
	void DrawAllShadowsOnTheScreen();
	void OnEntityDeleted(IEntityRender * pEntityRender);

  //========================================================================
  // Shaders pipeline

  virtual void DrawPoints(Vec3d v[], int nump, CFColor& col, int flags);
  virtual void DrawLines(Vec3d v[], int nump, CFColor& col, int flags, float fGround);
  void SetLogFuncs(bool set);

  int m_SizeBigArray;
  int m_SizeSysArray;
  GLfloat *m_BigArray;
  byte *m_SysArray;
  byte mCurTG[8];
  int m_EnableLights;

  virtual void EF_Release(int nFlags);
  virtual STexPic *EF_MakePhongTexture(int Exp);

  void EF_DrawREPreprocess(SRendItemPreprocess *ri, int Nums);

  void EF_InitWaveTables();
  void EF_InitRandTables();
  void EF_InitEvalFuncs(int nums);
  int  EF_RegisterFogVolume(float fMaxFogDist, float fFogLayerZ, CFColor color, int nIndex=-1, bool bCaustics=false);

  bool EF_AllocateBuffersVid();
  void EF_AllocateBuffers();
  void EF_PipelineInit();

  void EF_SetCameraInfo();

  static void EF_DrawWire();
  static void EF_DrawNormals();
  static void EF_DrawTangents();
  static void EF_Flush();

  void EF_DrawDebugLights();
  void EF_DrawDebugTools();
  
  void EF_Eval_DeformVerts(TArray<SDeform>* Defs);
  void EF_Eval_TexGen(SShaderPass *sfm);
  void EF_Eval_RGBAGen(SShaderPass *sfm);
  void EF_EvalNormalsRB(SShader *ef);

  STWarpZone *EF_SetWarpZone(SWarpSurf *sf, int *NumWarps, STWarpZone Warps[]);
  void EF_UpdateWarpZone(STWarpZone *wp, SWarpSurf *srf);
  bool EF_RenderWarpZone(STWarpZone *wp);
  bool EF_CalcWarpCamera(STWarpZone *wp, int nObject, CCamera& prevCam, CCamera& newCam);
  
  void EF_SetStateShaderState();
  void EF_ResetStateShaderState();
  bool EF_SetResourcesState(bool bSet);

  virtual void EF_PipelineShutdown();

  virtual void EF_LightMaterial(SLightMaterial *lm, int m_Flags);
  bool EF_SetLights(int Flags);
  void EF_SetHWLight(int Num, vec4_t Pos, CFColor& Diffuse, CFColor& Specular, float ca, float la, float qa);

  void EF_RenderPipeLine(void (*RenderFunc)());
  void EF_PipeLine(int nums, int nume, int nList, void (*RenderFunc)());
  void EF_FlushShader();
  void EF_PrintProfileInfo();
  void EF_Set2DMode(bool bSet, int orthox, int orthoy);
  virtual void EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, int nFog, CRendElement *re);
  virtual void EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, CRendElement *re);

  virtual void EF_CheckOverflow(int nVerts, int nTris, CRendElement *re);
  virtual void EF_EndEf3D (int nFlags);
  virtual void EF_EndEf2D(bool bSort);  // for 2d only
  virtual bool EF_SetLightHole(Vec3d vPos, Vec3d vNormal, int idTex, float fScale=1.0f, bool bAdditive=true);

  virtual void EF_PolygonOffset(bool bEnable, float fFactor, float fUnits);

//==========================================================================

public:  
  byte m_eCurColorOp[MAX_TMU];
  byte m_eCurAlphaOp[MAX_TMU];
  byte m_eCurColorArg[MAX_TMU];
  byte m_eCurAlphaArg[MAX_TMU];
  float m_fCurRGBScale[MAX_TMU];

  int m_nFrameCreateBuf;

  void EF_ApplyMatrixOps(TArray<SMatrixTransform>* MatrixOps, bool bEnable);

  void GLSetCull(ECull eCull);
  void EF_ClearBuffers(bool bForce, bool bOnlyDepth, float *Colors=NULL);

  void EF_SetObjectTransform(CCObject *obj, SShader *pSH, int nTransFlags);
  bool EF_ObjectChange(SShader *Shader, SRenderShaderResources *Res, int nObject, CRendElement *pRE);

  // Clip Planes support
  void EF_SetClipPlane (bool bEnable, float *pPlane, bool bRefract);

  void EF_SetColorOp(byte eCo, byte eAo, byte eCa, byte eAa);

  _inline void EF_SetGlobalColor(UCol& color)
  {
    m_RP.m_CurGlobalColor = color;
    EF_SelectTMU(0);
    EF_SetColorOp(255, 255, eCA_Texture | (eCA_Constant<<3), eCA_Texture | (eCA_Constant<<3));
  }

  _inline void EF_SetGlobalColor(float r, float g, float b, float a)
  {
    UCol color;
    color.bcolor[0] = (byte)(r * 255.0f);
    color.bcolor[1] = (byte)(g * 255.0f);
    color.bcolor[2] = (byte)(b * 255.0f);
    color.bcolor[3] = (byte)(a * 255.0f);
    m_RP.m_CurGlobalColor = color;
    EF_SelectTMU(0);
    EF_SetColorOp(255, 255, eCA_Texture | (eCA_Constant<<3), eCA_Texture | (eCA_Constant<<3));
  }
  _inline void EF_SetVertColor()
  {
    EF_SelectTMU(0);
    EF_SetColorOp(255, 255, eCA_Texture | (eCA_Diffuse<<3), eCA_Texture | (eCA_Diffuse<<3));
  }

  _inline void EF_FogCorrectionRestore(int bFogOverrided)
  {
    if (bFogOverrided)
    {
      if (bFogOverrided == 3)
      {
        glEnable(GL_FOG);
        m_FS.m_bEnable = true;
      }
      else
      {
        glFogfv(GL_FOG_COLOR, &m_FS.m_FogColor[0]);    
        if (bFogOverrided == 2)
        {
          glFogf(GL_FOG_END,   m_FS.m_FogEnd); 
          glFogf(GL_FOG_START, m_FS.m_FogStart); 
        }
      }
    }
  }
  _inline int EF_FogCorrection(bool bFogDisable, bool bFogVP)
  {
    int bFogOverride = 0;
    CFColor col;
    if (bFogDisable || bFogVP)
    {
      bFogDisable = true;
      if (m_FS.m_bEnable)
        glDisable(GL_FOG);
    }
    switch (m_CurState & GS_BLEND_MASK)
    {
      case GS_BLSRC_ONE | GS_BLDST_ONE:
        bFogOverride = 1;
        col = CFColor(0.0f, 0.0f, 0.0f, 0.0f);
        glFogfv(GL_FOG_COLOR,&col[0]);    
        break;
      case GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL:
        bFogOverride = 1;
        col = CFColor(0.5f, 0.5f, 0.5f, 0.0f);
        glFogfv(GL_FOG_COLOR,&col[0]);    
        break;
      case GS_BLSRC_ONE | GS_BLDST_ONEMINUSSRCALPHA:
        bFogOverride = 1;
        col = CFColor(0.0f, 0.0f, 0.0f, 0.0f);
        glFogfv(GL_FOG_COLOR,&col[0]);    
        break;
      case GS_BLSRC_ONE | GS_BLDST_ONEMINUSSRCCOL:
        bFogOverride = 1;
        col = CFColor(0.0f, 0.0f, 0.0f, 0.0f);
        glFogfv(GL_FOG_COLOR,&col[0]);    
        break;
      case GS_BLSRC_ZERO | GS_BLDST_ONEMINUSSRCCOL:
        bFogOverride = 1;
        col = CFColor(0.0f, 0.0f, 0.0f, 0.0f);
        glFogfv(GL_FOG_COLOR,&col[0]);    
        break;
      case GS_BLSRC_SRCALPHA | GS_BLDST_ONE:
        bFogOverride = 1;
        col = CFColor(0.0f, 0.0f, 0.0f, 0.0f);
        glFogfv(GL_FOG_COLOR,&col[0]);    
        break;
      case GS_BLSRC_ZERO | GS_BLDST_ONE:
        bFogOverride = 1;
        col = CFColor(0.0f, 0.0f, 0.0f, 0.0f);
        glFogfv(GL_FOG_COLOR,&col[0]);    
        break;
      case GS_BLSRC_DSTCOL | GS_BLDST_ZERO:
        bFogOverride = 2;
        col = CFColor(1.0f, 1.0f, 1.0f, 1.0f);
        glFogfv(GL_FOG_COLOR,&col[0]);    

        glFogf(GL_FOG_END,   gRenDev->m_FS.m_FogEnd*0.5f); 
        glFogf(GL_FOG_START, gRenDev->m_FS.m_FogStart*0.5f); 
        break;
    }
    if (bFogDisable)
      bFogOverride = 2;
    if (bFogVP)
      bFogOverride |= 4;
    return bFogOverride;
  }

  _inline void EF_SelectTMU(int Stage)
  {
    if (Stage != CTexMan::m_CurStage)
    {
      glActiveTextureARB(GL_TEXTURE0_ARB+Stage);
      glClientActiveTextureARB(GL_TEXTURE0_ARB+Stage);
      CTexMan::m_CurStage = Stage;
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

  _inline void EF_CommitTexTransforms(bool bEnable)
  {
    int i, j;
    int fl = m_RP.m_FlagsModificators;
    if (fl & RBMF_TCG)
    {
      for (i=0; i<4; i++)
      {
        if (fl & (RBMF_TCGOL0<<i))
        {
          EF_SelectTMU(i);
          if (bEnable)
          {
            SEfResTexture *pRT = gRenDev->m_RP.m_ShaderTexResources[i];
            float *fPlane = pRT->m_TexModificator.m_TexGenMatrix.GetData();
            for (j=0; j<4; j++)
            {
              glTexGeni(GL_S+j, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
              glTexGenfv(GL_S+j, GL_OBJECT_PLANE, fPlane);
              glEnable(GL_TEXTURE_GEN_S+j);
              fPlane += 4;
            }
          }
          else
          {
            for (j=0; j<4; j++)
            {
              glDisable(GL_TEXTURE_GEN_S+j);
            }
          }
        }

        if (fl & (RBMF_TCGRM0<<i))
        {
          EF_SelectTMU(i);
          if (bEnable)
          {
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
            glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
            glEnable(GL_TEXTURE_GEN_R);
          }
          else
          {
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_GEN_R);
          }
        }

        if (fl & (RBMF_TCGNM0<<i))
        {
          EF_SelectTMU(i);
          if (bEnable)
          {
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_NV);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_NV);
            glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_NV);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
            glEnable(GL_TEXTURE_GEN_R);
          }
          else
          {
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_GEN_R);
          }
        }

        if (fl & (RBMF_TCGSM0<<i))
        {
          EF_SelectTMU(i);
          if (bEnable)
          {
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
          }
          else
          {
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
          }
        }
      }
    }
    if (fl & RBMF_TCM)
    {
      for (i=0; i<4; i++)
      {
        if (fl & (RBMF_TCM0<<i))
        {
          EF_SelectTMU(i);
          glMatrixMode(GL_TEXTURE);
          if (bEnable)
          {
            SEfResTexture *pRT = gRenDev->m_RP.m_ShaderTexResources[i];
            glLoadMatrixf(pRT->m_TexModificator.m_TexMatrix.GetData());
          }
          else
            glLoadIdentity();
          glMatrixMode(GL_MODELVIEW);
        }
      }
    }
    if (!bEnable)
      m_RP.m_FlagsModificators &= ~(RBMF_TCM | RBMF_TCG);
  }
  _inline void EF_CommitTexStageState()
  {
    int i;
    m_RP.m_CurGlobalColor.dcolor = m_RP.m_NeedGlobalColor.dcolor;
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

  _inline void EF_CommitVLights()
  {
    uint i;
    if (m_RP.m_CurrentVLights != m_RP.m_EnabledVLights)
    {
      uint xorVL = (m_RP.m_CurrentVLights ^ m_RP.m_EnabledVLights) & 0xff;
      for (i=0; i<8; i++)
      {
        if (xorVL & (1<<i))
        {
          if (m_RP.m_CurrentVLights & (1<<i))
            glEnable(GL_LIGHT0 + i);
          else
            glDisable(GL_LIGHT0 + i);
          if ((uint)(1<<(i+1)) > xorVL)
            break;
        }
      }
      if (!m_RP.m_CurrentVLights && m_RP.m_EnabledVLights)
      {
        glDisable(GL_NORMALIZE);
        glDisable(GL_LIGHTING);
      }
      else
      if (m_RP.m_CurrentVLights && !m_RP.m_EnabledVLights)
      {
        glEnable(GL_NORMALIZE);
        glEnable(GL_LIGHTING);
      }
      m_RP.m_EnabledVLights = m_RP.m_CurrentVLights;
    }
    if (m_RP.m_EnabledVLights)
    {
      if ((m_RP.m_CurrentVLightFlags ^ m_RP.m_EnabledVLightFlags) & (LMF_NOADDSPECULAR | LMF_NOSPECULAR))
      {
        /*if (m_RP.m_CurrentVLightFlags & (LMF_NOADDSPECULAR | LMF_NOSPECULAR))
          m_pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
        else
          m_pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);*/
      }
      m_RP.m_EnabledVLightFlags = m_RP.m_CurrentVLightFlags;
    }
  }

  _inline void EF_CommitPS()
  {
    int fl = m_RP.m_PersFlags;
    if ( (fl & RBPF_TSNEEDSET) ^ ((fl & RBPF_TSWASSET)<<4) )
    {
      if (fl & RBPF_TSNEEDSET)
      {
        glEnable(GL_TEXTURE_SHADER_NV);
        m_RP.m_PersFlags |= RBPF_TSWASSET;
      }
      else
      {
        glDisable(GL_TEXTURE_SHADER_NV);
        m_RP.m_PersFlags &= ~RBPF_TSWASSET;
      }
    }
    if ( (fl & RBPF_PS1NEEDSET) ^ ((fl & RBPF_PS1WASSET)<<4) )
    {
      if (fl & RBPF_PS1NEEDSET)
      {
        glEnable(GL_REGISTER_COMBINERS_NV);
        m_RP.m_PersFlags |= RBPF_PS1WASSET;
      }
      else
      {
        glDisable(GL_REGISTER_COMBINERS_NV);
        m_RP.m_PersFlags &= ~RBPF_PS1WASSET;
      }
    }
    if ( (fl & RBPF_PS2NEEDSET) ^ ((fl & RBPF_PS2WASSET)<<4) )
    {
      if (fl & RBPF_PS2NEEDSET)
      {
        glEnable(GL_FRAGMENT_PROGRAM_ARB);
        m_RP.m_PersFlags |= RBPF_PS2WASSET;
      }
      else
      {
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
        m_RP.m_PersFlags &= ~RBPF_PS2WASSET;
      }
    }
  }
  _inline void EF_CommitVS()
  {
    int fl = m_RP.m_PersFlags;
    if ((fl & RBPF_VSNEEDSET) && !(fl & RBPF_VSWASSET))
    {
      glEnable(GL_VERTEX_PROGRAM_NV);
      m_RP.m_PersFlags |= RBPF_VSWASSET;
    }
    else
    if (!(fl & RBPF_VSNEEDSET))
    {
      if (fl & RBPF_VSWASSET)
      {
        glDisable(GL_VERTEX_PROGRAM_NV);
        m_RP.m_PersFlags &= ~RBPF_VSWASSET;
      }
      if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
        EF_CommitTexTransforms(true);
    }
  }

  int m_ArraysEnum[5];
  _inline void EF_CommitStreams()
  {
    // Set normals stream if required
    bool bResNormals = (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_bNeedNormals);
    if (bResNormals)
    {
      SArrayPointer_Normal vp;
      vp.ePT = eSrcPointer_TNormal;
      vp.mfSet(1);
    }


    // Enable/Disable array pointers
    if (SArrayPointer::m_CurEnabled != SArrayPointer::m_LastEnabled)
    {
      int i;
      int Cur = SArrayPointer::m_CurEnabled;
      int xorM = Cur ^ SArrayPointer::m_LastEnabled;
      int xorM1 = xorM & 0x1f;
      for (i=0; i<5; i++)
      {
        if (xorM1 & (1<<i))
        {
          if (Cur & (1<<i))
            glEnableClientState(m_ArraysEnum[i]);
          else
            glDisableClientState(m_ArraysEnum[i]);

          if ((1<<(i+1)) > xorM1)
            break;
        }
      }
      if (xorM & (0xff<<5))
      {
        for (i=0; i<m_numtmus; i++)
        {
          if (xorM & (0x20<<i))
          {
            EF_SelectTMU(i);
            if (Cur & (0x20<<i))
              glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            else
              glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            if ((0x20<<(i+1)) > xorM)
              break;
          }
        }
      }
      SArrayPointer::m_LastEnabled = SArrayPointer::m_CurEnabled;
    }
    if (SArrayPointer::m_LastEnabledPass && SArrayPointer::m_LastEnabledPass != SArrayPointer::m_CurEnabledPass)
    {
      int i;
      int Cur = SArrayPointer::m_CurEnabledPass;
      int xorM = Cur ^ SArrayPointer::m_LastEnabledPass;
      for (i=0; i<5; i++)
      {
        if (xorM & (1<<i))
        {
          if (!(Cur & (1<<i)))
            glDisableClientState(m_ArraysEnum[i]);

          if ((1<<(i+1)) > xorM)
            break;
        }
      }
      if (xorM & (0xff<<5))
      {
        for (i=0; i<m_numtmus; i++)
        {
          if (xorM & (0x20<<i))
          {
            if (!(Cur & (0x20<<i)))
            {
              EF_SelectTMU(i);
              glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }

            if ((1<<(i+1)) > xorM)
              break;
          }
        }
      }
      SArrayPointer::m_LastEnabledPass = SArrayPointer::m_CurEnabledPass;
    }
  }

  _inline int VB_BytesOffset(int nCurVB)
  {
    return m_RP.m_VidBufs[nCurVB].m_nOffs;
  }
  _inline int VB_BytesCount(int nCurVB)
  {
    return m_RP.m_VidBufs[nCurVB].m_nCount;
  }
  _inline void VB_Reset(int nCurVB)
  {
    m_RP.m_VidBufs[nCurVB].m_nOffs = m_RP.m_VidBufs[nCurVB].m_nCount;
  }
  _inline byte *VB_Lock(int nCurVB, int nSize, int& nOffs)
  {
    byte *pData;
    if (SUPPORTS_GL_ARB_vertex_buffer_object)
    {
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_RP.m_VidBufs[nCurVB].m_pVBDyn->m_VS[0].m_VertBuf.m_nID);
      pData = (byte *)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
      if (nSize+m_RP.m_VidBufs[nCurVB].m_nOffs > m_RP.m_VidBufs[nCurVB].m_nCount)
        m_RP.m_VidBufs[nCurVB].m_nOffs = 0;
      nOffs = m_RP.m_VidBufs[nCurVB].m_nOffs;
      pData += nOffs;
      m_RP.m_VidBufs[nCurVB].m_nOffs += nSize;
    }
    else
    if (SUPPORTS_GL_NV_fence)
    {
      if (nSize+m_RP.m_VidBufs[nCurVB].m_nOffs > m_RP.m_VidBufs[nCurVB].m_nCount)
      {
        if (!glTestFenceNV(m_RP.m_VidBufs[nCurVB].m_Fence))
        {
          m_RP.m_PS.m_NumNotFinishFences++;
          glFinishFenceNV(m_RP.m_VidBufs[nCurVB].m_Fence);
        }
        m_RP.m_VidBufs[nCurVB].m_nOffs = 0;
      }
      nOffs = m_RP.m_VidBufs[nCurVB].m_nOffs;
      pData = &m_RP.m_VidBufs[nCurVB].Ptr.PtrB[nOffs];
      m_RP.m_VidBufs[nCurVB].m_nOffs += nSize;
    }
    return pData;
  }
  _inline void VB_Unlock(int nCurVB)
  {
    if (SUPPORTS_GL_ARB_vertex_buffer_object)
    {
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_RP.m_VidBufs[nCurVB].m_pVBDyn->m_VS[0].m_VertBuf.m_nID);
      glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
    }
  }
  
  _inline void VB_Bind(int nStream)
  {
    int nOffsetV = m_RP.m_nStreamOffset[nStream];
    INT_PTR nBuf = m_RP.m_MergedStreams[nStream];
    if (SUPPORTS_GL_ARB_vertex_buffer_object)
    {
      switch(nStream)
      {
        case 0:
          {
            m_RP.m_nCurBufferID = m_RP.m_VidBufs[nBuf].m_pVBDyn->m_VS[0].m_VertBuf.m_nID;
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_RP.m_nCurBufferID);
            glVertexPointer(3, GL_FLOAT, m_RP.m_Stride, BUFFER_OFFSET(0+nOffsetV));
            SArrayPointer_Vertex::m_pLastPointer = (void *)m_RP.m_nCurBufferID;

            if (m_RP.m_OffsD)
            {
              glColorPointer(4, GL_UNSIGNED_BYTE, m_RP.m_Stride, BUFFER_OFFSET(m_RP.m_OffsD+nOffsetV));
              SArrayPointer_Color::m_pLastPointer = (void *)nBuf;
            }
            if (m_RP.m_OffsN)
            {
              glNormalPointer(GL_FLOAT, m_RP.m_Stride, BUFFER_OFFSET(m_RP.m_OffsN+nOffsetV));
              SArrayPointer_Normal::m_pLastPointer = (void *)nBuf;
            }
            if (m_RP.m_OffsT)
            {
              EF_SelectTMU(0);
              glTexCoordPointer(2, GL_FLOAT, m_RP.m_Stride, BUFFER_OFFSET(m_RP.m_OffsT+nOffsetV));
              SArrayPointer_Texture::m_pLastPointer[0] = (void *)nBuf;
              if (SArrayPointer_Texture::m_CurEnabled & (PFE_POINTER_TEX0<<1))
              {
                EF_SelectTMU(1);
                glTexCoordPointer(2, GL_FLOAT, m_RP.m_Stride, BUFFER_OFFSET(m_RP.m_OffsT+nOffsetV));
                SArrayPointer_Texture::m_pLastPointer[1] = (void *)nBuf;
              }
            }
          }
      	  break;
        case 1:
          { // Tangent vectors stream
            int Stride = sizeof(SPipTangents);
            EF_SelectTMU(2);
            glTexCoordPointer(3, GL_FLOAT, Stride, BUFFER_OFFSET(nOffsetV));
            SArrayPointer_Texture::m_pLastPointer[2] = (void *)nBuf;

            EF_SelectTMU(3);
            glTexCoordPointer(3, GL_FLOAT, Stride, BUFFER_OFFSET(nOffsetV+sizeof(Vec3)));
            SArrayPointer_Texture::m_pLastPointer[3] = (void *)nBuf;

            glNormalPointer(GL_FLOAT, Stride, BUFFER_OFFSET(nOffsetV+sizeof(Vec3)*2));
            SArrayPointer_Normal::m_pLastPointer = (void *)nBuf;
          }
          break;
        case 2:
          { // LM TC stream
            int Stride = sizeof(struct_VERTEX_FORMAT_TEX2F);
            EF_SelectTMU(1);
            glTexCoordPointer(2, GL_FLOAT, Stride, BUFFER_OFFSET(nOffsetV));
            SArrayPointer_Texture::m_pLastPointer[1] = (void *)nBuf;
          }
          break;
      }
    }
    else
    {
      byte *pData = &m_RP.m_VidBufs[nBuf].Ptr.PtrB[nOffsetV];
      switch(nStream)
      {
        case 0:
          {
            SArrayPointer_Vertex::m_pLastPointer = pData;
            glVertexPointer(3, GL_FLOAT, m_RP.m_Stride, pData);

            if (m_RP.m_OffsD)
            {
              SArrayPointer_Color::m_pLastPointer = pData+m_RP.m_OffsD;
              glColorPointer(4, GL_UNSIGNED_BYTE, m_RP.m_Stride, pData+m_RP.m_OffsD);
            }
            if (m_RP.m_OffsN)
            {
              SArrayPointer_Normal::m_pLastPointer = pData+m_RP.m_OffsN;
              glNormalPointer(GL_FLOAT, m_RP.m_Stride, pData+m_RP.m_OffsN);
            }
            if (m_RP.m_OffsT)
            {
              EF_SelectTMU(0);
              SArrayPointer_Texture::m_pLastPointer[0] = pData+m_RP.m_OffsT;
              glTexCoordPointer(2, GL_FLOAT, m_RP.m_Stride, pData+m_RP.m_OffsT);
              if (SArrayPointer_Texture::m_CurEnabled & (PFE_POINTER_TEX0<<1))
              {
                EF_SelectTMU(1);
                glTexCoordPointer(2, GL_FLOAT, m_RP.m_Stride, pData+m_RP.m_OffsT);
                SArrayPointer_Texture::m_pLastPointer[1] = pData+m_RP.m_OffsT;
              }
            }
          }
          break;
        case 1:
          { // Tangent vectors stream
            int Stride = sizeof(SPipTangents);
            EF_SelectTMU(2);
            SArrayPointer_Texture::m_pLastPointer[2] = pData;
            glTexCoordPointer(3, GL_FLOAT, Stride, pData);

            EF_SelectTMU(3);
            SArrayPointer_Texture::m_pLastPointer[3] = pData+sizeof(Vec3);
            glTexCoordPointer(3, GL_FLOAT, Stride, pData+sizeof(Vec3));

            SArrayPointer_Normal::m_pLastPointer = pData+sizeof(Vec3)*2;
            glNormalPointer(GL_FLOAT, Stride, pData+sizeof(Vec3)*2);
          }
          break;
        case 2:
          { // LM TC stream
            int Stride = sizeof(struct_VERTEX_FORMAT_TEX2F);
            EF_SelectTMU(1);
            SArrayPointer_Texture::m_pLastPointer[1] = pData;
            glTexCoordPointer(2, GL_FLOAT, Stride, pData);
          }
          break;
      }
    }
  }

  bool EF_PreDraw(SShaderPass *slw);

  _inline void EF_PostDraw()
  {
    if (!(m_RP.m_PersFlags & RBPF_VSWASSET))
    {
      if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
        EF_CommitTexTransforms(false);
    }
  }

  _inline void EF_Draw(SShader *sh, SShaderPass *sl)
  {
    if (!CV_r_nodrawshaders)
    {
      int bFogOverrided = 0;

      if (m_FS.m_bEnable)
        bFogOverrided = EF_FogCorrection(false, false);

      EF_PreDraw(sl);
      if (m_RP.m_pRE)
        m_RP.m_pRE->mfDraw(sh, sl);
      else
        EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
      EF_PostDraw();

      if (bFogOverrided)
      {
        glFogfv(GL_FOG_COLOR, &m_FS.m_FogColor[0]);    
        if (bFogOverrided == 2)
        {
          glFogf(GL_FOG_END,   m_FS.m_FogEnd); 
          glFogf(GL_FOG_START, m_FS.m_FogStart); 
        }
      }
    }
  }

  int m_sPrevX, m_sPrevY, m_sPrevWdt, m_sPrevHgt;
  bool m_bsPrev;
  void EF_Scissor(bool bEnable, int sX, int sY, int sWdt, int sHgt)
  {
    if (!CV_r_scissor)
      return;
    if (bEnable)
    {
      if (sX != m_sPrevX || sY != m_sPrevY || sWdt != m_sPrevWdt || sHgt != m_sPrevHgt)
      {
        m_sPrevX = sX;
        m_sPrevY = sY;
        m_sPrevWdt = sWdt;
        m_sPrevHgt = sHgt;
        glScissor(sX, m_height-sY-sHgt, sWdt, sHgt);
      }
      if (bEnable != m_bsPrev)
      {
        m_bsPrev = bEnable;
        glEnable (GL_SCISSOR_TEST);
      }
    }
    else
    {
      if (bEnable != m_bsPrev)
      {
        m_bsPrev = bEnable;
        m_sPrevWdt = 0;
        m_sPrevHgt = 0;
        glDisable (GL_SCISSOR_TEST);
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
        EF_DrawGeneralPasses(hs, ef, false, nStart, nEnd);
        break;
      case eSHP_DiffuseLight:
      case eSHP_SpecularLight:
      case eSHP_Light:
        EF_DrawLightPasses(hs, ef, nStart, nEnd);
        bLights = true;
        break;
      case eSHP_MultiLights:
        EF_DrawLightPasses_PS30(hs, ef, nStart, nEnd);
        bLights = true;
        break;
      case eSHP_MultiShadows:
        nEnd = EF_DrawMultiShadowPasses(hs, ef, nStart);
        bLights = true;
        break;
      case eSHP_Shadow:
        EF_DrawShadowPasses(hs, ef, nStart, nEnd);
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
  
  // Common draw functions
  void EF_DrawIndexedMesh (int nPrimType=R_PRIMV_TRIANGLES);
  
  void EF_DrawGeneralPasses(SShaderTechnique *hs, SShader *ef, bool bVolFog, int nStart, int nEnd);
  void EF_DrawGeometryInstancing_VS30(SShader *ef, SShaderPassHW *slw, CVProgram *curVP);
  void EF_DrawLightPasses(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd);
  void EF_DrawLightPasses_PS30(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd);
  void EF_DrawShadowPasses(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd);
  int  EF_DrawMultiShadowPasses(SShaderTechnique *hs, SShader *ef, int nStart);
  void EF_DrawFurPasses(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd, EShaderPassType eShPass);
  void EF_DrawSubsurfacePasses(SShaderTechnique *hs, SShader *ef);
  void EF_DrawLightShadowMask(int nLight);
  void EF_DrawFogOverlayPasses();
  void EF_DrawDetailOverlayPasses();

  void EF_FlushRefractedObjects(SShader *pSHRefr[], CRendElement *pRERefr[], CCObject *pObjRefr[], int nObjs, int nFlags, int DLDFlags);
  void EF_FlushHW();
  int  EF_Preprocess(SRendItemPre *ri, int First, int End);
  void EF_PreRender(int Stage);

  void EF_SetTexture(STexPic *tp);

//===================================================================
  
  int GetAnisotropicLevel()
  {
    if (GetFeatures() & RFT_ALLOWANISOTROPIC)
      return CV_r_texture_anisotropic_level;
    return 0;
  }


  // OpenGL extensions used in Cry OpenGL renderer
  static int CV_gl_useextensions;
  static int CV_gl_3dfx_gamma_control;
  static int CV_gl_arb_texture_compression;
  static int CV_gl_arb_multitexture;
  static int CV_gl_arb_pbuffer;
  static int CV_gl_arb_pixel_format;
  static int CV_gl_arb_buffer_region;
  static int CV_gl_arb_render_texture;
  static int CV_gl_arb_multisample;
  static int CV_gl_arb_vertex_program;
  static int CV_gl_arb_vertex_buffer_object;
  static int CV_gl_arb_fragment_program;
  static int CV_gl_arb_texture_env_combine;
  static int CV_gl_ext_swapcontrol;
  static int CV_gl_ext_bgra;
  static int CV_gl_ext_depth_bounds_test;
  static int CV_gl_ext_compiled_vertex_array;
  static int CV_gl_ext_texture_cube_map;
  static int CV_gl_ext_separate_specular_color;
  static int CV_gl_ext_secondary_color;
  static int CV_gl_ext_multi_draw_arrays;
  static int CV_gl_ext_paletted_texture;
  static int CV_gl_ext_stencil_two_side;
  static int CV_gl_ext_stencil_wrap;
  static int CV_gl_ext_texture_filter_anisotropic;
  static int CV_gl_ext_texture_env_add;
  static int CV_gl_ext_texture_env_combine;
  static int CV_gl_ext_texture_rectangle;
  static int CV_gl_hp_occlusion_test;
  static int CV_gl_nv_fog_distance;
  static int CV_gl_nv_texture_env_combine4;
  static int CV_gl_nv_point_sprite;
  static int CV_gl_nv_vertex_array_range;
  static int CV_gl_nv_fence;
  static int CV_gl_nv_register_combiners;
  static int CV_gl_nv_register_combiners2;
  static int CV_gl_nv_texgen_reflection;
  static int CV_gl_nv_texgen_emboss;
  static int CV_gl_nv_vertex_program;
  static int CV_gl_nv_vertex_program3;
  static int CV_gl_nv_texture_rectangle;
  static int CV_gl_nv_texture_shader;
  static int CV_gl_nv_texture_shader2;
  static int CV_gl_nv_texture_shader3;
  static int CV_gl_nv_fragment_program;
  static int CV_gl_nv_multisample_filter_hint;
  static int CV_gl_sgix_depth_texture;
  static int CV_gl_sgix_shadow;
  static int CV_gl_sgis_generate_mipmap;
  static int CV_gl_sgis_texture_lod;
  static int CV_gl_ati_separate_stencil;
  static int CV_gl_ati_fragment_shader;

  static int CV_gl_alpha_bits;

  static int CV_gl_psforce11;
  static int CV_gl_vsforce11;
  static int CV_gl_nv30_ps20;

  static int CV_gl_swapOnStart;
  static int CV_gl_clipplanes;

  static float CV_gl_normalmapscale;

  static float CV_gl_offsetfactor;
  static float CV_gl_offsetunits;

  static float CV_gl_pip_allow_var;
  static float CV_gl_pip_buff_size;

  static int CV_gl_rb_verts;
  static int CV_gl_rb_tris;

  static int CV_gl_maxtexsize;
  static int CV_gl_squaretextures;
  static int CV_gl_mipprocedures;
  static ICVar *CV_gl_texturefilter;

  virtual void MakeMatrix(const Vec3d & pos, const Vec3d & angles,const Vec3d & scale, Matrix44 * mat);
  
  struct TexInfo 
  { 
    char sTexName[128];
    int nId;
    ITexPic * pPic;
  };
  list2<TexInfo> m_lstLoadedTexInfo;
//  static float gCurrDynShadowMapOffset;

  const char * GetBufferComment(CVertexBuffer * dest);

//  void ClearAlphaBuffer(float fAlphaValue);

  virtual WIN_HWND GetHWND();
	virtual void SetColorOp(byte eCo, byte eAo, byte eCa, byte eAa);
};

//=============================================================================

struct SGLFuncs
{
  static int gluProject(float objx, float objy, float objz, const float model[16], const float proj[16], const int viewport[4], float * winx, float * winy, float * winz);
  static int gluUnProject(float winx, float winy, float winz, const float model[16], const float proj[16], const int viewport[4], float * objx, float * objy, float * objz);
  static void gluLookAt(float eyex, float eyey, float eyez, float centerx, float centery, float centerz, float upx, float upy, float upz, float *m);

  static void glTranslate(float *m, float x, float y, float z);
  static void glRotate(float *m, float a, float x, float y, float z);
};

extern CGLRenderer *gcpOGL;

class CGLTexMan : public CTexMan
{
  void GenerateMips_SW(GLenum tgt, byte* src, int wdt, int hgt, int mode, STexPic *ti);
  void BuildMips(GLenum tgt, byte* src, int wdt, int hgt, int depth, STexPic *ti, int srcFormat, int dstFormat, int blockSize, int DXTSize, int nMips);
  void BuildMips8(GLenum tgt, STexPic *ti, byte *data, bool bSub);
  void BuildMipsSub(byte* src, int wdt, int hgt);
  void BuildMipsSub_DSDT(byte* src, int wdt, int hgt);
  _inline void CGLTexMan::mfMakeS8T8_EdgePix(int x, int y, byte *src, byte *dst, int wdt, int hgt, int lSrcPitch, int lDstPitch);
  void MakeNormalizeVectorCubeMap(int size, STexPic *tp);
  void GetCubeVector(int i, int cubesize, int x, int y, float *vector);

  void DrawCubeSide( const float *angle, Vec3d& Pos, int tex_size, int side, int RendFlags);
  void CreateBufRegion(int Width, int Height);
  void ClearBuffer(int Width, int Height, bool bEnd, STexPic *pImage, int Side);

  void AmplifyGlare(SByteColor *glarepixels, int width, int height);
  void SmoothGlare(SByteColor *src, int src_w, int src_h, SLongColor *dst);
  void BlurGlare(SByteColor *src, int src_w, int src_h, SByteColor *dst, SLongColor *p, int boxw, int boxh);

  STextureTarget *CreateTextureTarget(int m_Bind, int Width, int Height);

  void GenerateFlareMap();
  void GenerateFogMaps();
  void GenerateGhostMap();
  void GenerateDepthLookup();
  float CalcFogVal(float fi, float fj);

  CPBuffer *m_EnvPBuffer;
  CPBuffer *m_PBuffer_256;
  TArray<SBufRegion> m_BufRegions;
  TArray<STextureTarget> m_TexTargets;
  int m_TempX, m_TempY, m_TempWidth, m_TempHeight;
  ETEX_Format m_CurCubemapFormat;
  CCamera m_PrevCamera;

protected:
  virtual STexPic *CreateTexture(const char *name, int wdt, int hgt, int Depth, uint flags, uint flags2, byte *dst, ETexType eTT, float fAmount1=-1.0f, float fAmount2=-1.0f, int DXTSize=0, STexPic *ti=NULL, int bind=0, ETEX_Format eTF=eTF_8888, const char *szSourceName=NULL);
  virtual STexPic *CopyTexture(const char *name, STexPic *ti, int CubeSide=-1);

public:  
  static int TexSize(int wdt, int hgt, int depth, int mode);
  static int GetTexSrcFormat(ETEX_Format eTF);
  static int GetTexDstFormat(ETEX_Format eTF);
  static void CalcMipsAndSize(STexPic *ti);
  static ETEX_Format GetTexFormat(int GLFormat);
  static BindNULL(int From)
  {
    int n = CTexMan::m_nCurStages;
    CTexMan::m_nCurStages = From;
    for (; From<n; From++)
    {
      if (CGLTexMan::m_TUState[From].m_Target)
      {
        gcpOGL->m_RP.m_FlagsModificators &= ~((RBMF_TCM0 | RBMF_TCGOL0 | RBMF_TCGRM0 | RBMF_TCGNM0 | RBMF_TCGSM0)<<From);
        if (From < gcpOGL->m_MaxActiveTexturesARBFixed)
        {
          gcpOGL->EF_SelectTMU(From);
          glDisable(CGLTexMan::m_TUState[From].m_Target);
          if (gcpOGL->m_RP.m_ClipPlaneEnabled==1 && SUPPORTS_GL_NV_texture_shader)
            glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_NONE);
        }
        CGLTexMan::m_TUState[From].m_Bind = 0;
        CGLTexMan::m_TUState[From].m_Target = 0;
      }
    }
    // Restore clip-planes status
    if (gcpOGL->m_RP.m_ClipPlaneWasOverrided && CTexMan::m_nCurStages<4 && SUPPORTS_GL_NV_texture_shader)
    {
      gcpOGL->m_RP.m_ClipPlaneWasOverrided = 1;
      gcpOGL->m_RP.m_ClipPlaneEnabled = 1;
      int nTMUs = min(gcpOGL->m_numtmus, 4);
      for (int i=0; i<nTMUs; i++)
      {
        gcpOGL->EF_SelectTMU(i);
        if (i == 3)
          glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_CULL_FRAGMENT_NV);
        else
          glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, CGLTexMan::m_TUState[i].m_Target);
      }
      glEnable(GL_TEXTURE_SHADER_NV);
    }
  }

  void MakePhongLookupTexture(float shininess, STexPic *ti);

  static _inline void SetTextureTarget(int Stage, int Target)
  {
    if (Target != CGLTexMan::m_TUState[Stage].m_Target)
    {
      if (CTexMan::m_CurStage < gcpOGL->m_MaxActiveTexturesARBFixed)
      {
        if (gcpOGL->m_RP.m_ClipPlaneEnabled==1 && SUPPORTS_GL_NV_texture_shader)
        {
          if (Stage == 3)
          {
            gcpOGL->m_RP.m_ClipPlaneEnabled = 0;
            gcpOGL->m_RP.m_ClipPlaneWasOverrided = 1;
            glDisable(GL_TEXTURE_SHADER_NV);
          }
          else
            glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, Target);
        }
        if (CGLTexMan::m_TUState[Stage].m_Target)
          glDisable(CGLTexMan::m_TUState[Stage].m_Target);
        glEnable(Target);
      }
      CGLTexMan::m_TUState[Stage].m_Target = Target;
    }
  }
  static _inline void ResetTextureTarget(int Stage)
  {
    if (CGLTexMan::m_TUState[Stage].m_Target)
    {
      if (CTexMan::m_CurStage < gcpOGL->m_MaxActiveTexturesARBFixed)
        glDisable(CGLTexMan::m_TUState[Stage].m_Target);
      CGLTexMan::m_TUState[Stage].m_Target = 0;
      if (gcpOGL->m_RP.m_ClipPlaneEnabled==1 && SUPPORTS_GL_NV_texture_shader)
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_NONE);
    }
  }

  CGLTexMan() : CTexMan()
  {
    m_EnvPBuffer = NULL;
    m_PBuffer_256 = NULL;
    m_CurCubemapBind = 0;
  }
  virtual ~CGLTexMan();
  
  virtual byte *GenerateDXT_HW(STexPic *ti, EImFormat eF, byte *dst, int *numMips, int *DXTSize, bool bMips=true);
  virtual void SetTexture(int Id, ETexType eTT);
  virtual bool SetFilter(char *filt);
  virtual void UpdateTextureData(STexPic *pic, byte *data, int USize, int VSize, bool bProc, int State, bool bPal);
  virtual void UpdateTextureRegion(STexPic *pic, byte *data, int X, int Y, int USize, int VSize);
  virtual STexPic *CreateTexture();
  virtual void GetAverageColor(SEnvTexture *cm, int nSide);
  virtual bool ScanEnvironmentCM (const char *name, int size, Vec3d& Pos);
  virtual void ScanEnvironmentCube(SEnvTexture *cm, int RendFlags, int Size, bool bLightCube);
  virtual void ScanEnvironmentTexture(SEnvTexture *cm, SShader *pSH, SRenderShaderResources *pRes, int RendFlags, bool bUseExistingREs);
  virtual void EndCubeSide(CCObject *obj, bool bNeedClear);
  virtual void StartCubeSide(CCObject *obj);
  virtual void DrawToTexture(Plane& Pl, STexPic *Tex, int RendFlags);
  virtual void DrawToTextureForDof(int Id) {};
  virtual void DrawToTextureForGlare(int Id);
  virtual void DrawToTextureForRainMap(int Id);
  virtual void StartHeatMap(int Id);
  virtual void EndHeatMap();
  virtual void StartRefractMap(int Id);
  virtual void EndRefractMap();
  virtual void StartNightMap(int Id);
  virtual void EndNightMap();
  virtual void DrawFlashBangMap(int Id, int RendFlags, CREFlashBang *pRE);

  // tiago: added
  virtual void StartScreenTexMap(int Id);
  virtual void EndScreenTexMap();
  virtual bool PreloadScreenFxMaps(void)  { return 0; }

  virtual void StartScreenMap(int Id);
  virtual void EndScreenMap();
  virtual void Update();
  virtual void GenerateFuncTextures();

  virtual STexPic *AddToHash(int Id, STexPic *ti);
  virtual void RemoveFromHash(int Id, STexPic *ti);
  virtual STexPic *GetByID(int Id);

  static TTextureMap m_RefTexs;

  static SGLTexUnit m_TUState[];

#ifdef DEBUGALLOC
#undef new
#endif
  void* operator new( size_t Size )
  {
    void *ptr = malloc(Size);
    memset(ptr, 0, Size);
    return ptr;
  }
  void operator delete( void *Ptr )
  {
    free (Ptr);
  }
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
};


#endif //gl_renderer