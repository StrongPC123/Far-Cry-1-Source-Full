////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   ivalidator.h
//  Version:     v1.00
//  Created:     3/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: IValidator interface used to check objects for warnings and errors
//               Report missing resources or invalid files.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ivalidator_h__
#define __ivalidator_h__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_WARNING_LENGTH	4096

enum EValidatorSeverity
{
	VALIDATOR_ERROR,
	VALIDATOR_WARNING,
	VALIDATOR_COMMENT
};

enum EValidatorModule
{
	VALIDATOR_MODULE_UNKNOWN,
	VALIDATOR_MODULE_RENDERER,
	VALIDATOR_MODULE_3DENGINE,
	VALIDATOR_MODULE_AI,
	VALIDATOR_MODULE_ANIMATION,
	VALIDATOR_MODULE_ENTITYSYSTEM,
	VALIDATOR_MODULE_SCRIPTSYSTEM,
	VALIDATOR_MODULE_SYSTEM,
	VALIDATOR_MODULE_SOUNDSYSTEM,
	VALIDATOR_MODULE_GAME,
	VALIDATOR_MODULE_MOVIE,
	VALIDATOR_MODULE_EDITOR
};

enum EValidatorFlags
{
	VALIDATOR_FLAG_FILE			= 0x0001,		// Indicate that required file was not found or file was invalid.
	VALIDATOR_FLAG_TEXTURE	= 0x0002,		// Problem with texture.
	VALIDATOR_FLAG_SCRIPT		= 0x0004,		// Problem with script.
	VALIDATOR_FLAG_SOUND		= 0x0008,		// Problem with sound.
	VALIDATOR_FLAG_AI				= 0x0010,		// Problem with AI.
};

struct SValidatorRecord
{
	//! Severety of this error.
	EValidatorSeverity severity;
	//! In which module error occured.
	EValidatorModule module;
	//! Error Text.
	const char *text;
	//! File which is missing or causing problem.
	const char *file;
	//! Additional description for this error.
	const char *description;
	//! Flags that suggest kind of error.
	int flags;

	//////////////////////////////////////////////////////////////////////////
	SValidatorRecord()
	{
		module = VALIDATOR_MODULE_UNKNOWN;
		text = NULL;
		file = NULL;
		description = NULL;
		severity = VALIDATOR_WARNING;
		flags = 0;
	}
};

/*! This interface will be givven to Validate methods of engine, for resources and objects validation.
 */
struct IValidator
{
	virtual void Report( SValidatorRecord &record ) = 0;
};

#endif // __ivalidator_h__
