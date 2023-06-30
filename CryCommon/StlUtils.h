//////////////////////////////////////////////////////////////////////
//
//	STL Utils header
//	
//	File: STLutils.h
//	Description : Various convenience utility functions for STL and alike
//  Used in Animation subsystem, and in some tools
//
//	History:
//	-:Created by Sergiy Migdalskiy
//
//////////////////////////////////////////////////////////////////////
#ifndef _STL_UTILS_HEADER_
#define _STL_UTILS_HEADER_

#include <algorithm>

// searches the given entry in the map by key, and if there is none, returns the default value
#ifdef WIN64
#include <map>
#define hash_map map
template <typename Map, typename key_type, typename mapped_type>
inline mapped_type find_in_map(const Map& mapKeyToValue, key_type key, mapped_type valueDefault)
#else
#if defined(LINUX)
#include "platform.h"
#include <ext/hash_map>
#else
#include <hash_map>
#endif
 // TODO: get rid of it and use map
template <typename Map>
inline typename Map::mapped_type find_in_map(const Map& mapKeyToValue, typename Map::key_type key, typename Map::mapped_type valueDefault)
#endif

{
	typename Map::const_iterator it = mapKeyToValue.find (key);
	if (it == mapKeyToValue.end())
		return valueDefault;
	else
		return it->second;
}

// searches the given entry in the map by key, and if there is none, returns the default value
// The values are taken/returned in REFERENCEs rather than values
#ifdef WIN64
template <typename Map, typename key_type, typename mapped_type>
inline mapped_type& find_in_map_ref(Map& mapKeyToValue, key_type key, mapped_type& valueDefault)
#else
template <typename Map>
inline typename Map::mapped_type& find_in_map_ref(Map& mapKeyToValue, typename Map::key_type key, typename Map::mapped_type& valueDefault)
#endif
{
	typename Map::iterator it = mapKeyToValue.find (key);
	if (it == mapKeyToValue.end())
		return valueDefault;
	else
		return it->second;
}

// auto-cleaner: upon destruction, calls the clear() method
template <class T>
class CAutoClear
{
public:
	CAutoClear (T* p): m_p(p) {}
	~CAutoClear () {m_p->clear();}
protected:
	T* m_p;
};


template <class Container>
unsigned sizeofArray (const Container& arr)
{
	return (unsigned)(sizeof(typename Container::value_type)*arr.size());
}

template <class Container>
unsigned sizeofVector (const Container& arr)
{
	return (unsigned)(sizeof(typename Container::value_type)*arr.capacity());
}

template <class Container>
unsigned sizeofArray (const Container& arr, unsigned nSize)
{
	return arr.empty()?0u:(unsigned)(sizeof(typename Container::value_type)*nSize);
}

template <class Container>
unsigned capacityofArray (const Container& arr)
{
	return (unsigned)(arr.capacity()*sizeof(arr[0]));
}

template <class T>
unsigned countElements (const std::vector<T>& arrT, const T& x)
{
	unsigned nSum = 0;
	for (typename std::vector<T>::const_iterator iter = arrT.begin(); iter != arrT.end(); ++iter)
		if (x == *iter)
			++nSum;
	return nSum;
}

// [Timur]
/** Contain extensions for STL library.
*/
namespace stl
{
	//////////////////////////////////////////////////////////////////////////
	//! Searches the given entry in the map by key, and if there is none, returns the default value
	//////////////////////////////////////////////////////////////////////////
#ifdef WIN64
	template <typename Map, typename key_type, typename mapped_type>
		inline mapped_type find_in_map(const Map& mapKeyToValue, key_type key, mapped_type valueDefault)
#else
	template <typename Map>
		inline typename Map::mapped_type find_in_map(const Map& mapKeyToValue, typename Map::key_type key, typename Map::mapped_type valueDefault)
#endif
	{
		typename Map::const_iterator it = mapKeyToValue.find (key);
		if (it == mapKeyToValue.end())
			return valueDefault;
		else
			return it->second;
	}

	// searches the given entry in the map by key, and if there is none, returns the default value
	// The values are taken/returned in REFERENCEs rather than values
#ifdef WIN64
	template <typename Map, typename key_type, typename mapped_type>
		inline mapped_type& find_in_map_ref(Map& mapKeyToValue, key_type key, mapped_type& valueDefault)
#else
	template <typename Map>
		inline typename Map::mapped_type& find_in_map_ref(Map& mapKeyToValue, typename Map::key_type key, typename Map::mapped_type& valueDefault)
#endif
	{
		typename Map::iterator it = mapKeyToValue.find (key);
		if (it == mapKeyToValue.end())
			return valueDefault;
		else
			return it->second;
	}

	//////////////////////////////////////////////////////////////////////////
	//! Fills vector with contents of map.
	//////////////////////////////////////////////////////////////////////////
	template <class Map,class Vector>
	inline void map_to_vector( const Map& theMap,Vector &array )
	{
		array.resize(0);
		array.reserve( theMap.size() );
		for (typename Map::const_iterator it = theMap.begin(); it != theMap.end(); ++it)
		{
			array.push_back( it->second );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//! Fills vector with contents of set.
	//////////////////////////////////////////////////////////////////////////
	template <class Set,class Vector>
	inline void set_to_vector( const Set& theSet,Vector &array )
	{
		array.resize(0);
		array.reserve( theSet.size() );
		for (typename Set::const_iterator it = theSet.begin(); it != theSet.end(); ++it)
		{
			array.push_back( *it );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//! Find and erase element from container.
	// @return true if item was find and erased, false if item not found.
	//////////////////////////////////////////////////////////////////////////
	template <class Container,class Value>
		inline bool find_and_erase( Container& container,const Value &value )
	{
		typename Container::iterator it = std::find( container.begin(),container.end(),value );
		if (it != container.end())
		{
			container.erase( it );
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	//! Push back to container unique element.
	// @return true if item added, false overwise.
	template <class Container,class Value>
		inline bool push_back_unique( Container& container,const Value &value )
	{
		if (std::find(container.begin(),container.end(),value) == container.end())
		{
			container.push_back( value );
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	//! Find element in container.
	// @return true if item found.
	template <class Container,class Value>
		inline bool find( Container& container,const Value &value )
	{
		return std::find(container.begin(),container.end(),value) != container.end();
	}

	//////////////////////////////////////////////////////////////////////////
	//! Convert arbitary class to const char*
	//////////////////////////////////////////////////////////////////////////
	template <class Type>
		inline const char* constchar_cast( const Type &type )
	{
		return type;
	}

	//! Specialization of string to const char cast.
	template <>
		inline const char* constchar_cast( const string &type )
	{
		return type.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	//! Case sensetive less key for any type convertable to const char*.
	//////////////////////////////////////////////////////////////////////////
	template <class Type>
	struct less_strcmp : public std::binary_function<Type,Type,bool> 
	{
		bool operator()( const Type &left,const Type &right ) const
		{
			return strcmp(constchar_cast(left),constchar_cast(right)) < 0;
		}
	};

	//////////////////////////////////////////////////////////////////////////
	//! Case insensetive less key for any type convertable to const char*.
	template <class Type>
	struct less_stricmp : public std::binary_function<Type,Type,bool> 
	{
		bool operator()( const Type &left,const Type &right ) const
		{
			return stricmp(constchar_cast(left),constchar_cast(right)) < 0;
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// Hash map usage:
	// typedef stl::hash_map<string,int, stl::hash_stricmp<string> > StringToIntHash;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//! Case sensetive string hash map compare structure.
	//////////////////////////////////////////////////////////////////////////
	template <class Key>
	class hash_strcmp
	{
	public:
		enum {	// parameters for hash table
			bucket_size = 4,	// 0 < bucket_size
			min_buckets = 8	};// min_buckets = 2 ^^ N, 0 < N

			size_t operator()( const Key& key ) const
			{
				unsigned int h = 0; 
				const char *s = constchar_cast(key);
				for (; *s; ++s) h = 5*h + *(unsigned char*)s;
				return size_t(h);

			};
			bool operator()( const Key& key1,const Key& key2 ) const
			{
				return strcmp(constchar_cast(key1),constchar_cast(key2)) < 0;
			}
	};

	//////////////////////////////////////////////////////////////////////////
	//! Case insensetive string hash map compare structure.
	template <class Key>
	class hash_stricmp
	{
	public:
		enum {	// parameters for hash table
			bucket_size = 4,	// 0 < bucket_size
			min_buckets = 8	};// min_buckets = 2 ^^ N, 0 < N

			size_t operator()( const Key& key ) const
			{
				unsigned int h = 0; 
				const char *s = constchar_cast(key);
				for (; *s; ++s) h = 5*h + tolower(*(unsigned char*)s);
				return size_t(h);

			};
			bool operator()( const Key& key1,const Key& key2 ) const
			{
				return stricmp(constchar_cast(key1),constchar_cast(key2)) < 0;
			}
	};
#if !defined(LINUX)
	// Support for both Microsoft and SGI kind of hash_map.
	template <class Key,class Value,class HashFunc>
	class hash_map : 
#ifdef _STLP_HASH_MAP // STL Port
		std::hash_map<Key,Value,HashFunc,HashFunc>
#else
		std::hash_map<Key,Value,HashFunc>
#endif // STL Port
	{};
#endif //LINUX
}

#endif