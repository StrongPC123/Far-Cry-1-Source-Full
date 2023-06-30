// ShaderTemplatePanel.cpp : implementation file
//

#include "stdafx.h"
#include "ShaderTemplatePanel.h"
#include "ShaderTemplateTool.h"									// CTemplateTool
#include "resource.h"														// IDC_SHADERTEMPL_XML

#include "Objects\\BaseObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShaderTemplatePanel dialog


CShaderTemplatePanel::CShaderTemplatePanel( CShaderTemplateTool *obj,XmlNodeRef &node,CWnd* pParent /*=NULL*/)
	: CDialog(CShaderTemplatePanel::IDD, pParent)
	, m_sParameterSetName(_T(""))
{
	//{{AFX_DATA_INIT(CShaderTemplatePanel)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_node							= node;
	m_object						= obj;
	m_sFilename					= "shader.stp";
	m_sParameterSetName = "unnamed";

	assert( m_object != 0 );


	Create( IDD,pParent );
}


void CShaderTemplatePanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShaderTemplatePanel)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_TEMPLATECOMBO, m_TemplateCombo);
	DDX_Control(pDX, IDC_SHADERTEMPL_MATERIALNAME, m_Material);
	DDX_Text(pDX, IDC_SHADERTEMPL_FILENAME, m_sFilename);
	DDX_Text(pDX, IDC_SHADERTEMPL_NAME, m_sParameterSetName);
}


BEGIN_MESSAGE_MAP(CShaderTemplatePanel, CDialog)
	//{{AFX_MSG_MAP(CShaderTemplatePanel)
	ON_WM_DESTROY()
	ON_WM_KILLFOCUS()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_TEMPLATECOMBO, OnCbnSelchangeTemplatecombo)
	ON_BN_CLICKED(IDC_SHADERTEMPL_COPY, OnBnClickedShadertemplCopy)
	ON_BN_CLICKED(IDC_SHADERTEMPL_SETDEFAULT, OnBnClickedShadertemplSetdefault)
	ON_BN_CLICKED(IDC_SHADERTEMPL_PASTE, OnBnClickedShadertemplPaste)
	ON_BN_CLICKED(IDC_SHADERTEMPL_LOAD, OnBnClickedShadertemplLoad)
	ON_BN_CLICKED(IDC_SHADERTEMPL_SAVE, OnBnClickedShadertemplSave)
	ON_BN_CLICKED(IDC_SHADERTEMPL_GETFROMSEL, OnBnClickedShadertemplGetfromsel)
	ON_BN_CLICKED(IDC_SHADERTEMPL_SELECTBYSTP, OnBnClickedShadertemplSelectbystp)
	ON_BN_CLICKED(IDC_SHADERTEMPL_TOLIB, OnBnClickedShadertemplTolib)
	ON_BN_CLICKED(IDC_SHADERTEMPL_FROMLIB, OnBnClickedShadertemplFromlib)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShaderTemplatePanel message handlers

BOOL CShaderTemplatePanel::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_TemplateCombo.SetWindowPos(0,0,0,190,180,SWP_NOREPOSITION | SWP_NOMOVE | SWP_NOZORDER);
	m_TemplateCombo.SetItemHeight(-1,15);

	CRect rc;
	CWnd *w=GetDlgItem(IDC_SHADERTEMPL_XML);
	w->GetWindowRect( rc );
	ScreenToClient(rc);

	m_propWnd.Create( WS_CHILD,rc,this );
	m_propWnd.ShowWindow( SW_SHOW );
	m_propWnd.SetSplitter( (2*rc.right)/5 );
	SetProperties( m_node );
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CShaderTemplatePanel::SetProperties( XmlNodeRef &props )
{
	m_node = props;
	// If properties window is already created.

	if (GetSafeHwnd())
	{
		if (m_node)
		{
			m_propWnd.CreateItems(m_node);
			m_propWnd.SetUpdateCallback( functor(*this,&CShaderTemplatePanel::OnPropertyChanged) );

			Invalidate();
			UpdateWindow();
		
			// calles OnSize()
//			SetWindowPos( NULL,0,0,rc.right,MIN((rc.bottom+yOffset*2),listSize.cy+yOffset*2)+30,SWP_NOMOVE );
		}
	}
}

void CShaderTemplatePanel::OnDestroy() 
{
	CDialog::OnDestroy();
}

void CShaderTemplatePanel::OnKillFocus(CWnd* pNewWnd) 
{
	CDialog::OnKillFocus(pNewWnd);	
}

void CShaderTemplatePanel::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	/*
	if (m_propWnd)
	{
		FRect fr = m_propWnd->GetWindowRect();
		CRect rc( fr.Min.X,fr.Min.Y,fr.Max.X,fr.Max.Y );
		ScreenToClient( &rc );
		
		m_propWnd->EnableUpdateCallback(false);
		m_propWnd->MoveWindow( 2,4,cx-2,fr.Height()+30,TRUE );
		m_propWnd->DividerWidth = (2*cx)/5;
		m_propWnd->Resizable = true;
		m_propWnd->ResizeList();
		m_propWnd->Resizable = false;
		m_propWnd->ForceRefresh();
		m_propWnd->EnableUpdateCallback(true);
	}
	*/
}


//////////////////////////////////////////////////////////////////////////
void CShaderTemplatePanel::OnPropertyChanged( XmlNodeRef node )
{
//	m_object->OnPropertyChange( node->getTag() );
}
void CShaderTemplatePanel::OnCbnSelchangeTemplatecombo()
{
	int sel=m_TemplateCombo.GetCurSel();	

	CString str;

	m_TemplateCombo.GetLBText(sel,str);

	IShader *sshader=GetIEditor()->GetRenderer()->EF_LoadShader(str.GetBuffer(64),eSH_World,0);

	if(sshader)
	{
		TArray<SShaderParam> &shaderparams=sshader->GetPublicParams();

	//	struct SShaderParam 
	//    string m_Name;
	//    EParamType m_Type;
	//    UParamVal m_Value;
	//
	//  union UParamVal
	//  {
	//    byte m_Byte;	
	//    short m_Short;
	//    int m_Int;
	//    float m_Float;
	//    char *m_String;
	//  };

		CString str="<Root>";

		int nParamCount=shaderparams.Num();

		for(int e=0;e<nParamCount;e++)
		{
			SShaderParam *ptr=&shaderparams[e];

			// Min Max attribute

			switch(ptr->m_Type)
			{
				case eType_BYTE:
					{
						CString val;	val.Format("%d",(int)(ptr->m_Value.m_Byte));
						str+="<"+CString(ptr->m_Name)+" type=\"Int\" value=\"" +val+ "\"/>";
					}
					break;
				case eType_SHORT:
					{
						CString val;	val.Format("%d",(int)(ptr->m_Value.m_Short));
						str+="<"+CString(ptr->m_Name)+" type=\"Int\" value=\"" +val+ "\"/>";
					}
					break;
				case eType_INT:
					{
						CString val;	val.Format("%d",(int)(ptr->m_Value.m_Int));
						str+="<"+CString(ptr->m_Name)+" type=\"Int\" value=\"" +val+ "\"/>";
					}
					break;
				case eType_FLOAT:
					{
						CString val;	val.Format("%f",(float)(ptr->m_Value.m_Float));
						str+="<"+CString(ptr->m_Name)+" type=\"Float\" value=\"" +val+ "\" Precision=\"5\"/>";
					}
					break;
				case eType_STRING:
					{
						CString val;	val.Format("%s",(char *)(ptr->m_Value.m_String));
						str+="<"+CString(ptr->m_Name)+" type=\"String\" value=\"" +val+ "\"/>";
					}
					break;

				case eType_UNKNOWN:
				default:
					assert(0);
					break;
			}
		}

		str+="</Root>";

		XmlParser xml;

		m_node=xml.parseBuffer(str);

		SetProperties(m_node);

		sshader->Release();
	}
}


void CShaderTemplatePanel::OnBnClickedShadertemplCopy()
{
	AfxMessageBox("todo");
}

void CShaderTemplatePanel::OnBnClickedShadertemplSetdefault()
{
	AfxMessageBox("todo");
}

void CShaderTemplatePanel::OnBnClickedShadertemplPaste()
{
	AfxMessageBox("todo");
}

void CShaderTemplatePanel::OnBnClickedShadertemplLoad()
{
	assert(m_object);

	m_object->OnBnClickedShadertemplLoad();

	m_sFilename=m_object->getFilename();

	UpdateData(false);
}

void CShaderTemplatePanel::OnBnClickedShadertemplSave()
{
	assert(m_object);

	m_object->OnBnClickedShadertemplSave();
}

void CShaderTemplatePanel::OnBnClickedShadertemplGetfromsel()
{
	AfxMessageBox("todo");
}

void CShaderTemplatePanel::OnBnClickedShadertemplSelectbystp()
{
	AfxMessageBox("todo");
}

void CShaderTemplatePanel::OnBnClickedShadertemplTolib()
{
	AfxMessageBox("todo");
}

void CShaderTemplatePanel::OnBnClickedShadertemplFromlib()
{
	AfxMessageBox("todo");
}
