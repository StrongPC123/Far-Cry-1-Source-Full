/*=============================================================================
  TexManShadows.cpp : Common Texture shadows manager implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/


#include "RenderPCH.h"
#include "../CommonRender.h"

STexPic* STexShadow::m_pBasisTex[2];

#define BASISMAPSIZE 32
STexShadow::STexShadow()
{
}

STexShadow::~STexShadow()
{
  int i;

  for(i=0; i<2; i++)
  {
    m_pHorizonTex[i]->Release(false);
  }
}

bool STexShadow::Init(byte *pHeightMap, int Width, int Height, bool bHeightmap)
{
  int i;
  FLOAT fRange = PI/4;
  int flags = FT_HASALPHA;
  char name[128];

  //now set up the basis maps
  if (!m_pBasisTex[0])
  {
    byte *pBasisMap[8];
    for(i=0; i<8; i++)
    {
      pBasisMap[i] = new BYTE[BASISMAPSIZE*BASISMAPSIZE];
      BuildBasisMap(pBasisMap[i],((FLOAT)(PI*i))/4.0f,fRange);
    }
    UCol *pBasisTex = new UCol[BASISMAPSIZE*BASISMAPSIZE];
    if(!BuildInterleavedMap(pBasisTex,pBasisMap[0],pBasisMap[1],pBasisMap[2],pBasisMap[3],BASISMAPSIZE,BASISMAPSIZE))
      return false;
    sprintf(name, "$AutoBasis_%d", gRenDev->m_TexGenID++);
    m_pBasisTex[0] = gRenDev->m_TexMan->CreateTexture(name, BASISMAPSIZE, BASISMAPSIZE, 1, flags, FT2_NODXT, (byte *)pBasisTex, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);

    if(!BuildInterleavedMap(pBasisTex,pBasisMap[4],pBasisMap[5],pBasisMap[6],pBasisMap[7],BASISMAPSIZE,BASISMAPSIZE))
      return false;
    sprintf(name, "$AutoBasis_%d", gRenDev->m_TexGenID++);
    m_pBasisTex[1] = gRenDev->m_TexMan->CreateTexture(name, BASISMAPSIZE, BASISMAPSIZE, 1, flags, FT2_NODXT, (byte *)pBasisTex, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);

    delete [] pBasisTex;
    for(i=0; i<8; i++)
    {
      delete [] pBasisMap[i];
    }
  }

  byte* pHorizonMap[8];

  /*float *pHeight = NULL;
  float fRangeH;
  if (!bHeightmap)
  {
    int x, y;
    pHeight = new float[Width*Height];
    memset(pHeight, 0, 4*Width*Height);
    int mx = Width-1;
    int my = Height-1;
    for(y=0; y<Height; y++)
    {
      for(x=0; x<Width; x++)
      {
        float fCur = pHeight[y*Width+x];
        byte *pNor = &pHeightMap[y*Width*3+x*3];
        float fDX = ((float)(pNor[2])-128.0f)/127.0f;
        float fDY = ((float)(pNor[1])-128.0f)/127.0f;
        pHeight[y*Width+((x+1)&mx)] = fCur-fDX; 
        pHeight[((y+1)&my)*Width+x] = fCur-fDY; 
      }
    }
    float fMinH = 99999.0f;
    float fMaxH = -99999.0f;
    for(y=0; y<Height; y++)
    {
      for(x=0; x<Width; x++)
      {
        fMinH = Min(pHeight[y*Width+x], fMinH); 
        fMaxH = Max(pHeight[y*Width+x], fMaxH); 
      }
    }
    fRangeH = fMaxH-fMinH;

    {
      byte *data = new byte[Width*Height*4];
      for(y=0; y<Height; y++)
      {
        for(x=0; x<Width; x++)
        {
          byte b = (byte)((pHeight[y*Width+x]-fMinH)/fRangeH*255.0f);
          data[y*Width*4+x*4+0] = b;
          data[y*Width*4+x*4+1] = b;
          data[y*Width*4+x*4+2] = b;
        }
      }
      ::WriteJPG(data, Width, Height, "Heightmap.jpg");
      delete [] data;
    }
  }*/

  //now set up the horizon maps
  for(i=0; i<8; i++)
  {
    pHorizonMap[i] = new BYTE[Width*Height];

    if(pHorizonMap[i] == NULL)
      return false;

    if (bHeightmap)
      BuildHorizonMap_FromHeightmap(pHorizonMap[i], pHeightMap, cry_cosf((FLOAT)(PI*i)/4.0f), cry_sinf((FLOAT)(PI*i)/4.0f), 15, Width, Height, TRUE, TRUE);
    else
      BuildHorizonMap_FromNormalmap(pHorizonMap[i], pHeightMap, cry_cosf((FLOAT)(PI*i)/4.0f), cry_sinf((FLOAT)(PI*i)/4.0f), 15, Width, Height, TRUE, TRUE);
  }

  //SAFE_DELETE_ARRAY(pHeight);

  UCol *pHorizonTex = new UCol[Width*Height];

  //build the interleave maps, use the Alpha to
  //basis maps are not-content specific - they are just LERP tables
  if(!BuildInterleavedMap(pHorizonTex,pHorizonMap[0],pHorizonMap[1],pHorizonMap[2],pHorizonMap[3],Width, Height))
    return false;
  sprintf(name, "$AutoHorizon_%d", gRenDev->m_TexGenID++);
  m_pHorizonTex[0] = gRenDev->m_TexMan->CreateTexture(name, Width, Height, 1, flags, FT2_NODXT, (byte *)pHorizonTex, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);

  if(!BuildInterleavedMap(pHorizonTex,pHorizonMap[4],pHorizonMap[5],pHorizonMap[6],pHorizonMap[7],Width, Height))
    return false;
  sprintf(name, "$AutoHorizon_%d", gRenDev->m_TexGenID++);
  m_pHorizonTex[1] = gRenDev->m_TexMan->CreateTexture(name, Width, Height, 1, flags, FT2_NODXT, (byte *)pHorizonTex, eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);

  delete [] pHorizonTex;
  
  for(i=0; i<8; i++)
  {
    delete [] pHorizonMap[i];
  }

  return true;
}

bool STexShadow::BuildInterleavedMap(UCol *newTex, BYTE *r, BYTE *g, BYTE *b, BYTE *a, INT width, INT height)
{
    BYTE *pBase,*pLine;
    INT x,y;

    pBase = (BYTE *)newTex;

    for(y = 0;y < height;y++)
    {
        pLine = &pBase[y*width*4];
        for(x = 0;x < width;x++)
        {
            if(r)
                pLine[2] = r[y*width + x];
            else
                pLine[2] = 128;
            if(g)
                pLine[1] = g[y*width + x];
            else
                pLine[1] = 128;
            if(b)
                pLine[0] = b[y*width + x];
            else
                pLine[0] = 128;
            if(a)
                pLine[3] = a[y*width + x];
            else
                pLine[3] = 128;
          pLine+=4;
        }
    }

    return TRUE;
}

// this builds a basis map
// the basis maps will be interleaved, or we will pack 3 of them into one texture
// (we could pack 4, but we can't use the alpha channel in our dot multiplies)
// but we will assume that the u,v represent the angle with respect to the center of the
// of the texture (.5,.5), So if we want the basis function for the light vector 
// (1,.5) we'd look at the point (1,.75)  
VOID STexShadow::BuildBasisMap(byte *pBasis, float PrimAngle, float fAngle2)
{
    INT x,y;
    FLOAT fEndX,fEndY,fAngle,fPercent;
    FLOAT u,v,dot,nx,ny,fsq;
    BYTE *pCurLine;

    u = cry_cosf(PrimAngle);
    v = cry_sinf(PrimAngle);

    for(y = 0;y < BASISMAPSIZE;y++)
    {
        pCurLine = &pBasis[y*BASISMAPSIZE];

        for(x = 0;x < BASISMAPSIZE;x++)
        {
            fEndX =  x - .5f*BASISMAPSIZE;
            fEndY =  y - .5f*BASISMAPSIZE;
            
            //take the dot product of the normalized vectors
            fsq = cry_sqrtf(fEndX*fEndX+fEndY*fEndY);
            if(fsq == 0)
            {
                fPercent = 128;
            }
            else
            {
                // we remember our definiton of dot product,
                // cos(Angle) = DotProduct of Normalzed vectors
                // so Angle = acos(DotProduct)
                nx = fEndX/fsq;
                ny = fEndY/fsq;
                dot = nx*u + ny*v;
                if( dot < -1.0f )
                    dot = -1.0f;
                if( dot > 1.0f )
                    dot = 1.0f;
                fAngle = cry_acosf(dot);
                if(fabs(fAngle) < fAngle2)
                {
                    fPercent = 128 + fabsf(fAngle-fAngle2)*127/fAngle2;
                    if(fPercent>255)
                        fPercent=255;
                }
                else
                {
                    fPercent = 128;
                }
            }
            *pCurLine = (BYTE)fPercent;
            pCurLine++;
        }
    }
}

#define MAPHEIGHT  0.04f
#define MAPHEIGHT2 (0.04f*255.0f)

VOID STexShadow::BuildHorizonMap_FromHeightmap(BYTE *pHor, BYTE* pHeight, FLOAT dx, FLOAT dy, INT iStepLength, INT iWidth,INT iHeight,BOOL bWrapU,BOOL bWrapV)
{
    INT x,y;
    INT imx,imy,i;
    FLOAT fpx,fpy;
    BYTE *pCurHorLine;
    FLOAT fMaxAngle,fNewAngle,fDeltaX,fDeltaY,fHeight;

    int mx = iWidth-1;
    int my = iHeight-1;

    for(y = 0;y < iHeight;y++)
    {
        pCurHorLine = &pHor[y*iWidth];
        for(x = 0;x < iWidth;x ++)
        {
            //we assume its completly visible at zero
            fMaxAngle = 0;
            fpx = (FLOAT)x;
            fpy = (FLOAT)y;
            for(i = 0;i < iStepLength;i++)
            {
                fpx += dx;
                fpy += dy;

                imx = (INT) fpx;
                imy = (INT) fpy;
                //don't check against ourselves for blocking
                if(imx == x && imy == y)
                    continue;

                fDeltaX = fpx - x;
                fDeltaY = fpy - y;
                fHeight = pHeight[((imy&my)*iWidth)+(imx&mx)]*MAPHEIGHT-pHeight[y*iWidth+x]*MAPHEIGHT;
                
                //this is the dot prodoct
                //Since we are dotting against the normal, (0,0,1) this is just
                // the Height normalized, or the Height over the length of the vector
                fNewAngle = fHeight/cry_sqrtf(fDeltaX*fDeltaX + fDeltaY*fDeltaY + fHeight*fHeight);
            
                //if we found a further obstruction, use it
                if(fNewAngle > fMaxAngle)
                    fMaxAngle = fNewAngle;
            }

            //centered around 128
            *pCurHorLine = 128 + (BYTE) ((FLOAT)fMaxAngle*127); 
            pCurHorLine++;
        }
    }
}

void STexShadow::BuildHorizonMap_FromNormalmap(BYTE *pHor, byte* pHeight, FLOAT dx, FLOAT dy, INT iStepLength, INT iWidth,INT iHeight,BOOL bWrapU,BOOL bWrapV)
{
  INT x,y;
  INT imx,imy,i;
  FLOAT fpx,fpy;
  BYTE *pCurHorLine;
  FLOAT fMaxAngle,fNewAngle,fHeight,fDeltaX,fDeltaY;

  int mx = iWidth-1;
  int my = iHeight-1;

  for(y=0; y<iHeight; y++)
  {
    pCurHorLine = &pHor[y*iWidth];
    for(x=0; x<iWidth; x++)
    {
      //we assume its completly visible at zero
      fMaxAngle = 0;
      fpx = (FLOAT)x;
      fpy = (FLOAT)y;
      for(i = 0;i < iStepLength;i++)
      {
        fpx += dx;
        fpy += dy;

        imx = (INT) fpx;
        imy = (INT) fpy;
        //don't check against ourselves for blocking
        if(imx == x && imy == y)
            continue;

        fDeltaX = fpx - x;
        fDeltaY = fpy - y;
        fHeight = pHeight[((imy&my)*iWidth*4)+(imx&mx)*4+3]*MAPHEIGHT-pHeight[y*iWidth*4+x*4+3]*MAPHEIGHT;
        
        //this is the dot prodoct
        //Since we are dotting against the normal, (0,0,1) this is just
        // the Height normalized, or the Height over the length of the vector
        fNewAngle = fHeight/cry_sqrtf(fDeltaX*fDeltaX + fDeltaY*fDeltaY + fHeight*fHeight);
    
        //if we found a further obstruction, use it
        if(fNewAngle > fMaxAngle)
          fMaxAngle = fNewAngle;
      }

      //centered around 128
      *pCurHorLine = 128 + (BYTE) ((FLOAT)fMaxAngle*127); 
      pCurHorLine++;
    }
  }
}
