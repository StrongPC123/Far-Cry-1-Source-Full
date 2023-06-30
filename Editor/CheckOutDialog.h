
////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   checkoutdialog.h
//  Version:     v1.00
//  Created:     22/5/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __checkoutdialog_h__
#define __checkoutdialog_h__
#pragma once

//////////////////////////////////////////////////////////////////////////
// CCheckOutDialog dialog

class CCheckOutDialog : public CDialog
{
	DECLARE_DYNAMIC(CCheckOutDialog)

	// Checkout dialog result.
	enum EResult
	{
		CHECKOUT,
		OVERWRITE,
		CANCEL,
	};

public:
	CCheckOutDialog( const CString &file,CWnd* pParent = NULL);   // standard constructor
	virtual ~CCheckOutDialog();

// Dialog Data
	enum { IDD = IDD_CHECKOUT };

	EResult GetResult() const { return m_result; };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedCheckout();
	afx_msg void OnBnClickedOk();

	CString m_file;
	CString m_text;
	EResult m_result;
public:
	virtual BOOL OnInitDialog();
};

#endif // __checkoutdialog_h__
