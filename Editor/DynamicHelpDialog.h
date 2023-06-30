////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   dynamichelpdialog.h
//  Version:     v1.00
//  Created:     12/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __dynamichelpdialog_h__
#define __dynamichelpdialog_h__
#pragma once

#include "XTToolkit.h"

// CDynamicHelpDialog dialog

class CDynamicHelpDialog : public CXTCBarDialog
{
	DECLARE_DYNAMIC(CDynamicHelpDialog)

	static void Open();
	static void OnIdle();

// Dialog Data
	enum { IDD = IDD_DYNAMIC_HELP };

	void OpenHelpFile( const CString &file,const CString &tooltipText );
	void DisplayDynamicHelp();
	void EditItem();

protected:
	class CHelpEdit : public CEdit {
		afx_msg UINT OnGetDlgCode()
		{
			return CEdit::OnGetDlgCode() | DLGC_WANTALLKEYS;
		}
	};

	CDynamicHelpDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDynamicHelpDialog();

	void SetEditMode( bool bEditMode );

	virtual void OnOK() {};
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void PostNcDestroy();

	afx_msg void OnBnClickedSave();
	afx_msg void OnUpdateSave( CCmdUI* pCmdUI );
	afx_msg void OnUpdateHelp( CCmdUI* pCmdUI );
	afx_msg void OnBnClickedHelp();
	afx_msg void OnTextChange();
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()

	CString GetParentText( CWnd *pWnd,const CString &text );

	//////////////////////////////////////////////////////////////////////////
	CString m_currentUrl;
	CString m_currentFile;
	bool m_bDynamicHelp;
	bool m_bEditMode;
	bool m_bHelpNotFound;
	bool m_bSkipSameWindow;
	
	CColorCtrl<CHelpEdit> m_text;
	CXTToolBar m_toolbar;
	CXTStatusBar m_status;

	CWnd *m_lastWindow;

	static CDynamicHelpDialog* m_instance;
};

#endif // __dynamichelpdialog_h__
