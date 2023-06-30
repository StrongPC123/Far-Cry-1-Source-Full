/*=============================================================================
  D3DRendPipeline.cpp : Direct3D specific rendering using shaders pipeline.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

    Revision history:
      * Created by Honitch Andrey
    
=============================================================================*/

#include "RenderPCH.h"
#include "DriverD3D9.h"
#include <ICryAnimation.h>
#include "..\common\shadow_renderer.h"
#include "D3DCGPShader.h"
#include "D3DCGVProgram.h"
#include "I3dengine.h"
#include "CryHeaders.h"

//============================================================================================
// Shaders rendering
//============================================================================================

//============================================================================================
// Init Shaders rendering

void CD3D9Renderer::EF_InitRandTables()
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

void CD3D9Renderer::EF_InitWaveTables()
{
  int i;
  
  //Init wave Tables
  for (i=0; i<1024; i++)
  {
    float f = (float)i;
    
    m_RP.m_tSinTable[i] = sin_tpl(f * (360.0f/1023.0f) * M_PI / 180.0f);
    m_RP.m_tHalfSinTable[i] = sin_tpl(f * (360.0f/1023.0f) * M_PI / 180.0f);
    if (m_RP.m_tHalfSinTable[i] < 0)
      m_RP.m_tHalfSinTable[i] = 0;
    m_RP.m_tCosTable[i] = cos_tpl(f * (360.0f/1023.0f) * M_PI / 180.0f);
    m_RP.m_tHillTable[i] = sin_tpl(f * (180.0f/1023.0f) * M_PI / 180.0f);
    
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

void CD3D9Renderer::EF_InitEvalFuncs(int num)
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

int CD3D9Renderer::EF_RegisterFogVolume(float fMaxFogDist, float fFogLayerZ, CFColor color, int nIndex, bool bCaustics)
{
  if (nIndex < 0)
  {
    SMFog Fog;
    memset(&Fog,0,sizeof(Fog));

    Fog.m_fMaxDist = fMaxFogDist;
    Fog.m_FogInfo.m_FogColor = color;
    Fog.m_Dist = fFogLayerZ;
    Fog.m_Color = color;
    Fog.m_Color.a = 1.0f;
    Fog.bCaustics = bCaustics;
    //Fog.m_FogInfo.m_FogColor = m_FogColor;
    Fog.m_Normal = Vec3d(0,0,1);

    m_RP.m_FogVolumes.AddElem(Fog);
    return m_RP.m_FogVolumes.Num()-1;
  }
  else
  {
    assert (nIndex < m_RP.m_FogVolumes.Num());
    SMFog *pFog = &m_RP.m_FogVolumes[nIndex];
    pFog->m_fMaxDist = fMaxFogDist;
    pFog->m_FogInfo.m_FogColor = color;
    pFog->m_Dist = fFogLayerZ;
    pFog->m_Color = color;
    pFog->m_Color.a = 1.0f;
    pFog->bCaustics = bCaustics;
    return nIndex;
  }
}



// Init vertex declarations (for fixed pipeline and for programmable pipeline)
void CD3D9Renderer::EF_InitD3DFixedPipeline()
{
  if (m_RP.m_D3DFixedPipeline[0][0].m_Declaration.Num())
    return;

  int i, j;
  SBufInfoTable *pOffs;

  // base formats declarations (stream 0)
  int n = 0;
  pOffs = &gBufInfoTable[n];
  {
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = 0;

    D3DVERTEXELEMENT9 elem[] =
    {
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 1;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_P3F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 2;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_P3F_COL4UB
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_DIFFUSE;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      {0, pOffs->OffsColor, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0}, // diffuse
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 3;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_P3F_TEX2F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_TEX1;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      {0, pOffs->OffsTC, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, // texture0
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 4;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_P3F_COL4UB_TEX2F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      {0, pOffs->OffsColor, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0}, // diffuse
      {0, pOffs->OffsTC, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, // texture0
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 5;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_TRP3F_COL4UB_TEX2F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0},  // position
      {0, pOffs->OffsColor, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0}, // diffuse
      {0, pOffs->OffsTC, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, // texture0
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 6;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_P3F_COL4UB_COL4UB
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      {0, pOffs->OffsColor, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0}, // normal
      {0, pOffs->OffsSecColor, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1}, // specular
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 7;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_P3F_N
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_NORMAL;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      {0, pOffs->OffsNormal, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},  // normal
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 8;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_P3F_N_COL4UB
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      {0, pOffs->OffsNormal, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},  // normal
      {0, pOffs->OffsColor, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0}, // diffuse
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 9;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_P3F_N_TEX2F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      {0, pOffs->OffsNormal, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},  // normal
      {0, pOffs->OffsTC, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, // texture0
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 10;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_P3F_N_COL4UB_TEX2F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      {0, pOffs->OffsNormal, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},  // normal
      {0, pOffs->OffsColor, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0}, // diffuse
      {0, pOffs->OffsTC, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, // texture0
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 11;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_P3F_N_COL4UB_COL4UB
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      {0, pOffs->OffsNormal, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},  // normal
      {0, pOffs->OffsColor, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0}, // diffuse
      {0, pOffs->OffsSecColor, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1}, // specular
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 12;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      {0, pOffs->OffsColor, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0}, // diffuse
      {0, pOffs->OffsSecColor, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1}, // specular
      {0, pOffs->OffsTC, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, // texture0
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 13;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_P3F_N_COL4UB_COL4UB_TEX2F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      {0, pOffs->OffsNormal, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0}, // normal
      {0, pOffs->OffsColor, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0}, // diffuse
      {0, pOffs->OffsSecColor, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1}, // specular
      {0, pOffs->OffsTC, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, // texture0
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 14;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_T3F_B3F_N3F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_TEX3;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},  // tangent
      {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},  // binormal
      {0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},    // tnormal
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 15;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_T2F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_TEX1;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0,  D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},  // tangent
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }
  n = 16;
  pOffs = &gBufInfoTable[n];
  { // VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F
    m_RP.m_D3DFixedPipeline[0][n].m_Handle = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2;

    D3DVERTEXELEMENT9 elem[] =
    {
      {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},  // position
      {0, pOffs->OffsColor, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0}, // diffuse
      {0, pOffs->OffsTC, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, // texture0
      {0, pOffs->OffsTC+8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1}, // texture1
      D3DDECL_END()
    };
    i = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Add(elem[i]);  // end of the declaration
      if (elem[i++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[0][n].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[0][n].m_pDeclaration = NULL;
  }

  // Additional streams:
  // stream 1 (tangent vectors)
  for (i=0; i<VERTEX_FORMAT_NUMS; i++)
  {
    if (!m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num())
      continue;

    for (j=0; j<m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num()-1; j++)
    {
      m_RP.m_D3DFixedPipeline[1][i].m_Declaration.AddElem(m_RP.m_D3DFixedPipeline[0][i].m_Declaration[j]);
    }
    D3DVERTEXELEMENT9 elem[] =
    {
      {1, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},  // tangent
      {1, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},  // binormal
      {1, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},    // tnormal
      D3DDECL_END()
    };
    n = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[1][i].m_Declaration.Add(elem[n]); 
      if (elem[n++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[1][i].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[1][i].m_pDeclaration = NULL;
  }

  // stream 2 (LM texcoords)
  for (int i=0; i<VERTEX_FORMAT_NUMS; i++)
  {
    if (!m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num())
      continue;

    for (int j=0; j<m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num()-1; j++)
    {
      m_RP.m_D3DFixedPipeline[2][i].m_Declaration.AddElem(m_RP.m_D3DFixedPipeline[0][i].m_Declaration[j]);
    }
    D3DVERTEXELEMENT9 elem[] =
    {
      {2, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},  // LM texcoord
      D3DDECL_END()
    };
    n = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[2][i].m_Declaration.Add(elem[n]); 
      if (elem[n++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[2][i].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[2][i].m_pDeclaration = NULL;
  }

  // stream 1 and 2 (tangent vectors and LM texcoords)
  for (int i=0; i<VERTEX_FORMAT_NUMS; i++)
  {
    if (!m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num())
      continue;

    for (int j=0; j<m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num()-1; j++)
    {
      m_RP.m_D3DFixedPipeline[3][i].m_Declaration.AddElem(m_RP.m_D3DFixedPipeline[0][i].m_Declaration[j]);
    }
    D3DVERTEXELEMENT9 elem[] =
    {
      {1, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},  // tangent
      {1, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},  // binormal
      {1, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},    // tnormal
      {2, 0,  D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},  // LM texcoord
      D3DDECL_END()
    };
    n = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[3][i].m_Declaration.Add(elem[n]); 
      if (elem[n++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[3][i].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[3][i].m_pDeclaration = NULL;
  }
  // stream 4 (BendInfo)
  for (int i=0; i<VERTEX_FORMAT_NUMS; i++)
  {
    if (!m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num())
      continue;

    for (int j=0; j<m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num()-1; j++)
    {
      m_RP.m_D3DFixedPipeline[4][i].m_Declaration.AddElem(m_RP.m_D3DFixedPipeline[0][i].m_Declaration[j]);
    }
    D3DVERTEXELEMENT9 elem[] =
    {
      {2, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},  
      {2, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1},  
      D3DDECL_END()
    };
    n = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[4][i].m_Declaration.Add(elem[n]); 
      if (elem[n++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[4][i].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[4][i].m_pDeclaration = NULL;
  }
  // stream 1 and 4 (BendInfo)
  for (int i=0; i<VERTEX_FORMAT_NUMS; i++)
  {
    if (!m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num())
      continue;

    for (int j=0; j<m_RP.m_D3DFixedPipeline[0][i].m_Declaration.Num()-1; j++)
    {
      m_RP.m_D3DFixedPipeline[5][i].m_Declaration.AddElem(m_RP.m_D3DFixedPipeline[0][i].m_Declaration[j]);
    }
    D3DVERTEXELEMENT9 elem[] =
    {
      {1, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},  // tangent
      {1, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},  // binormal
      {1, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},    // tnormal
      {2, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},  
      {2, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1},  
      D3DDECL_END()
    };
    n = 0;
    while (true)
    {
      m_RP.m_D3DFixedPipeline[5][i].m_Declaration.Add(elem[n]); 
      if (elem[n++].Stream == 0xff) break;
    }
    m_RP.m_D3DFixedPipeline[5][i].m_Declaration.Shrink();
    m_RP.m_D3DFixedPipeline[5][i].m_pDeclaration = NULL;
  }

  m_MaxVertBufferSize = CLAMP((int)(CV_d3d9_pip_buff_size), 5, 100)*1024*1024;
  m_CurVertBufferSize = 0;
}

_inline static void *sAlign0x20(byte *vrts)
{
  INT_PTR b = (INT_PTR)vrts;

  if (!(b & 0x1f))
    return vrts;

  b = (b+0x20)&~0x1f;

  return (void *)b;
}

// Init shaders pipeline
void CD3D9Renderer::EF_Init()
{
  bool nv = 0;
  int i;

  if (CV_r_logTexStreaming && !m_LogFileStr)
  {
    m_LogFileStr = fxopen ("Direct3DLogStreaming.txt", "w");
    if (m_LogFileStr)
    {      
      iLog->Log("Direct3D texture streaming log file '%s' opened\n", "Direct3DLogStreaming.txt");
      char time[128];
      char date[128];

      _strtime( time );
      _strdate( date );

      fprintf(m_LogFileStr, "\n==========================================\n");
      fprintf(m_LogFileStr, "Direct3D Textures streaming Log file opened: %s (%s)\n", date, time);
      fprintf(m_LogFileStr, "==========================================\n");
    }
  }

  m_RP.m_pCurFuncs = NULL;
  m_RP.m_MaxVerts = CV_d3d9_rb_verts;
  m_RP.m_MaxTris = CV_d3d9_rb_tris;

  iLog->Log("\nAllocate render buffer (%d verts, %d tris)...\n", m_RP.m_MaxVerts, m_RP.m_MaxTris);

  int n = 0;

  n += sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F) * m_RP.m_MaxVerts + 32;
  
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

  if (GetFeatures() & RFT_BUMP)
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
  buf += sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F) * m_RP.m_MaxVerts + 32;
  m_RP.m_RendIndices = (ushort *)sAlign0x20(buf);
  m_RP.m_SysRendIndices = m_RP.m_RendIndices;
  buf += sizeof(ushort)*3*m_RP.m_MaxTris+32;
  m_RP.m_pClientColors = (bvec4 *)sAlign0x20(buf);
  buf += sizeof(bvec4)*m_RP.m_MaxVerts+32;
  m_RP.m_pBaseTexCoordPointer = (SMRendTexVert *)sAlign0x20(buf);
  buf += sizeof(SMRendTexVert)*m_RP.m_MaxVerts+32;
  m_RP.m_pLMTexCoordPointer = (SMRendTexVert *)sAlign0x20(buf);
  buf += sizeof(SMRendTexVert)*m_RP.m_MaxVerts+32;
  m_RP.m_pFogVertValues = (float *)sAlign0x20(buf);
  buf += sizeof(float)*m_RP.m_MaxVerts+32;
  if (GetFeatures() & RFT_BUMP)
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

  EF_Restore();

  EF_InitWaveTables();
  EF_InitRandTables();
  EF_InitEvalFuncs(0);
  EF_InitFogVolumes();
  EF_InitD3DFixedPipeline();

  for (i=0; i<VERTEX_FORMAT_NUMS; i++)
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
  CCObject::m_ObjMatrices.reinit(32);
  CCObject::m_ObjMatrices[0].SetIdentity();

  if (!m_RP.m_TempObjects.Num())
    m_RP.m_TempObjects.Reserve(MAX_REND_OBJECTS);
  if (!m_RP.m_Objects.Num())
  {
    m_RP.m_Objects.Reserve(MAX_REND_OBJECTS);
    m_RP.m_Objects.SetUse(1);
    SAFE_DELETE_ARRAY(m_RP.m_ObjectsPool);
    m_RP.m_nNumObjectsInPool = 512;
    m_RP.m_ObjectsPool = new CCObject[m_RP.m_nNumObjectsInPool];
    for (int i=0; i<m_RP.m_nNumObjectsInPool; i++)
    {
      m_RP.m_TempObjects[i] = &m_RP.m_ObjectsPool[i];
      m_RP.m_TempObjects[i]->Init();
      m_RP.m_TempObjects[i]->m_Color = Col_White;
      m_RP.m_TempObjects[i]->m_ObjFlags = 0;
      m_RP.m_TempObjects[i]->m_Matrix.SetIdentity();
      m_RP.m_TempObjects[i]->m_RenderState = 0;
    }
    m_RP.m_VisObjects[0] = &m_RP.m_ObjectsPool[0];
  }

  for (i=0; i<eSHP_MAX; i++)
  {
    if (i == eSHP_DiffuseLight || i == eSHP_SpecularLight)
      m_SHPTable[i] = eSHP_Light;
    else
      m_SHPTable[i] = (EShaderPassType)i;
  }
  m_RP.m_Name_SpecularExp = CName("specularexp", eFN_Add);

  // create glare element
  m_RP.m_pREGlare = (CREGlare *)EF_CreateRE(eDATA_Glare);
  m_RP.m_pREHDR = (CREHDRProcess *)EF_CreateRE(eDATA_HDRProcess);

  //MakeShadowTextures();
}

// Invalidate shaders pipeline
void CD3D9Renderer::EF_Invalidate()
{
  int j;
  for (j=0; j<MAX_DYNVBS; j++)
  {
    SAFE_DELETE (m_RP.m_VBs[j].VBPtr_0);
  }
  SAFE_DELETE(m_RP.m_IndexBuf);
  SAFE_DELETE(m_RP.m_VB_Inst);
}

// Restore shaders pipeline
void CD3D9Renderer::EF_Restore()
{
  int j;

  if (!m_RP.m_MaxTris)
    return;

  EF_Invalidate();

  m_RP.m_IndexBuf = new DynamicIB<ushort>(m_pd3dDevice, m_RP.m_MaxTris*3);
  m_RP.m_VB_Inst = new DynamicVB <vec4_t>(m_pd3dDevice, 0, MAX_HWINST_PARAMS);

  for (j=0; j<MAX_DYNVBS; j++)
  {
    m_RP.m_VBs[j].VBPtr_10 = new DynamicVB <struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F>(m_pd3dDevice, 0, m_RP.m_MaxVerts*8);
  }
}

// Shutdown shaders pipeline
void CD3D9Renderer::EF_PipelineShutdown()
{
  int i, j;

  EF_Invalidate();

  CCObject::m_Waves.Free();
  SAFE_DELETE_ARRAY(m_RP.m_VisObjects);
  SAFE_DELETE_ARRAY(m_SysArray);  

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

// Release all vertex and pixel shaders
void CD3D9Renderer::EF_Release(int nFlags)
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

void DrawFullScreenQuad(float fLeftU, float fTopV, float fRightU, float fBottomV);

// Clear buffers (color, depth/stencil)
void CD3D9Renderer::EF_ClearBuffers(bool bForce, bool bOnlyDepth, float *Colors)
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
  
  int nFlags = D3DCLEAR_ZBUFFER;
  if (m_sbpp)
    nFlags |= D3DCLEAR_STENCIL;
  if (!bOnlyDepth)
  {
    // Fill float buffer by HDR fog color
    if (m_RP.m_PersFlags & RBPF_HDR)
    {
      CCGPShader_D3D *fpHDR = NULL;
      //if (m_nHDRType == 1)
        fpHDR = (CCGPShader_D3D *)PShaderForName(m_RP.m_PS_HDR_ClearScreen, "CGRC_HDR_ClearScreen_PS20");
      //else
      //if (m_nHDRType == 2)
      //  fpHDR = (CCGPShader_D3D *)PShaderForName(m_RP.m_PS_HDR_ClearScreen, "CGRC_HDR_ClearScreen_MRT_PS20");
      if (fpHDR)
      {
        EF_SetState(GS_NODEPTHTEST);
        D3DSetCull(eCULL_None);
        m_TexMan->m_Text_White->Set();
        fpHDR->mfSet(true);
        // Draw a fullscreen quad to sample the RT
        DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);
        fpHDR->mfSet(false);
      }
    }
    else
      nFlags |= D3DCLEAR_TARGET;
  }
  m_pd3dDevice->Clear(0, NULL, nFlags, cColor, 1.0f, 0);
  
  if (CV_r_measureoverdraw)
  {
    if (m_sbpp <= 4)
    {
      iLog->Log("Warning: Not enough stencil bits to measure overdraw: %d\n");
      CV_r_measureoverdraw = 0;
      m_RP.m_PersFlags &= ~RBPF_MEASUREOVERDRAW;
    }
    else
    {
      m_RP.m_PersFlags &= ~RBPF_MEASUREOVERDRAW;
      EF_SetState(GS_STENCIL | GS_DEPTHWRITE);
      m_RP.m_PersFlags |= RBPF_MEASUREOVERDRAW;
      EF_SetStencilState(STENC_FUNC(FSS_STENCFUNC_ALWAYS) |
                         STENCOP_FAIL(FSS_STENCOP_KEEP) |
                         STENCOP_ZFAIL(FSS_STENCOP_INCR) |
                         STENCOP_PASS(FSS_STENCOP_INCR),
                         0, 0xffffffff);
    }
  }
  else
    m_RP.m_PersFlags &= ~RBPF_MEASUREOVERDRAW;
}

//==========================================================================
// Calculate current scene node matrices
void CD3D9Renderer::EF_SetCameraInfo()
{
  m_RP.m_ViewOrg = m_cam.GetPos();

  float fm[16];
  GetModelViewMatrix(fm);
  m_ViewMatrix = (Matrix44&)fm;
  m_CameraMatrix = m_ViewMatrix;

  Matrix44 m;
  mathMatrixTranspose(m.GetData(), m_CameraMatrix.GetData(), g_CpuFlags);

  m_RP.m_CamVecs[0][0] = -m(2,0);
  m_RP.m_CamVecs[0][1] = -m(2,1);
  m_RP.m_CamVecs[0][2] = -m(2,2);

  m_RP.m_CamVecs[1][0] = m(1,0);
  m_RP.m_CamVecs[1][1] = m(1,1);
  m_RP.m_CamVecs[1][2] = m(1,2);

  m_RP.m_CamVecs[2][0] = m(0,0);
  m_RP.m_CamVecs[2][1] = m(0,1);
  m_RP.m_CamVecs[2][2] = m(0,2);

  m_RP.m_TransformFrame++;
  m_RP.m_FrameObject++;

  GetProjectionMatrix(&m_ProjMatrix(0,0));

  D3DXMatrixMultiply((D3DXMATRIXA16 *)m_CameraProjMatrix.GetData(), (D3DXMATRIXA16 *)m_CameraMatrix.GetData(), (D3DXMATRIXA16 *)m_ProjMatrix.GetData());
  D3DXMatrixInverse((D3DXMATRIXA16 *)m_InvCameraProjMatrix.GetData(), NULL, (D3DXMATRIXA16 *)m_CameraProjMatrix.GetData());

  m_RP.m_PersFlags &= ~RBPF_WASWORLDSPACE;
  m_RP.m_ObjFlags = FOB_TRANS_MASK;
}

_declspec(align(16)) static Matrix44 sIdentityMatrix(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1); 

// Get inverted matrix of the object matrix
// All matrices are 16 bytes alligned to speedup matrix calculations using SSE instructions
Matrix44 &CCObject::GetInvMatrix()
{
  if (m_InvMatrixId == 0)
    return sIdentityMatrix;
  if (m_InvMatrixId > 0)
    return m_ObjMatrices[m_InvMatrixId];

  //PROFILE_FRAME(Objects_ObjInvTransform);

  int n = m_ObjMatrices.size();
  m_ObjMatrices.resize(n+1);
  m_InvMatrixId = n;

  CRenderer *rd = gRenDev;
  Matrix44 &m = m_ObjMatrices[m_InvMatrixId];

  if (m_ObjFlags & FOB_TRANS_ROTATE)
  {
    mathMatrixInverse(m.GetData(), m_Matrix.GetData(), g_CpuFlags);
  }
  else
  if (m_ObjFlags & FOB_TRANS_SCALE)
  {
    float fiScaleX = 1.0f / m_Matrix(0,0);
    float fiScaleY = 1.0f / m_Matrix(1,1);
    float fiScaleZ = 1.0f / m_Matrix(2,2);
    m(0,0) = fiScaleX;
    m(0,1) = m_Matrix(0,1);
    m(0,2) = m_Matrix(0,2);
    m(0,3) = m_Matrix(0,3);

    m(1,0) = m_Matrix(1,0);
    m(1,1) = fiScaleY;
    m(1,2) = m_Matrix(1,2);
    m(1,3) = m_Matrix(1,3);

    m(2,0) = m_Matrix(2,0);
    m(2,1) = m_Matrix(2,1);
    m(2,2) = fiScaleZ;
    m(2,3) = m_Matrix(2,3);

    m(3,0) = -m_Matrix(3,0) * fiScaleX;
    m(3,1) = -m_Matrix(3,1) * fiScaleY;
    m(3,2) = -m_Matrix(3,2) * fiScaleZ;
    m(3,3) = m_Matrix(3,3);
  }
  else
  if (m_ObjFlags & FOB_TRANS_TRANSLATE)
  {
    m(0,0) = m_Matrix(0,0);
    m(0,1) = m_Matrix(0,1);
    m(0,2) = m_Matrix(0,2);
    m(0,3) = m_Matrix(0,3);

    m(1,0) = m_Matrix(1,0);
    m(1,1) = m_Matrix(1,1);
    m(1,2) = m_Matrix(1,2);
    m(1,3) = m_Matrix(1,3);

    m(2,0) = m_Matrix(2,0);
    m(2,1) = m_Matrix(2,1);
    m(2,2) = m_Matrix(2,2);
    m(2,3) = m_Matrix(2,3);

    m(3,0) = -m_Matrix(3,0);
    m(3,1) = -m_Matrix(3,1);
    m(3,2) = -m_Matrix(3,2);
    m(3,3) = m_Matrix(3,3);
  }
  else
    m.SetIdentity();

  return m;
}

// Get Project-ModelView matrix for the current object
// This matrix is passed to the vertex shader
// All matrices are 16 bytes alligned to speedup matrix calculations using SSE instructions
Matrix44 &CCObject::GetVPMatrix()
{
  CRenderer *rd = gRenDev;
  if (m_VPMatrixId == 0)
    return rd->m_CameraProjMatrix;
  if (m_VPMatrixId > 0 && m_VPMatrixFrame == rd->m_RP.m_TransformFrame)
    return m_ObjMatrices[m_VPMatrixId];
  m_VPMatrixFrame = rd->m_RP.m_TransformFrame;

  int n = m_ObjMatrices.size();
  m_ObjMatrices.resize(n+1);
  m_VPMatrixId = n;

  Matrix44 &m = m_ObjMatrices[m_VPMatrixId];

  mathMatrixMultiply(m.GetData(), rd->m_CameraProjMatrix.GetData(), m_Matrix.GetData(), g_CpuFlags);
  //D3DXMatrixMultiplyTranspose((D3DXMATRIX *)m.GetData(), (D3DXMATRIX *)m_Matrix.GetData(), (D3DXMATRIX *)rd->m_CameraProjMatrix.GetData());

  return m;
}

// Set scissor rectangle for the current object
void CCObject::SetScissor()
{
  if (CRenderer::CV_r_scissor)
  {
    if (m_nScissorX2)
    {
      int w = m_nScissorX2 - m_nScissorX1;
      int h = m_nScissorY2 - m_nScissorY1;
      gcpRendD3D->EF_Scissor(true, m_nScissorX1, m_nScissorY1, w, h);
    }
    else
      gcpRendD3D->EF_Scissor(false, 0, 0, 0, 0);
  }
}

// Sets per-object alpha blending state to fade-in/fade-out objects on the distance
void CCObject::SetAlphaState(CPShader *pPS, int nCurState)
{
  CD3D9Renderer *rd = gcpRendD3D;
  // Fake to prevent light coronas from custom blending
  if (!rd->m_RP.m_pRE || rd->m_RP.m_pRE->mfGetType() != eDATA_OcLeaf)
    return;
  if (rd->m_RP.m_pShader->m_Flags2 & EF2_IGNORERESOURCESTATES)
    return;

  if (!(nCurState & GS_BLEND_MASK) && !(rd->m_RP.m_ResourceState & GS_BLEND_MASK))
  {
    int State = rd->m_CurState &= ~GS_BLEND_MASK;
    State |= GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;
    State &= ~GS_DEPTHWRITE;
    rd->EF_SetState(State);
  }
  else
  if (rd->m_CurState & GS_DEPTHFUNC_EQUAL)
    rd->EF_SetState(rd->m_CurState & ~GS_DEPTHFUNC_EQUAL);

  rd->m_RP.m_fCurOpacity = rd->m_RP.m_pCurObject->m_Color.a;
  if (!pPS)
  {
    float fColor = 1.0f;
    int State = rd->m_RP.m_ResourceState & GS_BLEND_MASK;
    if (!State)
      State = nCurState & GS_BLEND_MASK;
    if (State == (GS_BLSRC_DSTCOL | GS_BLDST_ZERO) || State == (GS_BLSRC_ZERO | GS_BLDST_SRCCOL))
    {
      fColor = rd->m_RP.m_fCurOpacity;
      byte bCol = (byte)((1.0f-fColor)*255.0f);
      rd->m_RP.m_NeedGlobalColor.bcolor[0] = bCol;
      rd->m_RP.m_NeedGlobalColor.bcolor[1] = bCol;
      rd->m_RP.m_NeedGlobalColor.bcolor[2] = bCol;
      rd->m_RP.m_NeedGlobalColor.bcolor[3] = (byte)(rd->m_RP.m_fCurOpacity * 255.0f);
      rd->m_RP.m_TexStages[0].m_CO = eCO_ADD;
      rd->m_RP.m_TexStages[0].m_AO = eCO_MODULATE;
      rd->m_RP.m_TexStages[0].m_CA = eCA_Texture | (eCA_Constant<<3);
      rd->m_RP.m_TexStages[0].m_AA = eCA_Texture | (eCA_Constant<<3);
      rd->m_RP.m_FlagsPerFlush |= RBSI_GLOBALRGB;
    }
    else
    if (State == (GS_BLSRC_ONE | GS_BLDST_ONE))
    {
      fColor = rd->m_RP.m_fCurOpacity;
      byte bCol = (byte)(fColor*255.0f);
      rd->m_RP.m_NeedGlobalColor.bcolor[0] = bCol;
      rd->m_RP.m_NeedGlobalColor.bcolor[1] = bCol;
      rd->m_RP.m_NeedGlobalColor.bcolor[2] = bCol;
      rd->m_RP.m_NeedGlobalColor.bcolor[3] = bCol;
      rd->m_RP.m_TexStages[0].m_CO = eCO_MODULATE;
      rd->m_RP.m_TexStages[0].m_AO = eCO_MODULATE;
      rd->m_RP.m_TexStages[0].m_CA = eCA_Texture | (eCA_Constant<<3);
      rd->m_RP.m_TexStages[0].m_AA = eCA_Texture | (eCA_Constant<<3);
      rd->m_RP.m_FlagsPerFlush |= RBSI_GLOBALRGB;
    }
    else
    {
      rd->m_RP.m_NeedGlobalColor.bcolor[3] = (byte)(rd->m_RP.m_fCurOpacity * 255.0f);
      rd->m_RP.m_TexStages[0].m_AO = eCO_MODULATE;
      rd->m_RP.m_TexStages[0].m_AA = eCA_Texture | (eCA_Constant<<3);
      rd->m_RP.m_FlagsPerFlush |= RBSI_GLOBALALPHA;
    }
  }
}

// Set object transform for fixed pipeline shader
void CD3D9Renderer::EF_SetObjectTransform(CCObject *obj, SShader *pSH, int nTransFlags)
{
  if (nTransFlags & FOB_TRANS_MASK)
    mathMatrixMultiply(m_ViewMatrix.GetData(), m_CameraMatrix.GetData(), obj->m_Matrix.GetData(), g_CpuFlags);
  else
    m_ViewMatrix = m_CameraMatrix;
  if (!pSH || !(pSH->m_Flags & EF_HASVSHADER))
  {
    m_matView->LoadMatrix((D3DXMATRIX *)m_ViewMatrix.GetData());
    m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop());
    m_RP.m_PersFlags &= ~RBPF_MATRIXNOTLOADED;
  }
  else
    m_RP.m_PersFlags |= RBPF_MATRIXNOTLOADED;
  m_bInvertedMatrix = false;  
}

// Object changing handling (skinning, shadow maps updating, initial states setting, ...)
bool CD3D9Renderer::EF_ObjectChange(SShader *Shader, SRenderShaderResources *Res, int nObject, CRendElement *pRE)
{
  //PROFILE_FRAME(Objects_Changes);

  CCObject *obj = m_RP.m_VisObjects[nObject];
  if ((obj->m_ObjFlags & FOB_NEAREST) && ((m_RP.m_PersFlags & RBPF_DONTDRAWNEAREST) || CV_r_nodrawnear))
    return false;

  if (Shader)
  {
    if (m_RP.m_pIgnoreObject && !(Shader->m_Flags & EF_SKY) && IsEquivalent(m_RP.m_pIgnoreObject->GetTranslation(), obj->GetTranslation()))
      return false;
    if ((m_RP.m_PersFlags & RBPF_ONLYREFRACTED) && !(obj->m_ObjFlags & FOB_REFRACTED))
      return false;
    if ((m_RP.m_PersFlags & RBPF_IGNOREREFRACTED) && (obj->m_ObjFlags & FOB_REFRACTED))
      return false;
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
  if (nObject)
  {
    m_RP.m_PS.m_NumRendObjects++;

    // Skinning
    if (obj->m_pCharInstance && !CV_r_character_nodeform)
    {
      PROFILE_FRAME(Objects_ChangesSkinning);
      double time0 = 0;
      ticks(time0);
      m_RP.m_PS.m_NumRendSkinnedObjects++;
      CREOcLeaf *pREOCL = (CREOcLeaf *)pRE;
      int nFlags = 0;
      int nVertFormat = -1;
      if (pREOCL->m_pBuffer && pREOCL->m_pBuffer->m_pMats)
      {
        for (int i=0; i<pREOCL->m_pBuffer->m_pMats->Count(); i++)
        {
          CMatInfo *mi = pREOCL->m_pBuffer->m_pMats->Get(i);
          if (!mi->pRE || !mi->shaderItem.m_pShader)
            continue;
          SShader *pSH = (SShader *)mi->shaderItem.m_pShader->GetTemplate(-1);
          if (pSH->m_Flags3 & EF3_NODRAW)
            continue;
          if (nVertFormat < 0)
            nVertFormat = pSH->m_VertexFormatId;
          else
            nVertFormat = m_RP.m_VFormatsMerge[nVertFormat][pSH->m_VertexFormatId];

          if (Shader->m_Flags & EF_NEEDTANGENTS)
            nFlags |= SHPF_TANGENTS;
          if (mi->shaderItem.m_pShaderResources && mi->shaderItem.m_pShaderResources->m_bNeedNormals)
            nFlags |= SHPF_NORMALS;
        }
        pRE->mfCheckUpdate(nVertFormat, nFlags | FHF_FORANIM);
        CLeafBuffer *pLB = pREOCL->m_pBuffer->GetVertexContainer();
        bool bForceUpdate = (pLB->m_UpdateFrame == GetFrameID());
        if (pLB->m_pVertexBuffer && pLB->m_pVertexBuffer->m_bFenceSet)
          bForceUpdate = true;
        obj->m_pCharInstance->ProcessSkinning(obj->m_Matrix.GetTranslationOLD(), obj->m_Matrix, obj->m_nTemplId, obj->m_nLod, bForceUpdate);
      }
      unticks(time0);
      m_RP.m_PS.m_fSkinningTime += (float)(time0*1000.0*g_SecondsPerCycle);
    }

    // shadow map generation
    if (obj->m_pShadowCasters)
    {
      // process casters maps
      for(int i=0; i<obj->m_pShadowCasters->Count(); i++)
        if(obj->m_pShadowCasters->GetAt(i).m_pLS && obj->m_pShadowCasters->GetAt(i).m_pLS->GetShadowMapFrustum())
        {
#ifdef DO_RENDERLOG
          if (m_LogFile)
            Logv(SRendItem::m_RecurseLevel, "*** Prepare shadow maps for REOcLeaf***\n");
#endif
          PrepareDepthMap(obj->m_pShadowCasters->GetAt(i).m_pLS->GetShadowMapFrustum(), false);
          m_RP.m_pCurObject = obj;
        }

        // process receiver map
        /*if(obj->m_pShadowCasters->Count() && obj->m_pShadowCasters->GetAt(0).m_pLS)
          if(obj->m_pShadowCasters->GetAt(0).m_pLS->GetShadowMapFrustumPassiveCasters())
            PrepareDepthMap(obj->m_pShadowCasters->GetAt(0).m_pLS->GetShadowMapFrustumPassiveCasters(), false);
        m_RP.m_pCurObject = obj;*/
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
        if (m_LogFile)
          Logv(SRendItem::m_RecurseLevel, "*** Prepare nearest Z range ***\n");
        
        // set nice fov for weapons  
        Cam.SetFov( Cam.GetFov() * 0.6666f ); 
        SetCamera(Cam);
        m_Viewport.MaxZ = 0.1f;
        m_pd3dDevice->SetViewport(&m_Viewport);
        m_RP.m_Flags |= RBF_NEAREST;
      }
      else
      {
        if (m_LogFile)
          Logv(SRendItem::m_RecurseLevel, "*** Restore Z range ***\n");

        SetCamera(m_RP.m_PrevCamera);
        m_Viewport.MaxZ = 1.0f;
        m_pd3dDevice->SetViewport(&m_Viewport);
        m_RP.m_Flags &= ~RBF_NEAREST;
      }
    }
    if ((obj->m_ObjFlags & FOB_CUBE_MASK) && !bTheSameCM)
      m_TexMan->StartCubeSide(obj);
  }
  else
  {
    if (m_RP.m_Flags & RBF_NEAREST)
    {
      if (m_LogFile)
        Logv(SRendItem::m_RecurseLevel, "*** Restore Z range ***\n");
      SetCamera(m_RP.m_PrevCamera);
      m_Viewport.MaxZ = 1.0f;
      m_pd3dDevice->SetViewport(&m_Viewport);
      m_RP.m_Flags &= ~RBF_NEAREST;
    }
    m_RP.m_pCurObject->m_Matrix.SetIdentity();
    m_ViewMatrix = m_CameraMatrix;
    // Restore transform
    if (!Shader || !(Shader->m_Flags & EF_HASVSHADER))
    {
      m_RP.m_PersFlags &= ~RBPF_MATRIXNOTLOADED;
      m_matView->LoadMatrix((D3DXMATRIX *)&m_CameraMatrix(0,0));
      m_pd3dDevice->SetTransform(D3DTS_VIEW, m_matView->GetTop());
    }
    else
      m_RP.m_PersFlags |= RBPF_MATRIXNOTLOADED;
    m_bInvertedMatrix = false;  
  }
  m_RP.m_pPrevObject = m_RP.m_pCurObject;

  return true;
}

// Set clip plane for the current scene
// on NVidia GPUs we use fake clip planes using texkill PS instruction : m_RP.m_ClipPlaneEnabled = 1
// on ATI hardware we use native ClipPlanes : m_RP.m_ClipPlaneEnabled = 2
void CD3D9Renderer::EF_SetClipPlane (bool bEnable, float *pPlane, bool bRefract)
{
  if (!CV_d3d9_clipplanes)
    return;

  if (bEnable)
  {
#ifdef DO_RENDERLOG
    if (CV_r_log)
      Logv(SRendItem::m_RecurseLevel, "Set clip-plane\n");
#endif
    if (m_RP.m_ClipPlaneEnabled)
      return;
    float p[4];
    p[0] = pPlane[0];
    p[1] = pPlane[1];
    p[2] = pPlane[2];
    p[3] = pPlane[3];
    //if (bRefract)
      //p[3] += 0.1f;
    m_RP.m_ClipPlaneWasOverrided = 0;
    m_RP.m_bClipPlaneRefract = bRefract;
    m_RP.m_CurClipPlane.m_Normal.x = p[0];
    m_RP.m_CurClipPlane.m_Normal.y = p[1];
    m_RP.m_CurClipPlane.m_Normal.z = p[2];
    m_RP.m_CurClipPlane.m_Dist = p[3];
    m_RP.m_CurClipPlane.Init();

    m_RP.m_CurClipPlaneCull = m_RP.m_CurClipPlane;
    m_RP.m_CurClipPlaneCull.m_Dist = -m_RP.m_CurClipPlaneCull.m_Dist;
    int nGPU = m_Features & RFT_HW_MASK;
    if (m_d3dCaps.MaxUserClipPlanes > 0 && nGPU != RFT_HW_GF3 && nGPU != RFT_HW_GF2 && nGPU != RFT_HW_GFFX)
    {
      m_RP.m_ClipPlaneEnabled = 2;
      m_pd3dDevice->SetClipPlane(0, p);
      m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 1);
    }
    else
    {
      m_RP.m_ClipPlaneEnabled = 1;
    }
  }
  else
  {
#ifdef DO_RENDERLOG
    if (CV_r_log)
      Logv(SRendItem::m_RecurseLevel, "Reset clip-plane\n");
#endif
    m_RP.m_ClipPlaneEnabled = 0;
    m_RP.m_ClipPlaneWasOverrided = 0;

    if (m_d3dCaps.MaxUserClipPlanes > 0)
    {
      m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
    }
  }
}


//==============================================================================
// Shader Pipeline
//=======================================================================

// Software vertex deformations stages handling for the current shader
void CD3D9Renderer::EF_Eval_DeformVerts(TArray<SDeform>* Defs)
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

      case eDT_Beam:
        m_RP.m_pCurFuncs->BeamDeform(df);
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

// Software/hardware texture coordinates generating modes handling for the current shader
void CD3D9Renderer::EF_Eval_TexGen(SShaderPass *sfm)
{
  int j, n;
  SShaderTexUnit *shl;
  SShader *ef = m_RP.m_pShader;
  SMRendTexVert *src;
  int m;

  for (j=0; j<sfm->m_TUnits.Num(); j++)
  {
    shl = &sfm->m_TUnits[j];
    
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

// Software/hardware RGBA generating/modificating modes handling for the current shader
void CD3D9Renderer::EF_Eval_RGBAGen(SShaderPass *sfm)
{
  SShader *ef = m_RP.m_pShader;
  int n;
  UCol color;
  bool bSetCol = false;
  color = m_RP.m_NeedGlobalColor;

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
            m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
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
      m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
      break;

    case eERGB_StyleIntens:
      {
        CLightStyle *ls = CLightStyle::mfGetStyle(sfm->m_Style, m_RP.m_RealTime);
        color = sfm->m_FixedColor;
        color.bcolor[0] = (byte)((float)color.bcolor[0] * ls->m_fIntensity);
        color.bcolor[1] = (byte)((float)color.bcolor[1] * ls->m_fIntensity);
        color.bcolor[2] = (byte)((float)color.bcolor[2] * ls->m_fIntensity);
        bSetCol = true;
        m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
      }
      break;

    case eERGB_StyleColor:
      {
        CLightStyle *ls = CLightStyle::mfGetStyle(sfm->m_Style, m_RP.m_RealTime);
        color.dcolor = ls->m_Color.GetTrue();
        bSetCol = true;
        m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
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
          m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
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
            m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
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
            byte r = (byte)(CLAMP(v * sfm->m_RGBNoise->m_RangeR + sfm->m_RGBNoise->m_ConstR, 0.0f, 1.0f) * 255.0f);
            v = RandomNum();
            byte g = (byte)(CLAMP(v * sfm->m_RGBNoise->m_RangeG + sfm->m_RGBNoise->m_ConstG, 0.0f, 1.0f) * 255.0f);
            v = RandomNum();
            byte b = (byte)(CLAMP(v * sfm->m_RGBNoise->m_RangeB + sfm->m_RGBNoise->m_ConstB, 0.0f, 1.0f) * 255.0f);
            
            color.bcolor[0] = r;
            color.bcolor[1] = g;
            color.bcolor[2] = b;
            COLCONV(color.dcolor);
            bSetCol = true;
            m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
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
        m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
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
              m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
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
            m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
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
            byte a = (byte)(CLAMP(v * sfm->m_ANoise->m_RangeA + sfm->m_ANoise->m_ConstA, 0.0f, 1.0f) * 255.0f);
            
            color.bcolor[3] = a;
            bSetCol = true;
            m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
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
          m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
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
          m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
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
          m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
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
          m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
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
          m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
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

  if (bSetCol)
  {
    Exchange(color.bcolor[0], color.bcolor[2]);
    m_RP.m_NeedGlobalColor = color;
    //m_RP.m_FlagsPerFlush |= RBSI_GLOBALRGB | RBSI_GLOBALALPHA;
  }
}

// Software Normals generating/modificating modes handling for the current shader
void CD3D9Renderer::EF_EvalNormalsRB(SShader *ef)
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

// Updating of textures animating sequence
void CD3D9Renderer::EF_UpdateTextures(SShaderPass *Layer)
{
  SShaderTexUnit *stl;
  int j;
  for (j=0; j<Layer->m_TUnits.Num(); j++)
  {
    stl = &Layer->m_TUnits[j];
    
    if (!stl->m_TexPic && !stl->m_AnimInfo)
      break;
    
    stl->mfUpdate();
  }
}

// Updating of textures animating sequence
void CD3D9Renderer::EF_UpdateTextures(SShaderPassHW *Layer)
{
  SShaderTexUnit *stl;
  int j;
  for (j=0; j<Layer->m_TUnits.Num(); j++)
  {
    stl = &Layer->m_TUnits[j];
    
    if (!stl->m_TexPic && !stl->m_AnimInfo)
      break;
    
    stl->mfUpdate();
  }
}

//=================================================================================
// Set current stencil states 
void SStencil::mfSet()
{
  gRenDev->EF_SetStencilState(m_State, m_FuncRef, m_FuncMask);
}

void CRenderer::EF_SetStencilState(int st, uint nStencRef, uint nStencMask)
{
  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  if (nStencRef != m_CurStencRef)
  {
    m_CurStencRef = nStencRef;
    dv->SetRenderState(D3DRS_STENCILREF, nStencRef);
  }
  if (nStencMask != m_CurStencMask)
  {
    m_CurStencMask = nStencMask;
    dv->SetRenderState(D3DRS_STENCILMASK, nStencMask);
  }

  int Changed = st ^ m_CurStencilState;
  if (!Changed)
    return;
  if (Changed & FSS_STENCIL_TWOSIDED)
  {
    if (st & FSS_STENCIL_TWOSIDED)
      dv->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, TRUE);
    else
      dv->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE);
  }
  if (Changed & FSS_STENCFUNC_MASK)
  {
    int nCurFunc = st & FSS_STENCFUNC_MASK;
    switch(nCurFunc)
    {
      case FSS_STENCFUNC_ALWAYS:
        dv->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
    	  break;
      case FSS_STENCFUNC_NEVER:
        dv->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_NEVER);
    	  break;
      case FSS_STENCFUNC_LESS:
        dv->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESS);
    	  break;
      case FSS_STENCFUNC_LEQUAL:
        dv->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
    	  break;
      case FSS_STENCFUNC_GREATER:
        dv->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_GREATER);
    	  break;
      case FSS_STENCFUNC_GEQUAL:
        dv->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_GREATEREQUAL);
    	  break;
      case FSS_STENCFUNC_EQUAL:
        dv->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
    	  break;
      case FSS_STENCFUNC_NOTEQUAL:
        dv->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL);
    	  break;
      default:
        assert(false);
    }
  }
  if (Changed & FSS_STENCFAIL_MASK)
  {
    int nCurOp = (st & FSS_STENCFAIL_MASK);
    switch(nCurOp >> FSS_STENCFAIL_SHIFT)
    {
      case FSS_STENCOP_KEEP:
        dv->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
    	  break;
      case FSS_STENCOP_REPLACE:
        dv->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_REPLACE);
    	  break;
      case FSS_STENCOP_INCR:
        dv->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_INCRSAT);
    	  break;
      case FSS_STENCOP_DECR:
        dv->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_DECRSAT);
    	  break;
      case FSS_STENCOP_INCR_WRAP:
        dv->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_INCR);
        break;
      case FSS_STENCOP_DECR_WRAP:
        dv->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_DECR);
        break;
      case FSS_STENCOP_ZERO:
        dv->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_ZERO);
    	  break;
      default:
        assert(false);
    }
  }
  if (Changed & FSS_STENCZFAIL_MASK)
  {
    int nCurOp = (st & FSS_STENCZFAIL_MASK);
    switch(nCurOp >> FSS_STENCZFAIL_SHIFT)
    {
      case FSS_STENCOP_KEEP:
        dv->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
    	  break;
      case FSS_STENCOP_REPLACE:
        dv->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_REPLACE);
    	  break;
      case FSS_STENCOP_INCR:
        dv->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INCRSAT);
    	  break;
      case FSS_STENCOP_DECR:
        dv->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_DECRSAT);
    	  break;
      case FSS_STENCOP_INCR_WRAP:
        dv->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INCR);
        break;
      case FSS_STENCOP_DECR_WRAP:
        dv->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_DECR);
        break;
      case FSS_STENCOP_ZERO:
        dv->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_ZERO);
    	  break;
      default:
        assert(false);
    }
  }
  if (Changed & FSS_STENCPASS_MASK)
  {
    int nCurOp = (st & FSS_STENCPASS_MASK);
    switch(nCurOp >> FSS_STENCPASS_SHIFT)
    {
      case FSS_STENCOP_KEEP:
        dv->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
    	  break;
      case FSS_STENCOP_REPLACE:
        dv->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
    	  break;
      case FSS_STENCOP_INCR:
        dv->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCRSAT);
    	  break;
      case FSS_STENCOP_DECR:
        dv->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECRSAT);
    	  break;
      case FSS_STENCOP_INCR_WRAP:
        dv->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR);
        break;
      case FSS_STENCOP_DECR_WRAP:
        dv->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECR);
        break;
      case FSS_STENCOP_ZERO:
        dv->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_ZERO);
    	  break;
      default:
        assert(false);
    }
  }

  if (Changed & (FSS_STENCFUNC_MASK << FSS_CCW_SHIFT))
  {
    int nCurFunc = (st & (FSS_STENCFUNC_MASK << FSS_CCW_SHIFT));
    switch(nCurFunc >> FSS_CCW_SHIFT)
    {
      case FSS_STENCFUNC_ALWAYS:
        dv->SetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_ALWAYS);
    	  break;
      case FSS_STENCFUNC_NEVER:
        dv->SetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_NEVER);
    	  break;
      case FSS_STENCFUNC_LESS:
        dv->SetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_LESS);
    	  break;
      case FSS_STENCFUNC_LEQUAL:
        dv->SetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_LESSEQUAL);
    	  break;
      case FSS_STENCFUNC_GREATER:
        dv->SetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_GREATER);
    	  break;
      case FSS_STENCFUNC_GEQUAL:
        dv->SetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_GREATEREQUAL);
    	  break;
      case FSS_STENCFUNC_EQUAL:
        dv->SetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_EQUAL);
    	  break;
      case FSS_STENCFUNC_NOTEQUAL:
        dv->SetRenderState(D3DRS_CCW_STENCILFUNC, D3DCMP_NOTEQUAL);
    	  break;
      default:
        assert(false);
    }
  }
  if (Changed & (FSS_STENCFAIL_MASK << FSS_CCW_SHIFT))
  {
    int nCurOp = (st & (FSS_STENCFAIL_MASK << FSS_CCW_SHIFT));
    switch(nCurOp >> (FSS_STENCFAIL_SHIFT+FSS_CCW_SHIFT))
    {
      case FSS_STENCOP_KEEP:
        dv->SetRenderState(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP);
    	  break;
      case FSS_STENCOP_REPLACE:
        dv->SetRenderState(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_REPLACE);
    	  break;
      case FSS_STENCOP_INCR:
        dv->SetRenderState(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_INCRSAT);
    	  break;
      case FSS_STENCOP_DECR:
        dv->SetRenderState(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_DECRSAT);
    	  break;
      case FSS_STENCOP_INCR_WRAP:
        dv->SetRenderState(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_INCR);
        break;
      case FSS_STENCOP_DECR_WRAP:
        dv->SetRenderState(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_DECR);
        break;
      case FSS_STENCOP_ZERO:
        dv->SetRenderState(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_ZERO);
    	  break;
      default:
        assert(false);
    }
  }
  if (Changed & (FSS_STENCZFAIL_MASK << FSS_CCW_SHIFT))
  {
    int nCurOp = (st & (FSS_STENCZFAIL_MASK << FSS_CCW_SHIFT));
    switch(nCurOp >> (FSS_STENCZFAIL_SHIFT+FSS_CCW_SHIFT))
    {
      case FSS_STENCOP_KEEP:
        dv->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
    	  break;
      case FSS_STENCOP_REPLACE:
        dv->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_REPLACE);
    	  break;
      case FSS_STENCOP_INCR:
        dv->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_INCRSAT);
    	  break;
      case FSS_STENCOP_DECR:
        dv->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_DECRSAT);
    	  break;
      case FSS_STENCOP_INCR_WRAP:
        dv->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_INCR);
        break;
      case FSS_STENCOP_DECR_WRAP:
        dv->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_DECR);
        break;
      case FSS_STENCOP_ZERO:
        dv->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_ZERO);
    	  break;
      default:
        assert(false);
    }
  }
  if (Changed & (FSS_STENCPASS_MASK << FSS_CCW_SHIFT))
  {
    int nCurOp = (st & (FSS_STENCPASS_MASK << FSS_CCW_SHIFT));
    switch(nCurOp >> (FSS_STENCPASS_SHIFT+FSS_CCW_SHIFT))
    {
      case FSS_STENCOP_KEEP:
        dv->SetRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_KEEP);
    	  break;
      case FSS_STENCOP_REPLACE:
        dv->SetRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_REPLACE);
    	  break;
      case FSS_STENCOP_INCR:
        dv->SetRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_INCRSAT);
    	  break;
      case FSS_STENCOP_DECR:
        dv->SetRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_DECRSAT);
    	  break;
      case FSS_STENCOP_INCR_WRAP:
        dv->SetRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_INCR);
        break;
      case FSS_STENCOP_DECR_WRAP:
        dv->SetRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_DECR);
        break;
      case FSS_STENCOP_ZERO:
        dv->SetRenderState(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_ZERO);
    	  break;
      default:
        assert(false);
    }
  }

  m_CurStencilState = st;
}

// Set current render states 
void CRenderer::EF_SetState(int st)
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
  if (st & GS_DEPTHWRITE)
  {
    if (m_RP.m_LastVP && m_RP.m_pRE)
      m_RP.m_pRE->m_LastVP = m_RP.m_LastVP;
  }

  Changed = st ^ m_CurState;
  if (!Changed)
    return;
  
  //PROFILE_FRAME(State_RStates);

  LPDIRECT3DDEVICE9 dv = gcpRendD3D->mfGetD3DDevice();
  m_RP.m_PS.m_NumStateChanges++;

  if (Changed & (GS_DEPTHFUNC_EQUAL | GS_DEPTHFUNC_GREAT))
  {
    if (!(m_RP.m_FlagsPerFlush & RBSI_DEPTHFUNC))
    {
      if (st & (GS_DEPTHFUNC_EQUAL|GS_DEPTHFUNC_GREAT))
      {
        if (st & GS_DEPTHFUNC_EQUAL)
          dv->SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
        else
          dv->SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATER);
      }
      else
        dv->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    }
    else
    {
      st &= ~(GS_DEPTHFUNC_EQUAL|GS_DEPTHFUNC_GREAT);
      st |= (m_CurState & (GS_DEPTHFUNC_EQUAL|GS_DEPTHFUNC_GREAT));
    }
  }
  if (Changed & GS_POLYLINE)
  {
    if (m_polygon_mode == R_SOLID_MODE)
    {
      if (st & GS_POLYLINE)
        dv->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
      else
        dv->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
    }
  }

  if (Changed & (GS_NOCOLMASK|GS_COLMASKONLYALPHA|GS_COLMASKONLYRGB))
  {
    if (st & (GS_NOCOLMASK|GS_COLMASKONLYALPHA|GS_COLMASKONLYRGB))
    {
      if (st & GS_NOCOLMASK)
        dv->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
      else
      if (st & GS_COLMASKONLYALPHA)
        dv->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
      else
      if (st & GS_COLMASKONLYRGB)
        dv->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE);
    }
    else
      dv->SetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
  }
  
  if (Changed & GS_BLEND_MASK)
  {
    if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHABLEND))
    {
      if ((st & GS_BLEND_MASK) == GS_BLEND_MASK)
      {
        st &= ~GS_BLEND_MASK;
        st |= (m_CurState & GS_BLEND_MASK);
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
            iLog->Log("CD3D9Renderer::SetState: invalid src blend state bits '%d'", st & GS_BLSRC_MASK);
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
            if (m_nHDRType == 1 && (m_RP.m_PersFlags & RBPF_HDR))
              dst = D3DBLEND_ONE;
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
            iLog->Log("CD3D9Renderer::SetState: invalid dst blend state bits '%d'", st & GS_BLDST_MASK);
            break;
        }
        if (!(m_CurState & GS_BLEND_MASK))
          dv->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        dv->SetRenderState(D3DRS_SRCBLEND,  src);
        dv->SetRenderState(D3DRS_DESTBLEND, dst);
      }
      else
        dv->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }
    else
    {
      st &= ~GS_BLEND_MASK;
      st |= (m_CurState & GS_BLEND_MASK);
    }
  }
  
skip:
  if (Changed & GS_DEPTHWRITE)
  {
    if (!(m_RP.m_FlagsPerFlush & RBSI_DEPTHWRITE))
    {
      if (st & GS_DEPTHWRITE)
        dv->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
      else
        dv->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    }
    else
    {
      st &= ~GS_DEPTHWRITE;
      st |= (m_CurState & GS_DEPTHWRITE);
    }
  }
  
  if (Changed & GS_NODEPTHTEST)
  {
    if (!(m_RP.m_FlagsPerFlush & RBSI_DEPTHTEST))
    {
      if (st & GS_NODEPTHTEST)
        dv->SetRenderState(D3DRS_ZENABLE, FALSE);
      else
        dv->SetRenderState(D3DRS_ZENABLE, TRUE);
    }
    else
    {
      st &= ~GS_NODEPTHTEST;
      st |= (m_CurState & GS_NODEPTHTEST);
    }
  }

  if (Changed & GS_STENCIL)
  {
    if (!(m_RP.m_FlagsPerFlush & RBSI_STENCIL) && !(m_RP.m_PersFlags & RBPF_MEASUREOVERDRAW))
    {
      if (st & GS_STENCIL)
        dv->SetRenderState(D3DRS_STENCILENABLE, TRUE);
      else
        dv->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    }
    else
    {
      st &= ~GS_STENCIL;
      st |= (m_CurState & GS_STENCIL);
    }
  }

  if (Changed & GS_ALPHATEST_MASK)
  {
    if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHATEST))
    {
      if (st & GS_ALPHATEST_MASK)
      {
        if (!(m_CurState & GS_ALPHATEST_MASK))
          dv->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        switch (st & GS_ALPHATEST_MASK)
        {
          case GS_ALPHATEST_GREATER0:
            dv->SetRenderState(D3DRS_ALPHAREF, 0);
            dv->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
            break;
            
          case GS_ALPHATEST_LESS128:
            dv->SetRenderState(D3DRS_ALPHAREF, 0x80);
            dv->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_LESS);
            break;
            
          case GS_ALPHATEST_GEQUAL128:
            dv->SetRenderState(D3DRS_ALPHAREF, 0x80);
            dv->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
            break;

          case GS_ALPHATEST_GEQUAL64:
            dv->SetRenderState(D3DRS_ALPHAREF, 0x40);
            dv->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
            break;
        }
      }
      else
        dv->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    }
    else
    {
      st &= ~GS_ALPHATEST_MASK;
      st |= m_CurState & GS_ALPHATEST_MASK;
    }
  }
  m_CurState = st;
}

// Set current texture color op modes (used in fixed pipeline shaders)
void CD3D9Renderer::SetColorOp(byte eCo, byte eAo, byte eCa, byte eAa)
{
  EF_SetColorOp(eCo, eAo, eCa, eAa);
}

void CD3D9Renderer::EF_SetColorOp(byte eCo, byte eAo, byte eCa, byte eAa)
{
  int stage = m_TexMan->m_CurStage;
  
  if (eCo != 255 && eCo != m_eCurColorOp[stage])
  {
    switch (eCo)
    {
      case eCO_MODULATE:
      default:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
        break;

      case eCO_BLENDDIFFUSEALPHA:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);
        break;
        
      case eCO_BLENDTEXTUREALPHA:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
        break;
        
      case eCO_MODULATE4X:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
        break;
        
      case eCO_MODULATE2X:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
        break;
        
      case eCO_ADD:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_ADD);
        break;

      case eCO_ADDSIGNED:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_ADDSIGNED);
        break;

      case eCO_MULTIPLYADD:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD);
        break;

      case eCO_REPLACE:
      case eCO_DECAL:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        break;

      case eCO_ARG2:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
        break;

      case eCO_BUMPENVMAP:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_BUMPENVMAP);
        break;

      case eCO_MODULATEALPHA_ADDCOLOR:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATEALPHA_ADDCOLOR);
        break;

      case eCO_MODULATECOLOR_ADDALPHA:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATECOLOR_ADDALPHA);
        break;

      case eCO_MODULATEINVALPHA_ADDCOLOR:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATEINVALPHA_ADDCOLOR);
        break;

      case eCO_MODULATEINVCOLOR_ADDALPHA:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATEINVCOLOR_ADDALPHA);
        break;

      case eCO_DOTPRODUCT3:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3);
        break;

      case eCO_LERP:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_LERP);
        break;

      case eCO_SUBTRACT:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_SUBTRACT);
        break;

      case eCO_DISABLE:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
        break;
    }
    m_eCurColorOp[stage] = eCo;
  }

  if (eAo != 255 && eAo != m_eCurAlphaOp[stage])
  {
    switch (eAo)
    {
      case eCO_MODULATE:
      default:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        break;

      case eCO_BLENDDIFFUSEALPHA:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_BLENDDIFFUSEALPHA);
        break;
        
      case eCO_BLENDTEXTUREALPHA:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_BLENDTEXTUREALPHA);
        break;
        
      case eCO_MODULATE4X:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE4X);
        break;
        
      case eCO_MODULATE2X:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE2X);
        break;
        
      case eCO_ADD:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_ADD);
        break;

      case eCO_ADDSIGNED:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_ADDSIGNED);
        break;

      case eCO_MULTIPLYADD:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MULTIPLYADD);
        break;

      case eCO_REPLACE:
      case eCO_DECAL:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        break;

      case eCO_ARG2:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        break;

      case eCO_BUMPENVMAP:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_BUMPENVMAP);
        break;

      case eCO_SUBTRACT:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SUBTRACT);
        break;

      case eCO_LERP:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_LERP);
        break;

      case eCO_DISABLE:
        m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;
    }
    m_eCurAlphaOp[stage] = eAo;
  }

  if (eCa != 255 && eCa != m_eCurColorArg[stage])
  {
    if ((eCa & 7) != (m_eCurColorArg[stage] & 7))
    {
      switch (eCa & 7)
      {
        case eCA_Texture:
        default:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
          break;
        case eCA_Diffuse:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
          break;
        case eCA_Specular:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_SPECULAR);
          break;
        case eCA_Previous:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_CURRENT);
          break;
        case eCA_Constant:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_TFACTOR);
          break;
      }
    }
    if (((eCa>>3) & 7) != ((m_eCurColorArg[stage]>>3) & 7))
    {
      switch ((eCa >> 3) & 7)
      {
        case eCA_Texture:
        default:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_TEXTURE);
          break;
        case eCA_Diffuse:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
          break;
        case eCA_Specular:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_SPECULAR);
          break;
        case eCA_Previous:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
          break;
        case eCA_Constant:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_TFACTOR);
          break;
      }
    }
    if ((eCa >> 6) != (m_eCurColorArg[stage] >> 6))
    {
      switch (eCa >> 6)
      {
        case eCA_Texture:
        default:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG0, D3DTA_TEXTURE);
          break;
        case eCA_Diffuse:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG0, D3DTA_DIFFUSE);
          break;
        case eCA_Specular:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG0, D3DTA_SPECULAR);
          break;
        case eCA_Previous:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG0, D3DTA_CURRENT);
          break;
        case eCA_Constant:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_COLORARG0, D3DTA_TFACTOR);
          break;
      }
    }
    m_eCurColorArg[stage] = eCa;
  }

  if (eAa != 255 && eAa != m_eCurAlphaArg[stage])
  {
    if ((eAa & 7) != (m_eCurAlphaArg[stage] & 7))
    {
      switch (eAa & 7)
      {
        case eCA_Texture:
        default:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
          break;
        case eCA_Diffuse:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
          break;
        case eCA_Specular:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_SPECULAR);
          break;
        case eCA_Previous:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
          break;
        case eCA_Constant:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
          break;
      }
    }
    if (((eAa>>3) & 7) != ((m_eCurAlphaArg[stage]>>3) & 7))
    {
      switch ((eAa >> 3) & 7)
      {
        case eCA_Texture:
        default:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
          break;
        case eCA_Diffuse:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
          break;
        case eCA_Specular:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_SPECULAR);
          break;
        case eCA_Previous:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
          break;
        case eCA_Constant:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
          break;
      }
    }
    if ((eAa >> 6) != (m_eCurAlphaArg[stage] >> 6))
    {
      switch (eAa >> 6)
      {
        case eCA_Texture:
        default:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG0, D3DTA_TEXTURE);
          break;
        case eCA_Diffuse:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG0, D3DTA_DIFFUSE);
          break;
        case eCA_Specular:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG0, D3DTA_SPECULAR);
          break;
        case eCA_Previous:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG0, D3DTA_CURRENT);
          break;
        case eCA_Constant:
          m_pd3dDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG0, D3DTA_TFACTOR);
          break;
      }
    }
    m_eCurAlphaArg[stage] = eAa;
  }
}

// Set current geometry culling modes
void CD3D9Renderer::D3DSetCull(ECull eCull)
{ 
  if (eCull == m_eCull)
    return;

  if (eCull == eCULL_None)
    m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
  else
  {
    if ((eCull == eCULL_Back && !(m_RP.m_PersFlags & RBPF_DRAWMIRROR)) || (eCull != eCULL_Back && (m_RP.m_PersFlags & RBPF_DRAWMIRROR)))
      m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    else
      m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
  }
  m_eCull = eCull;
}

// Init states before rendering of the scene
void CD3D9Renderer::EF_PreRender(int Stage)
{
  int i;
  if (m_bDeviceLost)
    return;

  if (Stage & 1)
  { // Before preprocess
    m_RP.m_RealTime = iTimer->GetCurrTime();
  
    m_RP.m_Flags = 0;
    m_RP.m_pPrevObject = NULL;
    m_RP.m_FrameObject++;
    
    EF_SetCameraInfo();
    if (Stage == 1)
      CCGVProgram_D3D::mfSetGlobalParams();
  
    for (i=0; i<m_RP.m_DLights[SRendItem::m_RecurseLevel].Num(); i++)
    {
      CDLight *dl = m_RP.m_DLights[SRendItem::m_RecurseLevel][i];
      if (dl->m_Flags & DLF_FAKE)
        continue;

      if (dl->m_Flags & DLF_SUN)
      {
        Vec3d pos = GetCamera().GetPos();
        Vec3d delta = dl->m_Origin - pos;
        delta.Normalize();
        m_RP.m_SunDir = delta;
        m_RP.m_SunColor = dl->m_Color;
      }
    }
  //  if (i == m_RP.m_DLights.Num() && m_RP.m_SunDir == Vec3d(0,0,0))
    if (i == m_RP.m_DLights[SRendItem::m_RecurseLevel].Num() && IsEquivalent(m_RP.m_SunDir,Vec3d(0,0,0)))
      m_RP.m_SunDir = Vec3d(0,0,1);
  }

  if (Stage & 2)
  {  // After preprocess
    if (!m_RP.m_bStartPipeline && !m_bWasCleared && !(m_RP.m_PersFlags & RBPF_NOCLEARBUF))
    {
      m_RP.m_bStartPipeline = true;
      EF_ClearBuffers(false, false, NULL);
    }
  }
  m_RP.m_pCurLight = NULL;
}

// Restore states after rendering of the scene
void CD3D9Renderer::EF_PostRender()
{
  //FrameProfiler f("CD3D9Renderer:EF_PostRender", iSystem );

  EF_ObjectChange(NULL, NULL, 0, NULL);
  m_RP.m_pRE = NULL;

  HRESULT h;
  if (m_RP.m_PersFlags & RBPF_USESTREAM1)
  {
    m_RP.m_PersFlags &= ~RBPF_USESTREAM1;
    h = m_pd3dDevice->SetStreamSource( 1, NULL, 0, 0);
  }
  if (m_RP.m_PersFlags & RBPF_USESTREAM2)
  {
    m_RP.m_PersFlags &= ~RBPF_USESTREAM2;
    h = m_pd3dDevice->SetStreamSource( 2, NULL, 0, 0);
  }
  if (m_RP.m_PersFlags & RBPF_SETCLIPPLANE)
  {
    EF_SetClipPlane(false, NULL, false);
    m_RP.m_PersFlags &= ~RBPF_SETCLIPPLANE;
  }
  m_Viewport.MaxZ = 1.0f;
  m_pd3dDevice->SetViewport(&m_Viewport);
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
  m_RP.m_pCurObject = m_RP.m_VisObjects[0];

  //ResetToDefault();
}

//=================================================================================
// Check buffer overflow during geometry batching
void CD3D9Renderer::EF_CheckOverflow(int nVerts, int nInds, CRendElement *re)
{
  if (m_RP.m_pRE || (m_RP.m_RendNumVerts+nVerts >= m_RP.m_MaxVerts || m_RP.m_RendNumIndices+nInds >= m_RP.m_MaxTris*3))
  {
    m_RP.m_pRenderFunc();
    if (nVerts >= m_RP.m_MaxVerts)
    {
      iLog->Log("CD3D9Renderer::EF_CheckOverflow: numVerts > MAX (%d > %d)\n", nVerts, m_RP.m_MaxVerts);
      nVerts = m_RP.m_MaxVerts;
    }
    if (nInds >= m_RP.m_MaxTris*3)
    {
      iLog->Log("CD3D9Renderer::EF_CheckOverflow: numIndices > MAX (%d > %d)\n", nInds, m_RP.m_MaxTris*3);
      nInds = m_RP.m_MaxTris*3;
    }
    EF_Start(m_RP.m_pShader, m_RP.m_pStateShader, m_RP.m_pShaderResources, m_RP.m_pFogVolume ? (m_RP.m_pFogVolume-&m_RP.m_FogVolumes[0]) : 0, re);
  }
}


// Initialize of the new shader pipeline (only 2d)
void CD3D9Renderer::EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, CRendElement *re) 
{
  //FrameProfiler f("CD3D9Renderer:EF_Start", iSystem );

  m_RP.m_RendPass = 0;
  m_RP.m_RendNumIndices = 0;
  m_RP.m_RendNumVerts = 0;
  m_RP.m_FirstIndex = 0;
  m_RP.m_FirstVertex = 0;
  m_RP.m_BaseVertex = 0;
  m_RP.m_pShader = ef;
#ifdef PIPE_USE_INSTANCING
  m_RP.m_MergedObjects.SetUse(0);
#endif
  m_RP.m_pCurLightMaterial = NULL;
  m_RP.m_pStateShader = efState;
  m_RP.m_pShaderResources = Res;
  m_RP.m_FlagsPerFlush = 0;
  m_RP.m_FlagsModificators = 0;
  m_RP.m_pFogVolume = NULL;
  m_RP.m_pRE = NULL;
  m_RP.m_fCurOpacity = 1.0f;

  // Choose appropriate shader technique depend on some input parameters
  if (ef->m_HWTechniques.Num())
  {
    int nHW = EF_SelectHWTechnique(ef);
    if (nHW >= 0)
      m_RP.m_pCurTechnique = ef->m_HWTechniques[nHW];
    else
      m_RP.m_pCurTechnique = NULL;
  }
  else
    m_RP.m_pCurTechnique = NULL;

  m_RP.m_Frame++;
}

// Start of the new shader pipeline (3D pipeline version)
void CD3D9Renderer::EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, int numFog, CRendElement *re)
{
  m_RP.m_RendPass = 0;
  m_RP.m_FirstIndex = 0;
  m_RP.m_FirstVertex = 0;
  m_RP.m_BaseVertex = 0;
  m_RP.m_RendNumIndices = 0;
  m_RP.m_RendNumVerts = 0;
  m_RP.m_pShader = ef;
  m_RP.m_pStateShader = efState;
  m_RP.m_pShaderResources = Res;
  m_RP.m_pCurEnvTexture = NULL;
#ifdef PIPE_USE_INSTANCING
  m_RP.m_MergedObjects.SetUse(0);
#endif
  m_RP.m_pCurLightMaterial = NULL;
  m_RP.m_FlagsPerFlush = 0;
  m_RP.m_FlagsModificators = 0;
  m_RP.m_fCurOpacity = 1.0f;
  if (numFog && CV_r_VolumetricFog)
    m_RP.m_pFogVolume = &m_RP.m_FogVolumes[numFog];
  else
    m_RP.m_pFogVolume = NULL;
  m_RP.m_ObjFlags = m_RP.m_pCurObject->m_ObjFlags;

  m_RP.m_CurVFormat = ef->m_VertexFormatId;

  SBufInfoTable *pOffs = &gBufInfoTable[m_RP.m_CurVFormat];
  int Size = m_VertexSize[m_RP.m_CurVFormat];    
  m_RP.m_Stride = Size;
  m_RP.m_OffsD  = pOffs->OffsColor;
  m_RP.m_OffsT  = pOffs->OffsTC;
  m_RP.m_OffsN  = pOffs->OffsNormal;
  m_RP.m_NextPtr = m_RP.m_Ptr;
  m_RP.m_DynLMask = m_RP.m_pCurObject->m_DynLMMask;
  m_RP.m_MergedREs.SetUse(0);
  m_RP.m_MergedObjs.SetUse(0);

  if (!EF_BuildLightsList())
    iLog->Log("WARNING: CD3D9Renderer::EF_BuildLightsList: Too many light sources per render item (> 16). Shader: '%s'\n", ef->m_Name.c_str());

  // Choose appropriate shader technique depend on some input parameters
  if (ef->m_HWTechniques.Num())
  {
    m_RP.m_pRE = re;
    int nHW = EF_SelectHWTechnique(ef);
    if (nHW >= 0)
      m_RP.m_pCurTechnique = ef->m_HWTechniques[nHW];
    else
      m_RP.m_pCurTechnique = NULL;
  }
  else
    m_RP.m_pCurTechnique = NULL;

  m_RP.m_pRE = NULL;

  m_RP.m_Frame++;
}

//========================================================================================
// Set current HW light states (used in fixed pipeline shaders)
void CD3D9Renderer::EF_SetHWLight(int Num, vec4_t Pos, CFColor& Diffuse, CFColor& Specular, float ca, float la, float qa, float fRange)
{
  m_Lights[Num].Position.x = Pos[0];
  m_Lights[Num].Position.y = Pos[1];
  m_Lights[Num].Position.z = Pos[2];
  m_Lights[Num].Range = fRange;
  if (!Pos[3])
  {
    Vec3d v = Vec3d(Pos[0], Pos[1], Pos[2]);
    v.Normalize();
    m_Lights[Num].Direction.x = -v[0];
    m_Lights[Num].Direction.y = -v[1];
    m_Lights[Num].Direction.z = -v[2];
    m_Lights[Num].Type = D3DLIGHT_DIRECTIONAL;
  }
  else
    m_Lights[Num].Type = D3DLIGHT_POINT;

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

  m_RP.m_CurrentVLights |= 1<<Num;
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
    return 0.1f;
}

inline void TransformPosition33(Vec3& out, Vec3& in, Matrix33& m)
{
  out.x = in.x * m(0,0) + in.y * m(1,0) + in.z * m(2,0);
  out.y = in.x * m(0,1) + in.y * m(1,1) + in.z * m(2,1);
  out.z = in.x * m(0,2) + in.y * m(1,2) + in.z * m(2,2);
}

// Calculate light parameters used for HW vertex lighting (used in fixed pipeline shaders)
bool CD3D9Renderer::EF_SetLights(int Flags)
{
  //float MaxD3DRange = cry_sqrtf(FLT_MAX); // xbox libs freak out about illegal light ranges

  vec4_t Pos;
  Vec3d lpos;
  int n = 0;
  float fCA, fLA, fQA;
  Vec3d vCenterRE;
  float fRadRE;
  bool bCalcDist = false;
  CFColor cDiffuse, cSpecular;
  cSpecular = CFColor(0.0f);

  for (int i=0; i<m_RP.m_NumActiveDLights; i++)
  {
    if (i >= 16)
      break;
    CDLight *dl = m_RP.m_pActiveDLights[i];
    if ((Flags & LMF_IGNOREPROJLIGHTS) && (dl->m_Flags & DLF_PROJECT))
      continue;
    int nlM;
    bool bSpecularOnly = false;
    if (nlM=(Flags & LMF_LIGHT_MASK))
    {
      if ((nlM >> LMF_LIGHT_SHIFT) != i)
        continue;
    }
    if (dl->m_Flags & DLF_LM)
    {
      if ((m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_LIGHTMAP]) || m_RP.m_pCurObject->m_nLMId)
      {
        if (Flags & LMF_NOSPECULAR)
          continue;
        bSpecularOnly = true;
      }
    }

    // some statistics
    if (!(m_RP.m_StatLightMask & (1<<i)))
    {
      m_RP.m_StatLightMask |= (1<<i);
      m_RP.m_StatNumLights++;
    }

    Matrix44 m = m_RP.m_pCurObject->GetInvMatrix();
    TransformPosition(lpos, dl->m_Origin, m);
    //TransformPoint(m, dl->m_Origin, lpos);
    //TransformPosition33(lpos, dl->m_Origin-m_RP.m_pCurObject->m_Matrix.GetTranslationOLD(), Matrix33(GetTransposed44(m_RP.m_pCurObject->m_Matrix)));

    float fRange = 0;
    if (dl->m_Flags & DLF_DIRECTIONAL)
    {
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
      if (!bCalcDist)
      {
        bCalcDist = true;
        if (m_RP.m_pRE)
        {
          Vec3d vMins, vMaxs;
          m_RP.m_pRE->mfGetBBox(vMins, vMaxs);
          vCenterRE = (vMins + vMaxs) * 0.5f;
          vCenterRE += m_RP.m_pCurObject->GetTranslation();
          fRadRE = (vMaxs - vMins).Length() * 0.5f;
        }
        else
        {
          fRadRE = 1000.0f;
          vCenterRE = Vec3(0,0,0);
        }
      }
      float fDist = max(0.1f, (vCenterRE - dl->m_Origin).Length());
      float fMaxDist = CLAMP(fDist + fRadRE, dl->m_fRadius * 0.1f, dl->m_fRadius * 0.99f);
      float fMinDist = CLAMP(fDist - fRadRE, dl->m_fRadius * 0.1f, dl->m_fRadius * 0.99f);
      float fMinAtt = 1.0f / sAttenuation(fMinDist, dl->m_fRadius*0.9f);
      float fMaxAtt = 1.0f / sAttenuation(fMaxDist, dl->m_fRadius*0.9f);
      if(fabsf(fMinAtt - fMaxAtt) < 0.00001f)
      {
        fCA = fMinAtt;
        fLA = 0.0f;
      }
      else
      {
        fCA = max(0.01f, fMinAtt - (fMaxAtt - fMinAtt) / (fMaxDist - fMinDist) * fMinDist);
        fLA = max(0.0f, (fMinAtt - fCA) / fMinDist);
      }
      fQA = 0;
      fRange = (256.0f - fCA) / max(0.01f, fLA);
      if (fRange < 0)
        fRange = 0;
      cDiffuse = dl->m_Color;
    }
    /*static float sCA = 0;
    static float sLA = 0;
    static float sRange = 0;
    if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000)
      sCA -= 0.001f;
    if (GetAsyncKeyState(VK_NUMPAD3) & 0x8000)
      sCA += 0.001f;
    if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000)
      sLA -= 0.01f;
    if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000)
      sLA += 0.01f;
    if (GetAsyncKeyState(VK_NUMPAD7) & 0x8000)
      sRange -= 0.1f;
    if (GetAsyncKeyState(VK_NUMPAD9) & 0x8000)
      sRange += 0.1f;
    fCA += sCA;
    fLA += sLA;
    fRange += sRange;*/

    fCA = max(0.0f, fCA);
    fLA = max(0.0f, fLA);
    fQA = max(0.0f, fQA);
    Pos[0] = lpos.x;
    Pos[1] = lpos.y;
    Pos[2] = lpos.z;
    cDiffuse.a = 1.0f;
    if (!(Flags & LMF_NOSPECULAR))
      cSpecular = dl->m_SpecColor;
    if (bSpecularOnly)
      cDiffuse = Col_Black;
    EF_SetHWLight(n, Pos,
                 cDiffuse,
                 cSpecular,
                 fCA, fLA, fQA, fRange);
    n++;
  }
  m_RP.m_CurrentVLightFlags = Flags;

  return true;
}

// Initialize HW vertex lighting states for the fixed pipeline shader
void CD3D9Renderer::EF_LightMaterial(SLightMaterial *lm, int Flags)
{
  if (!(m_RP.m_pShader->m_Flags & EF_NEEDNORMALS))
    return;
  if (m_RP.m_ObjFlags & FOB_FOGPASS)
    return;

  //PROFILE_FRAME(State_LightStates);

  // Use fake lighting with TFactor
  if (!m_RP.m_NumActiveDLights && !(Flags & LMF_HASPSHADER))
  {
    EF_ConstantLightMaterial(lm, Flags);
    return;
  }

  if (!(Flags & LMF_IGNORELIGHTS) && EF_SetLights(Flags))
  {
    if (!(m_RP.m_CurrentVLights & 0xff) && !(Flags & LMF_HASPSHADER))
    {
      EF_ConstantLightMaterial(lm, Flags);
      return;
    }
    m_RP.m_FlagsPerFlush &= ~(RBSI_GLOBALRGB | RBSI_GLOBALALPHA);
    CFColor colAmb = EF_GetCurrentAmbient(lm, Flags);
    m_Material.Ambient.r = colAmb.r;
    m_Material.Ambient.g = colAmb.g;
    m_Material.Ambient.b = colAmb.b;
    m_Material.Ambient.a = colAmb.a;

    CFColor colDif = EF_GetCurrentDiffuse(lm, Flags);
    m_Material.Diffuse.r = colDif.r;
    m_Material.Diffuse.g = colDif.g;
    m_Material.Diffuse.b = colDif.b;
    m_Material.Diffuse.a = colAmb.a;

    // Disable emissive materials to match result to programmable pipeline
    // Pixel shaders don't support emmissive parameters
    //m_Material.Emissive.r = 0; //lm->Front.m_Emission.r;
    //m_Material.Emissive.g = 0; //lm->Front.m_Emission.g;
    //m_Material.Emissive.b = 0; //lm->Front.m_Emission.b;
    //m_Material.Emissive.a = 0; //lm->Front.m_Emission.a;

    if (!(Flags & LMF_NOSPECULAR))
    {
      m_Material.Specular.r = lm->Front.m_Specular.r;
      m_Material.Specular.g = lm->Front.m_Specular.g;
      m_Material.Specular.b = lm->Front.m_Specular.b;
      m_Material.Specular.a = lm->Front.m_Specular.a;

      m_Material.Power = lm->Front.m_SpecShininess;
      if (lm->Front.m_Specular == CFColor(0.0f))
        m_Material.Power = 0;
    }
    else
    {
      m_Material.Specular.r = 0;
      m_Material.Specular.g = 0;
      m_Material.Specular.b = 0;
      m_Material.Specular.a = 0;

      m_Material.Power = 0;
    }
  }
  m_pd3dDevice->SetMaterial(&m_Material); 
  m_RP.m_CurrentVLights |= 0x80000000;

  m_RP.m_TexStages[0].m_CA = eCA_Texture | (eCA_Diffuse<<3);
  m_RP.m_TexStages[0].m_AA = eCA_Texture | (eCA_Diffuse<<3);
}

//=============================================================
// Calculate matrices (usually texture matrices) for fixed pipeline shaders
// Set texture transform modes
void CD3D9Renderer::EF_ApplyMatrixOps(TArray<SMatrixTransform>* MatrixOps, bool bEnable)
{
  if (!MatrixOps)
    return;

  LPDIRECT3DDEVICE9 dv = m_pd3dDevice;
  int CurMatrix = D3DTS_VIEW;
  m_CurOpMatrix = NULL;
  int Stage = -1;
  bool bProjected;
  int nCoords = 2;
  int nCurMatrix;

  for (int i=0; i<MatrixOps->Num(); i++)
  {
    SMatrixTransform *mt = &MatrixOps->Get(i);
    nCurMatrix = mt->m_Matrix & 0xff;
    bProjected = (mt->m_Matrix & 0x8000) != 0;
    if (mt->m_Matrix >> 16)
      nCoords = (mt->m_Matrix >> 16);
    if (nCurMatrix != CurMatrix)
    {
      if (m_CurOpMatrix)
      {
        if (Stage >= 0)
          dv->SetTextureStageState( Stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
        dv->SetTransform((D3DTRANSFORMSTATETYPE)CurMatrix, m_CurOpMatrix);
        m_CurOpMatrix = NULL;
      }
      switch (nCurMatrix)
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
          dv->SetTransform((D3DTRANSFORMSTATETYPE)nCurMatrix, m_CurOpMatrix);
          if (Stage >= 0)
            dv->SetTextureStageState( Stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
        }
      }
      CurMatrix = nCurMatrix;
    }
    mt->mfSet(bEnable);
  }
  if (m_CurOpMatrix && bEnable)
  {
    if (Stage >= 0)
    {
      if (bProjected)
        dv->SetTextureStageState( Stage, D3DTSS_TEXTURETRANSFORMFLAGS, nCoords | D3DTTFF_PROJECTED);
      else
        dv->SetTextureStageState( Stage, D3DTSS_TEXTURETRANSFORMFLAGS, nCoords);
    }
    if (nCoords < 3)
    {
      m_CurOpMatrix->_31 = m_CurOpMatrix->_41;
      m_CurOpMatrix->_32 = m_CurOpMatrix->_42;
      m_CurOpMatrix->_33 = m_CurOpMatrix->_43;
      m_CurOpMatrix->_34 = m_CurOpMatrix->_44;
    }
    //D3DXMatrixTranspose(m_CurOpMatrix, m_CurOpMatrix);
    dv->SetTransform((D3DTRANSFORMSTATETYPE)nCurMatrix, m_CurOpMatrix);
  }
}

#include "../Common/NvTriStrip/NVTriStrip.h"

// Commit changed states to the hardware before drawing
bool CD3D9Renderer::EF_PreDraw(SShaderPass *sl, bool bSetVertexDecl)
{
  bool bRet = true;

  //PROFILE_FRAME(Draw_Predraw);

  EF_CommitShadersState();
  EF_CommitVLightsState();

  HRESULT hr;
  if (bSetVertexDecl)
  {
    hr = EF_SetVertexDeclaration(m_RP.m_FlagsModificators&7, m_RP.m_CurVFormat);
    if (hr != S_OK)
      return false;
  }

  if (!m_RP.m_pRE && m_RP.m_RendNumVerts && m_RP.m_RendNumIndices)
  {
    // Mergable geometry (no single render element)
    int nStart;
    int nSize = m_RP.m_Stride*m_RP.m_RendNumVerts;
    if (!(m_RP.m_FlagsPerFlush & RBSI_VERTSMERGED))
    {
      m_RP.m_FlagsPerFlush |= RBSI_VERTSMERGED;
      int nFormat = m_RP.m_CurVFormat;
      int nCurVB = m_RP.m_CurVB;
      if (m_RP.m_VBs[nCurVB].VBPtr_0->GetBytesOffset()+nSize >= (int)m_RP.m_VBs[nCurVB].VBPtr_0->GetBytesCount())
      {
        m_RP.m_VBs[nCurVB].VBPtr_0->Reset();
        m_RP.m_CurVB = (nCurVB + 1) & (MAX_DYNVBS-1);
      }
      nCurVB = m_RP.m_CurVB;
      Vec3 *pVB = m_RP.m_VBs[nCurVB].VBPtr_0->Lock(nSize, nStart);
      cryMemcpy(pVB, m_RP.m_Ptr.Ptr, nSize);
      m_RP.m_VBs[nCurVB].VBPtr_0->Unlock();
      m_RP.m_FirstVertex = 0;
      m_RP.m_MergedStreams[0] = m_RP.m_VBs[nCurVB];
      m_RP.m_nStreamOffset[0] = nStart;

      ushort *pIB = m_RP.m_IndexBuf->Lock(m_RP.m_RendNumIndices, nStart);
      cryMemcpy(pIB, m_RP.m_SysRendIndices, m_RP.m_RendNumIndices*sizeof(short));
      m_RP.m_IndexBuf->Unlock();
      m_RP.m_FirstIndex = nStart;
    }
    m_RP.m_MergedStreams[0].VBPtr_0->Bind(m_pd3dDevice, 0, m_RP.m_nStreamOffset[0], m_RP.m_Stride);
    m_RP.m_IndexBuf->Bind(m_pd3dDevice);

    if (m_RP.m_FlagsModificators & FHF_TANGENTS)
    {
      if (!(m_RP.m_FlagsPerFlush & RBSI_TANGSMERGED))
      {
        m_RP.m_FlagsPerFlush |= RBSI_TANGSMERGED;
        int i;
        int nCurVB = m_RP.m_CurVB;
        int nSize = m_RP.m_RendNumVerts*sizeof(SPipTangents);
        if (m_RP.m_VBs[nCurVB].VBPtr_0->GetBytesOffset()+nSize >= (int)m_RP.m_VBs[nCurVB].VBPtr_0->GetBytesCount())
        {
          m_RP.m_VBs[nCurVB].VBPtr_0->Reset();
          m_RP.m_CurVB = (nCurVB + 1) & (MAX_DYNVBS-1);
        }
        nCurVB = m_RP.m_CurVB;
        SPipTangents *dst = (SPipTangents *)m_RP.m_VBs[nCurVB].VBPtr_0->Lock(nSize, nStart);
        for (i=0; i<m_RP.m_MergedREs.Num(); i++)
        {
          CREOcLeaf *re = m_RP.m_MergedREs[i];
          CCObject *obj = m_RP.m_MergedObjs[i];
          CLeafBuffer *lb = re->m_pBuffer;
          CMatInfo *mi = re->m_pChunk;
          SPipTangents *src = (SPipTangents *)lb->m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
          src += mi->nFirstVertId;
          int nVerts = mi->nNumVerts;
          if (obj->m_ObjFlags & FOB_TRANS_ROTATE)
          {
            Matrix44 *mat = &obj->m_Matrix;
#ifdef DO_ASM
            _asm
            {
              mov ecx,    nVerts;
              mov eax,    mat
              mov esi,    src
              mov edi,    dst
              movaps      xmm2,xmmword ptr [eax]
              movaps      xmm4,xmmword ptr [eax+10h]
              movaps      xmm6,xmmword ptr [eax+20h]
align 16
_Loop:
              movlps      xmm1,qword ptr [esi]
              movss       xmm0,dword ptr [esi+8]
              shufps      xmm0,xmm0,0
              prefetcht0  [esi+10h] 
              mulps       xmm0,xmm6
              movaps      xmm3,xmm1
              shufps      xmm3,xmm1,55h
              mulps       xmm3,xmm4
              shufps      xmm1,xmm1,0
              mulps       xmm1,xmm2
              addps       xmm3,xmm1
              addps       xmm3,xmm0
              movaps      xmm1,xmm3     // r1 = vx, vy, vz, X
              mulps		    xmm1,xmm3			// r1 = vx * vx, vy * vy, vz * vz, X
              movhlps		  xmm5,xmm1			// r5 = vz * vz, X, X, X
              movaps		  xmm0,xmm1			// r0 = r1
              shufps	  	xmm0,xmm0, 1	// r0 = vy * vy, X, X, X
              addss	      xmm1,xmm0			// r0 = (vx * vx) + (vy * vy), X, X, X
              addss	      xmm1,xmm5			// r1 = (vx * vx) + (vy * vy) + (vz * vz), X, X, X
              sqrtss	    xmm1,xmm1			// r1 = sqrt((vx * vx) + (vy * vy) + (vz * vz)), X, X, X
              rcpss		    xmm1,xmm1			// r1 = 1/radius, X, X, X
              shufps		  xmm1,xmm1, 0	// r1 = 1/radius, 1/radius, 1/radius, X
              mulps		    xmm3,xmm1			// r3 = vx * 1/radius, vy * 1/radius, vz * 1/radius, X
              movhlps     xmm5,xmm3
              movlps      qword ptr [edi],xmm3
              movss       xmm0,dword ptr [esi+20]
              movss       dword ptr [edi+8],xmm5

              movlps      xmm1,qword ptr [esi+12]
              shufps      xmm0,xmm0,0
              mulps       xmm0,xmm6
              prefetcht0  [esi+20h] 
              movaps      xmm3,xmm1
              shufps      xmm3,xmm1,55h
              mulps       xmm3,xmm4
              shufps      xmm1,xmm1,0
              mulps       xmm1,xmm2
              addps       xmm3,xmm1
              addps       xmm3,xmm0
              movhlps     xmm5,xmm3
              movaps      xmm1,xmm3     // r1 = vx, vy, vz, X
              mulps		    xmm1,xmm3			// r1 = vx * vx, vy * vy, vz * vz, X
              movhlps		  xmm5,xmm1			// r5 = vz * vz, X, X, X
              movaps		  xmm0,xmm1			// r0 = r1
              shufps	  	xmm0,xmm0, 1	// r0 = vy * vy, X, X, X
              addss	      xmm1,xmm0			// r0 = (vx * vx) + (vy * vy), X, X, X
              addss	      xmm1,xmm5			// r1 = (vx * vx) + (vy * vy) + (vz * vz), X, X, X
              sqrtss	    xmm1,xmm1			// r1 = sqrt((vx * vx) + (vy * vy) + (vz * vz)), X, X, X
              rcpss		    xmm1,xmm1			// r1 = 1/radius, X, X, X
              shufps		  xmm1,xmm1, 0	// r1 = 1/radius, 1/radius, 1/radius, X
              mulps		    xmm3,xmm1			// r3 = vx * 1/radius, vy * 1/radius, vz * 1/radius, X
              movhlps     xmm5,xmm3
              movlps      qword ptr [edi+12],xmm3
              movss       xmm0,dword ptr [esi+32]
              movss       dword ptr [edi+20],xmm5

              movlps      xmm1,qword ptr [esi+24]
              shufps      xmm0,xmm0,0
              prefetcht0  [esi+30h] 
              mulps       xmm0,xmm6
              movaps      xmm3,xmm1
              shufps      xmm3,xmm1,55h
              mulps       xmm3,xmm4
              shufps      xmm1,xmm1,0
              mulps       xmm1,xmm2
              add         edi, 36
              addps       xmm3,xmm1
              addps       xmm3,xmm0
              movaps      xmm1,xmm3     // r1 = vx, vy, vz, X
              mulps		    xmm1,xmm3			// r1 = vx * vx, vy * vy, vz * vz, X
              add         esi, 36
              movhlps		  xmm5,xmm1			// r5 = vz * vz, X, X, X
              movaps		  xmm0,xmm1			// r0 = r1
              shufps	  	xmm0,xmm0, 1	// r0 = vy * vy, X, X, X
              addss	      xmm1,xmm0			// r0 = (vx * vx) + (vy * vy), X, X, X
              addss	      xmm1,xmm5			// r1 = (vx * vx) + (vy * vy) + (vz * vz), X, X, X
              sqrtss	    xmm1,xmm1			// r1 = sqrt((vx * vx) + (vy * vy) + (vz * vz)), X, X, X
              rcpss		    xmm1,xmm1			// r1 = 1/radius, X, X, X
              shufps		  xmm1,xmm1, 0	// r1 = 1/radius, 1/radius, 1/radius, X
              mulps		    xmm3,xmm1			// r3 = vx * 1/radius, vy * 1/radius, vz * 1/radius, X
              dec         ecx
              movhlps     xmm5,xmm3
              movlps      qword ptr [edi+24-36],xmm3
              movss       dword ptr [edi+32-36],xmm5

              jne         _Loop
              mov         dst, edi
            }
#else
            for (int j=0; j<mi->nNumVerts; j++)
            {
              dst->m_Tangent = mat->TransformVectorOLD(src->m_Tangent);
              dst->m_Binormal = mat->TransformVectorOLD(src->m_Binormal);
              dst->m_TNormal = mat->TransformVectorOLD(src->m_TNormal);
              src++;
              dst++;
            }
#endif
          }
          else
          {
            for (i=0; i<mi->nNumVerts; i++)
            {
              *dst = *src;
              src++;
              dst++;
            }
          }
        }
        m_RP.m_VBs[nCurVB].VBPtr_0->Unlock();
        m_RP.m_MergedStreams[1] = m_RP.m_VBs[nCurVB];
        m_RP.m_nStreamOffset[1] = nStart;
      }
      m_RP.m_MergedStreams[1].VBPtr_0->Bind(m_pd3dDevice, 1, m_RP.m_nStreamOffset[1], sizeof(SPipTangents));
      m_RP.m_PersFlags |= RBPF_USESTREAM1;
    }
    else
    if (m_RP.m_PersFlags & RBPF_USESTREAM1)
    {
      m_RP.m_PersFlags &= ~RBPF_USESTREAM1;
      m_pd3dDevice->SetStreamSource(1, NULL, 0, 0);
    }
    if (m_RP.m_FlagsModificators & RBMF_LMTCUSED)
    {
      if (!(m_RP.m_FlagsPerFlush & RBSI_LMTCMERGED))
      {
        m_RP.m_FlagsPerFlush |= RBSI_LMTCMERGED;
        nSize = sizeof(struct_VERTEX_FORMAT_TEX2F)*m_RP.m_RendNumVerts;
        int nCurVB = m_RP.m_CurVB;
        if (m_RP.m_VBs[nCurVB].VBPtr_0->GetBytesOffset()+nSize >= (int)m_RP.m_VBs[nCurVB].VBPtr_0->GetBytesCount())
        {
          m_RP.m_VBs[nCurVB].VBPtr_0->Reset();
          m_RP.m_CurVB = (nCurVB + 1) & (MAX_DYNVBS-1);
        }
        nCurVB = m_RP.m_CurVB;
        struct_VERTEX_FORMAT_TEX2F *dst = (struct_VERTEX_FORMAT_TEX2F *)m_RP.m_VBs[nCurVB].VBPtr_0->Lock(nSize, nStart);
        for (int i=0; i<m_RP.m_MergedREs.Num(); i++)
        {
          CREOcLeaf *re = m_RP.m_MergedREs[i];
          CCObject *pObj = m_RP.m_MergedObjs[i];
          int nNumVerts = re->m_pChunk->nNumVerts;
          if (!pObj->m_pLMTCBufferO)
          {
            dst += nNumVerts;
            continue;
          }
          struct_VERTEX_FORMAT_TEX2F *src = (struct_VERTEX_FORMAT_TEX2F *)pObj->m_pLMTCBufferO->m_pSecVertBuffer->m_VS[0].m_VData;
          src += re->m_pChunk->nFirstVertId;
          for (int n=0; n<nNumVerts; n++)
          {
            *dst++ = *src++;
          }
        }
        m_RP.m_VBs[nCurVB].VBPtr_0->Unlock();
        m_RP.m_MergedStreams[2] = m_RP.m_VBs[nCurVB];
        m_RP.m_nStreamOffset[2] = nStart;
      }
      m_RP.m_MergedStreams[2].VBPtr_0->Bind(m_pd3dDevice, 2, m_RP.m_nStreamOffset[2], sizeof(SMRendTexVert));
      m_RP.m_PersFlags |= RBPF_USESTREAM2;
    }
    else
    if (m_RP.m_PersFlags & RBPF_USESTREAM2)
    {
      m_RP.m_PersFlags &= ~RBPF_USESTREAM2;
      m_pd3dDevice->SetStreamSource(2, NULL, 0, 0);
    }
  }
  else
  if (m_RP.m_pRE)
    bRet = m_RP.m_pRE->mfPreDraw(sl);

  return bRet;
}

// Draw current indexed mesh
void CD3D9Renderer::EF_DrawIndexedMesh (int nPrimType)
{
  HRESULT h = 0;

  if (CV_r_nodrawshaders)
    return;

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
            m_RP.m_PS.m_NumDrawCalls++;
            if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, m_RP.m_BaseVertex, m->nFirstVertId, m->nNumVerts, m->nFirstIndexId, m->nNumIndices - 2)))
            {
              Error("CD3D9Renderer::EF_DrawIndexedMesh: DrawIndexedPrimitive error", h);
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
            m_RP.m_PS.m_NumDrawCalls++;
            switch (g->type)
            {
              case PT_STRIP:
                if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, m_RP.m_BaseVertex, m_RP.m_FirstVertex, mi->nNumVerts, g->offsIndex+offs, g->numIndices - 2)))
                {
                  Error("CD3D9Renderer::EF_DrawIndexedMesh: DrawIndexedPrimitive error", h);
                  return;
                }
                m_nPolygons += (g->numIndices - 2);
                break;

              case PT_LIST:
                if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_RP.m_BaseVertex, m_RP.m_FirstVertex, mi->nNumVerts, g->offsIndex+offs, g->numIndices / 3)))
                {
                  Error("CD3D9Renderer::EF_DrawIndexedMesh: DrawIndexedPrimitive error", h);
                  return;
                }
                m_nPolygons += (g->numIndices / 3);
                break;

              case PT_FAN:
                if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, m_RP.m_BaseVertex, m_RP.m_FirstVertex, mi->nNumVerts, g->offsIndex+offs, g->numIndices - 2)))
                {
                  Error("CD3D9Renderer::EF_DrawIndexedMesh: DrawIndexedPrimitive error", h);
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

  if (nFaces)
  {
    m_RP.m_PS.m_NumDrawCalls++;
    //m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    if (FAILED(h=m_pd3dDevice->DrawIndexedPrimitive(nType, m_RP.m_BaseVertex, m_RP.m_FirstVertex, m_RP.m_RendNumVerts, m_RP.m_FirstIndex, nFaces)))
    {
      Error("CD3D9Renderer::EF_DrawIndexedMesh: DrawIndexedPrimitive error", h);
      return;
    }
    m_nPolygons += nFaces;
  }
  else
  {
    int nnn = 0;
  }
}

// Draw volumetric fog passes (used in FP shaders and in programmable pipeline shaders)
void CD3D9Renderer::EF_DrawFogOverlayPasses()
{
  // Usually it means first pass in indoor engine (before shadow pass)
  if (m_RP.m_ObjFlags & FOB_ZPASS)
    return;
  if (m_RP.m_FlagsPerFlush & RBSI_FOGVOLUME)
    return;

#ifdef DO_RENDERLOG
  if (CRenderer::CV_r_log >= 3)
    Logv(SRendItem::m_RecurseLevel, "--- Fog Pass ---\n");
#endif

  float fWatLevel = iSystem->GetI3DEngine()->GetWaterLevel();
  SMFog *fb = m_RP.m_pFogVolume;
  SShader *sh = NULL;
  bool bVFFP = false;
  if (!(m_Features & RFT_HW_VS) || CV_r_Quality_BumpMapping == 0)
    bVFFP = true;
  if (!bVFFP || (m_RP.m_pShader->m_Flags & EF_HASVSHADER))
  {
    if (m_RP.m_pShader->m_eSort != eS_Water && fabs(fb->m_Dist-fWatLevel) < 0.1f && SRendItem::m_RecurseLevel <= 1)
    {
      if (m_RP.m_pRE)
      {
        float fDist;
#ifndef PIPE_USE_INSTANCING
        fDist = m_RP.m_pRE->mfMinDistanceToCamera(m_RP.m_pCurObject);
#else
        CCObject *pObj = m_RP.m_pCurObject;
        int nObj = 0;
        fDist = 999999.0f;
        while (true)
        {
          fDist = min(fDist, m_RP.m_pRE->mfMinDistanceToCamera(pObj));
          nObj++;
          if (nObj >= m_RP.m_MergedObjects.Num())
            break;
          pObj = gRenDev->m_RP.m_MergedObjects[nObj];
        }
#endif
        if (fDist < 40.0f && fb->bCaustics)
          sh = m_cEF.m_ShaderFogCaust;
        else
          sh = m_cEF.m_ShaderFog;
      }
      else
      if (fb->bCaustics)
        sh = m_cEF.m_ShaderFogCaust;
      else
        sh = m_cEF.m_ShaderFog;
    }
    else
      sh = m_cEF.m_ShaderFog;
  }
  else
    sh = m_cEF.m_ShaderFog_FP;

  if (sh && sh->m_HWTechniques.Num())
  {
    if (m_RP.m_pShader->m_eSort == eS_Water && m_RP.m_pFogVolume)
      m_RP.m_pFogVolume->m_Dist += 0.1f;
    EF_DrawGeneralPasses(sh->m_HWTechniques[0], sh, true, 0, sh->m_HWTechniques[0]->m_Passes.Num()-1, false);
    if (m_RP.m_pShader->m_eSort == eS_Water && m_RP.m_pFogVolume)
      m_RP.m_pFogVolume->m_Dist -= 0.1f;
  }
}

// Draw detail textures passes (used in FP shaders and in programmable pipeline shaders)
void CD3D9Renderer::EF_DrawDetailOverlayPasses()
{
  // Usually it means first pass in indoor engine (before shadow pass)
  if (m_RP.m_ObjFlags & FOB_ZPASS)
    return;

  if (!m_RP.m_pShaderResources || !m_RP.m_pShaderResources->m_Textures[EFTT_DETAIL_OVERLAY])
    return;

  SEfResTexture *rt = m_RP.m_pShaderResources->m_Textures[EFTT_DETAIL_OVERLAY];
  SShader *sh = m_RP.m_pShader;
  int i;
  SMFog fg;

  float fDistToCam = 500.0f;
  float fDist = CV_r_detaildistance;
  static TArray<float> sfDistRender;
  if (m_RP.m_pRE)
  {
#ifndef PIPE_USE_INSTANCING
    fDistToCam = m_RP.m_pRE->mfMinDistanceToCamera(m_RP.m_pCurObject);
    if (fDistToCam > fDist+1.0f)
      return;
#else
    CCObject *pObj = gRenDev->m_RP.m_pCurObject;
    int nObj = 0;
    float fDistToCam = 999999.0f;
    sfDistRender.SetUse(0);
    while (true)
    {
      float fDistObj = m_RP.m_pRE->mfMinDistanceToCamera(pObj);
      if (fDistObj <= fDist+1.0f)
        sfDistRender.AddElem(fDistObj);
      else
        sfDistRender.AddElem(-1.0f);
      fDistToCam = min(fDistToCam, fDistObj);
      nObj++;
      if (nObj >= gRenDev->m_RP.m_MergedObjects.Num())
        break;
      pObj = gRenDev->m_RP.m_MergedObjects[nObj];
    }
    if (fDistToCam > fDist+1.0f)
      return;
    EF_PushMatrix();
#endif
  }
  else
    return;

  PROFILE_FRAME(DrawShader_DetailPasses);

  SMFog *fb = gRenDev->m_RP.m_pFogVolume;
  fg.m_FogInfo.m_WaveFogGen.m_eWFType = eWF_None;
  gRenDev->m_RP.m_pFogVolume = &fg;

  CD3D9TexMan::BindNULL(2);
  EF_SelectTMU(1);
  gRenDev->m_TexMan->m_Text_Fog->Set();
  EF_SelectTMU(0);
  rt->m_TU.m_TexPic->Set();
  m_RP.m_CurrentVLights = 0;

  if (m_RP.m_FlagsPerFlush & RBSI_WASDEPTHWRITE)
    EF_SetState(GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL | GS_DEPTHFUNC_EQUAL);
  else
    EF_SetState(GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL);

  SParamComp_FogMatrix FM;
  vec4_t Vals;
  m_RP.m_FlagsModificators &= ~(7 | (RBMF_TCM | RBMF_TCG));
  if (m_RP.m_TexStages[0].TCIndex != 0)
  {
    m_RP.m_TexStages[0].TCIndex = 0;
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0);
  }
  EF_Scissor(false, 0, 0, 0, 0);

  if (!(sh->m_Flags & EF_HASVSHADER))
  {
    m_RP.m_PersFlags &= ~RBPF_VSNEEDSET;

    int n = CLAMP(CV_r_detailnumlayers, 1, 4);
    float fUScale = rt->m_TexModificator.m_Tiling[0];
    float fVScale = rt->m_TexModificator.m_Tiling[1];
    if (!fUScale)
      fUScale = CV_r_detailscale;
    if (!fVScale)
      fVScale = CV_r_detailscale;

    m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | 1);

    D3DXMATRIX mat, ma, *mi;
    for (i=0; i<n; i++)
    {
      fg.m_fMaxDist = fDist;

      D3DXMatrixIdentity(&mat);
      D3DXMatrixScaling(&ma, fUScale, fVScale, 1.0f);
      m_pd3dDevice->SetTransform(D3DTS_TEXTURE0, &ma);

#ifndef PIPE_USE_INSTANCING
      mi = EF_InverseMatrix();
      FM.m_Offs = 0;
      FM.mfGet4f(Vals);
      mat.m[0][0] = Vals[0];
      mat.m[1][0] = Vals[1];
      mat.m[2][0] = Vals[2];
      mat.m[3][0] = Vals[3];

      FM.m_Offs = 1;
      FM.mfGet4f(Vals);
      mat.m[0][1] = Vals[0];
      mat.m[1][1] = Vals[1];
      mat.m[2][1] = Vals[2];
      mat.m[3][1] = Vals[3];

      D3DXMatrixMultiply(&ma, mi, &mat);
      m_pd3dDevice->SetTransform(D3DTS_TEXTURE1, &ma );

      if (!m_RP.m_RCDetail)
        m_RP.m_RCDetail = CPShader::mfForName("CGRCDetailAtten", true);
      if (m_RP.m_RCDetail)
        m_RP.m_RCDetail->mfSet(true);

#ifdef DO_RENDERLOG
      if (CRenderer::CV_r_log >= 3)
        Logv(SRendItem::m_RecurseLevel, "+++ Detail Pass %d [Dist: %.3f]\n", m_RP.m_RendPass, fDist);
#endif

      // Draw primitives
      EF_Draw(sh, NULL);
#else
      int bFogOverrided = 0;
      EF_PreDraw(NULL);
      if (m_FS.m_bEnable)
        bFogOverrided = EF_FogCorrection(false, false);

      if (m_Features & (RFT_HW_RC | RFT_HW_PS20))
      {
        if (!m_RP.m_RCDetail)
          m_RP.m_RCDetail = CPShader::mfForName("CGRCDetailAtten");
        if (m_RP.m_RCDetail)
          m_RP.m_RCDetail->mfSet(true);
      }
      else
      {
        UCol col;
        col.dcolor = 0xff808080;
        if (m_RP.m_CurGlobalColor.dcolor != col.dcolor)
        {
          m_RP.m_CurGlobalColor = col;
          m_pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, m_RP.m_CurGlobalColor.dcolor);
        }
        EF_SelectTMU(0);
        EF_SetColorOp(eCO_REPLACE, eCO_REPLACE, DEF_TEXARG0, DEF_TEXARG0);
        EF_SelectTMU(1);
        EF_SetColorOp(eCO_BLENDTEXTUREALPHA, eCO_REPLACE, eCA_Constant|(eCA_Previous<<3), DEF_TEXARG0);
      }

      CCObject *pSaveObj = m_RP.m_pCurObject;
      CCObject *pObj = pSaveObj;
      int nObj = 0;
      while (true)
      {      
        if (sfDistRender[nObj] > 0)
        {
          if (nObj)
          {
            m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
            m_RP.m_FrameObject++;
            pObj = m_RP.m_pCurObject;
            EF_SetObjectTransform(pObj, sh, pObj->m_ObjFlags);
          }

          mi = EF_InverseMatrix();
          FM.m_Offs = 0;
          FM.mfGet4f(Vals);
          mat.m[0][0] = Vals[0];
          mat.m[1][0] = Vals[1];
          mat.m[2][0] = Vals[2];
          mat.m[3][0] = Vals[3];

          FM.m_Offs = 1;
          FM.mfGet4f(Vals);
          mat.m[0][1] = Vals[0];
          mat.m[1][1] = Vals[1];
          mat.m[2][1] = Vals[2];
          mat.m[3][1] = Vals[3];


          D3DXMatrixMultiply(&ma, mi, &mat);
          m_pd3dDevice->SetTransform(D3DTS_TEXTURE1, &ma );

#ifdef DO_RENDERLOG
          if (CRenderer::CV_r_log >= 3)
          {
            Vec3d vPos = pObj->GetTranslation();
            Logv(SRendItem::m_RecurseLevel, "+++ Detail Pass %d [Dist: %.3f] (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, fDist, pObj->m_VisId, vPos[0], vPos[1], vPos[2]);
          }
#endif

          {
            //PROFILE_FRAME(Draw_ShaderIndexMesh);
            if (m_RP.m_pRE)
              m_RP.m_pRE->mfDraw(sh, NULL);
            else
              EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
          }
        }
        nObj++;
        if (nObj >= m_RP.m_MergedObjects.Num())
          break;
      }
      EF_FogRestore(bFogOverrided);
      if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
        EF_CommitTexTransforms(false);
      else
        m_RP.m_FlagsModificators &= ~(RBMF_TCM | RBMF_TCG);
      if (pSaveObj != m_RP.m_pCurObject)
      {
        m_RP.m_pCurObject = pSaveObj;
        m_RP.m_FrameObject++;
        EF_SetObjectTransform(pSaveObj, sh, pSaveObj->m_ObjFlags);
      }
#endif
      fDist /= 2.0f;
      if (fDistToCam > fDist+1.0f)
        break;
      fUScale *= 2.0f;
      fVScale *= 2.0f;
    }

    D3DXMatrixIdentity(&mat);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    m_pd3dDevice->SetTransform(D3DTS_TEXTURE0, &mat);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    m_pd3dDevice->SetTransform(D3DTS_TEXTURE1, &mat);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1);
    m_RP.m_TexStages[1].TCIndex = 1;

    EF_SelectTMU(0);
  }
  else
  {
    if (!m_RP.m_VPDetail)
      m_RP.m_VPDetail = CVProgram::mfForName("CGVProgDetail");
    if (!m_RP.m_RCDetail)
      m_RP.m_RCDetail = CPShader::mfForName("CGRCDetailAtten");
    if (m_RP.m_RCDetail)
      m_RP.m_RCDetail->mfSet(true);

    CCGVProgram_D3D *vpD3D = (CCGVProgram_D3D *)m_RP.m_VPDetail;

    vpD3D->mfSet(true, NULL, VPF_DONTSETMATRICES);
    vpD3D->mfSetVariables(false, NULL);
    vpD3D->mfSetStateMatrices();
    SCGBind *pBindScale = vpD3D->mfGetParameterBind("DetailScaling");
    SCGBind *pBindTG0 = vpD3D->mfGetParameterBind("TexGen00");
    SCGBind *pBindTG1 = vpD3D->mfGetParameterBind("TexGen01");

    int n = CLAMP(CV_r_detailnumlayers, 1, 4);
    float fUScale = rt->m_TexModificator.m_Tiling[0];
    float fVScale = rt->m_TexModificator.m_Tiling[1];
    if (!fUScale)
      fUScale = CV_r_detailscale;
    if (!fVScale)
      fVScale = CV_r_detailscale;
    for (i=0; i<n; i++)
    {
      fg.m_fMaxDist = fDist;

      Vals[0] = fUScale;
      Vals[1] = fVScale;
      Vals[2] = 0;
      Vals[3] = 0;
      if (pBindScale)
        vpD3D->mfParameter4f(pBindScale, Vals);

      Plane plane;

#ifndef PIPE_USE_INSTANCING
      FM.m_Offs = 0;
      FM.mfGet4f(Vals);
      if (pBindTG0)
        vpD3D->mfParameter4f(pBindTG0, Vals);

      FM.m_Offs = 1;
      FM.mfGet4f(Vals);
      if (pBindTG1)
        vpD3D->mfParameter4f(pBindTG1, Vals);

#ifdef DO_RENDERLOG
      if (CRenderer::CV_r_log >= 3)
        Logv(SRendItem::m_RecurseLevel, "+++ Detail Pass %d [Dist: %.3f]\n", m_RP.m_RendPass, fDist);
#endif

      // Draw primitives
      EF_Draw(sh, NULL);
#else
      int bFogOverrided = 0;
      EF_PreDraw(NULL);
      if (m_FS.m_bEnable)
        bFogOverrided = EF_FogCorrection(false, false);

      CCObject *pSaveObj = m_RP.m_pCurObject;
      CCObject *pObj = pSaveObj;
      int nObj = 0;
      while (true)
      {       
        if (sfDistRender[nObj])
        {
          if (nObj)
          {
            m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
            m_RP.m_FrameObject++;
            pObj = m_RP.m_pCurObject;
          }

          FM.m_Offs = 0;
          FM.mfGet4f(Vals);
          if (pBindTG0)
            vpD3D->mfParameter4f(pBindTG0, Vals);

          FM.m_Offs = 1;
          FM.mfGet4f(Vals);
          if (pBindTG1)
            vpD3D->mfParameter4f(pBindTG1, Vals);

          vpD3D->mfSetVariables(true, NULL);
          vpD3D->mfSetStateMatrices();

#ifdef DO_RENDERLOG
          if (CRenderer::CV_r_log >= 3)
          {
            Vec3d vPos = pObj->GetTranslation();
            Logv(SRendItem::m_RecurseLevel, "+++ Detail Pass %d [Dist: %.3f] (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, fDist, pObj->m_VisId, vPos[0], vPos[1], vPos[2]);
          }
#endif

          {
            //PROFILE_FRAME(Draw_ShaderIndexMesh);
            if (m_RP.m_pRE)
              m_RP.m_pRE->mfDraw(sh, NULL);
            else
              EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
          }
        }
        nObj++;
        if (nObj >= m_RP.m_MergedObjects.Num())
          break;
      }
      EF_FogRestore(bFogOverrided);
      if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
        EF_CommitTexTransforms(false);
      else
        m_RP.m_FlagsModificators &= ~(RBMF_TCM | RBMF_TCG);
      if (pSaveObj != m_RP.m_pCurObject)
      {
        m_RP.m_pCurObject = pSaveObj;
        m_RP.m_FrameObject++;
      }
#endif

      fDist /= 2.0f;
      if (fDistToCam > fDist+1.0f)
        break;
      fUScale *= 2.0f;
      fVScale *= 2.0f;
    }
  }
  CD3D9TexMan::BindNULL(0);
  gRenDev->m_RP.m_pFogVolume = fb;
#ifdef PIPE_USE_INSTANCING
  EF_PopMatrix();
#endif
}

// Draw fur layer passes (used in programmable pipeline shaders only)
void CD3D9Renderer::EF_DrawFurPasses(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd, EShaderPassType eShPass)
{
  SShaderPassHW *slw;
  int i;

  CVProgram *curVP = NULL;
  CVProgram *newVP;

  m_RP.m_FlagsPerFlush |= RBSI_FURPASS;

  PROFILE_FRAME(DrawShader_FurPasses);

  slw = &hs->m_Passes[nStart];
  if (slw->m_TUnits.Num() < 3)
    return;
  CFurMap *fm = (CFurMap *)slw->m_TUnits[2].m_TexPic->m_pFuncMap;
  CFurNormalMap *fnm = (CFurNormalMap *)slw->m_TUnits[0].m_TexPic->m_pFuncMap;
  if (!fm || !fnm)
    return;

  SParamComp_User user;
  user.m_Name = "furlength";
  float fFurLength = user.mfGet();
  fFurLength = CLAMP(fFurLength, 0.001f, 4.0f);

  user.m_Name = "furnumlayers";
  int nLayers = (int)user.mfGet();
  nLayers = CLAMP(nLayers, 1, 60);
  fm->Update(nLayers);

  bool bFurSimulation = (eShPass == eSHP_SimulatedFur);

  fnm->Update(eShPass, iTimer->GetFrameTime(), slw, bFurSimulation);

  m_RP.m_CurrPass = slw;
  m_RP.m_FlagsModificators = (m_RP.m_FlagsModificators & ~7) | (slw->m_Flags & 3);

  EF_SetVertexStreams(slw->m_Pointers, 1);

  // Set all textures and HW TexGen modes for the current pass (ShadeLayer)
  newVP = slw->m_VProgram;
  if (newVP)
    m_RP.m_FlagsPerFlush |= RBSI_USEVP;
  if (slw->mfSetTextures())
  {
    fnm->Bind(0);
    
    // Set vertex program for the current pass if needed
    if (newVP != curVP)
    {
      if (newVP)
      {
        curVP = newVP;
        curVP->mfSet(true, slw, VPF_DONTSETMATRICES);
      }
      else
        curVP = NULL;
    }

    EF_ApplyMatrixOps(slw->m_MatrixOps, true);

    if (curVP)
    {
#ifndef PIPE_USE_INSTANCING
      curVP->mfSetStateMatrices();
#endif
      curVP->mfSetVariables(false, &slw->m_VPParamsNoObj);
    }
    else
    {
      m_RP.m_CurrentVLights = 0;
      m_RP.m_PersFlags &= ~RBPF_VSNEEDSET;
    }

    // Set Pixel shaders and Register combiners for the current pass
    if (slw->m_FShader)
      slw->m_FShader->mfSet(true, slw);
    else
      m_RP.m_PersFlags &= ~RBPF_PS1NEEDSET;

    if (m_RP.m_RendPass || (m_RP.m_ObjFlags & FOB_LIGHTPASS))
      EF_SetState(slw->m_SecondRenderState);
    else
      EF_SetState(slw->m_RenderState);
#ifndef PIPE_USE_INSTANCING

#ifdef DO_RENDERLOG
    if (CRenderer::CV_r_log >= 3)
      Logv(SRendItem::m_RecurseLevel, "+++ Shadow Pass %d [%d Samples]\n", m_RP.m_RendPass, nDeltaCasters);
#endif

    EF_Scissor(false, 0, 0, 0, 0);

    EF_Draw(ef, slw);

    m_RP.m_RendPass++;

#else //PIPE_USE_INSTANCING
    m_RP.m_RendPass++;

    int bFogOverrided = 0;
    // Unlock all VB (if needed) and set current streams
    EF_PreDraw(slw);
    if (m_FS.m_bEnable)
      bFogOverrided = EF_FogCorrection(false, false);

    int nObj = 0;
    CCObject *pSaveObj = m_RP.m_pCurObject;
    CCObject *pObj = pSaveObj;
    while (true)
    {       
      if (nObj)
      {
        m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
        m_RP.m_FrameObject++;
        pObj = m_RP.m_pCurObject;

        if (!curVP)
        {
          EF_SetObjectTransform(pObj, ef, pObj->m_ObjFlags);
          m_RP.m_CurrentVLights = 0;

          if (m_RP.m_FrameGTC == m_RP.m_Frame)
          {
            for (int nt=0; nt<gRenDev->m_TexMan->m_nCurStages; nt++)
            {
              if (m_RP.m_pGTC[nt])
              {
                EF_SelectTMU(nt);
                m_RP.m_pGTC[nt]->mfSet(true);
              }
            }
          }
        }
      }

      EF_Scissor(false, 0, 0, 0, 0);

#ifdef DO_RENDERLOG
      if (CRenderer::CV_r_log >= 3)
      {
        Vec3d vPos = pObj->GetTranslation();
        Logv(SRendItem::m_RecurseLevel, "+++ Fur Pass %d (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, pObj->m_VisId, vPos[0], vPos[1], vPos[2]);
      }
#endif

      if (curVP)
      {
        curVP->mfSetStateMatrices();
        curVP->mfSetVariables(true, &slw->m_VPParamsObj);
      }
      if (slw->m_FShader)
        slw->m_FShader->mfSetVariables(true, slw->m_CGFSParamsObj);
      else
      {
        if (slw->m_RGBComps && slw->m_RGBComps->m_Comps[0] && slw->m_RGBComps->m_Comps[0]->m_bDependsOnObject)
        {
          float *vals = slw->m_RGBComps->mfGet();
          UCol color;
          color.bcolor[2] = (byte)(vals[0] * 255.0f);
          color.bcolor[1] = (byte)(vals[1] * 255.0f);
          color.bcolor[0] = (byte)(vals[2] * 255.0f);
          color.bcolor[3] = (byte)(vals[3] * 255.0f);
          m_RP.m_NeedGlobalColor = color;
        }
        EF_CommitTexStageState();
      }
      CCGVProgram_D3D *vpD3D = (CCGVProgram_D3D *)curVP;
      vec4_t v;
      SParamComp_OSLightPos comp;
      m_RP.m_pCurLight = m_RP.m_pActiveDLights[0];
      comp.mfGet4f(v);
      vpD3D->mfParameter4f("LightPos", v);
      float fLodBias = -0.5f;
      m_pd3dDevice->SetSamplerState(2, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&fLodBias)));
      CREOcLeaf *re = (CREOcLeaf *)m_RP.m_pRE;
      CLeafBuffer *lb = re->m_pBuffer;
      v[2] = -lb->m_fMinU;
      v[0] = 1.0f / (lb->m_fMaxU + v[2]);
      v[3] = -lb->m_fMinV;
      v[1] = 1.0f / (lb->m_fMaxV + v[3]);
      vpD3D->mfParameter4f("ScaleBiasTC", v);

      for (i=0; i<nLayers; i++)
      {
        m_RP.m_StatNumPasses++;
		    float layer = float(i+1) / nLayers;
		    float length = fFurLength * layer;
		    float scale = -fFurLength * (1.0f*layer*layer + 0.4f*layer);
		    fm->Bind(layer, 2);
        float vModifiers[4];
        vModifiers[0] = length;
        vModifiers[1] = 0.0f;
        vModifiers[2] = 0.0f;
        vModifiers[3] = scale;
        vpD3D->mfParameter4f("Modifiers", vModifiers);

        {
          //PROFILE_FRAME(Draw_ShaderIndexMesh);
          if (m_RP.m_pRE)
            m_RP.m_pRE->mfDraw(ef, slw);
          else
            EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
        }
      }
      m_pd3dDevice->SetSamplerState(2, D3DSAMP_MIPMAPLODBIAS, 0);
      nObj++;
      if (nObj >= m_RP.m_MergedObjects.Num())
        break;
    }
    EF_FogRestore(bFogOverrided);
    if (!CVProgram::m_LastVP)
    {
      if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
        EF_CommitTexTransforms(false);
    }
    else
      m_RP.m_FlagsModificators &= ~(RBMF_TCM | RBMF_TCG);
    if (pSaveObj != m_RP.m_pCurObject)
    {
      m_RP.m_pCurObject = pSaveObj;
      m_RP.m_FrameObject++;
      if (!curVP)
        EF_SetObjectTransform(pSaveObj, ef, pSaveObj->m_ObjFlags);
    }
#endif
    slw->mfResetTextures();      
    EF_ApplyMatrixOps(slw->m_MatrixOps, false);
  }
  m_RP.m_pCurLight = NULL;
}

struct SShadowLight
{
  list2<ShadowMapLightSourceInstance> *pSmLI;
  CDLight *pDL;
};

static bool sbHasDot3LM;
static _inline int Compare(SShadowLight &a, SShadowLight &b)
{
  if (a.pSmLI->Count() > b.pSmLI->Count())
    return -1;
  if (a.pSmLI->Count() < b.pSmLI->Count())
    return 1;
  if (sbHasDot3LM)
  {
    if ((a.pDL->m_Flags & DLF_LM) < (b.pDL->m_Flags & DLF_LM))
      return -1;
    if ((a.pDL->m_Flags & DLF_LM) > (b.pDL->m_Flags & DLF_LM))
      return -1;
  }
  return 0;
}

#include <IEntityRenderState.h>

int CD3D9Renderer::EF_DrawMultiShadowPasses(SShaderTechnique *hs, SShader *ef, int nStart)
{
  int i, j;
  int nStartAmb = -1;
  int nEndAmb = -1;
  int nStartShadow = -1;
  int nEndShadow = -1;
  int nStartLight = -1;
  int nEndLight = -1;
  int nEnd = -1;
  nStart++;
  if (m_RP.m_pCurObject->m_ObjFlags & FOB_SELECTED)
  {
    int nnn = 0;
  }
  bool bMultiLights = false;
  for (i=nStart; i<hs->m_Passes.Num(); i++)
  {
    switch (hs->m_Passes[i].m_ePassType)
    {
      case eSHP_MultiLights:
        bMultiLights = true;
      case eSHP_SpecularLight:
      case eSHP_DiffuseLight:
      case eSHP_Light:
        if (nStartLight < 0)
          nStartLight = i;
        else
          nEndLight = i;
        break;
      case eSHP_General:
        if (nStartAmb < 0)
          nStartAmb = i;
        else
          nEndAmb = i;
        break;
      case eSHP_Shadow:
        if (nStartShadow < 0)
          nStartShadow = i;
        else
          nEndShadow = i;
        break;
      default:
        nEnd = i;
    }
  }
  if (nEnd < 0)
    nEnd = i-1;

  list2<ShadowMapLightSourceInstance> * lsources = (list2<ShadowMapLightSourceInstance>*)m_RP.m_pCurObject->m_pShadowCasters;
  if (!lsources || !lsources->Count())
    return nEnd;
  int nCaster = 0;
  /*if (ef->m_eSort != eS_TerrainShadowPass)
  {
    if (!CRenderer::CV_r_selfshadow || !(m_Features & RFT_SHADOWMAP_SELFSHADOW))
    {
      if(	lsources->Get(0)->m_pLS->GetShadowMapFrustum()->pOwner == lsources->Get(0)->m_pReceiver && 
        !(lsources->Get(0)->m_pLS->GetShadowMapFrustum()->dwFlags & SMFF_ACTIVE_SHADOW_MAP))
        nCaster++; // skip self shadowing pass
    }
  }*/

  if (nEndLight < 0)
    nEndLight = nStartLight;
  if (nEndShadow < 0)
    nEndShadow = nStartShadow;
  if (nEndAmb < 0)
    nEndAmb = nStartAmb;
  assert(nStartShadow>=0);
  assert(nStartLight>=0);

  int nFrustrums = 0;
  list2<ShadowMapLightSourceInstance> SmLI[16];
  SShadowLight SL[16];
  int nLights = m_RP.m_NumActiveDLights;
  bool bHasDot3LM = m_RP.m_pShaderResources && m_RP.m_pCurObject->m_nLMDirId;
  
  for (i=0; i<nLights; i++)
  {
    SL[i].pSmLI = &SmLI[i];
    SL[i].pDL = m_RP.m_pActiveDLights[i];
    int nLightID = SL[i].pDL->m_Id;
    for (j=nCaster; j<lsources->Count(); j++)
    {
      ShadowMapLightSourceInstance *Inst = lsources->Get(j);
      if (Inst->m_pLS->nDLightId == nLightID)
        SmLI[i].Add(*Inst);
    }
  }
  sbHasDot3LM = bHasDot3LM;
  ::Sort(&SL[0], nLights);
  int nStartLightWithoutSC = -1;
  m_RP.m_FlagsPerFlush &= ~RBSI_ALPHATEST;
  for (i=0; i<nLights; i++)
  {
    if (SL[i].pSmLI->Count())
    {
      m_RP.m_pCurObject->m_pShadowCasters = SL[i].pSmLI;
      m_RP.m_pActiveDLights[0] = SL[i].pDL;
      m_RP.m_NumActiveDLights = 1;
      m_RP.m_pCurLight = SL[i].pDL;
      // Draw shadows in back-buffer alpha-channel
      EF_DrawShadowPasses(hs, ef, nStartShadow, nEndShadow, true);
      if (bHasDot3LM && (SL[i].pDL->m_Flags & DLF_LM))
      { // Shadow maps on light-map case
        // Specular pass
        if (bMultiLights)
          EF_DrawLightPasses_PS30(hs, ef, nStartLight, nEndLight, true);
        else
          EF_DrawLightPasses(hs, ef, nStartLight, nEndLight, true);
        // LM pass
        EF_DrawGeneralPasses(hs, ef, false, nStartAmb, nEndAmb, true);
      }
      else
      {
        // Shadow maps on dynamically lighted object case
        if (bMultiLights)
          EF_DrawLightPasses_PS30(hs, ef, nStartLight, nEndLight, true);
        else
          EF_DrawLightPasses(hs, ef, nStartLight, nEndLight, true);
      }
    }
    else
    {
      if (nStartLightWithoutSC < 0)
        nStartLightWithoutSC = i;
      m_RP.m_pActiveDLights[i-nStartLightWithoutSC] = SL[i].pDL;
    }
  }
  if (nStartLightWithoutSC >= 0)
  {
    // Draw light sources without shadow-casters
    m_RP.m_NumActiveDLights = nLights-nStartLightWithoutSC;
    EF_DrawLightPasses(hs, ef, nStartLight, nEndLight, false);
  }
  m_RP.m_pCurObject->m_pShadowCasters = lsources;
  for (i=0; i<m_RP.m_NumActiveDLights; i++)
  {
    m_RP.m_pActiveDLights[i] = SL[i].pDL;
  }
  m_RP.m_NumActiveDLights = nLights;

  return nEnd;
}

// Draw shadow map passes (used in programmable pipeline shaders only)
void CD3D9Renderer::EF_DrawShadowPasses(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd, bool bDstAlpha)
{
  SShaderPassHW *slw;
  int i;

  if ((m_RP.m_ObjFlags & FOB_LIGHTPASS) && (ef->m_Flags & EF_USELIGHTS))
    return;

  m_RP.m_nCurLight = 0;
  CVProgram *curVP = NULL;
  CVProgram *newVP;

  list2<ShadowMapLightSourceInstance> * lsources = (list2<ShadowMapLightSourceInstance>*)m_RP.m_pCurObject->m_pShadowCasters;
  if (!lsources || !lsources->Count())
    return;
  m_RP.m_FlagsPerFlush |= RBSI_SHADOWPASS;

  PROFILE_FRAME(DrawShader_ShadowPasses);

  CDLight *pSaveLight = m_RP.m_pCurLight;

  int nCaster = 0;
  if (ef->m_eSort != eS_TerrainShadowPass)
  {
    if (!CRenderer::CV_r_selfshadow || !(m_Features & RFT_SHADOWMAP_SELFSHADOW))
    {
      if(	lsources->Get(0)->m_pLS->GetShadowMapFrustum()->pOwner == lsources->Get(0)->m_pReceiver && 
				!(lsources->Get(0)->m_pLS->GetShadowMapFrustum()->dwFlags & SMFF_ACTIVE_SHADOW_MAP))
        nCaster++; // skip self shadowing pass
    }
  }
  int nCasters = lsources->Count();
  int nDeltaCasters = 1;

  int nPass = 0;
  for (; nCaster<nCasters; nCaster+=nDeltaCasters)
  {
    m_RP.m_nCurStartCaster = nCaster;
    m_RP.m_StatNumPasses++;
    slw = &hs->m_Passes[nStart];
    for (i=nStart; i<=nEnd; i++, slw++)
    {
      int msk;
      bool bUseLight = false;
      if (msk = (slw->m_LightFlags & DLF_LIGHTTYPE_MASK))
      {
        bUseLight = true;
        if (!m_RP.m_NumActiveDLights)
          return;
        m_RP.m_pCurLight = m_RP.m_pActiveDLights[0];
        m_RP.m_nCurLight = m_RP.m_pCurLight->m_Id;
        if (!(msk & (m_RP.m_pCurLight->m_Flags & DLF_LIGHTTYPE_MASK)))
          continue;
      }
      if (slw->m_LMFlags & LMF_SAMPLES)
      {
        if (slw->m_LMFlags & LMF_4SAMPLES)
        {
          if (nCasters-nCaster < 4)
            continue;
          nDeltaCasters = 4;
        }
        else
        if (slw->m_LMFlags & LMF_3SAMPLES)
        {
          if (nCasters-nCaster < 3)
            continue;
          nDeltaCasters = 3;
        }
        else
        if (slw->m_LMFlags & LMF_2SAMPLES)
        {
          if (nCasters-nCaster < 2)
            continue;
          nDeltaCasters = 2;
        }
        else
        if (slw->m_LMFlags & LMF_1SAMPLES)
        {
          if (nCasters-nCaster < 1)
            continue;
          nDeltaCasters = 1;
        }
      }
      m_RP.m_CurrPass = slw;

      m_RP.m_FlagsModificators = (m_RP.m_FlagsModificators & ~7) | (slw->m_Flags & 3);

      EF_Eval_TexGen(slw);
      EF_Eval_RGBAGen(slw);
      EF_SetVertexStreams(slw->m_Pointers, 1);

      // Set all textures and HW TexGen modes for the current pass (ShadeLayer)
      newVP = slw->m_VProgram;
      if (newVP)
        m_RP.m_FlagsPerFlush |= RBSI_USEVP;
      if (slw->mfSetTextures())
      {        
        // Set vertex program for the current pass if needed
        if (newVP != curVP)
        {
          if (newVP)
          {
            curVP = newVP;
            curVP->mfSet(true, slw, VPF_DONTSETMATRICES);
          }
          else
            curVP = NULL;
        }

        EF_ApplyMatrixOps(slw->m_MatrixOps, true);

        if (curVP)
        {
#ifndef PIPE_USE_INSTANCING
          curVP->mfSetStateMatrices();
#endif
          curVP->mfSetVariables(false, &slw->m_VPParamsNoObj);
        }
        else
        {
          m_RP.m_CurrentVLights = 0;
          m_RP.m_PersFlags &= ~RBPF_VSNEEDSET;
        }

        // Set Pixel shaders and Register combiners for the current pass
        if (slw->m_FShader)
          slw->m_FShader->mfSet(true, slw);
        else
          m_RP.m_PersFlags &= ~RBPF_PS1NEEDSET;

        int State;
        if (m_RP.m_RendPass || (m_RP.m_ObjFlags & FOB_LIGHTPASS))
          State = slw->m_SecondRenderState;
        else
          State = slw->m_RenderState;
        if (bDstAlpha)
        {
          if (nPass)
            State = (State & ~GS_BLEND_MASK) | GS_BLSRC_ONE | GS_BLDST_ONE;
          nPass++;
        }
        EF_SetState(State);

#ifndef PIPE_USE_INSTANCING

#ifdef DO_RENDERLOG
        if (CRenderer::CV_r_log >= 3)
          Logv(SRendItem::m_RecurseLevel, "+++ Shadow Pass %d [%d Samples]\n", m_RP.m_RendPass, nDeltaCasters);
#endif

        EF_Scissor(false, 0, 0, 0, 0);

        EF_Draw(ef, slw);

        m_RP.m_RendPass++;

#else //PIPE_USE_INSTANCING
        m_RP.m_RendPass++;

        // Unlock all VB (if needed) and set current streams
        EF_PreDraw(slw);

        int bFogOverrided = 0;
        bool bFogDisable = (curVP && (curVP->m_Flags & VPFI_NOFOG));
        bool bFogVP = (m_RP.m_PersFlags & RBPF_HDR) || (curVP && (curVP->m_Flags & VPFI_VS30ONLY));
        bFogOverrided = EF_FogCorrection(bFogDisable, bFogVP);

        int nObj = 0;
        CCObject *pSaveObj = m_RP.m_pCurObject;
        CCObject *pObj = pSaveObj;
        while (true)
        {       
          if (nObj)
          {
            m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
            m_RP.m_FrameObject++;
            pObj = m_RP.m_pCurObject;

            if (!curVP)
            {
              EF_SetObjectTransform(pObj, ef, pObj->m_ObjFlags);
              m_RP.m_CurrentVLights = 0;

              if (m_RP.m_FrameGTC == m_RP.m_Frame)
              {
                for (int nt=0; nt<gRenDev->m_TexMan->m_nCurStages; nt++)
                {
                  if (m_RP.m_pGTC[nt])
                  {
                    EF_SelectTMU(nt);
                    m_RP.m_pGTC[nt]->mfSet(true);
                  }
                }
              }
            }
          }

          EF_Scissor(false, 0, 0, 0, 0);

#ifdef DO_RENDERLOG
          if (CRenderer::CV_r_log >= 3)
          {
            Vec3d vPos = pObj->GetTranslation();
            Logv(SRendItem::m_RecurseLevel, "+++ Shadow Pass %d [%d Samples] (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, nDeltaCasters, pObj->m_VisId, vPos[0], vPos[1], vPos[2]);
          }
#endif

          if (curVP)
          {
            curVP->mfSetStateMatrices();
            curVP->mfSetVariables(true, &slw->m_VPParamsObj);
          }
          if (slw->m_FShader)
            slw->m_FShader->mfSetVariables(true, slw->m_CGFSParamsObj);
          else
          {
            if (slw->m_RGBComps && slw->m_RGBComps->m_Comps[0] && slw->m_RGBComps->m_Comps[0]->m_bDependsOnObject)
            {
              float *vals = slw->m_RGBComps->mfGet();
              UCol color;
              color.bcolor[2] = (byte)(vals[0] * 255.0f);
              color.bcolor[1] = (byte)(vals[1] * 255.0f);
              color.bcolor[0] = (byte)(vals[2] * 255.0f);
              color.bcolor[3] = (byte)(vals[3] * 255.0f);
              m_RP.m_NeedGlobalColor = color;
            }
            EF_CommitTexStageState();
          }

          {
            //PROFILE_FRAME(Draw_ShaderIndexMesh);
            if (m_RP.m_pRE)
              m_RP.m_pRE->mfDraw(ef, slw);
            else
              EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
          }
          nObj++;
          if (nObj >= m_RP.m_MergedObjects.Num())
            break;
        }
        EF_FogRestore(bFogOverrided);
        if (!CVProgram::m_LastVP)
        {
          if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
            EF_CommitTexTransforms(false);
        }
        else
          m_RP.m_FlagsModificators &= ~(RBMF_TCM | RBMF_TCG);
        if (pSaveObj != m_RP.m_pCurObject)
        {
          m_RP.m_pCurObject = pSaveObj;
          m_RP.m_FrameObject++;
          if (!curVP)
            EF_SetObjectTransform(pSaveObj, ef, pSaveObj->m_ObjFlags);
        }
#endif
      }
      slw->mfResetTextures();      
      EF_ApplyMatrixOps(slw->m_MatrixOps, false);
      break;
    }
  }
  m_RP.m_nCurStartCaster = 0;

  m_RP.m_pCurLight = pSaveLight;
  if (m_RP.m_pCurLight)
    m_RP.m_nCurLight = m_RP.m_pCurLight->m_Id;
}

#define INST_PARAM_SIZE 4*sizeof(float)

void CD3D9Renderer::EF_DrawInstances(SShader *ef, SShaderPassHW *slw, int nCurInst, int nLastInst, int nUsage, byte bUsage[], int nInstMask)
{
  int i;

  if (!nCurInst)
  {
    // Set the stream 3 to be per instance data and iterate once per instance
    m_pd3dDevice->SetStreamSourceFreq(3, 1 | D3DSTREAMSOURCE_INSTANCEDATA);
    int nCompared = 0;
    EF_PreDraw(slw, false);
    int StreamMask = m_RP.m_FlagsModificators&7;
    SVertexDeclaration *vd;
    for (i=0; i<m_RP.m_CustomVD.Num(); i++)
    {
      vd = m_RP.m_CustomVD[i];
      if (vd->StreamMask == StreamMask && vd->VertFormat == m_RP.m_CurVFormat && vd->InstMask == nInstMask)
        break;
    }
    if (i == m_RP.m_CustomVD.Num())
    {
      vd = new SVertexDeclaration;
      m_RP.m_CustomVD.AddElem(vd);
      vd->StreamMask = StreamMask;
      vd->VertFormat = m_RP.m_CurVFormat;
      vd->InstMask = nInstMask;
      vd->m_pDeclaration = NULL;
      for (i=0; i<m_RP.m_D3DFixedPipeline[StreamMask][m_RP.m_CurVFormat].m_Declaration.Num()-1; i++)
      {
        vd->m_Declaration.AddElem(m_RP.m_D3DFixedPipeline[StreamMask][m_RP.m_CurVFormat].m_Declaration[i]);
      }
      D3DVERTEXELEMENT9 ve;
      ve.Stream = 3;
      ve.Type = D3DDECLTYPE_FLOAT4;
      ve.Method = D3DDECLMETHOD_DEFAULT;
      ve.Usage = D3DDECLUSAGE_TEXCOORD;
      for (i=0; i<nUsage; i++)
      {
        ve.Offset = i*INST_PARAM_SIZE;
        ve.UsageIndex = bUsage[i];
        vd->m_Declaration.AddElem(ve);
      }
      ve.Stream = 0xff;
      ve.Type = D3DDECLTYPE_UNUSED;
      ve.Usage = 0;
      ve.UsageIndex = 0;
      ve.Offset = 0;
      vd->m_Declaration.AddElem(ve);
    }
    HRESULT hr;
    if (!vd->m_pDeclaration)
    {
      hr = m_pd3dDevice->CreateVertexDeclaration(&vd->m_Declaration[0], &vd->m_pDeclaration);
      assert (hr == S_OK);
    }
    if (m_pLastVDeclaration != vd->m_pDeclaration)
    {
      m_pLastVDeclaration = vd->m_pDeclaration;
      hr = m_pd3dDevice->SetVertexDeclaration(vd->m_pDeclaration);
      assert (hr == S_OK);
    }
  }
  {
    //PROFILE_FRAME(Draw_ShaderIndexMesh);
    int nPolys = m_nPolygons;
    if (m_RP.m_pRE)
      m_RP.m_pRE->mfDraw(ef, slw);
    else
      EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
    int nPolysPerInst = m_nPolygons - nPolys;
    m_nPolygons += nPolysPerInst*(nLastInst-nCurInst);
  }
}

void CD3D9Renderer::EF_DrawGeometryInstancing_VS30(SShader *ef, SShaderPassHW *slw, CVProgram *curVP)
{
  PROFILE_FRAME(DrawShader_GeometryInstancing_VS30);

  int i, j, n;
  CCGVProgram_D3D *vp = (CCGVProgram_D3D *)curVP;
  int nOffs;
  int nUsage = 1;
  byte bUsage[16];
  Matrix44 m;
  int nCurInst = 0;
  SCGBind bind;
  bind.m_dwBind = 65536;
  bind.m_nBindComponents = 4;
  bind.m_dwFrameCreated = vp->m_dwFrame;
  mathMatrixTranspose(m.GetData(), m_CameraProjMatrix.GetData(), g_CpuFlags);
  vp->mfParameter(&bind, m.GetData(), 4);
  m_RP.m_RotatedMergedObjects.SetUse(0);
  m_RP.m_NonRotatedMergedObjects.SetUse(0);
  int nSimple = 0;
  for (i=0; i<m_RP.m_MergedObjects.Num(); i++)
  {
    CCObject *pObj = m_RP.m_MergedObjects[i];
    if (pObj->m_ObjFlags & FOB_TRANS_ROTATE)
      m_RP.m_RotatedMergedObjects.AddElem(pObj);
    else
      m_RP.m_NonRotatedMergedObjects.AddElem(pObj);
  }
  if (m_RP.m_NonRotatedMergedObjects.Num())
  {
    memset(bUsage, 0, sizeof(bUsage));
    bUsage[0] = 1;
    int nInstMask = 0x2;
    for (j=0; j<vp->m_Params_Inst.Num(); j++)
    {
      bUsage[nUsage] = nUsage+1;
      nInstMask |= 1<<(nUsage+1);
      nUsage++;
    }
    while (nCurInst < m_RP.m_NonRotatedMergedObjects.Num())
    {
      int nLastInst = m_RP.m_NonRotatedMergedObjects.Num() - 1;
      if ((nLastInst-nCurInst+1)*nUsage >= MAX_HWINST_PARAMS)
        nLastInst = nCurInst+(MAX_HWINST_PARAMS/nUsage)-1;
      byte *data = (byte *)m_RP.m_VB_Inst->Lock((nLastInst-nCurInst+1)*nUsage*INST_PARAM_SIZE, nOffs);
      CCObject *curObj = m_RP.m_pCurObject;
      n = 0;
      // Fill the stream 3 for per-instance data
      for (i=nCurInst; i<=nLastInst; i++)
      {
        CCObject *pObj = m_RP.m_NonRotatedMergedObjects[i];

        /*if (i != nLastInst)
        {
          CCObject *pObj = m_RP.m_NonRotatedMergedObjects[i+1];
          cryPrefetchNTSSE(pObj->m_Matrix.GetData());
        }*/

        float *fParm = (float *)&data[n*nUsage*INST_PARAM_SIZE];
        n++;
        float *fSrc = pObj->m_Matrix.GetData();
        fParm[0] = fSrc[12]; fParm[1] = fSrc[13]; fParm[2] = fSrc[14]; fParm[3] = fSrc[0];
        fParm += 4;

        //mathMatrixTranspose((float *)&data[nOffset], m_RP.m_MergedObjects[i]->GetVPMatrix().GetData(), g_CpuFlags);
        //memcpy(&data[n*nUsage*4*sizeof(float)], pObj->GetVPMatrix().GetData(), 4*4*sizeof(float));
        m_RP.m_pCurObject = pObj;
        m_RP.m_FrameObject++;
        for (j=0; j<vp->m_Params_Inst.Num(); j++)
        {
          float *v = vp->m_Params_Inst[j].mfGet();
          fParm[0] = v[0];
          fParm[1] = v[1];
          fParm[2] = v[2];
          fParm[3] = v[3];

          fParm += 4;
        }
      }
      m_RP.m_pCurObject = curObj;
      m_RP.m_VB_Inst->Unlock();

      // Set the first stream to be the indexed data and render N instances
      m_pd3dDevice->SetStreamSourceFreq(0, n | D3DSTREAMSOURCE_INDEXEDDATA);
      m_RP.m_VB_Inst->Bind(m_pd3dDevice, 3, nOffs, nUsage*INST_PARAM_SIZE);

      EF_DrawInstances(ef, slw, nCurInst, nLastInst, nUsage, bUsage, nInstMask);
      nCurInst = nLastInst+1;
    }
  }
  if (m_RP.m_RotatedMergedObjects.Num())
  {
    int nFlags = VPF_DONTSETMATRICES | VPF_INSTANCING_ROTATE;
    curVP->mfSet(true, slw, nFlags);

    nUsage = 4;
    memset(bUsage, 0, sizeof(bUsage));
    bUsage[0] = 1;
    bUsage[1] = 2;
    bUsage[2] = 3;
    bUsage[3] = 4;
    int nInstMask = 0x1e;
    for (j=0; j<vp->m_Params_Inst.Num(); j++)
    {
      bUsage[nUsage] = nUsage+1;
      nInstMask |= 1<<(nUsage+1);
      nUsage++;
    }
    Matrix44 m;
    int nCurInst = 0;
    SCGBind bind;
    while (nCurInst < m_RP.m_RotatedMergedObjects.Num())
    {
      int nLastInst = m_RP.m_RotatedMergedObjects.Num() - 1;
      if ((nLastInst-nCurInst+1)*nUsage >= MAX_HWINST_PARAMS)
        nLastInst = nCurInst+(MAX_HWINST_PARAMS/nUsage)-1;
      byte *data = (byte *)m_RP.m_VB_Inst->Lock((nLastInst-nCurInst+1)*nUsage*INST_PARAM_SIZE, nOffs);
      CCObject *curObj = m_RP.m_pCurObject;
      n = 0;
      // Fill the stream 3 for per-instance data
      for (i=nCurInst; i<=nLastInst; i++)
      {
        CCObject *pObj = m_RP.m_RotatedMergedObjects[i];

        float *fParm = (float *)&data[n*nUsage*INST_PARAM_SIZE];
        n++;
        mathMatrixTranspose(fParm, m_RP.m_RotatedMergedObjects[i]->GetVPMatrix().GetData(), g_CpuFlags);
        fParm += 4*4;
        m_RP.m_pCurObject = m_RP.m_RotatedMergedObjects[i];

        m_RP.m_pCurObject = pObj;
        m_RP.m_FrameObject++;
        for (j=0; j<vp->m_Params_Inst.Num(); j++)
        {
          float *v = vp->m_Params_Inst[j].mfGet();
          fParm[0] = v[0];
          fParm[1] = v[1];
          fParm[2] = v[2];
          fParm[3] = v[3];

          fParm += 4;
        }
      }
      m_RP.m_pCurObject = curObj;
      m_RP.m_VB_Inst->Unlock();

      // Set the first stream to be the indexed data and render N instances
      m_pd3dDevice->SetStreamSourceFreq(0, n | D3DSTREAMSOURCE_INDEXEDDATA);
      m_RP.m_VB_Inst->Bind(m_pd3dDevice, 3, nOffs, nUsage*INST_PARAM_SIZE);

      EF_DrawInstances(ef, slw, nCurInst, nLastInst, nUsage, bUsage, nInstMask);
      nCurInst = nLastInst+1;
    }
  }

  m_pd3dDevice->SetStreamSource(3, NULL, 0, 0);
  m_pd3dDevice->SetStreamSourceFreq(0, 1);
}

// Draw general/ambient passes (used in FP shaders and in programmable pipeline shaders)
void CD3D9Renderer::EF_DrawGeneralPasses(SShaderTechnique *hs, SShader *ef, bool bFog, int nStart, int nEnd, bool bDstAlpha)
{
  SShaderPassHW *slw;
  int i;

  if ((m_RP.m_ObjFlags & FOB_LIGHTPASS) && (ef->m_Flags & EF_USELIGHTS))
    return;
  if (!bFog && (m_RP.m_ObjFlags & FOB_FOGPASS))
    return;

  PROFILE_FRAME(DrawShader_GeneralPasses);

  m_RP.m_nCurLight = 0;
  CVProgram *curVP = NULL;
  CVProgram *newVP;

  slw = &hs->m_Passes[nStart];
  for (i=nStart; i<=nEnd; i++, slw++)
  {
    m_RP.m_StatNumPasses++;
    m_RP.m_CurrPass = slw;

    m_RP.m_FlagsModificators = (m_RP.m_FlagsModificators & ~7) | (slw->m_Flags & 3);

    EF_Eval_DeformVerts(slw->m_Deforms);
    EF_Eval_TexGen(slw);
    EF_Eval_RGBAGen(slw);
    EF_SetVertexStreams(slw->m_Pointers, 1);

    // Set all textures and HW TexGen modes for the current pass (ShadeLayer)
    newVP = slw->m_VProgram;
    if (newVP)
      m_RP.m_FlagsPerFlush |= RBSI_USEVP;
    if (slw->mfSetTextures())
    {
      bool bInstancing = false;
      
      // Set vertex program for the current pass if needed
      if (newVP != curVP)
      {
        if (newVP)
        {
          curVP = newVP;
          int nFlags = VPF_DONTSETMATRICES;
          if (CV_r_geominstancing && m_RP.m_MergedObjects.Num() > 2 && m_bDeviceSupportsInstancing && (curVP->m_Flags & VPFI_SUPPORTS_INSTANCING) && !(m_RP.m_FlagsModificators & RBMF_TCG))
          {
            bInstancing = true;
            nFlags |= VPF_INSTANCING_NOROTATE;
          }
          curVP->mfSet(true, slw, nFlags);
        }
        else
          curVP = NULL;
      }

      EF_ApplyMatrixOps(slw->m_MatrixOps, true);

      if (curVP)
      {
#ifndef PIPE_USE_INSTANCING
        curVP->mfSetStateMatrices();
#endif
        curVP->mfSetVariables(false, &slw->m_VPParamsNoObj);
      }
      else
      {
        if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_LMaterial && !(ef->m_LMFlags & LMF_DISABLE))
        {
          m_RP.m_pShaderResources->m_LMaterial->mfApply(slw->m_LMFlags);
          if (!(slw->m_LMFlags & (LMF_DISABLE | LMF_HASPSHADER)) && !(m_RP.m_CurrentVLights & 0xff))
            EF_ConstantLightMaterial(m_RP.m_pShaderResources->m_LMaterial, slw->m_LMFlags);
        }
        else
          m_RP.m_CurrentVLights = 0;
        m_RP.m_PersFlags &= ~RBPF_VSNEEDSET;
      }

      // Set Pixel shaders and Register combiners for the current pass
      if (slw->m_FShader)
      {
        if (!bInstancing)
          slw->m_FShader->mfSet(true, slw, 0);
        else
          slw->m_FShader->mfSet(true, slw, PSF_INSTANCING);
      }
      else
        m_RP.m_PersFlags &= ~RBPF_PS1NEEDSET;

      // Set Render states for the current pass
      int State;
      if (bFog)
      {
        State = slw->m_RenderState;
        if (m_RP.m_pShader->m_Flags2 & EF2_OPAQUE)
          State |= GS_DEPTHFUNC_EQUAL;
      }
      else
      if (m_RP.m_pCurObject->m_RenderState)
        State = m_RP.m_pCurObject->m_RenderState;
      else
      if (m_RP.m_RendPass || (m_RP.m_ObjFlags & FOB_LIGHTPASS))
        State = slw->m_SecondRenderState;
      else
        State = slw->m_RenderState;
      if (bDstAlpha)
        State = (State & ~GS_BLEND_MASK) | GS_BLSRC_DSTALPHA | GS_BLDST_ONEMINUSDSTALPHA;
      EF_SetState(State);
#ifndef PIPE_USE_INSTANCING

#ifdef DO_RENDERLOG
      if (CRenderer::CV_r_log >= 3)
      {
        if (bFog)
          Logv(SRendItem::m_RecurseLevel, "+++ Fog Pass %d\n", m_RP.m_RendPass);
        else
          Logv(SRendItem::m_RecurseLevel, "+++ General Pass %d\n", m_RP.m_RendPass);
      }
#endif

      m_RP.m_pCurObject->SetScissor();

      EF_Draw(ef, slw);

      m_RP.m_RendPass++;

#else //PIPE_USE_INSTANCING
      HRESULT h;
      m_RP.m_RendPass++;

      int bFogOverrided = 0;
      bool bFogDisable = bFog || (curVP && (curVP->m_Flags & VPFI_NOFOG));
      bool bFogVP = (m_RP.m_PersFlags & RBPF_HDR) || (curVP && (curVP->m_Flags & VPFI_VS30ONLY));
      bFogOverrided = EF_FogCorrection(bFogDisable, bFogVP);

      // Assume if HW supports PS3.0, VS3.0 are supported as well
      if (bInstancing)
      {
        // Using geometry instancing approach
        EF_DrawGeometryInstancing_VS30(ef, slw, curVP);
      }
      else
      {
        // Unlock all VB (if needed) and set current streams
        EF_PreDraw(slw);

        int nObj = 0;
        CCObject *pSaveObj = m_RP.m_pCurObject;
        CCObject *pObj = pSaveObj;
        while (true)
        {       
          if (nObj)
          {
            m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
            m_RP.m_FrameObject++;
            pObj = m_RP.m_pCurObject;

            if (m_RP.m_FlagsModificators & (RBMF_LMTCUSED | RBMF_BENDINFOUSED))
            {
              if (pObj->m_pLMTCBufferO && pObj->m_pLMTCBufferO->m_pVertexBuffer)
              {
                int nOffs;
                IDirect3DVertexBuffer9 *pBuf = (IDirect3DVertexBuffer9 *)pObj->m_pLMTCBufferO->m_pVertexBuffer->GetStream(VSF_GENERAL, &nOffs);
                h = m_pd3dDevice->SetStreamSource( 2, pBuf, nOffs, m_VertexSize[pObj->m_pLMTCBufferO->m_pVertexBuffer->m_vertexformat]);
              }
            }
            if (!curVP)
            {
              EF_SetObjectTransform(pObj, ef, pObj->m_ObjFlags);
              if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_LMaterial && !(ef->m_LMFlags & LMF_DISABLE))
                m_RP.m_pShaderResources->m_LMaterial->mfApply(slw->m_LMFlags);
              else
                m_RP.m_CurrentVLights = 0;

              if (m_RP.m_FrameGTC == m_RP.m_Frame)
              {
                for (int nt=0; nt<gRenDev->m_TexMan->m_nCurStages; nt++)
                {
                  if (m_RP.m_pGTC[nt])
                  {
                    EF_SelectTMU(nt);
                    m_RP.m_pGTC[nt]->mfSet(true);
                  }
                }
              }
            }
          }

          pObj->SetScissor();

  #ifdef DO_RENDERLOG
          if (CRenderer::CV_r_log >= 3)
          {
            Vec3d vPos = pObj->GetTranslation();
            if (bFog)
              Logv(SRendItem::m_RecurseLevel, "+++ Fog Pass %d (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, pObj->m_VisId, vPos[0], vPos[1], vPos[2]);
            else
              Logv(SRendItem::m_RecurseLevel, "+++ General Pass %d (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, pObj->m_VisId, vPos[0], vPos[1], vPos[2]);
          }
  #endif

          // Set per-object alpha-blending
          if (pObj->m_ObjFlags & FOB_HASALPHA)
            pObj->SetAlphaState(slw->m_FShader, State);

          if (curVP)
          {
            curVP->mfSetStateMatrices();
            curVP->mfSetVariables(true, &slw->m_VPParamsObj);
          }
          
          if (slw->m_FShader)
            slw->m_FShader->mfSetVariables(true, slw->m_CGFSParamsObj);
          else
          {
            if (slw->m_RGBComps && slw->m_RGBComps->m_Comps[0] && slw->m_RGBComps->m_Comps[0]->m_bDependsOnObject)
            {
              float *vals = slw->m_RGBComps->mfGet();
              UCol color;
              color.bcolor[2] = (byte)(vals[0] * 255.0f);
              color.bcolor[1] = (byte)(vals[1] * 255.0f);
              color.bcolor[0] = (byte)(vals[2] * 255.0f);
              color.bcolor[3] = (byte)(vals[3] * 255.0f);
              m_RP.m_NeedGlobalColor = color;
            }
            EF_CommitTexStageState();
          }

          {
            //PROFILE_FRAME(Draw_ShaderIndexMesh);
            if (m_RP.m_pRE)
              m_RP.m_pRE->mfDraw(ef, slw);
            else
              EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
          }
          nObj++;
          if (nObj >= m_RP.m_MergedObjects.Num())
            break;
        }
        EF_FogRestore(bFogOverrided);
        if (!CVProgram::m_LastVP)
        {
          if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
            EF_CommitTexTransforms(false);
        }
        else
          m_RP.m_FlagsModificators &= ~(RBMF_TCM | RBMF_TCG);
        if (pSaveObj != m_RP.m_pCurObject)
        {
          m_RP.m_pCurObject = pSaveObj;
          m_RP.m_FrameObject++;
          if (!curVP)
            EF_SetObjectTransform(pSaveObj, ef, pSaveObj->m_ObjFlags);
        }
      }
#endif
    }
    
    slw->mfResetTextures();      
    EF_ApplyMatrixOps(slw->m_MatrixOps, false);
  }
}

// Draw light passes (up to 4 light sources per single pass)
void CD3D9Renderer::EF_DrawLightPasses_PS30(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd, bool bDstAlpha)
{
  SShaderPassHW *slw;
  int i;

  m_RP.m_nCurLight = 0;
  CVProgram *curVP = NULL;

  PROFILE_FRAME(DrawShader_MultiLight_Passes);

  int nLight;
  int nLights = m_RP.m_NumActiveDLights;

  // make sure for each pass not more then 1 projected light
  if (gRenDev->m_RP.m_pCurObject->m_ObjFlags & FOB_SELECTED)
  {
    int nnn = 0;
  }

  int nStencState = 0;
  bool bStencState = m_RP.m_pStateShader && m_RP.m_pStateShader->m_State && m_RP.m_pStateShader->m_State->m_Stencil;

  slw = &hs->m_Passes[nStart];
  CDLight *ProjLights[16];
  int nProjLights = 0;
  CDLight *OtherLights[16];
  int nOtherLights = 0;
  CDLight *StencLights[16];
  int nStencLights = 0;
  bool bHasLM = m_RP.m_pCurObject->m_nLMId != 0;
  bool bHasAmb = ((m_RP.m_ObjFlags & FOB_LIGHTPASS) == 0);
  if (bDstAlpha || m_RP.m_RendPass)
  {
    bHasAmb = false;
  }
  for (nLight=0; nLight<nLights; nLight++)
  {
    CDLight *dl = m_RP.m_pActiveDLights[nLight];
    // ignore LM light sources for diffuse only materials with LM but without specular
    if (bHasLM && (dl->m_Flags & DLF_LM) && (slw->m_LMFlags & LMF_NOSPECULAR))
      continue;
    if (bStencState && (dl->m_Flags & DLF_CASTSHADOW_VOLUME))
      StencLights[nStencLights++] = dl;
    else
    if (dl->m_Flags & DLF_PROJECT)
      ProjLights[nProjLights++] = dl;
    else
      OtherLights[nOtherLights++] = dl;
  }
  if (!bHasAmb && !nOtherLights && !nProjLights && !nStencLights)
    return;
  float fFP = 0.334f;
  if (!(m_Features & RFT_HW_PS30))
    fFP = 0.5f;
  float fFO = 1.0f;
  int nO = 0;
  int nP = 0;
  float fO = 0;
  float fP = 0;
  int nMaxLights = (m_Features & RFT_HW_PS30) ? NUM_PPLIGHTS_PERPASS_PS30 : NUM_PPLIGHTS_PERPASS_PS2X;
  int nAmbLights = nMaxLights;
  if (bHasAmb)
  {
    for (i=nStart; i<=nEnd; i++, slw++)
    {
      if (slw->m_LMFlags & LMF_HASAMBIENT)
      {
        if (slw->m_LMFlags & LMF_HASDOT3LM)
        {
          if (!m_RP.m_pShaderResources || !m_RP.m_pCurObject->m_nLMDirId)
            continue;
        }
      }
      break;
    }
    nAmbLights = slw->m_nAmbMaxLights;
  }
  nAmbLights = min(nAmbLights, nMaxLights);
  slw = &hs->m_Passes[nStart];
  for (i=0; i<nStencLights; i++)
  {
    m_RP.m_LPasses[i].nProjectors = 0;
    m_RP.m_LPasses[i].nLights = 1;
    m_RP.m_LPasses[i].pLights[0] = StencLights[i];
  }
  for (; i<32; i++)
  {
    if (nP >= nProjLights && nO >= nOtherLights)
    {
      if (!i && bHasAmb)
      {
        m_RP.m_LPasses[0].nLights = 0;
        m_RP.m_LPasses[0].nProjectors = 0;
        i++;
      }
      break;
    }
    m_RP.m_LPasses[i].nLights = 0;
    m_RP.m_LPasses[i].nProjectors = 0;
    int j = 0;
    while (true)
    {
      fO += fFO;
      if ((int)fO >= nO+1 && nO < nOtherLights)
      {
        m_RP.m_LPasses[i].nLights++;
        m_RP.m_LPasses[i].pLights[j] = OtherLights[nO];
        j++;
        nO++;
      }
      if (nP >= nProjLights && nO >= nOtherLights)
        break;
      if (j == nAmbLights)
      {
        nAmbLights = nMaxLights;
        break;
      }
      if (m_RP.m_LPasses[i].nProjectors == 0)
      {
        fP += fFP;
        if ((int)fP >= nP+1 && nP < nProjLights)
        {
          m_RP.m_LPasses[i].nLights++;
          m_RP.m_LPasses[i].nProjectors++;
          m_RP.m_LPasses[i].pLights[j] = ProjLights[nP];
          j++;
          nP++;
        }
      }
      else
      {
        if (nO >= nOtherLights)
          break;
      }
      if (nP >= nProjLights && nO >= nOtherLights)
        break;
      if (j == nAmbLights)
      {
        nAmbLights = nMaxLights;
        break;
      }
    }
  }

  int nPasses = i;
  int nPass;
  m_RP.m_nCurLight = 0;
  m_RP.m_PersFlags |= RBPF_MULTILIGHTS;
  for (nPass=0; nPass<nPasses; nPass++)
  {
    m_RP.m_StatNumPasses++;
    m_RP.m_nCurLightPass = nPass;
    SLightPass *lp = &m_RP.m_LPasses[nPass];
    m_RP.m_ShaderLightMask = lp->nLights;
    m_RP.m_pCurLight = NULL;
    int Types[4];
    bool bUseOccl = false;
    for (i=0; i<lp->nLights; i++)
    {
      assert(lp->nLights <= nMaxLights);
      CDLight *dl = lp->pLights[i];
      if (dl->m_Flags & DLF_POINT)
        Types[i] = SLMF_POINT;
      else
      if (dl->m_Flags & DLF_PROJECT)
      {
        Types[i] = SLMF_PROJECTED;
        m_RP.m_pCurLight = dl;
      }
      else
        Types[i] = SLMF_DIRECT;
      if ((dl->m_Flags & DLF_LM) && bHasLM)
      {
        Types[i] |= SLMF_ONLYSPEC;
        if (*(int *)m_RP.m_pCurObject->m_OcclLights != 0)
        {
          byte nLight = dl->m_Id+1;
          if (m_RP.m_pCurObject->m_OcclLights[0] == nLight || m_RP.m_pCurObject->m_OcclLights[1] == nLight || m_RP.m_pCurObject->m_OcclLights[2] == nLight || m_RP.m_pCurObject->m_OcclLights[3] == nLight)
          {
            Types[i] |= SLMF_SPECOCCLUSION;
            bUseOccl = true;
          }
        }
      }
    }
    switch(lp->nLights)
    {
      case 2:
        if (Types[0] > Types[1])
        {
          Exchange(Types[0], Types[1]);
          Exchange(lp->pLights[0], lp->pLights[1]);
        }
  	    break;
      case 3:
        if (Types[0] > Types[1])
        {
          Exchange(Types[0], Types[1]);
          Exchange(lp->pLights[0], lp->pLights[1]);
        }
        if (Types[0] > Types[2])
        {
          Exchange(Types[0], Types[2]);
          Exchange(lp->pLights[0], lp->pLights[2]);
        }
        if (Types[1] > Types[2])
        {
          Exchange(Types[1], Types[2]);
          Exchange(lp->pLights[1], lp->pLights[2]);
        }
    	  break;
      case 4:
        {
          for (int i=0; i<4; i++)
          {
            for (int j=i; j<4; j++)
            {
              if (Types[i] > Types[j])
              {
                Exchange(Types[i], Types[j]);
                Exchange(lp->pLights[i], lp->pLights[j]);
              }
            }
          }
        }
        break;
    }

    for (i=0; i<lp->nLights; i++)
    {
      m_RP.m_ShaderLightMask |= Types[i] << (SLMF_LTYPE_SHIFT + i*4);
    }
    if (lp->nLights == 1 && bStencState)
    {
      nStencState = (lp->pLights[0]->m_Flags & DLF_CASTSHADOW_VOLUME) ? GS_STENCIL : 0;
    }
    for (i=nStart; i<=nEnd; i++, slw++)
    {
      m_RP.m_CurrPass = slw;
      m_RP.m_FlagsModificators = (m_RP.m_FlagsModificators & ~7) | (slw->m_Flags & 3);
      if (slw->m_LMFlags & LMF_HASAMBIENT)
      {
        if (nPass || !bHasAmb)
          continue;
        if (slw->m_LMFlags & LMF_HASDOT3LM)
        {
          if (!m_RP.m_pShaderResources || !m_RP.m_pCurObject->m_nLMDirId)
            continue;
        }
      }
      if ((slw->m_LMFlags & LMF_USEOCCLUSIONMAP) && !bUseOccl)
        continue;

      EF_Eval_TexGen(slw);
      EF_Eval_RGBAGen(slw);
      EF_SetVertexStreams(slw->m_Pointers, 1);

      // Set all textures and HW TexGen modes for the current pass (ShadeLayer)
      curVP = slw->m_VProgram;      
      if (curVP)
        m_RP.m_FlagsPerFlush |= RBSI_USEVP;
      if (slw->mfSetTextures())
      {
        if (curVP)
          curVP->mfSet(true, slw, VPF_DONTSETMATRICES);

        EF_ApplyMatrixOps(slw->m_MatrixOps, true);

        if (curVP)
        {
  #ifndef PIPE_USE_INSTANCING
          curVP->mfSetStateMatrices();
  #endif
          curVP->mfSetVariables(false, &slw->m_VPParamsNoObj);
        }
        else
        {
          m_RP.m_CurrentVLights = 0;
          m_RP.m_PersFlags &= ~RBPF_VSNEEDSET;
        }

        // Set Pixel shaders and Register combiners for the current pass
        if (slw->m_FShader)
          slw->m_FShader->mfSet(true, slw);
        else
          m_RP.m_PersFlags &= ~RBPF_PS1NEEDSET;

        int State;
        if (m_RP.m_RendPass || (m_RP.m_ObjFlags & FOB_LIGHTPASS))
          State = slw->m_SecondRenderState;
        else
          State = slw->m_RenderState;
        if (bDstAlpha)
          State = (State & ~GS_BLEND_MASK) | GS_BLSRC_ONEMINUSDSTALPHA | GS_BLDST_ONE | GS_COLMASKONLYRGB;
        EF_SetState(State | nStencState);

  #ifndef PIPE_USE_INSTANCING

  #ifdef DO_RENDERLOG
        if (CRenderer::CV_r_log >= 3)
          Logv(SRendItem::m_RecurseLevel, "+++ Light Pass %d [%d Lights]\n", m_RP.m_RendPass, m_RP.m_nCurLights);
  #endif

        EF_Scissor(false, 0, 0, 0, 0);

        EF_Draw(ef, slw);

        m_RP.m_RendPass++;

  #else //PIPE_USE_INSTANCING
        m_RP.m_RendPass++;

        int bFogOverrided = 0;
        // Unlock all VB (if needed) and set current streams
        EF_PreDraw(slw);
        bool bFogDisable = (curVP && (curVP->m_Flags & VPFI_NOFOG)) || (m_RP.m_PersFlags & RBPF_HDR);
        bool bFogVP = (m_RP.m_PersFlags & RBPF_HDR) || (curVP && (curVP->m_Flags & VPFI_VS30ONLY));
        bFogOverrided = EF_FogCorrection(bFogDisable, bFogVP);

        int nObj = 0;
        CCObject *pSaveObj = m_RP.m_pCurObject;
        CCObject *pObj = pSaveObj;
        HRESULT h;
        while (true)
        {       
          if (nObj)
          {
            m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
            m_RP.m_FrameObject++;
            pObj = m_RP.m_pCurObject;

            if (m_RP.m_FlagsModificators & (RBMF_LMTCUSED | RBMF_BENDINFOUSED))
            {
              if (pObj->m_pLMTCBufferO && pObj->m_pLMTCBufferO->m_pVertexBuffer)
              {
                int nOffs;
                IDirect3DVertexBuffer9 *pBuf = (IDirect3DVertexBuffer9 *)pObj->m_pLMTCBufferO->m_pVertexBuffer->GetStream(VSF_GENERAL, &nOffs);
                h = m_pd3dDevice->SetStreamSource( 2, pBuf, nOffs, m_VertexSize[pObj->m_pLMTCBufferO->m_pVertexBuffer->m_vertexformat]);
              }
            }

            if (!curVP)
            {
              EF_SetObjectTransform(pObj, ef, pObj->m_ObjFlags);
              m_RP.m_CurrentVLights = 0;

              if (m_RP.m_FrameGTC == m_RP.m_Frame)
              {
                for (int nt=0; nt<gRenDev->m_TexMan->m_nCurStages; nt++)
                {
                  if (m_RP.m_pGTC[nt])
                  {
                    EF_SelectTMU(nt);
                    m_RP.m_pGTC[nt]->mfSet(true);
                  }
                }
              }
            }
          }

          EF_Scissor(false, 0, 0, 0, 0);

  #ifdef DO_RENDERLOG
          if (CRenderer::CV_r_log >= 3)
          {
            Vec3d vPos = pObj->GetTranslation();
            Logv(SRendItem::m_RecurseLevel, "+++ Light Pass %d [%d Lights] (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, lp->nLights, pObj->m_VisId, vPos[0], vPos[1], vPos[2]);
          }
  #endif

          if (curVP)
          {
            curVP->mfSetStateMatrices();
            curVP->mfSetVariables(true, &slw->m_VPParamsObj);
          }
          // Set per-object alpha-blending
          if (pObj->m_ObjFlags & FOB_HASALPHA)
            pObj->SetAlphaState(slw->m_FShader, State);

          if (slw->m_FShader)
            slw->m_FShader->mfSetVariables(true, slw->m_CGFSParamsObj);
          else
          {
            if (slw->m_RGBComps && slw->m_RGBComps->m_Comps[0] && slw->m_RGBComps->m_Comps[0]->m_bDependsOnObject)
            {
              float *vals = slw->m_RGBComps->mfGet();
              UCol color;
              color.bcolor[2] = (byte)(vals[0] * 255.0f);
              color.bcolor[1] = (byte)(vals[1] * 255.0f);
              color.bcolor[0] = (byte)(vals[2] * 255.0f);
              color.bcolor[3] = (byte)(vals[3] * 255.0f);
              m_RP.m_NeedGlobalColor = color;
            }
            EF_CommitTexStageState();
          }

          {
            //PROFILE_FRAME(Draw_ShaderIndexMesh);
            if (m_RP.m_pRE)
              m_RP.m_pRE->mfDraw(ef, slw);
            else
              EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
          }
          nObj++;
          if (nObj >= m_RP.m_MergedObjects.Num())
            break;
        }
        EF_FogRestore(bFogOverrided);
        if (!CVProgram::m_LastVP)
        {
          if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
            EF_CommitTexTransforms(false);
        }
        else
          m_RP.m_FlagsModificators &= ~(RBMF_TCM | RBMF_TCG);
        if (pSaveObj != m_RP.m_pCurObject)
        {
          m_RP.m_pCurObject = pSaveObj;
          m_RP.m_FrameObject++;
          if (!curVP)
            EF_SetObjectTransform(pSaveObj, ef, pSaveObj->m_ObjFlags);
        }
  #endif
      }
      slw->mfResetTextures();      
      EF_ApplyMatrixOps(slw->m_MatrixOps, false);
      break;
    }
  }
  m_RP.m_ShaderLightMask = 0;
  m_RP.m_nCurLightPass = 0;
  m_RP.m_PersFlags &= ~RBPF_MULTILIGHTS;
}

// Draw light passes - for each light source (used in FP shaders and in programmable pipeline shaders)
void CD3D9Renderer::EF_DrawLightPasses(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd, bool bDstAlpha)
{
  int i, l;
  int nl = 0;
  int n;

  CVProgram *curVP = NULL;
  CVProgram *newVP;
  
  PROFILE_FRAME(DrawShader_LightPasses);

  // Just single pass for transparent objects
  //if (m_RP.m_RendPass && m_RP.m_fCurOpacity != 1.0f)
  //  return;

  int nStencState = 0;
  bool bStencState = m_RP.m_pStateShader && m_RP.m_pStateShader->m_State && m_RP.m_pStateShader->m_State->m_Stencil;
  
  //return;
  // For each light source
  for (l=0; l<m_RP.m_NumActiveDLights; l++)
  {
    m_RP.m_pCurLight = m_RP.m_pActiveDLights[l];
    m_RP.m_nCurLight = m_RP.m_pCurLight->m_Id;
    SShaderPassHW* slw = &hs->m_Passes[nStart];
    nl++;

    // Ignore diffuse passes for specular only light sources
    if ((m_RP.m_pCurLight->m_Flags & DLF_LM) && slw->m_ePassType == eSHP_DiffuseLight)
    {
      if ((m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_LIGHTMAP_DIR]) || m_RP.m_pCurObject->m_nLMId)
        continue;
    }

    if (bStencState)
      nStencState = (m_RP.m_pCurLight->m_Flags & DLF_CASTSHADOW_VOLUME) ? GS_STENCIL : 0;

    bool bOnlyLightPass;
    bool bShadowMask = false;

    n = 0;
    bool bBreak = false;
    // For each layer/pass
    for (i=nStart; i<=nEnd; i++, slw++)
    {
      int msk;
      if (msk = (slw->m_LightFlags & DLF_LIGHTTYPE_MASK))
      {
        if (!(msk & (m_RP.m_pCurLight->m_Flags & DLF_LIGHTTYPE_MASK)))
          continue;
        if (slw->m_LMFlags & LMF_USEOCCLUSIONMAP)
        {
          byte *nOccl = m_RP.m_pCurObject->m_OcclLights;
          if (*(int *)nOccl == 0)
            continue;
          byte nLight = m_RP.m_nCurLight+1;
          if (nLight != nOccl[0] && nLight != nOccl[1] && nLight != nOccl[2] && nLight != nOccl[3])
            continue;
        }
        if (slw->m_LightFlags & DLF_LM)
        {
          // Ignore OnlySpecular passes if light source is non-LM
          if (!(m_RP.m_pCurLight->m_Flags & DLF_LM))
            continue;
          if ((!m_RP.m_pShaderResources || !m_RP.m_pShaderResources->m_Textures[EFTT_LIGHTMAP_DIR]) && !m_RP.m_pCurObject->m_nLMId)
            continue;
          //assert (m_RP.m_pCurLight->m_SpecColor.r!=0 || m_RP.m_pCurLight->m_SpecColor.g!=0 || m_RP.m_pCurLight->m_SpecColor.b!=0);
          // Ignore specular pass if specular is less threshold
          if (m_RP.m_pCurLight->m_SpecColor.r<=0.01f && m_RP.m_pCurLight->m_SpecColor.g<=0.01f && m_RP.m_pCurLight->m_SpecColor.b<=0.01f)
            continue;
        }
      }
      if (slw->m_LMFlags & LMF_NOBUMP)
      {
        if (!m_RP.m_pShaderResources || !m_RP.m_pShaderResources->m_Textures[EFTT_BUMP] || !(m_RP.m_pShaderResources->m_Textures[EFTT_BUMP]->m_TU.m_nFlags & FTU_NOBUMP))
          continue;
        bBreak = true;
      }
      else
      if (bBreak)
        break;

      if (EF_RejectLightPass(slw))
        continue;

      bOnlyLightPass = EF_IsOnlyLightPass(slw);

      if (CV_r_bumpselfshadow && bOnlyLightPass && m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_BUMP] && m_RP.m_pShaderResources->m_Textures[EFTT_BUMP]->m_TU.m_TexPic && m_RP.m_pShaderResources->m_Textures[EFTT_BUMP]->m_TU.m_TexPic->m_pSH)
      {
        EF_DrawLightShadowMask(l);
        curVP = NULL;
        bShadowMask = true;
      }

      if (!n)
      {
        if (!(m_RP.m_StatLightMask & (1<<m_RP.m_nCurLight)))
        {
          m_RP.m_StatLightMask |= (1<<m_RP.m_nCurLight);
          m_RP.m_StatNumLights++;
        }
      }

      n++;
      m_RP.m_StatNumLightPasses++;

      m_RP.m_CurrPass = slw;

      if (m_RP.m_pCurLightMaterial)
        m_RP.m_pCurLightMaterial->mfApply(slw->m_LMFlags | (l<<LMF_LIGHT_SHIFT));
      else
        m_RP.m_CurrentVLights = 0;

      m_RP.m_FlagsModificators = (m_RP.m_FlagsModificators & ~15) | (slw->m_Flags & 3);
      if (m_RP.m_pCurObject->m_pLMTCBufferO && m_RP.m_pCurObject->m_pLMTCBufferO->m_pVertexBuffer && m_RP.m_pCurObject->m_pLMTCBufferO->m_pVertexBuffer->m_vertexformat != VERTEX_FORMAT_TEX2F)
        m_RP.m_FlagsModificators |= RBMF_BENDINFOUSED;

      EF_Eval_TexGen(slw);
      EF_Eval_RGBAGen(slw);
      EF_SetVertexStreams(slw->m_Pointers, 1);

      // Set all textures and HW TexGen modes for the current pass (ShadeLayer)
      newVP = slw->m_VProgram;
      if (newVP)
        m_RP.m_FlagsPerFlush |= RBSI_USEVP;
      if (slw->mfSetTextures())
      {        
        // Set vertex program for the current pass if needed
        if (newVP != curVP)
        {
          if (newVP)
          {
            curVP = newVP;
            curVP->mfSet(true, slw, VPF_DONTSETMATRICES);
          }
          else
            curVP = NULL;
        }

        EF_ApplyMatrixOps(slw->m_MatrixOps, true);

        if (!curVP)
          m_RP.m_PersFlags &= ~RBPF_VSNEEDSET;
        else
        {
#ifndef PIPE_USE_INSTANCING
          curVP->mfSetStateMatrices();
#endif
          curVP->mfSetVariables(false, &slw->m_VPParamsNoObj);
        }

        // Set Pixel shaders for the current pass
        if (slw->m_FShader)
          slw->m_FShader->mfSet(true, slw);
        else
          m_RP.m_PersFlags &= ~RBPF_PS1NEEDSET;

        // Set Render states for the current pass
        int State;
        if (m_RP.m_pCurObject->m_RenderState)
          State = m_RP.m_pCurObject->m_RenderState;
        else
        {
          if (m_RP.m_RendPass || (m_RP.m_ObjFlags & FOB_LIGHTPASS))
            State = slw->m_SecondRenderState;
          else
            State = slw->m_RenderState;
          if (bDstAlpha || bShadowMask)
            State = (State & ~GS_BLEND_MASK) | GS_BLSRC_ONEMINUSDSTALPHA | GS_BLDST_ONE | GS_COLMASKONLYRGB;
        }
        State |= nStencState;
        EF_SetState(State);

#ifndef PIPE_USE_INSTANCING

#ifdef DO_RENDERLOG
        if (CRenderer::CV_r_log >= 3)
          Logv(SRendItem::m_RecurseLevel, "+++ Light Pass %d [Light %d]\n", m_RP.m_RendPass, l);
#endif

        // Set scissor rect
        if (CV_r_scissor)
        {
          if (bOnlyLightPass && m_RP.m_pCurLight->m_sWidth && m_RP.m_pCurLight->m_sHeight)
            EF_Scissor(true, m_RP.m_pCurLight->m_sX, m_RP.m_pCurLight->m_sY, m_RP.m_pCurLight->m_sWidth, m_RP.m_pCurLight->m_sHeight);
          else
            m_RP.m_pCurObject->SetScissor();
        }

        EF_Draw(ef, slw);

        m_RP.m_RendPass++;

#else //PIPE_USE_INSTANCING
        m_RP.m_RendPass++;

        int bFogOverrided = 0;
        // Unlock all VB (if needed) and set current streams
        EF_PreDraw(slw);
        bool bFogDisable = (curVP && (curVP->m_Flags & VPFI_NOFOG));
        bool bFogVP = (m_RP.m_PersFlags & RBPF_HDR) || (curVP && (curVP->m_Flags & VPFI_VS30ONLY));
        bFogOverrided = EF_FogCorrection(bFogDisable, bFogVP);

        int nObj = 0;
        CCObject *pSaveObj = m_RP.m_pCurObject;
        CCObject *pObj = pSaveObj;
        bool bLightSicssorWasSet = false;

        // Set scissor rect
        if (CV_r_scissor)
        {
          if (bOnlyLightPass && m_RP.m_pCurLight->m_sWidth && m_RP.m_pCurLight->m_sHeight)
          {
            bLightSicssorWasSet = true;
            if (!(m_RP.m_ObjFlags & FOB_NOSCISSOR))
              EF_Scissor(true, m_RP.m_pCurLight->m_sX, m_RP.m_pCurLight->m_sY, m_RP.m_pCurLight->m_sWidth, m_RP.m_pCurLight->m_sHeight);
            else
              EF_Scissor(false, 0, 0, 0, 0);
          }
          else
            pObj->SetScissor();
        }
        while (true)
        {       
          if (nObj)
          {
            // Object changed
            m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
            pObj = m_RP.m_pCurObject;
            m_RP.m_FrameObject++;
            if (CV_r_scissor && !bLightSicssorWasSet)
              pObj->SetScissor();
            if (m_RP.m_FlagsModificators & (RBMF_LMTCUSED | RBMF_BENDINFOUSED))
            {
              if (pObj->m_pLMTCBufferO && pObj->m_pLMTCBufferO->m_pVertexBuffer)
              {
                int nOffs;
                IDirect3DVertexBuffer9 *pBuf = (IDirect3DVertexBuffer9 *)pObj->m_pLMTCBufferO->m_pVertexBuffer->GetStream(VSF_GENERAL, &nOffs);
                HRESULT h = m_pd3dDevice->SetStreamSource( 2, pBuf, nOffs, m_VertexSize[pObj->m_pLMTCBufferO->m_pVertexBuffer->m_vertexformat]);
              }
            }
            if (!curVP)
            {
              EF_SetObjectTransform(pObj, ef, pObj->m_ObjFlags);
              if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_LMaterial)
                m_RP.m_pShaderResources->m_LMaterial->mfApply(slw->m_LMFlags | (l<<LMF_LIGHT_SHIFT));
              else
                m_RP.m_CurrentVLights = 0;

              if (m_RP.m_FrameGTC == m_RP.m_Frame)
              {
                for (int nt=0; nt<gRenDev->m_TexMan->m_nCurStages; nt++)
                {
                  if (m_RP.m_pGTC[nt])
                  {
                    EF_SelectTMU(nt);
                    m_RP.m_pGTC[nt]->mfSet(true);
                  }
                }
              }
            }
          }

#ifdef DO_RENDERLOG
          if (CRenderer::CV_r_log >= 3)
          {
            Vec3d vPos = pObj->GetTranslation();
            Logv(SRendItem::m_RecurseLevel, "+++ Light Pass %d [light %d] (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, l, m_RP.m_pCurObject->m_VisId, vPos[0], vPos[1], vPos[2]);
          }
#endif

          // Set per-object alpha-blending
          if (pObj->m_ObjFlags & FOB_HASALPHA)
            pObj->SetAlphaState(slw->m_FShader, State);

          if (curVP)
          {
            curVP->mfSetStateMatrices();
            curVP->mfSetVariables(true, &slw->m_VPParamsObj);
          }

          if (slw->m_FShader)
            slw->m_FShader->mfSetVariables(true, slw->m_CGFSParamsObj);
          else
          {
            if (slw->m_RGBComps && slw->m_RGBComps->m_Comps[0] && slw->m_RGBComps->m_Comps[0]->m_bDependsOnObject)
            { // Set constant color which is depends on object
              float *vals = slw->m_RGBComps->mfGet();
              UCol color;
              color.bcolor[2] = (byte)(vals[0] * 255.0f);
              color.bcolor[1] = (byte)(vals[1] * 255.0f);
              color.bcolor[0] = (byte)(vals[2] * 255.0f);
              color.bcolor[3] = (byte)(vals[3] * 255.0f);
              m_RP.m_NeedGlobalColor = color;
            }
            EF_CommitTexStageState();
          }

          {
            //PROFILE_FRAME(Draw_ShaderIndexMesh);
            if (m_RP.m_pRE)
              m_RP.m_pRE->mfDraw(ef, slw);
            else
              EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
          }
          nObj++;
          if (nObj >= m_RP.m_MergedObjects.Num())
            break;
        }
        EF_FogRestore(bFogOverrided);
        if (!CVProgram::m_LastVP)
        {
          if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
            EF_CommitTexTransforms(false);
        }
        else
          m_RP.m_FlagsModificators &= ~(RBMF_TCM | RBMF_TCG);
        if (pSaveObj != m_RP.m_pCurObject)
        {
          m_RP.m_pCurObject = pSaveObj;
          m_RP.m_FrameObject++;
          if (!curVP)
            EF_SetObjectTransform(pSaveObj, ef, pSaveObj->m_ObjFlags);
        }
#endif
      }
      
      slw->mfResetTextures();      
      EF_ApplyMatrixOps(slw->m_MatrixOps, false);
      break;
    }
  }

  //m_RP.m_nCurLight = -1;
  //m_RP.m_pCurLight = NULL;
}

// Draw bump self-shadowing pass using horizon-maps technique (used in programmable pipeline shaders only)
void CD3D9Renderer::EF_DrawLightShadowMask(int nLight)
{
  STexPic *pTX = m_RP.m_pShaderResources->m_Textures[EFTT_BUMP]->m_TU.m_TexPic;
  STexShadow *pTSH = pTX->m_pSH;
  EF_SelectTMU(0);
  pTSH->m_pHorizonTex[0]->Set();
  //pTSH->m_pHorizonTex[0]->SaveJPG("Horiz0.jpg", false);
  EF_SelectTMU(1);
  pTSH->m_pBasisTex[0]->Set();
  //pTSH->m_pBasisTex[0]->SaveJPG("Basis0.jpg", false);
  EF_SelectTMU(2);
  pTSH->m_pHorizonTex[1]->Set();
  //pTSH->m_pHorizonTex[1]->SaveJPG("Horiz1.jpg", false);
  EF_SelectTMU(3);
  pTSH->m_pBasisTex[1]->Set();
  //pTSH->m_pBasisTex[1]->SaveJPG("Basis1.jpg", false);
  CTexMan::m_nCurStages = 4;
  EF_SetState(GS_COLMASKONLYALPHA);
  if (!m_RP.m_VPTexShadow)
    m_RP.m_VPTexShadow = CVProgram::mfForName("CGVProgTexShadow");
  if (!m_RP.m_RCTexShadow)
    m_RP.m_RCTexShadow = CPShader::mfForName("CGRCTexShadow");

  m_RP.m_FlagsModificators = (m_RP.m_FlagsModificators & ~0xf) | RBMF_TANGENTSUSED;

  m_RP.m_VPTexShadow->mfSet(true, NULL, VPF_DONTSETMATRICES);
  m_RP.m_VPTexShadow->mfSetVariables(false, NULL);
  m_RP.m_RCTexShadow->mfSet(true, NULL);
#ifndef PIPE_USE_INSTANCING

#ifdef DO_RENDERLOG
  if (CRenderer::CV_r_log >= 3)
    Logv(SRendItem::m_RecurseLevel, "+++ Bump shadow pass %d [light %d]\n", m_RP.m_RendPass, nLight);
#endif

  m_RP.m_VPTexShadow->mfSetStateMatrices();
  m_RP.m_VPTexShadow->mfSetVariables(true, NULL);
  m_RP.m_RCTexShadow->mfSetVariables(true, NULL);
  EF_Draw(m_RP.m_pShader, NULL);
#else
  EF_PreDraw(NULL);

  int nObj = 0;
  CCObject *pSaveObj = m_RP.m_pCurObject;
  CCObject *pObj = pSaveObj;
  while (true)
  {       
    if (nObj)
    {
      m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
      pObj = m_RP.m_pCurObject;
      m_RP.m_FrameObject++;
    }

#ifdef DO_RENDERLOG
    if (CRenderer::CV_r_log >= 3)
    {
      Vec3d vPos = pObj->GetTranslation();
      Logv(SRendItem::m_RecurseLevel, "+++ Bump shadow pass %d [light %d] (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, nLight, m_RP.m_pCurObject->m_VisId, vPos[0], vPos[1], vPos[2]);
    }
#endif

    if (m_RP.m_VPTexShadow)
    {
      m_RP.m_VPTexShadow->mfSetStateMatrices();
      m_RP.m_VPTexShadow->mfSetVariables(true, NULL);
    }
    if (m_RP.m_RCTexShadow)
      m_RP.m_RCTexShadow->mfSetVariables(true, NULL);

    {
      //PROFILE_FRAME(Draw_ShaderIndexMesh);
      if (m_RP.m_pRE)
        m_RP.m_pRE->mfDraw(m_RP.m_pShader, NULL);
      else
        EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
    }
    nObj++;
    if (nObj >= m_RP.m_MergedObjects.Num())
      break;
  }
  if (pSaveObj != m_RP.m_pCurObject)
  {
    m_RP.m_pCurObject = pSaveObj;
    m_RP.m_FrameObject++;
  }
#endif

  CD3D9TexMan::BindNULL(1);
}

// Draw sub-surface scattering passes (used in programmable pipeline shaders only)
void CD3D9Renderer::EF_DrawSubsurfacePasses(SShaderTechnique *hs, SShader *ef)
{
  int l;

  // For each light source
  for (l=0; l<m_RP.m_NumActiveDLights; l++)
  {
    m_RP.m_pCurLight = m_RP.m_pActiveDLights[l];
    m_RP.m_nCurLight = m_RP.m_pCurLight->m_Id;

    // Ignore diffuse passes for specular only light sources
    if ((m_RP.m_pCurLight->m_Flags & DLF_LM))
    {
      if ((m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_LIGHTMAP_DIR]) || m_RP.m_pCurObject->m_nLMId)
        continue;
    }

    //// ignore sun ?
    //if ((m_RP.m_pCurLight->m_Flags & DLF_SUN))
    //{
    //  continue;
    //}     

    EF_SetState(GS_BLSRC_ONE | GS_BLDST_ONE);      
    //EF_SetState(GS_BLSRC_ONEMINUSDSTALPHA | GS_BLDST_ONE);    


    CVProgram *m_VP;
    CPShader  *m_RC; 
    if (!m_RP.m_VPSubSurfaceScatering)
      m_RP.m_VPSubSurfaceScatering= CVProgram::mfForName("CGVProgSubSurface");
    if (!m_RP.m_RCSubSurfaceScatering)
      m_RP.m_RCSubSurfaceScatering= CPShader::mfForName("CGRCSubSurface");
    if (!m_RP.m_VPSubSurfaceScatering_pp)
      m_RP.m_VPSubSurfaceScatering_pp= CVProgram::mfForName("CGVProgSubSurface_pp");
    if (!m_RP.m_RCSubSurfaceScatering_pp)
      m_RP.m_RCSubSurfaceScatering_pp= CPShader::mfForName("CGRCSubSurface_pp");

    // setup texture stages textures
    STexPic *pSubSurfaceTex = m_RP.m_pShaderResources->m_Textures[EFTT_SUBSURFACE]->m_TU.m_TexPic;    
    EF_SelectTMU(0);
    pSubSurfaceTex->Set(); 

    STexPic *pBumpTex = m_RP.m_pShaderResources->m_Textures[EFTT_BUMP]->m_TU.m_TexPic;
    if(pBumpTex)
    {
      EF_SelectTMU(1);
      SetLodBias(-1.5);    
      pBumpTex->Set();       
    }        

    STexPic *pNormalizationCubeMapTex=gRenDev->m_TexMan->m_Text_NormalizeCMap;
    EF_SelectTMU(2);
    pNormalizationCubeMapTex->Set();      

    if(CRenderer::CV_r_subsurface_type==0 || !pBumpTex)
    {
      m_VP=m_RP.m_VPSubSurfaceScatering;
      m_RC=m_RP.m_RCSubSurfaceScatering;
    }

    // make sure to only use per-pixel subsurface scatering, when model has normal map texture
    if(CRenderer::CV_r_subsurface_type==1 && pBumpTex)
    {      
      m_VP=m_RP.m_VPSubSurfaceScatering_pp;
      m_RC=m_RP.m_RCSubSurfaceScatering_pp;
    }

    if(!m_VP || !m_RC)
    {
      return;
    }

    m_VP->mfSet(true, NULL);  
    m_VP->mfSetVariables(false, NULL);
    m_RC->mfSet(true, NULL);
    m_RC->mfSetVariables(false, NULL);

#ifndef PIPE_USE_INSTANCING

#ifdef DO_RENDERLOG
    if (CRenderer::CV_r_log >= 3)
      Logv(SRendItem::m_RecurseLevel, "+++ Subsurface scattering pass %d [light %d]\n", m_RP.m_RendPass, l);
#endif

    m_VP->mfSetVariables(true, NULL);
    m_RC->mfSetVariables(true, NULL);
    EF_Draw(m_RP.m_pShader, NULL);
#else
    EF_PreDraw(NULL);

    int nObj = 0;
    CCObject *pSaveObj = m_RP.m_pCurObject;
    CCObject *pObj = pSaveObj;
    while (true)
    {       
      if (nObj)
      {
        m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
        pObj = m_RP.m_pCurObject;
        m_RP.m_FrameObject++;
      }

#ifdef DO_RENDERLOG
      if (CRenderer::CV_r_log >= 3)
      {
        Vec3d vPos = pObj->GetTranslation();
        Logv(SRendItem::m_RecurseLevel, "+++ Subsurface scattering pass %d [light %d] (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, l, m_RP.m_pCurObject->m_VisId, vPos[0], vPos[1], vPos[2]);
      }
#endif

      m_VP->mfSetStateMatrices();
      m_VP->mfSetVariables(true, NULL);
      m_RC->mfSetVariables(true, NULL);

      {
        //PROFILE_FRAME(Draw_ShaderIndexMesh);
        if (m_RP.m_pRE)
          m_RP.m_pRE->mfDraw(m_RP.m_pShader, NULL);
        else
          EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
      }
      nObj++;
      if (nObj >= m_RP.m_MergedObjects.Num())
        break;
    }
    if (pSaveObj != m_RP.m_pCurObject)
    {
      m_RP.m_pCurObject = pSaveObj;
      m_RP.m_FrameObject++;
    }

    EF_SelectTMU(1);
    SetLodBias(0);
#endif
  }
  CD3D9TexMan::BindNULL(1);
}

// Flush HW shader technique (used in FP shaders and in programmable pipeline shaders)
void CD3D9Renderer::EF_FlushHW()
{
  SShader *ef = m_RP.m_pShader;
  int i;

  int nPolys = m_nPolygons;

  if (m_RP.m_ObjFlags & FOB_SELECTED)
  {
    int nnn = 0;
  }
 
  if (m_RP.m_pRE || (m_RP.m_RendNumIndices && m_RP.m_RendNumVerts))
  {
    if (m_RP.m_pShaderResources)
      m_RP.m_pCurLightMaterial = m_RP.m_pShaderResources->m_LMaterial;
    else
    if (ef->m_Flags2 & EF2_USELIGHTMATERIAL)
      m_RP.m_pCurLightMaterial = &m_RP.m_DefLightMaterial;
    else
      m_RP.m_pCurLightMaterial = NULL;

    // Set Z-Bias
    if (ef->m_Flags & EF_POLYGONOFFSET)
    {
      if (m_d3dCaps.RasterCaps & (D3DPRASTERCAPS_DEPTHBIAS | D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS))
      {
        float fOffs = (float)CV_d3d9_decaloffset;
        m_pd3dDevice->SetRenderState( D3DRS_DEPTHBIAS, *(DWORD*)(&fOffs) );
      }
    }

    SShaderTechnique *hs = m_RP.m_pCurTechnique;
    if (!hs)
      return;

    if (m_RP.m_pRE)
    {
      SRenderShaderResources *Res = m_RP.m_pShaderResources;
      int nFlags = hs->m_Flags;
      if (Res && Res->m_bNeedNormals)
        nFlags |= SHPF_NORMALS;
      if (ef->m_eSort == eS_TerrainLightPass || ef->m_eSort == eS_Terrain)
        nFlags |= FHF_TERRAIN;
      m_RP.m_pRE->mfCheckUpdate(ef->m_VertexFormatId, nFlags);
    }

    if (ef->m_Flags & EF_NEEDNORMALS)
      EF_EvalNormalsRB(ef);
    EF_Eval_DeformVerts(ef->m_Deforms);

    // Set culling mode
    if (!(m_RP.m_FlagsPerFlush & RBSI_NOCULL))
    {
      if (hs->m_eCull != -1)
        D3DSetCull(hs->m_eCull);
      else
        D3DSetCull(ef->m_eCull);
    }

    EF_SetVertexStreams(hs->m_Pointers, 1);
    EF_ApplyMatrixOps(hs->m_MatrixOps, true);

    // Draw shader passes
    bool bLights = false;
    if (hs->m_Passes.Num())
    {
      int nStart = 0;
      EShaderPassType eShPass = m_SHPTable[hs->m_Passes[0].m_ePassType];

      for (i=1; i<hs->m_Passes.Num(); i++)
      {
        if (m_SHPTable[hs->m_Passes[i].m_ePassType] != eShPass)
        {
          int nEnd = i-1;
          bLights |= EF_DrawPasses(eShPass, hs, ef, nStart, nEnd);
          i = nEnd+1;
          nStart = i;
          if (i >= hs->m_Passes.Num())
            break;
          eShPass = m_SHPTable[hs->m_Passes[i].m_ePassType];
        }
      }
      if (i-nStart > 0)
      {
        int nEnd = i-1;
        bLights |= EF_DrawPasses(eShPass, hs, ef, nStart, nEnd);
      }
    }
    if (bLights)
    {
      if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_SUBSURFACE])
        EF_DrawSubsurfacePasses(hs, ef);
    }

    // Restore matrix transform states
    EF_ApplyMatrixOps(hs->m_MatrixOps, false);

    // Reset Z-Bias
    if (ef->m_Flags & EF_POLYGONOFFSET)
    {
      if (m_d3dCaps.RasterCaps & (D3DPRASTERCAPS_DEPTHBIAS | D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS))
        m_pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, 0);
    }

    // Draw detail texture passes
    if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_DETAIL_OVERLAY] && CV_r_detailtextures)
      EF_DrawDetailOverlayPasses();

    // Draw volumetric fog passes
    if (m_RP.m_pFogVolume && CV_r_VolumetricFog)
      EF_DrawFogOverlayPasses();
  }
}

// Set/Restore shader resources overrided states
bool CD3D9Renderer::EF_SetResourcesState(bool bSet)
{
  if (m_RP.m_pShader->m_Flags2 & EF2_IGNORERESOURCESTATES)
    return true;

  bool bRes = true;
  if (bSet)
  {
    if (m_RP.m_pShaderResources->m_ResFlags & MTLFLAG_2SIDED)
    {
      D3DSetCull(eCULL_None);
      m_RP.m_FlagsPerFlush |= RBSI_NOCULL;
    }
    if ((m_RP.m_pShaderResources->m_AlphaRef) && !(m_RP.m_ObjFlags & FOB_FOGPASS))
    {
      if (!(m_CurState & GS_ALPHATEST_MASK))
        m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
      m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)(m_RP.m_pShaderResources->m_AlphaRef*255.0f));
      m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
      m_RP.m_FlagsPerFlush |= RBSI_ALPHATEST;
      m_CurState |= GS_ALPHATEST_GEQUAL128;
    }
    if (m_RP.m_pShaderResources->m_Opacity != 1.0f)
    {
      int State = m_CurState &= ~GS_BLEND_MASK;
      byte bCol = (byte)(m_RP.m_pShaderResources->m_Opacity*255.0f);
      if (m_RP.m_pShaderResources->m_ResFlags & MTLFLAG_ADDITIVE)
      {
        State |= GS_BLSRC_ONE | GS_BLDST_ONE;
        State &= ~GS_DEPTHWRITE;
        m_RP.m_FlagsPerFlush |= RBSI_GLOBALRGB;
        m_RP.m_NeedGlobalColor.bcolor[0] = bCol;
        m_RP.m_NeedGlobalColor.bcolor[1] = bCol;
        m_RP.m_NeedGlobalColor.bcolor[2] = bCol;
      }
      else
      {
        State |= GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;
        State &= ~GS_DEPTHWRITE;
        m_RP.m_FlagsPerFlush |= RBSI_GLOBALALPHA;
        m_RP.m_NeedGlobalColor.bcolor[3] = bCol;
      }
      EF_SetState(State);
      m_RP.m_ResourceState = State;

      m_RP.m_fCurOpacity = m_RP.m_pShaderResources->m_Opacity;

      m_RP.m_FlagsPerFlush |= RBSI_ALPHABLEND | RBSI_DEPTHWRITE | RBSI_ALPHAGEN;
      m_RP.m_ObjFlags &= ~FOB_LIGHTPASS;
    }
  }
  else
  {
    if ((m_RP.m_pShaderResources->m_AlphaRef) && !(m_RP.m_ObjFlags & FOB_FOGPASS))
    {
      switch (m_CurState & GS_ALPHATEST_MASK)
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

// Set overrided shader states from State shader
void CD3D9Renderer::EF_SetStateShaderState()
{
  SShader *ef = m_RP.m_pStateShader;
  SEfState *st = ef->m_State;

  if (st->m_bClearStencil)
    m_pd3dDevice->Clear(0, NULL, D3DCLEAR_STENCIL, 0, 1.0f, 0);
  
  if (st->m_Stencil)
    st->m_Stencil->mfSet();
  if (st->m_Flags & ESF_NOCULL)
  {
    D3DSetCull(eCULL_None);
    m_RP.m_FlagsPerFlush |= RBSI_NOCULL;
  }
  else
  if (st->m_Flags & ESF_CULLFRONT)
  {
    D3DSetCull(eCULL_Front);
    m_RP.m_FlagsPerFlush |= RBSI_NOCULL;
  }

  if (st->m_Flags & ESF_STATE)
  {
    EF_SetState(st->m_State);
    if (st->m_State & GS_NODEPTHTEST)
      m_RP.m_FlagsPerFlush |= RBSI_DEPTHTEST;
    if (st->m_State & (GS_DEPTHFUNC_EQUAL | GS_DEPTHFUNC_GREAT))
      m_RP.m_FlagsPerFlush |= RBSI_DEPTHFUNC;
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
  {
    Exchange(Col.bcolor[0], Col.bcolor[3]);
    m_RP.m_NeedGlobalColor = Col;
  }
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

// Reset overrided shader states from State shader
void CD3D9Renderer::EF_ResetStateShaderState()
{
  SShader *ef = m_RP.m_pStateShader;
  SEfState *st = ef->m_State;

  if (st->m_Flags & ESF_COLORMASK)
    m_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xf);

  if (st->m_Flags & ESF_POLYLINE)
    m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
}

// Flush current shader
void CD3D9Renderer::EF_FlushShader()
{
  SShader *ef = m_RP.m_pShader;
  int i;
  SShaderPass *slw;

  if (m_RP.m_pRE)  // Without buffer fill
  {
    EF_InitEvalFuncs(1);  // VP
    if (CV_r_rb_merge < 0)
      return;
  }
  else
  { // Buffer filling
    EF_InitEvalFuncs(0);
    if (!m_RP.m_RendNumIndices)
      return;

    if ((ef->m_Flags & EF_FOGSHADER) && !m_RP.m_pFogVolume)
      return;

    if (m_RP.m_RendNumIndices > m_RP.m_MaxTris*3)
    {
      iLog->Log("CD3D9Renderer::EF_FlushShader: Shader MaxTris hit for batched RenderElements\n");
      m_RP.m_RendNumIndices = m_RP.m_MaxTris*3;
    }
    if (m_RP.m_RendNumVerts > m_RP.m_MaxVerts)
    {
      iLog->Log("CD3D9Renderer::EF_FlushShader: Shader MaxVerts hit for batched RenderElements\n");
      m_RP.m_RendNumVerts = m_RP.m_MaxVerts;
    }
  }

  m_RP.m_ResourceState = 0;
  if (m_RP.m_pShaderResources)
  {
    if (!EF_SetResourcesState(true))
      return;
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
    if (bDraw2D)
      Set2DMode(false,800,600);
    return;
  }

  // In common shaders we don't have pixel or vertex shaders
  m_RP.m_PersFlags &= ~(RBPF_PS1NEEDSET | RBPF_VSNEEDSET);
  CD3D9TexMan::BindNULL(1);
  EF_SelectTMU(0);
  EF_Scissor(false, 0, 0, 0, 0);

  if (ef->m_Flags & EF_NEEDNORMALS)
    EF_EvalNormalsRB(ef);
  EF_Eval_DeformVerts(ef->m_Deforms);

  if (ef->m_Flags & EF_POLYGONOFFSET)
  {
    if (m_d3dCaps.RasterCaps & (D3DPRASTERCAPS_DEPTHBIAS | D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS))
    {
      float fOffs = (float)CV_d3d9_decaloffset;
      m_pd3dDevice->SetRenderState( D3DRS_DEPTHBIAS, *(DWORD*)&fOffs );
    }
  }

  if (!(m_RP.m_FlagsPerFlush & RBSI_NOCULL))
    D3DSetCull(ef->m_eCull);

  int nPolys = m_nPolygons;

  if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_LMaterial)
    m_RP.m_pShaderResources->m_LMaterial->mfApply(ef->m_LMFlags);
  else
    m_RP.m_CurrentVLights = 0;

  m_RP.m_StatNumPasses += ef->m_Passes.Num();
  slw = &ef->m_Passes[0];
  for (i=0; i<ef->m_Passes.Num(); i++, slw++)
  {
    EF_Eval_TexGen(slw);
    EF_Eval_RGBAGen(slw);

    // Set all textures and HW TexGen modes for the current pass (ShadeLayer)
    if (slw->mfSetTextures())
    {
      int nState;

      // Set Render states for the current pass
      if (m_RP.m_RendPass || (m_RP.m_pCurObject->m_ObjFlags & FOB_LIGHTPASS))
        nState = slw->m_SecondRenderState;
      else
        nState = slw->m_RenderState;
      EF_SetState(nState);

#ifndef PIPE_USE_INSTANCING

#ifdef DO_RENDERLOG
      if (CRenderer::CV_r_log >= 3)
        Logv(SRendItem::m_RecurseLevel, "+++ Pass %d \n", m_RP.m_RendPass);
#endif

      // Set scissor rect
      m_RP.m_pCurObject->SetScissor();

      EF_Draw(ef, slw);

      m_RP.m_RendPass++;

#else //PIPE_USE_INSTANCING
      m_RP.m_RendPass++;

      int bFogOverrided = 0;
      // Unlock all VB (if needed) and set current streams
      EF_PreDraw(slw);
      if (m_FS.m_bEnable)
        bFogOverrided = EF_FogCorrection(false, false);

      int nObj = 0;
      CCObject *pSaveObj = m_RP.m_pCurObject;
      CCObject *pObj = pSaveObj;
      bool bLightSicssorWasSet = false;

      while (true)
      {       
        if (nObj)
        {
          // Object changed
          m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
          pObj = m_RP.m_pCurObject;
          m_RP.m_FrameObject++;
          EF_SetObjectTransform(pObj, ef, pObj->m_ObjFlags);
          if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_LMaterial)
            m_RP.m_pShaderResources->m_LMaterial->mfApply(ef->m_LMFlags);
        }
        // Set scissor rect
        if (!(m_RP.m_ObjFlags & FOB_NOSCISSOR))
          pObj->SetScissor();
        else
          EF_Scissor(false, 0, 0, 0, 0);

        // Set per-object alpha blending state
        if (pObj->m_ObjFlags & FOB_HASALPHA)
          pObj->SetAlphaState(NULL, nState);

        // Commit per-texture states / color ops
        EF_CommitTexStageState();

#ifdef DO_RENDERLOG
        if (CRenderer::CV_r_log >= 3)
        {
          Vec3d vPos = pObj->GetTranslation();
          Logv(SRendItem::m_RecurseLevel, "+++ Pass %d (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, m_RP.m_pCurObject->m_VisId, vPos[0], vPos[1], vPos[2]);
        }
#endif
        {
          //PROFILE_FRAME(Draw_ShaderIndexMesh);
          if (m_RP.m_pRE)
            m_RP.m_pRE->mfDraw(ef, slw);
          else
            EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
        }
        nObj++;
        if (nObj >= m_RP.m_MergedObjects.Num())
          break;
      }
      EF_FogRestore(bFogOverrided);
      if (!CVProgram::m_LastVP)
      {
        if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
          EF_CommitTexTransforms(false);
      }
      else
        m_RP.m_FlagsModificators &= ~(RBMF_TCM | RBMF_TCG);
      if (pSaveObj != m_RP.m_pCurObject)
      {
        m_RP.m_pCurObject = pSaveObj;
        m_RP.m_FrameObject++;
        EF_SetObjectTransform(pSaveObj, ef, pSaveObj->m_ObjFlags);
      }
#endif
    }
    slw->mfResetTextures();      
  }

  if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_DETAIL_OVERLAY] && CV_r_detailtextures)
    EF_DrawDetailOverlayPasses();

  if (m_RP.m_pFogVolume && CV_r_VolumetricFog)
    EF_DrawFogOverlayPasses();

  if (ef->m_Flags & EF_POLYGONOFFSET)
  {
    if (m_d3dCaps.RasterCaps & (D3DPRASTERCAPS_DEPTHBIAS | D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS))
      m_pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, 0);
  }

  if (m_RP.m_pStateShader && m_RP.m_pStateShader->m_State)
    EF_ResetStateShaderState();
  if (m_RP.m_pShaderResources)
    EF_SetResourcesState(false);

  if (bDraw2D)
    Set2DMode(false,800,600);
}

// Flush current render item
void CD3D9Renderer::EF_Flush()
{
  CD3D9Renderer *rd = gcpRendD3D;
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

  //if ((int)rd->m_RP.m_pRE != 0xed45090)
  //  return;

  /*{
    Vec3d vObj = rd->m_RP.m_pCurObject->GetTranslation();
    Vec3d vv = Vec3d(1170.025, 1396.100, 74.500);
    //if ((vv-vObj).GetLength() > 0.1f)
    //  return;
  }*/

  if (ef->m_nPreprocess)
  {
    if(ef->m_nPreprocess & FSPR_PORTAL)
    {
      if (CV_r_portals && (rd->m_RP.m_PersFlags & (RBPF_DRAWPORTAL | RBPF_DRAWMIRROR)))
        return;
    }
    if(ef->m_nPreprocess & FSPR_HEATVIS)
    {
      if (rd->m_RP.m_PersFlags & RBPF_DRAWHEATMAP)
        rd->m_TexMan->EndHeatMap();
    }
    if(ef->m_nPreprocess & FSPR_SCANSCR)
    {
      if (rd->m_RP.m_PersFlags & RBPF_DRAWSCREENMAP)
        rd->m_TexMan->EndScreenMap();
    }
    if(ef->m_nPreprocess & FSPR_SCREENTEXMAP)
    {
      if (rd->m_RP.m_PersFlags & RBPF_DRAWSCREENTEXMAP)
      {
        rd->m_TexMan->EndScreenTexMap();          
        // reset flag state
        rd->m_RP.m_PersFlags&= ~RBPF_IGNOREREFRACTED;
      }
    }
  }

#ifdef DO_RENDERLOG
  if (rd->m_LogFile && CV_r_log == 3)
    rd->Logv(SRendItem::m_RecurseLevel, ".. Start Flush: '%s' ..\n", ef->m_Name.c_str());
#endif

  rd->m_RP.m_StatNumLightPasses = 0;
  rd->m_RP.m_StatNumLights = 0;
  rd->m_RP.m_StatNumPasses = 0;
  rd->m_RP.m_StatLightMask = 0;

  rd->m_RP.m_PS.m_NumRendBatches++;
#ifdef PIPE_USE_INSTANCING
if (rd->m_RP.m_MergedObjects.Num())
    rd->m_RP.m_PS.m_NumRendItems += rd->m_RP.m_MergedObjects.Num();
  else
#endif
    rd->m_RP.m_PS.m_NumRendItems++;

  CCObject *obj = rd->m_RP.m_pCurObject;
  int VPMId, IMId;
  if (!(rd->m_RP.m_ObjFlags & FOB_TRANS_MASK))
  {
    VPMId = rd->m_RP.m_pCurObject->m_VPMatrixId;
    IMId = rd->m_RP.m_pCurObject->m_InvMatrixId;
    rd->m_RP.m_pCurObject->m_VPMatrixId = 0;
    rd->m_RP.m_pCurObject->m_InvMatrixId = 0;
  }

  /*if (ef->m_eSort == eS_Water)
  {
    obj->m_DynLMMask |= 1;
  }*/
  rd->m_RP.m_DynLMask = CV_r_hwlights ? obj->m_DynLMMask : 0;

  if (!(ef->m_Flags & EF_HASVSHADER))
    rd->EF_SetObjectTransform(obj, ef, rd->m_RP.m_ObjFlags);

  if (CV_r_showlight->GetString()[0] != '0')
  {
    for (int i=0; i<rd->m_RP.m_DLights[SRendItem::m_RecurseLevel].Num(); i++)
    {
      CDLight *dl = rd->m_RP.m_DLights[SRendItem::m_RecurseLevel][i];
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

#ifdef DO_RENDERLOG
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
#endif

  rd->EF_FlushShader();

  if (rd->m_RP.m_DynLMask)
    rd->m_RP.m_PS.m_NumLitShaders++;

  if (!(rd->m_RP.m_ObjFlags & FOB_TRANS_MASK))
  {
    rd->m_RP.m_pCurObject->m_VPMatrixId = VPMId;
    rd->m_RP.m_pCurObject->m_InvMatrixId = IMId;
  }

#ifdef DO_RENDERLOG
  nNumPolys = rd->m_nPolygons - nNumPolys;

  if (bProfile)
  {
    unticks(time0);
    time0 = time0*1000.0*g_SecondsPerCycle;
  }

  if (CRenderer::CV_r_profileshaders)
  {
    if (CRenderer::CV_r_profileshaders == 1 || (rd->m_RP.m_ObjFlags & FOB_SELECTED))
    {
      SProfInfo pi;
      pi.Time = time0;
      pi.NumPolys = nNumPolys;
      pi.ef = ef;
      pi.m_nItems = 0;
      rd->m_RP.m_Profile.AddElem(pi);
    }
  }


  if (rd->m_LogFile)
  {
    if (CV_r_log == 4 && rd->m_RP.m_DynLMask)
    {
      for (int n=0; n<rd->m_RP.m_DLights[SRendItem::m_RecurseLevel].Num(); n++)
      {
        CDLight *dl = rd->m_RP.m_DLights[SRendItem::m_RecurseLevel][n];
        if (rd->m_RP.m_DynLMask & (1<<n))
          rd->Logv(SRendItem::m_RecurseLevel, "   Light %d (\"%s\")\n", n, dl->m_Name ? dl->m_Name : "<Unknown>");
      }
    }

    char *str;
    if (ef->m_HWTechniques.Num())
      str = "FlushHW";
    else
      str = "Flush";
    rd->Logv(SRendItem::m_RecurseLevel, "%s: '%s', (St: %s) Id:%d, ResId:%d, Obj:%d, Tech: %d, Cp: %d, Fog:%d, VF:%d, NL:%d, LPas:%d, Pas:%d (time: %f, %d polys)\n", str, ef->m_Name.c_str(), rd->m_RP.m_pStateShader ? rd->m_RP.m_pStateShader->m_Name.c_str() : "NULL", ef->m_Id, rd->m_RP.m_pShaderResources ? rd->m_RP.m_pShaderResources->m_Id : -1, rd->m_RP.m_pCurObject->m_VisId, rd->m_RP.m_pCurTechnique ? rd->m_RP.m_pCurTechnique->m_Id : -1, rd->m_RP.m_ClipPlaneEnabled, rd->m_RP.m_pFogVolume ? (rd->m_RP.m_pFogVolume-&rd->m_RP.m_FogVolumes[0]) : 0, ef->m_VertexFormatId, rd->m_RP.m_StatNumLights, rd->m_RP.m_StatNumLightPasses, rd->m_RP.m_StatNumPasses, time0, nNumPolys);
    if (rd->m_RP.m_ObjFlags & FOB_SELECTED)
    {
      if (rd->m_RP.m_FlagsPerFlush & RBSI_ALPHATEST)
        rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x), LM: %d, (AT) (Selected)\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_DynLMask);
      else
      if (rd->m_RP.m_FlagsPerFlush & RBSI_ALPHABLEND)
        rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x) (AB), LM: %d (Dist: %.3f) (Selected)\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_DynLMask, rd->m_RP.m_pRE ? rd->m_RP.m_pRE->mfDistanceToCameraSquared(*gRenDev->m_RP.m_pCurObject) : 0);
      else
        rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x), RE: 0x%x, LM: 0x%x (Selected)\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_pRE, rd->m_RP.m_DynLMask);
    }
    else
    {
      if (rd->m_RP.m_FlagsPerFlush & RBSI_ALPHATEST)
        rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x), LM: %d, (AT)\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_DynLMask);
      else
      if (rd->m_RP.m_FlagsPerFlush & RBSI_ALPHABLEND)
        rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x) (AB), LM: %d (Dist: %.3f)\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_DynLMask, rd->m_RP.m_pRE ? rd->m_RP.m_pRE->mfDistanceToCameraSquared(*gRenDev->m_RP.m_pCurObject) : 0);
      else
        rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x), RE: 0x%x, LM: 0x%x\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_pRE, rd->m_RP.m_DynLMask);
    }
  }
#endif
}

// Process all render item lists
void CD3D9Renderer::EF_EndEf3D(int nFlags)
{
  double time0 = 0;
  ticks(time0);

  if (m_bDeviceLost)
  {
    SRendItem::m_RecurseLevel--;
    return;
  }

  assert(SRendItem::m_RecurseLevel >= 1);
  if (SRendItem::m_RecurseLevel < 1)
  {
    iLog->Log("Error: CRenderer::EF_EndEf3D without CRenderer::EF_StartEf");
    return;
  }

  if (CV_r_nodrawshaders == 1)
  {
    SetClearColor(Vec3d(0,0,0));
    EF_ClearBuffers(false, false, NULL);
    SRendItem::m_RecurseLevel--;
    return;
  }

#ifdef USE_HDR
  bool bUseHDR = false;
  if ((nFlags & SHDF_ALLOWHDR) && SRendItem::m_RecurseLevel == 1 && (m_Features & RFT_HW_HDR) && CV_r_hdrrendering && !(m_RP.m_PersFlags & RBPF_MAKESPRITE))
    bUseHDR = true;
#endif
  m_RP.m_PersFlags &= ~(RBPF_DRAWNIGHTMAP | RBPF_DRAWHEATMAP);
  m_RP.m_RealTime = iTimer->GetCurrTime();
  
  if (CV_r_fullbrightness)  
  {
    m_RP.m_NeedGlobalColor.dcolor = -1;
    m_RP.m_FlagsPerFlush |= RBSI_RGBGEN | RBSI_ALPHAGEN;
  }

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

#ifdef USE_HDR
  if (bUseHDR)
    EF_AddEf(0, m_RP.m_pREHDR, CShader::m_ShaderHDRProcess, NULL, NULL, 0, 0, EFSLIST_LAST | eS_HDR);
#endif

  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_PREPROCESS_ID] = SRendItem::m_RendItems[EFSLIST_PREPROCESS_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_STENCIL_ID] = SRendItem::m_RendItems[EFSLIST_STENCIL_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_GENERAL_ID] = SRendItem::m_RendItems[EFSLIST_GENERAL_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_UNSORTED_ID] = SRendItem::m_RendItems[EFSLIST_UNSORTED_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_DISTSORT_ID] = SRendItem::m_RendItems[EFSLIST_DISTSORT_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_LAST_ID] = SRendItem::m_RendItems[EFSLIST_LAST_ID].Num();

#ifdef USE_HDR
  CD3D9TexMan *tm = (CD3D9TexMan *)m_TexMan;
  if (bUseHDR)
  {
    if (!tm->m_Text_HDRTarget || tm->m_Text_HDRTarget->m_Width != m_width || tm->m_Text_HDRTarget->m_Height != m_height)
      tm->GenerateHDRMaps();
    // Set float render target for HDR rendering
		if( tm->m_HDR_RT_FSAA )
		{
			assert(!m_pHDRTargetSurf);
			if (!m_pHDRTargetSurf)
			{
				m_pHDRTargetSurf = tm->m_HDR_RT_FSAA;
				m_pHDRTargetSurf->AddRef();
			}
			HRESULT hr = m_pd3dDevice->SetRenderTarget( 0, m_pHDRTargetSurf );				
		}
		else
		{
			STexPic *tp = tm->m_Text_HDRTarget;
			LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)tp->m_RefTex.m_VidTex;
			assert(pTexture);
			assert(!m_pHDRTargetSurf);
			if (!m_pHDRTargetSurf)
				pTexture->GetSurfaceLevel(0, &m_pHDRTargetSurf);
			assert(m_pHDRTargetSurf);
			//D3DSURFACE_DESC dtdsdRT;
			//m_pHDRTargetSurf->GetDesc(&dtdsdRT);
			HRESULT hr = m_pd3dDevice->SetRenderTarget(0, m_pHDRTargetSurf);
		}
    if (m_nHDRType == 2)
    {
      //tp = tm->m_Text_HDRTarget_K;
      //LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)tp->m_RefTex.m_VidTex;
      //if (!m_pHDRTargetSurf_K)
      //  pTexture->GetSurfaceLevel(0, &m_pHDRTargetSurf_K);
      //hr = m_pd3dDevice->SetRenderTarget(1, m_pHDRTargetSurf_K);
    }
    //hr = m_pd3dDevice->SetDepthStencilSurface(m_pTempZBuffer);
    //m_pCurZBuffer = m_pTempZBuffer;
    m_RP.m_PersFlags |= RBPF_HDR;
  }
  else
  if (!CV_r_hdrrendering && tm->m_Text_HDRTarget)
    tm->DestroyHDRMaps();
#endif

  EF_RenderPipeLine(EF_Flush);

  EF_DrawDebugTools();
  EF_RemovePolysFromScene();
  SRendItem::m_RecurseLevel--;

  unticks(time0);
  m_RP.m_PS.m_fFlushTime += (float)(time0*1000.0*g_SecondsPerCycle);
}

// Process all render item lists
void CD3D9Renderer::EF_RenderPipeLine(void (*RenderFunc)())
{
  EF_PipeLine(SRendItem::m_StartRI[SRendItem::m_RecurseLevel-1][EFSLIST_PREPROCESS_ID], SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_PREPROCESS_ID], EFSLIST_PREPROCESS_ID, RenderFunc);  // Preprocess and probably sky
  if (!(m_RP.m_PersFlags & RBPF_IGNORERENDERING))
  {
    EF_PipeLine(SRendItem::m_StartRI[SRendItem::m_RecurseLevel-1][EFSLIST_STENCIL_ID], SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_STENCIL_ID], EFSLIST_STENCIL_ID, RenderFunc);   // Unsorted list for indoor
    EF_PipeLine(SRendItem::m_StartRI[SRendItem::m_RecurseLevel-1][EFSLIST_GENERAL_ID], SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_GENERAL_ID], EFSLIST_GENERAL_ID, RenderFunc);    // Sorted list without preprocess
    EF_PipeLine(SRendItem::m_StartRI[SRendItem::m_RecurseLevel-1][EFSLIST_UNSORTED_ID], SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_UNSORTED_ID], EFSLIST_UNSORTED_ID, RenderFunc); // Unsorted list
    EF_PipeLine(SRendItem::m_StartRI[SRendItem::m_RecurseLevel-1][EFSLIST_DISTSORT_ID], SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_DISTSORT_ID], EFSLIST_DISTSORT_ID, RenderFunc);   // Sorted by distance elements
    if (SRendItem::m_RecurseLevel <= 1)
      EF_PipeLine(SRendItem::m_StartRI[SRendItem::m_RecurseLevel-1][EFSLIST_LAST_ID], SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_LAST_ID], EFSLIST_LAST_ID, RenderFunc);       // Sorted list without preprocess of all fog passes and screen shaders
  }
  else
    m_RP.m_PersFlags &= ~RBPF_IGNORERENDERING;
}

static char *sDescList[] = {"General", "DistSort", "Preprocess", "Stencil", "Last", "Unsorted"};

// Process render items list [nList] from [nums] to [nume]
// 1. Sorting of the list
// 2. Preprocess shaders handling
// 3. Process sorted ordered list of render items
void CD3D9Renderer::EF_PipeLine(int nums, int nume, int nList, void (*RenderFunc)())
{
  int i;
  SShader *pShader, *pCurShader, *pShaderState, *pCurShaderState;
  SRenderShaderResources *pRes, *pCurRes;
  int nObject, nCurObject;
  int nFog, nCurFog;
  
  if (nume-nums < 1)
    return;
  CheckDeviceLost();

  m_RP.m_pRenderFunc = RenderFunc;

  m_RP.m_nCurLightParam = -1;
  m_RP.m_pCurObject = m_RP.m_VisObjects[0];
  m_RP.m_pPrevObject = m_RP.m_pCurObject;

#ifdef DO_RENDERLOG
  if (CV_r_log)
    Logv(SRendItem::m_RecurseLevel, "*** Start frame for list %s ***\n", sDescList[nList]);
#endif

  EF_PreRender(1);

  if (nList==EFSLIST_PREPROCESS_ID || nList==EFSLIST_GENERAL_ID || nList==EFSLIST_LAST_ID)
  {
    {
      PROFILE_FRAME(State_Sorting);
      SRendItem::mfSort(&SRendItem::m_RendItems[nList][nums], nume-nums);
    }

    // If sort number of the first shader is 1 (eS_Preprocess)
    // run preprocess operations for the current frame
    if ((SRendItem::m_RendItems[nList][nums].SortVal.i.High >> 26) == eS_PreProcess)
      nums += EF_Preprocess(&SRendItem::m_RendItems[nList][0], nums, nume);
    if (m_RP.m_PersFlags & RBPF_IGNORERENDERING)
      return;
  }
  else
  if (nList==EFSLIST_DISTSORT_ID)
  {
    PROFILE_FRAME(State_SortingDist);
    SRendItem::mfSortByDist(&SRendItem::m_RendItems[nList][nums], nume-nums);
  }
#ifdef PIPE_USE_INSTANCING
  else
  if (nList==EFSLIST_STENCIL_ID)
  {
    PROFILE_FRAME(State_Sorting);
    int nStart = nums;
    for (i=nums; i<nume; i++)
    {
      SRendItemPre *ri = &SRendItem::m_RendItems[nList][i];
      int Type = ri->Item->mfGetType();
      if (Type == eDATA_ClearStencil || Type == eDATA_TriMeshShadow)
      {
        SRendItem::mfSortForStencil(&SRendItem::m_RendItems[nList][nStart], i-nStart);
        do
        {
          i++;
          if (i >= nume)
            break;
          ri = &SRendItem::m_RendItems[nList][i];
          Type = ri->Item->mfGetType();
        } while(Type == eDATA_ClearStencil || Type == eDATA_TriMeshShadow);
        nStart = i;
        i--;
      }
    }
    SRendItem::mfSortForStencil(&SRendItem::m_RendItems[nList][nStart], i-nStart);
  }
#endif

  m_RP.m_Flags |= RBF_3D;

  EF_PreRender(3);
  EF_PushMatrix();

  /*float plane[4];
  {
    plane[0] = 0;
    plane[1] = 0;
    plane[2] = 1;
    plane[3] = -50;
    EF_SetClipPlane(true, plane, false);
  }*/

  PROFILE_FRAME(DrawShader_PipeLine);

  UnINT64 oldVal;
  oldVal.SortVal = -1;
  nCurObject = -2;
  nCurFog = 0;
  pCurShader = NULL;
  pCurShaderState = NULL;
  pCurRes = NULL;
  bool bIgnore = false;
  bool bChanged;
  bool bUseBatching = (RenderFunc == EF_Flush);

  for (i=nums; i<nume; i++)
  {
    SRendItemPre *ri = &SRendItem::m_RendItems[nList][i];
    CRendElement *pRE = ri->Item;
#ifndef PIPE_USE_INSTANCING
    if (ri->SortVal.SortVal == oldVal.SortVal)
    {
      if (bIgnore)
        continue;
      // Optimizations: not necessary to check of changing shaders and objects
      // if sort value is the same (contains the same info about shaders, objects, fog volumes, ...)
      {
        //PROFILE_FRAME_TOTAL(Mesh_REPrepare);
        pRE->mfPrepare();
      }
      continue;
    }
    oldVal.SortVal = ri->SortVal.SortVal;
    SRendItem::mfGet(ri->SortVal, &nObject, &pShader, &pShaderState, &nFog, &pRes);
    bChanged = (pCurRes != pRes || pShader != pCurShader || pShaderState != pCurShaderState || nFog != nCurFog);
#else
    if (oldVal.i.High == ri->SortVal.i.High && !((oldVal.i.Low ^ ri->SortVal.i.Low) & 0x000fffff))
    {
      SRendItem::mfGetObj(ri->SortVal, &nObject);
      bChanged = false;
    }
    else
    {
      SRendItem::mfGet(ri->SortVal, &nObject, &pShader, &pShaderState, &nFog, &pRes);
      bChanged = true;
    }
    oldVal.SortVal = ri->SortVal.SortVal;
#endif
    if (nObject != nCurObject)
    {
      if (!bChanged && !pShader->m_Deforms && bUseBatching)
      {
        if (EF_TryToMerge(nObject, nCurObject, pRE))
          continue;
      }
      if (pCurShader)
      {
        m_RP.m_pRenderFunc();
        pCurShader = NULL;
        bChanged = true;
      }
      if (!EF_ObjectChange(pShader, pRes, nObject, pRE))
      {
        bIgnore = true;
        continue;
      }
      bIgnore = false;
      nCurObject = nObject;
    }

    if (bChanged)
    {
      if (pCurShader)
        m_RP.m_pRenderFunc();      
      EF_Start(pShader, pShaderState, pRes, nFog, pRE);
      nCurFog = nFog;
      pCurShader = pShader;
      pCurShaderState = pShaderState;
      pCurRes = pRes;
    }

    {
      //PROFILE_FRAME_TOTAL(Mesh_REPrepare);
      pRE->mfPrepare();
    }
  }
  if (pCurShader)
    m_RP.m_pRenderFunc();

  //EF_SetClipPlane(false, plane, false);

  EF_PostRender();
  EF_PopMatrix();


#ifdef DO_RENDERLOG
  if (CV_r_log)
    Logv(SRendItem::m_RecurseLevel, "*** End frame ***\n\n");
#endif
}

// Draw overlay geometry in wireframe mode
void CD3D9Renderer::EF_DrawWire()
{
  if (CV_r_showlines == 1)
    gcpRendD3D->EF_SetState(GS_POLYLINE | GS_NODEPTHTEST);
  else
  if (CV_r_showlines == 2)
    gcpRendD3D->EF_SetState(GS_POLYLINE);
  gcpRendD3D->SetMaterialColor(1,1,1,1);
  gRenDev->m_TexMan->m_Text_White->Set();
  gcpRendD3D->EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  if (gcpRendD3D->m_RP.m_pRE)
    gcpRendD3D->m_RP.m_pRE->mfCheckUpdate(gcpRendD3D->m_RP.m_pShader->m_VertexFormatId, 0);
  int StrVrt;

  gcpRendD3D->EF_SetObjectTransform(gcpRendD3D->m_RP.m_pCurObject, NULL, gcpRendD3D->m_RP.m_pCurObject->m_ObjFlags);

  void *verts = (void *)gcpRendD3D->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, 0);
  gcpRendD3D->EF_Draw(gcpRendD3D->m_RP.m_pShader, NULL);
}

// Draw geometry normal vectors
void CD3D9Renderer::EF_DrawNormals()
{
  HRESULT h;
  float len = CRenderer::CV_r_normalslength;
  int StrVrt, StrNrm;
  //if (gcpRendD3D->m_RP.m_pRE)
  //  gcpRendD3D->m_RP.m_pRE->mfCheckUpdate(gcpRendD3D->m_RP.m_pShader->m_VertexFormatId, SHPF_NORMALS);
  byte *verts = (byte *)gcpRendD3D->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, FGP_SRC | FGP_REAL);
  byte *norms = (byte *)gcpRendD3D->EF_GetPointer(eSrcPointer_Normal, &StrNrm, GL_FLOAT, eSrcPointer_Normal, FGP_SRC | FGP_REAL);
  if ((INT_PTR)norms > 256 && (INT_PTR)verts > 256)
  {
    gcpRendD3D->EF_SetObjectTransform(gcpRendD3D->m_RP.m_pCurObject, NULL, gcpRendD3D->m_RP.m_pCurObject->m_ObjFlags);

    int numVerts = gcpRendD3D->m_RP.m_RendNumVerts;

    gRenDev->m_TexMan->m_Text_White->Set();
    gcpRendD3D->EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
    gcpRendD3D->EF_SetState(GS_POLYLINE);
    struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *Verts = new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F[numVerts*2];

    uint col0 = 0x000000ff;
    uint col1 = 0x00ffffff;

    for (int v=0; v<numVerts*2; v+=2,verts+=StrVrt,norms+=StrNrm)
    {
      float *fverts = (float *)verts;
      float *fnorms = (float *)norms;
      Verts[v].xyz.x = fverts[0];
      Verts[v].xyz.y = fverts[1];
      Verts[v].xyz.z = fverts[2];
      Verts[v].color.dcolor = col0;

      Verts[v+1].xyz.x = fverts[0] + fnorms[0]*len;
      Verts[v+1].xyz.y = fverts[1] + fnorms[1]*len;
      Verts[v+1].xyz.z = fverts[2] + fnorms[2]*len;
      Verts[v+1].color.dcolor = col1;
    }
    gcpRendD3D->EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);
    h = gcpRendD3D->m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, numVerts, Verts, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    delete [] Verts;
  }
}

// Draw geometry tangent vectors
void CD3D9Renderer::EF_DrawTangents()
{
  HRESULT h;

  float len = CRenderer::CV_r_normalslength;
  //if (gcpRendD3D->m_RP.m_pRE)
  //  gcpRendD3D->m_RP.m_pRE->mfCheckUpdate(gcpRendD3D->m_RP.m_pShader->m_VertexFormatId, SHPF_TANGENTS);
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
  if ((INT_PTR)tangs>256 && (INT_PTR)binorm>256 && (INT_PTR)tnorm>256)
  {
    gcpRendD3D->EF_SetObjectTransform(gcpRendD3D->m_RP.m_pCurObject, NULL, gcpRendD3D->m_RP.m_pCurObject->m_ObjFlags);

    int numVerts = gcpRendD3D->m_RP.m_RendNumVerts;

    gRenDev->m_TexMan->m_Text_White->Set();
    gcpRendD3D->EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
    if (gcpRendD3D->m_polygon_mode == R_SOLID_MODE)
      gcpRendD3D->EF_SetState(GS_POLYLINE | GS_DEPTHWRITE);
    else
      gcpRendD3D->EF_SetState(0);
    struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *Verts = new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F[numVerts*6];

    for (int v=0; v<numVerts; v++,verts+=StrVrt,tangs+=StrTang, binorm+=StrBinorm, tnorm+=StrTNorm)
    {
      uint col0 = 0xffff0000;
      uint col1 = 0xffffffff;
      float *fverts = (float *)verts;
      float *fv = (float *)tangs;
      Verts[v*6+0].xyz.x = fverts[0];
      Verts[v*6+0].xyz.y = fverts[1];
      Verts[v*6+0].xyz.z = fverts[2];
      Verts[v*6+0].color.dcolor = col0;

      Verts[v*6+1].xyz.x = fverts[0] + fv[0]*len;
      Verts[v*6+1].xyz.y = fverts[1] + fv[1]*len;
      Verts[v*6+1].xyz.z = fverts[2] + fv[2]*len;
      Verts[v*6+1].color.dcolor = col1;

      col0 = 0x0000ff00;
      col1 = 0x00ffffff;
      fverts = (float *)verts;
      fv = (float *)binorm;
      Verts[v*6+2].xyz.x = fverts[0];
      Verts[v*6+2].xyz.y = fverts[1];
      Verts[v*6+2].xyz.z = fverts[2];
      Verts[v*6+2].color.dcolor = col0;

      Verts[v*6+3].xyz.x = fverts[0] + fv[0]*len;
      Verts[v*6+3].xyz.y = fverts[1] + fv[1]*len;
      Verts[v*6+3].xyz.z = fverts[2] + fv[2]*len;
      Verts[v*6+3].color.dcolor = col1;

      col0 = 0x000000ff;
      col1 = 0x00ffffff;
      fverts = (float *)verts;
      fv = (float *)tnorm;
      Verts[v*6+4].xyz.x = fverts[0];
      Verts[v*6+4].xyz.y = fverts[1];
      Verts[v*6+4].xyz.z = fverts[2];
      Verts[v*6+4].color.dcolor = col0;

      Verts[v*6+5].xyz.x = fverts[0] + fv[0]*len;
      Verts[v*6+5].xyz.y = fverts[1] + fv[1]*len;
      Verts[v*6+5].xyz.z = fverts[2] + fv[2]*len;
      Verts[v*6+5].color.dcolor = col1;
    }
    gcpRendD3D->EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB_TEX2F);
    h = gcpRendD3D->m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, numVerts*3, Verts, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F));
    delete [] Verts;
  }
}

// Draw light sources in debug mode
void CD3D9Renderer::EF_DrawDebugLights()
{
  static int sFrame = 0;
  if (m_nFrameUpdateID != sFrame)
  {
    int i;
    sFrame = m_nFrameID;

    gRenDev->m_TexMan->m_Text_White->Set();

    for (i=0; i<m_RP.m_DLights[SRendItem::m_RecurseLevel].Num(); i++)
    {
      CDLight *dl = m_RP.m_DLights[SRendItem::m_RecurseLevel][i];
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
        EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
        EF_SetVertexDeclaration(0, VERTEX_FORMAT_P3F_COL4UB);

        dir = dl->m_Orientation.m_vForward;
        rgt = dl->m_Orientation.m_vRight;
        float ang = dl->m_fLightFrustumAngle;          
        if (ang == 0)
          ang = 45.0f;
        org = dl->m_Origin;

        dir *= 0.3f;

        CFColor Col = dl->m_Color;

        Matrix44 m;
        Vec3d vertex = dir;

      //  vertex = m.GetRotation(rgt, ang)*vertex;
        vertex = Matrix33::CreateRotationAA(ang*gf_DEGTORAD,GetNormalized(rgt)) * vertex; //NOTE: angle need to be in radians
      //  Matrix mat = m.GetRotation(dir, 60);
        Matrix44 mat = Matrix33::CreateRotationAA(60*gf_DEGTORAD,GetNormalized(dir)); //NOTE: angle need to be in radians
        Vec3d tmpvertex;
        int ctr;

        //fill the inside of the light
        EnableTMU(false);
        struct_VERTEX_FORMAT_P3F_COL4UB Verts[32];
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
        m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, n-2, Verts, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB));

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
        m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, n-2, Verts, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB));
        SetCullMode(R_CULL_FRONT);
        m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

        //set the color to the color of the light
        Verts[0].xyz = org;
        Verts[0].color.dcolor = D3DRGBA(Col[0], Col[1], Col[2], 1.0f);

        //draw a point at the origin of the light
        m_pd3dDevice->DrawPrimitiveUP(D3DPT_POINTLIST, 1, Verts, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB));

        //draw a line in the center of the light
        Verts[0].xyz = org;
        Verts[0].color.dcolor = D3DRGBA(Col[0], Col[1], Col[2], 1.0f);

        tmpvertex = org + dir;
        Verts[1].xyz = tmpvertex;
        Verts[1].color.dcolor = D3DRGBA(Col[0], Col[1], Col[2], 1.0f);
        m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, Verts, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB));

        EnableTMU(true);
      }
      if (CV_r_debuglights == 2 && !(dl->m_Flags & DLF_DIRECTIONAL))
      {
        EF_SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);
        SetCullMode(R_CULL_NONE);
        SetMaterialColor(dl->m_Color[0], dl->m_Color[1], dl->m_Color[2], 0.25f);
        DrawBall(dl->m_Origin[0], dl->m_Origin[1], dl->m_Origin[2], dl->m_fRadius);
      }
    }
  }
}

// Draw debug geometry/info
void CD3D9Renderer::EF_DrawDebugTools()
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


static int __cdecl TimeProfCallback( const VOID* arg1, const VOID* arg2 )
{
  SProfInfo *pi1 = (SProfInfo *)arg1;
  SProfInfo *pi2 = (SProfInfo *)arg2;
  if (pi1->ef->m_fProfileTime > pi2->ef->m_fProfileTime)
    return -1;
  if (pi1->ef->m_fProfileTime < pi2->ef->m_fProfileTime)
    return 1;
  return 0;
}

static int __cdecl NameCallback( const VOID* arg1, const VOID* arg2 )
{
  SProfInfo *pi1 = (SProfInfo *)arg1;
  SProfInfo *pi2 = (SProfInfo *)arg2;
  if (pi1->ef > pi2->ef)
    return -1;
  if (pi1->ef < pi2->ef)
    return 1;
  return 0;
}

//#include "FMallocWindows.h"

// Print shaders profile info on the screen
void CD3D9Renderer::EF_PrintProfileInfo()
{
  double fTime = 0;
  int i;
  if (CV_r_profileshaders)
  { // group by name
    qsort(&m_RP.m_Profile[0], m_RP.m_Profile.Num(), sizeof(SProfInfo), NameCallback );

    for(i=0; i<m_RP.m_Profile.Num(); i++)
    {
      // if next is the same
      if( i<(m_RP.m_Profile.Num()-1) && m_RP.m_Profile[i].ef == m_RP.m_Profile[i+1].ef )
      {
        m_RP.m_Profile[i].Time += m_RP.m_Profile[i+1].Time;
        m_RP.m_Profile[i].m_nItems++;
        m_RP.m_Profile[i].NumPolys += m_RP.m_Profile[i+1].NumPolys;
        m_RP.m_Profile.DelElem(i+1);
        i--;
      }
    }
  }
  for (i=0; i<m_RP.m_Profile.Num(); i++)
  {
    m_RP.m_Profile[i].ef->m_fProfileTime = (float)(m_RP.m_Profile[i].Time+m_RP.m_Profile[i].ef->m_fProfileTime*50)/51.0f;
  }

  TextToScreenColor(1,14, 0,2,0,1, "Items: %d, Batches: %d, Obj: %d, DrawCalls: %d, Text: %d, Stat: %d, PShad: %d, VShad: %d",  m_RP.m_PS.m_NumRendItems, m_RP.m_PS.m_NumRendBatches, m_RP.m_PS.m_NumRendObjects, m_RP.m_PS.m_NumDrawCalls, m_RP.m_PS.m_NumTextChanges, m_RP.m_PS.m_NumStateChanges, m_RP.m_PS.m_NumPShadChanges, m_RP.m_PS.m_NumVShadChanges);
  TextToScreenColor(1,17, 0,2,0,1, "VShad: %d, PShad: %d, Text: %d, Lit Shaders: %d",  m_RP.m_PS.m_NumVShaders, m_RP.m_PS.m_NumPShaders, m_RP.m_PS.m_NumTextures, m_RP.m_PS.m_NumLitShaders);
  TextToScreenColor(1,20, 0,2,0,1, "Preprocess: %8.02f ms, Occl. queries: %8.02f ms",  m_RP.m_PS.m_fPreprocessTime, m_RP.m_PS.m_fOcclusionTime);
  TextToScreenColor(1,23, 0,2,0,1, "Skinning:   %8.02f ms (Skinned Objects: %d)",  m_RP.m_PS.m_fSkinningTime, m_RP.m_PS.m_NumRendSkinnedObjects);

  int nLine;
  { // sort by interpolated time and print
    qsort(&m_RP.m_Profile[0], m_RP.m_Profile.Num(), sizeof(SProfInfo), TimeProfCallback );

    for(nLine=0; nLine<m_RP.m_Profile.Num(); nLine++)
    {
      fTime += m_RP.m_Profile[nLine].Time;
      if (nLine >= 18)
        continue;
      TextToScreenColor(4,(27+(nLine*3)), 1,0,0,1, "%8.02f ms, %5d polys, '%s', %d item(s)", 
        m_RP.m_Profile[nLine].ef->m_fProfileTime, 
        m_RP.m_Profile[nLine].NumPolys,
        m_RP.m_Profile[nLine].ef->m_Name.c_str(),
        m_RP.m_Profile[nLine].m_nItems+1);
    }
  }
  int nShaders = nLine;
  TextToScreenColor(2,(28+(nLine*3)), 0,2,0,1, "Total unique shaders:   %d",  nShaders);
  TextToScreenColor(2,(31+(nLine*3)), 0,2,0,1, "Total flush time:   %8.02f ms",  fTime);
  TextToScreenColor(2,(34+(nLine*3)), 0,2,0,1, "Total shaders processing time:   %8.02f ms", m_RP.m_PS.m_fFlushTime);

  /*{
    int i = 0;
    double timeC = 0;
    double timeSSE = 0;

    CCamera cam = GetCamera();
    Vec3d camPos = cam.GetPos();
    AABB aabb;
    aabb.min = camPos+Vec3d(-10, -16,-16);
    aabb.max = camPos+Vec3d(16,32,16);
    Vec3d Origin = (aabb.min+aabb.max)*0.5f;
    Vec3d Extent = aabb.max - Origin;

    ticks(timeC);
    for (i=0; i<100000; i++)
    {
      cam.IsAABBVisible_exact(aabb, NULL);
    }
    unticks(timeC);

    ticks(timeSSE);
    for (i=0; i<100000; i++)
    {
      cam.IsAABBVisible_exact_SSE(Origin, Extent, NULL);
    }
    unticks(timeSSE);

    TextToScreenColor(8,(36+(nLine*3)), 0,2,0,1, "TimeC %8.02f ms, TimeSSE %8.02f ms", (float)(timeC*1000.0*m_RP.m_SecondsPerCycle), (float)(timeSSE*1000.0*m_RP.m_SecondsPerCycle));
  }*/
  /*{
    int i = 0;
    double timeC = 0;
    double timeC33 = 0;
    double time3DN = 0;
    double timeSSE = 0;
    _declspec(align(16)) Matrix m, mOut;
    m = m_cam.GetMatrix();
    m_RP.m_pCurObject = m_RP.m_VisObjects[0];
    Matrix33 m31;
    Matrix33 m32;
    m31.SetIdentity33();

    ticks(timeC33);
    for (i=0; i<100000; i++)
    {
      m31.Invert33();
    }
    unticks(timeC33);

    ticks(timeC);
    for (i=0; i<100000; i++)
    {
      QQinvertMatrixf(mOut.GetData(), m.GetData());
    }
    unticks(timeC);

    if (m_Cpu->mCpu[0].mFeatures & CFI_3DNOW)
    {
      ticks(time3DN);
      for (i=0; i<100000; i++)
      {
        invertMatrixf_3DNow(mOut.GetData(), m.GetData());
      }
      unticks(time3DN);
    }

    ticks(timeSSE);
    for (i=0; i<100000; i++)
    {
      invertMatrixf_SSE(mOut.GetData(), m.GetData());
    }
    unticks(timeSSE);

    TextToScreenColor(8,(36+(nLine*3)), 0,2,0,1, "TimeC: %8.02f ms, TimeC33: %8.02f ms,, Time3DN: %8.02f ms, TimeSSE: %8.02f ms", (float)(timeC*1000.0*m_RP.m_SecondsPerCycle), (float)(timeC33*1000.0*m_RP.m_SecondsPerCycle), (float)(time3DN*1000.0*m_RP.m_SecondsPerCycle), (float)(timeSSE*1000.0*m_RP.m_SecondsPerCycle));
  }*/
  /*{
    int i = 0;
    double timeC = 0;
    double timeSSE = 0;
    double timeSSENew = 0;
    byte *dataSrc = new byte[1024*1024*10];
    byte *dataDst = new byte[1024*1024*10];

    ticks(timeC);
    for (i=0; i<10; i++)
    {
      memcpy(dataDst, dataSrc, 1024*1024*10-128);
    }
    unticks(timeC);

    byte *Dst = (byte *)((int)(dataDst+15)&0xfffffff0);
    byte *Src = (byte *)((int)(dataSrc+15)&0xfffffff0);
    ticks(timeSSE);
    for (i=0; i<10; i++)
    {
      cryMemcpy(Dst, Src, 1024*1024*10-128);
    }
    unticks(timeSSE);

    ticks(timeSSENew);
    for (i=0; i<10; i++)
    {
      cryMemcpy(Dst, Src, 1024*1024*10-128, 0);
    }
    unticks(timeSSENew);

    delete [] dataSrc;
    delete [] dataDst;

    TextToScreenColor(8,(36+(nLine*3)), 0,2,0,1, "TimeC: %8.02f ms, TimeSSE: %8.02f ms", (float)(timeC*1000.0*g_SecondsPerCycle), (float)(timeSSE*1000.0*g_SecondsPerCycle));
  }*/
  /*{
    int i = 0;
    double timeCM10 = 0;
    double timeCF10 = 0;
    double timeCM60 = 0;
    double timeCF60 = 0;
    double timeCM110 = 0;
    double timeCF110 = 0;
    double timeCM510 = 0;
    double timeCF510 = 0;
    double timeCM1010 = 0;
    double timeCF1010 = 0;
    double timeCM5010 = 0;
    double timeCF5010 = 0;
    double timeCM10010 = 0;
    double timeCF10010 = 0;
    double timeCM100010 = 0;
    double timeCF100010 = 0;

    double timeUM10 = 0;
    double timeUF10 = 0;
    double timeUM60 = 0;
    double timeUF60 = 0;
    double timeUM110 = 0;
    double timeUF110 = 0;
    double timeUM510 = 0;
    double timeUF510 = 0;
    double timeUM1010 = 0;
    double timeUF1010 = 0;
    double timeUM5010 = 0;
    double timeUF5010 = 0;
    double timeUM10010 = 0;
    double timeUF10010 = 0;
    double timeUM100010 = 0;
    double timeUF100010 = 0;

    static FMallocWindows *pM;
    if (!pM)
    {
      pM = new FMallocWindows;
      pM->Init();
    }
    void *pPtr[1000];

    ticks(timeCM10);
    for (i=0; i<1000; i++)
      pPtr[i] = malloc(10);
    unticks(timeCM10);
    ticks(timeCF10);
    for (i=0; i<1000; i++)
      free(pPtr[i]);
    unticks(timeCF10);

    ticks(timeUM10);
    for (i=0; i<1000; i++)
      pPtr[i] = pM->Malloc(10, "Test");
    unticks(timeUM10);
    ticks(timeUF10);
    for (i=0; i<1000; i++)
      pM->Free(pPtr[i]);
    unticks(timeUF10);

    ticks(timeCM60);
    for (i=0; i<1000; i++)
      pPtr[i] = malloc(60);
    unticks(timeCM60);
    ticks(timeCF60);
    for (i=0; i<1000; i++)
      free(pPtr[i]);
    unticks(timeCF60);

    ticks(timeUM60);
    for (i=0; i<1000; i++)
      pPtr[i] = pM->Malloc(60, "Test");
    unticks(timeUM60);
    ticks(timeUF60);
    for (i=0; i<1000; i++)
      pM->Free(pPtr[i]);
    unticks(timeUF60);

    ticks(timeCM110);
    for (i=0; i<1000; i++)
      pPtr[i] = malloc(110);
    unticks(timeCM110);
    ticks(timeCF110);
    for (i=0; i<1000; i++)
      free(pPtr[i]);
    unticks(timeCF110);

    ticks(timeUM110);
    for (i=0; i<1000; i++)
      pPtr[i] = pM->Malloc(110, "Test");
    unticks(timeUM110);
    ticks(timeUF110);
    for (i=0; i<1000; i++)
      pM->Free(pPtr[i]);
    unticks(timeUF110);

    ticks(timeCM510);
    for (i=0; i<1000; i++)
      pPtr[i] = malloc(510);
    unticks(timeCM510);
    ticks(timeCF510);
    for (i=0; i<1000; i++)
      free(pPtr[i]);
    unticks(timeCF510);

    ticks(timeUM510);
    for (i=0; i<1000; i++)
      pPtr[i] = pM->Malloc(510, "Test");
    unticks(timeUM510);
    ticks(timeUF510);
    for (i=0; i<1000; i++)
      pM->Free(pPtr[i]);
    unticks(timeUF510);

    ticks(timeCM1010);
    for (i=0; i<1000; i++)
      pPtr[i] = malloc(1010);
    unticks(timeCM1010);
    ticks(timeCF1010);
    for (i=0; i<1000; i++)
      free(pPtr[i]);
    unticks(timeCF1010);

    ticks(timeUM1010);
    for (i=0; i<1000; i++)
      pPtr[i] = pM->Malloc(1010, "Test");
    unticks(timeUM1010);
    ticks(timeUF1010);
    for (i=0; i<1000; i++)
      pM->Free(pPtr[i]);
    unticks(timeUF1010);

    ticks(timeCM5010);
    for (i=0; i<1000; i++)
      pPtr[i] = malloc(5010);
    unticks(timeCM5010);
    ticks(timeCF5010);
    for (i=0; i<1000; i++)
      free(pPtr[i]);
    unticks(timeCF5010);

    ticks(timeUM5010);
    for (i=0; i<1000; i++)
      pPtr[i] = pM->Malloc(5010, "Test");
    unticks(timeUM5010);
    ticks(timeUF5010);
    for (i=0; i<1000; i++)
      pM->Free(pPtr[i]);
    unticks(timeUF5010);

    ticks(timeCM10010);
    for (i=0; i<1000; i++)
      pPtr[i] = malloc(10010);
    unticks(timeCM10010);
    ticks(timeCF10010);
    for (i=0; i<1000; i++)
      free(pPtr[i]);
    unticks(timeCF10010);

    ticks(timeUM10010);
    for (i=0; i<1000; i++)
      pPtr[i] = pM->Malloc(10010, "Test");
    unticks(timeUM10010);
    ticks(timeUF10010);
    for (i=0; i<1000; i++)
      pM->Free(pPtr[i]);
    unticks(timeUF10010);

    ticks(timeCM100010);
    for (i=0; i<1000; i++)
      pPtr[i] = malloc(100010);
    unticks(timeCM100010);
    ticks(timeCF100010);
    for (i=0; i<1000; i++)
      free(pPtr[i]);
    unticks(timeCF100010);

    ticks(timeUM100010);
    for (i=0; i<1000; i++)
      pPtr[i] = pM->Malloc(100010, "Test");
    unticks(timeUM100010);
    ticks(timeUF100010);
    for (i=0; i<1000; i++)
      pM->Free(pPtr[i]);
    unticks(timeUF100010);

    TextToScreenColor(1,(36+(nLine*3)), 0,2,0,1, "CM_10: %3.02f, CM_60: %3.02f, CM_110: %3.02f, CM_510: %3.02f, CM_1010: %3.02f, CM_5010: %3.02f, CM_10010: %3.02f, CM_100010: %3.02f", (float)(timeCM10*1000.0*g_SecondsPerCycle), (float)(timeCM60*1000.0*g_SecondsPerCycle), (float)(timeCM110*1000.0*g_SecondsPerCycle), (float)(timeCM510*1000.0*g_SecondsPerCycle), (float)(timeCM1010*1000.0*g_SecondsPerCycle), (float)(timeCM5010*1000.0*g_SecondsPerCycle), (float)(timeCM10010*1000.0*g_SecondsPerCycle), (float)(timeCM100010*1000.0*g_SecondsPerCycle));
    TextToScreenColor(1,(40+(nLine*3)), 0,2,0,1, "UM_10: %3.02f, UM_60: %3.02f, UM_110: %3.02f, UM_510: %3.02f, UM_1010: %3.02f, UM_5010: %3.02f, UM_10010: %3.02f, UM_100010: %3.02f", (float)(timeUM10*1000.0*g_SecondsPerCycle), (float)(timeUM60*1000.0*g_SecondsPerCycle), (float)(timeUM110*1000.0*g_SecondsPerCycle), (float)(timeUM510*1000.0*g_SecondsPerCycle), (float)(timeUM1010*1000.0*g_SecondsPerCycle), (float)(timeUM5010*1000.0*g_SecondsPerCycle), (float)(timeUM10010*1000.0*g_SecondsPerCycle), (float)(timeUM100010*1000.0*g_SecondsPerCycle));
    TextToScreenColor(1,(44+(nLine*3)), 0,2,0,1, "CF_10: %3.02f, CF_60: %3.02f, CF_110: %3.02f, CF_510: %3.02f, CF_1010: %3.02f, CF_5010: %3.02f, CF_10010: %3.02f, CF_100010: %3.02f", (float)(timeCF10*1000.0*g_SecondsPerCycle), (float)(timeCF60*1000.0*g_SecondsPerCycle), (float)(timeCF110*1000.0*g_SecondsPerCycle), (float)(timeCF510*1000.0*g_SecondsPerCycle), (float)(timeCF1010*1000.0*g_SecondsPerCycle), (float)(timeCF5010*1000.0*g_SecondsPerCycle), (float)(timeCF10010*1000.0*g_SecondsPerCycle), (float)(timeCF100010*1000.0*g_SecondsPerCycle));
    TextToScreenColor(1,(48+(nLine*3)), 0,2,0,1, "UF_10: %3.02f, UF_60: %3.02f, UF_110: %3.02f, UF_510: %3.02f, UF_1010: %3.02f, UF_5010: %3.02f, UF_10010: %3.02f, UF_100010: %3.02f", (float)(timeUF10*1000.0*g_SecondsPerCycle), (float)(timeUF60*1000.0*g_SecondsPerCycle), (float)(timeUF110*1000.0*g_SecondsPerCycle), (float)(timeUF510*1000.0*g_SecondsPerCycle), (float)(timeUF1010*1000.0*g_SecondsPerCycle), (float)(timeUF5010*1000.0*g_SecondsPerCycle), (float)(timeUF10010*1000.0*g_SecondsPerCycle), (float)(timeUF100010*1000.0*g_SecondsPerCycle));
  }*/
}

void CD3D9Renderer::EF_DrawREPreprocess(SRendItemPreprocess *ris, int Nums)
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

  EF_PreRender(1);

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
      if (!EF_ObjectChange(Shader, Res, nObject, re))
        continue;
      nCurObject = nObject;
    }

    if (Shader != CurShader, Res != CurRes || ShaderState != CurShaderState || nFog != nCurFog)
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
  int m_nPreprocess;
  int m_Num;
  int m_nObject;
  SShader *m_Shader;
  SRenderShaderResources *m_pRes;
  CRendElement *m_RE;
};

STWarpZone *CD3D9Renderer::EF_SetWarpZone(SWarpSurf *sf, int *NumWarps, STWarpZone Warps[])
{
  int i;
  STWarpZone *wp;
  Plane p, pl;

  sf->srf->mfGetPlane(pl);
  if (sf->nobj > 0)
  {
    p = pl;
    CCObject *obj = m_RP.m_VisObjects[sf->nobj];
    pl = TransformPlane(obj->GetMatrix(), p);
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

void CD3D9Renderer::EF_UpdateWarpZone(STWarpZone *wp, SWarpSurf *srf)
{
  if (wp->numSrf == MAX_WARPSURFS)
    return;
  wp->Surfs[wp->numSrf].nobj = srf->nobj;
  wp->Surfs[wp->numSrf].Shader = srf->Shader;
  wp->Surfs[wp->numSrf].srf = srf->srf;
  wp->numSrf++;
}

bool CD3D9Renderer::EF_CalcWarpCamera(STWarpZone *wp, int nObject, CCamera& prevCam, CCamera& newCam)
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
      vNewPos = Vec3d(pObj->m_Trans2[0], pObj->m_Trans2[1], pObj->m_Trans2[2]);
      vNewOccPos = vNewPos;
      vNewAngs = Vec3d(pObj->m_Angs2[0], pObj->m_Angs2[1], pObj->m_Angs2[2]);
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

    vNewAngs[0] = cry_asinf (vNewDir[2])/M_PI*180.0f;
    vNewAngs[1] = vPrevAngs[1] + 180.0f;
    vNewAngs[2] = -cry_atan2f (vNewDir[0], -vNewDir[1])/M_PI*180.0f;

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

bool CD3D9Renderer::EF_RenderWarpZone(STWarpZone *wp)
{
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
  eng->DrawLowDetail(DLD_TERRAIN_WATER | DLD_DETAIL_TEXTURES | DLD_DETAIL_OBJECTS | DLD_FAR_SPRITES | DLD_ENTITIES);

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
  int nPr1 = a.m_nPreprocess;
  int nPr2 = b.m_nPreprocess;

  if (nPr1 < nPr2)
    return -1;
  if (nPr1 > nPr2)
    return 1;
  return 0;
}

// Flush all refracted objects in the list
void CD3D9Renderer::EF_FlushRefractedObjects(SShader *pSHRefr[], CRendElement *pRERefr[], CCObject *pObjRefr[], int nRefrObjects, int nRefrFlags, int DLDFlags)
{
  if (nRefrFlags & 2)
  {
    Vec3d Angs = GetCamera().GetAngles();
    Vec3d Pos = GetCamera().GetPos();
    m_RP.m_PersFlags |= RBPF_IGNOREREFRACTED;
    if (nRefrObjects > 1)
      pSHRefr[0] = NULL;
    float fMinDist = 99999.0f;
    int nSelectObj = -1;
    for (int i=0; i<nRefrObjects; i++)
    {
      float fDist = pRERefr[i]->mfMinDistanceToCamera(pObjRefr[i]);
      if (fMinDist > fDist)
      {
        fMinDist = fDist;
        nSelectObj = i;
      }
    }
    if (nSelectObj >= 0)
    {
      m_RP.m_pRE = pRERefr[nSelectObj];
      m_RP.m_pCurObject = pObjRefr[nSelectObj];
      SEnvTexture *cm = gRenDev->m_cEF.mfFindSuitableEnvTex(Pos, Angs, false, DLDFlags, true, pSHRefr[0], NULL, NULL, false, NULL);
    }
    m_RP.m_PersFlags &= ~RBPF_IGNOREREFRACTED;
  }

  if (nRefrFlags & 1)
  {
    m_RP.m_bDrawToTexture = true;
    m_TexMan->StartRefractMap(TO_REFRACTMAP);
    m_RP.m_PersFlags |= RBPF_ONLYREFRACTED;
    EF_RenderPipeLine(EF_Flush);
    m_RP.m_PersFlags &= ~RBPF_ONLYREFRACTED;
    m_TexMan->EndRefractMap();
//    m_TexMan->StartScreenTexMap(TO_SCREENMAP);
//    EF_RenderPipeLine(EF_Flush);
//    m_TexMan->EndScreenTexMap();
    m_RP.m_bDrawToTexture = false;
    //m_RP.m_PersFlags |= RBPF_ONLYREFRACTED;
    //m_RP.m_PersFlags |= RBPF_IGNORERENDERING;
  }
}

// Current scene preprocess operations (drawing to texture, screen effects initializing, ...)
int CD3D9Renderer::EF_Preprocess(SRendItemPre *ri, int nums, int nume)
{
  int i, j;
  SShader *Shader;
  SShader *ShaderState;
  SRenderShaderResources *Res;
  int nObject;
  int nFog;

  SPreprocess Procs[512];
  int nProcs = 0;
  TArray<SRendItemPreprocess> RIs;
  TArray<SWarpSurf> srfs;
  int nPortals = 0;
  int nRefractedStuff = 0;

  int NumWarps = 0;
  STWarpZone Warps[MAX_WARPS];
  STWarpZone *wp;

  double time0 = 0;
  ticks(time0);

  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "*** Start preprocess frame ***\n");

  int DLDFlags = 0;
  int nReturn = 0;

  for (i=nums; i<nume; i++)
  {
    if (nProcs >= 512)
      break;
    SRendItem::mfGet(ri[i].SortVal, &nObject, &Shader, &ShaderState, &nFog, &Res);
    if ((ri[i].SortVal.i.High >> 26) != eS_PreProcess)
      break;
    nReturn++;
    for (j=0; j<32; j++)
    {
      int nMask = 1<<j;
      if (nMask >= FSPR_MAX || nMask > Shader->m_nPreprocess)
        break;
      if (nMask & Shader->m_nPreprocess)
      {
        Procs[nProcs].m_nPreprocess = j;
        Procs[nProcs].m_Num = i;
        Procs[nProcs].m_Shader = Shader;
        Procs[nProcs].m_pRes = Res;
        Procs[nProcs].m_RE = ri[i].Item;
        Procs[nProcs].m_nObject = nObject;
        nProcs++;
      }
    }
  }
  if (!nProcs)
    return 0;
  ::Sort(&Procs[0], nProcs);

  if (m_RP.m_pRenderFunc != EF_Flush)
    return nReturn;

  int nRefrObjects = 0;
  SShader *pSHRefr[8];
  CRendElement *pRERefr[8];
  CCObject *pObjRefr[8];
  
  for (i=0; i<nProcs; i++)
  {
    SPreprocess *pr = &Procs[i];
    if (!pr->m_Shader)
      continue;
    switch (pr->m_nPreprocess)
    {
      case SPRID_SCANCM:
        // Draw environment to cube texture
        if (!m_RP.m_bDrawToTexture)
        {
          CCObject *objIgn = m_RP.m_pIgnoreObject;
          Vec3d Pos;
          m_RP.m_pRE = pr->m_RE;
          if (pr->m_nObject < 0)
            Pos(0,0,0);
          else
          {
            CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
            if (!IsEquivalent(obj->GetTranslation(), Vec3d(0,0,0)))
              m_RP.m_pIgnoreObject = obj;
            Pos = obj->GetTranslation();
            m_RP.m_pCurObject = obj;
          }
          if (m_LogFile)
            Logv(SRendItem::m_RecurseLevel, "*** Draw environment to cube-map ***\n");
          if (!IsEquivalent(Pos,Vec3d(0,0,0)))
          {
            float fDistToCam = (Pos-m_cam.GetPos()).Length();
            SEnvTexture *cm = gRenDev->m_cEF.mfFindSuitableEnvCMap(Pos, false, pr->m_Shader->m_DLDFlags, fDistToCam);
          }
          m_RP.m_pIgnoreObject = objIgn;
        }
        break;
      case SPRID_SCANLCM:
        // Draw environment to cube texture
        if (!m_RP.m_bDrawToTexture)
        {
          CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
          if ((obj->m_ObjFlags & FOB_ENVLIGHTING) && CV_r_envlighting)
          {
            CCObject *objIgn = m_RP.m_pIgnoreObject;
            Vec3d Pos;
            m_RP.m_pRE = pr->m_RE;
            if (!IsEquivalent(obj->GetTranslation(), Vec3d(0,0,0)))
              m_RP.m_pIgnoreObject = obj;
            Pos = obj->GetTranslation();
            if (m_LogFile)
              Logv(SRendItem::m_RecurseLevel, "*** Draw environment to light cube-map (%.3f, %.3f, %.3f), Time: %.3f s ***\n", Pos.x, Pos.y, Pos.z, iTimer->GetAsyncCurTime());
            if (!IsEquivalent(Pos,Vec3d(0,0,0)))
            {
              float fDR;
              ICVar * pVarDR = iConsole->GetCVar("e_obj_view_dist_ratio");
              if (pVarDR)
              {
                fDR = pVarDR->GetFVal();
                pVarDR->Set(fDR/3.0f);
              }
              int nDLDFlags = -1;
              nDLDFlags &= ~(DLD_SHADOW_MAPS | DLD_ENTITIES | DLD_FIRST_PERSON_CAMERA_OWNER | DLD_TERRAIN_LIGHT | DLD_PARTICLES | DLD_FAR_SPRITES | DLD_DETAIL_TEXTURES | DLD_DETAIL_OBJECTS);
              float fDistToCam = (Pos-m_cam.GetPos()).Length();
              SEnvTexture *cm = gRenDev->m_cEF.mfFindSuitableEnvLCMap(Pos, false, nDLDFlags, fDistToCam, obj);
              if (pVarDR)
                pVarDR->Set(fDR);
            }
            m_RP.m_pIgnoreObject = objIgn;
          }
        }
        break;
      case SPRID_SCANSCR:
        m_TexMan->StartScreenMap(TO_ENVIRONMENT_SCR);
        break;
      case SPRID_SCANTEXWATER:
        if (!m_RP.m_bDrawToTexture && CV_r_waterreflections)
        {
          if (!(gRenDev->m_TexMan->m_Text_WaterMap->m_Flags & FT_BUILD))
          {
            I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
            float fTimeUpd = eng->GetDistanceToSectorWithWater();
            fTimeUpd *= CV_r_waterupdateFactor;
            if (fTimeUpd > 0.3f)
              fTimeUpd = 0.3f;
            Vec3d camAngs = GetCamera().GetAngles();
            Vec3d camPos = GetCamera().GetPos();
            float fDistCam = (camPos - m_RP.m_LastWaterPosUpdate).Length();
            float fDistAng = (camAngs - m_RP.m_LastWaterAngleUpdate).Length();
            float fFOV = GetCamera().GetFov();
            if (m_RP.m_fLastWaterUpdate-1.0f > m_RP.m_RealTime)
              m_RP.m_fLastWaterUpdate = m_RP.m_RealTime;
            if (m_RP.m_RealTime-m_RP.m_fLastWaterUpdate > fTimeUpd || fDistCam > CV_r_waterupdateDistance || fDistAng>CV_r_waterupdateDeltaAngle || fFOV != m_RP.m_fLastWaterFOVUpdate)
            {
              m_RP.m_fLastWaterUpdate = m_RP.m_RealTime;
              m_RP.m_LastWaterAngleUpdate = camAngs;
              m_RP.m_fLastWaterFOVUpdate = fFOV;
              m_RP.m_LastWaterPosUpdate = camPos;

              CCamera tmp_cam = GetCamera();
              Plane Pl;
              Pl.n = Vec3d(0,0,1);
              Pl.d = eng->GetWaterLevel();
              Vec3d vCamPos = tmp_cam.GetPos();
              if ((vCamPos | Pl.n) - Pl.d < 0)
              {
                Pl.d = -Pl.d;
                Pl.n = -Pl.n;
              }
              CCObject *savedObj = m_RP.m_pCurObject;
              CCObject *objIgn = m_RP.m_pIgnoreObject;
              CCObject *obj = NULL;
              if (pr->m_nObject > 0)
                obj = m_RP.m_VisObjects[pr->m_nObject];
              m_RP.m_PersFlags |= RBPF_DONTDRAWSUN;
              m_RP.m_pCurObject = obj;
              if (!IsEquivalent(obj->GetTranslation(), Vec3d(0,0,0)))
                m_RP.m_pIgnoreObject = obj;
              if (m_LogFile)
                Logv(SRendItem::m_RecurseLevel, "*** Draw environment to texture for water reflections ***\n");
              m_TexMan->DrawToTexture(Pl, gRenDev->m_TexMan->m_Text_WaterMap, pr->m_Shader->m_DLDFlags);
              m_RP.m_pCurObject = savedObj;
              m_RP.m_pIgnoreObject = objIgn;
              m_RP.m_PersFlags &= ~RBPF_DONTDRAWSUN;
            }
          }
        }
        break;
      case SPRID_SCANTEX:
        if (!m_RP.m_bDrawToTexture)
        { // Draw environment to texture
          bool bWater = (pr->m_Shader->m_nPreprocess & FSPR_SCANTEXWATER) != 0;
          bool bDraw = true;
          if (!bWater || CV_r_waterrefractions)
          {
            CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
            if (pr->m_Shader->m_fUpdateFactor > 0)
            {
              float fDist = pr->m_RE->mfMinDistanceToCamera(obj);
              if (fDist > 0)
              {
                fDist *= pr->m_Shader->m_fUpdateFactor;
                if (fDist > 0.5f)
                  fDist = 0.5f;
                if (m_RP.m_pCurObject->m_fLastUpdate-1.0f > m_RP.m_RealTime)
                  m_RP.m_pCurObject->m_fLastUpdate = m_RP.m_RealTime;
                if (m_RP.m_RealTime-m_RP.m_pCurObject->m_fLastUpdate < fDist)
                  bDraw = false;
                else
                  m_RP.m_pCurObject->m_fLastUpdate = m_RP.m_RealTime;
              }
            }
            if (bDraw)
            {
              Vec3d Angs = GetCamera().GetAngles();
              Vec3d Pos = GetCamera().GetPos();
              m_RP.m_pRE = pr->m_RE;

              bool bReflect = false;
              if ((pr->m_Shader->m_Flags3 & (EF3_CLIPPLANE_FRONT | EF3_REFLECTION)))
                bReflect = true;
              if (bReflect)
              {
                m_RP.m_PersFlags |= RBPF_DONTDRAWSUN;
                m_RP.m_pCurObject = obj;
                m_RP.m_pIgnoreObject = obj;
                SEnvTexture *cm = gRenDev->m_cEF.mfFindSuitableEnvTex(Pos, Angs, false, pr->m_Shader->m_DLDFlags, false, pr->m_Shader, pr->m_pRes, obj, bReflect, pr->m_RE);
                m_RP.m_pIgnoreObject = NULL;
                m_RP.m_PersFlags &= ~RBPF_DONTDRAWSUN;
              }
              else
              {
                nRefractedStuff |= 2;
                DLDFlags |= pr->m_Shader->m_DLDFlags;
                obj->m_ObjFlags |= FOB_REFRACTED;
                pSHRefr[nRefrObjects] = pr->m_Shader;
                pRERefr[nRefrObjects] = pr->m_RE;
                pObjRefr[nRefrObjects] = obj;
                if (nRefrObjects != 7)
                  nRefrObjects++;
              }
            }
          }
        }
        break;
      case SPRID_RAINOVERLAY:
        if (!m_RP.m_bDrawToTexture)
          m_TexMan->DrawToTextureForRainMap(TO_RAINMAP);
        break;
      case SPRID_REFRACTED:
        if (!m_RP.m_bDrawToTexture)
        {
          nRefractedStuff |= 1;
          CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
          obj->m_ObjFlags |= FOB_REFRACTED;
        }
        break;
      case SPRID_SCREENTEXMAP:
        EF_FlushRefractedObjects(pSHRefr, pRERefr, pObjRefr, nRefrObjects, nRefractedStuff, DLDFlags);
        nRefractedStuff = 0;
        nRefrObjects = 0;
        // copy screen to texture
        if (!m_RP.m_bDrawToTexture)
        {
          m_TexMan->StartScreenTexMap(TO_SCREENMAP);                    
          // always skip refractive objects..
          m_RP.m_PersFlags |= RBPF_IGNOREREFRACTED;            
        }
      case SPRID_DOFMAP:
        if (!m_RP.m_bDrawToTexture)
        {
          m_TexMan->DrawToTextureForDof(TO_DOFMAP);                    
        }
      case SPRID_SHADOWMAPGEN:
        {
          CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
          m_RP.m_pCurObject = obj;
          if (pr->m_RE->mfGetType() == eDATA_OcLeaf || pr->m_RE->mfGetType() == eDATA_ShadowMapGen)
          {
            CREOcLeaf * pRE = (CREOcLeaf*)pr->m_RE;
            //ShadowMapFrustum * pFr = obj->m_pShadowFrustum;

            // Please never comment this lines !!!
            //if (m_LogFile)
            //  Logv(SRendItem::m_RecurseLevel, "*** Prepare shadow maps for REOcLeaf***\n");
            //if(obj && pFr && !m_RP.m_bDrawToTexture)
            //  PrepareDepthMap(pFr, false);
          }
        }
        break;
      case SPRID_CORONA:
        if (pr->m_nObject>0 && !m_RP.m_bDrawToTexture)
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
        break;
      case SPRID_PORTAL:
        {
          if (!CV_r_portals)
            continue;

          if (m_RP.m_RecurseLevel+1 >= MAX_PORTAL_RECURSES)
            continue;

          if (!CV_r_portalsrecursive && m_RP.m_CurWarp)
            continue;

          if (!m_RP.m_CurWarp)
          {
            nPortals = srfs.Num();
            srfs.AddIndex(1);
            srfs[nPortals].nobj = pr->m_nObject;
            srfs[nPortals].srf = pr->m_RE;
            srfs[nPortals].Shader = pr->m_Shader;
          }
          else
          {
            Plane pl;
            pr->m_RE->mfGetPlane(pl);
            if (m_RP.m_CurWarp->plane.d != pl.d || !IsEquivalent(m_RP.m_CurWarp->plane.n,pl.n))
            {
              nPortals = srfs.Num();
              srfs.AddIndex(1);
              srfs[nPortals].nobj = pr->m_nObject;
              srfs[nPortals].srf = pr->m_RE;
              srfs[nPortals].Shader = pr->m_Shader;
            }
          }
        }
        break;
      default:
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
        break;
    }
  }
  if (RIs.Num())
  {
    if (m_LogFile)
      Logv(SRendItem::m_RecurseLevel, "*** Preprocess pipeline ***\n");
    SRendItemPreprocess::mfSort(&RIs[0], RIs.Num());
    EF_DrawREPreprocess(&RIs[0], RIs.Num());
  }

  EF_FlushRefractedObjects(pSHRefr, pRERefr, pObjRefr, nRefrObjects, nRefractedStuff, DLDFlags);

  if (srfs.Num())
  {
    for (i=0; i<srfs.Num(); i++)
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

      EF_PreRender(1);

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

          if (!EF_ObjectChange(sh, Res, ws->nobj, ws->srf))
            continue;
          nObj = ws->nobj;
        }
        if (ws->Shader != sh || ws->ShaderRes != Res)
        {
          if (sh)
          {
            m_RP.m_pRenderFunc();
            sh = NULL;
          }

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

  unticks(time0);
  m_RP.m_PS.m_fPreprocessTime += (float)(time0*1000.0*g_SecondsPerCycle);

  //ResetToDefault();

  return nReturn;
}

void CD3D9Renderer::EF_EndEf2D(bool bSort)
{
  int i;
  SShader *Shader, *CurShader, *ShaderState, *CurShaderState;
  SRenderShaderResources *Res, *CurRes;

  EF_AddClientPolys2D();

  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_PREPROCESS_ID] = SRendItem::m_RendItems[EFSLIST_PREPROCESS_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_STENCIL_ID] = SRendItem::m_RendItems[EFSLIST_STENCIL_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_GENERAL_ID] = SRendItem::m_RendItems[EFSLIST_GENERAL_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_UNSORTED_ID] = SRendItem::m_RendItems[EFSLIST_UNSORTED_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_DISTSORT_ID] = SRendItem::m_RendItems[EFSLIST_DISTSORT_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_LAST_ID] = SRendItem::m_RendItems[EFSLIST_LAST_ID].Num();
  int nums = SRendItem::m_StartRI[SRendItem::m_RecurseLevel-1][EFSLIST_GENERAL_ID];
  int nume = SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_GENERAL_ID];

  if (bSort)
  {
    SRendItem::mfSort(&SRendItem::m_RendItems[0][0], nume);
    // If sort number of the first shader is 1 (eS_Preprocess)
    // run preprocess operations for the current frame
    if ((SRendItem::m_RendItems[0][nums].SortVal.i.High >> 26) == eS_PreProcess)
      nums += EF_Preprocess(&SRendItem::m_RendItems[0][0], nums, nume);
  }

  EF_PreRender(1);
  //ResetToDefault();

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
