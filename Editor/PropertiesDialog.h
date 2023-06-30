#if !defined(AFX_PROPERTIESDIALOG_H__CBBFC362_FD42_4574_9E6A_AA5F3F171128__INCLUDED_)
#define AFX_PROPERTIESDIALOG_H__CBBFC362_FD42_4574_9E6A_AA5F3F171128__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertiesDialog.h : header file
//

#include "XTToolkit.h"
#include "Controls\PropertyCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CPropertiesDialog dialog

class CPropertiesDialog : public CXTResizeDialog
{
// Construction
public:
	CPropertiesDialog( const CString &title,XmlNodeRef &node,CWnd* pParent = NULL);   // standard constructor
	typedef Functor1<IVariable*> UpdateVarCallback;

// Dialog Data
	//{{AFX_DATA(CPropertiesDialog)
	enum { IDD = IDD_PROPERTIES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	void SetUpdateCallback( UpdateVarCallback cb ) { m_varCallback = cb; };
	CPropertyCtrl* GetPropertyCtrl() { return &m_wndProps; };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertiesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel();

	void OnPropertyChange( IVariable *pVar );

	// Generated message map functions
	//{{AFX_MSG(CPropertiesDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CPropertyCtrl m_wndProps;
	XmlNodeRef m_node;
	CString m_title;
	UpdateVarCallback m_varCallback;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTIESDIALOG_H__CBBFC362_FD42_4574_9E6A_AA5F3F171128__INCLUDED_)
