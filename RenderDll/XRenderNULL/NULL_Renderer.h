
//////////////////////////////////////////////////////////////////////
//
//  Crytek CryENGINE Source code
//  
//  File:PS2_Renderer.h - PS2 API 
//
//  History:
//  -Jan 31,2001:Originally created by Marco Corbetta
//  -: taken over by Andrey Khonich
//
//////////////////////////////////////////////////////////////////////

#ifndef PS2_RENDERER_H
#define PS2_RENDERER_H

#if _MSC_VER > 1000
# pragma once
#endif

/*
===========================================
The NULLRenderer interface Class
===========================================
*/

#define MAX_TEXTURE_STAGES 4

#include "list2.h"

typedef std::map<int,STexPic*> TTextureMap;
typedef TTextureMap::iterator TTextureMapItor;


//////////////////////////////////////////////////////////////////////
class CNULLRenderer : public CRenderer
{
  friend class CNULLTexMan;

public: 

  CNULLRenderer();
  ~CNULLRenderer();
  
#ifndef PS2 
  virtual WIN_HWND Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen,WIN_HINSTANCE hinst, WIN_HWND Glhwnd=0, WIN_HDC Glhdc=0, WIN_HGLRC hGLrc=0, bool bReInit=false);
#else //PS2
  virtual bool Init(int x,int y,int width,int height,unsigned int cbpp, int zbpp, int sbits, bool fullscreen, bool bReInit=false);
#endif  //endif 
  virtual WIN_HWND GetHWND();
  virtual bool SetCurrentContext(WIN_HWND hWnd);
  virtual bool CreateContext(WIN_HWND hWnd, bool bAllowFSAA=false);
  virtual bool DeleteContext(WIN_HWND hWnd);

  virtual int  CreateRenderTarget (int nWidth, int nHeight, ETEX_Format eTF=eTF_8888) {return 0;}
  virtual bool DestroyRenderTarget (int nHandle) {return true;}
  virtual bool SetRenderTarget (int nHandle) {return true;}

  virtual void  ShareResources( IRenderer *renderer );

  virtual void  MakeCurrent();

  virtual void  ShutDown(bool bReInit=false);

  virtual bool  SetGammaDelta(const float fGamma);

  virtual void  BeginFrame(void);
  virtual void  Update(void); 
  virtual void  GetMemoryUsage(ICrySizer* Sizer);
  virtual void  RefreshResources(int nFlags);
  virtual int   EnumDisplayFormats(TArray<SDispFormat>& Formats, bool bReset);
  //! Return all supported by video card video AA formats
  virtual int	EnumAAFormats(TArray<SAAFormat>& Formats, bool bReset) { return 0; }

  virtual bool	ChangeResolution(int nNewWidth, int nNewHeight, int nNewColDepth, int nNewRefreshHZ, bool bFullScreen);
  virtual void	Reset (void) {};
  virtual	void	CheckError(const char *comment);

  virtual void *GetDynVBPtr(int nVerts, int &nOffs, int Pool);
  virtual void DrawDynVB(int nOffs, int Pool, int nVerts);
  virtual void DrawDynVB(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pBuf, ushort *pInds, int nVerts, int nInds, int nPrimType);
  virtual CVertexBuffer	*CreateBuffer(int  buffersize,int vertexformat, const char *szSource, bool bDynamic=false);
  virtual void  CreateBuffer(int size, int vertexformat, CVertexBuffer *buf, int Type, const char *szSource);
  virtual void	ReleaseBuffer(CVertexBuffer *bufptr);
  virtual void	DrawBuffer(CVertexBuffer *src,SVertexStream *indicies,int numindices, int offsindex, int prmode,int vert_start=0,int vert_stop=0, CMatInfo *mi=NULL);
  virtual void	UpdateBuffer(CVertexBuffer *dest,const void *src,int size, bool bUnlock, int offs=0, int Type=0);
  virtual void  DrawTriStrip(CVertexBuffer *src, int vert_num);

  virtual void  CreateIndexBuffer(SVertexStream *dest,const void *src,int indexcount);
  virtual void  UpdateIndexBuffer(SVertexStream *dest,const void *src,int indexcount, bool bUnLock=true);
  virtual void  ReleaseIndexBuffer(SVertexStream *dest);

  virtual void  SetFenceCompleted(CVertexBuffer * buffer);

  // low-level Render-states
  virtual void  SetCullMode (int mode=R_CULL_BACK);
  virtual void  EnableTexGen(bool enable); 
  virtual void  SetTexgen(float scaleX,float scaleY,float translateX,float translateY);
  virtual void  SetTexgen3D(float x1, float y1, float z1, float x2, float y2, float z2);
  virtual void  SetLodBias(float value);
  virtual void  EnableVSync(bool enable);
  virtual void  EnableAALines(bool bEnable);
  virtual int   SetPolygonMode(int mode);
  virtual void  SetClipPlane(int id, float * params);

  // global fog 
  virtual void  SetFogColor(float * color);
  virtual bool  EnableFog (bool enable);
  virtual void  SetFog(float density,float fogstart,float fogend,const float *color,int fogmode);

  // matrix/camera/viewport manipulations
  virtual void  PushMatrix();
  virtual void  RotateMatrix(float a,float x,float y,float z);
  virtual void  RotateMatrix(const Vec3d & angels);
  virtual void  TranslateMatrix(float x,float y,float z);
  virtual void  ScaleMatrix(float x,float y,float z);
  virtual void  TranslateMatrix(const Vec3d &pos);
  virtual void  MultMatrix(float * mat);
  virtual void  PopMatrix();
  virtual void  LoadMatrix(const Matrix44 *src);
  virtual void  ProjectToScreen(float ptx, float pty, float ptz, float *sx, float *sy, float *sz );
  virtual int   UnProject(float sx, float sy, float sz, float *px, float *py, float *pz, const float modelMatrix[16], const float projMatrix[16], const int viewport[4]);
  virtual int   UnProjectFromScreen( float  sx, float  sy, float  sz, float *px, float *py, float *pz);
  virtual void  GetModelViewMatrix(float *mat);
  virtual void  GetModelViewMatrix(double *mat);
  virtual void  GetProjectionMatrix(double *mat);
  virtual void  GetProjectionMatrix(float *mat);
  virtual Vec3d GetUnProject(const Vec3d &WindowCoords,const CCamera &cam);
  virtual void  Set2DMode(bool enable, int ortox, int ortoy);
  virtual void  SetCamera(const CCamera &cam);
  virtual void  SetViewport(int x=0, int y=0, int width=0, int height=0);
  virtual void  SetScissor(int x=0, int y=0, int width=0, int height=0);
  virtual bool  ChangeDisplay(unsigned int width,unsigned int height,unsigned int cbpp);
  virtual void  ChangeViewport(unsigned int x,unsigned int y,unsigned int width,unsigned int height);
  virtual void  SetPerspective(const CCamera &cam);

  // draw helper functions
  virtual void  DrawObjSprites (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan);
  virtual void  DrawQuad(const Vec3d &right, const Vec3d &up, const Vec3d &origin,int nFlipMode=0);
  virtual void  DrawQuad(float dy,float dx, float dz, float x, float y, float z);
  virtual void  Draw3dBBox(const Vec3d &mins,const Vec3d &maxs, int nPrimType);
  virtual void  Flush3dBBox(const Vec3d &mins,const Vec3d &maxs,const bool bSolid);
  virtual void  WriteXY(CXFont *currfont,int x,int y, float xscale,float yscale,float r,float g,float b,float a,const char *message, ...);  
  virtual void	Draw2dImage(float xpos,float ypos,float w,float h,int textureid,float s0=0,float t0=0,float s1=1,float t1=1,float angle=0,float r=1,float g=1,float b=1,float a=1,float z=1);
  virtual void  DrawImage(float xpos,float ypos,float w,float h,int texture_id,float s0,float t0,float s1,float t1,float r,float g,float b,float a);
  virtual void  Draw2dLine  (float x1,float y1,float x2,float y2);
  virtual void  DrawLine(const Vec3d & vPos1, const Vec3d & vPos2);
  virtual void  DrawLineColor(const Vec3d & vPos1, const CFColor & vColor1, const Vec3d & vPos2, const CFColor & vColor2);
  virtual void  DrawBall(float x, float y, float z, float radius);
  virtual void  DrawBall(const Vec3d & pos, float radius );
  virtual void  DrawPoint(float x, float y, float z, float fSize = 0.0f);

  virtual char*	GetVertexProfile(bool bSupported) {return "NONE";}
  virtual char*	GetPixelProfile(bool bSupported) {return "NONE";}

  // basic textures operations
  virtual unsigned int DownLoadToVideoMemory(unsigned char *data,int w, int h, ETEX_Format eTFSrc, ETEX_Format eTFDst, int nummipmap, bool repeat=true, int filter=FILTER_BILINEAR, int Id=0, char *szCacheName=NULL, int flags=0);
  virtual void  UpdateTextureInVideoMemory(uint tid, unsigned char *data,int posx,int posy,int w,int h,ETEX_Format eTF=eTF_0888);
  virtual void  RemoveTexture(unsigned int TextureId);
  virtual void  RemoveTexture(ITexPic * pTexPic);
  virtual uint  LoadTexture(const char * filename,int *tex_type=NULL,unsigned int def_tid=0,bool compresstodisk=true,bool bWarn=true);
  virtual void  SetTexture(int tnum, ETexType eTT=eTT_Base);  
  virtual void  SetTexture3D(int tid3d);
  virtual int   LoadAnimatedTexture(const char * format,const int nCount);
  virtual int   GenerateAlphaGlowTexture(float k);
  virtual uint  MakeSprite(float object_scale, int tex_size, float angle, IStatObj * pStatObj, uchar * pTmpBuffer, uint def_tid);
  virtual uint  Make3DSprite(int nTexSize, float fAngleStep, IStatObj * pStatObj);
  virtual void  TransformTextureMatrix(float x, float y, float angle, float scale);
  virtual void  ResetTextureMatrix();
  virtual int   ScreenToTexture();
  virtual void  SetTexClampMode(bool clamp);
  virtual void  MakeMatrix(const Vec3d & pos, const Vec3d & angles,const Vec3d & scale, Matrix44 * mat);

  virtual void DrawPoints(Vec3d v[], int nump, CFColor& col, int flags) {}
  virtual void DrawLines(Vec3d v[], int nump, CFColor& col, int flags, float fGround) {}

  // texture unit manupulations
  virtual void  EnableTMU(bool enable); 
  virtual void  SelectTMU(int tnum);

  // misc functions
  virtual void  ResetToDefault();
  virtual void  ScreenShot(const char *filename=NULL);
  virtual void  ClearDepthBuffer();
  virtual void  ClearColorBuffer(const Vec3d vColor);
  virtual void  ReadFrameBuffer(unsigned char * pRGB, int nSizeX, int nSizeY, bool bBackBuffer, bool bRGBA, int nScaledX=-1, int nScaledY=-1);  
  virtual void  SetMaterialColor(float r, float g, float b, float a);
  virtual char *GetStatusText(ERendStats type);
  
  // font functions
  virtual bool  FontUploadTexture(class CFBitmap*, ETEX_Format eTF=eTF_8888);
  virtual	int   FontCreateTexture(int Width, int Height, byte *pData, ETEX_Format eTF=eTF_8888) { return -1; };
  virtual	bool  FontUpdateTexture(int nTexId, int X, int Y, int USize, int VSize, byte *pData);
  virtual void  FontReleaseTexture(class CFBitmap *pBmp);
  virtual void  FontSetTexture(class CFBitmap*, int nFilterMode);
  virtual void  FontSetTexture(int nTexId, int nFilterMode) {};
  virtual void  FontSetRenderingState(unsigned long nVirtualScreenWidth, unsigned long nVirtualScreenHeight);
  virtual void  FontSetBlending(int src, int dst);
  virtual void  FontRestoreRenderingState();
  virtual void  FontSetState(bool bRestore);
  virtual void  PrintToScreen(float x, float y, float size, const char *buf);
  virtual void  DrawString(int x, int y,bool bIgnoreColor,const char *message, ...) {};

  void PrepareDepthMap(ShadowMapFrustum * lof, bool make_new_tid);
  virtual void SetupShadowOnlyPass(int Num, ShadowMapFrustum * pFrustum, Vec3d * vShadowTrans, const float fShadowScale, Vec3d vObjTrans=Vec3d(0,0,0), float fObjScale=1.f, const Vec3d vObjAngles=Vec3d(0,0,0), Matrix44 * pObjMat=0);
	virtual void OnEntityDeleted(IEntityRender * pEntityRender);
	void DrawAllShadowsOnTheScreen();

  virtual void EF_PolygonOffset(bool bEnable, float fFactor, float fUnits) {};

private:
  void PS2SetDefaultState();
  void SetGamma(float fGamma);

  //========================================================================
  // Shaders pipeline

  void PS2SetCull(ECull eCull);

  void EF_InitWaveTables();
  void EF_InitRandTables();
  void EF_InitEvalFuncs(int nums);
  void EF_ClearBuffers(bool bForce=false, float *Colors=NULL);
  void EF_SetCameraInfo();
  void EF_SetObjectTransform(CCObject *obj);
  bool EF_ObjectChange(SShader *Shader, int nObject, CRendElement *re);
  void EF_PreRender(int Stage);

  void EF_Eval_DeformVerts(TArray<SDeform>* Defs);
  void EF_Eval_TexGen(SShaderPass *sfm);
  void EF_Eval_RGBAGen(SShaderPass *sfm);
  void EF_EvalNormalsRB(SShader *ef);

  void EF_DrawIndexedMesh (int nPrimType);
  void EF_FlushShader();
  void EF_RenderPipeLine(void (*RenderFunc)());
  void EF_PipeLine(int nums, int nume, int nList, int nSortType, void (*RenderFunc)());
  int  EF_Preprocess(SRendItemPre *ri, int First, int End);
  void EF_PrintProfileInfo();

  void EF_DrawDebugLights();
  void EF_DrawDebugTools();
  void EF_DrawFogOverlay();
  void EF_DrawDetailOverlay();
  void EF_DrawDecalOverlay();

  static void EF_Flush();
  static void EF_DrawWire();
  static void EF_DrawNormals();
  static void EF_DrawTangents();

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

  // init render pipeline
  void EF_PipelineInit();

  // shutdown render pipeline
  void EF_PipelineShutdown();

public:
  void EF_SetClipPlane (bool bEnable, float *pPlane, bool bRefract);
  void EF_SetColorOp(byte eCO);
  void EF_CalcObjectMatrix(CCObject *obj);

public:
  virtual STexPic *EF_MakePhongTexture(int Exp);
  virtual void EF_CheckOverflow(int nVerts, int nTris, CRendElement *re);
  virtual void EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, int nFog, CRendElement *re);
  virtual void EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, CRendElement *re);
  virtual bool EF_SetLightHole(Vec3d vPos, Vec3d vNormal, int idTex, float fScale=1.0f, bool bAdditive=true);
  virtual void EF_Release(int nFlags);
  virtual int  EF_RegisterFogVolume(float fMaxFogDist, float fFogLayerZ, CFColor color, int nIndex=-1, bool bCaustics=false);
  virtual void EF_LightMaterial(SLightMaterial *lm, int Flags);
	virtual void SetColorOp(byte eCo, byte eAo, byte eCa, byte eAa){};

  // Draw all shaded Render items
  virtual void EF_EndEf3D (int nFlags);

  /////////////////////////////////////////////////////////////////////////////////
  // 2d interface for the shaders
  /////////////////////////////////////////////////////////////////////////////////
  virtual void EF_EndEf2D(bool bSort);

};

//=============================================================================

extern CNULLRenderer *gcpNULL;

struct CNULLTexUnit
{
  int m_Bind;
  int m_Target;
  float m_fTexFilterLodBias;
};

class CNULLTexMan : public CTexMan
{

protected:
  virtual STexPic *CreateTexture(const char *name, int wdt, int hgt, int Depth, uint flags, uint flags2, byte *dst, ETexType eTT, float fAmount1=-1.0f, float fAmount2=-1.0f, int DXTSize=0, STexPic *ti=NULL, int bind=0, ETEX_Format eTF=eTF_8888, const char *szSourceName=NULL);
  virtual STexPic *CopyTexture(const char *name, STexPic *ti, int CubeSide=-1);

public:  
  static int TexSize(int wdt, int hgt, int mode);
  static int GetTexSrcFormat(ETEX_Format eTF);
  static int GetTexDstFormat(ETEX_Format eTF);
  static void CalcMipsAndSize(STexPic *ti);
  static ETEX_Format GetTexFormat(int GLFormat);
  static void BindNULL(int From)
  {
  }

  static _inline void SetTextureTarget(int Stage, int Target)
  {
  }

  static _inline void ResetTextureTarget(int Stage)
  {
  }

  CNULLTexMan() : CTexMan()
  {
  }
  virtual ~CNULLTexMan();
  
  virtual byte *GenerateDXT_HW(STexPic *ti, EImFormat eF, byte *dst, int *numMips, int *DXTSize, bool bMips=true);
  virtual void SetTexture(int Id, ETexType eTT);
  virtual bool SetFilter(char *filt);
  virtual void UpdateTextureData(STexPic *pic, byte *data, int USize, int VSize, bool bProc, int State, bool bPal);
  virtual void UpdateTextureRegion(STexPic *pic, byte *data, int X, int Y, int USize, int VSize) {};
  virtual STexPic *CreateTexture();
  virtual void GetAverageColor(SEnvTexture *cm, int nSide) {}
  virtual bool ScanEnvironmentCM (const char *name, int size, Vec3d& Pos) {return true;}
  virtual void ScanEnvironmentCube(SEnvTexture *cm, int RendFlags, int Size, bool bLightCube) {}
  virtual void ScanEnvironmentTexture(SEnvTexture *cm, SShader *pSH, SRenderShaderResources *pRes, int RendFlags, bool bUseExistingREs) {}
  virtual void EndCubeSide(CCObject *obj, bool bNeedClear) {}
  virtual void StartCubeSide(CCObject *obj) {}
  virtual void DrawToTexture(Plane& Pl, STexPic *Tex, int RendFlags) {}
  virtual void DrawToTextureForDof(int Id) {};
  virtual void DrawToTextureForGlare(int Id) {}
  virtual void DrawToTextureForRainMap(int Id) {}
  virtual void StartHeatMap(int Id) {}
  virtual void EndHeatMap() {}
  virtual void StartRefractMap(int Id) {};
  virtual void EndRefractMap() {};
  virtual void StartNightMap(int Id) {}
  virtual void EndNightMap() {}
  virtual void Update() {}
  virtual void GenerateFuncTextures() {}

  virtual void DrawFlashBangMap(int Id, int RendFlags, CREFlashBang *pRE) {}
  // tiago: added
  virtual void DrawMotionMap(void) {}
  virtual void DrawScreenContrastMap(void) {}
  virtual void DrawScreenColorTransferMap(void) {}    
  virtual void StartScreenTexMap(int Id) {}
  virtual void EndScreenTexMap() {}
  virtual bool PreloadScreenFxMaps(void)  { return 0; }

  virtual void StartScreenMap(int Id) {}
  virtual void EndScreenMap() {}

  virtual STexPic *AddToHash(int Id, STexPic *ti);
  virtual void RemoveFromHash(int Id, STexPic *ti);
  virtual STexPic *GetByID(int Id);

  static TTextureMap m_RefTexs;

  static int m_Format;

  static CNULLTexUnit m_TUState[8];

#ifdef DEBUGALLOC
#undef new
#endif
  void* operator new( size_t Size )
  {
    void *ptr = malloc(Size);
    memset(ptr, 0, Size);
    return ptr;
  }
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
};


#endif //ps2_renderer