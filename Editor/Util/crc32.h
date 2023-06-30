////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   crc32.h
//  Version:     v1.00
//  Created:     31/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __crc32_h__
#define __crc32_h__

#if _MSC_VER > 1000
#pragma once
#endif


class Crc32Gen {
public:
	Crc32Gen();
	//! Creates a CRC from a text string 
	static unsigned int GetCRC32( const char *text );
	static unsigned int GetCRC32( const char *data,int size,unsigned int ulCRC );

protected:
	unsigned int crc32_table[256];  //!< Lookup table array 
	void init_CRC32_Table();  //!< Builds lookup table array 
	unsigned int reflect( unsigned int ref, char ch); //!< Reflects CRC bits in the lookup table 
	unsigned int get_CRC32( const char *data,int size,unsigned int ulCRC );
};

#endif // __crc32_h__