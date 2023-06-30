////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   PreferencesDialog.h
//  Version:     v1.00
//  Created:     28/10/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Editor Preferences Dialog.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __preferences_dialog_h__
#define __preferences_dialog_h__
#pragma once

// CPreferencesDialog dialog
#include "XTToolkit.h"
#include "Include\IPreferencesPage.h"

class CPreferencesDialog : public CXTResizeDialog
{
	DECLARE_DYNAMIC(CPreferencesDialog)

public:
	CPreferencesDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPreferencesDialog();

// Dialog Data
	enum { IDD = IDD_PREFERENCES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnGetdispinfoListOptions(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedListOptions(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize( UINT nType,int cx,int cy );
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

	//////////////////////////////////////////////////////////////////////////
	void CreatePages( const CRect &rc );
	void FillTree();

protected:
	CTreeCtrl m_wndTree;
	CImageList m_imgList;

	//////////////////////////////////////////////////////////////////////////
	struct PageInfo : public CRefCountBase
	{
		CWnd *pWnd;
		IPreferencesPage *pPage;
		CString title;
		CString category;

		PageInfo() : pPage(0),pWnd(0) {};
	};
	std::vector<_smart_ptr<PageInfo> > m_pagesInfo;
	PageInfo* m_pSelected;
};

#endif // __preferences_dialog_h__