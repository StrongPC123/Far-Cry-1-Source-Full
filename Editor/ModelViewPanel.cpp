// ModelViewPanel.cpp : implementation file
//

#include "stdafx.h"
#include <ICryAnimation.h>
#include "ModelViewPanel.h"

#include "ModelViewport.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ModelViewPanel dialog


ModelViewPanel::ModelViewPanel( CModelViewport *vp,CWnd* pParent /* = NULL */)
	: CDialog(ModelViewPanel::IDD, pParent)
{
	m_modelView = vp;

	Create(MAKEINTRESOURCE(IDD),pParent);
	//{{AFX_DATA_INIT(ModelViewPanel)
	//}}AFX_DATA_INIT
}


void ModelViewPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ModelViewPanel)
	DDX_Control(pDX, IDC_LAYER, m_layer);
	DDX_Control(pDX, IDC_LOOP, m_loop);
	DDX_Control(pDX, IDC_LOCKBLENDTIMES, m_lockBlendTimes);
	DDX_Control(pDX, IDC_ANIMATIONS, m_animations);
	DDX_Control(pDX, IDC_BROWSE_OBJECT,m_browseObjectBtn);
	DDX_Control(pDX, IDC_WEAPONMODEL, m_attachBtn);
	DDX_Control(pDX, IDC_DETACH, m_detachBtn);
	DDX_Control(pDX, IDC_DETACHALL, m_detachAllBtn);
	DDX_Control(pDX, IDC_STOPANIMATION, m_stopAnimationBtn);
	DDX_Control(pDX, IDC_OBJECT, m_objectName);
	DDX_Control(pDX, IDC_BONE, m_boneName);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_SYNCHRONIZE_PHASE, m_syncPhase);
	DDX_Control(pDX, IDC_FILE, m_fileEdit);
}


BEGIN_MESSAGE_MAP(ModelViewPanel, CDialog)
	//{{AFX_MSG_MAP(ModelViewPanel)
	ON_BN_CLICKED(IDC_ANIMATE_LIGHTS, OnAnimateLights)
	ON_LBN_SELCHANGE(IDC_ANIMATIONS, OnSelchangeAnimations)
	ON_BN_CLICKED(IDC_LOOP, OnLoop)
	ON_BN_CLICKED(IDC_LOCKBLENDTIMES, OnLockBlendTimes)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_WEAPONMODEL, OnBnClickedAttachObject)
	ON_BN_CLICKED(IDC_DETACH, OnBnClickedDetachObject)
	ON_BN_CLICKED(IDC_DETACHALL, OnBnClickedDetachAll)
	ON_BN_CLICKED(IDC_STOPANIMATION, OnBnClickedStopAnimation)
	ON_BN_CLICKED(IDC_RESETANIMATIONS, OnBnClickedResetAnimations)
	ON_EN_CHANGE(IDC_BONE, OnEnChangeBone)
	ON_BN_CLICKED(IDC_BROWSE_OBJECT, OnBnClickedBrowseObject)
	ON_BN_CLICKED(IDC_BROWSE_FILE, OnBnClickedBrowseFile)
	ON_BN_CLICKED(IDC_RELOAD_FILE, OnBnClickedReloadFile)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ModelViewPanel message handlers

int ModelViewPanel::GetCurSel()
{
	return m_animations.GetCurSel();
}

CString ModelViewPanel::GetCurrAnimName()
{
	int sel = GetCurSel();
	if (sel == CB_ERR)
	{
		return "";
	}
	CString str;
	m_animations.GetText( sel,str );
	return str;
}

//////////////////////////////////////////////////////////////////////////
// [Sergiy]
// Adds an animation with the given name and duration (in seconds)
// to the animation list box/view.
// PARAMETERS:
// name      - the animation name
// fDuration - animation length [SECONDS]
void ModelViewPanel::AddAnimName( const CString &name, float fDuration)
{
	m_animations.AddString( name );
	//m_cAnimSequences.SetCurSel(0);
	
	/*
	CString str;
	str.Format( "%.1f", fDuration);
	int id = m_animations.InsertItem( m_animations.GetItemCount(),name );
	m_animations.SetItem( id,1,LVIF_TEXT,str,0,0,0,0 );
	*/
}

void ModelViewPanel::ClearAnims()
{
	m_animations.ResetContent();
	//m_animations.DeleteAllItems();
}

BOOL ModelViewPanel::OnInitDialog() 
{
	CDialog::OnInitDialog();

	
	m_animations.SetBkColor( RGB(0xE0,0xE0,0xE0) );
	m_layer.SetCurSel(0);

	m_blendInTime.Create( this,IDC_BLENDINTIME );
	m_blendInTime.SetValue( 0.125f );

	m_blendOutTime.Create( this,IDC_BLENDOUTTIME );
	m_blendOutTime.SetValue( 0.125f );
	m_blendOutTime.EnableWindow(FALSE);

	m_lockBlendTimes.SetCheck(BST_CHECKED);
	m_syncPhase.SetCheck(BST_CHECKED);

	m_loop.SetCheck(BST_CHECKED);

	m_boneName.SetWindowText( "weapon_bone" );

	/*
	m_animations.SetTextBkColor( RGB(0xE0,0xE0,0xE0) );

	CRect rc;
	m_animations.GetClientRect( rc );
	int w1 = rc.right - 60;
	int w2 = 50;

	m_animations.SetExtendedStyle( LVS_EX_FLATSB|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES );
	m_animations.InsertColumn( 1,"Animation",LVCFMT_LEFT,w1,0 );
	m_animations.InsertColumn( 2,"Frames",LVCFMT_LEFT,w2,1 );
	*/
	/*
	CWnd *frame = GetDlgItem( IDC_ANIMFRAME );
	if (frame)
	{
		// TODO: Add your specialized creation code here
		m_animBar.CreateEx( this,TBSTYLE_FLAT, WS_CHILD|WS_VISIBLE|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_FIXED);
		m_animBar.LoadToolBar(IDR_ANIMATION);
		
		CRect rc;
		frame->GetWindowRect( rc );
		ScreenToClient( rc );
		CRect rcb;
		CSize sz = m_animBar.CalcFixedLayout(FALSE,TRUE);
		int x = (rc.right-rc.left)/2 - (sz.cx)/2;
		int y = (rc.bottom-rc.top)/2 - (sz.cy)/2;
		m_animBar.MoveWindow( x+rc.left,y+rc.top,x+rc.right,y+rc.bottom );
	}
	*/
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL ModelViewPanel::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	// Route the commands to the MainFrame
	if (AfxGetMainWnd())
		AfxGetMainWnd()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	
	return CDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


bool ModelViewPanel::GetAnimLightState()
{
	return IsDlgButtonChecked( IDC_ANIMATE_LIGHTS ) == TRUE;
}

void ModelViewPanel::OnAnimateLights() 
{
	// TODO: Add your control notification handler code here
	AfxGetMainWnd()->SendMessage( WM_COMMAND,MAKEWPARAM(IDC_ANIMATE_LIGHTS,0),(LPARAM)GetSafeHwnd() );
}

bool ModelViewPanel::GetLooped()
{
	return m_loop.GetState() == BST_CHECKED;
}

bool ModelViewPanel::GetSynchronizePhase()
{
	return m_syncPhase.GetState() == BST_CHECKED;
}


void ModelViewPanel::OnSelchangeAnimations() 
{
	AfxGetMainWnd()->SendMessage( WM_COMMAND,MAKEWPARAM(ID_ANIM_PLAY,0),(LPARAM)GetSafeHwnd() );
}

void ModelViewPanel::OnLoop() 
{
	AfxGetMainWnd()->SendMessage( WM_COMMAND,MAKEWPARAM(ID_ANIM_PLAY,0),(LPARAM)GetSafeHwnd() );
}

void ModelViewPanel::OnLockBlendTimes ()
{
	m_blendOutTime.EnableWindow (!IsBlendTimeLock());
}

bool ModelViewPanel::IsBlendTimeLock()
{
	return (m_lockBlendTimes.GetState() & BST_CHECKED) != 0;
}
	
//////////////////////////////////////////////////////////////////////////
int ModelViewPanel::GetLayer() const
{
	return m_layer.GetCurSel();
}

//////////////////////////////////////////////////////////////////////////
float ModelViewPanel::GetBlendInTime()
{
	return m_blendInTime.GetValue();
}

//////////////////////////////////////////////////////////////////////////
float ModelViewPanel::GetBlendOutTime()
{
	return IsBlendTimeLock() ? GetBlendInTime() : m_blendOutTime.GetValue();
}

void ModelViewPanel::OnBnClickedAttachObject()
{
	CString bone,model;
	m_boneName.GetWindowText( bone );
	m_objectName.GetWindowText( model );
	m_modelView->AttachObject( model,bone );
}

void ModelViewPanel::OnBnClickedDetachObject()
{
	CString bone,model;
	m_boneName.GetWindowText( bone );
	m_objectName.GetWindowText( model );

	ICryCharInstance* pCharacter = m_modelView->GetCharacter();
	int nBone = pCharacter->GetModel()->GetBoneByName(bone);
	if (nBone >= 0)
		pCharacter->DetachAllFromBone(nBone);
}

void ModelViewPanel::OnBnClickedDetachAll()
{
	m_modelView->GetCharacter()->DetachAll();
}

void ModelViewPanel::OnBnClickedStopAnimation()
{
	int nLayer = GetLayer();
	m_modelView->StopAnimation(nLayer);
}

void ModelViewPanel::OnBnClickedResetAnimations()
{
	m_modelView->GetCharacter()->ResetAnimations();
}


//////////////////////////////////////////////////////////////////////////
void ModelViewPanel::OnEnChangeBone()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

//////////////////////////////////////////////////////////////////////////
void ModelViewPanel::OnBnClickedBrowseObject()
{
	CString relFileName;
	if (CFileUtil::SelectSingleFile( EFILE_TYPE_GEOMETRY,relFileName ))
	{
		m_objectName.SetWindowText( relFileName );
	}
}

//////////////////////////////////////////////////////////////////////////
void ModelViewPanel::ClearBones()
{
	m_boneName.ResetContent();
}

//////////////////////////////////////////////////////////////////////////
void ModelViewPanel::AddBone( const CString &bone )
{
	m_boneName.AddString( bone );
}

//////////////////////////////////////////////////////////////////////////
void ModelViewPanel::SelectBone( const CString &bone )
{
	m_boneName.SelectString( -1,bone );
}

//////////////////////////////////////////////////////////////////////////
void ModelViewPanel::OnBnClickedBrowseFile()
{
	CString file;
	if (CFileUtil::SelectSingleFile( EFILE_TYPE_GEOMETRY,file ))
	{
		m_modelView->LoadObject( file,1 );
		m_fileEdit.SetWindowText( file );
	}
}

//////////////////////////////////////////////////////////////////////////
void ModelViewPanel::SetFileName( const CString &filename )
{
	m_fileEdit.SetWindowText( filename );
}

//////////////////////////////////////////////////////////////////////////
void ModelViewPanel::OnBnClickedReloadFile()
{
	CString filename;
	m_fileEdit.GetWindowText( filename );
	m_modelView->LoadObject( filename,1 );
}
