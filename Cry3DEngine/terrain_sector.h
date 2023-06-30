////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain_sector.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef SECINFO_H
#define SECINFO_H

#define SHADOW_SHIFT_K     0.57f

#define ZERO_MML_LEVEL 0
#define MAX_MML_LEVEL  3
#define MAX_TEX_MML_LEVEL  1

#define DETAIL_TEX_DISTANCE (0.065f*m_fMaxViewDist)//64

#define MIN_ALLOWED_MERGED_SECTORS_DISTANCE 512

#define MAX_DETAIL_LAYERS_IN_SECTOR 7

#include "basicarea.h"

struct CSectorBeachInfo
{
  bool m_bBeachPresent;

  struct BeachPairStruct 
  { 
    BeachPairStruct () { ZeroStruct(*this); }
    Vec3d pos, water_dir; float distance; int busy; 
    Vec3d pos1,posm,pos2;
  };
#define MAX_BEACH_GROUPS 16
  list2<BeachPairStruct> m_arrlistBeachVerts[MAX_BEACH_GROUPS];
  list2<BeachPairStruct> m_lstUnsortedBeachVerts; // tmp
 
  CLeafBuffer * m_pLeafBufferBeach;
};

struct CSectorInfo : public CBasicArea, CSectorBeachInfo
{
public:
  CSectorInfo(CTerrain * pTerrain );
  ~CSectorInfo();

  CTerrain * m_pTerrain;
  float m_fDistance; // curr distance to sector
  int m_nOriginX, m_nOriginY; // sector origin
  float m_fMaxZ,m_fMidZ,m_fMinZ;  // sector bounds
  uchar m_cGeometryMML, m_cTextureMML, m_cNewTextMML; // calculated lods
  uchar m_bGroundVisible; // for finer occlusion
  unsigned int m_nTextureID, m_nLowLodTextureID; // curr texture id and always loaded low lod texture id
  int m_cLastTimeUsed, m_cLastTimeRendered; // unload sector after x sec of not in use
  bool m_bAllStaticsInFrustum;  // entire sector is visible, but maybe not dynamics
  OcclusionTestClient m_OcclusionTestClient;
  uchar m_cLastFrameUsed;
  bool m_bHasHoles; // sector has holes in the ground

  void SetTextures(bool bMakeUncompressedForEditing=false);
  void SetLOD();
  void SetMinMaxMidZ();
  void InitSectorBoundsAndErrorLevels(int _x1, int _y1, FILE * geom_file_to_read, FILE * geom_file_to_write);
  void CheckGeomCompWithLOD(int minMML);
  void RenderSector(CCObject * pTerrainCCObject);
  void MergeSectorIntoLowResTerrain(bool bCalcFarTerrain);
  void RenderEntities(CObjManager * pObjManager, bool bNotAllInFrustum, char*fake, int nStatics);

  void ReleaseHeightMapVertBuffer();
  int GetSecIndex() { return (m_nOriginX/CTerrain::GetSectorSize())*CTerrain::GetSectorsTableSize() + (m_nOriginY/CTerrain::GetSectorSize()); }

protected:

  float m_arrGeomErrors[MAX_MML_LEVEL+1]; // precalculated errors for each lod level

  int m_nSecVertsCount;

  struct CStripInfo {int begin,end;};

  struct CArrayInfo 
  { 
    list2<CStripInfo> strip_info; list2<unsigned short> idx_array; 
    inline void Clear() { strip_info.Clear(); idx_array.Clear(); } 
    inline void AddIndex(int _x, int _y, int _step)
    {
      unsigned short id = _x/_step*(CTerrain::GetSectorSize()/_step+1) + _y/_step;
      idx_array.Add(id);
    }

    inline void BeginStrip()
    {
      CStripInfo si;
      si.begin = idx_array.Count();
      si.end   = 0;
      strip_info.Add(si);
    }

    inline void EndStrip()
    {
      assert(strip_info.Count()); 
      strip_info.Last().end = idx_array.Count();
    }

		void GetMemoryUsage(ICrySizer* pSizer)
		{
			pSizer->AddContainer (strip_info);
			pSizer->AddContainer (idx_array);
		}
  };

  CArrayInfo m_ArrayInfo;//, low_array;
  void DrawArray(CArrayInfo * pArrayInfo, CCObject * pTerrainCCObject);
  void UpdateVarBuffer();
  void FillBuffer(int step);  
  int GetMML(int dist, int mmMin, int mmMax);

  int MakeSectorTextureDDS( int sec_id, int mml, bool bMakeUncompressedForEditing );

public:

  uint m_nDynLightMaskNoSun;
  uint m_nDynLightMask;

  void RenderBeach(IShader * pShader, float fZoomFactor, float fCamZ);
  void MakeBeachStage1();
  void MakeBeachStage2(FILE * hFileToSave);  
  void LoadBeach(FILE * hFileToLoad);

	CLeafBuffer * m_pLeafBuffer;
#define ARR_TEX_OFFSETS_SIZE 64
  float m_arrTexOffsets[ARR_TEX_OFFSETS_SIZE];
  IShader * m_pCurShader;

  struct VolumeInfo * m_pFogVolume;
  
  list2<unsigned short> m_lstLowResTerrainIdxArray[2];
	uint m_nLowResTerrainIdxRange[2][2]; // for dx optimizations
  void AddLowResSectorIndex(int _x, int _y, int _step, int _lod);

  int m_nLastMergedFrameID;
  void RemoveSectorTextures(bool bRemoveLowLod);
	void GenerateIndicesForQuad(int x1, int y1, int x2, int y2, CArrayInfo * pArrayInfo, ShadowMapFrustum * pFrustum, Vec3d * pvFrustPos, float fFrustScale);
	CLeafBuffer * MakeSubAreaLeafBuffer(const Vec3d & vPos, float fRadius, CLeafBuffer * pPrevLeafBuffer, IShader * pShader, bool bRecalcLeafBufferconst, const char * szLSourceName, ShadowMapFrustum * pFrustum, Vec3d * pvFrustPos, float fFrustScale);
  CArrayInfo m_ArrayInfo_MSALB;
	int LockSectorTexture(int & nTexDim);
	bool m_bLockTexture; // for editor

	uint GetLastTimeUsed() { return m_cLastTimeUsed; }
	uint GetLastTimeRendered() { return m_cLastTimeRendered; }
	void GetMemoryUsage(ICrySizer*pSizer);

  DetailTexInfo * m_arrDetailTexInfo[MAX_DETAIL_LAYERS_IN_SECTOR];
  void SetDetailLayersPalette();
	void UnloadHeighFieldTexture(float fDistanse, float fMaxViewDist);

	const void * GetShoreGeometry(int & nPosStride, int & nVertCount);

	uchar m_cPrevGeomMML; // current mml code
	uchar m_cCurrBoundCode;
};

struct CTerrainNode : public CSectorInfo
{
	CTerrainNode(CTerrain * pTerrain, int x1, int y1, int nNodeSize, int nTreeLevel, CTerrainNode * pParent) : CSectorInfo(pTerrain)
	{
		m_pParent = pParent;

		InitSectorBoundsAndErrorLevels(x1, y1, NULL, NULL);

		if(nNodeSize == CTerrain::GetSectorSize())
			memset(m_arrChilds,0,sizeof(m_arrChilds));
		else
		{
			int nSize = nNodeSize / 2;
			m_arrChilds[0] = new CTerrainNode(pTerrain, x1			, y1			, nSize, nTreeLevel, this);
			m_arrChilds[1] = new CTerrainNode(pTerrain, x1+nSize, y1			, nSize, nTreeLevel, this);
			m_arrChilds[2] = new CTerrainNode(pTerrain, x1			, y1+nSize, nSize, nTreeLevel, this);
			m_arrChilds[3] = new CTerrainNode(pTerrain, x1+nSize, y1+nSize, nSize, nTreeLevel, this);
		}
	}

	CTerrainNode * m_arrChilds[4];
	CTerrainNode * m_pParent;
};

#endif SECINFO_H
