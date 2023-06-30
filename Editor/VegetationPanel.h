////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   vegetationpanel.h
//  Version:     v1.00
//  Created:     31/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __vegetationpanel_h__
#define __vegetationpanel_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "XTToolkit.h"

#include "Controls\PropertyCtrl.h"
#include "Controls\MltiTree.h"

/////////////////////////////////////////////////////////////////////////////
// CVegetationPanel dialog

class CVegetationObject;
class CVegetationMap;
class CPanelPreview;

class CVegetationPanel : public CXTCBarDialog
{
// Construction
public:
	CVegetationPanel(class CVegetationTool *tool,CWnd* pParent = NULL);   // standard constructor

	void SetPreviewPanel( CPanelPreview *panel ) { m_previewPanel = panel; }
	void SetObjectPanel( class CVegetationObjectPanel *panel );
	void SelectObject( CVegetationObject *object );
	void UpdateObjectInTree( CVegetationObject *object,bool bUpdateInfo=true );
	void UpdateAllObjectsInTree();
	void UpdateInfo();

// Dialog Data
	//{{AFX_DATA(CVegetationPanel)
	enum { IDD = IDD_PANEL_VEGETATION };
	CSliderCtrl	m_radius;
	CColorCheckBox	m_paintButton;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVegetationPanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	void ReloadObjects();
	void AddObjectToTree( CVegetationObject *object,bool bExpandCategory=true );
	void RemoveObjectFromTree( CVegetationObject *object );

	void GetObjectsInCategory( const CString &category,std::vector<CVegetationObject*> &objects );
	void SendToControls();

	void GetSelectedObjects( std::vector<CVegetationObject*> &objects );

	// Generated message map functions
	//{{AFX_MSG(CVegetationPanel)
	afx_msg void OnReleasedcaptureRadius(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	afx_msg void OnAdd();
	afx_msg void OnClone();
	afx_msg void OnReplace();
	afx_msg void OnRemove();

	afx_msg void OnNewCategory();
	afx_msg void OnDistribute();
	afx_msg void OnDistributeMask();
	afx_msg void OnClear();
	afx_msg void OnClearMask();
	afx_msg void OnBnClickedImport();
	afx_msg void OnBnClickedExport();
	afx_msg void OnObjectSelectionChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnKeydownObjects(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnBeginlabeleditObjects(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnEndlabeleditObjects(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnHideObjects(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedScale();

	//////////////////////////////////////////////////////////////////////////
	// Controls.
	CString m_category;

	CVegetationTool *m_tool;

	CXTToolBar m_toolbar;

	//CMultiTree m_objectsTree;
	CMultiTree m_objectsTree;
	CImageList m_treeImageList;
	CImageList m_treeStateImageList;

	CPropertyCtrl m_propertyCtrl;
	CPanelPreview *m_previewPanel;

	CStatic m_info;

	typedef std::vector<CVegetationObject*> Selection;
	std::map<CString,HTREEITEM> m_categoryMap;
	CVegetationMap* m_vegetationMap;

	TSmartPtr<CVarBlock> m_varBlock;
};

#endif // __vegetationpanel_h__