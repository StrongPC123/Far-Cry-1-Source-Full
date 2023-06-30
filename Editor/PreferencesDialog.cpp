////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2003.
// -------------------------------------------------------------------------
//  File name:   PreferencesDialog.cpp
//  Version:     v1.00
//  Created:     28/10/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Editor Preferences Dialog.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PreferencesDialog.h"
#include "PreferencesPropertyPage.h"
#include "PreferencesStdPages.h"

#include "Settings.h"
#include "DisplaySettings.h"

// CPreferencesDialog dialog

IMPLEMENT_DYNAMIC(CPreferencesDialog, CXTResizeDialog)

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog dialog

CPreferencesDialog::CPreferencesDialog(CWnd* pParent /*=NULL*/)
: CXTResizeDialog(CPreferencesDialog::IDD, pParent)
{
	m_pSelected = NULL;

	static bool bAlreadyRegistered = false;
	if (!bAlreadyRegistered)
	{
		bAlreadyRegistered = true;
		GetIEditor()->GetClassFactory()->RegisterClass( new CStdPreferencesClassDesc );
	}
}

CPreferencesDialog::~CPreferencesDialog()
{}

void CPreferencesDialog::DoDataExchange(CDataExchange* pDX)
{
	CXTResizeDialog::DoDataExchange(pDX);
	DDX_Control( pDX,IDC_LIST_OPTIONS,m_wndTree );
	//{{AFX_DATA_MAP(CPreferencesDialog)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPreferencesDialog, CXTResizeDialog)
	//{{AFX_MSG_MAP(CPreferencesDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TVN_GETDISPINFO, IDC_LIST_OPTIONS, OnGetdispinfoListOptions)
	ON_NOTIFY(TVN_SELCHANGED, IDC_LIST_OPTIONS, OnSelchangedListOptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog message handlers

//////////////////////////////////////////////////////////////////////////
BOOL CPreferencesDialog::OnInitDialog()
{
	CXTResizeDialog::OnInitDialog();

	CRect rcWorkspace;
	GetDlgItem(IDC_WORKSPACE)->GetWindowRect(rcWorkspace);
	GetDlgItem(IDC_WORKSPACE)->ShowWindow( SW_HIDE );
	ScreenToClient(rcWorkspace);

	//m_imgList.Create(IDB_PREFERENCES, 16, 4, RGB(0, 0xFF, 0));
	CMFCUtils::LoadTrueColorImageList( m_imgList,IDB_PREFERENCES,16,RGB(0xFF,0,0xFF) );
	m_wndTree.SetImageList(&m_imgList, TVSIL_NORMAL);

	CreatePages( rcWorkspace );

	FillTree();

	SetResize( IDC_WORKSPACE,SZ_RESIZE(1) );
	SetResize( IDC_LIST_OPTIONS,SZ_VERRESIZE(1) );
	SetResize( IDC_LINE,CXTResizeRect(0,1,1,1) );
	SetResize( IDOK,SZ_REPOS(1) );
	SetResize( IDCANCEL,SZ_REPOS(1) );
	AutoLoadPlacement( "Dialogs\\Preferences" );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//////////////////////////////////////////////////////////////////////////
void CPreferencesDialog::OnSize( UINT nType,int cx,int cy )
{
	CXTResizeDialog::OnSize( nType,cx,cy );

	if (m_pSelected && m_pSelected->pWnd->GetSafeHwnd())
	{
		CRect rcWorkspace;
		GetDlgItem(IDC_WORKSPACE)->GetWindowRect(rcWorkspace);
		ScreenToClient(rcWorkspace);
		m_pSelected->pWnd->MoveWindow( rcWorkspace );
		m_pSelected->pWnd->RedrawWindow( );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPreferencesDialog::OnGetdispinfoListOptions(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if (pTVDispInfo->item.mask & (TVIF_SELECTEDIMAGE | TVIF_IMAGE))
	{
		if (m_wndTree.GetChildItem(pTVDispInfo->item.hItem) != NULL)
		{
			UINT nState = m_wndTree.GetItemState(pTVDispInfo->item.hItem, TVIF_STATE);
			pTVDispInfo->item.iSelectedImage = pTVDispInfo->item.iImage = nState & TVIS_EXPANDED? 3: 2;
		}
		else
		{
			//ASSERT(m_pSelected);

			pTVDispInfo->item.iSelectedImage = pTVDispInfo->item.iImage =
				(m_pSelected == (PageInfo*)pTVDispInfo->item.lParam? 0: 1);
		}
	}

	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CPreferencesDialog::OnSelchangedListOptions(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	//if (pNMTreeView->itemNew.lParam == 0)
		//return;

	if (pNMTreeView->itemNew.hItem)
	{

		PageInfo* pInfo = (PageInfo*)pNMTreeView->itemNew.lParam;
		if (!pInfo)
		{
			// Select first child.
			HTREEITEM hChildItem = m_wndTree.GetChildItem(pNMTreeView->itemNew.hItem);
			if (hChildItem)
				pInfo = (PageInfo*)m_wndTree.GetItemData(hChildItem);
		}

		if (pInfo != m_pSelected && pInfo)
		{
			//ASSERT(pPage);
			//ASSERT(pPage->IsKindOf(RUNTIME_CLASS(CXTPPropertyGrid)));
			CRect rcWorkspace;
			GetDlgItem(IDC_WORKSPACE)->GetWindowRect(rcWorkspace);
			ScreenToClient(rcWorkspace);

			pInfo->pWnd->MoveWindow( rcWorkspace );
			pInfo->pWnd->ShowWindow(SW_SHOW);	
			pInfo->pWnd->EnableWindow();
			pInfo->pWnd->ModifyStyle(0, WS_TABSTOP);

			if (m_pSelected)
			{
				m_pSelected->pWnd->ShowWindow(SW_HIDE);
			}
			m_pSelected = pInfo;
			//m_pSelected = (CPropertyGridEx*)pPage;

			m_wndTree.Invalidate(FALSE);
		} 

		HTREEITEM hItemParent = m_wndTree.GetParentItem(pNMTreeView->itemNew.hItem);
		if (m_pSelected != NULL && hItemParent)
		{

			m_wndTree.SetItemData(hItemParent, (DWORD)m_pSelected);
		}		

	}

	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CPreferencesDialog::OnOK() 
{
	// Call on OK for all pages.
	for (int i = 0; i < m_pagesInfo.size(); i++)
	{
		m_pagesInfo[i]->pPage->OnOK();
	}
	// Save settings.
	gSettings.Save();
	GetIEditor()->GetDisplaySettings()->SaveRegistry();

	CXTResizeDialog::OnOK();
}

//////////////////////////////////////////////////////////////////////////
void CPreferencesDialog::OnCancel() 
{
	int i;
	// QueryCancel for all pages.
	for (i = 0; i < m_pagesInfo.size(); i++)
	{
		if (!m_pagesInfo[i]->pPage->OnQueryCancel())
			return;
	}

	// Call on OK for all pages.
	for (i = 0; i < m_pagesInfo.size(); i++)
	{
		m_pagesInfo[i]->pPage->OnCancel();
	}
	CXTResizeDialog::OnCancel();
}

//////////////////////////////////////////////////////////////////////////
void CPreferencesDialog::CreatePages( const CRect &rc )
{
	std::vector<IClassDesc*> classes;
	GetIEditor()->GetClassFactory()->GetClassesBySystemID( ESYSTEM_CLASS_PREFERENCE_PAGE,classes );
	for (int i = 0; i < classes.size(); i++)
	{
		IUnknown *pUnknown = classes[i];

		IPreferencesPageCreator *pPageCreator = 0;
		if (FAILED(pUnknown->QueryInterface( &pPageCreator )))
			continue;

		int numPages = pPageCreator->GetPagesCount();
		for (int pindex = 0; pindex < numPages; pindex++)
		{
			IPreferencesPage *pPage = pPageCreator->CreatePage( pindex,rc,this );
			if (!pPage)
				continue;

			PageInfo *pInfo = new PageInfo;
			pInfo->pPage = pPage;
			pInfo->pWnd = pPage->GetWindow();
			assert( pInfo->pWnd );
			if (!pInfo->pWnd)
				continue;
			pInfo->category = pPage->GetCategory();
			pInfo->title = pPage->GetTitle();
			m_pagesInfo.push_back( pInfo );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CPreferencesDialog::FillTree()
{
	m_wndTree.SetRedraw(FALSE);
	m_wndTree.DeleteAllItems();

	std::map<CString,HTREEITEM,stl::less_stricmp<CString> > categoryMap;
	for (int i = 0; i < m_pagesInfo.size(); i++)
	{
		PageInfo *pInfo = m_pagesInfo[i];
		HTREEITEM hCategory = stl::find_in_map( categoryMap,pInfo->category,(HTREEITEM)0 );
		if (!hCategory)
		{
			// Make category.
			hCategory = m_wndTree.InsertItem( pInfo->category,I_IMAGECALLBACK,I_IMAGECALLBACK );
			categoryMap[pInfo->category] = hCategory;
			m_wndTree.Expand( hCategory,TVE_EXPAND );
		}
		HTREEITEM hItem = m_wndTree.InsertItem( pInfo->title,I_IMAGECALLBACK,I_IMAGECALLBACK,hCategory );
		m_wndTree.SetItemData( hItem,(DWORD_PTR)pInfo );
		m_wndTree.Expand( hItem,TVE_EXPAND );
		m_wndTree.Expand( hCategory,TVE_EXPAND );
	}
	m_wndTree.Expand( TVI_ROOT,TVE_EXPAND );
	m_wndTree.SetRedraw(TRUE);
}
