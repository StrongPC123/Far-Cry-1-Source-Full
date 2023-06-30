#if !defined(AFX_STRINGDLG_H__AFB020BD_DBC3_40C0_A03C_8531A6520F3B__INCLUDED_)
#define AFX_STRINGDLG_H__AFB020BD_DBC3_40C0_A03C_8531A6520F3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StringDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStringDlg dialog
class CStringDlg : public CDialog
{
// Construction
public:
	CStringDlg( const char *title = NULL,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStringDlg)
	enum { IDD = IDD_STRING };
	CString	m_strString;
	//}}AFX_DATA

	void SetString( const CString &str ) { m_strString = str; };
	CString GetString() { return m_strString; };


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStringDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStringDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CString m_title;
};

/////////////////////////////////////////////////////////////////////////////
// CStringDlg dialog
class CStringGroupDlg : public CDialog
{
// Construction
public:
	CStringGroupDlg( const char *title = NULL,CWnd* pParent = NULL);   // standard constructor

	// Dialog Data
	enum { IDD = IDD_STRING_GROUP };

	void SetString( const CString &str ) { m_strString = str; };
	CString GetString() { return m_strString; };

	void SetGroup( const CString &str ) { m_strGroup = str; };
	CString GetGroup() { return m_strGroup; };

// Overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:

	// Generated message map functions
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	CString	m_strString;
	CString	m_strGroup;
	CString m_title;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STRINGDLG_H__AFB020BD_DBC3_40C0_A03C_8531A6520F3B__INCLUDED_)
