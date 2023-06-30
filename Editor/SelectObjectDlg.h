#if !defined(AFX_SELECTOBJECTDLG_H__E32A4E43_163A_4B9D_B0F2_2CFDE13FC989__INCLUDED_)
#define AFX_SELECTOBJECTDLG_H__E32A4E43_163A_4B9D_B0F2_2CFDE13FC989__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectObjectDlg.h : header file
//
#include "XTToolkit.h"

class CBaseObject;

/////////////////////////////////////////////////////////////////////////////
// CSelectObjectDlg dialog

class CSelectObjectDlg : public CXTResizeDialog
{
// Construction
public:
	static CSelectObjectDlg* GetInstance();

// Dialog Data
	//{{AFX_DATA(CSelectObjectDlg)
	enum { IDD = IDD_SELECT_OBJECT };
	CListCtrl	m_list;
	static BOOL	m_bEntity;
	static BOOL	m_bPrefabs;
	static BOOL	m_bOther;
	static BOOL	m_bTagPoint;
	static BOOL	m_bAIPoint;
	static BOOL	m_bGroups;
	static BOOL	m_bVolumes;
	static BOOL	m_bShapes;
	static BOOL  m_bBrushes;
	static BOOL  m_bAutoselect;
	static BOOL  m_bTree;
	CCustomButton m_hideBtn;
	CCustomButton m_freezeBtn;
	CCustomButton m_selAllBtn;
	CCustomButton m_selNoneBtn;
	CCustomButton m_selInvBtn;
	CCustomButton m_selByPropertyName;
	CCustomButton m_selByPropertyValue;
	CEdit m_propertyFilterCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectObjectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CSelectObjectDlg(CWnd* pParent = NULL);   // standard constructor
	void OnObjectEvent( CBaseObject *pObject,int event );

	// Generated message map functions
	//{{AFX_MSG(CSelectObjectDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual void PostNcDestroy();

	afx_msg void OnSelAll();
	afx_msg void OnSelNone();
	afx_msg void OnSelInv();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnListType();
	afx_msg void OnColumnclickObjects(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateFastfind();
	afx_msg void OnClickObjects(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVisible();
	afx_msg void OnHidden();
	afx_msg void OnFrozen();
	afx_msg void OnHide();
	afx_msg void OnFreeze();
	afx_msg void OnMatchPropertyName();
	afx_msg void OnMatchPropertyValue();
	afx_msg void OnItemchangedObjects(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkObjects(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void AddObject( CBaseObject *obj,int level );
	void AddObjectRecursively( CBaseObject *obj,int level );
	void FillList();
	void EnableControls();
	void SelectItemObject( int iItem );
	bool IsPropertyMatch( CBaseObject *pObject );
	bool IsPropertyMatchVariable( IVariable *pVar );

	static int m_sortFlags;
	static int m_displayMode;
	bool m_picking;
	bool m_bIgnoreObjectCallback;

	CString m_nameFilter;
	CString m_propertyFilter;
	bool m_bMatchPropertyName;

	int m_listMask;

	CImageList m_imageList;

	struct IObjectManager* m_pObjMan;

	//! Single instance of this dialog.
	static CSelectObjectDlg* m_instance;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTOBJECTDLG_H__E32A4E43_163A_4B9D_B0F2_2CFDE13FC989__INCLUDED_)
