////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   inplaceedit.h
//  Version:     v1.00
//  Created:     5/6/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __inplaceedit_h__
#define __inplaceedit_h__

#if _MSC_VER > 1000
#pragma once
#endif

class CInPlaceEdit : public CEdit
{
public:
	typedef Functor0 OnChange;

	CInPlaceEdit( const CString& srtInitText,OnChange onchange );
	virtual ~CInPlaceEdit();

	// Attributes
	void SetText(const CString& strText);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInPlaceEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CInPlaceEdit)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	// Data
protected:
	CString m_strInitText;
	OnChange m_onChange;
};

#endif // __inplaceedit_h__