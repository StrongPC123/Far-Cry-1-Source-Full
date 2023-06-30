/*=============================================================================
  TexMan.h : Common texture manager declarations.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

      
#ifndef TEXMAN_INCLUDED
#define TEXMAN_INCLUDED

#include "Image/CImage.h"
#include <Names.h>
#include "../ResFile.h"

#define TX_FIRSTBIND 0x1000
#define TX_LASTBIND  0x4000
#define TO_NORMALIZE_CUBE_MAP     0xfff
#define TO_GLARE                  0xffe
#define TO_ENVIRONMENT_LIGHTCUBE_MAP   0xffd
#define TO_ENVIRONMENT_CUBE_MAP   0xffc
#define TO_TEXTURE_WATERMAP       0xffb
#define TO_ENVIRONMENT_TEX        0xff9
#define TO_FROMLIGHT              0xff8
#define TO_LIGHTMAP               0xfeb
#define TO_ENVIRONMENT_SCR        0xfea
#define TO_FOG                    0xfe9
#define TO_FROMOBJ                0xfe8

#define TO_FROMRE0                0xff0
#define TO_FROMRE1                0xff1
#define TO_FROMRE2                0xff2
#define TO_FROMRE3                0xff3
#define TO_FROMRE4                0xff4
#define TO_FROMRE5                0xff5
#define TO_FROMRE6                0xff6
#define TO_FROMRE7                0xff7

#define TO_CUSTOM_CUBE_MAP_FIRST  0xfe0  // 8 environment cube textures
#define TO_CUSTOM_CUBE_MAP_LAST   0xfe7

#define TO_LIGHT_CUBE_MAP         0xfef

#define TO_CUSTOM_TEXTURE_FIRST   0xfb0
#define TO_CUSTOM_TEXTURE_LAST    0xfbf

#define TO_RAINMAP                 0xfa0  // Rain texture map
#define TO_FLASHBANGMAP            0xfa1  // FlashBang texture map
#define TO_GHOST                   0xfa2  // Ghost texture map

#define TO_SCREENMAP               0xfa3  // Screen texture map
#define TO_PREVSCREENMAP           0xfa4  // Previous screen texture map
#define TO_SCREENLUMINOSITYMAP     0xfa5  // Screen luminosity map
#define TO_SCREENCURRLUMINOSITYMAP 0xfa6  // Screen luminosity map
#define TO_SCREENLOWMAP 0xfc0             // lowres - screen size /4
#define TO_SCREENAVGMAP 0xfc1             // average screen color, 2x2
#define TO_DOFMAP       0xfc2             // dof map - contains focal distance

#define TO_REFRACTMAP              0xfa8
#define TO_REFRACTSCREENMAP        0xfa9  

#define TO_ENVIRONMENT_LIGHTCUBE_MAP_REAL 0xf00
#define TO_ENVIRONMENT_LIGHTCUBE_MAP_REAL_MAX TO_ENVIRONMENT_LIGHTCUBE_MAP_REAL + MAX_ENVLIGHTCUBEMAPS

#define TO_ENVIRONMENT_CUBE_MAP_REAL 0xf40
#define TO_ENVIRONMENT_CUBE_MAP_REAL_MAX TO_ENVIRONMENT_CUBE_MAP_REAL + MAX_ENVCUBEMAPS

#define TO_ENVIRONMENT_TEX_MAP_REAL 0xf80
#define TO_ENVIRONMENT_TEX_MAP_REAL_MAX 0xf80 + MAX_ENVTEXTURES

#define TF_USEPAL 1

#define NUM_HDR_TONEMAP_TEXTURES 4
#define NUM_HDR_BLOOM_TEXTURES   3
#define NUM_HDR_STAR_TEXTURES    12

_inline int ilog2( int n )
{
  int k, c;

  if (n<=0)
    return 0;
  k = c = 0;
  while (true)
  {
    if (n & 1)
      c++;
    if (!(n>>=1))
      break;
    k++;
  }
  if (c <= 1)
    return 1<<k;
  else
    return 1<<(k+1);
}

////////////////////////////////////////////////////////////////////////////////////
// CBaseMap: Base class so that different implementations can be stored together //
////////////////////////////////////////////////////////////////////////////////////
class CBaseMap
{
protected:
  uint m_dwWidth, m_dwHeight;
  uint m_dwLevels;

public:
  STexPic *m_pTex;
  CBaseMap()
  {
    m_dwWidth = m_dwHeight = 1;
    m_dwLevels = 0;
    m_pTex = NULL;
  }
  ~CBaseMap()
  {
    m_pTex = NULL;
  }

  virtual HRESULT Initialize() = 0;
};

struct SBoxCol 
{
  UCol m_Colors[6];
};

struct SRefTex 
{
  SRefTex()
  {
    memset(this, 0, sizeof(SRefTex));
#ifndef _XBOX
    m_Pal = -1;
#endif
  }
  void *m_VidTex;
  int m_Type;
  byte bRepeats, bProjected; 
  int m_MipFilter;
  int m_MinFilter;
  int m_MagFilter;
  int m_AnisLevel;
#ifndef _XBOX
  int m_Pal;
#else  
  D3DPalette* m_pPal;
#endif
};

struct STexLoaded
{
  int m_NumTextures;
  STexPic * m_Textures[7];
};

typedef std::map<long,STexLoaded*> LoadedTexsMap;
typedef LoadedTexsMap::iterator LoadedTexsMapItor;

#define TEXTGT_2D 0
#define TEXTGT_CUBEMAP 1
#define TEXTGT_3D 2

struct SDtex
{
  char m_Name[32];
  int  m_Width, m_Height;
  int  m_Offset;            // source data stored
  char m_AnimName[32];      // next frame in animation chain
  ETEX_Format m_ETF;
  int   m_Flags;
  int   m_InFlags;
  int   m_Contents;
  int   m_Value;
  int   m_Color[3];
};

struct STexPool;

struct STexPoolItem
{
  STexPic *m_pTex;
  STexPool *m_pOwner;
  STexPoolItem *m_Next;
  STexPoolItem *m_Prev;
  STexPoolItem *m_NextFree;
  STexPoolItem *m_PrevFree;
  void *m_pAPITexture;
  float m_fLastTimeUsed;

  STexPoolItem ()
  {
    m_pTex = NULL;
    m_pAPITexture = NULL;
    m_pOwner = NULL;
    m_Next = NULL;
    m_Prev = NULL;
    m_NextFree = NULL;
    m_PrevFree = NULL;
  }

  _inline void Unlink()
  {
    if (!m_Next || !m_Prev)
      return;
    m_Next->m_Prev = m_Prev;
    m_Prev->m_Next = m_Next;
    m_Next = m_Prev = NULL;
  }

  _inline void Link( STexPoolItem* Before )
  {
    if (m_Next || m_Prev)
      return;
    m_Next = Before->m_Next;
    Before->m_Next->m_Prev = this;
    Before->m_Next = this;
    m_Prev = Before;
  }

  _inline void UnlinkFree()
  {
    if (!m_NextFree || !m_PrevFree)
      return;
    m_NextFree->m_PrevFree = m_PrevFree;
    m_PrevFree->m_NextFree = m_NextFree;
    m_NextFree = m_PrevFree = NULL;
  }
  _inline void LinkFree( STexPoolItem* Before )
  {
    if (m_NextFree || m_PrevFree)
      return;
    m_NextFree = Before->m_NextFree;
    Before->m_NextFree->m_PrevFree = this;
    Before->m_NextFree = this;
    m_PrevFree = Before;
  }
};

struct STexPool
{
  int m_Width;
  int m_Height;
  int m_Format;
  ETexType m_eTT;
  int m_nMips;
  int m_Size;
  void *m_pSysTexture;           // System texture for updating
  TArray <void *> m_SysSurfaces; // List of system surfaces for updating
  STexPoolItem m_ItemsList;
};

struct STexUploadInfo
{
  int nStartMip;
  int nEndMip;
  int m_TexId;
  STexPic *m_pTex;
  int m_LoadedSize;
};

#define TEXCACHE_VERSION 1

struct STexCacheFileHeader
{
  int m_Version;
  int m_SizeOf;
  byte m_nSides;
  byte m_nMips;
  char m_sExt[6];
  char m_sETF[16];
  int m_DstFormat;
  bool m_bPolyBump;
  bool m_bCloneSpace;
  FILETIME m_WriteTime[2];
};

struct STexCacheMipHeader
{
  int m_SizeOf;
  short m_USize;
  short m_VSize;
  int m_Size;
  int m_SizeWithMips;
};

//
// Base mipmap.
//
struct SMipmapBase
{
public:
  byte*     DataPtr;    // Pointer to data, valid only when locked.
  int       USize,  VSize;  // Power of two tile dimensions.
  int       DXTSize;
  SMipmapBase(int InUSize, int InVSize) : DataPtr(0), USize(InUSize), VSize(InVSize) {}
  SMipmapBase() {}
  virtual ~SMipmapBase() { DataPtr = NULL; }
};

//
// Texture mipmap.
//
struct SMipmap : public SMipmapBase
{
public:
  TArray<byte> DataArray; // Data.
  bool m_bUploaded;
  bool m_bLoading;
  SMipmap()
  {
    m_bUploaded = false;
    m_bLoading = false;
  }
  SMipmap(int InUSize, int InVSize) : SMipmapBase(InUSize, InVSize), m_bUploaded(false), m_bLoading(false) {}
  SMipmap(int InUSize, int InVSize, int InSize) : SMipmapBase(InUSize, InVSize), DataArray(InSize), m_bUploaded(false), m_bLoading(false) {}
  virtual ~SMipmap() { DataArray.Free(); }
  void Clear()
  {
    DataArray.Clear();
    m_bUploaded = false;
    m_bLoading = false;
  }
  void Init(int InUSize, int InVSize, int InSize)
  {
    DataArray.Reserve(InSize);
    USize = InUSize;
    VSize = InVSize;
    DataPtr = NULL;
    m_bUploaded = false;
    m_bLoading = false;
  }
};

struct STexShadow
{
  STexShadow();
  ~STexShadow();

  bool Init(byte *pHeightMap, int Width, int Height, bool bHeightmap);
  void BuildBasisMap(byte *pBasis, float PrimAngle, float fAngle2);
  void BuildHorizonMap_FromHeightmap(BYTE *pHor, byte* pHeight, FLOAT dx, FLOAT dy, INT iStepLength, INT iWidth,INT iHeight,BOOL bWrapU,BOOL bWrapV);
  void BuildHorizonMap_FromNormalmap(BYTE *pHor, byte* pHeight, FLOAT dx, FLOAT dy, INT iStepLength, INT iWidth,INT iHeight,BOOL bWrapU,BOOL bWrapV);
  bool BuildInterleavedMap(UCol *newTex, BYTE *r, BYTE *g, BYTE *b, BYTE *a, INT width, INT height);

  // scratch space for loading/computing the textures
  static STexPic *m_pBasisTex[2];
  STexPic *m_pHorizonTex[2];
};

class CTextureStreamCallback : public IStreamCallback
{
  virtual void StreamOnComplete (IReadStream* pStream, unsigned nError);
};

struct STexCacheInfo
{
  CTextureStreamCallback m_Callback;
  int m_TexId;
  int m_nStartLoadMip;
  int m_nEndLoadMip;
  int m_nSizeToLoad;
  int m_nCubeSide;
  void *m_pTempBufferToStream;
  float m_fStartTime;
  STexCacheInfo()
  {
    memset(&m_TexId, 0, sizeof(*this)-sizeof(CTextureStreamCallback));
  }
  ~STexCacheInfo()
  {
    SAFE_DELETE_ARRAY(m_pTempBufferToStream);
  }
};

#ifdef WIN64
#pragma warning( push )							//AMD Port
#pragma warning( disable : 4267 )				// conversion from 'size_t' to 'int', possible loss of data
#endif

struct STexPic : ITexPic
{
  STexPic()
  {
    if (!m_Root.m_Next)
    {
      m_Root.m_Next = &m_Root;
      m_Root.m_Prev = &m_Root;
    }
    m_CacheID = -1;
    m_Depth = 1;
  }
  int Size(int Flags)
  {
    int nSize = sizeof(*this);
    nSize += m_SourceName.size();
    nSize += m_Name.size();
    if (m_pPalette)
      nSize += sizeof(SRGBPixel) * 256;
    if (m_pData32)
      nSize += m_Width * m_Height * 4;
    if (m_p8to24table)
      nSize += 256 * sizeof(uint);
    if (m_p15to8table)
      nSize += 32768;

    return nSize;
  }

  virtual void AddRef()
  {
    m_nRefCounter++;
  }
  virtual void Release(int bForce);
  virtual void ReleaseDriverTexture();

  virtual void SetClamp(bool bEnable);
  virtual void SetFilter();
  virtual void SetWrapping();

	virtual const char *GetName();

  virtual void Set(int nTexSlot=-1);
  
  virtual int GetWidth();
  virtual int GetHeight();
  virtual int GetOriginalWidth()
  {
    return m_WidthOriginal;
  }
  virtual int GetOriginalHeight()
  {
    return m_HeightOriginal;
  }
  virtual int GetTextureID();
  virtual bool IsTextureLoaded(); 
  virtual int GetFlags()
  {
    return m_Flags;
  }
  virtual int GetFlags2()
  {
    return m_Flags2;
  }
  virtual byte *GetData32();

  virtual void SaveTGA(const char *name, bool bMips);
  virtual void SaveJPG(const char *name, bool bMips);
  virtual void PrecacheAsynchronously(float fDist, int Flags);
  virtual void Preload (int Flags);
  virtual int DstFormatFromTexFormat(ETEX_Format eTF);
  virtual int TexSize(int Width, int Height, int DstFormat);
  virtual bool SetFilter(int nFilter);

  void Unload();
  void Restore();

  int GetFileNames(char *name0, char *name1, int nLen);
  void GetCacheName(char *name);
  void SaveToCache();
  void LoadFromCache(int Flags, float fDist);
  void CreateMips();
  void RemoveMips(int nFromEnd);

  bool AddToPool(int nStartMip, int nMips);
  void RemoveFromPool();

  void RemoveFromSearchHash();
  void AddToSearchHash();

  STexPic *LoadFromCache(STexPic *ti, int flags, int flags2, char *texName, const char *szModelName);
  bool CreateCacheFile();

  virtual void BuildMips();
  virtual bool UploadMips(int nStartMip, int nEndMip);
  void FakeUploadMips(int nStartMip, int nEndMip);

  _inline void Relink(STexPic* Before)
  {
    if (!IsStreamed())
      return;
    if (m_Next && m_Prev)
    {
      m_Next->m_Prev = m_Prev;
      m_Prev->m_Next = m_Next;
    }
    m_Next = Before->m_Next;
    Before->m_Next->m_Prev = this;
    Before->m_Next = this;
    m_Prev = Before;
  }
  _inline void Unlink()
  {
    if (!m_Next || !m_Prev)
      return;
    m_Next->m_Prev = m_Prev;
    m_Prev->m_Next = m_Next;
    m_Next = m_Prev = NULL;
  }
  _inline void Link( STexPic* Before )
  {
    if (m_Next || m_Prev)
      return;
    m_Next = Before->m_Next;
    Before->m_Next->m_Prev = this;
    Before->m_Next = this;
    m_Prev = Before;
  }
  _inline bool IsStreamed()
  {
    if (m_ETF == eTF_Index || m_ETF == eTF_DSDT_MAG)
      return false;
    if (!m_Size || !m_Id || (m_Flags & FT_NOSTREAM))
      return false;
    return true;
  }
  int UpdateMip(float fDist);
  bool ValidateMipsData(int nStartMip, int nEndMip);

  STexPic *m_Next;
  STexPic *m_Prev;

  uint m_Bind;
  uint m_TargetType;
  int m_AccessFrame;
  SRefTex m_RefTex;
  uint m_Flags;
  uint m_Flags2;
  int m_Size;

  ETexType m_eTT;
  int m_nMips;

  CBaseMap *m_pFuncMap;
  STexLoaded *m_TL;
  STexShadow *m_pSH;
  int m_Id;
  bool m_bBusy;
  string m_SourceName;
  string m_Name;
  CName m_SearchName;
  int m_DXTSize;
  int m_CubeSide;
  int m_Width;
  int m_Height;
  int m_Depth;
  int m_WidthOriginal;
  int m_HeightOriginal;
  float m_fAnimSpeed;
  float m_fAmount1;
  float m_fAmount2;
  int m_DstFormat;
  ETEX_Format m_ETF;
  STexPic *m_NextTxt;
  STexPic *m_NextCMSide;
  float *m_Matrix;
  byte *m_pData32;
  int m_nRefCounter;

  // Used for Caching/Streaming
  int m_nFrameCache;
  float m_fMinDistance;
  STexPoolItem *m_pPoolItem;
  int m_CacheID;
  short m_nBaseMip;
  short m_nCustomMip;
  FILETIME m_WriteTime;
  int m_LoadFrame;
  int m_LoadedSize;
  SMipmap **m_Mips[6];   // Mipmaps in native format.
  STexCacheFileHeader m_CacheFileHeader;
  STexCacheMipHeader *m_pFileTexMips;

  // For paletted textures
  SRGBPixel *m_pPalette;
  byte *m_pData;
  bool m_bAlphaPal;
  uint  *m_p8to24table;
  uchar *m_p15to8table;

  static STexPic m_Root;

#ifdef DEBUGALLOC
#undef new
#endif
  void* operator new( size_t Size )
  {
    void *ptr = malloc(Size);
    memset(ptr, 0, Size);
    return ptr;
  }
  void operator delete( void *ptr )
  {
    free (ptr);
  }
  static bool SaveTga(unsigned char *sourcedata,int sourceformat,int w,int h,const char *filename,bool flip);
};

struct STexGrid
{
  int m_Width;
  int m_Height;
  STexPic *m_TP;
};

class CTexMan
{
  friend struct STexPic;
protected:
  int m_MinFilter;
  int m_MagFilter;
  int m_MipFilter;

  int m_CurTexMaxSize;
  int m_CurTexMinSize;
  int m_CurTexSkyResolution;
  int m_CurTexSkyQuality;
  int m_CurTexResolution;
  int m_CurTexQuality;
  int m_CurTexBumpResolution;
  int m_CurTexBumpQuality;
  int m_CurTexNormalMapCompressed;

  LoadedTexsMap m_TexsMap;
  int m_CurCubemapBind;
  int m_CurMotionId;
  int m_CurMotionTexSize;

private:  

  _inline void FillRGBA_32to32(byte *dst, byte *src, int width, int height)
  {
    int Size = width*height;
    for(int i=0; i<Size; i++)
    {
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
      dst[3] = src[3];
      src += 4;
      dst += 4;
    }
  }
  _inline void FillRGBA_32to32(byte *dst, byte *src, int Size)
  {
    for(int i=0; i<Size; i++)
    {
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
      dst[3] = src[3];
      src += 4;
      dst += 4;
    }
  }

  _inline void FillBGRA_32to32(byte *dst, byte *src, int width, int height)
  {
    int Size = width*height;
    for(int i=0; i<Size; i++)
    {
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
      dst[3] = src[3];
      src += 4;
      dst += 4;
    }
  }
  _inline void FillBGRA_32to32(byte *dst, byte *src, int Size)
  {
    for(int i=0; i<Size; i++)
    {
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
      dst[3] = src[3];
      src += 4;
      dst += 4;
    }
  }

  _inline void FillRGBA_24to32(byte *dst, byte *src, int width, int height)
  {
    int Size = width*height;
    for(int i=0; i<Size; i++)
    {
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      dst[3] = 255;
      src += 3;
      dst += 4;
    }
  }
  _inline void FillRGBA_24to32(byte *dst, byte *src, int Size)
  {
    for(int i=0; i<Size; i++)
    {
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      dst[3] = 255;
      src += 3;
      dst += 4;
    }
  }

  _inline void FillBGRA_24to32(byte *dst, byte *src, int width, int height)
  {
    int Size = width*height;
    for(int i=0; i<Size; i++)
    {
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
      dst[3] = 255;
      src += 3;
      dst += 4;
    }
  }
  _inline void FillBGRA_24to32(byte *dst, byte *src, int Size)
  {
    for(int i=0; i<Size; i++)
    {
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
      dst[3] = 255;
      src += 3;
      dst += 4;
    }
  }

  _inline void FillRGBA_8to32(byte *dst, byte *src, byte pal[256][4], int width, int height, STexPic *ti)
  {
    int l;
    int a = 255;
    int Size = width*height;
    for(int i=0; i<Size; i++)
    {
      l = src[i];
      dst[0] = pal[l][0];
      dst[1] = pal[l][1];
      dst[2] = pal[l][2];
      dst[3] = pal[l][3];
      a &= pal[l][3];
      dst += 4;
    }

    if (a != 255)
      ti->m_Flags |= FT_HASALPHA;
  }
  _inline void FillRGBA_8to32(byte *dst, byte *src, byte pal[256][4], int Size, STexPic *ti)
  {
    int l;
    int a = 255;
    for(int i=0; i<Size; i++)
    {
      l = src[i];
      dst[0] = pal[l][0];
      dst[1] = pal[l][1];
      dst[2] = pal[l][2];
      dst[3] = pal[l][3];
      a &= pal[l][3];
      dst += 4;
    }
    
    if (a != 255)
      ti->m_Flags |= FT_HASALPHA;
  }

  _inline void FillBGRA_8to32(byte *dst, byte *src, byte pal[256][4], int width, int height, STexPic *ti)
  {
    int l;
    int a = 255;
    int Size = width*height;
    for(int i=0; i<Size; i++)
    {
      l = src[i];
      dst[0] = pal[l][2];
      dst[1] = pal[l][1];
      dst[2] = pal[l][0];
      dst[3] = pal[l][3];
      a &= pal[l][3];
      dst += 4;
    }

    if (a != 255)
      ti->m_Flags |= FT_HASALPHA;
  }
  _inline void FillBGRA_8to32(byte *dst, byte *src, byte pal[256][4], int Size, STexPic *ti)
  {
    int l;
    int a = 255;
    for(int i=0; i<Size; i++)
    {
      l = src[i];
      dst[0] = pal[l][2];
      dst[1] = pal[l][1];
      dst[2] = pal[l][0];
      dst[3] = pal[l][3];
      a &= pal[l][3];
      dst += 4;
    }
    
    if (a != 255)
      ti->m_Flags |= FT_HASALPHA;
  }

protected:

  void MipMap8Bit (STexPic *ti, byte *in, byte *out, int width, int height);
  void MipMap32Bit (STexPic *ti, byte *in, byte *out, int width, int height);
  void ImgResample(uint *out, int ox, int oy, uint *in, int ix, int iy);
  void ImgResample8(byte *out, int ox, int oy, byte *in, int ix, int iy);

  void ImagePreprocessing(CImageFile* im, uint flags, uint flags2, byte eTT, STexPic *ti);
  void GenerateNormalMap(CImageFile** im, uint flags, uint flags2, byte eTT, float fAmount1, float fAmount2, STexPic *ti);
  byte *GenerateNormalMap(byte *src, int width, int height, uint flags, uint flags2, byte eTT, float fAmount, STexPic *ti, int& nMips, int& nSize, ETEX_Format eTF);
  void MergeNormalMaps(byte *src[2], CImageFile *im[2], int nMips[2]);

  STexPic *TextureInfoForName(const char *nameTex, int numT, byte eTT, uint flags, uint flags2, int bind);
  STexPic *LoadFromImage (const char *name, uint flags, uint flags2, byte eTT, int bind, STexPic *ti, float fAmount1=-1.0f, float fAmount2=-1.0f);
  STexPic *UploadImage(CImageFile* im, const char *name, uint flags, uint flags2, byte eTT, int bind, STexPic *ti, float fAmount1=-1.0f, float fAmount2=-1.0f);
  byte    *ConvertRGB_Gray(byte *src, STexPic *ti, int flags, ETEX_Format eTF);
  void    BuildImageGamma(int wdt, int hgt, byte *dst, bool bHasAlpha);

public:
  CTexMan();
  virtual ~CTexMan() {};
  int GetMinFilter() {return m_MinFilter;}
  int GetMagFilter() {return m_MagFilter;}
  int GetMipFilter() {return m_MipFilter;}

  byte *ImgConvertDXT_RGBA(byte *dst, STexPic *ti, int DXTSize);
  byte *ImgConvertRGBA_DXT(byte *dst, STexPic *ti, int& DXTSize, int& nMips, int bits, bool bUseExistingMips);

  void Shutdown(void);
  virtual bool GetCubeColor(Vec3 *Pos, CFColor *cols);
  virtual bool SetFilter(char *filt)=0;
  virtual void UpdateTextureData(STexPic *pic, byte *data, int USize, int VSize, bool bProc, int State, bool bPal)=0;
  virtual void UpdateTextureRegion(STexPic *pic, byte *data, int X, int Y, int USize, int VSize)=0;
  virtual STexPic *CreateTexture()=0;
  virtual STexPic *CreateTexture(const char *name, int wdt, int hgt, int depth, uint flags, uint flags2, byte *dst, ETexType eTT, float fAmount1=-1.0f, float fAmount2=-1.0f, int DXTSize=0, STexPic *ti=NULL, int bind=0, ETEX_Format eTF=eTF_8888, const char *szSourceName=NULL)=0;
  virtual STexPic *CopyTexture(const char *name, STexPic *ti, int CubeSide=-1)=0;
  virtual byte *GenerateDXT_HW(STexPic *ti, EImFormat eF, byte *dst, int *numMips, int *DXTSize, bool bMips=true) = 0;
  virtual bool IsTextureLoaded(const char *pName);
  virtual void ReloadTextures ();
  virtual void PreloadTextures (int Flags);

  virtual void DrawToTexture(Plane& Pl, STexPic *Tex, int RendFlags)=0;
  virtual void DrawToTextureForGlare(int Id)=0;
  virtual void DrawToTextureForRainMap(int Id)=0;
  virtual void StartHeatMap(int Id)=0;
  virtual void EndHeatMap()=0;
  virtual void StartRefractMap(int Id)=0;
  virtual void EndRefractMap()=0;
  virtual void StartNightMap(int Id)=0;
  virtual void EndNightMap()=0;
  virtual void DrawFlashBangMap(int Id, int RendFlags, CREFlashBang *pRE)=0;
  virtual void StartScreenMap(int Id)=0;
  virtual void EndScreenMap()=0;  
  virtual void StartScreenTexMap(int Id)=0;
  virtual void EndScreenTexMap()=0;
  virtual void DrawToTextureForDof(int Id)=0;
  virtual bool PreloadScreenFxMaps(void)=0;

  virtual bool ScanEnvironmentCM (const char *name, int size, Vec3d& Pos)=0;
  virtual void GetAverageColor(SEnvTexture *cm, int nSide)=0;
  virtual void ScanEnvironmentCube(SEnvTexture *cm, int RendFlags, int Size, bool bLightCube)=0;
  virtual void ScanEnvironmentTexture(SEnvTexture *cm, SShader *pSH, SRenderShaderResources *pRes, int RendFlags, bool bUseExistingREs)=0;
  virtual void EndCubeSide(CCObject *obj, bool bNeedClear)=0;
  virtual void StartCubeSide(CCObject *obj)=0;
  virtual void Update()=0;
  virtual void SetGridTexture(STexPic *tp);

  void UnloadOldTextures(STexPic *pExclude);
  bool CreateCacheFile();
  STexPic *LoadFromCache(STexPic *ti, int flags, int flags2, char *texName, const char *szModelName, ETexType eTT);
  void CreatePools();
  STexPool *CreatePool(int nWidth, int nHeight, int nMips, int nFormat, ETexType eTT);

  virtual STexPic *GetByID(int Id)=0;
  virtual STexPic *GetByName(const char *szName);
  virtual STexPic *AddToHash(int Id, STexPic *ti) {return NULL;}
  virtual void RemoveFromHash(int Id, STexPic *ti) {};
  virtual void SetTexture(int Id, ETexType eTT) {};
  virtual void GenerateFuncTextures()=0;
  virtual void LoadDefaultTextures();

  STexPic *LoadTexture(const char* nameTex, uint flags, uint flags2, byte eTT=eTT_Base, float fAmount1=-1.0f, float fAmount2=-1.0f, int bind=0, int numT=-1);
  STexPic *LoadCubeTex(const char *mapname, uint flags, uint flags2, int State, byte eTT, int RState, int Id, int BindId, float fAmount=-1.0f);
  
  void ClearAll(int nFlags);
  void ReloadAll(int nFlags);
  bool ReloadFile(const char *fileName, int nFlags);
  _inline int GetCurTMU() { return m_CurStage; }
  void ValidateTexSize();
  void CheckTexLimits(STexPic *pExclude);

  byte *ConvertNMToPalettedFormat(byte *src, STexPic *ti, ETEX_Format eTF=eTF_0888);
  void GenerateNMPalette();

public:
  static bool m_bRGBA;

  static int m_CurStage;
  static int m_nCurStages;

  short m_nEnvCX, m_nEnvCY, m_nEnvCZ;
  SBoxCol *m_EnvGridColors;

  STexPic *m_LastTex;
  char m_CurTexFilter[128];
  int m_CurAnisotropic;
  SRGBPixel m_NMPalette[16][16];
  byte m_NMPaletteLookup[256];
  bool m_bPaletteWasLoaded;

  void *m_pCurCubeTexture;
  STexPic *m_CurCubeFaces[6];

  TArray<STexGrid> m_TGrids;
  int m_nTexSizeHistory;
  int m_TexSizeHistory[8];
  int m_nProcessedTextureID1;
  STexPic *m_pProcessedTexture1;
  int m_nProcessedTextureID2;
  STexPic *m_pProcessedTexture2;
  int m_nPhaseProcessingTextures;
  int m_nCustomMip;

  byte m_TexData[256*256*4];

  int m_StatsCurTexMem;
  TArray<STexPic *>m_Textures;
  TArray<int> m_FreeSlots;
  STexPic *m_FirstCMSide;
  STexPic *m_LastCMSide;
  STexPic *m_LastCMStreamed;

  CResFile *m_TexCache;
  int m_LoadBytes;
  int m_UpLoadBytes;
  int m_Streamed;
  float m_fStreamDistFactor;

  STexPic *m_Text_White;
  STexPic *m_Text_WhiteShadow;
  STexPic *m_Text_WhiteBump;
  STexPic *m_Text_Gradient;
  STexPic *m_Text_Depth;
  STexPic *m_Text_Atten2D;
  STexPic *m_Text_Atten1D;
  STexPic *m_Text_Edge;
  STexPic *m_Text_NoTexture;
  STexPic *m_Text_NoiseVolumeMap;
  STexPic *m_Text_NormalizeCMap;
  STexPic *m_Text_EnvLCMap;        // Use for scan environment to CubeMap
  STexPic *m_Text_EnvCMap;        // Use for scan environment to CubeMap
  STexPic *m_Text_EnvTex;         // Use for scan environment to Texture
  STexPic *m_Text_EnvScr;         // Use for scan environment to Texture
  STexPic *m_Text_Glare;      
  STexPic *m_Text_HeatMap;    
  STexPic *m_Text_FurNormalMap;    
  STexPic *m_Text_FurLightMap;    
  STexPic *m_Text_Fur;    
  STexPic *m_Text_RefractMap;    
  STexPic *m_Text_WaterMap;
  STexPic *m_Text_MotionBlurMap;
  STexPic *m_Text_NightVisMap;
  STexPic *m_Text_FlashBangMap;
  STexPic *m_Text_RainMap;
  STexPic *m_Text_LightCMap;
  STexPic *m_Text_FromRE[8];
  STexPic *m_Text_FromObj;
  STexPic *m_Text_FromLight;
  STexPic *m_Text_Fog;
  STexPic *m_Text_Fog_Enter;
  STexPic *m_Text_VFog;
  STexPic *m_Text_Flare;  
  STexPic *m_Text_Ghost;
  STexPic *m_Text_DepthLookup;    
  STexPic *m_Text_FlashBangFlash;            // flash texture used in flasgbang fx 
  STexPic *m_Text_ScreenNoise;               // screen noise texture
  STexPic *m_Text_HeatPalete;                // heat vision pallete texture
  STexPic *m_Text_ScreenMap;                 // screen buffer
  STexPic *m_Text_ScreenMap_HDR;             // screen buffer
  STexPic *m_Text_PrevScreenMap;             // previous screen buffer
  STexPic *m_Text_ScreenLuminosityMap;       // screen luminosity map 
  STexPic *m_Text_ScreenCurrLuminosityMap;   // current screen luminosity map 
  STexPic *m_Text_ScreenLowMap;              // screen buffer size/4
  STexPic *m_Text_ScreenAvg1x1;              // average screen buffer 2x2
  STexPic *m_Text_DofMap;                    // depth of field texture
  
  STexPic *m_Text_Gray;   

  STexPic *m_Text_HDRTarget;
  //STexPic *m_Text_HDRTarget_K;
  STexPic *m_Text_HDRTarget_Temp;
  STexPic *m_Text_HDRTargetScaled[2];
  STexPic *m_Text_HDRBrightPass;
  STexPic *m_Text_HDRStarSource;
  STexPic *m_Text_HDRBloomSource;
  STexPic *m_Text_HDRAdaptedLuminanceCur;
  STexPic *m_Text_HDRAdaptedLuminanceLast;
  STexPic *m_Text_HDRToneMaps[NUM_HDR_TONEMAP_TEXTURES];
  STexPic *m_Text_HDRBloomMaps[NUM_HDR_BLOOM_TEXTURES];
  STexPic *m_Text_HDRStarMaps[NUM_HDR_STAR_TEXTURES][2];

  SEnvTexture m_EnvLCMaps[MAX_ENVLIGHTCUBEMAPS];
  SEnvTexture m_EnvCMaps[MAX_ENVCUBEMAPS];
  SEnvTexture m_EnvTexts[MAX_ENVTEXTURES];

  SEnvTexture m_CustomCMaps[16];
  SEnvTexture m_CustomTextures[16];

  TArray<STexPool *> m_TexPools;
  STexPoolItem m_FreeTexPoolItems;

  STexPic m_Templates[EFTT_MAX];

#ifdef DEBUGALLOC
#undef new
#endif
  void* operator new( size_t Size ) { void *ptr = malloc(Size); memset(ptr, 0, Size); return ptr; }
  void operator delete( void *Ptr ) { free(Ptr); }
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
};


void WriteJPG(byte *dat, int wdt, int hgt, char *name);
void WriteTGA(byte *dat, int wdt, int hgt, char *name, int dest_bits_per_pixel=24);
_inline void WriteTGA32(byte *dat, int wdt, int hgt, char *name)
{
  WriteTGA(dat, wdt, hgt, name, 32);
}
void WriteDDS(byte *dat, int wdt, int hgt, int Size, const char *name, EImFormat eF, int NumMips);
ETEX_Format sImageFormat2TexFormat(EImFormat eImF);

#ifdef WIN64
#pragma warning( pop )							//AMD Port
#endif

#endif

