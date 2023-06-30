// PanelTreeBrowser.cpp : implementation file
//

#include "stdafx.h"
#include "PanelTreeBrowser.h"
#include "PanelPreview.h"

#include "ViewManager.h"
#include "Viewport.h"

#include "Objects\EntityScript.h"

#include "EntityPrototype.h"
#include "EntityPrototypeLibrary.h"
#include "EntityPrototypeManager.h"
#include "Prefabs\PrefabManager.h"
#include "Settings.h"

#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Sort the item in reverse alphabetical order.
static int CALLBACK TreeCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// lParamSort contains a pointer to the tree control.
	// The lParam of an item is just its handle, as specified with SetItemData
	CTreeCtrl* pTreeCtrl = (CTreeCtrl*)lParamSort;

	int nChilds1 = (pTreeCtrl->ItemHasChildren((HTREEITEM)lParam1) == TRUE) ? 1 : 0;
	int nChilds2 = (pTreeCtrl->ItemHasChildren((HTREEITEM)lParam2) == TRUE) ? 1 : 0;
	if (nChilds1 != nChilds2)
		return nChilds2 - nChilds1;

	CString strItem1 = pTreeCtrl->GetItemText((HTREEITEM) lParam1);
	CString strItem2 = pTreeCtrl->GetItemText((HTREEITEM) lParam2);

	return stricmp(strItem1, strItem2);
}

//////////////////////////////////////////////////////////////////////////
// Simple sub string searching case insensitive.
//////////////////////////////////////////////////////////////////////////
inline const char* strstri( const char *s1, const char *s2) 
{
	int i,j,k; 
	for(i=0;s1[i];i++) 
		for(j=i,k=0;tolower(s1[j])==tolower(s2[k]);j++,k++) 
			if(!s2[k+1]) 
				return (s1+i); 

	return NULL; 
} 

/////////////////////////////////////////////////////////////////////////////
// CPanelTreeBrowser dialog
StdMap<CString,int> CPanelTreeBrowser::m_selectionHistory;
CPanelTreeBrowser::FileHistory CPanelTreeBrowser::m_fileHistory;

CPanelTreeBrowser::CPanelTreeBrowser(CWnd* pParent /*=NULL*/)
	: CXTResizeDialog(CPanelTreeBrowser::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPanelTreeBrowser)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_panelPreview = NULL;
	m_panelPreviewId = 0;
}

CPanelTreeBrowser::~CPanelTreeBrowser()
{
	if (m_panelPreviewId)
		GetIEditor()->RemoveRollUpPage( ROLLUP_OBJECTS,m_panelPreviewId );
	m_panelPreviewId = 0;
	m_panelPreview = 0;
}

void CPanelTreeBrowser::DoDataExchange(CDataExchange* pDX)
{
	CXTResizeDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BROWSER_TREE, m_tree);
	DDX_Control(pDX, IDC_RELOAD,m_reloadBtn );
	DDX_Control(pDX, IDC_FILTER,m_filter );
}


BEGIN_MESSAGE_MAP(CPanelTreeBrowser, CXTResizeDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_BROWSER_TREE, OnDblclkBrowserTree)
	ON_NOTIFY(TVN_SELCHANGED, IDC_BROWSER_TREE, OnSelchangedBrowserTree)
	ON_NOTIFY(TVN_BEGINDRAG, IDC_BROWSER_TREE, OnBeginDrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_NOTIFY(NM_CLICK, IDC_BROWSER_TREE, OnClickBrowserTree)
	ON_BN_CLICKED(IDC_RELOAD, OnReload)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPanelTreeBrowser message handlers

BOOL CPanelTreeBrowser::OnInitDialog() 
{
	CXTResizeDialog::OnInitDialog();

	m_dragImage = 0;
	m_bFiles = false;

	CMFCUtils::LoadTrueColorImageList( m_cImageList,IDB_TREE_VIEW,16,RGB(255,0,255) );
	
	// Create the list
	//m_cImageList.Create(IDB_TREE_VIEW, 16, 1, RGB (255, 0, 255));

	// Attach it to the control
	m_tree.SetImageList(&m_cImageList, TVSIL_NORMAL);
	//m_tree.SetImageList(&m_cImageList, TVSIL_STATE);

	m_tree.SetIndent( 0 );
	//m_tree.SetBkColor( RGB(0xE0,0xE0,0xE0) );
	
	SetResize( IDC_BROWSER_TREE,SZ_RESIZE(1) );
	SetResize( IDC_FILTER,SZ_RESIZE(1) );

	// Add the Master CD folder to the list
	//HTREEITEM hRoot = m_tree.InsertItem( "Root", 2, 2, TVI_ROOT);

	/*
	RecurseDirectory( "c:\\mastercd\\Objects", TVI_ROOT, "*.cgf" );

	//m_tree.Expand( m_tree.GetRootItem(), TVE_COLLAPSE);
	//CRect rc;
	//GetClientRect( rc );
	//int h = m_tree.GetVisibleCount()*m_tree.GetItemHeight();
	//SetWindowPos( NULL,0,0,rc.right,h+80,SWP_NOMOVE );

	m_tree.Expand( m_tree.GetFirstVisibleItem(), TVE_COLLAPSE);
	*/
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CPanelTreeBrowser::OnDestroy()
{
	CXTResizeDialog::OnDestroy();
}

void CPanelTreeBrowser::Create( SelectCallback &cb,const CString &searchSpec,CWnd *parent,int flags )
{
	CXTResizeDialog::Create( IDD,parent );

	//////////////////////////////////////////////////////////////////////////
	SetResize( IDC_RELOAD,SZ_HORREPOS(1) );
	//////////////////////////////////////////////////////////////////////////

	m_flags = flags;
	m_bSelectOnClick = flags & SELECT_ONCLICK;
	m_bDragDropEnabled = !(flags & NO_DRAGDROP);

	m_searchspec = searchSpec;
	Refresh(false);

	m_selectCallback = cb;
}

//////////////////////////////////////////////////////////////////////////
void CPanelTreeBrowser::Refresh( bool bReloadFiles )
{
	CWaitCursor wait;
	CString searchSpec = m_searchspec;

	char dir[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char fname[_MAX_FNAME];
	char fext[_MAX_EXT];

	_splitpath( searchSpec,drive,dir,fname,fext );

	CString searchPath = Path::GetPath( searchSpec );
	CString fileSpec = searchSpec.Mid( searchPath.GetLength() );

	m_path = dir;
	m_fileSpec = fileSpec;

	//@HACK
	// hardcoded names.
	if (searchSpec == "*EntityClass")
	{
		// Entity classes.
		FillEntityScripts();
	}
	else if (searchSpec == "*EntityPrototype")
	{
		// Entity prototypes.
		FillEntityPrototypes();
	}
	else if (searchSpec == "*Prefabs")
	{
		// Entity prototypes.
		FillPrefabs();
	}
	else
	{
		FileHistory::iterator it = m_fileHistory.find(fileSpec);
		if (it == m_fileHistory.end() || bReloadFiles)
		{
			// Load files from disk.
			FilesInfo finfo;
			LoadFiles( finfo );
			FillFiles( finfo );
		}
		else {
			FillFiles( it->second );
		}
		m_bFiles = true;

		/*
		// Just files.
		RecurseDirectory( GetIEditor()->GetMasterCDFolder(),dir,TVI_ROOT,fileSpec );
		*/
		CString numFiles;
		numFiles.Format( "%d Files",m_itemsMap.GetCount() );
		SetDlgItemText( IDC_NUM_FILES,numFiles );
	}

	SelectLastKnownItem();
}

//////////////////////////////////////////////////////////////////////////
void CPanelTreeBrowser::AddPreviewPanel()
{
	if (m_panelPreview)
		return;
	if (!(m_flags & NO_PREVIEW) && gSettings.bPreviewGeometryWindow)
	{
		// Searching geometries.
		if (strstr(m_fileSpec,"*.ccgf") != 0 || strstr(m_fileSpec,"*.cgf") != 0 || strstr(m_fileSpec,"*.cga") != 0)
		{
			// Create Preview.
			m_panelPreview = new CPanelPreview( AfxGetMainWnd() );
			m_panelPreviewId = GetIEditor()->AddRollUpPage( ROLLUP_OBJECTS,"Object Preview",m_panelPreview );
		}
	}
}

void CPanelTreeBrowser::SelectLastKnownItem()
{
	//Select item from history.
	int index;
	if (m_selectionHistory.Find(m_path+m_fileSpec,index))
	{
		int cnt = 0;
		HTREEITEM hItem = GetTreeItemByIndex( m_tree.GetRootItem(),index,cnt );
		if (hItem) {
			m_tree.Select( hItem,TVGN_CARET );
			m_tree.EnsureVisible( hItem );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CPanelTreeBrowser::SelectFile( const CString &filename )
{
	SelectCallback cb = m_selectCallback;
	m_selectCallback = 0;
	HTREEITEM hItem = stl::find_in_map( m_fileToItem,filename,(HTREEITEM)0 );
	if (hItem)
	{
		m_tree.Select( hItem,TVGN_CARET );
		m_tree.EnsureVisible( hItem );
		//m_tree.SetItemBold( hItem,TRUE );
		//m_tree.SetItemColor( hItem,RGB(255,0,0) );
	}
	m_selectCallback = cb;
}

//////////////////////////////////////////////////////////////////////////
void CPanelTreeBrowser::FillEntityScripts()
{
	m_tree.SetRedraw(FALSE);
	m_fileToItem.clear();
	m_itemsMap.Clear();
	m_tree.DeleteAllItems();

	CString filter;
	m_filter.GetWindowText( filter );
	bool bFilter = !filter.IsEmpty();

	StdMap<CString,HTREEITEM> items;
	// Entity scripts.
	std::vector<CEntityScript*> scripts;
	CEntityScriptRegistry::Instance()->GetScripts( scripts );
	for (int i = 0; i < scripts.size(); i++)
	{
		// If class is not usable simply skip it.
		if (!scripts[i]->IsUsable())
			continue;

		CString name = scripts[i]->GetName();

		if (bFilter)
		{
			if (strstri(name,filter) == 0)
				continue;
		}

		HTREEITEM hRoot = TVI_ROOT;
		char *token;

		CString clsFile = scripts[i]->GetRelativeFile();
		clsFile.Replace( "Scripts\\Default\\Entities\\","" );
		char classFile[1024];
		strcpy( classFile,clsFile );

		token = strtok( classFile,"\\/" );

		CString itemName;
		while (token)
		{
			CString strToken = token;

			token = strtok( NULL,"\\/" );
			if (!token)
				break;

			itemName += strToken+"\\";
			if (!items.Find( itemName,hRoot ))
			{
				hRoot = m_tree.InsertItem(strToken, 0, 0, hRoot );
				m_tree.SetItemData( hRoot,(DWORD_PTR)hRoot );// Fort sorting.
				items.Insert( itemName,hRoot );
			}
		}

		HTREEITEM hNewItem = m_tree.InsertItem(name, 3, 3, hRoot );
		m_tree.SetItemData( hNewItem,(DWORD_PTR)hNewItem );// Fort sorting.
		m_tree.SetItemState(hNewItem, TVIS_BOLD, TVIS_BOLD);
		m_itemsMap[hNewItem] = name;
	}
	CString numFiles;
	numFiles.Format( "%d Entities",m_itemsMap.GetCount() );
	SetDlgItemText( IDC_NUM_FILES,numFiles );

	SortTree();

	m_tree.SetRedraw(TRUE);
}
	
//////////////////////////////////////////////////////////////////////////
void CPanelTreeBrowser::FillEntityPrototypes()
{
	m_tree.SetRedraw(FALSE);
	m_fileToItem.clear();
	m_itemsMap.Clear();
	m_tree.DeleteAllItems();

	CString filter;
	m_filter.GetWindowText( filter );
	bool bFilter = !filter.IsEmpty();

	CEntityPrototypeManager *protMan = GetIEditor()->GetEntityProtManager();

	for (int j = 0; j < protMan->GetLibraryCount(); j++)
	{
		IDataBaseLibrary *lib = protMan->GetLibrary(j);

		if (bFilter)
		{
			if (strstri(lib->GetName(),filter) == 0)
				continue;
		}

		std::map<CString,HTREEITEM> groupMap;

		HTREEITEM hLibItem = m_tree.InsertItem( lib->GetName(), 0, 0 );
		m_tree.SetItemData( hLibItem,(DWORD_PTR)hLibItem );// Fort sorting.
		for (int i = 0; i < lib->GetItemCount(); i++)
		{
			CEntityPrototype* prototype = (CEntityPrototype*)lib->GetItem(i);

			CString groupName = prototype->GetGroupName();

			HTREEITEM hGroupItem = hLibItem;
			if (!groupName.IsEmpty())
			{
				HTREEITEM hItem = stl::find_in_map( groupMap,groupName,(HTREEITEM)0 );
				if (hItem)
				{
					hGroupItem = hItem;
				}
				else
				{
					hGroupItem = m_tree.InsertItem( groupName, 0, 0, hLibItem );
					m_tree.SetItemData( hGroupItem,(DWORD_PTR)hGroupItem );// Fort sorting.
					groupMap[groupName] = hGroupItem;
				}
			}
			CString itemName = prototype->GetShortName();
			HTREEITEM hNewItem = m_tree.InsertItem(itemName, 3, 3, hGroupItem );
			m_tree.SetItemData( hNewItem,(DWORD_PTR)hNewItem );// Fort sorting.
			m_tree.SetItemState(hNewItem, TVIS_BOLD, TVIS_BOLD);

			CString fullItemName = GuidUtil::ToString(prototype->GetGUID());
			m_itemsMap[hNewItem] = fullItemName;
		}
	}

	CString numFiles;
	numFiles.Format( "%d Entities",m_itemsMap.GetCount() );
	SetDlgItemText( IDC_NUM_FILES,numFiles );

	SortTree();

	m_tree.SetRedraw(TRUE);
}


//////////////////////////////////////////////////////////////////////////
void CPanelTreeBrowser::FillPrefabs()
{
	m_tree.SetRedraw(FALSE);
	m_fileToItem.clear();
	m_itemsMap.Clear();
	m_tree.DeleteAllItems();

	CString filter;
	m_filter.GetWindowText( filter );
	bool bFilter = !filter.IsEmpty();

	CPrefabManager *pManager = GetIEditor()->GetPrefabManager();

	for (int j = 0; j < pManager->GetLibraryCount(); j++)
	{
		IDataBaseLibrary *lib = pManager->GetLibrary(j);

		if (bFilter)
		{
			if (strstri(lib->GetName(),filter) == 0)
				continue;
		}

		std::map<CString,HTREEITEM> groupMap;

		HTREEITEM hLibItem = m_tree.InsertItem( lib->GetName(), 0, 0 );
		m_tree.SetItemData( hLibItem,(DWORD_PTR)hLibItem );// Fort sorting.
		for (int i = 0; i < lib->GetItemCount(); i++)
		{
			CEntityPrototype* prototype = (CEntityPrototype*)lib->GetItem(i);

			CString groupName = prototype->GetGroupName();

			HTREEITEM hGroupItem = hLibItem;
			if (!groupName.IsEmpty())
			{
				HTREEITEM hItem = stl::find_in_map( groupMap,groupName,(HTREEITEM)0 );
				if (hItem)
				{
					hGroupItem = hItem;
				}
				else
				{
					hGroupItem = m_tree.InsertItem( groupName, 0, 0, hLibItem );
					m_tree.SetItemData( hGroupItem,(DWORD_PTR)hGroupItem );// Fort sorting.
					groupMap[groupName] = hGroupItem;
				}
			}
			CString itemName = prototype->GetShortName();
			HTREEITEM hNewItem = m_tree.InsertItem(itemName, 1, 1, hGroupItem );
			m_tree.SetItemData( hNewItem,(DWORD_PTR)hNewItem );// Fort sorting.
			m_tree.SetItemState(hNewItem, TVIS_BOLD, TVIS_BOLD);

			CString fullItemName = GuidUtil::ToString(prototype->GetGUID());
			m_itemsMap[hNewItem] = fullItemName;
		}
	}

	CString numFiles;
	numFiles.Format( "%d Prefabs",m_itemsMap.GetCount() );
	SetDlgItemText( IDC_NUM_FILES,numFiles );

	SortTree();

	m_tree.SetRedraw(TRUE);
}

//////////////////////////////////////////////////////////////////////////
int CPanelTreeBrowser::RecurseDirectory( const CString &basePath,const CString &searchPath, HTREEITEM hRoot, LPCSTR pszFileSpec )
{
	////////////////////////////////////////////////////////////////////////
	// Enumerate all files in the passed directory which match to the the
	// passed pattern. Also continue with adding all subdirectories
	////////////////////////////////////////////////////////////////////////

	CString szFolder = searchPath;

	//char szFolder[_MAX_PATH];
	//strcpy( szFolder,searchPath );

	CFileEnum cTempFiles;
	__finddata64_t sFile;
	char szFilePath[_MAX_PATH];
	HTREEITEM hNewRoot, hNewItem;

	ASSERT(pszFileSpec);

	CString fileSpec = pszFileSpec;
	std::vector<CString> fileSpecs;

	// Split file spec with ';'
	while (!fileSpec.IsEmpty())
	{
		int splitpos = fileSpec.Find(';');
		if (splitpos < 0)
		{
			fileSpecs.push_back(fileSpec);
			break;
		}
		fileSpecs.push_back(fileSpec.Mid(0,splitpos));
		fileSpec = fileSpec.Mid(splitpos+1);
	}

	// Make the path ready for appening a folder or filename
	szFolder = Path::AddBackslash(szFolder);
	int numFiles = 0;

	// Start the enumeration of the files
	if (cTempFiles.StartEnumeration(szFolder, "*.*", &sFile))
	{
		do
		{
			// Construct the full filepath of the current file
			strcpy(szFilePath, szFolder);
			strcat(szFilePath, sFile.name);

			// Have we found a directory ?
			if (sFile.attrib & _A_SUBDIR)
			{
				// Skip the parent directory entries
				if (_stricmp(sFile.name, ".") == 0 ||
					_stricmp(sFile.name, "..") == 0)
				{
					continue;
				}

				// Add it to the list and start recursion
				hNewRoot = m_tree.InsertItem(sFile.name, 0, 0, hRoot);
				m_tree.SetItemData( hNewRoot,(DWORD_PTR)hNewRoot );// Fort sorting.
				int num = RecurseDirectory( basePath,szFilePath, hNewRoot, pszFileSpec);
				m_tree.Expand( hNewRoot,TVE_COLLAPSE);
				m_tree.SortChildren(hNewRoot);
				if (num == 0)
				{
					// Delete empty directory item.
					m_tree.DeleteItem( hNewRoot );
				}
				numFiles += num;

				continue;
			}

			int nImage = 1;

			// Check if the file name maches the pattern
			bool bFileMatch = false;
			for (int i = 0; i < fileSpecs.size(); i++)
			{
				if (PathMatchSpec(sFile.name,fileSpecs[i]))
				{
					bFileMatch = true;
					break;
				}
				nImage = 2;
			}
			if (!bFileMatch)
				continue;

			// Remove the extension from the name
			PathRenameExtension(sFile.name, "");

			// Add the file to the list
			hNewItem = m_tree.InsertItem(sFile.name, nImage, nImage, hRoot);
			m_tree.SetItemData( hNewItem,(DWORD_PTR)hNewItem );// Fort sorting.
			m_tree.SetItemState(hNewItem, TVIS_BOLD, TVIS_BOLD);

			m_itemsMap[hNewItem] = szFilePath;
      

			numFiles++;

		} while (cTempFiles.GetNextFile(&sFile));
	}
	return numFiles;
}

//////////////////////////////////////////////////////////////////////////
void CPanelTreeBrowser::LoadFiles( FilesInfo &info )
{
	CString fileSpec = m_fileSpec;
	CString currFileSpec;
	static CFileUtil::FileArray files;

	info.files.clear();

	// Reserve many files.
	files.resize(0);
	files.reserve( 10000 );

	// Split file spec with ';'
	while (!fileSpec.IsEmpty())
	{
		int splitpos = fileSpec.Find(';');
		if (splitpos < 0)
		{
			currFileSpec = fileSpec;
			CFileUtil::ScanDirectory( m_path,currFileSpec,files,true );
			break;
		}
		CString currFileSpec = fileSpec.Mid(0,splitpos);
		CFileUtil::ScanDirectory( m_path,currFileSpec,files,true );
		fileSpec = fileSpec.Mid(splitpos+1);
	}
	
	info.files.reserve( files.size() );
	for (int i = 0; i < files.size(); i++)
	{
		info.files.push_back( files[i].filename );
	}
	//std::sort( info.files.begin(),info.files.end() );

	m_fileHistory[m_fileSpec] = info;
}

//////////////////////////////////////////////////////////////////////////
void CPanelTreeBrowser::FillFiles( FilesInfo &finfo )
{
	m_tree.SetRedraw( FALSE );
	m_fileToItem.clear();
	m_itemsMap.Clear();

	m_tree.DeleteAllItems();

	int offsetPath = Path::AddBackslash(m_path).GetLength();

	CString filter;
	m_filter.GetWindowText( filter );
	bool bFilter = !filter.IsEmpty();

	CString filename,ext,path;
	std::map<CString,HTREEITEM> pathmap;
	for (int i = 0; i < finfo.files.size(); i++)
	{
		Path::Split( finfo.files[i],path,filename,ext );
		
		if (bFilter)
		{
			if (strstri(filename,filter) == 0)
				continue;
		}

		HTREEITEM hGroup = TVI_ROOT;

		if (!path.IsEmpty())
		{
			hGroup = stl::find_in_map( pathmap,path,(HTREEITEM)0 );
			if (!hGroup)
			{
				hGroup = TVI_ROOT;
				int startPos = 0;
				int prevPos = 0;
				CString subpath;
				while (startPos < path.GetLength())
				{
					prevPos = startPos;
					int pos = path.Find( '\\',startPos );
					if (pos >= 0)
					{
						startPos = pos+1;
					}
					else
						startPos = path.GetLength();
					subpath = path.Mid(0,pos);
					HTREEITEM hItem = stl::find_in_map( pathmap,subpath,(HTREEITEM)0 );
					if (!hItem)
					{
						CString subGroupName = path.Mid(prevPos,startPos-prevPos-1);
						hGroup = m_tree.InsertItem( subGroupName,0,0,hGroup );
						m_tree.SetItemData( hGroup,(DWORD_PTR)hGroup );// Fort sorting.
						pathmap[subpath] = hGroup;
					}
					else
						hGroup = hItem;
				}
			}
		}

		int nImage = 1;
		if (stricmp(ext,".cga")==0)
		{
			nImage = 2;
		}
		HTREEITEM hNewItem = m_tree.InsertItem( filename, nImage, nImage, hGroup );
		m_tree.SetItemData( hNewItem,(DWORD_PTR)hNewItem );// Fort sorting.
		//m_tree.SetItemState(hNewItem, TVIS_BOLD, TVIS_BOLD);
		CString filename = Path::AddBackslash(m_path) + finfo.files[i];
		m_itemsMap[hNewItem] = filename;
		m_fileToItem[filename] = hNewItem;
	}

	SortTree();

	m_tree.SetRedraw( TRUE );
}

//////////////////////////////////////////////////////////////////////////
void CPanelTreeBrowser::OnDblclkBrowserTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

	if (m_tree.GetSelectedItem() == NULL)
		return;
		
	CString file = GetSelectedFile();
	if (file.IsEmpty())
		return;

	AcceptFile( file );
}

CString CPanelTreeBrowser::GetSelectedFile()
{
	////////////////////////////////////////////////////////////////////////
	// Return the path of the currently selected file. If there is no
	// currently selected file, just a NULL terminated string will be
	// returned
	////////////////////////////////////////////////////////////////////////

	CString strFileName;
	m_itemsMap.Find( m_tree.GetSelectedItem(),strFileName );
	return strFileName;
}

void CPanelTreeBrowser::AcceptFile( const CString &file )
{
	if (!file.IsEmpty())
	{
		// Select this file.
		if (m_selectCallback)
			m_selectCallback( file );
		//GetIEditor()->StartObjectCreation( m_objectType,file );
	}
}

bool CPanelTreeBrowser::GetTreeItemIndex( HTREEITEM hRoot,HTREEITEM hItem,int &index )
{
	HTREEITEM hIt = hRoot;
	while (hIt != NULL)
	{
		if (hIt == hItem)
			return true;

		index++;

		if (m_tree.ItemHasChildren(hIt))
		{
			if (GetTreeItemIndex( m_tree.GetNextItem(hIt,TVGN_CHILD),hItem,index ))
			{
				return true;
			}
		}
		hIt = m_tree.GetNextSiblingItem( hIt );
	}
	return false;
}

HTREEITEM CPanelTreeBrowser::GetTreeItemByIndex( HTREEITEM hRoot,int itemIndex,int &index )
{
	HTREEITEM hIt = hRoot;
	while (hIt != NULL)
	{
		if (index == itemIndex)
			return hIt;

		index++;
		if (m_tree.ItemHasChildren(hIt))
		{
			HTREEITEM h = GetTreeItemByIndex( m_tree.GetNextItem(hIt,TVGN_CHILD),itemIndex,index );
			if (h != NULL)
			{
				return h;
			}
		}
		hIt = m_tree.GetNextSiblingItem( hIt );
	}
	return 0;
}

void CPanelTreeBrowser::OnSelchangedBrowserTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	int index = 0;
	GetTreeItemIndex( m_tree.GetRootItem(),m_tree.GetSelectedItem(),index );

	// Store selected item in history for this browser.
	m_selectionHistory[m_path+m_fileSpec] = index;

	// If geometry update preview.
	if (m_panelPreview)
	{
		CString file = "";
		m_itemsMap.Find( pNMTreeView->itemNew.hItem,file );
		if (!file.IsEmpty())
		{
			// Check if preview panel is expended.
			m_panelPreview->LoadFile( file );
		}
	}
	if (m_bSelectOnClick)
	{
		CString file = "";
		m_itemsMap.Find( pNMTreeView->itemNew.hItem,file );
		if (!file.IsEmpty())
		{
			if (m_selectCallback)
				m_selectCallback( file );
		}
	}
	
	*pResult = 0;
}

void CPanelTreeBrowser::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	if (!m_bDragDropEnabled)
		return;

	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	m_draggedFile = "";
	m_itemsMap.Find( hItem,m_draggedFile );
	if (m_draggedFile.IsEmpty())
		return;

	m_tree.Select( hItem,TVGN_CARET );

	m_dragImage = m_tree.CreateDragImage( hItem );
	if (m_dragImage)
	{
		m_dragImage->BeginDrag(0, CPoint(-10, -10));

		CRect rc;
		AfxGetMainWnd()->GetWindowRect( rc );
		
		CPoint p = pNMTreeView->ptDrag;
		ClientToScreen( &p );
		p.x -= rc.left;
		p.y -= rc.top;
		
		m_dragImage->DragEnter( AfxGetMainWnd(),p );
		SetCapture();
		GetIEditor()->EnableUpdate( false );
	}
	
	*pResult = 0;
}

void CPanelTreeBrowser::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect rc;
	AfxGetMainWnd()->GetWindowRect( rc );
		
	ClientToScreen( &point );
	point.x -= rc.left;
	point.y -= rc.top;

	if (m_dragImage)
	{
		m_dragImage->DragMove( point );
	}
	
	//CXTResizeDialog::OnMouseMove(nFlags, point);
}

void CPanelTreeBrowser::OnLButtonUp(UINT nFlags, CPoint point) 
{
	//CXTResizeDialog::OnLButtonUp(nFlags, point);

	if (m_dragImage)
	{
		CPoint p;
		GetCursorPos( &p );

		GetIEditor()->EnableUpdate( true );

		m_dragImage->DragLeave( AfxGetMainWnd() );
		m_dragImage->EndDrag();
		delete m_dragImage;
		m_dragImage = 0;
		ReleaseCapture();

		CViewport *pView = GetIEditor()->GetViewManager()->GetViewportAtPoint(p);
		if (pView)
		{
			// Drag and drop into one of views.
			// Start object creation.
			AcceptFile( m_draggedFile ); 
		}
	}
}

void CPanelTreeBrowser::OnClickBrowserTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CPanelTreeBrowser::OnReload()
{
	Refresh(true);
}

//////////////////////////////////////////////////////////////////////////
void CPanelTreeBrowser::SortItem( HTREEITEM hParent )
{
	TVSORTCB tvs;
	tvs.hParent = hParent;
	tvs.lpfnCompare = TreeCompareProc;
	tvs.lParam = (LPARAM)&m_tree;
	m_tree.SortChildrenCB(&tvs);

	// Look at all of the root-level items
	HTREEITEM hCurrent = m_tree.GetNextItem( hParent,TVGN_CHILD );
	while (hCurrent != NULL)
	{
		SortItem( hCurrent );
		// Try to get the next item
		hCurrent = m_tree.GetNextItem(hCurrent, TVGN_NEXT); 
	}
}

//////////////////////////////////////////////////////////////////////////
void CPanelTreeBrowser::SortTree()
{
	SortItem( TVI_ROOT );
}