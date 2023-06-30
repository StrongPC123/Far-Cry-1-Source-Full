#if !defined(AFX_PANELTREEBROWSER_H__ED063BD6_1FEA_4541_A216_BF7DA4DF4727__INCLUDED_)
#define AFX_PANELTREEBROWSER_H__ED063BD6_1FEA_4541_A216_BF7DA4DF4727__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PanelTreeBrowser.h : header file
//

#include "XTToolkit.h"

/////////////////////////////////////////////////////////////////////////////
// CPanelTreeBrowser dialog

class CPanelTreeBrowser : public CXTResizeDialog
{
// Construction
public:
	typedef Functor1<CString> SelectCallback;

	CPanelTreeBrowser(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPanelTreeBrowser();

// Dialog Data
	enum { IDD = IDD_PANEL_TREEBROWER };

	enum EFlags {
		NO_DRAGDROP = 0x001,			// Disable drag&drop of items to view,
		NO_PREVIEW = 0x002,
		SELECT_ONCLICK = 0x004, // Select callback when item is selected in tree view.
	};

	void Create( SelectCallback &cb,const CString &searchSpec,CWnd *parent,int flags=0 );
	void SetSelectCallback( SelectCallback &cb ) { m_selectCallback = cb; };
	void SelectFile( const CString &filename );
	void AddPreviewPanel();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPanelTreeBrowser)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	struct FilesInfo
	{
		std::vector<CString> files;
	};

	virtual void OnOK() {};
	virtual void OnCancel() {};

	// Generated message map functions
	//{{AFX_MSG(CPanelTreeBrowser)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnDblclkBrowserTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedBrowserTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnClickBrowserTree(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void SelectLastKnownItem();

	int RecurseDirectory( const CString &basePath,const CString &searchPath, HTREEITEM hRoot, LPCSTR pszFileSpec );
	CString	GetSelectedFile();
	void AcceptFile( const CString &file );

	void FillEntityScripts();
	void FillEntityPrototypes();
	void FillPrefabs();
	void FillFiles( FilesInfo &finfo );
	void LoadFiles( FilesInfo &info );

	void SortTree();
	void SortItem( HTREEITEM hParent );

	void Refresh( bool bReloadFiles );
	void OnReload();

	bool GetTreeItemIndex( HTREEITEM hRoot,HTREEITEM hItem,int &index );
	HTREEITEM GetTreeItemByIndex( HTREEITEM hRoot,int itemIndex,int &index );

	//////////////////////////////////////////////////////////////////////////
private:
	int m_flags;
	CImageList m_cImageList;
	StdMap<HTREEITEM,CString> m_itemsMap;
	std::map<CString,HTREEITEM,stl::less_stricmp<CString> > m_fileToItem;
	
	CString m_path;
	CString m_fileSpec;
	CString m_searchspec;

	CXTTreeCtrl	m_tree;
	CImageList *m_dragImage;
	CString m_draggedFile;
	CEdit m_filter;
	
	CCustomButton m_reloadBtn;

	SelectCallback m_selectCallback;

	class CPanelPreview *m_panelPreview;
	int m_panelPreviewId;

	bool m_bFiles;

	bool m_bDragDropEnabled;
	bool m_bSelectOnClick;

	static StdMap<CString,int> m_selectionHistory;

	typedef std::map<CString,FilesInfo> FileHistory;
	static FileHistory m_fileHistory;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PANELTREEBROWSER_H__ED063BD6_1FEA_4541_A216_BF7DA4DF4727__INCLUDED_)
