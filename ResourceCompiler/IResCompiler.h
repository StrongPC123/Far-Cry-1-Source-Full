////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   irescompiler.h
//  Version:     v1.00
//  Created:     4/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: IResourceCompiler interface.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __irescompiler_h__
#define __irescompiler_h__
#pragma once

#include <stdio.h>

struct IConvertor;
struct CryChunkedFile;
struct IRCLog;

/** Main interface of resource compiler.
*/
struct IResourceCompiler
{
	//! Register new convertor.
	virtual void RegisterConvertor( IConvertor *conv ) = 0;

	//! Use this instead of fopen.
	virtual FILE*	OpenFile( const char *filename,const char *mode ) = 0;

	//! Get timestamp of file.
	//virtual bool GetFileTime( const char *filename,FILETIME *ftimeModify, FILETIME*ftimeCreate) = 0;

	//! Do resource compilation for this file.
	//! \param outroot path to the root folder e.g.c:\MasterCD
	//! @param filename Full filename including path to the file that needs compiling.
	virtual bool CompileFile( const char *filename,  const char *outroot, const char *outpath ) = 0;

	//! Load and parse the Crytek Chunked File into the universal (very big) structure
	//! The caller should then call Release on the structure to free the mem
	//! @param filename Full filename including path to the file
	virtual CryChunkedFile* LoadCryChunkedFile (const char* szFileName) = 0;

	//! Returns the main application window
	virtual HWND GetHWnd() = 0;

	// returns a handle to an empty window to be used with DirectX
	virtual HWND GetEmptyWindow() = 0;

	//! If the physics was successfully initialized, then returns the pointer to the physics engine;
	//! otherwise returns NULL
	virtual class IPhysicalWorld* GetPhysicalWorld() = 0;
	
	//! \return is always a valid pointer
	virtual IRCLog *GetIRCLog()=0;

	//! \inszName full name like in 3dsmax
	virtual void AddDependencyMaterial( const char *inszSrcFilename, const char *inszMatName, const char *inszScriptName )=0;

	//! \inszPathFileName absolute path names
	virtual void AddDependencyFile( const char *inszSrcFilename, const char *inszPathFileName )=0;
};


// this is the plugin function that's exported by plugins
// Registers all convertors residing in this DLL
extern "C" {
typedef void  (__stdcall* FnRegisterConvertors )(IResourceCompiler*pRC);
}

#endif // __irescompiler_h__
