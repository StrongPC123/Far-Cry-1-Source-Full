#if !defined(AFX_PANELDISPLATHIDE_H__5CFD89B5_08F6_42D0_B8DA_AF3949C77A60__INCLUDED_)
#define AFX_PANELDISPLATHIDE_H__5CFD89B5_08F6_42D0_B8DA_AF3949C77A60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PanelDisplatHide.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPanelDisplayHide dialog

class CPanelDisplayHide : public CDialog
{
// Construction
public:
	CPanelDisplayHide(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPanelDisplayHide)
	enum { IDD = IDD_PANEL_DISPLAY_HIDE };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPanelDisplayHide)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};
	
	void	SetCheckButtons();
	uint m_mask;

	void SetMask();

	// Generated message map functions
	//{{AFX_MSG(CPanelDisplayHide)
	virtual BOOL OnInitDialog();
	afx_msg void OnHideAll();
	afx_msg void OnHideNone();
	afx_msg void OnHideInvert();
	afx_msg void OnChangeHideMask();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PANELDISPLATHIDE_H__5CFD89B5_08F6_42D0_B8DA_AF3949C77A60__INCLUDED_)
