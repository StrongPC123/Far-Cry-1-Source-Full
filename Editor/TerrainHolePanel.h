#if !defined(AFX_TERRAINHOLEPANEL_H__766784E4_9912_4B6B_B89E_1F96F295AA29__INCLUDED_)
#define AFX_TERRAINHOLEPANEL_H__766784E4_9912_4B6B_B89E_1F96F295AA29__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TerrainHolePanel.h : header file
//

#include "Controls\ColorCheckBox.h"

/////////////////////////////////////////////////////////////////////////////
// CTerrainHolePanel dialog

class CTerrainHolePanel : public CDialog
{
// Construction
public:
	CTerrainHolePanel(class CTerrainHoleTool *tool,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTerrainHolePanel)
	enum { IDD = IDD_PANEL_TERRAIN_HOLE };
	CSliderCtrl	m_radius;
	//CColorCheckBox	m_removeHole;
	//CColorCheckBox	m_makeHole;
	CColorCheckBox	m_removeHole;
	CColorCheckBox	m_makeHole;
	//}}AFX_DATA

	void SetMakeHole( bool bEnable );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTerrainHolePanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	// Generated message map functions
	//{{AFX_MSG(CTerrainHolePanel)
	afx_msg void OnHoleMake();
	afx_msg void OnHoleRemove();
	virtual BOOL OnInitDialog();
	afx_msg void OnReleasedcaptureRadius(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CTerrainHoleTool *m_tool;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERRAINHOLEPANEL_H__766784E4_9912_4B6B_B89E_1F96F295AA29__INCLUDED_)
