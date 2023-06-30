////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   TextureTool.h
//  Version:     v1.00
//  Created:     11/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Terrain modification tool.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __TextureTool_h__
#define __TextureTool_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "EditTool.h"

// forward declarations.
struct IStatObj;

//////////////////////////////////////////////////////////////////////////
//! CTextureTool is a tool that allow to change texture properties on static objects.
//
class CTextureTool : public CEditTool
{
	DECLARE_DYNCREATE(CTextureTool)
public:
	CTextureTool();
	virtual ~CTextureTool();

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

	void Modify();

private:
	//! Single selected element (Object + Face).
	struct SelectedElem
	{
		// Selected static object.
		IStatObj *object;
		//! World position of selected object.
		Vec3 pos;
		//! Index of selected face.
		int face;
		Vec3 v[3];
	};

	void PickSelection( Vec3 &rayOrg,Vec3 &rayDir,bool remove=false );
	int	GetSelection( IStatObj *obj,int faceIndex ) const;

	Vec3 m_pointerPos;
	float m_brushRadius;
	bool m_bRemoveSelection;

	Vec3 m_raySrc,m_rayTrg;

	std::vector<SelectedElem> m_selection;

	IEditor *m_ie;

	int m_panelId;
	class CTexturePanel *m_panel;
};


#endif // __TextureTool_h__
