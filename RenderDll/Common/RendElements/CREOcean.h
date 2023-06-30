#ifndef _CREOCEAN_H_
#define _CREOCEAN_H_

#include "../nvTriStrip/nvTriStrip.h"

struct SREOceanStats
{
  float m_StatsTimeFFTTable;
  float m_StatsTimeFFT;
  float m_StatsTimeUpdateVerts;
  float m_StatsTimeTexUpdate;
  float m_StatsTimeRendOcean;
  int m_StatsAllocatedSectors;
  int m_StatsNumRendOceanSectors;
};

#define OCEANPI         3.141592654f
#define OCEANGRID       64
#define LOG_OCEANGRID   6

#define NUM_LODS        5
#define LOD_MASK        7
#define LOD_LEFTSHIFT   3
#define LOD_RIGHTSHIFT  4
#define LOD_TOPSHIFT    5
#define LOD_BOTTOMSHIFT 6

#define OSF_VISIBLE     1
#define OSF_FIRSTTIME   2
#define OSF_NEEDHEIGHTS 4
#define OSF_LODUPDATED  8
#define OSF_BLEND       0x10

#define OVP_NOHEIGHT      0
#define OVP_NOHEIGHT_SPL  1
#define OVP_HEIGHT        2
#define OVP_HEIGHT_SPL    3
#define NUM_OVP           4

#define NUM_OCEANVBS      8

struct SOceanSector
{
  float x, y;
  int RenderState;
  int nLod;
  int m_Frame;
  int m_Flags;

  SOceanSector ()
  {
    m_Flags = OSF_FIRSTTIME;
    m_Frame = -1;
  }
};

struct SOceanIndicies
{
  float m_fLastAccess;
  int m_nInds;
  ushort *m_pIndicies;
  TArray<SPrimitiveGroup> m_Groups;
};

/**
* A random number class. It implements the <em>mersenne twister</em> algorithm to produce 
* "high quality" pseudo random numbers.
*/
class CRERandom  
{
public:
  enum EMersenneTwisterInitialSeed
  {
    INITIAL_SEED = 4357U
  };

public:
  CRERandom( unsigned long ulSeed = INITIAL_SEED ) : m_ulIndex( 0 )
  {
    SetSeed( ulSeed );
  }

  ~CRERandom() {};

  void SetSeed( unsigned long ulSeed )
  {
    m_ulState[ 0 ] = ( ulSeed | 1 ) & 0xFFFFFFFFU;
    for( m_ulIndex = 1; m_ulIndex < N; ++m_ulIndex )
    {
        m_ulState[ m_ulIndex ] = ( m_ulState[ m_ulIndex - 1 ] * GENERATOR ) & 0xFFFFFFFFU;
    }
  }

  unsigned long GetInteger()
  {
    if( N == m_ulIndex )
    {
      Reload();
    }

    unsigned long ulY = m_ulState[ m_ulIndex++ ];
    ulY ^= TemperingShiftU( ulY );
    ulY ^= TemperingShiftS( ulY ) & TEMPERING_MASK_B;
    ulY ^= TemperingShiftT( ulY ) & TEMPERING_MASK_C;
    ulY ^= TemperingShiftL( ulY );

    return( ulY ); 
  }

  double Get()
  {
    return( GetInteger() * (double) 2.3283064365386963e-10 ); // i.e. GetInteger() / 2^32
  }

  double GetInRange( double dLower = 0.0, double dUpper = 1.0 )
  {
    return( dLower + Get() * ( dUpper - dLower ) );
  }

  double GetGauss( double dMean = 0.0, double dStdDeviation = 1.0 )
  {
    double dX1; 
    double dX2;
    double dW;

    // perform box muller test
    do
    {
      dX1 = 2.0 * Get() - 1.0;
      dX2 = 2.0 * Get() - 1.0;
      dW  = dX1 * dX1 + dX2 * dX2;
    } while( dW >= 1.0 );
    
    dW = sqrt_tpl( -2.0f * log_tpl( dW ) / dW );
    dX1 *= dW;

    return( dMean + dX1 * dStdDeviation );
  }

private:
  enum EMersenneTwisterConstants
  {
      GENERATOR   = 69069U,

      N = 624U,
      M = 397U,

      MATRIX_A   = 0x9908B0DFU,
      UPPER_MASK = 0x80000000U,
      LOWER_MASK = 0x7FFFFFFFU,

      TEMPERING_MASK_B = 0x9D2C5680U,
      TEMPERING_MASK_C = 0xEFC60000U
  };

  void Reload()
  {
    const unsigned long c_ulMag01[ 2 ] = { 0x0, MATRIX_A };

    unsigned long ulY;
    for( m_ulIndex = 0; m_ulIndex < N - M; ++m_ulIndex ) 
    {
        ulY = ( m_ulState[ m_ulIndex     ] & UPPER_MASK ) |
              ( m_ulState[ m_ulIndex + 1 ] & LOWER_MASK );
        
        m_ulState[ m_ulIndex ] = m_ulState[ m_ulIndex + M ] ^ ( ulY >> 1 ) ^ c_ulMag01[ ulY & 0x1 ];
    }
    
    for( ; m_ulIndex < N  - 1; ++m_ulIndex)
    {
        ulY = ( m_ulState[ m_ulIndex     ] & UPPER_MASK ) | 
              ( m_ulState[ m_ulIndex + 1 ] & LOWER_MASK );

        m_ulState[ m_ulIndex ] = m_ulState[ m_ulIndex + ( M - N ) ] ^ ( ulY >> 1 ) ^ c_ulMag01[ ulY & 0x1 ];
    }

    ulY = ( m_ulState[ N - 1 ] & UPPER_MASK ) | ( m_ulState[ 0 ] & LOWER_MASK );
    m_ulState[ N - 1 ] = m_ulState[ M - 1 ] ^ ( ulY >> 1 ) ^ c_ulMag01[ ulY & 0x1 ];
    
    m_ulIndex = 0;
  }

  unsigned long TemperingShiftU( unsigned long ulX )
  {
    return( ulX >> 11 );
  }

  unsigned long TemperingShiftS( unsigned long ulX )
  {
    return( ulX << 7 );
  }

  unsigned long TemperingShiftT( unsigned long ulX )
  {
    return( ulX << 15 );
  }

  unsigned long TemperingShiftL( unsigned long ulX )
  {
    return( ulX >> 18 );
  }

private:
  unsigned long m_ulIndex;

  unsigned long m_ulState[ N ];
};

//=============================================================================

class CREOcean : public CRendElement
{
  friend class CRender3D;
  friend class CGLRenderer;
  friend class CD3D9Renderer;

public:

  static CREOcean *m_pStaticOcean;
  CREOcean()
  {
    m_nFrameLoad = 0;
    m_pBuffer = NULL;
    mfSetType(eDATA_Ocean);
    mfUpdateFlags(FCEF_TRANSFORM);
    GenerateGeometry();
    m_pStaticOcean = this;
  }

  virtual ~CREOcean();

public:

  void GenerateGeometry();
  void GenerateIndices(int nLodCode);
  void DrawOceanSector(SOceanIndicies *oi);
  void SmoothLods_r(SOceanSector *os, float fSize, int minLod);
  void LinkVisSectors(float fSize);
  float GetWaterZElevation(float fX, float fY);

  void PostLoad( unsigned long ulSeed, float fWindDirection, float fWindSpeed, float fWaveHeight, float fDirectionalDependence, float fChoppyWavesFactor, float fSuppressSmallWavesFactor );
  void Update( float fTime );
  void UpdateTexture(void);
  void PrepareHMap();

private:
  int m_nFrameLoad;
  float m_fGravity;
  float m_fDepth;

  TArray <unsigned short> m_pIndices[NUM_LODS];

private:
  float *mfFillAdditionalBuffer(SOceanSector *os, int nSplashes, SSplash *pSplashes[], int& nCurSize, int nLod, float fSize);

  void FFT(int iDir, float* real, float* imag);
  void FFT2DReal(float cmpX[OCEANGRID][OCEANGRID]);
  void FFT2D(int iDir, float cmpX[OCEANGRID][OCEANGRID], float cmpY[OCEANGRID][OCEANGRID] );
  _inline float sqrf( float x )
  {
    return (x * x);
  }

  _inline float PhillipsSpectrum( float fKx, float fKy )
  {
    float fKLength = cry_sqrtf(sqrf(fKx) + sqrf(fKy));

    if( fKLength < 1e-8f )
      fKLength = 1e-8f;

    float fScale = 1.0f / fKLength;
    fKx *= fScale;
    fKy *= fScale;

    return (m_fWaveHeight * cry_expf(-1.0f / sqrf(fKLength * m_fLargestPossibleWave) - sqrf(fKLength * m_fSuppressSmallWaves)) * cry_powf(fKx*m_fWindX + fKy*m_fWindY, m_fDirectionalDependence) / cry_powf(fKLength, 4.0f));
  }

  _inline int GetHash(float x, float y)
  {
    return (int)(x*0.133f+y*0.356f) & 255;
  }

  _inline SOceanSector *GetSectorByPos(float x, float y, bool bCreate=true)
  {
    int i;

    int hash = GetHash(x, y);
    for (i=0; i<m_OceanSectorsHash[hash].Num(); i++)
    {
      SOceanSector *os = &m_OceanSectorsHash[hash][i];
      if (os->x == x && os->y == y)
        return os;
    }
    m_RS.m_StatsAllocatedSectors++;
    SOceanSector os;
    os.x = x;
    os.y = y;
    os.m_Flags |= OSF_FIRSTTIME;
    os.nLod = NUM_LODS-1;
    int n = m_OceanSectorsHash[hash].Num();
    m_OceanSectorsHash[hash].AddElem(os);
    return &m_OceanSectorsHash[hash][n];
  }
  int GetLOD(Vec3d camera, Vec3d pos);

private:

  TArray<SOceanSector> m_OceanSectorsHash[256];
  SOceanIndicies *m_OceanIndicies[1<<(LOD_BOTTOMSHIFT+1)];
  TArray<SOceanSector *> m_VisOceanSectors;
  float m_fSectorSize;

  TArray<unsigned short> m_DWQIndices;
  TArray<struct_VERTEX_FORMAT_P3F_COL4UB> m_DWQVertices;

  float m_H0X[OCEANGRID+1][OCEANGRID+1];
  float m_H0Y[OCEANGRID+1][OCEANGRID+1];

  //static _declspec(align(16)) float m_HX[OCEANGRID][OCEANGRID];
  //static _declspec(align(16)) float m_HY[OCEANGRID][OCEANGRID];

  //static _declspec(align(16)) float m_NX[OCEANGRID][OCEANGRID];
  //static _declspec(align(16)) float m_NY[OCEANGRID][OCEANGRID];

  //static _declspec(align(16)) float m_DX[OCEANGRID][OCEANGRID];
  //static _declspec(align(16)) float m_DY[OCEANGRID][OCEANGRID];
	DEFINE_ALIGNED_DATA_STATIC( float, m_HX[OCEANGRID][OCEANGRID], 16 );
	DEFINE_ALIGNED_DATA_STATIC( float, m_HY[OCEANGRID][OCEANGRID], 16 );

	DEFINE_ALIGNED_DATA_STATIC( float, m_NX[OCEANGRID][OCEANGRID], 16 );
	DEFINE_ALIGNED_DATA_STATIC( float, m_NY[OCEANGRID][OCEANGRID], 16 );

	DEFINE_ALIGNED_DATA_STATIC( float, m_DX[OCEANGRID][OCEANGRID], 16 );
	DEFINE_ALIGNED_DATA_STATIC( float, m_DY[OCEANGRID][OCEANGRID], 16 );

  vec2_t m_Pos[OCEANGRID+1][OCEANGRID+1];
  Vec3d m_Normals[OCEANGRID+1][OCEANGRID+1];

  float m_aAngularFreq[OCEANGRID][OCEANGRID];
  float m_aKScale[OCEANGRID][OCEANGRID];

  float m_fWindX;    
  float m_fWindY;
  float m_fWindSpeed;
  float m_fSpeed;
  float m_fLargestPossibleWave;
  float m_fSuppressSmallWaves;
  float m_fWaveHeight;
  float m_fDirectionalDependence;
  float m_fChoppyWaveFactor;
  float m_fSuppressSmallWavesFactor;

  CVertexBuffer *m_pBuffer;

  Vec3d m_MinBound;
  Vec3d m_MaxBound;

  //===========================================================================

  _inline float GetHMap(float x, float y)
  {
    float dDownLandZ;

    if( x<-m_fExpandHM || y<-m_fExpandHM || x>=m_fTerrainSize+m_fExpandHM || y>=m_fTerrainSize+m_fExpandHM)
      dDownLandZ = 0;
    else
    {
      // convert into hmap space
      x *= m_fInvHMUnitSize;
      y *= m_fInvHMUnitSize;

      x += m_fExpandHMap;
      y += m_fExpandHMap;

      long nX = QInt(x);
      long nY = QInt(y);
      long nX1 = (nX+1 >= m_nHMapSize) ? nX : nX+1;
      long nY1 = (nY+1 >= m_nHMapSize) ? nY : nY+1;

      float dx = x - nX;
      float dy = y - nY;

      float dDownLandZ0 = (1.0f-dx) * m_HMap[nX*m_nHMapSize+nY]  + dx * m_HMap[nX1*m_nHMapSize+nY];
      float dDownLandZ1 = (1.0f-dx) * m_HMap[nX*m_nHMapSize+nY1] + dx * m_HMap[nX1*m_nHMapSize+nY1];
      dDownLandZ = (1-dy) * dDownLandZ0 + dy * dDownLandZ1;
    }
    return dDownLandZ;
  }

public:
#if !defined(PS2) && !defined (GC)
  CVProgram * m_VPs[NUM_OVP];
  CVProgram * m_VPQ;
#endif
  void *m_pVertsPool[NUM_OCEANVBS];
  void *m_VertDecl;
  void *m_VertDeclHeightSplash;
  int m_nCurVB;
  int m_nNumVertsInPool;
  bool m_bLockedVB;

  float *m_HMap;
  int    m_nHMUnitSize;
  float  m_fExpandHM;
  float  m_fExpandHMap;
  int    m_nHMapSize;
  float  m_fInvHMUnitSize;
  float  m_fTerrainSize;

  void InitVB();
  struct_VERTEX_FORMAT_TEX2F *GetVBPtr(int nVerts);
  void UnlockVBPtr();

  void mfDrawOceanSectors();
  void mfDrawOceanScreenLod();

public:
  virtual void mfPrepare();
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);
  virtual bool mfCompile(SShader *ef, char *scr);
  virtual void *mfGetPointer(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags);
  virtual bool mfPreDraw(SShaderPass *sl);
  virtual void mfReset();
  virtual int Size()
  {
    int i;
    int nSize = sizeof(*this);
    nSize += m_DWQVertices.GetMemoryUsage();
    nSize += m_DWQIndices.GetMemoryUsage();

    nSize += sizeof(m_Pos);
    nSize += sizeof(m_Normals);
    nSize += m_VisOceanSectors.GetMemoryUsage() + 12;
    for (i=0; i<(1<<(LOD_BOTTOMSHIFT+1)); i++)
    {
      SOceanIndicies *oi = m_OceanIndicies[i];
      if (!oi)
        continue;

      nSize += sizeof(SOceanIndicies);
      nSize += oi->m_nInds * sizeof(short);
      nSize += oi->m_Groups.GetMemoryUsage();
    }
    for (i=0; i<256; i++)
    {
      nSize += m_OceanSectorsHash[i].GetMemoryUsage()+12;
    }
    // CREOcean::m_HMap array
    nSize += m_nHMapSize * m_nHMapSize * sizeof(float);

    return nSize;
  }

  static SREOceanStats m_RS;

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


#endif // _CREOCEAN_H_
