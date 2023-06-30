#if !defined(AFX_VOLUMEPANEL_H__6E64DC13_4C7B_440F_9A46_B6E18D3B0BEA__INCLUDED_)
#define AFX_VOLUMEPANEL_H__6E64DC13_4C7B_440F_9A46_B6E18D3B0BEA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VolumePanel.h : header file
//
#include "ObjectPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CVolumePanel dialog

class CVolumePanel : public CObjectPanel
{
// Construction
public:
	CVolumePanel(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CVolumePanel)
	enum { IDD = IDD_PANEL_VOLUME };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	void SetSize( Vec3 &size );
	Vec3 GetSize();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVolumePanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVolumePanel)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CNumberCtrl m_size[3];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VOLUMEPANEL_H__6E64DC13_4C7B_440F_9A46_B6E18D3B0BEA__INCLUDED_)
