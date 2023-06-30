// Viewport.cpp: implementation of the CViewport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ViewManager.h"
#include "Viewport.h"
#include "EditTool.h"
#include "Heightmap.h"
#include "Settings.h"

#include "Util\AVI_Writer.h"
#include "Objects\ObjectManager.h"
#include <ITimer.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CViewport,CWnd)

BEGIN_MESSAGE_MAP (CViewport, CWnd)
	//{{AFX_MSG_MAP(CViewport)
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
	ON_WM_SETCURSOR()
END_MESSAGE_MAP ()

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
bool CViewport::m_bDegradateQuality = false;

CViewport::CViewport()
{
	m_cViewMenu.LoadMenu(IDR_VIEW_OPTIONS);

	m_selectionTollerance = 0;

	// View mode
	m_eViewMode = NothingMode;

	//////////////////////////////////////////////////////////////////////////
	// Init standart cursors.
	//////////////////////////////////////////////////////////////////////////
	m_hCurrCursor = NULL;
	m_hDefaultCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	m_hHitCursor = AfxGetApp()->LoadCursor(IDC_POINTER_OBJHIT);
	m_hMoveCursor = AfxGetApp()->LoadCursor(IDC_POINTER_OBJECT_MOVE);
	m_hRotateCursor = AfxGetApp()->LoadCursor(IDC_POINTER_OBJECT_ROTATE);
	m_hScaleCursor = AfxGetApp()->LoadCursor(IDC_POINTER_OBJECT_SCALE);
	m_hSelectionPlusCursor = AfxGetApp()->LoadCursor(IDC_POINTER_PLUS);
	m_hSelectionMinusCursor = AfxGetApp()->LoadCursor(IDC_POINTER_MINUS);

	m_activeAxis = AXIS_TERRAIN;

	m_constructionOriginalMatrix.SetIdentity();
	m_constructionMatrix.SetIdentity();
	m_constructionViewTM.SetIdentity();
	m_viewTM.SetIdentity();

	m_bRMouseDown = false;

	m_pMouseOverObject = 0;

	m_pAVIWriter = 0;
	m_bAVICreation = false;
	m_bAVIPaused = false;
}

CViewport::~CViewport()
{
	StopAVIRecording();
}

void CViewport::OnDestroy() 
{
	////////////////////////////////////////////////////////////////////////
	// Remove the view from the list
	////////////////////////////////////////////////////////////////////////
	CWnd::OnDestroy();

	//GetIEditor()->GetViewManager()->UnregisterView( this ); // Register the view
}

bool CViewport::Create(CWnd *hWndParent, int id,const char *szTitle )
{
	////////////////////////////////////////////////////////////////////////
	// Create the window with a custom window class, load the cross cursor.
	// Also create a render window with the requested type. This function
	// is intended to be called only once
	////////////////////////////////////////////////////////////////////////

	RECT rcDefault = {0,0,0,0};
	bool bReturn;

	m_name = szTitle;

	// Create the window
	/*
	bReturn = CreateEx(NULL, AfxRegisterWndClass(CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW, 
		AfxGetApp()->LoadStandardCursor(IDC_CROSS), NULL, NULL), szTitle, WS_CHILD
		| WS_VISIBLE | WS_CLIPCHILDREN, rcDefault, hWndParent, NULL);
		*/

	int wndStyle = (hWndParent != 0) ? WS_CHILD : WS_POPUP;

	bReturn = CreateEx(NULL, AfxRegisterWndClass(CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW|CS_OWNDC, 
		AfxGetApp()->LoadStandardCursor(IDC_ARROW), NULL, NULL), szTitle,wndStyle|WS_CLIPCHILDREN,
		rcDefault, hWndParent, NULL);

	ShowWindow( SW_HIDE );

	ASSERT(bReturn);
 
	return bReturn;
}

//////////////////////////////////////////////////////////////////////////
void CViewport::SetActive( bool bActive )
{
	m_bActive = bActive;
	if (!m_bActive)
	{
		// If any AVI recording goes on, turn it off.
		if (IsAVIRecording())
			StopAVIRecording();
	}
}

//////////////////////////////////////////////////////////////////////////
bool CViewport::IsActive() const
{
	return m_bActive;
}


//////////////////////////////////////////////////////////////////////////
BOOL CViewport::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	// TODO: Add your message handler code here and/or call default
	float z = GetZoomFactor() + (zDelta / 120.0f) * 0.5f;

	SetZoomFactor( z );

	GetIEditor()->GetViewManager()->SetZoomFactor( z );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
CString CViewport::GetName() const
{
	return m_name;
}

//////////////////////////////////////////////////////////////////////////
void CViewport::SetName( const CString &name )
{
	m_name = name;
	if (IsWindow(GetSafeHwnd()))
		SetWindowText(name);
}

//////////////////////////////////////////////////////////////////////////
CCryEditDoc* CViewport::GetDocument()
{
	return GetIEditor()->GetDocument();
}

//////////////////////////////////////////////////////////////////////////
BOOL CViewport::OnEraseBkgnd(CDC* pDC) 
{
	////////////////////////////////////////////////////////////////////////
	// Erase the background of the window (only if no render has been
	// attached)
	////////////////////////////////////////////////////////////////////////

	RECT rect;
	CBrush cFillBrush;
	
	// Get the rect of the client window
	GetClientRect(&rect);
	
	// Create the brush
	cFillBrush.CreateSolidBrush(0x00F0F0F0);

	// Fill the entire client area
	pDC->FillRect(&rect, &cFillBrush);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void CViewport::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	
	RECT rect;
	// Get the rect of the client window
	GetClientRect(&rect);
	
	// Create the brush
	CBrush cFillBrush;
	cFillBrush.CreateSolidBrush(0x00F0F0F0);

	// Fill the entire client area
	dc.FillRect(&rect, &cFillBrush);
}

//////////////////////////////////////////////////////////////////////////
void CViewport::OnActivate()
{
	////////////////////////////////////////////////////////////////////////
	// Make this edit window the current one
	////////////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////////////
void CViewport::OnDeactivate()
{
}

//////////////////////////////////////////////////////////////////////////
void CViewport::ResetContent()
{
	m_pMouseOverObject = 0;
}

//////////////////////////////////////////////////////////////////////////
void CViewport::UpdateContent( int flags )
{
	UpdateConstrPlane();
}

//////////////////////////////////////////////////////////////////////////
void CViewport::Update()
{
	// Record AVI.
	if (m_pAVIWriter)
		AVIRecordFrame();
}

//////////////////////////////////////////////////////////////////////////
CPoint CViewport::WorldToView( Vec3 wp )
{
	CPoint p;
	p.x = wp.x;
	p.y = wp.y;
	return p;
}

//////////////////////////////////////////////////////////////////////////
Vec3	CViewport::ViewToWorld( CPoint vp,bool *collideWithTerrain,bool onlyTerrain )
{
	Vec3d wp;
	wp.x = vp.x;
	wp.y = vp.y;
	wp.z = 0;
	if (collideWithTerrain)
		*collideWithTerrain = true;
	return wp;
}

void	CViewport::ViewToWorldRay( CPoint vp,Vec3 &raySrc,Vec3 &rayDir )
{
	raySrc(0,0,0);
	rayDir(0,0,-1);
}

void CViewport::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// CPointF ptMarker;
	CPoint ptCoord;
	int iCurSel = -1;
	RECT rcClient;

	if (GetIEditor()->IsInGameMode())
	{
		// Ignore clicks while in game.
		return;
	}

	GetClientRect(&rcClient);
	// Save the mouse down position
	m_cMouseDownPos = point;

	// Check Edit Tool.
	if (GetIEditor()->GetEditTool())
	{
		if (GetIEditor()->GetEditTool()->MouseCallback( this,eMouseLDown,point,nFlags ))
		{
			return;
		}
	}

	ResetSelectionRegion();

	Vec3d pos = SnapToGrid( ViewToWorld( point ) );
	GetIEditor()->SetMarkerPosition( pos );

	// Show marker position in the status bar
	//sprintf(szNewStatusText, "X:%g Y:%g Z:%g",pos.x,pos.y,pos.z );

	// Swap X/Y
	int unitSize = 1;
	CHeightmap *pHeightmap = GetIEditor()->GetHeightmap();
	if (pHeightmap)
		unitSize = pHeightmap->GetUnitSize();
	float hx = pos.y / unitSize;
	float hy = pos.x / unitSize;
	float hz = GetIEditor()->GetTerrainElevation(pos.x,pos.y);

	char szNewStatusText[512];
	sprintf(szNewStatusText, "Heightmap Coordinates: HX:%g HY:%g HZ:%g",hx,hy,hz );
	GetIEditor()->SetStatusText(szNewStatusText);


	// Get contrl key status.
	bool bAltClick = CheckVirtualKey(VK_MENU);
	bool bCtrlClick = (nFlags & MK_CONTROL);
	bool bShiftClick = (nFlags & MK_SHIFT);
	bool bNoRemoveSelection = bCtrlClick || bAltClick;
	bool bUnselect = bAltClick;

	bool bLockSelection = GetIEditor()->IsSelectionLocked();
	
	int numUnselected = 0;
	int numSelected = 0;

//	m_activeAxis = 0;

	ObjectHitInfo hitInfo(this,point);
	if (bAltClick || bCtrlClick || bShiftClick)
	{
		// If adding or removing selection from the object, ignore hitting selection axis.
		hitInfo.bIgnoreAxis = true;
	}
	if (HitTest( point,hitInfo ))
	{
		//if (hitInfo.axis != 0)
			//GetIEditor()->SetAxisConstrains( (AxisConstrains)hitInfo.axis );
		if (hitInfo.axis != 0)
			SetAxisConstrain( hitInfo.axis );
	}
	CBaseObject *hitObj = hitInfo.object;

	int editMode = GetIEditor()->GetEditMode();

	if (hitObj)
	{
		Matrix44 tm = hitInfo.object->GetWorldTM();
		SetConstrPlane( point,tm );
	}
	else
	{
		Matrix44 tm;
		tm.SetIdentity();
		tm.SetTranslationOLD( pos );
		SetConstrPlane( point,tm );
	}

	if (editMode == eEditModeMove)
	{
		SetViewMode( MoveMode );
		// Check for Move to position.
		if (bCtrlClick && bShiftClick)
		{
			// Ctrl-Click on terain will move selected objects to specified location.
			MoveSelectionToPos( pos );
			bLockSelection = true;
		}

		if (hitObj && hitObj->IsSelected() && !bNoRemoveSelection)
			bLockSelection = true;
	}
	else  if (editMode == eEditModeRotate)
	{
		SetViewMode( RotateMode );
		if (hitObj && hitObj->IsSelected() && !bNoRemoveSelection)
			bLockSelection = true;
	}
	else if (editMode == eEditModeScale)
	{
		SetViewMode( ScaleMode );
		if (hitObj && hitObj->IsSelected() && !bNoRemoveSelection)
			bLockSelection = true;
	}
	else if (hitObj != 0 && GetIEditor()->GetSelectedObject() == hitObj && !bCtrlClick && !bUnselect)
	{
		bLockSelection = true;
	}

	if (!bLockSelection)
	{
		// If not selection locked.
		BeginUndo();

		if (!bNoRemoveSelection)
		{
			// Current selection should be cleared
			numUnselected = GetIEditor()->GetObjectManager()->ClearSelection();
		}
		
		if (hitObj)
		{
			numSelected = 1;

			if (!bUnselect)
			{
				if (hitObj->IsSelected())
					bUnselect = true;
			}
			
			if (!bUnselect)
				GetIEditor()->GetObjectManager()->SelectObject( hitObj );
			else
				GetIEditor()->GetObjectManager()->UnselectObject( hitObj );
		}
		if (IsUndoRecording())
			AcceptUndo( "Select Object(s)" );
		
		if (numSelected == 0 || editMode == eEditModeSelect)
		{
			// If object is not selected.
			// Capture mouse input for this window.
			SetViewMode( SelectMode );
		}
	}

	if (GetViewMode() == MoveMode ||
			GetViewMode() == RotateMode ||
			GetViewMode() == ScaleMode)
	{
		BeginUndo();
	}

	//////////////////////////////////////////////////////////////////////////
	// Change cursor, must be before Capture mouse.
	//////////////////////////////////////////////////////////////////////////
	SetObjectCursor(hitObj,true);

	//////////////////////////////////////////////////////////////////////////
	CaptureMouse();
	//////////////////////////////////////////////////////////////////////////
	
	CWnd::OnLButtonDown(nFlags, point);
}

void CViewport::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (GetIEditor()->IsInGameMode())
	{
		// Ignore clicks while in game.
		return;
	}

	// Check Edit Tool.
	if (GetIEditor()->GetEditTool())
	{
		if (GetIEditor()->GetEditTool()->MouseCallback( this,eMouseLUp,point,nFlags ))
		{
			return;
		}
	}

	// Reset the status bar caption
	GetIEditor()->SetStatusText("Ready");

	//////////////////////////////////////////////////////////////////////////
	if (IsUndoRecording())
	{
		if (GetViewMode() == MoveMode)
		{
			AcceptUndo( "Move Selection" );
		}
		else if (GetViewMode() == RotateMode)
		{
			AcceptUndo( "Rotate Selection" );
		}
		else if (GetViewMode() == ScaleMode)
		{
			AcceptUndo( "Scale Selection" );
		}
		else
		{
			CancelUndo();
		}
	}
	//////////////////////////////////////////////////////////////////////////

	if (GetViewMode() == SelectMode && (!GetIEditor()->IsSelectionLocked()))
	{
		bool bUnselect = CheckVirtualKey(VK_MENU);
		if (!m_selectedRect.IsRectEmpty())
			SelectObjectsInRect( m_selectedRect,!bUnselect );

		if (GetIEditor()->GetEditMode() == eEditModeSelectArea)
		{
			BBox box;
			GetIEditor()->GetSelectedRegion( box );

			if (fabs(box.min.x-box.max.x) > 0.5f && fabs(box.min.y-box.max.y) > 0.5f)
			{
				//@FIXME: restore it later.
				//Timur[1/14/2003]
				//SelectRectangle( box,!bUnselect );
				//SelectObjectsInRect( m_selectedRect,!bUnselect );
				GetIEditor()->GetObjectManager()->SelectObjects( box,bUnselect );
				GetIEditor()->UpdateViews(eUpdateObjects);
			}
		}
	}
	// Release the restriction of the cursor
	ReleaseMouse();
	
	if (GetIEditor()->GetEditMode() != eEditModeSelectArea)
	{
		ResetSelectionRegion();
	}
	// Reset selected rectangle.
	m_selectedRect.SetRectEmpty();

	// Restore default editor axis constrain.
	if (GetIEditor()->GetAxisConstrains() != GetAxisConstrain())
	{
		SetAxisConstrain( GetIEditor()->GetAxisConstrains() );
		SetConstrPlane( point,m_constructionOriginalMatrix );
	}

	SetViewMode( NothingMode );
	
	CWnd::OnLButtonUp(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CViewport::OnMButtonDown(UINT nFlags, CPoint point) 
{
	if (GetIEditor()->IsInGameMode())
	{
		// Ignore double clicks while in game.
		return;
	}

	// Move the viewer to the mouse location.
	// Check Edit Tool.
	if (GetIEditor()->GetEditTool())
	{
		if (GetIEditor()->GetEditTool()->MouseCallback( this,eMouseMDown,point,nFlags ))
		{
			return;
		}
	}

	CWnd::OnMButtonDown(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CViewport::OnMButtonUp(UINT nFlags, CPoint point) 
{
	if (GetIEditor()->IsInGameMode())
	{
		// Ignore double clicks while in game.
		return;
	}

	// Move the viewer to the mouse location.
	// Check Edit Tool.
	if (GetIEditor()->GetEditTool())
	{
		if (GetIEditor()->GetEditTool()->MouseCallback( this,eMouseMUp,point,nFlags ))
		{
			return;
		}
	}

	CWnd::OnMButtonDown(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CViewport::OnMButtonDblClk(UINT nFlags, CPoint point) 
{
	if (GetIEditor()->IsInGameMode())
	{
		// Ignore double clicks while in game.
		return;
	}

	// Move the viewer to the mouse location.
	// Check Edit Tool.
	if (GetIEditor()->GetEditTool())
	{
		if (GetIEditor()->GetEditTool()->MouseCallback( this,eMouseMDblClick,point,nFlags ))
		{
			return;
		}
	}

/*
	Vec3d v = ViewToWorld( point );
	Vec3d p = GetIEditor()->GetViewerPos();
	float height = p.z - GetIEditor()->GetTerrainElevation(p.x,p.y);
	if (height < 1) height = 1;
	p.x = v.x;
	p.y = v.y;
	p.z = GetIEditor()->GetTerrainElevation( p.x,p.y ) + height;
	GetIEditor()->SetViewerPos( p );
*/	
	CWnd::OnMButtonDblClk(nFlags, point);
}



void CViewport::OnMouseMove(UINT nFlags, CPoint point)
{
	if (GetIEditor()->IsInGameMode())
	{
		// Ignore while in game.
		return;
	}

	SetObjectCursor(0);

	// Check Edit Tool.
	if (GetIEditor()->GetEditTool())
	{
		if (GetIEditor()->GetEditTool()->MouseCallback( this,eMouseMove,point,nFlags ))
		{
			return;
		}
	}

	Vec3d pos = SnapToGrid( ViewToWorld( point ) );
	
	GetIEditor()->SetMarkerPosition( pos );

	// get world/local coordinate system setting.
	int coordSys = GetIEditor()->GetReferenceCoordSys();

	// get current axis constrains.
	if (GetViewMode() == MoveMode)
	{
		GetIEditor()->RestoreUndo();

		Vec3 v;
		//m_cMouseDownPos = point;
		bool followTerrain = false;
		if (m_activeAxis == AXIS_TERRAIN)
		{
			followTerrain = true;
			Vec3 p1 = SnapToGrid(ViewToWorld( m_cMouseDownPos ));
			Vec3 p2 = SnapToGrid(ViewToWorld( point ));
			v = p2 - p1;
			v.z = 0;
		}
		else
		{
			Vec3 p1 = MapViewToCP(m_cMouseDownPos);
			Vec3 p2 = MapViewToCP(point);
			if (p1.IsZero() || p2.IsZero())
				return;
			v = GetCPVector(p1,p2);

			//Matrix invParent = m_parentConstructionMatrix;
			//invParent.Invert();
			//p1 = invParent.TransformVector(p1);
			//p2 = invParent.TransformVector(p2);
			//v = p2 - p1;
		}

		GetIEditor()->GetSelection()->Move( v,followTerrain, coordSys!=COORDS_LOCAL );
		return;
	}
	else if (GetViewMode() == RotateMode)
	{
		GetIEditor()->RestoreUndo();

		Vec3 ang(0,0,0);
		float ax = point.x - m_cMouseDownPos.x;
		float ay = point.y - m_cMouseDownPos.y;
		switch (m_activeAxis)
		{
			case AXIS_X: ang.x = ay; break;
			case AXIS_Y: ang.y = ay; break;
			case AXIS_Z: ang.z = ay; break;
			case AXIS_XY: ang(ax,ay,0); break;
			case AXIS_XZ: ang(ax,0,ay); break;
			case AXIS_YZ: ang(0,ay,ax); break;
			case AXIS_TERRAIN: ang(ax,ay,0); break;
		};

		ang = m_viewManager->GetGrid()->SnapAngle(ang);

		//m_cMouseDownPos = point;
		GetIEditor()->GetSelection()->Rotate( ang,coordSys!=COORDS_LOCAL );
		return;
	}
	else if (GetViewMode() == ScaleMode)
	{
		GetIEditor()->RestoreUndo();

		Vec3 scl(0,0,0);
		float ay = 1.0f - 0.01f*(point.y - m_cMouseDownPos.y);
		if (ay < 0.01f) ay = 0.01f;
		switch (m_activeAxis)
		{
			case AXIS_X: scl(ay,1,1); break;
			case AXIS_Y: scl(1,ay,1); break;
			case AXIS_Z: scl(1,1,ay); break;
			case AXIS_XY: scl(ay,ay,ay); break;
			case AXIS_XZ: scl(ay,ay,ay); break;
			case AXIS_YZ: scl(ay,ay,ay); break;
			case AXIS_TERRAIN: scl(ay,ay,ay); break;
		};
		// Ignore axis,only uniform scale.
		scl(ay,ay,ay);
		GetIEditor()->GetSelection()->Scale( scl,coordSys!=COORDS_LOCAL );
		return;
	}
	else if (GetViewMode() == SelectMode)
	{
		// Ignore select when selection locked.
		if (GetIEditor()->IsSelectionLocked())
			return;

		CRect rc( m_cMouseDownPos,point );
		if (GetIEditor()->GetEditMode() == eEditModeSelectArea)
			OnDragSelectRectangle( CPoint(rc.left,rc.top),CPoint(rc.right,rc.bottom),false );
		else
		{
			m_selectedRect = rc;
			m_selectedRect.NormalizeRect();
		}
		//else
			//OnDragSelectRectangle( CPoint(rc.left,rc.top),CPoint(rc.right,rc.bottom),true );
	}

	if (!(nFlags & MK_RBUTTON))
	{
		// Track mouse movements.
		ObjectHitInfo hitInfo(this,point);
		if (HitTest( point,hitInfo ))
		{
			SetObjectCursor(hitInfo.object);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CViewport::MoveSelectionToPos( Vec3 &pos )
{
	BeginUndo();
	// Find center of selection.
	Vec3 center = GetIEditor()->GetSelection()->GetCenter();
	GetIEditor()->GetSelection()->Move( pos-center,false,true );
	AcceptUndo( "Move Selection" );
}

//////////////////////////////////////////////////////////////////////////
void CViewport::ResetSelectionRegion()
{
	BBox box( Vec3(0,0,0),Vec3(0,0,0) );
	GetIEditor()->SetSelectedRegion( box );
	m_selectedRect.SetRectEmpty();
}

void CViewport::SetSelectionRectangle( CPoint p1,CPoint p2 )
{
	m_selectedRect = CRect(p1,p2);
	m_selectedRect.NormalizeRect();
}

//////////////////////////////////////////////////////////////////////////
void CViewport::OnDragSelectRectangle( CPoint pnt1,CPoint pnt2,bool bNormilizeRect )
{
	Vec3 org;
	BBox box;
	box.Reset();

	Vec3 p1 = ViewToWorld( pnt1 ); 
	Vec3 p2 = ViewToWorld( pnt2 );
	org = p1;
	// Calculate selection volume.
	if (!bNormilizeRect)
	{
		box.Add( p1 );
		box.Add( p2 );
	} else {
		CRect rc( pnt1,pnt2 );
		rc.NormalizeRect();
		box.Add( ViewToWorld( CPoint(rc.left,rc.top) ));
		box.Add( ViewToWorld( CPoint(rc.right,rc.top) ));
		box.Add( ViewToWorld( CPoint(rc.left,rc.bottom) ));
		box.Add( ViewToWorld( CPoint(rc.right,rc.bottom) ));
	}

	box.min.z = -10000;
	box.max.z = 10000;	
	GetIEditor()->SetSelectedRegion( box );

	// Show marker position in the status bar
	float w = box.max.x - box.min.x;
	float h = box.max.y - box.min.y;
	char szNewStatusText[512];
	sprintf(szNewStatusText, "X:%g Y:%g Z:%g  W:%g H:%g",org.x,org.y,org.z,w,h );
	GetIEditor()->SetStatusText(szNewStatusText);
}

//////////////////////////////////////////////////////////////////////////
void CViewport::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if (GetIEditor()->IsInGameMode())
	{
		// Ignore double clicks while in game.
		return;
	}

	// Check Edit Tool.
	if (GetIEditor()->GetEditTool())
	{
		if (GetIEditor()->GetEditTool()->MouseCallback( this,eMouseLDblClick,point,nFlags ))
		{
			return;
		}
	}

	// If shift clicked, Move the camera to this place.
	if (nFlags & MK_SHIFT)
	{
		// Get the heightmap coordinates for the click position
		Vec3d v = ViewToWorld( point );
		if (!(v.x == 0 && v.y == 0 && v.z == 0))
		{
			Vec3d p = GetIEditor()->GetViewerPos();
			float height = p.z - GetIEditor()->GetTerrainElevation(p.x,p.y);
			if (height < 1) height = 1;
			p.x = v.x;
			p.y = v.y;
			p.z = GetIEditor()->GetTerrainElevation( p.x,p.y ) + height;
			GetIEditor()->SetViewerPos( p );
		}
	}
	else
	{
		// Check if double clicked on object.
		ObjectHitInfo hitInfo(this,point);
		HitTest( point,hitInfo );

		CBaseObject *hitObj = hitInfo.object;
		if (hitObj)
		{
			// Fire double click event on hitted object.
			hitObj->OnEvent( EVENT_DBLCLICK );
		}
	}
	
	CWnd::OnLButtonDblClk(nFlags, point);
}


void CViewport::CaptureMouse()
{
	if (GetCapture() != this)
	{
		SetCapture();
	}
}
	
void CViewport::ReleaseMouse()
{
	if (GetCapture() == this)
	{
		ReleaseCapture();
	}
}

void CViewport::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (GetIEditor()->IsInGameMode())
	{
		// Ignore key downs while in game.
		return;
	}

	if (GetIEditor()->GetEditTool())
	{
		if (GetIEditor()->GetEditTool()->OnKeyDown( this,nChar,nRepCnt,nFlags ))
			return;
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CViewport::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (GetIEditor()->IsInGameMode())
	{
		// Ignore key downs while in game.
		return;
	}

	if (GetIEditor()->GetEditTool())
	{
		if (GetIEditor()->GetEditTool()->OnKeyUp( this,nChar,nRepCnt,nFlags ))
			return;
	}
	
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

//////////////////////////////////////////////////////////////////////////
void CViewport::SetCurrentCursor( HCURSOR hCursor )
{
	m_hCurrCursor = hCursor;
	if (m_hCurrCursor)
		SetCursor( m_hCurrCursor );
	else
		SetCursor( m_hDefaultCursor );
}

//////////////////////////////////////////////////////////////////////////
void CViewport::SetObjectCursor( CBaseObject *hitObj,bool bChangeNow )
{
	//HCURSOR hPrevCursor = m_hCurrCursor;
	if (m_pMouseOverObject)
	{
		m_pMouseOverObject->SetHighlight(false);
	}
	m_pMouseOverObject = hitObj;
	bool bHitSelectedObject = false;
	if (m_pMouseOverObject)
	{
		if (GetViewMode() != SelectMode)
		{
			m_pMouseOverObject->SetHighlight(true);
			m_cursorStr = m_pMouseOverObject->GetName();
			m_hCurrCursor = m_hHitCursor;
			if (m_pMouseOverObject->IsSelected())
				bHitSelectedObject = true;
		}
	}
	else
	{
		m_cursorStr = "";
		m_hCurrCursor = 0;
	}
	// Get contrl key status.
	bool bAltClick = CheckVirtualKey(VK_MENU);
	bool bCtrlClick = CheckVirtualKey(VK_CONTROL);
	bool bShiftClick = CheckVirtualKey(VK_SHIFT);
	bool bNoRemoveSelection = bCtrlClick || bAltClick;
	bool bUnselect = bAltClick;
	bool bLockSelection = GetIEditor()->IsSelectionLocked();

	if (GetViewMode() == SelectMode)
	{
		if (bCtrlClick)
			m_hCurrCursor = m_hSelectionPlusCursor;
		if (bAltClick)
			m_hCurrCursor = m_hSelectionMinusCursor;
	}
	else if ((bHitSelectedObject && !bNoRemoveSelection) || bLockSelection)
	{
		int editMode = GetIEditor()->GetEditMode();
		if (editMode == eEditModeMove)
		{
			m_hCurrCursor = m_hMoveCursor;
		}
		else if (editMode == eEditModeRotate)
		{
			m_hCurrCursor = m_hRotateCursor;
		}
		else if (editMode == eEditModeScale)
		{
			m_hCurrCursor = m_hScaleCursor;
		}
	}
	if (bChangeNow)
	{
		if (GetCapture() == NULL)
		{
			if (m_hCurrCursor)
				SetCursor( m_hCurrCursor );
			else
				SetCursor( m_hDefaultCursor );
		}
	}
}

BOOL CViewport::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// Check Edit Tool.
	if (GetIEditor()->GetEditTool())
	{
		if (GetIEditor()->GetEditTool()->OnSetCursor( this ))
		{
			return TRUE;
		}
	}
	if (m_hCurrCursor != NULL)
	{
		SetCursor( m_hCurrCursor );
		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

//////////////////////////////////////////////////////////////////////////
void CViewport::SetConstrPlane( CPoint cursor,const Vec3& planeOrigin )
{
	Matrix44 tm;
	tm.SetIdentity();
	tm.SetTranslationOLD(planeOrigin);
	SetConstrPlane(cursor,tm);
}

//////////////////////////////////////////////////////////////////////////
void CViewport::SetConstrPlane( CPoint cursor,const Matrix44 &xform )
{
	Vec3 raySrc(0,0,0),rayDir(1,0,0);
	ViewToWorldRay( cursor,raySrc,rayDir );
	//Vec3 rayDir = (xform.GetTranslationOLD() - m_viewTM..GetTranslationOLD()).GetNormalized();

	int axis = GetAxisConstrain();
	int coordSys = GetIEditor()->GetReferenceCoordSys();

	Vec3 xAxis(1,0,0);
	Vec3 yAxis(0,1,0);
	Vec3 zAxis(0,0,1);

	m_constructionOriginalMatrix = xform;
	m_constructionMatrix = xform;
	// Remove scale component.
	m_constructionMatrix.NoScale();
	Vec3 pos = m_constructionMatrix.GetTranslationOLD();
	// Remove position component.
	m_constructionMatrix.SetTranslationOLD( Vec3(0,0,0) );

	if (coordSys == COORDS_LOCAL)
	{
		xAxis = GetTransposed44(m_constructionMatrix)*xAxis;
		yAxis = GetTransposed44(m_constructionMatrix)*yAxis;
		zAxis = GetTransposed44(m_constructionMatrix)*zAxis;
	}

	if (axis == AXIS_X)
	{
		// X direction.
		Vec3 n;
		float d1 = fabs(rayDir.Dot(yAxis));
		float d2 = fabs(rayDir.Dot(zAxis));
		if (d1 > d2) n = yAxis; else n = zAxis;

		//Vec3 n = Vec3(0,0,1);
		Vec3 v1 = n.Cross( xAxis );
		Vec3 v2 = n.Cross( v1 );
		m_constructionPlane.Init( pos,pos+v1,pos+v2 );
	}
	else if (axis == AXIS_Y)
	{
		// Y direction.
		Vec3 n;
		float d1 = fabs(rayDir.Dot(xAxis));
		float d2 = fabs(rayDir.Dot(zAxis));
		if (d1 > d2) n = xAxis; else n = zAxis;

		Vec3 v1 = n.Cross( yAxis );
		Vec3 v2 = n.Cross( v1 );
		m_constructionPlane.Init( pos,pos+v1,pos+v2 );
	}
	else if (axis == AXIS_Z)
	{
		// Z direction.
		Vec3 n;
		float d1 = fabs(rayDir.Dot(xAxis));
		float d2 = fabs(rayDir.Dot(yAxis));
		if (d1 > d2) n = xAxis; else n = yAxis;

		Vec3 v1 = n.Cross( zAxis );
		Vec3 v2 = n.Cross( v1 );
		m_constructionPlane.Init( pos,pos+v1,pos+v2 );
	}
	else if (axis == AXIS_XY)
	{
		m_constructionPlane.Init( pos,pos+xAxis,pos+yAxis );
	}
	else if (axis == AXIS_XZ)
	{
		m_constructionPlane.Init( pos,pos+xAxis,pos+zAxis );
	}
	else if (axis == AXIS_YZ)
	{
		m_constructionPlane.Init( pos,pos+yAxis,pos+zAxis );
	}
}

//////////////////////////////////////////////////////////////////////////
Vec3 CViewport::MapViewToCP( CPoint point )
{
	if (m_activeAxis == AXIS_TERRAIN)
	{
		return SnapToGrid(ViewToWorld(point));
	}

	Vec3 raySrc(0,0,0),rayDir(1,0,0);
	ViewToWorldRay( point,raySrc,rayDir );

	float t,k;
	Vec3 v;

	k = rayDir.Dot( m_constructionPlane.n );
	if (fabs(k) > 0.000001f)
	{
		t = (m_constructionPlane.d - (raySrc.Dot(m_constructionPlane.n))) / k;
		v = raySrc + t*rayDir;

		if (t < 0)
			v.Set(0,0,0);
	}
	else
	{
		v.Set(0,0,0);
	}

	// Snap value to grid.
	v = SnapToGrid(v);

	return v;
}

//////////////////////////////////////////////////////////////////////////
Vec3 CViewport::GetCPVector( const Vec3 &p1,const Vec3 &p2 )
{
	Vec3 v = p2 - p1;

	int axis = m_activeAxis;

	int coords = GetIEditor()->GetReferenceCoordSys();

	Vec3 xAxis(1,0,0);
	Vec3 yAxis(0,1,0);
	Vec3 zAxis(0,0,1);
	// In local coordinate system transform axises by construction matrix.
	if (coords == COORDS_LOCAL)
	{
    //CHANGED_BY_IVO
		//xAxis = m_constructionMatrix.TransformVector(xAxis);
		//yAxis = m_constructionMatrix.TransformVector(yAxis);
		//zAxis = m_constructionMatrix.TransformVector(zAxis);

		xAxis = GetTransposed44(m_constructionMatrix)*(xAxis);
		yAxis = GetTransposed44(m_constructionMatrix)*(yAxis);
		zAxis = GetTransposed44(m_constructionMatrix)*(zAxis);

	}
	else if (coords == COORDS_VIEW)
	{
		Matrix44 vtm = m_constructionViewTM;
		//vtm.Invert();

		//CHANGED_BY_IVO
		//xAxis = vtm.TransformVector(xAxis);
		//yAxis = vtm.TransformVector(yAxis);
		//zAxis = vtm.TransformVector(zAxis);

		xAxis = GetTransposed44(vtm)*(xAxis);
		yAxis = GetTransposed44(vtm)*(yAxis);
		zAxis = GetTransposed44(vtm)*(zAxis);
	}

	if (axis == AXIS_X || axis == AXIS_Y || axis == AXIS_Z)
	{
		// Project vector v on transformed axis x,y or z.
		Vec3 axisVector;
		if (axis == AXIS_X)
			axisVector = xAxis;
		if (axis == AXIS_Y)
			axisVector = yAxis;
		if (axis == AXIS_Z)
			axisVector = zAxis;
		
		// Project vector on construction plane into the one of axises.
		v = v.Dot(axisVector) * axisVector;
	}
	else if (axis == AXIS_XY)
	{
		// Project vector v on transformed plane x/y.
		Vec3 planeNormal = xAxis.Cross(yAxis);
		Vec3 projV = v.Dot(planeNormal) * planeNormal;
		v = v - projV;
	}
	else if (axis == AXIS_XZ)
	{
		// Project vector v on transformed plane x/y.
		Vec3 planeNormal = xAxis.Cross(zAxis);
		Vec3 projV = v.Dot(planeNormal) * planeNormal;
		v = v - projV;
	}
	else if (axis == AXIS_YZ)
	{
		// Project vector v on transformed plane x/y.
		Vec3 planeNormal = yAxis.Cross(zAxis);
		Vec3 projV = v.Dot(planeNormal) * planeNormal;
		v = v - projV;
	}
	else if (axis == AXIS_TERRAIN)
	{
		v.z = 0;
	}
	return v;
}

//////////////////////////////////////////////////////////////////////////
//! Update current construction plane.
void CViewport::UpdateConstrPlane()
{
	CRect rc;
	GetClientRect( rc );
	SetConstrPlane( CPoint(rc.Width()/2,rc.Height()/2),m_constructionOriginalMatrix );
}

//////////////////////////////////////////////////////////////////////////
void CViewport::SetAxisConstrain( int axis )
{
	m_activeAxis = axis;
};

//////////////////////////////////////////////////////////////////////////
bool CViewport::HitTest( CPoint point,ObjectHitInfo &hitInfo,int flags )
{
	Vec3 raySrc(0,0,0),rayDir(1,0,0);
	ViewToWorldRay( point,raySrc,rayDir );
	hitInfo.view = this;
	hitInfo.point2d = point;
	return GetIEditor()->GetObjectManager()->HitTest( raySrc,rayDir,m_selectionTollerance,hitInfo );
}

//////////////////////////////////////////////////////////////////////////
void CViewport::SetZoomFactor(float fZoomFactor)
{
	m_fZoomFactor = fZoomFactor;
	if (gSettings.viewports.bSync2DViews)
		GetViewManager()->SetZoom2D( fZoomFactor );
};

//////////////////////////////////////////////////////////////////////////
float CViewport::GetZoomFactor() const
{
	if (gSettings.viewports.bSync2DViews)
	{
    m_fZoomFactor = GetViewManager()->GetZoom2D();
	}
	return m_fZoomFactor;
};

//////////////////////////////////////////////////////////////////////////
Vec3d CViewport::SnapToGrid( Vec3d vec )
{
	return m_viewManager->GetGrid()->Snap(vec);
}

//////////////////////////////////////////////////////////////////////////
bool CViewport::CheckVirtualKey( int virtualKey )
{
	GetAsyncKeyState(virtualKey);
	if (GetAsyncKeyState(virtualKey))
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CViewport::BeginUndo()
{
	DegradateQuality(true);
	GetIEditor()->BeginUndo();
}

//////////////////////////////////////////////////////////////////////////
void CViewport::AcceptUndo( const CString &undoDescription )
{
	DegradateQuality(false);
	GetIEditor()->AcceptUndo( undoDescription );
	GetIEditor()->UpdateViews(eUpdateObjects);
}

//////////////////////////////////////////////////////////////////////////
void CViewport::CancelUndo()
{
	DegradateQuality(false);
	GetIEditor()->CancelUndo();
	GetIEditor()->UpdateViews(eUpdateObjects);
}
	
//////////////////////////////////////////////////////////////////////////
void CViewport::RestoreUndo()
{
	GetIEditor()->RestoreUndo();
}

//////////////////////////////////////////////////////////////////////////
bool CViewport::IsUndoRecording() const
{
	return GetIEditor()->IsUndoRecording();
}

//////////////////////////////////////////////////////////////////////////
void CViewport::DegradateQuality( bool bEnable )
{
	m_bDegradateQuality = bEnable;
}

//////////////////////////////////////////////////////////////////////////
CSize CViewport::GetIdealSize() const
{
	return CSize(0,0);
}

//////////////////////////////////////////////////////////////////////////
//! Called to select rectangle in viewport.
void CViewport::SelectObjectsInRect( const CRect &selectRect,bool bSelect )
{
	// Ignore too small rectangles.
	if (selectRect.Width() < 5 || selectRect.Height() < 5)
		return;
	
	GetIEditor()->GetObjectManager()->SelectObjectsInRect( this,selectRect,bSelect );
	/*
	CUndo undo( "Select Object(s)" );

	CRect objrc,temprc;

	BBox box;
	std::vector<CBaseObject*> objects;
	GetIEditor()->GetObjectManager()->GetObjects( objects );
	for (int i = 0; i < objects.size(); i++)
	{
		CBaseObject *pObj = objects[i];
		if (!pObj->IsSelectable())
			continue;

		// Retrieve world space bound box.
		pObj->GetBoundBox( box );
		// Project bounding box to screen.

		if (!IsBoundsVisible(box))
			continue;

		// transform all 8 vertices into world space
		CPoint p[8] = 
		{ 
			WorldToView(Vec3d(box.min.x,box.min.y,box.min.z)),
			WorldToView(Vec3d(box.min.x,box.max.y,box.min.z)),
			WorldToView(Vec3d(box.max.x,box.min.y,box.min.z)),
			WorldToView(Vec3d(box.max.x,box.max.y,box.min.z)),
			WorldToView(Vec3d(box.min.x,box.min.y,box.max.z)),
			WorldToView(Vec3d(box.min.x,box.max.y,box.max.z)),
			WorldToView(Vec3d(box.max.x,box.min.y,box.max.z)),
			WorldToView(Vec3d(box.max.x,box.max.y,box.max.z))
		};

		objrc.left = 10000;
		objrc.top = 10000;
		objrc.right = -10000;
		objrc.bottom = -10000;
		// find new min/max values
		for(int i=0; i<8; i++)
		{
			objrc.left = min(objrc.left,p[i].x);
			objrc.right = max(objrc.right,p[i].x);
			objrc.top = min(objrc.top,p[i].y);
			objrc.bottom = max(objrc.bottom,p[i].y);
		}
		if (objrc.IsRectEmpty())
		{
			// Make objrc at least of size 1.
			objrc.bottom += 1;
			objrc.right += 1;
		}
		if (temprc.IntersectRect( objrc,selectRect ))
		{
			// Should select this object.
			if (bSelect)
				GetIEditor()->GetObjectManager()->SelectObject( pObj );
			else
				GetIEditor()->GetObjectManager()->UnselectObject( pObj );
		}
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
bool CViewport::IsBoundsVisible( const BBox &box ) const
{
	// Always visible in standart implementation.
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CViewport::HitTestLine( const Vec3 &lineP1,const Vec3 &lineP2,CPoint hitpoint,int pixelRadius )
{
	CPoint p1 = WorldToView( lineP1 );
	CPoint p2 = WorldToView( lineP2 );

	float dist = PointToLineDistance2D( Vec3(p1.x,p1.y,0),Vec3(p2.x,p2.y,0),Vec3(hitpoint.x,hitpoint.y,0) );
	if (dist <= pixelRadius)
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////
void CViewport::StartAVIRecording( const char *filename )
{
	StopAVIRecording();
	m_bAVICreation = true;
	m_pAVIWriter = new CAVI_Writer;

	CRect rc;
	GetClientRect(rc);
	int w = rc.Width() & (~3); // must be dividable by 4
	int h = rc.Height()& (~3); // must be dividable by 4
	m_aviFrame.Allocate( w,h );

	m_aviFrameRate = gSettings.aviSettings.nFrameRate;
	if (!m_pAVIWriter->OpenFile( filename,w,h ))
	{
		delete m_pAVIWriter;
		m_pAVIWriter = 0;
	}
	m_aviLastFrameTime = GetIEditor()->GetSystem()->GetITimer()->GetCurrTime();
	m_bAVICreation = false;

	// Forces parent window (ViewPane) to redraw intself.
	if (GetParent())
		GetParent()->Invalidate();
}

//////////////////////////////////////////////////////////////////////////
void CViewport::StopAVIRecording()
{
	m_bAVICreation = true;
	if (m_pAVIWriter)
	{
		delete m_pAVIWriter;
		// Forces parent window (ViewPane) to redraw intself.
		if (GetParent())
			GetParent()->Invalidate();
	}
	m_pAVIWriter = 0;
	if (m_aviFrame.IsValid())
		m_aviFrame.Release();
	m_bAVICreation = false;
}

//////////////////////////////////////////////////////////////////////////
void CViewport::AVIRecordFrame()
{
	if (m_pAVIWriter && !m_bAVICreation && !m_bAVIPaused)
	{
		float currTime = GetIEditor()->GetSystem()->GetITimer()->GetCurrTime();
		if (currTime - m_aviLastFrameTime > 1.0f / m_aviFrameRate)
		{
			m_aviLastFrameTime = currTime;
			//m_renderer->ReadFrameBuffer( (unsigned char*)m_aviFrame.GetData(),m_aviFrame.GetWidth(),m_aviFrame.GetHeight(),true,false );

			int width = m_aviFrame.GetWidth();
			int height = m_aviFrame.GetHeight();
			//int w = m_rcClient.Width();
			//int h = m_rcClient.Height();

			CBitmap bmp;
			CDC dcMemory;
			CDC *pDC = GetDC();
			dcMemory.CreateCompatibleDC(pDC);

			bmp.CreateCompatibleBitmap(pDC,width,height);

			CBitmap* pOldBitmap = dcMemory.SelectObject(&bmp);
			//dcMemory.BitBlt( 0,0,width,height,pDC,0,0,SRCCOPY );
			dcMemory.StretchBlt( 0,height,width,-height,pDC,0,0,width,height,SRCCOPY );

			BITMAP bmpInfo;
			bmp.GetBitmap( &bmpInfo );
			bmp.GetBitmapBits( width*height*(bmpInfo.bmBitsPixel/8),m_aviFrame.GetData() );
			int bpp = bmpInfo.bmBitsPixel/8;
/*
			char *pTemp = new char [width*height*bpp];
			char *pSrc = (char*)m_aviFrame.GetData();
			char *pTrg = (char*)m_aviFrame.GetData() + width*(height-1)*bpp;
			int lineSize = width*bpp;
			for (int h = 0; h < height; h++)
			{
				memmove( pTemp,pSrc,lineSize );
				memmove( pSrc,pTrg,lineSize );
				memmove( pTrg,pTemp,lineSize );
				pSrc += lineSize;
				pTrg -= lineSize;
			}
			delete []pTemp;
			*/

			dcMemory.SelectObject(pOldBitmap);

			ReleaseDC(pDC);

			m_pAVIWriter->AddFrame( m_aviFrame );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CViewport::IsAVIRecording() const
{
	return m_pAVIWriter != NULL;
};

//////////////////////////////////////////////////////////////////////////
void CViewport::PauseAVIRecording( bool bPause )
{
	m_bAVIPaused = bPause;
}
