#if !defined(AFX_TERRAINCURVE_H__F70A9C14_0450_42D4_AA50_04A57DF7E35E__INCLUDED_)
#define AFX_TERRAINCURVE_H__F70A9C14_0450_42D4_AA50_04A57DF7E35E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TerrainCurve.h : header file
//

#include "CurveWnd.h"

#define IDC_CURVE_WND 1111

/////////////////////////////////////////////////////////////////////////////
// CTerrainCurve dialog

class CTerrainCurve : public CDialog
{
// Construction
public:
	CTerrainCurve(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTerrainCurve)
	enum { IDD = IDD_TERRAIN_CURVE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTerrainCurve)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTerrainCurve)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CCurveWnd m_wndCurve;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERRAINCURVE_H__F70A9C14_0450_42D4_AA50_04A57DF7E35E__INCLUDED_)
