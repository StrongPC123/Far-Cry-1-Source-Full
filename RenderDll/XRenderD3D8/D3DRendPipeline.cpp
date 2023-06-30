/*=============================================================================
  D3DRendPipeline.cpp : Direct3D specific rendering using shaders pipeline.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

    Revision history:
  		* Created by Honitch Andrey
    
=============================================================================*/

#include "stdafx.h"
#include "DriverD3D8.h"
#include "..\common\shadow_renderer.h"
#include "D3DPShaders.h"
#include "D3DCGVProgram.h"
#include "I3dengine.h"
#include "CryHeaders.h"

//============================================================================================
// Shaders rendering
//============================================================================================

//============================================================================================
// Init Shaders rendering

void CD3D8Renderer::EF_InitRandTables()
{
  int i;
  float f;

  for (i=0; i<256; i++)
  {
    f = (float)rand() / 32767.0f;
    m_RP.m_tRandFloats[i] = f + f - 1.0f;

    m_RP.m_tRandBytes[i] = (byte)((float)rand() / 32767.0f * 255.0f);
  }
}

void CD3D8Renderer::EF_InitWaveTables()
{
  int i;
  
  //Init wave Tables
  for (i=0; i<1024; i++)
  {
    float f = (float)i;
    
    m_RP.m_tSinTable[i] = (float)sin(f * (360.0/1023.0) * M_PI / 180.0);
    m_RP.m_tHalfSinTable[i] = (float)sin(f * (360.0/1023.0) * M_PI / 180.0);
    if (m_RP.m_tHalfSinTable[i] < 0)
      m_RP.m_tHalfSinTable[i] = 0;
    m_RP.m_tCosTable[i] = (float)cos(f * (360.0/1023.0) * M_PI / 180.0);
    m_RP.m_tHillTable[i] = (float)sin(f * (180.0/1023.0) * M_PI / 180.0);
    
    if (i < 512)
      m_RP.m_tSquareTable[i] = 1.0f;
    else
      m_RP.m_tSquareTable[i] = -1.0f;
    
    m_RP.m_tSawtoothTable[i] = f / 1024.0f;
    m_RP.m_tInvSawtoothTable[i] = 1.0f - m_RP.m_tSawtoothTable[i];
    
    if (i < 512)
    {
      if (i < 256)
        m_RP.m_tTriTable[i] = f / 256.0f;
      else
        m_RP.m_tTriTable[i] = 1.0f - m_RP.m_tTriTable[i-256];
    }
    else
      m_RP.m_tTriTable[i] = 1.0f - m_RP.m_tTriTable[i-512];
  }
}

void CD3D8Renderer::EF_InitEvalFuncs(int num)
{
  switch(num)
  {
    case 0:
      m_RP.m_pCurFuncs = &m_RP.m_EvalFuncs_C;
      break;
    default:
    case 1:
      m_RP.m_pCurFuncs = &m_RP.m_EvalFuncs_RE;
      break;
  }
}

int CD3D8Renderer::EF_RegisterFogVolume(float fMaxFogDist, float fFogLayerZ, CFColor color)
{
  SMFog Fog;
  memset(&Fog,0,sizeof(Fog));

  Fog.m_fMaxDist = fMaxFogDist;
  Fog.m_FogInfo.m_FogColor = color;
  Fog.m_Dist = fFogLayerZ;
  Fog.m_Color = color;
  Fog.m_Color.a = 1.0f;
  //Fog.m_FogInfo.m_FogColor = m_FogColor;
  Fog.m_Normal = Vec3d(0,0,1);

  m_RP.m_FogVolumes.AddElem(Fog);

  return m_RP.m_FogVolumes.Num()-1;
}

void CD3D8Renderer::EF_InitFogVolumes()
{
  SMFog Fog;
  memset(&Fog,0,sizeof(Fog));
  m_RP.m_FogVolumes.AddElem(Fog); // register fake zero element
}


struct SVB
{
  int Stride;
  int OffsD;
  int OffsT;
  int OffsN;
  int FVF;
  UStaticVB mVB;
};


static SVB sSVB[] = 
{
  {STRIDE_D_1T, 12, 16, 0, D3DFVF_VERTEX_D_1T},
  {STRIDE_D_2T, 12, 16, 0, D3DFVF_VERTEX_D_2T},
  {STRIDE_D_3T, 12, 16, 0, D3DFVF_VERTEX_D_3T},
  {STRIDE_D_4T, 12, 16, 0, D3DFVF_VERTEX_D_4T},
  {STRIDE_N_D_1T, 24, 28, 12, D3DFVF_VERTEX_N_D_1T},
  {STRIDE_N_D_2T, 24, 28, 12, D3DFVF_VERTEX_N_D_2T},
  {STRIDE_N_D_3T, 24, 28, 12, D3DFVF_VERTEX_N_D_3T},
  {STRIDE_N_D_4T, 24, 28, 12, D3DFVF_VERTEX_N_D_4T},
  {STRIDE_TR_D_1T, 16, 24, 0, D3DFVF_TRVERTEX_D_1T},
  {STRIDE_TR_D_2T, 16, 24, 0, D3DFVF_TRVERTEX_D_2T},
  {STRIDE_TR_D_3T, 16, 24, 0, D3DFVF_TRVERTEX_D_3T},
  {STRIDE_TR_D_4T, 16, 24, 0, D3DFVF_TRVERTEX_D_4T},
  {0}
};

void CD3D8Renderer::EF_InitD3DFixedPipeline()
{
  if (m_RP.m_D3DFixedPipeline[0][0].m_Declaration.Num())
    return;
  
  int n = 0;
  {
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = sSVB[0].FVF;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT4)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  {
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = sSVB[1].FVF;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT4)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD1, D3DVSDT_FLOAT4)); // texture coords 1
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  {
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = sSVB[2].FVF;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT4)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD1, D3DVSDT_FLOAT4)); // texture coords 1
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD2, D3DVSDT_FLOAT4)); // texture coords 2
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  {
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = sSVB[3].FVF;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT4)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD1, D3DVSDT_FLOAT4)); // texture coords 1
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD2, D3DVSDT_FLOAT4)); // texture coords 2
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD3, D3DVSDT_FLOAT4)); // texture coords 3
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  {
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = sSVB[4].FVF;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3));    // normal
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT4)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  {
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = sSVB[5].FVF;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3));    // normal
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT4)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD1, D3DVSDT_FLOAT4)); // texture coords 1
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  {
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = sSVB[6].FVF;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3));    // normal
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT4)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD1, D3DVSDT_FLOAT4)); // texture coords 1
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD2, D3DVSDT_FLOAT4)); // texture coords 2
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  {
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = sSVB[7].FVF;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3));    // normal
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT4)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD1, D3DVSDT_FLOAT4)); // texture coords 1
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD2, D3DVSDT_FLOAT4)); // texture coords 2
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD3, D3DVSDT_FLOAT4)); // texture coords 3
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  {
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = sSVB[8].FVF;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT4)); // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT4)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  {
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = sSVB[9].FVF;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT4)); // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT4)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD1, D3DVSDT_FLOAT4)); // texture coords 1
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  {
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = sSVB[10].FVF;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT4)); // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT4)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD1, D3DVSDT_FLOAT4)); // texture coords 1
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD2, D3DVSDT_FLOAT4)); // texture coords 2
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  {
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = sSVB[11].FVF;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT4)); // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT4)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD1, D3DVSDT_FLOAT4)); // texture coords 1
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD2, D3DVSDT_FLOAT4)); // texture coords 2
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD3, D3DVSDT_FLOAT4)); // texture coords 3
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }

  // base formats declarations (stream 0)
  n = 16;
  {
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = 0;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  { // VERTEX_FORMAT_P3F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  { // VERTEX_FORMAT_P3F_COL4UB
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_DIFFUSE;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  { // VERTEX_FORMAT_P3F_TEX2F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_TEX1;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  { // VERTEX_FORMAT_P3F_COL4UB_TEX2F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  { // VERTEX_FORMAT_TRP3F_COL4UB_TEX2F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT4));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  { // VERTEX_FORMAT_P3F_COL4UB_COL4UB
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_SPECULAR, D3DVSDT_D3DCOLOR));   // specular
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  { // VERTEX_FORMAT_P3F_N
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_NORMAL;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3));  // normal
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  { // VERTEX_FORMAT_P3F_N_COL4UB
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3));  // normal
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  { // VERTEX_FORMAT_P3F_N_TEX2F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3));  // normal
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }
  { // VERTEX_FORMAT_P3F_N_COL4UB_TEX2F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1;
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_STREAM(0));
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));  // position
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3));  // normal
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));   // diffuse
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2)); // texture coords 0
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.AddElem(D3DVSD_END());
    n++;
  }

  // Additional streams:
  // stream 1 (tangent vectors)
  for (int i=0; i<32; i++)
  {
    if (!m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num())
      continue;
    m_RP.m_D3DFixedPipeline[1][i].m_Handle = m_RP.m_D3DFixedPipeline[0][i].m_Handle;
    for (int j=0; j<m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num()-1; j++)
    {
      m_RP.m_D3DFixedPipeline[1][i].m_Declaration.AddElem(m_RP.m_D3DFixedPipeline[0][i].m_Declaration[j]);
    }
    m_RP.m_D3DFixedPipeline[1][i].m_Declaration.AddElem(D3DVSD_STREAM(1));
    m_RP.m_D3DFixedPipeline[1][i].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD2, D3DVSDT_FLOAT3)); // tangent
    m_RP.m_D3DFixedPipeline[1][i].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD3, D3DVSDT_FLOAT3)); // binormals
    m_RP.m_D3DFixedPipeline[1][i].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_BLENDWEIGHT, D3DVSDT_FLOAT3)); // tnormals
    m_RP.m_D3DFixedPipeline[1][i].m_Declaration.AddElem(D3DVSD_END());
  }

  // stream 2 (LM texcoords)
  for (int i=0; i<32; i++)
  {
    if (!m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num())
      continue;
    for (int j=0; j<m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num()-1; j++)
    {
      m_RP.m_D3DFixedPipeline[2][i].m_Declaration.AddElem(m_RP.m_D3DFixedPipeline[0][i].m_Declaration[j]);
    }
    m_RP.m_D3DFixedPipeline[2][i].m_Declaration.AddElem(D3DVSD_STREAM(2));
    m_RP.m_D3DFixedPipeline[2][i].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD1, D3DVSDT_FLOAT2)); // LM texcoords
    m_RP.m_D3DFixedPipeline[2][i].m_Declaration.AddElem(D3DVSD_END());
  }

  // stream 1 and 2 (tangent vectors and LM texcoords)
  for (int i=0; i<32; i++)
  {
    if (!m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num())
      continue;
    for (int j=0; j<m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num()-1; j++)
    {
      m_RP.m_D3DFixedPipeline[3][i].m_Declaration.AddElem(m_RP.m_D3DFixedPipeline[0][i].m_Declaration[j]);
    }
    m_RP.m_D3DFixedPipeline[3][i].m_Declaration.AddElem(D3DVSD_STREAM(1));
    m_RP.m_D3DFixedPipeline[3][i].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD2, D3DVSDT_FLOAT3)); // tangent
    m_RP.m_D3DFixedPipeline[3][i].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD3, D3DVSDT_FLOAT3)); // binormals
    m_RP.m_D3DFixedPipeline[3][i].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_BLENDWEIGHT, D3DVSDT_FLOAT3)); // tnormals
    m_RP.m_D3DFixedPipeline[3][i].m_Declaration.AddElem(D3DVSD_STREAM(2));
    m_RP.m_D3DFixedPipeline[3][i].m_Declaration.AddElem(D3DVSD_REG(D3DVSDE_TEXCOORD1, D3DVSDT_FLOAT2)); // LM texcoord
    m_RP.m_D3DFixedPipeline[3][i].m_Declaration.AddElem(D3DVSD_END());
  }

  m_MaxVertBufferSize = (int)10*1024*1024;
  m_CurVertBufferSize = 0;
}

_inline static void *sAlign0x20(byte *vrts)
{
  int b = (int)vrts;

  if (!(b & 0x1f))
    return vrts;

  b = (b+0x20)&0xffffffe0;

  return (void *)b;
}

void CD3D8Renderer::EF_Init()
{
  bool nv = 0;

  m_RP.m_pCurFuncs = NULL;
  m_RP.m_MaxVerts = CV_d3d8_rb_verts;
  m_RP.m_MaxTris = CV_d3d8_rb_tris;

  iLog->Log("\nAllocate render buffer (%d verts, %d tris)...\n", m_RP.m_MaxVerts, m_RP.m_MaxTris);

  int n = 0;

  n += STRIDE_N_D_4T * m_RP.m_MaxVerts + 32;
  
  //m_RP.mFogVertValues
  n += sizeof(float)*m_RP.m_MaxVerts+32;
  //m_RP.mBaseTexCoordPointer
  n += sizeof(SMRendTexVert)*m_RP.m_MaxVerts+32;
  //m_RP.mLMTexCoordPointer
  n += sizeof(SMRendTexVert)*m_RP.m_MaxVerts+32;
  //m_RP.mClientColors
  n += sizeof(bvec4)*m_RP.m_MaxVerts+32;

  //m_RP.mRendIndices;
  n += sizeof(ushort)*3*m_RP.m_MaxTris+32;

  int i;
  if (GetFeatures() & RFT_BUMP_DOT3)
  {
    //m_RP.mBinormals
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    //m_RP.mTangents
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    //m_RP.mTNormals
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    for (i=0; i<4; i++)
    {
      //m_RP.mHalfAngleVectors
      n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
      //m_RP.mLightVectors
      n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    }
    //m_RP.mAttenuation
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    //m_RP.mLAttenSpec0
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    //m_RP.mLAttenSpec1
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
  }

  byte *buf = new byte [n];
  m_SysArray = buf;
  if (!buf)
    iConsole->Exit("Can't allocate buffers for RB");

  memset(buf, 0, n);

  m_RP.m_Ptr.Ptr = sAlign0x20(buf);
  buf += STRIDE_N_D_4T * m_RP.m_MaxVerts + 32;
  m_RP.m_RendIndices = (ushort *)sAlign0x20(buf);
  buf += sizeof(ushort)*3*m_RP.m_MaxTris+32;
  m_RP.m_pClientColors = (bvec4 *)sAlign0x20(buf);
  buf += sizeof(bvec4)*m_RP.m_MaxVerts+32;
  m_RP.m_pBaseTexCoordPointer = (SMRendTexVert *)sAlign0x20(buf);
  buf += sizeof(SMRendTexVert)*m_RP.m_MaxVerts+32;
  m_RP.m_pLMTexCoordPointer = (SMRendTexVert *)sAlign0x20(buf);
  buf += sizeof(SMRendTexVert)*m_RP.m_MaxVerts+32;
  m_RP.m_pFogVertValues = (float *)sAlign0x20(buf);
  buf += sizeof(float)*m_RP.m_MaxVerts+32;
  if (GetFeatures() & RFT_BUMP_DOT3)
  {
    m_RP.m_pBinormals = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    m_RP.m_pTangents = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    m_RP.m_pTNormals = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    for (i=0; i<4; i++)
    {
      m_RP.m_pLightVectors[i] = (Vec3d *)sAlign0x20(buf);
      buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
      m_RP.m_pHalfAngleVectors[i] = (Vec3d *)sAlign0x20(buf);
      buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    }
    m_RP.m_pAttenuation = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    m_RP.m_pLAttenSpec0 = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    m_RP.m_pLAttenSpec1 = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
  }
  m_RP.m_IndexBuf = new StaticIB <ushort>(m_pd3dDevice, m_RP.m_MaxTris*3);

  sSVB[0].mVB.VBPtr_D_1T   = new StaticVB <SPipeVertex_D_1T>(m_pd3dDevice, D3DFVF_VERTEX_D_1T, m_RP.m_MaxVerts);
  sSVB[1].mVB.VBPtr_D_2T   = new StaticVB <SPipeVertex_D_2T>(m_pd3dDevice, D3DFVF_VERTEX_D_2T, m_RP.m_MaxVerts);
  sSVB[2].mVB.VBPtr_D_3T   = new StaticVB <SPipeVertex_D_3T>(m_pd3dDevice, D3DFVF_VERTEX_D_3T, m_RP.m_MaxVerts);
  sSVB[3].mVB.VBPtr_D_4T   = new StaticVB <SPipeVertex_D_4T>(m_pd3dDevice, D3DFVF_VERTEX_D_4T, m_RP.m_MaxVerts);
  sSVB[4].mVB.VBPtr_N_D_1T   = new StaticVB <SPipeVertex_N_D_1T>(m_pd3dDevice, D3DFVF_VERTEX_N_D_1T, m_RP.m_MaxVerts);
  sSVB[5].mVB.VBPtr_N_D_2T   = new StaticVB <SPipeVertex_N_D_2T>(m_pd3dDevice, D3DFVF_VERTEX_N_D_2T, m_RP.m_MaxVerts);
  sSVB[6].mVB.VBPtr_N_D_3T   = new StaticVB <SPipeVertex_N_D_3T>(m_pd3dDevice, D3DFVF_VERTEX_N_D_3T, m_RP.m_MaxVerts);
  sSVB[7].mVB.VBPtr_N_D_4T   = new StaticVB <SPipeVertex_N_D_4T>(m_pd3dDevice, D3DFVF_VERTEX_N_D_4T, m_RP.m_MaxVerts);
  sSVB[8].mVB.VBPtr_TR_D_1T   = new StaticVB <SPipeTRVertex_D_1T>(m_pd3dDevice, D3DFVF_TRVERTEX_D_1T, m_RP.m_MaxVerts);
  sSVB[9].mVB.VBPtr_TR_D_2T   = new StaticVB <SPipeTRVertex_D_2T>(m_pd3dDevice, D3DFVF_TRVERTEX_D_2T, m_RP.m_MaxVerts);
  sSVB[10].mVB.VBPtr_TR_D_3T   = new StaticVB <SPipeTRVertex_D_3T>(m_pd3dDevice, D3DFVF_TRVERTEX_D_3T, m_RP.m_MaxVerts);
  sSVB[11].mVB.VBPtr_TR_D_4T   = new StaticVB <SPipeTRVertex_D_4T>(m_pd3dDevice, D3DFVF_TRVERTEX_D_4T, m_RP.m_MaxVerts);

  // Used for sprites
  m_RP.m_VBSprites.VBPtr_D_T2F   = new StaticVB <struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F>(m_pd3dDevice, D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1, m_RP.m_MaxVerts);

  EF_InitWaveTables();
  EF_InitRandTables();
  EF_InitEvalFuncs(0);
  EF_InitFogVolumes();
  EF_InitD3DFixedPipeline();

  for (int i=0; i<VERTEX_FORMAT_NUMS; i++)
  {
    for (int j=0; j<VERTEX_FORMAT_NUMS; j++)
    {
      SVertBufComps Cps[2];
      GetVertBufComps(&Cps[0], i);
      GetVertBufComps(&Cps[1], j);

      bool bNeedTC = Cps[1].m_bHasTC | Cps[0].m_bHasTC;
      bool bNeedCol = Cps[1].m_bHasColors | Cps[0].m_bHasColors;
      bool bNeedSecCol = Cps[1].m_bHasSecColors | Cps[0].m_bHasSecColors;
      bool bNeedNormals = Cps[1].m_bHasNormals | Cps[0].m_bHasNormals;
      m_RP.m_VFormatsMerge[i][j] = VertFormatForComponents(bNeedCol, bNeedSecCol, bNeedNormals, bNeedTC);
    }
  }

//==================================================

  CCObject::m_Waves.Create(32);
  m_RP.m_VisObjects = new CCObject *[MAX_REND_OBJECTS];

  if (!m_RP.m_TempObjects.Num())
    m_RP.m_TempObjects.Reserve(MAX_REND_OBJECTS);
  if (!m_RP.m_Objects.Num())
  {
    m_RP.m_Objects.Reserve(MAX_REND_OBJECTS);
    m_RP.m_Objects.SetUse(1);
    SAFE_DELETE_ARRAY(m_RP.m_ObjectsPool);
    m_RP.m_nNumObjectsInPool = 384;
    m_RP.m_ObjectsPool = new CCObject[m_RP.m_nNumObjectsInPool];
    for (int i=0; i<m_RP.m_nNumObjectsInPool; i++)
    {
      m_RP.m_TempObjects[i] = &m_RP.m_ObjectsPool[i];
      m_RP.m_TempObjects[i]->Init();
      m_RP.m_TempObjects[i]->m_Color = Col_White;
      m_RP.m_TempObjects[i]->m_ObjFlags = 0;
      m_RP.m_TempObjects[i]->m_Matrix.Identity();
      m_RP.m_TempObjects[i]->m_RenderState = 0;
    }
    m_RP.m_VisObjects[0] = &m_RP.m_ObjectsPool[0];
  }

  m_RP.m_DLights.Create(64);
  m_RP.m_DLights.SetUse(0);

  m_RP.m_pREGlare = (CREGlare *)EF_CreateRE(eDATA_Glare);
}

void CD3D8Renderer::EF_PipelineShutdown()
{
  int i, j;

  i = 0;
  while (sSVB[i].Stride)
  {
    SAFE_DELETE (sSVB[i].mVB.VBPtr_D_1T);
    i++;
  }
  CCObject::m_Waves.Free();
  SAFE_DELETE_ARRAY(m_RP.m_VisObjects);
  SAFE_DELETE_ARRAY(m_SysArray);  

  m_RP.m_DLights.Free();
  m_RP.m_FogVolumes.Free();
  SAFE_RELEASE(m_RP.m_pREGlare);

  for (i=0; i<CREClientPoly2D::mPolysStorage.GetSize(); i++)
  {
    SAFE_RELEASE(CREClientPoly2D::mPolysStorage[i]);
  }
  CREClientPoly2D::mPolysStorage.Free();

  for (j=0; j<4; j++)
  {
    for (i=0; i<CREClientPoly::mPolysStorage[j].GetSize(); i++)
    {
      SAFE_RELEASE(CREClientPoly::mPolysStorage[j][i]);
    }
    CREClientPoly::mPolysStorage[j].Free();
  }

  EF_Release(EFRF_VSHADERS);
  CCGVProgram_D3D::mfDeleteSharedScripts();
}

void CD3D8Renderer::EF_Release(int nFlags)
{
  int i;

  if (nFlags & EFRF_VSHADERS)
  {
    for (i=0; i<CVProgram::m_VPrograms.Num(); i++)
    {
      CVProgram::m_VPrograms[i]->mfReset();
    }
  }
  if (nFlags & EFRF_PSHADERS)
  {
    for (i=0; i<CPShader::m_PShaders.Num(); i++)
    {
      CPShader::m_PShaders[i]->mfReset();
    }
  }
}

void CD3D8Renderer::EF_ClearBuffer(bool bForce, float *Colors)
{
  if (m_bWasCleared && !bForce)
    return;

  m_bWasCleared = true;

  DWORD cColor;
  if (Colors)
    cColor = D3DRGBA(Colors[0], Colors[1], Colors[2], Colors[3]);
  else
  if(m_polygon_mode==R_WIREFRAME_MODE)
  {
    m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
    cColor = D3DRGBA(0.25,0.5,1,0);
  }
  else
  {
    m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
    cColor = D3DRGBA(m_vClearColor[0], m_vClearColor[1], m_vClearColor[2], 0);
  }
  
  if (m_sbpp)
    m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, cColor, 1.0f, 0);
  else
    m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, cColor, 1.0f, 0);
  
}

//==========================================================================

void CD3D8Renderer::EF_SetCameraInfo()
{
  m_RP.m_ViewOrg = m_cam.GetPos();

  float fm[16];
  GetModelViewMatrix(fm);
  m_ViewMatrix = Matrix(fm);
  m_CameraMatrix = m_ViewMatrix;
  Matrix m = m_CameraMatrix;
  m.Transpose();

  m_RP.m_CamVecs[0][0] = -m(2,0);
  m_RP.m_CamVecs[0][1] = -m(2,1);
  m_RP.m_CamVecs[0][2] = -m(2,2);

  m_RP.m_CamVecs[1][0] = m(1,0);
  m_RP.m_CamVecs[1][1] = m(1,1);
  m_RP.m_CamVecs[1][2] = m(1,2);

  m_RP.m_CamVecs[2][0] = m(0,0);
  m_RP.m_CamVecs[2][1] = m(0,1);
  m_RP.m_CamVecs[2][2] = m(0,2);

  GetProjectionMatrix(&m_ProjMatrix(0,0));
}

void CCObject::CalcInvertMatrix()
{
  if (m_InvMatrixId)
    return;
  int n = m_InvMatrices.Num();
  m_InvMatrices.AddIndex(1);
  m_InvMatrixId = n;

  CRenderer *rd = gRenDev;
  Matrix44 &m = m_InvMatrices[m_InvMatrixId];

  if (m_ObjFlags & FOB_TRANS_USEROTATE)
  {
    D3DXMatrixInverse((D3DXMATRIX *)m.GetData(), NULL, (D3DXMATRIX *)m_Matrix.GetData());
  }
  else
  if (m_ObjFlags & FOB_TRANS_USESCALE)
  {
    m = m_Matrix;
    float fiScaleX = 1.0f / m(0,0);
    float fiScaleY = 1.0f / m(1,1);
    float fiScaleZ = 1.0f / m(2,2);
    m(0,0) = fiScaleX;
    m(1,1) = fiScaleY;
    m(2,2) = fiScaleZ;
    m(3,0) = -m(3,0) * fiScaleX;
    m(3,1) = -m(3,1) * fiScaleY;
    m(3,2) = -m(3,2) * fiScaleZ;
  }
  else
  if (m_ObjFlags & FOB_TRANS_USETRANSLATE)
  {
    m = m_Matrix;
    m(3,0) = -m(3,0);
    m(3,1) = -m(3,1);
    m(3,2) = -m(3,2);
  }
  else
    m = m_Matrix;
}


void CCObject::CalcMatrix()
{
  if (m_ObjFlags & FOB_MATRIXCALCULATED)
    return;
  m_ObjFlags |= FOB_MATRIXCALCULATED;
  if (m_ObjFlags & FOB_USEMATRIX)
  {
    m_ObjFlags |= FOB_TRANS_MASK;
    return;
  }
  m_ObjFlags &= ~FOB_TRANS_MASK;

  D3DXMATRIX mat, ma;
  D3DXMatrixIdentity(&ma);
  //if (obj->m_Scale != Vec3d(1.f,1.f,1.f))
  if ( !IsEquivalent(m_Scale,Vec3d(1.f,1.f,1.f)) )
  {
    m_ObjFlags |= FOB_TRANS_USESCALE;
    D3DXMatrixScaling(&mat, m_Scale.x, m_Scale.y, m_Scale.z);    
    D3DXMatrixMultiply(&ma, &ma, &mat);
  }
  if (m_Angs.x)
  {
    m_ObjFlags |= FOB_TRANS_USEROTATE;
    D3DXMatrixRotationX(&mat, m_Angs.x*M_PI/180.0f);    
    D3DXMatrixMultiply(&ma, &ma, &mat);
  }
  if (m_Angs.y)
  {
    m_ObjFlags |= FOB_TRANS_USEROTATE;
    D3DXMatrixRotationY(&mat, m_Angs.y*M_PI/180.0f);    
    D3DXMatrixMultiply(&ma, &ma, &mat);
  }
  if (m_Angs.z)
  {
    m_ObjFlags |= FOB_TRANS_USEROTATE;
    D3DXMatrixRotationZ(&mat, m_Angs.z*M_PI/180.0f);    
    D3DXMatrixMultiply(&ma, &ma, &mat);
  }
  // if (obj->m_Trans != Vec3d(0,0,0))
  if ( !IsEquivalent(m_Trans,Vec3d(0,0,0)) )
  {
    m_ObjFlags |= FOB_TRANS_USETRANSLATE;
    D3DXMatrixTranslation(&mat, m_Trans.x, m_Trans.y, m_Trans.z);
    D3DXMatrixMultiply(&ma, &ma, &mat);
  }
  cryMemcpy(&m_Matrix(0,0), &ma, sizeof(float)*4*4);
}

void CD3D8Renderer::EF_SetObjectTransform(CCObject *obj)
{
  obj->CalcMatrix();

  if (obj->m_ObjFlags & FOB_TRANS_MASK)
    D3DXMatrixMultiply((D3DXMATRIX *)&m_ViewMatrix(0,0), (D3DXMATRIX *)&m_RP.m_pCurObject->m_Matrix(0,0), (D3DXMATRIX *)&m_CameraMatrix(0,0));
  else
    m_ViewMatrix = m_CameraMatrix;
  m_matView->LoadMatrix((D3DXMATRIX *)&m_ViewMatrix(0,0));
  m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop());
  m_bInvertedMatrix = false;  
}

bool CD3D8Renderer::EF_ObjectChange(SShader *Shader, int nObject, CRendElement *pRE)
{
  CCObject *obj = m_RP.m_VisObjects[nObject];
  if ((obj->m_ObjFlags & FOB_NEAREST) && ((m_RP.m_PersFlags & RBPF_DONTDRAWNEAREST) || CV_r_nodrawnear))
    return false;

  if (Shader)
  {
  // if (m_RP.m_pIgnoreObject && !(Shader->m_Flags & EF_SKY) && m_RP.m_pIgnoreObject->m_Trans == obj->m_Trans)
	  if (m_RP.m_pIgnoreObject && !(Shader->m_Flags & EF_SKY) && IsEquivalent(m_RP.m_pIgnoreObject->m_Trans,obj->m_Trans))
    {
      if (!m_RP.m_pIgnoreShader || m_RP.m_pIgnoreShader == Shader)
        return false;
    }
  }

  if (obj == m_RP.m_pPrevObject)
    return true;

  m_RP.m_FrameObject++;
  m_RP.m_pCurObject = obj;

  bool bTheSameCM = false;
  if (m_RP.m_pPrevObject && (m_RP.m_pPrevObject->m_ObjFlags & FOB_CUBE_MASK))
  {
    bTheSameCM = (obj && (obj->m_ObjFlags & FOB_CUBE_MASK) == (m_RP.m_pPrevObject->m_ObjFlags & FOB_CUBE_MASK));
    if (!bTheSameCM)
    {
      bool bNeedClear;
      if (obj && (obj->m_ObjFlags & FOB_CUBE_MASK))
        bNeedClear = false;
      else
        bNeedClear = true;
      m_TexMan->EndCubeSide(m_RP.m_pPrevObject, bNeedClear);
    }
  }
  
  int flags = 0;
  if (obj->m_VisId)
  {
    m_RP.m_PS.m_NumRendObjects++;
    if (obj->m_Color.a != 1.0f)
      obj->m_ObjFlags |= FOB_HASALPHA;
    if (obj->m_fBending)
      obj->m_ObjFlags |= FOB_BENDED;

    // Skinning
    if (obj->m_pCharInstance && !CV_r_character_nodeform)
    {
      double time0 = 0;
      ticks(time0);
      m_RP.m_PS.m_NumRendSkinnedObjects++;
      CREOcLeaf *pREOCL = (CREOcLeaf *)pRE;
      SShader *sh = m_RP.m_pShader;
      m_RP.m_pShader = Shader;
      int nFlags = 0;
      if (Shader->m_Flags & EF_NEEDNORMALS)
        nFlags |= FHF_NORMALSUSED;
      if (Shader->m_Flags & EF_NEEDTANGENTS)
        nFlags |= FHF_TANGENTSUSED;
      pRE->mfCheckUpdate(nFlags);
      m_RP.m_pShader = sh;
      CLeafBuffer *pLB = pREOCL->m_pBuffer->GetVertexContainer();
      bool bForceUpdate = (pLB->m_UpdateFrame == GetFrameID());
      obj->m_pCharInstance->ProcessSkinning(obj->m_Matrix, obj->m_nTemplId, obj->m_nLod, bForceUpdate);
      unticks(time0);
      m_RP.m_PS.m_fSkinningTime += (float)(time0*1000.0*m_RP.m_SecondsPerCycle);
    }

    if(obj->m_ObjFlags & FOB_NEAREST)
      flags |= RBF_NEAREST;

    if ((flags ^ m_RP.m_Flags) & RBF_NEAREST)
    {
      if (flags & RBF_NEAREST)
      {
        CCamera Cam = GetCamera();
        m_RP.m_PrevCamera = Cam;
        Cam.SetZMin(0.01f);
        Cam.SetZMax(40.0f);
        
        // set nice fov for weapons  
        static float weapon_fov_k = 0.6666f;//iSystem->GetIniVar("weapon_fov_k","engine.ini",0.6666f);
        Cam.SetFov( Cam.GetFov() * weapon_fov_k ); 
        SetCamera(Cam);
        m_Viewport.MaxZ = 0.1f;
        m_pd3dDevice->SetViewport(&m_Viewport);
      }
      else
      {
        SetCamera(m_RP.m_PrevCamera);
        m_Viewport.MaxZ = 1.0f;
        m_pd3dDevice->SetViewport(&m_Viewport);
      }

      m_RP.m_Flags &= ~RBF_NEAREST;
      m_RP.m_Flags |= (flags & RBF_NEAREST);
    }
    if ((obj->m_ObjFlags & FOB_CUBE_MASK) && !bTheSameCM)
      m_TexMan->StartCubeSide(obj);
    
    EF_SetObjectTransform(obj);
  }
  else
  {
    if (m_RP.m_Flags & RBF_NEAREST)
    {
      SetCamera(m_RP.m_PrevCamera);
      m_Viewport.MaxZ = 1.0f;
      m_pd3dDevice->SetViewport(&m_Viewport);
      m_RP.m_Flags &= ~RBF_NEAREST;
    }
    m_RP.m_pCurObject->m_Matrix.Identity();
    m_ViewMatrix = m_CameraMatrix;
    m_matView->LoadMatrix((D3DXMATRIX *)&m_CameraMatrix(0,0));
    m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop());
    m_bInvertedMatrix = false;  
  }
  m_RP.m_pPrevObject = m_RP.m_pCurObject;

  return true;
}


void CD3D8Renderer::EF_SetClipPlane (bool bEnable, float *pPlane, bool bRefract)
{
}

//==============================================================================
// Shader Pipeline

void CD3D8Renderer::EF_CheckOverflow(int nVerts, int nInds, CRendElement *re)
{
  if (m_RP.m_pRE || (m_RP.m_RendNumVerts+nVerts >= m_RP.m_MaxTris || m_RP.m_RendNumIndices+nInds >= m_RP.m_MaxTris*3))
  {
    m_RP.m_pRenderFunc();
    if (nVerts >= m_RP.m_MaxVerts)
    {
      iLog->Log("CD3D8Renderer::EF_CheckOverflow: numVerts > MAX (%d > %d)\n", nVerts, m_RP.m_MaxVerts);
      nVerts = m_RP.m_MaxVerts;
    }
    if (nInds >= m_RP.m_MaxTris*3)
    {
      iLog->Log("CD3D8Renderer::EF_CheckOverflow: numIndices > MAX (%d > %d)\n", nInds, m_RP.m_MaxTris*3);
      nInds = m_RP.m_MaxTris*3;
    }
    EF_Start(m_RP.m_pShader, m_RP.m_pStateShader, m_RP.m_pShaderResources, m_RP.m_pFogVolume ? (m_RP.m_pFogVolume-&m_RP.m_FogVolumes[0]) : 0, re);
  }
}

//=======================================================================


void CD3D8Renderer::EF_Eval_DeformVerts(TArray<SDeform>* Defs)
{
  int i;

  if (!Defs)
    return;

  for (i=0; i<Defs->Num(); i++)
  {
    SDeform *df = &Defs->Get(i);
    switch (df->m_eType)
    {
      case eDT_Wave:
        m_RP.m_pCurFuncs->WaveDeform(df);
        break;

      case eDT_Flare:
        m_RP.m_pCurFuncs->FlareDeform(df);
        break;

      case eDT_VerticalWave:
        m_RP.m_pCurFuncs->VerticalWaveDeform(df);
        break;

      case eDT_Bulge:
        m_RP.m_pCurFuncs->BulgeDeform(df);
        break;

      case eDT_FromCenter:
        m_RP.m_pCurFuncs->FromCenterDeform(df);
        break;

      case eDT_Squeeze:
        m_RP.m_pCurFuncs->SqueezeDeform(df);
        break;

      default:
        iLog->Log("Unknown deform type %d in Shader '%s'\n", df->m_eType, m_RP.m_pShader->m_Name);
        break;
    }
  }
}

void CD3D8Renderer::EF_Eval_TexGen(SShaderPass *sfm)
{
  int j, n;
  SShaderTexUnit *shl;
  SShader *ef = m_RP.m_pShader;
  SMRendTexVert *src;
  int m;

  for (j=0; j<sfm->m_TUnits.Num(); j++)
  {
    shl = sfm->m_TUnits[j];
    
    if (!shl->m_TexPic || shl->m_GTC)
      continue;

    switch (shl->m_eGenTC)
    {
      case eGTC_NoFill:
        break;

      case eGTC_None:
        break;

      case eGTC_Base:
        if (!m_RP.m_pRE)
        {
          m = m_RP.m_RendNumVerts;
          src = m_RP.m_pBaseTexCoordPointer;
          byte *ptr = m_RP.m_Ptr.PtrB + m_RP.m_OffsT + j*16;
          for (n=0; n<m; n++, ptr+=m_RP.m_Stride)
          {
            *(float *)(ptr) = src[n].vert[0];
            *(float *)(ptr+4) = src[n].vert[1];
          }
        }
        break;

      case eGTC_LightMap:
        if (!m_RP.m_pRE)
        {
          m = m_RP.m_RendNumVerts;
          src = m_RP.m_pLMTexCoordPointer;
          byte *ptr = m_RP.m_Ptr.PtrB + m_RP.m_OffsT + j*16;
          for (n=0; n<m; n++, ptr+=m_RP.m_Stride)
          {
            *(float *)(ptr) = src[n].vert[0];
            *(float *)(ptr+4) = src[n].vert[1];
          }
        }
        break;

      case eGTC_Environment:
        m_RP.m_pCurFuncs->ETC_Environment(j);
        break;

      case eGTC_Projection:
        if (shl->m_TexPic->m_Matrix)
          m_RP.m_pCurFuncs->ETC_Projection(j, shl->m_TexPic->m_Matrix, 1, 1);
        break;

      case eGTC_SphereMap:
        m_RP.m_pCurFuncs->ETC_SphereMap(j);
        break;

      case eGTC_SphereMapEnvironment:
        m_RP.m_pCurFuncs->ETC_SphereMapEnvironment(j);
        break;

      case eGTC_ShadowMap:
        m_RP.m_pCurFuncs->ETC_ShadowMap(j);
        break;

      default:
        iLog->Log("ERROR: invalid eGenTC '%d' specified for Shader\n", shl->m_eGenTC);
        break;
    }
  }
}

void CD3D8Renderer::EF_Eval_RGBAGen(SShaderPass *sfm)
{
  SShader *ef = m_RP.m_pShader;
  int n;
  UCol color;
  bool bSetCol = false;
  color.dcolor = -1;

  switch(sfm->m_eEvalRGB)
  {
    case eERGB_NoFill:
      break;

    case eERGB_Identity:
      {
        if (m_RP.m_pRE)
        {
          if (!(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
          {
            color.dcolor = -1;
            bSetCol = true;
            m_RP.m_FlagsPerFlush = RBSI_RGBGEN;
          }
        }
        else
        {
          byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
          for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride)
          {
            *(uint *)ptr = -1;
          }
        }
      }
      break;

    case eERGB_FromClient:
      if (!m_RP.m_pRE)
      {
        if (!gbRgb)
        {
          byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
          byte *src = (byte *)(m_RP.m_pClientColors[0]);
          for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride, src+=4)
          {
            *(uint *)ptr = *(uint *)(src);
          }
        }
        else
        {
          byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
          byte *src = (byte *)(&m_RP.m_pClientColors[0]);
          for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride, src+=4)
          {
            ptr[2] = src[0];
            ptr[1] = src[1];
            ptr[0] = src[2];
            ptr[3] = src[3];
          }
        }
      }
      break;

    case eERGB_OneMinusFromClient:
      if (!m_RP.m_pRE)
      {
        if (!gbRgb)
        {
          byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
          for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride)
          {
            ptr[0] = 255 - m_RP.m_pClientColors[n][0];
            ptr[1] = 255 - m_RP.m_pClientColors[n][1];
            ptr[2] = 255 - m_RP.m_pClientColors[n][2];
          }
        }
        else
        {
          byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
          for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride)
          {
            ptr[0] = 255 - m_RP.m_pClientColors[n][2];
            ptr[1] = 255 - m_RP.m_pClientColors[n][1];
            ptr[2] = 255 - m_RP.m_pClientColors[n][0];
          }
        }
      }
      break;

    case eERGB_Fixed:
      color = sfm->m_FixedColor;
      bSetCol = true;
      m_RP.m_FlagsPerFlush = RBSI_RGBGEN;
      break;

    case eERGB_StyleIntens:
      {
        CLightStyle *ls = CLightStyle::mfGetStyle(sfm->m_Style, m_RP.m_RealTime);
        color = sfm->m_FixedColor;
        color.bcolor[0] = (byte)((float)color.bcolor[0] * ls->m_fIntensity);
        color.bcolor[1] = (byte)((float)color.bcolor[1] * ls->m_fIntensity);
        color.bcolor[2] = (byte)((float)color.bcolor[2] * ls->m_fIntensity);
        bSetCol = true;
        m_RP.m_FlagsPerFlush = RBSI_RGBGEN;
      }
      break;

    case eERGB_StyleColor:
      {
        CLightStyle *ls = CLightStyle::mfGetStyle(sfm->m_Style, m_RP.m_RealTime);
        color.dcolor = ls->m_Color.GetTrue();
        bSetCol = true;
        m_RP.m_FlagsPerFlush = RBSI_RGBGEN;
      }
      break;

    case eERGB_Comps:
      {
        if (sfm->m_RGBComps)
        {
          float *vals = sfm->m_RGBComps->mfGet();
          color.bcolor[0] = (byte)(vals[0] * 255.0f);
          color.bcolor[1] = (byte)(vals[1] * 255.0f);
          color.bcolor[2] = (byte)(vals[2] * 255.0f);
          color.bcolor[3] = (byte)(vals[3] * 255.0f);
          bSetCol = true;
          m_RP.m_FlagsPerFlush = RBSI_RGBGEN;
        }
      }
      break;

    case eERGB_Wave:
      if (sfm->m_WaveEvalRGB)
      {
        if (m_RP.m_pRE)
        {
          if (!(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
          {
            float val = SEvalFuncs::EvalWaveForm(sfm->m_WaveEvalRGB);
            if (val < 0)
              val = 0;
            if (val > 1)
              val = 1;

            color.bcolor[0] = color.bcolor[1] = color.bcolor[2] = (int)(val * 255.0f);
            COLCONV(color.dcolor);
            bSetCol = true;
            m_RP.m_FlagsPerFlush = RBSI_RGBGEN;
          }
        }
        else
          m_RP.m_pCurFuncs->ERGB_Wave(sfm->m_WaveEvalRGB, color);
      }
      break;

    case eERGB_Noise:
      if (sfm->m_RGBNoise)
      {
        if (m_RP.m_pRE)
        {
          if (!(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
          {
            float v = RandomNum();
            byte r = (byte)(Clamp<float>(v * sfm->m_RGBNoise->m_RangeR + sfm->m_RGBNoise->m_ConstR, 0.0f, 1.0f) * 255.0f);
            v = RandomNum();
            byte g = (byte)(Clamp<float>(v * sfm->m_RGBNoise->m_RangeG + sfm->m_RGBNoise->m_ConstG, 0.0f, 1.0f) * 255.0f);
            v = RandomNum();
            byte b = (byte)(Clamp<float>(v * sfm->m_RGBNoise->m_RangeB + sfm->m_RGBNoise->m_ConstB, 0.0f, 1.0f) * 255.0f);
            
            color.bcolor[0] = r;
            color.bcolor[1] = g;
            color.bcolor[2] = b;
            COLCONV(color.dcolor);
            bSetCol = true;
            m_RP.m_FlagsPerFlush = RBSI_RGBGEN;
          }
        }
        else
          m_RP.m_pCurFuncs->ERGB_Noise(sfm->m_RGBNoise, color);
      }
      break;

    case eERGB_Object:
      if (m_RP.m_pRE)
      {
        if (m_RP.m_pCurObject && !(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
        {
          bSetCol = true;
          color.bcolor[0] = (byte)(m_RP.m_pCurObject->m_Color[0] * 255.0f);
          color.bcolor[1] = (byte)(m_RP.m_pCurObject->m_Color[1] * 255.0f);
          color.bcolor[2] = (byte)(m_RP.m_pCurObject->m_Color[2] * 255.0f);
          m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
        }
      }
      else
        m_RP.m_pCurFuncs->ERGB_Object();
      break;

    case eERGB_OneMinusObject:
      if (m_RP.m_pRE)
      {
        if (m_RP.m_pCurObject && !(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
        {
          bSetCol = true;
          color.bcolor[0] = (byte)((1.0f - m_RP.m_pCurObject->m_Color[0]) * 255.0f);
          color.bcolor[1] = (byte)((1.0f - m_RP.m_pCurObject->m_Color[1]) * 255.0f);
          color.bcolor[2] = (byte)((1.0f - m_RP.m_pCurObject->m_Color[2]) * 255.0f);
          m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
        }
      }
      else
        m_RP.m_pCurFuncs->ERGB_OneMinusObject();
      break;

    case eERGB_RE:
      if (m_RP.m_pRE && !(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
      {
        bSetCol = true;
        color.bcolor[0] = (byte)(m_RP.m_pRE->m_Color[0] * 255.0f);
        color.bcolor[1] = (byte)(m_RP.m_pRE->m_Color[1] * 255.0f);
        color.bcolor[2] = (byte)(m_RP.m_pRE->m_Color[2] * 255.0f);
        m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
      }
      break;
      
    case eERGB_OneMinusRE:
      if (m_RP.m_pRE && !(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
      {
        bSetCol = true;
        color.bcolor[0] = (byte)((1.0f - m_RP.m_pRE->m_Color[0]) * 255.0f);
        color.bcolor[1] = (byte)((1.0f - m_RP.m_pRE->m_Color[1]) * 255.0f);
        color.bcolor[2] = (byte)((1.0f - m_RP.m_pRE->m_Color[2]) * 255.0f);
        m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
      }
      break;

    case eERGB_World:
      if (m_RP.m_pRE && !(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
      {
        bSetCol = true;
        color.bcolor[0] = (byte)(gRenDev->m_WorldColor[0] * 255.0f);
        color.bcolor[1] = (byte)(gRenDev->m_WorldColor[1] * 255.0f);
        color.bcolor[2] = (byte)(gRenDev->m_WorldColor[2] * 255.0f);
        m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
      }
      break;

    default:
      assert(0);
  }

  switch(sfm->m_eEvalAlpha)
  {
    case eEALPHA_NoFill:
      break;

    case eEALPHA_Identity:
      if (sfm->m_eEvalRGB!=eERGB_Identity && sfm->m_eEvalRGB!=eERGB_Fixed)
      {
        if (m_RP.m_pRE)
        {
          if (!(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
          {
            color.bcolor[3] = 255;
            bSetCol = true;
            m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
          }
        }
        else
        {
          byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
          for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride)
          {
            ptr[3] = 255;
          }
        }
      }
      break;

    case eEALPHA_Fixed:
      {
        if (sfm->m_eEvalRGB == eERGB_Fixed)
          break;
        if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
        {
          color.bcolor[3] = sfm->m_FixedColor.bcolor[3];
          bSetCol = true;
          m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
        }
      }
      break;

    case eEALPHA_Style:
      {
        CLightStyle *ls = CLightStyle::mfGetStyle(sfm->m_Style, m_RP.m_RealTime);
        color.bcolor[3] = (byte)((float)sfm->m_FixedColor.bcolor[3] * ls->m_fIntensity);
        bSetCol = true;
        m_RP.m_FlagsPerFlush = RBSI_ALPHAGEN;
      }
      break;

    case eEALPHA_Comps:
      {
        if (sfm->m_eEvalRGB == eERGB_Comps)
          break;
        if (sfm->m_RGBComps)
        {
          float *vals = sfm->m_RGBComps->mfGet();
          if (m_RP.m_pRE)
          {
            if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
            {
              color.bcolor[3] = (byte)(vals[0] * 255.0f);
              bSetCol = true;
              m_RP.m_FlagsPerFlush = RBSI_ALPHAGEN;
            }
          }
          else
          {
            byte a = (byte)(vals[0] * 255.0f);
            byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
            for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride)
            {
              ptr[3] = a;
            }
          }
        }
      }
      break;

    case eEALPHA_Wave:
      if (sfm->m_WaveEvalAlpha)
      {
        if (m_RP.m_pRE)
        {
          if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
          {
            float val = SEvalFuncs::EvalWaveForm(sfm->m_WaveEvalAlpha);
            if (val < 0)
              val = 0;
            if (val > 1)
              val = 1;
            
            color.bcolor[3] = (int)(val * 255.0f);
            bSetCol = true;
            m_RP.m_FlagsPerFlush = RBSI_ALPHAGEN;
          }
        }
        else
          m_RP.m_pCurFuncs->EALPHA_Wave(sfm->m_WaveEvalAlpha, color);
      }
      break;

    case eEALPHA_Noise:
      if (sfm->m_ANoise)
      {
        if (m_RP.m_pRE)
        {
          if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
          {
            float v = RandomNum();
            byte a = (byte)(Clamp<float>(v * sfm->m_ANoise->m_RangeA + sfm->m_ANoise->m_ConstA, 0.0f, 1.0f) * 255.0f);
            
            color.bcolor[3] = a;
            bSetCol = true;
            m_RP.m_FlagsPerFlush = RBSI_ALPHAGEN;
          }
        }
        else
          m_RP.m_pCurFuncs->EALPHA_Noise(sfm->m_ANoise, color);
      }
      break;

    case eEALPHA_Beam:
      m_RP.m_pCurFuncs->EALPHA_Beam();
      break;

    case eEALPHA_Object:
      if (m_RP.m_pRE)
      {
        if (m_RP.m_pCurObject && !(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
        {
          bSetCol = true;
          color.bcolor[3] = (byte)(m_RP.m_pCurObject->m_Color[3] * 255.0f);
          m_RP.m_FlagsPerFlush = RBSI_ALPHAGEN;
        }
      }
      else
        m_RP.m_pCurFuncs->EALPHA_Object();
      break;

    case eEALPHA_OneMinusObject:
      if (m_RP.m_pRE)
      {
        if (m_RP.m_pCurObject && !(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
        {
          bSetCol = true;
          color.bcolor[3] = (byte)((1.0f - m_RP.m_pCurObject->m_Color[3]) * 255.0f);
          m_RP.m_FlagsPerFlush = RBSI_ALPHAGEN;
        }
      }
      else
        m_RP.m_pCurFuncs->EALPHA_OneMinusObject();
      break;

    case eEALPHA_RE:
      if (m_RP.m_pRE)
      {
        if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
        {
          bSetCol = true;
          color.bcolor[3] = (byte)(m_RP.m_pRE->m_Color[3] * 255.0f);
          m_RP.m_FlagsPerFlush = RBSI_ALPHAGEN;
        }
      }
      break;
      
    case eEALPHA_OneMinusRE:
      if (m_RP.m_pRE)
      {
        if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
        {
          bSetCol = true;
          color.bcolor[3] = (byte)((1.0f - m_RP.m_pCurObject->m_Color[3]) * 255.0f);
          m_RP.m_FlagsPerFlush = RBSI_ALPHAGEN;
        }
      }
      break;

    case eEALPHA_World:
      if (m_RP.m_pRE)
      {
        if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
        {
          bSetCol = true;
          color.bcolor[3] = (byte)(gRenDev->m_WorldColor[3] * 255.0f);
          m_RP.m_FlagsPerFlush = RBSI_ALPHAGEN;
        }
      }
      break;

    case eEALPHA_FromClient:
      if (!m_RP.m_pRE)
      {
        if (sfm->m_eEvalRGB!=eERGB_FromClient)
        {
          byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
          for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride)
          {
            ptr[3] = m_RP.m_pClientColors[n][3];
          }
        }
      }
      break;

    default:
      assert(0);
  }

  // Fog blending for non-opaque surfaces
  /*if (m_RP.NumFog)
  {
  m_RP.bFogBlend = false;
  switch(sfm->m_eFogBlendComponents)
  {
  case eFBLC_OnlyColor:
  m_RP.bFogBlend = true;
  m_RP.m_pCurFuncs->BLC_OnlyColor();
  break;

  case eFBLC_ColorAlpha:
  m_RP.bFogBlend = true;
  m_RP.m_pCurFuncs->BLC_ColorAlpha();
  break;

  case eFBLC_OnlyAlpha:
  m_RP.bFogBlend = true;
  m_RP.m_pCurFuncs->BLC_OnlyAlpha();
  break;
  }
  }*/
  if (bSetCol)
    EF_SetGlobalColor(color);
}

void CD3D8Renderer::EF_EvalNormalsRB(SShader *ef)
{
  if (!ef->m_NormGen)
    return;
  
  SNormalsGen *ng = ef->m_NormGen;
  
  if (ng->m_eNormal == eNORM_Identity || !m_RP.m_OffsN)
    return;
  
  int i;
  int nums = m_RP.m_RendNumVerts;
  byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsN;
  
  switch(ng->m_eNormal)
  {
  case eNORM_Custom:
    for (i=0; i<nums; i++, ptr+=m_RP.m_Stride)
    {
      float *tsnrm = (float *)ptr;
      tsnrm[0] = ng->m_CustomNormal[0];
      tsnrm[1] = ng->m_CustomNormal[1];
      tsnrm[2] = ng->m_CustomNormal[2];
    }
    break;
  }
}

void CD3D8Renderer::EF_UpdateTextures(SShaderPass *Layer)
{
  SShaderTexUnit *stl;
  int j;
  for (j=0; j<Layer->m_TUnits.Num(); j++)
  {
    stl = Layer->m_TUnits[j];
    
    if (!stl->m_TexPic && !stl->m_AnimInfo)
      break;
    
    stl->mfUpdate();
  }
}

void CD3D8Renderer::EF_UpdateTextures(SShaderPassHW *Layer)
{
  SShaderTexUnit *stl;
  int j;
  for (j=0; j<Layer->m_TUnits.Num(); j++)
  {
    stl = Layer->m_TUnits[j];
    
    if (!stl->m_TexPic && !stl->m_AnimInfo)
      break;
    
    stl->mfUpdate();
  }
}

//=================================================================================

void CD3D8Renderer::SetState(int st)
{
  int Changed;
  int src, dst;

  if (m_RP.m_Flags & RBF_SHOWLINES)
    st |= GS_NODEPTHTEST;
  
  if (st & (GS_DEPTHWRITE | GS_DEPTHFUNC_EQUAL))
  {
    if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHABLEND))
      m_RP.m_FlagsPerFlush |= RBSI_WASDEPTHWRITE;
  }

  Changed = st ^ mCurState;
  if (!Changed)
    return;
  
  if (Changed & GS_DEPTHFUNC_EQUAL)
  {
    if (st & GS_DEPTHFUNC_EQUAL)
      m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
    else
      m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
  }

  if (Changed & GS_POLYLINE)
  {
    if (st & GS_POLYLINE)
      m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
    else
      m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
  }
  
  if (Changed & GS_NOCOLMASK)
  {
    if (st & GS_NOCOLMASK)
      m_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
    else
      m_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
  }
  if (Changed & GS_COLMASKONLYALPHA)
  {
    if (st & GS_COLMASKONLYALPHA)
      m_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
    else
      m_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
  }
  
  if (Changed & GS_BLEND_MASK)
  {
    if ((st & GS_BLEND_MASK) == GS_BLEND_MASK)
    {
      st &= ~GS_BLEND_MASK;
      st |= (mCurState & GS_BLEND_MASK);
      goto skip;
    }

    if (st & GS_BLEND_MASK)
    {
      // Source factor
      switch (st & GS_BLSRC_MASK)
      {
        case GS_BLSRC_ZERO:
          src = D3DBLEND_ZERO;
          break;
          
        case GS_BLSRC_ONE:
          src = D3DBLEND_ONE;
          break;
          
        case GS_BLSRC_DSTCOL:
          src = D3DBLEND_DESTCOLOR;
          break;
          
        case GS_BLSRC_ONEMINUSDSTCOL:
          src = D3DBLEND_INVDESTCOLOR;
          break;
          
        case GS_BLSRC_SRCALPHA:
          src = D3DBLEND_SRCALPHA;
          break;
          
        case GS_BLSRC_ONEMINUSSRCALPHA:
          src = D3DBLEND_INVSRCALPHA;
          break;
          
        case GS_BLSRC_DSTALPHA:
          src = D3DBLEND_DESTALPHA;
          break;
          
        case GS_BLSRC_ONEMINUSDSTALPHA:
          src = D3DBLEND_INVDESTALPHA;
          break;
          
        case GS_BLSRC_ALPHASATURATE:
          src = D3DBLEND_SRCALPHASAT;
          break;
          
        default:
          iLog->Log("CD3D8Renderer::SetState: invalid src blend state bits '%d'", st & GS_BLSRC_MASK);
          break;
      }
      
      //Destination factor
      switch (st & GS_BLDST_MASK)
      {
        case GS_BLDST_ZERO:
          dst = D3DBLEND_ZERO;
          break;
          
        case GS_BLDST_ONE:
          dst = D3DBLEND_ONE;
          break;
          
        case GS_BLDST_SRCCOL:
          dst = D3DBLEND_SRCCOLOR;
          break;
          
        case GS_BLDST_ONEMINUSSRCCOL:
          dst = D3DBLEND_INVSRCCOLOR;
          break;
          
        case GS_BLDST_SRCALPHA:
          dst = D3DBLEND_SRCALPHA;
          break;
          
        case GS_BLDST_ONEMINUSSRCALPHA:
          dst = D3DBLEND_INVSRCALPHA;
          break;
          
        case GS_BLDST_DSTALPHA:
          dst = D3DBLEND_DESTALPHA;
          break;
          
        case GS_BLDST_ONEMINUSDSTALPHA:
          dst = D3DBLEND_INVDESTALPHA;
          break;
          
        default:
          iLog->Log("CD3D8Renderer::SetState: invalid dst blend state bits '%d'", st & GS_BLDST_MASK);
          break;
      }
      m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,  src);
      m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, dst);
    }
    else
      m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
  }
  
skip:
  if (Changed & GS_DEPTHWRITE)
  {
    if (st & GS_DEPTHWRITE)
      m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    else
      m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
  }
  
  if (Changed & GS_NODEPTHTEST)
  {
    if (st & GS_NODEPTHTEST)
      m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    else
      m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
  }
  
  if (Changed & GS_ALPHATEST_MASK)
  {
    if (st & GS_ALPHATEST_MASK)
    {
      switch (st & GS_ALPHATEST_MASK)
      {
        case GS_ALPHATEST_GREATER0:
          m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
          m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0);
          m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
          break;
          
        case GS_ALPHATEST_LESS128:
          m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
          m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x80);
          m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_LESS);
          break;
          
        case GS_ALPHATEST_GEQUAL128:
          m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
          m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x80);
          m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
          break;
      }
    }
    else
      m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
  }
  mCurState = st;
}

void CD3D8Renderer::EF_SetColorOp(byte co)
{
  int stage = m_TexMan->m_CurStage;
  
  if (co == eCurColorOp[stage])
    return;
  switch (co)
  {
    case eCO_MODULATE:
    default:
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
      break;

    case eCO_ONLYCOLOR_MODULATE:
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
      break;

    case eCO_BLENDDIFFUSEALPHA:
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);
      break;
      
    case eCO_BLENDTEXTUREALPHA:
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_BLENDTEXTUREALPHA);
      break;
      
    case eCO_MODULATE4X:
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
      break;
      
    case eCO_MODULATE2X:
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
      break;
      
    case eCO_ADD:
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_ADD);
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_ADD);
      break;

    case eCO_ADD_MODULATE:
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_ADD);
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
      break;

    case eCO_ADDSIGNED:
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_ADDSIGNED);
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_ADDSIGNED);
      break;
      
    case eCO_REPLACE:
    case eCO_DECAL:
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
      break;
      
    case eCO_BUMPENVMAP:
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_BUMPENVMAP);
      break;
      
    case eCO_DISABLE:
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
      m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
      break;
  }
  eCurColorOp[stage] = co;
}

void CD3D8Renderer::D3DSetCull(ECull eCull)
{ 
  if (eCull == m_eCull)
    return;

  if (eCull == eCULL_None)
    m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
  else
  {
    if (eCull == eCULL_Back)
      m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    else
      m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
  }
  m_eCull = eCull;
}


void CD3D8Renderer::EF_PreRender()
{
  m_RP.m_RealTime = iTimer->GetCurrTime();
  ResetToDefault();
  
  m_RP.m_Flags = 0;
  m_RP.m_pPrevObject = NULL;
  m_RP.m_FrameObject++;
  
  EF_SetCameraInfo();

  if (!m_RP.m_bStartPipeline && !m_bWasCleared && !(m_RP.m_PersFlags & RBPF_NOCLEARBUF))
  {
    m_RP.m_bStartPipeline = true;
    EF_ClearBuffer(false, NULL);
  }
}

void CD3D8Renderer::EF_PostRender()
{
  EF_ObjectChange(NULL, 0, NULL);
  m_RP.m_pRE = NULL;

  HRESULT h;
  if (m_RP.m_PersFlags & RBPF_USESTREAM1)
  {
    m_RP.m_PersFlags &= ~RBPF_USESTREAM1;
    h = m_pd3dDevice->SetStreamSource( 1, NULL, 0);
  }
  if (m_RP.m_PersFlags & RBPF_USESTREAM2)
  {
    m_RP.m_PersFlags &= ~RBPF_USESTREAM2;
    h = m_pd3dDevice->SetStreamSource( 2, NULL, 0);
  }
  if (m_RP.m_PersFlags & RBPF_SETCLIPPLANE)
  {
    EF_SetClipPlane(false, NULL, false);
    m_RP.m_PersFlags &= ~RBPF_SETCLIPPLANE;
  }
  m_Viewport.MaxZ = 1.0f;
  m_pd3dDevice->SetViewport(&m_Viewport);

  ResetToDefault();
}

//=================================================================================

bool CD3D8Renderer::EF_PreDraw()
{
  bool bRet = true;
  if (m_RP.m_bLocked)
  {
    if (!m_RP.m_pRE)
    {
      m_RP.m_VB.VBPtr_D_1T->Unlock();
      m_RP.m_IndexBuf->Unlock();
    }
    m_RP.m_bLocked = false;
  }
  if (!m_RP.m_pRE)
  {
    m_pd3dDevice->SetStreamSource(0, m_RP.m_VB.VBPtr_D_1T->GetInterface(), m_RP.m_Stride);
    m_pd3dDevice->SetIndices(m_RP.m_IndexBuf->GetInterface(), 0);
  }
  else
    bRet = m_RP.m_pRE->mfPreDraw();

  return bRet;
}

// Initialize of the new shader pipeline (only 2d)
void CD3D8Renderer::EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, CRendElement *re) 
{
  m_RP.m_RendPass = 0;
  m_RP.m_bLocked = false;
  m_RP.m_RendNumIndices = 0;
  m_RP.m_RendNumVerts = 0;
  m_RP.m_FirstIndex = 0;
  m_RP.m_FirstVertex = 0;
  m_RP.m_pShader = ef;
  m_RP.mCurLightMaterial = NULL;
  m_RP.m_pStateShader = efState;
  m_RP.m_pShaderResources = Res;
  m_RP.m_DynLMask = 0;
  m_RP.m_FlagsPerFlush = 0;
  m_RP.m_FlagsModificators = 0;
  m_RP.m_pFogVolume = NULL;
  m_RP.m_pRE = NULL;
  m_RP.m_fCurOpacity = 1.0f;

  int flt = ef->m_FLT;
  if (flt < FLT_N && CRenderer::CV_r_shownormals)
    flt |= FLT_N;
  m_RP.mFT = flt;

  int n = ef->m_nVB + 8;
  
  m_RP.m_Stride = sSVB[n].Stride;
  m_RP.m_OffsD  = sSVB[n].OffsD;
  m_RP.m_OffsT  = sSVB[n].OffsT;
  m_RP.m_OffsN  = sSVB[n].OffsN;
  m_RP.m_NextPtr = m_RP.m_Ptr;
  m_RP.m_CurD3DVFormat = n;
  m_RP.m_VB = sSVB[n].mVB;

  m_RP.m_Frame++;
}

// Initialize of the new shader pipeline
void CD3D8Renderer::EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, int numFog, CRendElement *re)
{
  m_RP.m_RendPass = 0;
  m_RP.m_bLocked = false;
  m_RP.m_FirstIndex = 0;
  m_RP.m_FirstVertex = 0;
  m_RP.m_RendNumIndices = 0;
  m_RP.m_RendNumVerts = 0;
  m_RP.m_pShader = ef;
  m_RP.m_pStateShader = efState;
  m_RP.m_pShaderResources = Res;
  m_RP.m_DynLMask = 0;
  m_RP.m_pRE = NULL;
  m_RP.mCurLightMaterial = NULL;
  m_RP.m_FlagsPerFlush = 0;
  m_RP.m_FlagsModificators = 0;
  m_RP.m_fCurOpacity = 1.0f;
  if (numFog && CV_r_VolumetricFog)
    m_RP.m_pFogVolume = &m_RP.m_FogVolumes[numFog];
  else
    m_RP.m_pFogVolume = NULL;

  int n = ef->m_nVB;
  int flt = ef->m_FLT;
  if (flt < FLT_N && CV_r_shownormals || ef->m_LightMaterial)
    flt |= FLT_N;
  m_RP.mFT = flt;
  if (flt & FLT_N)
    n += 4;

  m_RP.m_Stride = sSVB[n].Stride;
  m_RP.m_OffsD  = sSVB[n].OffsD;
  m_RP.m_OffsT  = sSVB[n].OffsT;
  m_RP.m_OffsN  = sSVB[n].OffsN;
  m_RP.m_NextPtr = m_RP.m_Ptr;
  m_RP.m_VB = sSVB[n].mVB;
  m_RP.m_CurD3DVFormat = ef->m_nVB;
  m_RP.m_Frame++;
}

//========================================================================================

void CD3D8Renderer::EF_SetHWLight(int Num, vec4_t Pos, CFColor& Diffuse, CFColor& Specular, float ca, float la, float qa, float fRange)
{
  m_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0);

  m_Lights[Num].Position.x = Pos[0];
  m_Lights[Num].Position.y = Pos[1];
  m_Lights[Num].Position.z = Pos[2];
  m_Lights[Num].Range = fRange;
  if (!Pos[3])
  {
    Vec3d v = Vec3d(Pos[0], Pos[1], Pos[2]);
    v.Normalize();
    m_Lights[Num].Direction.x = v[0];
    m_Lights[Num].Direction.y = v[1];
    m_Lights[Num].Direction.z = v[2];
    m_Lights[Num].Type = D3DLIGHT_DIRECTIONAL;
  }
  else
    m_Lights[Num].Type = D3DLIGHT_POINT;

  m_Lights[Num].Ambient.r = 1.0f;
  m_Lights[Num].Ambient.g = 1.0f;
  m_Lights[Num].Ambient.b = 1.0f;
  m_Lights[Num].Ambient.a = 1.0f;

  m_Lights[Num].Diffuse.r = Diffuse.r;
  m_Lights[Num].Diffuse.g = Diffuse.g;
  m_Lights[Num].Diffuse.b = Diffuse.b;
  m_Lights[Num].Diffuse.a = Diffuse.a;

  m_Lights[Num].Specular.r = Specular.r;
  m_Lights[Num].Specular.g = Specular.g;
  m_Lights[Num].Specular.b = Specular.b;
  m_Lights[Num].Specular.a = Specular.a;

  m_Lights[Num].Attenuation0 = ca;
  m_Lights[Num].Attenuation1 = la;
  m_Lights[Num].Attenuation2 = qa;

  m_pd3dDevice->SetLight(Num, &m_Lights[Num]);
  m_pd3dDevice->LightEnable(Num, TRUE);

  m_EnableLights |= 1<<Num;
}

static float sAttenuation(float Distance, float Radius)
{
  if(Distance <= Radius)
  {
    float A = Distance / Radius;
    float B = (2 * A * A * A - 3 * A * A + 1);

    return B / A * A * 2.0f;
  }
  else
    return 0.0f;
}


bool CD3D8Renderer::EF_SetLights(int Flags, bool Enable)
{
  if (!Enable)
  {
    if (m_EnableLights)
    {
      m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
      m_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
      for (int i=0; i<8; i++)
      {
        if (m_EnableLights & (1<<i))
          m_pd3dDevice->LightEnable(i, FALSE);
      }
    }
    m_EnableLights = 0;
    return false;
  }
  float MaxD3DRange = sqrtf(FLT_MAX); // xbox libs freak out about illegal light ranges

  vec4_t Pos;
  Vec3d lpos;
  int n = 0;
  float fCA, fLA, fQA;
  Vec3d vCenterRE;
  float fRadRE;
  bool bCalcDist = false;
  CFColor cDiffuse;
  for (int i=0; i<m_RP.m_DLights.Num(); i++)
  {
    if (i >= 32)
      break;
    CDLight *dl = m_RP.m_DLights[i];
    if (dl->m_Flags & DLF_DETAIL)
      continue;
    if (!(m_RP.m_DynLMask & (1<<i)))
      continue;
    if ((Flags & LMF_IGNOREPROJLIGHTS) && (dl->m_Flags & DLF_PROJECT))
      continue;
    if (Flags & LMF_LIGHT_MASK)
    {
      if (((Flags & LMF_LIGHT_MASK) >> LMF_LIGHT_SHIFT) != i)
        continue;
    }
    // some statistics
    if (!(m_RP.m_StatLightMask & (1<<i)))
    {
      m_RP.m_StatLightMask |= (1<<i);
      m_RP.m_StatNumLights++;
    }
    TransformPosition(lpos, dl->m_Origin, m_RP.m_pCurObject->GetInvMatrix());
    float fRange = 0;
    if (dl->m_Flags & DLF_DIRECTIONAL)
    {
      lpos.Normalize();
      Pos[3] = 0.0f;
      fCA = 1.0f;
      fLA = 0.0f;
      fQA = 0.0f;
      cDiffuse = dl->m_Color;
    }
    else
    {
      fRange = dl->m_fRadius;
      Pos[3] = 1.0f;
      if (m_RP.m_pRE && !bCalcDist)
      {
        Vec3d vMins, vMaxs;
        bCalcDist = true;
        m_RP.m_pRE->mfGetBBox(vMins, vMaxs);
        vCenterRE = (vMins + vMaxs) * 0.5f;
        vCenterRE.x += m_RP.m_pCurObject->m_Matrix(3,0);
        vCenterRE.y += m_RP.m_pCurObject->m_Matrix(3,1);
        vCenterRE.z += m_RP.m_pCurObject->m_Matrix(3,2);
        fRadRE = (vMaxs - vMins).Length() * 0.5f;
      }
      float fDist = Max(0.1f, (vCenterRE - dl->m_Origin).Length());
      float fMaxDist = Clamp(fDist + fRadRE, dl->m_fRadius * 0.0005f, dl->m_fRadius * 0.99f);
      float fMinDist = Clamp(fDist - fRadRE, dl->m_fRadius * 0.001f,  dl->m_fRadius * 0.995f);
      float fMinAtt = 1.0f / sAttenuation(fMinDist, dl->m_fRadius);
      float fMaxAtt = 1.0f / sAttenuation(fMaxDist, dl->m_fRadius);
      if(Abs(fMinAtt - fMaxAtt) < 0.00001f)
      {
        fCA	= fMinAtt;
        fLA	= 0.0f;
      }
      else
      {
        fCA = Max(0.01f, fMinAtt - (fMaxAtt - fMinAtt) / (fMaxDist - fMinDist) * fMinDist);
        fLA = Max(0.0f, (fMinAtt - fCA) / fMinDist);
      }
      fQA = 0.0f;
      fRange = (256.0f - fCA) / Max(0.01f, fLA);
      cDiffuse = dl->m_Color;
    }
    fCA = Max(0.0f, fCA);
    fLA = Max(0.0f, fLA);
    fQA = Max(0.0f, fQA);
    Pos[0] = lpos.x;
    Pos[1] = lpos.y;
    Pos[2] = lpos.z;
    cDiffuse.a = 1.0f;
    EF_SetHWLight(n, Pos,
                 cDiffuse,
                 dl->m_SpecColor,
                 fCA, fLA, fQA, fRange);
    n++;
  }

  if (m_EnableLights)
  {
    m_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
    return true;
  }
  return false;
}

void CD3D8Renderer::EF_LightMaterial(SLightMaterial *lm, bool bEnable, int Flags)
{
  if (m_RP.m_pShader->m_Flags3 & EF3_NOLIGHTSOURCE)
    return;
  if (!(m_RP.m_pShader->m_Flags & EF_NEEDNORMALS))
    return;

  PROFILE_FRAME_TOTAL(State_LightStates);

  if (!bEnable)
  {
    EF_SetLights(Flags, false);
    return;
  }

  CFColor colAmb;
  if (Flags & LMF_NOAMBIENT)
    colAmb = Col_Black;
  else
    colAmb = m_RP.m_pCurObject->m_AmbColor;
  if (!m_RP.m_pCurObject || !(m_RP.m_pCurObject->m_ObjFlags & FOB_IGNOREMATERIALAMBIENT))
  {
    colAmb.r *= lm->Front.m_Ambient.r;
    colAmb.g *= lm->Front.m_Ambient.g;
    colAmb.b *= lm->Front.m_Ambient.b;
    colAmb.a *= lm->Front.m_Ambient.a;
  }
  if (!(Flags & LMF_IGNORELIGHTS) && EF_SetLights(Flags, true))
  {
    CFColor colDif = lm->Front.m_Diffuse;
    if (colDif.a != 1.0f && (Flags & LMF_NOALPHA))
      colDif.a = 1.0f;
    else
      colDif.a = m_RP.m_fCurOpacity;
    switch (lm->side)
    {
      case SLightMaterial::FRONT:
        m_Material.Ambient.r = colAmb.r;
        m_Material.Ambient.g = colAmb.g;
        m_Material.Ambient.b = colAmb.b;
        m_Material.Ambient.a = colAmb.a;

        m_Material.Diffuse.r = colDif.r;
        m_Material.Diffuse.g = colDif.g;
        m_Material.Diffuse.b = colDif.b;
        m_Material.Diffuse.a = colDif.a;

        m_Material.Specular.r = lm->Front.m_Specular.r;
        m_Material.Specular.g = lm->Front.m_Specular.g;
        m_Material.Specular.b = lm->Front.m_Specular.b;
        m_Material.Specular.a = lm->Front.m_Specular.a;

        m_Material.Power = lm->Front.m_SpecShininess;
        if (lm->Front.m_Specular == CFColor(0.0f))
          m_Material.Power = 0;
        break;

      case SLightMaterial::BACK:
        m_Material.Ambient.r = lm->Back.m_Ambient.r;
        m_Material.Ambient.g = lm->Back.m_Ambient.g;
        m_Material.Ambient.b = lm->Back.m_Ambient.b;
        m_Material.Ambient.a = lm->Back.m_Ambient.a;

        m_Material.Diffuse.r = lm->Back.m_Diffuse.r;
        m_Material.Diffuse.g = lm->Back.m_Diffuse.g;
        m_Material.Diffuse.b = lm->Back.m_Diffuse.b;
        m_Material.Diffuse.a = lm->Back.m_Diffuse.a;

        m_Material.Specular.r = lm->Back.m_Specular.r;
        m_Material.Specular.g = lm->Back.m_Specular.g;
        m_Material.Specular.b = lm->Back.m_Specular.b;
        m_Material.Specular.a = lm->Back.m_Specular.a;

        m_Material.Power = lm->Back.m_SpecShininess;
        if (lm->Back.m_Specular == CFColor(0.0f))
          m_Material.Power = 0;
        break;

      case SLightMaterial::BOTH:
        m_Material.Ambient.r = lm->Front.m_Ambient.r;
        m_Material.Ambient.g = lm->Front.m_Ambient.g;
        m_Material.Ambient.b = lm->Front.m_Ambient.b;
        m_Material.Ambient.a = lm->Front.m_Ambient.a;

        m_Material.Diffuse.r = lm->Front.m_Diffuse.r;
        m_Material.Diffuse.g = lm->Front.m_Diffuse.g;
        m_Material.Diffuse.b = lm->Front.m_Diffuse.b;
        m_Material.Diffuse.a = lm->Front.m_Diffuse.a;

        m_Material.Specular.r = lm->Front.m_Specular.r;
        m_Material.Specular.g = lm->Front.m_Specular.g;
        m_Material.Specular.b = lm->Front.m_Specular.b;
        m_Material.Specular.a = lm->Front.m_Specular.a;

        m_Material.Power = lm->Front.m_SpecShininess;
        if (lm->Front.m_Specular == CFColor(0.0f))
          m_Material.Power = 0;
        break;
    }
  }
  m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
  m_pd3dDevice->SetMaterial(&m_Material);
  EF_SetVertColor();
}

//=============================================================


void CD3D8Renderer::EF_ApplyMatrixOps(TArray<SMatrixTransform>* MatrixOps, bool bEnable)
{
  if (!MatrixOps)
    return;

  LPDIRECT3DDEVICE8 dv = m_pd3dDevice;
  int CurMatrix = D3DTS_VIEW;
  m_CurOpMatrix = NULL;
  int Stage = -1;

  for (int i=0; i<MatrixOps->Num(); i++)
  {
    SMatrixTransform *mt = &MatrixOps->Get(i);
    if (mt->m_Matrix != CurMatrix)
    {
      if (m_CurOpMatrix)
      {
        if (Stage >= 0)
          dv->SetTextureStageState( Stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
        dv->SetTransform((D3DTRANSFORMSTATETYPE)CurMatrix, m_CurOpMatrix);
        m_CurOpMatrix = NULL;
      }
      switch (mt->m_Matrix)
      {
        case D3DTS_TEXTURE0:
          m_CurOpMatrix = &m_TexMatrix[0];
          Stage = 0;
          break;
        case D3DTS_TEXTURE1:
          m_CurOpMatrix = &m_TexMatrix[1];
          Stage = 1;
          break;
        case D3DTS_TEXTURE2:
          m_CurOpMatrix = &m_TexMatrix[2];
          Stage = 2;
          break;
        case D3DTS_TEXTURE3:
          m_CurOpMatrix = &m_TexMatrix[3];
          Stage = 3;
          break;
        default:
          Stage = -1;
      }
      if (!bEnable && mt->m_Matrix != D3DTS_PROJECTION && mt->m_Matrix != D3DTS_VIEW)
      {
        if (m_CurOpMatrix)
        {
          D3DXMatrixIdentity(m_CurOpMatrix);
          dv->SetTransform((D3DTRANSFORMSTATETYPE)mt->m_Matrix, m_CurOpMatrix);
          if (Stage >= 0)
            dv->SetTextureStageState( Stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
        }
      }
      CurMatrix = mt->m_Matrix;
    }
    mt->mfSet(bEnable);
  }
  if (m_CurOpMatrix && bEnable)
  {
    if (Stage >= 0)
      dv->SetTextureStageState( Stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
    dv->SetTransform((D3DTRANSFORMSTATETYPE)CurMatrix, m_CurOpMatrix);
  }
}

#include "../Common/NvTriStrip/NVTriStrip.h"

void CD3D8Renderer::EF_DrawIndexedMesh (int nPrimType)
{
  D3DPRIMITIVETYPE nType;
  int nFaces;
  
  switch (nPrimType)
  {
    case R_PRIMV_TRIANGLES:
      nType = D3DPT_TRIANGLELIST;
      nFaces = m_RP.m_RendNumIndices/3;
      break;

    case R_PRIMV_TRIANGLE_STRIP:
      nType = D3DPT_TRIANGLESTRIP;
      nFaces = m_RP.m_RendNumIndices-2;
      break;

    case R_PRIMV_TRIANGLE_FAN:
      nType = D3DPT_TRIANGLEFAN;
      nFaces = m_RP.m_RendNumIndices-2;
      break;

    case R_PRIMV_MULTI_STRIPS:
      {
        list2<CMatInfo> *mats = m_RP.m_pRE->mfGetMatInfoList();
        if (mats)
        {
          CMatInfo *m = mats->Get(0);
          for (int i=0; i<mats->Count(); i++, m++)
          {
            if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, m->nFirstVertId, m->nNumVerts, m->nFirstIndexId, m->nNumIndices - 2)))
            {
              Error("CD3D8Renderer::EF_DrawIndexedMesh: DrawIndexedPrimitive error", h);
              return;
            }
            m_nPolygons += (m->nNumIndices - 2);
          }
        }
        return;
      }
      break;

    case R_PRIMV_MULTI_GROUPS:
      {
        CMatInfo *mi = m_RP.m_pRE->mfGetMatInfo();
        if (mi)
        {
          int offs = mi->nFirstIndexId;
          for (int i=0; i<mi->m_dwNumSections; i++)
          {
            SPrimitiveGroup *g = &mi->m_pPrimitiveGroups[i];
            switch (g->type)
            {
              case PT_STRIP:
                if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, m_RP.m_FirstVertex, mi->nNumVerts, g->offsIndex+offs, g->numIndices - 2)))
                {
                  Error("CD3D8Renderer::EF_DrawIndexedMesh: DrawIndexedPrimitive error", h);
                  return;
                }
                m_nPolygons += (g->numIndices - 2);
                break;

              case PT_LIST:
                if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_RP.m_FirstVertex, mi->nNumVerts, g->offsIndex+offs, g->numIndices / 3)))
                {
                  Error("CD3D8Renderer::EF_DrawIndexedMesh: DrawIndexedPrimitive error", h);
                  return;
                }
                m_nPolygons += (g->numIndices / 3);
                break;

              case PT_FAN:
                if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, m_RP.m_FirstVertex, mi->nNumVerts, g->offsIndex+offs, g->numIndices - 2)))
                {
                  Error("CD3D8Renderer::EF_DrawIndexedMesh: DrawIndexedPrimitive error", h);
                  return;
                }
                m_nPolygons += (g->numIndices - 2);
                break;
            }
          }
        }
        return;
      }
      break;

    default:
      assert(0);
  }

  if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(nType, m_RP.m_FirstVertex, m_RP.m_RendNumVerts, m_RP.m_FirstIndex, nFaces)))
  {
    Error("CD3D8Renderer::EF_DrawIndexedMesh: DrawIndexedPrimitive error", h);
    return;
  }
  m_nPolygons += nFaces;
}

void CD3D8Renderer::EF_DrawFogOverlay()
{

}

void CD3D8Renderer::EF_DrawDetailOverlay()
{
  // Usually it means first pass in indoor engine (before shadow pass)
  if (m_RP.m_ObjFlags & FOB_ZPASS)
    return;

  if (!m_RP.m_pShaderResources || !m_RP.m_pShaderResources->m_Textures[EFTT_DETAIL_OVERLAY])
    return;

  SEfResTexture *rt = m_RP.m_pShaderResources->m_Textures[EFTT_DETAIL_OVERLAY];
  SShader *sh = m_RP.m_pShader;
  int i;

  float fDistToCam = 500.0f;
  float fDist = CV_r_detaildistance;
  if (m_RP.m_pRE)
  {
    fDistToCam = m_RP.m_pRE->mfMinDistanceToCamera();
    if (fDistToCam > fDist+1.0f)
      return;
  }
  CD3D8TexMan::BindNULL(2);
  EF_SelectTMU(1);
  CTexMan::m_Text_Fog->Set();
  EnableTMU(true);
  EF_SelectTMU(0);
  rt->m_TU.m_TexPic->Set();

  if (m_RP.m_FlagsPerFlush & RBSI_WASDEPTHWRITE)
    SetState(GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL | GS_DEPTHFUNC_EQUAL);
  else
    SetState(GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL);

  if (!(sh->m_Flags & EF_HASVSHADER))
  {
    float param[4];
    int n = Clamp(CV_r_detailnumlayers, 1, 4);
    float fUScale = rt->m_TexModificator.m_Tiling[0];
    float fVScale = rt->m_TexModificator.m_Tiling[1];
    if (!fUScale)
      fUScale = CV_r_detailscale;
    if (!fVScale)
      fVScale = CV_r_detailscale;

    for (i=0; i<n; i++)
    {
      /*glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glScalef(fUScale, fVScale, 1.0f);
      glMatrixMode(GL_MODELVIEW);

      EF_SelectTMU(1);

      float intens = -0.5f / fDist;
      param[0] = intens*m_ViewMatrix.m_values[0][2];
      param[1] = intens*m_ViewMatrix.m_values[1][2];
      param[2] = intens*m_ViewMatrix.m_values[2][2];
      param[3] = intens*m_ViewMatrix.m_values[3][2] + 0.5f;
      glTexGenfv(GL_S, GL_OBJECT_PLANE, param);

      param[0] = param[1] = param[2] = 0;
      param[3] = 0.5f;
      glTexGenfv(GL_T, GL_OBJECT_PLANE, param);

      glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
      glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
      glEnable(GL_TEXTURE_GEN_S);
      glEnable(GL_TEXTURE_GEN_T);*/

      EF_SelectTMU(0);

      if (!m_RP.m_RCDetail)
        m_RP.m_RCDetail = CPShader::mfForName("RCDetailAtten", false);
      if (m_RP.m_RCDetail)
      {
        m_RP.m_RCDetail->mfSet(true);
        param[0] = param[1] = param[2] = 0.5f; param[3] = 1.0f;
        CPShader_D3D::mfSetFloat4f(0, param);
      }

      // Draw primitives
      EF_Draw(sh, NULL);

      fDist /= 2.0f;
      if (fDistToCam > fDist+1.0f)
        break;
      fUScale *= 2.0f;
      fVScale *= 2.0f;
    }

    /*glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    EF_SelectTMU(1);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);*/
    EF_SelectTMU(0);
  }
  else
  {
    float param[4];
    if (!m_RP.m_VPDetail)
      m_RP.m_VPDetail = CVProgram::mfForName("CGVProgDetail", true);
    if (!m_RP.m_RCDetail)
      m_RP.m_RCDetail = CPShader::mfForName("RCDetailAtten", false);
    if (m_RP.m_RCDetail)
    {
      m_RP.m_RCDetail->mfSet(true);
      param[0] = param[1] = param[2] = 0.5f; param[3] = 1.0f;
      CPShader_D3D::mfSetFloat4f(0, param);
    }

    gRenDev->m_RP.m_FlagsModificators &= ~0x7;
    m_RP.m_VPDetail->mfSet(true, NULL, 1);

    EF_PreDraw();

    CCGVProgram_D3D *vpD3D = (CCGVProgram_D3D *)m_RP.m_VPDetail;
    SCGBind *pBindScale = vpD3D->mfGetParameterBind("DetailScaling");
    SCGBind *pBindTG0 = vpD3D->mfGetParameterBind("TexGen00");
    SCGBind *pBindTG1 = vpD3D->mfGetParameterBind("TexGen01");

    int n = Clamp(CV_r_detailnumlayers, 1, 4);
    float fUScale = rt->m_TexModificator.m_Tiling[0];
    float fVScale = rt->m_TexModificator.m_Tiling[1];
    if (!fUScale)
      fUScale = CV_r_detailscale;
    if (!fVScale)
      fVScale = CV_r_detailscale;
    for (i=0; i<n; i++)
    {
      param[0] = fUScale;
      param[1] = fVScale;
      param[2] = 0;
      param[3] = 0;
      if (pBindScale)
        vpD3D->mfParameter4f(pBindScale, param);

      Plane plane;

      float intens = -0.25f / fDist;
      Plane plane00;
      plane00.n.x = intens*m_CameraMatrix(0,2);
      plane00.n.y = intens*m_CameraMatrix(1,2);
      plane00.n.z = intens*m_CameraMatrix(2,2);
      plane00.d     = intens*m_CameraMatrix(3,2);

      plane = TransformPlane(m_RP.m_pCurObject->m_Matrix, plane00);
      plane.d += 0.5f;
      if (pBindTG0)
        vpD3D->mfParameter4f(pBindTG0, &plane.n.x);
      plane.n.x = plane.n.y = plane.n.z = 0;
      plane.d = 0.49f;
      if (pBindTG1)
        vpD3D->mfParameter4f(pBindTG1, param);

      // Draw primitives
      EF_Draw(sh, NULL);

      fDist /= 2.0f;
      if (fDistToCam > fDist+1.0f)
        break;
      fUScale *= 2.0f;
      fVScale *= 2.0f;
    }

    m_RP.m_VPDetail->mfSet(false, NULL, 1);
    if (m_RP.m_RCDetail)
      m_RP.m_RCDetail->mfSet(false);
  }
  CD3D8TexMan::BindNULL(1);
}

void CD3D8Renderer::EF_DrawDecalOverlay()
{
  // Usually it means first pass in indoor engine (before shadow pass)
  if (m_RP.m_ObjFlags & FOB_ZPASS)
    return;

  if (!m_RP.m_pShaderResources || !m_RP.m_pShaderResources->m_Textures[EFTT_DECAL_OVERLAY])
    return;

  SEfResTexture *rt = m_RP.m_pShaderResources->m_Textures[EFTT_DECAL_OVERLAY];
  SShader *sh = m_RP.m_pShader;

  SelectTMU(0);
  rt->m_TU.m_TexPic->Set();

  if (m_RP.m_FlagsPerFlush & RBSI_WASDEPTHWRITE)
    SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_DEPTHFUNC_EQUAL);
  else
    SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);
  EF_SetColorOp(eCO_REPLACE);

  bool bMatrChanged = false;

}

void CD3D8Renderer::EF_DrawLayersHW(SShaderTechnique *hs, SShader *pSH)
{
  SShaderPassHW *slw;
  int i;
  int pass;

  pass = hs->m_Passes.Num();
  if (!pass)
    return;

  if ((m_RP.m_ObjFlags & FOB_LIGHTPASS) && (pSH->m_Flags & EF_USELIGHTS))
    return;
  if (m_RP.m_ObjFlags & FOB_FOGPASS)
    return;

  CVProgram *curVP = hs->m_VProgram;
  CVProgram *newVP;
  
  // Enable vertex shader
  if (curVP)
    curVP->mfSet(true, NULL);

  slw = &hs->m_Passes[0];
  for (i=0; i<pass; i++, slw++)
  {
    /*if (slw->m_Conditions)
    {
      SPassConditions *pc = slw->m_Conditions;
      if (pc->m_Flags & SHPF_AMBIENT)
      {
        if (m_RP.m_pCurObject && (m_RP.m_pCurObject->m_ObjFlags & FOB_LIGHTPASS))
          continue;
      }
      if (pc->m_Flags & SHPF_HASLM)
      {
        if (!m_RP.m_pShaderResources || !m_RP.m_pShaderResources->m_Textures[EFTT_LIGHTMAP].m_TU.m_TexPic)
          continue;
      }
    }*/
    if (CRenderer::CV_r_log >= 4)
      Logv(SRendItem::m_RecurseLevel, "+++ Pass %d\n", m_RP.m_RendPass);

    m_RP.m_StatNumPasses++;
    m_RP.m_CurrPass = slw;

    if (slw->m_LightMaterial)
    {
      if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_LMaterial)
        m_RP.m_pShaderResources->m_LMaterial->mfApply(slw->m_LMFlags);
      else
        slw->m_LightMaterial->mfApply(slw->m_LMFlags);
    }

    if (slw->m_Stencil)
      slw->m_Stencil->mfSet();

    EF_Eval_DeformVerts(slw->m_Deforms);
    EF_Eval_TexGen(slw);
    EF_Eval_RGBAGen(slw);

    if (slw->m_VProgram)
      newVP = slw->m_VProgram;
    else
      newVP = hs->m_VProgram;
    
    // Set vertex program for the current pass if needed
    if (newVP != curVP)
    {
      if (curVP)
      {
        curVP->mfSet(false, slw);
        curVP = NULL;
      }
      if (newVP)
      {
        curVP = newVP;
        curVP->mfSet(true, slw);
      }
    }

    //EF_SetArrayPointers(slw->m_Pointers, 1);
    EF_ApplyMatrixOps(slw->m_MatrixOps, true);

    if (!curVP)
    {
      HRESULT hr = m_pd3dDevice->SetVertexShader(m_RP.m_D3DFixedPipeline[m_RP.m_FlagsModificators&7][m_RP.m_CurD3DVFormat].m_Handle);    
      if (FAILED(hr))
        continue;
    }

    // Unlock all VB (if needed) and set current streams
    EF_PreDraw();
    
    // Set all textures and HW TexGen modes for the current pass (ShadeLayer)
    if (slw->mfSetTextures(true))
    {
      // Set Render states for the current pass
      if (m_RP.m_pCurObject->m_RenderState)
        SetState(m_RP.m_pCurObject->m_RenderState);
      else
      {
        if (m_RP.m_RendPass || (m_RP.m_ObjFlags & FOB_LIGHTPASS))
          SetState(slw->m_SecondRenderState);
        else
          SetState(slw->m_RenderState);
      }
      
      // Set Pixel shaders and Register combiners for the current pass
      if (slw->m_FShader)
        slw->m_FShader->mfSet(true, slw);

      EF_Draw(pSH, slw);

      m_RP.m_RendPass++;

      // Reset Pixel shaders and Register combiners for the current pass
      if (slw->m_FShader)
        slw->m_FShader->mfSet(false, slw);
    }

    // Reset HW TexGen modes for the current pass (ShadeLayer)
    slw->mfSetTextures(false);
    
    EF_ApplyMatrixOps(slw->m_MatrixOps, false);

    if (slw->m_Stencil)
      slw->m_Stencil->mfReset();

    if (slw->m_LightMaterial)
      slw->m_LightMaterial->mfDisable();
  }

  // Disable vertex shader
  if (curVP)
    curVP->mfSet(false, NULL);
}

void CD3D8Renderer::EF_DrawLightsHW(SShaderTechnique *hs, SShader *ef, int Stage)
{
  SShaderPassHW *slw;
  int i, l;
  int nl = 0;
  int n;

  if (!hs->m_LightPasses[Stage].Num())
    return;

  CVProgram *curVP = hs->m_VProgram;
  CVProgram *newVP;
  
  // Set vertex shader
  if (curVP)
    curVP->mfSet(true, NULL);

  // For each light source
  for (l=0; l<m_RP.m_NumActiveDLights; l++)
  {
    m_RP.m_pCurLight = m_RP.m_pActiveDLights[l];
    m_RP.m_nCurLight = m_RP.m_pCurLight->m_Id;
    TArray<SShaderPassHW>* ll;
    ll = &hs->m_LightPasses[Stage];
    nl++;

    // Ignore diffuse passes for specular only light sources
    if ((m_RP.m_pCurLight->m_Flags & DLF_LM) && Stage==0)
    {
      if ((m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_LIGHTMAP_DIR]) || m_RP.m_pCurObject->m_nLMId)
        continue;
    }

    if (m_RP.m_pStateShader && m_RP.m_pStateShader->m_State && m_RP.m_pStateShader->m_State->m_Stencil)
    {
      if (m_RP.m_pCurLight->m_Flags & DLF_CASTSHADOW_VOLUME)
        m_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
      else
        m_pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    }

    n = 0;
    bool bBreak = false;
    // For each layer/pass
    for (i=0; i<ll->Num(); i++)
    {
      slw = &ll[0][i];
      int msk;
      if (msk = (slw->m_LightFlags & DLF_LIGHTTYPE_MASK))
      {
        if (!(msk & (m_RP.m_pCurLight->m_Flags & DLF_LIGHTTYPE_MASK)))
          continue;
        if (slw->m_LightFlags & DLF_LM)
        {
          if (!(m_RP.m_pCurLight->m_Flags & DLF_LM))
            continue;
          if ((!m_RP.m_pShaderResources || !m_RP.m_pShaderResources->m_Textures[EFTT_LIGHTMAP_DIR]) && !m_RP.m_pCurObject->m_nLMId)
            continue;
          assert (m_RP.m_pCurLight->m_SpecColor.r!=0 || m_RP.m_pCurLight->m_SpecColor.g!=0 || m_RP.m_pCurLight->m_SpecColor.b!=0);
        }
      }
      if (slw->m_LMFlags & LMF_NOBUMP)
      {
        if (!m_RP.m_pShaderResources || !m_RP.m_pShaderResources->m_Textures[EFTT_BUMP] || !m_RP.m_pShaderResources->m_Textures[EFTT_BUMP]->m_TU.m_bNoBump)
          continue;
        bBreak = true;
      }
      else
      if (bBreak)
        break;

      if (EF_RejectLightPass(slw))
        continue;

      if (!n)
      {
        if (!(m_RP.m_StatLightMask & (1<<m_RP.m_nCurLight)))
        {
          m_RP.m_StatLightMask |= (1<<m_RP.m_nCurLight);
          m_RP.m_StatNumLights++;
        }
      }

      if (CRenderer::CV_r_log >= 3)
        Logv(SRendItem::m_RecurseLevel, "+++ Light pass %d for light %d\n", m_RP.m_RendPass, l);

      n++;
      m_RP.m_StatNumLightPasses++;

      m_RP.m_CurrPass = slw;

      // Set stencil states for the current pass if needed
      if (slw->m_Stencil)
        slw->m_Stencil->mfSet();

      EF_Eval_TexGen(slw);
      EF_Eval_RGBAGen(slw);

      //EF_SetArrayPointers(slw->m_Pointers, 1);
      EF_ApplyMatrixOps(slw->m_MatrixOps, true);
      
      if (slw->m_VProgram)
        newVP = slw->m_VProgram;
      else
        newVP = hs->m_VProgram;
      
      // Set vertex program for the current pass if needed
      if (newVP != curVP)
      {
        if (curVP)
        {
          curVP->mfSet(false, slw);
          curVP = NULL;
        }
        if (newVP)
        {
          curVP = newVP;
          curVP->mfSet(true, slw);
        }
      }
      if (!curVP)
      {
        HRESULT hr = m_pd3dDevice->SetVertexShader(m_RP.m_D3DFixedPipeline[m_RP.m_FlagsModificators&7][m_RP.m_CurD3DVFormat].m_Handle);
        if (FAILED(hr))
          continue;
      }

      if (slw->m_LightMaterial)
      {
        if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_LMaterial)
          m_RP.m_pShaderResources->m_LMaterial->mfApply(slw->m_LMFlags | (l<<LMF_LIGHT_SHIFT));
        else
          slw->m_LightMaterial->mfApply(slw->m_LMFlags | (l<<LMF_LIGHT_SHIFT));
      }

      // Unlock all VB (if needed) and set current streams
      EF_PreDraw();
      
      // Set all textures and HW TexGen modes for the current pass (ShadeLayer)
      if (slw->mfSetTextures(true))
      {
        // Set Render states for the current pass
        if (m_RP.m_pCurObject->m_RenderState)
          SetState(m_RP.m_pCurObject->m_RenderState);
        else
        {
          if (m_RP.m_RendPass || (m_RP.m_ObjFlags & FOB_LIGHTPASS))
            SetState(slw->m_SecondRenderState);
          else
            SetState(slw->m_RenderState);
        }

        // Set Pixel shaders and Register combiners for the current pass
        if (slw->m_FShader)
          slw->m_FShader->mfSet(true, slw);

        EF_Draw(ef, slw);

        m_RP.m_RendPass++;

        // Reset Pixel shaders and Register combiners for the current pass
        if (slw->m_FShader)
          slw->m_FShader->mfSet(false, slw);
      }

      // Reset HW TexGen modes for the current pass (ShadeLayer)
      slw->mfSetTextures(false);
      
      EF_ApplyMatrixOps(slw->m_MatrixOps, false);

      if (slw->m_Stencil)
        slw->m_Stencil->mfReset();

      // Disable light materials for the current pass
      if (slw->m_LightMaterial)
        SLightMaterial::mfDisable();
    }
  }

  // Disable vertex shader
  if (curVP)
    curVP->mfSet(false, NULL);

  m_RP.m_nCurLight = -1;
  m_RP.m_pCurLight = NULL;
}

void CD3D8Renderer::EF_FlushHW()
{
  guard(CD3D8Renderer::EF_FlushHW);

  SShader *ef = m_RP.m_pShader;

  int nPolys = m_nPolygons;

  int nHW;
  int nPrevObjMask = m_RP.m_pPrevRObject ? (m_RP.m_pPrevRObject->m_ObjFlags & FOB_MASKCONDITIONS) : 0;
  int nObjMask = m_RP.m_ObjFlags & FOB_MASKCONDITIONS;

  if (!EF_BuildLightsList())
    iLog->Log("WARNING: CGLRenderer::EF_BuildLightsList: Too many light sources per render item (> 16). Shader: '%s'\n", m_RP.m_pShader->m_Name.c_str());

  //if (m_RP.m_PrevShaderID == ef->m_Id && m_RP.m_PrevDynLMask == m_RP.m_DynLMask && nPrevObjMask == nObjMask)
  //  nHW = m_RP.m_CurTechnique;
  //else
  {
    m_RP.m_PrevShaderID = ef->m_Id;
    m_RP.m_PrevDynLMask = m_RP.m_DynLMask;
    m_RP.m_pPrevRObject = m_RP.m_pCurObject;

    nHW = EF_SelectHWTechnique(ef);
    m_RP.m_CurTechnique = nHW;
  }
  SShaderTechnique *hs = ef->m_HWTechniques[nHW];

  if (hs->m_Flags & FHF_TANGENTSUSED)
    m_RP.m_FlagsModificators |= RBMF_TANGENTSUSED;
  if (hs->m_Flags & FHF_LMTCUSED)
    m_RP.m_FlagsModificators |= RBMF_LMTCUSED;

  if (m_RP.m_pRE)
    m_RP.m_pRE->mfCheckUpdate(hs->m_Flags);

  if (ef->m_Flags & EF_NEEDNORMALS)
    EF_EvalNormalsRB(ef);
  EF_Eval_DeformVerts(ef->m_Deforms);

  if (ef->m_Flags & EF_POLYGONOFFSET)
  {
    if (m_d3dCaps.RasterCaps & D3DPRASTERCAPS_ZBIAS)
      m_pd3dDevice->SetRenderState( D3DRS_ZBIAS, CV_d3d8_decaloffset );
  }

  if (!(m_RP.m_FlagsPerFlush & RBSI_NOCULL))
  {
    if (hs->m_eCull != -1)
      D3DSetCull(hs->m_eCull);
    else
      D3DSetCull(ef->m_eCull);
  }

  EF_ApplyMatrixOps(hs->m_MatrixOps, true);

  if (hs->m_Flags & FHF_FIRSTLIGHT)
  {
    EF_DrawLightsHW(hs, ef, 0); // Diffuse passes
    EF_DrawLayersHW(hs, ef);    // Ambient passes
    EF_DrawLightsHW(hs, ef, 1); // Specular passes
  }
  else
  {
    EF_DrawLayersHW(hs, ef);    // Ambient passes
    EF_DrawLightsHW(hs, ef, 0); // Diffuse passes
    EF_DrawLightsHW(hs, ef, 1); // Specular passes
  }

  EF_ApplyMatrixOps(hs->m_MatrixOps, false);

  if (ef->m_Flags & EF_POLYGONOFFSET)
    m_pd3dDevice->SetRenderState(D3DRS_ZBIAS, 0);

  if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_DECAL_OVERLAY] && CV_r_decaltextures)
    EF_DrawDecalOverlay();

  if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_DETAIL_OVERLAY] && CV_r_detailtextures)
    EF_DrawDetailOverlay();

  if (m_RP.m_pFogVolume && CV_r_VolumetricFog)
    EF_DrawFogOverlay();

  unguard;
}

bool CD3D8Renderer::EF_SetResourcesState(bool bSet)
{
  bool bRes = true;
  if (bSet)
  {
    if ((m_RP.m_ObjFlags & FOB_ZPASS) && (!(m_RP.m_pShader->m_Flags2 & EF2_OPAQUE) || m_RP.m_pShaderResources->m_Opacity != 1.0f))
      return false;

    if (m_RP.m_pShaderResources->m_ResFlags & MTLFLAG_2SIDED)
    {
      D3DSetCull(eCULL_None);
      m_RP.m_FlagsPerFlush |= RBSI_NOCULL;
    }
    if (m_RP.m_pShaderResources->m_AlphaRef)
    {
      if (!(mCurState & GS_ALPHATEST_MASK))
        m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
      m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)(m_RP.m_pShaderResources->m_AlphaRef*255.0f));
      m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
      m_RP.m_FlagsPerFlush |= RBSI_ALPHATEST;
    }
    else
    if (m_RP.m_pShaderResources->m_Opacity != 1.0f)
    {
      m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      mCurState &= ~GS_BLEND_MASK;
      if (m_RP.m_pShaderResources->m_ResFlags & MTLFLAG_ADDITIVE)
      {
        mCurState |= GS_BLSRC_ONE | GS_BLDST_ONE;
        m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
        m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
      }
      else
      {
        mCurState |= GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;
        m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
        m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
      }
      m_RP.m_FlagsPerFlush |= RBSI_ALPHABLEND;

      if (mCurState & GS_DEPTHWRITE)
      {
        mCurState &= ~GS_DEPTHWRITE;
        m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
      }
      m_RP.m_FlagsPerFlush |= RBSI_DEPTHWRITE;

      m_RP.m_fCurOpacity = m_RP.m_pShaderResources->m_Opacity;
      SetMaterialColor(1, 1, 1, m_RP.m_fCurOpacity);

      m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
      m_RP.m_ObjFlags &= ~FOB_LIGHTPASS;
    }
  }
  else
  {
    if (m_RP.m_pShaderResources->m_AlphaRef)
    {
      switch (mCurState & GS_ALPHATEST_MASK)
      {
        case GS_ALPHATEST_GREATER0:
          m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
          m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0);
          m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
          break;
          
        case GS_ALPHATEST_LESS128:
          m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
          m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x80);
          m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_LESS);
          break;
          
        case GS_ALPHATEST_GEQUAL128:
          m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
          m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x80);
          m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
          break;

        default:
          m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
          break;
      }
    }
  }
  return true;
}

void CD3D8Renderer::EF_SetStateShaderState()
{
  SShader *ef = m_RP.m_pStateShader;
  SEfState *st = ef->m_State;

  if (st->m_bClearStencil)
    m_pd3dDevice->Clear(0, NULL, D3DCLEAR_STENCIL, 0, 1.0f, 0);
  
  if (st->m_Stencil)
    st->m_Stencil->mfSet();
  if (st->m_Flags & ESF_STATE)
  {
    SetState(st->m_State);
  }
  bool bSetCol = false;
  UCol Col;
  Col.dcolor = 0;
  if (st->m_Flags & ESF_RGBGEN)
  {
    m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
    switch (st->m_eEvalRGB)
    {
    case eERGB_Fixed:
      Col.bcolor[0] = st->m_FixedColor[0];
      Col.bcolor[1] = st->m_FixedColor[1];
      Col.bcolor[2] = st->m_FixedColor[2];
      Col.bcolor[3] = st->m_FixedColor[3];
      break;
    }
    bSetCol = true;
  }
  if (st->m_Flags & ESF_ALPHAGEN)
  {
    m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
    switch (st->m_eEvalAlpha)
    {
      case eEALPHA_Fixed:
        Col.bcolor[3] = st->m_FixedColor[3];
        break;
    }
    bSetCol = true;
  }
  if (bSetCol)
    EF_SetGlobalColor(Col);
  if (st->m_Flags & ESF_COLORMASK)
  {
    m_RP.m_FlagsPerFlush |= RBSI_COLORMASK;
    int mask = 0;
    for (int i=0; i<4; i++)
    {
      if (st->m_ColorMask[i])
        mask |= 1<<i;
    }
    m_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, mask);
  }
}

void CD3D8Renderer::EF_ResetStateShaderState()
{
  SShader *ef = m_RP.m_pStateShader;
  SEfState *st = ef->m_State;

  if (st->m_Stencil)
    st->m_Stencil->mfReset();

  if (st->m_Flags & ESF_COLORMASK)
    m_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
}

void CD3D8Renderer::EF_FlushShader()
{
  SShader *ef = m_RP.m_pShader;
  int i;
  SShaderPass *sfm;

  guard(CD3D8Renderer::EF_FlushShader);

  EF_SetVertColor();
  
  if (m_RP.m_pRE)  // Without buffer fill
    EF_InitEvalFuncs(1);  // VP
  else
  { // Buffer filling
    EF_InitEvalFuncs(0);
    if (!m_RP.m_RendNumIndices)
      return;

    if ((ef->m_Flags & EF_FOGSHADER) && !m_RP.m_pFogVolume)
      return;

    if (m_RP.m_RendNumIndices > m_RP.m_MaxTris*3)
    {
      iLog->Log("CD3D8Renderer::EF_FlushShader: Shader MAX_RENDTRIS hit\n");
      m_RP.m_RendNumIndices = m_RP.m_MaxTris*3;
    }
    if (m_RP.m_RendNumVerts > m_RP.m_MaxVerts)
    {
      iLog->Log("CD3D8Renderer::EF_FlushShader: Shader MAX_RENDVERTS hit\n");
      m_RP.m_RendNumVerts = m_RP.m_MaxVerts;
    }
  }

  if (m_RP.m_pShaderResources)
  {
    if (!EF_SetResourcesState(true))
      return;
  }

  bool bLM = false;
  if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_LMaterial)
  {
    m_RP.m_pShaderResources->m_LMaterial->mfApply(ef->m_LMFlags);
    bLM = true;
  }
  else
  if (ef->m_LightMaterial)
  {
    ef->m_LightMaterial->mfApply(ef->m_LMFlags);
    bLM = true;
  }

  if (m_RP.m_pStateShader && m_RP.m_pStateShader->m_State)
    EF_SetStateShaderState();

  bool bDraw2D = false;
  if (!(m_RP.m_Flags & RBF_2D) && (m_RP.m_FlagsPerFlush & RBSI_DRAWAS2D))
  {
    bDraw2D = true;
    Set2DMode(true,800,600);
  }

  // Draw using HW shader 
  if (ef->m_HWTechniques.Num())
  {
    EF_FlushHW();
    if (m_RP.m_pStateShader && m_RP.m_pStateShader->m_State)
      EF_ResetStateShaderState();
    if (m_RP.m_pShaderResources)
      EF_SetResourcesState(false);
    if (bLM)
      SLightMaterial::mfDisable();
    if (bDraw2D)
      Set2DMode(false,800,600);
    return;
  }

  if ((ef->m_Flags & EF_FOGSHADER) && !m_RP.m_pFogVolume)
  {
    if (m_RP.m_pStateShader && m_RP.m_pStateShader->m_State)
      EF_ResetStateShaderState();
    if (m_RP.m_pShaderResources)
      EF_SetResourcesState(false);
    if (bLM)
      SLightMaterial::mfDisable();
    if (bDraw2D)
      Set2DMode(false,800,600);
    return;
  }

  CD3D8TexMan::BindNULL(1);
  EF_SelectTMU(0);

  if (m_RP.m_pRE)
    m_RP.m_pRE->mfCheckUpdate(0);

  if (ef->m_Flags & EF_NEEDNORMALS)
    EF_EvalNormalsRB(ef);
  EF_Eval_DeformVerts(ef->m_Deforms);

  if (ef->m_Flags & EF_POLYGONOFFSET)
    m_pd3dDevice->SetRenderState( D3DRS_ZBIAS, CV_d3d8_decaloffset );

  if (!(m_RP.m_FlagsPerFlush & RBSI_NOCULL))
    D3DSetCull(ef->m_eCull);

  int nPolys = m_nPolygons;

  sfm = &ef->m_Passes[0];
  for (i=0; i<ef->m_Passes.Num(); i++, sfm++)
  {
    if (sfm->m_Stencil)
      sfm->m_Stencil->mfSet();
    EF_Eval_TexGen(sfm);
    EF_Eval_RGBAGen(sfm);

    HRESULT hr = m_pd3dDevice->SetVertexShader(m_RP.m_D3DFixedPipeline[m_RP.m_FlagsModificators&7][m_RP.m_CurD3DVFormat].m_Handle);
    if (FAILED(hr))
      continue;

    // Unlock all VB (if needed) and set current stream
    EF_PreDraw();

    // Set all textures and HW TexGen modes for the current pass (ShadeLayer)
    if (sfm->mfSetTextures(true))
    {
      // Set Render states for the current pass
      SetState(m_RP.m_RendPass ? sfm->m_SecondRenderState : sfm->m_RenderState);

      EF_Draw(ef, sfm);

      m_RP.m_RendPass++;
    }
    if (sfm->m_Stencil)
      sfm->m_Stencil->mfReset();
  }

  if (ef->m_Flags & EF_POLYGONOFFSET)
    m_pd3dDevice->SetRenderState(D3DRS_ZBIAS, 0);

  if (bLM)
    SLightMaterial::mfDisable();

  if (m_RP.m_pStateShader && m_RP.m_pStateShader->m_State)
    EF_ResetStateShaderState();
  if (m_RP.m_pShaderResources)
    EF_SetResourcesState(false);

  if (bDraw2D)
    Set2DMode(false,800,600);

  unguardf(("(Shader: '%s')",ef->m_Name));
}

void CD3D8Renderer::EF_Flush()
{
  CD3D8Renderer *rd = gcpRendD3D;
  SShader *ef = rd->m_RP.m_pShader;
  if (rd->m_RP.m_ExcludeShader)
  {
    if (!strcmp(rd->m_RP.m_ExcludeShader, ef->m_Name.c_str()))
      return;
    if(rd->m_RP.m_ExcludeShader[0] == '#')
    if (strstr(ef->m_Name.c_str(), rd->m_RP.m_ExcludeShader))
      return;
  }
  if (rd->m_RP.m_ShowOnlyShader)
  {
    if (strcmp(rd->m_RP.m_ShowOnlyShader, ef->m_Name.c_str()))
      return;
  }
  if (ef->m_Flags2 & EF2_PREPR_PORTAL)
  {
    if (CV_r_portals && (rd->m_RP.m_PersFlags & (RBPF_DRAWPORTAL | RBPF_DRAWMIRROR)))
      return;
  }

  if (rd->m_LogFile && CV_r_log == 3)
    rd->Logv(SRendItem::m_RecurseLevel, ".. Start Flush: '%s' ..\n", ef->m_Name.c_str());

  rd->m_RP.m_StatNumLightPasses = 0;
  rd->m_RP.m_StatNumLights = 0;
  rd->m_RP.m_StatNumPasses = 0;
  rd->m_RP.m_StatLightMask = 0;

  rd->m_RP.m_PS.m_NumRendShaders++;

  CCObject *obj = rd->m_RP.m_pCurObject;
  rd->m_RP.m_ObjFlags = obj->m_ObjFlags;
  if ((rd->m_RP.m_ObjFlags & FOB_ZPASS) || !CV_r_hwlights)
    rd->m_RP.m_DynLMask = 0;
  else
    rd->m_RP.m_DynLMask |= obj->m_DynLMMask;

  if (CV_r_showlight->GetString()[0] != '0')
  {
    for (int i=0; i<rd->m_RP.m_DLights.Num(); i++)
    {
      CDLight *dl = rd->m_RP.m_DLights[i];
      if (!dl)
        continue;
      char *str = CV_r_showlight->GetString();
      if (strcmp(str, "0") != 0)
      {
        if (!dl->m_Name || !strstr(dl->m_Name, str))
          rd->m_RP.m_DynLMask &= ~(1<<i);
      }
    }
  }

  if ((ef->m_Flags3 & EF3_CLIPPLANE) && !(rd->m_RP.m_PersFlags & RBPF_SETCLIPPLANE) && !rd->m_RP.m_ClipPlaneEnabled)
  {
    switch (ef->m_Flags3 & EF3_CLIPPLANE)
    {
      case EF3_CLIPPLANE_WATER_FRONT:
      case EF3_CLIPPLANE_WATER_BACK:
        {
          SPlane Pl;
          Pl.m_Normal = Vec3d (0, 0, 1);
          Pl.m_Dist = iSystem->GetI3DEngine()->GetWaterLevel();
          if (ef->m_Flags3 & EF3_CLIPPLANE_WATER_BACK)
          {
            Pl.m_Normal = -Pl.m_Normal;
            Pl.m_Dist = -Pl.m_Dist;
          }
          Pl.m_Dist = -Pl.m_Dist;
          rd->EF_SetClipPlane(true, &Pl.m_Normal.x, false);
          rd->m_RP.m_PersFlags |= RBPF_SETCLIPPLANE;
        }
    }
  }
  else
  if (!(ef->m_Flags3 & EF3_CLIPPLANE) && (rd->m_RP.m_PersFlags & RBPF_SETCLIPPLANE))
  {
    rd->EF_SetClipPlane(false, NULL, false);
    rd->m_RP.m_PersFlags &= ~RBPF_SETCLIPPLANE;
  }

  bool bProfile;
  double time0 = 0;
  int nNumPolys = rd->m_nPolygons;
  if (CRenderer::CV_r_profileshaders || CRenderer::CV_r_log>=2)
  {
    bProfile = true;
    ticks(time0);
  }
  else
    bProfile = false;

  if (rd->m_LogFile && CV_r_log >= 3)
    rd->Logv(SRendItem::m_RecurseLevel, "\n");

  rd->EF_FlushShader();

  nNumPolys = rd->m_nPolygons - nNumPolys;

  if (bProfile)
  {
    unticks(time0);
    time0 = time0*1000.0*rd->m_RP.m_SecondsPerCycle;
  }

  if (CRenderer::CV_r_profileshaders)
  {
    SProfInfo pi;
    pi.Time = time0;
    pi.NumPolys = nNumPolys;
    pi.ef = ef;
    rd->m_RP.m_Profile.AddElem(pi);
  }

  if (rd->m_RP.m_DynLMask)
    rd->m_RP.m_PS.m_NumLitShaders++;

  if (rd->m_LogFile)
  {
    if (CV_r_log == 4 && rd->m_RP.m_DynLMask)
    {
      for (int n=0; n<rd->m_RP.m_DLights.Num(); n++)
      {
        CDLight *dl = rd->m_RP.m_DLights[n];
        if (rd->m_RP.m_DynLMask & (1<<n))
          rd->Logv(SRendItem::m_RecurseLevel, "   Light %d (\"%s\")\n", n, dl->m_Name ? dl->m_Name : "<Unknown>");
      }
    }

    char *str;
    if (ef->m_HWTechniques.Num())
      str = "FlushHW";
    else
      str = "Flush";
    rd->Logv(SRendItem::m_RecurseLevel, "%s: '%s', (St: %s) Id:%d, Obj:%d(%s), Tech: %d, Cp: %d, Fog:%d, VF:%d, NL:%d, LPas:%d, Pas:%d (time: %f, %d polys)\n", str, ef->m_Name.c_str(), rd->m_RP.m_pStateShader ? rd->m_RP.m_pStateShader->m_Name.c_str() : "NULL", ef->m_Id, rd->m_RP.m_pCurObject->m_VisId, (rd->m_RP.m_pCurObject->m_ObjFlags & FOB_USEMATRIX) ? "Matr" : "NoMatr", rd->m_RP.m_CurTechnique, rd->m_RP.m_ClipPlaneEnabled, rd->m_RP.m_pFogVolume ? (rd->m_RP.m_pFogVolume-&rd->m_RP.m_FogVolumes[0]) : 0, rd->m_RP.m_pShader->m_VertexFormatId, rd->m_RP.m_StatNumLights, rd->m_RP.m_StatNumLightPasses, rd->m_RP.m_StatNumPasses, time0, nNumPolys);
    if (rd->m_RP.m_FlagsPerFlush & RBSI_ALPHATEST)
      rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x), LM: %d, (AT)\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_DynLMask);
    else
    if (rd->m_RP.m_FlagsPerFlush & RBSI_ALPHABLEND)
      rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x) (AB), LM: %d (Dist: %.3f)\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_DynLMask, rd->m_RP.m_pRE ? rd->m_RP.m_pRE->mfDistanceToCameraSquared(*gRenDev->m_RP.m_pCurObject) : 0);
    else
      rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x), LM: %d\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_DynLMask);
  }
}

void CD3D8Renderer::EF_EndEf3D(bool bSort)
{
  double time0 = 0;
  ticks(time0);

  if (CV_r_nodrawshaders == 1)
  {
    SetClearColor(Vec3d(0,0,0));
    EF_ClearBuffer(false, NULL);
    return;
  }

  m_RP.m_PersFlags &= ~(RBPF_DRAWNIGHTMAP | RBPF_DRAWHEATMAP);

  // tiago: commented out this. This is done in 3dengine now
  //if (CV_r_glare && SRendItem::m_RecurseLevel == 1)  
  //  EF_AddEf(0, m_RP.m_pREGlare, gcEf.m_GlareShader, NULL, NULL, -1);

  if (CV_r_fullbrightness)  
  {
    EF_SetGlobalColor(1, 1, 1, 1);
    m_RP.m_FlagsPerFlush |= RBSI_RGBGEN | RBSI_ALPHAGEN;
  }

  m_RP.m_RealTime = iTimer->GetCurrTime();

  if (CV_r_excludeshader->GetString()[0] != '0')
    m_RP.m_ExcludeShader = CV_r_excludeshader->GetString();
  else
    m_RP.m_ExcludeShader = NULL;

  if (CV_r_showonlyshader->GetString()[0] != '0')
    m_RP.m_ShowOnlyShader = CV_r_showonlyshader->GetString();
  else
    m_RP.m_ShowOnlyShader = NULL;

  EF_UpdateSplashes(m_RP.m_RealTime);
  EF_AddClientPolys3D();
  EF_AddClientPolys2D();

  EF_RenderPipeLine(EF_Flush);

  EF_DrawDebugTools();
  EF_RemovePolysFromScene();

  SRendItem::m_RecurseLevel--;

  unticks(time0);
  m_RP.m_PS.m_fFlushTime += (float)(time0*1000.0*m_RP.m_SecondsPerCycle);
}

void CD3D8Renderer::EF_RenderPipeLine(void (*RenderFunc)())
{
  int numepp = SRendItem::m_RendItems[EFSLIST_PREPROCESS_ID].Num();
  int numeds = SRendItem::m_RendItems[EFSLIST_DISTSORT_ID].Num();
  int numeus = SRendItem::m_RendItems[EFSLIST_UNSORTED_ID].Num();
  int numegn = SRendItem::m_RendItems[EFSLIST_GENERAL_ID].Num();
  int numels = SRendItem::m_RendItems[EFSLIST_LAST_ID].Num();

  int numspp = SRendItem::m_StartRI[EFSLIST_PREPROCESS_ID];
  int numsds = SRendItem::m_StartRI[EFSLIST_DISTSORT_ID];
  int numsus = SRendItem::m_StartRI[EFSLIST_UNSORTED_ID];
  int numsgn = SRendItem::m_StartRI[EFSLIST_GENERAL_ID];
  int numsls = SRendItem::m_StartRI[EFSLIST_LAST_ID];

  EF_PipeLine(numspp, numepp, EFSLIST_PREPROCESS_ID, 1, RenderFunc);  // Preprocess and probably sky
  EF_PipeLine(numsus, numeus, EFSLIST_UNSORTED_ID, 0, RenderFunc);   // Unsorted list for indoor
  EF_PipeLine(numsgn, numegn, EFSLIST_GENERAL_ID, 1, RenderFunc);    // Sorted list without preprocess
  EF_PipeLine(numsds, numeds, EFSLIST_DISTSORT_ID, 2, RenderFunc);   // Sorted by distance elements
  EF_PipeLine(numsls, numels, EFSLIST_LAST_ID, 1, RenderFunc);       // Sorted list without preprocess of all fog passes and screen shaders
}

void CD3D8Renderer::EF_PipeLine(int nums, int nume, int nList, int nSortType, void (*RenderFunc)())
{
  int i;
  CCObject *savedObj;
  SShader *Shader, *CurShader, *ShaderState, *CurShaderState;
  SRenderShaderResources *Res, *CurRes;
  int nObject, nCurObject;
  int nFog, nCurFog;
  
  if (nume > 16384)
  {
    nume = 16384;
    iLog->Log("CD3D8Renderer::EF_PipeLine: too many items for %d list ( > 16384 )\n", nList);
  }
  
  if (nume-nums < 1)
    return;

  m_RP.m_Flags |= RBF_3D;

  m_RP.m_pRenderFunc = RenderFunc;

  m_RP.m_nCurLightParam = -1;
  savedObj = m_RP.m_pCurObject;
  m_RP.m_pCurObject = m_RP.m_VisObjects[0];
  m_RP.m_pPrevObject = m_RP.m_pCurObject;

  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "*** Start frame for list %d ***\n", nList);

  if (nSortType == 1)
  {
    SRendItem::mfSort(&SRendItem::m_RendItems[nList][nums], nume-nums);

    // If sort number of the first shader is 1 (eS_Preprocess)
    // run preprocess operations for the current frame
    if ((SRendItem::m_RendItems[nList][nums].SortVal.i.High >> 26) == eS_PreProcess)
      nums += EF_Preprocess(&SRendItem::m_RendItems[nList][0], nums, nume);
  }
  else
  if (nSortType == 2)
  {
    SRendItem::mfSortByDist(&SRendItem::m_RendItems[nList][nums], nume-nums);
  }

  m_RP.m_Flags |= RBF_3D;

  EF_PreRender();
  EF_PushMatrix();

  UnINT64 oldVal;
  oldVal.SortVal = -1;
  nCurObject = -2;
  nCurFog = 0;
  CurShader = NULL;
  CurShaderState = NULL;
  CurRes = NULL;

  for (i=nums; i<nume; i++)
  {
    SRendItemPre *ri = &SRendItem::m_RendItems[nList][i];
    CRendElement *re = ri->Item;
    if (ri->SortVal.SortVal == oldVal.SortVal)
    {
      // Optimizations: not necessary to check of changing shaders and objects
      // if sort value is the same (contains the same info about shaders, objects, fog volumes, ...)
      re->mfPrepare();
      continue;
    }
    oldVal.SortVal = ri->SortVal.SortVal;

    SRendItem::mfGet(ri->SortVal, &nObject, &Shader, &ShaderState, &nFog, &Res);

    if (nObject != nCurObject)
    {
      if (CurShader)
      {
        m_RP.m_pRenderFunc();
        CurShader = NULL;
      }
      if (!EF_ObjectChange(Shader, nObject, re))
        continue;
      nCurObject = nObject;
    }

    if (Shader != CurShader || CurRes != Res || ShaderState != CurShaderState || nFog != nCurFog)
    {
      if (CurShader)
        m_RP.m_pRenderFunc();

      EF_Start(Shader, ShaderState, Res, nFog, re);

      nCurFog = nFog;
      CurShader = Shader;
      CurShaderState = ShaderState;
      CurRes = Res;
    }

    re->mfPrepare();
  }
  if (CurShader)
    m_RP.m_pRenderFunc();
  
  EF_PostRender();

  EF_PopMatrix();

  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "*** End frame ***\n\n");
  
  m_RP.m_pCurObject = savedObj;
}

void CD3D8Renderer::EF_DrawWire()
{
  if (CV_r_showlines == 1)
    gcpRendD3D->SetState(GS_POLYLINE | GS_NODEPTHTEST);
  else
  if (CV_r_showlines == 2)
    gcpRendD3D->SetState(GS_POLYLINE);
  gcpRendD3D->SetMaterialColor(1,1,1,1);
  CTexMan::m_Text_White->Set();
  gcpRendD3D->EF_SetColorOp(eCO_MODULATE);
  if (gcpRendD3D->m_RP.m_pRE)
    gcpRendD3D->m_RP.m_pRE->mfCheckUpdate(0);
  int StrVrt;
  void *verts = (void *)gcpRendD3D->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, 0);
  gcpRendD3D->EF_Draw(gcpRendD3D->m_RP.m_pShader, NULL);
}

void CD3D8Renderer::EF_DrawNormals()
{
  float len = CRenderer::CV_r_normalslength;
  if (gcpRendD3D->m_bEditor)
    len *= 100.0f;
  int StrVrt, StrNrm;
  byte *verts = (byte *)gcpRendD3D->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, FGP_SRC | FGP_REAL);
  byte *norms = (byte *)gcpRendD3D->EF_GetPointer(eSrcPointer_TNormal, &StrNrm, GL_FLOAT, eSrcPointer_TNormal, FGP_SRC | FGP_REAL);
  if ((int)norms > 256)
  {
    int numVerts = gcpRendD3D->m_RP.m_RendNumVerts;

    CTexMan::m_Text_White->Set();
    gcpRendD3D->EF_SetColorOp(eCO_MODULATE);
    gcpRendD3D->SetState(GS_POLYLINE);
    SPipeVertex_D_1T *Verts = new SPipeVertex_D_1T[numVerts*2];

    uint col0 = 0x000000ff;
    uint col1 = 0x00ffffff;

    for (int v=0; v<numVerts*2; v+=2,verts+=StrVrt,norms+=StrNrm)
    {
      float *fverts = (float *)verts;
      float *fnorms = (float *)norms;
      Verts[v].xyz[0] = fverts[0];
      Verts[v].xyz[1] = fverts[1];
      Verts[v].xyz[2] = fverts[2];
      Verts[v].color.dcolor = col0;

      Verts[v+1].xyz[0] = fverts[0] + fnorms[0]*len;
      Verts[v+1].xyz[1] = fverts[1] + fnorms[1]*len;
      Verts[v+1].xyz[2] = fverts[2] + fnorms[2]*len;
      Verts[v+1].color.dcolor = col1;
    }
    gcpRendD3D->m_pd3dDevice->SetVertexShader( D3DFVF_VERTEX_D_1T );
    h = gcpRendD3D->m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, numVerts, Verts, STRIDE_D_1T);
    delete [] Verts;
  }
}

void CD3D8Renderer::EF_DrawTangents()
{
  float len = CRenderer::CV_r_normalslength;
  if (gcpRendD3D->m_bEditor)
    len *= 100.0f;
  if (gcpRendD3D->m_RP.m_pRE)
    gcpRendD3D->m_RP.m_pRE->mfCheckUpdate(FHF_TANGENTSUSED);
  int StrVrt, StrTang, StrBinorm, StrTNorm;
  byte *verts = NULL;
  byte *tangs = NULL;
  byte *binorm = NULL;
  byte *tnorm = NULL;
  int flags = 0;
  if (CRenderer::CV_r_showtangents == 1)
    flags = FGP_SRC | FGP_REAL;
  else
    flags = FGP_REAL;
  verts = (byte *)gcpRendD3D->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, flags);
  tangs = (byte *)gcpRendD3D->EF_GetPointer(eSrcPointer_Tangent, &StrTang, GL_FLOAT, eSrcPointer_Tangent, flags);
  binorm = (byte *)gcpRendD3D->EF_GetPointer(eSrcPointer_Binormal, &StrBinorm, GL_FLOAT, eSrcPointer_Binormal, flags);
  tnorm = (byte *)gcpRendD3D->EF_GetPointer(eSrcPointer_TNormal, &StrTNorm, GL_FLOAT, eSrcPointer_TNormal, flags);
  if ((int)tangs>256 && (int)binorm>256 && (int)tnorm>256)
  {
    int numVerts = gcpRendD3D->m_RP.m_RendNumVerts;

    CTexMan::m_Text_White->Set();
    gcpRendD3D->EF_SetColorOp(eCO_MODULATE);
    if (gcpRendD3D->m_polygon_mode == R_SOLID_MODE)
      gcpRendD3D->SetState(GS_POLYLINE);
    else
      gcpRendD3D->SetState(0);
    SPipeVertex_D_1T *Verts = new SPipeVertex_D_1T[numVerts*6];

    for (int v=0; v<numVerts*6; v+=6,verts+=StrVrt,tangs+=StrTang, binorm+=StrBinorm, tnorm+=StrTNorm)
    {
      uint col0 = 0x00ff0000;
      uint col1 = 0x00ffffff;
      float *fverts = (float *)verts;
      float *fv = (float *)tangs;
      Verts[v].xyz[0] = fverts[0];
      Verts[v].xyz[1] = fverts[1];
      Verts[v].xyz[2] = fverts[2];
      Verts[v].color.dcolor = col0;

      Verts[v+1].xyz[0] = fverts[0] + fv[0]*len;
      Verts[v+1].xyz[1] = fverts[1] + fv[1]*len;
      Verts[v+1].xyz[2] = fverts[2] + fv[2]*len;
      Verts[v+1].color.dcolor = col1;

      col0 = 0x0000ff00;
      col1 = 0x00ffffff;
      fverts = (float *)verts;
      fv = (float *)binorm;
      Verts[v+2].xyz[0] = fverts[0];
      Verts[v+2].xyz[1] = fverts[1];
      Verts[v+2].xyz[2] = fverts[2];
      Verts[v+2].color.dcolor = col0;

      Verts[v+3].xyz[0] = fverts[0] + fv[0]*len;
      Verts[v+3].xyz[1] = fverts[1] + fv[1]*len;
      Verts[v+3].xyz[2] = fverts[2] + fv[2]*len;
      Verts[v+3].color.dcolor = col1;

      col0 = 0x000000ff;
      col1 = 0x00ffffff;
      fverts = (float *)verts;
      fv = (float *)tnorm;
      Verts[v+4].xyz[0] = fverts[0];
      Verts[v+4].xyz[1] = fverts[1];
      Verts[v+4].xyz[2] = fverts[2];
      Verts[v+4].color.dcolor = col0;

      Verts[v+5].xyz[0] = fverts[0] + fv[0]*len;
      Verts[v+5].xyz[1] = fverts[1] + fv[1]*len;
      Verts[v+5].xyz[2] = fverts[2] + fv[2]*len;
      Verts[v+5].color.dcolor = col1;
    }
    gcpRendD3D->m_pd3dDevice->SetVertexShader( D3DFVF_VERTEX_D_1T );
    h = gcpRendD3D->m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, numVerts*3, Verts, STRIDE_D_1T);
    delete [] Verts;
  }
}

void CD3D8Renderer::EF_DrawDebugLights()
{
  static int sFrame = 0;
  if (m_nFrameID != sFrame)
  {
    int i;
    sFrame = m_nFrameID;

    HRESULT hr = m_pd3dDevice->SetVertexShader( D3DFVF_VERTEX_D );

    for (i=0; i<m_RP.m_DLights.Num(); i++)
    {
      CDLight *dl = m_RP.m_DLights[i];
      if (!dl)
        continue;
      char *str = CV_r_showlight->GetString();
      if (strcmp(str, "0") != 0)
      {
        if (!dl->m_Name || strcmp(str, dl->m_Name))
          continue;
      }
      SetMaterialColor(dl->m_Color[0], dl->m_Color[1], dl->m_Color[2], dl->m_Color[3]);
      if (dl->m_Flags & DLF_DIRECTIONAL)
        DrawPoint(dl->m_Origin[0], dl->m_Origin[1], dl->m_Origin[2], 10);
      else
        if (dl->m_Flags & DLF_POINT)
          DrawBall(dl->m_Origin[0], dl->m_Origin[1], dl->m_Origin[2], 0.05f);
      if (dl->m_Flags & DLF_PROJECT)
      {
        Vec3d dir, rgt, org;

        dir = dl->m_Orientation.m_vForward;
        rgt = dl->m_Orientation.m_vRight;
        float ang = dl->m_fLightFrustumAngle;          
        if (ang == 0)
          ang = 45.0f;
        org = dl->m_Origin;

        dir *= 0.3f;

        CFColor Col = dl->m_Color;

        Matrix m;
        Vec3d vertex = dir;

			//	vertex = m.GetRotation(rgt, ang)*vertex;
				vertex = Matrix33::GetRotationAA33(ang*gf_DEGTORAD,GetNormalized(rgt)) * vertex; //NOTE: angle need to be in radians
			//	Matrix mat = m.GetRotation(dir, 60);
				Matrix mat = Matrix33::GetRotationAA33(60*gf_DEGTORAD,GetNormalized(dir)); //NOTE: angle need to be in radians
				Vec3d tmpvertex;
        int ctr;

        //fill the inside of the light
        EnableTMU(false);
        SPipeVertex_D Verts[32];
        memset(Verts, 0, sizeof(Verts));
        CFColor cl = Col*0.3f;
        int n = 0;
        Verts[n].xyz = org;
        Verts[n].color.dcolor = D3DRGBA(cl[0], cl[1], cl[2], 1.0f);
        n++;
        tmpvertex = org + vertex;
        Verts[n].xyz = tmpvertex;
        Verts[n].color.dcolor = D3DRGBA(Col[0], Col[1], Col[2], 1.0f);
        n++;
        for (ctr=0; ctr<6; ctr++)
        {
          vertex = mat * vertex;
          Vec3d tmpvertex = org + vertex;
          Verts[n].xyz = tmpvertex;
          Verts[n].color.dcolor = D3DRGBA(Col[0], Col[1], Col[2], 1.0f);
          n++;
        }
        m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, n-2, Verts, STRIDE_D);

        //draw the inside of the light with lines and the outside filled
        m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
        SetCullMode(R_CULL_NONE);
        n = 0;
        Verts[n].xyz = org;
        Verts[n].color.dcolor = D3DRGBA(0.3f, 0.3f, 0.3f, 1.0f);
        n++;
        tmpvertex = org + vertex;
        Verts[n].xyz = tmpvertex;
        Verts[n].color.dcolor = D3DRGBA(1.0f, 1.0f, 1.0f, 1.0f);
        n++;
        for (ctr=0; ctr<6; ctr++)
        {
          vertex = mat * vertex;
          Vec3d tmpvertex = org + vertex;
          Verts[n].xyz = tmpvertex;
          Verts[n].color.dcolor = D3DRGBA(1.0f, 1.0f, 1.0f, 1.0f);
          n++;
        }
        m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, n-2, Verts, STRIDE_D);
        SetCullMode(R_CULL_BACK);
        m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

        //set the color to the color of the light
        Verts[0].xyz = org;
        Verts[0].color.dcolor = D3DRGBA(Col[0], Col[1], Col[2], 1.0f);

        //draw a point at the origin of the light
        m_pd3dDevice->DrawPrimitiveUP(D3DPT_POINTLIST, 1, Verts, STRIDE_D);

        //draw a line in the center of the light
        Verts[0].xyz = org;
        Verts[0].color.dcolor = D3DRGBA(Col[0], Col[1], Col[2], 1.0f);
        Verts[1].xyz = org + dir;
        Verts[1].color.dcolor = D3DRGBA(Col[0], Col[1], Col[2], 1.0f);
        m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, Verts, STRIDE_D);

        EnableTMU(true);
      }
      if (CV_r_debuglights == 2 && !(dl->m_Flags & DLF_DIRECTIONAL))
      {
        SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);
        SetCullMode(R_CULL_NONE);
        SetMaterialColor(dl->m_Color[0], dl->m_Color[1], dl->m_Color[2], 0.25f);
        EF_SetColorOp(eCO_MODULATE);
        DrawBall(dl->m_Origin[0], dl->m_Origin[1], dl->m_Origin[2], dl->m_fRadius);
      }
    }
  }
}

void CD3D8Renderer::EF_DrawDebugTools()
{
  if (CV_r_showlines)
    EF_RenderPipeLine(EF_DrawWire);

  if (CV_r_shownormals)
    EF_RenderPipeLine(EF_DrawNormals);

  if (CV_r_showtangents)
    EF_RenderPipeLine(EF_DrawTangents);

  if (SRendItem::m_RecurseLevel==1 && CV_r_debuglights)
    EF_DrawDebugLights();
}


int TimeProfCallback( const VOID* arg1, const VOID* arg2 )
{
  SProfInfo *pi1 = (SProfInfo *)arg1;
  SProfInfo *pi2 = (SProfInfo *)arg2;
  if (pi1->ef->m_fProfileTime > pi2->ef->m_fProfileTime)
    return -1;
  if (pi1->ef->m_fProfileTime < pi2->ef->m_fProfileTime)
    return 1;
  return 0;
}

int NameCallback( const VOID* arg1, const VOID* arg2 )
{
  SProfInfo *pi1 = (SProfInfo *)arg1;
  SProfInfo *pi2 = (SProfInfo *)arg2;
  if (pi1->ef > pi2->ef)
    return -1;
  if (pi1->ef < pi2->ef)
    return 1;
  return 0;
}

void CD3D8Renderer::EF_PrintProfileInfo()
{
  double fTime = 0;
  { // group by name
    qsort(&m_RP.m_Profile[0], m_RP.m_Profile.Num(), sizeof(SProfInfo), NameCallback );

    for(int i=0; i<m_RP.m_Profile.Num(); i++)
    {
      // if next is the same
      if( i<(m_RP.m_Profile.Num()-1) && m_RP.m_Profile[i].ef == m_RP.m_Profile[i+1].ef )
      {
        m_RP.m_Profile[i].Time += m_RP.m_Profile[i+1].Time;
        m_RP.m_Profile[i].NumPolys += m_RP.m_Profile[i+1].NumPolys;
        m_RP.m_Profile.DelElem(i+1);
        i--;
      }
    }
  }

  TextToScreenColor(8,14, 0,2,0,1, "Shad.: %d, VShad: %d, PShad: %d, RComb: %d, Text: %d, Lit Shaders: %d",  m_RP.m_PS.m_NumRendShaders, m_RP.m_PS.m_NumVShaders, m_RP.m_PS.m_NumPShaders, m_RP.m_PS.m_NumRCombiners, m_RP.m_PS.m_NumTextures, m_RP.m_PS.m_NumLitShaders);
  TextToScreenColor(8,17, 0,2,0,1, "Preprocess: %8.02f ms",  m_RP.m_PS.m_fPreprocessTime);
  TextToScreenColor(8,20, 0,2,0,1, "Skinning:   %8.02f ms",  m_RP.m_PS.m_fSkinningTime);

  int nLine;


  { // sort by interpolated time and print
    qsort(&m_RP.m_Profile[0], m_RP.m_Profile.Num(), sizeof(SProfInfo), TimeProfCallback );

    for(nLine=0; nLine<m_RP.m_Profile.Num(); nLine++)
    {
      fTime += m_RP.m_Profile[nLine].Time;
      if (nLine >= 18)
        continue;
      m_RP.m_Profile[nLine].ef->m_fProfileTime = (m_RP.m_Profile[nLine].ef->m_fProfileTime + (float)m_RP.m_Profile[nLine].Time);
      TextToScreenColor(4,(24+(nLine*3)), 1,0,0,1, "%8.02f ms, %5d polys, '%s'", m_RP.m_Profile[nLine].Time, m_RP.m_Profile[nLine].NumPolys, m_RP.m_Profile[nLine].ef->m_Name.c_str());
    }
  }
  if (nLine > 18)
    nLine = 18;
  TextToScreenColor(8,(26+(nLine*3)), 0,2,0,1, "Total draw time:   %8.02f ms",  fTime);
  TextToScreenColor(8,(29+(nLine*3)), 0,2,0,1, "Total shaders processing time (flush):   %8.02f ms", m_RP.m_PS.m_fFlushTime);
}

void CD3D8Renderer::EF_DrawREPreprocess(SRendItemPreprocess *ris, int Nums)
{
  int i;
  CCObject *savedObj;
  SShader *Shader, *CurShader, *ShaderState, *CurShaderState;
  SRenderShaderResources *Res, *CurRes;
  int nObject, nCurObject;
  int nFog, nCurFog;

  if (Nums < 1)
    return;

  savedObj = m_RP.m_pCurObject;
  m_RP.m_pCurObject = m_RP.m_VisObjects[0];
  m_RP.m_pPrevObject = m_RP.m_pCurObject;

  EF_PreRender();

  EF_PushMatrix();  

  UnINT64 oldVal;
  oldVal.SortVal = -1;
  nCurObject = -2;
  nCurFog = 0;
  CurShader = NULL;
  CurShaderState = NULL;
  CurRes = NULL;

  for (i=0; i<Nums; i++)
  {
    SRendItemPreprocess *ri = &ris[i];
    CRendElement *re = ri->Item;
    if (ri->SortVal.SortVal == oldVal.SortVal)
    {
      // Optimizations: not necessary to check of changing shaders and objects
      // if sort value is the same (consist the same info about shaders, objects, fog volumes, ...)
      re->mfPrepare();
      continue;
    }
    oldVal.SortVal = ri->SortVal.SortVal;

    SRendItem::mfGet(ri->SortVal, &nObject, &Shader, &ShaderState, &nFog, &Res);

    if (nObject != nCurObject)
    {
      if (CurShader)
        EF_Flush();
      if (!EF_ObjectChange(Shader, nObject, re))
        continue;
      nCurObject = nObject;
    }

    if (Shader != CurShader || Res != CurRes || ShaderState != CurShaderState || nFog != nCurFog)
    {
      if (CurShader)
        m_RP.m_pRenderFunc();

      EF_Start(Shader, ShaderState, Res, nFog, re);

      nCurFog = nFog;
      CurShader = Shader;
      CurShaderState = ShaderState;
      CurRes = Res;
    }

    re->mfPrepare();
  }
  if (CurShader)
    m_RP.m_pRenderFunc();
  
  EF_PostRender();

  EF_PopMatrix();

  m_RP.m_pCurObject = savedObj;
}

struct SPreprocess
{
  int m_Num;
  int m_nObject;
  SShader *m_Shader;
  SRenderShaderResources *m_pRes;
  CRendElement *m_RE;
};

STWarpZone *CD3D8Renderer::EF_SetWarpZone(SWarpSurf *sf, int *NumWarps, STWarpZone Warps[])
{
  int i;
  STWarpZone *wp;
  Plane p, pl;

  sf->srf->mfGetPlane(pl);
  if (sf->nobj > 0)
  {
    p = pl;
    CCObject *obj = m_RP.m_VisObjects[sf->nobj];
    obj->CalcMatrix();
    pl = TransformPlane(obj->m_Matrix, p);
  }
  for (i=0; i<NumWarps[0]; i++)
  {
   // if (pl.d == Warps[i].plane.d && pl.n == Warps[i].plane.n)
		if (pl.d == Warps[i].plane.d && IsEquivalent(pl.n,Warps[i].plane.n))
      break;
  }
  if (i == NumWarps[0])
  {
    NumWarps[0]++;
    wp = &Warps[i];
    wp->numSrf = 0;
    wp->plane.d = pl.d;
    wp->plane.n = pl.n;
  }
  EF_UpdateWarpZone(&Warps[i], sf);

  return &Warps[i];
}

void CD3D8Renderer::EF_UpdateWarpZone(STWarpZone *wp, SWarpSurf *srf)
{
  if (wp->numSrf == MAX_WARPSURFS)
    return;
  wp->Surfs[wp->numSrf].nobj = srf->nobj;
  wp->Surfs[wp->numSrf].Shader = srf->Shader;
  wp->Surfs[wp->numSrf].srf = srf->srf;
  wp->numSrf++;
}

bool CD3D8Renderer::EF_CalcWarpCamera(STWarpZone *wp, int nObject, CCamera& prevCam, CCamera& newCam)
{
  Vec3d vPrevPos = prevCam.GetPos();
  Vec3d vPrevAngs = prevCam.GetAngles();
  Vec3d vNewPos, vNewOccPos, vNewAngs;
  bool bMirror = true;
  if (nObject > 0)
  {
    CCObject *pObj = m_RP.m_VisObjects[nObject];
    if (pObj->m_ObjFlags & FOB_PORTAL)
    {
      bMirror = false;
      m_RP.m_PersFlags &= ~RBPF_DRAWMIRROR;
      m_RP.m_PersFlags |= RBPF_DRAWPORTAL;
      vNewPos = pObj->m_Trans2;
      vNewOccPos = vNewPos;
      vNewAngs = pObj->m_Angs2;
    }
  }
  if (bMirror)
  {
    float fDot = vPrevPos.Dot(wp->plane.n) - wp->plane.d;
    vNewPos = vPrevPos + wp->plane.n * -2.0f*fDot;
    vNewOccPos = vPrevPos + wp->plane.n * -0.99f*fDot;

    if (fDot < 0)
      m_RP.m_MirrorClipSide = 1;
    else
      m_RP.m_MirrorClipSide = 2;

    fDot = m_RP.m_CamVecs[0].Dot(wp->plane.n);
    Vec3d vNewDir = m_RP.m_CamVecs[0] + wp->plane.n * -2.0f*fDot;

    vNewAngs[0] = asinf (vNewDir[2])/M_PI*180.0f;
    vNewAngs[1] = vPrevAngs[1] + 180.0f;
    vNewAngs[2] = -atan2f (vNewDir[0], -vNewDir[1])/M_PI*180.0f;

    m_RP.m_PersFlags &= ~RBPF_DRAWPORTAL;
    m_RP.m_PersFlags |= RBPF_DRAWMIRROR;
  }

  newCam.SetPos(vNewPos);
  newCam.SetOccPos(vNewOccPos);
  newCam.SetAngle(vNewAngs);
  if (bMirror)
    newCam.SetScale(Vec3d(-1.0f, 1.0f, 1.0f));
  newCam.Update();  

  return true;
}

bool CD3D8Renderer::EF_RenderWarpZone(STWarpZone *wp)
{
  guard(CD3D8Renderer::EF_RenderWarpZone);

  if (!wp->numSrf)
    return false;

  CCamera prevCam = GetCamera();
  CCamera newCam = prevCam;

  int prevFlags = m_RP.m_PersFlags;

  if (!EF_CalcWarpCamera(wp, wp->Surfs[0].nobj, prevCam, newCam))
    return false;

  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "\n    ********** Start Render Portal **********\n");

  STWarpZone *prevWarp = m_RP.m_CurWarp;
  m_RP.m_CurWarp = wp;

  m_matProj->Push();
  EF_PushMatrix();

  iSystem->SetViewCamera(newCam);
  SetCamera(newCam);

  float plane[4];
  plane[0] = wp->plane.n[0];
  plane[1] = wp->plane.n[1];
  plane[2] = wp->plane.n[2];
  plane[3] = -wp->plane.d;

  EF_PushFog();
  EF_SetClipPlane(true, plane, false);

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  eng->DrawLowDetail(DLD_TERRAIN_WATER | DLD_DETAIL_TEXTURES | DLD_DETAIL_OBJECTS | DLD_STATIC_OBJECTS | DLD_FAR_SPRITES | DLD_ENTITIES);

  /*unsigned char *pic=new unsigned char [m_width*m_height*4];
  glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, pic);
  WriteTGA(pic, m_width, m_height,"Warp.tga");
  delete [] pic;*/

  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "\n    ********** End Render Portal **********\n");

  m_RP.m_PersFlags &= ~(RBPF_DRAWMIRROR | RBPF_DRAWPORTAL);
  m_RP.m_PersFlags |= prevFlags & (RBPF_DRAWMIRROR | RBPF_DRAWPORTAL);

  EF_SetClipPlane(false, plane, false);
  EF_PopFog();

  m_RP.m_CurWarp = prevWarp;

  iSystem->SetViewCamera(prevCam);
  SetCamera(prevCam);

  EF_PopMatrix();
  m_matProj->Pop();

  unguard

  return true;
}

void sTransformPlane( float *u, const float *v, const float *m )
{
  float v0=v[0], v1=v[1], v2=v[2], v3=v[3];
#define M(row,col)  m[col*4+row]
  u[0] = v0 * M(0,0) + v1 * M(1,0) + v2 * M(2,0) + v3 * M(3,0);
  u[1] = v0 * M(0,1) + v1 * M(1,1) + v2 * M(2,1) + v3 * M(3,1);
  u[2] = v0 * M(0,2) + v1 * M(1,2) + v2 * M(2,2) + v3 * M(3,2);
  u[3] = v0 * M(0,3) + v1 * M(1,3) + v2 * M(2,3) + v3 * M(3,3);
#undef M
}

static _inline int Compare(SPreprocess &a, SPreprocess &b)
{
  SShader *sh1 = a.m_Shader;
  SShader *sh2 = b.m_Shader;

  if ((sh1->m_Flags2 & EF2_PREPR_CORONA) < (sh2->m_Flags2 & EF2_PREPR_CORONA))
    return 1;
  if ((sh1->m_Flags2 & EF2_PREPR_CORONA) > (sh2->m_Flags2 & EF2_PREPR_CORONA))
    return -1;
  return 0;
}

int CD3D8Renderer::EF_Preprocess(SRendItemPre *ri, int nums, int nume)
{
  int i, m;
  SShader *Shader;
  SShader *ShaderState;
  SRenderShaderResources *Res;
  int nObject;
  int nFog;

  TArray<SPreprocess> Procs;
  SPreprocess Proc;
  TArray<SRendItemPreprocess> RIs;
  SWarpSurf srfs[1024];
  int nPortals = 0;

  int NumWarps = 0;
  STWarpZone Warps[MAX_WARPS];
  STWarpZone *wp;

  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "*** Start preprocess frame ***\n");

  for (i=nums; i<nume; i++)
  {
    SRendItem::mfGet(ri[i].SortVal, &nObject, &Shader, &ShaderState, &nFog, &Res);
    if ((ri[i].SortVal.i.High >> 26) != eS_PreProcess)
      break;
    Proc.m_Num = i;
    Proc.m_Shader = Shader;
    Proc.m_pRes = Res;
    Proc.m_RE = ri[i].Item;
    Proc.m_nObject = nObject;
    Procs.AddElem(Proc);
  }
  if (!Procs.Num())
    return 0;
  m = Procs.Num();
  
  for (i=0; i<m; i++)
  {
    SPreprocess *pr = &Procs[i];
    if (!pr->m_Shader)
      continue;
    // Draw environment to cube texture
    if ((pr->m_Shader->m_Flags2 & EF2_PREPR_SCANCM) && !m_RP.m_bDrawToTexture)
    {
      CCObject *objIgn = m_RP.m_pIgnoreObject;
      SShader *shadIgn = m_RP.m_pIgnoreShader;
      Vec3d Pos;
      m_RP.m_pRE = pr->m_RE;
      if (pr->m_nObject < 0)
        Pos.Set(0, 0, 0);
      else
      {
        CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
        m_RP.m_pIgnoreObject = obj;
        m_RP.m_pIgnoreShader = pr->m_Shader;
        if (obj->m_ObjFlags & FOB_USEMATRIX)
          Pos = Vec3d(obj->m_Matrix(3,0), obj->m_Matrix(3,1), obj->m_Matrix(3,2));
        else
          Pos = obj->m_Trans;
      }
      if (m_LogFile)
        Logv(SRendItem::m_RecurseLevel, "*** Draw environment to cube-map ***\n");
   //   if (Pos != Vec3d(0, 0, 0))
			if (!IsEquivalent(Pos,Vec3d(0,0,0)))
        SEnvTexture *cm = gcEf.mfFindSuitableEnvCMap(Pos, false, pr->m_Shader->m_DLDFlags);
      m_RP.m_pIgnoreObject = objIgn;
      m_RP.m_pIgnoreShader = shadIgn;
    }
    else
    if ((pr->m_Shader->m_Flags2 & EF2_PREPR_SCANSCR) && !m_RP.m_bDrawToTexture)
    { // Draw screen to texture
    }
    else
    if ((pr->m_Shader->m_Flags2 & EF2_PREPR_SCANTEX) && !m_RP.m_bDrawToTexture)
    { // Draw environment to texture
      bool bWater = (pr->m_Shader->m_Flags2 & EF2_PREPR_SCANTEXWATER) ? 1 : 0;
      bool bDraw = true;
      if (!bWater || CV_r_waterrefractions)
      {
        if (pr->m_Shader->m_fUpdateFactor > 0)
        {
          m_RP.m_VisObjects[pr->m_nObject]->CalcMatrix();
          float fDist = pr->m_RE->mfMinDistanceToCamera();
          if (fDist > 0)
          {
            fDist *= pr->m_Shader->m_fUpdateFactor;
            if (fDist > 0.5f)
              fDist = 0.5f;
            if (m_RP.m_RealTime-m_RP.m_pCurObject->m_fLastUpdate < fDist)
              bDraw = false;
            else
              m_RP.m_pCurObject->m_fLastUpdate = m_RP.m_RealTime;
          }
        }
        if (bDraw)
        {
          CCObject *objIgn = m_RP.m_pIgnoreObject;      
          SShader *shadIgn = m_RP.m_pIgnoreShader;
          if (!CV_r_selfrefract)
          {
            m_RP.m_pIgnoreObject = m_RP.m_VisObjects[pr->m_nObject];
            m_RP.m_pIgnoreShader = pr->m_Shader;
          }
          Vec3d Angs = GetCamera().GetAngles();
          Vec3d Pos = GetCamera().GetPos();
          m_RP.m_pRE = pr->m_RE;
          if (m_LogFile)
            Logv(SRendItem::m_RecurseLevel, "*** Draw environment to texture ***\n");
          
          int DLDFlags = 0;
          if (bWater)
          {
            m_RP.m_PersFlags |= RBPF_DONTDRAWSUN;
            DLDFlags |= DLD_TERRAIN_WATER | DLD_DETAIL_TEXTURES;
          }
          m_RP.m_pShader = pr->m_Shader;
          m_RP.m_pCurObject = m_RP.m_pIgnoreObject;
          DLDFlags |= pr->m_Shader->m_DLDFlags;
          SEnvTexture *cm = gcEf.mfFindSuitableEnvTex(Pos, Angs, false, DLDFlags);
          m_RP.m_pIgnoreObject = objIgn;
          m_RP.m_pIgnoreShader = shadIgn;
        }
      }
    }
    if ((pr->m_Shader->m_Flags2 & EF2_PREPR_SCANTEXWATER) && !m_RP.m_bDrawToTexture)
    {
      if (CV_r_waterreflections)
      {
        if (!(CTexMan::m_Text_WaterMap->m_Flags & FT_BUILD))
        {
          I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
          float fDist = eng->GetDistanceToSectorWithWater();
          fDist *= CV_r_waterupdateFactor;
          if (fDist > 0.3f)
            fDist = 0.3f;
          Vec3d Angs = GetCamera().GetAngles();
          if (m_RP.m_RealTime-m_RP.m_fLastWaterUpdate > fDist || fabs(Angs[2]-m_RP.m_fLastWaterAngleUpdate)>CV_r_waterupdateDeltaAngle)
          {
            m_RP.m_fLastWaterUpdate = m_RP.m_RealTime;
            m_RP.m_fLastWaterAngleUpdate = Angs[2];

            CCamera tmp_cam = GetCamera();
            Plane Pl;
            Pl.n = Vec3d(0,0,1);
            Pl.d = eng->GetWaterLevel();
            CCObject *savedObj = m_RP.m_pCurObject;
            CCObject *objIgn = m_RP.m_pIgnoreObject;
            CCObject *obj = NULL;
            SShader *shIgn = m_RP.m_pIgnoreShader;
            if (pr->m_nObject > 0)
            {
              obj = m_RP.m_VisObjects[pr->m_nObject];
              m_RP.m_VisObjects[pr->m_nObject]->CalcMatrix();
            }
            m_RP.m_PersFlags |= RBPF_DONTDRAWSUN;
            m_RP.m_pCurObject = obj;
            m_RP.m_pIgnoreObject = obj;
            m_RP.m_pIgnoreShader = pr->m_Shader;
            if (m_LogFile)
              Logv(SRendItem::m_RecurseLevel, "*** Draw environment to texture for water reflections ***\n");
            m_TexMan->DrawToTexture(Pl, CTexMan::m_Text_WaterMap, pr->m_Shader->m_DLDFlags);
            m_RP.m_pCurObject = savedObj;
            m_RP.m_pIgnoreObject = objIgn;
            m_RP.m_pIgnoreShader = shIgn;
            m_RP.m_PersFlags &= ~RBPF_DONTDRAWSUN;
          }
        }
      }
    }
    else
    if ((pr->m_Shader->m_Flags2 & EF2_PREPR_SHADOWMAPGEN))
    {
      CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
      m_RP.m_pCurObject = obj;
/*      if (pr->m_RE->mfGetType() == eDATA_ShadowMap)
      {
        CREShadowMap * pRE = (CREShadowMap*)pr->m_RE;

        // Please never comment this lines !!!
        if (m_LogFile)
          Logv(SRendItem::m_RecurseLevel, "*** Prepare shadow maps for REShadowMap***\n");
        if(obj && pRE->m_pShadowFrustum && ((ShadowMapLightSource*)pRE->m_pShadowFrustum)->m_LightFrustums.Count() && !m_RP.m_bDrawToTexture)
          PrepareDepthMap(&((ShadowMapLightSource*)pRE->m_pShadowFrustum)->m_LightFrustums[0], false);
      }
      else*/
      if (pr->m_RE->mfGetType() == eDATA_OcLeaf)
      {
        CREOcLeaf * pRE = (CREOcLeaf*)pr->m_RE;
        ShadowMapFrustum * pFr = obj->m_pShadowFrustum;

        // Please never comment this lines !!!
        if (m_LogFile)
          Logv(SRendItem::m_RecurseLevel, "*** Prepare shadow maps for REOcLeaf***\n");
        if(obj && pFr && !m_RP.m_bDrawToTexture)
          PrepareDepthMap(pFr, false);
      }
    }
    else
    if ((pr->m_Shader->m_Flags2 & EF2_PREPR_CORONA) && pr->m_nObject>0)
    {
      if (pr->m_RE->mfGetType() == eDATA_Flare)
      {
        CREFlare *fl = (CREFlare *)pr->m_RE;
        CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
        if (m_LogFile)
          Logv(SRendItem::m_RecurseLevel, "*** Check corona visibility ***\n");
        bool bVis = fl->mfCheckVis(obj);
      }
    }
    else
    if (pr->m_Shader->m_Flags2 & EF2_PREPR_OUTSPACE)
    {
      CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
      m_RP.m_pCurObject = obj;
      CREOutSpace * pRE = (CREOutSpace*)pr->m_RE;

      if(obj && pRE && !m_RP.m_bDrawToTexture)
        PrepareOutSpaceTextures(pRE);
    }
    else
    if (pr->m_Shader->m_Flags2 & EF2_PREPR_PORTAL)
    {
      if (!CV_r_portals)
        continue;

      if (m_RP.m_RecurseLevel+1 >= MAX_PORTAL_RECURSES)
        continue;

      if (!CV_r_portalsrecursive && m_RP.m_CurWarp)
        continue;

      if (!m_RP.m_CurWarp)
      {
        srfs[nPortals].nobj = pr->m_nObject;
        srfs[nPortals].srf = pr->m_RE;
        srfs[nPortals].Shader = pr->m_Shader;
        nPortals++;
      }
      else
      {
        Plane pl;
        pr->m_RE->mfGetPlane(pl);

			//	if (m_RP.m_CurWarp->plane.d != pl.d || m_RP.m_CurWarp->plane.n != pl.n)
				if (m_RP.m_CurWarp->plane.d != pl.d || !IsEquivalent(m_RP.m_CurWarp->plane.n,pl.n))
        {
          srfs[nPortals].nobj = pr->m_nObject;
          srfs[nPortals].srf = pr->m_RE;
          srfs[nPortals].Shader = pr->m_Shader;
          nPortals++;
        }
      }
    }
    else
    {
      if (pr->m_nObject < 0)
        continue;
      SRendItemPreprocess rip;
      SRendItemPre *r = &ri[pr->m_Num];
      rip.Item = r->Item;
      rip.m_Object = m_RP.m_VisObjects[pr->m_nObject];
      if (rip.m_Object->m_ObjFlags & FOB_CUBE_MASK)
        RIs.AddElem(rip);
    }
  }
  if (RIs.Num())
  {
    if (m_LogFile)
      Logv(SRendItem::m_RecurseLevel, "*** Preprocess pipeline ***\n");
    SRendItemPreprocess::mfSort(&RIs[0], RIs.Num());
    EF_DrawREPreprocess(&RIs[0], RIs.Num());
  }

  if (nPortals)
  {
    for (i=0; i<nPortals; i++)
    {
      EF_SetWarpZone(&srfs[i], &NumWarps, Warps);
    }

    m_RP.m_RecurseLevel++;
    m_RP.m_WasPortals += NumWarps;
    m_RP.m_CurPortal += NumWarps;

    for (i=0; i<NumWarps; i++)
    {
      wp = &Warps[i];

      if (m_LogFile)
        Logv(SRendItem::m_RecurseLevel, "*** Start rendering of warp zone %d ***\n", i);
      EF_RenderWarpZone(wp);
      m_RP.m_CurPortal--;

      EF_PreRender();

      // Render translucent mirror surface(s)
      int nObj = -1;
      SShader *sh = NULL;
      SRenderShaderResources *Res = NULL;
      for (int j=0; j<wp->numSrf; j++)
      {
        SWarpSurf *ws = &wp->Surfs[j];
        if (nObj != ws->nobj)
        {
          if (sh)
            EF_Flush();

          if (!EF_ObjectChange(sh, ws->nobj, ws->srf))
            continue;
          nObj = ws->nobj;
        }
        if (ws->Shader != sh || ws->ShaderRes != Res)
        {
          if (sh)
            m_RP.m_pRenderFunc();
          EF_Start(ws->Shader, NULL, ws->ShaderRes, 0, ws->srf);
          sh = ws->Shader;
        }

        ws->srf->mfPrepare();
      }
      if (sh)
        m_RP.m_pRenderFunc();
    }
    m_RP.m_RecurseLevel--;
  }

  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "*** End preprocess frame ***\n");

  return m;
}

void CD3D8Renderer::EF_EndEf2D(bool bSort)
{
  int i;
  SShader *Shader, *CurShader, *ShaderState, *CurShaderState;
  SRenderShaderResources *Res, *CurRes;

  EF_AddClientPolys2D();

  int nums = SRendItem::m_StartRI[0];
  int nume = SRendItem::m_RendItems[0].Num();

  if (bSort)
  {
    SRendItem::mfSort(&SRendItem::m_RendItems[0][0], nume);
    // If sort number of the first shader is 1 (eS_Preprocess)
    // run preprocess operations for the current frame
    if ((SRendItem::m_RendItems[0][nums].SortVal.i.High >> 26) == eS_PreProcess)
      nums += EF_Preprocess(&SRendItem::m_RendItems[nums][0], nums, nume);
  }

  EF_PreRender();
  ResetToDefault();

  CurShader = NULL;
  CurRes = NULL;
  CurShaderState = NULL;
  UnINT64 oldVal;
  oldVal.SortVal = -1;
  m_RP.m_Flags = RBF_2D;

  for (i=nums; i<nume; i++)
  {
    SRendItemPre *ri = &SRendItem::m_RendItems[0][i];
    CRendElement *re = ri->Item;
    if (ri->SortVal.SortVal == oldVal.SortVal)
    {
      re->mfPrepare();
      continue;
    }
    oldVal.SortVal = ri->SortVal.SortVal;

    SRendItem::mfGet(ri->SortVal, &Shader, &ShaderState, &Res);
    if (Shader != CurShader || Res != CurRes || ShaderState != CurShaderState)
    {
      if (CurShader)
        m_RP.m_pRenderFunc();

      EF_Start(Shader, ShaderState, Res, re);

      CurShader = Shader;
      CurShaderState = ShaderState;
      CurRes = Res;
    }

    re->mfPrepare();
  }
  if (CurShader)
    m_RP.m_pRenderFunc();

  EF_PostRender();
}
