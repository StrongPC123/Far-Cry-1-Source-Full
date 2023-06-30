////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   CryAnimationScriptCommands.h
//  Version:     v1.00
//  Created:     1/15/2003 by Sergiy Migdalskiy
//  Compilers:   Visual Studio.NET
//  Description: Declarations necessary for scripting extensions to the
//               Cry Character Animation subsyste.
// -------------------------------------------------------------------------
//  Notes:
//    This file is separated from the ICryAnimation.h for the sake of fast
//    compilation: it's only used in the game code once, and in the animation
//    system once. When some additional extension is added to the animation
//    scripting interface, only this file is changed, and no need to recompile
//    the whole project
//
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __CRY_ANIMATION_SCRIPT_COMMANDS_HDR__
#define __CRY_ANIMATION_SCRIPT_COMMANDS_HDR__

enum CryAnimationScriptCommandEnum
{
	// Start emitting particles from all characters, for test
	CASCMD_TEST_PARTICLES = 0,
	// Stop emitting all particles on all characters
	CASCMD_STOP_PARTICLES = 1,
	// Dump animation memory usage
	CASCMD_DUMP_ANIMATIONS = 2,
	// Dump all loaded models
	CASCMD_DUMP_MODELS = 3,
	// Unload all animations that haven't been touched for the *(int*)pParams number of frames
	// if pParam == NULL, default number of frames should be used
	CASCMD_TRASH_ANIMATIONS = 4,
	// Unload the given animation (by the file name, may be without the path and extension)
	// (char*)pParams is the 0-terminated string with the animation file name
	CASCMD_UNLOAD_ANIMATION = 5,
	// cleans up the decals on all characters
	CASCMD_CLEAR_DECALS = 6,
	// dumps the decals for all characters
	CASCMD_DUMP_DECALS = 7,
	//starts many animations simultaneously; uses (CASCmdStartMultiAnims*)pParams param block
	CASCMD_START_MANY_ANIMS = 8,
	// draws (one-time) all the bones of all the characters in the system
	CASCMD_DEBUG_DRAW = 9,
	// dumps the ModelState info, used to hunt down the invisible guy bug
	CASCMD_DUMP_STATES = 10,
	// dumps all models into the special ASCII file
	CASCMD_EXPORT_MODELS_ASCII = 11
};

struct CASCmdStartAnim
{
	const char* szAnimName;
	int nLayer;
};

struct CASCmdStartMultiAnims
{
	const CASCmdStartAnim* pAnims;
	int numAnims;
	float fBlendTime;
};

#endif