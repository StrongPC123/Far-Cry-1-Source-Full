////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   EditorUtils.h
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Utility classes used by Editor.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __EditorUtils_h__
#define __EditorUtils_h__

#if _MSC_VER > 1000
#pragma once
#endif

//! Typedef for vector.
typedef Vec3d Vec3;

//! Typedef for quaternion.
//typedef CryQuat Quat;


#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define LINE_EPS (0.00001f)

template <class T> inline void ZeroStruct( T &t ) { memset( &t,0,sizeof(t) ); }

//! Checks heap for errors.	
struct HeapCheck
{
	//! Runs consistency checks on the heap.
	static void Check( const char *file,int line );
};

#ifdef _DEBUG
#define HEAP_CHECK HeapCheck::Check( __FILE__,__LINE__ );
#else
#define HEAP_CHECK
#endif


/*!
 * StdMap Wraps std::map to provide easier to use interface.	
 */
template <class Key,class Value>
struct StdMap
{
private:
	typedef std::map<Key,Value> Map;
	Map m;

public:
	typedef typename Map::iterator Iterator;
	typedef typename Map::const_iterator ConstIterator;

	void	Insert( const Key& key,const Value &value ) { m[key] = value; }
	int		GetCount() const { return m.size(); };
	bool	IsEmpty() const { return m.empty(); };
	void	Clear() { m.clear(); }
	int 	Erase( const Key &key ) { return m.erase(key); };
	Value& operator[]( const Key &key ) { return m[key]; };
	bool Find( const Key& key,Value &value ) const
	{
		Map::const_iterator it = m.find(key);
		if (it == m.end())
			return false;
		value = it->second;
		return true;
	}
	Iterator Find( const Key& key ) { return m.find(key); }
	ConstIterator Find( const Key& key ) const { return m.find(key); }

	bool FindKeyByValue( const Value &value,Key& key ) const
	{
		for (Map::const_iterator it = m.begin(); it != m.end(); ++it)
		{
			if (it->second == value)
			{
				key = it->first;
				return true;
			}	
		}
		return false;
	}

	Iterator Begin() { return m.begin(); };
	Iterator End() { return m.end(); };
	ConstIterator Begin() const { return m.begin(); };
	ConstIterator End() const { return m.end(); };

	void	GetAsVector( std::vector<Value> &array ) const
	{
		array.resize( m.size() );
		int i = 0;
		for (Map::const_iterator it = m.begin(); it != m.end(); ++it)
			array[i++] = it->second;
	}
};

//////////////////////////////////////////////////////////////////////////
/** This class keeps file version information.
*/
struct Version
{
	int v[4];

	Version() {
		v[0] = v[1] = v[2] = v[3] = 0;
	}
	Version( const int vers[] ) {
		v[0] = vers[0];
		v[1] = vers[1];
		v[2] = vers[2];
		v[3] = 1;
	}

	explicit Version( const char* s )
	{
		v[0] = v[1] = v[2] = v[3] = 0;
		
		char t[50];	char* p;
		strcpy(t,s);
		
		if(!(p = strtok(t,"."))) return;
		v[3] = atoi(p);
		if(!(p = strtok(NULL,"."))) return;
		v[2] = atoi(p);		
		if(!(p = strtok(NULL,"."))) return;
		v[1] = atoi(p);		
		if(!(p = strtok(NULL,"."))) return;
		v[0] = atoi(p);
	}

	bool operator <( const Version &v2 ) const {
		if (v[3] < v2.v[3]) return true;
		if (v[3] > v2.v[3]) return false;
		
		if (v[2] < v2.v[2]) return true;
		if (v[2] > v2.v[2]) return false;
		
		if (v[1] < v2.v[1]) return true;
		if (v[1] > v2.v[1]) return false;
		
		if (v[0] < v2.v[0]) return true;
		if (v[0] > v2.v[0]) return false;
		return false;
	}
	bool operator ==( const Version &v1 ) const {
		if (v[0] == v1.v[0] && v[1] == v1.v[1] &&
				v[2] == v1.v[2] && v[3] == v1.v[3]) return true;
		return false;
	}
	bool operator >( const Version &v1) const {
		return !(*this < v1);
	}
	bool operator >=( const Version &v1) const {
		return (*this == v1) || (*this > v1);
	}
	bool operator <=( const Version &v1) const {
		return (*this == v1) || (*this < v1);
	}
	
	int& operator[](int i)       { return v[i];}
	int  operator[](int i) const { return v[i];} 

	CString ToString() const {
		char s[1024];
		sprintf( s,"%d.%d.%d",v[2],v[1],v[0] );
		return s;
	}

	CString ToFullString() const {
		char s[1024];
		sprintf( s,"%d.%d.%d.%d",v[3],v[2],v[1],v[0] );
		return s;
	}
};

//////////////////////////////////////////////////////////////////////////
//
// Convert String representation of color to RGB integer value.
//
//////////////////////////////////////////////////////////////////////////
inline COLORREF String2Color( const CString &val )
{
	unsigned int r=0,g=0,b=0;
	int res = 0;
	res = sscanf( val,"R:%d,G:%d,B:%d",&r,&g,&b );
	if (res != 3)
		res = sscanf( val,"R:%d G:%d B:%d",&r,&g,&b );
	if (res != 3)
		res = sscanf( val,"%d,%d,%d",&r,&g,&b );
	if (res != 3)
		res = sscanf( val,"%d %d %d",&r,&g,&b );
	if (res != 3)
	{
		sscanf( val,"%x",&r );
		return r;
	}

	return RGB(r,g,b);
}

// Converts COLORREF to Vector.
inline Vec3 Rgb2Vec( COLORREF color )
{
	return Vec3( GetRValue(color)/255.0f,GetGValue(color)/255.0f,GetBValue(color)/255.0f );
}

// Converts COLORREF to Vector.
inline COLORREF Vec2Rgb( const Vec3 &color )
{
	return RGB( color.x*255,color.y*255,color.z*255 );
}

//////////////////////////////////////////////////////////////////////////
// Tokenize string.
//////////////////////////////////////////////////////////////////////////
inline CString TokenizeString( const CString &str,LPCSTR pszTokens, int& iStart )
{
	assert( iStart >= 0 );

	if( pszTokens == NULL )
	{
		return str;
	}

	LPCSTR pszPlace = (LPCSTR)str + iStart;
	LPCSTR pszEnd = (LPCSTR)str + str.GetLength();
	if( pszPlace < pszEnd )
	{
		int nIncluding = (int)strspn( pszPlace,pszTokens );;

		if( (pszPlace+nIncluding) < pszEnd )
		{
			pszPlace += nIncluding;
			int nExcluding = (int)strcspn( pszPlace, pszTokens );

			int iFrom = iStart+nIncluding;
			int nUntil = nExcluding;
			iStart = iFrom+nUntil+1;

			return (str.Mid( iFrom, nUntil ) );
		}
	}

	// return empty string, done tokenizing
	iStart = -1;
	return "";
}

/*! Collection of Utility MFC functions.
*/
struct CMFCUtils
{
	//////////////////////////////////////////////////////////////////////////
	// Load true color image into image list.
	static BOOL LoadTrueColorImageList( CImageList &imageList,UINT nIDResource,int nIconWidth,COLORREF colMaskColor );
};

#endif // __EditorUtils_h__

