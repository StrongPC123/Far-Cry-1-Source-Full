////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   ObjectEvent.h
//  Version:     v1.00
//  Created:     13/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ObjectEvent_h__
#define __ObjectEvent_h__
#pragma once

//! Standart objects types.
enum ObjectType
{
	OBJTYPE_GROUP			= 1<<0,
	OBJTYPE_TAGPOINT	= 1<<1,
	OBJTYPE_AIPOINT		= 1<<2,
	OBJTYPE_ENTITY		= 1<<3,
	OBJTYPE_SHAPE			= 1<<4,
	OBJTYPE_VOLUME		= 1<<5,
	OBJTYPE_BRUSH			= 1<<6,
	OBJTYPE_PREFAB		= 1<<7,
	OBJTYPE_ANY = 0xFFFFFFFF,
};

//////////////////////////////////////////////////////////////////////////
//! Events that objects may want to handle.
//! Passed to OnEvent method of CBaseObject.
enum ObjectEvent
{
	EVENT_INGAME	= 1,	//!< Signals that editor is switching into the game mode.
	EVENT_OUTOFGAME,		//!< Signals that editor is switching out of the game mode.
	EVENT_REFRESH,			//!< Signals that editor is refreshing level.
	EVENT_AFTER_LOAD,		//!< Signals that editor is finished loading all objects.
	EVENT_PLACE_STATIC,	//!< Signals that editor needs to place all static objects on terrain.
	EVENT_REMOVE_STATIC,//!< Signals that editor needs to remove all static objects from terrain.
	EVENT_DBLCLICK,			//!< Signals that object have been double clicked.
	EVENT_KEEP_HEIGHT,	//!< Signals that object must preserve its height over changed terrain.
	EVENT_UNLOAD_ENTITY,//!< Signals that entities scripts must be unloaded.
	EVENT_RELOAD_ENTITY,//!< Signals that entities scripts must be reloaded.
	EVENT_RELOAD_TEXTURES,//!< Signals that all posible textures in objects should be reloaded.
	EVENT_RELOAD_GEOM,	//!< Signals that all posible geometries should be reloaded.
	EVENT_UNLOAD_GEOM,	//!< Signals that all posible geometries should be unloaded.
	EVENT_MISSION_CHANGE,	//!< Signals that mission have been changed.
	EVENT_CLEAR_AIGRAPH,//!< Signals that ai graph is about to be deleted.
	EVENT_PREFAB_REMAKE,//!< Recreate all objects in prefabs.

	EVENT_PHYSICS_GETSTATE,//!< Signals that entities should accept thier phyical state from game.
	EVENT_PHYSICS_RESETSTATE,//!< Signals that physics state must be reseted on objects.
};

#endif // __ObjectEvent_h__

