// CustomFileDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CustomFileDialog.h"

#include <io.h>
#include <ICryPak.h>
#include <ISound.h>

// Maximal number of history entries.
#define MAX_HISTORY_ENTRIES 10

static struct FileIconsMapping
{
	char ext[8];
	int nIconID;
} s_FileIconsMapping[] = 
{
	{ "cgf",3 },
	{ "cga",4 },
	{ "jpg",5 },
	{ "dds",6 },
	{ "tga",7 },
	{ "bmp",8 },
	{ "wav",9 },
	{ "ogg",9 },
	{ "mp3",9 },
};

//////////////////////////////////////////////////////////////////////////
// CCustomFileDialog dialog
//////////////////////////////////////////////////////////////////////////
bool CCustomFileDialog::m_bPreviewOn = true;

IMPLEMENT_DYNAMIC(CCustomFileDialog, CXTResizeDialog)
CCustomFileDialog::CCustomFileDialog( OpenParams &fp,CWnd* pParent /*=NULL*/)
	: CXTResizeDialog(CCustomFileDialog::IDD, pParent),
	m_fp(fp)
{
	m_pIPak = GetIEditor()->GetSystem()->GetIPak();
	assert( m_pIPak );

	m_currentFilter = 0;
	m_rcPreview.SetRect(0,0,2,2);

	m_pSound = NULL;

	//////////////////////////////////////////////////////////////////////////
	//[Timur] @FIXME remove it.
	//m_fp.filetype = EFILE_TYPE_TEXTURE;
	//m_fp.initialDir = "";
	//////////////////////////////////////////////////////////////////////////
	m_fp.initialDir.Replace( '/','\\' );
	m_fp.initialFile.Replace( '/','\\' );
	m_fp.initialDir.Replace( '/','\\' );
	m_fp.initialFile.Replace( '/','\\' );

	if (m_fp.filetype == EFILE_TYPE_ANY)
	{
		if (m_fp.filter.IsEmpty())
			m_fp.filter = "All Files (*.*)|*.*||";
	}
	else if (m_fp.filetype == EFILE_TYPE_GEOMETRY)
	{
		if (m_fp.filter.IsEmpty())
			m_fp.filter = "Geometry Files (*.cgf,*.cga)|*.cgf;*.cga|All Files (*.*)|*.*||";
	}
	else if (m_fp.filetype == EFILE_TYPE_TEXTURE)
	{
		if (m_fp.filter.IsEmpty())
			m_fp.filter = "Texture Files|*.dds;*.jpg;*.tga;*.pcx;*.bmp;*.gif;*.pgm;*.raw|All Files (*.*)|*.*||";
	}
	else if (m_fp.filetype == EFILE_TYPE_SOUND)
	{
		if (m_fp.filter.IsEmpty())
			m_fp.filter = "Sound Files (*.wav,*.mp3,*.ogg)|*.wav;*.mp3;*.ogg|All Files|*.*||";
	}

	//////////////////////////////////////////////////////////////////////////
	// Fill shortcuts.
	//////////////////////////////////////////////////////////////////////////
	m_shortcuts[EFILE_TYPE_ANY].push_back( ShortcutInfo("Root","",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_ANY].push_back( ShortcutInfo("Objects","Objects",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_ANY].push_back( ShortcutInfo("Textures","Textures",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_ANY].push_back( ShortcutInfo("Scripts","Scripts",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_ANY].push_back( ShortcutInfo("Sound","Sounds",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_ANY].push_back( ShortcutInfo("Music","Music",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_ANY].push_back( ShortcutInfo("Shaders","Shaders",IDI_FOLDER) );

	m_shortcuts[EFILE_TYPE_GEOMETRY].push_back( ShortcutInfo("Objects","Objects",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_GEOMETRY].push_back( ShortcutInfo("Natural","Objects\\Natural",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_GEOMETRY].push_back( ShortcutInfo("GLMs","Objects\\GLM",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_GEOMETRY].push_back( ShortcutInfo("Outdoor","Objects\\Outdoor",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_GEOMETRY].push_back( ShortcutInfo("Characters","Objects\\Characters",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_GEOMETRY].push_back( ShortcutInfo("Weapons","Objects\\Weapons",IDI_FOLDER) );

	m_shortcuts[EFILE_TYPE_TEXTURE].push_back( ShortcutInfo("Textures","Textures",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_TEXTURE].push_back( ShortcutInfo("Terrain","Textures\\Terrain",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_TEXTURE].push_back( ShortcutInfo("GLM","Textures\\glm",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_TEXTURE].push_back( ShortcutInfo("Detail","Textures\\Detail",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_TEXTURE].push_back( ShortcutInfo("Animated","Textures\\Animated",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_TEXTURE].push_back( ShortcutInfo("Decal","Textures\\Decal",IDI_FOLDER) );

	m_shortcuts[EFILE_TYPE_SOUND].push_back( ShortcutInfo("Sounds","Sounds",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_SOUND].push_back( ShortcutInfo("Music","Music",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_SOUND].push_back( ShortcutInfo("Sounds","Sounds",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_SOUND].push_back( ShortcutInfo("Weapons","Sounds\\Weapons",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_SOUND].push_back( ShortcutInfo("Vehicles","Sounds\\Vechicle",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_SOUND].push_back( ShortcutInfo("Player","Sounds\\Player",IDI_FOLDER) );
	m_shortcuts[EFILE_TYPE_SOUND].push_back( ShortcutInfo("AI","Sounds\\ai",IDI_FOLDER) );	

	//////////////////////////////////////////////////////////////////////////
	// Fill icons mapping.
	for (int i = 0; i < sizeof(s_FileIconsMapping)/sizeof(s_FileIconsMapping[0]); i++)
	{
		m_extToIcons[s_FileIconsMapping[i].ext] = s_FileIconsMapping[i].nIconID;
	}
}

CCustomFileDialog::~CCustomFileDialog()
{
	if (m_previewBitmap.m_hObject)
		m_previewBitmap.DeleteObject();

	if (m_pSound)
		m_pSound->Stop();
}

void CCustomFileDialog::DoDataExchange(CDataExchange* pDX)
{
	CXTResizeDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HISTORY, m_historyCtrl);
	DDX_Control(pDX, IDC_LOOKIN, m_lookinCtrl);
	DDX_Control(pDX, IDC_FILELIST, m_fileListCtrl);
	DDX_Control(pDX, IDC_SHORTCUTS, m_shortcutsCtrl);
	DDX_Control(pDX, IDC_FILENAME, m_filenameCtrl);
	DDX_Control(pDX, IDC_FILESTYPE, m_filesTypeCtrl);
	DDX_Control(pDX, IDC_BACK, m_btnBack);
	DDX_Control(pDX, IDC_UP, m_btnUp);
	DDX_Control(pDX, IDC_FILEINFO, m_fileInfoCtrl);
	DDX_Control(pDX, IDC_PREVIEW_ON, m_previewOn);
}


BEGIN_MESSAGE_MAP(CCustomFileDialog, CXTResizeDialog)
	ON_BN_CLICKED(IDC_BACK, OnBnClickedBack)
	ON_NOTIFY(NM_DBLCLK, IDC_FILELIST, OnNMDblclkFilelist)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FILELIST, OnLvnItemChangedFilelist)
	ON_BN_CLICKED(IDC_UP, OnBnClickedUp)
	ON_CBN_SELENDOK(IDC_HISTORY, OnCbnSelendokHistory)
	ON_CBN_SELENDOK(IDC_FILESTYPE, OnCbnSelendokFilestype)
	ON_CBN_SELENDOK(IDC_FILENAME, OnCbnSelendokFilename)
	ON_NOTIFY(LVN_KEYDOWN, IDC_FILELIST, OnLvnKeydownFilelist)
	ON_CBN_SELENDOK(IDC_LOOKIN, OnCbnSelendokLookin)
	ON_NOTIFY(CBEN_ENDEDIT, IDC_FILENAME, OnCbenEndeditFilename)
	ON_MESSAGE( XTWM_OUTBAR_NOTIFY,  OnOutbarNotify )
	ON_BN_CLICKED(IDC_PREVIEW_ON, OnBnClickedPreviewOn)
END_MESSAGE_MAP()


// CCustomFileDialog message handlers

BOOL CCustomFileDialog::OnInitDialog()
{
	CXTResizeDialog::OnInitDialog();
	
	m_btnBack.SetIcon( MAKEINTRESOURCE(IDI_FILEOPEN_BACK) );
	m_btnUp.SetIcon( MAKEINTRESOURCE(IDI_FILEOPEN_UP) );
	
	//////////////////////////////////////////////////////////////////////////
	// Init FileList Control.
	//m_imageListFiles.Create(IDB_FILES_IMAGE, 16, 1, RGB (255, 0, 255));
/*
	// Create the bitmap associated with image list, this
	// is done so we can use high color while maintaining
	// background transparency...
	HBITMAP hBmp = (HBITMAP)::LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_FILES_IMAGE),
		IMAGE_BITMAP, 0, 0, 0); 

	// Create the image list from the HBITMAP handle we just
	// created...
	m_imageListFiles.Create(16, 16, ILC_COLOR32|ILC_MASK, 8, 1);
	m_imageListFiles.Add(CBitmap::FromHandle(hBmp), RGB(255, 0, 255));
	m_imageListFiles.SetOverlayImage( 1,1 );

	if (hBmp != NULL)
		::DeleteObject( hBmp );
*/
	CMFCUtils::LoadTrueColorImageList( m_imageListFiles,IDB_FILES_IMAGE,16,RGB(255,0,255) );
	m_imageListFiles.SetOverlayImage( 1,1 );

	
	//////////////////////////////////////////////////////////////////////////
	// Init FileList Control.
	m_fileListCtrl.SetImageList( &m_imageListFiles,LVSIL_SMALL );
	// Associate the image list with the combo box
	m_lookinCtrl.SetImageList(&m_imageListFiles);

	if (m_fp.bMultiSelection)
	{
		// Remove
		m_fileListCtrl.ModifyStyle( LVS_SINGLESEL,0 );
	}
	else
	{
		m_fileListCtrl.ModifyStyle( 0,LVS_SINGLESEL );
	}

	//////////////////////////////////////////////////////////////////////////
	// Init directory.
	//////////////////////////////////////////////////////////////////////////
	LoadHistory();
	ParseFilter( m_fp.filter );

	if (m_fp.initialDir.IsEmpty())
	{
		// Take last directory from history.
		if (m_historyCtrl.GetCount() > 0)
		{
			m_historyCtrl.GetLBText(0,m_fp.initialDir);
		}
	}

	if (m_fp.initialFile.IsEmpty())
		SetCurrentDir( m_fp.initialDir,false );

	InitShortcuts();

	m_previewOn.SetCheck( (m_bPreviewOn)?BST_CHECKED:BST_UNCHECKED );

	//////////////////////////////////////////////////////////////////////////
	// Init model preview control.
	//////////////////////////////////////////////////////////////////////////
	if (m_bPreviewOn)
	{
		if (m_fp.filetype == EFILE_TYPE_GEOMETRY)
		{
			m_previewCtrl.SubclassDlgItem( IDC_PREVIEW,this );
			m_previewCtrl.ModifyStyle( SS_BITMAP,SS_OWNERDRAW );
			m_previewCtrl.ShowWindow(SW_SHOW);
			m_previewCtrl.SetRotation(true);
		}
		else if (m_fp.filetype == EFILE_TYPE_TEXTURE)
		{
			m_previewImageCtrl.SubclassDlgItem( IDC_PREVIEW,this );
			m_previewImageCtrl.ModifyStyle( SS_OWNERDRAW|SS_GRAYFRAME,SS_BITMAP );
			m_previewImageCtrl.GetClientRect(m_rcPreview);
			m_previewImageCtrl.SetBitmap( NULL );
			m_previewImageCtrl.ShowWindow(SW_SHOW);
		}
		else if (m_fp.filetype == EFILE_TYPE_SOUND)
		{
			m_previewOn.SetWindowText( "Play Sound" );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Select initial file.
	if (!m_fp.initialFile.IsEmpty())
		SetCurrentFile( m_fp.initialFile );

	//////////////////////////////////////////////////////////////////////////
	// Make dialog resizable.
	//////////////////////////////////////////////////////////////////////////

	SetResize( IDOK,SZ_REPOS(1) );
	SetResize( IDCANCEL,SZ_REPOS(1) );

	SetResize( IDC_FILELIST,SZ_RESIZE(1) );
	SetResize( IDC_SHORTCUTS,SZ_VERRESIZE(1) );
	
	SetResize( IDC_FILENAME,CXTResizeRect(0,1,1,1) );
	SetResize( IDC_FILESTYPE,CXTResizeRect(0,1,1,1) );

	SetResize( IDC_PREVIEW,SZ_VERREPOS(1) );
	SetResize( IDC_FILEINFO,SZ_VERREPOS(1) );
	SetResize( IDC_PREVIEW_ON,SZ_VERREPOS(1) );

	SetResize( IDC_STATIC_FILENAME,SZ_VERREPOS(1) );
	SetResize( IDC_STATIC_FILESYPE,SZ_VERREPOS(1) );

	SetResize( IDC_HISTORY,SZ_HORRESIZE(1) );
	SetResize( IDC_LOOKIN,SZ_HORRESIZE(1) );
	SetResize( IDC_BACK,SZ_HORREPOS(1) );
	SetResize( IDC_UP,SZ_HORREPOS(1) );

	AutoLoadPlacement( "Dialogs\\FileOpen" );

	m_fileListCtrl.SetFocus();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CCustomFileDialog::OnOK()
{
	// Add this path to history, remove it is already present somewhere.
	for (int i = 0; i < m_historyCtrl.GetCount(); i++)
	{
		CString text;
		m_historyCtrl.GetLBText(i,text);
		if (stricmp(text,m_currentDir) == 0)
		{
			m_historyCtrl.DeleteString(i);
			break;
		}
	}
	m_historyCtrl.InsertString( 0,m_currentDir );
	if (m_historyCtrl.GetCount() > MAX_HISTORY_ENTRIES)
	{
		// Delete less recent history item if max entries overflow.
		m_historyCtrl.DeleteString( m_historyCtrl.GetCount()-1 );
	}
	SaveHistory();

	// Fill selected files.
	m_selectedFiles.clear();
	if (m_fp.bMultiSelection)
	{
		POSITION pos = m_fileListCtrl.GetFirstSelectedItemPosition();
		while (pos != NULL)
		{
			CString dir = Path::AddBackslash(m_currentDir);
			int nItem = m_fileListCtrl.GetNextSelectedItem(pos);
			FileInfo &file = m_files[nItem];
			// Check if file ot directory.
			if (!(file.attrib & _A_SUBDIR))
			{
				m_selectedFiles.push_back( dir+file.filename );
			}
		}
	}
	else
	{
		m_selectedFiles.push_back( GetFilePath() );
	}

	CXTResizeDialog::OnOK();
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::OnCancel()
{
	m_selectedFiles.clear();
	m_currentFile = "";
	m_currentDir = "";

	CXTResizeDialog::OnCancel();
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::OnBnClickedBack()
{
	if (!m_directoryStack.empty())
	{
		CString prevDir = m_directoryStack.back();
		m_directoryStack.pop_back();
		SetCurrentDir( prevDir,false );
	}
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::OnBnClickedUp()
{
	if (m_currentDir.IsEmpty())
		return;
	// Move level up from current dir.
	CString dir = m_currentDir;
	int i = dir.GetLength()-1;
	const char *sdir = dir;
	while (i >= 0 && sdir[i] != '\\' && sdir[i] != '/')
	{
		i--;
	}
	if (i > 0)
    dir = dir.Mid( 0,i );
	else
		dir = "";
	
	SetCurrentDir( Path::RemoveBackslash(dir) );
}


//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::OnNMDblclkFilelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	NMITEMACTIVATE *pActivate = (NMITEMACTIVATE*)pNMHDR;
	ActivateItem( pActivate->iItem );
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::OnLvnItemChangedFilelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMIA = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
	*pResult = 0;
  m_currentFile = "";
	int item = pNMIA->iItem;

	if (!(pNMIA->uChanged & LVIF_STATE))
		return;

	if (!(pNMIA->uNewState & LVIS_SELECTED) && (pNMIA->uOldState & LVIS_SELECTED))
	{
		if (m_fileListCtrl.GetSelectedCount() < 1)
		{
			// If item deselected, turn off preview.
			PreviewOff();
		}
	}

	if (!(pNMIA->uNewState & LVIS_FOCUSED))
		return;

	if (item >= 0 && item < m_files.size())
	{
		if (!(m_files[item].attrib & _A_SUBDIR))
		{
			m_currentFile = m_files[item].filename;
		}
	}
	m_filenameCtrl.SetWindowText( m_currentFile );
	PreviewItem( item );
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::AddFilteredFiles( const CString &searchFilter )
{
	_finddata_t fd;
	intptr_t fhandle;
	fhandle = m_pIPak->FindFirst( searchFilter,&fd );
	if (fhandle != -1)
	{
		do {
			// Skip back folders.
			if (fd.attrib & _A_SUBDIR) // skip if directory.
				continue;
			// Add file description to current directory listing.
			FileInfo file;
			file.attrib = fd.attrib;
			file.filename = fd.name;
			file.size = fd.size;
			file.time_access = fd.time_access;
			file.time_create = fd.time_create;
			file.time_write = fd.time_write;
			m_files.push_back( file );
		} while (m_pIPak->FindNext( fhandle,&fd ) == 0);
		m_pIPak->FindClose(fhandle);
	}
}

//////////////////////////////////////////////////////////////////////////
bool CCustomFileDialog::FileInfoCompare( const CCustomFileDialog::FileInfo &f1,const CCustomFileDialog::FileInfo &f2 )
{
	if ((f1.attrib & _A_SUBDIR) && !(f2.attrib & _A_SUBDIR))
		return true;
	if (!(f1.attrib & _A_SUBDIR) && (f2.attrib & _A_SUBDIR))
		return false;

	return stricmp(f1.filename,f2.filename) < 0;
}

//////////////////////////////////////////////////////////////////////////
bool CCustomFileDialog::ScanDirectory( const CString &dir )
{
	m_files.clear();

	// Get current filter.
	FileFilter &filter = m_fileFilters[m_currentFilter];

	CString searchPath = Path::AddBackslash(dir) + "*.*";

	// Add all directories.
	_finddata_t fd;
	intptr_t fhandle;
	
	fhandle = m_pIPak->FindFirst( searchPath,&fd );
	if (fhandle != -1)
	{
		do {
			// Skip back folders.
			if (fd.name[0] == '.')
				continue;
			if (!(fd.attrib & _A_SUBDIR)) // skip if not directory.
				continue;
			// Add file description to current directory listing.
			FileInfo file;
			file.attrib = fd.attrib;
			file.filename = fd.name;
			file.size = fd.size;
			file.time_access = fd.time_access;
			file.time_create = fd.time_create;
			file.time_write = fd.time_write;
			m_files.push_back( file );
		} while (m_pIPak->FindNext( fhandle,&fd ) == 0);
		m_pIPak->FindClose(fhandle);
	}

	if (!m_overrideFilter.IsEmpty())
	{
		CString fpath,fname,fext;
		Path::Split( m_overrideFilter,fpath,fname,fext );
		if (fname.IsEmpty())
			fname = "*";
		if (fext.IsEmpty())
			fext = "*";
		searchPath = Path::Make(Path::AddBackslash(dir)+fpath,fname,fext);
		AddFilteredFiles( searchPath );
	}
	else
	{
		// Get files for each filter.
		for (int i = 0; i < filter.filters.size(); i++)
		{
			searchPath = Path::AddBackslash(dir) + filter.filters[i];
			AddFilteredFiles( searchPath );
		}
	}

	// Sort files.
	std::sort( m_files.begin(),m_files.end(),FileInfoCompare );

	if (m_files.empty())
		return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::FillFiles()
{
	// Clear list.
	m_fileListCtrl.DeleteAllItems();

	int numfiles = m_files.size();

	for (int i = 0; i < numfiles; i++)
	{
		FileInfo &file = m_files[i];

		UINT nImage = 2;
		if (file.attrib & _A_SUBDIR)
		{
			// Subdirectory.
			if (file.attrib & _A_IN_CRYPAK)
				nImage = 1; // in Pak.
			else
				nImage = 0; // Normal directory.
		}
		else
		{
			// Find icon id associated with this extension.
			CString ext = Path::GetExt(file.filename);
			ext.MakeLower();
			nImage = stl::find_in_map( m_extToIcons,ext,nImage );
		}

		int nItem = m_fileListCtrl.InsertItem( i,file.filename,nImage );
		// In Pak files have differnt color.
		if (file.attrib & _A_IN_CRYPAK)
		{
			m_fileListCtrl.SetItemState( nItem,INDEXTOOVERLAYMASK(1),LVIS_OVERLAYMASK );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::SetCurrentDir( const CString &dir,bool bRememberPrevious )
{
	if (bRememberPrevious && dir != m_currentDir)
	{
		m_directoryStack.push_back( m_currentDir );
	}

	m_currentDir = Path::RemoveBackslash(dir);
	ScanDirectory( m_currentDir );
	FillFiles();

	FillLookinControl();
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::SetCurrentFile( const CString &curFile )
{
	// Find file and select it.
	CString path = Path::GetPath(curFile);
	CString file = Path::GetFile(curFile);
	SetCurrentDir( path );
	// find file in current directory.

	for (int i = 0; i < m_files.size(); i++)
	{
		if (stricmp(m_files[i].filename,file) == 0)
		{
			// File found, select it.
			SelectItem( i );
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::SelectItem( int item )
{
	m_fileListCtrl.SetItemState( item,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED );
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::RefreshDirectory()
{
	SetCurrentDir( m_currentDir,false );
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::ParseFilter( const CString &inputFilter )
{
	m_fileFilters.clear();

	int split = 0;
	CString filter = inputFilter;
	
	CString splitToken = TokenizeString(filter,"|",split);
	while (splitToken != "")
	{
		FileFilter flt;
		flt.description = splitToken;
		
		splitToken = TokenizeString( filter,"|",split);
		if (splitToken == "")
			break;

		// Parse 
		CString filters = splitToken;

		int pos = 0;
		CString resToken = TokenizeString( filters,";",pos);
		while (resToken != "")
		{
			flt.filters.push_back( resToken );
			resToken = TokenizeString( filters,";",pos);
		};

		m_fileFilters.push_back( flt );

		splitToken = TokenizeString( filter,"|",split);
	}


	for (int i = 0; i < m_fileFilters.size(); i++)
	{
		m_filesTypeCtrl.AddString( m_fileFilters[i].description );
	}
	// Select first filter.
	m_filesTypeCtrl.SetCurSel(0);
	m_currentFilter = 0;
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::ActivateItem( int item )
{
	// Find item.
	if (item < 0 || item >= m_files.size())
		return;

	FileInfo &file = m_files[item];
	m_currentFile = file.filename;
	m_filenameCtrl.SetWindowText( m_currentFile );

	// Check if file ot directory.
	if (file.attrib & _A_SUBDIR)
	{
		// Go to this directory.
		SetCurrentDir( Path::AddBackslash(m_currentDir) + file.filename );
	}
	else
	{
		// Select this file.
		OnOK();
	}
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::PreviewOff()
{
	m_fileInfoCtrl.SetWindowText( "" );
	if (m_pSound)
		m_pSound->Stop();
	m_pSound = 0;
	if (m_bPreviewOn)
	{
		if (m_previewCtrl.m_hWnd)
		{
			m_previewCtrl.ReleaseObject();
		}
		if (m_previewImageCtrl.m_hWnd)
		{
			m_previewImageCtrl.SetBitmap( NULL );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::PreviewItem( int item )
{
	CString filename = GetFilePath();

	bool bSubDir = false;

	CString fileinfo;
	if (item >= 0 || item < m_files.size())
	{
		FileInfo &file = m_files[item];
		
		if (file.attrib & _A_SUBDIR)
			bSubDir = true;
		else
		{
			fileinfo.Format( "%s\r\nFile Size: %dK",(const char*)file.filename,file.size/1024 );
		}
	}

	if (m_bPreviewOn)
	{
		// Find item.
		if (m_fp.filetype == EFILE_TYPE_GEOMETRY)
		{
			if (m_previewCtrl.m_hWnd)
			{
				if (!bSubDir)
				{
					m_previewCtrl.LoadFile( filename );
				}
				else
					m_previewCtrl.ReleaseObject();
			}
		}
		else if (m_fp.filetype == EFILE_TYPE_TEXTURE)
		{
			if (m_previewImageCtrl.m_hWnd)
			{
				CImage image;
				if (!bSubDir && CImageUtil::LoadImage( filename,image ))
				{
					CString imginfo;
					imginfo.Format( "\r\n%dx%d",image.GetWidth(),image.GetHeight() );
					fileinfo += imginfo;

					CImage scaledImage;
					scaledImage.Allocate( m_rcPreview.Width(),m_rcPreview.Height() );
					CImageUtil::ScaleToFit( image,scaledImage );

					scaledImage.SwapRedAndBlue();

					if (m_previewBitmap.m_hObject)
						m_previewBitmap.DeleteObject();
					m_previewBitmap.CreateBitmap( scaledImage.GetWidth(),scaledImage.GetHeight(), 1, 32, scaledImage.GetData() );
					//m_bmpFinalTexPrev.SetBitmapBits( preview.GetSize(),(DWORD*)preview.GetData() );
					m_previewImageCtrl.SetBitmap( m_previewBitmap );
				}
				else
				{
					m_previewImageCtrl.SetBitmap( NULL );
				}
			}
		}
		else if (m_fp.filetype == EFILE_TYPE_SOUND)
		{
			// Release previous sound.
			if (!bSubDir)
			{
				// Play preview sound.
				ISoundSystem *pSoundSystem = GetIEditor()->GetSystem()->GetISoundSystem();
				if (pSoundSystem)
				{
					m_pSound = pSoundSystem->LoadSound( filename,FLAG_SOUND_2D|FLAG_SOUND_STEREO|FLAG_SOUND_16BITS|FLAG_SOUND_LOAD_SYNCHRONOUSLY );
					if (m_pSound)
						m_pSound->Play();
				}
			}
		}
	}

	m_fileInfoCtrl.SetWindowText( fileinfo );
}

//////////////////////////////////////////////////////////////////////////
CString CCustomFileDialog::GetFileName() const
{
	return m_currentFile;
}

//////////////////////////////////////////////////////////////////////////
CString CCustomFileDialog::GetFilePath() const
{
	return Path::AddBackslash(m_currentDir) + m_currentFile;
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::LoadHistory()
{
	CString section,key,value;
	section.Format( "Dialogs\\FileOpen\\History%d",(int)m_fp.filetype );

	int i;
	// Save history items to registry.
	for (i = 0; i < MAX_HISTORY_ENTRIES; i++)
	{
		key.Format( "%d",i );
		value = AfxGetApp()->GetProfileString( section,key );
		if (!value.IsEmpty())
		{
			m_historyCtrl.AddString( value );
		}
	}
	m_historyCtrl.SetCurSel(0);

	m_filenameCtrl.ResetContent();
	section = "Dialogs\\FileOpen";
	for (i = 0; i < MAX_HISTORY_ENTRIES; i++)
	{
		key.Format( "File%d",i );
		value = AfxGetApp()->GetProfileString( section,key );
		if (!value.IsEmpty())
			m_filenameCtrl.InsertItem( i,value );
	}
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::SaveHistory()
{
	CString section,key,value;
	section.Format( "Dialogs\\FileOpen\\History%d",(int)m_fp.filetype );

	int i;
	// Save history items to registry.
	for (i = 0; i < m_historyCtrl.GetCount(); i++)
	{
		 m_historyCtrl.GetLBText(i,value);
		key.Format( "%d",i );
		AfxGetApp()->WriteProfileString( section,key,value );
	}

	section = "Dialogs\\FileOpen";
	for (i = 0; i < m_filenameCtrl.GetCount(); i++)
	{
		m_filenameCtrl.GetLBText( i,value );
		key.Format( "File%d",i );
		AfxGetApp()->WriteProfileString( section,key,value );
	}
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::OnCbnSelendokHistory()
{
	// TODO: Add your control notification handler code here
	int nIndex = m_historyCtrl.GetCurSel();
	if (nIndex != CB_ERR)
	{
		CString str;
		m_historyCtrl.GetLBText( nIndex,str );
		SetCurrentDir( str );
	}
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::OnCbnSelendokFilestype()
{
	int nIndex = m_filesTypeCtrl.GetCurSel();
	if (nIndex != CB_ERR)
	{
		m_currentFilter = nIndex;
		// Refresh directory.
		RefreshDirectory();
	}
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::OnCbnSelendokFilename()
{
	int nIndex = m_filenameCtrl.GetCurSel();
	if (nIndex != CB_ERR)
	{
		CString text;
		m_filenameCtrl.GetLBText(nIndex,text);
		m_overrideFilter = text;
		RefreshDirectory();
	}
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::OnLvnKeydownFilelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

	int wKey = pLVKeyDow->wVKey;

	*pResult = 0;

	// Refresh directory if F5 is pressed.
	if (wKey == VK_F5)
	{
		RefreshDirectory();
		*pResult = 1;
	}
	else if (wKey == VK_RETURN)
	{
		*pResult = 1;
		if (m_fileListCtrl.GetSelectedCount() < 2)
		{

			// Activate item.
			POSITION pos = m_fileListCtrl.GetFirstSelectedItemPosition();
			if (pos != NULL)
			{
				int nItem = m_fileListCtrl.GetNextSelectedItem(pos);
				ActivateItem( nItem );
			}
		}
		else
		{
			OnOK();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::FillLookinControl()
{
	m_lookinCtrl.ResetContent();

	int iIndex = 0;

	char szCurrDir[_MAX_PATH];
	GetCurrentDirectory( sizeof(szCurrDir),szCurrDir );
	// Add root folder.
	m_lookinCtrl.InsertItem( iIndex,szCurrDir,iIndex,0,0 );
	iIndex++;

	// Add current directories to lookin control.
	CString dir = m_currentDir;
	dir.Replace( '/','\\' );
	while (!dir.IsEmpty())
	{
		int pos = dir.Find( '\\' );
		CString subdir;
		if (pos >= 0)
			subdir = dir.Mid(0,pos);
		else
			subdir = dir;

		m_lookinCtrl.InsertItem( iIndex,subdir,iIndex+1,0,0 );
		iIndex++;

		if (pos < 0)
			break;
		dir = dir.Mid(pos+1);
	}

	// Select last item.
	m_lookinCtrl.SetCurSel( m_lookinCtrl.GetCount()-1 );
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::OnCbnSelendokLookin()
{
	int nIndex = m_lookinCtrl.GetCurSel();
	if (nIndex != CB_ERR)
	{
		if (nIndex == 0)
		{
			// Go to root.
			SetCurrentDir( "" );
		}
		else
		{
			// Need to take first nIndex subdirs from path.
			CString dir = m_currentDir;
			dir.Replace( '/','\\' );
			int pos = 0;
			for (int i = 0; i < nIndex; i++)
			{
				pos = dir.Find( '\\',pos+1 );
				if (pos < 0)
					break;
			}
			if (pos > 0)
			{
				dir = dir.Mid(0,pos);
				SetCurrentDir( dir );
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::InitShortcuts()
{
	Shortcuts *pShortcuts = &m_shortcuts[m_fp.filetype];
	for (int i = 0; i < pShortcuts->size(); i++)
	{
		ShortcutInfo &shortcut = (*pShortcuts)[i];
		m_shortcutsCtrl.AddMenuItem( shortcut.nIconID,shortcut.name );
	}

	//m_shortcutsCtrl.SetBkColor( RGB(100,100,100) );
	m_shortcutsCtrl.SetColors( RGB(0xff,0xff,0xff), RGB(100,100,100));
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::OnCbenEndeditFilename(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMCBEENDEDIT *pEndEdit = (NMCBEENDEDIT*)pNMHDR;
	if (pEndEdit->iWhy == CBENF_RETURN || pEndEdit->iWhy == CBENF_KILLFOCUS)
	{
		CString text = pEndEdit->szText;
		m_overrideFilter = text;
		RefreshDirectory();

		CString lbtext;
		// Insert item to filenames combo box.
		for (int i = 0; i < m_filenameCtrl.GetCount(); i++)
		{
			m_filenameCtrl.GetLBText( i,lbtext );
			if (stricmp(text,lbtext) == 0)
			{
				m_filenameCtrl.DeleteItem(i);
				break;
			}
		}
		m_filenameCtrl.InsertItem( 0,text );
		if (m_filenameCtrl.GetCount() > MAX_HISTORY_ENTRIES)
		{
			// Delete last item.
			m_filenameCtrl.DeleteItem(m_filenameCtrl.GetCount()-1);
		}
	}

	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
LRESULT CCustomFileDialog::OnOutbarNotify( WPARAM lParam, LPARAM wParam)
{
	if ((int)lParam == -1) // -1 means no selection 
		return 0;

	switch( wParam ) // control id.
	{
	case IDC_SHORTCUTS:
		{
			// Get the menu item.
			XT_CONTENT_ITEM* pContentItems = m_shortcutsCtrl.GetMenuItem((int)lParam);
			ASSERT(pContentItems);

			int nIndex = pContentItems->m_nIndex;
			Shortcuts *pShortcuts = &m_shortcuts[m_fp.filetype];
			if (nIndex >= 0 && nIndex < pShortcuts->size())
			{
				ShortcutInfo &shortcut = (*pShortcuts)[nIndex];
				SetCurrentDir( shortcut.directory );
			}
		}
		break;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// CFilesListCtrl
//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP( CFilesListCtrl,CListCtrl )
	ON_WM_GETDLGCODE()
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
BOOL CFilesListCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			// Send LVN_KEYDOWN message with Return.
			NMLVKEYDOWN notify;
			notify.hdr.code = LVN_KEYDOWN;
			notify.hdr.hwndFrom = m_hWnd;
			notify.hdr.idFrom = GetDlgCtrlID();
			notify.flags = 0;
			notify.wVKey = VK_RETURN;

			GetParent()->SendMessage( WM_NOTIFY,(WPARAM)GetDlgCtrlID(),(LPARAM)(&notify) );
			return TRUE;
		}
	}
	return CListCtrl::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////
UINT CFilesListCtrl::OnGetDlgCode()
{
	return DLGC_WANTMESSAGE;
}

//////////////////////////////////////////////////////////////////////////
void CCustomFileDialog::OnBnClickedPreviewOn()
{
	bool bPreviewOn =  m_previewOn.GetCheck() == BST_CHECKED;
	if (!bPreviewOn)
	{
		PreviewOff();
		m_bPreviewOn = bPreviewOn;
	}
	else
	{
		m_bPreviewOn = bPreviewOn;
		// Show preview.
		if (m_fileListCtrl.GetSelectedCount() == 1)
		{
			POSITION pos = m_fileListCtrl.GetFirstSelectedItemPosition();
			while (pos != NULL)
			{
				int nItem = m_fileListCtrl.GetNextSelectedItem(pos);
				PreviewItem( nItem );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
int CCustomFileDialog::GetSelectedCount() const
{
	return m_selectedFiles.size();
}

//////////////////////////////////////////////////////////////////////////
CString CCustomFileDialog::GetSelectedFile( int nIndex ) const
{
	if (nIndex >= 0 && nIndex < m_selectedFiles.size())
	{
		return m_selectedFiles[nIndex];
	}
	return "";
}