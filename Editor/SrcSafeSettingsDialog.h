////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   srcsafesettingsdialog.h
//  Version:     v1.00
//  Created:     22/5/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __srcsafesettingsdialog_h__
#define __srcsafesettingsdialog_h__
#pragma once

// CSrcSafeSettingsDialog dialog

class CSrcSafeSettingsDialog : public CDialog
{
	DECLARE_DYNAMIC(CSrcSafeSettingsDialog)

public:
	CSrcSafeSettingsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSrcSafeSettingsDialog();

// Dialog Data
	enum { IDD = IDD_SOURCESAFESETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedBrowseExe();
	afx_msg void OnBnClickedBrowseDatabase();

private:
	CString m_username;
	CString m_exeFile;
	CString m_database;
	CString m_project;
public:
	virtual BOOL OnInitDialog();
};

#endif // __srcsafesettingsdialog_h__