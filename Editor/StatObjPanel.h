#if !defined(AFX_STATOBJPANEL_H__56D339C8_0619_4253_9224_7B4617D90E12__INCLUDED_)
#define AFX_STATOBJPANEL_H__56D339C8_0619_4253_9224_7B4617D90E12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StatObjPanel.h : header file
//
#include "ObjectPanel.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// CStatObjPanel dialog

class CStatObjPanel : public CDialog
{
// Construction
public:
	CStatObjPanel(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStatObjPanel)
	enum { IDD = IDD_PANEL_STATICOBJ };
	//}}AFX_DATA

	void SetObject( class CStaticObject *obj );
	CStaticObject* GetObject() const { return m_object; }
	void UpdateObject();
	void SetMultiSelect( bool bEnable );
	bool IsMultiSelect() const { return m_multiSelect; };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStatObjPanel)
	protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	// Generated message map functions
	//{{AFX_MSG(CStatObjPanel)
	afx_msg void OnStatobjReload();
	afx_msg void OnReload();
	afx_msg void OnMassChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CStaticObject* m_object;

	bool m_multiSelect;
	CCustomButton	m_loadBtn;
	CCustomButton	m_reloadBtn;
	CEdit	m_objectName;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATOBJPANEL_H__56D339C8_0619_4253_9224_7B4617D90E12__INCLUDED_)
