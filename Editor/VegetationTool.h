////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   VegetationTool.h
//  Version:     v1.00
//  Created:     11/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Places vegetation on terrain.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __VegetationTool_h__
#define __VegetationTool_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "EditTool.h"

class CVegetationObject;
struct CVegetationInstance;
class CVegetationMap;

//////////////////////////////////////////////////////////////////////////
class CVegetationTool : public CEditTool
{
	DECLARE_DYNCREATE(CVegetationTool)
public:
	CVegetationTool();
	virtual ~CVegetationTool();

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

	void SetMode( bool paintMode );

	void Distribute();
	void DistributeMask( const char *maskFile );
	void Clear();
	void ClearMask( const char *maskFile );

	void ScaleObjects();

	void HideSelectedObjects( bool bHide );
	void RemoveSelectedObjects();

	void PaintBrush();
	void PlaceThing();
	void GetSelectedObjects( std::vector<CVegetationObject*> &objects );

	void ClearThingSelection();

	static void RegisterTool( CRegistrationContext &rc );

private:
	void SelectThing( CVegetationInstance *thing,bool bSelObject=true );
	bool SelectThingAtPoint( CViewport *view,CPoint point,bool bNotSelect=false );
	void SelectThingsInRect( CViewport *view,CRect rect );
	void MoveSelected( const Vec3 &offset,bool bFollowTerrain );
	void ScaleSelected( float fScale );

	// Specific mouse events handlers.
	bool OnLButtonDown( CViewport *view,UINT nFlags,CPoint point );
	bool OnLButtonUp( CViewport *view,UINT nFlags,CPoint point );
	bool OnMouseMove( CViewport *view,UINT nFlags,CPoint point );

	static void Command_Activate();

	CPoint m_mouseDownPos;
	CPoint m_mousePos;
	CPoint m_prevMousePos;
	Vec3 m_pointerPos;
	static float m_brushRadius;

	bool m_bPlaceMode;
	bool m_bPaintingMode;

	enum OpMode {
		OPMODE_NONE,
		OPMODE_PAINT,
		OPMODE_SELECT,
		OPMODE_MOVE,
		OPMODE_SCALE
	};
	OpMode m_opMode;

	IEditor *m_ie;
	std::vector<CVegetationObject*> m_selectedObjects;

	CVegetationMap*	m_vegetationMap;

	//! Selected vegetation instances
	std::vector<CVegetationInstance*> m_selectedThings;

	int m_panelId;
	class CVegetationPanel *m_panel;
	int m_panelPreviewId;
	class CPanelPreview *m_panelPreview;
};


#endif // __VegetationTool_h__
