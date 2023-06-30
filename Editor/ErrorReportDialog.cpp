////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   errorreportdialog.cpp
//  Version:     v1.00
//  Created:     30/5/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ErrorReportDialog.h"
#include "ErrorReport.h"
#include "Clipboard.h"

#include "Objects\BaseObject.h"
#include "material\Material.h"

//////////////////////////////////////////////////////////////////////////
CErrorReportDialog* CErrorReportDialog::m_instance = 0;

// CErrorReportDialog dialog
#define BITMAP_ERROR 0
#define BITMAP_WARNING 1
#define BITMAP_COMMENT 2

#define COLUMN_TEXT 0
#define COLUMN_FILE 1
#define COLUMN_INFO 2

static int __stdcall CompareItems( LPARAM p1,LPARAM p2,LPARAM sort )
{
	CErrorRecord *err1 = (CErrorRecord*)p1;
	CErrorRecord *err2 = (CErrorRecord*)p2;
	if (err1->severity < err2->severity)
		return -1;
	else if (err1->severity > err2->severity)
		return 1;
	else
		return stricmp(err1->error,err2->error);
}

IMPLEMENT_DYNAMIC(CErrorReportDialog, CXTResizeDialog)
CErrorReportDialog::CErrorReportDialog( CErrorReport *report,CWnd* pParent /*=NULL*/)
	: CXTResizeDialog(CErrorReportDialog::IDD, pParent)
{
	m_pErrorReport = report;
}

CErrorReportDialog::~CErrorReportDialog()
{
}

//////////////////////////////////////////////////////////////////////////
void CErrorReportDialog::Open( CErrorReport *pReport )
{
	if (m_instance)
	{
		delete m_instance;
	}
	m_instance = new CErrorReportDialog( pReport );
	m_instance->Create( CErrorReportDialog::IDD,AfxGetMainWnd() );
	m_instance->ShowWindow( SW_SHOW );
}

//////////////////////////////////////////////////////////////////////////
void CErrorReportDialog::Close()
{
	if (m_instance)
		m_instance->DestroyWindow();
}

void CErrorReportDialog::DoDataExchange(CDataExchange* pDX)
{
	CXTResizeDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ERRORS, m_errors);
}


BEGIN_MESSAGE_MAP(CErrorReportDialog, CXTResizeDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_ERRORS, OnNMDblclkErrors)
	ON_BN_CLICKED(IDC_SELECTOBJECTS, OnSelectObjects)
	ON_BN_CLICKED(IDC_COPY, OnCopyToClipboard)
END_MESSAGE_MAP()


// CErrorReportDialog message handlers

BOOL CErrorReportDialog::OnInitDialog()
{
	CXTResizeDialog::OnInitDialog();

	//m_imageList.Create(IDB_ERROR_REPORT, 16, 1, RGB (255, 255, 255));
	CMFCUtils::LoadTrueColorImageList( m_imageList,IDB_ERROR_REPORT,16,RGB(255,255,255) );

	SetResize( IDC_ERRORS,SZ_RESIZE(1) );
	SetResize( IDOK,SZ_REPOS(1) );
	SetResize( IDC_BOTTOM_FRAME,CXTResizeRect(0,1,1,1) );
	SetResize( IDC_SELECTOBJECTS,SZ_VERREPOS(1) );
	SetResize( IDC_COPY,SZ_VERREPOS(1) );

	AutoLoadPlacement( "Dialogs\\ErrorReport" );


	CRect rc;
	m_errors.GetClientRect( rc );
	int w1 = rc.right*2/4;
	int w2 = rc.right*1/4;
	int w3 = rc.right*1/4;
	m_errors.SetExtendedStyle( LVS_EX_FLATSB|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP );
	m_errors.InsertColumn( COLUMN_TEXT,_T("Error"),LVCFMT_LEFT,w1,0 );
	m_errors.InsertColumn( COLUMN_FILE,_T("File"),LVCFMT_LEFT,w2,1 );
	m_errors.InsertColumn( COLUMN_INFO,_T("Info"),LVCFMT_LEFT,w3,2 );

	m_errors.SetImageList( &m_imageList,LVSIL_SMALL );

	ReloadErrors();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CErrorReportDialog::ReloadErrors()
{
	m_errors.SetRedraw(FALSE);
	m_errors.DeleteAllItems();

	// Store localarray of error records.
	m_errorRecords.clear();
	m_errorRecords.reserve( m_pErrorReport->GetErrorCount() );
	for (int i = 0; i < m_pErrorReport->GetErrorCount(); i++)
	{
		CErrorRecord &err = m_pErrorReport->GetError(i);
		m_errorRecords.push_back( err );
	}
	
	for (int i = 0; i < m_errorRecords.size(); i++)
	{
		CErrorRecord &err = m_errorRecords[i];
		const char *str = err.error;

		LVITEM item;
		ZeroStruct(item);
		item.mask = LVIF_IMAGE|LVIF_TEXT|LVIF_PARAM;
		item.iItem = i;
		item.iSubItem = 0;
		item.pszText = const_cast<char*>(str);
		item.iImage = BITMAP_COMMENT;
		item.lParam = (LPARAM)(&err);

		if (err.severity == CErrorRecord::ESEVERITY_ERROR)
			item.iImage = BITMAP_ERROR;
		else if (err.severity == CErrorRecord::ESEVERITY_WARNING)
			item.iImage = BITMAP_WARNING;
		else if (err.severity == CErrorRecord::ESEVERITY_COMMENT)
			item.iImage = BITMAP_COMMENT;
		
		int id = m_errors.InsertItem( &item );
		//m_list.SetItem( id,COLUMN_NAME,LVIF_PARAM|LVIF_INDENT|LVIF_IMAGE,0,iImage,0,0,(LPARAM)obj,level );
		if (err.file.IsEmpty())
		{
			if (err.pMaterial)
			{
				err.file = err.pMaterial->GetFullName();
			}
		}
		m_errors.SetItem( id,COLUMN_FILE,LVIF_TEXT,(const char*)err.file,0,0,0,0 );

		if (err.pObject)
		{
			m_errors.SetItem( id,COLUMN_INFO,LVIF_TEXT,CString("Object: ")+err.pObject->GetName(),0,0,0,0 );
		}
		else if (err.pMaterial)
		{
			m_errors.SetItem( id,COLUMN_INFO,LVIF_TEXT,CString("Material: ")+err.pMaterial->GetName(),0,0,0,0 );
		}
	}
	m_errors.SortItems( CompareItems,0 );

	m_errors.SetRedraw(TRUE);
}

//////////////////////////////////////////////////////////////////////////
void CErrorReportDialog::PostNcDestroy()
{
	CXTResizeDialog::PostNcDestroy();
	if (m_instance)
		delete m_instance;
	m_instance = 0;
}

void CErrorReportDialog::OnNMDblclkErrors(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMITEMACTIVATE *lpNM = (NMITEMACTIVATE*)pNMHDR;

	CErrorRecord *err = (CErrorRecord*)m_errors.GetItemData( lpNM->iItem );
	if (err)
	{
		if (err->pObject)
		{
			CUndo undo( "Select Object(s)" );
			// Clear other selection.
			GetIEditor()->ClearSelection();
			// Select this object.
			GetIEditor()->SelectObject( err->pObject );
		}
		else if (err->pMaterial)
		{
			GetIEditor()->OpenDataBaseLibrary( EDB_MATERIAL_LIBRARY,err->pMaterial );
		}
	}

	// TODO: Add your control notification handler code here
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CErrorReportDialog::OnOK()
{
	DestroyWindow();
}

//////////////////////////////////////////////////////////////////////////
void CErrorReportDialog::OnCancel()
{
	DestroyWindow();
}

//////////////////////////////////////////////////////////////////////////
void CErrorReportDialog::OnSelectObjects()
{
	CUndo undo( "Select Object(s)" );
	// Clear other selection.
	GetIEditor()->ClearSelection();
	POSITION pos = m_errors.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_errors.GetNextSelectedItem(pos);
		CErrorRecord *pError = (CErrorRecord*)m_errors.GetItemData( nItem );
		if (pError && pError->pObject)
		{
			// Select this object.
			GetIEditor()->SelectObject( pError->pObject );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CErrorReportDialog::OnCopyToClipboard()
{
	CClipboard clipboard;

	CString str;

	POSITION pos = m_errors.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_errors.GetNextSelectedItem(pos);
		CErrorRecord *pError = (CErrorRecord*)m_errors.GetItemData( nItem );		
		if (pError)
		{
			str += pError->GetErrorText() + "\r\n";
		}
	}
	clipboard.PutString( str,"Errors" );
}
