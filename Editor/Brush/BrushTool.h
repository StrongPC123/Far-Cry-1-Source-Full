////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   BrushTool.h
//  Version:     v1.00
//  Created:     11/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Terrain modification tool.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __BrushTool_h__
#define __BrushTool_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "..\EditTool.h"

//////////////////////////////////////////////////////////////////////////
class CBrushTool : public CEditTool
{
	DECLARE_DYNCREATE(CBrushTool)
public:
	CBrushTool();
	virtual ~CBrushTool();

	//////////////////////////////////////////////////////////////////////////
	// CEditTool overrides.
	//////////////////////////////////////////////////////////////////////////
	virtual void BeginEditParams( IEditor *ie,int flags );
	virtual void EndEditParams();

	virtual void Display( DisplayContext &dc );
	virtual bool MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );

	// Key down.
	virtual bool OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags );
	virtual bool OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags );
	
	// Delete itself.
	virtual void Release() { delete this; };
	//////////////////////////////////////////////////////////////////////////
	
private:
	void NewBrush( CViewport *view,CPoint point );
	void StretchBrush( CViewport* view,CPoint point );
	void CalcLastBrushSize();

	// Specific mouse events handlers.
	bool OnLButtonDown( CViewport *view,UINT nFlags,CPoint point );
	bool OnLButtonUp( CViewport *view,UINT nFlags,CPoint point );
	bool OnMouseMove( CViewport *view,UINT nFlags,CPoint point );

	//! Operation modes of this viewport.
	enum BrushOpMode
	{
		BrushNoMode = 0,
		BrushMoveMode,
		BrushCreateMode,
		BrushStretchMode,
	};

	
	IEditor *m_IEditor;

	//////////////////////////////////////////////////////////////////////////
	//! Bounds of last selected brush or selection.
	BBox m_lastBrushBounds;

	//! Current brush tool operation mode.
	BrushOpMode m_mode;
	bool m_bMouseCaptured;
	CPoint m_mouseDownPos;
};


#endif // __BrushTool_h__
