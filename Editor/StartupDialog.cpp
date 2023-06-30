// StartupDialog.cpp : implementation file
//

#include "stdafx.h"
#include "StartupDialog.h"
#include "GameExporter.h"
#include "StringDlg.h"
#include "CryEdit.h"
#include "CryEditDoc.h"
#include "SelectFileDlg.h"

#include <afxpriv.h>
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStartupDialog dialog


CStartupDialog::CStartupDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CStartupDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStartupDialog)
	m_optSelection = 0;
	m_strMapFileName = _T("");
	m_strDirName = _T("Untitled");
	//}}AFX_DATA_INIT
}


void CStartupDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStartupDialog)
	DDX_Control(pDX, IDC_BASE, m_cUseAsBase);
	DDX_Control(pDX, IDC_RECENT_MAPS, m_lstRecentMaps);
	DDX_Radio(pDX, IDC_CREATE_NEW, m_optSelection);
	DDX_Text(pDX, IDC_MAP_FILENAME, m_strMapFileName);
	DDX_Text(pDX, IDC_DIR_NAME, m_strDirName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStartupDialog, CDialog)
	//{{AFX_MSG_MAP(CStartupDialog)
	ON_BN_CLICKED(IDC_BROWSE_FOR_MAP, OnBrowseForMap)
	ON_BN_CLICKED(IDC_CHANGE_LEV_DIR, OnChangeLevDir)
	ON_LBN_SELCHANGE(IDC_RECENT_MAPS, OnSelectedRecentDoc)
	ON_LBN_DBLCLK(IDC_RECENT_MAPS, OnDblClickRecentDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStartupDialog message handlers

void CStartupDialog::OnOK() 
{
	//////////////////////////////////////////////////////////////////////
	// Perform the action selected by the user
	//////////////////////////////////////////////////////////////////////

	UpdateData(TRUE);
	CString strSelection;
	char szLevelRoot[_MAX_PATH];
	char szFileName[_MAX_PATH];
	int nResult;
	SNoiseParams sParam;
	CFile cFile;
	CString strFilePath, strTempCopyStr;
	CBitmap bmpLoad;
	bool bReturn;
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;
	DWORD *pImageData;
	FILE *hFile;

	BeginWaitCursor();

	switch (m_optSelection)
	{
	case 0:
		//////////////////////////////////////////////////////////////////////
		// Create a new map
		//////////////////////////////////////////////////////////////////////

		CLogFile::WriteLine("User choose to create new map");

		// Ask first
		if (GetIEditor()->IsModified())
		{
			// Ask the user about saving / aborting
			nResult = MessageBox("Save changes to document ?",
				"CryEngine Sandbox", MB_ICONQUESTION | MB_YESNOCANCEL | MB_APPLMODAL);
		
			// Evaluate the user's choice
			switch (nResult)
			{
			case IDCANCEL:
				// Abort saving
				return;
				break;

			case IDYES:
				// Save first
				GetIEditor()->GetDocument()->DoSave(szFileName, TRUE);
				break;

			case IDNO:
				// Don't save, go on
				break;

			default:
				ASSERT(FALSE);
				break;
			}
		}

		// Construct the directory name
		sprintf(szLevelRoot, "%sLevels\\%s", GetIEditor()->GetMasterCDFolder(),
			m_strDirName.GetBuffer(1));

		// Does the directory already exist ?
		if (PathFileExists(szLevelRoot))
		{
			AfxMessageBox("Directory / level aready exists, choose another name !");
			return;
		}

		// Create the directory
		CLogFile::WriteLine("Creating level directory");
		CFileUtil::CreateDirectory( szLevelRoot );

		// Should we create the new map based on the current ?
		if (m_cUseAsBase.GetCheck() == 0)
		{
			CLogFile::WriteLine("User doesn not want to use current document as base, throw it away");

			// No, throw away the current map by creating a new empty document
			if (!GetIEditor()->GetDocument()->OnNewDocument())
				return;

			// We now erased the previous settings, ask the user if we should load / 
			// generate some default ones to provide him a better starting point
			nResult = MessageBox("Do you want to create a default level as starting point ? This is highly" \
				"  recommended if you are using the editor for the first time or if you want to quickly create a new map.",
				"Create default level ?", MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL);

			if (nResult == IDYES)
			{
				CLogFile::WriteLine("Creating default level");

				// Load or generate a heightmap
				strFilePath = GetIEditor()->GetDocument()->GetMasterCDFolder() + "EditorTemplate\\Default.hm";
				if (PathFileExists(strFilePath.GetBuffer(0)))
				{
					CLogFile::WriteLine("Hightmap template file found");

					// Heightmap file exists, we can load it
					GetIEditor()->GetDocument()->m_cHeightmap.Read(strFilePath);
				}
				else
				{
					CLogFile::WriteLine("No heightmap template found, generating one");

					// We don't have a heightmap template, Generate the heightmap
					sParam.bBlueSky = false;
					sParam.bValid = true;
					sParam.fFade = 0.46f;
					sParam.fFrequency =  7.0f;
					sParam.fFrequencyStep = 2.0f;
					sParam.iCover = 0;
					sParam.iHeight = GetIEditor()->GetDocument()->m_cHeightmap.GetWidth();
					sParam.iWidth = GetIEditor()->GetDocument()->m_cHeightmap.GetHeight();
					sParam.iPasses = 8;
					sParam.iRandom = 1;
					sParam.iSharpness = 0;
					sParam.iSmoothness = 0;
					GetIEditor()->GetDocument()->m_cHeightmap.GenerateTerrain(sParam);
					GetIEditor()->GetDocument()->m_cHeightmap.MakeIsle();
				}
				
				/*
				// Load static object settings
				strFilePath = GetIEditor()->GetDocument()->GetMasterCDFolder() + "EditorTemplate\\Default.sto";
				if (PathFileExists(strFilePath.GetBuffer(0)))
				{
					CLogFile::WriteLine("Static object template found");

					// File exists, we can safely load it
					CXmlArchive ar;
					ar.Load( strFilePath );
					GetIEditor()->GetDocument()->ClearStaticObjects();
					GetIEditor()->GetDocument()->SerializeStaticObjSettings(ar);
				}
				*/

				// Load layer settings
				strFilePath = GetIEditor()->GetDocument()->GetMasterCDFolder() + "EditorTemplate\\Default.lay";
				if (PathFileExists(strFilePath.GetBuffer(0)))
				{
					CLogFile::WriteLine("Layer template found");

					// File exists, we can safely load it
					CXmlArchive ar;
					ar.Load( strFilePath );
					GetIEditor()->GetDocument()->SerializeLayerSettings(ar);
				}

				// Load detail texture
				strFilePath = GetIEditor()->GetDocument()->GetMasterCDFolder() + "EditorTemplate\\Default.bmp";
				if (PathFileExists(strFilePath.GetBuffer(0)))
				{
					CLogFile::WriteLine("Detail texture template found");

					// File exists, we can safely load it

					// Open the bitmap and extract the size
					hFile = fopen(strFilePath.GetBuffer(0), "r");
					ASSERT(hFile);
			
					// Read in the info header
					fread(&bfh, sizeof(BITMAPFILEHEADER), 1, hFile);
					fread(&bih, sizeof(BITMAPINFOHEADER), 1, hFile);
					ASSERT(bih.biWidth && bih.biHeight);

					fclose(hFile);

					// Load the bitmap
					bReturn = bmpLoad.Attach(::LoadImage(AfxGetInstanceHandle(),strFilePath.GetBuffer(0), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | 
						LR_LOADFROMFILE));
					ASSERT(bReturn);

					// Allocate memory for the image data
					pImageData = new DWORD[bih.biWidth * bih.biHeight];
					ASSERT(pImageData);

					// Get the image data from the bitmap
					bmpLoad.GetBitmapBits(bih.biWidth * bih.biHeight * sizeof(DWORD), pImageData);

					// Free the image data memory
					delete [] pImageData;
					pImageData = 0;
				}

				// Copy the skybox
				
				// Create the subfolders
				strFilePath = CString(szLevelRoot) + "\\Skybox";
				_mkdir(strFilePath.GetBuffer(0));

				// Copy the JPG files
				strFilePath = GetIEditor()->GetDocument()->GetMasterCDFolder() + "EditorTemplate\\Skybox\\12.jpg";
				if (PathFileExists(strFilePath.GetBuffer(0)))
				{
					CLogFile::WriteLine("Skybox template found");

					VERIFY(CopyFile(strFilePath.GetBuffer(0), (CString(szLevelRoot) + 
						"\\Skybox\\12.jpg").GetBuffer(0), FALSE));
				}
				strFilePath = GetIEditor()->GetDocument()->GetMasterCDFolder() + "EditorTemplate\\Skybox\\34.jpg";
				if (PathFileExists(strFilePath.GetBuffer(0)))
				{
					VERIFY(CopyFile(strFilePath.GetBuffer(0), (CString(szLevelRoot) + 
						"\\Skybox\\34.jpg").GetBuffer(0), FALSE));
				}
				strFilePath = GetIEditor()->GetDocument()->GetMasterCDFolder() + "EditorTemplate\\Skybox\\5.jpg";
				if (PathFileExists(strFilePath.GetBuffer(0)))
				{
					VERIFY(CopyFile(strFilePath.GetBuffer(0), (CString(szLevelRoot) + 
						"\\Skybox\\5.jpg").GetBuffer(0), FALSE));
				} 
			}
		}
	
		// Save the document to this folder
		PathAddBackslash(szLevelRoot);
		sprintf(szFileName, "%s%s.cry", szLevelRoot, m_strDirName.GetBuffer(1));
		if (!GetIEditor()->GetDocument()->DoSave(szFileName, TRUE))
			return;

		// Only ask about a full export if the user choose to create a default level
		if (nResult == IDYES)
		{
			// Ask the user if he wants to do a full export ?
			nResult = MessageBox("Do you want to make a full export before starting to edit the level ? This" \
				" means creating a complete copy / export of the data for the engine. If you are going" \
				" to change all default settings before previewing the level anyway, choosing 'No' will" \
				" speed up the loading process.", "Do export first ?", MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL);
		}

		// Did the user create a default level and wants a full export ?
		if (nResult == IDYES)
		{
			CLogFile::WriteLine("User choose to perform a full export");

			// Export the default values. Make a full export which involves generating the
			// surface texture
			//GetIEditor()->GetDocument()->ExportToGame(true);
			CGameExporter gameExporter( GetIEditor()->GetSystem() );
			gameExporter.Export(true,true);
		}
		else
		{
			// Export the default values. This is relatively fast because no surface
			// texture will be generated
			//GetIEditor()->GetDocument()->ExportToGame(false);
			CGameExporter gameExporter( GetIEditor()->GetSystem() );
			gameExporter.Export(false,true);
		}

		/*
		// Let the engine load the new empty map
		CEngineSingleton::GetGameInterface()->GetInterface()->LoadLevel(szFileName);
		*/

		break;

	case 1:
		//////////////////////////////////////////////////////////////////////
		// Open an recent map
		//////////////////////////////////////////////////////////////////////

		if (m_lstRecentMaps.GetCurSel() == LB_ERR)
		{
			// No selection
			AfxMessageBox("Please select a map before pressing OK");
			return;
		}

		CLogFile::WriteLine("User choose to open an map");

		// Open the map
		m_lstRecentMaps.GetText(m_lstRecentMaps.GetCurSel(), strSelection);
		if (!AfxGetApp()->OpenDocumentFile(strSelection))
			return;
		break;

	case 2:
		//////////////////////////////////////////////////////////////////////
		// Open a map
		//////////////////////////////////////////////////////////////////////

		if (!PathFileExists(m_strMapFileName.GetBuffer(0)))
		{
			// File does not exist
			AfxMessageBox("Please specify / select a valid filename before pressing OK");
			return;
		}

		CLogFile::WriteLine("User choose to open an map");

		// Open the map
		if(!AfxGetApp()->OpenDocumentFile(m_strMapFileName))
			return;
		break;

	default:
		ASSERT(FALSE);
		break;
	}
	
	EndWaitCursor();

	CDialog::OnOK();
}

BOOL CStartupDialog::OnInitDialog() 
{
	//////////////////////////////////////////////////////////////////////
	// Fill the list box with the names of the recent maps
	//////////////////////////////////////////////////////////////////////

	UINT i;
	CRecentFileList *pList = ((CCryEditApp *) (AfxGetApp()))->GetRecentFileList();

	CDialog::OnInitDialog();
	
	CLogFile::WriteLine("Loading startup dialog...");

	ASSERT(pList);

	if (!pList)
		return TRUE;
	
	// Loop trough the array and add each string
	for (i=0; i<(unsigned int) pList->GetSize() - 1; i++)
	{
		// Does the file exist ?
		if (PathFileExists((* pList)[i].GetBuffer(0)))
			// Add it to the list
			m_lstRecentMaps.InsertString(0, (* pList)[i]);
	}

	// You can only use the current map as base if there is one
	m_cUseAsBase.EnableWindow(!GetIEditor()->GetDocument()->GetPathName().IsEmpty());
	m_cUseAsBase.SetCheck(0);
			
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CStartupDialog::OnBrowseForMap() 
{
	//////////////////////////////////////////////////////////////////////
	// Browse for a map
	//////////////////////////////////////////////////////////////////////

	CSelectFileDlg cDialog;
	CString strFileName;
	
	if (cDialog.SelectFileName(&strFileName, NULL, CString("*.cry"), CString("Levels"))) 
	{
		// Save the filename in the text box
		m_strMapFileName = strFileName;

		// Activate the correct option
		m_optSelection = 2;

		// Update the controls
		UpdateData(FALSE);
	}
}

void CStartupDialog::OnChangeLevDir() 
{
	//////////////////////////////////////////////////////////////////////
	// Let the user create a directory in the level subfolder where all
	// the data is stored and exported
	//////////////////////////////////////////////////////////////////////

	char szLevelRoot[_MAX_PATH];
	char szCompBuf[2];
	CStringDlg cDialog;
	bool bPathIsValid = true;
	UINT i;

	// Query for a name
	cDialog.m_strString = m_strDirName;
	cDialog.DoModal();

	// Verify new name
	if (cDialog.m_strString.IsEmpty())
		bPathIsValid = false;

	// User entered string, check for invalid chars
	for (i=0; i<(unsigned int) cDialog.m_strString.GetLength(); i++)
	{
		// Paste the char into the compare buffer
		szCompBuf[0] = cDialog.m_strString.GetBuffer(0)[i];
		szCompBuf[1] = '\0';

		// Is the char invalid ?
		if (strpbrk(szCompBuf, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_ ") == NULL)
		{
			bPathIsValid = false;
			break;
		}
	}
	
	// Valid ?
	if (!bPathIsValid)
	{
		AfxMessageBox("Level name is invalid, please use only a-z, A-Z and 0-9 !");
		return;
	}

	// Construct the directory name
	sprintf(szLevelRoot, "%sLevels\\%s", GetIEditor()->GetDocument()->GetMasterCDFolder().GetBuffer(1),
		cDialog.m_strString.GetBuffer(0));

	// Does the directory already exist ?
	if (PathFileExists(szLevelRoot))
	{
		AfxMessageBox("Directory / level aready exists, choose another name !");
		return;
	}

	// Write it into the control
	m_strDirName = cDialog.m_strString;

	// Chose option 1, "Create new Level"
	m_optSelection = 0;
	
	UpdateData(FALSE);
}

void CStartupDialog::OnSelectedRecentDoc()
{
	//////////////////////////////////////////////////////////////////////
	// User selected an item in the recent document list. Make the
	// second choice the default one
	//////////////////////////////////////////////////////////////////////

	m_optSelection = 1;
	UpdateData(FALSE);
}

void CStartupDialog::OnDblClickRecentDoc()
{
	//////////////////////////////////////////////////////////////////////
	// User selected an item in the recent document list. Open this map
	//////////////////////////////////////////////////////////////////////

	// Make the second choice the default one for the case that the
	// loading fails and the user gets dropped back to the dialog
	m_optSelection = 1;
	UpdateData(FALSE);

	// Open this map
	OnOK();
}
