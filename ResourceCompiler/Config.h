////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   config.h
//  Version:     v1.00
//  Created:     4/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __config_h__
#define __config_h__
#pragma once


#include "iconfig.h"

/** Implementation of IConfig interface.
*/
class Config : public IConfig
{
public:
	Config();
	virtual ~Config();

	virtual void Release() { delete this; };

	//! Clone configuration.
	virtual IConfig* Clone() const;

	//! Check if configuration has this key.
	virtual bool HasKey( const char *key ) const;

	//! Get value of key.
	//! @return true if option found, false if not.
	virtual bool Get( const char *key,float &value ) const;
	virtual bool Get( const char *key,int &value ) const;
	virtual bool Get( const char *key,bool &value ) const;
	virtual bool Get( const char *key,CString &value ) const;

	//////////////////////////////////////////////////////////////////////////
	void Set( const char *key,const char* value );

	void Merge( IConfig *inpConfig );

	virtual Config *GetInternalRepresentation( void );

private:
	typedef std::hash_map<CString,CString,stl::hash_stricmp<CString> > Map;
	Map m_map;
};

#endif // __config_h__