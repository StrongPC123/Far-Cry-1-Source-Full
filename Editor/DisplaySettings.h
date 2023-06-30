////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   DisplaySettings.h
//  Version:     v1.00
//  Created:     3/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Display Settings definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DisplaySettings_h__
#define __DisplaySettings_h__

#if _MSC_VER > 1000
#pragma once
#endif

enum EDisplayMode {
	DISPLAYMODE_SOLID = 0,
	DISPLAYMODE_WIREFRAME,
};

//////////////////////////////////////////////////////////////////////////
enum EDisplayRenderFlags
{
	RENDER_FLAG_BBOX			= 1<<0,
	RENDER_FLAG_BEACHES		= 1<<1,
	RENDER_FLAG_DECALS		= 1<<2,
	RENDER_FLAG_DETAILTEX	= 1<<3,
	RENDER_FLAG_FOG				= 1<<4,
	RENDER_FLAG_INDOORS		= 1<<5,
	RENDER_FLAG_LIVINGOBJ	= 1<<6,
	RENDER_FLAG_STATICOBJ	= 1<<7,
	RENDER_FLAG_SHADOWMAPS= 1<<8,
	RENDER_FLAG_SKYBOX		= 1<<9,
	RENDER_FLAG_TERRAIN		= 1<<10,
	RENDER_FLAG_WATER			= 1<<11,
	RENDER_FLAG_DETAILOBJ	= 1<<12,

	RENDER_FLAG_PARTICLES = 1<<14,

	// Debugging options.
	//RENDER_FLAG_PROFILE = 1<<20,
	//RENDER_FLAG_AIDEBUGDRAW = 1<<21,
	//RENDER_FLAG_MEMINFO = 1<<22,
};

//////////////////////////////////////////////////////////////////////////
enum EDisplaySettingsFlags
{
	SETTINGS_NOCOLLISION = 0x01,//!< Disable collision with terrain.
	SETTINGS_NOLABELS = 0x02,		//!< Do not draw labels.
	SETTINGS_PHYSICS = 0x04,		//!< Physics simulation is enabled.
	SETTINGS_HIDE_TRACKS = 0x08, //!< Enable displaying of animation tracks in views.
	SETTINGS_HIDE_LINKS = 0x10, //!< Enable displaying of links between objects.
	SETTINGS_HIDE_HELPERS = 0x20, //!< Disable displaying of all object helpers.
};

//////////////////////////////////////////////////////////////////////////
enum EDebugSettingsFlags
{
	DBG_TIMEPROFILE				= 0x001,
	DBG_MEMINFO						= 0x002,
	DBG_MEMSTATS					= 0x004,
	DBG_TEXTURE_MEMINFO		= 0x008,
	DBG_AI_DEBUGDRAW			= 0x010,
	DBG_PHYSICS_DEBUGDRAW	= 0x020,
	DBG_RENDERER_PROFILE	= 0x040,
	DBG_RENDERER_PROFILESHADERS	= 0x080,
	DBG_RENDERER_OVERDRAW	= 0x100,
	DBG_RENDERER_RESOURCES	= 0x200,
	DBG_FRAMEPROFILE				= 0x400,
	DBG_DEBUG_LIGHTS				= 0x800,
};

/*!
 *	CDisplaySettings is a collection of information about how to display current views.
 */
class CDisplaySettings
{
public:
	CDisplaySettings();
	~CDisplaySettings();

	void PostInitApply();

	void SetSettings( int flags ) { m_flags = flags; };
	int GetSettings() const { return m_flags; };
	
	void SetObjectHideMask( int m_objectHideMask );
	int GetObjectHideMask() const { return m_objectHideMask; }
	
	void SetRenderFlags( int flags  );
	int GetRenderFlags() const { return m_renderFlags; }

	void SetDebugFlags( int flags  );
	int GetDebugFlags() const { return m_debugFlags; }

	void SetDisplayMode( EDisplayMode mode ) { m_displayMode = mode; };
	EDisplayMode GetDisplayMode() const { return m_displayMode; }

	void DisplayLabels( bool bEnable );
	bool IsDisplayLabels() const { return (m_flags&SETTINGS_NOLABELS) == 0; };

	void DisplayTracks( bool bEnable );
	bool IsDisplayTracks() const { return (m_flags&SETTINGS_HIDE_TRACKS) == 0; };

	void DisplayLinks( bool bEnable );
	bool IsDisplayLinks() const { return (m_flags&SETTINGS_HIDE_LINKS) == 0; };

	void DisplayHelpers( bool bEnable );
	bool IsDisplayHelpers() const { return (m_flags&SETTINGS_HIDE_HELPERS) == 0; };

	void SetLabelsDistance( float dist ) { m_labelsDistance = dist; };
	float GetLabelsDistance() const { return m_labelsDistance; };

	void SaveRegistry();
	void LoadRegistry();

private:
	// Restrict access.
	CDisplaySettings( const CDisplaySettings& ) {}
	void operator=( const CDisplaySettings& ) {}

	void SetCVar( const char *cvar,bool val );
	void SetCVarInt( const char *cvar,int val );
	void SaveValue( const char *sSection,const char *sKey,int value );
	void LoadValue( const char *sSection,const char *sKey,int &value );
	void LoadValue( const char *sSection,const char *sKey,bool &value );

	int m_objectHideMask;
	int m_renderFlags;
	int m_flags;
	EDisplayMode m_displayMode;
	bool m_displayLabels;

	//! Debug/profile settings.
	//! @see EDebugSettingsFlags.
	int m_debugFlags;
	float m_labelsDistance;
};

#endif // __DisplaySettings_h__
