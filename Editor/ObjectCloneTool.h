////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ObjectCloneTool.h
//  Version:     v1.00
//  Created:     18/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Definition of ObjectCloneTool, edit tool for cloning of objects..
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ObjectCloneTool_h__
#define __ObjectCloneTool_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "EditTool.h"

class CBaseObject;

/*!
 *	CObjectCloneTool, When created duplicate current selection, and manages cloned selection.
 *	
 */

class CObjectCloneTool : public CEditTool
{
public:
	DECLARE_DYNCREATE(CObjectCloneTool)

	CObjectCloneTool();
	~CObjectCloneTool();

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CEditTool
	bool MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );
	
	// Delete itself.
	void Release() { delete this; };

	virtual void BeginEditParams( IEditor *ie,int flags );
	virtual void EndEditParams();

	virtual void Display( DisplayContext &dc );
	virtual bool OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags );
	virtual bool OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags ) { return false; };
	//////////////////////////////////////////////////////////////////////////

	void Accept();
	void Abort();

private:
	void CloneSelection();
	void SetConstrPlane( CViewport *view,CPoint point );

	CSelectionGroup *m_selection;
	Vec3 m_origin;
	bool m_bSetConstrPlane;
};


#endif // __ObjectCloneTool_h__
