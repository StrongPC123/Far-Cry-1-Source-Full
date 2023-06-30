////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   errorreportdialog.h
//  Version:     v1.00
//  Created:     30/5/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __errorreportdialog_h__
#define __errorreportdialog_h__
#pragma once

// CErrorReportDialog dialog
#include "XTToolkit.h"
#include "ErrorReport.h"

class CErrorReportDialog : public CXTResizeDialog
{
	DECLARE_DYNAMIC(CErrorReportDialog)

public:
	CErrorReportDialog( CErrorReport *report,CWnd* pParent = NULL);   // standard constructor
	virtual ~CErrorReportDialog();

	static void Open( CErrorReport *pReport );
	static void Close();

// Dialog Data
	enum { IDD = IDD_ERROR_REPORT };

protected:
	virtual void OnOK();
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	afx_msg void OnNMDblclkErrors(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSelectObjects();
	afx_msg void OnCopyToClipboard();

	void ReloadErrors();

	DECLARE_MESSAGE_MAP()
	
	CListCtrl m_errors;

	CErrorReport *m_pErrorReport;
	CImageList m_imageList;

	static CErrorReportDialog* m_instance;

	std::vector<CErrorRecord> m_errorRecords;
};

#endif // __errorreportdialog_h__