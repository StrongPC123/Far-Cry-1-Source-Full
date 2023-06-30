////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   detail_grass.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _DETAILGRASS_H_
#define _DETAILGRASS_H_

class CVertexBuffer;
struct GrassType;
class CTerrain;
struct CLeafBuffer;

const int   DETAIL_GRASS_PIP_BUFFER_SIZE = 50000;
const float CAMERA_GRASS_SHIFT = 9;

class CDetailGrass : public Cry3DEngineBase
{
  void CreateSectorGrass(const int range, const int step, struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pGrassVertices);
  void CreateSectorGrassInUnit(const int x, const int y, const int nStep, 
		struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pGrassVertices);
  void AddIndexedArray(GrassType * o, float X, float Y, float Z, float fbr, float fSizeRatio, 
		float fXSign, float fYSign, int nSwapXY, struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pGrassVertices);

  list2<unsigned short> m_GrassIndices;
//  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * m_pGrassVertices;
  int m_GrassVerticesCount;
  int m_GrassFocusX, m_GrassFocusY; // grass focus
  int m_GrassTID;
  list2<GrassType*> m_GrassModelsArray;
  int m_nGrassDensity;
  CTerrain * m_pTerrain;
  list2<GrassType*> m_arrlstSurfaceObjects[MAX_SURFACE_TYPES_COUNT];

  CLeafBuffer * m_pLeafBuffer;
  IShader * m_pShader;
  float m_arrfShaderInfo[16];

public:
  CDetailGrass(XDOM::IXMLDOMNodeListPtr pDetTexTagList);
  ~CDetailGrass();
  void RenderDetailGrass(CTerrain * pTerrain);
  void UpdateGrass() { m_GrassFocusX=m_GrassFocusY=-CTerrain::GetTerrainSize(); }
	void PreloadResources();

	static bool PrepareBufferCallback(CLeafBuffer * pLeafBuffer, bool bNeedTangents);
	// todo: update
	//void GetMemoryUsage(ICrySizer*pSizer);
	int GetMemoryUsage();
};

#endif // _DETAILGRASS_H_
