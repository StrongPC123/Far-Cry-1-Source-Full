////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   EnvironmentTool.h
//  Version:     v1.00
//  Created:     11/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Places Environment on terrain.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __EnvironmentTool_h__
#define __EnvironmentTool_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "EditTool.h"

//////////////////////////////////////////////////////////////////////////
class CEnvironmentTool : public CEditTool
{
	DECLARE_DYNCREATE(CEnvironmentTool)
public:
	CEnvironmentTool();
	virtual ~CEnvironmentTool();

	virtual void BeginEditParams( IEditor *ie,int flags );
	virtual void EndEditParams();

	virtual void Display( DisplayContext &dc ) {};

	// Ovverides from CEditTool
	bool MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags ) { return true; };

	// Key down.
	bool OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags ) { return true;};
	bool OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags ) { return true;};
	
	// Delete itself.
	void Release() { delete this; };

private:
	IEditor *m_ie;

	int m_panelId;
	class CEnvironmentPanel *m_panel;
};


#endif // __EnvironmentTool_h__
