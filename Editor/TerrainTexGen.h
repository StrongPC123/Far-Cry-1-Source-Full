////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   TerrainTexGen.h
//  Version:     v1.00
//  Created:     8/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __terraintexgen_h__
#define __terraintexgen_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "TerrainGrid.h"

// forward declaration.
class CLayer;
struct LightingSettings;
class CTerrainGrid;

enum ETerrainTexGenFlags
{
	ETTG_LIGHTING					= 0x001,
	ETTG_SHOW_WATER				= 0x002,
	ETTG_KEEP_LAYERMASKS	= 0x004,
	ETTG_ABGR							= 0x008,
	ETTG_STATOBJ_SHADOWS	= 0x010,
	ETTG_QUIET						= 0x020, //!< Disable all logging and progress indication.
	ETTG_INVALIDATE_LAYERS = 0x040,//!< Invalidate stored layers information.
	ETTG_NOTEXTURE				= 0x080,	//!< Not generate texture information (Only lighting).
	ETTG_USE_LIGHTMAPS		= 0x100,	//!< Use lightmaps for sectors.
	ETTG_STATOBJ_PAINTBRIGHTNESS = 0x200,	//!< Paint brightness of vegetation objects.
	ETTG_FAST_LLIGHTING = 0x400,	//!< Use fastest possible lighting algorithm.
	ETTG_NO_TERRAIN_SHADOWS	= 0x0800, //! Do not calculate terrain shadows (Even if specified in lighting settings)
};

/** Class that generates terrain surface texture.
*/
class CTerrainTexGen
{
public:
	CTerrainTexGen();
	CTerrainTexGen( int resolution );
	~CTerrainTexGen();

	/** Set lighting array to be used for this texture generator.
	*/
	void SetLightingBits( CBitArray *array );

	/** Generate terrain surface texture.
			@param surfaceTexture Output image where texture will be stored, it must already be allocated.
				to the size of surfaceTexture.
			@param sector Terrain sector to generate texture for.
			@param rect Region on the terrain where texture must be generated within sector..
			@return true if texture was generated.
	*/ 
	bool GenerateSectorTexture( CPoint sector,const CRect &rect,int flags,CImage &surfaceTexture );

	//! Generate whole surface texture.
	bool GenerateSurfaceTexture( int flags,CImage &surfaceTexture );

	//! Query layer mask for pointer to layer.
	CByteImage* GetLayerMask( CLayer* layer );

	//! Invalidate layer valid flags for all sectors..
	void InvalidateLayers();

	//! Invalidate all lighting valid flags for all sectors..
	void InvalidateLighting();

private:
	void Init( int resolution );

	struct SLayerInfo
	{
		CLayer* pLayer;
		// Layer mask for this layer.
		CByteImage *layerMask;
		SLayerInfo()
		{
			pLayer = 0;
			layerMask = 0;
		}
	};
	struct SectorInfo
	{
		int flags;
		CImage *lightmap; // Lightmap for this sector.
	};

	//////////////////////////////////////////////////////////////////////////
	// Layers.
	//////////////////////////////////////////////////////////////////////////
	int GetLayerCount() const;
	CLayer* GetLayer( int id ) const;
	void ClearLayers();

	/** Prepare texture layers for usage.
			Autogenerate layer masks that needed,
			Rescale layer masks to requested size.
	*/
	void UpdateSectorLayers( CPoint sector );

	//////////////////////////////////////////////////////////////////////////

//	void GetLightmapTexture( CPoint sector, CImage &surfaceTexture, const CRect &rect, const CImage &lightmap );

	void BlendLightmap( CPoint sector,CImage &surfaceTexture,const CRect &rect,const CImage &lightmap,const Vec3 &blendColor );
	void ConvertToABGR( CImage &surfaceTexture,const CRect &rect );
	bool IsOccluded( float *pHeightmapData, int iWidth, int iHeight,int iStartX, int iStartY, int lightDirX,int lightDirY,float fClimbPerStep );

	
	//! calculate the shadow (only hills)
	//! \param inpHeightmapData must not be 0
	//! \param iniResolution
	//! \param iniShift
	//! \param iniX [0..iniWidth-1]
	//! \param iniY [0..iniHeight-1]
	//! \param invSunVector normalized vector to the sun
	//! \param infShadowBlur defines the slope blurring, 0=no blurring, .. (angle would be better but is slower)
	//! \return 0..1
	float GetSunAmount( const float *inpHeightmapData,int iniX, int iniY, float infInvHeightScale,
		const Vec3& vSunShadowVector, const float infShadowBlur ) const;
/*
	//! noisy result, without precalculation
	//! calculate the amount of sky hemisphere (only hills)
	//! \param inpHeightmapData must not be 0
	//! \param iniWidth
	//! \param iniHeight
	//! \param iniX [0..iniWidth-1]
	//! \param iniY [0..iniHeight-1]
	//! \param iniQuality 0=returns always 1.0f (no sampling), 1=low quality, 10=highest quality
	//! \return 0.0f=no hemisphere visible .. 1.0f =full hemisphere visible
	float GetSkyAccessibilityNoise( const float *inpHeightmapData, const int iniWidth, const int iniHeight,
		const int iniX, const int iniY, const int iniQuality, const float infHeightScale ) const;
*/

	//! use precalculated data, bilinear filtered
	//! \param iniX [0..m_resolution-1]
	//! \param iniY [0..m_resolution-1]
	//! \return 0.0f=no hemisphere visible .. 1.0f =full hemisphere visible
	float GetSkyAccessibilityFast( const int iniX, const int iniY ) const;

	//! use precalculated data, bilinear filtered
	//! \param iniX [0..m_resolution-1]
	//! \param iniY [0..m_resolution-1]
	//! \return 0.0f=no sun visible .. 1.0f =full sun visible
	float GetSunAccessibilityFast( const int iniX, const int iniY ) const;

	// Calculate lightmap for sector.
	bool GenerateLightmap( CPoint sector,LightingSettings *ls,CImage &lightmap,int genFlags,CBitArray *pLightingBits=0 );
	void GenerateShadowmap( CPoint sector,CByteImage &shadowmap,float shadowAmmount,const Vec3 &sunVector );
	void UpdateSectorHeightmap( CPoint sector );
	bool UpdateWholeHeightmap();
	//! Caluclate max terrain height (Optimizes calculation of shadows and sky accessibility).
	void CalcTerrainMaxZ();

	//! Log generation progress.
	void Log( const char *format,... );

	//! you have to do it for the whole map and that can take a min but using the result is just a lookup
	//! upate m_SkyAccessiblity and m_SunAccessiblity
	//! \return true=success, false otherwise
	bool RefreshAccessibility( const LightingSettings *inpLS,int genFlags );

public:
	//////////////////////////////////////////////////////////////////////////
	// Sectors.
	//////////////////////////////////////////////////////////////////////////
	//! Get rectangle for this sector.
	void GetSectorRect( CPoint sector,CRect &rect );
	//! Get terrain sector.
	int GetSectorFlags( CPoint sector );
	//! Get terrain sector info.
	SectorInfo& GetSectorInfo( CPoint sector );
	//! Add flags to sector.
	void SetSectorFlags( CPoint sector,int flags );
	//! Calculate terrain heightap data for sector.
	void InvalidateSector( CPoint sector );
	//////////////////////////////////////////////////////////////////////////

private:
	//////////////////////////////////////////////////////////////////////////
	// Vars.
	//////////////////////////////////////////////////////////////////////////
	CBitArray *m_pLightingBits;

	bool m_bLog;
	bool m_bNotValid;

	//! Target texture resolution.
	unsigned int m_resolution;
	//! (1 << m_resolutionShift) == m_resolition.
	unsigned int m_resolutionShift;
	//! Resolution of sector.
	int m_sectorResolution;
	//! Number of sectors per side.
	int m_numSectors;

	//! Size of one pixel on generated texture in meters.
	float m_pixelSizeInMeters;

	//! Highest point on terrain. use CalcTerrainMaxZ() to update this value
	float m_terrainMaxZ;

	// Used Layers.
	std::vector<SLayerInfo> m_layers;

	//! If have water leyer.
	CLayer *m_waterLayer;

	//! Heightmap for this terrain(not scaled up to the target resolution)
	CHeightmap *m_heightmap;
	CVegetationMap *m_vegetationMap;

	//! Sector grid.
	std::vector<SectorInfo> m_sectorGrid;

	//! Local heightmap (scaled up to the target resolution)
	CFloatImage m_hmap;

	// ----------------------------------------------------

	//! Amount of sky that is visible for each point (4MB for 2048x2048)
	//! (not scaled up from source, but might be scaled down), 0=no sky accessible, 255= full sky accessible
	CByteImage m_SkyAccessiblity;
	//! to detect changes to refresh m_SkyAccessiblity
	int	m_iCachedSkyAccessiblityQuality;

	// ----------------------------------------------------

	//! Amount of sun that is visible for each point (4MB for 2048x2048)
	//! (not scaled up from source, but might be scaled down), 0=no sun accessible, 255= full sun accessible
	CByteImage m_SunAccessiblity;
	//! to detect changes to refresh m_SunAccessiblity
	int	m_iCachedSunBlurLevel;
	//! to detect changes to refresh m_SunAccessiblity
	Vec3 m_vCachedSunDirection;

	// ----------------------------------------------------

};


#endif // __terraintexgen_h__
