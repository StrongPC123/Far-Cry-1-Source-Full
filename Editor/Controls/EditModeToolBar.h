#if !defined(AFX_EDITMODETOOLBAR_H__1A7415BB_DA82_4A27_BA14_F06B74A05080__INCLUDED_)
#define AFX_EDITMODETOOLBAR_H__1A7415BB_DA82_4A27_BA14_F06B74A05080__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditModeToolBar.h : header file
//
#include "SelectionCombo.h"

/////////////////////////////////////////////////////////////////////////////
// CEditModeToolBar window

class CEditModeToolBar : public CXTToolBar
{
// Construction
public:
	CEditModeToolBar();

	BOOL Create( CWnd *pParentWnd,DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,UINT nID = AFX_IDW_TOOLBAR );

// Attributes
public:

// Operations
public:
	CString GetSelection();
	void SetSelection( const CString &name );
	void AddSelection( const CString &name );
	void RemoveSelection( const CString &name );

	void SetCurrentLayer( const CString &layerName );
	void SetGridSize( float size );
	void NextSelectMask();

	// Data
	//{{AFX_DATA(CEditModeToolBar)
	CString	m_szSelection;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditModeToolBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditModeToolBar();

	void OnSnapMenu( CPoint pos );
	void OnAxisTerrainMenu( CPoint pos );

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditModeToolBar)
	afx_msg void OnSelectionChanged();
	afx_msg void OnNotifyOnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnToolbarDropDown(NMHDR* pnhdr, LRESULT *plr);
	afx_msg void OnUpdateCoordsRefSys(CCmdUI *pCmdUI);
	afx_msg void OnCoordsRefSys();
	afx_msg void OnUpdateSelectionMask(CCmdUI *pCmdUI);
	afx_msg void OnSelectionMask();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()

	CSelectionCombo m_selections;
	CXTFlatComboBox m_refCoords;
	CXTFlatComboBox m_selectionMask;
	//CExtComboBox m_selections;

	CMenu m_gridMenu;
	RefCoordSys m_coordSys;

	int m_objectSelectionMask;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITMODETOOLBAR_H__1A7415BB_DA82_4A27_BA14_F06B74A05080__INCLUDED_)
