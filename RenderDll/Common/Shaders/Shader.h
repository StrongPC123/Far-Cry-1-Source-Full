/*=============================================================================
  Shader.h : Shaders declarations.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#ifndef __SHADER_H__
#define __SHADER_H__

#include <string>
#include <Names.h>
#include "../Defs.h"


// Shader.h
// Shader pipeline common declarations.

class CSunFlares;
class CCObject;
struct SShaderPass;
struct STexPic;
struct SShader;
struct SLightMaterial;
struct SShaderTexUnit;
struct SMatrixTransform;
struct SShaderPassHW;
class CRendElement;

#define MAX_SHADERS 4096
#define MAX_SHADER_RES 16384
#define SHADER_VERSION 1.00

//=========================================================

/*----------------------------------------------------------------------------
  TDList.
----------------------------------------------------------------------------*/

//
// Simple double-linked list template.
//
template< class T > class TDList : public T
{
public:
  TDList* Next;
  TDList** PrevLink;
  void Unlink()
  {
    if( Next )
      Next->PrevLink = PrevLink;
    if (PrevLink)
      *PrevLink = Next;
  }
  void Link( TDList*& Before )
  {
    if( Before )
      Before->PrevLink = &Next;
    Next     = Before;
    PrevLink = &Before;
    Before   = this;
  }
};


//=========================================================

#define MAX_ORIENTS 64

#define FOR_ORTHO 1

struct SCoord
{
  Vec3d m_Org;
  Vec3d m_Vecs[3];  //forward, right, up (matrix 3x3)
};

// Polygon orientation types
struct SOrient
{
  SCoord m_Coord;
  uint m_Flags;
};

//===========================================================

struct SPair
{
  std::string MacroName;
  std::string Macro;
};

struct SFXParam
{
  std::string m_Name;      // Parameter name
  EParamType m_Type;       // Parameter type
  int m_nRows;             // Matrix rows
  int m_nColumns;          // Matrix columns
  UParamVal m_Value;       // Default value
  std::string m_Annotations; // Additional parameters (scripts)
  std::string m_Assign;      // Parameter app handling type
  SFXParam()
  {
    m_Type = eType_UNKNOWN;
    m_Value.m_Color[0] = m_Value.m_Color[1] = m_Value.m_Color[2] = m_Value.m_Color[3] = 0;
    m_nRows = 0;
    m_nColumns = 0;
  }
};

struct SFXStruct
{
  std::string m_Name;
  std::string m_Struct;
};

struct SFXSampler
{
  std::string m_Name;
  int m_nTextureID;
  int m_nFilterFlag;
};

struct SFXTexture
{
  int m_nResourceSlot;
  ETexType m_nResType;
  std::string m_Name;
  std::string m_ResName;
};

enum EShaderVersion
{
  eSHV_Unknown,
  eSHV_PS_Auto,
  eSHV_PS_1_1,
  eSHV_PS_1_3,
  eSHV_PS_1_4,
  eSHV_PS_2_0,
  eSHV_PS_2_x,
  eSHV_PS_3_0,

  eSHV_VS_Auto,
  eSHV_VS_1_1,
  eSHV_VS_2_0,
  eSHV_VS_2_a,
  eSHV_VS_3_0,
};
//=============================================================================

// Volume fog definitions
struct SFogInfo
{
  CFColor m_FogColor;
  int m_FogAxis;
  float m_FogDensity;
  SWaveForm m_WaveFogGen;

  int Size()
  {
    int nSize = sizeof(SFogInfo);
    return nSize;
  }
};

struct SMFog
{
  int m_NumBrush;
  float m_SideDists[6];
  CFColor m_Color;
  float m_fMaxDist;
  SFogInfo m_FogInfo;
  Vec3d m_Normal;
  float m_Dist;
  bool bCaustics;
};

struct SSkyInfo
{
  STexPic *m_SkyBox[3];
  float m_fSkyLayerHeight;

  int Size()
  {
    int nSize = sizeof(SSkyInfo);
    return nSize;
  }
  SSkyInfo()
  {
    memset(this, 0, sizeof(SSkyInfo));
  }
  ~SSkyInfo();
};

//=================================================================
// Normals generating types
enum EEvalNormal
{
  eNORM_Identity,
  eNORM_Front,
  eNORM_Back,
  eNORM_Custom,
  eNORM_Wave,
  eNORM_Edge,
  eNORM_InvEdge,
};

// ================================================================
// RGB generating types
enum EEvalRGB
{
  eERGB_NoFill=0,
  eERGB_FromClient,
  eERGB_Identity,
  eERGB_Object,
  eERGB_OneMinusObject,
  eERGB_RE,
  eERGB_OneMinusRE,
  eERGB_OneMinusFromClient,
  eERGB_World,
  eERGB_Wave,
  eERGB_Noise,
  eERGB_Comps,

  eERGB_Fixed,
  eERGB_StyleColor,
  eERGB_StyleIntens,
};

// Alpha generating types
enum EEvalAlpha
{
  eEALPHA_NoFill=0,
  eEALPHA_FromClient,
  eEALPHA_Identity,
  eEALPHA_Object,
  eEALPHA_OneMinusObject,
  eEALPHA_RE,
  eEALPHA_OneMinusRE,
  eEALPHA_OneMinusFromClient,
  eEALPHA_World,
  eEALPHA_Wave,
  eEALPHA_Noise,
  eEALPHA_Portal,
  eEALPHA_Comps,

  eEALPHA_Fixed,
  eEALPHA_Beam,
  eEALPHA_Style,
};

struct SStencil
{
  int m_State;
  int m_FuncRef;
  int m_FuncMask;

  SStencil()
  {
    m_State = 0;
    m_FuncRef = 0;
    m_FuncMask = 0;
  }
  void mfSet();
  int Size() {return sizeof(*this);};
};

#define ESF_STATE    1
#define ESF_ALPHAGEN 2
#define ESF_RGBGEN   4
#define ESF_COLORMASK 8
#define ESF_POLYLINE  0x10
#define ESF_NOCULL    0x20
#define ESF_CULLFRONT 0x40

struct SEfState
{
  SStencil *m_Stencil;
  int m_Flags;
  int m_State;
  bool m_bClearStencil;
  EEvalAlpha m_eEvalAlpha;
  EEvalRGB m_eEvalRGB;
  byte m_FixedColor[4];
  byte m_ColorMask[4];

  int Size()
  {
    int nSize = sizeof(SEfState);
    if (m_Stencil)
      nSize += m_Stencil->Size();

    return nSize;
  }
};


// Blending components for fog calculation
enum EFogBlendComponents
{
  eFBLC_None,
  eFBLC_OnlyColor,
  eFBLC_ColorAlpha,
  eFBLC_OnlyAlpha
};

struct SParam;
struct SCGParam4f;

struct SRGBGenNoise
{
  float m_ConstR;
  float m_RangeR;

  float m_ConstG;
  float m_RangeG;

  float m_ConstB;
  float m_RangeB;

  int Size()
  {
    int nSize = sizeof(SRGBGenNoise);
    return nSize;
  }
};

struct SAlphaGenNoise
{
  float m_ConstA;
  float m_RangeA;
  int Size()
  {
    int nSize = sizeof(SAlphaGenNoise);
    return nSize;
  }
};

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

struct SCGScript
{
  CName m_Name;
  char *m_Script;

  ~SCGScript()
  {
  }
  SCGScript()
  {
    m_Script = NULL;
  }

  void Release()
  {
    SAFE_DELETE_ARRAY(m_Script);
    mfRemoveFromList();
    delete this;
  }
  void mfRemoveFromList();

  int Size(bool bShared)
  {
    bool bOk = false;
    if (bShared)
    {
      if (m_Name.GetIndex())
        bOk = true;
    }
    else
    {
      if (!m_Name.GetIndex())
        bOk = true;
    }
    if (!bOk)
      return 0;
    int nSize = sizeof(*this);
    if (m_Script)
      nSize += strlen(m_Script)+1;

    return nSize;
  }
};

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

//=============================================================================
// Vertex programms / Vertex shaders (VP/VS)

// References in script files
struct SRefEfs
{
  int m_Ind;
  int m_Offset;
  int m_Size;
};

// References for loaded Shaders
struct SRefEfsLoaded
{
  SShader *m_Ef;
};

//=====================================================================


typedef std::map<string,SRefEfs*> ShaderFilesMap;
typedef ShaderFilesMap::iterator ShaderFilesMapItor;

typedef std::map<string,SRefEfsLoaded> LoadedShadersMap;
typedef LoadedShadersMap::iterator LoadedShadersMapItor;

typedef std::map<string,string> ShaderMacro;
typedef ShaderMacro::iterator ShaderMacroItor;

struct SLocalMacros
{
  int m_nOffset;
  ShaderMacro *m_Macros;
};

//======================================================================

static _inline float *sfparam(Vec3 param)
{
  static float sparam[4];
  sparam[0] = param.x;
  sparam[1] = param.y;
  sparam[2] = param.z;
  sparam[3] = 1.0f;

  return &sparam[0];
}

static _inline float *sfparam(float param)
{
  static float sparam[4];
  sparam[0] = param;
  sparam[1] = 0;
  sparam[2] = 0;
  sparam[3] = 1.0f;
  return &sparam[0];
}

static _inline float *sfparam(float param0, float param1, float param2, float param3)
{
  static float sparam[4];
  sparam[0] = param0;
  sparam[1] = param1;
  sparam[2] = param2;
  sparam[3] = param3;
  return &sparam[0];
}

// Vertex program and pixel shader per-instance flags
#define VPVST_FOGGLOBAL   1
#define VPVST_CLIPPLANES3 2
#define VPVST_SPECANTIALIAS 4
#define VPVST_NOFOG       8
#define VPVST_NOISE       0x10
#define VPVST_HDR         0x20
#define VPVST_HDRLM       0x40
#define VPVST_HDRREAL     0x80
#define VPVST_INSTANCING_NOROT 0x10000000
#define VPVST_INSTANCING_ROT   0x20000000
#define VPVST_3DC         0x40000000
#define VPVST_3DC_A       0x80000000

#define VPVST_TCM0       RBMF_TCM0
#define VPVST_TCGOL0     RBMF_TCGOL0
#define VPVST_TCGRM0     RBMF_TCGRM0
#define VPVST_TCGNM0     RBMF_TCGNM0
#define VPVST_TCGSM0     RBMF_TCGSM0
#define VPVST_TCMASK     (RBMF_TCG | RBMF_TCM)

// Vertex program flags (m_Flags)
#define VPFI_NOFOG      1
#define VPFI_UNIFIEDPOS 2
#define VPFI_AUTOENUMTC 4
#define VPFI_NOISE      8
#define VPFI_DEFAULTPOS 0x10
#define VPFI_VS20ONLY   0x20
#define VPFI_PROJECTED  0x40
#define VPFI_VS30ONLY   0x80
#define VPFI_SUPPORTS_INSTANCING  0x100
#define VPFI_SUPPORTS_MULTILIGHTS 0x200

#define VPFI_PRECACHEPHASE   0x400
#define VPFI_WASGENERATED    0x800
#define VPFI_NOSPECULAR      0x1000
#define VPFI_FX              0x2000

// Vertex program input flags (passed via mfSet function)
#define VPF_SETPOINTERSFORSHADER 1
#define VPF_SETPOINTERSFORPASS   2
#define VPF_DONTSETMATRICES     4
#define VPF_INSTANCING_NOROTATE 8
#define VPF_INSTANCING_ROTATE   16

class CVProgram
{
public:
  static CVProgram *m_LastVP;
  static int m_LastTypeVP;
  static int m_LastLTypeVP;

  int m_nRefCounter;
  int m_Frame;
  uint64 m_nMaskGen;
  int m_nFailed;
  static int m_FrameObj;
  int m_FrameLoad;
  int m_Flags;
  int m_Id;
  FILETIME m_WriteTime;
  ShaderMacro m_Macros;
  string m_Name;
  bool m_bCGType;
  int m_CGProfileType;

public:
  CVProgram()
  {
    m_nRefCounter = 0;
    m_Id = 0;
    m_Flags = 0;
    m_nMaskGen = 0;
  }
  _inline const char * mfGetName()
  {
    return m_Name.c_str();
  }
  virtual ~CVProgram() {}

#if !defined(PS2) && !defined (GC) && !defined (NULL_RENDERER)
  static TArray<CVProgram *> m_VPrograms;
#endif
  static CVProgram *mfForName(const char *name, std::vector<SFXStruct>& Structs, std::vector<SPair>& Macros, char *entryFunc, EShaderVersion eSHV, uint64 nMaskGen=0);
  static CVProgram *mfForName(const char *name, uint64 nMaskGen=0);
  static void mfClearAll();
  static void mfReloadScript(const char *szPath, const char *szName, int nFlags, uint64 nMaskGen);

  virtual void Release() = 0;
  virtual void mfReload(int nFlags);
  virtual int  Size() = 0;
  virtual bool mfCompile(char *scr) {return 0;}
  virtual bool mfSet(bool bStat, SShaderPassHW *slw, int nFlags=VPF_SETPOINTERSFORSHADER) {return 0;}
  virtual void mfSetVariables(bool bObj, TArray<SCGParam4f>* Vars) {}
  virtual void mfSetStateMatrices() {}
  virtual void mfReset() {}
  virtual void mfPrecache() {}
  virtual bool mfHasPointer(ESrcPointer ePtr) {return false;}
  virtual const char *mfGetCurScript() {return NULL;}
  virtual void mfGatherFXParameters(const char *buf, SShaderPassHW *pSHPass, std::vector<SFXParam>& Params, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures, SShader *ef) = 0;
  virtual void mfPostLoad() = 0;
  virtual int  mfVertexFormat(bool &bUseTangents, bool &bUseLM) = 0;
};

void SkipCharacters(char **buf, const char *toSkip);

_inline char *sGetFuncName(const char *pFunc)
{
  static char func[128];
  const char *b = pFunc;
  while(*b > 0x20)  { b++; }
  while(*b <= 0x20)  { b++; }
  int n = 0;
  while(*b > 0x20 && *b != '(')  { func[n++] = *b++; }
  func[n] = 0;

  return func;
}

//=============================================================================
// Pixel shaders (PS)

// Pixel shader flags (m_Flags)
#define PSFI_NOFOG      1
#define PSFI_PS20ONLY   2
#define PSFI_AUTOENUMTC 4
#define PSFI_PS30ONLY   8
#define PSFI_SUPPORTS_INSTANCING  0x10
#define PSFI_SUPPORTS_MULTILIGHTS 0x20
#define PSFI_PRECACHEPHASE   0x100
#define PSFI_WASGENERATED    0x200
#define PSFI_NOSPECULAR      0x400
#define PSFI_FX              0x800
#define PSFI_MRT             0x1000
#define PSFI_PS2XONLY        0x2000

// Pixel shader input flags (passed via mfSet function)
#define PSF_INSTANCING          1

class CPShader
{
public:
  static CPShader *m_CurRC;

  static CPShader *m_LastVP;
  static int m_LastTypeVP;
  static int m_LastLTypeVP;

  int m_nRefCounter;
  int m_Frame;
  uint64 m_nMaskGen;
  int m_nFailed;
  int m_FrameLoad;
  int m_Flags;
  int m_Id;
  FILETIME m_WriteTime;
  ShaderMacro m_Macros;
  string m_Name;
  bool m_bActive;
  bool m_bCGType;
  int m_CGProfileType;

public:
  CPShader()
  {
    m_Id = 0;
    m_nRefCounter = 0;
    m_bActive = false;
    m_Flags = 0;
    m_nMaskGen = 0;
  }
  virtual ~CPShader() {}

  _inline const char * mfGetName()
  {
    return m_Name.c_str();
  }

#if !defined(PS2) && !defined (GC) && !defined (NULL_RENDERER)
  static TArray<CPShader *> m_PShaders;

  static CPShader *mfForName(const char *name, uint64 nMaskGen=0);
  static CPShader *mfForName(const char *name, std::vector<SFXStruct>& Structs, std::vector<SPair>& Macros, char *entryFunc, EShaderVersion eSHV, uint64 nMaskGen=0);
#endif
  static void mfClearAll();
  static void mfReloadScript(const char *szPath, const char *szName, int nFlags, uint64 nMaskGen);

  virtual void Release() = 0;
  virtual void mfReload(int nFlags);
  virtual void mfEnable() = 0;
  virtual void mfDisable() = 0;
  virtual int  Size() = 0;
  virtual bool mfCompile(char *scr) {return 0;}
  virtual void mfReset() {}
  virtual void mfPrecache() {}
  virtual bool mfIsCombiner() = 0;
  virtual bool mfSet(bool bStat, SShaderPassHW *slw=NULL, int nFlags=0) {return 0;}
  virtual void mfSetVariables(bool bObj, TArray<SCGParam4f>* Vars) {}
  virtual const char *mfGetCurScript() {return NULL;}
  virtual void mfGatherFXParameters(const char *buf, SShaderPassHW *pSHPass, std::vector<SFXParam>& Params, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures, SShader *ef) = 0;
  virtual void mfPostLoad() = 0;
};

_inline CPShader *PShaderForName(CPShader *&Store, const char *szName)
{
#ifndef NULL_RENDERER
  if (!Store)
    Store = CPShader::mfForName(szName);
  assert(Store);
#endif
  return Store;
}

_inline CVProgram *VShaderForName(CVProgram *&Store, const char *szName)
{
#ifndef NULL_RENDERER
  if (!Store)
    Store = CVProgram::mfForName(szName);
  assert(Store);
#endif
  return Store;
}

#define SHADER_BIND_SAMPLER 0x08000000

#define NUM_PPLIGHTS_PERPASS_PS30 4
#define NUM_PPLIGHTS_PERPASS_PS2X 3

//=============================================================================

struct SShaderCacheHeader
{
  int m_SizeOf;
  ushort m_MajorVer;
  ushort m_MinorVer;
  SShaderCacheHeader()
  {
    m_SizeOf = sizeof(SShaderCacheHeader);
  }
};

struct SShaderCacheHeaderItem
{
  int m_nMask;
  int m_nVariables;
};
struct SShaderCacheHeaderItemVar
{
  char m_Name[60];
  short m_Reg;
  short m_nCount;
};

struct SShaderCache
{
  CName m_Name;
  SShaderCacheHeader m_Header;
  class CResFile *m_pRes;
  SShaderCache()
  {
    m_pRes = NULL;
  }
  ~SShaderCache();
};

_inline void SortLightTypes(int Types[4], int nCount)
{
  switch(nCount)
  {
    case 2:
      if (Types[0] > Types[1])
        Exchange(Types[0], Types[1]);
  	  break;
    case 3:
      if (Types[0] > Types[1])
        Exchange(Types[0], Types[1]);
      if (Types[0] > Types[2])
        Exchange(Types[0], Types[2]);
      if (Types[1] > Types[2])
        Exchange(Types[1], Types[2]);
  	  break;
    case 4:
      {
        for (int i=0; i<4; i++)
        {
          for (int j=i; j<4; j++)
          {
            if (Types[i] > Types[j])
              Exchange(Types[i], Types[j]);
          }
        }
      }
  	  break;
  }
}

//=============================================================================
// Static and dynamic Parameters for RC, PS and VP/VS

enum EParamComp
{
  EParamComp_Unknown,
  EParamComp_LightMatrix,
  EParamComp_ObjMatrix,
  EParamComp_ObjScale,
  EParamComp_User,
  EParamComp_CameraMatrix,
  EParamComp_PlantsTMoving,
  EParamComp_FogMatrix,
  EParamComp_VFogMatrix,
  EParamComp_FogEnterMatrix,
  EParamComp_EnvColor,
  EParamComp_Matrix,
  EParamComp_WaterProjMatrix,
  EParamComp_WaterLevel,
  EParamComp_TexProjMatrix,
  EParamComp_BumpAmount,
  EParamComp_GeomCenter,
  EParamComp_Const,
  EParamComp_Wave,
  EParamComp_ObjWave,
  EParamComp_LightIntens,
  EParamComp_SunDirect,
  EParamComp_SunFlarePos,
  EParamComp_LightPos,
  EParamComp_OSLightPos,
  EParamComp_SLightPos,
  EParamComp_FromRE,
  EParamComp_REColor,
  EParamComp_TempMatr,
  EParamComp_VolFogColor,
  EParamComp_VolFogDensity,
  EParamComp_ObjRefrFactor,
  EParamComp_FromObject,
  EParamComp_ObjVal,
  EParamComp_ObjColor,
  EParamComp_Bending,
  EParamComp_SunColor,
  EParamComp_WorldColor,
  EParamComp_LightDirectFactor,
  EParamComp_LightBright,
  EParamComp_LightColor,
  EParamComp_DiffuseColor,
  EParamComp_SpecularPower,
  EParamComp_SpecLightColor,
  EParamComp_AmbLightColor,
  EParamComp_CameraAngle,
  EParamComp_CameraPos,
  EParamComp_ObjPos,
  EParamComp_OSCameraPos,
  EParamComp_SCameraPos,
  EParamComp_ClipPlane,
  EParamComp_Time,
  EParamComp_Distance,
  EParamComp_MinDistance,
  EParamComp_MinLightDistance,
  EParamComp_Random,
  EParamComp_Fog,
  EParamComp_HeatFactor,
  EParamComp_Opacity,
  EParamComp_MatrixTCG,
  EParamComp_MatrixTCM,
  EParamComp_FlashBangBrightness,
  EParamComp_ScreenSize,
  EParamComp_DofFocalParams,
  EParamComp_LightOcclusion,
  EParamComp_LightOcclusions,

  EParamComp_LightsColor,
  EParamComp_SpecLightsColor,
  EParamComp_LightsIntens,
  EParamComp_OSLightsPos,
  EParamComp_LightsType,
  EParamComp_LightsNum,
};

struct SParamComp
{
  byte m_eType;
  byte m_bDependsOnObject;

  SParamComp()
  {
    m_eType = EParamComp_Unknown;
    m_bDependsOnObject = 0;
  }
  virtual ~SParamComp() {}
  virtual float mfGet() { return 0; }
  virtual void mfGet4f(vec4_t v) {}
  virtual bool mfIsEqual(SParamComp *p) = 0;
  virtual void mfCopy(const SParamComp* pc) = 0;
  virtual int Size() = 0;
    
  static SParamComp *mfAdd(SParamComp *pPC);

  static TArray<SParamComp *> m_ParamComps;
};

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

struct SParamComp_User : public SParamComp
{
  bool m_bTimeModulate;
  CName m_Name;

  virtual int Size()
  {
    int nSize = sizeof(SParamComp_User);
    return nSize;
  }

  SParamComp_User()
  {
    m_eType = EParamComp_User;
    m_bTimeModulate = false;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_User *src = (SParamComp_User *)p;
    if (m_eType == src->m_eType && m_Name == src->m_Name && m_bTimeModulate == src->m_bTimeModulate) 
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_User *src = (SParamComp_User *)p;
    m_eType = src->m_eType;
    m_Name = src->m_Name;
    m_bTimeModulate = src->m_bTimeModulate;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif


struct SParamComp_FlashBangBrightness : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(SParamComp_FlashBangBrightness);
    return nSize;
  }

  SParamComp_FlashBangBrightness()
  {
    m_eType = EParamComp_FlashBangBrightness;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_FlashBangBrightness *src = (SParamComp_FlashBangBrightness *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_FlashBangBrightness *src = (SParamComp_FlashBangBrightness *)p;
    m_eType = src->m_eType;
  }
  virtual float mfGet();
};

struct SParamComp_ScreenSize : public SParamComp
{
  int m_Offs;
  int m_Op; // 0(NoOp), 1(+), 2(*), 3(/), 4(-)
  float m_Sign;
  float m_Operand;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_ScreenSize()
  {
    m_eType = EParamComp_ScreenSize;
    m_Op = 0;
    m_Sign = 0;
    m_Operand = 0;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_ScreenSize *src = (SParamComp_ScreenSize *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs && m_Op == src->m_Op && m_Sign == src->m_Sign && m_Operand == src->m_Operand)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_ScreenSize *src = (SParamComp_ScreenSize *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_Op = src->m_Op;
    m_Sign = src->m_Sign;
    m_Operand = src->m_Operand;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};

struct SParamComp_DofFocalParams : public SParamComp
{
  int m_Offs;
  int m_Op; // 0(NoOp), 1(+), 2(*), 3(/), 4(-)
  float m_Sign;
  float m_Operand;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_DofFocalParams()
  {
    m_eType = EParamComp_DofFocalParams;
    m_Op = 0;
    m_Sign = 0;
    m_Operand = 0;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_DofFocalParams *src = (SParamComp_DofFocalParams *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs && m_Op == src->m_Op && m_Sign == src->m_Sign && m_Operand == src->m_Operand)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_DofFocalParams *src = (SParamComp_DofFocalParams *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_Op = src->m_Op;
    m_Sign = src->m_Sign;
    m_Operand = src->m_Operand;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};

struct SParamComp_VFogMatrix : public SParamComp
{
  int m_Offs;

  virtual int Size()
  {
    int nSize = sizeof(SParamComp_VFogMatrix);
    return nSize;
  }

  SParamComp_VFogMatrix()
  {
    m_eType = EParamComp_VFogMatrix;
  }
  virtual ~SParamComp_VFogMatrix()
  {
  }

  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_VFogMatrix *src = (SParamComp_VFogMatrix *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_VFogMatrix *src = (SParamComp_VFogMatrix *)p;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_FogMatrix : public SParamComp
{
  int m_Offs;

  virtual int Size()
  {
    int nSize = sizeof(SParamComp_FogMatrix);
    return nSize;
  }

  SParamComp_FogMatrix()
  {
    m_eType = EParamComp_FogMatrix;
  }
  virtual ~SParamComp_FogMatrix()
  {
  }

  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_FogMatrix *src = (SParamComp_FogMatrix *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_FogMatrix *src = (SParamComp_FogMatrix *)p;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_FogEnterMatrix : public SParamComp
{
  int m_Offs;

  virtual int Size()
  {
    int nSize = sizeof(SParamComp_FogEnterMatrix);
    return nSize;
  }

  SParamComp_FogEnterMatrix()
  {
    m_eType = EParamComp_FogEnterMatrix;
  }
  virtual ~SParamComp_FogEnterMatrix()
  {
  }

  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_FogEnterMatrix *src = (SParamComp_FogEnterMatrix *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_FogEnterMatrix *src = (SParamComp_FogEnterMatrix *)p;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_EnvColor : public SParamComp
{
  int m_Offs;

  virtual int Size()
  {
    int nSize = sizeof(SParamComp_EnvColor);
    return nSize;
  }

  SParamComp_EnvColor()
  {
    m_eType = EParamComp_EnvColor;
  }
  virtual ~SParamComp_EnvColor()
  {
  }

  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_EnvColor *src = (SParamComp_EnvColor *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_EnvColor *src = (SParamComp_EnvColor *)p;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_Matrix : public SParamComp
{
  static SShader *m_pLastShader;
  static Matrix44 m_LastMatrix;
  static int m_Frame;
  int m_Offs;
  TArray<SMatrixTransform> m_Transforms;

  virtual int Size();

  SParamComp_Matrix()
  {
    m_eType = EParamComp_Matrix;
  }
  virtual ~SParamComp_Matrix()
  {
    m_Transforms.Free();
  }

  virtual bool mfIsEqual(SParamComp *p)
  {
    return false;
  }
  virtual void mfCopy(const SParamComp* p);
  //{
  //  SParamComp_Matrix *src = (SParamComp_Matrix *)p;
  //  m_eType = src->m_eType;
  //  m_Offs = src->m_Offs;
  //  m_Transforms.Copy(src->m_Transforms);
  //  m_bDependsOnObject = 1;
  //}
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_ObjScale : public SParamComp
{
  int m_Offs;

  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }

  SParamComp_ObjScale()
  {
    m_eType = EParamComp_ObjScale;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_ObjScale *src = (SParamComp_ObjScale *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_ObjScale *src = (SParamComp_ObjScale *)p;
    m_eType = src->m_eType;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};

struct SParamComp_ObjMatrix : public SParamComp
{
  int m_Offs;

  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }

  SParamComp_ObjMatrix()
  {
    m_eType = EParamComp_ObjMatrix;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_ObjMatrix *src = (SParamComp_ObjMatrix *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_ObjMatrix *src = (SParamComp_ObjMatrix *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_CameraMatrix : public SParamComp
{
  int m_Offs;

  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }

  SParamComp_CameraMatrix()
  {
    m_eType = EParamComp_CameraMatrix;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_CameraMatrix *src = (SParamComp_CameraMatrix *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_CameraMatrix *src = (SParamComp_CameraMatrix *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};

struct SParamComp_LightMatrix : public SParamComp
{
  int m_Offs;

  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }

  SParamComp_LightMatrix()
  {
    m_eType = EParamComp_LightMatrix;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_LightMatrix *src = (SParamComp_LightMatrix *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_LightMatrix *src = (SParamComp_LightMatrix *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_WaterProjMatrix : public SParamComp
{
  int m_Offs;

  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }

  SParamComp_WaterProjMatrix()
  {
    m_eType = EParamComp_WaterProjMatrix;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_WaterProjMatrix *src = (SParamComp_WaterProjMatrix *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_WaterProjMatrix *src = (SParamComp_WaterProjMatrix *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
  }
  virtual float mfGet();
};

struct SParamComp_TexProjMatrix : public SParamComp
{
  int m_Offs;
  int m_Stage;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_TexProjMatrix()
  {
    m_eType = EParamComp_TexProjMatrix;
    m_Stage = 0xf;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_TexProjMatrix *src = (SParamComp_TexProjMatrix *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs && m_Stage == src->m_Stage)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_TexProjMatrix *src = (SParamComp_TexProjMatrix *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_Stage = src->m_Stage;
    m_bDependsOnObject = 1;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_Const : public SParamComp
{
  float m_Val;

  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_Const()
  {
    m_eType = EParamComp_Const;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_Const *src = (SParamComp_Const *)p;
    if (m_eType == src->m_eType && m_Val == src->m_Val)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_Const *src = (SParamComp_Const *)p;
    m_eType = src->m_eType;
    m_Val = src->m_Val;
  }
  virtual float mfGet() { return m_Val; }
};

struct SParamComp_Wave : public SParamComp
{
  SWaveForm m_WF;

  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }

  SParamComp_Wave()
  {
    m_eType = EParamComp_Wave;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_Wave *src = (SParamComp_Wave *)p;
    if (m_eType == src->m_eType && m_WF == src->m_WF)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_Wave *src = (SParamComp_Wave *)p;
    m_eType = src->m_eType;
    m_WF = src->m_WF;
  }
  virtual float mfGet();
};

struct SParamComp_ObjWave : public SParamComp
{
  bool m_bObjX;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_ObjWave()
  {
    m_eType = EParamComp_ObjWave;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_ObjWave *src = (SParamComp_ObjWave *)p;
    if (m_eType == src->m_eType && m_bObjX == src->m_bObjX)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_ObjWave *src = (SParamComp_ObjWave *)p;
    m_eType = src->m_eType;
    m_bObjX = src->m_bObjX;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_WaterLevel : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_WaterLevel()
  {
    m_eType = EParamComp_WaterLevel;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_WaterLevel *src = (SParamComp_WaterLevel *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_WaterLevel *src = (SParamComp_WaterLevel *)p;
    m_eType = src->m_eType;
  }
  virtual float mfGet();
};

struct SParamComp_SunDirect : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_SunDirect()
  {
    m_eType = EParamComp_SunDirect;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_SunDirect *src = (SParamComp_SunDirect *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_SunDirect *src = (SParamComp_SunDirect *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
  }
  virtual float mfGet();
};

struct SParamComp_SunFlarePos : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_SunFlarePos()
  {
    m_eType = EParamComp_SunFlarePos;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_SunFlarePos *src = (SParamComp_SunFlarePos *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_SunFlarePos *src = (SParamComp_SunFlarePos *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
  }
  virtual float mfGet();
};

struct SParamComp_LightPos : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_LightPos()
  {
    m_eType = EParamComp_LightPos;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_LightPos *src = (SParamComp_LightPos *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_LightPos *src = (SParamComp_LightPos *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
  }
  virtual float mfGet();
};

struct SParamComp_LightIntens : public SParamComp
{
  bool m_bInv;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_LightIntens()
  {
    m_eType = EParamComp_LightIntens;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_LightIntens *src = (SParamComp_LightIntens *)p;
    if (m_eType == src->m_eType && m_bInv == src->m_bInv)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_LightIntens *src = (SParamComp_LightIntens *)p;
    m_eType = src->m_eType;
    m_bInv = src->m_bInv;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
  virtual void mfGet4f(vec4_t v);
};
struct SParamComp_LightsIntens : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_LightsIntens()
  {
    m_eType = EParamComp_LightsIntens;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_LightsIntens *src = (SParamComp_LightsIntens *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_LightsIntens *src = (SParamComp_LightsIntens *)p;
    m_eType = src->m_eType;
    m_bDependsOnObject = 1;
  }
  virtual void mfGet4f(vec4_t v);
};


struct SParamComp_LightBright : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_LightBright()
  {
    m_eType = EParamComp_LightBright;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_LightBright *src = (SParamComp_LightBright *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_LightBright *src = (SParamComp_LightBright *)p;
    m_eType = src->m_eType;
  }
  virtual float mfGet();
};

struct SParamComp_LightDirectFactor : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_LightDirectFactor()
  {
    m_eType = EParamComp_LightDirectFactor;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_LightDirectFactor *src = (SParamComp_LightDirectFactor *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_LightDirectFactor *src = (SParamComp_LightDirectFactor *)p;
    m_eType = src->m_eType;
  }
  virtual float mfGet();
};

struct SParamComp_OSLightPos : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_OSLightPos()
  {
    m_eType = EParamComp_OSLightPos;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_OSLightPos *src = (SParamComp_OSLightPos *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_OSLightPos *src = (SParamComp_OSLightPos *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
  virtual void mfGet4f(vec4_t v);
};
struct SParamComp_OSLightsPos : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_OSLightsPos()
  {
    m_eType = EParamComp_OSLightsPos;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_OSLightsPos *src = (SParamComp_OSLightsPos *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_OSLightsPos *src = (SParamComp_OSLightsPos *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_LightsType : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_LightsType()
  {
    m_eType = EParamComp_LightsType;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_LightsType *src = (SParamComp_LightsType *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_LightsType *src = (SParamComp_LightsType *)p;
    m_eType = src->m_eType;
  }
  virtual void mfGet4f(vec4_t v);
};
struct SParamComp_LightsNum : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_LightsNum()
  {
    m_eType = EParamComp_LightsNum;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_LightsNum *src = (SParamComp_LightsNum *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_LightsNum *src = (SParamComp_LightsNum *)p;
    m_eType = src->m_eType;
  }
  virtual float mfGet(void);
};

struct SParamComp_SLightPos : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_SLightPos()
  {
    m_eType = EParamComp_SLightPos;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_SLightPos *src = (SParamComp_SLightPos *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_SLightPos *src = (SParamComp_SLightPos *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_SCameraPos : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_SCameraPos()
  {
    m_eType = EParamComp_SCameraPos;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_SCameraPos *src = (SParamComp_SCameraPos *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_SCameraPos *src = (SParamComp_SCameraPos *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_LightOcclusion : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_LightOcclusion()
  {
    m_eType = EParamComp_LightOcclusion;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_LightOcclusion *src = (SParamComp_LightOcclusion *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_LightOcclusion *src = (SParamComp_LightOcclusion *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_LightOcclusions : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_LightOcclusions()
  {
    m_eType = EParamComp_LightOcclusions;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_LightOcclusions *src = (SParamComp_LightOcclusions *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_LightOcclusions *src = (SParamComp_LightOcclusions *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_VolFogColor : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_VolFogColor()
  {
    m_eType = EParamComp_VolFogColor;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_VolFogColor *src = (SParamComp_VolFogColor *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_VolFogColor *src = (SParamComp_VolFogColor *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
  }
  virtual float mfGet();
};

struct SParamComp_VolFogDensity : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_VolFogDensity()
  {
    m_eType = EParamComp_VolFogDensity;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_VolFogDensity *src = (SParamComp_VolFogDensity *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_VolFogDensity *src = (SParamComp_VolFogDensity *)p;
    m_eType = src->m_eType;
  }
  virtual float mfGet();
};

struct SParamComp_FromRE : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_FromRE()
  {
    m_eType = EParamComp_FromRE;
    m_Offs = -1;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_FromRE *src = (SParamComp_FromRE *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_FromRE *src = (SParamComp_FromRE *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
  }

  virtual float mfGet();
};

struct SParamComp_REColor : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_REColor()
  {
    m_eType = EParamComp_REColor;
    m_Offs = -1;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_REColor *src = (SParamComp_REColor *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_REColor *src = (SParamComp_REColor *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
  }

  virtual float mfGet();
};

struct SParamComp_TempMatr : public SParamComp
{
  int m_MatrID0;
  int m_MatrID1;
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_TempMatr()
  {
    m_eType = EParamComp_TempMatr;
    m_MatrID0 = 0;
    m_MatrID1 = 0;
    m_Offs = -1;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_TempMatr *src = (SParamComp_TempMatr *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs && m_MatrID0 == src->m_MatrID0 && m_MatrID1 == src->m_MatrID1)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_TempMatr *src = (SParamComp_TempMatr *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_MatrID0 = src->m_MatrID0;
    m_MatrID1 = src->m_MatrID1;
  }

  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_ObjRefrFactor : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_ObjRefrFactor()
  {
    m_eType = EParamComp_ObjRefrFactor;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_ObjRefrFactor *src = (SParamComp_ObjRefrFactor *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_ObjRefrFactor *src = (SParamComp_ObjRefrFactor *)p;
    m_eType = src->m_eType;
    m_bDependsOnObject = 1;
  }

  virtual float mfGet();
};

struct SParamComp_FromObject : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_FromObject()
  {
    m_eType = EParamComp_FromObject;
    m_Offs = 0;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_FromObject *src = (SParamComp_FromObject *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_FromObject *src = (SParamComp_FromObject *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};

struct SParamComp_WorldColor : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_WorldColor()
  {
    m_eType = EParamComp_WorldColor;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_WorldColor *src = (SParamComp_WorldColor *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_WorldColor *src = (SParamComp_WorldColor *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_SunColor : public SParamComp
{
  int m_Offs;
  float m_Mult;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_SunColor()
  {
    m_eType = EParamComp_SunColor;
    m_Mult = 1.0f;
    m_Offs = 0;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_SunColor *src = (SParamComp_SunColor *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs && m_Mult == src->m_Mult)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_SunColor *src = (SParamComp_SunColor *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_Mult = src->m_Mult;
  }
  virtual float mfGet();
  virtual void mfGet4f(vec4_t v);
};


struct SParamComp_ObjVal : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_ObjVal()
  {
    m_eType = EParamComp_ObjVal;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_ObjVal *src = (SParamComp_ObjVal *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_ObjVal *src = (SParamComp_ObjVal *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};

struct SParamComp_GeomCenter : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_GeomCenter()
  {
    m_eType = EParamComp_GeomCenter;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_GeomCenter *src = (SParamComp_GeomCenter *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_GeomCenter *src = (SParamComp_GeomCenter *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};

struct SParamComp_Bending : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_Bending()
  {
    m_eType = EParamComp_Bending;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_Bending *src = (SParamComp_Bending *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_Bending *src = (SParamComp_Bending *)p;
    m_eType = src->m_eType;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};

struct SParamComp_ObjColor : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_ObjColor()
  {
    m_eType = EParamComp_ObjColor;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_ObjColor *src = (SParamComp_ObjColor *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_ObjColor *src = (SParamComp_ObjColor *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};

struct SParamComp_LightColor : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_LightColor()
  {
    m_eType = EParamComp_LightColor;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_LightColor *src = (SParamComp_LightColor *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_LightColor *src = (SParamComp_LightColor *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
  }
  virtual float mfGet();
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_LightsColor : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_LightsColor()
  {
    m_eType = EParamComp_LightsColor;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_LightsColor *src = (SParamComp_LightsColor *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_LightsColor *src = (SParamComp_LightsColor *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_DiffuseColor : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_DiffuseColor()
  {
    m_eType = EParamComp_DiffuseColor;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_DiffuseColor *src = (SParamComp_DiffuseColor *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_DiffuseColor *src = (SParamComp_DiffuseColor *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
  }
  virtual float mfGet();
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_SpecularPower : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_SpecularPower()
  {
    m_eType = EParamComp_SpecularPower;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_SpecularPower *src = (SParamComp_SpecularPower *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_SpecularPower *src = (SParamComp_SpecularPower *)p;
    m_eType = src->m_eType;
  }
  virtual float mfGet();
};

struct SParamComp_SpecLightColor : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_SpecLightColor()
  {
    m_eType = EParamComp_SpecLightColor;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_SpecLightColor *src = (SParamComp_SpecLightColor *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_SpecLightColor *src = (SParamComp_SpecLightColor *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
  }
  virtual float mfGet();
  virtual void mfGet4f(vec4_t v);
};
struct SParamComp_SpecLightsColor : public SParamComp
{
  int m_Offs;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_SpecLightsColor()
  {
    m_eType = EParamComp_SpecLightsColor;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_SpecLightsColor *src = (SParamComp_SpecLightsColor *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_SpecLightsColor *src = (SParamComp_SpecLightsColor *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_AmbLightColor : public SParamComp
{
  int m_Offs;
  float m_fMul;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_AmbLightColor()
  {
    m_eType = EParamComp_AmbLightColor;
    m_fMul = 1.0f;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_AmbLightColor *src = (SParamComp_AmbLightColor *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs && m_fMul == src->m_fMul)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_AmbLightColor *src = (SParamComp_AmbLightColor *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_fMul = src->m_fMul;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_BumpAmount : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_BumpAmount()
  {
    m_eType = EParamComp_BumpAmount;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_BumpAmount *src = (SParamComp_BumpAmount *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_BumpAmount *src = (SParamComp_BumpAmount *)p;
    m_eType = src->m_eType;
  }
  virtual float mfGet();
};

struct SParamComp_Fog : public SParamComp
{
  int m_Type;

  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_Fog()
  {
    m_eType = EParamComp_Fog;
    m_Type = -1;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_Fog *src = (SParamComp_Fog *)p;
    if (m_eType == src->m_eType && m_Type == src->m_Type)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_Fog *src = (SParamComp_Fog *)p;
    m_eType = src->m_eType;
    m_Type = src->m_Type;
  }
  virtual float mfGet();
};

struct SParamComp_PlantsTMoving : public SParamComp
{
  int m_Offs;
  static Matrix44 m_Matrix;
  static float m_LastAX;
  static float m_LastAY;
  SWaveForm m_WFX;
  SWaveForm m_WFY;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_PlantsTMoving()
  {
    m_eType = EParamComp_PlantsTMoving;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_PlantsTMoving *src = (SParamComp_PlantsTMoving *)p;
    if (m_eType == src->m_eType && m_WFX == src->m_WFX && m_WFY == src->m_WFY && m_Offs == src->m_Offs)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_PlantsTMoving *src = (SParamComp_PlantsTMoving *)p;
    m_eType = src->m_eType;
    m_WFX = src->m_WFX;
    m_WFY = src->m_WFY;
    m_Offs = src->m_Offs;
    m_bDependsOnObject = 1;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_CameraAngle : public SParamComp
{
  int m_Offs;
  int m_Op; // 0(NoOp), 1(+), 2(*), 3(/), 4(-)
  float m_Sign;
  float m_Operand;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_CameraAngle()
  {
    m_eType = EParamComp_CameraAngle;
    m_Op = 0;
    m_Sign = 0;
    m_Operand = 0;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_CameraAngle *src = (SParamComp_CameraAngle *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs && m_Op == src->m_Op && m_Sign == src->m_Sign && m_Operand == src->m_Operand)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_CameraAngle *src = (SParamComp_CameraAngle *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_Op = src->m_Op;
    m_Sign = src->m_Sign;
    m_Operand = src->m_Operand;
  }
  virtual float mfGet();
};

struct SParamComp_CameraPos : public SParamComp
{
  int m_Offs;
  int m_Op; // 0(NoOp), 1(+), 2(*), 3(/), 4(-)
  float m_Sign;
  float m_Operand;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_CameraPos()
  {
    m_eType = EParamComp_CameraPos;
    m_Op = 0;
    m_Sign = 1.0f;
    m_Operand = 0;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_CameraPos *src = (SParamComp_CameraPos *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs && m_Op == src->m_Op && m_Sign == src->m_Sign && m_Operand == src->m_Operand)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_CameraPos *src = (SParamComp_CameraPos *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_Op = src->m_Op;
    m_Sign = src->m_Sign;
    m_Operand = src->m_Operand;
  }
  virtual float mfGet();
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_ObjPos : public SParamComp
{
  int m_Offs;
  int m_Op; // 0(NoOp), 1(+), 2(*), 3(/), 4(-)
  float m_Sign;
  float m_Operand;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_ObjPos()
  {
    m_eType = EParamComp_ObjPos;
    m_Op = 0;
    m_Sign = 0;
    m_Operand = 0;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_ObjPos *src = (SParamComp_ObjPos *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs && m_Op == src->m_Op && m_Sign == src->m_Sign && m_Operand == src->m_Operand)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_ObjPos *src = (SParamComp_ObjPos *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_Op = src->m_Op;
    m_Sign = src->m_Sign;
    m_Operand = src->m_Operand;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};

struct SParamComp_OSCameraPos : public SParamComp
{
  int m_Offs;
  int m_Op; // 0(NoOp), 1(+), 2(*), 3(/), 4(-)
  float m_Sign;
  float m_Operand;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_OSCameraPos()
  {
    m_eType = EParamComp_OSCameraPos;
    m_Op = 0;
    m_Sign = 1.0f;
    m_Operand = 0;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_OSCameraPos *src = (SParamComp_OSCameraPos *)p;
    if (m_eType == src->m_eType && m_Offs == src->m_Offs && m_Op == src->m_Op && m_Sign == src->m_Sign && m_Operand == src->m_Operand)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_OSCameraPos *src = (SParamComp_OSCameraPos *)p;
    m_eType = src->m_eType;
    m_Offs = src->m_Offs;
    m_Op = src->m_Op;
    m_Sign = src->m_Sign;
    m_Operand = src->m_Operand;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_ClipPlane : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_ClipPlane()
  {
    m_eType = EParamComp_ClipPlane;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_ClipPlane *src = (SParamComp_ClipPlane *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_ClipPlane *src = (SParamComp_ClipPlane *)p;
    m_eType = src->m_eType;
    m_bDependsOnObject = 1;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_MatrixTCG : public SParamComp
{
  int m_Stage;
  int m_Row;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_MatrixTCG()
  {
    m_eType = EParamComp_MatrixTCG;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_MatrixTCG *src = (SParamComp_MatrixTCG *)p;
    if (m_eType == src->m_eType && m_Stage == src->m_Stage && m_Row == src->m_Row)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_MatrixTCG *src = (SParamComp_MatrixTCG *)p;
    m_eType = src->m_eType;
    m_Stage = src->m_Stage;
    m_Row = src->m_Row;
    m_bDependsOnObject = 1;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_MatrixTCM : public SParamComp
{
  int m_Stage;
  int m_Row;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_MatrixTCM()
  {
    m_eType = EParamComp_MatrixTCM;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_MatrixTCM *src = (SParamComp_MatrixTCM *)p;
    if (m_eType == src->m_eType && m_Stage == src->m_Stage && m_Row == src->m_Row)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_MatrixTCM *src = (SParamComp_MatrixTCM *)p;
    m_eType = src->m_eType;
    m_Stage = src->m_Stage;
    m_Row = src->m_Row;
    m_bDependsOnObject = 0;
  }
  virtual void mfGet4f(vec4_t v);
};

struct SParamComp_Time : public SParamComp
{
  float m_Scale;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_Time()
  {
    m_eType = EParamComp_Time;
    m_Scale = 1.0f;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_Time *src = (SParamComp_Time *)p;
    if (m_eType == src->m_eType && m_Scale == src->m_Scale)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_Time *src = (SParamComp_Time *)p;
    m_eType = src->m_eType;
    m_Scale = src->m_Scale;
  }
  virtual float mfGet();
};

struct SParamComp_Distance : public SParamComp
{
  float m_Scale;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_Distance()
  {
    m_eType = EParamComp_Distance;
    m_Scale = 1.0f;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_Distance *src = (SParamComp_Distance *)p;
    if (m_eType == src->m_eType && m_Scale == src->m_Scale)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_Distance *src = (SParamComp_Distance *)p;
    m_eType = src->m_eType;
    m_Scale = src->m_Scale;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};

struct SParamComp_MinDistance : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_MinDistance()
  {
    m_eType = EParamComp_MinDistance;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_MinDistance *src = (SParamComp_MinDistance *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_MinDistance *src = (SParamComp_MinDistance *)p;
    m_eType = src->m_eType;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};
struct SParamComp_MinLightDistance : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_MinLightDistance()
  {
    m_eType = EParamComp_MinLightDistance;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_MinLightDistance *src = (SParamComp_MinLightDistance *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_MinLightDistance *src = (SParamComp_MinLightDistance *)p;
    m_eType = src->m_eType;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};


struct SParamComp_Random : public SParamComp
{
  float m_Scale;
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_Random()
  {
    m_eType = EParamComp_Random;
    m_Scale = 1.0f;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_Random *src = (SParamComp_Random *)p;
    if (m_eType == src->m_eType && m_Scale == src->m_Scale)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_Random *src = (SParamComp_Random *)p;
    m_eType = src->m_eType;
    m_Scale = src->m_Scale;
  }
  virtual float mfGet();
};

struct SParamComp_HeatFactor : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_HeatFactor()
  {
    m_eType = EParamComp_HeatFactor;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_HeatFactor *src = (SParamComp_HeatFactor *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_HeatFactor *src = (SParamComp_HeatFactor *)p;
    m_eType = src->m_eType;
    m_bDependsOnObject = 1;
  }
  virtual float mfGet();
};

struct SParamComp_Opacity : public SParamComp
{
  virtual int Size()
  {
    int nSize = sizeof(*this);
    return nSize;
  }
  SParamComp_Opacity()
  {
    m_eType = EParamComp_Opacity;
  }
  virtual bool mfIsEqual(SParamComp *p)
  {
    SParamComp_Opacity *src = (SParamComp_Opacity *)p;
    if (m_eType == src->m_eType)
      return true;
    return false;
  }
  virtual void mfCopy(const SParamComp* p)
  {
    SParamComp_Opacity *src = (SParamComp_Opacity *)p;
    m_eType = src->m_eType;
  }
  virtual float mfGet();
};

#define PF_CGTYPE     1
#define PF_CANMERGED  2
#define PF_DONTALLOW_DYNMERGE 4
#define PF_INTEGER    8

struct SParam
{
  static vec4_t m_sFVals;
  unsigned int m_Flags;
  unsigned int m_Reg;
  SParamComp *m_Comps[4];

  int Size()
  {
    int nSize = sizeof(SParam);
    return nSize;
  }
  SParam& operator = (const SParam& sl)
  {
    int i;

    m_Reg = sl.m_Reg;
    for (i=0; i<4; i++)
    {
      m_Comps[i] = sl.m_Comps[i];
    }
    m_Flags = sl.m_Flags;
    return *this;
  }
  
  SParam()
  {
    m_Flags = 0;
    m_Reg = 0;
    m_Comps[0] = m_Comps[1] = m_Comps[2] = m_Comps[3] = NULL;
  }
  float *mfGet()
  {
    if (m_Flags & PF_CANMERGED)
    {
      m_Comps[0]->mfGet4f(m_sFVals);
    }
    else
    {
      for (int i=0; i<4; i++)
      {
        if (m_Comps[i])
          m_sFVals[i] = m_Comps[i]->mfGet();
        else
          m_sFVals[i] = 0;
      }
    }
    return m_sFVals;
  }
  bool mfIsValid()
  {
    if (m_Comps[0] || m_Comps[1] || m_Comps[2] || m_Comps[3])
      return true;
    return false;
  }
  void mfOptimize()
  {
    if ((m_Comps[0] && m_Comps[0]->m_eType == EParamComp_ObjWave) ||
        (m_Comps[1] && m_Comps[1]->m_eType == EParamComp_ObjWave) ||
        (m_Comps[2] && m_Comps[2]->m_eType == EParamComp_ObjWave) ||
        (m_Comps[3] && m_Comps[3]->m_eType == EParamComp_ObjWave))
    {
      m_Flags |= PF_DONTALLOW_DYNMERGE;
    }
    else
    if ((m_Comps[0] && m_Comps[0]->m_eType == EParamComp_PlantsTMoving) ||
        (m_Comps[1] && m_Comps[1]->m_eType == EParamComp_PlantsTMoving) ||
        (m_Comps[2] && m_Comps[2]->m_eType == EParamComp_PlantsTMoving) ||
        (m_Comps[3] && m_Comps[3]->m_eType == EParamComp_PlantsTMoving))
    {
      m_Flags |= PF_DONTALLOW_DYNMERGE;
    }

    if (m_Comps[0] && m_Comps[0]->m_eType == EParamComp_WorldColor &&
        m_Comps[1] && m_Comps[1]->m_eType == EParamComp_WorldColor &&
        m_Comps[2] && m_Comps[2]->m_eType == EParamComp_WorldColor &&
      (!m_Comps[3] || m_Comps[3]->m_eType == EParamComp_Opacity))
    {
      m_Flags |= PF_CANMERGED;
      return;
    }
    if (m_Comps[0] && m_Comps[0]->m_eType == EParamComp_LightColor &&
        m_Comps[1] && m_Comps[1]->m_eType == EParamComp_LightColor &&
        m_Comps[2] && m_Comps[2]->m_eType == EParamComp_LightColor &&
      (!m_Comps[3] || m_Comps[3]->m_eType == EParamComp_Opacity))
    {
      m_Flags |= PF_CANMERGED;
      return;
    }
    if (m_Comps[0] && m_Comps[0]->m_eType == EParamComp_DiffuseColor &&
        m_Comps[1] && m_Comps[1]->m_eType == EParamComp_DiffuseColor &&
        m_Comps[2] && m_Comps[2]->m_eType == EParamComp_DiffuseColor &&
       (!m_Comps[3] || m_Comps[3]->m_eType == EParamComp_Opacity))
    {
      m_Flags |= PF_CANMERGED;
      return;
    }
    if (m_Comps[0] && m_Comps[0]->m_eType == EParamComp_SpecLightColor &&
        m_Comps[1] && m_Comps[1]->m_eType == EParamComp_SpecLightColor &&
        m_Comps[2] && m_Comps[2]->m_eType == EParamComp_SpecLightColor &&
      (!m_Comps[3] || m_Comps[3]->m_eType == EParamComp_Opacity))
    {
      m_Flags |= PF_CANMERGED;
      return;
    }
    if (m_Comps[0] && m_Comps[0]->m_eType == EParamComp_SunColor &&
        m_Comps[1] && m_Comps[1]->m_eType == EParamComp_SunColor &&
        m_Comps[2] && m_Comps[2]->m_eType == EParamComp_SunColor &&
        !m_Comps[3])
    {
      m_Flags |= PF_CANMERGED;
      return;
    }
    if (m_Comps[0] && m_Comps[0]->m_eType == EParamComp_AmbLightColor &&
        m_Comps[1] && m_Comps[1]->m_eType == EParamComp_AmbLightColor &&
        m_Comps[2] && m_Comps[2]->m_eType == EParamComp_AmbLightColor &&
        m_Comps[3] && m_Comps[3]->m_eType == EParamComp_Opacity)
    {
      m_Flags |= PF_CANMERGED;
      return;
    }
    if (m_Comps[0] && m_Comps[0]->m_eType == EParamComp_ObjWave &&
        m_Comps[1] && m_Comps[1]->m_eType == EParamComp_ObjWave &&
        m_Comps[2] && m_Comps[2]->m_eType == EParamComp_Bending)
    {
      m_Flags |= PF_CANMERGED;
      return;
    }

    if (m_Comps[0] && m_Comps[0]->m_eType == EParamComp_PlantsTMoving &&
        m_Comps[1] && m_Comps[1]->m_eType == EParamComp_PlantsTMoving &&
        m_Comps[2] && m_Comps[2]->m_eType == EParamComp_PlantsTMoving &&
        m_Comps[3] && m_Comps[3]->m_eType == EParamComp_PlantsTMoving)
    {
      m_Flags |= PF_CANMERGED;
      return;
    }

    if (m_Comps[0] && m_Comps[0]->m_eType == EParamComp_OSLightPos &&
        m_Comps[1] && m_Comps[1]->m_eType == EParamComp_OSLightPos &&
        m_Comps[2] && m_Comps[2]->m_eType == EParamComp_OSLightPos)
    {
      if (!m_Comps[3] || m_Comps[3]->m_eType == EParamComp_OSLightPos || (m_Comps[3]->m_eType == EParamComp_Const && m_Comps[3]->mfGet() == 1.0f))
        m_Flags |= PF_CANMERGED;
      return;
    }
    if (m_Comps[0] && m_Comps[0]->m_eType == EParamComp_SLightPos &&
        m_Comps[1] && m_Comps[1]->m_eType == EParamComp_SLightPos &&
        m_Comps[2] && m_Comps[2]->m_eType == EParamComp_SLightPos &&
        m_Comps[3] && m_Comps[3]->m_eType == EParamComp_MinLightDistance)
    {
      m_Flags |= PF_CANMERGED;
      return;
    }
    if (m_Comps[0] && m_Comps[0]->m_eType == EParamComp_LightOcclusion &&
        m_Comps[1] && m_Comps[1]->m_eType == EParamComp_LightOcclusion &&
        m_Comps[2] && m_Comps[2]->m_eType == EParamComp_LightOcclusion &&
        m_Comps[3] && m_Comps[3]->m_eType == EParamComp_LightOcclusion)
    {
      m_Flags |= PF_CANMERGED;
      return;
    }
    if (m_Comps[0] && m_Comps[0]->m_eType == EParamComp_OSCameraPos &&
        m_Comps[1] && m_Comps[1]->m_eType == EParamComp_OSCameraPos &&
        m_Comps[2] && m_Comps[2]->m_eType == EParamComp_OSCameraPos)
    {
      if (!m_Comps[3] || m_Comps[3]->m_eType == EParamComp_OSCameraPos || (m_Comps[3]->m_eType == EParamComp_Const && m_Comps[3]->mfGet() == 1.0f))
        m_Flags |= PF_CANMERGED;
      return;
    }
    if (m_Comps[0] && m_Comps[0]->m_eType == EParamComp_SCameraPos &&
        m_Comps[1] && m_Comps[1]->m_eType == EParamComp_SCameraPos &&
        m_Comps[2] && m_Comps[2]->m_eType == EParamComp_SCameraPos &&
        m_Comps[3] && m_Comps[3]->m_eType == EParamComp_MinDistance)
    {
      m_Flags |= PF_CANMERGED;
      return;
    }

    if (m_Comps[0] && m_Comps[0]->m_eType == EParamComp_LightIntens)
    {
      SParamComp_LightIntens *pcLS = (SParamComp_LightIntens *)m_Comps[0];
      if (!pcLS->m_bInv)
      {
        if (m_Comps[1] && m_Comps[1]->m_eType == EParamComp_LightIntens)
        {
          pcLS = (SParamComp_LightIntens *)m_Comps[1];
          if (pcLS->m_bInv)
          {
            if (m_Comps[2] && m_Comps[2]->m_eType == EParamComp_Const)
            {
              SParamComp_Const *pcCS = (SParamComp_Const *)m_Comps[2];
              if (pcCS->m_Val == 0.5f)
              {
                bool bMerge = false;
                if (!m_Comps[3])
                  bMerge = true;
                else
                if (m_Comps[3]->m_eType == EParamComp_Const)
                {
                  SParamComp_Const *pcCS = (SParamComp_Const *)m_Comps[3];
                  if (pcCS->m_Val == 0)
                    bMerge = true;
                }
                if (bMerge)
                  m_Flags |= PF_CANMERGED;
              }
            }
          }
        }
      }
    }
  }
};

enum ECGParam
{
  ECGP_Unknown,
  ECGP_Float4,
  ECGP_Matr_World,
  ECGP_Matr_ViewProj,
  ECGP_Matr_View,
  ECGP_Matr_View_I,
  ECGP_Matr_View_T,
  ECGP_Matr_View_IT,
  ECGP_Matr_Obj,
  ECGP_Matr_Obj_I,
  ECGP_Matr_Obj_T,
  ECGP_Matr_Obj_IT,
};

struct SCGBind
{
  CName m_Name;
  int m_dwBind;
  int m_dwFrameCreated;
  short m_nComponents;
  short m_nBindComponents;
  SCGBind *m_pNext;
  SCGBind()
  {
    m_nComponents = 1;
    m_nBindComponents = 1;
    m_pNext = NULL;
    m_dwBind = 0;
    m_dwFrameCreated = 0;
  }
};

struct SCGBindConst : SCGBind
{
  float m_Val[4];
};

struct SCGParam : SCGBind
{
  ECGParam m_eCGParamType;
  SCGParam()
  {
    m_eCGParamType = ECGP_Unknown;
  }

  int Size()
  {
    int nSize = sizeof(SCGParam);
    return nSize;
  }
};

struct SCGParam4f : public SParam, SCGParam
{
  SCGParam4f()
  {
    m_Flags = PF_CGTYPE;
  }
};

struct SCGMatrix : SCGParam
{
};

//=========================================================================
// HW matrix transformation types
struct SMatrixTransform
{
  SParam m_Params[4];
  int m_Matrix;
  int m_Offs;
  int m_Stage;
  
  virtual void mfSet(bool bSet) {};
  virtual void mfSet(Matrix44& matr) {};
  
  int Size()
  {
    int nSize = sizeof(SMatrixTransform);
    return nSize;
  }
};

struct SMatrixTransform_Identity : public SMatrixTransform
{
  virtual void mfSet(bool bSet);
  virtual void mfSet(Matrix44& matr);
};

struct SMatrixTransform_Scale : public SMatrixTransform
{
  virtual void mfSet(bool bSet);
  virtual void mfSet(Matrix44& matr);
};

struct SMatrixTransform_Translate : public SMatrixTransform
{
  virtual void mfSet(bool bSet);
  virtual void mfSet(Matrix44& matr);
};

struct SMatrixTransform_Matrix : public SMatrixTransform
{
  virtual void mfSet(bool bSet);
  virtual void mfSet(Matrix44& matr);
};

struct SMatrixTransform_LightCMProject : public SMatrixTransform
{
  virtual void mfSet(bool bSet);
  virtual void mfSet(Matrix44& matr);
};

struct SMatrixTransform_Rotate : public SMatrixTransform
{
  virtual void mfSet(bool bSet);
  virtual void mfSet(Matrix44& matr);
};


//=========================================================================
// Dynamic lights evaluating via shaders

enum ELightStyle
{
  eLS_Intensity,
  eLS_RGB,
};

enum ELightMoveType
{
  eLMT_Wave,
  eLMT_Patch,
};

struct SLightMove
{
  ELightMoveType m_eLMType;
  SWaveForm m_Wave;
  Vec3d m_Dir;
  float m_fSpeed;

  int Size()
  {
    int nSize = sizeof(SLightMove);
    return nSize;
  }
};

//
struct SLightEval
{
  SLightEval()
  {
    memset(this, 0, sizeof(SLightEval));
  }

  SLightMove *m_LightMove;
  ELightStyle m_EStyleType;  
  int m_LightStyle;
  SParam m_ProjRotate;
  Vec3d m_LightRotate;
  Vec3d m_LightOffset;

  int Size()
  {
    int nSize = sizeof(SLightEval);
    if (m_LightMove)
      nSize += m_LightMove->Size();

    return nSize;
  }
  ~SLightEval()
  {
    SAFE_DELETE(m_LightMove);
  }
};

class CLightStyle
{
public:
  TArray<CFColor> m_Map;
  float m_TimeIncr;
  CFColor m_Color;
  float m_fIntensity;
  float m_LastTime;

  static TArray <CLightStyle *> m_LStyles;  

  int Size()
  {
    int nSize = sizeof(CLightStyle);
    nSize += m_Map.GetSize() * sizeof(CFColor);
    return nSize;
  }

  CLightStyle()
  {
    m_LastTime = 0;
    m_fIntensity = 1.0f;
    m_Color = Col_White;
    m_TimeIncr = 60.0f;
  }
  static _inline CLightStyle *mfGetStyle(int nStyle, float fTime)
  {
    if (nStyle >= m_LStyles.Num() || !m_LStyles[nStyle])
      return NULL;
    m_LStyles[nStyle]->mfUpdate(fTime);
    return m_LStyles[nStyle];
  }
  _inline void mfUpdate(float fTime)
  {
    float m = fTime * m_TimeIncr;
    if (m != m_LastTime)
    {
      m_LastTime = m;
      if (m_Map.Num())
      {
        if (m_Map.Num() == 1)
        {
          m_Color = m_Map[0];
        }
        else
        {
          int first = (int)QInt(m);
          int second = (first + 1);
          CFColor A = m_Map[first % m_Map.Num()];
          CFColor B = m_Map[second % m_Map.Num()];
          float fLerp = m-(float)first;
          m_Color = LERP(A, B, fLerp);
        }
        m_fIntensity = (m_Color.r * 0.3f) + (m_Color.g * 0.59f) + (m_Color.b * 0.11f);
      }
    }
  }
};

#ifndef GL_BYTE

#define GL_BYTE                             0x1400
#define GL_UNSIGNED_BYTE                    0x1401
#define GL_SHORT                            0x1402
#define GL_UNSIGNED_SHORT                   0x1403
#define GL_INT                              0x1404
#define GL_UNSIGNED_INT                     0x1405
#define GL_FLOAT                            0x1406
#define GL_2_BYTES                          0x1407
#define GL_3_BYTES                          0x1408
#define GL_4_BYTES                          0x1409
#define GL_DOUBLE                           0x140A

#endif



#define PFE_POINTER_VERT      1
#define PFE_POINTER_NORMAL    2
#define PFE_POINTER_COLOR     4
#define PFE_POINTER_SECCOLOR  8
#define PFE_POINTER_FOGCOORD  0x10
#define PFE_POINTER_TEX0      0x20

struct SArrayPointer
{
  static int m_CurEnabled;
  static int m_LastEnabled;
  static int m_CurEnabledPass;
  static int m_LastEnabledPass;

  ESrcPointer ePT;       // Current source (data place)
  EDstPointer eDst;      // Destination target
  int Stage;
  int Type;
  int NumComponents;

  int Size()
  {
    int nSize = sizeof(SArrayPointer);
    return nSize;
  }

  SArrayPointer()
  {
    ePT = eSrcPointer_Unknown;
    Type = 0;
    Stage = 0;
    NumComponents = 0;
  }
  bool operator == (SArrayPointer p)
  {
    if (ePT==p.ePT && Stage==p.Stage && Type==p.Type && NumComponents==p.NumComponents)
      return true;
    return false;
  }

  virtual void mfSet(int Id) {}
  virtual bool mfCompile(char *scr, SShader *ef) {return false;}
  virtual const char *mfGetName() {return NULL;}

  static TArray<SArrayPointer *> m_Arrays;
  static SArrayPointer *AddNew(SArrayPointer& New);
};

struct SArrayPointer_Vertex : public SArrayPointer
{
  static void *m_pLastPointer;
  static int m_nFrameCreateBuf;
  virtual void mfSet(int Id);
  virtual bool mfCompile(char *scr, SShader *ef);
};

struct SArrayPointer_Normal : public SArrayPointer
{
  static void *m_pLastPointer;
  static int m_nFrameCreateBuf;
  SArrayPointer_Normal()
  {
    ePT = eSrcPointer_Normal;
    eDst = eDstPointer_Normal;
    NumComponents = 3;
    Type = GL_FLOAT;
  }
  virtual void mfSet(int Id);
  virtual bool mfCompile(char *scr, SShader *ef);
};

struct SArrayPointer_Texture : public SArrayPointer
{
  static void *m_pLastPointer[8];
  static int m_nFrameCreateBuf[8];
  virtual void mfSet(int Id);
  virtual bool mfCompile(char *scr, SShader *ef);
};

struct SArrayPointer_Color : public SArrayPointer
{
  static void *m_pLastPointer;
  static int m_nFrameCreateBuf;
  virtual void mfSet(int Id);
  virtual bool mfCompile(char *scr, SShader *ef);
};

struct SArrayPointer_SecColor : public SArrayPointer_Color
{
  static void *m_pLastPointer;
  static int m_nFrameCreateBuf;
  virtual void mfSet(int Id);
  virtual bool mfCompile(char *scr, SShader *ef);
};

//================================================================
// Deformations / Morphing
enum EDeformType
{
  eDT_Unknown,
  eDT_Wave,
  eDT_VerticalWave,
  eDT_Bulge,
  eDT_Squeeze,
  eDT_FromCenter,
  eDT_Flare,
  eDT_Beam,
};

// Deformation stage definition
struct SDeform
{
  EDeformType m_eType;
  SWaveForm m_DeformGen;
  float m_ScaleVerts;
  float m_fFlareSize;

  int Size()
  {
    int nSize = sizeof(SDeform);
    return nSize;
  }
};


//=========================================================================
// HW Shader Layer

struct ICVar;

#define SHPF_TANGENTS        1
#define SHPF_LMTC            2
#define SHPF_NORMALS         4
#define SHPF_AMBIENT         8
#define SHPF_HASLM           16
#define SHPF_SHADOW          32
#define SHPF_RADIOSITY       64
#define SHPF_ALLOW_SPECANTIALIAS 128
#define SHPF_BUMP            256
#define SHPF_USEDSECONDRS    512

struct CVarCond
{
  ICVar *m_Var;
  float m_Val;

  int Size()
  {
    int nSize = sizeof(CVarCond);
    return nSize;
  }
};

// General Shader Pass definitions (include Texture layers for Multitexturing)
struct SShaderPass   
{
  TArray <SShaderTexUnit> m_TUnits;  // List of texture units

  uint m_RenderState;          // Render state flags
  uint m_SecondRenderState;    // Render state flags for all subsequent passes

  SWaveForm *m_WaveEvalRGB;    // Wave form definition for RGB evaluation (usually software evaluations)
  SRGBGenNoise *m_RGBNoise;    // Noise parameters definition for RGB evaluation
  SParam *m_RGBComps;          // Components declarations for RGB evaluation

  EEvalRGB m_eEvalRGB;         // Type of RGB evaluation

  SWaveForm *m_WaveEvalAlpha;  // Wave form definition for Alpha evaluation (usually software evaluations)
  SAlphaGenNoise *m_ANoise;    // Noise parameters definition for Alpha evaluation
  EEvalAlpha m_eEvalAlpha;     // Type of Alpha evaluation

  UCol m_FixedColor;           // Constant RGBA values
  ushort m_Flags;              // Different usefull Pass flags (SHPF_)
  byte m_Style;                // Light style Id for RGBA animating using LightStyle tables

  //=====================================================

  SShaderPass()
  {
    m_RenderState = GS_DEPTHWRITE;
    m_SecondRenderState = GS_BLSRC_ONE | GS_BLDST_ONE;

    m_WaveEvalRGB = NULL;
    m_RGBNoise = NULL;
    m_RGBComps = NULL;
    m_eEvalRGB = eERGB_NoFill;

    m_WaveEvalAlpha = NULL;
    m_ANoise = NULL;
    m_eEvalAlpha = eEALPHA_NoFill;
    m_FixedColor.dcolor = -1;
    m_Flags = 0;
    m_Style = 0;
  }

  int Size()
  {
    int i;
    int nSize = sizeof(SShaderPass);
    for (i=0; i<m_TUnits.Num(); i++)
    {
      nSize += m_TUnits[i].Size();
    }
    if (m_WaveEvalRGB)
      nSize += m_WaveEvalRGB->Size();
    if (m_WaveEvalAlpha)
      nSize += m_WaveEvalAlpha->Size();
    if (m_RGBComps)
      nSize += m_RGBComps->Size();
    if (m_RGBNoise)
      nSize += m_RGBNoise->Size();
    if (m_ANoise)
      nSize += m_ANoise->Size();

    return nSize;
  }
  SShaderPass& operator = (const SShaderPass& sl)
  {
    memcpy(this, &sl, sizeof(SShaderPass));
    if (sl.m_WaveEvalAlpha)
    {
      m_WaveEvalAlpha = new SWaveForm;
      *m_WaveEvalAlpha = *sl.m_WaveEvalAlpha;
    }
    if (sl.m_RGBComps)
    {
      m_RGBComps = new SParam;
      *m_RGBComps = *sl.m_RGBComps;
    }
    if (sl.m_WaveEvalRGB)
    {
      m_WaveEvalRGB = new SWaveForm;
      *m_WaveEvalRGB = *sl.m_WaveEvalRGB;
    }
    TArray<SShaderTexUnit> L;
    m_TUnits = L;
    if (sl.m_TUnits.Num())
    {
      mfAddNewTexUnits(sl.m_TUnits.Num());
      for (int j=0; j<sl.m_TUnits.Num(); j++)
      {
        sl.m_TUnits[j].mfCopy(&m_TUnits[j]);
      }
    }
    return *this;
  }

  void mfAddNewTexUnits(int nums)
  {
    SShaderTexUnit tl;
    for (int i=0; i<nums; i++)
    {
      m_TUnits.AddElem(tl);
    }
    m_TUnits.Shrink();
  }
  void mfFree()
  {
    if (m_WaveEvalAlpha)
    {
      delete m_WaveEvalAlpha;
      m_WaveEvalAlpha = NULL;
    }
    if (m_WaveEvalRGB)
    {
      delete m_WaveEvalRGB;
      m_WaveEvalRGB = NULL;
    }
    for (int j=0; j<m_TUnits.Num(); j++)
    {
      SShaderTexUnit *tl = &m_TUnits[j];
      tl->mfFree();
    }
    m_TUnits.Free();
    if (m_RGBComps)
      delete m_RGBComps;
  }
  bool mfSetTextures();
  void mfResetTextures();
};

enum EShaderPassType
{
  eSHP_General = 0,
  eSHP_Light,
  eSHP_DiffuseLight,
  eSHP_SpecularLight,
  eSHP_MultiLights,
  eSHP_Shadow,
  eSHP_MultiShadows,
  eSHP_Fur,
  eSHP_SimulatedFur,
  eSHP_MAX,
};

// Shader pass definition for HW shaders
struct SShaderPassHW : public SShaderPass
{ 
  EShaderPassType m_ePassType;
  int m_LMFlags;                  // Light material flags (LMF_)
  int m_LightFlags;               // Dynamic light sources flag (DLF_)
  int m_nAmbMaxLights;            // Number of PP light sources if ambient pass is present

  CVProgram *m_VProgram;          // Pointer to the vertex shader for the current pass
  TArray<SCGParam4f>   m_VPParamsNoObj;  // Vertex shader parameters
  TArray<SCGParam4f>   m_VPParamsObj;  // Vertex shader parameters

  CPShader *m_FShader;         // Pointer to fragment shader (could be CG PShader or register combiner)
  TArray<SCGParam4f> *m_CGFSParamsNoObj;  // Pixel shader parameters
  TArray<SCGParam4f> *m_CGFSParamsObj;   // Pixel shader parameters

  TArray<SArrayPointer *> m_Pointers;     // Vertex attributes definitions
  TArray<SMatrixTransform> *m_MatrixOps;  // Matrix transformations
  TArray<SDeform> *m_Deforms;             // Deform stages (software evaluation)
  
  SShaderPassHW();

  int Size()
  {
    int nSize = sizeof(SShaderPassHW) - sizeof(SShaderPass);
    nSize += SShaderPass::Size();
    nSize += sizeof(SCGParam4f) * m_VPParamsNoObj.GetSize();
    nSize += sizeof(SCGParam4f) * m_VPParamsObj.GetSize();
    if (m_CGFSParamsNoObj)
      nSize += sizeof(SParam) * m_CGFSParamsNoObj->GetSize();
    if (m_CGFSParamsObj)
      nSize += sizeof(SParam) * m_CGFSParamsObj->GetSize();
    if (m_MatrixOps)
      nSize += m_MatrixOps->GetSize() * sizeof(SMatrixTransform) + 12;
    if (m_Deforms)
    {
      for (int i=0; i<m_Deforms->GetSize(); i++)
      {
        nSize += m_Deforms->Get(i).Size();
      }
    }
    return nSize;
  }
  void mfFree()
  {
    SShaderPass::mfFree();

    if (m_CGFSParamsNoObj)
      delete m_CGFSParamsNoObj;
    if (m_CGFSParamsObj)
      delete m_CGFSParamsObj;
    m_Pointers.Free();
    m_VPParamsNoObj.Free();
    m_VPParamsObj.Free();
    SAFE_DELETE(m_MatrixOps);
    SAFE_DELETE(m_Deforms);
    SAFE_RELEASE(m_VProgram);
    SAFE_RELEASE(m_FShader);
  }

  SShaderPassHW& operator = (const SShaderPassHW& sl)
  {
    SShaderPass *dsl = (SShaderPass *)this;
    *dsl = sl;

    if (sl.m_VPParamsNoObj.Num())
      m_VPParamsNoObj.Copy(sl.m_VPParamsNoObj);
    if (sl.m_CGFSParamsNoObj)
    {
      m_CGFSParamsNoObj = new TArray<SCGParam4f>;
      m_CGFSParamsNoObj->Copy(*sl.m_CGFSParamsNoObj);
    }
    if (sl.m_CGFSParamsObj)
    {
      m_CGFSParamsObj = new TArray<SCGParam4f>;
      m_CGFSParamsObj->Copy(*sl.m_CGFSParamsObj);
    }
    if (sl.m_VPParamsObj.Num())
      m_VPParamsObj.Copy(sl.m_VPParamsObj);

    if (sl.m_Deforms)
    {
      m_Deforms = new TArray<SDeform>;
      m_Deforms->Copy(*sl.m_Deforms);
    }

    if (sl.m_Pointers.Num())
    {
      m_Pointers.Copy(sl.m_Pointers);
    }

    if (sl.m_MatrixOps)
    {
      m_MatrixOps = new TArray<SMatrixTransform>;
      m_MatrixOps->Copy(*sl.m_MatrixOps);
    }
    if (sl.m_VProgram)
      sl.m_VProgram->m_nRefCounter++;
    if (sl.m_FShader)
      sl.m_FShader->m_nRefCounter++;

    return *this;
  }
};

//===================================================================================
// Hardware Stage for HW only Shaders

#define SHCF_NOLIGHTS        1
#define SHCF_SINGLELIGHT     2
#define SHCF_MULTIPLELIGHTS  4
#define SHCF_ONLYDIRECTIONAL 0x10
#define SHCF_HASPROJECTEDLIGHTS 0x20
#define SHCF_LIGHT (SHCF_ONLYDIRECTIONAL | SHCF_HASPROJECTEDLIGHTS | SHCF_MULTIPLELIGHTS | SHCF_SINGLELIGHT | SHCF_NOLIGHTS | SHCF_HEATVISION)
#define SHCF_IGNOREDIRECTIONAL 0x40
#define SHCF_HASVCOLORS      0x80
#define SHCF_HASALPHATEST    0x100
#define SHCF_HASALPHABLEND   0x200
#define SHCF_INFOGVOLUME     0x400
#define SHCF_HASLM           0x800
#define SHCF_RETEXBIND1      0x1000
#define SHCF_RETEXBIND2      0x2000
#define SHCF_RETEXBIND3      0x3000
#define SHCF_RETEXBIND4      0x4000
#define SHCF_RETEXBIND5      0x5000
#define SHCF_RETEXBIND6      0x6000
#define SHCF_RETEXBIND7      0x7000
#define SHCF_RETEXBIND_MASK  0x7000
#define SHCF_HASDOT3LM       0x8000
#define SHCF_HASRESOURCE     0x10000
#define SHCF_SPECULAR        FOB_SPECULAR
#define SHCF_BENDED          FOB_BENDED
#define SHCF_ALPHABLENDED    FOB_HASALPHA
#define SHCF_NOBUMP          FOB_NOBUMP
#define SHCF_INSHADOW        FOB_INSHADOW
#define SHCF_HEATVISION      FOB_HEATVISION
#define SHCF_HOTAMBIENT      FOB_HOTAMBIENT
#define SHCF_ENVLCMAP        FOB_ENVLIGHTING

// Hardware technique conditions:
// Render pipeline choses appropriate technique based on some flags, console variables,
// parameters (conditions).
// Number of conditions (in the list: m_HWConditions) should be exactly the same as number of HW techniques (in the list: m_HWTechniques)
struct SHWConditions
{
  int m_Flags;       // conditions flags
  int m_NumVars;     // number of console variables
  CVarCond *m_Vars;  // list of the console variables

  int Size()
  {
    int nSize = sizeof(SHWConditions);
    for (int i=0; i<m_NumVars; i++)
    {
      nSize += m_Vars[i].Size();
    }
    return nSize;
  }

  SHWConditions()
  {
    m_Flags = 0;
    m_NumVars = 0;
    m_Vars = NULL;
  }
};

#define FHF_TANGENTS     SHPF_TANGENTS
#define FHF_LMTC         SHPF_LMTC
#define FHF_NORMALS      SHPF_NORMALS
#define FHF_FIRSTLIGHT   8
#define FHF_FORANIM      0x10
#define FHF_TERRAIN      0x20
#define FHF_NOMERGE      0x40

struct SShaderTechnique
{
  int m_Id;                                // Technique Id (order in tech. list Id)
  ECull m_eCull;                           // Culling type
  TArray<SArrayPointer *> m_Pointers;      // Vertex attributes definitions
  TArray<SMatrixTransform> *m_MatrixOps;   // Matrix transformations
  TArray <SShaderPassHW> m_Passes;         // General passes
  int m_Flags;                             // Different flags (FHF_)

  int Size()
  {
    int i;
    int nSize = sizeof(SShaderTechnique);
    nSize += m_Pointers.GetSize() * sizeof(SArrayPointer *);
    if (m_MatrixOps)
    {
      nSize += 12;
      for (i=0; i<m_MatrixOps->GetSize(); i++)
      {
        nSize += m_MatrixOps->Get(i).Size();
      }
    }
    for (i=0; i<m_Passes.GetSize(); i++)
    {
      nSize += m_Passes[i].Size();
    }
    return nSize;
  }
  SShaderTechnique()
  {
    m_MatrixOps = NULL;
    m_Flags = 0;
    m_eCull = (ECull)-1;
  }
  SShaderTechnique& operator = (const SShaderTechnique& sl)
  {
    memcpy(this, &sl, sizeof(SShaderTechnique));
    if (sl.m_Passes.Num())
    {
      m_Passes.Copy(sl.m_Passes);
      for (int i=0; i<sl.m_Passes.Num(); i++)
      {
        const SShaderPassHW *s = &sl.m_Passes[i];
        SShaderPassHW *d = &m_Passes[i];
        *d = *s;
      }
    }

    if (sl.m_Pointers.Num())
    {
      m_Pointers.Copy(sl.m_Pointers);
    }

    if (sl.m_MatrixOps)
    {
      m_MatrixOps = new TArray<SMatrixTransform>;
      m_MatrixOps->Copy(*sl.m_MatrixOps);
    }

    return *this;
  }

  ~SShaderTechnique()
  {
    for (int i=0; i<m_Passes.Num(); i++)
    {
      SShaderPassHW *sl = &m_Passes[i];
      
      sl->mfFree();
    }
    m_Passes.Free();
  }

#ifdef DEBUGALLOC
#undef new
#endif
  void* operator new( size_t Size ) { void *ptr = malloc(Size); memset(ptr, 0, Size); return ptr; }
  void operator delete( void *Ptr ) { free(Ptr); }
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
};

//===============================================================================

//Timur[9/16/2002]
struct SMapTexInfoOld
{
  char name[64];
  float shift[2];
  float rotate;
  float scale[2];
  int   contents;
  int   flags;
  int   value;

  SMapTexInfoOld()
  {
    memset(this, 0, sizeof(SMapTexInfoOld));
    strcpy(name, "T00/notex");
    scale[0] = scale[1] = 0.5f;
  }
  ~SMapTexInfoOld()
  {
  }
  void SetName(const char *p)
  {
    strcpy(name, p);
  }
};


struct SNormalsGen
{
  EEvalNormal m_eNormal;
  Vec3d m_CustomNormal;
  SWaveForm m_WaveEvalNormal;

  int Size()
  {
    int nSize = sizeof(SNormalsGen);
    return nSize;
  }
};

// Fill type/components (for optimal update)
#define FLT_BASE 1
#define FLT_COL  2
#define FLT_LM   4
#define FLT_SYSBASE 8
#define FLT_SYSLM 16
#define FLT_N    32

struct SSideMaterial;

// General Shader structure
struct SShader : public IShader
{
  string m_Name;       // } FIXME: This fields order is very important
  int m_Id;                 // } (see SShader::operator =)
  
  int m_nRefCounter;
  float m_fProfileTime;
  uint m_Flags;             // Different usefull flags EF_  (see IShader.h)
  uint m_Flags2;            // Different usefull flags EF2_ (see IShader.h)
  uint m_Flags3;            // Different usefull flags EF3_ (see IShader.h)
  ECull m_eCull;            // Base shader culling type (Can be overriden by Pass m_eCull value)
  EF_Sort m_eSort;          // Sort category eS_ (see IShader.h)
  EShClass m_eClass;        // Shader class eSH_ (see IShader.h)
  float m_fUpdateFactor;           // Updating time factor for drawing to the texture
  uint m_DLDFlags;                 // Preprocessor shader flags for "DrawLowDetail" function (see I3DEngine.h)
  int m_nPreprocess;        // Preprocess flags for shader (see IShader.h)

  SEfTemplates *m_Templates; // List of all templates registered in this shader
  int m_VertexFormatId;      // Base vertex format for the shader (see VertexFormats.h)
  
/////////////////////////////////////////////////////////
// Passes definitions (only common shaders)
/////////////////////////////////////////////////////////
  TArray<SShaderPass> m_Passes;
/////////////////////////////////////////////////////////

  // Hardware techniques (only HW shaders)
  TArray<SHWConditions> m_HWConditions;  // List of all conditions
  TArray <SShaderTechnique *> m_HWTechniques;
  CVProgram *m_DefaultVProgram;
  TArray<SShaderParam> m_PublicParams; // Shader public params (can be enumerated by func.: GetPublicParams)
    
  TArray<SDeform> *m_Deforms; // Deform stages (software evaluation)
  SNormalsGen *m_NormGen; // Custom Generating of Normals (software evaluation)
  SFogInfo *m_FogInfo;    // Volume fog info
  SSkyInfo *m_Sky;        // Sky zone info
  CSunFlares *m_Flares;   // Lens flares from the sun/light source
  TArray<CRendElement *> m_REs; // List of all render elements registered in the shader

  SLightEval *m_EvalLights;        // Light evaluation parameters (only for light shaders)
  int m_LMFlags;                   // Light material types flag

  SEfState *m_State;               // Special state (valid only for state shaders)

  uint64 m_nMaskGen;
  SShaderGen *m_ShaderGenParams;   // BitMask params used in automatic script generation
  TArray<SShader *> *m_DerivedShaders;
  SShader *m_pGenShader;

  int m_nRefreshFrame; // Current frame for shader reloading (to avoid multiple reloading)

  FILETIME m_WriteTime;

//============================================================================

  void mfFree();
  SShader()
  {
    m_fProfileTime = 0;
    m_eSort = eS_Unknown;
  }
  virtual ~SShader();

  //===================================================================================

  // IShader interface
	virtual void AddRef() { m_nRefCounter++; };
  virtual int GetID() { return m_Id; }
  virtual int GetRefCount() { return m_nRefCounter; }
  virtual const char *GetName() { return m_Name.c_str(); }
  virtual EF_Sort GetSort() { return m_eSort; }
  virtual int GetCull() { return m_eCull; }
  virtual int GetFlags() { return m_Flags; }
  virtual int GetFlags2() { return m_Flags2; }
  virtual int GetFlags3() { return m_Flags3; }
  virtual int GetLFlags() { return m_LMFlags; }
  virtual void SetFlags3(int Flags) { m_Flags3 |= Flags; }
  virtual int GetRenderFlags() { return m_DLDFlags; }
  virtual void SetRenderFlags(int nFlags) { m_DLDFlags = nFlags; }
  virtual bool Reload(int nFlags);
  virtual TArray<CRendElement *> *GetREs () { return &m_REs; }
  virtual bool AddTemplate(SRenderShaderResources *Res, int& TemplId, const char *Name=NULL, bool bSetPreferred=true, uint64 nMaskGen=0);
  virtual void RemoveTemplate(int TemplId);
  virtual IShader *GetTemplate(int num) { return (IShader *)mfGetTemplate(num); }
  virtual SEfTemplates *GetTemplates () { return m_Templates; }
  virtual int GetTexId ();
  virtual unsigned int GetUsedTextureTypes (void);
  virtual uint GetPreprocessFlags() { return m_nPreprocess; }
  virtual int GetVertexFormat(void) { return m_VertexFormatId; }
	virtual uint64 GetGenerationMask() { return m_nMaskGen; }
	virtual SShaderGen* GetGenerationParams()
  {
    if (m_ShaderGenParams)
      return m_ShaderGenParams;
    if (m_pGenShader)
      return m_pGenShader->m_ShaderGenParams;
    return NULL;
  }

  virtual TArray<SShaderParam>& GetPublicParams()
  {
    return m_PublicParams;
  }
  virtual void Release(bool bForce=false)
  {
#ifndef NULL_RENDERER
    if (!bForce)
    {
      if (m_Flags & EF_SYSTEM)
        return;
      m_nRefCounter--;
      if (m_nRefCounter)
        return;
    }
    delete this;
#endif
  }

  //=======================================================================================

  virtual void mfNewTemplates()
  {
    m_Templates = new SEfTemplates;
  }
  virtual void mfDeleteTemplates()
  {
    delete m_Templates;
    m_Templates = NULL;
  }

  SShader *mfGetTemplate(int num)
  {
    if (m_Templates)
    {
      if (num>=0 && num < m_Templates->m_TemplShaders.Num() && m_Templates->m_TemplShaders[num])
        return m_Templates->m_TemplShaders[num];
      else
      if (m_Templates->m_Preferred)
        return m_Templates->m_Preferred;
    }
    return this;
  }

  virtual bool mfSetOpacity(float Opa, int Mode); // Mode=0 - Opacity, Mode=1 - Additive
  virtual void mfAddDeform(SDeform& ds)
  {
    if (!m_Deforms)
      m_Deforms = new TArray<SDeform>;
    m_Deforms->AddElem(ds);
  }
  virtual void mfRemoveDeform(int nOffset)
  {
    if (!m_Deforms)
      return;
    m_Deforms->Remove(nOffset, 1);
    if (!m_Deforms->Num())
    {
      m_Deforms->Free();
      delete [] m_Deforms;
      m_Deforms = NULL;
    }
  }
  virtual ITexPic *GetBaseTexture(int *nPass, int *nTU);
  
  SShader& operator = (const SShader& src);
  STexPic *mfFindBaseTexture(TArray<SShaderPass>& Passes, int *nPass, int *nTU, int nT);
  STexPic *mfFindBaseTexture(TArray<SShaderPassHW>& Passes, int *nPass, int *nTU, int nT);

  int mfSize();

  // All loaded shaders list
  static TArray<SShader *> m_Shaders_known;
  static TArray<SRenderShaderResources *> m_ShaderResources_known;
  
  virtual bool mfIsValidTime(CCObject *obj, float curtime);
  virtual int Size(int Flags)
  {
    return mfSize();
  }

#ifdef DEBUGALLOC
#undef new
#endif
  void* operator new( size_t Size ) { void *ptr = malloc(Size); memset(ptr, 0, Size); return ptr; }
  void operator delete( void *Ptr ) { free(Ptr); }
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
};


//////////////////////////////////////////////////////////////////////////

inline void 
SParamComp_Matrix::mfCopy(const SParamComp* p)
{
	SParamComp_Matrix *src = (SParamComp_Matrix *)p;
	m_eType = src->m_eType;
	m_Offs = src->m_Offs;
	m_Transforms.Copy(src->m_Transforms);
	m_bDependsOnObject = 1;
}


#endif

