// PanelDisplayRender.cpp : implementation file
//

#include "stdafx.h"
#include "PanelDisplayRender.h"
#include "DisplaySettings.h"
#include "Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define LEAVE_FLAGS (RENDER_FLAG_BBOX)

/////////////////////////////////////////////////////////////////////////////
// CPanelDisplayRender dialog


CPanelDisplayRender::CPanelDisplayRender(CWnd* pParent /*=NULL*/)
	: CDialog(CPanelDisplayRender::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPanelDisplayRender)
	m_beaches = FALSE;
	m_decals = FALSE;
	m_detailTex = FALSE;
	m_fog = FALSE;
	m_shadowMaps = FALSE;
	m_skyBox = FALSE;
	m_staticObj = FALSE;
	m_terrain = FALSE;
	m_water = FALSE;
	m_detailObj = FALSE;
	m_particles = FALSE;
	
	m_dbg_time_profile = FALSE;
	m_dbg_frame_profile = FALSE;
	m_dbg_aidebugdraw = FALSE;
	m_dbg_physicsDebugDraw = FALSE;
	m_dbg_memory_info = FALSE;
	m_dbg_mem_stats = FALSE;
	m_dbg_texture_meminfo = FALSE;
	m_dbg_renderer_profile = FALSE;
	m_dbg_renderer_profile_shaders = FALSE;
	m_dbg_renderer_overdraw = FALSE;
	m_dbg_renderer_resources = FALSE;
	m_dbg_debug_lights = FALSE;
	//}}AFX_DATA_INIT

	Create( IDD,pParent );
}


void CPanelDisplayRender::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DISPLAY_ALL, m_selectAllBtn);
	DDX_Control(pDX, IDC_DISPLAY_NONE, m_selectNoneBtn);
	DDX_Control(pDX, IDC_DISPLAY_INVERT, m_selectInvertBtn);

	// Render flags.
	DDX_Check(pDX, IDC_DISPLAY_BEACHES, m_beaches);
	DDX_Check(pDX, IDC_DISPLAY_DECALS, m_decals);
	DDX_Check(pDX, IDC_DISPLAY_DETAILTEX, m_detailTex);
	DDX_Check(pDX, IDC_DISPLAY_FOG, m_fog);
	DDX_Check(pDX, IDC_DISPLAY_SHADOWMAPS, m_shadowMaps);
	DDX_Check(pDX, IDC_DISPLAY_SKYBOX, m_skyBox);
	DDX_Check(pDX, IDC_DISPLAY_STATICOBJ, m_staticObj);
	DDX_Check(pDX, IDC_DISPLAY_TERRAIN, m_terrain);
	DDX_Check(pDX, IDC_DISPLAY_WATER, m_water);
	DDX_Check(pDX, IDC_DISPLAY_DETAILOBJ, m_detailObj);
	DDX_Check(pDX, IDC_DISPLAY_PARTICLES, m_particles);
	
	// Debug flags.
	DDX_Check(pDX, IDC_DISPLAY_PROFILE, m_dbg_time_profile);
	DDX_Check(pDX, IDC_DISPLAY_PROFILE2, m_dbg_frame_profile);
	DDX_Check(pDX, IDC_DISPLAY_AIDEBUGDRAW, m_dbg_aidebugdraw);
	DDX_Check(pDX, IDC_MEM_INFO, m_dbg_memory_info);
	DDX_Check(pDX, IDC_MEM_STATS, m_dbg_mem_stats);
	DDX_Check(pDX, IDC_TEXMEM_STATS, m_dbg_texture_meminfo);
	DDX_Check(pDX, IDC_DISPLAY_PHYSICSDEBUGDRAW, m_dbg_physicsDebugDraw);
	DDX_Check(pDX, IDC_DISPLAY_PROFILERENDERER, m_dbg_renderer_profile);
	DDX_Check(pDX, IDC_DISPLAY_PROFILESHADERS, m_dbg_renderer_profile_shaders);
	DDX_Check(pDX, IDC_DISPLAY_OVERDRAW, m_dbg_renderer_overdraw);
	DDX_Check(pDX, IDC_DISPLAY_RENDERERRESOURCES, m_dbg_renderer_resources);
	DDX_Check(pDX, IDC_DISPLAY_DEBUG_LIGHTS, m_dbg_debug_lights);
}


BEGIN_MESSAGE_MAP(CPanelDisplayRender, CDialog)
	ON_BN_CLICKED(IDC_DISPLAY_FOG, OnChangeRenderFlag)
	ON_BN_CLICKED(IDC_DISPLAY_ALL, OnDisplayAll)
	ON_BN_CLICKED(IDC_DISPLAY_NONE, OnDisplayNone)
	ON_BN_CLICKED(IDC_DISPLAY_INVERT, OnDisplayInvert)
	ON_BN_CLICKED(IDC_FILL_MODE, OnFillMode)
	ON_BN_CLICKED(IDC_WIREFRAME_MODE, OnWireframeMode)
	ON_BN_CLICKED(IDC_DISPLAY_SKYBOX, OnChangeRenderFlag)
	ON_BN_CLICKED(IDC_DISPLAY_TERRAIN, OnChangeRenderFlag)
	ON_BN_CLICKED(IDC_DISPLAY_WATER, OnChangeRenderFlag)
	ON_BN_CLICKED(IDC_DISPLAY_BEACHES, OnChangeRenderFlag)
	ON_BN_CLICKED(IDC_DISPLAY_SHADOWMAPS, OnChangeRenderFlag)
	ON_BN_CLICKED(IDC_DISPLAY_DETAILTEX, OnChangeRenderFlag)
	ON_BN_CLICKED(IDC_DISPLAY_STATICOBJ, OnChangeRenderFlag)
	ON_BN_CLICKED(IDC_DISPLAY_DECALS, OnChangeRenderFlag)
	ON_BN_CLICKED(IDC_DISPLAY_DETAILOBJ, OnChangeRenderFlag)
	ON_BN_CLICKED(IDC_DISPLAY_PARTICLES, OnChangeRenderFlag)
	ON_BN_CLICKED(IDC_HIDE_HELPERS, OnBnClickedHideHelpers)

	// Debug flags.
	ON_BN_CLICKED(IDC_MEM_INFO, OnChangeDebugFlag)
	ON_BN_CLICKED(IDC_DISPLAY_PROFILE, OnChangeDebugFlag)
	ON_BN_CLICKED(IDC_DISPLAY_PROFILE2, OnChangeDebugFlag)
	ON_BN_CLICKED(IDC_DISPLAY_AIDEBUGDRAW, OnChangeDebugFlag)
	ON_BN_CLICKED(IDC_MEM_STATS, OnChangeDebugFlag)
	ON_BN_CLICKED(IDC_TEXMEM_STATS, OnChangeDebugFlag)
	ON_BN_CLICKED(IDC_DISPLAY_PHYSICSDEBUGDRAW, OnChangeDebugFlag)
	ON_BN_CLICKED(IDC_DISPLAY_PROFILERENDERER, OnChangeDebugFlag)
	ON_BN_CLICKED(IDC_DISPLAY_PROFILESHADERS, OnChangeDebugFlag)
	ON_BN_CLICKED(IDC_DISPLAY_OVERDRAW, OnChangeDebugFlag)
	ON_BN_CLICKED(IDC_DISPLAY_RENDERERRESOURCES, OnChangeDebugFlag)
	ON_BN_CLICKED(IDC_DISPLAY_DEBUG_LIGHTS, OnChangeDebugFlag)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPanelDisplayRender message handlers

BOOL CPanelDisplayRender::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetControls();
	
	if (!GetIEditor()->GetDisplaySettings()->IsDisplayHelpers())
		CheckDlgButton( IDC_HIDE_HELPERS,BST_CHECKED );
	else
		CheckDlgButton( IDC_HIDE_HELPERS,BST_UNCHECKED );

	if (GetIEditor()->GetDisplaySettings()->GetDisplayMode() == DISPLAYMODE_SOLID)
	{
		CheckDlgButton( IDC_FILL_MODE,BST_CHECKED );
		CheckDlgButton( IDC_WIREFRAME_MODE,BST_UNCHECKED );
	}
	else
	{
		CheckDlgButton( IDC_FILL_MODE,BST_UNCHECKED );
		CheckDlgButton( IDC_WIREFRAME_MODE,BST_CHECKED );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPanelDisplayRender::SetControls()
{
	uint flags = GetIEditor()->GetDisplaySettings()->GetRenderFlags();
	m_beaches = (flags&RENDER_FLAG_BEACHES)?TRUE:FALSE;
	m_decals = (flags&RENDER_FLAG_DECALS)?TRUE:FALSE;
	m_detailTex = (flags&RENDER_FLAG_DETAILTEX)?TRUE:FALSE;
	m_fog = (flags&RENDER_FLAG_FOG)?TRUE:FALSE;
	m_shadowMaps = (flags&RENDER_FLAG_SHADOWMAPS)?TRUE:FALSE;
	m_skyBox = (flags&RENDER_FLAG_SKYBOX)?TRUE:FALSE;
	m_staticObj = (flags&RENDER_FLAG_STATICOBJ)?TRUE:FALSE;
	m_terrain = (flags&RENDER_FLAG_TERRAIN)?TRUE:FALSE;
	m_water = (flags&RENDER_FLAG_WATER)?TRUE:FALSE;
	m_detailObj = (flags&RENDER_FLAG_DETAILOBJ)?TRUE:FALSE;
	m_particles = (flags&RENDER_FLAG_PARTICLES)?TRUE:FALSE;
	
	flags = GetIEditor()->GetDisplaySettings()->GetDebugFlags();
	m_dbg_time_profile = (flags&DBG_TIMEPROFILE)?TRUE:FALSE;
	m_dbg_frame_profile = (flags&DBG_FRAMEPROFILE)?TRUE:FALSE;
	m_dbg_aidebugdraw = (flags&DBG_AI_DEBUGDRAW)?TRUE:FALSE;
	m_dbg_physicsDebugDraw = (flags&DBG_PHYSICS_DEBUGDRAW)?TRUE:FALSE;
	m_dbg_memory_info = (flags&DBG_MEMINFO)?TRUE:FALSE;
	m_dbg_mem_stats = (flags&DBG_MEMSTATS)?TRUE:FALSE;
	m_dbg_texture_meminfo = (flags&DBG_TEXTURE_MEMINFO)?TRUE:FALSE;
	m_dbg_renderer_profile = (flags&DBG_RENDERER_PROFILE)?TRUE:FALSE;
	m_dbg_renderer_profile_shaders = (flags&DBG_RENDERER_PROFILESHADERS)?TRUE:FALSE;
	m_dbg_renderer_overdraw = (flags&DBG_RENDERER_OVERDRAW)?TRUE:FALSE;
	m_dbg_renderer_resources = (flags&DBG_RENDERER_RESOURCES)?TRUE:FALSE;
	m_dbg_debug_lights = (flags&DBG_DEBUG_LIGHTS)?TRUE:FALSE;

	UpdateData(FALSE);
}

void CPanelDisplayRender::OnChangeRenderFlag() 
{
	UpdateData(TRUE);
	uint flags = GetIEditor()->GetDisplaySettings()->GetRenderFlags();
	flags = 0;

	flags |= (m_beaches)?RENDER_FLAG_BEACHES:0;
	flags |= (m_decals)?RENDER_FLAG_DECALS:0;
	flags |= (m_detailTex)?RENDER_FLAG_DETAILTEX:0;
	flags |= (m_fog)?RENDER_FLAG_FOG:0;
	flags |= (m_shadowMaps)?RENDER_FLAG_SHADOWMAPS:0;
	flags |= (m_skyBox)?RENDER_FLAG_SKYBOX:0;
	flags |= (m_staticObj)?RENDER_FLAG_STATICOBJ:0;
	flags |= (m_terrain)?RENDER_FLAG_TERRAIN:0;
	flags |= (m_water)?RENDER_FLAG_WATER:0;
	flags |= (m_detailObj)?RENDER_FLAG_DETAILOBJ:0;
	flags |= (m_particles)?RENDER_FLAG_PARTICLES:0;
	
	GetIEditor()->GetDisplaySettings()->SetRenderFlags( flags );
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayRender::OnChangeDebugFlag() 
{
	UpdateData(TRUE);
	uint flags = GetIEditor()->GetDisplaySettings()->GetDebugFlags();
	flags = 0;

	flags |= (m_dbg_time_profile)?DBG_TIMEPROFILE:0;
	flags |= (m_dbg_frame_profile)?DBG_FRAMEPROFILE:0;
	flags |= (m_dbg_aidebugdraw)?DBG_AI_DEBUGDRAW:0;
	flags |= (m_dbg_physicsDebugDraw)?DBG_PHYSICS_DEBUGDRAW:0;
	flags |= (m_dbg_memory_info)?DBG_MEMINFO:0;
	flags |= (m_dbg_mem_stats)?DBG_MEMSTATS:0;
	flags |= (m_dbg_texture_meminfo)?DBG_TEXTURE_MEMINFO:0;
	flags |= (m_dbg_renderer_profile)?DBG_RENDERER_PROFILE:0;
	flags |= (m_dbg_renderer_profile_shaders)?DBG_RENDERER_PROFILESHADERS:0;
	flags |= (m_dbg_renderer_overdraw)?DBG_RENDERER_OVERDRAW:0;
	flags |= (m_dbg_renderer_resources)?DBG_RENDERER_RESOURCES:0;
	flags |= (m_dbg_debug_lights)?DBG_DEBUG_LIGHTS:0;

	GetIEditor()->GetDisplaySettings()->SetDebugFlags( flags );
}


//////////////////////////////////////////////////////////////////////////
void CPanelDisplayRender::OnDisplayAll() 
{
	uint flags = GetIEditor()->GetDisplaySettings()->GetRenderFlags();
	flags |= 0xFFFFFFFF & ~(LEAVE_FLAGS);
	GetIEditor()->GetDisplaySettings()->SetRenderFlags( flags );
	SetControls();
}

void CPanelDisplayRender::OnDisplayNone() 
{
	uint flags = GetIEditor()->GetDisplaySettings()->GetRenderFlags();
	flags &= (LEAVE_FLAGS);
	GetIEditor()->GetDisplaySettings()->SetRenderFlags( flags );
	SetControls();
}

void CPanelDisplayRender::OnDisplayInvert() 
{
	uint flags0 = GetIEditor()->GetDisplaySettings()->GetRenderFlags();
	uint flags = ~flags0;
	flags &= ~(LEAVE_FLAGS);
	flags |= flags0 & (LEAVE_FLAGS);
	GetIEditor()->GetDisplaySettings()->SetRenderFlags( flags );
	SetControls();
}

void CPanelDisplayRender::OnFillMode() 
{
	GetIEditor()->GetDisplaySettings()->SetDisplayMode( DISPLAYMODE_SOLID );
}

void CPanelDisplayRender::OnWireframeMode() 
{
	GetIEditor()->GetDisplaySettings()->SetDisplayMode( DISPLAYMODE_WIREFRAME );
}

//////////////////////////////////////////////////////////////////////////
void CPanelDisplayRender::OnBnClickedHideHelpers()
{
	if (IsDlgButtonChecked( IDC_HIDE_HELPERS ))
		GetIEditor()->GetDisplaySettings()->DisplayHelpers( false );
	else
		GetIEditor()->GetDisplaySettings()->DisplayHelpers( true );
}
