// Heightmap.h: interface for the CHeightmap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HEIGHTMAP_H__F92AE690_FC38_4249_BEA9_51384504DF9E__INCLUDED_)
#define AFX_HEIGHTMAP_H__F92AE690_FC38_4249_BEA9_51384504DF9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Heightmap data type
typedef float t_hmap;

#define NUM_SECTORS 32 //!< Sector table dimensions (32*32)

// From Engine.
//! Surface type mask, masks bits resereved to store surface id.
#define SURFACE_TYPE_MASK (1|2|4)
//! Lighting bit, if not set this terrain pixel is considered in shadow.
#define TERRAIN_LIGHT_BIT 8
#define TERRAIN_BURNOUT_BIT 8
//! Heigtmap bitmask, masks all bits out of heightmap data reserved for special use.
#define TERRAIN_BITMASK (SURFACE_TYPE_MASK|TERRAIN_LIGHT_BIT)

enum EHeighmapInfo
{
	//! Hole flag in heighmap info.
	HEIGHTMAP_INFO_HOLE		= 0x01,
	//! Light flag in heighmap info.
	HEIGHTMAP_INFO_LIGHT	= 0x02,
};

//! Surface id mask in heighmap info.
#define HEIGHTMAP_INFO_SFTYPE_MASK (0x04|0x08|0x010)
//! Surface id shift in heighmap info.
#define HEIGHTMAP_INFO_SFTYPE_SHIFT	2

class CXmlArchive;
class CDynamicArray2D;
struct SNoiseParams;
class CVegetationMap;
class CTerrainGrid;

struct SSectorInfo
{
	//! Size of terrain unit.
	int unitSize;
	//! Sector size in meters.
	int sectorSize;
	//! Size of texture for one sector in pixels.
	int sectorTexSize;
	//! Number of sectors on one side of terrain.
	int numSectors;
	//! Size of whole terrain surface texture.
	int surfaceTextureSize;
};

/** Heightmap info.
*/
class CHeightmap  
{
public:
	// Construcktion / destrucktion
	CHeightmap();
	virtual ~CHeightmap();

	// Member data access
	uint GetWidth() { return m_iWidth; };
	uint GetHeight() { return m_iHeight; };

	//! Get size of every heightmap unit in meters.
	int GetUnitSize() const { return m_unitSize; };

	//! Convert from world coordinates to heightmap coordinates.
	CPoint WorldToHmap( const Vec3 &wp );
	//! Convert from heightmap coordinates to world coordinates.
	Vec3 HmapToWorld( CPoint hpos );

	//! Get access to vegetation map.
	CVegetationMap* GetVegetationMap();

	//! Set size of current surface texture.
	void SetSurfaceTextureSize( int width,int height );

	//! Returns information about sectors on terrain.
	//! @param si Structure filled with quered data.
	void GetSectorsInfo( SSectorInfo &si );
	
	t_hmap* GetData() { return m_pHeightmap; };
	bool GetDataEx(t_hmap *pData, UINT iDestWidth, bool bSmooth = true, bool bNoise = true);

	//! Fill image data.
	//! @param resolution Resolution of needed heightmap.
	//! @rect trgRect Target rectangle in scaled heightmap.
	//! @param image, preallocated image of same size as srcRect where heightmap data will be stored.
	bool GetData( CRect &trgRect,CFloatImage &hmap, bool bSmooth=true,bool bNoise=true );

	//////////////////////////////////////////////////////////////////////////
	// Terrain Grid functions.
	//////////////////////////////////////////////////////////////////////////
	CTerrainGrid* GetTerrainGrid() const { return m_terrainGrid; };
	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	void SetXY( uint x,uint y,t_hmap iVal ) { m_pHeightmap[x + y*m_iWidth] = iVal; };
	t_hmap& GetXY( uint x,uint y ) { return m_pHeightmap[x + y*m_iWidth]; };
	t_hmap GetXY( uint x,uint y ) const { return m_pHeightmap[x + y*m_iWidth]; };

	//! Calculate heightmap slope at givven point.
	//! @return clamped to 0-255 range slope value.
	float GetSlope( int x,int y );

	bool GetPreviewBitmap( DWORD *pBitmapData,int width,bool bSmooth = true, bool bNoise = true);

	unsigned char& InfoAt( int x,int y ) { return m_info.ValueAt(x,y); };
	const unsigned char& InfoAt( int x,int y ) const { return m_info.ValueAt(x,y); };
	unsigned char* GetInfoData() { return m_info.GetData(); };

	// Hold / fetch
	void Fetch();
	void Hold();
	bool Read(CString strFileName);

	// (Re)Allocate / deallocate
	void Resize( int iWidth, int iHeight,int unitSize,bool bCleanOld=true );
	void CleanUp();

	void InvalidateLayers();

	// Importing / exporting
	void Serialize( CXmlArchive &xmlAr,bool bSerializeVegetation );
	void SerializeVegetation( CXmlArchive &xmlAr  );
	void SaveImage( LPCSTR pszFileName, bool bNoiseAndFilter = true);
	void LoadBMP( LPCSTR pszFileName, bool bNoiseAndFilter = true);

	//! Save heightmap to PGM file format.
	void SavePGM( const CString &pgmFile );
	//! Load heightmap from PGM file format.
	void LoadPGM( const CString &pgmFile );

	//! Save heightmap in RAW format.
	void	SaveRAW(  const CString &rawFile );
	//! Load heightmap from RAW format.
	void	LoadRAW(  const CString &rawFile );
	
	// Actions
	void Smooth();
	void Smooth( CFloatImage &hmap,const CRect &rect );

	void Noise();
	void Normalize();
	void RemoveWater();
	void Invert();
	void MakeIsle();
	void SmoothSlope();
	void Randomize();
	void MakeBeaches();
	void LowerRange(float fFactor);
	void Flatten(float fFactor);
	void GenerateTerrain(const SNoiseParams &sParam);
	void Clear();

	//! Calculate surface type in rectangle.
	//! @param rect if Rectangle is null surface type calculated for whole heightmap.
	void CalcSurfaceTypes( const CRect *rect = NULL );
	void SetLightingBit( CBitArray &lightbits,int w,int h );
	void ClearLightingBit();

	// Drawing
	void DrawSpot(unsigned long iX, unsigned long iY, 
		uint8 iWidth, float fAddVal, float fSetToHeight = -1.0f,
		bool bAddNoise = false);

	void DrawSpot2( int iX, int iY, int radius,float insideRadius, float fHeigh,float fHardness=1.0f,bool bAddNoise = false,float noiseFreq=1,float noiseScale=1 );
	void SmoothSpot( int iX, int iY, int radius, float fHeigh,float fHardness );
	void RiseLowerSpot( int iX, int iY, int radius,float insideRadius, float fHeigh,float fHardness=1.0f,bool bAddNoise = false,float noiseFreq=1,float noiseScale=1 );

	//! Make hole in the terrain.
	void MakeHole( int x1,int y1,int width,int height,bool bMake );

	//! Export terrain block.
	void ExportBlock( const CRect &rect,CXmlArchive &ar );
	//! Import terrain block, return offset of block to new position.
	CPoint ImportBlock( CXmlArchive &ar,CPoint newPos,bool bUseNewPos=true );

	//! Update terrain block in engine terrain.
	void UpdateEngineTerrain( int x1,int y1,int width,int height,bool bElevation,bool bInfoBits );
	//! Update all engine terrain.
	void UpdateEngineTerrain( bool bOnlyElevation=true );

	//! Synchronize engine hole bit with bit stored in editor heightmap.
	void UpdateEngineHole( int x1,int y1,int width,int height );

	void SetWaterLevel( float waterLevel );
	float GetWaterLevel() const { return m_fWaterLevel; };

	void CopyData(t_hmap *pDataOut)
	{
		memcpy(pDataOut, m_pHeightmap, sizeof(t_hmap) * m_iWidth * m_iHeight);
	};

	//! Copy from different heightmap data.
	void CopyFrom( t_hmap *pHmap,unsigned char *pInfo,int resolution );

	//! Dump to log sizes of all layers.
	//! @return Total size allocated for layers.
	int LogLayerSizes();

protected:
	
	// Helper functionss
	__inline void ClampToAverage(t_hmap *pValue, float fAverage);
	__inline float ExpCurve(float v, float fCover, float fSharpness);

	void SetModified();

	// Verify internal class state
	__inline void Verify() 
	{ 
		ASSERT(m_iWidth && m_iHeight); 
		ASSERT(!IsBadWritePtr(m_pHeightmap, sizeof(t_hmap) * m_iWidth * m_iHeight));
	};

	// Initialization
	void InitNoise();

private:
	float m_fWaterLevel;

	t_hmap *m_pHeightmap;
	CDynamicArray2D *m_pNoise;
	
	CByteImage m_info;

	UINT m_iWidth;
	UINT m_iHeight;

	//! Size of surface texture.
	int m_textureSize;
	//! Number of sectors per grid side.
	int m_numSectors;
	int m_unitSize;

	// When heightmap is rescaled to different size,its cached for faster future retrieval.
	int m_cachedResolution;
	CMemoryBlock m_cachedHeightmap;

	//! Vegetation map for this heightmap.
	CVegetationMap *m_vegetationMap;

	//! Terrain grid.
	CTerrainGrid* m_terrainGrid;
};


//////////////////////////////////////////////////////////////////////////
// Inlined implementation of get slope.
inline float CHeightmap::GetSlope( int x,int y )
{
	//assert( x >= 0 && x < m_iWidth && y >= 0 && y < m_iHeight );
	if (x <= 0 || y <= 0 || x >= m_iWidth-1 || y >= m_iHeight-1)
		return 0;

	t_hmap *p = &m_pHeightmap[x+y*m_iWidth];
	float h = *p;
	int w = m_iWidth;
	float fs = (fabs(*(p+1)		- h) +
		          fabs(*(p-1)		- h) +
		          fabs(*(p+w)		- h) +
		          fabs(*(p-w)		- h) +
		          fabs(*(p+w+1)	- h) +
		          fabs(*(p-w-1) - h) +
       			  fabs(*(p+w-1) - h) +
	        	  fabs(*(p-w+1) - h));
	fs = fs*8;
	if (fs > 255.0f) fs = 255.0f;
	return fs;
};

#endif // !defined(AFX_HEIGHTMAP_H__F92AE690_FC38_4249_BEA9_51384504DF9E__INCLUDED_)