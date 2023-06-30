////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   TerrainHoleTool.h
//  Version:     v1.00
//  Created:     11/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Terrain modification tool.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __TerrainHoleTool_h__
#define __TerrainHoleTool_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "EditTool.h"

//////////////////////////////////////////////////////////////////////////
class CTerrainHoleTool : public CEditTool
{
	DECLARE_DYNCREATE(CTerrainHoleTool)
public:
	CTerrainHoleTool();
	virtual ~CTerrainHoleTool();

	virtual void BeginEditParams( IEditor *ie,int flags );
	virtual void EndEditParams();

	virtual void Display( DisplayContext &dc );

	// Ovverides from CEditTool
	bool MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );

	// Key down.
	bool OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags );
	bool OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags );
	
	// Delete itself.
	void Release() { delete this; };

	void SetBrushRadius( float r ) { m_brushRadius = r; };
	float GetBrushRadius() const { return m_brushRadius; };

	void SetMakeHole( bool bEnable ) { m_bMakeHole = bEnable; }

	void Modify();

private:
	Vec3 m_pointerPos;
	float m_brushRadius;
	bool m_bMakeHole;

	IEditor *m_ie;

	int m_panelId;
	class CTerrainHolePanel *m_panel;
};


#endif // __TerrainHoleTool_h__
