#if !defined(AFX_DIMENSIONSDIALOG_H__83F79962_0046_4B61_986F_430F473684E5__INCLUDED_)
#define AFX_DIMENSIONSDIALOG_H__83F79962_0046_4B61_986F_430F473684E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DimensionsDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDimensionsDialog dialog

class CDimensionsDialog : public CDialog
{
// Construction
public:
	UINT GetDimensions();
	void SetDimensions(UINT iWidth);
	CDimensionsDialog(CWnd* pParent = NULL);   // standard constructor

	bool GetCompressionQuality() const { return m_bQuality; };

// Dialog Data
	//{{AFX_DATA(CDimensionsDialog)
	enum { IDD = IDD_DIMENSIONS };
	int m_iSelection;
	BOOL m_bQuality;
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDimensionsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDimensionsDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIMENSIONSDIALOG_H__83F79962_0046_4B61_986F_430F473684E5__INCLUDED_)
