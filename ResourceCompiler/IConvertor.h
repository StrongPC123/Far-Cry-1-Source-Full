////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   convertor.h
//  Version:     v1.00
//  Created:     4/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __convertor_h__
#define __convertor_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "ConvertContext.h"

/** Conertor interface, all convertor must implement this interface.
*/
struct IConvertor
{
	//! Release memory of interface.
	virtual void Release() = 0;

	//! Process file
	//! \return success
	virtual bool Process( ConvertContext &cc ) = 0;

	//! Construct the name of the file that will be produced from the source file.
	//! Put this name into outputFile field, if successful
	//! Returns true if successful or false if can't convert this file
	// @param cc is the context of conversion; it contains valid sourceFile and may or may not contain outputFile
	virtual bool GetOutputFile(ConvertContext &cc) = 0;

	//! Return platforms supported by this convertor.
	virtual int GetNumPlatforms() const = 0;
	//! Get supported platform.
	//! @param index Index of platform must be in range 0 < index < GetNumPlatforms().
	virtual Platform GetPlatform( int index ) const = 0;

	//! Get number of supported extensions.
	virtual int GetNumExt() const = 0;
	//! Get supported extension.
	//! @param index Index of extension must be in range 0 < index < GetNumExt().
	virtual const char* GetExt( int index ) const = 0;

	// this should retrieve the timestamp of the convertor executable:
	// when it was created by the linker, normally. This date/time is used to
	// compare with the compiled file date/time and even if the compiled file
	// is not older than the source file, it will be recompiled if it's older than the
	// convertor
	virtual DWORD GetTimestamp() const = 0;
};

#endif // __convertor_h__
