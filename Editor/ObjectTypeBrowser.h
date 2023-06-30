#if !defined(AFX_OBJECTTYPEBROWSER_H__F0DB32FF_8910_4B45_B1CC_949E0BAF13FD__INCLUDED_)
#define AFX_OBJECTTYPEBROWSER_H__F0DB32FF_8910_4B45_B1CC_949E0BAF13FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ObjectTypeBrowser.h : header file
//
class CObjectCreateTool;

/////////////////////////////////////////////////////////////////////////////
// ObjectTypeBrowser dialog

class ObjectTypeBrowser : public CDialog
{
// Construction
public:
	ObjectTypeBrowser(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ObjectTypeBrowser)
	enum { IDD = IDD_OBJTYPE_BROWSER };
	CListBox	m_list;
	//}}AFX_DATA

	void SetCategory( CObjectCreateTool *createTool,const CString &category );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ObjectTypeBrowser)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	// Generated message map functions
	//{{AFX_MSG(ObjectTypeBrowser)
	afx_msg void OnSelchangeList();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CString m_category;

	CBrush m_listBrush;
	int m_lastSel;

	CObjectCreateTool *m_createTool;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OBJECTTYPEBROWSER_H__F0DB32FF_8910_4B45_B1CC_949E0BAF13FD__INCLUDED_)
