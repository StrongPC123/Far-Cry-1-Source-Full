////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PreferencesPage.h
//  Version:     v1.00
//  Created:     28/10/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PreferencesPage_h__
#define __PreferencesPage_h__
#pragma once

#include "Include\IPreferencesPage.h"
#include "Controls\PropertyCtrl.h"

//////////////////////////////////////////////////////////////////////////
// Class description implementation.
//////////////////////////////////////////////////////////////////////////
class CPreferencesPropertyPageClassDesc : public IPreferencesPageClassDesc
{
public:
	IPreferencesPage* CreatePage( const CRect &rc,CWnd *pParentWnd );
};


//////////////////////////////////////////////////////////////////////////
// Implementation of PreferencePage with properties control.
//////////////////////////////////////////////////////////////////////////
class CPreferencesPropertyPage : public CWnd, public IPreferencesPage
{
	DECLARE_DYNAMIC(CPreferencesPropertyPage);
public:
	CPreferencesPropertyPage();
	
	BOOL Create( const CRect &rc,CWnd *pParentWnd );

	virtual const char* GetCategory() { return "aa"; }
	virtual const char* GetTitle()  { return "bb"; }
		
	//////////////////////////////////////////////////////////////////////////
	// Implements IPreferencesPage.
	//////////////////////////////////////////////////////////////////////////
	virtual CWnd* GetWindow() { return this; }
	virtual void Release() { delete this; }

	virtual void OnApply();
	virtual void OnCancel();
	virtual bool OnQueryCancel();
	virtual void OnSetActive( bool bActive );
	virtual void OnOK();

	virtual CVarBlock* GetVars() { return m_pVars; };

	//////////////////////////////////////////////////////////////////////////
	// CVariable helper functions.
	//////////////////////////////////////////////////////////////////////////
	void AddVariable( CVariableBase &var,const char *varName,unsigned char dataType=IVariable::DT_SIMPLE );
	void AddVariable( CVariableArray &table,CVariableBase &var,const char *varName,unsigned char dataType=IVariable::DT_SIMPLE );

	template <class T>
	void CopyVar( CVariable<T> &var,T &value,bool bSet )
	{
		if (bSet)
			var = value;
		else
			value = var;
	}

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnSize(UINT nType, int cx, int cy);

	//////////////////////////////////////////////////////////////////////////
protected:
	CPropertyCtrl m_wndProps;
	_smart_ptr<CVarBlock> m_pVars;
};

#endif // __PreferencesPage_h__

