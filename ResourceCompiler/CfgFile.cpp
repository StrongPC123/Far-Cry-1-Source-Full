////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   cfgfile.cpp
//  Version:     v1.00
//  Created:     4/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

//
// Configuration file class.
// Use format similar to windows .ini files.
//
#include "StdAfx.h"
#include "cfgfile.h"
#include "Config.h"
#include "DebugLog.h"

//#define  Log DebugLog
 #define  Log while (false)


///////////////////////////////////////////////////////////////////////////////
//
// Class CfgFile implementation.
//
///////////////////////////////////////////////////////////////////////////////

CfgFile::CfgFile() 
{
	// Create IsEmpty section.
	Section section;
	section.name = "";
	m_sections.push_back( section );

	m_modified = false;
}


CfgFile::CfgFile( const CString &fileName )
{
	// Create IsEmpty section.
	Section section;
	section.name = "";
	m_sections.push_back( section );
	m_modified = false;

	Load( fileName );
}

CfgFile::CfgFile( const char *buf,int bufSize ) {
	// Create IsEmpty section.
	Section section;
	section.name = "";
	m_sections.push_back( section );
	m_modified = false;

	LoadBuf( buf,bufSize );
}

CfgFile::~CfgFile()
{
}

//!
void CfgFile::SetFileName( const CString &fileName )
{
	m_fileName = fileName;
}

// Load configuration file.
bool CfgFile::Load( const CString &fileName )
{
	m_fileName = fileName;
	m_modified = false;

	FILE *file = fopen( fileName.GetString(),"rb" );
	if (!file)
	{
		char szCWD[0x400];
		getcwd(szCWD, sizeof(szCWD));
		Log ("Can't open \"%s\"", fileName.GetString());
		Log ("CWD=%s", szCWD);
		return false;
	}

	fseek( file,0,SEEK_END );
	int size = ftell(file);
	fseek( file,0,SEEK_SET );
		
	// Read whole file to memory.
	char *s = (char*)malloc( size+1 );
	memset( s,0,size+1 );
	fread( s,1,size,file );
	LoadBuf( s,size );
	free(s);

	fclose(file);

	return true;
}




// Save configuration file, with the stored name in m_fileName
bool CfgFile::Save( void )
{
	FILE *file = fopen(m_fileName.GetBuffer(),"wb");

	if(!file)
		return(false);

	// Loop on sections.
	for (std::vector<Section>::iterator si = m_sections.begin(); si != m_sections.end(); si++)
	{
		Section &sec =*si;

		if(sec.name!="")
			fprintf(file,"[%s]\r\n",sec.name);																						// section

		for (std::list<Entry>::iterator it = sec.entries.begin(); it != sec.entries.end(); ++it)
		{
			if((*it).key=="")
				fprintf(file,"%s\r\n",(*it).value.GetBuffer());															// comment
			 else
				fprintf(file,"%s=%s\r\n",(*it).key.GetBuffer(),(*it).value.GetBuffer());		// key=value
		}
	}

	fclose(file);
	return(true);
}


void CfgFile::UpdateOrCreateEntry( const char *inszSection, const char *inszKey, const char *inszValue )
{
	Section *sec = FindSection( inszSection );				assert(sec);

	for(std::list<Entry>::iterator it = sec->entries.begin(); it != sec->entries.end(); ++it)
	{
		if(stricmp(it->key.GetString(),inszKey) == 0)				// Key found
			if(!it->IsComment())									// update key
			{
				if(it->value==inszValue)
					return;

				it->value=inszValue;
				m_modified=true;
				return;
			}
	}

	// Create new key
	Entry entry;
	entry.key = inszKey;
	entry.value = inszValue;

	sec->entries.push_back(entry);
	m_modified=true;
}



void CfgFile::LoadBuf( const char *buf,size_t bufSize )
{

	// Read entries from config CString buffer.
	Section *curr_section = &m_sections.front();	// Empty section.

	const char *s = buf;
	size_t size = bufSize;

	CString ss = s;

	char str[4096];
	size_t i = 0;

	while (i < size)
	{
//		while (j < size && s[j] != '\n') j++;

		sscanf( &s[i],"%[^\n]s",str );

		Log ("Parsing line: \"%s\"", str);

		i+=strlen(str);
		if(s[i]==10)i++;
		
		bool bComment=false;

		// Is comment?
		Entry entry;
		entry.key = "";
		entry.value = str;

		bComment=entry.IsComment();

		if (bComment)
			Log ("It's a comment");

		// Analyze entry CString, split on key and value and store in lists.
		CString entrystr = str;

		if(!bComment)
			entrystr.Trim();

		if(bComment || entrystr.IsEmpty())
		{
			Log ("Empty!");
			// Add this comment to current section.
			curr_section->entries.push_back( entry );
			continue;
		}


		{
			int splitter = entrystr.Find( '=' );
			Log ("found splitter at %d", splitter);

			if (splitter > 0) 
			{
				// Key found.
				entry.key = entrystr.Mid( 0,splitter );	// Before spliter is key name.
				Log ("Key: %s", entry.key.GetString());
				entry.value = entrystr.Mid( splitter+1 );	// Everything after splittes is value CString.
				Log ("Value: %s", entry.value.GetString());
				entry.key.Trim();
				entry.value.Trim();
				Log ("Key: %s, Value: %s", entry.key.GetString(), entry.value.GetString());

				// Add this entry to current section.
				curr_section->entries.push_back( entry );
			}
			else 
			{
				// If not key then probably section CString.
				if (entrystr[0] == '[' && entrystr[entrystr.GetLength()-1] == ']')
				{
					// World in bracets is section name.
					Section section;
					section.name = entrystr.Mid( 1,entrystr.GetLength()-2 ); // Remove bracets.
					Log ("Section! name: %s", section.name.GetString());
					m_sections.push_back( section );
					// Set current section.
					curr_section = &m_sections.back();
				} 
				else
				{
					// Just IsEmpty key value.
					entry.key = "";
					entry.value = entrystr;

					curr_section->entries.push_back( entry );
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
CfgFile::Section* CfgFile::FindSection( const CString &section )
{
	return &m_sections[Find(section.GetString())];
}

//////////////////////////////////////////////////////////////////////////
bool CfgFile::SetConfig( const char *section, IConfig *config )
{
	Section *sec = FindSection( section );					assert(sec);

	for (std::list<Entry>::iterator it = sec->entries.begin(); it != sec->entries.end(); ++it)
	{
		if(!(*it).IsComment())
			config->Set( (*it).key.GetString(),(*it).value.GetString() );
	}
	return true;
}
