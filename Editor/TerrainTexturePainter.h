////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terraintexturepainter.h
//  Version:     v1.00
//  Created:     8/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __terraintexturepainter_h__
#define __terraintexturepainter_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "EditTool.h"
#include "TerrainTexGen.h"

class CHeightmap;
class CTerrainGrid;
class CLayer;

/** Terrain Texture brush types.
*/
enum ETextureBrushType
{
	ET_BRUSH_PAINT = 1,
	ET_BRUSH_SMOOTH,
};

/** Terrain texture brush.
*/
struct CTextureBrush
{
	// Type of this brush.
	ETextureBrushType type;
	//! Radius of brush.
	float radius;
	//! How hard this brush.
	float hardness;
	float value;
	bool bErase;
	// Options.
	bool bUpdateVegetation;
	bool bPreciseLighting;
	bool bTerrainShadows;
	bool bObjectShadows;

	// Max/Min.
	float maxRadius,minRadius;

	CTextureBrush()
	{
		bUpdateVegetation = true;
		type = ET_BRUSH_PAINT;
		radius = 2;
		hardness = 0.2f;
		value = 255;
		bErase = false;
		minRadius = 0.01f;
		maxRadius = 32;
		bPreciseLighting = true;
		bTerrainShadows = true;
		bObjectShadows = true;
	}
};

//////////////////////////////////////////////////////////////////////////
class CTerrainTexturePainter : public CEditTool
{
	DECLARE_DYNCREATE(CTerrainTexturePainter)
public:
	CTerrainTexturePainter();
	virtual ~CTerrainTexturePainter();

	virtual void BeginEditParams( IEditor *ie,int flags );
	virtual void EndEditParams();

	virtual void Display( DisplayContext &dc );

	// Ovverides from CEditTool
	bool MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );

	// Key down.
	bool OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags );
	bool OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags ) { return false; };
	
	// Delete itself.
	void Release() { delete this; };

	void SetBrush( const CTextureBrush &brush ) { m_brush = brush; };
	void GetBrush( CTextureBrush &brush ) const { brush = m_brush; };

	void Paint();

private:
	//////////////////////////////////////////////////////////////////////////
	// Private methods.
	//////////////////////////////////////////////////////////////////////////
	CLayer* GetSelectedLayer() const;

	void PaintSector( CPoint sector,CPoint texp,CLayer *pLayer );

	//////////////////////////////////////////////////////////////////////////
	Vec3 m_pointerPos;

	//! Number of sectors per side.
	int m_numSectors;
	//! Size of sector texture.
	int m_sectorTexSize;
	//! Size of sector in meters.
	int m_sectorSize;
	//! Texture Pixels per meter.
	float m_pixelsPerMeter;
	//! Size of whole terrain texture.
	int m_surfaceTextureSize;

	std::vector<unsigned char> m_texBlock;
	
	// Cache often used interfaces.
	I3DEngine *m_3DEngine;
	IRenderer *m_renderer;
	CHeightmap *m_heightmap;
	CTerrainGrid *m_terrainGrid;

	CTerrainTexGen m_terrTexGen;

	static CTextureBrush m_brush;
};

#endif // __terraintexturepainter_h__
