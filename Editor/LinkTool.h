////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek GmbH, 2002.
// -------------------------------------------------------------------------
//  File name:   LinkTool.h
//  Version:     v1.00
//  Created:     5/7/2002 by Lennert.
//  Compilers:   Visual C++ 6.0
//  Description: Definition of CLinkTool, tool used to link objects.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __LinkTool_h__
#define __LinkTool_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "EditTool.h"
#include "Objects\ObjectManager.h"

class CBaseObject;
class CEntity;

//////////////////////////////////////////////////////////////////////////
class CLinkTool : public CEditTool,public IObjectSelectCallback
{
private:
	CBaseObject *m_pChild;
	Vec3d m_StartDrag;
	Vec3d m_EndDrag;
public:
	DECLARE_DYNAMIC(CLinkTool)

	CLinkTool(); // IPickObjectCallback *callback,CRuntimeClass *targetClass=NULL );
	~CLinkTool();

	// Ovverides from CEditTool
	bool MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );
	// Delete itself.
	void Release() { delete this; };

	virtual void BeginEditParams( IEditor *ie,int flags ) {};
	virtual void EndEditParams() {};

	virtual void Display( DisplayContext &dc );
	virtual bool OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags );
	virtual bool OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags ) { return false; };

	virtual bool OnSelectObject( CBaseObject *obj ) {return false;}
	virtual bool OnSetCursor( CViewport *vp );

private:
	bool IsRelevant( CBaseObject *obj ) { return true; }
	bool ChildIsValid(CBaseObject *pParent, CBaseObject *pChild, int nDir=3);

	HCURSOR m_hLinkCursor;
	HCURSOR m_hLinkNowCursor;
	HCURSOR m_hCurrCursor;
};


#endif // __LinkTool_h__
