// SelectFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SelectFileDlg.h"

#include <sys/stat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CString CSelectFileDlg::m_strLastPath;
CString CSelectFileDlg::m_strLastSearchFolder;

/////////////////////////////////////////////////////////////////////////////
// CSelectFileDlg dialog

CSelectFileDlg::CSelectFileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectFileDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectFileDlg)
	//}}AFX_DATA_INIT

	m_previewModelEnabled = false;
}


void CSelectFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectFileDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectFileDlg, CDialog)
	//{{AFX_MSG_MAP(CSelectFileDlg)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectFileDlg message handlers

void CSelectFileDlg::UpdatePreview(CString strFileToPreview)
{
	////////////////////////////////////////////////////////////////////////
	// Update the preview to show informations about the passed file
	////////////////////////////////////////////////////////////////////////

	char szInfoText[_MAX_PATH + 512];
	char szCompactPath[_MAX_PATH];
	struct _stat sStat;

	if (strFileToPreview.IsEmpty())
	{
		////////////////////////////////////////////////////////////////////////
		// Show empty preview windows when there is no file to
		// preview
		////////////////////////////////////////////////////////////////////////

		GetDlgItem(IDC_OBJ_DESCRIPTION)->SetWindowText("");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
	}
	else
	{
		////////////////////////////////////////////////////////////////////////
		// Preview the selected file
		////////////////////////////////////////////////////////////////////////
		Vec3 size(0,0,0);
		if (m_previewModelEnabled)
		{
			//char dir[1024];
			//GetCurrentDirectory( 1024,dir );
			//SetCurrentDirectory( GetIEditor()->GetMasterCDFolder() );
			m_previewModel.LoadFile( strFileToPreview );
			//SetCurrentDirectory( dir );
			size = m_previewModel.GetSize();
		}

		ASSERT(PathFileExists(strFileToPreview));

		// Compact the path to fit into the text field
		PathCompactPathEx(szCompactPath, strFileToPreview.GetBuffer(0), 33, NULL);

		// Obtain file informations
		VERIFY(_stat(strFileToPreview.GetBuffer(0), &sStat) == 0);

		// Format the information string
		sprintf(szInfoText, "%s\nDim: %.2f X %.2f X %.2f\nFileSize: %i KB\n\nCreated: %sModified: %sAcessed: %s", szCompactPath,
			size.x,size.y,size.z,
			sStat.st_size / 1024, ctime(&sStat.st_ctime), ctime(&sStat.st_mtime), ctime(&sStat.st_atime));

		// Display it in the static text
		GetDlgItem(IDC_OBJ_DESCRIPTION)->SetWindowText(szInfoText);

		// Enable the OK button since we now have a valid selection
		GetDlgItem(IDOK)->EnableWindow(TRUE);
	}
}

BOOL CSelectFileDlg::OnInitDialog() 
{
	////////////////////////////////////////////////////////////////////////
	// Create the tree control
	////////////////////////////////////////////////////////////////////////

	RECT rc;
	CDialog::OnInitDialog();
	PSTR pszFolder = m_strLastPath.GetBuffer(0);
	PSTR pszNextSeg = NULL;
	char szSubStr[_MAX_PATH];
	UINT iCurPos = 0, iCurSubStrPos = 0;
	HTREEITEM hItem;

	// Create the tree control
	::SetRect(&rc, 10, 10, 265, 306);
	m_cFileTree.Create(NULL, rc, this, NULL);

	// Get the root item
	hItem = m_cFileTree.GetRootItem();

	// Re-open the last parsed folder in case we still have the same search
	// folder
	if (pszFolder[0] != '\0' && 
		  m_strLastSearchFolder == m_cFileTree.GetSearchFolder() &&
			m_cFileTree.ItemHasChildren(hItem))
	{
		// Get the first child
		hItem = m_cFileTree.GetChildItem(hItem);
		ASSERT(hItem);

		// Parse the entire folder string
		while (true)
		{
			// Is the current char a path separator or the null character ?
			if (pszFolder[iCurPos] == '\0')
			{
				// End of the folder string reached, the last string is anyway
				// the filename, throw it away
				break;
			}
			else if (pszFolder[iCurPos] == '\\')
			{
				// Terminate the substring
				szSubStr[iCurSubStrPos] = '\0';
				
				// Find and expand the specified item
				do
				{
					// Did we find our item ?
					if (m_cFileTree.GetItemText(hItem) == CString(szSubStr))
					{
						// Expand this item
						m_cFileTree.Expand(hItem, TVE_EXPAND);

						// Continue to search inside this leaf
						hItem = m_cFileTree.GetChildItem(hItem);

						break;
					}

					// Advance to the next child item
					hItem = m_cFileTree.GetNextSiblingItem(hItem);

					// Quit when we are unable to find the item specified in the path
				} while (hItem);

				// Start a new substring
				iCurSubStrPos = 0;

				// Skip the backslash
				iCurPos++;
			}

			// Add the current char to the folder substring
			szSubStr[iCurSubStrPos++] = pszFolder[iCurPos++];
		}
	}

	if (m_previewModelEnabled)
	{
		CRect rc;
		CWnd *frame = GetDlgItem( IDC_OBJ_PREVIEW );
		frame->GetClientRect( rc );
		m_previewModel.Create( this,rc );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CSelectFileDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	////////////////////////////////////////////////////////////////////////
	// If the user selected a new file or folder, show preview
	////////////////////////////////////////////////////////////////////////
	
	if (((NMHDR *) lParam)->code == TVN_SELCHANGED)
	{
		// Get the filename from the list and update the preview
		UpdatePreview(m_cFileTree.GetSelectedFile());
	}

	return CDialog::OnNotify(wParam, lParam, pResult);
}

bool CSelectFileDlg::SelectFileName(CString *pFileNameOut, CString *pRelativeFileNameOut,
									                  CString strFileSpec, CString strSearchFolder)
{
	////////////////////////////////////////////////////////////////////////
	// Show the dialog box and let the user select a filename from the
	// search folder (see FileTree.h for the meaning of the strSearchFolder
	// string). The function returns true when the uses choose a file
	////////////////////////////////////////////////////////////////////////

	int iReturn;
	char szRelativePath[_MAX_PATH];

	// Set the serach path and the file specification
	m_cFileTree.SetSearchFolder(strSearchFolder);
	m_cFileTree.SetFileSpec(strFileSpec);

	m_previewModelEnabled = false;
	if (_stricmp(PathFindExtension(strFileSpec.GetBuffer(0)), ".cid") == 0 ||
		  _stricmp(PathFindExtension(strFileSpec.GetBuffer(0)), ".cgf") == 0)
	{
		//m_previewModelEnabled = true;
	}

	// Show the dialog
	iReturn = DoModal();

	// Write the selected filename and path into the string
	if (pFileNameOut)
		*pFileNameOut = m_strSelectedFile;

	// Should we also output a relative filename ?
	if (pRelativeFileNameOut)
	{
		// Create relative path
		PathRelativePathTo(szRelativePath, GetIEditor()->GetMasterCDFolder(), 
			FILE_ATTRIBUTE_DIRECTORY, m_strSelectedFile, NULL);

		// Remove the leading backslash
		memmove(szRelativePath, &szRelativePath[1], strlen(szRelativePath));
		szRelativePath[strlen(szRelativePath)] = '\0';

		*pRelativeFileNameOut = szRelativePath;
	}

	return iReturn == IDOK;
}

void CSelectFileDlg::OnOK()
{
	////////////////////////////////////////////////////////////////////////
	// Save the name of the selected file
	////////////////////////////////////////////////////////////////////////

	// Save the full path of the selected file and the search root folder
	m_strSelectedFile = m_cFileTree.GetSelectedFile();
	m_strLastSearchFolder = m_cFileTree.GetSearchFolder();

	// Save the last opened folder
	m_strLastPath = &m_strSelectedFile.GetBuffer(0)[strlen(GetIEditor()->GetMasterCDFolder()) + 
		m_cFileTree.GetSearchFolder().GetLength() + 1];

	CDialog::OnOK();
}

void CSelectFileDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// Set current dir back to MasterCD folder.
	SetCurrentDirectory( GetIEditor()->GetMasterCDFolder() );
}
