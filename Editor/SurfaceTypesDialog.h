#include "afxwin.h"
#if !defined(AFX_SURFACETYPESDIALOG_H__1E979D5B_B223_4E4E_82CC_D00CAF5A9C88__INCLUDED_)
#define AFX_SURFACETYPESDIALOG_H__1E979D5B_B223_4E4E_82CC_D00CAF5A9C88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SurfaceTypesDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSurfaceTypesDialog dialog
class CSurfaceType;

class CSurfaceTypesDialog : public CDialog
{
// Construction
public:
	CSurfaceTypesDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSurfaceTypesDialog)
	enum { IDD = IDD_SURFACE_TYPES };
	CComboBox	m_materials;
	CListBox	m_detailObjects;
	CListBox	m_types;
	CString	m_currSurfaceTypeName;
	//}}AFX_DATA

	void SetSelectedSurfaceType( const CString &srfTypeName );


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSurfaceTypesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	class CCryEditDoc* GetDoc();

	void ReloadSurfaceTypes();
	void ReloadDetailObjects();
	void SetCurrentSurfaceType( CSurfaceType *sf );
	void SerializeSurfaceTypes( class CXmlArchive &xmlAr );
	void DeleteNew();
	void ReloadMaterials();
	CSurfaceType* m_currSurfaceType;

	std::vector<CSurfaceType*> m_srfTypes;
	std::set<CSurfaceType*> m_newSurfaceTypes;

	CString m_firstSelected;

	CNumberCtrl m_scaleX;
	CNumberCtrl m_scaleY;
	CComboBox m_projCombo;
	
	// Custom buttons.
	CCustomButton m_btnSTAdd;
	CCustomButton m_btnSTRemove;
	CCustomButton m_btnSTClone;
	CCustomButton m_btnSTRename;

	CCustomButton m_btnBrowseDetailTex;
	CCustomButton m_btnBrowseBumpmapTex;

	CCustomButton m_btnDOAdd;
	CCustomButton m_btnDORemove;

	// Generated message map functions
	//{{AFX_MSG(CSurfaceTypesDialog)
	afx_msg void OnAddType();
	afx_msg void OnRemoveType();
	afx_msg void OnRenameType();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectSurfaceType();
	afx_msg void OnDetailTexture();
	afx_msg void OnAddDetailObject();
	afx_msg void OnRemoveDetailObject();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnImport();
	afx_msg void OnExport();
	afx_msg void OnDblclkSurfaceTypes();
	afx_msg void OnCloneSftype();
	afx_msg void OnSelendokMaterial();
	afx_msg void OnDetailScaleUpdate();
	afx_msg void OnEnChangeScalex();
	afx_msg void OnCbnSelendokProjection();
	afx_msg void OnBnClickedSftypeBrowseBumpmap();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SURFACETYPESDIALOG_H__1E979D5B_B223_4E4E_82CC_D00CAF5A9C88__INCLUDED_)
