////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   DisplaySettings.cpp
//  Version:     v1.00
//  Created:     3/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Display Settings implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h" 
#include "DisplaySettings.h"
#include "Objects\ClassDesc.h"
#include "Settings.h"

#include <IConsole.h>

//////////////////////////////////////////////////////////////////////////
CDisplaySettings::CDisplaySettings()
{
	m_flags = 0;
	m_objectHideMask = 0;
	m_displayMode = DISPLAYMODE_SOLID;
	m_displayLabels = true;
	// All flags enabled by default.
	m_renderFlags = (1<<15)-1;
	m_renderFlags &= ~(RENDER_FLAG_BBOX);
	m_debugFlags = 0;
	m_labelsDistance = 100;
}

//////////////////////////////////////////////////////////////////////////
CDisplaySettings::~CDisplaySettings()
{
}

void CDisplaySettings::SaveRegistry()
{
	int displayLabels = m_displayLabels;
	SaveValue("Settings","ObjectHideMask",m_objectHideMask);
	SaveValue("Settings","RenderFlags",m_renderFlags );
	SaveValue("Settings","DisplayMode",(int)m_displayMode);
	SaveValue("Settings","DisplayFlags",m_flags );
	SaveValue("Settings","DebugFlags",m_debugFlags );
	SaveValue("Settings","LabelsDistance",m_labelsDistance );
}

void CDisplaySettings::LoadRegistry()
{
	int dispMode = (int)m_displayMode;
	LoadValue( "Settings","ObjectHideMask",m_objectHideMask );
	LoadValue( "Settings","RenderFlags",m_renderFlags );
	LoadValue( "Settings","DisplayMode",dispMode );
	m_displayMode = (EDisplayMode)dispMode;
	LoadValue( "Settings","DisplayFlags",m_flags );
	LoadValue("Settings","DebugFlags",m_debugFlags );
	int temp = m_labelsDistance;
	LoadValue("Settings","LabelsDistance",temp );
	m_labelsDistance = temp;

	gSettings.objectHideMask = m_objectHideMask;
}

void CDisplaySettings::SetObjectHideMask( int hideMask )
{
	int prevMask = m_objectHideMask;
	m_objectHideMask = hideMask;

	gSettings.objectHideMask = m_objectHideMask;
};

//////////////////////////////////////////////////////////////////////////
void CDisplaySettings::PostInitApply()
{
	SetRenderFlags( m_renderFlags );
	SetDebugFlags( m_debugFlags );
}

//////////////////////////////////////////////////////////////////////////
void CDisplaySettings::SetRenderFlags( int flags )
{
	int prev = m_renderFlags;
	m_renderFlags = flags;

	//////////////////////////////////////////////////////////////////////////
	//SetCVar( "e_bboxes",m_renderFlags&RENDER_FLAG_BBOX );
	SetCVar( "e_beach",m_renderFlags&RENDER_FLAG_BEACHES );
	SetCVar( "e_decals",m_renderFlags&RENDER_FLAG_DECALS );
	SetCVar( "e_detail_texture",m_renderFlags&RENDER_FLAG_DETAILTEX );
	SetCVar( "e_fog",m_renderFlags&RENDER_FLAG_FOG );
	//SetCVar( "e_indoors",m_renderFlags&RENDER_FLAG_INDOORS );
	SetCVar( "e_bflyes",m_renderFlags&RENDER_FLAG_LIVINGOBJ );
	SetCVar( "e_bugs",m_renderFlags&RENDER_FLAG_LIVINGOBJ );
	SetCVar( "e_vegetation",m_renderFlags&RENDER_FLAG_STATICOBJ );
	SetCVar( "e_shadow_maps",m_renderFlags&RENDER_FLAG_SHADOWMAPS );
	SetCVar( "e_sky_box",m_renderFlags&RENDER_FLAG_SKYBOX );
	SetCVar( "e_terrain",m_renderFlags&RENDER_FLAG_TERRAIN );
	SetCVar( "e_water_ocean",m_renderFlags&RENDER_FLAG_WATER );
	SetCVar( "e_detail_objects",m_renderFlags&RENDER_FLAG_DETAILOBJ );
	SetCVar( "e_particles",m_renderFlags&RENDER_FLAG_PARTICLES );
}

//////////////////////////////////////////////////////////////////////////
void CDisplaySettings::SetDebugFlags( int flags )
{
	m_debugFlags = flags;
	
	//
	SetCVar( "e_time_profiling",m_debugFlags&DBG_TIMEPROFILE );
	SetCVar( "ai_debugdraw",m_debugFlags&DBG_AI_DEBUGDRAW );
	SetCVarInt( "r_LogUsedTextures",(m_debugFlags&DBG_TEXTURE_MEMINFO) ? 2:0 );
	SetCVarInt( "memstats",(m_debugFlags&DBG_MEMSTATS) ? 1000:0 );
	SetCVarInt( "p_draw_helpers",(m_debugFlags&DBG_PHYSICS_DEBUGDRAW) ? 5634:0 );

  SetCVarInt( "r_Profile",(m_debugFlags&DBG_RENDERER_PROFILE) ? 4:0 );
	SetCVar( "r_ProfileShaders",(m_debugFlags&DBG_RENDERER_PROFILESHADERS) );

	SetCVarInt( "r_MeasureOverdraw",(m_debugFlags&DBG_RENDERER_OVERDRAW) ? 4:0 );
	SetCVarInt( "r_Stats",(m_debugFlags&DBG_RENDERER_RESOURCES) ? 4:0 );
	SetCVarInt( "e_debug_light",(m_debugFlags&DBG_DEBUG_LIGHTS) ? 1:0 );

	ISystem *pSystem = GetIEditor()->GetSystem();
	if (pSystem)
	{
		bool bOn = (m_debugFlags&DBG_FRAMEPROFILE) != 0;
		pSystem->GetIProfileSystem()->Enable( bOn,bOn );
	}
}

//////////////////////////////////////////////////////////////////////////
void CDisplaySettings::SetCVar( const char *cvar,bool value )
{
	ICVar *var = GetIEditor()->GetSystem()->GetIConsole()->GetCVar(cvar);
	if (var) {
		var->Set( (value)?1:0 );
	}
	else
	{
		CLogFile::FormatLine( "Console Variable %s not declared",cvar );
	}
}

//////////////////////////////////////////////////////////////////////////
void CDisplaySettings::SetCVarInt( const char *cvar,int value )
{
	ICVar *var = GetIEditor()->GetSystem()->GetIConsole()->GetCVar(cvar);
	if (var) {
		var->Set( value );
	}
	else
	{
		CLogFile::FormatLine( "Console Variable %s not declared",cvar );
	}
}

//////////////////////////////////////////////////////////////////////////
void CDisplaySettings::DisplayLabels( bool bEnable )
{
	if (bEnable)
		m_flags &= ~SETTINGS_NOLABELS;
	else
		m_flags |= SETTINGS_NOLABELS;
}

//////////////////////////////////////////////////////////////////////////
void CDisplaySettings::DisplayTracks( bool bEnable )
{
	if (bEnable)
		m_flags &= ~SETTINGS_HIDE_TRACKS;
	else
		m_flags |= SETTINGS_HIDE_TRACKS;
}

//////////////////////////////////////////////////////////////////////////
void CDisplaySettings::DisplayLinks( bool bEnable )
{
	if (bEnable)
		m_flags &= ~SETTINGS_HIDE_LINKS;
	else
		m_flags |= SETTINGS_HIDE_LINKS;
}

//////////////////////////////////////////////////////////////////////////
void CDisplaySettings::DisplayHelpers( bool bEnable )
{
	if (bEnable)
		m_flags &= ~SETTINGS_HIDE_HELPERS;
	else
		m_flags |= SETTINGS_HIDE_HELPERS;
}

//////////////////////////////////////////////////////////////////////////
void CDisplaySettings::SaveValue( const char *sSection,const char *sKey,int value )
{
	AfxGetApp()->WriteProfileInt( sSection,sKey,value );
}

//////////////////////////////////////////////////////////////////////////
void CDisplaySettings::LoadValue( const char *sSection,const char *sKey,int &value )
{
	value = AfxGetApp()->GetProfileInt(sSection,sKey,value );
}

//////////////////////////////////////////////////////////////////////////
void CDisplaySettings::LoadValue( const char *sSection,const char *sKey,bool &value )
{
	value = AfxGetApp()->GetProfileInt(sSection,sKey,value );
}