////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   guidutil.h
//  Version:     v1.00
//  Created:     20/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Utility functions to work with GUIDs.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __guidutil_h__
#define __guidutil_h__
#pragma once

struct GuidUtil
{
	//! Convert GUID to string in the valid format.
	//! The valid format for a GUID is {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} where X is a hex digit. 
	static const char* ToString( REFGUID guid );
	//! Convert from guid string in valid format to GUID class.
	static GUID FromString( const char* guidString );
	static bool IsEmpty( REFGUID guid );
	
	static const GUID NullGuid;
};

/** Used to compare GUID keys.
*/
struct guid_less_predicate
{
	bool operator()( REFGUID guid1,REFGUID guid2 ) const
	{
		return memcmp(&guid1,&guid2,sizeof(GUID)) < 0;
	}
};

//////////////////////////////////////////////////////////////////////////
inline bool GuidUtil::IsEmpty( REFGUID guid )
{
	return guid == NullGuid;
}

//////////////////////////////////////////////////////////////////////////
inline const char* GuidUtil::ToString( REFGUID guid )
{
	static char guidString[64];
	//unsigned short Data4 = *((unsigned short*)guid.Data4);
	//unsigned long Data5 = *((unsigned long*)&guid.Data4[2]);
	//unsigned short Data6 = *((unsigned short*)&guid.Data4[6]);
	sprintf( guidString,"{%.8X-%.4X-%.4X-%.2X%.2X-%.2X%.2X%.2X%.2X%.2X%.2X%}",guid.Data1,guid.Data2,guid.Data3, guid.Data4[0],guid.Data4[1],
				guid.Data4[2],guid.Data4[3],guid.Data4[4],guid.Data4[5],guid.Data4[6],guid.Data4[7] );
	return guidString;
}

//////////////////////////////////////////////////////////////////////////
inline GUID GuidUtil::FromString( const char *guidString )
{
	GUID guid;
	unsigned int d[8];
	memset( &d,0,sizeof(guid) );
	guid.Data1 = 0;
	guid.Data2 = 0;
	guid.Data3 = 0;
	sscanf( guidString,"{%8X-%4hX-%4hX-%2X%2X-%2X%2X%2X%2X%2X%2X%}",
				&guid.Data1,&guid.Data2,&guid.Data3, &d[0],&d[1],&d[2],&d[3],&d[4],&d[5],&d[6],&d[7] );
	guid.Data4[0] = d[0];
	guid.Data4[1] = d[1];
	guid.Data4[2] = d[2];
	guid.Data4[3] = d[3];
	guid.Data4[4] = d[4];
	guid.Data4[5] = d[5];
	guid.Data4[6] = d[6];
	guid.Data4[7] = d[7];

	return guid;
}

#endif // __guidutil_h__
