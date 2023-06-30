////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   PickObjectTool.h
//  Version:     v1.00
//  Created:     18/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Definition of PickObjectTool, tool used to pick objects.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PickObjectTool_h__
#define __PickObjectTool_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "EditTool.h"
#include "Objects\ObjectManager.h"

//////////////////////////////////////////////////////////////////////////
class CPickObjectTool : public CEditTool,public IObjectSelectCallback
{
public:
	DECLARE_DYNAMIC(CPickObjectTool)

	CPickObjectTool( IPickObjectCallback *callback,CRuntimeClass *targetClass=NULL );
	~CPickObjectTool();

	//! If set to true, pick tool will not stop picking after first pick.
	void SetMultiplePicks( bool bEnable ) { m_bMultiPick = bEnable; };

	// Ovverides from CEditTool
	bool MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );
	
	// Delete itself.
	void Release() { delete this; };

	virtual void BeginEditParams( IEditor *ie,int flags );
	virtual void EndEditParams() {};

	virtual void Display( DisplayContext &dc ) {};
	virtual bool OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags );
	virtual bool OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags ) { return false; };

	virtual bool OnSelectObject( CBaseObject *obj );

private:
	bool IsRelevant( CBaseObject *obj );

	//! Object that requested pick.
	IPickObjectCallback* m_callback;

	//! If target class specified, will pick only objects that belongs to that runtime class.
	CRuntimeClass* m_targetClass;

	bool m_bMultiPick;
};


#endif // __PickObjectTool_h__
