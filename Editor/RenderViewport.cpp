// RenderViewport.cpp : implementation filefov
//

#include "stdafx.h"
#include "RenderViewport.h"
#include "DisplaySettings.h"
#include "EditTool.h"
#include "CryEditDoc.h"

#include "GameEngine.h"

#include "Objects\ObjectManager.h"
#include "Objects\BaseObject.h"
#include "Objects\CameraObject.h"
#include "VegetationMap.h"
#include "ViewManager.h"

#include "ProcessInfo.h"
#include "Settings.h"

#include <I3DEngine.h>
#include "IPhysics.h"
#include <IAISystem.h>
#include <IConsole.h>
#include <ITimer.h>
#include ".\renderviewport.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CRenderViewport,CViewport)

/////////////////////////////////////////////////////////////////////////////
// CRenderViewport

CRenderViewport::CRenderViewport()
{
	m_camera.Init( 800,600,gSettings.viewports.fDefaultFov );
	m_camera.SetZMin( 0.1f );
	m_camera.SetZMax( 10000.0f );
	m_camera.SetPos( Vec3d(1000,1000,60) );
	m_camera.SetAngle( Vec3d(0,0,0) );
	m_camera.Update();
	GetIEditor()->GetSystem()->SetViewCamera(m_camera);
	m_bRenderContextCreated = false;

	m_bInRotateMode = false;
	m_bInMoveMode = false;

	m_bWireframe = GetIEditor()->GetDisplaySettings()->GetDisplayMode() == DISPLAYMODE_WIREFRAME;
	m_bDisplayLabels = GetIEditor()->GetDisplaySettings()->IsDisplayLabels();

	//for (int i = 0; i < 255; i++)
		//GetAsyncKeyState( i );

	m_cameraObjectId = GUID_NULL;

	m_moveSpeed = 1;

	m_bSequenceCamera = false;
	m_bRMouseDown = false;
	m_bUpdating = false;

	m_nPresedKeyState = 0;
}

CRenderViewport::~CRenderViewport()
{
}


BEGIN_MESSAGE_MAP(CRenderViewport, CViewport)
	//{{AFX_MSG_MAP(CRenderViewport)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
	ON_WM_SETCURSOR()
	ON_WM_DESTROY()
	ON_COMMAND(ID_SWITCHCAMERA_DEFAULTCAMERA, OnSwitchcameraDefaultcamera)
	ON_COMMAND(ID_SWITCHCAMERA_SEQUENCECAMERA, OnSwitchcameraSequencecamera)
	ON_COMMAND(ID_SWITCHCAMERA_SELECTEDCAMERA, OnSwitchcameraSelectedcamera)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CRenderViewport message handlers

int CRenderViewport::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CViewport::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_renderer = GetIEditor()->GetRenderer();
	if (!m_renderer)
	{
    // Create renderer.
		m_renderer = GetIEditor()->GetSystem()->CreateRenderer( false,AfxGetInstanceHandle(),GetSafeHwnd() );
		SetViewerPos( Vec3d(1000,1000,100) );
		//CSystem::Instance()->SetCurrentRenderer( m_renderer );
	}
	m_engine = GetIEditor()->Get3DEngine();
	assert( m_engine );
	m_pAnimationSystem = GetIEditor()->GetSystem()->GetIAnimationSystem();

	CreateRenderContext();
	
	return 0;
}

void CRenderViewport::OnSize(UINT nType, int cx, int cy) 
{
	CViewport::OnSize(nType, cx, cy);

	if(!cx || !cy)
		return;

	GetClientRect( m_rcClient );

	m_viewSize.cx = cx;
	m_viewSize.cy = cy;
}

BOOL CRenderViewport::OnEraseBkgnd(CDC* pDC) 
{
	CGameEngine *ge = GetIEditor()->GetGameEngine();
	if (ge && ge->IsLevelLoaded())
	{
		return TRUE;
	}
	return FALSE;
}

void CRenderViewport::OnPaint() 
{
	// TODO: Add your message handler code here
	
	// Do not call CViewport::OnPaint() for painting messages
	CGameEngine *ge = GetIEditor()->GetGameEngine();
	if (ge && ge->IsLevelLoaded())
  {
    CViewport::OnPaint();
		Update();
  }
	else
	{
    CPaintDC dc(this); // device context for painting
		CRect rc;
		GetClientRect(rc);
		dc.FillRect( rc,CBrush::FromHandle((HBRUSH)GetStockObject(GRAY_BRUSH)) );
	}
}

void CRenderViewport::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (GetIEditor()->IsInGameMode())
		return;

	// Convert point to position on terrain.
	if (!m_renderer)
		return;

	// TODO: Add your message handler code here and/or call default
	CViewport::OnLButtonDown(nFlags, point);
}

void CRenderViewport::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (GetIEditor()->IsInGameMode())
		return;

	// Convert point to position on terrain.
	if (!m_renderer)
		return;
	// TODO: Add your message handler code here and/or call default
	CViewport::OnLButtonUp(nFlags, point);
}

void CRenderViewport::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_bRMouseDown = true;

	if (GetIEditor()->IsInGameMode())
		return;

	if (GetIEditor()->GetEditTool())
	{
		GetIEditor()->GetEditTool()->MouseCallback( this,eMouseRDown,point,nFlags );
	}

	CViewport::OnRButtonDown(nFlags, point);
	m_bInRotateMode = true;
	m_mousePos = point;

	CaptureMouse();
}

void CRenderViewport::OnRButtonUp(UINT nFlags, CPoint point) 
{
	m_bRMouseDown = false;
	if (GetIEditor()->IsInGameMode())
		return;
	
	CViewport::OnRButtonUp(nFlags, point);

	m_bInRotateMode = false;

	ReleaseMouse();

	//////////////////////////////////////////////////////////////////////////
	// To update object cursor.
	//////////////////////////////////////////////////////////////////////////
	// Track mouse movements.
	ObjectHitInfo hitInfo(this,point);
	HitTest( point,hitInfo );
	SetObjectCursor(hitInfo.object,true);

	// Update viewports after done with rotating.
	GetIEditor()->UpdateViews(eUpdateObjects);
}

void CRenderViewport::OnMButtonDown(UINT nFlags, CPoint point) 
{
	if (GetIEditor()->IsInGameMode())
		return;

	if (!(nFlags & MK_CONTROL) && !(nFlags & MK_SHIFT))
	{
		m_bInMoveMode = true;
		m_mousePos = point;
		CaptureMouse();
	}
	
	CViewport::OnMButtonDown(nFlags, point);
}

void CRenderViewport::OnMButtonUp(UINT nFlags, CPoint point) 
{
	if (GetIEditor()->IsInGameMode())
		return;

	m_bInMoveMode = false;
	m_mousePos = point;
	ReleaseMouse();

	// Update viewports after done with moving viewport.
	GetIEditor()->UpdateViews(eUpdateObjects);
	
	CViewport::OnMButtonUp(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (GetIEditor()->IsInGameMode())
		return;

	SetObjectCursor(0);

	if(!m_nPresedKeyState)
		m_nPresedKeyState	=	1;

	if (point == m_mousePos)
	{
		CViewport::OnMouseMove(nFlags, point);
		return;
	}

	float speedScale = 1;
	speedScale *= gSettings.cameraMoveSpeed;
	
	if (nFlags & MK_CONTROL)
	{
		speedScale *= gSettings.cameraFastMoveSpeed;
	}

	if (m_bInRotateMode && m_bInMoveMode)
	{
		// Zoom.
		Matrix44 m = m_camera.GetVCMatrixD3D9();
		//Vec3d xdir(m[0][0],m[1][0],m[2][0]);
		Vec3d xdir(0,0,0);
		//xdir.Normalize();

		Vec3d zdir(m[0][2],m[1][2],m[2][2]);
		zdir.Normalize();

		//m_camera.SetPos( m_camera.GetPos() + ydir*(m_mousePos.y-point.y),xdir*(m_mousePos.x-point.x) );
		Vec3d pos = GetViewerPos();
		pos = pos + 0.1f*xdir*(point.x-m_mousePos.x)*speedScale + 0.2f*zdir*(m_mousePos.y-point.y)*speedScale;
		SetViewerPos( pos );
//		GetIEditor()->MoveViewer(0.1f*xdir*(point.x-m_mousePos.x) + 0.2f*zdir*(m_mousePos.y-point.y));

		CPoint pnt = m_mousePos;
		ClientToScreen( &pnt );
		SetCursorPos( pnt.x,pnt.y );
		return;
	}
	else if (m_bInRotateMode)
	{
		// Look
		Vec3d angles( point.y-m_mousePos.y,0,-point.x+m_mousePos.x );
		SetViewerAngles( GetViewerAngles() + angles*0.2f );

		CPoint pnt = m_mousePos;
		ClientToScreen( &pnt );
		SetCursorPos( pnt.x,pnt.y );
		return;
	}
	else if (m_bInMoveMode)
	{
		// Slide.
		Matrix44 m = m_camera.GetVCMatrixD3D9();
		Vec3d xdir(m[0][0],m[1][0],m[2][0]);
		Vec3d ydir(m[0][1],m[1][1],m[2][1]);
		xdir.Normalize();
		ydir.Normalize();

		//m_camera.SetPos( m_camera.GetPos() + ydir*(m_mousePos.y-point.y),xdir*(m_mousePos.x-point.x) );
		Vec3d pos = GetViewerPos();
		pos += 0.1f*xdir*(point.x-m_mousePos.x)*speedScale + 0.1f*ydir*(m_mousePos.y-point.y)*speedScale;
		SetViewerPos( pos );
//		GetIEditor()->MoveViewer(0.1f*xdir*(point.x-m_mousePos.x) +  0.1f*ydir*(m_mousePos.y-point.y));

		CPoint pnt = m_mousePos;
		ClientToScreen( &pnt );
		SetCursorPos( pnt.x,pnt.y );
		return;
	}

	CViewport::OnMouseMove(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::Update()
{
	if (!m_renderer || !m_engine)
		return;

	if (GetIEditor()->IsInGameMode())
		return;

	if (!IsWindowVisible())
		return;

	if (!GetIEditor()->GetDocument()->IsDocumentReady())
		return;

	CGameEngine *ge = GetIEditor()->GetGameEngine();
	if (!ge || !ge->IsLevelLoaded())
		return;

	// Prevents rendering recursion due to recursive Paint messages.
	if (m_bUpdating)
		return;
	m_bUpdating = true;

	FUNCTION_PROFILER( GetIEditor()->GetSystem(),PROFILE_EDITOR );

	// Render
	if (!m_bRenderContextCreated)
	{
		if (!CreateRenderContext())
			return;
	}
	m_renderer->SetCurrentContext( m_hWnd );

	m_renderer->SetClearColor( Vec3(1,0,0) );
	m_renderer->BeginFrame();

	m_renderer->ChangeViewport(0,0,m_rcClient.right,m_rcClient.bottom);
	
	OnRender();

	// Print results.
	GetIEditor()->GetSystem()->GetITimer()->MeasureTime((LPCSTR)(-1));

	// Render all kind of system statistical data.
	GetIEditor()->GetSystem()->RenderStatistics();
	m_renderer->FlushTextMessages();

	m_renderer->Update();

	GetIEditor()->GetRenderer()->MakeCurrent();

	if (GetFocus() == this)
		ProcessKeys();

	CViewport::Update();

	m_bUpdating = false;
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::SetCameraObject( CBaseObject *cameraObject )
{
	if (cameraObject)
	{
		m_cameraObjectId = cameraObject->GetId();
		if (GetParent()) {
			GetParent()->SetWindowText( cameraObject->GetName() );
			GetParent()->Invalidate();
		}
		GetViewManager()->SetCameraObjectId( m_cameraObjectId );
	}
	else
	{
		// Switch to normal view.
		m_cameraObjectId = GUID_NULL;
		if (GetParent()) {
			GetParent()->SetWindowText( GetName() );
			GetParent()->Invalidate();
		}
		GetViewManager()->SetCameraObjectId( m_cameraObjectId );
	}
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CRenderViewport::GetCameraObject() const
{
	CBaseObject *cameraObject = 0;
	
	if (m_bSequenceCamera)
	{
		m_cameraObjectId = GetViewManager()->GetCameraObjectId();
	}
	if (m_cameraObjectId != GUID_NULL)
	{
		// Find camera object from id.
		cameraObject = GetIEditor()->GetObjectManager()->FindObject(m_cameraObjectId);
	}
	return cameraObject;
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::OnRender()
{
	FUNCTION_PROFILER( GetIEditor()->GetSystem(),PROFILE_EDITOR );
	
	//float proj = (float)rc.bottom/(float)rc.right;
	//if (proj > 1) proj = 1;
	//m_camera.Init( rc.right,rc.bottom,3.14f/2.0f,10000.0f,proj );
	//m_camera.Init( 800,600,3.14f/2.0f,10000.0f,proj );

	CBaseObject *cameraObject = GetCameraObject();
	if (cameraObject)
	{
		// Find camera object
		// This is a camera object.
		float fov = gSettings.viewports.fDefaultFov;
		if (cameraObject->IsKindOf(RUNTIME_CLASS(CCameraObject)))
		{
			CCameraObject *camObj = (CCameraObject*)cameraObject;
			fov = camObj->GetFOV();
			//m_camera.SetFov(fov);
		}
		else
		{
			//m_camera.SetFov(m_stdFOV);
		}
		/*
		int screenW = m_rcClient.right - m_rcClient.left;
		int screenH = m_rcClient.bottom - m_rcClient.top;	
		float fAspect = gSettings.viewports.fDefaultAspectRatio;
		int w =	 screenH/fAspect;
		int h =	 screenH;
		//int sx = (screenW - w)/2;
		int sx = 0;
		//m_renderer->ChangeViewport( sx,0,w,h );
		//float proj = (float)h/(float)w;
		*/
		
		int w = m_rcClient.right - m_rcClient.left;
		int h = m_rcClient.bottom - m_rcClient.top;	
		float fAspectRatio = (float)h/(float)w;
		if (fAspectRatio > 1.2f) fAspectRatio = 1.2f;
		m_camera.Init( w,h,fov,m_camera.GetZMax(),fAspectRatio );
	}
	else
	{
		// Normal camera.
		m_cameraObjectId = GUID_NULL;
		//m_camera.SetFov(m_stdFOV);
		int w = m_rcClient.right - m_rcClient.left;
		int h = m_rcClient.bottom - m_rcClient.top;	
		//m_renderer->ChangeViewport( 0,0,w,h );
		float fAspectRatio = (float)h/(float)w;
		if (fAspectRatio > 1.2f) fAspectRatio = 1.2f;
		m_camera.Init( w,h,gSettings.viewports.fDefaultFov,m_camera.GetZMax(),fAspectRatio );
	}

	m_camera.SetPos( GetViewerPos() );
	m_camera.SetAngle( GetViewerAngles() );
	m_camera.Update();
	GetIEditor()->GetSystem()->SetViewCamera(m_camera);
	m_engine->SetCamera(m_camera);

	if (GetIEditor()->GetDisplaySettings()->GetDisplayMode() == DISPLAYMODE_WIREFRAME)
		m_renderer->SetPolygonMode(R_WIREFRAME_MODE);
	else
		m_renderer->SetPolygonMode(R_SOLID_MODE);

	m_engine->Update();
	m_engine->Draw();

  // Draw engine stats
	IConsole * console = GetIEditor()->GetSystem()->GetIConsole();
	ICVar * pDispInfoCVar = console->GetCVar("r_DisplayInfo");
	if (pDispInfoCVar && pDispInfoCVar->GetIVal())
  {
    // Draw 3dengine stats and get last text cursor position
    float nTextPosX=101-20, nTextPosY=-2, nTextStepY=3;
    m_engine->DisplayInfo(nTextPosX, nTextPosY, nTextStepY);
  }

	//m_engine->SetRenderCallback( 0,0 );

	m_renderer->EF_StartEf();
	m_renderer->ResetToDefault();
	m_renderer->SelectTMU(0);
	m_renderer->EnableTMU(false);

	//////////////////////////////////////////////////////////////////////////
	// Setup two infinite lights for helpers drawing.
	//////////////////////////////////////////////////////////////////////////
	CDLight light[2];
  light[0].m_Origin = m_camera.GetPos();
	light[0].m_Color.Set(1,1,1,1);
	light[0].m_SpecColor.Set(1,1,1,1);
  light[0].m_fRadius = 1000000;
	light[0].m_fStartRadius = 0;
	light[0].m_fEndRadius = 1000000;
  light[0].m_Flags |= DLF_DIRECTIONAL;
	m_renderer->EF_ADDDlight( &light[0] );
	
	light[1].m_Origin = Vec3(100000,100000,100000);
	light[1].m_Color.Set(1,1,1,1);
	light[1].m_SpecColor.Set(1,1,1,1);
  light[1].m_fRadius = 1000000;
	light[1].m_fStartRadius = 0;
	light[1].m_fEndRadius = 1000000;
  light[1].m_Flags |= DLF_DIRECTIONAL;
	m_renderer->EF_ADDDlight( &light[1] );
	//////////////////////////////////////////////////////////////////////////

	RenderAll();

	m_engine->SetupDistanceFog();
	m_renderer->EF_EndEf3D(SHDF_SORT | SHDF_ALLOWHDR);
  m_renderer->EF_ClearLightsList();

	//////////////////////////////////////////////////////////////////////////
	// Draw 2D helpers.
	//////////////////////////////////////////////////////////////////////////
	m_renderer->Set2DMode( true,m_rcClient.right,m_rcClient.bottom );
	//////////////////////////////////////////////////////////////////////////
	// Draw selected rectangle.
	//////////////////////////////////////////////////////////////////////////
	if (!m_selectedRect.IsRectEmpty())
	{
		//m_renderer->EnableBlend(true);
		//m_renderer->SetBlendMode();
  	m_renderer->SetState(GS_DEPTHWRITE | GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);
		m_renderer->SetMaterialColor( 1,1,1,0.4f );
		//CPoint p1 = ViewToWorld(m_selectedRect.
		CPoint p1 = CPoint( m_selectedRect.left,m_selectedRect.top );
		CPoint p2 = CPoint( m_selectedRect.right,m_selectedRect.bottom );
		m_renderer->DrawLine( Vec3(p1.x,p1.y,0),Vec3(p2.x,p1.y,0) );
		m_renderer->DrawLine( Vec3(p1.x,p2.y,0),Vec3(p2.x,p2.y,0) );
		m_renderer->DrawLine( Vec3(p1.x,p1.y,0),Vec3(p1.x,p2.y,0) );
		m_renderer->DrawLine( Vec3(p2.x,p1.y,0),Vec3(p2.x,p2.y,0) );
	}

	// Draw Axis arrow in lower right corner.
	if (!IsAVIRecording())
	{
		DrawAxis();
	}
	// Display cursor string.
	RenderCursorString();
	if (gSettings.viewports.bShowSafeFrame)
		RenderSafeFrame();

	m_renderer->Set2DMode( false,m_rcClient.right,m_rcClient.bottom );
}

static IRenderer *g_pRenderer;
static void g_DrawLine(float *v1,float *v2)
{
	Vec3d V1, V2;
	//V1.Set(v1);
	//V2.Set(v2);
	V1.Set(v1[0],v1[1],v1[2]);
	V2.Set(v2[0],v2[1],v2[2]);
 	g_pRenderer->DrawLine(V1,V2);
}

void CRenderViewport::RenderAll()
{
	m_renderer->ResetToDefault();

	// Draw all objects.
	DisplayContext dctx;
	dctx.settings = GetIEditor()->GetDisplaySettings();
	dctx.view = this;
	dctx.renderer = m_renderer;
	dctx.engine = m_engine;
	dctx.box.min=Vec3d( -100000,-100000,-100000 );
	dctx.box.max=Vec3d( 100000,100000,100000 );
	dctx.camera = &m_camera;
	if (!dctx.settings->IsDisplayLabels())
	{
		dctx.flags |= DISPLAY_HIDENAMES;
	}
	if (dctx.settings->IsDisplayLinks())
	{
		dctx.flags |= DISPLAY_LINKS;
	}
	if (m_bDegradateQuality)
	{
		dctx.flags |= DISPLAY_DEGRADATED;
	}
	if (dctx.settings->GetRenderFlags() & RENDER_FLAG_BBOX)
	{
		dctx.flags |= DISPLAY_BBOX;
	}

	dctx.flags |= DISPLAY_TRACKS;
	dctx.flags |= DISPLAY_TRACKTICKS;

	if (GetIEditor()->GetReferenceCoordSys() == COORDS_WORLD)
	{
		dctx.flags |= DISPLAY_WORLDSPACEAXIS;
	}

	//dctx.renderer->SetBlendMode();
	//dctx.renderer->EnableBlend(true);
	dctx.SetState( GS_DEPTHWRITE|GS_BLSRC_SRCALPHA|GS_BLDST_ONEMINUSSRCALPHA );
	GetIEditor()->GetObjectManager()->Display( dctx );
	//dctx.renderer->EnableBlend(false);
	//dctx.renderer->SetState(GS_DEPTHWRITE);

	BBox box;
	GetIEditor()->GetSelectedRegion( box );
	if (!IsEquivalent(box.min,box.max,0))
		RenderTerrainGrid( box.min.x,box.min.y,box.max.x,box.max.y );

	//RenderMarker();

	g_pRenderer = m_renderer;
	IPhysicalWorld *physWorld = GetIEditor()->GetSystem()->GetIPhysicalWorld();
	if (physWorld)
		physWorld->DrawPhysicsHelperInformation(g_DrawLine);

	IAISystem *aiSystem = GetIEditor()->GetSystem()->GetAISystem();
	if (aiSystem)
		aiSystem->DebugDraw(m_renderer);

	if (dctx.settings->GetDebugFlags() & DBG_MEMINFO)
	{
		ProcessMemInfo mi;
		CProcessInfo::QueryMemInfo( mi );
		int MB = 1024*1024;
		CString str;
		str.Format( "WorkingSet=%dMb, PageFile=%dMb, PageFaults=%d",mi.WorkingSet/MB,mi.PagefileUsage/MB,mi.PageFaultCount );
		m_renderer->TextToScreenColor( 1,1,1,0,0,1,str );
	}

	// Display editing tool.
	if (GetIEditor()->GetEditTool())
	{
		GetIEditor()->GetEditTool()->Display( dctx );
	}

	/*
	// Draw Construction plane.
	{
		m_renderer->EnableBlend(true);
		m_renderer->SetBlendMode();
		m_renderer->SetCullMode(R_CULL_NONE);
		Vec3 p = m_constructionOriginalMatrix.GetTranslationOLD();
		Vec3 n = m_constructionPlane.n;
		Vec3 dir = Vec3( 1,0,0 );
		Vec3 u = (dir.Cross(n)).GetNormalized();
		Vec3 v = (u.Cross(n)).GetNormalized();
		dctx.SetColor( 1,0,1,0.2f );
		float s = 4;
		dctx.DrawQuad( p, p+u*s,p+u*s+v*s,p+v*s );

		dctx.SetColor( 0,0,1,1 );
		dctx.DrawLine( p, p+u*s );
		dctx.DrawLine( p+u*s,p+u*s+v*s );
		dctx.DrawLine( p+u*s+v*s,p+v*s );
		dctx.DrawLine( p,p+v*s );
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
namespace {
	inline Vec3 NegY( const Vec3 &v,float y )
	{
		return Vec3(v.x,y-v.y,v.z);
	}
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::DrawAxis()
{
	float colx[4] = {1,0,0,1};// Red
	float coly[4] = {0,1,0,1};// Green
	float colz[4] = {0,0,1,1};// Blue
	float colw[4] = {1,1,1,1};// White

	int size = 25;
	Vec3 pos( 25,25,0 );

	Vec3 x(size,0,0);
	Vec3 y(0,size,0);
	Vec3 z(0,0,size);

	float height = m_rcClient.Height();
	Vec3 hvec(height,height,height);

	Matrix44 camMatrix = m_camera.GetVCMatrixD3D9();
//	camMatrix.Invert();
  //CHANGED_BY_IVO
	//x = camMatrix.TransformVector(x);
	//y = camMatrix.TransformVector(y);
	//z = camMatrix.TransformVector(z);
	x = GetTransposed44(camMatrix)*(x);
	y = GetTransposed44(camMatrix)*(y);
	z = GetTransposed44(camMatrix)*(z);

	//m_renderer->EnableDepthTest(false);
	//m_renderer->EnableDepthWrites(false);
	m_renderer->SetState(GS_NODEPTHTEST);
	m_renderer->SetCullMode( R_CULL_DISABLE );

	Vec3 src =	NegY(pos,height);
	Vec3 trgx = NegY(pos+x,height);
	Vec3 trgy = NegY(pos+y,height);
	Vec3 trgz = NegY(pos+z,height);

	m_renderer->SetMaterialColor( colx[0],colx[1],colx[2],colx[3] );
	m_renderer->DrawLine( src,trgx );
	m_renderer->SetMaterialColor( coly[0],coly[1],coly[2],coly[3] );
	m_renderer->DrawLine( src,trgy );
	m_renderer->SetMaterialColor( colz[0],colz[1],colz[2],colz[3] );
	m_renderer->DrawLine( src,trgz );

	m_renderer->Draw2dLabel( trgx.x,trgx.y,1.1f,colw,true,"x" );
	m_renderer->Draw2dLabel( trgy.x,trgy.y,1.1f,colw,true,"y" );
	m_renderer->Draw2dLabel( trgz.x,trgz.y,1.1f,colw,true,"z" );

	m_renderer->SetState(GS_DEPTHWRITE);
	//m_renderer->EnableDepthTest(true);
	//m_renderer->EnableDepthWrites(true);
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::RenderCursorString()
{
	if (m_cursorStr.IsEmpty())
		return;

	CPoint point;
	GetCursorPos( &point );
	ScreenToClient( &point );

//	float d = GetViewerPos().Distance(pos) * 0.02;

	// Display hit object name.
	float col[4] = { 1,1,1,1 };
	m_renderer->Draw2dLabel( point.x+12,point.y+4,1.2f,col,false,"%s",(const char*)m_cursorStr );
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::RenderSafeFrame()
{
	m_renderer->SetState(GS_BLSRC_SRCALPHA|GS_BLDST_ONEMINUSSRCALPHA|GS_NODEPTHTEST);
	int screenW = m_rcClient.right - m_rcClient.left;
	int screenH = m_rcClient.bottom - m_rcClient.top;	
	float fAspect = gSettings.viewports.fDefaultAspectRatio;
	int w =	 screenH/fAspect;
	int h =	 screenH;
	int sx = (screenW - w)/2;

	//float proj = (float)h/(float)w;
	m_renderer->SetMaterialColor( 0,1,1,0.5f ); // cyan
	m_renderer->DrawLine( Vec3(sx,0,0),Vec3(sx+w,0,0) );
	m_renderer->DrawLine( Vec3(sx,1,0),Vec3(sx+w,1,0) );

	m_renderer->DrawLine( Vec3(sx,h-2,0),Vec3(sx+w,h-2,0) );
	m_renderer->DrawLine( Vec3(sx,h-1,0),Vec3(sx+w,h-1,0) );

	m_renderer->DrawLine( Vec3(sx,0,0),Vec3(sx,h,0) );
	m_renderer->DrawLine( Vec3(sx-1,0,0),Vec3(sx-1,h,0) );

	m_renderer->DrawLine( Vec3(sx+w,0,0),Vec3(sx+w,h,0) );
	m_renderer->DrawLine( Vec3(sx+w+1,0,0),Vec3(sx+w+1,h,0) );
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::RenderMarker()
{
	/*
	Vec3 p = GetIEditor()->GetMarkerPosition();
	float verts[4][5];
	memset( verts,0,sizeof(verts) );
	float dist = (p - m_camera.GetPos()).Length();
	float size = 0.5f;
	float offset = 0.1f;
	float x = p.x;
	float y = p.y;

	verts[0][0] = x-size;
	verts[0][1] = y-size;
	verts[0][2] = GetIEditor()->GetTerrainElevation( x-size,y-size ) + offset;

	verts[1][0] = x+size;
	verts[1][1] = y-size;
	verts[1][2] = GetIEditor()->GetTerrainElevation( x+size,y-size ) + offset;

	verts[3][0] = x+size;
	verts[3][1] = y+size;
	verts[3][2] = GetIEditor()->GetTerrainElevation( x+size,y+size ) + offset;

	verts[2][0] = x-size;
	verts[2][1] = y+size;
	verts[2][2] = GetIEditor()->GetTerrainElevation( x-size,y+size ) + offset;

	m_renderer->SetMaterialColor( 1,0,0.8f,0.6f );
	m_renderer->SetBlendMode();
	m_renderer->EnableBlend( true );
	m_renderer->DrawTriStrip(&(CVertexBuffer(verts,VERTEX_FORMAT_P3F_TEX2F)),4);
	m_renderer->SetMaterialColor( 1,1,0,0.6f );
	m_renderer->DrawBall( p,size*0.8f );
	m_renderer->ResetToDefault();
	*/
}

inline bool SortCameraObjectsByName( CCameraObject *pObject1,CCameraObject *pObject2 )
{
	return stricmp(pObject1->GetName(),pObject2->GetName()) < 0;
}

void CRenderViewport::OnTitleMenu( CMenu &menu )
{
	m_bWireframe = GetIEditor()->GetDisplaySettings()->GetDisplayMode() == DISPLAYMODE_WIREFRAME;
	m_bDisplayLabels = GetIEditor()->GetDisplaySettings()->IsDisplayLabels();

	menu.AppendMenu( MF_STRING|(m_bWireframe)?MF_CHECKED:MF_UNCHECKED,1,"Wireframe" );
	menu.AppendMenu( MF_STRING|(m_bDisplayLabels)?MF_CHECKED:MF_UNCHECKED,2,"Labels" );
	menu.AppendMenu( MF_STRING|(gSettings.viewports.bShowSafeFrame)?MF_CHECKED:MF_UNCHECKED,3,"Show Safe Frame" );

	// Add Cameras.
	std::vector<CCameraObject*> objects;
	((CObjectManager*)GetIEditor()->GetObjectManager())->GetCameras( objects );
	if (!objects.empty())
	{
		std::sort( objects.begin(),objects.end(),SortCameraObjectsByName );
		menu.AppendMenu( MF_SEPARATOR,0,"" );
		menu.AppendMenu( MF_STRING|(m_cameraObjectId == GUID_NULL && !m_bSequenceCamera)?MF_CHECKED:MF_UNCHECKED,1000,"Default Camera" );
		menu.AppendMenu( MF_STRING|(m_bSequenceCamera)?MF_CHECKED:MF_UNCHECKED,1001,"Sequence Camera" );
		menu.AppendMenu( MF_SEPARATOR,0,"" );

		static CMenu subMenu;
		if (subMenu.m_hMenu)
			subMenu.DestroyMenu();
		subMenu.CreatePopupMenu();
		menu.AppendMenu( MF_POPUP,(UINT_PTR)subMenu.GetSafeHmenu(),"Camera" );
		for (int i = 0; i < objects.size(); i++)
		{
			int state = (m_cameraObjectId == objects[i]->GetId() && !m_bSequenceCamera)?MF_CHECKED:MF_UNCHECKED;
			subMenu.AppendMenu( MF_STRING|state,1002+i,objects[i]->GetName() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::OnTitleMenuCommand( int id )
{
	if (id >= 1000 && id < 2000)
	{
		if (id == 1000)
		{
			m_bSequenceCamera = false;
			SetCameraObject(0);
		}
		else if (id == 1001)
		{
			m_bSequenceCamera = true;
			SetCameraObject(0);
		}
		else
		{
			m_bSequenceCamera = false;
			// Switch to Camera Object.
			std::vector<CCameraObject*> objects;
			((CObjectManager*)GetIEditor()->GetObjectManager())->GetCameras( objects );
			std::sort( objects.begin(),objects.end(),SortCameraObjectsByName );
			int index = id - 1002;
			if (index < objects.size())
			{
				SetCameraObject( objects[index] );
			}
		}
	}
	switch (id)
	{
		case 1:
			m_bWireframe = !m_bWireframe;
			if (m_bWireframe)
				GetIEditor()->GetDisplaySettings()->SetDisplayMode( DISPLAYMODE_WIREFRAME );
			else
				GetIEditor()->GetDisplaySettings()->SetDisplayMode( DISPLAYMODE_SOLID );
			break;
		case 2:
			m_bDisplayLabels = !m_bDisplayLabels;
			GetIEditor()->GetDisplaySettings()->DisplayLabels( m_bDisplayLabels );
			break;
		case 3:
			gSettings.viewports.bShowSafeFrame = !gSettings.viewports.bShowSafeFrame;
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
BOOL CRenderViewport::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (GetIEditor()->IsInGameMode())
		return FALSE;

	// TODO: Add your message handler code here and/or call default
	Matrix44 m = m_camera.GetVCMatrixD3D9();
	Vec3d zdir(m[0][2],m[1][2],m[2][2]);
	zdir.Normalize();
		
	Vec3d pos = GetViewerPos();
	pos += 0.2f*zdir*(zDelta);
	SetViewerPos( pos );
//	GetIEditor()->MoveViewer(0.2f*zdir*(zDelta));
	
	return CViewport::OnMouseWheel(nFlags, zDelta, pt);
}

void CRenderViewport::SetCamera( const CCamera &camera )
{
	m_camera = camera;
	SetViewerPos( m_camera.GetPos() );
	SetViewerAngles( m_camera.GetAngles() );
}

Vec3 CRenderViewport::GetViewerPos() const
{
	CBaseObject *cameraObject = GetCameraObject();
	if (!cameraObject)
		return GetIEditor()->GetViewerPos();
	else
	{
		return cameraObject->GetWorldPos();
	}
}

Vec3 CRenderViewport::GetViewerAngles() const
{
	CBaseObject *cameraObject = GetCameraObject();
	if (!cameraObject)
		return GetIEditor()->GetViewerAngles();
	else
	{
		Matrix44 tm = cameraObject->GetWorldTM();
		tm.NoScale();

		//CHANGED_BY_IVO
		//Quat quat(tm);
		//Quat quat = CovertMatToQuat<float>( GetTransposed44(tm) );
		Quat quat = Quat( GetTransposed44(tm) );

		return RAD2DEG(Ang3::GetAnglesXYZ(Matrix33(quat)));
	}
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::SetViewerPos( const Vec3 &pos )
{
	Vec3 p = pos;
	// If no collision flag set do not check for terrain elevation.
	if ((GetIEditor()->GetDisplaySettings()->GetSettings()&SETTINGS_NOCOLLISION) == 0)
	{
		float z = GetIEditor()->GetTerrainElevation( p.x,p.y );
		if (p.z < z+1) p.z = z+1;
	}

	CBaseObject *cameraObject = GetCameraObject();
	if (!cameraObject)
	{
		GetIEditor()->SetViewerPos( p );
	}
	else
	{
		if(!m_nPresedKeyState || m_nPresedKeyState==1)
		{
			CUndo undo("Move Camera");
			Matrix44 tm = cameraObject->GetWorldTM();
			tm.SetTranslationOLD(pos);
			cameraObject->SetWorldTM( tm );
		}
		else
		{
			Matrix44 tm = cameraObject->GetWorldTM();
			tm.SetTranslationOLD(pos);
			cameraObject->SetWorldTM( tm );
		}
	}

	if(m_nPresedKeyState==1)
		m_nPresedKeyState=2;

	UpdateConstrPlane();
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::SetViewerAngles( const Vec3 &angles )
{
	CBaseObject *cameraObject = GetCameraObject();
	if (!cameraObject)
	{
		GetIEditor()->SetViewerAngles( angles );
	}
	else
	{
		if(!m_nPresedKeyState	|| m_nPresedKeyState==1)
		{
			CUndo	undo("Turn Camera");
			Matrix44 tm;
			tm.SetIdentity();

			//tm.RotateMatrix_fix( angles	);
			tm=Matrix44::CreateRotationZYX(-angles*gf_DEGTORAD)*tm;	//NOTE:	angles in	radians	and	negated	

			Vec3 pos = cameraObject->GetWorldTM().GetTranslationOLD();
			tm.SetTranslationOLD(pos);
			cameraObject->SetWorldTM(	tm );
		}
		else
		{
			Matrix44 tm;
			tm.SetIdentity();

			//tm.RotateMatrix_fix( angles	);
			tm=Matrix44::CreateRotationZYX(-angles*gf_DEGTORAD)*tm;	//NOTE:	angles in	radians	and	negated	

			Vec3 pos = cameraObject->GetWorldTM().GetTranslationOLD();
			tm.SetTranslationOLD(pos);
			cameraObject->SetWorldTM(	tm );
		}
	}

	if(m_nPresedKeyState==1)
		m_nPresedKeyState=2;

	UpdateConstrPlane();
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::RenderTerrainGrid( float x1,float y1,float x2,float y2 )
{
	if (!m_engine)
		return;

	float x,y;

	struct_VERTEX_FORMAT_P3F_TEX2F verts[4];
	memset( verts,0,sizeof(verts) );

	float step = MAX( y2-y1,x2-x1 );
	if (step < 0.1)
		return;
	step = step / 100.0f;
	
	//if (step > 2) step = 2;
	//x1 = (((int)x1)/2)*2;
	//y1 = (((int)y1)/2)*2;
	//x2 = (((int)x2)/2)*2;
	//y2 = (((int)y2)/2)*2;
	m_renderer->SetMaterialColor( 1,1,1,1 );

	float z1 = m_engine->GetTerrainElevation( x1,y1 );
	float z2 = m_engine->GetTerrainElevation( x2,y2 );
	float z3 = m_engine->GetTerrainElevation( x2,y1 );
	float z4 = m_engine->GetTerrainElevation( x1,y2 );

	m_renderer->DrawLine( Vec3(x1,y1,z1),Vec3(x1,y1,z1+0.5) );
	m_renderer->DrawLine( Vec3(x2,y2,z2),Vec3(x2,y2,z2+0.5) );
	m_renderer->DrawLine( Vec3(x2,y1,z3),Vec3(x2,y1,z3+0.5) );
	m_renderer->DrawLine( Vec3(x1,y2,z4),Vec3(x1,y2,z4+0.5) );

	m_renderer->SetMaterialColor( 1,0,0,0.5 );
	m_renderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
	//m_renderer->SetBlendMode();
	//m_renderer->EnableBlend( true );
	//m_renderer->EnableDepthTest( false );

	float offset = 0.01f;
	for (y = y1; y < y2+step; y += step)
	{
		for (x = x1; x < x2+step; x += step)
		{
			verts[0].xyz.x = x;
			verts[0].xyz.y = y;
			verts[0].xyz.z = m_engine->GetTerrainElevation( x,y ) + offset;

			verts[1].xyz.x = x+step;
			verts[1].xyz.y = y;
			verts[1].xyz.z = m_engine->GetTerrainElevation( x+step,y ) + offset;

			verts[3].xyz.x = x+step;
			verts[3].xyz.y = y+step;
			verts[3].xyz.z = m_engine->GetTerrainElevation( x+step,y+step ) + offset;

			verts[2].xyz.x = x;
			verts[2].xyz.y = y+step;
			verts[2].xyz.z = m_engine->GetTerrainElevation( x,y+step ) + offset;
			
			//m_renderer->DrawLine( Vec3d(x,y,z),Vec3d(x+step,y,z) );
			//m_renderer->DrawLine( Vec3d(x,y,z),Vec3d(x,y+step,z) );
			/*
			float 
			//m_renderer->DrawLine( Vec3d(x,y,z),Vec3d(x+step,y,z) );
			//m_renderer->DrawLine( Vec3d(x,y,z),Vec3d(x,y+step,z) );
			m_renderer->DrawQuad( x+step*0.5f,y+step*0.5f,(z1+z2+z3+z4)/4 );

			float data[] = 
			{
				-dx+x, dy+y,-object_Scale4+z,  0, 0, // 1,1,1,alpha,//NOTE: totaly stupid
					dx+x,-dy+y,-object_Scale4+z,   0, 0, // 1,1,1,alpha,
					-dx+x, dy+y, object_Scale4+z,  -1, 1, // 1,1,1,alpha,
					dx+x,-dy+y, object_Scale4+z,   0, 1, // 1,1,1,alpha,
			};
			
			GetRenderer()->SetMaterialColor(1,1,1,alpha);
			GetRenderer()->SetTexture(tid);
			*/
			m_renderer->DrawTriStrip(&(CVertexBuffer(verts,VERTEX_FORMAT_P3F_TEX2F)),4);
		}
	}


	Vec3 p1,p2;
	// Draw yellow border lines.
	m_renderer->SetMaterialColor( 1,1,0,1 );

	for (y = y1; y < y2+step; y += step)
	{
		p1.x = x1;
		p1.y = y;
		p1.z = m_engine->GetTerrainElevation( p1.x,p1.y ) + offset;

		p2.x = x1;
		p2.y = y+step;
		p2.z = m_engine->GetTerrainElevation( p2.x,p2.y ) + offset;
		m_renderer->DrawLine( p1,p2 );

		p1.x = x2+step;
		p1.y = y;
		p1.z = m_engine->GetTerrainElevation( p1.x,p1.y ) + offset;

		p2.x = x2+step;
		p2.y = y+step;
		p2.z = m_engine->GetTerrainElevation( p2.x,p2.y ) + offset;
		m_renderer->DrawLine( p1,p2 );
	}
	for (x = x1; x < x2+step; x += step)
	{
		p1.x = x;
		p1.y = y1;
		p1.z = m_engine->GetTerrainElevation( p1.x,p1.y ) + offset;

		p2.x = x+step;
		p2.y = y1;
		p2.z = m_engine->GetTerrainElevation( p2.x,p2.y ) + offset;
		m_renderer->DrawLine( p1,p2 );

		p1.x = x;
		p1.y = y2+step;
		p1.z = m_engine->GetTerrainElevation( p1.x,p1.y ) + offset;

		p2.x = x+step;
		p2.y = y2+step;
		p2.z = m_engine->GetTerrainElevation( p2.x,p2.y ) + offset;
		m_renderer->DrawLine( p1,p2 );
	}

	//m_renderer->EnableDepthTest(true);
	m_renderer->SetState(GS_DEPTHWRITE);

	m_renderer->ResetToDefault();
}

void CRenderViewport::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	/*
	m_camera.Update();
	Matrix m = m_camera.GetMatrix();		
	Vec3d zdir(m[0][2],m[1][2],m[2][2]);
	zdir.Normalize();
	Vec3d xdir(m[0][0],m[1][0],m[2][0]);
	xdir.Normalize();
		
	//m_camera.SetPos( m_camera.GetPos() + ydir*(m_mousePos.y-point.y),xdir*(m_mousePos.x-point.x) );
	Vec3d pos = GetViewerPos();

	if (nChar == VK_UP)
	{
		// move forward
		pos = pos - 0.5f*zdir;
		SetViewerPos( pos );
	}
	if (nChar == VK_DOWN)
	{
		// move backward
		pos = pos + 0.5f*zdir;
		SetViewerPos( pos );
	}
	if (nChar == VK_LEFT)
	{
		// move left
		pos = pos - 0.5f*xdir;
		SetViewerPos( pos );
	}
	if (nChar == VK_RIGHT)
	{
		// move right
		pos = pos + 0.5f*xdir;
		SetViewerPos( pos );
	}
	*/
	
	CViewport::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CRenderViewport::ProcessKeys()
{
	if (GetFocus() != this)
		return;

	m_camera.Update();
	Matrix44 m = m_camera.GetVCMatrixD3D9();		
	Vec3d zdir(m[0][2],m[1][2],m[2][2]);
	zdir.Normalize();
	Vec3d xdir(m[0][0],m[1][0],m[2][0]);
	xdir.Normalize();
		
	//m_camera.SetPos( m_camera.GetPos() + ydir*(m_mousePos.y-point.y),xdir*(m_mousePos.x-point.x) );
	Vec3d pos = GetViewerPos();

	IConsole * console = GetIEditor()->GetSystem()->GetIConsole();

	float speedScale = 60.0f * GetIEditor()->GetSystem()->GetITimer()->GetFrameTime();
	if (speedScale > 20) speedScale = 20;

	speedScale *= gSettings.cameraMoveSpeed;

	if (CheckVirtualKey(VK_SHIFT))
	{
		speedScale *= gSettings.cameraFastMoveSpeed;
	}

	bool bCtrl = CheckVirtualKey(VK_LCONTROL) || CheckVirtualKey(VK_RCONTROL);
	//bool bAlt = GetAsyncKeyState(VK_LALT) || GetAsyncKeyState(VK_RCONTROL);

	if (bCtrl)
		return;

  bool bIsPressedSome = false;

	if (CheckVirtualKey(VK_UP) || CheckVirtualKey('W'))
	{
		// move forward
		bIsPressedSome = true;
		if(!m_nPresedKeyState)
			m_nPresedKeyState	=	1;
		pos = pos - speedScale*m_moveSpeed*zdir;
		SetViewerPos( pos );
		//GetIEditor()->MoveViewer(-speedScale*m_moveSpeed*zdir);
	}

	if (CheckVirtualKey(VK_DOWN) || CheckVirtualKey('S'))
	{
		// move backward
		bIsPressedSome = true;
		if(!m_nPresedKeyState)
			m_nPresedKeyState	=	1;
		pos	=	pos	+	speedScale*m_moveSpeed*zdir;
		SetViewerPos( pos );
		//GetIEditor()->MoveViewer(speedScale*m_moveSpeed*zdir);
	}

	if (CheckVirtualKey(VK_LEFT) || CheckVirtualKey('A'))
	{
		// move left
		bIsPressedSome = true;
		if(!m_nPresedKeyState)
			m_nPresedKeyState	=	1;
		pos	=	pos	-	speedScale*m_moveSpeed*xdir;
		SetViewerPos( pos );
		//GetIEditor()->MoveViewer(-speedScale*m_moveSpeed*xdir);
	}

	if (CheckVirtualKey(VK_RIGHT) || CheckVirtualKey('D'))
	{
		// move right
		bIsPressedSome = true;
		if(!m_nPresedKeyState)
			m_nPresedKeyState	=	1;
		pos	=	pos	+	speedScale*m_moveSpeed*xdir;
		SetViewerPos( pos );
		//GetIEditor()->MoveViewer(speedScale*m_moveSpeed*xdir);
	}

	if (CheckVirtualKey(VK_RBUTTON))
	{
		bIsPressedSome = true;
	}

	if(!bIsPressedSome)
		m_nPresedKeyState=0;

}

//////////////////////////////////////////////////////////////////////////
CPoint CRenderViewport::WorldToView( Vec3 wp )
{
	CPoint p;
	float x,y,z;

	m_renderer->ProjectToScreen( wp.x,wp.y,wp.z,&x,&y,&z );
	p.x = (x / 100) * m_rcClient.Width();
	p.y = (y / 100) * m_rcClient.Height();
	return p;
}

//////////////////////////////////////////////////////////////////////////
Vec3	CRenderViewport::ViewToWorld( CPoint vp,bool *collideWithTerrain,bool onlyTerrain )
{
	if (!m_renderer)
		return Vec3(0,0,0);

	m_renderer->SetCurrentContext( m_hWnd );

	CRect rc = m_rcClient;
	//m_renderer->ChangeViewport( 0,0,rc.right-rc.left,rc.bottom-rc.top );

	Vec3 pos0,pos1;
	float wx,wy,wz;
	m_renderer->UnProjectFromScreen( vp.x,rc.bottom-vp.y,0,&wx,&wy,&wz );
	if (!_finite(wx) || !_finite(wy) || !_finite(wz))
		return Vec3(0,0,0);
	pos0( wx,wy,wz );

	m_renderer->UnProjectFromScreen( vp.x,rc.bottom-vp.y,1,&wx,&wy,&wz );
	if (!_finite(wx) || !_finite(wy) || !_finite(wz))
		return Vec3(0,0,0);
	pos1( wx,wy,wz );

	Vec3 v = (pos1-pos0);
	v = GetNormalized(v);
	v = v*2000.0f;

	if (!_finite(v.x) || !_finite(v.y) || !_finite(v.z))
		return Vec3(0,0,0);
	/*
	GetIEditor()->GetSystem()->GetILog()->Log( "x:%f, y%f, z:%f",v.x,v.y,v.z );
	*/

	Vec3 colp(0,0,0);

	IPhysicalWorld *world = GetIEditor()->GetSystem()->GetIPhysicalWorld();
	if (!world)
		return colp;
	
	vectorf vPos(pos0.x,pos0.y,pos0.z);
	vectorf vDir(v.x,v.y,v.z);
	int objTypes = ent_terrain;
	if (!onlyTerrain && !GetIEditor()->IsTerrainAxisIgnoreObjects())
		objTypes |= ent_static;
	int flags = rwi_stop_at_pierceable|rwi_ignore_terrain_holes;
	//flags = 31;
	ray_hit hit;
	hit.pCollider = 0;
	int col = world->RayWorldIntersection( vPos,vDir,objTypes,flags,&hit,1 );

	bool hitTerrain = hit.bTerrain;
	if (hit.dist > 0 && !hit.bTerrain && hit.pCollider != 0)
	{
		// Check if we collided with collision entity of selected object. 
		bool bCollidedWithSelection = false;
		CSelectionGroup *sel = GetIEditor()->GetSelection();
		for (int i = 0; i < sel->GetCount(); i++)
		{
			if (sel->GetObject(i)->GetCollisionEntity() == hit.pCollider)
			{
				bCollidedWithSelection = true;
				break;
			}
		}
		if (bCollidedWithSelection)
		{
			// Repeat collision test, but skip selected collider.
			col = world->RayWorldIntersection( vPos,vDir,objTypes,flags,&hit,1,hit.pCollider );

			/*
			pe_status_pos statusPos;
			hit.pCollider->GetStatus( &statusPos );
			Vec3 size = statusPos.BBox[1] - statusPos.BBox[0];
			if (size.Length() < 20)
			{
				// We collided small static object, ignore it, and collide only with terrain.
				objTypes = ent_terrain;
				col = world->RayWorldIntersection( vPos,vDir,objTypes,flags,&hit,1 );
				hitTerrain = true;
			}
			*/
		}
	}

	if (collideWithTerrain)
		*collideWithTerrain = hitTerrain;
	
	if (hit.dist > 0)
	{ 
		if (hit.pCollider != 0 && !hit.bTerrain)
		{
			//pe_status_pos statusPos;
			//hit.pCollider->GetStatus( &statusPos );
			//BBox box( statusPos.BBox[0],statusPos.BBox[1] );
		}
		colp = hit.pt;
		if (hitTerrain)
		{
			colp.z = m_engine->GetTerrainElevation( colp.x,colp.y );
		}
	}

	return colp;
}

//////////////////////////////////////////////////////////////////////////
void	CRenderViewport::ViewToWorldRay( CPoint vp,Vec3 &raySrc,Vec3 &rayDir )
{
	if (!m_renderer)
		return;

	CRect rc = m_rcClient;

	m_renderer->SetCurrentContext( m_hWnd );
	//m_renderer->ChangeViewport( 0,0,rc.right-rc.left,rc.bottom-rc.top );

	Vec3 pos0,pos1;
	float wx,wy,wz;
	m_renderer->UnProjectFromScreen( vp.x,rc.bottom-vp.y,0,&wx,&wy,&wz );
	if (!_finite(wx) || !_finite(wy) || !_finite(wz))
		return;
	if (fabs(wx) > 1000000 || fabs(wy) > 1000000 || fabs(wz) > 1000000)
		return;
	pos0( wx,wy,wz );
	m_renderer->UnProjectFromScreen( vp.x,rc.bottom-vp.y,1,&wx,&wy,&wz );
	if (!_finite(wx) || !_finite(wy) || !_finite(wz))
		return;
	if (fabs(wx) > 1000000 || fabs(wy) > 1000000 || fabs(wz) > 1000000)
		return;
	pos1( wx,wy,wz );

	Vec3 v = (pos1-pos0);
	v = GetNormalized(v);

	raySrc = pos0;
	rayDir = v;
}

//////////////////////////////////////////////////////////////////////////
float CRenderViewport::GetScreenScaleFactor( const Vec3 &worldPoint )
{
	float dist = GetDistance( m_camera.GetPos(), worldPoint );
	if (dist < m_camera.GetZMin())
		dist = m_camera.GetZMin();
	return dist;
}

BOOL CRenderViewport::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_hCurrCursor == NULL && !m_cursorStr.IsEmpty())
	{
		m_cursorStr = "";
		// Display cursor string.
	}
	return CViewport::OnSetCursor(pWnd, nHitTest, message);
}

void CRenderViewport::OnDestroy()
{
	DestroyRenderContext();
	CViewport::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::DrawTextLabel( DisplayContext &dc,const Vec3& pos,float size,const CFColor& color,const char *text )
{
	float col[4] = { color.r,color.g,color.b,color.a };
	m_renderer->DrawLabelEx( pos,size,col,true,true,text );
}

//////////////////////////////////////////////////////////////////////////
bool CRenderViewport::HitTest( CPoint point,ObjectHitInfo &hitInfo,int flags )
{
	hitInfo.camera = &m_camera;
	return CViewport::HitTest( point,hitInfo,flags );
}

//////////////////////////////////////////////////////////////////////////
bool CRenderViewport::IsBoundsVisible( const BBox &box ) const
{
	// If at least part of bbox is visible then its visible.
	return m_camera.IsAABBVisibleFast( AABB(box.min,box.max) );
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::CenterOnSelection()
{
	if (!GetIEditor()->GetSelection()->IsEmpty())
	{
		CSelectionGroup *sel = GetIEditor()->GetSelection();
		BBox bounds = sel->GetBounds();
		Vec3 selPos = sel->GetCenter();
		float size = GetLength(bounds.max - bounds.min);
		Vec3 pos = selPos;
		pos += Vec3(0,size*2,size);
		//pos.z = GetIEditor()->GetTerrainElevation(pos.x,pos.y)+5;
		GetIEditor()->SetViewerPos( pos );
		Vec3 dir = GetNormalized(selPos - pos);
		dir=ConvertVectorToCameraAngles(dir);
		GetIEditor()->SetViewerAngles( dir );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CRenderViewport::CreateRenderContext()
{
	// Create context.
	if (m_renderer && !m_bRenderContextCreated)
	{
		m_bRenderContextCreated = true;
		m_renderer->CreateContext( m_hWnd );
		// Make main context current.
		m_renderer->MakeCurrent();
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CRenderViewport::DestroyRenderContext()
{
	// Destroy render context.
	if (m_renderer && m_bRenderContextCreated)
	{
		// Do not delete primary context.
		if (m_hWnd != m_renderer->GetHWND())
			m_renderer->DeleteContext(m_hWnd);
		m_bRenderContextCreated = false;
	}
}
void CRenderViewport::OnSwitchcameraDefaultcamera()
{
	m_bSequenceCamera = false;
	SetCameraObject(0);
}

void CRenderViewport::OnSwitchcameraSequencecamera()
{
	m_bSequenceCamera = true;
	SetCameraObject(0);
}

void CRenderViewport::OnSwitchcameraSelectedcamera()
{
	CBaseObject *pObject = GetIEditor()->GetSelectedObject();
	if (pObject && pObject->IsKindOf(RUNTIME_CLASS(CCameraObject)))
		SetCameraObject( pObject );
}