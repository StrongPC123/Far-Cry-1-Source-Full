////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   numberctrledit.h
//  Version:     v1.00
//  Created:     26/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __numberctrledit_h__
#define __numberctrledit_h__

#if _MSC_VER > 1000
#pragma once
#endif

class CNumberCtrlEdit : public CEdit
{
	CNumberCtrlEdit(const CNumberCtrlEdit& d);
	CNumberCtrlEdit& operator=(const CNumberCtrlEdit& d);

public:
	typedef Functor0 UpdateCallback;

	CNumberCtrlEdit() {};

	// Attributes
	void SetText(const CString& strText);

	//! Set callback function called when number edit box is really updated.
	void SetUpdateCallback( UpdateCallback func ) { m_onUpdate = func; }

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumberCtrlEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CNumberCtrlEdit)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnChar( UINT nChar,UINT nRepCnt,UINT nFlags );
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	// Data
protected:
	CString m_strInitText;
	UpdateCallback m_onUpdate;
};

#endif // __numberctrledit_h__