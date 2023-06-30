
#ifndef __CSHADER_H__
#define __CSHADER_H__

#include <map>

struct SRenderBuf;
class CRendElement;
struct SEmitter;
struct SParticleInfo;
struct SPartMoveStage;
struct SSunFlare;

//===================================================================

#define MAX_ENVLIGHTCUBEMAPS 16
#define ENVLIGHTCUBEMAP_SIZE 16
#define MAX_ENVLIGHTCUBEMAPSCANDIST_UPDATE 16
#define MAX_ENVLIGHTCUBEMAPSCANDIST_THRESHOLD 2

#define MAX_ENVCUBEMAPS 4
#define MAX_ENVCUBEMAPSCANDIST_THRESHOLD 1

#define MAX_ENVTEXTURES 4
#define MAX_ENVTEXSCANDIST 0.1f

struct SNameAlias
{
  CName m_Alias;
  CName m_Name;
};

struct SEnvTexture
{
  bool m_bInprogress;
  bool m_bReady;
  bool m_bWater;
  bool m_bReflected;
  int m_MaskReady;
  int m_Id;
  int m_TexSize;
  // Result Cube-Map or 2D RT texture
  STexPic *m_Tex;
  // Temporary 2D texture
  STexPic *m_TexTemp;
  float m_TimeLastUsed;
  Vec3d m_CamPos;
  Vec3d m_ObjPos;
  Vec3d m_Angle;
  int m_nFrameReset;
  // Cube maps average colors (used for RT radiosity)
  int m_nFrameCreated[6];
  UCol m_EnvColors[6];
  void *m_RenderTargets[6];
};

//===================================================================

#define MAX_EF_FILES 256

struct SRegTemplate
{
  char m_Name[64];
  SShader *m_pShader;
};


#define SF_RELOAD 1

class CShader
{
friend struct SShader;

private:

  void mfStartScriptPreprocess();
  int mfRemoveScript_ifdef(char *posStart, char *posEnd, bool bRemoveAll, int nPos, char *buf, const char *fileName);
  char *mfPreprCheckMacros (char *buf, const char *nameFile);
  char *mfPreprCheckConditions (char *buf, const char *nameFile);
  char *mfPreprCheckIncludes (char *buf, const char *drn, const char *name);
  int mfLoadSubdir (char *drn, int n);

  int mfReadTexSequence(SShader *ef, TArray<STexPic *>& txs, const char *name, byte eTT, int Flags, int Flags2, float fAmount1=-1.0f, float fAmount2=-1.0f);

  SShader *mfNewShader(EShClass Class, int num);

  char *mfRescanScript(int type, int nInd, SShader *pSHOrg, uint64 nMaskGen);
  void mfScanScript (char *scr, int n);
  bool mfCompileShaderGen(SShader *ef, SShaderGen *shg, char *scr);
  SShaderGenBit *mfCompileShaderGenProperty(SShader *ef, char *scr);
  SShader *mfCompileShader(SShader *ef, char *scr);

  void mfCompileFogParms(SShader *ef, char *scr);
  void mfCompileDeform(SShader *ef, SDeform *df, char *dname, char *scr);
  bool mfCompileSunFlares(SShader *ef, char *name, char *scr);
  bool mfCompileFlare(SShader *ef, SSunFlare *fl, char *scr);
  bool mfCompileParams(SShader *ef, char *scr);
  bool mfCompileRenderParams(SShader *ef, char *scr);
  bool mfCompilePublic(SShader *ef, char *scr);
  void mfParseLightStyle(CLightStyle *ls, char *lstr);
  bool mfCompileLightStyle(SShader *ef, int num, char *scr);
  void mfCompileParamComps(SParam *pr, char *scr, SShader *ef);
  uint mfCompileRendState(SShader *ef, SShaderPass *sm, char *scr);
  bool mfCompileSequence(SShader *ef, SShaderTexUnit *tl, int nLayer, char *scr, ETexType eTT, int Flags, int Flags2, float fBumpAmount);
  void mfCompileOrient(SShader *ef, int num, char *scr);

  void mfCompileLightMove(SShader *ef, char *nameMove, SLightEval *le, char *scr);
  void mfCompileEvalLight(SShader *ef, char *scr);
  void mfCompileStencil(SShader *ef, char *params);
  void mfCompileState(SShader *ef, char *params);

  STexPic *mfTryToLoadTexture(const char *nameTex, int Flags, int Flags2, byte eTT, SShader *sh, float fAmount1=-1.0f, float fAmount2=-1.0f);
  STexPic *mfLoadResourceTexture(const char *nameTex, const char *path, int Flags, int Flags2, byte eTT, SShader *sh, SEfResTexture *Tex, float fAmount1=-1.0f, float fAmount2=-1.0f);
  bool mfCheckAnimatedSequence(SShaderTexUnit *tl, STexPic *tx);
  STexPic *mfCheckTemplateTexName(char *mapname, ETexType eTT, short &nFlags);
  void mfCheckShaderResTextures(TArray<SShaderPass> &Dst, SShader *ef, SRenderShaderResources *Res);
  void mfCheckShaderResTexturesHW(TArray<SShaderPassHW> &Dst, SShader *ef, SRenderShaderResources *Res);
  int mfTemplateNameToId(char *name);
  void mfCompileTemplate(SShader *ef, char *scr);
  bool mfCompileTexGen(char *name, char *params, SShader *ef, SShaderTexUnit *ml);

  SShader *mfCompile(SShader *ef, char *scr);

  void mfCompileLayers(SShader *ef, char *scr, TArray<SShaderPassHW>& Layers, EShaderPassType eType);
  void mfCompileVarsPak(char *scr, TArray<CVarCond>& Vars, SShader *ef);
  void mfCompileHWConditions(SShader *ef, char *scr, SShaderTechnique *hs, int Id);
  SShaderTechnique *mfCompileHW(SShader *ef, char *scr, int Id);
  bool mfUpdateMergeStatus(SShaderTechnique *hs, TArray<SCGParam4f> *p);
  bool mfCompileHWShadeLayer(SShader *ef, char *scr, TArray<SShaderPassHW>& Layers);

  void mfClEfCompile(SShader *ef, char *scr, char *name);
  
  bool mfReloadShaderScript(const char *szShaderName, int nFlags, SShader *pSH);
  void mfRefreshResources(SShader *eft, SRenderShaderResources *Res);
  bool mfAddTemplate(SRenderShaderResources *Res, SShader *ef, int IdTempl, const char *Name=NULL, uint64 nMaskGen=0);
  void mfRegisterDefaultTemplates();
  void mfUnregisterDefaultTemplates();
  bool mfSetOpacity (SShaderPass *Layer, float Opa, SShader *ef, int Mode);
  void mfRefreshLayer(SShaderPass *sl, SShader *sh);
  bool mfReloadShader(const char *szName, int nFlags);
  bool mfReloadShaderFile(const char *szName, int nFlags);
  void mfCheckAffectedFiles(const char *ShadersPath, int nCheckFile, TArray<char *>& CheckNames, TArray<char *>& AffectedFiles);

public:
  char *m_pCurScript;
  ShaderMacro m_Macros;
  TArray<SLocalMacros> m_LocalMacros;

  bool PackCache();
  SShaderCacheHeaderItem *GetCacheItem(SShaderCache *pCache, int nMask);
  bool FreeCacheItem(SShaderCache *pCache, int nMask);
  bool AddCacheItem(SShaderCache *pCache, SShaderCacheHeaderItem *pItem, byte *pData, int nLen, bool bFlush);
  SShaderCache *OpenCacheFile(const char *szName, float fVersion);
  bool FlushCacheFile(SShaderCache *pCache);
  bool CloseCacheFile(SShaderCache *pCache);

  char *mfScriptPreprocessor (char *buf, const char *drn, const char *name);
  uint64 mfScriptPreprocessorMask(SShader *pSH, int nOffset);
  bool mfReloadAllShaders(int nFlags);
  bool mfReloadFile(const char *szPath, const char *szName, int nFlags);
  const char *mfTemplateTexIdToName(int Id);
  bool mfGetParmComps(int comp, SParam *vpp, char *name, char *params, SShader *ef);
  bool mfCompilePlantsTMoving(char *scr, SShader *ef, TArray<SParam>* Params, int reg);
  void mfCompileParamMatrix(char *scr, SShader *ef, SParamComp_Matrix *pcm);
  bool mfCompileParam(char *scr, SShader *ef, TArray<SParam>* Params);
  bool mfCompileCGParam(char *scr, SShader *ef, TArray<SCGParam4f>* Params);
  void mfCheckObjectDependParams(TArray<SParam>* PNoObj, TArray<SParam>* PObj);
  void mfCheckObjectDependParams(TArray<SCGParam4f>* PNoObj, TArray<SCGParam4f>* PObj);
  void mfCompileArrayPointer(TArray<SArrayPointer *>& Pointers, char *scr, SShader *ef);
  ESrcPointer mfParseSrcPointer(char *type, SShader *ef);
  bool mfCompileLayer(SShader *ef, int num, char *scr, SShaderPass *sm);

  bool mfRegisterTemplate(int nTemplId, char *Name, bool bReplace, bool bDefault=false);
  void mfLoadFromFiles(int num);
  
  void mfBeginFrame();

  void mfCompileMatrixOp(TArray<SMatrixTransform>* MatrixOps, char *scr, char *name, SShader *ef);

  SEnvTexture *mfFindSuitableEnvLCMap(Vec3d& Pos, bool bMustExist, int RendFlags, float fDistToCam, CCObject *pObj=NULL);
  SEnvTexture *mfFindSuitableEnvCMap(Vec3d& Pos, bool bMustExist, int RendFlags, float fDistToCam);
  SEnvTexture *mfFindSuitableEnvTex(Vec3d& Pos, Vec3d& Angs, bool bMustExist, int RendFlags, bool bUseExistingREs, SShader *pSH, SRenderShaderResources *pRes, CCObject *pObj, bool bReflect, CRendElement *pRE);

private:

  ShaderFilesMap *m_RefEfs[2];
  LoadedShadersMap m_RefEfsLoaded;

public:
  bool m_bInitialized;

  char m_ShadersPath[2][128];
  char m_ShadersCache[128];
  char *m_ModelsPath;
  char *m_TexturesPath;
  char *m_SystemPath;

  char m_HWPath[128];
  
  SOrient m_Orients[MAX_ORIENTS];
  int m_NumOrients;

  string m_FileNames[2][MAX_EF_FILES];
  short m_nFrameReload[2][MAX_EF_FILES];
  FILETIME m_WriteTime[2][MAX_EF_FILES];
  int m_NumFiles[2];
  int m_nFrameForceReload;

  int m_NightMapReady;
  int m_nCountNightMap;

  int m_CurEfsNum;
  SShader *m_CurShader;

  TArray<SNameAlias> m_AliasNames;
  TArray<SNameAlias> m_CustomAliasNames;

  static SShader *m_DefaultShader;

#ifndef NULL_RENDERER
  static SShader *m_ShaderVFog;
  static SShader *m_ShaderVFogCaust;
  static SShader *m_ShaderFog;
  static SShader *m_ShaderFogCaust;
  static SShader *m_ShaderFog_FP;
  static SShader *m_ShaderFogCaust_FP;
  static SShader *m_ShaderStateNoCull;
  static SShader *m_ZBuffPassShader;
  static SShader *m_ShadowMapShader;
	static SShader *m_ShaderHDRProcess;
  static SShader *m_GlareShader;
  static SShader *m_ShaderSunFlares;
  static SShader *m_ShaderLightStyles;
  static SShader *m_ShaderCGPShaders;
  static SShader *m_ShaderCGVProgramms;

#else
  static SShaderItem m_DefaultShaderItem;
#endif

  TArray<SRegTemplate> m_KnownTemplates;

  int m_Frame;
  SRenderShaderResources *m_pCurResources;

  Matrix44 m_TempMatrices[4][8];

  bool m_bReload;

  int m_Nums;
  int m_MaxNums;
  int m_FirstCopyNum;

  bool m_bNeedSysBuf;
  bool m_bNeedCol;
  bool m_bNeedSecCol;
  bool m_bNeedNormal;
  bool m_bNeedTangents;
  int  m_nTC;


public:
  CShader()
  {
    m_bInitialized = false;
    m_CurEfsNum = 0;
    m_NumFiles[0] = m_NumFiles[1] = 0;
    m_DefaultShader = NULL;
  }

  void mfShutdown (void);
  void mfClearAll (void);
  void mfClearShaders (TArray<SShader *> &Efs, int *Nums);
  
  SShader *mfCopyShader(SShader *ef);
  void mfCompileWaveForm(SWaveForm *wf, char *scr);
  void mfCompileRGBAStyle(char *scr, SShader *ef, SShaderPass *sp, bool bRGB);
  void mfCompileRGBNoise(SRGBGenNoise *cn, char *scr, SShader *ef);
  void mfCompileAlphaNoise(SAlphaGenNoise *cn, char *scr, SShader *ef);
  void mfAddToHash (char *Name, SShader *ef);
  void mfAddToHashLwr (char *Name, SShader *ef);
  void mfRemoveFromHash (SShader *ef);
  //STexPic *mfLoadCubeTex(const char *mapname, uint flags, uint flags2, int State, ETexType eTT, int RState, int Id, int BindId=0);

  int mfReadAllImgFiles(SShader *ef, SShaderTexUnit *tl, STexAnim *ta, char *name);

  void mfInit(void);
  void mfShaderNameForAlias(const char *nameAlias, char *nameEf, int nSize);
  SRenderShaderResources *mfCreateShaderResources(const SInputShaderResources *Res, bool bShare);
  SShaderItem mfShaderItemForName (const char *name, EShClass Class, bool bShare, const char *templName, int flags, const SInputShaderResources *Res=NULL, uint64 nMaskGen=0);
  SShader *mfForName (const char *name, EShClass Class, int flags, const SInputShaderResources *Res=NULL, uint64 nMaskGen=0);
  SShader *mfSpawn (char *name, SShader *ef, SShader *efGen, uint64 nMaskGen);

  bool    mfParseFXTechnique_MergeParameters (std::vector<SFXStruct>& Structs, std::vector<SFXParam>& Params, int nNum, SShader *ef, bool bPixelShader, const char *szShaderName);
  bool    mfParseFXTechnique_LoadShaderTexture (SFXSampler *smp, SFXTexture *tx, SShaderPassHW *pShPass, SShader *ef, int nIndex, byte ColorOp, byte AlphaOp, byte ColorArg, byte AlphaArg);
  bool    mfParseFXTechnique_LoadShader (const char *szShaderCom, SShaderPassHW *pShPass, SShader *ef, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures, std::vector<SFXStruct>& Structs, std::vector<SFXParam>& Params, std::vector<SPair>& Macros, bool bPixelShader);
  bool    mfParseFXTechniquePass (char *buf, char *annotations, SShaderTechnique *pShTech, SShader *ef, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures, std::vector<SFXStruct>& Structs, std::vector<SFXParam>& Params);
  bool    mfParseFXTechnique (char *buf, char *annotations, SShader *ef, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures, std::vector<SFXStruct>& Structs, std::vector<SFXParam>& Params);
  bool    mfParseFXSampler(char *buf, char *name, SShader *ef, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures);
  SShader *mfParseFX (char *buf, SShader *ef, SShader *efGen, uint64 nMaskGen);

  char *mfFindInAllText (char *name, char *&pBuf, SShader *shGen, uint64 nMaskGen);
  char **mfListInScript (char *scr);
  char *mfScriptForFileName(const char *name, SShader *shGen, uint64 nMaskGen);

  void mfSetDefaults(void);
  void mfOptimizeShader(SShader *ef, TArray<SShaderPass>& Layers, int Stage);
  void mfOptimizeShaderHW(SShader *ef, TArray<SShaderPassHW>& Layers, int Stage);
  void mfConstruct(SShader *ef);

#ifdef WIN64
#pragma warning( push )							//AMD Port
#pragma warning( disable : 4267 )				// conversion from 'size_t' to 'XXX', possible loss of data
#endif

  int Size()
  {
    int i, j;
    int nSize = sizeof(*this);

    nSize += m_KnownTemplates.GetMemoryUsage();
    nSize += m_AliasNames.GetMemoryUsage();

    for (i=0; i<2; i++)
    {
      for (j=0; j<MAX_EF_FILES; j++)
      {
        if (!m_FileNames[i][j].empty())
          nSize += m_FileNames[i][j].capacity();
      }
    }

    return nSize;
  }
};

#ifdef WIN64
#pragma warning( pop )							//AMD Port
#endif

//=====================================================================

struct SSunFlare
{
  float   m_Scale;
  float   m_Loc;  // position offset on axis
  CFColor m_Color;
  STexPic *m_Tex;
  Vec3d   m_Position;
  float   m_RenderSize;

  int Size()
  {
    int nSize = sizeof(SSunFlare);
    return nSize;
  }
  SSunFlare()
  {
    m_Tex = NULL;
    m_Scale = 1.0f;
    m_Loc = 0;
    m_Color = Col_White;
  }
  ~SSunFlare();
};

class CSunFlares
{
private:

public:
  static TArray<CSunFlares *> m_SunFlares;
  static CSunFlares *m_CurFlares;

  char m_Name[32];
  int m_NumFlares;
  SSunFlare *m_Flares;

//==================================================================

  int Size()
  {
    int nSize = sizeof(CSunFlares);
    for (int i=0; i<m_NumFlares; i++)
    {
      nSize += m_Flares[i].Size();
    }
    return nSize;
  }

  static CSunFlares *mfForName(char *name);

  CSunFlares()
  {
    m_NumFlares = 0;
    m_Flares = NULL;
  }
  ~CSunFlares()
  {
    if (m_Flares)
      delete [] m_Flares;
    m_NumFlares = 0;
  }
};

//=====================================================================

#endif  // __CSHADER_H__
