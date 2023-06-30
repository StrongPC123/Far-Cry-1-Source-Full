////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   entityscriptdialog.h
//  Version:     v1.00
//  Created:     25/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __entityscriptdialog_h__
#define __entityscriptdialog_h__
#pragma once

#include "XTToolkit.h"
// CEntityScriptDialog dialog

class CEntityScript;

class CEntityScriptDialog : public CXTResizeDialog
{
	DECLARE_DYNAMIC(CEntityScriptDialog)
public:
	typedef Functor0 Callback;

	CEntityScriptDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEntityScriptDialog();

// Dialog Data
	enum { IDD = IDD_DB_ENTITY_METHODS };

	void SetScript( CEntityScript *script,IEntity *m_entity );

	void SetOnReloadScript( Callback cb ) { m_OnReloadScript = cb; };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	virtual void OnOK() {};
	virtual void OnCancel() {};

	void	ReloadMethods();
	void	ReloadEvents();

	void GotoMethod( const CString &method );

	afx_msg void OnEditScript();
	afx_msg void OnReloadScript();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDblclkMethods();
	afx_msg void OnGotoMethod();
	afx_msg void OnAddMethod();
	afx_msg void OnRunMethod();

	DECLARE_MESSAGE_MAP()

	CCustomButton m_addMethodBtn;
	CCustomButton	m_runButton;
	CCustomButton	m_gotoMethodBtn;
	CCustomButton	m_editScriptBtn;
	CCustomButton	m_reloadScriptBtn;
	CCustomButton	m_removeButton;
	CStatic	m_scriptName;
	CListBox	m_methods;
	CString	m_selectedMethod;

	TSmartPtr<CEntityScript> m_script;
	IEntity *m_entity;
	CBrush m_grayBrush;

	Callback m_OnReloadScript;
};

#endif // __entityscriptdialog_h__