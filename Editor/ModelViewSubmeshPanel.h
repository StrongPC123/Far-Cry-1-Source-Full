#if !defined(AFX_MODELVIEWSUBMESHPANEL_H__3C441EB6_B02A_4D2G_A5E9_587E334E9F48__INCLUDED_)
#define AFX_MODELVIEWSUBMESHPANEL_H__3C441EB6_B02A_4D2G_A5E9_587E334E9F48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModelViewSubmeshPanel.h : header file
//

#include "afxwin.h"
#include "XTToolkit.h"
/////////////////////////////////////////////////////////////////////////////
// ModelViewSubmeshPanel dialog

class ModelViewSubmeshPanel : public CXTResizeDialog
{
// Construction
public:
	ModelViewSubmeshPanel( class CModelViewport *vp,CWnd* pParent = NULL);   // standard constructor

	// Adds an animation with the given name and duration (in seconds)
	// to the animation list box/view.
	void ReinitSubmeshes();
	CString GetCurrAnimName();
	bool GetAnimLightState();

	void ClearBones();
	void AddBone( const CString &bone );
	void SelectBone( const CString &bone );

	bool GetLooped();
	bool GetSynchronizePhase();

	int GetLayer() const;
	float GetBlendInTime();
	float GetBlendOutTime();

	bool IsBlendTimeLock();

	void SetFileName( const CString &filename );

// Dialog Data
	//{{AFX_DATA(ModelViewPanel)
	enum { IDD = IDD_MODELVIEW_MULTIPART };
	CXTCheckListBox	m_submeshes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ModelViewPanel)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	int GetCurSubmesh();
	bool IsChecked(int nSubmesh);
	int GetItemBySubmesh(int nSubmesh);
	// Generated message map functions
	//{{AFX_MSG(ModelViewPanel)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeSubmeshes();
	afx_msg void OnBnClickedAddSubmesh();
	afx_msg void OnBnClickedReloadSubmesh();
	afx_msg void OnBnClickedChangeSubmesh();
	afx_msg void OnBnClickedRemoveSubmesh();
	afx_msg void OnBnClickedSubmeshVisible();
	afx_msg void OnCmdCheckChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CModelViewport *m_modelView;
	CCustomButton m_addSubmeshBtn;
	CCustomButton m_removeSubmeshBtn;
	CCustomButton m_reloadSubmeshBtn;
	//CButton	m_submeshVisible;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODELVIEWPANEL_H__3C441EB6_B02A_4D2F_A5E9_587E334E9F48__INCLUDED_)
