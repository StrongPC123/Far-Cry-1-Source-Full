#if !defined(AFX_BUILDINGPANEL_H__96E64797_3913_440B_93E3_E4ACB5934454__INCLUDED_)
#define AFX_BUILDINGPANEL_H__96E64797_3913_440B_93E3_E4ACB5934454__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BuildingPanel.h : header file
//

#include "ObjectPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CBuildingPanel dialog

class CBuildingPanel : public CDialog, public IPickObjectCallback
{
// Construction
public:
	CBuildingPanel(CWnd* pParent = NULL);   // standard constructor
	~CBuildingPanel();

// Dialog Data
	//{{AFX_DATA(CBuildingPanel)
	enum { IDD = IDD_PANEL_BUILDING };
	CCustomButton	m_unbindBuilding;
	CColorCheckBox	m_bindButton;
	CButton	m_wireframe;
	CButton	m_portals;
	CCustomButton	m_hideNone;
	CCustomButton	m_hideInvert;
	CCustomButton	m_hideAll;
	CColoredListBox	m_hiddenSectors;
	CListCtrl	m_helpers;
	CCustomButton	m_browseButton;
	CCustomButton	m_spawnButton;
	//}}AFX_DATA

	void	SetBuilding( class CBuilding *obj );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBuildingPanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	virtual void OnUpdate() {};

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	// Ovverriden from IPickObjectCallback
	virtual void OnPick( CBaseObject *picked );
	virtual void OnCancelPick();

	void OnCreateCallback( class CObjectCreateTool *tool,class CBaseObject *object );
	void RefreshList();

	// Generated message map functions
	//{{AFX_MSG(CBuildingPanel)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnChange();
	afx_msg void OnSpawn();
	afx_msg void OnSelchangeHiddenSectors();
	afx_msg void OnHideAll();
	afx_msg void OnHideNone();
	afx_msg void OnHideInvert();
	afx_msg void OnBind();
	afx_msg void OnUnbind();
	afx_msg void OnWireframe();
	afx_msg void OnBnClickedPortals();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBuilding *m_building;
	CObjectCreateTool *m_createTool;
	int m_currHelper;
	bool m_picking;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUILDINGPANEL_H__96E64797_3913_440B_93E3_E4ACB5934454__INCLUDED_)
