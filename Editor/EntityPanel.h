#if !defined(AFX_ENTITYPANEL_H__E88E89D7_62BA_4E1F_82D8_17B58F96CA46__INCLUDED_)
#define AFX_ENTITYPANEL_H__E88E89D7_62BA_4E1F_82D8_17B58F96CA46__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EntityPanel.h : header file
//

#include "PickObjectTool.h"
#include "Controls\PickObjectButton.h"
#include "XTToolkit.h"

/////////////////////////////////////////////////////////////////////////////
// CEntityPanel dialog

class CEntityPanel : public CXTResizeDialog, public IPickObjectCallback
{
// Construction
public:
	CEntityPanel(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEntityPanel)
	enum { IDD = IDD_PANEL_ENTITY };
	CCustomButton	m_sendEvent;
	CCustomButton	m_runButton;
	CCustomButton	m_gotoMethodBtn;
	CCustomButton	m_editScriptBtn;
	CCustomButton	m_addMethodBtn;
	CCustomButton	m_addMissionBtn;
	CCustomButton	m_reloadScriptBtn;
	CCustomButton	m_removeButton;
	CCustomButton	m_prototypeButton;
	CTreeCtrl	m_eventTree;
	CPickObjectButton m_pickButton;
	CListBox	m_methods;
	CString	m_selectedMethod;
	//}}AFX_DATA

	void SetEntity( class CEntity *entity );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEntityPanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	void	ReloadMethods();
	void	ReloadEvents();

	// Ovverriden from IPickObjectCallback
	virtual void OnPick( CBaseObject *picked );
	virtual void OnCancelPick();

	void GotoMethod( const CString &method );

	// Generated message map functions
	//{{AFX_MSG(CEntityPanel)
	afx_msg void OnEditScript();
	afx_msg void OnReloadScript();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDblclkMethods();
	afx_msg void OnGotoMethod();
	afx_msg void OnAddMethod();
	virtual BOOL OnInitDialog();
	afx_msg void OnEventAdd();
	afx_msg void OnDestroy();
	afx_msg void OnEventRemove();
	afx_msg void OnSelChangedEventTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickEventTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClickEventTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRunMethod();
	afx_msg void OnEventSend();
	afx_msg void OnBnAddMission();
	afx_msg void OnPrototype();
	afx_msg void OnBnClickedGetphysics();
	afx_msg void OnBnClickedResetphysics();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CImageList m_treeImageList;
	
	StdMap<HTREEITEM,CString> m_sourceEventsMap;
	StdMap<HTREEITEM,int> m_targetEventsMap;

	CEntity* m_entity;
	CBrush m_grayBrush;
	class CPickObjectTool *m_pickTool;

	CString m_currentSourceEvent;
	int m_currentTrgEventId;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENTITYPANEL_H__E88E89D7_62BA_4E1F_82D8_17B58F96CA46__INCLUDED_)
