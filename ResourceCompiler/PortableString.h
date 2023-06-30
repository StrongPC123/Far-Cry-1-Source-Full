// defines the portable string that will replace ATL string on platforms without ATL
#ifndef _RC_PORTABLE_STRING_HDR_
#define _RC_PORTABLE_STRING_HDR_

#include <stdio.h>


class CPortableString: public string
{
	//char& operator [] (int i) {return (*this)[i];}
	//char operator [] (int i) const {return (*this)[i];}
public:
	//operator const char* () const {return this->c_str();}

	CPortableString(){}
	CPortableString(const char* szThat): string(szThat){}
	CPortableString(const char* szThat, size_t nCount): string (szThat, nCount) {}
	CPortableString(const string& str):string(str) {}
  
	size_t GetLength() const {return length();}

	CPortableString Mid( int iFirst ) const {return iFirst<length() ? c_str() + iFirst : CPortableString();}
	CPortableString Mid (int iFirst, int nCount) const {return iFirst<length() ? CPortableString(c_str()+iFirst, nCount):CPortableString();}
	CPortableString Right (int nCount)const
	{
		size_t nLength = GetLength();
		if( nCount >= nLength )
		{
			return( *this );
		}

		return( CPortableString( GetString()+nLength-nCount, nCount ) );
	}

	char* GetBuffer() {return &(*this)[0];}
	const char* GetString()const {return c_str();}

	int Replace( char chOld, char chNew )
	{
		int nCount = 0;
		if (chOld != chNew)
		for (iterator it = begin(); it != end(); ++it)
			if (*it == chOld)
			{
				*it = chNew;
				++nCount;
			}
		return nCount;
	}

	// Format data using format string 'pszFormat'
	void __cdecl Format( const char* pszFormat, ... )
	{
		va_list argList;
		va_start( argList, pszFormat );
		char szBuf[0x400];
		_vsnprintf (szBuf, sizeof(szBuf), pszFormat, argList);
		va_end( argList );
	}

	static int GetFormattedLength( LPCSTR pszFormat, va_list args ) throw()
	{
		return _vscprintf( pszFormat, args );
	}

	// Find the first occurrence of string 'pszSub', starting at index 'iStart'
	int Find( const char* pszSub, int iStart = 0 ) const 
	{
		// nLength is in XCHARs
		int nLength = GetLength();
		if( iStart > nLength )
		{
			return( -1 );
		}

		// find first matching substring
		const char* psz = strstr( GetString()+iStart, pszSub );

		// return -1 for not found, distance from beginning otherwise
		return( (psz == NULL) ? -1 : int( psz-GetString() ) );
	}
	// Find the first occurrence of character 'ch', starting at index 'iStart'
	int Find( char ch, int iStart = 0 ) const throw()
	{
		// nLength is in XCHARs
		int nLength = GetLength();
		if( iStart >= nLength)
		{
			return( -1 );
		}

		// find first single character
		const char* psz = strchr( GetString()+iStart, ch );

		// return -1 if not found and index otherwise
		return( (psz == NULL) ? -1 : int( psz-GetString() ) );
	}

	CPortableString Left (int nCount) const
	{
		return CPortableString(GetString(), nCount);
	}

	bool IsEmpty() const {return empty();}


	// Remove all leading and trailing whitespace
	CPortableString& Trim()
	{
		return( TrimRight().TrimLeft() );
	}

	inline static bool IsWhitespace (char c)
	{
		return (c >= '\x09' && c <= '\x0D') || c == ' ';
	}

	CPortableString& TrimLeft()
	{
		for (size_t i = 0; i < length(); ++i)
			if (!IsWhitespace(c_str()[i]))
			{
				erase (begin(), begin()+i);
				break;
			}
		return *this;
	}

	CPortableString& TrimRight()
	{
		for (int i = length()-1; i >= 0; --i)
			if (!IsWhitespace(c_str()[i]))
			{
				resize(i+1);
				break;
			}
		return *this;
	}


	//const CPortableString operator + (const CPortableString& str) const {return <const string&>(::operator +(str);}
};

// compatibility with STL Utils



#endif