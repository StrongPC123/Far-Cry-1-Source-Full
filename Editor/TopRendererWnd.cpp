// TopRendererWnd.cpp: implementation of the CTopRendererWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TopRendererWnd.h"
#include "Heightmap.h"
#include "VegetationMap.h"
#include "DisplaySettings.h"
#include "EditTool.h"
#include "ViewManager.h"
#include "Objects\ObjectManager.h"

#include "TerrainTexGen.h"
//#include "Image.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Size of the surface texture
#define SURFACE_TEXTURE_WIDTH 512

#define MARKER_SIZE 6.0f
#define MARKER_DIR_SIZE 10.0f
#define SELECTION_RADIUS 30.0f

#define GL_RGBA 0x1908
#define GL_BGRA 0x80E1


//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CTopRendererWnd,C2DViewport)

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CTopRendererWnd, C2DViewport)
	//{{AFX_MSG_MAP(CTopRendererWnd)
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// Used to give each static object type a different color
static uint sVegetationColors[16] =
{
	0xFFFF0000,
	0xFF00FF00,
	0xFF0000FF,
	0xFFFFFFFF,
	0xFFFF00FF,
	0xFFFFFF00,
	0xFF00FFFF,
	0xFF7F00FF,
	0xFF7FFF7F,
	0xFFFF7F00,
	0xFF00FF7F,
	0xFF7F7F7F,
	0xFFFF0000,
	0xFF00FF00,
	0xFF0000FF,
	0xFFFFFFFF,
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTopRendererWnd::CTopRendererWnd()
{
	////////////////////////////////////////////////////////////////////////
	// Set the window type member of the base class to the correct type and
	// create the initial surface texture
	////////////////////////////////////////////////////////////////////////

	CLogFile::WriteLine("Top render window created");

	m_bShowHeightmap = false;
	m_bShowStatObjects = false;

	////////////////////////////////////////////////////////////////////////
	// Surface texture
	////////////////////////////////////////////////////////////////////////

	CLogFile::WriteLine("Creating initial surface texture for top render window...");

	m_textureSize.cx = SURFACE_TEXTURE_WIDTH;
	m_textureSize.cy = SURFACE_TEXTURE_WIDTH;

	m_heightmapSize = CSize(1,1);

	m_terrainTextureId = 0;

	m_vegetationTextureId = 0;
	
	// Create a new surface texture image
	ResetSurfaceTexture();

	m_bContentsUpdated = false;

	m_gridAlpha = 0.3f;
	m_colorGridText = RGB(255,255,255);
	m_colorAxisText = RGB(255,255,255);
	m_colorBackground = RGB(128,128,128);
}

//////////////////////////////////////////////////////////////////////////
CTopRendererWnd::~CTopRendererWnd()
{
	////////////////////////////////////////////////////////////////////////
	// Destroy the attached render and free the surface texture
	////////////////////////////////////////////////////////////////////////
	CLogFile::WriteLine("Top render window destroied"); 
}

//////////////////////////////////////////////////////////////////////////
void CTopRendererWnd::SetType( EViewportType type )
{
	C2DViewport::SetType( ET_ViewportXY );
}

//////////////////////////////////////////////////////////////////////////
void CTopRendererWnd::ResetContent()
{
	C2DViewport::ResetContent();
	ResetSurfaceTexture();

	// Reset texture ids.
	m_terrainTextureId = 0;
	m_vegetationTextureId = 0;
}

//////////////////////////////////////////////////////////////////////////
void CTopRendererWnd::UpdateContent( int flags )
{
	C2DViewport::UpdateContent(flags);
	if (!GetIEditor()->GetDocument())
		return;

	CHeightmap *heightmap = GetIEditor()->GetHeightmap();
	if (!heightmap)
		return;

	if (!m_hWnd)
		return;

	if (!IsWindowVisible())
		return;

	m_heightmapSize.cx = heightmap->GetWidth() * heightmap->GetUnitSize();
	m_heightmapSize.cy = heightmap->GetHeight() * heightmap->GetUnitSize();

	UpdateSurfaceTexture( flags );
	m_bContentsUpdated = true;
}

//////////////////////////////////////////////////////////////////////////
void CTopRendererWnd::UpdateSurfaceTexture( int flags )
{
	////////////////////////////////////////////////////////////////////////
	// Generate a new surface texture
	////////////////////////////////////////////////////////////////////////
	if (flags & eUpdateHeightmap)
	{
		CLogFile::FormatLine("Updating top view surface data (%i x %i)...",	m_textureSize.cx, m_textureSize.cy);
		
		if (!m_bShowHeightmap)
		{
			m_textureSize.cx = SURFACE_TEXTURE_WIDTH;
			m_textureSize.cy = SURFACE_TEXTURE_WIDTH;
			
			m_terrainTexture.Allocate( m_textureSize.cx,m_textureSize.cy );
			
			// Fill in the surface data into the array. Apply lighting and water, use
			// the settings from the document
			//bReturn = cSurfaceTexture.GenerateSurface( (unsigned long*)m_terrainTexture.GetData(),m_textureSize.cx,m_textureSize.cy,GEN_USE_LIGHTING|GEN_SHOW_WATER|GEN_ABGR );
			//texGen.GenerateSurfaceTexture( ETTG_LIGHTING|ETTG_SHOW_WATER|ETTG_ABGR,m_terrainTexture );
			CTerrainTexGen texGen;
			texGen.GenerateSurfaceTexture( ETTG_LIGHTING|ETTG_SHOW_WATER|ETTG_FAST_LLIGHTING|ETTG_ABGR,m_terrainTexture );
		}
		else
		{
			m_textureSize.cx = 512;
			m_textureSize.cy = 512;
			m_terrainTexture.Allocate( m_textureSize.cx,m_textureSize.cy );
			
			// Show heightmap data.
			//GLOBAL_GET_DOC->m_cHeightmap.GetDataEx(pHeightmap, (m_rcView.right - m_rcView.left), false, false);
			GetIEditor()->GetHeightmap()->GetPreviewBitmap( (DWORD*)m_terrainTexture.GetData(),m_textureSize.cx,false,false );
		}
	}

	if (flags == eUpdateStatObj)
	{
		// If The only update flag is Update of static objects, display them.
		m_bShowStatObjects = true;
	}

	if (flags & eUpdateStatObj)
	{
		if (m_bShowStatObjects)
		{
			DrawStaticObjects();
		}
	}
	//if (::IsWindow(m_hWnd))
		//RedrawWindow();
}

//////////////////////////////////////////////////////////////////////////
void CTopRendererWnd::DrawStaticObjects()
{
	if (!m_bShowStatObjects)
		return;

	CVegetationMap *vegetationMap = GetIEditor()->GetVegetationMap();
	int srcW = vegetationMap->GetNumSectors();
	int srcH = vegetationMap->GetNumSectors();

	CRect rc = CRect(0,0,srcW,srcH);
	BBox updateRegion = GetIEditor()->GetViewManager()->GetUpdateRegion();
	if (updateRegion.min.x > -10000)
	{
		// Update region valid.
		CRect urc;
		CPoint p1 = CPoint( vegetationMap->WorldToSector(updateRegion.min.y),vegetationMap->WorldToSector(updateRegion.min.x) );
		CPoint p2 = CPoint( vegetationMap->WorldToSector(updateRegion.max.y),vegetationMap->WorldToSector(updateRegion.max.x) );
		urc = CRect( p1.x-1,p1.y-1,p2.x+1,p2.y+1 );
		rc &= urc;
	}

	int trgW = rc.right - rc.left;
	int trgH = rc.bottom - rc.top;

	if (trgW <= 0 || trgH <= 0)
		return;

	m_vegetationTexturePos = CPoint( rc.left,rc.top );
	m_vegetationTextureSize = CSize( srcW,srcH );
	m_vegetationTexture.Allocate( trgW,trgH );

	uint *trg = m_vegetationTexture.GetData();
	vegetationMap->DrawToTexture( trg,trgW,trgH,rc.left,rc.top );
}

//////////////////////////////////////////////////////////////////////////
void CTopRendererWnd::ResetSurfaceTexture()
{
	////////////////////////////////////////////////////////////////////////
	// Create a surface texture that consists entirely of water
	////////////////////////////////////////////////////////////////////////

	unsigned int i, j;
	DWORD *pSurfaceTextureData = NULL;
	DWORD *pPixData = NULL, *pPixDataEnd = NULL;
	CBitmap bmpLoad;
	BOOL bReturn;

	CLogFile::WriteLine("Resetting surface texture for top render window...");

	// Load the water texture out of the ressource
	bReturn = bmpLoad.Attach(::LoadBitmap(AfxGetApp()->m_hInstance,	MAKEINTRESOURCE(IDB_WATER)));
	ASSERT(bReturn);

	// Allocate new memory to hold the bitmap data
	CImage waterImage;
	waterImage.Allocate( 128,128 );

	// Retrieve the bits from the bitmap
	bmpLoad.GetBitmapBits( 128*128*sizeof(DWORD), waterImage.GetData() );
 
	// Allocate memory for the surface texture
	m_terrainTexture.Allocate( m_textureSize.cx,m_textureSize.cy );

	// Fill the surface texture with the water texture, tile as needed
	for (j=0; j<m_textureSize.cy; j++)
		for (i=0; i<m_textureSize.cx; i++)
		{
			m_terrainTexture.ValueAt(i,j) = waterImage.ValueAt( i&127,j&127 );
		}

	if (::IsWindow(m_hWnd))
	{
		Invalidate(FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTopRendererWnd::Draw( DisplayContext &dc )
{
	FUNCTION_PROFILER( GetIEditor()->GetSystem(),PROFILE_EDITOR );

	////////////////////////////////////////////////////////////////////////
	// Perform the rendering for this window
	////////////////////////////////////////////////////////////////////////
	if (!m_bContentsUpdated)
		UpdateContent( 0xFFFFFFFF );

	////////////////////////////////////////////////////////////////////////
	// Render the 2D map
	////////////////////////////////////////////////////////////////////////
	if (!m_terrainTextureId)
	{
		//GL_BGRA_EXT
		if (m_terrainTexture.IsValid())
		{
			m_terrainTextureId = m_renderer->DownLoadToVideoMemory( (unsigned char*)m_terrainTexture.GetData(),m_textureSize.cx,m_textureSize.cy,eTF_8888,eTF_8888,0,0,0 );
			//m_terrainTexture.Release();
		}
	}

	if (m_terrainTextureId && m_terrainTexture.IsValid())
	{
		m_renderer->UpdateTextureInVideoMemory( m_terrainTextureId,(unsigned char*)m_terrainTexture.GetData(),0,0,m_textureSize.cx,m_textureSize.cy,eTF_8888 );
		//m_renderer->RemoveTexture( m_terrainTextureId );
		//m_terrainTextureId = m_renderer->DownLoadToVideoMemory( (unsigned char*)m_terrainTexture.GetData(),m_textureSize.cx,m_textureSize.cy,eTF_8888,eTF_8888,0,0,0 );
		m_terrainTexture.Release();
	}


	if (m_bShowStatObjects)
	{
		if (m_vegetationTexture.IsValid())
		{
			int w = m_vegetationTexture.GetWidth();
			int h = m_vegetationTexture.GetHeight();
			uint *tex = m_vegetationTexture.GetData();
			if (!m_vegetationTextureId)
			{
				m_vegetationTextureId = m_renderer->DownLoadToVideoMemory( (unsigned char*)tex,w,h,eTF_8888,eTF_8888,0,0,FILTER_NONE );
			}
			else
			{
				int px = m_vegetationTexturePos.x;
				int py = m_vegetationTexturePos.y;
				m_renderer->UpdateTextureInVideoMemory( m_vegetationTextureId,(unsigned char*)tex,px,py,w,h,eTF_8888 );
			}
			m_vegetationTexture.Release();
		}
	}
	

	// Reset states
	m_renderer->ResetToDefault();
	// Disable depth test
	//m_renderer->EnableDepthTest(false);
	//m_renderer->EnableDepthWrites(false);
	dc.SetState(GS_NODEPTHTEST);

	//m_renderer->EnableBlend(false);
	// Draw the map into the view
	
	//m_renderer->Draw2dImage(iX, iY+iHeight, iWidth, -iHeight, m_terrainTextureId);
	Matrix44 tm = GetScreenTM();
	Matrix44 rot;
	rot.SetIdentity();
	rot.BuildFromVectors( Vec3(0,1,0),Vec3(1,0,0),Vec3(0,0,1),Vec3(0,0,0) );

	dc.PushMatrix( rot*tm );
	//m_renderer->DrawImage( 0,0,m_heightmapSize.cx,m_heightmapSize.cy, m_terrainTextureId,0,1,1,0,1,1,1,1 );
	m_renderer->DrawImage( 0,0,m_heightmapSize.cx,m_heightmapSize.cy, m_terrainTextureId, 0,1, 1,0,  1,1,1,1 );
	dc.PopMatrix();

	//m_renderer->EnableDepthTest(true);
	//m_renderer->EnableDepthWrites(true);

	//m_renderer->EnableBlend(true);
	dc.SetState(GS_NODEPTHTEST | GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);

/*
	// Draw the static objects into the view
	if (m_bShowStatObjects && m_vegetationTextureId)
	{
		m_renderer->SetBlendMode();
		m_renderer->EnableBlend(true);

		GetMapRect( m_textureSize,rcMap );

		iWidth = (m_vegetationTextureSize.cx * m_fZoomFactor) * ((float)m_textureSize.cx/m_vegetationTextureSize.cx);
		iHeight = (m_vegetationTextureSize.cy * m_fZoomFactor) * ((float)m_textureSize.cy/m_vegetationTextureSize.cy);

		// Convert the window coordinates into coordinates for the renderer
		iX =  ((float)rcMap.left / rcWnd.right * 800.0f);
		iY =  ((float)rcMap.top / rcWnd.bottom * 600.0f);
		iWidth = ((float)iWidth / rcWnd.right * 800.0f);
		iHeight = ((float)iHeight / rcWnd.bottom * 600.0f);
		m_renderer->Draw2dImage(iX, iY+iHeight, iWidth, -iHeight, m_vegetationTextureId);
		m_renderer->EnableBlend(false);
	}
*/

	C2DViewport::Draw( dc );
}

//////////////////////////////////////////////////////////////////////////
void CTopRendererWnd::OnTitleMenu( CMenu &menu )
{
	bool labels = GetIEditor()->GetDisplaySettings()->IsDisplayLabels();
	menu.AppendMenu( MF_STRING|(labels)?MF_CHECKED:MF_UNCHECKED,1,"Labels" );
	menu.AppendMenu( MF_STRING|(m_bShowHeightmap)?MF_CHECKED:MF_UNCHECKED,2,"Show Heightmap" );
	menu.AppendMenu( MF_STRING|(m_bShowStatObjects)?MF_CHECKED:MF_UNCHECKED,3,"Show Static Objects" );
}

//////////////////////////////////////////////////////////////////////////
void CTopRendererWnd::OnTitleMenuCommand( int id )
{
	switch (id)
	{
		case 1:
			GetIEditor()->GetDisplaySettings()->DisplayLabels( !GetIEditor()->GetDisplaySettings()->IsDisplayLabels() );
			break;
		case 2:
			m_bShowHeightmap = !m_bShowHeightmap;
			UpdateContent( 0xFFFFFFFF );
			break;
		case 3:
			m_bShowStatObjects = !m_bShowStatObjects;
			if (m_bShowStatObjects)
				UpdateContent( eUpdateStatObj );
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
Vec3d	CTopRendererWnd::ViewToWorld( CPoint vp,bool *collideWithTerrain,bool onlyTerrain )
{
	Vec3d wp = C2DViewport::ViewToWorld( vp,collideWithTerrain,onlyTerrain );
	wp.z = GetIEditor()->GetTerrainElevation( wp.x,wp.y );
	return wp;
}