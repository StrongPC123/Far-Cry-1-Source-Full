////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PreferencesStdPages.h
//  Version:     v1.00
//  Created:     29/10/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PreferencesStdPages_h__
#define __PreferencesStdPages_h__
#pragma once

#include "Include\IPreferencesPage.h"

//////////////////////////////////////////////////////////////////////////
class CStdPreferencesClassDesc : public IPreferencesPageClassDesc, public IPreferencesPageCreator
{
	ULONG m_refCount;
public:
	CStdPreferencesClassDesc() : m_refCount(0) {};

	//////////////////////////////////////////////////////////////////////////
	// IUnkown implementation.
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( const IID &riid, void **ppvObj );
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();
	//////////////////////////////////////////////////////////////////////////

	virtual REFGUID ClassID();

	//////////////////////////////////////////////////////////////////////////
	virtual int GetPagesCount();
	virtual IPreferencesPage* CreatePage( int index,const CRect &rc,CWnd *pParentWnd );
};

#endif // __PreferencesStdPages_h__

