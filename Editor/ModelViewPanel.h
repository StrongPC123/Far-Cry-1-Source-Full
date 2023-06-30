#if !defined(AFX_MODELVIEWPANEL_H__3C441EB6_B02A_4D2F_A5E9_587E334E9F48__INCLUDED_)
#define AFX_MODELVIEWPANEL_H__3C441EB6_B02A_4D2F_A5E9_587E334E9F48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModelViewPanel.h : header file
//

#include "Controls\AnimSequences.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// ModelViewPanel dialog

class ModelViewPanel : public CDialog
{
// Construction
public:
	ModelViewPanel( class CModelViewport *vp,CWnd* pParent = NULL);   // standard constructor

	// Adds an animation with the given name and duration (in seconds)
	// to the animation list box/view.
	void AddAnimName( const CString &name, float fDurationSeconds);
	void ClearAnims();
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
	enum { IDD = IDD_MODELVIEW_PANEL };
	CComboBox	m_layer;
	CButton	m_loop;
	CButton m_lockBlendTimes;
	CColoredListBox	m_animations;
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

	int GetCurSel();
	// Generated message map functions
	//{{AFX_MSG(ModelViewPanel)
	virtual BOOL OnInitDialog();
	afx_msg void OnAnimateLights();
	afx_msg void OnSelchangeAnimations();
	afx_msg void OnLoop();
	afx_msg void OnLockBlendTimes();
	afx_msg void OnBnClickedAttachObject();
	afx_msg void OnBnClickedDetachObject();
	afx_msg void OnBnClickedDetachAll();
	afx_msg void OnBnClickedStopAnimation();
	afx_msg void OnBnClickedResetAnimations();
	afx_msg void OnBnClickedBrowseObject();
	afx_msg void OnEnChangeBone();
	afx_msg void OnBnClickedBrowseFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CModelViewport *m_modelView;
	CNumberCtrl m_blendInTime;
	CNumberCtrl m_blendOutTime;
	CCustomButton m_browseObjectBtn;
	CCustomButton m_attachBtn;
	CCustomButton m_detachBtn;
	CCustomButton m_detachAllBtn;
	CCustomButton m_stopAnimationBtn;
	CComboBox m_boneName;
	CEdit m_objectName;
	// If checked, the animation is started with phase synchronized with the layer0
	CButton m_syncPhase;

	CEdit m_fileEdit;
public:
	afx_msg void OnBnClickedReloadFile();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODELVIEWPANEL_H__3C441EB6_B02A_4D2F_A5E9_587E334E9F48__INCLUDED_)
