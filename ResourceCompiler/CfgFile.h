////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   cfgfile.h
//  Version:     v1.00
//  Created:     4/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History: 03/14/2003 MM added save support
//
////////////////////////////////////////////////////////////////////////////

#ifndef __cfgfile_h__
#define __cfgfile_h__
#pragma once

#include "ICfgFile.h"						// ICfgFile

class Config;

/** Configuration file class.
		Use format similar to windows .ini files.
*/
class CfgFile :public ICfgFile
{
public:
	// Config file entry structure, filled by readSection method.
	struct Entry 
	{
		CString			key;			//!< keys (for comments this is "")
		CString			value;		//!< values and comments (with leading ; or //)

		bool IsComment( void ) const
		{
			const char *pBegin=value.GetString();
			while(*pBegin==' ' || *pBegin=='\t')pBegin++;

			// "//" (comment)
			if(pBegin[0]=='/' && pBegin[1]=='/')return(true);

			// ';' (comment)
			if(pBegin[0] == ';')return(true);

			// emptyline (comment)
			if(pBegin[0]==0)return(true);

			return(false);
		}
	};

	//! constructor
	CfgFile();

	//! destructor
	~CfgFile();

	//!
	void SetFileName( const CString &fileName );

	//! Automatically load new config file.
	explicit CfgFile( const CString &fileName );

	//! Load config from memory buffer.
	CfgFile( const char *buf, int bufSize );

	//////////////////////////////////////////////////////////////////////////

	// interface ICfgFile - described in ICfgFile.h
	
	virtual void Release() { delete(this); }
	virtual bool Load( const CString &fileName );
	virtual bool Save( void );
	virtual void UpdateOrCreateEntry( const char *inszSection, const char *inszKey, const char *inszValue );

	//////////////////////////////////////////////////////////////////////////
	//! Copy section keys to config.
	//! @return true if success, false if section not found.
	bool SetConfig( const char *section, IConfig *config );

private:

	void	LoadBuf( const char *buf,size_t bufSize );

	struct Section
	{
		CString						name;						//!< Section name. (the first one has the name "" and is used if no section was specified)
		std::list<Entry>	entries;				//!< List of entries.
	};

	// Private methods.

	//! @return pointer to the section with the given name or pointer to the first unnamed section ""
	Section* FindSection( const CString &section );

	// Private fields.
	CString							m_fileName;			//!< Configuration file name.
	int									m_modified;			//!< Set to true if config file been modified.

	std::vector<Section>	m_sections;			//!< List of sections in config file. (the first one has the name "" and is used if no section was specified)

public:

	const char *GetSectionName(unsigned int n)
	{
		if(n>=m_sections.size())
			return(NULL);
		
		return(m_sections[n].name.GetString());
	};
	
	int Find(const char *sectionname)
	{
		for(std::vector<Section>::iterator it = m_sections.begin(); it!=m_sections.end(); ++it)
		{
			if (stricmp(it->name.GetString(), sectionname) == 0) return it-m_sections.begin();
		};
		return 0;
	};
};


#endif // __cfgfile_h__
