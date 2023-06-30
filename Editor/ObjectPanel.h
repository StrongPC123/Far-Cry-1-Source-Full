#include "afxwin.h"
#if !defined(AFX_OBJECTPANEL_H__796370A4_3BF7_4B2E_B4E7_7101945033E7__INCLUDED_)
#define AFX_OBJECTPANEL_H__796370A4_3BF7_4B2E_B4E7_7101945033E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ObjectPanel.h : header file
//

#include "XTToolkit.h"

class CBaseObject;

/////////////////////////////////////////////////////////////////////////////
// CObjectPanel dialog

class CObjectPanel : public CXTResizeDialog
{
// Construction
public:
	struct SParams
	{
		CString name;
		COLORREF color;
		float area;
		float helperScale;
		//bool flatten;
		//bool shared;
		CString layer;
	};
	//! If multiSe s true, object panel works as multi selection panel.
	CObjectPanel(CWnd* pParent = NULL);   // standard constructor
	~CObjectPanel();

// Dialog Data
	//{{AFX_DATA(CObjectPanel)
	enum { IDD = IDD_OBJECT_PANEL };
	CButton	m_colorCtrl;
	CString	m_name;
	CNumberCtrl m_area;
	CNumberCtrl m_helperSize;
	CEdit m_nameCtrl;
	//CButton m_flattenCtrl;
	//CButton m_sharedCtrl;
	CCustomButton m_layerBtn;
	CCustomButton m_mtlBtn;
	CStatic m_layerName;

	//BOOL	m_flatten;
	//BOOL	m_bShared;
	//}}AFX_DATA

	void SetMultiSelect( bool bEnable );
	bool IsMultiSelect() const { return m_multiSelect; };
	void SetParams( CBaseObject *obj,const SParams &params );
	void GetParams( SParams &params );

	CBaseObject* GetObject() const { return m_obj; }


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectPanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	virtual void	OnUpdate();

// Implementation
protected:
	virtual void OnOK();
	virtual void OnCancel() {};

	// Generated message map functions
	//{{AFX_MSG(CObjectPanel)
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnObjectColor();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL OnInitDialog();
	//afx_msg void OnShared();
	afx_msg void OnUpdateName();
	//afx_msg void OnUpdateFlatten();
	afx_msg void OnUpdateArea();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBaseObject *m_obj;
	bool m_multiSelect;
	COLORREF m_color;
	CString m_currentLayer;
public:
	afx_msg void OnBnClickedLayer();
	afx_msg void OnChangeName();
	afx_msg void OnBnClickedMaterial();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OBJECTPANEL_H__796370A4_3BF7_4B2E_B4E7_7101945033E7__INCLUDED_)
