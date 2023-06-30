// Layer.h: interface for the CLayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LAYER_H__92D64D54_E1C2_4D48_9664_8BD50F7324F8__INCLUDED_)
#define AFX_LAYER_H__92D64D54_E1C2_4D48_9664_8BD50F7324F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//! Single texture layer
class CLayer : public CObject
{
public:
	CLayer();
	virtual ~CLayer();

	// Name
	CString GetLayerName() { return m_strLayerName; };
	void SetLayerName(CString strName) { m_strLayerName = strName; };

	// Slope
	float GetLayerMinSlope() { return m_minSlope; };
	float GetLayerMaxSlope() { return m_maxSlope; };
	void SetLayerMinSlope( float min ) { m_minSlope = min; InvalidateMask(); };
	void SetLayerMaxSlope( float max ) { m_maxSlope = max; InvalidateMask(); };

	// Altitude
	UINT GetLayerStart() { return m_iLayerStart; };
	UINT GetLayerEnd() { return m_iLayerEnd; };
	void SetLayerStart(UINT iStart) { m_iLayerStart = iStart; InvalidateMask(); };
	void SetLayerEnd(UINT iEnd) { m_iLayerEnd = iEnd; InvalidateMask(); };

	//! Calculate memory size allocated for this layer.
	int GetSize() const;

	//////////////////////////////////////////////////////////////////////////
	// Layer Mask
	//////////////////////////////////////////////////////////////////////////
	unsigned char GetLayerMaskPoint( uint x,uint y ) { return m_layerMask.ValueAt(x,y); }
	void SetLayerMaskPoint( uint x,uint y,unsigned char c ) { m_layerMask.ValueAt(x,y) = c; }

	//////////////////////////////////////////////////////////////////////////
	// Update current mask.
	//////////////////////////////////////////////////////////////////////////
	//! Update current mask for this sector and put it into target mask data.
	//! @param mask Mask image returned from function, may not be not initialized when calling function.
	//! @return true is mask for this sector exist, false if not.
	bool UpdateMaskForSector( CPoint sector,const CRect &sectorRect,const CFloatImage &hmap,CByteImage& mask );
	bool UpdateMask( const CFloatImage &hmap,CByteImage& mask );
	CByteImage& GetMask();
	//////////////////////////////////////////////////////////////////////////

	void UpdateLayerMask16(float *pHeightmapPixels, UINT iHeightmapWidth,UINT iHeightmapHeight, bool bStreamFromDisk);
	void GenerateWaterLayer16(float *pHeightmapPixels, UINT iHeightmapWidth,UINT iHeightmapHeight,float waterLevel );

	//////////////////////////////////////////////////////////////////////////
	int GetMaskResolution() const { return m_maskResolution; };
	
	// Texture
	int GetTextureWidth() { return m_cTextureDimensions.cx; };
	int GetTextureHeight() { return m_cTextureDimensions.cy; };
	CSize GetTextureDimensions() { return m_cTextureDimensions; };
	CString GetTextureFilename();
	void DrawLayerTexturePreview(LPRECT rcPos, CDC *pDC);
	bool LoadTexture(CString strFileName);
	void FillWithColor( COLORREF col,int width,int height );
	bool LoadTexture(LPCTSTR lpBitmapName, UINT iWidth, UINT iHeight);
	bool LoadTexture(DWORD *pBitmapData, UINT iWidth, UINT iHeight);
	void ExportTexture(CString strFileName);
	void ExportMask( const CString &strFileName );
	bool HasTexture() { return GetTextureWidth() != NULL; };
	
	uint& GetTexturePixel( int x,int y ) { return m_texture.ValueAt(x,y); };


	//! Load a BMP texture into the layer mask.
	bool LoadMask( const CString &strFileName );
	void GenerateLayerMask( CByteImage &mask,int width,int height );

	//! Release allocated mask.
	void	ReleaseMask();
	
	// Serialisation
	void Serialize( CXmlArchive &xmlAr );

	// Call if mask was Modified.
	void InvalidateMask();
	//! Invalidate one sector of the layer mask.
	void InvalidateMaskSector( CPoint sector );

	// Check if layer is valid.
	bool IsValid() { return m_layerMask.IsValid(); }

	// In use
	bool IsInUse() { return m_bLayerInUse; };
	void SetInUse(bool bNewState) { m_bLayerInUse = bNewState; };

	bool IsAutoGen() const { return m_bAutoGen; };
	void SetAutoGen( bool bAutoGen );

	//////////////////////////////////////////////////////////////////////////
	// True if layer is currently selected.
	bool IsSelected() const { return m_bSelected; }
	//! Mark layer as selected or not.
	void SetSelected( bool bSelected ) { m_bSelected = bSelected; };
	
	//////////////////////////////////////////////////////////////////////////
	void SetSurfaceType( const CString &sfType ) { m_surfaceType = sfType; }
	CString GetSurfaceType() { return m_surfaceType; }

	void SetSmooth( bool bSmooth ) { m_bSmooth = bSmooth; InvalidateMask(); };
	bool IsSmooth() const { return m_bSmooth; };

	//! Load texture if it was unloaded.
	void PrecacheTexture();
	//! Load mask if it was unloaded.
	void PrecacheMask();

	//////////////////////////////////////////////////////////////////////////
	//! Mark all layer mask sectors as invalid.
	void InvalidateAllSectors();
	void SetAllSectorsValid();

	//! Compress mask.
	void CompressMask();

	//! Export layer block.
	void ExportBlock( const CRect &rect,CXmlArchive &ar );
	//! Import layer block.
	void ImportBlock( CXmlArchive &ar,CPoint offset );

	//! Allocate mask grid for layer.
	void AllocateMaskGrid();

protected:
	enum SectorMaskFlags
	{
		SECTOR_MASK_VALID = 1,
	};

	// Convert the layer from BGR to RGB
	void BGRToRGB();

	uchar& GetSector( CPoint sector );

	//! Autogenerate layer mask.
	void AutogenLayerMask( const CRect &rect,const CFloatImage &hmap,CByteImage& mask );

	//! Get native resolution for layer mask (For not autogen levels).
	int GetNativeMaskResolution() const;

private:
	// Name
	CString m_strLayerName;

	// Layer texture
	CBitmap m_bmpLayerTexPrev;
	CDC m_dcLayerTexPrev;
	CString m_strLayerTexPath;
	CSize m_cTextureDimensions;
	//DWORD *m_pdwLayerTextureData;

	CImage m_texture;

	// Layer parameters
	UINT m_iLayerStart;
	UINT m_iLayerEnd;

	// Slope
	float m_minSlope;
	float m_maxSlope;

	uint m_currWidth;
	uint m_currHeight;

	//////////////////////////////////////////////////////////////////////////
	// Mask.
	//////////////////////////////////////////////////////////////////////////
	// Layer mask data
	CString m_maskFile;
	CByteImage m_layerMask;
	//! Mask used when scaled mask version is needed.
	CByteImage m_scaledMask;

	//! Native layer resolution (not for autogenerated layers).
	//int m_nativeResolution;
	int m_maskResolution;
	
	//BYTE *m_pLayerMask;
	bool m_bNeedUpdate;

	bool m_bCompressedMaskValid;
	CMemoryBlock m_compressedMask;

	// Should this layer be used during terrain generation ?
	bool m_bLayerInUse;

	bool m_bAutoGen;
	bool m_bNoise;
	float m_noiseSize;

	//! True if need to smooth layer.
	bool m_bSmooth;
	
	//! True if layer is selected as current.
	bool m_bSelected;

	CString m_surfaceType;

	//! Internal instance count, used for Uniq name assignment.
	static UINT m_iInstanceCount;

	//////////////////////////////////////////////////////////////////////////
	// Layer Sectors.
	//////////////////////////////////////////////////////////////////////////
	std::vector<unsigned char> m_maskGrid;
	int m_numSectors;
};

#endif // !defined(AFX_LAYER_H__92D64D54_E1C2_4D48_9664_8BD50F7324F8__INCLUDED_)