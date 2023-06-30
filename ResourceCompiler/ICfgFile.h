////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   ICfgFile.h
//  Version:     v1.00
//  Created:     3/14/2003 by MM.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __icfgfile_h__
#define __icfgfile_h__
#pragma once

/** Configuration file interface.
		Use format similar to windows .ini files.
*/
struct ICfgFile
{
	virtual ~ICfgFile() {}

	//! Delete instance of configuration file class.
	virtual void Release() = 0;

	//! Load configuration file.
	//! @return true=success, false otherwise
	virtual bool Load( const CString &fileName ) = 0;

	//! Save configuration file, with the stored name in m_fileName
	//! @return true=success, false otherwise
	virtual bool Save( void ) = 0;

	//!
	//! @param inszSection
	//! @param inszKey
	//! @param inszValue
	virtual void UpdateOrCreateEntry( const char *inszSection, const char *inszKey, const char *inszValue ) = 0;

	virtual bool SetConfig( const char *section, IConfig *config ) = 0;

	virtual const char *GetSectionName(unsigned int n) = 0;
	virtual int Find(const char *sectionname) = 0;
};

#endif // __icfgfile_h__
