#include "afxwin.h"
#if !defined(AFX_INFOBAR_H__F7C1D490_054E_43F7_BBF5_C9D6180E1936__INCLUDED_)
#define AFX_INFOBAR_H__F7C1D490_054E_43F7_BBF5_C9D6180E1936__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InfoBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInfoBar dialog

class CInfoBar : public CDialog
{
// Construction
public:
	CInfoBar();   // standard constructor

// Dialog Data
	enum { IDD = IDD_INFO_BAR };
	CColorCheckBox m_vectorLock;
	CColorCheckBox m_lockSelection;
	CColorCheckBox m_terrainCollision;
	CColorCheckBox m_physicsBtn;
	CColorCheckBox m_syncPlayerBtn;

	void IdleUpdate();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInfoBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	void OnVectorUpdate( bool followTerrain );
	
	void OnVectorUpdateX();
	void OnVectorUpdateY();
	void OnVectorUpdateZ();

	void SetVector( const Vec3 &v );
	void SetVectorRange( float min,float max );
	Vec3 GetVector();
	void EnableVector( bool enable );

	// Callbacks from number control.
	void OnBeginVectorUpdate( CNumberCtrl *ctrl );
	void OnEndVectorUpdate( CNumberCtrl *ctrl );

	// Generated message map functions
	//{{AFX_MSG(CInfoBar)
	afx_msg void OnVectorLock();
	afx_msg void OnLockSelection();
	afx_msg void OnUpdateLockSelection(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMoveSpeed();
	afx_msg void OnBnClickedTerrainCollision();
	afx_msg void OnBnClickedPhysics();

	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CNumberCtrl m_posCtrl[3];
	bool m_enabledVector;

	CNumberCtrl m_moveSpeed;

	float m_width,m_height;
	//int m_heightMapX,m_heightMapY;

	int m_prevEditMode;
	int m_numSelected;
	int m_prevMoveSpeed;

	bool m_bVectorLock;
	bool m_bSelectionLocked;

	bool m_bEditingMode;
	CString m_sLastText;

	CEditTool *m_editTool;
public:
	afx_msg void OnBnClickedSyncplayer();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INFOBAR_H__F7C1D490_054E_43F7_BBF5_C9D6180E1936__INCLUDED_)
