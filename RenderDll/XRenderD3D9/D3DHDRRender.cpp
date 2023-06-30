/*=============================================================================
  D3DHDRRender.cpp : Direct3D specific high dynamic range post-processing
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

    Revision history:
      * Created by Honitch Andrey
    
=============================================================================*/

#include "RenderPCH.h"
#include "DriverD3D9.h"
#include "D3DCGPShader.h"
#include "D3DCGVProgram.h"

//=============================================================================

//----------------------------------------------------------------------------------------
// Render Target Management functions
//   Allocation order affects performance.  These functions try to find an optimal allocation
//   order for an arbitrary number of render targets.
//-----------------------------------------------------------------------------------------

static DWORD FormatSize( D3DFORMAT Format ) 
{
  switch ( Format )
  {
  case D3DFMT_R16F:
    return 2;
  case D3DFMT_A8R8G8B8:
  case D3DFMT_X8R8G8B8:
  case D3DFMT_G16R16:
  case D3DFMT_G16R16F:
  case D3DFMT_R32F:
    return 4;
  case D3DFMT_A16B16G16R16:
  case D3DFMT_A16B16G16R16F:
    return 8;
  default:
    assert(0);
  }
  return 4;
}

//  defer allocation of render targets until we have a list of all required targets...
//  then, sort the list to make "optimal" use of GPU memory
struct DeferredRenderTarget
{
  DWORD               dwWidth;
  DWORD               dwHeight;
  D3DFORMAT           Format;
  STexPic             **lplpStorage;
  char                szName[64];
  DWORD               dwPitch;
  float               fPriority;
};

std::vector< DeferredRenderTarget* > g_vAllRenderTargets;
#define SORT_RENDERTARGETS TRUE

struct DescendingRenderTargetSort
{
  bool operator()( DeferredRenderTarget* drtStart, DeferredRenderTarget *drtEnd ) { return (drtStart->dwPitch*drtStart->fPriority) > (drtEnd->dwPitch*drtEnd->fPriority); }
};

//  This function just clears the vector
void StartRenderTargetList( )
{
  std::vector< DeferredRenderTarget* >::iterator _it = g_vAllRenderTargets.begin();

  while ( _it != g_vAllRenderTargets.end() )
  {
    DeferredRenderTarget *drt = (DeferredRenderTarget*)*_it++;
    delete drt;
  }

  g_vAllRenderTargets.clear();
}

//  Add a render target to the list
void AllocateDeferredRenderTarget( DWORD dwWidth, DWORD dwHeight, D3DFORMAT Format, float fPriority, const char *szName, STexPic **pStorage )
{
  DeferredRenderTarget *drt = new DeferredRenderTarget;
  drt->dwWidth          = dwWidth;
  drt->dwHeight         = dwHeight;
  drt->Format           = Format;
  drt->fPriority        = fPriority;
  drt->lplpStorage      = pStorage;
  strcpy(drt->szName, szName);
  drt->dwPitch          = dwWidth * FormatSize( Format );
  g_vAllRenderTargets.push_back( drt );
}

//  Now, sort and allocate all render targets
bool EndRenderTargetList( )
{
  if ( SORT_RENDERTARGETS )
    std::sort( g_vAllRenderTargets.begin(), g_vAllRenderTargets.end(), DescendingRenderTargetSort() );

  std::vector< DeferredRenderTarget* >::iterator _it = g_vAllRenderTargets.begin();
  bool bRes = true;

  while ( _it != g_vAllRenderTargets.end() )
  {
    DeferredRenderTarget *drt = (DeferredRenderTarget*)*_it++;
    CD3D9TexMan *tm = (CD3D9TexMan *)gcpRendD3D->m_TexMan;
    STexPic *tp = tm->CreateTexture(drt->dwWidth, drt->dwHeight, drt->Format, D3DUSAGE_RENDERTARGET, false, drt->szName);
    if (tp)
      *drt->lplpStorage = tp;
    else
      bRes = false;
  }
  StartRenderTargetList( );
  return S_OK;
}


STexPic *CD3D9TexMan::CreateTexture(int nWidth, int nHeight, D3DFORMAT d3dFMT, int d3dUsage, bool bMips, const char *szName)
{
  LPDIRECT3DTEXTURE9 pTexture;
  STexPic *tp;
  if(FAILED(D3DXCreateTexture(gcpRendD3D->mfGetD3DDevice(), nWidth, nHeight, bMips ? D3DX_DEFAULT : 1, d3dUsage, d3dFMT, D3DPOOL_DEFAULT, &pTexture)))
    return NULL;
  int nFlags = FT_NOREMOVE | FT_NOSTREAM;
  if (!bMips)
    nFlags |= FT_NOMIPS;
  tp = LoadTexture(szName, nFlags, FT2_NODXT | FT2_RENDERTARGET, eTT_Base, -1.0f, -1.0f);
  STexPicD3D *ti = (STexPicD3D *)tp;
  ti->m_RefTex.m_VidTex = (void *)pTexture;
  ti->m_RefTex.m_MipFilter = bMips ? GetMipFilter() : D3DTEXF_NONE;
  ti->m_RefTex.m_MinFilter = GetMinFilter();
  ti->m_RefTex.m_MagFilter = GetMagFilter();
  ti->m_RefTex.m_AnisLevel = 1;
  ti->m_RefTex.m_Type = TEXTGT_2D;
  ti->m_DstFormat = d3dFMT;
  ti->m_Width = nWidth;
  ti->m_Height = nHeight;
  CD3D9TexMan::CalcMipsAndSize(ti);
  AddToHash(ti->m_Bind, ti);
  ti->Unlink();
  ti->Link(&STexPic::m_Root);
  gRenDev->m_TexMan->m_StatsCurTexMem += ti->m_Size;

  HRESULT hr = S_OK;
  PDIRECT3DSURFACE9 pSurface = NULL;
  hr = pTexture->GetSurfaceLevel(0, &pSurface);
  if (SUCCEEDED(hr))
    gcpRendD3D->m_pd3dDevice->ColorFill(pSurface, NULL, D3DCOLOR_ARGB(0, 0, 0, 0));
  SAFE_RELEASE( pSurface );

  return ti;
}


void CD3D9TexMan::DestroyHDRMaps()
{
  CD3D9Renderer *r = gcpRendD3D;
  int i;

	SAFE_RELEASE( m_HDR_RT_FSAA );

  if (m_Text_HDRTarget)
    m_Text_HDRTarget->Release(true);
  m_Text_HDRTarget = NULL;
  if (m_Text_HDRTarget_Temp)
    m_Text_HDRTarget_Temp->Release(true);
  m_Text_HDRTarget_Temp = NULL;
  //if (m_Text_HDRTarget_K)
  //  m_Text_HDRTarget_K->Release(true);
  //m_Text_HDRTarget_K = NULL;

  if (m_Text_ScreenMap_HDR)
    m_Text_ScreenMap_HDR->Release(true);
  m_Text_ScreenMap_HDR = NULL;

  if (r->m_TexMan->m_Text_HDRTargetScaled[0])
    r->m_TexMan->m_Text_HDRTargetScaled[0]->Release(true);
  r->m_TexMan->m_Text_HDRTargetScaled[0] = NULL;
  if (r->m_TexMan->m_Text_HDRTargetScaled[1])
    r->m_TexMan->m_Text_HDRTargetScaled[1]->Release(true);
  r->m_TexMan->m_Text_HDRTargetScaled[1] = NULL;

  if (r->m_TexMan->m_Text_HDRBrightPass)
    r->m_TexMan->m_Text_HDRBrightPass->Release(true);
  r->m_TexMan->m_Text_HDRBrightPass = NULL;

  if (r->m_TexMan->m_Text_HDRStarSource)
    r->m_TexMan->m_Text_HDRStarSource->Release(true);
  r->m_TexMan->m_Text_HDRStarSource = NULL;

  if (r->m_TexMan->m_Text_HDRBloomSource)
    r->m_TexMan->m_Text_HDRBloomSource->Release(true);
  r->m_TexMan->m_Text_HDRBloomSource = NULL;

  if (r->m_TexMan->m_Text_HDRAdaptedLuminanceCur)
    r->m_TexMan->m_Text_HDRAdaptedLuminanceCur->Release(true);
  r->m_TexMan->m_Text_HDRAdaptedLuminanceCur = NULL;

  if (r->m_TexMan->m_Text_HDRAdaptedLuminanceLast)
    r->m_TexMan->m_Text_HDRAdaptedLuminanceLast->Release(true);
  r->m_TexMan->m_Text_HDRAdaptedLuminanceLast = NULL;

  for(i=0; i<NUM_HDR_TONEMAP_TEXTURES; i++)
  {
    if (m_Text_HDRToneMaps[i])
      m_Text_HDRToneMaps[i]->Release(true);
    m_Text_HDRToneMaps[i] = NULL;
  }

  for(i=0; i<NUM_HDR_BLOOM_TEXTURES; i++)
  {
    if (m_Text_HDRBloomMaps[i])
      m_Text_HDRBloomMaps[i]->Release(true);
    m_Text_HDRBloomMaps[i] = NULL;
  }

  for(i=0; i<NUM_HDR_STAR_TEXTURES; i++)
  {
    if (m_Text_HDRStarMaps[i][0])
      m_Text_HDRStarMaps[i][0]->Release(true);
    m_Text_HDRStarMaps[i][0] = NULL;
    if (m_Text_HDRStarMaps[i][1])
      m_Text_HDRStarMaps[i][1]->Release(true);
    m_Text_HDRStarMaps[i][1] = NULL;
  }
}

void CD3D9TexMan::GenerateHDRMaps()
{
  int i;
  char szName[256];

  CD3D9Renderer *r = gcpRendD3D;
  r->m_dwHDRCropWidth = r->m_width - r->m_width % 8;
  r->m_dwHDRCropHeight = r->m_height - r->m_height % 8;

  DestroyHDRMaps();

  StartRenderTargetList();

  if (r->m_nHDRType == 2)
  {
    AllocateDeferredRenderTarget(r->m_width, r->m_height, D3DFMT_A8R8G8B8, 1.0f, "$HDRTarget", &m_Text_HDRTarget);
    //AllocateDeferredRenderTarget(r->m_width, r->m_height, D3DFMT_A8R8G8B8, 1.0f, "$HDRTarget_K", &m_Text_HDRTarget_K);
    AllocateDeferredRenderTarget(r->m_width, r->m_height, D3DFMT_A16B16G16R16F, 1.0f, "$HDRTarget_Temp", &m_Text_HDRTarget_Temp);
    AllocateDeferredRenderTarget(r->m_width, r->m_height, D3DFMT_A8R8G8B8, 0.8f, "$HDRScreenMap", &m_Text_ScreenMap_HDR);
  }
  else
  {
		if( r->CV_r_fsaa == 2 )
		{
			HRESULT hr = r->m_pd3dDevice->CreateRenderTarget( r->m_width, r->m_height, D3DFMT_A16B16G16R16F, ConvertFSAASamplesToType( r->m_FSAA_samples ), r->m_FSAA_quality, FALSE, &m_HDR_RT_FSAA, 0 );
			//if( SUCCEEDED( hr ) )
			//	iLog->Log( "HDR-FSAA: Created multi-sampled HDR render target for current FSAA settings (samples = %d / quality = %d).", r->m_FSAA_samples, r->m_FSAA_quality );
			//else
			//	iLog->Log( "HDR-FSAA: Multi-sampled HDR render target creation failed!" );
		}
    AllocateDeferredRenderTarget(r->m_width, r->m_height, D3DFMT_A16B16G16R16F, 1.0f, "$HDRTarget", &m_Text_HDRTarget);
    AllocateDeferredRenderTarget(r->m_width, r->m_height, D3DFMT_A16B16G16R16F, 0.8f, "$HDRScreenMap", &m_Text_ScreenMap_HDR);
  }

  // Scaled version of the HDR scene texture
  if (r->m_bDeviceSupportsMRT)
  {
    AllocateDeferredRenderTarget(r->m_dwHDRCropWidth/4, r->m_dwHDRCropHeight/4, D3DFMT_G16R16F, 1.f, "$HDRTargetScaled0", &m_Text_HDRTargetScaled[0]);
    AllocateDeferredRenderTarget(r->m_dwHDRCropWidth/4, r->m_dwHDRCropHeight/4, D3DFMT_G16R16F, 1.f, "$HDRTargetScaled1", &m_Text_HDRTargetScaled[1]);
  }
  else
  {
    AllocateDeferredRenderTarget(r->m_dwHDRCropWidth/4, r->m_dwHDRCropHeight/4, D3DFMT_A16B16G16R16F, 1.f, "$HDRTargetScaled", &m_Text_HDRTargetScaled[0] );
    m_Text_HDRTargetScaled[1] = NULL;
  }

  AllocateDeferredRenderTarget(r->m_dwHDRCropWidth/4+2, r->m_dwHDRCropHeight/4+2, D3DFMT_A8R8G8B8, 0.5f, "$HDRBrightPass", &m_Text_HDRBrightPass);
  AllocateDeferredRenderTarget(r->m_dwHDRCropWidth/4+2, r->m_dwHDRCropHeight/4+2, D3DFMT_A8R8G8B8, 0.5f, "$HDRStarSource", &m_Text_HDRStarSource);
  AllocateDeferredRenderTarget(r->m_dwHDRCropWidth/8+2, r->m_dwHDRCropHeight/8+2, D3DFMT_A8R8G8B8, 0.8f, "$HDRBloomSource", &m_Text_HDRBloomSource);
  AllocateDeferredRenderTarget(1, 1, r->m_HDR_FloatFormat_Scalar, 0.1f, "$HDRAdaptedLuminanceCur", &m_Text_HDRAdaptedLuminanceCur);
  AllocateDeferredRenderTarget(1, 1, r->m_HDR_FloatFormat_Scalar, 0.1f, "$HDRAdaptedLuminanceLast", &m_Text_HDRAdaptedLuminanceLast);

  // For each scale stage, create a texture to hold the intermediate results
  // of the luminance calculation
  for(i=0; i<NUM_HDR_TONEMAP_TEXTURES; i++)
  {
    int iSampleLen = 1 << (2*i);
    sprintf(szName, "$HDRToneMap_%d", i);
    AllocateDeferredRenderTarget(iSampleLen, iSampleLen, r->m_HDR_FloatFormat_Scalar, 0.7f, szName, &m_Text_HDRToneMaps[i]);
  }

  // Create the temporary blooming effect textures
  // Texture has a black border of single texel thickness to fake border 
  // addressing using clamp addressing
  for(i=1; i<NUM_HDR_BLOOM_TEXTURES; i++)
  {
    sprintf(szName, "$HDRBloomMap_%d", i);
    AllocateDeferredRenderTarget(r->m_dwHDRCropWidth/8+2, r->m_dwHDRCropHeight/8+2, D3DFMT_A8R8G8B8, 0.8f, szName, &m_Text_HDRBloomMaps[i]);
  }
  AllocateDeferredRenderTarget(r->m_dwHDRCropWidth/8, r->m_dwHDRCropHeight/8, D3DFMT_A8R8G8B8, 0.5f, "$HDRBloomMap_0", &m_Text_HDRBloomMaps[0]);

  // Create the star effect textures
  for(i=0; i<NUM_HDR_STAR_TEXTURES; i++)
  {
    if (r->m_bDeviceSupportsMRT )
    {
      sprintf(szName, "$HDRStarMap_%d(0)", i);
      AllocateDeferredRenderTarget(r->m_dwHDRCropWidth/4, r->m_dwHDRCropHeight/4, D3DFMT_G16R16F, 0.8f, szName, &m_Text_HDRStarMaps[i][0]);
      sprintf(szName, "$HDRStarMap_%d(1)", i);
      AllocateDeferredRenderTarget(r->m_dwHDRCropWidth/4, r->m_dwHDRCropHeight/4, D3DFMT_G16R16F, 0.8f, szName, &m_Text_HDRStarMaps[i][1]);
    }
    else
    {
      sprintf(szName, "$HDRStarMap_%d", i);
      AllocateDeferredRenderTarget(r->m_dwHDRCropWidth/4, r->m_dwHDRCropHeight/4, D3DFMT_A16B16G16R16F, 0.8f, szName, &m_Text_HDRStarMaps[i][0]);
      m_Text_HDRStarMaps[i][1] = NULL;
    }
  }

  EndRenderTargetList();
}

//=============================================================================

//----------------------------------------------------------
// Star generation

// Define each line of the star.
typedef struct STARLINE
{
  int		nPasses ;
  float	fSampleLength ;
  float	fAttenuation ;
  float	fInclination ;

} *LPSTARLINE ;

// Simple definition of the star.
typedef struct STARDEF
{
  TCHAR	*szStarName ;
  int		nStarLines ;
  int		nPasses ;
  float	fSampleLength ;
  float	fAttenuation ;
  float	fInclination ;
  bool	bRotation ;

} *LPSTARDEF ;


// Simple definition of the sunny cross filter
typedef struct STARDEF_SUNNYCROSS
{
  TCHAR	*szStarName ;
  float	fSampleLength ;
  float	fAttenuation ;
  float	fInclination ;

} *LPSTARDEF_SUNNYCROSS ;


// Star form library
enum ESTARLIBTYPE
{
  STLT_DISABLE			= 0,

  STLT_CROSS,
  STLT_CROSSFILTER,
  STLT_SNOWCROSS,
  STLT_VERTICAL,
  NUM_BASESTARLIBTYPES,

  STLT_SUNNYCROSS			= NUM_BASESTARLIBTYPES,

  NUM_STARLIBTYPES,
} ;

//----------------------------------------------------------
// Star generation object

class CStarDef
{
public:
  TCHAR			m_strStarName[256] ;

  int				m_nStarLines ;
  LPSTARLINE		m_pStarLine ;	// [m_nStarLines]
  float			m_fInclination ;
  bool			m_bRotation ;	// Rotation is available from outside ?

  // Static library
public:
  static CStarDef		*ms_pStarLib ;
  static D3DXCOLOR	ms_avChromaticAberrationColor[8] ;

  // Public method
public:
  CStarDef() ;
  CStarDef(const CStarDef& src) ;
  ~CStarDef() ;

  CStarDef& operator =(const CStarDef& src) {
    Initialize(src) ;
    return *this ;
  }

  HRESULT Construct() ;
  void Destruct() ;
  void Release() ;

  HRESULT Initialize(const CStarDef& src) ;

  HRESULT Initialize(ESTARLIBTYPE eType) {
    return Initialize(ms_pStarLib[eType]) ;
  }

  /// Generic simple star generation
  HRESULT Initialize(const TCHAR *szStarName,
    int nStarLines,
    int nPasses,
    float fSampleLength,
    float fAttenuation,
    float fInclination,
    bool bRotation) ;

  HRESULT Initialize(const STARDEF& starDef)
  {
    return Initialize(starDef.szStarName,
      starDef.nStarLines,
      starDef.nPasses,
      starDef.fSampleLength,
      starDef.fAttenuation,
      starDef.fInclination,
      starDef.bRotation) ;
  }

  /// Specific star generation
  //  Sunny cross filter
  HRESULT Initialize_SunnyCrossFilter(const TCHAR *szStarName = TEXT("SunnyCross"),
    float fSampleLength = 1.0f,
    float fAttenuation = 0.88f,
    float fLongAttenuation = 0.95f,
    float fInclination = D3DXToRadian(0.0f)) ;


  // Public static method
public:
  /// Create star library
  static HRESULT InitializeStaticStarLibs() ;
  static HRESULT DeleteStaticStarLibs() ;

  /// Access to the star library
  static const CStarDef& GetLib(DWORD dwType) {
    return ms_pStarLib[dwType] ;
  }

  static const D3DXCOLOR& GetChromaticAberrationColor(DWORD dwID) {
    return ms_avChromaticAberrationColor[dwID] ;
  }
} ;

//============================================================================

// Glare form library
enum EGLARELIBTYPE
{
  GLT_DISABLE					= 0,

  GLT_CAMERA,
  GLT_NATURAL,
  GLT_CHEAPLENS,
  //GLT_AFTERIMAGE,
  GLT_FILTER_CROSSSCREEN,
  GLT_FILTER_CROSSSCREEN_SPECTRAL,
  GLT_FILTER_SNOWCROSS,
  GLT_FILTER_SNOWCROSS_SPECTRAL,
  GLT_FILTER_SUNNYCROSS,
  GLT_FILTER_SUNNYCROSS_SPECTRAL,
  GLT_CINECAM_VERTICALSLITS,
  GLT_CINECAM_HORIZONTALSLITS,

  NUM_GLARELIBTYPES,
  GLT_USERDEF					= -1,
  GLT_DEFAULT					= GLT_FILTER_CROSSSCREEN,
} ;


// Simple glare definition
typedef struct GLAREDEF
{
  TCHAR			*szGlareName ;
  float			fGlareLuminance ;

  float			fBloomLuminance ;
  float			fGhostLuminance ;
  float			fGhostDistortion ;
  float			fStarLuminance ;
  ESTARLIBTYPE	eStarType ;
  float			fStarInclination ;

  float			fChromaticAberration ;

  float			fAfterimageSensitivity ;	// Current weight
  float			fAfterimageRatio ;			// Afterimage weight
  float			fAfterimageLuminance ;

} *LPGLAREDEF ;


class CGlareDef
{
public:
  TCHAR		m_strGlareName[256] ;

  float		m_fGlareLuminance ;		// Total glare intensity (not effect to "after image")
  float		m_fBloomLuminance ;
  float		m_fGhostLuminance ;
  float		m_fGhostDistortion ;
  float		m_fStarLuminance ;
  float		m_fStarInclination ;

  float		m_fChromaticAberration ;

  float		m_fAfterimageSensitivity ;	// Current weight
  float		m_fAfterimageRatio ;		// Afterimage weight
  float		m_fAfterimageLuminance ;

  CStarDef	m_starDef ;

  // Static library
public:
  static CGlareDef	*ms_pGlareLib ;

  // Public method
public:
  CGlareDef() ;
  CGlareDef(const CGlareDef& src) ;
  ~CGlareDef() ;

  CGlareDef& operator =(const CGlareDef& src) {
    Initialize(src) ;
    return *this ;
  }

  HRESULT Construct() ;
  void Destruct() ;
  void Release() ;

  HRESULT Initialize(const CGlareDef& src) ;

  HRESULT Initialize(const TCHAR *szStarName,
    float fGlareLuminance,
    float fBloomLuminance,
    float fGhostLuminance,
    float fGhostDistortion,
    float fStarLuminance,
    ESTARLIBTYPE eStarType,
    float fStarInclination,
    float fChromaticAberration,
    float fAfterimageSensitivity,	// Current weight
    float fAfterimageRatio,			// After Image weight
    float fAfterimageLuminance) ;

  HRESULT Initialize(const GLAREDEF& glareDef)
  {
    return Initialize(glareDef.szGlareName,
      glareDef.fGlareLuminance,
      glareDef.fBloomLuminance,
      glareDef.fGhostLuminance,
      glareDef.fGhostDistortion,
      glareDef.fStarLuminance,
      glareDef.eStarType,
      glareDef.fStarInclination,
      glareDef.fChromaticAberration,
      glareDef.fAfterimageSensitivity,
      glareDef.fAfterimageRatio,
      glareDef.fAfterimageLuminance) ;
  }

  HRESULT Initialize(EGLARELIBTYPE eType) {
    return Initialize(ms_pGlareLib[eType]) ;
  }


  // Public static method
public:
  /// Create glare library
  static HRESULT InitializeStaticGlareLibs() ;
  static HRESULT DeleteStaticGlareLibs() ;

  /// Access to the glare library
  static const CGlareDef& GetLib(DWORD dwType) {
    return ms_pGlareLib[dwType] ;
  }
} ;

//----------------------------------------------------------
// Dummy to generate static object of glare
class __CGlare_GenerateStaticObjects
{
public:
	__CGlare_GenerateStaticObjects() {
		CStarDef::InitializeStaticStarLibs() ;
		CGlareDef::InitializeStaticGlareLibs() ;
	}

	~__CGlare_GenerateStaticObjects() {
		CGlareDef::DeleteStaticGlareLibs() ;
		CStarDef::DeleteStaticStarLibs() ;
	}

	static __CGlare_GenerateStaticObjects ms_staticObject ;
} ;

__CGlare_GenerateStaticObjects __CGlare_GenerateStaticObjects::ms_staticObject ;

CGlareDef g_GlareDef;


#define _Rad	D3DXToRadian

// Static star library information
static STARDEF s_aLibStarDef[NUM_BASESTARLIBTYPES] =
{
  //	star name		        lines	passes	length	attn	rotate			bRotate
  {	TEXT("Disable"),	    0,		0,		0.0f,	0.0f,	_Rad(00.0f),	false,	},	// STLT_DISABLE

  {	TEXT("Cross"),		    4,		3,		1.0f,	0.85f,	_Rad(0.0f),		true,	},	// STLT_CROSS
  {	TEXT("CrossFilter"),    4,		3,		1.0f,	0.95f,	_Rad(0.0f),		true,	},	// STLT_CROSS
  {	TEXT("snowCross"),	    6,		3,		1.0f,	0.96f,	_Rad(20.0f),	true,	},	// STLT_SNOWCROSS
  {	TEXT("Vertical"),		2,		3,		1.0f,	0.96f,	_Rad(00.0f),	false,	},	// STLT_VERTICAL
} ;
static int s_nLibStarDefs = sizeof(s_aLibStarDef) / sizeof(STARDEF) ;


// Static glare library information
static GLAREDEF s_aLibGlareDef[NUM_GLARELIBTYPES] =
{
  //	glare name						        glare	bloom	ghost	distort	star	star type
  //	rotate			C.A		current	after	ai lum
  {	TEXT("Disable"),						0.0f,	0.0f,	0.0f,	0.01f,	0.0f,	STLT_DISABLE,
    _Rad(0.0f),		0.5f,	0.00f,	0.00f,	0.0f,  },	// GLT_DISABLE

  {	TEXT("Camera"),					        1.5f,	1.2f,	1.0f,	0.00f,	1.0f,	STLT_CROSS,
  _Rad(00.0f),	0.5f,	0.25f,	0.90f,	1.0f,  },	// GLT_CAMERA
  {	TEXT("Natural Bloom"),			        1.5f,	1.2f,	0.0f,	0.00f,	0.0f,	STLT_DISABLE,
  _Rad(00.0f),	0.0f,	0.40f,  0.85f,	0.5f,  },	// GLT_NATURAL
  {	TEXT("Cheap Lens Camera"),		        1.25f,	2.0f,	1.5f,	0.05f,	2.0f,	STLT_CROSS,
  _Rad(00.0f),	0.5f,	0.18f,	0.95f,	1.0f,  },	// GLT_CHEAPLENS
  /*	
  {	TEXT("Afterimage"),				        1.5f,	1.2f,	0.5f,	0.00f,	0.7f,	STLT_CROSS,
  _Rad(00.0f),	0.5f,	0.1f,	0.98f,	2.0f,  },	// GLT_AFTERIMAGE
  */
  {	TEXT("Cross Screen Filter"),	        1.0f,	2.0f,	1.7f,	0.00f,	1.5f,	STLT_CROSSFILTER,
  _Rad(25.0f),	0.5f,	0.20f,	0.93f,	1.0f,  },	// GLT_FILTER_CROSSSCREEN
  {	TEXT("Spectral Cross Filter"),	        1.0f,	2.0f,	1.7f,	0.00f,	1.8f,	STLT_CROSSFILTER,
  _Rad(70.0f),	1.5f,	0.20f,	0.93f,	1.0f,  },	// GLT_FILTER_CROSSSCREEN_SPECTRAL
  {	TEXT("Snow Cross Filter"),		        1.0f,	2.0f,	1.7f,	0.00f,	1.5f,	STLT_SNOWCROSS,
  _Rad(10.0f),	0.5f,	0.20f,	0.93f,	1.0f,  },	// GLT_FILTER_SNOWCROSS
  {	TEXT("Spectral Snow Cross"),	        1.0f,	2.0f,	1.7f,	0.00f,	1.8f,	STLT_SNOWCROSS,
  _Rad(40.0f),	1.5f,	0.20f,	0.93f,	1.0f,  },	// GLT_FILTER_SNOWCROSS_SPECTRAL
  {	TEXT("Sunny Cross Filter"),		        1.0f,	2.0f,	1.7f,	0.00f,	1.5f,	STLT_SUNNYCROSS,
  _Rad(00.0f),	0.5f,	0.20f,	0.93f,	1.0f,  },	// GLT_FILTER_SUNNYCROSS
  {	TEXT("Spectral Sunny Cross"),	        1.0f,	2.0f,	1.7f,	0.00f,	1.8f,	STLT_SUNNYCROSS,
  _Rad(45.0f),	1.5f,	0.20f,	0.93f,	1.0f,  },	// GLT_FILTER_SUNNYCROSS_SPECTRAL
  {	TEXT("Cine Camera Vertical Slits"),	    1.0f,	2.0f,	1.5f,	0.00f,	1.0f,	STLT_VERTICAL,
  _Rad(90.0f),	0.5f,	0.20f,	0.93f,	1.0f,  },	// GLT_CINECAM_VERTICALSLIT
  {	TEXT("Cine Camera Horizontal Slits"),	1.0f,	2.0f,	1.5f,	0.00f,	1.0f,	STLT_VERTICAL,
  _Rad(00.0f),	0.5f,	0.20f,	0.93f,	1.0f,  },	// GLT_CINECAM_HORIZONTALSLIT
} ;
static int s_nLibGlareDefs = sizeof(s_aLibGlareDef) / sizeof(GLAREDEF) ;



//----------------------------------------------------------
// Information object for star generation 

CStarDef	*CStarDef::ms_pStarLib	= NULL ;
D3DXCOLOR	CStarDef::ms_avChromaticAberrationColor[] ;


CStarDef::CStarDef()
{
  Construct() ;
}

CStarDef::CStarDef(const CStarDef& src)
{
  Construct() ;
  Initialize(src) ;
}

CStarDef::~CStarDef()
{
  Destruct() ;
}


HRESULT CStarDef::Construct()
{
  ZeroMemory( m_strStarName, sizeof(m_strStarName) );

  m_nStarLines	= 0 ;
  m_pStarLine		= NULL ;
  m_fInclination	= 0.0f ;

  m_bRotation		= false ;

  return S_OK ;
}

void CStarDef::Destruct()
{
  Release() ;
}

void CStarDef::Release()
{
  SAFE_DELETE_ARRAY(m_pStarLine) ;
  m_nStarLines = 0 ;
}


HRESULT CStarDef::Initialize(const CStarDef& src)
{
  if (&src == this) {
    return S_OK ;
  }

  // Release the data
  Release() ;

  // Copy the data from source
  lstrcpyn( m_strStarName, src.m_strStarName, 255 );
  m_nStarLines	= src.m_nStarLines ;
  m_fInclination	= src.m_fInclination ;
  m_bRotation		= src.m_bRotation ;

  m_pStarLine = new STARLINE[m_nStarLines] ;
  for (int i = 0 ; i < m_nStarLines ; i ++) {
    m_pStarLine[i] = src.m_pStarLine[i] ;
  }

  return S_OK ;
}


/// generic simple star generation
HRESULT CStarDef::Initialize(const TCHAR *szStarName,
                             int nStarLines,
                             int nPasses,
                             float fSampleLength,
                             float fAttenuation,
                             float fInclination,
                             bool bRotation)
{
  // Release the data
  Release() ;

  // Copy from parameters
  lstrcpyn( m_strStarName, szStarName, 255 );
  m_nStarLines	= nStarLines ;
  m_fInclination	= fInclination ;
  m_bRotation		= bRotation ;

  m_pStarLine = new STARLINE[m_nStarLines] ;

  float fInc = D3DXToRadian(360.0f / (float)m_nStarLines) ;
  for (int i = 0 ; i < m_nStarLines ; i ++)
  {
    m_pStarLine[i].nPasses			= nPasses ;
    m_pStarLine[i].fSampleLength	= fSampleLength ;
    m_pStarLine[i].fAttenuation		= fAttenuation ;
    m_pStarLine[i].fInclination		= fInc * (float)i ;
  }

  return S_OK ;
}


/// Specific start generation
//  Sunny cross filter
HRESULT CStarDef::Initialize_SunnyCrossFilter(const TCHAR *szStarName,
                                              float fSampleLength,
                                              float fAttenuation,
                                              float fLongAttenuation,
                                              float fInclination)
{
  // Release the data
  Release() ;

  // Create parameters
  lstrcpyn( m_strStarName, szStarName, 255 );
  m_nStarLines	= 8 ;
  m_fInclination	= fInclination ;
  //	m_bRotation		= true ;
  m_bRotation		= false ;

  m_pStarLine = new STARLINE[m_nStarLines] ;
  float fInc = D3DXToRadian(360.0f / (float)m_nStarLines) ;
  for (int i = 0 ; i < m_nStarLines ; i ++)
  {
    m_pStarLine[i].fSampleLength	= fSampleLength ;
    m_pStarLine[i].fInclination		= fInc * (float)i + D3DXToRadian(0.0f) ;

    if ( 0 == (i % 2) ) {
      m_pStarLine[i].nPasses		= 3 ;
      m_pStarLine[i].fAttenuation	= fLongAttenuation ;	// long
    }
    else {
      m_pStarLine[i].nPasses		= 3 ;
      m_pStarLine[i].fAttenuation	= fAttenuation ;
    }
  }

  return S_OK ;
}

HRESULT CStarDef::InitializeStaticStarLibs()
{
  if (ms_pStarLib) {
    return S_OK ;
  }

  ms_pStarLib = new CStarDef[NUM_STARLIBTYPES] ;

  // Create basic form
  for (int i = 0 ; i < NUM_BASESTARLIBTYPES ; i ++) {
    ms_pStarLib[i].Initialize(s_aLibStarDef[i]) ;
  }

  // Create special form
  // Sunny cross filter
  ms_pStarLib[STLT_SUNNYCROSS].Initialize_SunnyCrossFilter() ;

  // Initialize color aberration table
  /*
  {
  D3DXCOLOR(0.5f, 0.5f, 0.5f,  0.0f),
  D3DXCOLOR(1.0f, 0.2f, 0.2f,  0.0f),
  D3DXCOLOR(0.2f, 0.6f, 0.2f,  0.0f),
  D3DXCOLOR(0.2f, 0.2f, 1.0f,  0.0f),
  } ;
  */
  D3DXCOLOR avColor[8] =
  {
    /*
    D3DXCOLOR(0.5f, 0.5f, 0.5f,  0.0f),
    D3DXCOLOR(0.3f, 0.3f, 0.8f,  0.0f),
    D3DXCOLOR(0.2f, 0.2f, 1.0f,  0.0f),
    D3DXCOLOR(0.2f, 0.4f, 0.5f,  0.0f),
    D3DXCOLOR(0.2f, 0.6f, 0.2f,  0.0f),
    D3DXCOLOR(0.5f, 0.4f, 0.2f,  0.0f),
    D3DXCOLOR(0.7f, 0.3f, 0.2f,  0.0f),
    D3DXCOLOR(1.0f, 0.2f, 0.2f,  0.0f),
    */

    D3DXCOLOR(0.5f, 0.5f, 0.5f,  0.0f),	// w
      D3DXCOLOR(0.8f, 0.3f, 0.3f,  0.0f),
      D3DXCOLOR(1.0f, 0.2f, 0.2f,  0.0f),	// r
      D3DXCOLOR(0.5f, 0.2f, 0.6f,  0.0f),
      D3DXCOLOR(0.2f, 0.2f, 1.0f,  0.0f),	// b
      D3DXCOLOR(0.2f, 0.3f, 0.7f,  0.0f),
      D3DXCOLOR(0.2f, 0.6f, 0.2f,  0.0f),	// g
      D3DXCOLOR(0.3f, 0.5f, 0.3f,  0.0f),
  } ;

  memcpy( ms_avChromaticAberrationColor, avColor, sizeof(D3DXCOLOR) * 8 ) ;
  /*
  ms_avChromaticAberrationColor[0] = D3DXCOLOR(0.5f, 0.5f, 0.5f,  0.0f) ;
  ms_avChromaticAberrationColor[1] = D3DXCOLOR(0.7f, 0.3f, 0.3f,  0.0f) ;
  ms_avChromaticAberrationColor[2] = D3DXCOLOR(1.0f, 0.2f, 0.2f,  0.0f) ;
  ms_avChromaticAberrationColor[3] = D3DXCOLOR(0.5f, 0.5f, 0.5f,  0.0f) ;
  ms_avChromaticAberrationColor[4] = D3DXCOLOR(0.2f, 0.6f, 0.2f,  0.0f) ;
  ms_avChromaticAberrationColor[5] = D3DXCOLOR(0.2f, 0.4f, 0.5f,  0.0f) ;
  ms_avChromaticAberrationColor[6] = D3DXCOLOR(0.2f, 0.3f, 0.8f,  0.0f) ;
  ms_avChromaticAberrationColor[7] = D3DXCOLOR(0.2f, 0.2f, 1.0f,  0.0f) ;
  */
  return S_OK ;
}

HRESULT CStarDef::DeleteStaticStarLibs()
{
  // Delete all libraries
  SAFE_DELETE_ARRAY( ms_pStarLib ) ;

  return S_OK ;
}



//----------------------------------------------------------
// Glare definition

CGlareDef	*CGlareDef::ms_pGlareLib	= NULL ;

CGlareDef::CGlareDef()
{
  Construct() ;
}

CGlareDef::CGlareDef(const CGlareDef& src)
{
  Construct() ;
  Initialize(src) ;
}

CGlareDef::~CGlareDef()
{
  Destruct() ;
}


HRESULT CGlareDef::Construct()
{
  ZeroMemory( m_strGlareName, sizeof(m_strGlareName) );

  m_fGlareLuminance	= 0.0f ;
  m_fBloomLuminance	= 0.0f ;
  m_fGhostLuminance	= 0.0f ;
  m_fStarLuminance	= 0.0f ;
  m_fStarInclination	= 0.0f ;
  m_fChromaticAberration	= 0.0f ;

  m_fAfterimageSensitivity	= 0.0f ;
  m_fAfterimageRatio			= 0.0f ;
  m_fAfterimageLuminance		= 0.0f ;

  return S_OK ;
}

void CGlareDef::Destruct()
{
  m_starDef.Release() ;
}

void CGlareDef::Release()
{
}

HRESULT CGlareDef::Initialize(const CGlareDef& src)
{
  if (&src == this) {
    return S_OK ;
  }

  // Release the data
  Release() ;

  // Copy data from source
  lstrcpyn( m_strGlareName, src.m_strGlareName, 255 );
  m_fGlareLuminance	= src.m_fGlareLuminance ;

  m_fBloomLuminance	= src.m_fBloomLuminance ;
  m_fGhostLuminance	= src.m_fGhostLuminance ;
  m_fGhostDistortion	= src.m_fGhostDistortion ;
  m_fStarLuminance	= src.m_fStarLuminance ;
  m_fStarLuminance	= src.m_fStarLuminance ;
  m_fStarInclination	= src.m_fStarInclination ;
  m_fChromaticAberration	= src.m_fChromaticAberration ;

  m_fAfterimageSensitivity	= src.m_fStarLuminance ;
  m_fAfterimageRatio			= src.m_fStarLuminance ;
  m_fAfterimageLuminance		= src.m_fStarLuminance ;

  m_starDef			= src.m_starDef ;

  return S_OK ;
}


HRESULT CGlareDef::Initialize(const TCHAR *szStarName,
                              float fGlareLuminance,
                              float fBloomLuminance,
                              float fGhostLuminance,
                              float fGhostDistortion,
                              float fStarLuminance,
                              ESTARLIBTYPE eStarType,
                              float fStarInclination,
                              float fChromaticAberration,
                              float fAfterimageSensitivity,	// Current weight
                              float fAfterimageRatio,		// Afterimage weight
                              float fAfterimageLuminance)
{
  // Release the data
  Release() ;

  // Create parameters
  lstrcpyn( m_strGlareName, szStarName, 255 );
  m_fGlareLuminance	= fGlareLuminance ;

  m_fBloomLuminance	= fBloomLuminance ;
  m_fGhostLuminance	= fGhostLuminance ;
  m_fGhostDistortion	= fGhostDistortion ;
  m_fStarLuminance	= fStarLuminance ;
  m_fStarInclination	= fStarInclination ;
  m_fChromaticAberration	= fChromaticAberration ;

  m_fAfterimageSensitivity	= fAfterimageSensitivity ;
  m_fAfterimageRatio			= fAfterimageRatio ;
  m_fAfterimageLuminance		= fAfterimageLuminance ;

  // Create star form data
  m_starDef			= CStarDef::GetLib(eStarType) ;

  return S_OK ;
}


HRESULT CGlareDef::InitializeStaticGlareLibs()
{
  if (ms_pGlareLib) {
    return S_OK ;
  }

  CStarDef::InitializeStaticStarLibs() ;
  ms_pGlareLib = new CGlareDef[NUM_GLARELIBTYPES] ;

  // Create glare form
  for (int i = 0 ; i < NUM_GLARELIBTYPES ; i ++) {
    ms_pGlareLib[i].Initialize(s_aLibGlareDef[i]) ;
  }

  return S_OK ;
}


HRESULT CGlareDef::DeleteStaticGlareLibs()
{
  // Delete all libraries
  SAFE_DELETE_ARRAY( ms_pGlareLib ) ;

  return S_OK ;
}



//=============================================================================

// Texture coordinate rectangle
struct CoordRect
{
  float fLeftU, fTopV;
  float fRightU, fBottomV;
};

// Screen quad vertex format
struct ScreenVertex
{
  D3DXVECTOR4 p; // position
  D3DCOLOR color;
  D3DXVECTOR2 t; // texture coordinate
};

// log helper
#define LOG_EFFECT(msg)\
  if (gRenDev->m_LogFile)\
  {\
  gRenDev->Logv(SRendItem::m_RecurseLevel, msg);\
  }\

// ===============================================================
// SetTexture - sets texture stage

inline void SetTexture(CD3D9Renderer *pRenderer, STexPic *pTex, int iStage, int iMinFilter, int iMagFilter, bool bClamp)
{
  pRenderer->EF_SelectTMU(iStage);
  if(pTex)
  {
    pTex->m_RefTex.m_MinFilter=iMinFilter; 
    pTex->m_RefTex.m_MagFilter=iMagFilter;
    pTex->m_RefTex.bRepeats=(!bClamp);  
    pTex->Set(); 
   }
  else
  {
    pRenderer->SetTexture(0);
  }
}

PDIRECT3DSURFACE9 GetSurfaceTP(STexPic *tp)
{
  PDIRECT3DSURFACE9 pSurf = NULL;
  if (tp)
  {
    LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)tp->m_RefTex.m_VidTex;
    assert(pTexture);
    HRESULT hr = pTexture->GetSurfaceLevel(0, &pSurf);
  }
  return pSurf;
}

#define NV_CACHE_OPTS_ENABLED TRUE

//-----------------------------------------------------------------------------
// Name: DrawFullScreenQuad
// Desc: Draw a properly aligned quad covering the entire render target
//-----------------------------------------------------------------------------
void DrawFullScreenQuad(float fLeftU, float fTopV, float fRightU, float fBottomV)
{
  D3DSURFACE_DESC dtdsdRT;
  PDIRECT3DSURFACE9 pSurfRT;
  CD3D9Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = rd->m_pd3dDevice;

  // Acquire render target width and height
  dv->GetRenderTarget(0, &pSurfRT);
  pSurfRT->GetDesc(&dtdsdRT);
  pSurfRT->Release();

  // Ensure that we're directly mapping texels to pixels by offset by 0.5
  // For more info see the doc page titled "Directly Mapping Texels to Pixels"
  int nWidth = min((int)dtdsdRT.Width, rd->GetWidth());
  int nHeight = min((int)dtdsdRT.Height, rd->GetHeight());
  float fWidth5 = (float)nWidth;
  float fHeight5 = (float)nHeight;
  fWidth5 = (NV_CACHE_OPTS_ENABLED) ?  (2.f*fWidth5) - 0.5f : fWidth5 - 0.5f;
  fHeight5 = (NV_CACHE_OPTS_ENABLED) ? (2.f*fHeight5)- 0.5f : fHeight5 - 0.5f;

  // Draw the quad
  ScreenVertex svQuad[4];

  svQuad[0].p = D3DXVECTOR4(-0.5f, -0.5f, 0.5f, 1.0f);
  svQuad[0].color = -1;
  svQuad[0].t = D3DXVECTOR2(fLeftU, fTopV);
  if (NV_CACHE_OPTS_ENABLED)
  {
    float tWidth = fRightU - fLeftU;
    float tHeight = fBottomV - fTopV;
    svQuad[1].p = D3DXVECTOR4(fWidth5, -0.5f, 0.5f, 1.f);
    svQuad[1].t = D3DXVECTOR2(fLeftU + (tWidth*2.f), fTopV);

    svQuad[2].p = D3DXVECTOR4(-0.5f, fHeight5, 0.5f, 1.f);
    svQuad[2].t = D3DXVECTOR2(fLeftU, fTopV + (tHeight*2.f));
  }
  else
  {
    svQuad[1].p = D3DXVECTOR4(fWidth5, -0.5f, 0.5f, 1.0f);
    svQuad[1].color = -1;
    svQuad[1].t = D3DXVECTOR2(fRightU, fTopV);

    svQuad[2].p = D3DXVECTOR4(-0.5f, fHeight5, 0.5f, 1.0f);
    svQuad[2].color = -1;
    svQuad[2].t = D3DXVECTOR2(fLeftU, fBottomV);

    svQuad[3].p = D3DXVECTOR4(fWidth5, fHeight5, 0.5f, 1.0f);
    svQuad[3].color = -1;
    svQuad[3].t = D3DXVECTOR2(fRightU, fBottomV);
  }

  rd->EF_SetState(GS_NODEPTHTEST);
  rd->EF_SetVertexDeclaration(0, VERTEX_FORMAT_TRP3F_COL4UB_TEX2F);
  dv->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, (NV_CACHE_OPTS_ENABLED)?1:2, svQuad, sizeof(ScreenVertex));
}

void DrawFullScreenQuad(CoordRect c)
{
  DrawFullScreenQuad( c.fLeftU, c.fTopV, c.fRightU, c.fBottomV );
}

//-----------------------------------------------------------------------------
// Name: GetTextureRect()
// Desc: Get the dimensions of the texture
//-----------------------------------------------------------------------------
HRESULT GetTextureRect(STexPic *pTexture, RECT* pRect)
{
  pRect->left = 0;
  pRect->top = 0;
  pRect->right = pTexture->m_Width;
  pRect->bottom = pTexture->m_Height;

  return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetTextureCoords()
// Desc: Get the texture coordinates to use when rendering into the destination
//       texture, given the source and destination rectangles
//-----------------------------------------------------------------------------
HRESULT GetTextureCoords( STexPic *pTexSrc, RECT* pRectSrc, STexPic *pTexDest, RECT* pRectDest, CoordRect* pCoords)
{
  float tU, tV;

  // Validate arguments
  if( pTexSrc == NULL || pTexDest == NULL || pCoords == NULL )
    return E_INVALIDARG;

  // Start with a default mapping of the complete source surface to complete 
  // destination surface
  pCoords->fLeftU = 0.0f;
  pCoords->fTopV = 0.0f;
  pCoords->fRightU = 1.0f; 
  pCoords->fBottomV = 1.0f;

  // If not using the complete source surface, adjust the coordinates
  if(pRectSrc != NULL)
  {
    // These delta values are the distance between source texel centers in 
    // texture address space
    tU = 1.0f / pTexSrc->m_Width;
    tV = 1.0f / pTexSrc->m_Height;

    pCoords->fLeftU += pRectSrc->left * tU;
    pCoords->fTopV += pRectSrc->top * tV;
    pCoords->fRightU -= (pTexSrc->m_Width - pRectSrc->right) * tU;
    pCoords->fBottomV -= (pTexSrc->m_Height - pRectSrc->bottom) * tV;
  }

  // If not drawing to the complete destination surface, adjust the coordinates
  if(pRectDest != NULL)
  {
    // These delta values are the distance between source texel centers in 
    // texture address space
    tU = 1.0f / pTexDest->m_Width;
    tV = 1.0f / pTexDest->m_Height;

    pCoords->fLeftU -= pRectDest->left * tU;
    pCoords->fTopV -= pRectDest->top * tV;
    pCoords->fRightU += (pTexDest->m_Width - pRectDest->right) * tU;
    pCoords->fBottomV += (pTexDest->m_Height - pRectDest->bottom) * tV;
  }

  return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GaussianDistribution
// Desc: Helper function for GetSampleOffsets function to compute the 
//       2 parameter Gaussian distrubution using the given standard deviation
//       rho
//-----------------------------------------------------------------------------
float GaussianDistribution( float x, float y, float rho )
{
  float g = 1.0f / sqrtf(2.0f * D3DX_PI * rho * rho);
  g *= expf( -(x*x + y*y)/(2*rho*rho) );

  return g;
}

//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_GaussBlur5x5
// Desc: Get the texture coordinate offsets to be used inside the GaussBlur5x5
//       pixel shader.
//-----------------------------------------------------------------------------
HRESULT GetSampleOffsets_GaussBlur5x5(DWORD dwD3DTexWidth, DWORD dwD3DTexHeight, D3DXVECTOR4* avTexCoordOffset, D3DXVECTOR4* avSampleWeight, FLOAT fMultiplier)
{
  float tu = 1.0f / (float)dwD3DTexWidth ;
  float tv = 1.0f / (float)dwD3DTexHeight ;

  D3DXVECTOR4 vWhite( 1.0f, 1.0f, 1.0f, 1.0f );

  float totalWeight = 0.0f;
  int index=0;
  for(int x=-2; x<=2; x++)
  {
    for(int y=-2; y<=2; y++)
    {
      // Exclude pixels with a block distance greater than 2. This will
      // create a kernel which approximates a 5x5 kernel using only 13
      // sample points instead of 25; this is necessary since 2.0 shaders
      // only support 16 texture grabs.
      if( abs(x) + abs(y) > 2 )
        continue;

      // Get the unscaled Gaussian intensity for this offset
      avTexCoordOffset[index] = D3DXVECTOR4(x*tu, y*tv, 0, 1);
      avSampleWeight[index] = vWhite * GaussianDistribution( (float)x, (float)y, 1.0f);
      totalWeight += avSampleWeight[index].x;

      index++;
    }
  }

  // Divide the current weight by the total weight of all the samples; Gaussian
  // blur kernels add to 1.0f to ensure that the intensity of the image isn't
  // changed when the blur occurs. An optional multiplier variable is used to
  // add or remove image intensity during the blur.
  for(int i=0; i<index; i++)
  {
    avSampleWeight[i] /= totalWeight;
    avSampleWeight[i] *= fMultiplier;
  }

  return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_GaussBlur5x5Bilinear
// Desc: Get the texture coordinate offsets to be used inside the GaussBlur5x5
//       pixel shader.
//-----------------------------------------------------------------------------
HRESULT GetSampleOffsets_GaussBlur5x5Bilinear(DWORD dwD3DTexWidth, DWORD dwD3DTexHeight, D3DXVECTOR4* avTexCoordOffset, D3DXVECTOR4* avSampleWeight, FLOAT fMultiplier)
{
  float tu = 1.0f / (float)dwD3DTexWidth ;
  float tv = 1.0f / (float)dwD3DTexHeight ;
  float totalWeight = 0.0f;
  D3DXVECTOR4 vWhite( 1.f, 1.f, 1.f, 1.f );
  float fWeights[5];

  int index = 0;
  for (int x=-2; x<=2; x++, index++)
  {
    fWeights[index] = GaussianDistribution( (float)x, 0.f, 1.f );
  }

  //  compute weights for the 2x2 taps.  only 9 bilinear taps are required to sample the entire area.
  index = 0;
  for (int y=-2; y<=2; y+=2)
  {
    float tScale = (y==2)?fWeights[4] : (fWeights[y+2] + fWeights[y+3]);
    float tFrac  = fWeights[y+2] / tScale;
    float tOfs   = ((float)y + (1.f-tFrac)) * tv;
    for (int x=-2; x<=2; x+=2, index++)
    {
      float sScale = (x==2)?fWeights[4] : (fWeights[x+2] + fWeights[x+3]);
      float sFrac  = fWeights[x+2] / sScale;
      float sOfs   = ((float)x + (1.f-sFrac)) * tu;
      avTexCoordOffset[index] = D3DXVECTOR4(sOfs, tOfs, 0, 1);
      avSampleWeight[index]   = vWhite * sScale * tScale;
      totalWeight += sScale * tScale;
    }
  }

  for ( int i=0; i<index; i++)
  {
    avSampleWeight[i] *= (fMultiplier / totalWeight);
  }

  return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_Bloom
// Desc: Get the texture coordinate offsets to be used inside the Bloom
//       pixel shader.
//-----------------------------------------------------------------------------
HRESULT GetSampleOffsets_Bloom(DWORD dwD3DTexSize, float afTexCoordOffset[15], D3DXVECTOR4* avColorWeight, float fDeviation, float fMultiplier)
{
  int i=0;
  float tu = 1.0f / (float)dwD3DTexSize;

  // Fill the center texel
  float weight = fMultiplier * GaussianDistribution( 0, 0, fDeviation );
  avColorWeight[0] = D3DXVECTOR4( weight, weight, weight, 1.0f );

  afTexCoordOffset[0] = 0.0f;

  // Fill the first half
  for(i=1; i<8; i++)
  {
    // Get the Gaussian intensity for this offset
    weight = fMultiplier * GaussianDistribution( (float)i, 0, fDeviation );
    afTexCoordOffset[i] = i * tu;

    avColorWeight[i] = D3DXVECTOR4( weight, weight, weight, 1.0f );
  }

  // Mirror to the second half
  for(i=8; i<15; i++)
  {
    avColorWeight[i] = avColorWeight[i-7];
    afTexCoordOffset[i] = -afTexCoordOffset[i-7];
  }

  return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_BloomBilinear
// Desc: Get the texture coordinate offsets to be used inside the Bloom
//       pixel shader.
//-----------------------------------------------------------------------------
HRESULT GetSampleOffsets_BloomBilinear(DWORD dwD3DTexSize, float afTexCoordOffset[15], D3DXVECTOR4* avColorWeight, float fDeviation, float fMultiplier)
{
  int i=0;
  float tu = 1.0f / (float)dwD3DTexSize;

  //  store all the intermediate offsets & weights, then compute the bilinear
  //  taps in a second pass
  //  NOTE: always go left-to-right.
  float tmpWeightArray[15];
  float tmpOffsetArray[15];

  // Fill the center texel
  tmpWeightArray[7] = fMultiplier * GaussianDistribution( 0, 0, fDeviation );
  tmpOffsetArray[7] = 0.0f;

  // Fill the first half
  for(i=0; i<7; i++)
  {
    // Get the Gaussian intensity for this offset
    tmpWeightArray[i] = fMultiplier * GaussianDistribution( (float)(7-i), 0, fDeviation );
    tmpOffsetArray[i] = -(float)(7-i) * tu;
  }

  // Mirror to the second half
  for(i=8; i<15; i++)
  {
    tmpWeightArray[i] = tmpWeightArray[14-i];
    tmpOffsetArray[i] =-tmpOffsetArray[14-i];
  }

  //  now, munge these into bilinear weights

  for(i=0; i<7; i++)
  {
    float sScale = tmpWeightArray[i*2] + tmpWeightArray[i*2+1];
    float sFrac  = tmpWeightArray[i*2] / sScale;
    afTexCoordOffset[i] = tmpOffsetArray[i*2] + (1.f-sFrac)*tu;
    avColorWeight[i] = D3DXVECTOR4(sScale, sScale, sScale, 2.f);
  }

  afTexCoordOffset[7] = tmpOffsetArray[14];
  avColorWeight[7] = D3DXVECTOR4( tmpWeightArray[14], tmpWeightArray[14], tmpWeightArray[14], 1.f );

  return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_DownScale4x4
// Desc: Get the texture coordinate offsets to be used inside the DownScale4x4
//       pixel shader.
//-----------------------------------------------------------------------------
HRESULT GetSampleOffsets_DownScale4x4( DWORD dwWidth, DWORD dwHeight, D3DXVECTOR4 avSampleOffsets[])
{
  if(NULL == avSampleOffsets)
    return E_INVALIDARG;

  float tU = 1.0f / dwWidth;
  float tV = 1.0f / dwHeight;

  // Sample from the 16 surrounding points. Since the center point will be in
  // the exact center of 16 texels, a 0.5f offset is needed to specify a texel
  // center.
  int index=0;
  for(int y=0; y<4; y++)
  {
    for(int x=0; x<4; x++)
    {
      avSampleOffsets[index].x = (x - 1.5f) * tU;
      avSampleOffsets[index].y = (y - 1.5f) * tV;
      avSampleOffsets[index].z = 0;
      avSampleOffsets[index].w = 1;

      index++;
    }
  }

  return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_DownScale4x4Bilinear
// Desc: Get the texture coordinate offsets to be used inside the DownScale4x4
//       pixel shader.
//-----------------------------------------------------------------------------
HRESULT GetSampleOffsets_DownScale4x4Bilinear( DWORD dwWidth, DWORD dwHeight, D3DXVECTOR4 avSampleOffsets[])
{
  if ( NULL == avSampleOffsets )
    return E_INVALIDARG;

  float tU = 1.0f / dwWidth;
  float tV = 1.0f / dwHeight;

  // Sample from the 16 surrounding points.  Since bilinear filtering is being used, specific the coordinate
  // exactly halfway between the current texel center (k-1.5) and the neighboring texel center (k-0.5)

  int index=0;
  for(int y=0; y<4; y+=2)
  {
    for(int x=0; x<4; x+=2, index++)
    {
      avSampleOffsets[index].x = (x - 1.f) * tU;
      avSampleOffsets[index].y = (y - 1.f) * tV;
      avSampleOffsets[index].z = 0;
      avSampleOffsets[index].w = 1;
    }
  }

  return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_DownScale2x2
// Desc: Get the texture coordinate offsets to be used inside the DownScale2x2
//       pixel shader.
//-----------------------------------------------------------------------------
HRESULT GetSampleOffsets_DownScale2x2( DWORD dwWidth, DWORD dwHeight, D3DXVECTOR4 avSampleOffsets[] )
{
  if( NULL == avSampleOffsets )
    return E_INVALIDARG;

  float tU = 1.0f / dwWidth;
  float tV = 1.0f / dwHeight;

  // Sample from the 4 surrounding points. Since the center point will be in
  // the exact center of 4 texels, a 0.5f offset is needed to specify a texel
  // center.
  int index=0;
  for( int y=0; y < 2; y++ )
  {
    for( int x=0; x < 2; x++ )
    {
      avSampleOffsets[index].x = (x - 0.5f) * tU;
      avSampleOffsets[index].y = (y - 0.5f) * tV;
      avSampleOffsets[index].z = 0;
      avSampleOffsets[index].w = 1;

      index++;
    }
  }

  return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetSampleOffsets_DownScale2x2Bilinear
// Desc: Get the texture coordinate offsets to be used inside the DownScale2x2
//       pixel shader.
//-----------------------------------------------------------------------------
HRESULT GetSampleOffsets_DownScale2x2Bilinear( DWORD dwWidth, DWORD dwHeight, D3DXVECTOR4 avSampleOffsets[] )
{
  if( NULL == avSampleOffsets )
    return E_INVALIDARG;

  float tU = 1.0f / dwWidth;
  float tV = 1.0f / dwHeight;

  // Sample from the 4 surrounding points. Since the center point will be in
  // the exact center of 4 texels, a 0.5f offset is needed to specify a texel
  // center.
  int index=0;
  for(int y=0; y<2; y+=2 )
  {
    for(int x=0; x<2; x+=2)
    {
      avSampleOffsets[index].x = x * tU;
      avSampleOffsets[index].y = y * tV;
      avSampleOffsets[index].z = 0;
      avSampleOffsets[index].w = 1;

      index++;
    }
  }

  return S_OK;
}

bool HDR_SceneToSceneScaled()
{
  HRESULT hr = S_OK;
  D3DXVECTOR4 avSampleOffsets[16];
  CD3D9Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = rd->m_pd3dDevice;

  // Get the new render target surface
  PDIRECT3DSURFACE9 pSurfScaledScene[2];
  PDIRECT3DSURFACE9 pSurfTempScene;
  pSurfScaledScene[0] = GetSurfaceTP(rd->m_TexMan->m_Text_HDRTargetScaled[0]);
  pSurfScaledScene[1] = GetSurfaceTP(rd->m_TexMan->m_Text_HDRTargetScaled[1]);
  pSurfTempScene = GetSurfaceTP(rd->m_TexMan->m_Text_HDRTarget_Temp);

  // Create a 1/4 x 1/4 scale copy of the HDR texture. Since bloom textures
  // are 1/8 x 1/8 scale, border texels of the HDR texture will be discarded 
  // to keep the dimensions evenly divisible by 8; this allows for precise 
  // control over sampling inside pixel shaders.
  CCGPShader_D3D *fpDownScale;
  CCGPShader_D3D *fpTemp;

  // Place the rectangle in the center of the back buffer surface
  RECT rectSrc;
  rectSrc.left = (rd->GetWidth() - rd->m_dwHDRCropWidth) / 2;
  rectSrc.top = (rd->GetHeight() - rd->m_dwHDRCropHeight) / 2;
  rectSrc.right = rectSrc.left + rd->m_dwHDRCropWidth;
  rectSrc.bottom = rectSrc.top + rd->m_dwHDRCropHeight;

  if (rd->m_nHDRType == 2)
  {
    SetTexture(rd, rd->m_TexMan->m_Text_HDRTarget, 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
    //SetTexture(rd, rd->m_TexMan->m_Text_HDRTarget_K, 1, D3DTEXF_POINT, D3DTEXF_POINT, true);
    SetTexture(rd, rd->m_TexMan->m_Text_White, 1, D3DTEXF_POINT, D3DTEXF_POINT, true);
    dv->SetRenderTarget(0, pSurfTempScene);
    fpTemp = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRTemp, "CGRC_HDR_MRTK_2_FP16_PS20");
    assert(fpTemp);
    if (fpTemp)
    {
      fpTemp->mfSet(true);
      DrawFullScreenQuad(0, 0, 1, 1);
      fpTemp->mfSet(false);
    }
    SetTexture(rd, NULL, 1, D3DTEXF_POINT, D3DTEXF_POINT, true);
    rd->EF_SelectTMU(0);
  }

  // Get the texture coordinates for the render target
  CoordRect coords;
  GetTextureCoords(rd->m_TexMan->m_Text_HDRTarget, &rectSrc, rd->m_TexMan->m_Text_HDRTargetScaled[0], NULL, &coords);

  int nBitPlanes = rd->m_bDeviceSupportsMRT ? 2 : 1;
  for (int i=0; i<nBitPlanes; i++)
  {
    dv->SetRenderTarget(0, pSurfScaledScene[i]);

    if (!rd->m_bDeviceSupportsMRT)
    {
      if (rd->m_bDeviceSupportsFP16Filter)
        fpDownScale = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRDownScale4x4, "CGRC_HDR_DownScale4x4_Bilinear_PS20");
      else
        fpDownScale = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRDownScale4x4, "CGRC_HDR_DownScale4x4_PS20");
    }
    else
    {
      if (rd->m_bDeviceSupportsFP16Filter)
      {
        if (!i)
          fpDownScale = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRDownScale4x4_RG, "CGRC_HDR_DownScale4x4_Bilinear_RG_PS20");
        else
          fpDownScale = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRDownScale4x4_BA, "CGRC_HDR_DownScale4x4_Bilinear_BA_PS20");
      }
      else
      {
        if (!i)
          fpDownScale = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRDownScale4x4_RG, "CGRC_HDR_DownScale4x4_RG_PS20");
        else
          fpDownScale = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRDownScale4x4_BA, "CGRC_HDR_DownScale4x4_BA_PS20");
      }
    }

    if (!fpDownScale)
      return false;
    fpDownScale->mfSet(true, 0);

    if (!i)
    {
      // Get the sample offsets used within the pixel shader
      SCGBind *bind = fpDownScale->mfGetParameterBind("vSampleOffsets");
      assert(bind);
      if (rd->m_bDeviceSupportsFP16Filter)
      {
        GetSampleOffsets_DownScale4x4Bilinear(rd->GetWidth(), rd->GetHeight(), avSampleOffsets);
        fpDownScale->mfParameter(bind, &avSampleOffsets[0].x, 4);
        SetTexture(rd, rd->m_TexMan->m_Text_HDRTarget, 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);
      }
      else
      {
        GetSampleOffsets_DownScale4x4(rd->GetWidth(), rd->GetHeight(), avSampleOffsets);
        fpDownScale->mfParameter(bind, &avSampleOffsets[0].x, 16);
        if (rd->m_nHDRType == 2)
          SetTexture(rd, rd->m_TexMan->m_Text_HDRTarget_Temp, 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
        else
          SetTexture(rd, rd->m_TexMan->m_Text_HDRTarget, 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
      }
    }

    // Draw a fullscreen quad
    DrawFullScreenQuad(coords);
  }

  fpDownScale->mfSet(false, 0);
  SAFE_RELEASE(pSurfScaledScene[0]);
  SAFE_RELEASE(pSurfScaledScene[1]);
  SAFE_RELEASE(pSurfTempScene);
  if (rd->m_bDeviceSupportsMRT)
    dv->SetRenderTarget(1, NULL);

  return true;
}

bool HDR_MeasureLuminance()
{
  HRESULT hr = S_OK;
  int i, x, y, index;
  D3DXVECTOR4 avSampleOffsets[16];
  CD3D9Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = rd->m_pd3dDevice;
  CCGPShader_D3D *fpSampleAvgLum = NULL;
  CCGPShader_D3D *fpResampleAvgLum = NULL;
  CCGPShader_D3D *fpResampleAvgLumExp = NULL;
  SCGBind *bind;
  bool bResult = false;

  DWORD dwCurTexture = NUM_HDR_TONEMAP_TEXTURES-1;

  // Sample log average luminance
  PDIRECT3DSURFACE9 apSurfToneMap[NUM_HDR_TONEMAP_TEXTURES] = {0};

  // Retrieve the tonemap surfaces
  for(i=0; i<NUM_HDR_TONEMAP_TEXTURES; i++)
  {
    apSurfToneMap[i] = GetSurfaceTP(rd->m_TexMan->m_Text_HDRToneMaps[i]);
    if (!apSurfToneMap[i])
      goto LCleanReturn;
  }

  // Initialize the sample offsets for the initial luminance pass.
  float tU, tV;
  tU = 1.0f / (3.0f * rd->m_TexMan->m_Text_HDRToneMaps[dwCurTexture]->m_Width);
  tV = 1.0f / (3.0f * rd->m_TexMan->m_Text_HDRToneMaps[dwCurTexture]->m_Height);

  index=0;
  for(x=-1; x<=1; x++)
  {
    for(y=-1; y<=1; y++)
    {
      avSampleOffsets[index].x = x * tU;
      avSampleOffsets[index].y = y * tV;
      avSampleOffsets[index].z = 0;
      avSampleOffsets[index].w = 1;

      index++;
    }
  }


  // After this pass, the CTexMan::m_Text_HDRToneMaps[NUM_TONEMAP_TEXTURES-1] texture will contain
  // a scaled, grayscale copy of the HDR scene. Individual texels contain the log 
  // of average luminance values for points sampled on the HDR texture.
  if (rd->m_bDeviceSupportsMRT)
    fpSampleAvgLum = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRSampleAvgLum, "CGRC_HDR_SampleAvgLum_FromMRT_PS20");
  else
    fpSampleAvgLum = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRSampleAvgLum, "CGRC_HDR_SampleAvgLum_PS20");
  if (!fpSampleAvgLum)
    goto LCleanReturn;
  fpSampleAvgLum->mfSet(true, 0);
  bind = fpSampleAvgLum->mfGetParameterBind("vSampleOffsets");
  assert(bind);
  fpSampleAvgLum->mfParameter(bind, &avSampleOffsets[0].x, 9);

  if (rd->m_bDeviceSupportsMRT)
  {
    SetTexture(rd, rd->m_TexMan->m_Text_HDRTargetScaled[0], 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
    SetTexture(rd, rd->m_TexMan->m_Text_HDRTargetScaled[1], 1, D3DTEXF_POINT, D3DTEXF_POINT, true);
  }
  else
    SetTexture(rd, rd->m_TexMan->m_Text_HDRTargetScaled[0], 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
  dv->SetRenderTarget(0, apSurfToneMap[dwCurTexture]);

  // Draw a fullscreen quad to sample the RT
  DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);

  if (rd->m_bDeviceSupportsMRT )
    dv->SetTexture(1, NULL);

  dwCurTexture--;

  // Initialize the sample offsets for the iterative luminance passes
  while(dwCurTexture > 0)
  {
    dv->SetRenderTarget(0, apSurfToneMap[dwCurTexture]);
    if (rd->m_bDeviceSupportsFP16Filter)
    {
      GetSampleOffsets_DownScale4x4Bilinear(rd->m_TexMan->m_Text_HDRToneMaps[dwCurTexture+1]->m_Width, rd->m_TexMan->m_Text_HDRToneMaps[dwCurTexture+1]->m_Height, avSampleOffsets);
      // Each of these passes continue to scale down the log of average
      // luminance texture created above, storing intermediate results in 
      // HDRToneMaps[1] through HDRToneMaps[NUM_HDR_TONEMAP_TEXTURES-1].
      fpResampleAvgLum = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRResampleAvgLum, "CGRC_HDR_ResampleAvgLum_Bilinear_PS20");
      if (!fpResampleAvgLum)
        goto LCleanReturn;
      fpResampleAvgLum->mfSet(true, 0);
      bind = fpResampleAvgLum->mfGetParameterBind("vSampleOffsets");
      assert(bind);
      fpResampleAvgLum->mfParameter(bind, &avSampleOffsets[0].x, 4);
      dv->SetRenderTarget(0, apSurfToneMap[dwCurTexture]);
      SetTexture(rd, rd->m_TexMan->m_Text_HDRToneMaps[dwCurTexture+1], 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);
    }
    else
    {
      GetSampleOffsets_DownScale4x4(rd->m_TexMan->m_Text_HDRToneMaps[dwCurTexture+1]->m_Width, rd->m_TexMan->m_Text_HDRToneMaps[dwCurTexture+1]->m_Height, avSampleOffsets);

      // Each of these passes continue to scale down the log of average
      // luminance texture created above, storing intermediate results in 
      // HDRToneMaps[1] through HDRToneMaps[NUM_HDR_TONEMAP_TEXTURES-1].
      fpResampleAvgLum = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRResampleAvgLum, "CGRC_HDR_ResampleAvgLum_PS20");
      if (!fpResampleAvgLum)
        goto LCleanReturn;
      fpResampleAvgLum->mfSet(true, 0);
      bind = fpResampleAvgLum->mfGetParameterBind("vSampleOffsets");
      assert(bind);
      fpResampleAvgLum->mfParameter(bind, &avSampleOffsets[0].x, 16);
      SetTexture(rd, rd->m_TexMan->m_Text_HDRToneMaps[dwCurTexture+1], 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
    }

    // Draw a fullscreen quad to sample the RT
    DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);

    dwCurTexture--;
  }

  if (rd->m_bDeviceSupportsFP16Filter)
  {
    GetSampleOffsets_DownScale4x4Bilinear(rd->m_TexMan->m_Text_HDRToneMaps[1]->m_Width, rd->m_TexMan->m_Text_HDRToneMaps[1]->m_Height, avSampleOffsets);
    // Perform the final pass of the average luminance calculation. This pass
    // scales the 4x4 log of average luminance texture from above and performs
    // an exp() operation to return a single texel cooresponding to the average
    // luminance of the scene in CTexMan::m_Text_HDRToneMaps[0].
    fpResampleAvgLumExp = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRResampleAvgLumExp, "CGRC_HDR_ResampleAvgLumExp_Bilinear_PS20");
    if (!fpResampleAvgLumExp)
      goto LCleanReturn;
    fpResampleAvgLumExp->mfSet(true, 0);
    bind = fpResampleAvgLumExp->mfGetParameterBind("vSampleOffsets");
    assert(bind);
    fpResampleAvgLumExp->mfParameter(bind, &avSampleOffsets[0].x, 4);
    SetTexture(rd, rd->m_TexMan->m_Text_HDRToneMaps[1], 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);
  }
  else
  {
    GetSampleOffsets_DownScale4x4(rd->m_TexMan->m_Text_HDRToneMaps[1]->m_Width, rd->m_TexMan->m_Text_HDRToneMaps[1]->m_Height, avSampleOffsets);

    // Each of these passes continue to scale down the log of average
    // luminance texture created above, storing intermediate results in 
    // HDRToneMaps[1] through HDRToneMaps[NUM_HDR_TONEMAP_TEXTURES-1].
    fpResampleAvgLumExp = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRResampleAvgLumExp, "CGRC_HDR_ResampleAvgLumExp_PS20");
    if (!fpResampleAvgLumExp)
      goto LCleanReturn;
    fpResampleAvgLumExp->mfSet(true, 0);
    bind = fpResampleAvgLumExp->mfGetParameterBind("vSampleOffsets");
    assert(bind);
    fpResampleAvgLumExp->mfParameter(bind, &avSampleOffsets[0].x, 16);
    SetTexture(rd, rd->m_TexMan->m_Text_HDRToneMaps[1], 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
  }

  dv->SetRenderTarget(0, apSurfToneMap[0]);

  // Draw a fullscreen quad to sample the RT
  DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);

  bResult = true;
LCleanReturn:
  for(i=0; i<NUM_HDR_TONEMAP_TEXTURES; i++)
  {
    SAFE_RELEASE(apSurfToneMap[i]);
  }
  if (fpSampleAvgLum)
    fpSampleAvgLum->mfSet(false, 0);
  if (fpResampleAvgLum)
    fpResampleAvgLum->mfSet(false, 0);
  if (fpResampleAvgLumExp)
    fpResampleAvgLumExp->mfSet(false, 0);

  return bResult;
}

//-----------------------------------------------------------------------------
// Name: HDR_CalculateAdaptation()
// Desc: Increment the user's adapted luminance
//-----------------------------------------------------------------------------
bool HDR_CalculateAdaptation()
{
  HRESULT hr = S_OK;
  CD3D9Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = rd->m_pd3dDevice;
  CD3D9TexMan *tm = (CD3D9TexMan *)rd->m_TexMan;
  CCGPShader_D3D *fpCalcAdaptedLum = NULL;
  bool bResult = false;

  // Swap current & last luminance
  STexPic *pTexSwap = tm->m_Text_HDRAdaptedLuminanceLast;
  tm->m_Text_HDRAdaptedLuminanceLast = tm->m_Text_HDRAdaptedLuminanceCur;
  tm->m_Text_HDRAdaptedLuminanceCur = pTexSwap;

  PDIRECT3DSURFACE9 pSurfAdaptedLum = GetSurfaceTP(tm->m_Text_HDRAdaptedLuminanceCur);
  if(!pSurfAdaptedLum)
    return false;

  // This simulates the light adaptation that occurs when moving from a 
  // dark area to a bright area, or vice versa. The g_pTexAdaptedLuminance
  // texture stores a single texel cooresponding to the user's adapted 
  // level.
  fpCalcAdaptedLum = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRCalcAdaptedLum, "CGRC_HDR_CalculateAdaptedLum_PS20");
  if (!fpCalcAdaptedLum)
    goto LCleanReturn;
  fpCalcAdaptedLum->mfSet(true, 0);
  float v[4];
  v[0] = iTimer->GetFrameTime();
  v[1] = v[2] = v[3] = 0;
  fpCalcAdaptedLum->mfParameter4f("fElapsedTime", v);

  dv->SetRenderTarget(0, pSurfAdaptedLum);
  SetTexture(rd, tm->m_Text_HDRAdaptedLuminanceLast, 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
  SetTexture(rd, tm->m_Text_HDRToneMaps[0], 1, D3DTEXF_POINT, D3DTEXF_POINT, true);

  // Draw a fullscreen quad to sample the RT
  DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);

  bResult = true;

LCleanReturn:
  if (fpCalcAdaptedLum)
    fpCalcAdaptedLum->mfSet(false, 0);
  rd->EF_SelectTMU(0);
  SAFE_RELEASE(pSurfAdaptedLum);

  return bResult;
}

//-----------------------------------------------------------------------------
// Name: SceneScaled_To_BrightPass
// Desc: Run the bright-pass filter on CTexman::m_Text_HDRTargetScaled and place the result
//       in CTexMan::m_Text_HDRBrightPass
//-----------------------------------------------------------------------------
bool HDR_SceneScaledToBrightPass()
{
  HRESULT hr = S_OK;
  CD3D9Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = rd->m_pd3dDevice;
  CD3D9TexMan *tm = (CD3D9TexMan *)rd->m_TexMan;
  CCGPShader_D3D *fpBrightPassFilter = NULL;
  bool bResult = false;

  // Get the new render target surface
  PDIRECT3DSURFACE9 pSurfBrightPass = GetSurfaceTP(tm->m_Text_HDRBrightPass);
  if(!pSurfBrightPass)
    return false;

	// clear destination surface
	dv->ColorFill(pSurfBrightPass, NULL, D3DCOLOR_ARGB(0, 0, 0, 0));

  // Get the rectangle describing the sampled portion of the source texture.
  // Decrease the rectangle to adjust for the single pixel black border.
  RECT rectSrc;
  GetTextureRect(tm->m_Text_HDRTargetScaled[0], &rectSrc);
  InflateRect(&rectSrc, -1, -1);

  // Get the destination rectangle.
  // Decrease the rectangle to adjust for the single pixel black border.
  RECT rectDest;
  GetTextureRect(tm->m_Text_HDRBrightPass, &rectDest);
  InflateRect(&rectDest, -1, -1);

  // Get the correct texture coordinates to apply to the rendered quad in order 
  // to sample from the source rectangle and render into the destination rectangle
  CoordRect coords;
  GetTextureCoords(tm->m_Text_HDRTargetScaled[0], &rectSrc, tm->m_Text_HDRBrightPass, &rectDest, &coords);

  // The bright-pass filter removes everything from the scene except lights and
  // bright reflections
  if (rd->m_bDeviceSupportsMRT)
    fpBrightPassFilter = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRBrightPassFilter, "CGRC_HDR_BrightPassFilter_FromMRT_PS20");
  else
    fpBrightPassFilter = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRBrightPassFilter, "CGRC_HDR_BrightPassFilter_PS20");
  if (!fpBrightPassFilter)
    goto LCleanReturn;
  fpBrightPassFilter->mfSet(true, 0);

  float v[4];
  v[0] = CRenderer::CV_r_hdrlevel;
  v[1] = CRenderer::CV_r_hdrbrightoffset;
  v[2] = CRenderer::CV_r_hdrbrightthreshold;
  v[3] = 0;

  fpBrightPassFilter->mfParameter4f("Params", v);

  dv->SetRenderTarget(0, pSurfBrightPass);
  if (rd->m_bDeviceSupportsMRT)
  {
    SetTexture(rd, tm->m_Text_HDRTargetScaled[0], 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
    SetTexture(rd, tm->m_Text_HDRTargetScaled[1], 1, D3DTEXF_POINT, D3DTEXF_POINT, true);
    SetTexture(rd, tm->m_Text_HDRAdaptedLuminanceCur, 2, D3DTEXF_POINT, D3DTEXF_POINT, true);
  }
  else
  {
    SetTexture(rd, tm->m_Text_HDRTargetScaled[0], 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
    SetTexture(rd, tm->m_Text_HDRAdaptedLuminanceCur, 1, D3DTEXF_POINT, D3DTEXF_POINT, true);
  }

  dv->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
  dv->SetScissorRect(&rectDest);

  // Draw a fullscreen quad to sample the RT
  DrawFullScreenQuad(coords);

  dv->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

  bResult = true;
LCleanReturn:
  SAFE_RELEASE(pSurfBrightPass);
  rd->EF_SelectTMU(0);
  if (fpBrightPassFilter)
    fpBrightPassFilter->mfSet(false, 0);

  return bResult;
}

//-----------------------------------------------------------------------------
// Name: HDR_BrightPass_To_StarSource
// Desc: Perform a 5x5 gaussian blur on g_pTexBrightPass and place the result
//       in g_pTexStarSource. The bright-pass filtered image is blurred before
//       being used for star operations to avoid aliasing artifacts.
//-----------------------------------------------------------------------------
bool HDR_BrightPassToStarSource()
{
  HRESULT hr = S_OK;
  CD3D9Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = rd->m_pd3dDevice;
  CD3D9TexMan *tm = (CD3D9TexMan *)rd->m_TexMan;
  CCGPShader_D3D *fpGaussBlur5x5 = NULL;
  bool bResult = false;
  SCGBind *bind = NULL;

  D3DXVECTOR4 avSampleOffsets[16];
  D3DXVECTOR4 avSampleWeights[16];

  // Get the new render target surface
  PDIRECT3DSURFACE9 pSurfStarSource = GetSurfaceTP(tm->m_Text_HDRStarSource);
  if(!pSurfStarSource)
    return false;

	// clear destination surface
	dv->ColorFill(pSurfStarSource, NULL, D3DCOLOR_ARGB(0, 0, 0, 0));

  // Get the destination rectangle.
  // Decrease the rectangle to adjust for the single pixel black border.
  RECT rectDest;
  GetTextureRect(tm->m_Text_HDRStarSource, &rectDest);
  InflateRect(&rectDest, -1, -1);

  // Get the correct texture coordinates to apply to the rendered quad in order 
  // to sample from the source rectangle and render into the destination rectangle
  CoordRect coords;
  GetTextureCoords(tm->m_Text_HDRBrightPass, NULL, tm->m_Text_HDRStarSource, &rectDest, &coords);

  // The gaussian blur smooths out rough edges to avoid aliasing effects
  // when the star effect is run
  if (rd->m_bDeviceSupportsFP16Filter)
  {
    GetSampleOffsets_GaussBlur5x5Bilinear(tm->m_Text_HDRBrightPass->m_Width, tm->m_Text_HDRBrightPass->m_Height, avSampleOffsets, avSampleWeights, 1.0f);
    fpGaussBlur5x5 = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRGaussBlur5x5_Bilinear, "CGRC_HDR_GaussBlur5x5_Bilinear_PS20");
  }
  else
  {
    GetSampleOffsets_GaussBlur5x5(tm->m_Text_HDRBrightPass->m_Width, tm->m_Text_HDRBrightPass->m_Height, avSampleOffsets, avSampleWeights, 1.0f);
    fpGaussBlur5x5 = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRGaussBlur5x5, "CGRC_HDR_GaussBlur5x5_PS20");
  }

  if (!fpGaussBlur5x5)
    goto LCleanReturn;
  fpGaussBlur5x5->mfSet(true, 0);

  bind = fpGaussBlur5x5->mfGetParameterBind("vSampleOffsets");
  assert(bind);
  if (rd->m_bDeviceSupportsFP16Filter)
    fpGaussBlur5x5->mfParameter(bind, &avSampleOffsets[0].x, 9);
  else
    fpGaussBlur5x5->mfParameter(bind, &avSampleOffsets[0].x, 13);

  bind = fpGaussBlur5x5->mfGetParameterBind("vSampleWeights");
  assert(bind);
  if (rd->m_bDeviceSupportsFP16Filter)
    fpGaussBlur5x5->mfParameter(bind, &avSampleWeights[0].x, 9);
  else
    fpGaussBlur5x5->mfParameter(bind, &avSampleWeights[0].x, 13);

  dv->SetRenderTarget(0, pSurfStarSource);
  if (rd->m_bDeviceSupportsFP16Filter)
    SetTexture(rd, tm->m_Text_HDRBrightPass, 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);
  else
    SetTexture(rd, tm->m_Text_HDRBrightPass, 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
  dv->SetScissorRect(&rectDest);
  dv->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

  // Draw a fullscreen quad
  DrawFullScreenQuad(coords);

  dv->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

  bResult = true;
LCleanReturn:
  SAFE_RELEASE(pSurfStarSource);
  rd->EF_SelectTMU(0);
  if (fpGaussBlur5x5)
    fpGaussBlur5x5->mfSet(false, 0);

  return bResult;
}

//-----------------------------------------------------------------------------
// Name: HDR_StarSource_To_BloomSource
// Desc: Scale down g_pTexStarSource by 1/2 x 1/2 and place the result in 
//       CTexMan::m_Text_HDRBloomSource
//-----------------------------------------------------------------------------
bool HDR_StarSourceToBloomSource()
{
  HRESULT hr = S_OK;
  CD3D9Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = rd->m_pd3dDevice;
  CD3D9TexMan *tm = (CD3D9TexMan *)rd->m_TexMan;
  CCGPShader_D3D *fpDownScale2x2 = NULL;
  bool bResult = false;
  SCGBind *bind = NULL;
  D3DXVECTOR4 avSampleOffsets[4];

  // Get the new render target surface
  PDIRECT3DSURFACE9 pSurfBloomSource = GetSurfaceTP(tm->m_Text_HDRBloomSource);
  if(!pSurfBloomSource)
    return false;

	// clear destination surface
	dv->ColorFill(pSurfBloomSource, NULL, D3DCOLOR_ARGB(0, 0, 0, 0));

  // Get the rectangle describing the sampled portion of the source texture.
  // Decrease the rectangle to adjust for the single pixel black border.
  RECT rectSrc;
  GetTextureRect(tm->m_Text_HDRStarSource, &rectSrc);
  InflateRect(&rectSrc, -1, -1);

  // Get the destination rectangle.
  // Decrease the rectangle to adjust for the single pixel black border.
  RECT rectDest;
  GetTextureRect(tm->m_Text_HDRBloomSource, &rectDest);
  InflateRect(&rectDest, -1, -1);

  // Get the correct texture coordinates to apply to the rendered quad in order 
  // to sample from the source rectangle and render into the destination rectangle
  CoordRect coords;
  GetTextureCoords(tm->m_Text_HDRStarSource, &rectSrc, tm->m_Text_HDRBloomSource, &rectDest, &coords);

  if (rd->m_bDeviceSupportsFP16Filter)
  {
    GetSampleOffsets_DownScale2x2Bilinear(tm->m_Text_HDRBrightPass->m_Width, tm->m_Text_HDRBrightPass->m_Height, avSampleOffsets);
    fpDownScale2x2 = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRDownScale2x2, "CGRC_HDR_DownScale2x2_Bilinear_PS20");
  }
  else
  {
    GetSampleOffsets_DownScale2x2(tm->m_Text_HDRBrightPass->m_Width, tm->m_Text_HDRBrightPass->m_Height, avSampleOffsets);
    fpDownScale2x2 = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRDownScale2x2, "CGRC_HDR_DownScale2x2_PS20");
  }

  // Create an exact 1/2 x 1/2 copy of the source texture
  if (!fpDownScale2x2)
    goto LCleanReturn;
  fpDownScale2x2->mfSet(true, 0);

  bind = fpDownScale2x2->mfGetParameterBind("vSampleOffsets");
  assert(bind);
  if (rd->m_bDeviceSupportsFP16Filter)
    fpDownScale2x2->mfParameter(bind, &avSampleOffsets[0].x, 1);
  else
    fpDownScale2x2->mfParameter(bind, &avSampleOffsets[0].x, 4);

  dv->SetRenderTarget(0, pSurfBloomSource);
  if (rd->m_bDeviceSupportsFP16Filter)
    SetTexture(rd, tm->m_Text_HDRStarSource, 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);
  else
    SetTexture(rd, tm->m_Text_HDRStarSource, 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
  dv->SetScissorRect(&rectDest);
  dv->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

  // Draw a fullscreen quad
  DrawFullScreenQuad(coords);
 
  dv->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

  bResult = true;
LCleanReturn:
  SAFE_RELEASE(pSurfBloomSource);
  if (fpDownScale2x2)
    fpDownScale2x2->mfSet(false, 0);

  return bResult;
}

//-----------------------------------------------------------------------------
// Name: HDR_RenderBloom()
// Desc: Render the blooming effect
//-----------------------------------------------------------------------------
bool HDR_RenderBloom()
{
  HRESULT hr = S_OK;
  CD3D9Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = rd->m_pd3dDevice;
  CD3D9TexMan *tm = (CD3D9TexMan *)rd->m_TexMan;
  CCGPShader_D3D *fpGaussBlur5x5 = NULL;
  CCGPShader_D3D *fpBloom = NULL;
  bool bResult = false;
  SCGBind *bind = NULL;
  int i;

  D3DXVECTOR4 avSampleOffsets[16];
  FLOAT       afSampleOffsets[16];
  D3DXVECTOR4 avSampleWeights[16];

  PDIRECT3DSURFACE9 pSurfScaledHDR0 = GetSurfaceTP(tm->m_Text_HDRTargetScaled[0]);
  PDIRECT3DSURFACE9 pSurfBloom = GetSurfaceTP(tm->m_Text_HDRBloomMaps[0]);
  PDIRECT3DSURFACE9 pSurfHDR = GetSurfaceTP(tm->m_Text_HDRTarget);
  PDIRECT3DSURFACE9 pSurfTempBloom = GetSurfaceTP(tm->m_Text_HDRBloomMaps[1]);
  PDIRECT3DSURFACE9 pSurfBloomSource = GetSurfaceTP(tm->m_Text_HDRBloomMaps[2]);

  // Clear the bloom texture
  dv->ColorFill(pSurfBloom, NULL, D3DCOLOR_ARGB(0, 0, 0, 0));

  if (g_GlareDef.m_fGlareLuminance <= 0.0f || g_GlareDef.m_fBloomLuminance <= 0.0f)
  {
    hr = S_OK;
    goto LCleanReturn;
  }

  RECT rectSrc;
  GetTextureRect(tm->m_Text_HDRBloomSource, &rectSrc);
  InflateRect(&rectSrc, -1, -1);

  RECT rectDest;
  GetTextureRect(tm->m_Text_HDRBloomMaps[2], &rectDest);
  InflateRect(&rectDest, -1, -1);

  CoordRect coords;
  GetTextureCoords(tm->m_Text_HDRBloomSource, &rectSrc, tm->m_Text_HDRBloomMaps[2], &rectDest, &coords);

  if (rd->m_bDeviceSupportsFP16Filter)
  {
    hr = GetSampleOffsets_GaussBlur5x5Bilinear(tm->m_Text_HDRBloomSource->m_Width, tm->m_Text_HDRBloomSource->m_Height, avSampleOffsets, avSampleWeights, 1.0f);
    fpGaussBlur5x5 = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRGaussBlur5x5_Bilinear, "CGRC_HDR_GaussBlur5x5_Bilinear_PS20");
  }
  else
  {
    hr = GetSampleOffsets_GaussBlur5x5(tm->m_Text_HDRBloomSource->m_Width, tm->m_Text_HDRBloomSource->m_Height, avSampleOffsets, avSampleWeights, 1.0f);
    fpGaussBlur5x5 = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRGaussBlur5x5, "CGRC_HDR_GaussBlur5x5_PS20");
  }

  if (!fpGaussBlur5x5)
    goto LCleanReturn;
  fpGaussBlur5x5->mfSet(true, 0);

  bind = fpGaussBlur5x5->mfGetParameterBind("vSampleOffsets");
  assert(bind);
  if (rd->m_bDeviceSupportsFP16Filter)
    fpGaussBlur5x5->mfParameter(bind, &avSampleOffsets[0].x, 9);
  else
    fpGaussBlur5x5->mfParameter(bind, &avSampleOffsets[0].x, 13);

  bind = fpGaussBlur5x5->mfGetParameterBind("vSampleWeights");
  assert(bind);
  if (rd->m_bDeviceSupportsFP16Filter)
    fpGaussBlur5x5->mfParameter(bind, &avSampleWeights[0].x, 9);
  else
    fpGaussBlur5x5->mfParameter(bind, &avSampleWeights[0].x, 13);

  dv->SetRenderTarget(0, pSurfBloomSource);
  if (rd->m_bDeviceSupportsFP16Filter)
    SetTexture(rd, tm->m_Text_HDRBloomSource, 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);
  else
    SetTexture(rd, tm->m_Text_HDRBloomSource, 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
  dv->SetScissorRect(&rectDest);
  dv->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

  // Draw a fullscreen quad to sample the RT
  DrawFullScreenQuad(coords);

  dv->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

  int n;
  if (rd->m_bDeviceSupportsFP16Filter)
  {
    hr = GetSampleOffsets_BloomBilinear(tm->m_Text_HDRBloomMaps[2]->m_Width, afSampleOffsets, avSampleWeights, 3.0f, 2.0f);
    n = 8;
    fpBloom = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRBloom, "CGRC_HDR_Bloom_Bilinear_PS20");
  }
  else
  {
    hr = GetSampleOffsets_Bloom(tm->m_Text_HDRBloomMaps[2]->m_Width, afSampleOffsets, avSampleWeights, 3.0f, 2.0f);
    n = 16;
    fpBloom = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRBloom, "CGRC_HDR_Bloom_PS20");
  }
  for(i=0; i<n; i++ )
  {
    avSampleOffsets[i] = D3DXVECTOR4(afSampleOffsets[i], 0.0f, 0, 1);
  }

  if (!fpBloom)
    goto LCleanReturn;
  fpBloom->mfSet(true, 0);

  bind = fpBloom->mfGetParameterBind("vSampleOffsets");
  assert(bind);
  fpBloom->mfParameter(bind, &avSampleOffsets[0].x, n);

  bind = fpBloom->mfGetParameterBind("vSampleWeights");
  assert(bind);
  fpBloom->mfParameter(bind, &avSampleWeights[0].x, n);

  dv->SetRenderTarget(0, pSurfTempBloom);
  if (rd->m_bDeviceSupportsFP16Filter)
    SetTexture(rd, tm->m_Text_HDRBloomMaps[2], 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);
  else
    SetTexture(rd, tm->m_Text_HDRBloomMaps[2], 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
  dv->SetScissorRect(&rectDest);
  dv->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

  // Draw a fullscreen quad to sample the RT
  DrawFullScreenQuad( coords );

  dv->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

  if (rd->m_bDeviceSupportsFP16Filter)
  {
    hr = GetSampleOffsets_BloomBilinear(tm->m_Text_HDRBloomMaps[1]->m_Width, afSampleOffsets, avSampleWeights, 3.0f, 2.0f);
    n = 8;
  }
  else
  {
    hr = GetSampleOffsets_Bloom(tm->m_Text_HDRBloomMaps[1]->m_Width, afSampleOffsets, avSampleWeights, 3.0f, 2.0f);
    n = 16;
  }
  for(i=0; i<n; i++ )
  {
    avSampleOffsets[i] = D3DXVECTOR4(afSampleOffsets[i], 0.0f, 0, 1);
  }

  GetTextureRect(tm->m_Text_HDRBloomMaps[1], &rectSrc);
  InflateRect(&rectSrc, -1, -1);

  GetTextureCoords(tm->m_Text_HDRBloomMaps[1], &rectSrc, tm->m_Text_HDRBloomMaps[0], NULL, &coords);

  bind = fpBloom->mfGetParameterBind("vSampleOffsets");
  assert(bind);
  fpBloom->mfParameter(bind, &avSampleOffsets[0].x, n);

  bind = fpBloom->mfGetParameterBind("vSampleWeights");
  assert(bind);
  fpBloom->mfParameter(bind, &avSampleWeights[0].x, n);

  dv->SetRenderTarget(0, pSurfBloom);
  if (rd->m_bDeviceSupportsFP16Filter)
    SetTexture(rd, tm->m_Text_HDRBloomMaps[1], 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);
  else
    SetTexture(rd, tm->m_Text_HDRBloomMaps[1], 0, D3DTEXF_POINT, D3DTEXF_POINT, true);

  // Draw a fullscreen quad to sample the RT
  DrawFullScreenQuad(coords);

  bResult = true;

LCleanReturn:
  SAFE_RELEASE(pSurfBloomSource);
  SAFE_RELEASE(pSurfTempBloom);
  SAFE_RELEASE(pSurfBloom);
  SAFE_RELEASE(pSurfHDR);
  SAFE_RELEASE(pSurfScaledHDR0);

  return bResult;
}

//-----------------------------------------------------------------------------
// Name: HDR_RenderStar()
// Desc: Render the blooming effect
//-----------------------------------------------------------------------------
bool HDR_RenderStar()
{
  HRESULT hr = S_OK;
  CD3D9Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = rd->m_pd3dDevice;
  CD3D9TexMan *tm = (CD3D9TexMan *)rd->m_TexMan;
  CCGPShader_D3D *fpStar = NULL;
  CCGPShader_D3D *fpMergeTextures = NULL;
  bool bResult = false;
  SCGBind *bind = NULL;

  D3DXVECTOR4 avSampleOffsets[16];
  D3DXVECTOR4 avSampleWeights[16];

  int i, d, p, s; // Loop variables

  LPDIRECT3DSURFACE9 pSurfStar0 = GetSurfaceTP(tm->m_Text_HDRStarMaps[0][0]);
  if(!pSurfStar0)
    return false;
  LPDIRECT3DSURFACE9 pSurfStar1 = GetSurfaceTP(tm->m_Text_HDRStarMaps[0][1]);

  // Clear the star texture
  dv->ColorFill(pSurfStar0, NULL, D3DCOLOR_ARGB(0, 0, 0, 0));
  SAFE_RELEASE(pSurfStar0);
  if (pSurfStar1)
  {
    dv->ColorFill(pSurfStar1, NULL, D3DCOLOR_ARGB(0, 0, 0, 0));
    SAFE_RELEASE(pSurfStar1);
  }

  // Avoid rendering the star if it's not being used in the current glare
  if(g_GlareDef.m_fGlareLuminance <= 0.0f || g_GlareDef.m_fStarLuminance <= 0.0f)
    return true;

  // Initialize the constants used during the effect
  const CStarDef& starDef = g_GlareDef.m_starDef;
  const float fTanFoV = atanf(D3DX_PI/8) ;
  const D3DXVECTOR4 vWhite( 1.0f, 1.0f, 1.0f, 1.0f );
  static const s_maxPasses = 3 ;
  static const int nSamples = 8 ;
  static D3DXVECTOR4 s_aaColor[s_maxPasses][8] ;
  static const D3DXCOLOR s_colorWhite(0.63f, 0.63f, 0.63f, 0.0f) ;

  rd->EF_SetState(GS_NODEPTHTEST);

  PDIRECT3DSURFACE9 pSurfDest[2] = {NULL, NULL};

  // Set aside all the star texture surfaces as a convenience
  PDIRECT3DSURFACE9 apSurfStar[NUM_HDR_STAR_TEXTURES][2] = {0};
  for(i=0; i<NUM_HDR_STAR_TEXTURES; i++)
  {
    apSurfStar[i][0] = GetSurfaceTP(tm->m_Text_HDRStarMaps[i][0]);
    if(!apSurfStar[i][0])
      goto LCleanReturn;
    if (rd->m_bDeviceSupportsMRT)
    {
      apSurfStar[i][1] = GetSurfaceTP(tm->m_Text_HDRStarMaps[i][1]);
      if(!apSurfStar[i][1])
        goto LCleanReturn;
    }
  }

  float srcW;
  srcW = (FLOAT)tm->m_Text_HDRStarSource->m_Width;
  float srcH;
  srcH= (FLOAT)tm->m_Text_HDRStarSource->m_Height;

  for (p=0; p<s_maxPasses; p++)
  {
    float ratio;
    ratio = (float)(p + 1) / (float)s_maxPasses;

    for (s=0; s<nSamples; s++)
    {
      D3DXCOLOR chromaticAberrColor ;
      D3DXColorLerp(&chromaticAberrColor, &( CStarDef::GetChromaticAberrationColor(s) ), &s_colorWhite, ratio);
      D3DXColorLerp( (D3DXCOLOR*)&(s_aaColor[p][s]), &s_colorWhite, &chromaticAberrColor, g_GlareDef.m_fChromaticAberration);
    }
  }

  float radOffset;
  radOffset = g_GlareDef.m_fStarInclination + starDef.m_fInclination ;

  STexPic *pTexSource[2];

  // Direction loop
  for (d=0; d<starDef.m_nStarLines; d++)
  {
    CONST STARLINE& starLine = starDef.m_pStarLine[d];

    pTexSource[0] = tm->m_Text_HDRStarSource;
    pTexSource[1] = NULL;

    float rad;
    rad = radOffset + starLine.fInclination;
    float sn, cs;
    sn = sinf(rad), cs = cosf(rad);
    D3DXVECTOR2 vtStepUV;
    vtStepUV.x = sn / srcW * starLine.fSampleLength;
    vtStepUV.y = cs / srcH * starLine.fSampleLength;

    float attnPowScale;
    attnPowScale = (fTanFoV + 0.1f) * 1.0f * (160.0f + 120.0f) / (srcW + srcH) * 1.2f;

    // 1 direction expansion loop
    rd->EF_SetState(GS_NODEPTHTEST);

    int iWorkTexture;
    iWorkTexture = 1 ;
    for (p=0; p<starLine.nPasses; p++)
    {
      if (p == starLine.nPasses-1)
      {
        // Last pass move to other work buffer
        pSurfDest[0] = apSurfStar[d+4][0];
        pSurfDest[1] = apSurfStar[d+4][1];
      }
      else
      {
        pSurfDest[0] = apSurfStar[iWorkTexture][0];
        pSurfDest[1] = apSurfStar[iWorkTexture][1];
      }

      // Sampling configration for each stage
      for (i=0; i<nSamples; i++)
      {
        float lum;
        lum = powf(starLine.fAttenuation, attnPowScale * i);

        avSampleWeights[i] = s_aaColor[starLine.nPasses - 1 - p][i] * lum * (p+1.0f) * 0.5f;

        // Offset of sampling coordinate
        avSampleOffsets[i].x = vtStepUV.x * i;
        avSampleOffsets[i].y = vtStepUV.y * i;
        avSampleOffsets[i].z = 0;
        avSampleOffsets[i].w = 1;
        if (fabs(avSampleOffsets[i].x) >= 0.9f || fabs(avSampleOffsets[i].y) >= 0.9f)
        {
          avSampleOffsets[i].x = 0.0f;
          avSampleOffsets[i].y = 0.0f;
          avSampleWeights[i] *= 0.0f;
        }
      }
      D3DSURFACE_DESC srcDesc;
      LPDIRECT3DTEXTURE9 pTex = (LPDIRECT3DTEXTURE9)pTexSource[0]->m_RefTex.m_VidTex;
      pTex->GetLevelDesc(0, &srcDesc);
      int nBitPlanes = 1;
      BOOL bSplitOutput = FALSE;
      if (rd->m_bDeviceSupportsMRT && srcDesc.Format==D3DFMT_G16R16F)
      {
        fpStar = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRStar, "CGRC_HDR_Star_PS20");
        nBitPlanes = 2;
        bSplitOutput = FALSE;
      }
      else
      if (rd->m_bDeviceSupportsMRT && (srcDesc.Format==D3DFMT_A8R8G8B8))
      {
        fpStar = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRStar_MRT, "CGRC_HDR_Star_MRT_PS20");
        nBitPlanes = 1;
        bSplitOutput = TRUE;
      }
      else
      {
        fpStar = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRStar, "CGRC_HDR_Star_PS20");
        nBitPlanes = 1;
        bSplitOutput = FALSE;
      }

      if (!fpStar)
        goto LCleanReturn;
      fpStar->mfSet(true, 0);

      bind = fpStar->mfGetParameterBind("vSampleOffsets");
      assert(bind);
      fpStar->mfParameter(bind, &avSampleOffsets[0].x, nSamples);

      for (int iBitPlane=0; iBitPlane<nBitPlanes; iBitPlane++)
      {
        //  mux weights for multi-planar outputs
        for ( int s=0; s<nSamples; s++)
        {
          D3DXVECTOR4 weight = avSampleWeights[s];
          if ( iBitPlane&1 )
          {
            avSampleWeights[s].x = weight.z;
            avSampleWeights[s].y = weight.w;
            avSampleWeights[s].z = weight.x;
            avSampleWeights[s].w = weight.y;
          }
        }
        bind = fpStar->mfGetParameterBind("vSampleWeights");
        assert(bind);
        fpStar->mfParameter(bind, &avSampleWeights[0].x, nSamples);
     
        dv->SetRenderTarget( 0, pSurfDest[iBitPlane] );
        SetTexture(rd, pTexSource[iBitPlane], 0, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);

        if (bSplitOutput)
          dv->SetRenderTarget(1, pSurfDest[1]);

        // Draw a fullscreen quad to sample the RT
        DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);
      }

      if (bSplitOutput)
        dv->SetRenderTarget(1, NULL);

      // Setup next expansion
      vtStepUV *= nSamples ;
      attnPowScale *= nSamples ;

      // Set the work drawn just before to next texture source.
      pTexSource[0] = tm->m_Text_HDRStarMaps[iWorkTexture][0];
      pTexSource[1] = tm->m_Text_HDRStarMaps[iWorkTexture][1];

      iWorkTexture += 1;
      if (iWorkTexture > 2)
        iWorkTexture = 1;
    }
  }

  pSurfDest[0] = apSurfStar[0][0];
  pSurfDest[1] = apSurfStar[0][1];

  int nBitPlanes = (rd->m_bDeviceSupportsMRT) ? 2 : 1;

  for (int iBitPlane=0; iBitPlane<nBitPlanes; iBitPlane++)
  {
    for(i=0; i<starDef.m_nStarLines; i++)
    {
      SetTexture(rd, tm->m_Text_HDRStarMaps[i+4][iBitPlane], i, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);

      avSampleWeights[i] = vWhite * 1.0f / (FLOAT) starDef.m_nStarLines;
    }
  
    CHAR strTechnique[256];
    _snprintf(strTechnique, 256, "CGRC_HDR_MergeTextures_%d_PS20", starDef.m_nStarLines);
    strTechnique[255] = 0;
    fpMergeTextures = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRMergeTextures[starDef.m_nStarLines], strTechnique);
    if (!fpMergeTextures)
      goto LCleanReturn;
    fpMergeTextures->mfSet(true, 0);

    bind = fpMergeTextures->mfGetParameterBind("vSampleWeights");
    assert(bind);
    fpMergeTextures->mfParameter(bind, &avSampleWeights[0].x, starDef.m_nStarLines);

    dv->SetRenderTarget(0, pSurfDest[iBitPlane]);

    // Draw a fullscreen quad to sample the RT
    DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);
  }

  bResult = true;

LCleanReturn:
  for (int iPlane=0; iPlane<2; iPlane++)
  {
    for(i=0; i<NUM_HDR_STAR_TEXTURES; i++)
    {
      SAFE_RELEASE(apSurfStar[i][iPlane]);
    }
  }
  rd->EF_SelectTMU(0);
  if (fpMergeTextures)
    fpMergeTextures->mfSet(false, 0);

  return bResult;
}

bool HDR_RenderFinalScene()
{
  HRESULT hr = S_OK;
  CD3D9Renderer *rd = gcpRendD3D;
  LPDIRECT3DDEVICE9 dv = rd->m_pd3dDevice;
  CD3D9TexMan *tm = (CD3D9TexMan *)rd->m_TexMan;
  CCGPShader_D3D *fpFinalScene = NULL;
  bool bResult = false;

  // Draw the high dynamic range scene texture to the low dynamic range
  // back buffer. As part of this final pass, the scene will be tone-mapped
  // using the user's current adapted luminance, blue shift will occur
  // if the scene is determined to be very dark, and the post-process lighting
  // effect textures will be added to the scene.
  if (rd->m_bDeviceSupportsMRT)
    fpFinalScene = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRFinalScene, "CGRC_HDR_FinalScene_BlueShift_ToneMap_FromMRT_PS20");
  else
    fpFinalScene = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDRFinalScene, "CGRC_HDR_FinalScene_BlueShift_ToneMap_PS20");
  if (!fpFinalScene)
    return false;
  fpFinalScene->mfSet(true, 0);

  float fKeyValue = CRenderer::CV_r_hdrlevel;
  float fBloomScale = 1.0f;
  float fStarScale = 0.5f;

  float v[4];
  v[0] = fStarScale;
  v[1] = fBloomScale;
  v[2] = 0;
  v[3] = fKeyValue;

  fpFinalScene->mfParameter4f("Params", v);

  dv->SetRenderTarget(0, rd->mfGetBackSurface());
  if (rd->m_bDeviceSupportsMRT)
  {
    SetTexture(rd, tm->m_Text_HDRTarget, 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
    SetTexture(rd, tm->m_Text_HDRBloomMaps[0], 1, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);
    SetTexture(rd, tm->m_Text_HDRStarMaps[0][0], 2, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);
    SetTexture(rd, tm->m_Text_HDRStarMaps[0][1], 3, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);
    SetTexture(rd, tm->m_Text_HDRAdaptedLuminanceCur, 4, D3DTEXF_POINT, D3DTEXF_POINT, true);
  }
  else
  {
    if (rd->m_nHDRType == 2)
      SetTexture(rd, tm->m_Text_HDRTarget_Temp, 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
    else
      SetTexture(rd, tm->m_Text_HDRTarget, 0, D3DTEXF_POINT, D3DTEXF_POINT, true);
    SetTexture(rd, tm->m_Text_HDRBloomMaps[0], 1, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);
    SetTexture(rd, tm->m_Text_HDRStarMaps[0][0], 2, D3DTEXF_LINEAR, D3DTEXF_LINEAR, true);
    SetTexture(rd, tm->m_Text_HDRAdaptedLuminanceCur, 3, D3DTEXF_POINT, D3DTEXF_POINT, true);
  }

  DrawFullScreenQuad( 0.0f, 0.0f, 1.0f, 1.0f );

  fpFinalScene->mfSet(false, 0);
  rd->EF_SelectTMU(0);

  return true;
}

void HDR_DrawDebug()
{
#ifdef USE_HDR
  if (!CRenderer::CV_r_hdrrendering)
    return;
#endif

  CD3D9Renderer *rd = gcpRendD3D;

  rd->EF_SetState(GS_NODEPTHTEST);
  int iTmpX, iTmpY, iTempWidth, iTempHeight;
  rd->GetViewport(&iTmpX, &iTmpY, &iTempWidth, &iTempHeight);   
  rd->EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  rd->Set2DMode(true, 1, 1);
  rd->m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
  rd->m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
  CCGPShader_D3D *fpShowR = NULL;
  CCGPShader_D3D *fpShowMRT = NULL;
  fpShowR = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDR_ShowR, "CGRC_HDR_ShowR_PS20");
  fpShowMRT = (CCGPShader_D3D *)PShaderForName(rd->m_RP.m_PS_HDR_ShowRG_MRT, "CGRC_HDR_ShowRG_MRT_PS20");

  rd->SetViewport(10, 400, 100, 100);   
  if (rd->m_nHDRType == 2)
    rd->DrawImage(0, 0, 1, 1, rd->m_TexMan->m_Text_HDRTarget_Temp->m_Bind, 0, 1, 1, 0, 1,1,1,1);
  else
    rd->DrawImage(0, 0, 1, 1, rd->m_TexMan->m_Text_HDRTarget->m_Bind, 0, 1, 1, 0, 1,1,1,1);

  rd->SetViewport(120, 400, 100, 100);   
  if (rd->m_bDeviceSupportsMRT && fpShowMRT)
  {
    fpShowMRT->mfSet(true, 0);
    rd->EF_SelectTMU(1);
    rd->m_TexMan->m_Text_HDRTargetScaled[1]->Set();
    rd->EF_SelectTMU(0);
    rd->DrawImage(0, 0, 1, 1, rd->m_TexMan->m_Text_HDRTargetScaled[0]->m_Bind, 0, 1, 1, 0, 1,1,1,1);
    rd->EF_SelectTMU(1);
    rd->SetTexture(0);
    rd->EF_SelectTMU(0);
    fpShowMRT->mfSet(false, 0);
  }
  else
    rd->DrawImage(0, 0, 1, 1, rd->m_TexMan->m_Text_HDRTargetScaled[0]->m_Bind, 0, 1, 1, 0, 1,1,1,1);

  if (fpShowR)
  {
    fpShowR->mfSet(true, 0);

    rd->SetViewport(10, 510, 100, 100);   
    rd->DrawImage(0, 0, 1, 1, rd->m_TexMan->m_Text_HDRToneMaps[0]->m_Bind, 0, 1, 1, 0, 1,1,1,1);

    rd->SetViewport(120, 510, 100, 100);   
    rd->DrawImage(0, 0, 1, 1, rd->m_TexMan->m_Text_HDRAdaptedLuminanceCur->m_Bind, 0, 1, 1, 0, 1,1,1,1);

    fpShowR->mfSet(false, 0);
  }

  rd->SetViewport(230, 400, 100, 100);   
  rd->DrawImage(0, 0, 1, 1, rd->m_TexMan->m_Text_HDRBrightPass->m_Bind, 0, 1, 1, 0, 1,1,1,1);

  rd->SetViewport(340, 400, 100, 100);   
  rd->DrawImage(0, 0, 1, 1, rd->m_TexMan->m_Text_HDRStarSource->m_Bind, 0, 1, 1, 0, 1,1,1,1);

  rd->SetViewport(450, 400, 100, 100);   
  rd->DrawImage(0, 0, 1, 1, rd->m_TexMan->m_Text_HDRBloomSource->m_Bind, 0, 1, 1, 0, 1,1,1,1);

  rd->SetViewport(560, 400, 100, 100);   
  rd->DrawImage(0, 0, 1, 1, rd->m_TexMan->m_Text_HDRBloomMaps[0]->m_Bind, 0, 1, 1, 0, 1,1,1,1);

  rd->SetViewport(670, 400, 100, 100);   
  if (rd->m_bDeviceSupportsMRT && fpShowMRT)
  {
    fpShowMRT->mfSet(true, 0);
    rd->EF_SelectTMU(1);
    rd->m_TexMan->m_Text_HDRStarMaps[0][1]->Set();
    rd->EF_SelectTMU(0);
    rd->DrawImage(0, 0, 1, 1, rd->m_TexMan->m_Text_HDRStarMaps[0][0]->m_Bind, 0, 1, 1, 0, 1,1,1,1);
    rd->EF_SelectTMU(1);
    rd->SetTexture(0);
    rd->EF_SelectTMU(0);
    fpShowMRT->mfSet(false, 0);
  }
  else
    rd->DrawImage(0, 0, 1, 1, rd->m_TexMan->m_Text_HDRStarMaps[0][0]->m_Bind, 0, 1, 1, 0, 1,1,1,1);

  //rd->SetViewport(300, 210, 100, 100);   
  //rd->DrawImage(0, 0, 1, 1, rd->m_TexMan->m_Text_ScreenMap_HDR->m_Bind, 0, 1, 1, 0, 1,1,1,1);

  rd->Set2DMode(false, 1, 1);
  rd->SetViewport(iTmpX, iTmpY, iTempWidth, iTempHeight);   
}

void CD3D9Renderer::EF_HDRPostProcessing()
{
  static EGLARELIBTYPE seGlareType = (EGLARELIBTYPE)-1;

  PROFILE_FRAME(Draw_HDR_PostProcessing);

#ifdef USE_HDR
  if ((int)seGlareType != CV_r_hdrrendering)
  {
    if (CV_r_hdrrendering >= NUM_GLARELIBTYPES)
    {
      ICVar *var = iConsole->GetCVar("r_HDRRendering");
      if (var)
        var->Set(NUM_GLARELIBTYPES-1);
    }
    else
    if (CV_r_hdrrendering < 0)
    {
      ICVar *var = iConsole->GetCVar("r_HDRRendering");
      if (var)
        var->Set(1);
    }
    seGlareType = (EGLARELIBTYPE)CV_r_hdrrendering;
    g_GlareDef.Initialize(seGlareType);
  }
#endif
  CD3D9TexMan *tm = (CD3D9TexMan *)m_TexMan;
  if (!tm->m_Text_HDRTarget)
    return;
  if (tm->m_Text_HDRTarget->m_Width != m_width || tm->m_Text_HDRTarget->m_Height != m_height)
    tm->GenerateHDRMaps();

	if( tm->m_HDR_RT_FSAA )
	{
		STexPic *tp = tm->m_Text_HDRTarget;
		LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)tp->m_RefTex.m_VidTex;
		assert(pTexture);
		LPDIRECT3DSURFACE9 pSurf;
		pTexture->GetSurfaceLevel(0, &pSurf);
		assert(pSurf);

		HRESULT hr = m_pd3dDevice->StretchRect( tm->m_HDR_RT_FSAA, 0, pSurf, 0, D3DTEXF_NONE );
		pSurf->Release();
	}

  bool bMeasureLuminance = true;

  EF_SetState(GS_NODEPTHTEST);
  D3DSetCull(eCULL_None);
  CD3D9TexMan::BindNULL(1);
  EF_SelectTMU(0);
  m_RP.m_PersFlags &= ~(RBPF_VSNEEDSET | RBPF_PS1NEEDSET);
  m_RP.m_FlagsModificators = 0;
  m_RP.m_CurrentVLights = 0;
  m_RP.m_FlagsPerFlush = 0;
  EF_CommitShadersState();
  EF_CommitVLightsState();
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
  if (m_RP.m_TexStages[0].TCIndex != 0)
  {
    m_RP.m_TexStages[0].TCIndex = 0;
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0);
  }
  EF_Scissor(false, 0, 0, 0, 0);
  m_RP.m_pShader = NULL;

  if (m_nHDRType == 2)
    m_pd3dDevice->SetRenderTarget(1, NULL);

  m_pCurBackBuffer = m_pBackBuffer;

  // Create a scaled copy of the scene
  HDR_SceneToSceneScaled();

  // Setup tone mapping technique
  if (bMeasureLuminance)
    HDR_MeasureLuminance();

  // Calculate the current luminance adaptation level
  HDR_CalculateAdaptation();

  // Now that luminance information has been gathered, the scene can be bright-pass filtered
  // to remove everything except bright lights and reflections.
  HDR_SceneScaledToBrightPass();

  // Blur the bright-pass filtered image to create the source texture for the star effect
  HDR_BrightPassToStarSource();

  // Scale-down the source texture for the star effect to create the source texture
  // for the bloom effect
  HDR_StarSourceToBloomSource();

  // Render post-process lighting effects
  HDR_RenderBloom();
  HDR_RenderStar();

  // Render final scene to the back buffer
  HDR_RenderFinalScene();

  SAFE_RELEASE(m_pHDRTargetSurf);
  SAFE_RELEASE(m_pHDRTargetSurf_K);
  m_RP.m_PersFlags &= ~RBPF_HDR;

  ResetToDefault();
}

