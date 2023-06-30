// PreviewModelCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "PreviewModelCtrl.h"

#include <I3DEngine.h>
#include <IEntitySystem.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPreviewModelCtrl
CPreviewModelCtrl::CPreviewModelCtrl()
{
	m_renderer = 0;
	m_engine = 0;
	m_object = 0;
	m_character = 0;
	m_entity = 0;
	m_nTimer = 0;
	m_size(0,0,0);

	m_bRotate = false;
	m_rotateAngle = 0;

	m_renderer = GetIEditor()->GetRenderer();
	m_engine = GetIEditor()->Get3DEngine();
  m_pAnimationSystem = GetIEditor()->GetSystem()->GetIAnimationSystem();

	m_fov = 60;
	m_camera.SetFov( DEG2RAD(m_fov) );
	m_camera.Init( 800,600,3.14f/4.0f );
	m_camera.SetZMin( 0.01f );
	m_camera.SetZMax( 10000 );

	m_bInRotateMode = false;
	m_bInMoveMode = false;

	CDLight l;
  l.m_Origin = Vec3(10,10,10);
	float L = 0.5f;
	l.m_Color.r = L;  l.m_Color.g = L; l.m_Color.b = L; l.m_Color.a = 1;
	l.m_SpecColor.r = L; l.m_SpecColor.g = L; l.m_SpecColor.b = L; l.m_SpecColor.a = 1;
	l.m_fRadius = 1000;
	l.m_fStartRadius = 0;
	l.m_fEndRadius = 1000;
  l.m_Flags |= DLF_POINT;
	m_lights.push_back( l );

	l.m_Origin = Vec3(-10,-10,-10);
	l.m_Color.r = L;  l.m_Color.g = L; l.m_Color.b = L; l.m_Color.a = 1;
	l.m_SpecColor.r = L; l.m_SpecColor.g = L; l.m_SpecColor.b = L; l.m_SpecColor.a = 1;
  l.m_fRadius = 1000;
	l.m_fStartRadius = 0;
	l.m_fEndRadius = 1000;
  l.m_Flags |= DLF_POINT;
	m_lights.push_back( l );

	m_bContextCreated = false;

	m_bHaveAnythingToRender = false;
	m_bGrid = false;
	m_bUpdate = false;

	BBox box( Vec3(-2,-2,-2), Vec3(2,2,2) );
	SetCameraLookAtBox( box );
}

CPreviewModelCtrl::~CPreviewModelCtrl()
{
	/*
	IRenderer *pr = CSystem::Instance()->SetCurrentRenderer( m_renderer );
	I3DEngine *pe = CSystem::Instance()->SetCurrent3DEngine( m_3dEngine );

	if (m_renderer)
		m_renderer->ShutDown();
	m_renderer = 0;

		//delete m_renderer;
	/*
	if (m_3dEngine)
		m_3dEngine->Release3DEngine();
	*/
/*
	CSystem::Instance()->SetCurrentRenderer( pr );
	CSystem::Instance()->SetCurrent3DEngine( pe );
	*/
}


BEGIN_MESSAGE_MAP(CPreviewModelCtrl, CWnd)
	//{{AFX_MSG_MAP(CPreviewModelCtrl)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPreviewModelCtrl message handlers

//////////////////////////////////////////////////////////////////////////
BOOL CPreviewModelCtrl::Create( CWnd *pWndParent,const CRect &rc,DWORD dwStyle )
{
	BOOL bReturn = CreateEx( NULL, AfxRegisterWndClass(CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW|CS_OWNDC, 
		AfxGetApp()->LoadStandardCursor(IDC_ARROW), NULL, NULL), NULL,dwStyle,
		rc, pWndParent, NULL);

	return bReturn;
}

//////////////////////////////////////////////////////////////////////////
int CPreviewModelCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CPreviewModelCtrl::CreateContext()
{
	// Create context.
	if (m_renderer && !m_bContextCreated)
	{
		m_bContextCreated = true;
    m_renderer->CreateContext( m_hWnd );
		// Make main context current.
		m_renderer->MakeCurrent();
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	/*
	// Create context.
	if (m_renderer && !m_bContextCreated)
	{
		m_bContextCreated = true;
    m_renderer->CreateContext( m_hWnd );
		// Make main context current.
		m_renderer->MakeCurrent();
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::ReleaseObject()
{
	if (m_object)
		m_engine->ReleaseObject( m_object );
	if (m_character)
		m_pAnimationSystem->RemoveCharacter( m_character );
	m_object = 0;
	m_character = 0;
	m_entity = 0;
	m_bHaveAnythingToRender = false;
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::LoadFile( const CString &modelFile,bool changeCamera )
{
	m_bHaveAnythingToRender = false;
	if (!m_hWnd)
		return;
	if (!m_renderer)
		return;

	ReleaseObject();

	if (modelFile.IsEmpty())
	{
		if (m_nTimer != 0)
			KillTimer(m_nTimer);
		m_nTimer = 0;
		Invalidate();
		return;
	}

	if (stricmp( Path::GetExt(modelFile),"cga" ) == 0)
	{
		// Load CGA animated object.
		m_character = m_pAnimationSystem->MakeCharacter( modelFile );
		if(!m_character)
		{
			Warning( "Loading of geometry object %s failed.",(const char*)modelFile );
			if (m_nTimer != 0)
				KillTimer(m_nTimer);
			m_nTimer = 0;
			Invalidate();
			return;
		}
		m_character->GetBBox( m_bboxMin,m_bboxMax );
	}
	else
	{
		// Load object.
		m_object = m_engine->MakeObject( modelFile,NULL,evs_ShareAndSortForCache );
		if(!m_object)
		{
			Warning( "Loading of geometry object %s failed.",(const char*)modelFile );
			if (m_nTimer != 0)
				KillTimer(m_nTimer);
			m_nTimer = 0;
			Invalidate();
			return;
		}
		m_bboxMin = m_object->GetBoxMin();
		m_bboxMax = m_object->GetBoxMax();
	}

	m_bHaveAnythingToRender = true;

	// No timer.
	/*
	if (m_nTimer == 0)
		m_nTimer = SetTimer(1,200,NULL);
	*/
	
	if (changeCamera)
	{
		BBox box;
		box.min = m_bboxMin;
		box.max = m_bboxMax;
		SetCameraLookAtBox( box );
		//SetOrbitAngles( Vec3(0,0,0) );
	}
	Invalidate();
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::SetEntity( IEntityRender *entity )
{
	m_bHaveAnythingToRender = false;
	if (m_entity != entity)
	{
		m_entity = entity;
		if (m_entity)
		{
			m_bHaveAnythingToRender = true;
			m_entity->GetBBox( m_bboxMin,m_bboxMax );
		}
		Invalidate();
	}
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::SetObject( IStatObj *pObject )
{
	if (m_object != pObject)
	{
		m_bHaveAnythingToRender = false;
		m_object = pObject;
		if (m_object)
		{
			m_bHaveAnythingToRender = true;
			m_bboxMin = m_object->GetBoxMin();
			m_bboxMax = m_object->GetBoxMax();
		}
		Invalidate();
	}
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::SetCameraLookAtBox( const BBox &box )
{
	Vec3d v = box.max - box.min;
	float radius = v.Length()/2.0f;

	m_camTarget = (box.max + box.min) * 0.5f;
	m_camRadius = radius*1.8f;
	m_camAngles(0,0,0);
	m_camAngles.x = 30;
	m_camAngles.y = 0;
	m_camAngles.z = -30;
	m_camera.SetPos(m_camTarget + Vec3(m_camRadius,m_camRadius,m_camRadius) );
	m_camera.SetAngle(m_camAngles);
	SetOrbitAngles( m_camAngles );
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
	bool res = Render();
	if (!res)
	{
		RECT rect;
		// Get the rect of the client window
		GetClientRect(&rect);
		
		// Create the brush
		CBrush cFillBrush;
		cFillBrush.CreateSolidBrush(RGB(128,128,128));
		
		// Fill the entire client area
		dc.FillRect(&rect, &cFillBrush);
	}
}

//////////////////////////////////////////////////////////////////////////
BOOL CPreviewModelCtrl::OnEraseBkgnd(CDC* pDC) 
{
	if (m_bHaveAnythingToRender)
		return TRUE;

	return CWnd::OnEraseBkgnd(pDC);
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::SetCamera( CCamera &cam )
{
	m_camera.SetPos( cam.GetPos() );
	m_camera.SetAngle( cam.GetAngles() );

	CRect rc;
	GetClientRect(rc);
	//m_camera.SetFov(m_stdFOV);
	int w = rc.Width();
	int h = rc.Height();
	float proj = (float)h/(float)w;
	if (proj > 1.2f) proj = 1.2f;
	m_camera.Init( w,h,DEG2RAD(m_fov),m_camera.GetZMax(),proj );

	m_camera.Update();
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::SetOrbitAngles( const Vec3d &ang )
{
	float dist = (m_camera.GetPos() - m_camTarget).Length();

	Vec3d cangles = ang;
	Vec3d v(0,0,dist);
  cangles=ConvertToRad(cangles);

	//Matrix tm; tm.Identity();
  //tm.Rotate(cangles);
  Matrix44 tm=ViewMatrix(cangles);

	v = tm*v;

	m_camera.SetPos( v + m_camTarget );
	m_camera.SetAngle( m_camAngles );

}

//////////////////////////////////////////////////////////////////////////
bool CPreviewModelCtrl::Render()
{
	if (!m_bHaveAnythingToRender)
	{
		return false;
	}
	if (!m_bContextCreated)
	{
		if (!CreateContext())
			return false;
	}

	//IRenderer *pr = CSystem::Instance()->SetCurrentRenderer( m_renderer );
	SetCamera( m_camera );

	CRect rc;
	GetClientRect(rc);
	
	m_renderer->SetCurrentContext( m_hWnd );
	m_renderer->ChangeViewport(0,0,rc.right,rc.bottom);
	m_renderer->SetClearColor( Vec3(0.5f,0.5f,0.5f) );
	m_renderer->BeginFrame();
	
	m_renderer->SetCamera( m_camera );

	m_renderer->SetPolygonMode( R_SOLID_MODE );

	// Render object.
	m_renderer->EF_ClearLightsList();
	m_renderer->EF_StartEf();
	m_renderer->ResetToDefault();

	// Add lights.
	for (int i = 0; i < m_lights.size(); i++)
	{
		m_renderer->EF_ADDDlight( &m_lights[i] );
	}
		
	SRendParams rp;
	rp.vPos = Vec3(0,0,0);
	rp.vAngles = Vec3(0,0,0);
	rp.nDLightMask = 0x3;
	rp.vAmbientColor = Vec3d(1,1,1);
  rp.dwFObjFlags |= FOB_TRANS_MASK;

	if (m_bRotate)
	{
		rp.vAngles.Set( 0,0,m_rotateAngle );
		m_rotateAngle += 0.1f;
	}

	if (m_object)
		m_object->Render( rp,Vec3(zero),0 );

	if (m_entity)
		m_entity->DrawEntity( rp );

	if (m_character)
		m_character->Draw( rp,Vec3(zero) );

	m_renderer->EF_EndEf3D(SHDF_SORT);
	m_renderer->EF_ClearLightsList();

	if (m_bGrid)
		DrawGrid();

	m_renderer->FlushTextMessages();
	m_renderer->Update();
	
	// Restore main context.
	m_renderer->MakeCurrent();
	
	return true;
}

void CPreviewModelCtrl::DrawGrid()
{
	// Draw grid.
	float step = 0.1f;
	float XR = 5;
	float YR = 5;

	m_renderer->ResetToDefault();
	m_renderer->SetState(GS_DEPTHWRITE | GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);
	//m_renderer->SetBlendMode();
	//m_renderer->EnableBlend( true );
	m_renderer->SetMaterialColor( 0.6f,0.6f,0.6f,0.3f );
	// Draw grid.
	for (float x = -XR; x < XR; x+=step)
	{
		if (fabs(x) > 0.01)
			m_renderer->DrawLine( Vec3d(x,-YR,0),Vec3d(x,YR,0) );
	}
	for (float y = -YR; y < YR; y+=step)
	{
		if (fabs(y) > 0.01)
			m_renderer->DrawLine( Vec3d(-XR,y,0),Vec3d(XR,y,0) );
	}

	// Draw axis.
	m_renderer->SetMaterialColor( 1,0,0,0.3f );
	m_renderer->DrawLine( Vec3d(-XR,0,0),Vec3d(XR,0,0) );
	
	m_renderer->SetMaterialColor( 0,1,0,0.3f );
	m_renderer->DrawLine( Vec3d(0,-YR,0),Vec3d(0,YR,0) );

	m_renderer->SetMaterialColor( 0,0,1,0.3f );
	m_renderer->DrawLine( Vec3d(0,0,-YR),Vec3d(0,0,YR) );
}

void CPreviewModelCtrl::OnTimer(UINT_PTR nIDEvent) 
{
	if (IsWindowVisible())
	{
		if (m_bHaveAnythingToRender)
			Invalidate();
	}
	
	CWnd::OnTimer(nIDEvent);
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::DeleteRenderContex()
{
	ReleaseObject();

	// Destroy render context.
	if (m_renderer && m_bContextCreated)
	{
		m_renderer->DeleteContext(m_hWnd);
		m_bContextCreated = false;
	}
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::OnDestroy() 
{
	DeleteRenderContex();

	CWnd::OnDestroy();
	
	if (m_nTimer)
		KillTimer( m_nTimer );
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_bInRotateMode = true;
	m_mousePos = point;
	if (!m_bInMoveMode)
		SetCapture();
}

void CPreviewModelCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bInRotateMode = false;
	if (!m_bInMoveMode)
		ReleaseCapture();
}

void CPreviewModelCtrl::OnMButtonDown(UINT nFlags, CPoint point) 
{
	m_bInRotateMode = true;
	m_bInMoveMode = true;
	m_mousePos = point;
	//if (!m_bInMoveMode)
	SetCapture();
}

void CPreviewModelCtrl::OnMButtonUp(UINT nFlags, CPoint point) 
{
	m_bInRotateMode = false;
	m_bInMoveMode = false;
	ReleaseCapture();
}

void CPreviewModelCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CWnd::OnMouseMove(nFlags, point);

	Invalidate();
	if (point == m_mousePos)
		return;

	if (m_bInRotateMode && m_bInMoveMode)
	{
		// Zoom.
		Matrix44 m = m_camera.GetVCMatrixD3D9();
		//Vec3d xdir(m[0][0],m[1][0],m[2][0]);
		Vec3d xdir(0,0,0);
		//xdir.Normalize();

		Vec3d zdir(m[0][2],m[1][2],m[2][2]);
		zdir.Normalize();

		float step = 0.002f;
		float dx = (point.x-m_mousePos.x);
		float dy = (point.y-m_mousePos.y);
//		dx = pow(dx,1.05f );
		//dy = pow(dy,1.05f );
		//m_camera.SetPos( m_camera.GetPos() + ydir*(m_mousePos.y-point.y),xdir*(m_mousePos.x-point.x) );
		m_camera.SetPos( m_camera.GetPos() + step*xdir*dx +  step*zdir*dy );
		SetCamera( m_camera );

		CPoint pnt = m_mousePos;
		ClientToScreen( &pnt );
		SetCursorPos( pnt.x,pnt.y );
	}
	else if (m_bInRotateMode)
	{
		// Look
		Vec3d angles( point.y-m_mousePos.y,0,-point.x+m_mousePos.x );
		//m_camera.SetAngle( m_camera.GetAngles() + angles*0.2f );
		m_camAngles += angles;

		SetOrbitAngles( m_camAngles );
		
		CPoint pnt = m_mousePos;
		ClientToScreen( &pnt );
		SetCursorPos( pnt.x,pnt.y );
	}
	else if (m_bInMoveMode)
	{
		// Slide.
		Matrix44 m = m_camera.GetVCMatrixD3D9();
		Vec3d xdir(m[0][0],m[1][0],m[2][0]);
		Vec3d ydir(m[0][1],m[1][1],m[2][1]);
		xdir.Normalize();
		ydir.Normalize();

		float dist = (m_camera.GetPos() - m_camTarget).Length();

		float step = 0.001f;
		float dx = (point.x-m_mousePos.x);
		float dy = (point.y-m_mousePos.y);
		//dx =  pow( dx,1.2f );
		//dy =  pow( dy,1.2f );
		//m_camera.SetPos( m_camera.GetPos() + ydir*(m_mousePos.y-point.y),xdir*(m_mousePos.x-point.x) );
		m_camera.SetPos( m_camera.GetPos() - step*xdir*dx +  step*ydir*dy );

		SetCamera( m_camera );

		// Calc camera target.
	  Vec3d angles = m_camAngles;

		angles=ConvertToRad(angles);

		//Matrix44 tm; tm.Identity();
		//tm.Rotate(angles);
    Matrix44 tm=ViewMatrix(angles);

		Vec3d v(0,0,dist);
		v = m_camera.GetPos() - tm*v;
		m_camTarget = v;


		m_mousePos = point;

		CPoint pnt = m_mousePos;
		ClientToScreen( &pnt );
		SetCursorPos( pnt.x,pnt.y );
	}
	//Invalidate();
}

void CPreviewModelCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_bInMoveMode = true;
	m_mousePos = point;
	if (!m_bInRotateMode)
		SetCapture();
}

void CPreviewModelCtrl::OnRButtonUp(UINT nFlags, CPoint point) 
{
	m_bInMoveMode = false;
	m_mousePos = point;
	if (!m_bInRotateMode)
		ReleaseCapture();
}

BOOL CPreviewModelCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	Matrix44 m = m_camera.GetVCMatrixD3D9();
	Vec3d zdir(m[0][2],m[1][2],m[2][2]);
	zdir.Normalize();

	//m_camera.SetPos( m_camera.GetPos() + ydir*(m_mousePos.y-point.y),xdir*(m_mousePos.x-point.x) );
	m_camera.SetPos( m_camera.GetPos() + 0.002f*zdir*(zDelta) );
	SetCamera( m_camera );
	Invalidate();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType,cx,cy );
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::EnableUpdate( bool bUpdate )
{
	m_bUpdate = bUpdate;
	// No timer.
/*
	if (bUpdate)
	{
		if (m_nTimer == 0)
			m_nTimer = SetTimer(1,50,NULL);
	}
	else
	{
		if (m_nTimer != 0)
			KillTimer(m_nTimer);
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::Update()
{
	if (m_bUpdate && m_bHaveAnythingToRender)
	{
		if (IsWindowVisible())
			Invalidate(FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////
void CPreviewModelCtrl::SetRotation( bool bEnable )
{
	m_bRotate = bEnable;
}
