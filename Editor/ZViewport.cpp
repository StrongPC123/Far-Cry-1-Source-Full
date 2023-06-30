// TopRendererWnd.cpp: implementation of the CZViewport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZViewport.h"
#include "DisplaySettings.h"
#include "ViewManager.h"
#include "Objects\ObjectManager.h"
#include "EditTool.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MAX_WORLD_SIZE 10000

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CZViewport,C2DViewport)

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CZViewport, C2DViewport)
	//{{AFX_MSG_MAP(CZViewport)
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CZViewport::CZViewport()
{
	m_origin(0,0,0);
	m_zoom = 1;
}

//////////////////////////////////////////////////////////////////////////
CZViewport::~CZViewport()
{
}

//////////////////////////////////////////////////////////////////////////
void CZViewport::SetType( EViewportType type )
{
	C2DViewport::SetType( ET_ViewportXZ );
}

//////////////////////////////////////////////////////////////////////////
CSize CZViewport::GetIdealSize() const
{
	return CSize(80,0);
}

//////////////////////////////////////////////////////////////////////////
void CZViewport::Draw( DisplayContext &dc )
{
	DrawGrid( dc,true );
	DrawViewer( dc );
	DrawSelectedObjects( dc );

	// Display editing tool.
	if (GetIEditor()->GetEditTool())
	{
		GetIEditor()->GetEditTool()->Display( dc );
	}
}

//////////////////////////////////////////////////////////////////////////
CPoint CZViewport::WorldToView( Vec3d wp )
{
	CPoint p;
	
	float fScale = m_zoom;

	p.x = 0;
	p.y = m_rcClient.Height() - (wp.z - m_origin.z)*fScale;

	return p;
}

//////////////////////////////////////////////////////////////////////////
Vec3d	CZViewport::ViewToWorld( CPoint vp,bool *collideWithTerrain,bool onlyTerrain )
{
	Vec3d wp(0,0,0);

	float fScale = m_zoom;

	wp.x = 0;
	wp.y = 0;
	wp.z = (m_rcClient.Height() - vp.y)/fScale + m_origin.z;
	return wp;
}

//////////////////////////////////////////////////////////////////////////
void	CZViewport::ViewToWorldRay( CPoint vp,Vec3 &raySrc,Vec3 &rayDir )
{
	raySrc = ViewToWorld( vp );
	raySrc.y = MAX_WORLD_SIZE;
	rayDir(0,-1,0);
}

//////////////////////////////////////////////////////////////////////////
void CZViewport::SetScrollOffset( float x,float y,bool bLimits )
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

	m_origin.z = y;

	CalculateViewTM();
	//GetViewManager()->UpdateViews(eUpdateObjects);
	Invalidate(FALSE);
}

void CZViewport::GetScrollOffset( float &x,float &y )
{
	x = 0;
	y = m_origin.z;
}

//////////////////////////////////////////////////////////////////////////
void CZViewport::CalculateViewTM()
{
	Matrix44 tm;
	tm.SetIdentity();
	m_constructionViewTM.SetIdentity();
	float fScale = m_zoom;
	Vec3 origin = m_origin;
	//float origin[2] = { -m_cScrollOffset.x,-m_cScrollOffset.y };

	float height = m_rcClient.Height()/fScale;

	m_constructionViewTM.BuildFromVectors( Vec3(1,0,0),Vec3(0,0,1),Vec3(0,1,0),Vec3(0,0,0) );
	tm.BuildFromVectors( Vec3(1,0,0)*fScale,Vec3(0,0,1)*fScale,Vec3(0,-1,0)*fScale,Vec3(0,0,0) );
	tm.SetTranslationOLD( Vec3(-origin.x,height+origin.z,0)*fScale );

	SetViewTM( tm );
}

//////////////////////////////////////////////////////////////////////////
Vec3 CZViewport::MapViewToCP( CPoint point )
{
	return SnapToGrid( ViewToWorld( point ) );
}

//////////////////////////////////////////////////////////////////////////
Vec3 CZViewport::GetCPVector( const Vec3 &p1,const Vec3 &p2 )
{
	return Vec3(0,0,p2.z - p1.z);
}

//////////////////////////////////////////////////////////////////////////
void CZViewport::SetZoomFactor( float fZoomFactor )
{
	m_zoom = fZoomFactor;
}

//////////////////////////////////////////////////////////////////////////
float CZViewport::GetZoomFactor() const
{
	return m_zoom;
}

//////////////////////////////////////////////////////////////////////////
void CZViewport::DrawSelectedObjects( DisplayContext &dc )
{
	CSelectionGroup *pSelection = GetIEditor()->GetSelection();
	for (int i = 0; i < pSelection->GetCount(); i++)
	{
		DrawObject( dc,pSelection->GetObject(i) );
	}
}

//////////////////////////////////////////////////////////////////////////
void CZViewport::DrawObject( DisplayContext &dc,CBaseObject *object )
{
	assert( object );

	// Draw selected world bounding box of object.
	BBox box;
	object->GetBoundBox( box );
	
	dc.SetColor( RGB(240,0,0) );
	//dc.DrawWireBox( box.min,box.max );

	CPoint p1 = WorldToView( box.min );
	CPoint p2 = WorldToView( box.max );

	p1.x = 20;
	p2.x = m_rcClient.right-15;

	dc.SetLineWidth(2);
	dc.DrawWireBox( Vec3(p1.x,p1.y,-1),Vec3(p2.x,p2.y,1) );
	dc.SetLineWidth(1);

	//m_renderer->DrawLine( Vec3(0,y,0),Vec3(width,y,0) );
}

//////////////////////////////////////////////////////////////////////////
void CZViewport::DrawViewer( DisplayContext &dc )
{
	CPoint p = WorldToView( GetIEditor()->GetViewerPos() );
	float x = m_rcClient.Width()/2;
	float y = p.y;
	float s = 6;
	dc.SetColor( RGB(0,0,255) );
	//dc.DrawLine( Vec3(x,p.y-s,0),Vec3(x+s,p.y,0) );
	//dc.DrawLine( Vec3(x+s,p.y,0),Vec3(x,p.y+s,0) );
	//dc.DrawLine( Vec3(x,p.y+s,0),Vec3(x-s,p.y,0) );
	//dc.DrawLine( Vec3(x-s,p.y,0),Vec3(x,p.y-s,0) );
	//dc.DrawWireBox( Vec3(x-s,p.y-s,-1),Vec3(x+s,p.y+s,1) );

	// Quad.
	dc.DrawLine( Vec3(x-s,y-s,0),Vec3(x+s,y-s,0) );
	dc.DrawLine( Vec3(x-s,y+s,0),Vec3(x+s,y+s,0) );
	dc.DrawLine( Vec3(x-s,y-s,0),Vec3(x-s,y+s,0) );
	dc.DrawLine( Vec3(x+s,y-s,0),Vec3(x+s,y+s,0) );

	dc.DrawLine( Vec3(x-s,y-s,0),Vec3(x+s,y+s,0) );
	dc.DrawLine( Vec3(x+s,y-s,0),Vec3(x-s,y+s,0) );
}

//////////////////////////////////////////////////////////////////////////
bool CZViewport::HitTest( CPoint point,ObjectHitInfo &hitInfo,int flags )
{
	BBox box;
	CRect rect;
	CSelectionGroup *pSelection = GetIEditor()->GetSelection();
	if (!pSelection->IsEmpty())
	{
		hitInfo.view = this;
		hitInfo.point2d = point;
		hitInfo.distance = 0;
		//hitInfo.object = pSelection->GetObject(0);

		CSelectionGroup *pSelection = GetIEditor()->GetSelection();
		for (int i = 0; i < pSelection->GetCount(); i++)
		{
			pSelection->GetObject(i)->GetBoundBox( box );
			CPoint p1 = WorldToView( box.min );
			CPoint p2 = WorldToView( box.max );
			p1.x = 20;
			p2.x = m_rcClient.right-15;
			rect.SetRect( p1,p2 );
			rect.NormalizeRect();
			rect.OffsetRect( CPoint(1,5) );
			if (rect.PtInRect(point))
			{
				hitInfo.object = pSelection->GetObject(i);
				return true;		
			}
		}
	}
	return false;
}