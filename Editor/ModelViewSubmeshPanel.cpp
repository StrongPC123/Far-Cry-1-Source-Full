// ModelViewSubmeshPanel.cpp : implementation file
//

#include "stdafx.h"
#include "ICryAnimation.h"
#include "ModelViewSubmeshPanel.h"

#include "ModelViewport.h"
#include "StringUtils.h"
#include "CryFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ModelViewSubmeshPanel dialog


ModelViewSubmeshPanel::ModelViewSubmeshPanel( CModelViewport *vp,CWnd* pParent /* = NULL */)
	: CXTResizeDialog(ModelViewSubmeshPanel::IDD, pParent)
{
	m_modelView = vp;

	Create(MAKEINTRESOURCE(IDD),pParent);
	//{{AFX_DATA_INIT(ModelViewSubmeshPanel)
	//}}AFX_DATA_INIT
}


void ModelViewSubmeshPanel::DoDataExchange(CDataExchange* pDX)
{
	CXTResizeDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ModelViewSubmeshPanel)
	DDX_Control(pDX, IDC_MODELVIEW_SUBMESH_PANEL, m_submeshes);

	//DDX_Control(pDX, IDC_SUBMESH_VISIBLE, m_submeshVisible);
	DDX_Control(pDX, IDC_ADDSUBMESH, m_addSubmeshBtn);
	DDX_Control(pDX, IDC_RELOADSUBMESH, m_reloadSubmeshBtn);
	DDX_Control(pDX, IDC_REMOVESUBMESH, m_removeSubmeshBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ModelViewSubmeshPanel, CXTResizeDialog)
	//{{AFX_MSG_MAP(ModelViewSubmeshPanel)
	ON_LBN_SELCHANGE(IDC_MODELVIEW_SUBMESH_PANEL, OnSelchangeSubmeshes)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_ADDSUBMESH, OnBnClickedAddSubmesh)
	ON_BN_CLICKED(IDC_RELOADSUBMESH, OnBnClickedReloadSubmesh)
	ON_BN_CLICKED(IDC_CHANGESUBMESH, OnBnClickedChangeSubmesh)
	ON_BN_CLICKED(IDC_REMOVESUBMESH, OnBnClickedRemoveSubmesh)
	//ON_BN_CLICKED(IDC_SUBMESH_VISIBLE, OnBnClickedSubmeshVisible)
	ON_CLBN_CHKCHANGE(IDC_MODELVIEW_SUBMESH_PANEL, OnCmdCheckChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ModelViewSubmeshPanel message handlers

int ModelViewSubmeshPanel::GetCurSubmesh()
{
	return m_submeshes.GetItemData (m_submeshes.GetCurSel());
}

int ModelViewSubmeshPanel::GetItemBySubmesh(int nSubmesh)
{
	for (int nItem = 0; nItem < m_submeshes.GetCount(); ++nItem)
	{
		if (m_submeshes.GetItemData(nItem) == nSubmesh)
			return nItem;
	}

	return -1;
}

bool ModelViewSubmeshPanel::IsChecked(int nSubmesh)
{
	return m_submeshes.GetCheck (GetItemBySubmesh(nSubmesh)) == BST_CHECKED;
}


void ModelViewSubmeshPanel::OnCmdCheckChange()
{
	ICryCharInstance* pCharacter = m_modelView->GetCharacter();
	if (pCharacter)
	for (int nSubmesh = 0; nSubmesh < m_submeshes.GetCount(); ++nSubmesh)
	{
		ICryCharSubmesh* pSubmesh = pCharacter->GetSubmesh(nSubmesh);
		if (pSubmesh)
			pSubmesh->SetVisible(IsChecked(nSubmesh));
	}
	OnSelchangeSubmeshes();
}

void ModelViewSubmeshPanel::ReinitSubmeshes()
{
	m_submeshes.ResetContent();
	ICryCharInstance* pCharacter = m_modelView->GetCharacter();
	for (unsigned nSubmesh = 0; nSubmesh < pCharacter->NumSubmeshes(); ++nSubmesh)
	{
		ICryCharSubmesh* pSubmesh = pCharacter->GetSubmesh(nSubmesh);
		if (pSubmesh)
		{
			char szBuf[16];
			sprintf (szBuf, "%2d: ", nSubmesh);
			int nItem = m_submeshes.AddString( szBuf + CString(pSubmesh->GetModel()->GetFileName()));
			m_submeshes.SetCheck(nItem, pSubmesh->IsVisible()?BST_CHECKED:BST_UNCHECKED);
			m_submeshes.SetItemData(nItem, nSubmesh);
		}
	}

	OnSelchangeSubmeshes();

	m_modelView->OnSubmeshSetChanged();
}

BOOL ModelViewSubmeshPanel::OnInitDialog() 
{
	CXTResizeDialog::OnInitDialog();
	
	//m_submeshes.SetBkColor( RGB(0xE0,0xF0,0xF0) );

	SetResize(IDC_MODELVIEW_SUBMESH_PANEL,SZ_RESIZE(1));
	SetResize(IDC_ADDSUBMESH,SZ_HORRESIZE(1));
	SetResize(IDC_REMOVESUBMESH,SZ_HORREPOS(1));
	SetResize(IDC_RELOADSUBMESH,SZ_HORREPOS(1));
	m_submeshes.SetCheckStyle(BS_AUTOCHECKBOX);
	ReinitSubmeshes();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL ModelViewSubmeshPanel::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	// Route the commands to the MainFrame
	if (AfxGetMainWnd())
		AfxGetMainWnd()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	
	return CXTResizeDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


void ModelViewSubmeshPanel::OnSelchangeSubmeshes() 
{
	int nSubmesh = GetCurSubmesh();
	ICryCharSubmesh* pSubmesh = m_modelView->GetCharacter()->GetSubmesh(nSubmesh);
	if (pSubmesh && nSubmesh > 0)
	{
		m_reloadSubmeshBtn.EnableWindow(TRUE);
		m_removeSubmeshBtn.EnableWindow(TRUE);
	}
	else
	{
		m_reloadSubmeshBtn.EnableWindow(FALSE);
		m_removeSubmeshBtn.EnableWindow(FALSE);
	}
}

void ModelViewSubmeshPanel::OnBnClickedChangeSubmesh()
{
	int nSubmesh = GetCurSubmesh();
	ICryCharInstance* pCharacter = m_modelView->GetCharacter();
	if (nSubmesh > 0 && nSubmesh < pCharacter->NumSubmeshes())
	{
		CString strFile;
		if (CFileUtil::SelectSingleFile(EFILE_TYPE_GEOMETRY, strFile))
		{
			m_modelView->SetSubmesh (nSubmesh, strFile);
			ReinitSubmeshes();
		}
	}
}

void ModelViewSubmeshPanel::OnBnClickedReloadSubmesh()
{
	int nSubmesh = GetCurSubmesh();
	ICryCharInstance* pCharacter = m_modelView->GetCharacter();
	if (pCharacter)
	{
		ICryCharSubmesh* pSubmesh = pCharacter->GetSubmesh(nSubmesh);
		if (pSubmesh)
		{
			CString strFile = pSubmesh->GetModel()->GetFileName();
			pCharacter->RemoveSubmesh(nSubmesh);
			m_modelView->SetSubmesh(nSubmesh, strFile);
			ReinitSubmeshes();
		}
	}
}

void ModelViewSubmeshPanel::OnBnClickedAddSubmesh()
{
	CString relFileName;
	//if (CFileUtil::SelectSingleFile( EFILE_TYPE_GEOMETRY,relFileName ))
	//	m_modelView->AddSubmesh(relFileName);

	std::vector<CString> arrFiles;
	if (CFileUtil::SelectMultipleFiles( EFILE_TYPE_ANY, arrFiles, "Crytek Geometry File|*.cgf|Character List|*.cl|All Files|*.*"))
	{
		for (std::vector<CString>::iterator it = arrFiles.begin(); it != arrFiles.end(); ++it)
		{
			it->TrimLeft();
			it->TrimRight();
			if (!stricmp(CryStringUtils::FindExtension(*it),"cgf"))
				m_modelView->AddSubmesh(*it);
			else
			{
				CCryFile file;
				if (!file.Open (*it, "rt"))
					continue;
					
				std::vector<char> arrChars;
				arrChars.resize (file.GetLength()+1, '\0');
				if (file.Read(&arrChars[0], arrChars.size()-1))
				{
					// find the relative path of the character list file
					CString strDirectory = CryStringUtils::GetParentDirectory(string((const char*)*it)).c_str();
					strDirectory += "\\";

					int nPos = 0;
					CString str = &arrChars[0];
					arrChars.clear();
					CString strNext;
					
					while ((strNext = TokenizeString(str, "\n\r", nPos)).GetLength())
					{
						if (strNext[0] == '\\' || (strNext[0] && strNext[1] == ':'))
						{
							// this is an absolute path, we don't transform it
						}
						else
						{
							// this is a relative path, transform it
							strNext = strDirectory + strNext;
						}
						m_modelView->AddSubmesh(strNext);
					};
				}
			}
		}
	}

	ReinitSubmeshes();
}

void ModelViewSubmeshPanel::OnBnClickedRemoveSubmesh()
{
	int nSubmesh = GetCurSubmesh();
	ICryCharInstance* pCharacter = m_modelView->GetCharacter();
	pCharacter->RemoveSubmesh(nSubmesh);

	ReinitSubmeshes();	
}

void ModelViewSubmeshPanel::OnBnClickedSubmeshVisible()
{
	ICryCharSubmesh* pSubmesh = m_modelView->GetCharacter()->GetSubmesh(GetCurSubmesh());
	//if (pSubmesh)
	//	pSubmesh->SetVisible(m_submeshVisible.GetCheck() == BST_CHECKED);
}


