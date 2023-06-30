#if !defined(AFX_SOUNDOBJECTPANEL_H__4B0AC961_9C91_4325_B46C_36C191348B14__INCLUDED_)
#define AFX_SOUNDOBJECTPANEL_H__4B0AC961_9C91_4325_B46C_36C191348B14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SoundObjectPanel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSoundObjectPanel dialog

class CSoundObjectPanel : public CDialog
{
// Construction
public:
	CSoundObjectPanel(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSoundObjectPanel)
	enum { IDD = IDD_PANEL_SOUNDOBJECT };
	//}}AFX_DATA

	void SetSound( class CSoundObject *sound );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoundObjectPanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	CNumberCtrl	m_outerRadius;
	CNumberCtrl	m_innerRadius;
	CNumberCtrl	m_volume;

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	// Generated message map functions
	//{{AFX_MSG(CSoundObjectPanel)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CSoundObject *m_sound;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOUNDOBJECTPANEL_H__4B0AC961_9C91_4325_B46C_36C191348B14__INCLUDED_)
