#include "RenderPCH.h"
#include "RendElement.h"
#include "CREOcean.h"
#include "../NvTriStrip/NVTriStrip.h"
#include "I3dengine.h"

SREOceanStats CREOcean::m_RS;
CREOcean *CREOcean::m_pStaticOcean = NULL;

DEFINE_ALIGNED_DATA( float, CREOcean::m_HX[OCEANGRID][OCEANGRID], 16 ); 
DEFINE_ALIGNED_DATA( float, CREOcean::m_HY[OCEANGRID][OCEANGRID], 16 ); 

DEFINE_ALIGNED_DATA( float, CREOcean::m_NX[OCEANGRID][OCEANGRID], 16 ); 
DEFINE_ALIGNED_DATA( float, CREOcean::m_NY[OCEANGRID][OCEANGRID], 16 ); 

DEFINE_ALIGNED_DATA( float, CREOcean::m_DX[OCEANGRID][OCEANGRID], 16 ); 
DEFINE_ALIGNED_DATA( float, CREOcean::m_DY[OCEANGRID][OCEANGRID], 16 ); 

CREOcean::~CREOcean()
{
  m_pStaticOcean = NULL;
  if (m_pBuffer)
  {
    gRenDev->ReleaseBuffer(m_pBuffer);
    m_pBuffer = NULL;
  }
  SAFE_DELETE_ARRAY(m_HMap);
  for (int i=0; i<NUM_LODS; i++)
  {
    m_pIndices[i].Free();
  }
}

float CREOcean::GetWaterZElevation(float fX, float fY)
{
  if (!m_HMap)
    return 0;
  CRenderer *r = gRenDev;
  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  float fSize = (float)CRenderer::CV_r_oceansectorsize;
  float fHeightScale = (float)CRenderer::CV_r_oceanheightscale;
  float fWaterLevel = eng->GetWaterLevel();
  float fHScale = fabsf(gRenDev->m_RP.m_ViewOrg.z-fWaterLevel);
  float fMaxDist = eng->GetMaxViewDistance() / 1.0f;
  float fGrid = (float)OCEANGRID;

  float fZH = GetHMap(fX, fY);
  if (fZH >= fWaterLevel)
    return fWaterLevel;

  float fXPart = fX / fSize;
  float fYPart = fY / fSize;
  float fXLerp = (fXPart - (int)fXPart) * fGrid;
  float fYLerp = (fYPart - (int)fYPart) * fGrid;
  int nXMin = (int)fXLerp;
  int nXMax = nXMin+1;
  int nYMin = (int)fYLerp;
  int nYMax = nYMin+1;
  fXLerp = fXLerp - (int)fXLerp;
  fYLerp = fYLerp - (int)fYLerp;

  if (fHScale < 5.0f)
    fHScale = LERP(0.1f, 0.5f, fHScale/5.0f);
  else
  if (fHScale < 20)
    fHScale = LERP(0.5f, 1.0f, (fHScale-5.0f)/15.0f);
  else
    fHScale = 1.0f;

  float fZ00 = -m_HX[nYMin][nXMin] * fHScale;
  float fZ01 = -m_HX[nYMin][nXMax] * fHScale;
  float fZ10 = -m_HX[nYMax][nXMin] * fHScale;
  float fZ11 = -m_HX[nYMax][nXMax] * fHScale;
  float fZ0 = LERP(fZ00, fZ01, fXLerp);
  float fZ1 = LERP(fZ10, fZ11, fXLerp);
  float fZ = LERP(fZ0, fZ1, fYLerp);
  //fZ *= fHeightScale;

  float fHeightDelta = fWaterLevel - fZH;
  fHeightScale *= CLAMP(fHeightDelta * 0.066f, 0, 1);

  return fZ * fHeightScale + fWaterLevel;
}

void CREOcean::PrepareHMap()
{
  int nDim=0;

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  m_nHMUnitSize = eng->GetHeightMapUnitSize();
  int nExp = 8;
  m_fExpandHMap = (float)nExp;
  m_fExpandHM = (float)m_nHMUnitSize * (float)nExp;
  m_fInvHMUnitSize = 1.0f / (float)m_nHMUnitSize;
  m_fTerrainSize = (float)eng->GetTerrainSize();
  eng->MakeUnderWaterSmoothHMap(m_nHMUnitSize);
  ushort *shm = eng->GetUnderWaterSmoothHMap(nDim);

	if(!nDim)
		return; // terrain not present

  float fWaterLevel = eng->GetWaterLevel();

  if (m_HMap)
    delete [] m_HMap;
  int nDimO = nDim+nExp*2;
  m_nHMapSize = nDimO;
  m_HMap = new float [nDimO*nDimO];
  float f;
  for (int y=-nExp; y<nDim+nExp; y++)
  {
    for (int x=-nExp; x<nDim+nExp; x++)
    {
      if (x>0 && x<nDim-1 && y>0 && y<nDim-1)
        m_HMap[(y+nExp)*nDimO+x+nExp] = (float)shm[y*nDim+x] / 256.0f;
      else
      {
        int nX = x;
        int nY = y;
        float fLerpX = -1.0f;
        float fLerpY = -1.0f;
        
        if (nX<=0)
          nX = 1;
        else
        if (nX>=nDim-1)
          nX = nDim-2;

        if (nY<=0)
          nY = 1;
        else
        if (nY>=nDim-1)
          nY = nDim-2;

        f = (float)shm[nY*nDim+nX] / 256.0f;

        if (nX != x)
          fLerpX = 1.0f - fabsf((float)(nX-x)) / (float)(nExp+1);
        if (nY != y)
          fLerpY = 1.0f - fabsf((float)(nY-y)) / (float)(nExp+1);

        float fX, fY;

        if (fLerpX >= 0)
          fX = LERP(fWaterLevel/2.0f, f, fLerpX);
        if (fLerpY >= 0)
          fY = LERP(fWaterLevel/2.0f, f, fLerpY);
        if (fLerpX >= 0 && fLerpY >= 0)
          m_HMap[(y+nExp)*nDimO+x+nExp] = (fX + fY) / 2.0f;
        else
        if (fLerpX >= 0)
          m_HMap[(y+nExp)*nDimO+x+nExp] = fX;
        else
        if (fLerpY >= 0)
          m_HMap[(y+nExp)*nDimO+x+nExp] = fY;
        else
          m_HMap[(y+nExp)*nDimO+x+nExp] = f;
      }
    }
  }
}

void CREOcean::mfPrepare()
{
  CRenderer *rd = gRenDev;

  if (m_nFrameLoad != rd->m_nFrameLoad)
  {
    m_nFrameLoad = rd->m_nFrameLoad;
    PrepareHMap();
  }

  rd->EF_CheckOverflow(0, 0, this);

  Update(rd->m_RP.m_RealTime * m_fSpeed);

  double time0 = 0;
  ticks(time0);

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  float fWaterLevel = eng->GetWaterLevel();
  float fHScale = fabsf(gRenDev->m_RP.m_ViewOrg.z-fWaterLevel);
  if (fHScale < 5.0f)
    fHScale = LERP(0.1f, 0.5f, fHScale/5.0f);
  else
  if (fHScale < 20)
    fHScale = LERP(0.5f, 1.0f, (fHScale-5.0f)/15.0f);
  else
    fHScale = 1.0f;
  m_MinBound.Set(9999.0f, 9999.0f, 9999.0f);
  m_MaxBound.Set(-9999.0f, -9999.0f, -9999.0f);

  if (!m_pBuffer)
    m_pBuffer = gRenDev->CreateBuffer((OCEANGRID+1)*(OCEANGRID+1), VERTEX_FORMAT_P3F_N, "Ocean", true);
  rd->UpdateBuffer(m_pBuffer, NULL, 0, 0, 0, 1);

  static const float fScale = 1.0f / (float)OCEANGRID;

  struct_VERTEX_FORMAT_P3F_N *pVertices = (struct_VERTEX_FORMAT_P3F_N *)m_pBuffer->m_VS[VSF_GENERAL].m_VData;
  for(int vy=0; vy<OCEANGRID+1; vy++)
  {
    int y = vy & (OCEANGRID-1);
    for(int vx=0; vx<(OCEANGRID+1); vx++)
    {
      int x = vx & (OCEANGRID-1);
      m_Pos[vy][vx][0] = vx * fScale + m_DX[y][x] * m_fChoppyWaveFactor;
      m_Pos[vy][vx][1] = vy * fScale + m_DY[y][x] * m_fChoppyWaveFactor;
      pVertices->xyz.x = m_Pos[vy][vx][0];
      pVertices->xyz.y = m_Pos[vy][vx][1];
      float fZ = -m_HX[y][x] * fHScale;
      pVertices->xyz.z = fZ;
      m_MinBound.x = min(m_Pos[vy][vx][0], m_MinBound.x);
      m_MinBound.y = min(m_Pos[vy][vx][1], m_MinBound.y);
      m_MinBound.z = min(fZ, m_MinBound.z);

      m_MaxBound.x = max(m_Pos[vy][vx][0], m_MaxBound.x);
      m_MaxBound.y = max(m_Pos[vy][vx][1], m_MaxBound.y);
      m_MaxBound.z = max(fZ, m_MaxBound.z);

      m_Normals[vy][vx] = Vec3d(m_NX[y][x], m_NY[y][x], 64.0f);
      pVertices->normal = m_Normals[vy][vx];
      m_Normals[vy][vx].NormalizeFast();

      pVertices++;
    }       
  }
  unticks(time0);
  m_RS.m_StatsTimeUpdateVerts = (float)(time0*1000.0*g_SecondsPerCycle);

  UpdateTexture();

  rd->m_RP.m_pRE = this;
  rd->m_RP.m_RendNumIndices = 0;
  rd->m_RP.m_RendNumVerts = (OCEANGRID+1)*(OCEANGRID+1);
  rd->m_RP.m_FirstVertex = 0;
}

int CREOcean::GetLOD(Vec3d camera, Vec3d pos)
{
  float dist = (pos-camera).GetLength();
  dist /= CRenderer::CV_r_oceanloddist;

  return (int)CLAMP(dist, 0.0f, (float)(NUM_LODS-1));
}

void *CREOcean::mfGetPointer(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags)
{

  switch(ePT) 
  {
    case eSrcPointer_Vert:
      {
        struct_VERTEX_FORMAT_P3F_N *pVertices = (struct_VERTEX_FORMAT_P3F_N *)m_pBuffer->m_VS[VSF_GENERAL].m_VData;
        *Stride = sizeof(struct_VERTEX_FORMAT_P3F_N);
        return &pVertices->xyz.x;
      }
    case eSrcPointer_Normal:
      {
        struct_VERTEX_FORMAT_P3F_N *pVertices = (struct_VERTEX_FORMAT_P3F_N *)m_pBuffer->m_VS[VSF_GENERAL].m_VData;
        *Stride = sizeof(struct_VERTEX_FORMAT_P3F_N);
        return &pVertices->normal.x;
      }
  }
  return NULL;
}


void CREOcean::GenerateGeometry()
{
  m_pBuffer = gRenDev->CreateBuffer((OCEANGRID+1)*(OCEANGRID+1), VERTEX_FORMAT_P3F_N, "Ocean", true);

  for (int lod=0; lod<NUM_LODS; lod++)
  {
    int nl = 1<<lod;
    // set indices
    int iIndex = 0;
    int yStep = (OCEANGRID+1) * nl;
    for(int a=0; a<(OCEANGRID+1)-1; a+=nl )
    {
      for(int i=0; i<(OCEANGRID+1); i+=nl, iIndex+=nl )
      {
        m_pIndices[lod].AddElem(iIndex);
        m_pIndices[lod].AddElem(iIndex + yStep);
      }

      int iNextIndex = (a+nl) * (OCEANGRID+1);

      if(a < (OCEANGRID+1)-1-nl)
      {
        m_pIndices[lod].AddElem(iIndex + yStep - nl);
        m_pIndices[lod].AddElem(iNextIndex);
      }
      iIndex = iNextIndex;
    }
    m_pIndices[lod].Shrink();
  }
}

void CREOcean::SmoothLods_r(SOceanSector *os, float fSize, int minLod)
{
  if (os->m_Frame != gRenDev->m_cEF.m_Frame)
    return;
  if (os->nLod > minLod+1)
  {
    os->nLod = minLod+1;
    SOceanSector *osLeft;
    SOceanSector *osRight;
    SOceanSector *osTop;
    SOceanSector *osBottom;

    // Left
    osLeft = GetSectorByPos(os->x-fSize, os->y, false);
    // Right
    osRight = GetSectorByPos(os->x+fSize, os->y, false);
    // Top
    osTop = GetSectorByPos(os->x, os->y-fSize, false);
    // Bottom
    osBottom = GetSectorByPos(os->x, os->y+fSize, false);
    if (osLeft)
      SmoothLods_r(osLeft, fSize, os->nLod);
    if (osRight)
      SmoothLods_r(osRight, fSize, os->nLod);
    if (osTop)
      SmoothLods_r(osTop, fSize, os->nLod);
    if (osBottom)
      SmoothLods_r(osBottom, fSize, os->nLod);
  }
}

void CREOcean::LinkVisSectors(float fSize)
{
  for (int i=0; i<m_VisOceanSectors.Num(); i++)
  {
    SOceanSector *os = m_VisOceanSectors[i];
    SOceanSector *osLeft;
    SOceanSector *osRight;
    SOceanSector *osTop;
    SOceanSector *osBottom;

    // Left
    osLeft = GetSectorByPos(os->x-fSize, os->y, false);
    // Right
    osRight = GetSectorByPos(os->x+fSize, os->y, false);
    // Top
    osTop = GetSectorByPos(os->x, os->y-fSize, false);
    // Bottom
    osBottom = GetSectorByPos(os->x, os->y+fSize, false);
    if (osLeft)
      SmoothLods_r(osLeft, fSize, os->nLod);
    if (osRight)
      SmoothLods_r(osRight, fSize, os->nLod);
    if (osTop)
      SmoothLods_r(osTop, fSize, os->nLod);
    if (osBottom)
      SmoothLods_r(osBottom, fSize, os->nLod);
  }
}

void CREOcean::PostLoad(unsigned long ulSeed, float fWindDirection, float fWindSpeed, float fWaveHeight, float fDirectionalDependence, float fChoppyWavesFactor, float fSuppressSmallWavesFactor)
{
  m_fWindX                    = cry_cosf( fWindDirection );    
  m_fWindY                    = cry_sinf( fWindDirection );
  m_fWindSpeed                = fWindSpeed;
  m_fWaveHeight               = fWaveHeight;
  m_fDirectionalDependence    = fDirectionalDependence;
  m_fChoppyWaveFactor         = fChoppyWavesFactor;
  m_fSuppressSmallWavesFactor = fSuppressSmallWavesFactor;

  m_fLargestPossibleWave =  m_fWindSpeed * m_fWindSpeed / m_fGravity;   
  m_fSuppressSmallWaves  = m_fLargestPossibleWave * m_fSuppressSmallWavesFactor;

  // init H0
  CRERandom kRnd( ulSeed );
  float fOneBySqrtTwo = 1.0f / cry_sqrtf(2.0f);
  int i, j;
  for(j=-OCEANGRID/2; j<=OCEANGRID/2; j++)
  {
    int y = j + OCEANGRID/2;
    for(i=-OCEANGRID/2; i<=OCEANGRID/2; i++)
    {
      int x = i + OCEANGRID/2;
      float rndX = (float)kRnd.GetGauss();
      float rndY = (float)kRnd.GetGauss();
      rndX *= fOneBySqrtTwo * cry_sqrtf(PhillipsSpectrum(2.0f*PI*i, 2.0f*PI*j));
      rndY *= fOneBySqrtTwo * cry_sqrtf(PhillipsSpectrum(2.0f*PI*i, 2.0f*PI*j));

      m_H0X[y][x] = rndX;
      m_H0Y[y][x] = rndY;
    }
  } 

  // precalc length of K
  for(j=-OCEANGRID/2; j<OCEANGRID/2; j++)
  {
    int y = j + OCEANGRID/2;
    for(i=-OCEANGRID/2; i<OCEANGRID/2; i++)
    {
      int x = i + OCEANGRID/2;
      float fKLength = cry_sqrtf(sqrf(2.0f*PI*i) + sqrf(2.0f*PI*j));
      if( fKLength < 1e-8f )
        fKLength = 1e-8f;
      m_aKScale[y][x] = 1.0f / fKLength;
    }
  }

  // init angular frequencies
  for(j=-OCEANGRID/2; j<OCEANGRID/2; j++)
  {
    int y = j + OCEANGRID/2;
    for(i=-OCEANGRID/2; i<OCEANGRID/2; i++)
    {
      int x = i + OCEANGRID/2;
      float fKLength = cry_sqrtf(sqrf(2.0f*PI*i) + sqrf(2.0f*PI*j));
      m_aAngularFreq[y][x] = cry_sqrtf(m_fGravity * fKLength) * cry_tanhf(fKLength * m_fDepth);
    }
  }
}

void CREOcean::FFT( int iDir, float* real, float* imag )
{
  long nn,i,i1,j,k,i2,l,l1,l2;
  float c1,c2,treal,timag,t1,t2,u1,u2,z;
  
  nn = OCEANGRID;
  
  // Do the bit reversal
  i2 = nn >> 1;
  j = 0;
  for( i = 0; i < nn - 1; ++i )
  {
    if( i < j )
    {
      treal = real[ i ];
      timag = imag[ i ];
      real[ i ] = real[ j ];
      imag[ i ] = imag[ j ];
      real[ j ] = treal;
      imag[ j ] = timag;
    }

    k = i2;
    while( k <= j )
    {
      j -= k;
      k >>= 1;
    }

    j += k;
  }
  
  // Compute the FFT
  c1 = -1.0f;
  c2 = 0.0f;
  l2 = 1;
  for(l=0; l<LOG_OCEANGRID; l++)
  {
    l1 = l2;
    l2 <<= 1;
    u1 = 1.0;
    u2 = 0.0;
    for( j = 0; j < l1; ++j )
    {
      for( i = j; i < nn; i += l2 )
      {
        i1 = i + l1;
        t1 = u1 * real[i1] - u2 * imag[i1];
        t2 = u1 * imag[i1] + u2 * real[i1];
        real[i1] = real[i] - t1;
        imag[i1] = imag[i] - t2;
        real[i] += t1;
        imag[i] += t2;
      }

      z =  u1 * c1 - u2 * c2;
      u2 = u1 * c2 + u2 * c1;
      u1 = z;
    }

    c2 = crySqrtf(( 1.0f - c1 ) / 2.0f);
    
    if( 1 == iDir )
    {
      c2 = -c2;
    }

    c1 = crySqrtf(( 1.0f + c1 ) / 2.0f);
  }
  
  // Scaling for forward transform
  if( 1 == iDir )
  {
    for(i=0; i<nn; ++i)
    {
      real[i] /= (float) nn;
      imag[i] /= (float) nn;
    }
  }
}

void FFTSSE_64(float* ar, float* ai);

void CREOcean::FFT2D(int iDir, float cmpX[OCEANGRID][OCEANGRID], float cmpY[OCEANGRID][OCEANGRID])
{
  float real[OCEANGRID];
  float imag[OCEANGRID];


#if !defined(_XBOX) && !defined(WIN64) && !defined(LINUX)
	// NOTE: AMD64 port: implement
  if ((g_CpuFlags & CPUF_SSE) && CRenderer::CV_r_sse && !(((int)&cmpX[0][0]) & 0xf) && !(((int)&cmpY[0][0]) & 0xf) && OCEANGRID == 64)
  {
    FFTSSE_64(&cmpY[0][0], &cmpX[0][0]);
    return;
  }
#endif
  
  int i, j;

  // Transform the rows
  for(j=0; j<OCEANGRID; j++)
  {
    for(i=0; i<OCEANGRID; i++)
    {
      real[i] = cmpX[j][i];
      imag[i] = cmpY[j][i];
    }

    FFT( iDir, real, imag );
    
    for(i=0; i<OCEANGRID; i++)
    {
      cmpX[j][i] = real[i];
      cmpY[j][i] = imag[i];
    }
  }
  
  // Transform the columns
  for(i=0; i<OCEANGRID; i++)
  {
    for(j=0; j<OCEANGRID; j++)
    {
      real[j] = cmpX[j][i];
      imag[j] = cmpY[j][i];
    }

    FFT( iDir, real, imag );

    for(j=0; j<OCEANGRID; j++)
    {
      cmpX[j][i] = real[j];
      cmpY[j][i] = imag[j];
    }
  }
}

void CREOcean::Update( float fTime )
{
  double time0 = 0;
  ticks(time0);

  float fK = 2.0f * PI;
  for(int j=-OCEANGRID/2; j<OCEANGRID/2; j++)
  {
    int jn = j & (OCEANGRID-1);
    for(int i=-OCEANGRID/2; i<OCEANGRID/2; i++)
    {
      int x = i + OCEANGRID/2;
      int y = j + OCEANGRID/2;
      unsigned int val = (int)(m_aAngularFreq[y][x] * 1024.0f / (PI*2) * fTime);
      float fSin = gRenDev->m_RP.m_tSinTable[val&0x3ff];
      float fCos = gRenDev->m_RP.m_tSinTable[(val+512)&0x3ff];

      int x1 = -i + OCEANGRID/2;
      int y1 = -j + OCEANGRID/2;

      float hx = (m_H0X[y][x] + m_H0X[y1][x1]) * fCos -
                 (m_H0Y[y][x] + m_H0Y[y1][x1]) * fSin;

      float hy = (m_H0X[y][x] - m_H0X[y1][x1]) * fSin + 
                 (m_H0Y[y][x] - m_H0Y[y1][x1]) * fCos;

      int in = i & (OCEANGRID-1);

      // update height
      m_HX[jn][in] = hx;
      m_HY[jn][in] = hy;

      // update normal
      float fKx = fK * i;
      float fKy = fK * j;
      m_NX[jn][in] = (-hy * fKx - hx * fKy);
      m_NY[jn][in] = ( hx * fKx - hy * fKy);

      // update displacement vector for choppy waves 
      fKx *= m_aKScale[y][x];
      fKy *= m_aKScale[y][x];
      m_DX[jn][in] = ( hy * fKx + hx * fKy);
      m_DY[jn][in] = (-hx * fKx + hy * fKy);
    }
  }
  unticks(time0);
  m_RS.m_StatsTimeFFTTable = (float)(time0*1000.0*g_SecondsPerCycle);

  ticks(time0);
  FFT2D(-1, m_HX, m_HY);    
  FFT2D(-1, m_NX, m_NY);    
  FFT2D(-1, m_DX, m_DY);    
  unticks(time0);
  m_RS.m_StatsTimeFFT = (float)(time0*1000.0*g_SecondsPerCycle);
}

bool CREOcean::mfCompile(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;
  
  enum {eSeed = 1, eSpeed, eGravity, eDepth, eWindSpeed, eWindDirection, eWaveHeight, eDirectionalDependence, eChoppyWaveFactor, eSuppressSmallWavesFactor};
  static tokenDesc commands[] =
  {
    {eSpeed, "Speed"},
    {eGravity, "Gravity"},
    {eDepth, "Depth"},
    {eWindDirection, "WindDirection"},
    {eWindSpeed, "WindSpeed"},
    {eWaveHeight, "WaveHeight"},
    {eDirectionalDependence, "DirectionalDependence"},
    {eChoppyWaveFactor, "ChoppyWaveFactor"},
    {eSuppressSmallWavesFactor, "SuppressSmallWavesFactor"},
    {0, 0},
  };

  float fWindDirection = 90.0f;
  float fWindSpeed = m_fWindSpeed;
  float fWaveHeight = m_fWaveHeight;
  float fDirectionalDependence = m_fDirectionalDependence;
  float fChoppyWaveFactor = m_fChoppyWaveFactor;
  float fSuppressSmallWavesFactor = m_fSuppressSmallWavesFactor;
  m_fSpeed = 1.0f;
  m_fGravity = 9.81f;
  m_fDepth = 10;

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    data = NULL;
    if (name)
      data = name;
    else
    if (params)
      data = params;
      
    switch (cmd)
    {
      case eGravity: 
        if (!data || !data[0])
        {
          Warning( 0,0,"missing Gravity argument for Ocean Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        m_fGravity = shGetFloat(data);
        break;

      case eDepth: 
        if (!data || !data[0])
        {
          Warning( 0,0,"missing Depth argument for Ocean Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        m_fDepth = shGetFloat(data);
        break;

      case eSpeed: 
        if (!data || !data[0])
        {
          Warning( 0,0,"missing Speed argument for Ocean Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        m_fSpeed = shGetFloat(data);
        break;

      case eWindDirection: 
        if (!data || !data[0])
        {
          Warning( 0,0,"missing WindDirection argument for Ocean Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        fWindDirection = shGetFloat(data);
        break;

      case eWindSpeed: 
        if (!data || !data[0])
        {
          Warning( 0,0,"missing WindSpeed argument for Ocean Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        fWindSpeed = shGetFloat(data);
        break;

      case eWaveHeight: 
        if (!data || !data[0])
        {
          Warning( 0,0,"missing WaveHeight argument for Ocean Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        fWaveHeight = shGetFloat(data);
        break;

      case eDirectionalDependence: 
        if (!data || !data[0])
        {
          Warning( 0,0,"missing DirectionalDependence argument for Ocean Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        fDirectionalDependence = shGetFloat(data);
        break;

      case eChoppyWaveFactor: 
        if (!data || !data[0])
        {
          Warning( 0,0,"missing ChoppyWaveFactor argument for Ocean Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        fChoppyWaveFactor = shGetFloat(data);
        break;

      case eSuppressSmallWavesFactor: 
        if (!data || !data[0])
        {
          Warning( 0,0,"missing SuppressSmallWavesFactor argument for Ocean Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        fSuppressSmallWavesFactor = shGetFloat(data);
        break;
    }
  }

  PostLoad(4357, fWindDirection, fWindSpeed, fWaveHeight, fDirectionalDependence, fChoppyWaveFactor, fSuppressSmallWavesFactor);
  m_CustomTexBind[0] = 0;

#if !defined(PS2) && !defined (GC) && !defined (NULL_RENDERER)
  if (!m_VPs[OVP_NOHEIGHT])
  {
    m_VPs[OVP_NOHEIGHT]      = CVProgram::mfForName("CGVProgOcean_NoHeight");
    m_VPs[OVP_NOHEIGHT_SPL] = CVProgram::mfForName("CGVProgOcean_NoHeight_Splash");

    m_VPs[OVP_HEIGHT]      = CVProgram::mfForName("CGVProgOcean");
    m_VPs[OVP_HEIGHT_SPL] = CVProgram::mfForName("CGVProgOcean_Splash");
  }
  if (!m_VPQ)
    m_VPQ = CVProgram::mfForName("CGVProgOcean_SL");
#endif

  return true;
}

