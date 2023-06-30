#ifndef _COMPRESSIONHELPER_H_
#define _COMPRESSIONHELPER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ICompressionHelper.h"				// ICompressionHelper
#include "StaticCharCompressor.h"			// CStaticCharCompressor

//#define GATHER_CHARSTATISTICS					// only needed during development - to build up the compression table


class CCompressionHelper :public ICompressionHelper
{
public:
	//! constructor
	CCompressionHelper();
	//! destructor
	virtual ~CCompressionHelper();

	// interface ICompressionHelper ---------------------------------------------

	virtual bool Write( CStream &outStream, const unsigned char inChar );
	virtual bool Read( CStream &inStream, unsigned char &outChar );
	virtual bool Write( CStream &outStream, const char *inszString );
	virtual bool Read( CStream &inStream, char *outszString, const DWORD indwStringSize );

private: // -------------------------------------------------------------------

	CStaticCharCompressor				m_CharCompressor;							//!< based on static Huffman compresson
#ifdef GATHER_CHARSTATISTICS
	DWORD												m_dwCharStats[256];						//!<
	DWORD												m_dwWriteBitsCompressed;			//!<
	DWORD												m_dwWriteBitsUncompressed;		//!<
#endif
};

#endif //_COMPRESSIONHELPER_H_
