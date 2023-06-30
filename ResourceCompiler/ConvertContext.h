////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   convertcontext.h
//  Version:     v1.00
//  Created:     4/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __convertcontext_h__
#define __convertcontext_h__
#pragma once

#include "PathUtil.h"

struct IResourceCompiler;
struct IConfig;
struct IRCLog;
struct ICfgFile;

/** Enumeration of supported platforms.
*/
enum Platform
{
	PLATFORM_UNKNOWN,	//!< Unknown platform.
	PLATFORM_PC,
	PLATFORM_XBOX,
	PLATFORM_PS2,
	PLATFORM_GAMECUBE,

	// This entry must be last in Platform enum.
	PLATFORM_LAST,
};

/** ConvertContext is a description of what should be processed by convertor.
*/
struct ConvertContext
{
	// this string must be prepended to all paths (it's with the trailing slash)
	CString masterFolder;

	//! Source file that needs to be converted.
	CString sourceFile;
	//! Output file that would be created from sourceFile. (relative or absolute)
	CString outputFile;

	CString getSourcePath() {return masterFolder + Path::Make(sourceFolder,sourceFile);}
	CString getOutputPath() {return masterFolder + Path::Make(outputFolder,outputFile);}
	CString getOutputFolderPath(){return masterFolder + outputFolder;}

	//! Source file folder, so that the source file can be opened regardless whether the 
	//! path is given in sourceFile or not
	CString sourceFolder;
	//! Output folder
	CString outputFolder;

	//! Pointer to resource compiler interface.
	IResourceCompiler *pRC;
	//! Configuration settings for this file.
	IConfig *config;
	//! Returns the log to put the messages during conversion to:
	//! use Log(), LogWarning() and LogError()
	IRCLog *pLog;

	//! Dont log much info into log.
	bool bQuiet;

	//! file specific config file, must not be 0 (if there was no file, a empty instace with the right filename is created)
	ICfgFile *pFileSpecificConfig;

	//! presets config (for use with the image compiler)
	ICfgFile *presets;

	//! Platform to which file must be processed.
	Platform platform;
};

#endif // __convertcontext_h__