// InfoBar.cpp : implementation file
//

#include "stdafx.h"
#include "InfoBar.h"
#include "EditTool.h"
#include "Objects\ObjectManager.h"
#include "DisplaySettings.h"
#include "Settings.h"
#include "GameEngine.h"

#include <IConsole.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CInfoBar, CDialog)
	//{{AFX_MSG_MAP(CInfoBar)
	ON_BN_CLICKED(IDC_VECTOR_LOCK, OnVectorLock)
	ON_BN_CLICKED(ID_LOCK_SELECTION, OnLockSelection)
	ON_UPDATE_COMMAND_UI(ID_LOCK_SELECTION, OnUpdateLockSelection)
	ON_EN_UPDATE(IDC_MOVE_SPEED, OnUpdateMoveSpeed)
	ON_EN_UPDATE(IDC_POSX,OnVectorUpdateX)
	ON_EN_UPDATE(IDC_POSY,OnVectorUpdateY)
	ON_EN_UPDATE(IDC_POSZ,OnVectorUpdateZ)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_TERRAIN_COLLISION, OnBnClickedTerrainCollision)
	ON_BN_CLICKED(IDC_PHYSICS, OnBnClickedPhysics)
	ON_BN_CLICKED(IDC_SYNCPLAYER, OnBnClickedSyncplayer)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfoBar dialog
CInfoBar::CInfoBar()
{
	//{{AFX_DATA_INIT(CInfoBar)
	//}}AFX_DATA_INIT

	m_enabledVector = false;
	m_bVectorLock = false;
	m_prevEditMode = 0;
	m_bSelectionLocked = false;
	m_editTool = 0;
	m_bEditingMode = false;
	m_prevMoveSpeed = 0;
}

void CInfoBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInfoBar)
	DDX_Control(pDX, IDC_VECTOR_LOCK, m_vectorLock);
	DDX_Control(pDX, ID_LOCK_SELECTION, m_lockSelection);
	DDX_Control(pDX, IDC_TERRAIN_COLLISION, m_terrainCollision);
	DDX_Control(pDX, IDC_VECTOR_LOCK, m_vectorLock);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_PHYSICS, m_physicsBtn);
	DDX_Control(pDX, IDC_SYNCPLAYER, m_syncPlayerBtn);
}

void CInfoBar::OnVectorUpdateX()
{
	if (m_bVectorLock)
	{
		Vec3 v = GetVector();
		SetVector( Vec3(v.x,v.x,v.x) );
	}
	OnVectorUpdate(false);
}

void CInfoBar::OnVectorUpdateY()
{
	if (m_bVectorLock)
	{
		Vec3 v = GetVector();
		SetVector( Vec3(v.y,v.y,v.y) );
	}
	OnVectorUpdate(false);
}

void CInfoBar::OnVectorUpdateZ()
{
	if (m_bVectorLock)
	{
		Vec3 v = GetVector();
		SetVector( Vec3(v.z,v.z,v.z) );
	}
	OnVectorUpdate(false);
}

void CInfoBar::OnVectorUpdate( bool followTerrain )
{
	int emode = GetIEditor()->GetEditMode();
	if (emode != eEditModeMove && emode != eEditModeRotate && emode != eEditModeScale)
		return;

	CSelectionGroup *selection = GetIEditor()->GetObjectManager()->GetSelection();
	if (selection->IsEmpty()) {
		return;
	}

	GetIEditor()->RestoreUndo();

	Vec3 v = GetVector();

	bool bWorldSpace = GetIEditor()->GetReferenceCoordSys() == COORDS_WORLD;

	CBaseObject *obj = GetIEditor()->GetSelectedObject();

	Matrix44 tm;
	AffineParts ap;
	if (obj)
	{
		tm = obj->GetWorldTM();
		ap.SpectralDecompose(tm);
	}

	if (emode == eEditModeMove)
	{
		if (obj)
		{
			if (bWorldSpace)
			{
				tm.SetTranslationOLD(v);
				obj->SetWorldTM(tm);
			}
			else
				obj->SetPos( v );
		} else {
			GetIEditor()->GetSelection()->Move( v,followTerrain,bWorldSpace );
		}
	}
	if (emode == eEditModeRotate)
	{
		if (obj)
		{
			if (bWorldSpace)
			{
				tm.SetIdentity();

				tm=Matrix44::CreateRotationZYX(-v*gf_DEGTORAD)*tm; //NOTE: angles in radians and negated 

				tm=Matrix33::CreateScale( Vec3d(ap.scale.x,ap.scale.y,ap.scale.z) )*tm;
				
				tm.SetTranslationOLD(ap.pos);
				obj->SetWorldTM(tm);
			}
			else
				obj->SetAngles( v );
		} else {
			GetIEditor()->GetSelection()->Rotate( v,bWorldSpace );
		}
	}
	if (emode == eEditModeScale)
	{
		if (v.x == 0 || v.y == 0 || v.z == 0)
			return;

		if (obj)
		{
			if (bWorldSpace)
			{
				tm.SetIdentity();

				tm= GetTransposed44( Matrix44(ap.rot) );
				tm=Matrix33::CreateScale( Vec3d(v.x,v.y,v.z) ) * tm;

				tm.SetTranslationOLD(ap.pos);
				obj->SetWorldTM(tm);
			}
			else
				obj->SetScale( v );
		} else {
			GetIEditor()->GetSelection()->Scale( v,bWorldSpace );
		}
	}
}

void CInfoBar::IdleUpdate()
{
	bool updateUI = false;
	// Update Width/Height of selection rectangle.
	BBox box;
	GetIEditor()->GetSelectedRegion( box );
	float width = box.max.x - box.min.x;
	float height = box.max.y - box.min.y;
	if (m_width != width || m_height != height)
	{
		m_width = width;
		m_height = height;
		updateUI = true;
	}

	Vec3 marker = GetIEditor()->GetMarkerPosition();

	/*
	// Get active viewport.
	int hx = marker.x / 2;
	int hy = marker.y / 2;
	if (m_heightMapX != hx || m_heightMapY != hy)
	{
		m_heightMapX = hx;
		m_heightMapY = hy;
		updateUI = true;
	}
	*/

	bool bWorldSpace = GetIEditor()->GetReferenceCoordSys() == COORDS_WORLD;
	
	CSelectionGroup *selection = GetIEditor()->GetSelection();
	if (selection->GetCount() != m_numSelected)
	{
		m_numSelected = selection->GetCount();
		updateUI = true;
	}

	if (GetIEditor()->GetEditTool() != m_editTool)
	{
		updateUI = true;
		m_editTool = GetIEditor()->GetEditTool();
	}

	CString str;
	if (m_editTool)
	{
		str = m_editTool->GetStatusText();
		if (str!=m_sLastText)
			updateUI=true;
	}

	if (updateUI)
	{
		if (!m_editTool)
		{
			if (m_numSelected == 0)
				str = "None Selected";
			else if (m_numSelected == 1)
				str = "1 Object Selected";
			else
				str.Format( "%d Objects Selected",m_numSelected );
		}
		SetDlgItemText( IDC_INFO_WH,str );
		m_sLastText=str;
	}

	if (gSettings.cameraMoveSpeed != m_prevMoveSpeed)
	{
		m_prevMoveSpeed = gSettings.cameraMoveSpeed;
		m_moveSpeed.SetValue( gSettings.cameraMoveSpeed );
	}

	{
		int settings = GetIEditor()->GetDisplaySettings()->GetSettings();
		bool noCollision = settings & SETTINGS_NOCOLLISION;
		if ((m_terrainCollision.GetCheck() == 1 && !noCollision) ||
				(m_terrainCollision.GetCheck() == 0 && noCollision))
		{
			m_terrainCollision.SetCheck( (noCollision)?BST_CHECKED:BST_UNCHECKED );
		}

		bool bPhysics = GetIEditor()->GetGameEngine()->GetSimulationMode();
		if ((m_physicsBtn.GetCheck() == 1 && !bPhysics) ||
				(m_physicsBtn.GetCheck() == 0 && bPhysics))
		{
			m_physicsBtn.SetCheck( (bPhysics)?BST_CHECKED:BST_UNCHECKED );
		}

		bool bSyncPlayer = GetIEditor()->GetGameEngine()->IsSyncPlayerPosition();
		if ((m_syncPlayerBtn.GetCheck() == 0 && !bSyncPlayer) ||
				(m_syncPlayerBtn.GetCheck() == 1 && bSyncPlayer))
		{
			m_syncPlayerBtn.SetCheck( (!bSyncPlayer)?BST_CHECKED:BST_UNCHECKED );
		}
	}

	bool bSelLocked = GetIEditor()->IsSelectionLocked();
	if (bSelLocked != m_bSelectionLocked)
	{
		m_bSelectionLocked = bSelLocked;
		m_lockSelection.SetCheck( (m_bSelectionLocked)?1:0 );
	}


	if (GetIEditor()->GetSelection()->IsEmpty())
	{
		if (m_lockSelection.IsWindowEnabled())
			m_lockSelection.EnableWindow( FALSE );
	}
	else
	{
		if (!m_lockSelection.IsWindowEnabled())
			m_lockSelection.EnableWindow( TRUE );
	}

	//////////////////////////////////////////////////////////////////////////
	// Update vector.
	//////////////////////////////////////////////////////////////////////////

	if (selection->IsEmpty())
	{
		// Show marker position.
		EnableVector(false);
		SetVector( marker );
		SetVectorRange( -100000,100000 );
		return;
	}

	Vec3 v(0,0,0);

	CBaseObject *obj = GetIEditor()->GetSelectedObject();

	int emode = GetIEditor()->GetEditMode();

	bool enable = false;
	if (obj)
	{
		v = obj->GetWorldPos();
	}
	float min=0,max=10000;

	if (emode == eEditModeMove)
	{
		if (obj)
		{
			if (bWorldSpace)
				v = obj->GetWorldTM().GetTranslationOLD();
			else
				v = obj->GetPos();
		}
		enable = true;
		min = -10000;
		max = 10000;
	}
	if (emode == eEditModeRotate)
	{
		if (obj)
		{
			if (bWorldSpace)
			{
				AffineParts ap;
				ap.SpectralDecompose(obj->GetWorldTM());
				v = RAD2DEG(Ang3::GetAnglesXYZ(Matrix33(ap.rot)));
			}
			else
				v = obj->GetAngles();
		}
		enable = true;
		min = -10000;
		max = 10000;
	}
	if (emode == eEditModeScale)
	{
		if (obj)
		{
			if (bWorldSpace)
			{
				AffineParts ap;
				ap.SpectralDecompose(obj->GetWorldTM());
				v = ap.scale;
			}
			else
				v = obj->GetScale();
		}
		enable = true;
		min = -10000;
		max = 10000;
	}

	// If Edit mode changed.
	if (m_prevEditMode != emode)
	{
		// Scale mode enables vector lock.
		m_bVectorLock = emode == eEditModeScale;
		m_vectorLock.SetCheck( (m_bVectorLock)?1:0 );

		// Change undo strings.
		CString undoString = "Modify Object(s)";
		int mode = GetIEditor()->GetEditMode();
		switch (mode)
		{
		case eEditModeMove:
			undoString = "Move Object(s)";
			break;
		case eEditModeRotate:
			undoString = "Rotate Object(s)";
			break;
		case eEditModeScale:
			undoString = "Scale Object(s)";
			break;
		}
		m_posCtrl[0].EnableUndo( undoString );
		m_posCtrl[1].EnableUndo( undoString );
		m_posCtrl[2].EnableUndo( undoString );
	}

	SetVectorRange( min,max );
	EnableVector(enable);
	
	//if (!m_bEditingMode)
	SetVector(v);

	m_prevEditMode = emode;
}

void CInfoBar::SetVector( const Vec3 &v )
{
	m_posCtrl[0].SetValue(v.x);
	m_posCtrl[1].SetValue(v.y);
	m_posCtrl[2].SetValue(v.z);
}
	
Vec3 CInfoBar::GetVector()
{
	Vec3 v;
	v.x = m_posCtrl[0].GetValue();
	v.y = m_posCtrl[1].GetValue();
	v.z = m_posCtrl[2].GetValue();
	return v;
}

void CInfoBar::EnableVector( bool enable )
{
	if (m_enabledVector != enable)
	{
		m_enabledVector = enable;
		m_posCtrl[0].EnableWindow(enable);
		m_posCtrl[1].EnableWindow(enable);
		m_posCtrl[2].EnableWindow(enable);

		m_vectorLock.EnableWindow(enable);
	}
}
void CInfoBar::SetVectorRange( float min,float max )
{
	m_posCtrl[0].SetRange( min,max );
	m_posCtrl[1].SetRange( min,max );
	m_posCtrl[2].SetRange( min,max );
}


void CInfoBar::OnVectorLock() 
{
	// TODO: Add your control notification handler code here
	m_bVectorLock = !m_vectorLock.GetCheck();
	m_vectorLock.SetCheck(m_bVectorLock);

	/*
	//GetDlgItem(ID_LOCK_SELECTION)->ModifyStyle(0,BS_OWNERDRAW);
	m_lockSelBtn.SubclassDlgItem( ID_LOCK_SELECTION,this );
	m_lockSelBtn.LoadBitmaps( IDB_LOCK_SELECTION_UP,IDB_LOCK_SELECTION_DOWN,IDB_LOCK_SELECTION_FOCUS,IDB_LOCK_SELECTION_DISABLE );
	m_lockSelBtn.SizeToContent();
	*/
}

void CInfoBar::OnLockSelection() 
{
	// TODO: Add your control notification handler code here
	BOOL locked = !m_lockSelection.GetCheck();
	m_lockSelection.SetCheck(locked);
	m_bSelectionLocked = (locked)?true:false;
	GetIEditor()->LockSelection( m_bSelectionLocked );
}

void CInfoBar::OnUpdateLockSelection(CCmdUI* pCmdUI)
{
	if (GetIEditor()->GetSelection()->IsEmpty())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);

	bool bSelLocked = GetIEditor()->IsSelectionLocked();
	pCmdUI->SetCheck( (bSelLocked)?1:0 );
}

void CInfoBar::OnUpdateMoveSpeed() 
{
	gSettings.cameraMoveSpeed = m_moveSpeed.GetValue();
}

BOOL CInfoBar::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_posCtrl[0].Create( this,IDC_POSX );
	m_posCtrl[1].Create( this,IDC_POSY );
	m_posCtrl[2].Create( this,IDC_POSZ );

	//m_posCtrl[0].SetBeginUpdateCallback( functor(*this,&CInfoBar::OnBeginVectorUpdate) );
	//m_posCtrl[1].SetBeginUpdateCallback( functor(*this,&CInfoBar::OnBeginVectorUpdate) );
	//m_posCtrl[2].SetBeginUpdateCallback( functor(*this,&CInfoBar::OnBeginVectorUpdate) );
	//m_posCtrl[0].SetEndUpdateCallback( functor(*this,&CInfoBar::OnEndVectorUpdate) );
	//m_posCtrl[1].SetEndUpdateCallback( functor(*this,&CInfoBar::OnEndVectorUpdate) );
	//m_posCtrl[2].SetEndUpdateCallback( functor(*this,&CInfoBar::OnEndVectorUpdate) );

	m_posCtrl[0].EnableWindow(FALSE);
	m_posCtrl[1].EnableWindow(FALSE);
	m_posCtrl[2].EnableWindow(FALSE);

	m_moveSpeed.Create( this,IDC_MOVE_SPEED );
	m_moveSpeed.SetRange( 0.01f,100 );

	GetDlgItem(IDC_INFO_WH)->SetFont( CFont::FromHandle( (HFONT)::GetStockObject(DEFAULT_GUI_FONT)) );
	SetDlgItemText(IDC_INFO_WH,"");

	m_vectorLock.SetIcon( MAKEINTRESOURCE(IDI_LOCK) );
	m_terrainCollision.SetIcon( MAKEINTRESOURCE(IDI_TERRAIN_COLLISION) );
	m_syncPlayerBtn.SetToolTip( "Synchronize Player with Camera" );

	/*
	CRect rc;
	GetDlgItem(IDC_TOOLBAR)->GetWindowRect(rc);
	ScreenToClient(rc);
	//m_toolBar.Create( WS_CHILD|WS_VISIBLE,rc,this,IDC_TOOLBAR );
	//m_toolBar.
	//m_toolBar.LoadImages()
 
	m_toolBar.CreateEx( this,TBSTYLE_FLAT, WS_CHILD|WS_VISIBLE|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_FIXED);
	m_toolBar.LoadToolBar(IDR_ANIMATION);
	m_toolBar.MoveWindow( rc );
	*/

	//GetDlgItem(ID_LOCK_SELECTION)->EnableWindow(TRUE);

	m_moveSpeed.SetValue( gSettings.cameraMoveSpeed );
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInfoBar::OnBeginVectorUpdate( CNumberCtrl *ctrl )
{
	if (!GetIEditor()->IsUndoRecording())
	{
		CLogFile::WriteLine( "Begin Undo" );
		GetIEditor()->BeginUndo();
		m_bEditingMode = true;
	}
}
	
void CInfoBar::OnEndVectorUpdate( CNumberCtrl *ctrl )
{
	CString str = "Modify Object(s)";
	int mode = GetIEditor()->GetEditMode();
	switch (mode)
	{
		case eEditModeMove:
			str = "Move Object(s)";
			break;
		case eEditModeRotate:
			str = "Rotate Object(s)";
			break;
		case eEditModeScale:
			str = "Scale Object(s)";
			break;
	}
	if (GetIEditor()->IsUndoRecording())
	{
		GetIEditor()->AcceptUndo( str );
	}
	m_bEditingMode = false;
	///IdleUpdate();
}

void CInfoBar::OnBnClickedTerrainCollision()
{
	AfxGetMainWnd()->SendMessage( WM_COMMAND,MAKEWPARAM(ID_TERRAIN_COLLISION,0),0 );
}

void CInfoBar::OnBnClickedPhysics()
{
	AfxGetMainWnd()->SendMessage( WM_COMMAND,MAKEWPARAM(ID_SWITCH_PHYSICS,0),0 );
}

//////////////////////////////////////////////////////////////////////////
void CInfoBar::OnBnClickedSyncplayer()
{
	AfxGetMainWnd()->SendMessage( WM_COMMAND,MAKEWPARAM(ID_GAME_SYNCPLAYER,0),0 );
}
