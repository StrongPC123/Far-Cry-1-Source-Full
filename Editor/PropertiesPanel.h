#if !defined(AFX_PROPERTIESPANEL_H__AD3E2ECE_EFEB_4A4C_81A7_216B2BC11BC5__INCLUDED_)
#define AFX_PROPERTIESPANEL_H__AD3E2ECE_EFEB_4A4C_81A7_216B2BC11BC5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertiesPanel.h : header file
//

#include "Controls\PropertyCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CPropertiesPanel dialog

class CPropertiesPanel : public CDialog
{
// Construction
public:
	CPropertiesPanel( CWnd* pParent = NULL );   // standard constructor

	typedef Functor0 UpdateCallback;

// Dialog Data
	//{{AFX_DATA(CPropertiesPanel)
	enum { IDD = IDD_PANEL_PROPERTIES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	void ReloadValues();

	void DeleteVars();
	void AddVars( class CVarBlock *vb,const UpdateCallback &func=NULL );

	void SetMultiSelect( bool bEnable );
	bool IsMultiSelect() const { return m_multiSelect; };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertiesPanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	void OnPropertyChanged( XmlNodeRef node );

	// Generated message map functions
	//{{AFX_MSG(CPropertiesPanel)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CPropertyCtrl m_wndProps;
	XmlNodeRef m_template;
	bool m_multiSelect;
	TSmartPtr<CVarBlock> m_varBlock;

	std::list<UpdateCallback> m_updateCallbacks;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTIESPANEL_H__AD3E2ECE_EFEB_4A4C_81A7_216B2BC11BC5__INCLUDED_)
