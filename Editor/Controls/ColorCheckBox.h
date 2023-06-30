#if !defined(AFX_COLORCHECKBOX_H__8D5C1FE0_9A53_4247_8CF6_C23C0C829291__INCLUDED_)
#define AFX_COLORCHECKBOX_H__8D5C1FE0_9A53_4247_8CF6_C23C0C829291__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorCheckBox.h : header file
//

#define STD_PUSHED_COLOR	(RGB(255,255,0))

typedef CColorCtrl<CColorPushButton<CButton> > CColoredPushButton;
typedef CColorCtrl<CColorPushButton<CButton> > CCustomButton;

/////////////////////////////////////////////////////////////////////////////
// CColorCheckBox window
class CColorCheckBox : public CColoredPushButton
{
	DECLARE_DYNCREATE( CColorCheckBox )
// Construction
public:
	CColorCheckBox();

	int	GetCheck() const { return m_nChecked; };
	void SetCheck(int nCheck);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorCheckBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorCheckBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorCheckBox)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

//	DECLARE_MESSAGE_MAP()

	int m_nChecked;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORCHECKBOX_H__8D5C1FE0_9A53_4247_8CF6_C23C0C829291__INCLUDED_)
