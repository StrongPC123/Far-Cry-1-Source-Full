#if !defined(AFX_TEXTUREPANEL_H__6D3249D3_ED60_4A93_B21C_6CFCCB1117CA__INCLUDED_)
#define AFX_TEXTUREPANEL_H__6D3249D3_ED60_4A93_B21C_6CFCCB1117CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TexturePanel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TexturePanel dialog

class CTexturePanel : public CDialog
{
// Construction
public:
	CTexturePanel(class CTextureTool *tool,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTexturePanel)
	enum { IDD = IDD_PANEL_TEXTURE_TOOL };
	CCustomButton	m_apply;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTexturePanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	// Generated message map functions
	//{{AFX_MSG(CTexturePanel)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CTextureTool *m_tool;

	CNumberCtrl m_shift[2];
	CNumberCtrl m_scale[2];
	CNumberCtrl m_rotate;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTUREPANEL_H__6D3249D3_ED60_4A93_B21C_6CFCCB1117CA__INCLUDED_)
