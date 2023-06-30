/*=============================================================================
  IShader.h : Shaders common interface.
  Copyright (c) 2001-2002 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honich Andrey

=============================================================================*/

#ifndef _ISHADER_H_
#define _ISHADER_H_

#include "TString.h"
#include "TArrays.h"
#include "smartptr.h"

#include "Cry_XOptimise.h"
#ifdef LINUX
#	include <platform.h>
#endif

struct IShader;
struct SShader;
class CMaterial;
struct SShaderTexUnit;
struct STexAnim;
struct SShaderPass;
struct CLeafBuffer;
struct SShaderItem;
struct STexPic;
struct SParam;
struct ICryCharInstance;
struct IDeformableRenderMesh;
class CPShader;
template	<class T> class list2;

union UCol
{
  DWORD dcolor;
  bvec4 bcolor;
};


//================================================================
// HW Culling type
enum ECull
{
  eCULL_Back = 0,
  eCULL_Front,
  eCULL_None
};

//=========================================================================
// Array Pointers for HW Shaders

enum ESrcPointer
{
  eSrcPointer_Unknown,
  eSrcPointer_Vert,
  eSrcPointer_Color,
  eSrcPointer_SecColor,
  eSrcPointer_Tex,
  eSrcPointer_TexLM,
  eSrcPointer_Normal,
  eSrcPointer_Binormal,
  eSrcPointer_Tangent,
  eSrcPointer_TNormal,
  eSrcPointer_LightVector,
  eSrcPointer_LightVector_Terrain,
  eSrcPointer_NormLightVector,
  eSrcPointer_HalfAngleVector,
  eSrcPointer_HalfAngleVector_Terrain,
  eSrcPointer_Attenuation,
  eSrcPointer_LAttenuationSpec0,
  eSrcPointer_LAttenuationSpec1,
  eSrcPointer_LAttenuationSpec0_Terrain,
  eSrcPointer_LAttenuationSpec1_Terrain,
  eSrcPointer_Refract,
  eSrcPointer_Project,
  eSrcPointer_ProjectTexture,
  eSrcPointer_ProjectAttenFromCamera,
  eSrcPointer_Detail,
  eSrcPointer_Max,
};

enum EDstPointer
{
  eDstPointer_Unknown,
  eDstPointer_Vert,
  eDstPointer_Normal,
  eDstPointer_Color,
  eDstPointer_SecColor,
  eDstPointer_FogC,
  eDstPointer_Tex0,
  eDstPointer_Tex1,
  eDstPointer_Tex2,
  eDstPointer_Tex3,
  eDstPointer_Tex4,
  eDstPointer_Tex5,
  eDstPointer_Tex6,
  eDstPointer_Tex7,
  eDstPointer_Max,
};

class COrthoNormalBasis  
{
public:
  COrthoNormalBasis()
  {
    m_vForward = Vec3(1,0,0);
    m_vUp = Vec3(0,0,1);
    m_vRight = Vec3(0,1,0);
  }
  COrthoNormalBasis(const COrthoNormalBasis& basis)
  {
    m_vForward = basis.m_vForward;
    m_vUp = basis.m_vUp;
    m_vRight = basis.m_vRight;
  }
  COrthoNormalBasis(const Vec3& vForward, const Vec3& vUp, const Vec3& vRight)
  {
    m_vForward = vForward;
    m_vUp = vUp;
    m_vRight = vRight;
  }

  ~COrthoNormalBasis()
  {
  }
  COrthoNormalBasis& operator=(const COrthoNormalBasis& basis)
  {
    m_vForward = basis.m_vForward;
    m_vUp = basis.m_vUp;
    m_vRight = basis.m_vRight;
    return *this;
  };

  Matrix44 matrixBasisToXYZ() const
  {
    Matrix44 mat;
    mat.SetIdentity();

    mat(0,0) = m_vForward.x;   mat(0,1) = m_vForward.y;   mat(0,2) = m_vForward.z; 
    mat(1,0) = m_vUp.x;        mat(1,1) = m_vUp.y;        mat(1,2) = m_vUp.z; 
    mat(2,0) = m_vRight.x;     mat(2,1) = m_vRight.y;     mat(2,2) = m_vRight.z; 

    return mat;
  }
  Matrix44 matrixXYZToBasis() const
  {
    Matrix44 mat;
    mat.SetIdentity();

    mat(0,0)	= m_vForward.x;		mat(0,1) = m_vUp.x;				mat(0,2) = m_vRight.x; 
    mat(1,0)	= m_vForward.y;		mat(1,1) = m_vUp.y;				mat(1,2) = m_vRight.y; 
    mat(2,0)	= m_vForward.z;		mat(2,1) = m_vUp.z;				mat(2,2) = m_vRight.z; 

    return mat;
  }
  Matrix44 matrixBasisToDestBasis(const COrthoNormalBasis& dest) const
  {
    return (dest.matrixXYZToBasis() * this->matrixBasisToXYZ());
  }

  COrthoNormalBasis& rotate(const Vec3& axis, const float degrees)
  {
    //Vec3 a = axis;
    //a.Normalize();
    Matrix33 m;
    //m = m.GetRotation(a, degrees);
		m.SetRotationAA(DEG2RAD(degrees),GetNormalized(axis));
    m_vForward = m * m_vForward;
    m_vRight = m * m_vRight;
    m_vUp = m * m_vUp;

    return *this;
  };

  Vec3 m_vForward; //tangent
  Vec3 m_vUp;      //binormal
  Vec3 m_vRight;   //normal
};


//////////////////////////////////////////////////////////////////////
// CCObject::m_ObjFlags: Flags used by shader pipeline
#define FOB_SORTPOLYS 1
#define FOB_SELECTED  2
#define FOB_NEAREST   4
#define FOB_PORTAL    8

#define FOB_CUBE_POSX 0x10
#define FOB_CUBE_NEGX 0x20
#define FOB_CUBE_POSY 0x40
#define FOB_CUBE_NEGY 0x80
#define FOB_CUBE_POSZ 0x100
#define FOB_CUBE_NEGZ 0x200
#define FOB_TEXTURE   0x400
#define FOB_CUBE_MASK 0x7f0
#define FOB_CUBE_SHIFT 4
#define FOB_IGNOREMATERIALAMBIENT 0x800

#define FOB_DRSUN         0x1000
#define FOB_REFRACTED     0x2000

#define FOB_ZPASS         0x4000
#define FOB_LIGHTPASS     0x8000
#define FOB_FOGPASS       0x10000
#define FOB_RENDER_INTO_SHADOWMAP 0x20000
#define FOB_SPECULAR      0x40000
#define FOB_IGNOREREPOINTER 0x80000
#define FOB_ENVLIGHTING   0x100000
#define FOB_BENDED        0x200000
#define FOB_NOBUMP        0x400000
#define FOB_HASALPHA      0x800000
#define FOB_HOTAMBIENT    0x1000000
#define FOB_MASKCONDITIONS (FOB_BENDED | FOB_HASALPHA | FOB_SPECULAR | FOB_NOBUMP | FOB_INSHADOW | FOB_HEATVISION | FOB_HOTAMBIENT | FOB_ENVLIGHTING)
#define FOB_REMOVED       0x2000000
#define FOB_HEATVISION    0x4000000
#define FOB_INSHADOW      0x8000000

#define FOB_TRANS_ROTATE     0x10000000
#define FOB_TRANS_SCALE      0x20000000
#define FOB_TRANS_TRANSLATE  0x40000000
#define FOB_TRANS_MASK (FOB_TRANS_ROTATE | FOB_TRANS_SCALE | FOB_TRANS_TRANSLATE)

#define FOB_NOSCISSOR     0x80000000

struct SWaveForm;
struct SWaveForm2;

typedef TAllocator16<Matrix44> MatrixAllocator16;
typedef TGrowArray<Matrix44, MatrixAllocator16> MatrixArray16;



//=========================================================================

enum EParamType
{
	eType_UNKNOWN,
	eType_BYTE,
	eType_BOOL,
	eType_SHORT,
	eType_INT,
	eType_FLOAT,
	eType_STRING,
	eType_FCOLOR,
	eType_VECTOR
};

union UParamVal
{
	byte m_Byte;
	bool m_Bool;
	short m_Short;
	int m_Int;
	float m_Float;
	char *m_String;
	float m_Color[4];
	float m_Vector[3];
};

struct SShaderParam
{
	// in order to facilitate the memory allocation tracking, we're using here this class;
	// if you don't like it, please write a substitute for all string within the project and use them everywhere
	char m_Name[32];
	EParamType m_Type;
	UParamVal m_Value;
	int m_nMaterial;

	SShaderParam()
	{
		m_nMaterial = -1;
		m_Value.m_Int = 0;
	}
	size_t Size()
	{
		size_t nSize = sizeof(*this);
		nSize += sizeof(m_Name);
		if (m_Type == eType_STRING)
			nSize += strlen (m_Value.m_String) + 1;

		return nSize;
	}
	~SShaderParam()
	{
		if (m_Type == eType_STRING)
			delete [] m_Value.m_String;
	}
	static bool SetParam(const char *name, TArray<SShaderParam> *Params, UParamVal& pr, int nMaterial)
	{
		int i;
		for (i=0; i<Params->Num(); i++)
		{
			SShaderParam *sp = &Params->Get(i);
			if (!sp)
				continue;
			const char *nm = sp->m_Name;
			int n = 0;
			char ch = 0;
			int ind = -1;
			while (true)
			{
				ch = name[n];
				if (!ch)
					break;
				if (ch == '[')
				{
					char dig[16];
					int m = 0;
					n++;
					while(true)
					{
						ch = name[n];
						if (!ch)
							break;
						if (ch == ']')
							break;
						dig[m++] = ch;
						dig[m] = 0;
						n++;
					}
					if (!ch)
						break;
					ind = atoi(dig);
					ch = 0;
					break;
				}
				if (ch != nm[n])
					break;
				n++;
			}
			if (!ch)
			{
				sp->m_nMaterial = nMaterial;
				switch (sp->m_Type)
				{
				case eType_INT:
				case eType_FLOAT:
				case eType_SHORT:
					sp->m_Value.m_Int = pr.m_Int;
					break;

				case eType_VECTOR:
					if (ind >= 0)
						sp->m_Value.m_Vector[ind] = pr.m_Vector[ind];
					else
					{
						sp->m_Value.m_Vector[0] = pr.m_Vector[0];
						sp->m_Value.m_Vector[1] = pr.m_Vector[1];
						sp->m_Value.m_Vector[2] = pr.m_Vector[2];
					}
					break;

				case eType_FCOLOR:
					if (ind >= 0)
						sp->m_Value.m_Color[ind] = pr.m_Color[ind];
					else
					{
						sp->m_Value.m_Color[0] = pr.m_Color[0];
						sp->m_Value.m_Color[1] = pr.m_Color[1];
						sp->m_Value.m_Color[2] = pr.m_Color[2];
						sp->m_Value.m_Color[3] = pr.m_Color[3];
					}
					break;

				case eType_STRING:
					{
						char *str = pr.m_String;
						size_t len = strlen(str)+1;
						sp->m_Value.m_String = new char [len];
						strcpy(sp->m_Value.m_String, str);
					}
					break;
				}
				break;
			}
		}
		if (i == Params->Num())
			return false;
		return true;
	}

	static float GetFloat(const char *name, TArray<SShaderParam> *Params, int nMaterial)
	{
		int i;
		for (i=0; i<Params->Num(); i++)
		{
			SShaderParam *sp = &Params->Get(i);
			if (!sp)
				continue;
			if (sp->m_nMaterial != nMaterial)
				continue;
			const char *nm = sp->m_Name;
			int n = 0;
			char ch = 0;
			int ind = -1;
			while (true)
			{
				ch = name[n];
				if (!ch)
					break;
				if (ch == '[')
				{
					char dig[16];
					int m = 0;
					n++;
					while(true)
					{
						ch = name[n];
						if (!ch)
							break;
						if (ch == ']')
							break;
						dig[m++] = ch;
						dig[m] = 0;
						n++;
					}
					if (!ch)
						break;
					ind = atoi(dig);
					ch = 0;
					break;
				}
				if (tolower(ch) != nm[n])
					break;
				n++;
			}
			if (!ch)
			{
				switch (sp->m_Type)
				{
				case eType_INT:
				case eType_FLOAT:
				case eType_SHORT:
					return sp->m_Value.m_Float;
					break;

				case eType_VECTOR:
					if (ind >= 0)
						return sp->m_Value.m_Vector[ind];
					else
						return -9999;
					break;

				case eType_FCOLOR:
					if (ind >= 0)
						return sp->m_Value.m_Color[ind];
					else
						return -9999;
					break;

				case eType_STRING:
					{
						return -9999;
					}
					break;
				}
			}
		}
		return -9999;
	}
};


//////////////////////////////////////////////////////////////////////
// Objects using in shader pipeline

// Size of CCObject currently is 256 bytes. If you add new members better to ask me before or just
// make sure CCObject is cache line aligned (64 bytes)
class CCObject
{
public:

  CCObject()
  {
    m_ShaderParams = NULL;
    m_bShaderParamCreatedInRenderer = false;
    m_VPMatrixId = -1;
    m_VPMatrixFrame = -1;
  }
  ~CCObject();

  short m_Id;
  short m_Counter;
  short m_VisId;
  short m_VPMatrixId;
  uint m_ObjFlags;
  Matrix44 m_Matrix;
  bool m_bShaderParamCreatedInRenderer;
  TArray<SShaderParam> *m_ShaderParams;
  // Lightmap textures ID's
	short m_nLMId;
	short m_nLMDirId;
  short m_nOcclId;
	short m_nHDRLMId;
  uint m_Reserved1;

  uint m_DynLMMask;
  float m_Trans2[3];
  float m_Angs2[3];
  // Different useful vars (ObjVal component in shaders)
  // [0] - used for blending trees sun-rabbits on distance (0-1)
  float m_TempVars[5]; 
  float m_fDistanceToCam;

  CRendElement *m_RE;
  IShader *m_EF;
  void *m_CustomData;
  uint m_RenderState;

  float m_SortId; // Custom sort value

  short m_InvMatrixId;
  short m_VPMatrixFrame;
  short m_NumCM; // Custom Cube/Texture id (indoor engine soft shadows)
  union
  {
    short m_nLod;
    short m_NumWFX;
    short m_TexId0;
  };
  union
  {
    short m_nTemplId;
    short m_NumWFY;
    short m_TexId1;
  };
  union
  {
    short m_FrameLight;
    bool m_bVisible;
  };
  short m_LightStyle;
  short m_nScissorX1, m_nScissorY1, m_nScissorX2, m_nScissorY2;
  
  union
  {
    float m_fRefract;
    float m_StartTime;
	};
  union
  {
    float m_fBump;
    float m_fLightFadeTime;
  };
  float m_fHeatFactor;  

  list2<struct ShadowMapLightSourceInstance> * m_pShadowCasters; // list of shadow casters 
  struct ShadowMapFrustum * m_pShadowFrustum; // will define projection of shadow from this object

  Vec3d m_AmbColor;
  CFColor m_Color;

  IDeformableRenderMesh * m_pCharInstance;
  struct CLeafBuffer *m_pLMTCBufferO;

  union
  {
	  ITexPic	*m_pLightImage;
    CDLight *m_pLight;
    float m_fBending;
    float m_fLastUpdate;
    byte m_OcclLights[4];
  };

  _inline Vec3 GetTranslation() const
  {
    return m_Matrix.GetTranslationOLD();
  }
  _inline float GetScaleX() const
  {
    return cry_sqrtf(m_Matrix(0,0)*m_Matrix(0,0) + m_Matrix(0,1)*m_Matrix(0,1) + m_Matrix(0,2)*m_Matrix(0,2));
  }
  _inline float GetScaleZ() const
  {
    return cry_sqrtf(m_Matrix(2,0)*m_Matrix(2,0) + m_Matrix(2,1)*m_Matrix(2,1) + m_Matrix(2,2)*m_Matrix(2,2));
  }
  _inline bool IsMergable()
  {
    if (m_Color.a != 1.0f)
      return false;
    if (m_pShadowCasters)
      return false;
    if (m_pCharInstance)
      return false;
    return true;
  }

  static TArray<SWaveForm2> m_Waves;
  static MatrixArray16 m_ObjMatrices;
  void Init();

  void CloneObject(CCObject *srcObj)
  {
    int Id = m_Id;
    int VisId = m_VisId;
    memcpy(this, srcObj, sizeof(*srcObj));
    m_Id = Id;
    m_VisId = VisId;
  }

  Matrix44 &GetMatrix()
  {
    return m_Matrix;
  }

  Matrix44 &GetInvMatrix();
  Matrix44 &GetVPMatrix();

  void SetScissor();
  void SetAlphaState(CPShader *pPS, int nCurState);

  virtual void AddWaves(SWaveForm2 **wf);
  virtual void SetShaderFloat(const char *Name, float Val);
  virtual void RemovePermanent();

#ifdef _RENDERER
#ifdef DEBUGALLOC
#undef new
#endif
	// Sergiy: it's enough to allocate 16 bytes more, even on 64-bit machine
	// - and we need to store only the offset, not the actual pointer
  void* operator new( size_t Size )
  {
    byte *ptr = (byte *)malloc(Size+16+4);
    memset(ptr, 0, Size+16+4);
    byte *bPtrRes = (byte *)((INT_PTR)(ptr+4+16) & ~0xf);
    ((byte**)bPtrRes)[-1] = ptr;

    return bPtrRes;
  }
  void* operator new[](size_t Size)
  {
    byte *ptr = (byte *)malloc(Size+16+2*sizeof(INT_PTR));
    memset(ptr, 0, Size+16+2*sizeof(INT_PTR));
    byte *bPtrRes = (byte *)((INT_PTR)(ptr+16+2*sizeof(INT_PTR)) & ~0xf);
    ((byte**)bPtrRes)[-2] = ptr;

    return bPtrRes-sizeof(INT_PTR);
  }
  void operator delete( void *Ptr )
  {
    byte *bActualPtr = ((byte **)Ptr)[-1];
		assert (bActualPtr <= (byte*)Ptr && (byte*)Ptr-bActualPtr < 20);
    free ((void *)bActualPtr);
  }

  void operator delete[]( void *Ptr )
  {
		byte *bActualPtr = ((byte **)Ptr)[-1];
		free ((void *)bActualPtr);
  }
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
#endif // _RENDERER
};


//=================================================================================

// Materials
#define LMF_IGNORELIGHTS 1
#define LMF_BUMPMATERIAL 2
#define LMF_NOAMBIENT    4
#define LMF_COLMAT_AMB   8
#define LMF_POLYOFFSET   0x10
#define LMF_NOALPHA      0x20
#define LMF_IGNOREPROJLIGHTS 0x40
#define LMF_NOBUMP       0x80
#define LMF_DISABLE      0x100
#define LMF_NOSPECULAR   0x200
#define LMF_NOADDSPECULAR 0x400
#define LMF_HASPSHADER   0x800
#define LMF_DIVIDEAMB4   0x1000
#define LMF_DIVIDEAMB2   0x2000
#define LMF_DIVIDEDIFF4  0x4000
#define LMF_DIVIDEDIFF2  0x8000

#define LMF_LIGHT0       0x10000
#define LMF_LIGHT1       0x20000
#define LMF_LIGHT2       0x30000
#define LMF_LIGHT3       0x40000
#define LMF_LIGHT4       0x50000
#define LMF_LIGHT5       0x60000
#define LMF_LIGHT6       0x70000
#define LMF_LIGHT7       0x80000

#define LMF_LIGHT_MASK   0xf0000

#define LMF_1SAMPLES     0x100000
#define LMF_2SAMPLES     0x200000
#define LMF_3SAMPLES     0x400000
#define LMF_4SAMPLES     0x800000
#define LMF_SAMPLES      0xf00000

#define LMF_LIGHT_SHIFT  16

#define LMF_ONLYMATERIALAMBIENT 0x1000000
#define LMF_USEOCCLUSIONMAP     0x2000000
#define LMF_HASAMBIENT          0x4000000
#define LMF_HASDOT3LM           0x8000000

struct SSideMaterial
{
  SSideMaterial::SSideMaterial() 
    : m_Ambient(1.0f, 1.0f, 1.0f, 1.0f),
    m_Diffuse(1.0f, 1.0f, 1.0f, 1.0f),
    m_Specular(1.0f, 1.0f, 1.0, 1.0f),
    m_Emission(0.0f, 0.0f, 0.0f, 1.0f),
    m_SpecShininess(10.0f) {}
  
  CFColor m_Ambient;
  CFColor m_Diffuse;
  CFColor m_Specular;
  CFColor m_Emission;
  float m_SpecShininess;

  _inline friend bool operator ==(const SSideMaterial &m1, const SSideMaterial &m2)
  {
    if (m1.m_Ambient == m2.m_Ambient && m1.m_Diffuse == m2.m_Diffuse && m1.m_Specular == m2.m_Specular && m1.m_Emission == m2.m_Emission && m1.m_SpecShininess == m2.m_SpecShininess)
      return true;
    return false;
  }
};

struct SLightMaterial
{
  SLightMaterial()
  {
    name[0] = 0;
    bNeverReplace = false;
    m_nRefCounter = 0;
  }

  int Id;
  char name[64];

  enum Side {FRONT, BACK, BOTH} side;

  int m_nRefCounter;
  SSideMaterial Front;
  SSideMaterial Back;
  bool bNeverReplace;

  int Size()
  {
    int nSize = sizeof(SLightMaterial);
    return nSize;
  }
  void mfApply(int Flags);
  void Release();

  static SLightMaterial* mfAdd(char *name, SLightMaterial *Compare=NULL);
  static SLightMaterial* mfGet(char *name);

  static SLightMaterial* current_material;
  static int m_ObjFrame;

  static TArray<SLightMaterial *> known_materials;
};


//=========================================================================================

// Wave form evaluators
enum EWaveForm
{
  eWF_None,
  eWF_Sin,
  eWF_HalfSin,
  eWF_InvHalfSin,
  eWF_Square,
  eWF_Triangle,
  eWF_SawTooth,
  eWF_InvSawTooth,
  eWF_Hill,
  eWF_InvHill,
};

#define WFF_CLAMP 1
#define WFF_LERP  2

// Wave form definition
struct SWaveForm
{
  EWaveForm m_eWFType;
  byte m_Flags;

  float m_Level;
  float m_Level1;
  float m_Amp;
  float m_Amp1;
  float m_Phase;
  float m_Phase1;
  float m_Freq;
  float m_Freq1;


  int Size()
  {
    int nSize = sizeof(SWaveForm);
    return nSize;
  }
  SWaveForm()
  {
    memset(this, 0, sizeof(SWaveForm));
  }
  bool operator == (SWaveForm wf)
  {
    if (m_eWFType == wf.m_eWFType && m_Level == wf.m_Level && m_Amp == wf.m_Amp && m_Phase == wf.m_Phase && m_Freq == wf.m_Freq && m_Level1 == wf.m_Level1 && m_Amp1 == wf.m_Amp1 && m_Phase1 == wf.m_Phase1 && m_Freq1 == wf.m_Freq1 && m_Flags == wf.m_Flags)
      return true;
    return false;
  }

  SWaveForm& operator += (const SWaveForm& wf )
  {
    m_Level  += wf.m_Level;
    m_Level1 += wf.m_Level1;
    m_Amp  += wf.m_Amp;
    m_Amp1 += wf.m_Amp1;
    m_Phase  += wf.m_Phase;
    m_Phase1 += wf.m_Phase1;
    m_Freq  += wf.m_Freq;
    m_Freq1 += wf.m_Freq1;
    return *this;
  }
};

struct SWaveForm2
{
  EWaveForm m_eWFType;

  float m_Level;
  float m_Amp;
  float m_Phase;
  float m_Freq;

  SWaveForm2()
  {
    memset(this, 0, sizeof(SWaveForm2));
  }
  bool operator == (SWaveForm2 wf)
  {
    if (m_eWFType == wf.m_eWFType && m_Level == wf.m_Level && m_Amp == wf.m_Amp && m_Phase == wf.m_Phase && m_Freq == wf.m_Freq)
      return true;
    return false;
  }

  SWaveForm2& operator += (const SWaveForm2& wf )
  {
    m_Level  += wf.m_Level;
    m_Amp  += wf.m_Amp;
    m_Phase  += wf.m_Phase;
    m_Freq  += wf.m_Freq;
    return *this;
  }
};


//======================================================
// Texture coords generating types
enum EGenTC
{
  eGTC_NoFill = 0,
  eGTC_None,
  eGTC_LightMap,
  eGTC_Quad,
  eGTC_Base,
  eGTC_Projection,
  eGTC_Environment,
  eGTC_SphereMap,
  eGTC_SphereMapEnvironment,
  eGTC_ShadowMap,
};


// Color operations
enum EColorOp
{
  eCO_NOSET,
  eCO_DISABLE,
  eCO_REPLACE,
  eCO_DECAL,
  eCO_ARG2,
  eCO_MODULATE,
  eCO_MODULATE2X,
  eCO_MODULATE4X,
  eCO_BLENDDIFFUSEALPHA,
  eCO_BLENDTEXTUREALPHA,
  eCO_DETAIL,
  eCO_ADD,
  eCO_ADDSIGNED,
  eCO_ADDSIGNED2X,
  eCO_MULTIPLYADD,
  eCO_BUMPENVMAP,
  eCO_BLEND,
  eCO_MODULATEALPHA_ADDCOLOR,
  eCO_MODULATECOLOR_ADDALPHA,
  eCO_MODULATEINVALPHA_ADDCOLOR,
  eCO_MODULATEINVCOLOR_ADDALPHA,
  eCO_DOTPRODUCT3,
  eCO_LERP,
  eCO_SUBTRACT,
};

enum EColorArg
{
  eCA_Specular,
  eCA_Texture,
  eCA_Diffuse,
  eCA_Previous,
  eCA_Constant,
};

#define DEF_TEXARG0 (eCA_Texture|(eCA_Diffuse<<3))
#define DEF_TEXARG1 (eCA_Texture|(eCA_Previous<<3))

// Animating Texture sequence definition
struct STexAnim
{
  TArray<STexPic *> m_TexPics;
  int m_Rand;
  int m_NumAnimTexs;
  bool m_bLoop;
  float m_Time;

  int Size()
  {
    int nSize = sizeof(STexAnim);
    nSize += m_TexPics.GetSize() * sizeof(STexPic *);
    return nSize;
  }

  STexAnim()
  {
    m_Rand = 0;
    m_NumAnimTexs = 0;
    m_bLoop = false;
    m_Time = 0.0f;
  }
  ~STexAnim()
  {
    for (int i=0; i<m_TexPics.Num(); i++)
    {
      ITexPic *pTP = (ITexPic *)m_TexPics[i];
      SAFE_RELEASE(pTP);
    }
  }
  STexAnim& operator = (const STexAnim& sl)
  {
    for (int i=0; i<sl.m_TexPics.Num(); i++)
    {
      ITexPic *pTP = (ITexPic *)sl.m_TexPics[i];
      if (pTP)
        pTP->AddRef();
      m_TexPics.AddElem(sl.m_TexPics[i]);
    }
    m_Rand = sl.m_Rand;
    m_NumAnimTexs = sl.m_NumAnimTexs;
    m_bLoop = sl.m_bLoop;
    m_Time = sl.m_Time;

    return *this;
  }
};

struct SGenTC     
{
  int m_Mask;
  bool m_bDependsOnObject;
  virtual ~SGenTC()
  {
  }
  SGenTC()
  {
    m_Mask = 0x3;
    m_bDependsOnObject = false;
  }
  virtual SGenTC *mfCopy() = 0;
  virtual bool mfSet(bool bEnable) = 0;
  virtual void mfCompile(char *params, SShader *ef) = 0;
  virtual int Size() = 0;
};

// Type of the texture
enum ETexType
{
  eTT_Base,
  eTT_Cubemap,
  eTT_AutoCubemap,
  eTT_Bumpmap,
  eTT_DSDTBump,
  eTT_Rectangle,
	eTT_3D
};

#define FFILT_NONE   0
#define FFILT_POINT  1
#define FFILT_LINEAR 2
#define FFILT_ANISOTROPIC 3

#define FTU_OPAQUE 1
#define FTU_CLAMP  2
#define FTU_NOMIPS 4
#define FTU_NOBUMP 8
#define FTU_PROJECTED       0x10
#define FTU_FILTERBILINEAR  0x20
#define FTU_FILTERTRILINEAR 0x40
#define FTU_FILTERNEAREST   0x80
#define FTU_FILTERLINEAR    0x100
#define FTU_BUMPPLANTS      0x1000
#define FTU_NOSCALE         0x2000

// General texture layer
struct SShaderTexUnit     
{
  SShaderTexUnit()
  {
    memset(this, 0, sizeof(SShaderTexUnit));
  }

  void mfFree()
  {
    SAFE_DELETE(m_GTC);
    if (m_AnimInfo)
    {
      SAFE_DELETE(m_AnimInfo);
    }
    else
    if (m_ITexPic)
      m_ITexPic->Release(false);
  }

  ~SShaderTexUnit()
  {
    mfFree();
  }

  union
  {
    STexPic *m_TexPic; 
    ITexPic *m_ITexPic;
  };
  STexAnim *m_AnimInfo;
  SGenTC *m_GTC;

  byte m_eColorOp;
  byte m_eAlphaOp;
  byte m_eColorArg;
  byte m_eAlphaArg;
  byte m_eGenTC;
  byte m_eTexType;
  byte m_eHDRColorOp;
  short m_nFlags;
  float m_fTexFilterLodBias;

  int Size()
  {
    int nSize = sizeof(SShaderTexUnit);
    if (m_AnimInfo)
      nSize += m_AnimInfo->Size();
    if (m_GTC)
      nSize += m_GTC->Size();
    return nSize;
  }
  int GetTexFlags() const
  {
    int Flags = 0;
    if (m_nFlags & FTU_NOMIPS)
      Flags |= FT_NOMIPS;
    if (m_nFlags & FTU_CLAMP)
      Flags |= FT_CLAMP;
    if (m_nFlags & FTU_NOSCALE)
      Flags |= FT_NORESIZE;
    return Flags;
  }
  int GetTexFlags2() const
  {
    int Flags = 0;
    return Flags;
  }

  int mfSetTexture(int nt=-1);
  void mfUpdate(void);
  void mfUpdateAnim(CCObject *obj, int o);

  void mfCopy (SShaderTexUnit *dtl) const
  {
    dtl->mfFree();

    memcpy(dtl, this, sizeof(SShaderTexUnit));
    if (m_AnimInfo)
    {
      dtl->m_AnimInfo = new STexAnim;
      *dtl->m_AnimInfo = *m_AnimInfo;
    }
    if (m_GTC)    
      dtl->m_GTC = m_GTC->mfCopy();
    if (m_ITexPic)
      m_ITexPic->AddRef();
  }
};


enum ETexModRotateType
{
  ETMR_NoChange,
  ETMR_Fixed,
  ETMR_Constant,
  ETMR_Oscillated,
};

enum ETexModMoveType
{
  ETMM_NoChange,
  ETMM_Fixed,
  ETMM_Constant,
  ETMM_Jitter,
  ETMM_Pan,
  ETMM_Stretch,
  ETMM_StretchRepeat,
};

enum ETexGenType
{
  ETG_Stream,
  ETG_World,
  ETG_Camera,
  ETG_WorldEnvMap,
  ETG_CameraEnvMap,
  ETG_NormalMap,
  ETG_SphereMap,
};

#define CASE_TEXMOD(var_name)\
	if(!stricmp(#var_name,szParamName))\
	{\
		var_name = fValue;\
		return true;\
	}\

#define CASE_TEXMODANGLE(var_name)\
	if(!stricmp(#var_name,szParamName))\
{\
	var_name = Degr2Word(fValue);\
	return true;\
}\

#define CASE_TEXMODBYTE(var_name)\
	if(!stricmp(#var_name,szParamName))\
{\
	var_name = (byte)fValue;\
	return true;\
}\

#define CASE_TEXMODBOOL(var_name)\
	if(!stricmp(#var_name,szParamName))\
{\
	var_name = (fValue==1.f);\
	return true;\
}\

struct SEfTexModificator
{
	bool SetMember(const char * szParamName, float fValue)
	{
		CASE_TEXMODBYTE(m_eTGType);
		CASE_TEXMODBYTE(m_eRotType);
		CASE_TEXMODBYTE(m_eUMoveType);
		CASE_TEXMODBYTE(m_eVMoveType);
		CASE_TEXMODBOOL(m_bTexGenProjected);

		CASE_TEXMOD(m_Tiling[0]);
		CASE_TEXMOD(m_Tiling[1]);
		CASE_TEXMOD(m_Tiling[2]);
		CASE_TEXMOD(m_Offs[0]);
		CASE_TEXMOD(m_Offs[1]);
		CASE_TEXMOD(m_Offs[2]);

		CASE_TEXMODANGLE(m_Rot[0]);
		CASE_TEXMODANGLE(m_Rot[1]);
		CASE_TEXMODANGLE(m_Rot[2]);
		CASE_TEXMODANGLE(m_RotOscRate[0]);
		CASE_TEXMODANGLE(m_RotOscRate[1]);
		CASE_TEXMODANGLE(m_RotOscRate[2]);
		CASE_TEXMODANGLE(m_RotOscAmplitude[0]);
		CASE_TEXMODANGLE(m_RotOscAmplitude[1]);
		CASE_TEXMODANGLE(m_RotOscAmplitude[2]);
		CASE_TEXMODANGLE(m_RotOscPhase[0]);
		CASE_TEXMODANGLE(m_RotOscPhase[1]);
		CASE_TEXMODANGLE(m_RotOscPhase[2]);
    CASE_TEXMOD(m_RotOscCenter[0]);
    CASE_TEXMOD(m_RotOscCenter[1]);
    CASE_TEXMOD(m_RotOscCenter[2]);

		CASE_TEXMOD(m_UOscRate);
		CASE_TEXMOD(m_VOscRate);
		CASE_TEXMOD(m_UOscAmplitude);
		CASE_TEXMOD(m_VOscAmplitude);
		CASE_TEXMOD(m_UOscPhase);
		CASE_TEXMOD(m_VOscPhase);

		return false;
	}

  byte m_eTGType;
  byte m_eRotType;
  byte m_eUMoveType;
  byte m_eVMoveType;
  bool m_bTexGenProjected;

  float m_Tiling[3];
  float m_Offs[3];

  ushort m_Rot[3];
  ushort m_RotOscRate[3];
  ushort m_RotOscAmplitude[3];
  ushort m_RotOscPhase[3];
  float m_RotOscCenter[3];

  float m_UOscRate;
  float m_VOscRate;
  float m_UOscAmplitude;
  float m_VOscAmplitude;
  float m_UOscPhase;
  float m_VOscPhase;

  // This members are used only during updating of the matrices
  int m_nFrameUpdated;
  int m_nLastRecursionLevel;
  int m_UpdateFlags;
  Matrix44 m_TexMatrix;
  Matrix44 m_TexGenMatrix;
  float m_LastUTime;
  float m_LastVTime;
  float m_CurrentUJitter;
  float m_CurrentVJitter;

  _inline friend bool operator != (const SEfTexModificator &m1, const SEfTexModificator &m2)
  {
    if (m1.m_eTGType != m2.m_eTGType ||
        m1.m_eRotType != m2.m_eRotType ||
        m1.m_eUMoveType != m2.m_eUMoveType ||
        m1.m_eVMoveType != m2.m_eVMoveType ||
        m1.m_bTexGenProjected != m2.m_bTexGenProjected ||
        m1.m_UOscRate != m2.m_UOscRate ||
        m1.m_VOscRate != m2.m_VOscRate ||
        m1.m_UOscAmplitude != m2.m_UOscAmplitude ||
        m1.m_VOscAmplitude != m2.m_VOscAmplitude ||
        m1.m_UOscPhase != m2.m_UOscPhase ||
        m1.m_VOscPhase != m2.m_VOscPhase)
      return true;
    for (int i=0; i<3; i++)
    {
      if (m1.m_Tiling[i] != m2.m_Tiling[i] ||
          m1.m_Offs[i] != m2.m_Offs[i] ||
          m1.m_Rot[i] != m2.m_Rot[i] ||
          m1.m_RotOscRate[i] != m2.m_RotOscRate[i] ||
          m1.m_RotOscAmplitude[i] != m2.m_RotOscAmplitude[i] ||
          m1.m_RotOscPhase[i] != m2.m_RotOscPhase[i] ||
          m1.m_RotOscCenter[i] != m2.m_RotOscCenter[i])
        return true;
    }
    return false;
  }
};

struct SEfResTexture
{
	// in order to facilitate the memory allocation tracking, we're using here this class;
	// if you don't like it, please write a substitute for all string within the project and use them everywhere
  CryBasicString m_Name;
  byte m_TexFlags;
  byte m_Amount;
  bool m_bUTile;
  bool m_bVTile;

  SEfTexModificator m_TexModificator;
  SShaderTexUnit m_TU;

  void Update(int nTU);

  _inline friend bool operator != (const SEfResTexture &m1, const SEfResTexture &m2)
  {
    if (stricmp(m1.m_Name.c_str(), m2.m_Name.c_str()) != 0 ||
        m1.m_TexFlags != m2.m_TexFlags || 
        m1.m_Amount != m2.m_Amount ||
        m1.m_bUTile != m2.m_bUTile ||
        m1.m_bVTile != m2.m_bVTile ||
        m1.m_TexModificator != m2.m_TexModificator)
      return true;
    return false;
  }

  bool IsNeedTexTransform()
  {
    if (m_TexModificator.m_eRotType != ETMR_NoChange || m_TexModificator.m_eUMoveType != ETMM_NoChange || m_TexModificator.m_eVMoveType != ETMM_NoChange)
      return true;
    return false;
  }
  bool IsNeedTexGen()
  {
    if (m_TexModificator.m_eTGType != ETG_Stream)
      return true;
    return false;
  }
  int Size()
  {
    int nSize = sizeof(SEfResTexture) - sizeof(SShaderTexUnit);
    nSize += m_Name.size();
    nSize += m_TU.Size();

    return nSize;
  }

  ~SEfResTexture()
  {
  }

  void Reset()
  {
    memset(this, 0, sizeof(*this));
    m_bUTile = true;
    m_bVTile = true;
    m_Amount = 100;
    m_TexModificator.m_Tiling[0] = 1.0f;
    m_TexModificator.m_Tiling[1] = 1.0f;
    m_TexModificator.m_Tiling[2] = 1.0f;
    m_TexModificator.m_TexMatrix.SetIdentity();
    m_TexModificator.m_nFrameUpdated = -1;
  }
  SEfResTexture& operator=(const SEfResTexture& src)
  {
    src.m_TU.mfCopy(&m_TU);
    memcpy(&m_TexModificator, &src.m_TexModificator, sizeof(SEfTexModificator));
    m_Amount = src.m_Amount;
    m_TexFlags = src.m_TexFlags;
    m_Name = src.m_Name;
    m_bUTile = src.m_bUTile;
    m_bVTile = src.m_bVTile;

    return *this;
  }

  SEfResTexture()
  {
    Reset();
  }
};

#define EFTT_DIFFUSE           0
#define EFTT_BUMP              1
#define EFTT_BUMP_DIFFUSE      2
#define EFTT_GLOSS             3
#define EFTT_CUBEMAP           4
#define EFTT_PHONG             5
#define EFTT_BUMP_HEIGHT       6
#define EFTT_ATTENUATION2D     7
#define EFTT_SPECULAR          8
#define EFTT_DETAIL_OVERLAY    9
#define EFTT_REFLECTION        10
#define EFTT_SUBSURFACE        11
#define EFTT_ATTENUATION1D     12
#define EFTT_OPACITY           13
#define EFTT_LIGHTMAP          14
#define EFTT_LIGHTMAP_HDR      15
#define EFTT_LIGHTMAP_DIR      16
#define EFTT_OCCLUSION         17
#define EFTT_DECAL_OVERLAY     18
#define EFTT_NORMALMAP         19

#define EFTT_MAX               20

struct SBaseShaderResources
{
  int m_ResFlags;
  float m_Opacity;
  float m_AlphaRef;
  SLightMaterial *m_LMaterial;
  TArray<SShaderParam> m_ShaderParams;
  // in order to facilitate the memory allocation tracking, we're using here this class;
  // if you don't like it, please write a substitute for all string within the project and use them everywhere
  CryBasicString m_TexturePath;

  int Size()
  {
    int nSize = sizeof(SBaseShaderResources) + m_ShaderParams.GetMemoryUsage();
    return nSize;
  }
  SBaseShaderResources& operator=(const SBaseShaderResources& src)
  {
    m_ResFlags = src.m_ResFlags;
    m_Opacity = src.m_Opacity;
    m_AlphaRef = src.m_AlphaRef;
    m_LMaterial = src.m_LMaterial;
    m_ShaderParams.Copy(src.m_ShaderParams);
    return *this;
  }

  SBaseShaderResources()
  {
    m_ResFlags = 0;
    m_Opacity = 1.0f;
    m_AlphaRef = 0;
    m_LMaterial = NULL;
  }

  ~SBaseShaderResources()
  {
    m_ShaderParams.Free();
  }
};

struct SRenderShaderResources : SBaseShaderResources
{
  int m_Id;
  int m_nRefCounter;
  int m_nCheckedTemplates;
  bool m_bNeedNormals;
  SEfResTexture *m_Textures[EFTT_MAX];

  int m_nLastTexture;
  int m_nFrameLoad;
  float m_fMinDistanceLoad;

  void AddTextureMap(int Id)
  {
    assert (Id >=0 && Id < EFTT_MAX);
    m_Textures[Id] = new SEfResTexture;
  }
  int Size()
  {
    int nSize = sizeof(SRenderShaderResources);
    for (int i=0; i<EFTT_MAX; i++)
    {
      if (m_Textures[i])
        nSize += m_Textures[i]->Size();
    }
    return nSize;
  }
  SRenderShaderResources& operator=(const SRenderShaderResources& src)
  {
    SBaseShaderResources::operator = (src);
    int i;
    for (i=0; i<EFTT_MAX; i++)
    {
      if (!src.m_Textures[i])
        continue;
      AddTextureMap(i);
      *src.m_Textures[i] = *m_Textures[i];
    }
    return *this;
  }
  SRenderShaderResources(struct SInputShaderResources *pSrc);

  void PostLoad()
  {
    m_bNeedNormals = false;
    int i;
    m_nLastTexture = 0;
    for (i=0; i<EFTT_MAX; i++)
    {
      if (!m_Textures[i])
        continue;
      m_nLastTexture = i;
      m_Textures[i]->m_TexModificator.m_UpdateFlags = 0;
      if (m_Textures[i]->m_TexModificator.m_eTGType == ETG_WorldEnvMap || m_Textures[i]->m_TexModificator.m_eTGType == ETG_CameraEnvMap)
      {
        m_bNeedNormals = true;
        break;
      }
    }
  }

  bool IsNeedNormals()
  {
    return m_bNeedNormals;
  }

  void Reset()
  {
    for (int i=0; i<EFTT_MAX; i++)
    {
      m_Textures[i] = NULL;
    }
  }
  SRenderShaderResources()
  {
    Reset();
    m_Id = 0;
    m_nLastTexture = 0;
    m_nCheckedTemplates = 0;
    m_bNeedNormals = false;
  }

  ~SRenderShaderResources();
  virtual void Release()
  {
#ifdef NULL_RENDERER
		return;
#endif
    m_nRefCounter--;
    if (!m_nRefCounter)
      delete this;
  }
  void AddRef() { m_nRefCounter++; }
};


struct SInputShaderResources : public SBaseShaderResources
{
  SEfResTexture m_Textures[EFTT_MAX];

  int Size()
  {
    int nSize = SBaseShaderResources::Size() - sizeof(SEfResTexture) * EFTT_MAX;
    nSize += m_TexturePath.size();
    for (int i=0; i<EFTT_MAX; i++)
    {
      nSize += m_Textures[i].Size();
    }
    return nSize;
  }
  SInputShaderResources& operator=(const SInputShaderResources& src)
  {
    SBaseShaderResources::operator = (src);
    m_TexturePath = src.m_TexturePath;
    int i;
    for (i=0; i<EFTT_MAX; i++)
    {
      m_Textures[i] = src.m_Textures[i];
    }
    return *this;
  }

  SInputShaderResources()
  {
    for (int i=0; i<EFTT_MAX; i++)
    {
      m_Textures[i].Reset();
    }
  }
  SInputShaderResources(SRenderShaderResources *pSrc)
  {
    if (pSrc)
    {
      m_TexturePath = pSrc->m_TexturePath;
      m_ResFlags = pSrc->m_ResFlags;
      m_Opacity = pSrc->m_Opacity;
      m_AlphaRef = pSrc->m_AlphaRef;
      m_LMaterial = pSrc->m_LMaterial;
      m_ShaderParams.Copy(pSrc->m_ShaderParams);
    }
    for (int i=0; i<EFTT_MAX; i++)
    {
      if (pSrc && pSrc->m_Textures[i])
      {
        m_Textures[i] = *pSrc->m_Textures[i];
      }
      else
      {
        m_Textures[i].Reset();
      }
    }
  }


  ~SInputShaderResources()
  {
  }
};


// Texture formats
enum ETEX_Format
{
  eTF_Unknown = 0,
  eTF_Index,
  eTF_HSV,
  eTF_0888,
  eTF_8888,  // Usually BGRA
  eTF_RGBA,  // Used only in CGLRenderer::DownLoadToVideoMemory
  eTF_8000,
  eTF_0565,
  eTF_0555,
  eTF_4444,
  eTF_1555,
  eTF_DXT1,
  eTF_DXT3,
  eTF_DXT5,

  // Only for normal maps
  eTF_SIGNED_HILO16,
  eTF_SIGNED_HILO8,
  eTF_SIGNED_RGB8,
  eTF_RGB8,

  // Only for DSDT bump (3 floats)
  eTF_DSDT_MAG,
  eTF_DSDT,

  eTF_V8U8,
  eTF_V16U16,

  eTF_0088,  // Luminance, Alpha
  eTF_DEPTH,
};

//===================================================================================
// Shader gen structure (used for automatic shader script generating)

#define SHGF_HIDDEN 1

struct SShaderGenBit
{
  SShaderGenBit()
  {
    m_Mask = 0;
    m_Flags = 0;
  }
	string m_ParamName;
	string m_ParamProp;
	string m_ParamDesc;
	uint64 m_Mask;
  uint m_Flags;
};

struct SShaderGen
{
	TArray<SShaderGenBit *> m_BitMask;
};

//===========================================================================

//====================================================================
// Template types

#define EFT_DEFAULT      0  // Default shader as it is
#define EFT_DECAL        1  // Only decal without lights
#define EFT_INVLIGHT   3  // Decal with HW lights (DLight should be added)
#define EFT_WHITESHADOW 4  // Depth map
#define EFT_WHITE        7  // Just white texture with Z write
#define EFT_FROMMAX      14  // Max
#define EFT_HEATVISION  19 // Heat vision template
#define EFT_DOF         20 // Depth of field template
#define EFT_USER_FIRST    30   // First user defined template (each user's template must be registered)


//====================================================================================

// Phys. material flags
#define MATF_NOCLIP 1

//====================================================================================

#define EFSLIST_PREPROCESS_ID 2
#define EFSLIST_DISTSORT_ID   1
#define EFSLIST_GENERAL_ID    0
#define EFSLIST_STENCIL_ID    3
#define EFSLIST_LAST_ID       4
#define EFSLIST_UNSORTED_ID   5
#define EFSLIST_MASK    (15<<28)

#define EFSLIST_PREPROCESS (EFSLIST_PREPROCESS_ID << 28) // Rendering 1st  (preprocess items)
#define EFSLIST_STENCIL    (EFSLIST_STENCIL_ID << 28)    // Rendering 2nd (opaque surfaces with stencil shadows supporting)
#define EFSLIST_LAST       (EFSLIST_LAST_ID << 28)       // Rendering last (6th) (sorted list of all fog passes, screen shaders, ...)
#define EFSLIST_GENERAL    (EFSLIST_GENERAL_ID << 28)    // Rendering 4th  (default sorted list for most render items)
#define EFSLIST_DISTSORT   (EFSLIST_DISTSORT_ID << 28)   // Rendering 5th (blended sorted by distance render items)
#define EFSLIST_UNSORTED   (EFSLIST_UNSORTED_ID << 28)   // Rendering 3rd (unsorted shadow passes on terrain)

struct SEfTemplates
{
  TArray<SShader *> m_TemplShaders;  // All templates (Shaders)
  int m_nPreferred;
  int m_nMaskAuto;
  SShader *m_Preferred;

  SEfTemplates ()
  {
    memset(this, 0, sizeof(SEfTemplates));
    m_nPreferred = -1;
  }
  ~SEfTemplates()
  {
  }
  void mfFree(SShader *ef);

  void mfSetPreferred(SShader *ef);
  void mfClear(SShader *ef);
  void mfReserve(int Num)
  {
    if (Num >= m_TemplShaders.Num())
      m_TemplShaders.ReserveNew(Num + 1);
  }
  int Size()
  {
    int nSize = sizeof(SEfTemplates);
    nSize += m_TemplShaders.GetSize() * sizeof(SShader *);
    return nSize;
  }
};


// Class of the shader
#define MAX_SH_CLASS 16

enum EShClass
{
  eSH_Misc,
  eSH_World,
  eSH_ClientEffect,
  eSH_ClientPoly,
  eSH_Model,
  eSH_Indoor,
  eSH_MotModel,
  eSH_Screen,
  eSH_Temp,
  eSH_Tree_Leaves,
  eSH_Tree_Branches,
  eSH_Max
};

//================================================================
// Sorting type of the Shader
enum EF_Sort
{
  eS_Unknown = 0,
  eS_PreProcess = 1,                   // Preprocess shader (draw to texture, visibility check, ...)
  eS_Sky = 2,                          // Sky box shader
  eS_ZBuff = 3,
  eS_ShadowMap = 4,		
  eS_Terrain = 5,                      // Terrain surface
  eS_TerrainShadowPass = 6,            // Terrain shadow pass
  eS_TerrainDetailTextures = 7,        // Terrain detail textures
  eS_TerrainLightPass = 8,             // Terrain light overlay pass
  eS_TerrainFogPass = 9,               // Terrain fog overlay pass
  eS_Stencil = 10,                     // Stencil buffer pass
  eS_Opaque = 11,                      // General opaque surfaces
  eS_Decal = 12,                       // Decals on surfaces
  eS_Trees = 13,                       // Trees
  eS_SeeThrough = 14,                  // Alphatest shaders (Z-buffer writing is enabled)
  eS_Banner = 15,                      // Blend shaders (Z-buffer writing is disabled)
  eS_UnderWater = 16,                  // Under water objects
  eS_Water = 17,                       // Water surface
  eS_WaterBeach = 18,                  // Water beach
  eS_Additive = 19,                    // Additive blend polygons
  eS_Sprites = 20,                     // Billboards / sprites
  eS_TerrainDetailObjects = 21,        // Terrain detail grass
  eS_Particles = 22,	                 // All terrain particles
  eS_OcclusionTest = 23,               // Occlusion test geometry
  eS_HeatVision = 24,	
  eS_MuzzleFlash = 25,
  eS_Nearest = 26,	                   // Nearest (first person weapon)
  eS_FogShader = 27,                   // Fog polygon
  eS_FogShader_Trans = 28,             // Fog polygon
  eS_Refractive = 29,                  // Refractive objects
  eS_HDR = 30,                         // HDR post-processing
  eS_Glare = 31,                       // Glare fullscreen polygon
  eS_Max = 32
};

// Different preprocess flags for shaders that require preprocessing (like recursive render to texture, screen effects, visibility check, ...)
// SShader->m_nPreprocess flags in priority order
#define  SPRID_CORONA         0 
#define  FSPR_CORONA          (1<<SPRID_CORONA)
#define  SPRID_NIGHTVIS       1
#define  FSPR_NIGHTVIS        (1<<SPRID_NIGHTVIS)
#define  SPRID_DOFMAP         2
#define  FSPR_DOFMAP          (1<<SPRID_DOFMAP)
#define  SPRID_PORTAL         3
#define  FSPR_PORTAL          (1<<SPRID_PORTAL)
#define  SPRID_SCANCM         4
#define  FSPR_SCANCM          (1<<SPRID_SCANCM)
#define  SPRID_SCANSCR        5
#define  FSPR_SCANSCR         (1<<SPRID_SCANSCR)
#define  SPRID_SCANTEXWATER   6
#define  FSPR_SCANTEXWATER    (1<<SPRID_SCANTEXWATER)
#define  SPRID_SCANTEX        7
#define  FSPR_SCANTEX         (1<<SPRID_SCANTEX)
#define  SPRID_SCANLCM        8
#define  FSPR_SCANLCM         (1<<SPRID_SCANLCM)
#define  SPRID_CUSTOMCM       9
#define  FSPR_CUSTOMCM        (1<<SPRID_CUSTOMCM)
#define  SPRID_CUSTOMTEXTURE  10
#define  FSPR_CUSTOMTEXTURE   (1<<SPRID_CUSTOMTEXTURE)
#define  SPRID_FLASHBANG      11
#define  FSPR_FLASHBANG       (1<<SPRID_FLASHBANG)

#define  SPRID_SHADOWMAPGEN   12
#define  FSPR_SHADOWMAPGEN    (1<<SPRID_SHADOWMAPGEN)
#define  SPRID_RAINOVERLAY    13
#define  FSPR_RAINOVERLAY     (1<<SPRID_RAINOVERLAY)
#define  SPRID_REFRACTED      14
#define  FSPR_REFRACTED       (1<<SPRID_REFRACTED)

  // tiago: added
#define  SPRID_HEATVIS        15
#define  FSPR_HEATVIS         (1<<SPRID_HEATVIS)
#define  SPRID_SCREENTEXMAP   16
#define  FSPR_SCREENTEXMAP    (1<<SPRID_SCREENTEXMAP)

#define  FSPR_MAX             0x40000


// SShader::m_Flags
// Different useful flags
#define EF_NEEDTANGENTS  1       // Shader needs tangent vectors array
#define EF_BUMPAUTOPROC  2
#define EF_BUMPAUTOPIC   4
#define EF_HASVSHADER    8
#define EF_HASCULL       0x10
#define EF_NOBREAKABLELIGHT 0x20
#define EF_OVERLAY       0x40
#define EF_ORIENT        0x80
#define EF_MATERIAL      0x100
#define EF_TEMPLNAMES    0x200
#define EF_SKY_HDR       0x400
#define EF_LIGHTSTYLE    0x800
#define EF_SUNFLARES     0x2000
#define EF_NEEDNORMALS   0x4000  // Need normals operations
#define EF_OFFSETBUMP    0x8000
#define EF_NOTFOUND      0x10000
#define EF_DEFAULT       0x20000
#define EF_SKY           0x40000
#define EF_USELIGHTS     0x80000
#define EF_ALLOW3DC      0x100000
#define EF_FOGSHADER     0x200000
#define EF_USEPROJLIGHTS 0x400000
#define EF_POLYGONOFFSET 0x800000
#define EF_NOMIPMAPS     0x1000000
#define EF_COMPILEDLAYERS 0x2000000
#define EF_USEMTLAYERS   0x4000000
#define EF_DIFFUSELIGHT  0x8000000
#define EF_CLIENTEFFECT  0x10000000
#define EF_SYSTEM        0x20000000
#define EF_HASDIFFUSEMAP 0x40000000
#define EF_NOSPOTS       0x80000000

// SShader::Flags2
// Additional Different useful flags
#define EF2_ALLOW_FENCE    0x1
#define EF2_PREPR_OUTSPACE 0x2
#define EF2_NOCASTSHADOWS  0x4
#define EF2_FOGOVERLAY1    0x8
#define EF2_FOGOVERLAY2    0x10
#define EF2_FOGOVERLAY     0x18
#define EF2_TESSSIZE       0x20
#define EF2_HASOPAQUE      0x40
#define EF2_CUSTOMANIMTEX  0x80
#define EF2_DONTSORTBYDIST 0x100
#define EF2_HASSUNFLARE    0x200
#define EF2_OPAQUE         0x400
#define EF2_TEMPLATE       0x800
#define EF2_IGNORERESOURCESTATES  0x1000
#define EF2_USELIGHTMATERIAL  0x2000
#define EF2_REDEPEND       0x4000
#define EF2_DEFAULTVERTEXFORMAT 0x8000

// SShader::Flags3
// Additional Different useful flags
#define EF3_NODRAW            1
#define EF3_NEEDSYSBUF        2
#define EF3_HASLM             4
#define EF3_HASRGBGEN         8
#define EF3_HASALPHAGEN       0x10
#define EF3_CLIPPLANE_BACK    0x20
#define EF3_CLIPPLANE_FRONT   0x40
#define EF3_CLIPPLANE_WATER_FRONT 0x80
#define EF3_CLIPPLANE_WATER_BACK  0x100
#define EF3_CLIPPLANE         (EF3_CLIPPLANE_BACK | EF3_CLIPPLANE_FRONT | EF3_CLIPPLANE_WATER_BACK | EF3_CLIPPLANE_WATER_FRONT)
#define EF3_SCREENTEXTURE       0x200
#define EF3_IGNOREDIRECTIONALLIGHT  0x400
#define EF3_HASVCOLORS        0x1000
#define EF3_HASALPHATEST      0x2000
#define EF3_HASALPHABLEND     0x4000
#define EF3_USEPARENTSORT     0x8000
#define EF3_USEPARENTCULL     0x10000
#define EF3_REFLECTION        0x20000
#define EF3_REBUILD           0x40000
#define EF3_NODETAIL          0x80000
#define EF3_DEPTHWRITE        0x100000
#define EF3_NOTEMPLATE        0x200000
#define EF3_HASRCOMBINER      0x400000
#define EF3_HASPSHADER        0x800000
#define EF3_HASAMBPASSES      0x1000000
#define EF3_TESSSIZE          0x2000000
#define EF3_SHAREVERTS        0x4000000

#define EF3_PREPARELV      0x10000000
#define EF3_PREPAREHAV     0x20000000
#define EF3_PREPARELAS0    0x40000000
#define EF3_PREPARELAS1    0x80000000
#define EF3_PREPARE_MASK   (EF3_PREPARELV | EF3_PREPAREHAV | EF3_PREPARELAS0 | EF3_PREPARELAS1)

struct IShader
{
public:
	virtual int GetID() = 0;
	virtual void AddRef() = 0;
  virtual void Release(bool bForce=false) = 0;
  virtual int GetRefCount() = 0;
  virtual const char *GetName() = 0;
	virtual EF_Sort GetSort() = 0;
	virtual int GetFlags() = 0;
	virtual int GetFlags2() = 0;
	virtual int GetFlags3() = 0;
  virtual int GetRenderFlags() = 0;
  virtual void SetRenderFlags(int nFlags) = 0;
  virtual int GetLFlags() = 0;
  virtual int GetCull() = 0;
  virtual uint GetPreprocessFlags() = 0;
	virtual void SetFlags3(int Flags) = 0;
  virtual bool Reload(int nFlags) = 0;
  virtual TArray<CRendElement *> *GetREs () = 0;
  virtual bool AddTemplate(SRenderShaderResources *Res, int& TemplId, const char *Name=NULL, bool bSetPreferred=false, uint64 nMaskGen=0) = 0;
  virtual void RemoveTemplate(int TemplId) = 0;
  virtual IShader *GetTemplate(int num) = 0;
  virtual SEfTemplates *GetTemplates() = 0;
  virtual TArray<SShaderParam>& GetPublicParams() = 0;
  virtual int GetTexId () = 0;
  virtual ITexPic *GetBaseTexture(int *nPass, int *nTU) = 0;
  virtual unsigned int GetUsedTextureTypes(void) = 0;
  virtual int GetVertexFormat(void) = 0;
  virtual int Size(int Flags) = 0;
	virtual uint64 GetGenerationMask() = 0;
	virtual SShaderGen* GetGenerationParams() = 0;
};

struct SShaderItem
{
	SShaderItem()
	{
		m_pShader=0;
		m_pShaderResources=0;
	}
  IShader *m_pShader;
  SRenderShaderResources *m_pShaderResources;
  int GetSort(IShader *pSh)
  {
    int nSort = pSh->GetSort();
    if (m_pShaderResources && m_pShaderResources->m_Opacity != 1.0f)
    {
      if (nSort <= eS_Opaque)
        nSort = eS_Banner | (nSort & EFSLIST_MASK);
    }
    return nSort;
  }
  _inline bool IsTransparent()
  { // note: if you change this function please check bTransparent variable in CLeafBuffer::Render()
    IShader *pSH = m_pShader->GetTemplate(-1);
    if (!(pSH->GetFlags2() & EF2_OPAQUE) || (m_pShaderResources && m_pShaderResources->m_Opacity != 1.0f))
      return true;
    return false;
  }
};

//////////////////////////////////////////////////////////////////////////
// Used in IMatInfo:Set/GetFlags
//////////////////////////////////////////////////////////////////////////
#define MIF_POLYBUMP      1
#define MIF_INVPOLYBUMP   2
#define MIF_PHYSIC        4
#define MIF_NOCASTSHADOWS 8
#define MIF_COLLIDABLE	 16
#define MIF_INVALID			 32 // Set when material marked as invalid
#define MIF_WASUSED			 64 // Set when material assigned to some object
#define MIF_CHILD				128 // Set if material is parent material

//////////////////////////////////////////////////////////////////////////
// Material class used by renderer and 3D engine.
//////////////////////////////////////////////////////////////////////////
struct IMatInfo
{
	virtual ~IMatInfo() {};

	//////////////////////////////////////////////////////////////////////////
	// Reference counting.
	//////////////////////////////////////////////////////////////////////////
	virtual void AddRef() = 0;
	virtual void Release() = 0;
  virtual int GetNumRefs() = 0;

	//////////////////////////////////////////////////////////////////////////
	// material name
	//////////////////////////////////////////////////////////////////////////
	//! Set material name, (Do not use this directly, to change material name use I3DEngine::RenameMatInfo method).
	virtual void SetName(const char * pName) = 0;
	//! Returns material name.
	virtual const char *GetName() = 0;

	//! Material flags.
	//! @see MIF_INVALID
	//	see MIF_ flags (like MIF_INVALID)
	virtual void SetFlags( int flags ) = 0;
	virtual int GetFlags() const = 0;

	// shader item
	virtual void SetShaderItem(SShaderItem & _ShaderItem) = 0;
	virtual const SShaderItem & GetShaderItem() = 0;

	//! Used by custom material to override shader item.
	virtual bool OverrideShaderItem( int subMtlId,SShaderItem &si ) = 0;
	
	// shader params
	virtual void SetShaderParams(TArray<SShaderParam> * _pShaderParams) = 0;
	virtual const TArray<SShaderParam> * GetShaderParams() = 0;

	//////////////////////////////////////////////////////////////////////////
	// Sub materials access.
	//////////////////////////////////////////////////////////////////////////
	//! Returns number of child sub materials holded by this material.
	virtual int GetSubMtlCount() = 0;
	//! Return sub material at specified index.
	virtual IMatInfo* GetSubMtl( int i ) = 0;
	//! Adds new sub material to the end of sub materials list.
	virtual void AddSubMtl( IMatInfo *pMtl ) = 0;
	//! Deletes material from sub material list.
	virtual void RemoveSubMtl( IMatInfo *pMtl ) = 0;
	//! Remove all child sub materials at once.
	virtual void RemoveAllSubMtls() = 0;
};

//////////////////////////////////////////////////////////////////////
struct CMatInfo : public IMatInfo
{
  CMatInfo()
  {
		m_nRefCount = 0;
		sScriptMaterial[0]=0;
		sMaterialName[0] = 0;
		m_Id=0;
		m_Flags=0;
		nGamePhysMatId=0;
		m_dwNumSections=0;
		m_pPrimitiveGroups=0;
		m_vCenter.Set(0,0,0);
		m_fRadius=0;
		nFirstIndexId=0;
		nNumIndices=0;
		nFirstVertId=0;
		nNumVerts=0;
		pRE=0;
		fAlpha=0;
		pShaderParams=0;

    fAlpha=1;
    m_Id = -1;
		pMatEnt=0;

		pSubMtls = 0;
		
		m_nCGFMaterialID=-1;
  }

  ~CMatInfo()
	{
		RemoveAllSubMtls();
	}
  virtual int GetNumRefs() { return m_nRefCount; };

  char sScriptMaterial[32];
	char sMaterialName[64];

	//! Number of references to this material.
	int m_nRefCount;
  int m_Id;
	int m_nCGFMaterialID;
	//! Material flags.
	//! @see EMatInfoFlags
  int m_Flags;
  int  nGamePhysMatId;

  ushort m_dwNumSections;
  SPrimitiveGroup *m_pPrimitiveGroups;

  Vec3 m_vCenter;
  float m_fRadius;
  int nFirstIndexId;
  int nNumIndices;
  int nFirstVertId;
  int nNumVerts;

  SShaderItem shaderItem;
  CREOcLeaf * pRE;

  float fAlpha;

	struct MAT_ENTITY * pMatEnt; // used by resource compiler
	TArray<SShaderParam> *pShaderParams;

	//! Array of Sub materials.
	typedef TArray<IMatInfo*> SubMtls;
	SubMtls *pSubMtls;

	//////////////////////////////////////////////////////////////////////////
	// IMatInfo implementation
	//////////////////////////////////////////////////////////////////////////
	// material name
	int Size();

  CMatInfo& operator=(const CMatInfo& src)
  {
    memcpy(this, &src, sizeof(CMatInfo));
    if (shaderItem.m_pShader)
      shaderItem.m_pShader->AddRef();
    if (shaderItem.m_pShaderResources)
      shaderItem.m_pShaderResources->AddRef();
    return *this;
  };

	virtual void AddRef() { m_nRefCount++; };
	virtual void Release()
	{
		if (--m_nRefCount <= 0)
			delete this;
	}

	virtual void SetName(const char * pName)
	{
		strncpy( sMaterialName,pName,sizeof(sMaterialName) );
		sMaterialName[sizeof(sMaterialName)-1] = 0;
	}
	virtual const char* GetName() { return sMaterialName; };

	virtual void SetFlags( int flags ) { m_Flags = flags; };
	virtual int GetFlags() const { return m_Flags; };

	// shader item
	virtual void SetShaderItem(SShaderItem & _ShaderItem) { shaderItem = _ShaderItem; }
	virtual const SShaderItem & GetShaderItem() { return shaderItem; };
	
	// shader params
	virtual void SetShaderParams(TArray<SShaderParam> * _pShaderParams) { pShaderParams = _pShaderParams; }
	virtual const TArray<SShaderParam> * GetShaderParams() { return pShaderParams; }

	//////////////////////////////////////////////////////////////////////////
	virtual bool OverrideShaderItem( int subMtlId,SShaderItem &si )
	{
		// Assume that the root material is the first material, sub materials start from index 1.
		if (subMtlId == 0)
		{
			si = shaderItem;
			return true;
		}
		if (pSubMtls)
		{
			if (subMtlId-1 < (int)pSubMtls->size())
			{
				si = (*pSubMtls)[subMtlId-1]->GetShaderItem();;
				return true;
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Sub materials.
	//////////////////////////////////////////////////////////////////////////
	virtual int GetSubMtlCount()
	{
		if (pSubMtls)
			return pSubMtls->size();
		return 0;
	}
	virtual IMatInfo* GetSubMtl( int i )
	{
		assert( pSubMtls );
		//ASSERT( i >= 0 && i < pSubMtls->size() );
		return (*pSubMtls)[i];
	}
	virtual void AddSubMtl( IMatInfo *pMtl )
	{
		assert( pMtl );
		if (!pSubMtls)
			pSubMtls = new SubMtls;
		pSubMtls->push_back( pMtl );
		
		pMtl->SetFlags(pMtl->GetFlags() | MIF_CHILD);
	}
	virtual void RemoveSubMtl( IMatInfo *pMtl )
	{
		assert( pMtl );
		if (pSubMtls)
		{
			for (unsigned int i = 0; i < pSubMtls->size(); i++)
			{
				if ((*pSubMtls)[i] == pMtl)
				{
					pSubMtls->Delete(i);
					break;
				}
			}
		}
	}
	virtual void RemoveAllSubMtls()
	{
		if (pSubMtls)
			delete pSubMtls;
		pSubMtls = 0;
	}
};

//////////////////////////////////////////////////////////////////////
// DLights
#define DLF_DETAIL					1
#define DLF_DIRECTIONAL			2
#define DLF_DYNAMIC					4				//dynamic lights
#define DLF_ACTIVE					8				//light is active/disactive
#define DLF_CASTSHADOW_MAPS 0x10		//light casting shadows
#define DLF_POINT						0x20
#define DLF_PROJECT					0x40
#define DLF_CASTSHADOW_VOLUME			0x80		//light casting shadows
#define DLF_POSITIONCHANGED 0x100
#define DLF_NOATTENUATION   0x200
#define DLF_UPDATED					0x400
#define DLF_INWORLDSPACE  	0x800
#define DLF_TEMP   					0x1000
#define DLF_STATIC_ADDED		0x2000	//this static light has been already added to the list
#define DLF_HASAMBIENT			0x4000
#define DLF_HEATSOURCE			0x8000
#define DLF_LIGHTSOURCE			0x10000
#define DLF_FAKE      			0x20000 //actually it's not LS, just render elements (Flares, beams, ...)
#define DLF_SUN      			  0x40000 //only sun may use this flag
#define DLF_COPY     			  0x80000
#define DLF_LOCAL    			  0x100000
#define DLF_LM      			  0x200000
#define DLF_THIS_AREA_ONLY  0x400000 // affects only current area/sector
#define DLF_AMBIENT_LIGHT		0x800000 // only used to add better ambient lighting to polybump characters
#define DLF_IGNORE_OWNER		0x1000000 // do not affect light owner object
#define DLF_IGNORE_TERRAIN	0x2000000 // do not affect heightmap
#define DLF_ONLY_FOR_HIGHSPEC	0x4000000 //!< This light is active as dynamic light only for high spec machines.
#define DLF_SPECULAR_ONLY_FOR_HIGHSPEC	0x8000000 //!< This light have specular component enabled only for high spec machines.
#define DLF_LMDOT3          0x10000000
#define DLF_FAKE_RADIOSITY  0x20000000
#define DLF_LMOCCL			0x40000000

#define DLF_LIGHTTYPE_MASK (DLF_DIRECTIONAL | DLF_POINT | DLF_PROJECT)

struct IEntity;
struct ShadowMapLightSourceInstance;

// Marco's NOTE: Andrey / Vlad please subividide this class
// by putting the members into functions and use the names
// info only if in debug mode, other strcutres can be
// allocated only if needed in the constructor and destroyed in
// the destructor, and you can even define an operator -> to
// access the data into the various structures inside the class

//////////////////////////////////////////////////////////////////////
class CDLight
{
public:

	//! constructor
	CDLight( void )
  {
    memset(this, 0, sizeof(CDLight));
    m_fLightFrustumAngle = 45.0f;
    m_fRadius = 4.0f;
    m_fDirectFactor = 1.0f;
    m_Flags = DLF_LIGHTSOURCE;
    m_Orientation.m_vForward = Vec3(1,0,0);
    m_Orientation.m_vUp    = Vec3(0,1,0);
    m_Orientation.m_vRight = Vec3(0,0,1);
    m_NumCM = -1;
		m_nEntityLightId = -1;
//		m_nStaticLightId = -1;
  }

	//! destructor
  ~CDLight()
  {
		SAFE_RELEASE( m_pShader );
		SAFE_RELEASE( m_pLightImage );
  }

	//!
  bool Parse( void )
  {
    if (!m_Name[0])
      return false;

    if (strncmp(m_Name, "ls_", 3)==0 || strstr(m_Name, "_ls"))
			m_Flags |= DLF_LIGHTSOURCE;

    if (strncmp(m_Name, "hs_", 3)==0 || strstr(m_Name, "_hs"))
			m_Flags |= DLF_HEATSOURCE;

    return true;
  }
  void MakeBaseParams()
  {
    m_BaseOrigin = m_Origin;
    m_BaseColor = m_Color;
    m_BaseSpecColor = m_SpecColor;
    m_fBaseRadius = m_fRadius;
    m_fBaseLightFrustumAngle = m_fLightFrustumAngle;
    m_BaseProjAngles = m_ProjAngles;
  }

	//! assignment operator
  CDLight& operator=( const CDLight& dl )
  {
    memcpy(this, &dl, sizeof(CDLight));
		if (m_pShader)
			m_pShader->AddRef();
		if (m_pLightImage)
			m_pLightImage->AddRef();
    m_Flags |= DLF_COPY;
    return *this;
  }

  int															m_Id;
  Vec3														m_Origin;					 //world space position
  Vec3														m_BaseOrigin;					 //world space position
  CFColor													m_Color;									//!< clampled diffuse light color
  CFColor													m_BaseColor;									//!< clampled diffuse light color
  CFColor 												m_SpecColor;
  CFColor 												m_BaseSpecColor;
  Vec3														m_vObjectSpacePos;		 //Object space position
  float														m_fRadius;
  float														m_fBaseRadius;
  float   												m_fDirectFactor;
  float														m_fStartRadius;
  float														m_fEndRadius;
  float														m_fLastTime;
  int     												m_NumCM;

  // Scissor parameters (2d extent)
  short   					m_sX;
  short   					m_sY;
  short   					m_sWidth;
  short   					m_sHeight;
	// Far/near planes
	float							m_fNear;
	float							m_fFar;

  struct IEntityRender *					m_pOwner;
//	int m_nStaticLightId;
  
	//for static spot light sources casting volumetric shadows	
	int m_nReserved; // compensates for the vtbl
  COrthoNormalBasis								m_Orientation;
	int															m_CustomTextureId;
	Matrix44												m_TextureMatrix;

	CCObject *											m_pObject[4][4];								//!< Object for light coronas and light flares

	//the light image
	ITexPic*												m_pLightImage;
  float														m_fLightFrustumAngle;
  float														m_fBaseLightFrustumAngle;
  float                           m_fAnimSpeed;

  IShader*												m_pShader;
  Vec3														m_ProjAngles;
  Vec3														m_BaseProjAngles;

  uint														m_Flags;									//!< flags from above (prefix DLF_)

  char														m_Name[64];
  int                             m_nLightStyle;
  float                           m_fCoronaScale;

  float														m_fStartTime;
  float														m_fLifeTime;							//!< lsource will be removed after this number of seconds

  char														m_sDebugName[8];					//!< name of light creator (for debuging, pointer can't be used since entity may be deleted)

  ShadowMapLightSourceInstance *	m_pShadowMapLightSource;	//!<

  CLeafBuffer *										m_arrLightLeafBuffers[8]; //!< array of leafbuffers used for heightmap lighting pass

	int															m_nEntityLightId;					//!<
	int															m_nFrameID;								//!<

/*
	
	// next change:

	inline CFColor GetClampedDiffColor( void )
	{
		CFColor ret=m_Color; ret.Clamp(); return(ret);
	}

private:
	CFColor													m_Color;									//!< non clampled diffuse light color
*/

	ICryCharInstance * m_pCharInstance; // pointer to character this source is attached to
};

#include "RendElement.h"

#endif// _ISHADER_H_