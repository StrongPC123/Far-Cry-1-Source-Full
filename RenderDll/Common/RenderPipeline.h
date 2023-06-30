
/*=============================================================================
RenderPipeline.h : Shaders pipeline declarations.
Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
  * Created by Honitch Andrey
  
=============================================================================*/


#ifndef __RENDERPIPELINE_H__
#define __RENDERPIPELINE_H__

//====================================================================

#define PIPE_USE_INSTANCING

#define NUMRI_LISTS 6
#define MAX_HWINST_PARAMS 16384

typedef union UnINT64
{
  uint64 SortVal;
  struct
  {
    uint Low;
    uint High;
  }i;
} UnINT64;

struct SRendItemPre
{
  UnINT64 SortVal;
  CRendElement *Item;
  union
  {
    uint ObjSort;
    float fDist;
  };
  uint DynLMask;
};

struct SRendItemLight : SRendItemPre
{
};

struct SRendItemStenc : SRendItemPre
{
};

struct SRendItem : SRendItemPre
{
  static void mfCalcRefractVectors(int Type, byte *Dst, int StrDst);
  static void mfCalcLightAttenuation(int type, byte *Dst, int StrideDst);
  static void mfCalcProjectVectors(int Type, float *Mat, float RefractIndex, byte *Dst, int StrDst);

  static void mfCalcLightVectors(byte *lv, int Stride);
  static void mfCalcNormLightVectors(byte *lv, int Stride, int Type);
  static void mfCalcHalfAngles(int type, byte *ha, int StrHA);

  static void mfCalcProjectAttenFromCamera(byte *dst, int Str);

  static void mfCalcLAttenuationSpec0(byte *dst, int StrDst, byte *lv, int StrLV, int type);
  static void mfCalcLAttenuationSpec1(byte *dst, int StrDst, byte *lv, int StrLV, int type);

  static void mfCalcLightVectors_Terrain(byte *lv, int Stride);
  static void mfCalcLAttenuationSpec0_Terrain(byte *dst, int StrDst, byte *lv, int StrLV, int type);
  static void mfCalcLAttenuationSpec1_Terrain(byte *dst, int StrDst, byte *lv, int StrLV, int type);
  static void mfCalcHalfAngles_Terrain(int type, byte *ha, int StrHA);

  static void mfCalcTangentSpaceVectors();
  static void mfComputeTangent(const Vec3d& v0, const Vec3d& v1, const Vec3d& v2, const float t0[2], const float t1[2], const float t2[2], Vec3d &tangent, Vec3d& binormal, float& sign, Vec3d& face_normal);
  
  //==================================================
  static void *mfGetPointerCommon(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags);


#ifdef PIPE_USE_INSTANCING
  static _inline void mfAdd(CRendElement *Item, CCObject *pObj, SShader *Shader, int ResId, SShader *EfState, int numFog, int nTempl, int nSort=0)
  {
    int nList = nSort>>28;
    nSort &= 0xffff;
    int n = m_RendItems[nList].Num();
    m_RendItems[nList].AddIndex(1);
    SRendItemPre *ri = &m_RendItems[nList][n];
    int IdState = EfState ? EfState->m_Id : 0;
    nSort = (nSort > 0) ? nSort : Shader->m_eSort;
    int ObjNum;
    if (pObj)
    {
      ObjNum = pObj->m_VisId;
      ri->ObjSort = (pObj->m_ObjFlags & 0xffff0000) | pObj->m_nLMId;
      ri->DynLMask = pObj->m_DynLMMask;
    }
    else
    {
      ObjNum = 0;
      ri->ObjSort = 0;
      ri->DynLMask = 0;
    }
    ri->SortVal.i.Low = (ObjNum<<20) | (IdState<<8) | (numFog);
    ri->SortVal.i.High = (nSort<<26) | (Shader->m_Id<<14) | (ResId);
    ri->Item = Item;
  }
  static _inline void mfGet(UnINT64 flag, int *nObject, SShader **Shader, SShader **ShaderState, int *numFog, SRenderShaderResources **Res)
  {
    *numFog = flag.i.Low & 0x3f;
    *Shader = SShader::m_Shaders_known[(flag.i.High>>14) & 0xfff];
    int n = (flag.i.Low>>8) & 0xfff;
    *ShaderState = n ? SShader::m_Shaders_known[n] : NULL;
    *nObject = (flag.i.Low>>20) & 0xfff;
    n = flag.i.High & 0x3fff;
    *Res = (n) ? SShader::m_ShaderResources_known[n] : NULL;
  }
  static _inline void mfGetObj(UnINT64 flag, int *nObject)
  {
    *nObject = (flag.i.Low>>20) & 0xfff;
  }
  static _inline void mfGet(UnINT64 flag, SShader **Shader, SShader **ShaderState, SRenderShaderResources **Res)
  {
    *Shader = SShader::m_Shaders_known[(flag.i.High>>14) & 0xfff];
    int n = (flag.i.Low>>8) & 0xfff;
    *ShaderState = n ? SShader::m_Shaders_known[n] : NULL;
    n = flag.i.High & 0x3fff;
    *Res = (n) ? SShader::m_ShaderResources_known[n] : NULL;
  }
  static _inline SShader *mfGetShader(UnINT64 flag)
  {
    return SShader::m_Shaders_known[(flag.i.High>>14) & 0xfff];
  }
#else
  static _inline void mfAdd(CRendElement *Item, CCObject *pObj, SShader *Shader, int ResId, SShader *EfState, int numFog, int nTempl, int nSort=0)
  {
    int nList = nSort>>28;
    nSort &= 0xffff;
    int n = m_RendItems[nList].Num();
    m_RendItems[nList].AddIndex(1);
    SRendItemPre *ri = &m_RendItems[nList][n];
    int IdState = EfState ? EfState->m_Id : 0;
    nSort = (nSort > 0) ? nSort : Shader->m_eSort;
    int ObjNum = pObj ? pObj->m_VisId : 0;
    ri->SortVal.i.Low = (Shader->m_Id<<20) | (IdState<<8) | (numFog);
    ri->SortVal.i.High = (nSort<<26) | (ObjNum<<15) | (ResId);
    ri->Item = Item;
  }
  static _inline void mfGet(UnINT64 flag, int *nObject, SShader **Shader, SShader **ShaderState, int *numFog, SRenderShaderResources **Res)
  {
    *numFog = flag.i.Low & 0x3f;
    *Shader = SShader::m_Shaders_known[(flag.i.Low>>20) & 0xfff];
    int n = (flag.i.Low>>8) & 0xfff;
    *ShaderState = n ? SShader::m_Shaders_known[n] : NULL;
    *nObject = (flag.i.High>>15) & 0x7ff;
    n = flag.i.High & 0x3fff;
    *Res = (n) ? SShader::m_ShaderResources_known[n] : NULL;
  }
  static _inline void mfGet(UnINT64 flag, SShader **Shader, SShader **ShaderState, SRenderShaderResources **Res)
  {
    *Shader = SShader::m_Shaders_known[(flag.i.Low>>20) & 0xfff];
    int n = (flag.i.Low>>8) & 0xfff;
    *ShaderState = n ? SShader::m_Shaders_known[n] : NULL;
    n = flag.i.High & 0x3fff;
    *Res = (n) ? SShader::m_ShaderResources_known[n] : NULL;
  }
#endif

  // Sort by SortVal member of RI
  static void mfSort(SRendItemPre *First, int Num);
  // Special Sorting ignoring shadow maps
  static void mfSortForStencil(SRendItemPre *First, int Num);
  // Sort by distance
  static void mfSortByDist(SRendItemPre *First, int Num);
  // Sort by light
  static void mfSortByLight(SRendItemPre *First, int Num);

  static int m_RecurseLevel;
  static int m_StartRI[8][NUMRI_LISTS];
  static int m_EndRI[8][NUMRI_LISTS];
  static TArray<SRendItemPre> m_RendItems[];    
};

struct SRendItemPreprocess : public SRendItem
{
  CCObject *m_Object;
  
  static void mfSort(SRendItemPreprocess *First, int Num);
};

struct SRefSprite
{
  CCObject *m_pObj;
};

//==================================================================

#define RNF_NOSHINE 0x1000

struct SMRendVert;
struct SMRendTexVert;
struct SShaderPass;
struct SEvalFuncs;

union UPipeVertex
{
  void               *Ptr;
  byte               *PtrB;
  float              *PtrF;

  Vec3d                                   *VBPtr_0;
  struct_VERTEX_FORMAT_P3F                *VBPtr_1;
  struct_VERTEX_FORMAT_P3F_COL4UB         *VBPtr_2;
  struct_VERTEX_FORMAT_P3F_TEX2F          *VBPtr_3;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F   *VBPtr_4;
  struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F *VBPtr_5;
  struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB  *VBPtr_6;
  struct_VERTEX_FORMAT_P3F_N              *VBPtr_7;
  struct_VERTEX_FORMAT_P3F_N_COL4UB       *VBPtr_8;
  struct_VERTEX_FORMAT_P3F_N_TEX2F        *VBPtr_9;
  struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *VBPtr_10;
  SPipTangents                            *VBPtr_11;
  struct_VERTEX_FORMAT_TEX2F              *VBPtr_12;
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F *VBPtr_13;
};

//==================================================================

#define MAX_DYNVBS 8

struct SMFenceBufs
{
  uint m_Fence;
  UPipeVertex Ptr;
  CVertexBuffer *m_pVBDyn;
  int m_nOffs;
  int m_nCount;
};

//==================================================================

#define MAX_PORTAL_RECURSES 4
#define MAX_WARPSURFS 64
#define MAX_WARPS 8

struct SWarpSurf
{
  CRendElement *srf;
  int nobj;
  SShader *Shader;
  SRenderShaderResources *ShaderRes;
};

struct STWarpZone
{
  Plane plane;
  int numSrf;
  SWarpSurf Surfs[MAX_WARPSURFS];
};

//==================================================================
// StencilStates

#define FSS_STENCFUNC_ALWAYS   0x0
#define FSS_STENCFUNC_NEVER    0x1
#define FSS_STENCFUNC_LESS     0x2
#define FSS_STENCFUNC_LEQUAL   0x3
#define FSS_STENCFUNC_GREATER  0x4
#define FSS_STENCFUNC_GEQUAL   0x5
#define FSS_STENCFUNC_EQUAL    0x6
#define FSS_STENCFUNC_NOTEQUAL 0x7
#define FSS_STENCFUNC_MASK     0x7

#define FSS_STENCIL_TWOSIDED   0x8

#define FSS_CCW_SHIFT          16

#define FSS_STENCOP_KEEP    0x0
#define FSS_STENCOP_REPLACE 0x1
#define FSS_STENCOP_INCR    0x2
#define FSS_STENCOP_DECR    0x3
#define FSS_STENCOP_ZERO    0x4
#define FSS_STENCOP_INCR_WRAP 0x5
#define FSS_STENCOP_DECR_WRAP 0x6

#define FSS_STENCFAIL_SHIFT   4
#define FSS_STENCFAIL_MASK    (0x7 << FSS_STENCFAIL_SHIFT)

#define FSS_STENCZFAIL_SHIFT  8
#define FSS_STENCZFAIL_MASK   (0x7 << FSS_STENCZFAIL_SHIFT)

#define FSS_STENCPASS_SHIFT   12
#define FSS_STENCPASS_MASK    (0x7 << FSS_STENCPASS_SHIFT)

#define STENC_FUNC(op) (op)
#define STENC_CCW_FUNC(op) (op << FSS_CCW_SHIFT)
#define STENCOP_FAIL(op) (op << FSS_STENCFAIL_SHIFT)
#define STENCOP_ZFAIL(op) (op << FSS_STENCZFAIL_SHIFT)
#define STENCOP_PASS(op) (op << FSS_STENCPASS_SHIFT)
#define STENCOP_CCW_FAIL(op) (op << (FSS_STENCFAIL_SHIFT+FSS_CCW_SHIFT))
#define STENCOP_CCW_ZFAIL(op) (op << (FSS_STENCZFAIL_SHIFT+FSS_CCW_SHIFT))
#define STENCOP_CCW_PASS(op) (op << (FSS_STENCPASS_SHIFT+FSS_CCW_SHIFT))

//==================================================================

#ifdef DIRECT3D9
#include <d3d9.h>
#endif

#if defined (DIRECT3D8) || defined (DIRECT3D9)
template <class IndexType> class DynamicIB;

struct SD3DFixedVShader
{
#ifdef DIRECT3D8
  TArray<DWORD> m_Declaration;  
#elif DIRECT3D9
  TArray<D3DVERTEXELEMENT9> m_Declaration;  
  LPDIRECT3DVERTEXDECLARATION9 m_pDeclaration;
#endif
  DWORD m_Handle;
};


#endif

#if defined (DIRECT3D8) || defined (DIRECT3D9)
#define MAX_DYNVBS 8
template < class VertexType > class DynamicVB;
union UDynamicVB
{
  DynamicVB <Vec3d>                                   *VBPtr_0;
  DynamicVB <struct_VERTEX_FORMAT_P3F>                *VBPtr_1;
  DynamicVB <struct_VERTEX_FORMAT_P3F_COL4UB>         *VBPtr_2;
  DynamicVB <struct_VERTEX_FORMAT_P3F_TEX2F>          *VBPtr_3;
  DynamicVB <struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F>   *VBPtr_4;
  DynamicVB <struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F> *VBPtr_5;
  DynamicVB <struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB>  *VBPtr_6;
  DynamicVB <struct_VERTEX_FORMAT_P3F_N>              *VBPtr_7;
  DynamicVB <struct_VERTEX_FORMAT_P3F_N_COL4UB>       *VBPtr_8;
  DynamicVB <struct_VERTEX_FORMAT_P3F_N_TEX2F>        *VBPtr_9;
  DynamicVB <struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F> *VBPtr_10;
  DynamicVB <SPipTangents>                            *VBPtr_11;
  DynamicVB <struct_VERTEX_FORMAT_TEX2F>              *VBPtr_12;
  DynamicVB <struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F> *VBPtr_13;
};
struct SVertexDeclaration
{
  int StreamMask;
  int VertFormat;
  int InstMask;
  TArray<D3DVERTEXELEMENT9> m_Declaration;  
  LPDIRECT3DVERTEXDECLARATION9 m_pDeclaration;
};
#endif

struct SProfInfo
{
  int NumPolys;
  SShader *ef;
  double Time;
  int m_nItems;
};

struct SVrect
{
  int x,y,width,height;
};

struct SPlane
{
  byte m_Type;
  byte m_SignBits;
  Vec3d m_Normal;
  float m_Dist;
  void Init()
  {
    if ( m_Normal[0] == 1.0f )
      m_Type = PLANE_X;
    if ( m_Normal[1] == 1.0f )
      m_Type = PLANE_Y;
    if ( m_Normal[2] == 1.0f )
      m_Type = PLANE_Z;
    else
      m_Type = PLANE_NON_AXIAL;

    // for fast box on planeside test
    int bits = 0;
    int j;
    for (j=0; j<3; j++)
    {
      if (m_Normal[j] < 0)
        bits |= 1<<j;
    }
    m_SignBits = bits;
  }
};

struct SStatItem
{
  int m_Nums;
  double m_fTime;
};

#define STARTPROFILE(Item) { ticks(Item.m_fTime); Item.m_Nums++; }
#define ENDPROFILE(Item) { unticks(Item.m_fTime); }

struct SPipeStat
{
  int m_NumRendBatches;
  int m_NumRendItems;
  int m_NumRendObjects;
  int m_NumDrawCalls;
  int m_NumTextChanges;
  int m_NumStateChanges;
  int m_NumRendSkinnedObjects;
  int m_NumLitShaders;
  int m_NumVShadChanges;
  int m_NumPShadChanges;
  int m_NumVShaders;
  int m_NumPShaders;
  int m_NumTextures;
  int m_TexturesSize;
  int m_MeshUpdateBytes;
  float m_fOverdraw;
  float m_fSkinningTime;
  float m_fPreprocessTime;
  float m_fFlushTime;
  float m_fTexUploadTime;
  float m_fOcclusionTime;
  int m_NumNotFinishFences;
  int m_NumFences;
  float m_fEnvCMapUpdateTime;
};

struct STexStageInfo
{
  int        nMipFilter; 
  byte       Repeat;
  byte       Projected;
  int        MagFilter;
  int        MinFilter;
  int        Anisotropic;
  int        TCIndex;
  // Per-stage color operations
  byte       m_CO;
  byte       m_CA;
  byte       m_AO;
  byte       m_AA;
#ifndef _XBOX
  int        Palette;
#else
  void*      Palette;
#endif
  STexPic    *Texture;
  STexStageInfo()
  {
    Flush();
  }
  void Flush()
  {
    TCIndex = -1;
    nMipFilter = -1;
    Repeat = (bool)-1;
    Anisotropic = 255;
    Texture = NULL;
#ifndef _XBOX
    Palette = -1;
#else
    Palette = NULL;
#endif
  }
};


struct SSplash
{
  int m_Id;
  Vec3d m_Pos;
  float m_fForce;
  eSplashType m_eType;
  float m_fStartTime;
  float m_fLastTime;
  float m_fCurRadius;
};

// m_RP.m_Flags
#define RBF_2D               0x10
#define RBF_USEFENCES        0x100

#define RBF_MODIF_TC         0x1000
#define RBF_MODIF_VERT       0x2000
#define RBF_MODIF_COL        0x4000
#define RBF_MODIF_MASK       0xf000

#define RBF_NEAREST          0x10000
#define RBF_3D               0x40000
#define RBF_SHOWLINES        0x80000

// m_RP.m_PersFlags
#define RBPF_DONTDRAWSUN     1
#define RBPF_SETCLIPPLANE    2
#define RBPF_USESTREAM1      4
#define RBPF_USESTREAM2      8
#define RBPF_MEASUREOVERDRAW 0x10
#define RBPF_DONTDRAWNEAREST 0x20
#define RBPF_DRAWPORTAL      0x40
#define RBPF_DRAWMIRROR      0x80
#define RBPF_NOSHINE         0x100
#define RBPF_NOCLEARBUF      0x400
#define RBPF_DRAWNIGHTMAP    0x800
#define RBPF_DRAWHEATMAP     0x1000
#define RBPF_DRAWMOTIONMAP   0x2000
#define RBPF_DRAWSCREENMAP   0x4000
#define RBPF_MATRIXNOTLOADED 0x8000

#define RBPF_DRAWSCREENTEXMAP 0x10000 // tiago: added
#define RBPF_ONLYREFRACTED    0x20000
#define RBPF_IGNORERENDERING  0x40000
#define RBPF_IGNOREREFRACTED  0x80000

#define RBPF_PS1WASSET         0x100000
#define RBPF_PS2WASSET         0x200000
#define RBPF_VSWASSET          0x400000
#define RBPF_TSWASSET          0x800000

#define RBPF_PS1NEEDSET        0x1000000
#define RBPF_PS2NEEDSET        0x2000000
#define RBPF_VSNEEDSET         0x4000000
#define RBPF_TSNEEDSET         0x8000000

#define RBPF_WASWORLDSPACE     0x10000000
#define RBPF_MAKESPRITE        0x20000000
#define RBPF_MULTILIGHTS       0x40000000
#define RBPF_HDR               0x80000000

// m_RP.m_FlagsModificators
#define RBMF_TANGENTSUSED    SHPF_TANGENTS
#define RBMF_LMTCUSED        SHPF_LMTC
#define RBMF_BENDINFOUSED    4
#define RBMF_OBJUSESROTATE   8

// Texture transform flags (4)
#define RBMF_TCM0               0x100
// Object linear texgen flags (4)
#define RBMF_TCGOL0             0x1000
// Reflection map texgen flags (4)
#define RBMF_TCGRM0             0x10000
// Normal map texgen flags (4)
#define RBMF_TCGNM0             0x100000
// Normal map texgen flags (4)
#define RBMF_TCGSM0             0x1000000

#define RBMF_NOUPDATE           0x10000000

#define RBMF_TCG                0xffff000
#define RBMF_TCM                0xf00


// m_RP.m_FlagsPerFlush
#define RBSI_BLEND           0x1
#define RBSI_NOCULL          0x2
#define RBSI_WASDEPTHWRITE   0x4
#define RBSI_DRAWAS2D        0x8
#define RBSI_ALPHABLEND      0x10
#define RBSI_ALPHATEST       0x20
#define RBSI_DEPTHWRITE      0x40
#define RBSI_FOGVOLUME       0x80
#define RBSI_TEXSTATE        0x100
#define RBSI_ALPHAGEN        0x200
#define RBSI_RGBGEN          0x400
#define RBSI_COLORMASK       0x800
#define RBSI_INDEXSTREAM     0x1000
#define RBSI_USEVP           0x2000
#define RBSI_DEPTHTEST       0x4000
#define RBSI_DEPTHFUNC       0x8000
#define RBSI_SHADOWPASS      0x10000
#define RBSI_STENCIL         0x20000
#define RBSI_GLOBALRGB       0x40000
#define RBSI_GLOBALALPHA     0x80000
#define RBSI_VERTSMERGED     0x100000
#define RBSI_LMTCMERGED      0x200000
#define RBSI_TANGSMERGED     0x400000
#define RBSI_FURPASS         0x800000
#define RBSI_MERGED          0x1000000
#define RBSI_USE_LM          0x2000000
#define RBSI_USE_HDRLM       0x4000000
#define RBSI_USE_3DC         0x8000000
#define RBSI_USE_3DC_A       0x10000000
#define RBSI_USE_SPECANTIALIAS 0x20000000

// m_RP.m_HWCurState
// m_RP.m_HWPrevState
#define RPHW_VPROGRAM     1
#define RPHW_RCOMBINER    2
#define RPHW_PSHADER      4

// m_RP.m_ShaderLightMask
#define SLMF_DIRECT       0
#define SLMF_POINT        1
#define SLMF_PROJECTED    2
#define SLMF_TYPE_MASK    (SLMF_POINT | SLMF_PROJECTED)
#define SLMF_ONLYSPEC     4
#define SLMF_SPECOCCLUSION 8
#define SLMF_LTYPE_SHIFT  16

struct SLightPass
{
  CDLight *pLights[4];
  int nLights;
  int nProjectors;
};

// Render status structure
struct SRenderPipeline
{
  SShader *m_pShader;        
  CCObject *m_pCurObject;
  SShader *m_pStateShader;        
  CRendElement *m_pRE;
  SRenderShaderResources *m_pShaderResources;
  CCObject *m_pPrevObject;
#ifdef PIPE_USE_INSTANCING
  TArray<CCObject *> m_MergedObjects;
  TArray<CCObject *> m_RotatedMergedObjects;
  TArray<CCObject *> m_NonRotatedMergedObjects;
#endif
  int m_FrameGTC;
  SGenTC *m_pGTC[MAX_TMU];
  STexStageInfo m_TexStages[MAX_TMU];

  short m_TransformFrame;
  UCol m_CurGlobalColor;
  UCol m_NeedGlobalColor;
  
  SEvalFuncs *m_pCurFuncs;
  SEvalFuncs_RE m_EvalFuncs_RE;
  SEvalFuncs_C m_EvalFuncs_C;

  float m_RealTime;

  int m_ObjFlags;
  int m_Flags;                   // Reset on start pipeline
  int m_PersFlags;               // Never reset
  int m_FlagsPerFlush;
  int m_FlagsModificators;
  int m_FrameObject;
  float m_fCurSpecShininess;
  CName m_Name_SpecularExp;
  int m_ClipPlaneEnabled;
  CCObject *m_pIgnoreObject;
  
  bool m_bStartPipeline;
  bool m_bDrawToTexture;
  int m_MirrorClipSide;
  int m_RenderFrame;
  int m_RendPass;
  int m_ResourceState;

  SEnvTexture *m_pCurEnvTexture;

  int m_StatNumLights;
  int m_StatNumLightPasses;
  int m_StatNumPasses;
  int m_StatLightMask;
  char *m_ExcludeShader;
  char *m_ShowOnlyShader;
  
  TArray<SProfInfo> m_Profile;

  Vec3d m_ViewOrg;
  
  int m_CurrentVLightFlags;
  int m_EnabledVLightFlags;
  int m_EnabledVLights;
  int m_CurrentVLights;

  int m_ShaderLightMask;
  int m_DynLMask;
  CDLight *m_pCurLight;
  int m_nCurLight;
  int m_nCurLightPass;
  int m_NumActiveDLights;  
  CDLight *m_pActiveDLights[16];
  int m_nCurLightParam;
  SLightPass m_LPasses[32];

  SLightMaterial *m_pCurLightMaterial;
  SLightMaterial m_DefLightMaterial;

  SLightIndicies *m_pCurLightIndices;
  SLightIndicies  m_FakeLightIndices;

  UPipeVertex m_Ptr;
  UPipeVertex m_NextPtr;
  int m_Stride;
  int m_OffsT;
  int m_OffsD;
  int m_OffsN;
  int m_CurVFormat;
  TArray<CREOcLeaf *> m_MergedREs;
  TArray<CCObject *> m_MergedObjs;

  float m_fLastWaterFOVUpdate;
  Vec3d m_LastWaterAngleUpdate;
  Vec3d m_LastWaterPosUpdate;
  float m_fLastWaterUpdate;

#if defined (DIRECT3D8) || defined (DIRECT3D9)
  UDynamicVB m_VBs[MAX_DYNVBS];
  DynamicIB <ushort> *m_IndexBuf;
  UDynamicVB m_MergedStreams[3];
  int m_nStreamOffset[3];
  SD3DFixedVShader m_D3DFixedPipeline[8][VERTEX_FORMAT_NUMS]; 
  DynamicVB <vec4_t> *m_VB_Inst;
  TArray<SVertexDeclaration *>m_CustomVD;
#endif
  ushort *m_RendIndices;        
  ushort *m_SysRendIndices;        
  int m_CurVB;
#ifdef OPENGL
  int m_NumFences;
  int m_CurFence;
  int m_FenceCount;
  SMFenceBufs m_VidBufs[MAX_DYNVBS];
  SVertexStream m_IBDyn;
  int m_IBDynOffs;
  int m_IBDynSize;
  int m_MergedStreams[3];
  int m_nStreamOffset[3];
#endif

  int m_RendNumIndices;             
  int m_RendNumVerts;
  int m_FirstIndex;
  int m_FirstVertex;
  int m_BaseVertex;

  int m_VFormatsMerge[VERTEX_FORMAT_NUMS][VERTEX_FORMAT_NUMS];

  ECull m_eCull;

  void (*m_pRenderFunc)();

  SMFog *m_pFogVolume;       

  bool m_bClipPlaneRefract;
  SPlane m_CurClipPlane;
  SPlane m_CurClipPlaneCull;
  int m_ClipPlaneWasOverrided;
  int m_nClipPlaneTMU;

  int m_FT;
  
  SEfResTexture *m_ShaderTexResources[MAX_TMU];

  CVProgram *m_CurVP;
  CPShader  *m_CurPS;
  CVProgram *m_LastVP;

  CPShader  *m_RCSun;
  CPShader  *m_RCDetail;
  CPShader  *m_RCSprites;
  CPShader  *m_RCSprites_Heat;
  CPShader  *m_RCSprites_FV;
  CVProgram *m_VPDetail;
  CVProgram *m_VPTransformTexture;
  CVProgram *m_VPTexShadow;
  CVProgram *m_VPFog;
  CPShader *m_RCFog;
  CPShader *m_RCTexShadow;
  CVProgram *m_VPPlantBendingSpr;
  CVProgram *m_VPPlantBendingSpr_FV;
  CPShader  *m_RCBlur;
  CVProgram  *m_VPBlur;
  CVProgram *m_VPSubSurfaceScatering;
  CPShader  *m_RCSubSurfaceScatering;
  CVProgram *m_VPSubSurfaceScatering_pp;
  CPShader  *m_RCSubSurfaceScatering_pp;
  CVProgram  *m_VPFur_NormGen;
  CVProgram  *m_VPFur_OffsGen;
  CPShader  *m_RCFur_NormGen;
  CPShader  *m_RCFur_OffsGen;

  CVProgram *m_VP_BaseCol;

  CPShader  *m_PS_HDRTemp;
  CPShader  *m_PS_HDRDownScale2x2;
  CPShader  *m_PS_HDRDownScale4x4;
  CPShader  *m_PS_HDRDownScale4x4_RG;
  CPShader  *m_PS_HDRDownScale4x4_BA;
  CPShader  *m_PS_HDRSampleAvgLum;
  CPShader  *m_PS_HDRResampleAvgLum;
  CPShader  *m_PS_HDRResampleAvgLumExp;
  CPShader  *m_PS_HDRCalcAdaptedLum;
  CPShader  *m_PS_HDRBrightPassFilter;
  CPShader  *m_PS_HDRGaussBlur5x5;
  CPShader  *m_PS_HDRGaussBlur5x5_Bilinear;
  CPShader  *m_PS_HDRBloom;
  CPShader  *m_PS_HDRStar;
  CPShader  *m_PS_HDRStar_MRT;
  CPShader  *m_PS_HDRFinalScene;
  CPShader  *m_PS_HDRMergeTextures[8];
  CPShader  *m_PS_HDR_ShowR;
  CPShader  *m_PS_HDR_ShowRG_MRT;
  CPShader  *m_PS_HDR_Base;
  CPShader  *m_PS_HDR_BaseCol;
  CPShader  *m_PS_HDR_BaseCol_FV;
  CPShader  *m_PS_HDR_AmbBase;
  CPShader  *m_PS_HDR_AmbBaseCol;
  CPShader  *m_PS_HDR_AmbBaseConst;
  CPShader  *m_PS_HDR_BaseConst;
  CPShader  *m_PS_HDR_AmbBaseCol_FV;
  CPShader  *m_PS_HDR_DrawFlare;
  CPShader  *m_PS_HDR_SkyFake;
  CPShader  *m_PS_HDR_Sky;
  CPShader  *m_PS_HDR_ClearScreen;
  CPShader  *m_PS_GaussBlurSep;
  CVProgram  *m_VS_GaussBlurSep;

  SShaderPass *m_CurrPass;

  int m_nCurStartCaster;
  
  int m_Frame;
  int m_FillFrame;
  
  SShaderTechnique *m_pCurTechnique;
  float m_fDistanceToCameraSquared;
  float m_fCurOpacity;
  Matrix44 m_WaterProjMatrix;
  Vec3d m_CamVecs[3];
  Vec3d m_OSCameraPos;

  SPipeStat m_PS;

  Vec3d m_SunDir;
  CFColor m_SunColor;

  int m_nLMStage;
  INT_PTR m_nCurBufferID;       // For ATI vertex object support	//AMD Port
  int m_nCurBufferOffset;   // For ATI vertex object support

  int m_MaxVerts;
  int m_MaxTris;

  int m_RECustomTexBind[8];
  CFColor m_REColor;
  float m_RECustomData[64];

  STexPic *m_AnimTexs[4][32];
  int m_CurLayerNum;

  Vec3d m_Center;

  CCamera m_PrevCamera;

//=========================================================================
// Per vertex attributes
  Vec3d *m_pBinormals;     
  Vec3d *m_pTangents;     
  Vec3d *m_pTNormals;     
  Vec3d *m_pLightVectors[4];     
  Vec3d *m_pHalfAngleVectors[4];     
  Vec3d *m_pAttenuation;
  Vec3d *m_pLAttenSpec0;     
  Vec3d *m_pLAttenSpec1;     

  SMRendTexVert *m_pBaseTexCoordPointer;  
  SMRendTexVert *m_pLMTexCoordPointer;      
  float *m_pFogVertValues;                  
  bvec4 *m_pClientColors;                   

//===================================================================
// Input render data
  TArray<SSplash> m_Splashes;
  int m_NumVisObjects;
  CCObject **m_VisObjects;
  TArray<CDLight *> m_DLights[8];
  TArray <CCObject *> m_RejectedObjects;
  TArray <CCObject *> m_Objects;
  TArray <CCObject *> m_TempObjects;
  TArray<SMFog> m_FogVolumes;
  CCObject *m_ObjectsPool;
  int m_nNumObjectsInPool;

  //================================================================
  // Portals support
  float m_fMinDepthRange;
  float m_fMaxDepthRange;

  int m_RecurseLevel;
  int m_WasPortals;
  int m_CurPortal;
  STWarpZone *m_CurWarp;

  //================================================================
  // Sprites
  TArray<SRefSprite> m_Sprites;

  //================================================================
  // Temporary mesh
  TArray<class CRETempMesh *> m_TempMeshes[2];
  TArray<class CRETempMesh *> *m_CurTempMeshes;

  //================================================================
  // Glare Heat and Night
  class CREGlare *m_pREGlare;
  class CREHDRProcess *m_pREHDR;
  int m_HeatSize;
  int m_NightSize;
  int m_GlareSize;
  int m_RainMapSize;
  int m_FlashBangSize;

  //=================================================================
  // WaveForm tables
  float m_tSinTable[1024];
  float m_tHalfSinTable[1024];
  float m_tCosTable[1024];
  float m_tSquareTable[1024];
  float m_tTriTable[1024];
  float m_tSawtoothTable[1024];
  float m_tInvSawtoothTable[1024];
  float m_tHillTable[1024];

  float m_tRandFloats[256];
  byte  m_tRandBytes[256];
};

#define MAX_REND_OBJECTS 4096

#endif	// __RENDERPIPELINE_H__
