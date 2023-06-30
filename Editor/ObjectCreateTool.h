////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ObjectCreateTool.h
//  Version:     v1.00
//  Created:     18/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Definition of ObjectCreateTool, edit tool for creating objects..
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ObjectCreateTool_h__
#define __ObjectCreateTool_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "EditTool.h"
#include "Objects\ObjectManager.h"

class CBaseObject;

//////////////////////////////////////////////////////////////////////////
class CObjectCreateTool : public CEditTool
{
public:
	DECLARE_DYNCREATE(CObjectCreateTool)

	// This callback called when creation tool created new object.
	typedef Functor2<CObjectCreateTool*,CBaseObject*> CreateCallback;

	CObjectCreateTool( CreateCallback createCallback=0 );
	~CObjectCreateTool();

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CEditTool
	bool MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );
	
	// Delete itself.
	void Release() { delete this; };

	virtual void BeginEditParams( IEditor *ie,int flags );
	virtual void EndEditParams();

	virtual void Display( DisplayContext &dc ) {};
	virtual bool OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags );
	virtual bool OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags ) { return false; };
	virtual bool OnSetCursor( CViewport *vp );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// CObjectCreateTool methods.
	void SelectCategory( const CString &category );
	//! Start creation of object with givven type.
	void StartCreation( const CString &type,const CString &param="" );
	//! Cancel creation of object.
	void CancelCreation();
	//! Cancel creation of object.
	void AcceptCreation();

protected:
	//! Callback called when file selected in file browser.
	void OnSelectFile( CString file );

	void CloseFileBrowser();

private:
	int m_objectBrowserPanelId;
	int m_fileBrowserPanelId;

	HCURSOR m_hCreateCursor;
	CreateCallback m_createCallback;

	//! Created object type.
	CString m_objectType;
	//! Created object.
	CBaseObjectPtr m_object;
};


#endif // __ObjectCreateTool_h__
