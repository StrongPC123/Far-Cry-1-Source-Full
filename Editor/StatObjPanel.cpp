// StatObjPanel.cpp : implementation file
//

#include "stdafx.h"
#include "StatObjPanel.h"
#include "Objects\ObjectManager.h"
#include "Objects\BaseObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "Objects\BaseObject.h"
#include "Objects\StatObj.h"

/////////////////////////////////////////////////////////////////////////////
// CStatObjPanel dialog


CStatObjPanel::CStatObjPanel(CWnd* pParent /*=NULL*/)
	: CDialog(CStatObjPanel::IDD,pParent)
{
	//{{AFX_DATA_INIT(CStatObjPanel)
	//}}AFX_DATA_INIT

	m_object = 0;
	m_multiSelect = false;
	Create( IDD,pParent );
}


void CStatObjPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStatObjPanel)
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_STATOBJ_RELOAD, m_loadBtn);
	DDX_Control(pDX, IDC_STATOBJ_NAME, m_objectName);
	DDX_Control(pDX, IDC_RELOAD, m_reloadBtn);
}


BEGIN_MESSAGE_MAP(CStatObjPanel, CDialog)
	//{{AFX_MSG_MAP(CStatObjPanel)
	ON_BN_CLICKED(IDC_STATOBJ_RELOAD, OnStatobjReload)
	ON_BN_CLICKED(IDC_RELOAD, OnReload)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStatObjPanel message handlers

void CStatObjPanel::OnStatobjReload() 
{
	// Open stat obj file.
	CString relFile;
	if (!GetIEditor()->SelectFile( SUPPORTED_MODEL_FILTER,"Objects",relFile ))
		return;
	if (m_multiSelect)
	{
		CSelectionGroup *selection = GetIEditor()->GetSelection();
		for (int i = 0; i < selection->GetCount(); i++)
		{
			CBaseObject *pBaseObj=selection->GetObject(i);
			if (pBaseObj->IsKindOf(RUNTIME_CLASS(CStaticObject)))
				((CStaticObject*)pBaseObj)->LoadObject(relFile);
			if (!i)
			{
				CString str = ((CStaticObject*)pBaseObj)->GetObjectName();
				if (str.Find( "Objects\\" ) == 0)
					str.Replace( "Objects\\","" );
				m_objectName.SetWindowText( str );
			}
		}
	}else
	{
		assert( m_object );
		m_object->LoadObject(relFile);
		SetObject( m_object );
	}
}

//////////////////////////////////////////////////////////////////////////
void CStatObjPanel::UpdateObject()
{
	CStaticObject *pObj=NULL;
	if (m_multiSelect)
	{
		CSelectionGroup *selection = GetIEditor()->GetSelection();
		if (selection->IsEmpty())
			return;
		for (int i=0;i<selection->GetCount();i++)
		{
			CBaseObject *pBaseObj=selection->GetObject(i);
			if (pBaseObj->IsKindOf(RUNTIME_CLASS(CStaticObject)))
			{
				pObj=(CStaticObject*)pBaseObj;
				break;
			}
		}
		assert(pObj);
	}else
	{
		pObj=m_object;
	}
	CString str = pObj->GetObjectName();
	if (str.Find( "Objects\\" ) == 0)
	{
		str.Replace( "Objects\\","" );
	}
	m_objectName.EnableWindow(TRUE);
	m_objectName.SetWindowText( str );
}

//////////////////////////////////////////////////////////////////////////
void CStatObjPanel::SetObject( CStaticObject *obj )
{
	assert( obj );
	m_object = obj;

	UpdateObject();
}

void CStatObjPanel::SetMultiSelect( bool bEnable )
{
	m_multiSelect = bEnable;
	if (bEnable)
	{
		UpdateObject();
		CSelectionGroup *selection = GetIEditor()->GetSelection();
		CStaticObject *pFirst=NULL;
		for (int i=0;i<selection->GetCount();i++)
		{
			CBaseObject *pBaseObj=selection->GetObject(i);
			CStaticObject *pObj=NULL;
			if (pBaseObj->IsKindOf(RUNTIME_CLASS(CStaticObject)))
				pObj=(CStaticObject*)pBaseObj;
			else
				continue;
			if (!pFirst)
			{
				pFirst=pObj;
				continue;
			}
			if (pFirst->GetObjectName()!=pObj->GetObjectName())
				m_objectName.EnableWindow(FALSE);
		}
	}
}

void CStatObjPanel::OnReload() 
{
	if (m_multiSelect)
	{
		CSelectionGroup *selection = GetIEditor()->GetSelection();
		for (int i = 0; i < selection->GetCount(); i++)
		{
			CBaseObject *pBaseObj=selection->GetObject(i);
			if (pBaseObj->IsKindOf(RUNTIME_CLASS(CStaticObject)))
				((CStaticObject*)pBaseObj)->ReloadObject();
		}
	}else
	{
		assert( m_object );
		m_object->ReloadObject();
	}
}

BOOL CStatObjPanel::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}