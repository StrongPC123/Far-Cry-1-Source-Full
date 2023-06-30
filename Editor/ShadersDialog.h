#if !defined(AFX_SHADERSDIALOG_H__31583206_BDB8_48F9_8527_2E3B42CFFACC__INCLUDED_)
#define AFX_SHADERSDIALOG_H__31583206_BDB8_48F9_8527_2E3B42CFFACC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShadersDialog.h : header file
//

#include "XTToolkit.h"
#include "Controls\TextEditorCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CShadersDialog dialog

class CShadersDialog : public CXTResizeDialog
{
// Construction
public:
	CShadersDialog(const CString &selection,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CShadersDialog)
	enum { IDD = IDD_SHADERS };
	CListBox	m_shaders;
	CTextEditorCtrl	m_shaderText;
	CButton m_saveButton;
	CString	m_selection;
	//}}AFX_DATA

	CString GetSelection() { return m_selection; };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShadersDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CShadersDialog)
	afx_msg void OnSelchangeShaders();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkShaders();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CString m_sel;
	CBrush m_brush;
public:
	afx_msg void OnBnClickedEdit();
	afx_msg void OnBnClickedSave();
	afx_msg void OnEnChangeText();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHADERSDIALOG_H__31583206_BDB8_48F9_8527_2E3B42CFFACC__INCLUDED_)
