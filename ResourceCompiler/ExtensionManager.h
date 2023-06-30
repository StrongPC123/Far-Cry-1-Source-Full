////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   extensionmanager.h
//  Version:     v1.00
//  Created:     5/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __extensionmanager_h__
#define __extensionmanager_h__
#pragma once

// forward declarations.
struct IConvertor;

/** Manages mapping between file extensions and convertors.
*/
class ExtensionManager
{
public:
	ExtensionManager();
	~ExtensionManager();
	//! Register new convertor with extension manager.
	//!  \param conv must not be 0
	//!  \param rc must not be 0
	void RegisterConvertor( IConvertor *conv, IResourceCompiler *rc );
	//! Unregister all convertors.
	void UnregisterAll();

	//! Find convertor that matches given platform and extension.
	IConvertor* FindConvertor( Platform platform,const char *ext ) const;

private:
	// Maps extension to convertors.
	typedef std::multimap<CString,IConvertor*,stl::less_stricmp<CString> > ExtMap;
	ExtMap m_extMap[PLATFORM_LAST];
	std::vector<IConvertor*> m_convertors;
};

#endif // __extensionmanager_h__
