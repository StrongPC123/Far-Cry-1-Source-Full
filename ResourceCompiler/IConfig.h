////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   iconfig.h
//  Version:     v1.00
//  Created:     4/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __iconfig_h__
#define __iconfig_h__
#pragma once

class Config;

/** Configuration options interface.
*/
struct IConfig
{
	//! Delete instance of configuration class.
	virtual void Release() = 0;

	//! Clone configuration.
	virtual IConfig* Clone() const = 0;

	//! Check if configuration has this key.
	virtual bool HasKey( const char *key ) const = 0;

	//! Get value of key. the value isn't modified if the key isn't found
	//! @return true if option found, false if not.
	virtual bool Get( const char *key,float &value ) const = 0;
	virtual bool Get( const char *key,int &value ) const = 0;
	virtual bool Get( const char *key,bool &value ) const = 0;
	virtual bool Get( const char *key,CString &value ) const = 0;

	virtual void Set( const char *key,const char* value ) = 0;

	virtual Config *GetInternalRepresentation( void )=0;

	template <typename T>
	T GetAs (const char* key, T defaultValue)
	{
		Get (key, defaultValue);
		return defaultValue;
	}
};

#endif // __iconfig_h__
