////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek GmbH, 2002.
// -------------------------------------------------------------------------
//  File name:   LinkTool.h
//  Version:     v1.00
//  Created:     5/7/2002 by Lennert.
//  Compilers:   Visual C++ 6.0
//  Description: Definition of CUnlinkTool, tool used to unlink objects.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __UnlinkTool_h__
#define __UnlinkTool_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "EditTool.h"
#include "Objects\ObjectManager.h"

//////////////////////////////////////////////////////////////////////////
class CUnlinkTool : public CEditTool,public IObjectSelectCallback
{
public:
	DECLARE_DYNAMIC(CUnlinkTool)

	CUnlinkTool(); // IPickObjectCallback *callback,CRuntimeClass *targetClass=NULL );
	~CUnlinkTool();

	// Ovverides from CEditTool
	bool MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags ) {return false;}
	// Delete itself.
	void Release() { delete this; };

	virtual void BeginEditParams( IEditor *ie,int flags ) {};
	virtual void EndEditParams() {};

	virtual void Display( DisplayContext &dc ) {};
	virtual bool OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags ) {return false;}
	virtual bool OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags ) { return false; };

	virtual bool OnSelectObject( CBaseObject *obj ) {return false;}

private:
	bool IsRelevant( CBaseObject *obj ) {return true;}
};


#endif // __UnlinkTool_h__
