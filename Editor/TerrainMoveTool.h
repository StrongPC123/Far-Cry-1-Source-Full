////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   TerrainMoveTool.h
//  Version:     v1.00
//  Created:     11/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Terrain modification tool.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __TerrainMoveTool_h__
#define __TerrainMoveTool_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "EditTool.h"

//////////////////////////////////////////////////////////////////////////
class CTerrainMoveTool : public CEditTool
{
	DECLARE_DYNCREATE(CTerrainMoveTool)
public:
	CTerrainMoveTool();
	virtual ~CTerrainMoveTool();

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

	void Move();

	void SetArchive( CXmlArchive *ar );

private:
	Vec3 m_pointerPos;
	CXmlArchive* m_archive;

	CRect m_srcRect;

	IEditor *m_ie;
};


#endif // __TerrainMoveTool_h__
