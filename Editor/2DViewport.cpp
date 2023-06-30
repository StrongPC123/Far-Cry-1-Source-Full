// TopRendererWnd.cpp: implementation of the C2DViewport class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "2DViewport.h"
#include "CryEditDoc.h"

#include "Grid.h"
#include "DisplaySettings.h"
#include "EditTool.h"
#include "ViewManager.h"
#include "Settings.h"
#include "Objects\ObjectManager.h"

// Include OpenGL
//#include "GL\gl.h"
#include "Brush\Brush.h"
#include "Objects\BrushObject.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MARKER_SIZE 6.0f
#define MARKER_DIR_SIZE 10.0f
#define SELECTION_RADIUS 30.0f

#define GL_RGBA 0x1908
#define GL_BGRA 0x80E1

#define BACKGROUND_COLOR Vec3(1.0f,1.0f,1.0f)
#define SELECTION_RECT_COLOR Vec3(0.8f,0.8f,0.8f)
#define MINOR_GRID_COLOR	Vec3(0.55f,0.55f,0.55f)
#define MAJOR_GRID_COLOR	Vec3(0.6f,0.6f,0.6f)
#define AXIS_GRID_COLOR		Vec3(0,0,0)
#define GRID_TEXT_COLOR		Vec3(0,0,1.0f)

#define MAX_WORLD_SIZE 10000

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(C2DViewport,CViewport)

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(C2DViewport, CViewport)
	//{{AFX_MSG_MAP(C2DViewport)
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
static int m_fontList = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
C2DViewport::C2DViewport()
{
	CLogFile::WriteLine("2D Viewport Created");

	// Scroll offset equals origin
	m_rcSelect.SetRect( 0,0,0,0 );

	m_viewType = ET_ViewportXY;
	m_axis = VPA_XY;

	m_bShowTerrain = true;
	m_gridAlpha = 1;

	m_origin2D.Set(0,0,0);

	//m_colorGridText = RGB(0,0,255);
	//m_colorAxisText = RGB(0,0,0);
	//m_colorBackground = Vec2Rgb(BACKGROUND_COLOR);
	//m_gridAlpha = 0.3f;
	m_colorGridText = RGB(220,220,220);
	m_colorAxisText = RGB(220,220,220);
	m_colorBackground = RGB(128,128,128);

	m_screenTM.SetIdentity();

	m_fZoomFactor = 1;
}

//////////////////////////////////////////////////////////////////////////
C2DViewport::~C2DViewport()
{
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::SetType( EViewportType type )
{
	//assert( type == ET_ViewportXY || type == ET_ViewportXZ || type == ET_ViewportYZ );
	m_viewType = type;

	switch (m_viewType)
	{
	case ET_ViewportXY:
		m_axis = VPA_XY;
		break;
	case ET_ViewportXZ:
		m_axis = VPA_XZ;
		break;
	case ET_ViewportYZ:
		m_axis = VPA_YZ;
		break;
	};

	SetAxis( m_axis );
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::SetAxis( EViewportAxis axis )
{
	m_axis = axis;
	switch (m_axis)
	{
	case VPA_XY:
		m_cullAxis = 2;
		break;
	case VPA_XZ:
		m_cullAxis = 1;
		break;
	case VPA_YZ:
		m_cullAxis = 0;
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::CalculateViewTM()
{
	Matrix44 tm;
	tm.SetIdentity();
	m_constructionViewTM.SetIdentity();
	float fScale = GetZoomFactor();
	Vec3 origin = GetOrigin2D();
	//float origin[2] = { -m_cScrollOffset.x,-m_cScrollOffset.y };

	float height = m_rcClient.Height()/fScale;

	Vec3 v1;
	switch (m_axis)
	{
		case VPA_XY:

			tm = Matrix33::CreateScale( Vec3(fScale,-fScale,fScale) ) * tm;	// No fScale for Z

			tm.SetTranslationOLD( Vec3(-origin.x,height+origin.y,0)*fScale );
			break;
		case VPA_XZ:
			//tm.ScaleMatrix( fScale,fScale,fScale );
			//tm.RotateX( 90 * PI/180.0f );
			m_constructionViewTM.BuildFromVectors( Vec3(1,0,0),Vec3(0,0,1),Vec3(0,1,0),Vec3(0,0,0) );
			//m_constructionViewTM.BuildFromVectors( Vec3(1,0,0),Vec3(0,0,1),Vec3(0,1,0),Vec3(0,0,0) );
			tm.BuildFromVectors( Vec3(1,0,0)*fScale,Vec3(0,0,1)*fScale,Vec3(0,-1,0)*fScale,Vec3(0,0,0) );
			tm.SetTranslationOLD( Vec3(-origin.x,height+origin.z,0)*fScale );
			break;

		case VPA_YZ:
			//tm.RotateY( 180 * PI/180.0f );
			m_constructionViewTM.BuildFromVectors( Vec3(0,1,0),Vec3(0,0,1),Vec3(1,0,0),Vec3(0,0,0) );
			//m_constructionViewTM.BuildFromVectors( Vec3(0,0,1),Vec3(1,0,0),Vec3(0,1,0),Vec3(0,0,0) );
			tm.BuildFromVectors( Vec3(0,0,1)*fScale,Vec3(1,0,0)*fScale,Vec3(0,-1,0)*fScale,Vec3(0,0,0) ); // No fScale for Z
			tm.SetTranslationOLD( Vec3(-origin.y,height+origin.z,0)*fScale );
			break;
	}
  SetViewTM( m_constructionViewTM );
	m_screenTM = tm;
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::ResetContent()
{
	CViewport::ResetContent();
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::UpdateContent( int flags )
{
	CViewport::UpdateContent(flags);
	if (flags & eUpdateObjects)
	{
		Invalidate(FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_bRMouseDown = true;
	SetObjectCursor(0);

	if (GetIEditor()->IsInGameMode())
		return;

	if (GetIEditor()->GetEditTool())
	{
		GetIEditor()->GetEditTool()->MouseCallback( this,eMouseRDown,point,nFlags );
	}

	// Save the mouse down position
	m_RMouseDownPos = point;

	m_prevZoomFactor = GetZoomFactor();
	//m_prevScrollOffset = m_cScrollOffset;

	CaptureMouse();
	SetViewMode( ScrollZoomMode );
	
	CViewport::OnRButtonDown(nFlags, point);

	Invalidate(FALSE);
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::OnRButtonUp(UINT nFlags, CPoint point) 
{
	m_bRMouseDown = false;
	ReleaseMouse();
	SetViewMode( NothingMode );
	
	CViewport::OnRButtonUp(nFlags, point);

	GetViewManager()->UpdateViews(eUpdateObjects);
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::OnMButtonDown(UINT nFlags, CPoint point)
{
	////////////////////////////////////////////////////////////////////////
	// User pressed the middle mouse button
	////////////////////////////////////////////////////////////////////////

	CViewport::OnMButtonDown(nFlags, point);

	// Save the mouse down position
	m_RMouseDownPos = point;

	/*
	// Are we in spline mode or scroll / zoom mode ?
	if (GetViewMode() == SplineMode)
	{
		// TODO
	}
	else*/ 
	CaptureMouse();
	Invalidate(FALSE);
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::OnMButtonUp(UINT nFlags, CPoint point)
{
	ReleaseMouse();
	Invalidate(FALSE);
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::OnLButtonDown(UINT nFlags, CPoint point)
{
	////////////////////////////////////////////////////////////////////////
	// User pressed the left mouse button
	////////////////////////////////////////////////////////////////////////
	if (GetViewMode() != NothingMode)
		return;

	m_cMouseDownPos = point;
	m_prevZoomFactor = GetZoomFactor();
	//m_prevScrollOffset = m_cScrollOffset;

	CViewport::OnLButtonDown(nFlags, point);
	Invalidate(FALSE);
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::OnLButtonUp(UINT nFlags, CPoint point)
{
	////////////////////////////////////////////////////////////////////////
	// Process the various events depending on the selection and the view 
	// mode
	////////////////////////////////////////////////////////////////////////
	CViewport::OnLButtonUp(nFlags, point);
	
	GetViewManager()->UpdateViews(eUpdateObjects);
}

//////////////////////////////////////////////////////////////////////////
BOOL C2DViewport::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	float z = GetZoomFactor();
	float scale = 1.2f * fabs(zDelta/120.0f);
	if (zDelta > 0)
	{
		z = z * scale;
	}
	else
	{
		z = z / scale;
	}
	SetZoom( z,m_cMousePos );

	GetViewManager()->UpdateViews(eUpdateObjects);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::OnMouseMove(UINT nFlags, CPoint point)
{	
	m_cMousePos = point;

	if (GetViewMode() == ScrollZoomMode)
	{
		CRect rc;
		// You can only scroll while the middle mouse button is down
		if (nFlags & MK_RBUTTON || nFlags & MK_MBUTTON)
		{
			if (nFlags & MK_SHIFT)
			{
				// Get the dimensions of the window
				GetClientRect(&rc);

				CRect rc;
				GetClientRect( rc );
				int w = rc.right;
				int h = rc.bottom;

				// Zoom to mouse position.
				float z = m_prevZoomFactor + (point.y - m_RMouseDownPos.y) * 0.02f;
				SetZoom( z,m_RMouseDownPos );
			}
			else
			{
				// Set the new scrolled coordinates
				float fScale = GetZoomFactor();
				float ofsx,ofsy;
				GetScrollOffset( ofsx,ofsy );
				ofsx -= (point.x - m_RMouseDownPos.x)/fScale;
				ofsy += (point.y - m_RMouseDownPos.y)/fScale;
				SetScrollOffset( ofsx,ofsy );
				m_RMouseDownPos = point;
			}
		}
		return;
	}

	CViewport::OnMouseMove(nFlags, point);

	//////////////////////////////////////////////////////////////////////////
	//@FIXME: REMOVE
	//GetViewManager()->UpdateViews(eUpdateObjects);
	Invalidate(FALSE);
	//////////////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::SetScrollOffset( float x,float y,bool bLimits )
{
	if (bLimits)
	{
		float maxMapSize = 4096;
		// Limit scroll offsets.
		x = max(x,-maxMapSize);
		y = max(y,-maxMapSize);
		x = min(x,maxMapSize);
		y = min(y,maxMapSize);
	}

	Vec3 org = GetOrigin2D();
	switch (m_axis)
	{
		case VPA_XY:
			org.x = x;	org.y = y;
			break;
		case VPA_XZ:
			org.x = x;	org.z = y;
			break;
		case VPA_YZ:
			org.y = x;	org.z = y;
			break;
	}

	SetOrigin2D(org);

	CalculateViewTM();
	//GetViewManager()->UpdateViews(eUpdateObjects);
	Invalidate(FALSE);
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::GetScrollOffset( float &x,float &y )
{
	Vec3 origin = GetOrigin2D();
	switch (m_axis)
	{
		case VPA_XY:
			x = origin.x;
			y = origin.y;
			break;
		case VPA_XZ:
			x = origin.x;
			y = origin.z;
			break;
		case VPA_YZ:
			x = origin.y;
			y = origin.z;
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::SetZoom( float fZoomFactor,CPoint center )
{
	if (fZoomFactor < 0.01f)
		fZoomFactor = 0.01f;

	float prevz = GetZoomFactor();

	// Zoom to mouse position.
	float ofsx,ofsy;
	GetScrollOffset( ofsx,ofsy );
	
	float s1 = GetZoomFactor();
	float s2 = fZoomFactor;

	SetZoomFactor( fZoomFactor );

	// Calculate new offset to center zoom on mouse.
	float x2 = center.x;
	float y2 = m_rcClient.Height() - center.y;
	ofsx = -(x2/s2 - x2/s1 - ofsx);
	ofsy = -(y2/s2 - y2/s1 - ofsy);
	SetScrollOffset( ofsx,ofsy,false );

	CalculateViewTM();
	
	//GetViewManager()->UpdateViews(eUpdateObjects);
	Invalidate(FALSE);
}

//////////////////////////////////////////////////////////////////////////
afx_msg void C2DViewport::OnSize(UINT nType, int cx, int cy)
{
	////////////////////////////////////////////////////////////////////////
	// Re-evaluate the zoom / scroll offset values
	// TODO: Restore the zoom rectangle instead of resetting it
	////////////////////////////////////////////////////////////////////////

	CViewport::OnSize(nType, cx, cy);

	GetClientRect( &m_rcClient );
	CalculateViewTM();
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::OnPaint() 
{
	CRect rc;
	CPaintDC dc(this); // device context for painting
	
	//Draw( dc,dc.m_ps.rcPaint );
	Render();
}

//////////////////////////////////////////////////////////////////////////
BOOL C2DViewport::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
int C2DViewport::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CViewport::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_renderer = GetIEditor()->GetRenderer();
	if (!m_renderer)
	{
    // Create renderer.
		m_renderer = GetIEditor()->GetSystem()->CreateRenderer( false,AfxGetInstanceHandle(),GetSafeHwnd() );
	}
	
	if (m_renderer)
	{
    m_renderer->CreateContext( m_hWnd );
		m_renderer->MakeCurrent();


		/*
		if (!m_fontList)
		{
			// Create font list for this view.
			// create GL font

			CFont font;

			font.CreateFont(
				12, // logical height of font 
				6,  // logical average character width 
				0,  // angle of escapement 
				0,  // base-line orientation angle 
				0,  // font weight 
				0,  // italic attribute flag 
				0,  // underline attribute flag 
				0,  // strikeout attribute flag 
				0,  // character set identifier 
				0,  // output precision 
				0,  // clipping precision 
				0,  // output quality 
				0,  // pitch and family 
				"system font"   // pointer to typeface name string 
				);

			CClientDC dc(this);
			CFont* def_font = dc.SelectObject(&font);
			
		  m_fontList = glGenLists (256);
			if (m_fontList == 0)
				CLogFile::WriteLine( "Couldn't create font drawlists" );

			if (!wglUseFontBitmapsA( dc.GetSafeHdc(), 0, 255, m_fontList) )
				CLogFile::WriteLine( "wglUseFontBitmaps failed" );

			dc.SelectObject(def_font);
			// Done with the font. Delete the font object.
			font.DeleteObject();
		}
		*/
	}

	// Caluclate view tranform.
	CalculateViewTM();
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::Update()
{
	//Render();
	CViewport::Update();
}

//////////////////////////////////////////////////////////////////////////
CPoint C2DViewport::WorldToView( Vec3 wp )
{
	CPoint p;
	
	float fScale = GetZoomFactor();
	Vec3 origin = GetOrigin2D();

	int ix = 0;
	int iy = 0;
	switch (m_axis)
	{
	case VPA_XY:
		ix = 0; iy = 1;
		break;
	case VPA_XZ:
		ix = 0; iy = 2;
		break;
	case VPA_YZ:
		ix = 1; iy = 2;
		break;
	}

	p.x = (wp[ix] - origin[ix])*fScale;
	p.y = m_rcClient.Height() - (wp[iy] - origin[iy])*fScale;

	return p;
}

//////////////////////////////////////////////////////////////////////////
Vec3	C2DViewport::ViewToWorld( CPoint vp,bool *collideWithTerrain,bool onlyTerrain )
{
	Vec3 wp(0,0,0);

	float fScale = GetZoomFactor();
	Vec3 origin = GetOrigin2D();

	int ix = 0;
	int iy = 0;
	switch (m_axis)
	{
		case VPA_XY:
			ix = 0; iy = 1;
			break;
		case VPA_XZ:
			ix = 0; iy = 2;
			break;
		case VPA_YZ:
			ix = 1; iy = 2;
			break;
	}
	wp[ix] = (vp.x)/fScale + origin[ix];
	wp[iy] = (m_rcClient.Height()-vp.y)/fScale + origin[iy];
	return wp;
}

//////////////////////////////////////////////////////////////////////////
void	C2DViewport::ViewToWorldRay( CPoint vp,Vec3 &raySrc,Vec3 &rayDir )
{
	raySrc = ViewToWorld( vp );
	switch (m_axis)
	{
		case VPA_XY:
			raySrc.z = MAX_WORLD_SIZE;
			rayDir(0,0,-1);
			break;
		case VPA_XZ:
			raySrc.y = MAX_WORLD_SIZE;
			rayDir(0,-1,0);
			break;
		case VPA_YZ:
			raySrc.x = MAX_WORLD_SIZE;
			rayDir(-1,0,0);
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
float C2DViewport::GetScreenScaleFactor( const Vec3 &worldPoint )
{
	return 400.0f / GetZoomFactor();
	//return 100.0f / ;
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::OnTitleMenu( CMenu &menu )
{
	bool labels = GetIEditor()->GetDisplaySettings()->IsDisplayLabels();
	bool bGrid = GetIEditor()->GetViewManager()->GetGrid()->bEnabled;
	menu.AppendMenu( MF_STRING|(labels)?MF_CHECKED:MF_UNCHECKED,1,"Labels" );
	menu.AppendMenu( MF_STRING|(bGrid)?MF_CHECKED:MF_UNCHECKED,2,"Grid" );
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::OnTitleMenuCommand( int id )
{
	switch (id)
	{
		case 1:
			GetIEditor()->GetDisplaySettings()->DisplayLabels( !GetIEditor()->GetDisplaySettings()->IsDisplayLabels() );
			break;
		case 2:
			{
				CGrid *grid = GetIEditor()->GetViewManager()->GetGrid();
				grid->Enable( !grid->IsEnabled() );
			}
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::OnDestroy()
{
	if (m_renderer)
		m_renderer->DeleteContext( m_hWnd );
	CViewport::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::Render()
{
	if (GetIEditor()->IsInGameMode())
		return;

	if (!m_renderer)
		return;

	if (!IsWindowVisible())
		return;

	if (!GetIEditor()->GetDocument()->IsDocumentReady())
		return;

	FUNCTION_PROFILER( GetIEditor()->GetSystem(),PROFILE_EDITOR );

	CRect rc;
	GetClientRect( rc );

	CalculateViewTM();

	// Render
	m_renderer->SetCurrentContext( m_hWnd );
	m_renderer->BeginFrame();
	m_renderer->ChangeViewport(0,0,rc.right,rc.bottom);
	m_renderer->SetPolygonMode( R_SOLID_MODE );
	m_renderer->ClearColorBuffer( Rgb2Vec(m_colorBackground) );
	m_renderer->Set2DMode( true,m_rcClient.right,m_rcClient.bottom );

	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, m_rcClient.right, 0,m_rcClient.bottom, -9999.0, 9999.0);
	glMatrixMode(GL_MODELVIEW);
	*/
	
	//////////////////////////////////////////////////////////////////////////
	// Draw viewport elements here.
	//////////////////////////////////////////////////////////////////////////
	// Calc world bounding box for objects rendering.
	m_displayBounds = GetWorldBounds( CPoint(0,0),CPoint(m_rcClient.Width(),m_rcClient.Height()) );

	// Draw all objects.
	DisplayContext dc;
	dc.settings = GetIEditor()->GetDisplaySettings();
	dc.view = this;
	dc.renderer = m_renderer;
	dc.engine = GetIEditor()->Get3DEngine();
	dc.flags = DISPLAY_2D;
	dc.box = m_displayBounds;
	dc.camera = &GetIEditor()->GetSystem()->GetViewCamera();

	if (!dc.settings->IsDisplayLabels())
	{
		dc.flags |= DISPLAY_HIDENAMES;
	}
	if (dc.settings->IsDisplayLinks())
	{
		dc.flags |= DISPLAY_LINKS;
	}
	if (m_bDegradateQuality)
	{
		dc.flags |= DISPLAY_DEGRADATED;
	}

	m_renderer->EF_StartEf();

	//m_renderer->EnableDepthTest(false);
	//m_renderer->EnableDepthWrites(false);
	m_renderer->SetState(GS_NODEPTHTEST);
	Draw( dc );
	//m_renderer->EnableDepthTest(true);
	//m_renderer->EnableDepthWrites(true);
	m_renderer->EF_EndEf3D(SHDF_SORT);
		
	m_renderer->FlushTextMessages();
	m_renderer->Update();

	// Return back from 2D mode.
	m_renderer->Set2DMode( false,m_rcClient.right,m_rcClient.bottom );


	GetIEditor()->GetRenderer()->MakeCurrent();
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::Draw( DisplayContext &dc )
{
	DrawGrid(dc);
	DrawObjects(dc);
	DrawSelection(dc);
	DrawViewerMarker(dc);
	DrawAxis(dc);
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::DrawGrid( DisplayContext &dc,bool bNoXNumbers )
{
	CGrid *pGrid = GetIEditor()->GetViewManager()->GetGrid();
	float gridSize = pGrid->size;

	if (gridSize < 0.00001)
		return;

	//////////////////////////////////////////////////////////////////////////
	bool bShowGrid = pGrid->IsEnabled();

	///if (!bShowGrid)
		//return;

	float fScale = GetZoomFactor();

	int width = m_rcClient.Width();
	int height = m_rcClient.Height();

	float origin[2];
	GetScrollOffset( origin[0],origin[1] );

	//////////////////////////////////////////////////////////////////////////
	// Draw major blocks

	Matrix44 tm;
	tm.SetIdentity();
	dc.PushMatrix( tm );

		//Matrix tm = GetViewTM();
		//glMultMatrixf( (float*)(&tm) );

	int gx,gy;

	//////////////////////////////////////////////////////////////////////////
	// Draw Minor grid lines.
	//////////////////////////////////////////////////////////////////////////

	int firstGridLineX = origin[0] / gridSize - 1;
	int firstGridLineY = origin[1] / gridSize - 1;

	int numGridLinesX = (m_rcClient.Width()/fScale) / gridSize + 1;
	int numGridLinesY = (m_rcClient.Height()/fScale) / gridSize + 1;

	float pixelsPerGrid = gridSize*fScale;

	if (pixelsPerGrid > 4)
	{
		dc.SetColor( MINOR_GRID_COLOR,m_gridAlpha );

		// Draw horizontal grid lines.
		for (gy = firstGridLineY; gy < firstGridLineY+numGridLinesY+1; gy++)
		{
			float y = height - (gy*gridSize - origin[1])*fScale;
			m_renderer->DrawLine( Vec3(0,y,0),Vec3(width,y,0) );
		}
		// Draw vertical grid lines.
		for (gx = firstGridLineX; gx < firstGridLineX+numGridLinesX+1; gx++)
		{
			float x = (gx*gridSize - origin[0])*fScale;
			m_renderer->DrawLine( Vec3(x,0,0),Vec3(x,height,0) );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Draw Major grid lines.
	//////////////////////////////////////////////////////////////////////////
	gridSize = gridSize * pGrid->majorLine;

	int iters = 0;
	pixelsPerGrid = gridSize*fScale;
	while (pixelsPerGrid < 20 && iters < 20)
	{
		gridSize = gridSize*2;
		pixelsPerGrid = gridSize*fScale;
		iters++;
	}
	if (pixelsPerGrid < 20)
		return;

	dc.SetColor( MAJOR_GRID_COLOR,m_gridAlpha );

	// Draw major grid lines.
	firstGridLineX = origin[0] / gridSize;
	firstGridLineY = origin[1] / gridSize;
	firstGridLineX--;
	firstGridLineX--;

	numGridLinesX = 2 + (m_rcClient.Width()/fScale) / gridSize;
	numGridLinesY = 2 + (m_rcClient.Height()/fScale) / gridSize;
	numGridLinesX += 2;
	numGridLinesX += 2;

	// Draw horizontal grid lines.
	for (gy = firstGridLineY; gy < firstGridLineY+numGridLinesY; gy++)
	{
		float y = height - (gy*gridSize - origin[1])*fScale;
		if (gy != 0)
		{
			m_renderer->DrawLine( Vec3(0,y,0),Vec3(width,y,0) );
		}
		else
		{
			dc.SetColor( AXIS_GRID_COLOR );
			m_renderer->DrawLine( Vec3(0,y,0),Vec3(width,y,0) );
			dc.SetColor( MAJOR_GRID_COLOR,m_gridAlpha );
		}
	}
	// Draw vertical grid lines.
	for (gx = firstGridLineX; gx < firstGridLineX+numGridLinesX; gx++)
	{
		float x = (gx*gridSize - origin[0])*fScale;
		if (gx != 0)
			m_renderer->DrawLine( Vec3(x,0,0),Vec3(x,height,0) );
		else
		{
			dc.SetColor( AXIS_GRID_COLOR );
			m_renderer->DrawLine( Vec3(x,0,0),Vec3(x,height,0) );
			dc.SetColor( MAJOR_GRID_COLOR,m_gridAlpha );
		}
	}

	// Draw numbers.
	{
		char text[64];
		dc.SetColor( m_colorGridText );

		if (!bNoXNumbers)
		{
			// Draw horizontal grid text.
			for (gx = firstGridLineX; gx < firstGridLineX+numGridLinesX; gx++)
			{
				sprintf( text, "%i",(int)(gx*gridSize) );
				float x = (gx*gridSize - origin[0])*fScale;
				dc.Draw2dTextLabel( x,10,1,text );
			}
		}
		// Draw vertical grid text.
		for (gy = firstGridLineY; gy < firstGridLineY+numGridLinesY; gy++)
		{
			sprintf( text, "%i",(int)(gy*gridSize) );
			float y = (gy*gridSize - origin[1])*fScale;
			dc.Draw2dTextLabel( 2,height-y,1,text );
		}
	}

	dc.PopMatrix();
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::DrawAxis( DisplayContext &dc )
{
	int ix = 0;
	int iy = 0;
	float cl = 0.85f;
	char xstr[2],ystr[2],zstr[2];
	Vec3 colx,coly,colz;
	Vec3 colorX(cl,0,0);
	Vec3 colorY(0,cl,0);
	Vec3 colorZ(0,0,cl);
	switch (m_axis)
	{
	case VPA_XY:
		strcpy( xstr,"x" );
		strcpy( ystr,"y" );
		strcpy( zstr,"z" );
		colx = colorX;
		coly = colorY;
		colz = colorZ;
		break;
	case VPA_XZ:
		strcpy( xstr,"x" );
		strcpy( ystr,"z" );
		strcpy( zstr,"y" );
		colx = colorX;
		coly = colorZ;
		colz = colorY;
		break;
	case VPA_YZ:
		strcpy( xstr,"y" );
		strcpy( ystr,"z" );
		strcpy( zstr,"x" );
		colx = colorY;
		coly = colorZ;
		colz = colorX;
		break;
	}

	int width = m_rcClient.Width();
	int height = m_rcClient.Height();

	int size = 25;
	Vec3 pos( 30,height-15,0 );

	m_renderer->SetMaterialColor( colx.x,colx.y,colx.z,1 );
	m_renderer->DrawLine( pos,pos + Vec3(size,0,0) );
	
	
	m_renderer->SetMaterialColor( coly.x,coly.y,coly.z,1 );
	m_renderer->DrawLine( pos,pos - Vec3(0,size,0) );
	
	
	//m_renderer->SetMaterialColor( 0,0,1,1 );
	//m_renderer->DrawLine( pos,pos + Vec3(0,0,20) );
	//gl.PrintText( pos.x+size,pos.y,"x" );

	dc.SetColor( m_colorAxisText );
	pos.x -= 3;
	pos.y -= 4;
	dc.Draw2dTextLabel( pos.x+size+4,pos.y-2,1,xstr );
	dc.Draw2dTextLabel( pos.x+3,pos.y-size,1,ystr );
	dc.Draw2dTextLabel( pos.x-5,pos.y+5,1,zstr );
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::DrawSelection( DisplayContext &dc )
{
	BBox box;
	GetIEditor()->GetSelectedRegion( box );
	if (!IsEquivalent(box.min,box.max,0))
	{
		//m_renderer->SetMaterialColor( SELECTION_RECT_COLOR.x,SELECTION_RECT_COLOR.y,SELECTION_RECT_COLOR.z,1 ); // white
		//m_renderer->DrawWireB
		/*
		CRect rc;
		CPoint p1 = WorldToView( box.min );
		CPoint p2 = WorldToView( box.max );
		rc.SetRect( p1,p2 );
		rc.NormalizeRect();
		// Draw selection.
		if (rc.left != rc.right || rc.top != rc.bottom)
		{
			// Draw select rectangle.
			m_renderer->SetMaterialColor( SELECTION_RECT_COLOR.x,SELECTION_RECT_COLOR.y,SELECTION_RECT_COLOR.z,1 ); // white
			m_renderer->DrawLine( Vec3(rc.left,rc.top,0),Vec3(rc.right,rc.top,0) );
			m_renderer->DrawLine( Vec3(rc.right,rc.top,0),Vec3(rc.right,rc.bottom,0) );
			m_renderer->DrawLine( Vec3(rc.right,rc.bottom,0),Vec3(rc.left,rc.bottom,0) );
			m_renderer->DrawLine( Vec3(rc.left,rc.bottom,0),Vec3(rc.left,rc.top,0) );
		}
		*/
	}

	if (!IsEquivalent(box.min,box.max,0))
	{
		switch (m_axis)
		{
		case VPA_XY:
			box.min.z = box.max.z = 0;
			break;
		case VPA_XZ:
			box.min.y = box.max.y = 0;
			break;
		case VPA_YZ:
			box.min.x = box.max.x = 0;
			break;
		}

		dc.PushMatrix( GetScreenTM() );
		dc.SetColor( SELECTION_RECT_COLOR.x,SELECTION_RECT_COLOR.y,SELECTION_RECT_COLOR.z,1 );
		dc.DrawWireBox( box.min,box.max );
		dc.PopMatrix();
	}

	if (!m_selectedRect.IsRectEmpty())
	{
		dc.SetColor( SELECTION_RECT_COLOR.x,SELECTION_RECT_COLOR.y,SELECTION_RECT_COLOR.z,1 );
		CPoint p1 = CPoint( m_selectedRect.left,m_selectedRect.top );
		CPoint p2 = CPoint( m_selectedRect.right,m_selectedRect.bottom );
		dc.DrawLine( Vec3(p1.x,p1.y,0),Vec3(p2.x,p1.y,0) );
		dc.DrawLine( Vec3(p1.x,p2.y,0),Vec3(p2.x,p2.y,0) );
		dc.DrawLine( Vec3(p1.x,p1.y,0),Vec3(p1.x,p2.y,0) );
		dc.DrawLine( Vec3(p2.x,p1.y,0),Vec3(p2.x,p2.y,0) );
	}
}

//////////////////////////////////////////////////////////////////////////
Vec3 C2DViewport::SnapToGrid( Vec3 vec )
{
	CGrid *pGrid = GetIEditor()->GetViewManager()->GetGrid();
	return pGrid->Snap( vec );
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::DrawViewerMarker( DisplayContext &dc )
{
	float noScale = 1.0f/GetZoomFactor();

	Vec3 viewAngles = GetIEditor()->GetViewerAngles();

	Matrix44 tm;
	switch (m_axis)
	{
	case VPA_XY:
		tm = Matrix44::CreateRotationZYX(-DEG2RAD(Vec3(0,0,viewAngles.z)));
		break;
	case VPA_XZ:
		tm = Matrix44::CreateRotationZYX(-DEG2RAD(Vec3(0,viewAngles.y,0)));
		break;
	case VPA_YZ:
		tm = Matrix44::CreateRotationZYX(-DEG2RAD(Vec3(viewAngles.x,0,0)));
		break;
	}
	tm.SetTranslationOLD( GetIEditor()->GetViewerPos() );

	dc.PushMatrix( tm*GetScreenTM() );

	Vec3 dim(MARKER_SIZE,MARKER_SIZE/2,MARKER_SIZE);
	dc.SetColor( RGB(0,0,255) ); // red
	dc.DrawWireBox( -dim*noScale,dim*noScale );

	float fov = GetIEditor()->GetSystem()->GetViewCamera().GetFov();

	Vec3 q[4];
	float dist = 30;
	float ta = (float)tan(0.5f*fov);
	float w = dist * ta;
	float h = w * gSettings.viewports.fDefaultAspectRatio; //  ASPECT ??
	//float h = w / GetAspect();
	q[0] = Vec3( w,-dist, h) * noScale;
	q[1] = Vec3(-w,-dist, h) * noScale;
	q[2] = Vec3(-w,-dist,-h) * noScale;
	q[3] = Vec3( w,-dist,-h) * noScale;

	// Draw frustum.
	dc.DrawLine( Vec3(0,0,0),q[0] );
	dc.DrawLine( Vec3(0,0,0),q[1] );
	dc.DrawLine( Vec3(0,0,0),q[2] );
	dc.DrawLine( Vec3(0,0,0),q[3] );

	// Draw quad.
	dc.DrawPolyLine( q,4 );

	dc.PopMatrix();
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::DrawObjects( DisplayContext &dc )
{
	dc.renderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);
	//dc.renderer->SetBlendMode();
	//dc.renderer->EnableBlend(true);
 	dc.PushMatrix( GetScreenTM() );
	GetIEditor()->GetObjectManager()->Display( dc );

	// Display editing tool.
	if (GetIEditor()->GetEditTool())
	{
		GetIEditor()->GetEditTool()->Display( dc );
	}
	dc.PopMatrix();
	dc.renderer->SetState(GS_DEPTHWRITE);
	//dc.renderer->EnableBlend(false);
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::DrawBrush( DisplayContext &dc,struct SBrush *brush,const Matrix44 &brushTM,int flags )
{
	float fScale = GetZoomFactor();

	//Matrix tm = brushTM * GetViewTM();
	//glMultMatrixf( (float*)(&tm) );
	dc.PushMatrix( brushTM );

	// if selected.
	bool bSelected = flags == 1;

	if (bSelected)
	{
		dc.SetLineWidth(2);
		//glEnable(GL_LINE_STIPPLE);
		//glLineStipple( 10,0xAAAA );
	}

	int i, j;
	for (i = 0; i < brush->m_Faces.size(); i++)
	{
		SBrushFace *f = brush->m_Faces[i];
		
		// if (f->m_Plane.normal[0] <= 0)
		//if (f->m_Plane.normal[m_cullAxis] <= 0)
			//continue;

    //CHANGED_BY_IVO
		//Vec3 norm = brushTM.TransformVector( f->m_Plane.normal );
		Vec3 norm = GetTransposed44(brushTM) * ( f->m_Plane.normal );

		if (norm[m_cullAxis] <= 0)
			continue;

		SBrushPoly *poly = f->m_Poly;
		if (!poly)
			continue;

		int numv = poly->m_Pts.size();
		//glBegin(GL_LINE_LOOP);
		for (j=0; j < numv; j++)
		{
			int k = ((j+1) < numv) ? j+1 : 0;
			SBrushVert &vert = poly->m_Pts[j];
			SBrushVert &vert1 = poly->m_Pts[k];
			dc.DrawLine( vert.xyz,vert1.xyz );
			//glVertex3fv( &vert.xyz[0] );
			//glVertex3fv(&poly->m_Pts[j].xyz[0]);
		}
		//glEnd();
	}

	brush->m_bounds;
	
	if (bSelected)
	{
		//glDisable(GL_LINE_STIPPLE);
		dc.SetLineWidth(1);
	}

	dc.PopMatrix();

	if (bSelected)
	{
		int ix = 0;
		int iy = 0;
		switch (m_axis)
		{
		case VPA_XY:
			ix = 0; iy = 1;
			break;
		case VPA_XZ:
			ix = 0; iy = 2;
			break;
		case VPA_YZ:
			ix = 1; iy = 2;
			break;
		}
		Vec3 p1(brush->m_bounds.min[ix],brush->m_bounds.min[iy],0);
		Vec3 p2(brush->m_bounds.max[ix],brush->m_bounds.max[iy],0);
	}
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::DrawTextLabel( DisplayContext &dc,const Vec3& pos,float size,const CFColor& color,const char *text )
{
	float fCol[4] = { color.r,color.g,color.b,color.a };
	Vec3 p = GetScreenTM().TransformPointOLD( pos );
	m_renderer->Draw2dLabel( p.x,p.y,size,fCol,false,"%s",text);
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::SetConstrPlane( CPoint cursor,const Matrix44 &xform )
{
	m_constructionOriginalMatrix = xform;
	m_constructionMatrix = xform;
	// Remove scale component.
	m_constructionMatrix.NoScale();
	Vec3 pos = m_constructionMatrix.GetTranslationOLD();
	// Remove position component.
	m_constructionMatrix.SetTranslationOLD( Vec3(0,0,0) );

	switch (m_axis)
	{
	case VPA_XY:
		m_constructionPlane.Init( pos,pos+Vec3(1,0,0),pos+Vec3(0,1,0) );
		break;
	case VPA_XZ:
		m_constructionPlane.Init( pos,pos+Vec3(1,0,0),pos+Vec3(0,0,1) );
		break;
	case VPA_YZ:
		m_constructionPlane.Init( pos,pos+Vec3(0,1,0),pos+Vec3(0,0,1) );
		break;
	}
}

Vec3 C2DViewport::GetCPVector( const Vec3 &p1,const Vec3 &p2 )
{
	return CViewport::GetCPVector(p1,p2);
}

//////////////////////////////////////////////////////////////////////////
BBox C2DViewport::GetWorldBounds( CPoint pnt1,CPoint pnt2 )
{
	Vec3 org;
	BBox box;
	box.Reset();
	box.Add( ViewToWorld( pnt1 ) );
	box.Add( ViewToWorld( pnt2 ) );

	int maxSize = MAX_WORLD_SIZE;
	switch (m_axis)
	{
	case VPA_XY:
		box.min.z = -maxSize;
		box.max.z = maxSize;
		break;
	case VPA_XZ:
		box.min.y = -maxSize;
		box.max.y = maxSize;
		break;
	case VPA_YZ:
		box.min.x = -maxSize;
		box.max.x = maxSize;
		break;
	}
	return box;
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::OnDragSelectRectangle( CPoint pnt1,CPoint pnt2,bool bNormilizeRect )
{
	Vec3 org;
	BBox box;
	box.Reset();

	Vec3 p1 = ViewToWorld( pnt1 );
	Vec3 p2 = ViewToWorld( pnt2 );
	org = p1;

	// Calculate selection volume.
	box.Add( p1 );
	box.Add( p2 );

	int maxSize = MAX_WORLD_SIZE;

	char szNewStatusText[512];
	float w,h;

	switch (m_axis)
	{
	case VPA_XY:
		box.min.z = -maxSize;
		box.max.z = maxSize;
		
		w = box.max.x - box.min.x;
		h = box.max.y - box.min.y;
		sprintf(szNewStatusText, "X:%g Y:%g W:%g H:%g",org.x,org.y,w,h );
		break;
	case VPA_XZ:
		box.min.y = -maxSize;
		box.max.y = maxSize;

		w = box.max.x - box.min.x;
		h = box.max.z - box.min.z;
		sprintf(szNewStatusText, "X:%g Z:%g  W:%g H:%g",org.x,org.z,w,h );
		break;
	case VPA_YZ:
		box.min.x = -maxSize;
		box.max.x = maxSize;
		
		w = box.max.y - box.min.y;
		h = box.max.z - box.min.z;
		sprintf(szNewStatusText, "Y:%g Z:%g  W:%g H:%g",org.y,org.z,w,h );
		break;
	}

	GetIEditor()->SetSelectedRegion( box );

	// Show marker position in the status bar
	GetIEditor()->SetStatusText(szNewStatusText);
}

//////////////////////////////////////////////////////////////////////////
bool C2DViewport::HitTest( CPoint point,ObjectHitInfo &hitInfo,int flags )
{
	hitInfo.bounds = m_displayBounds;
	return CViewport::HitTest( point,hitInfo,flags );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
bool C2DViewport::IsBoundsVisible( const BBox &box ) const
{
	// If at least part of bbox is visible then its visible.
	if (m_displayBounds.IsIntersectBox( box ))
    return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::CenterOnSelection()
{
	CSelectionGroup *sel = GetIEditor()->GetSelection();
	if (sel->IsEmpty())
		return;

	//SetZoomFactor(1);

	BBox bounds = sel->GetBounds();
	Vec3 selPos = sel->GetCenter();

	float size = GetLength(bounds.max - bounds.min);

	//CPoint p1 = WorldToView(bounds.min);
	//CPoint p2 = WorldToView(bounds.max);
	
	Vec3 v1 = ViewToWorld( CPoint(m_rcClient.left,m_rcClient.bottom) );
	Vec3 v2 = ViewToWorld( CPoint(m_rcClient.right,m_rcClient.top) );
	Vec3 vofs = (v2-v1) * 0.5f;
	selPos -= vofs;
	SetOrigin2D( selPos );

	Invalidate(FALSE);

	//SetZoomFactor( size / 10 );

	/*
	CSelectionGroup *sel = GetIEditor()->GetSelection();
	BBox bounds = sel->GetBounds();
	Vec3 selPos = sel->GetCenter();
	float size = GetLength(bounds.max - bounds.min);
	Vec3 pos = selPos;
	pos += Vec3(0,size*2,size);
	//pos.z = GetIEditor()->GetTerrainElevation(pos.x,pos.y)+5;
	GetIEditor()->SetViewerPos( pos );
	Vec3 dir = GetNormalized(selPos - pos);
	dir = ConvertVectorToCameraAngles(dir);
	GetIEditor()->SetViewerAngles( dir );
	*/
}

//////////////////////////////////////////////////////////////////////////
Vec3 C2DViewport::GetOrigin2D() const
{
	if (gSettings.viewports.bSync2DViews)
		return GetViewManager()->GetOrigin2D();
	else
		return m_origin2D;
}

//////////////////////////////////////////////////////////////////////////
void C2DViewport::SetOrigin2D( const Vec3 &org )
{
	m_origin2D = org;
	if (gSettings.viewports.bSync2DViews)
		GetViewManager()->SetOrigin2D( m_origin2D );
}
