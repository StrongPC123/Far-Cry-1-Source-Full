////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   TerrainModifyTool.h
//  Version:     v1.00
//  Created:     11/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Terrain modification tool.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __TerrainModifyTool_h__
#define __TerrainModifyTool_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "EditTool.h"

// {9BC941A8-44F1-4b91-B270-E7EF55167E48}
DEFINE_GUID( TERRAIN_MODIFY_TOOL_GUID, 0x9bc941a8, 0x44f1, 0x4b91, 0xb2, 0x70, 0xe7, 0xef, 0x55, 0x16, 0x7e, 0x48);

enum BrushType
{
	eBrushFlatten = 1,
	eBrushSmooth,
	eBrushRiseLower,
	eBrushTypeLast
};

struct CTerrainBrush
{
	// Type of this brush.
	BrushType type;
	//! Outside Radius of brush.
	float radius;
	//! Inside Radius of brush.
	float radiusInside;
	//! Height where to paint.
	float height;
	//! How hard this brush.
	float hardness;
	//! Is this brush have noise.
	bool bNoise;	
	//! Scale of applied noise.
	float noiseScale;
	//! Frequency of applied noise.
	float noiseFreq;
	//! True if objects should be repositioned on modified terrain.
	bool bRepositionObjects;

	CTerrainBrush() {
		type = eBrushFlatten;
		radius  = 2;
		radiusInside = 1;
		height = 1;
		bNoise = false;
		hardness = 0.2f;
		noiseScale = 50;
		noiseFreq = 50;
		bRepositionObjects = false;
	}
};

//////////////////////////////////////////////////////////////////////////
class CTerrainModifyTool : public CEditTool
{
	DECLARE_DYNCREATE(CTerrainModifyTool)
public:
	CTerrainModifyTool();
	virtual ~CTerrainModifyTool();

	//! Register this tool to editor system.
	static void RegisterTool( CRegistrationContext &rc );

	virtual void BeginEditParams( IEditor *ie,int flags );
	virtual void EndEditParams();

	virtual void Display( DisplayContext &dc );

	// Ovverides from CEditTool
	bool MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );

	// Key down.
	bool OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags );
	bool OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags ) { return false; };
	bool OnSetCursor( CViewport *vp );
	
	// Delete itself.
	void Release() { delete this; };

	void SetActiveBrushType( BrushType type );
	BrushType GetActiveBrushType() const { return m_currentBrushType; }
	void SetBrush( const CTerrainBrush &brush ) { m_brush[m_currentBrushType] = brush; };
	void GetBrush( CTerrainBrush &brush ) const { brush = m_brush[m_currentBrushType]; };
	void EnableBrushNoise( bool enable ) { m_brush[m_currentBrushType].bNoise = enable; };

	void Paint();

private:
	void UpdateUI();
	//////////////////////////////////////////////////////////////////////////
	// Commands.
	static void Command_Activate();
	static void Command_Flatten();
	static void Command_Smooth();
	static void Command_RiseLower();
	//////////////////////////////////////////////////////////////////////////
	
	Vec3 m_pointerPos;
	IEditor *m_ie;

	int m_panelId;
	class CTerrainModifyPanel *m_panel;

	static BrushType m_currentBrushType;
	static CTerrainBrush m_brush[eBrushTypeLast];
	CTerrainBrush *m_pBrush;

	bool m_bSmoothOverride;
	bool m_bQueryHeightMode;
	bool m_bPaintingMode;
	bool m_bLowerTerrain;

	CPoint m_MButtonDownPos;
	float m_prevRadius;
	float m_prevRadiusInside;
	float m_prevHeight;
	
	HCURSOR m_hPickCursor;
	HCURSOR m_hPaintCursor;
	HCURSOR m_hFlattenCursor;
	HCURSOR m_hSmoothCursor;
};


#endif // __TerrainModifyTool_h__
