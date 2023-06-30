#if !defined(AFX_FILETREE_H__9848DF42_CD7E_4ADD_96F9_E01605C420D6__INCLUDED_)
#define AFX_FILETREE_H__9848DF42_CD7E_4ADD_96F9_E01605C420D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileTree.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileTree window

class CFileTree : public CTreeCtrl
{
// Construction
public:
	CFileTree();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileTree)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	CString GetSelectedFile();
	virtual ~CFileTree();

	// Sets the search pattern that decides which files should be displayed
	void SetFileSpec(CString strFileSpec) { m_strFileSpec = strFileSpec; };

	// Spezifies the root folder of the search, it is constructed as follows:
	// X:\%MasterCD%\%SearchFolder%
	void SetSearchFolder(CString strSearchFolder) { m_strSearchFolder = strSearchFolder; };
	CString GetSearchFolder() { return m_strSearchFolder; };

	// Generated message map functions
protected:
	//{{AFX_MSG(CFileTree)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	CImageList m_cImageList;

	void RecurseDirectory(char szFolder[_MAX_PATH], HTREEITEM hRoot, PSTR pszFileSpec);
	typedef std::map<HTREEITEM,CString> Files;
	Files m_cFileNames;

	CString m_strFileSpec;
	CString m_strSearchFolder;

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILETREE_H__9848DF42_CD7E_4ADD_96F9_E01605C420D6__INCLUDED_)
