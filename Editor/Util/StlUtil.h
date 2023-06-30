////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   stlutil.h
//  Version:     v1.00
//  Created:     27/9/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Utility functions to simplify usage of STL.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __stlutil_h__
#define __stlutil_h__

#if _MSC_VER > 1000
#pragma once
#endif

/** Contain extensions for STL library.
*/
namespace stl
{
	//////////////////////////////////////////////////////////////////////////
	//! Searches the given entry in the map by key, and if there is none, returns the default value
	//////////////////////////////////////////////////////////////////////////
#ifdef WIN64 // workaround
	template <class Map, typename mapped_type, typename key_type>
	inline mapped_type& find_in_map( Map& mapKeyToValue, const key_type &key, mapped_type &valueDefault)
	{
		Map::iterator it = mapKeyToValue.find(key);
		if (it == mapKeyToValue.end())
			return valueDefault;
		else
		{
			mapped_type &ref = it->second;
			return ref;
		}
	}
#else //WIN64
	template <class Map>
	inline Map::mapped_type& find_in_map( Map& mapKeyToValue, const Map::key_type &key, Map::mapped_type &valueDefault)
	{
		Map::iterator it = mapKeyToValue.find(key);
		if (it == mapKeyToValue.end())
			return valueDefault;
		else
		{
			return it->second;
		}
	}
#endif //WIN64

#ifdef WIN64 // workaround
	template <class Map, typename mapped_type, typename key_type>
	inline const mapped_type& find_in_map(const Map& mapKeyToValue, const key_type &key, const mapped_type &valueDefault)
	{
		Map::const_iterator it = mapKeyToValue.find (key);
		if (it == mapKeyToValue.end())
			return valueDefault;
		else
		{
			const mapped_type &ref = it->second;
			return ref;
		}
	}
#else //WIN64
	template <class Map>
	inline const Map::mapped_type& find_in_map(const Map& mapKeyToValue, const Map::key_type &key, const Map::mapped_type &valueDefault)
	{
		Map::const_iterator it = mapKeyToValue.find (key);
		if (it == mapKeyToValue.end())
			return valueDefault;
		else
		{
			return it->second;
		}
	}
#endif //WIN64


	//////////////////////////////////////////////////////////////////////////
	//! Fills Vector with contents of Map.
	//////////////////////////////////////////////////////////////////////////
	template <class Map,class Value>
	inline void map_to_vector( const Map& theMap,std::vector<Value> &array )
	{
		array.clear();
		array.reserve( theMap.size() );
		for (Map::const_iterator it = theMap.begin(); it != theMap.end(); ++it)
		{
			array.push_back( it->second );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//! Find and erase element from container.
	// @return true if item was find and erased, false if item not found.
	//////////////////////////////////////////////////////////////////////////
	template <class Container,class Value>
	inline bool find_and_erase( Container& container,const Value &value )
	{
		Container::iterator it = std::find( container.begin(),container.end(),value );
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
	//! Case sensetive string hash map compare structure.
	template <class Key>
	class hash_strcmp
	{
	public:
		enum {	// parameters for hash table
			bucket_size = 4,	// 0 < bucket_size
			min_buckets = 8	};// min_buckets = 2 ^^ N, 0 < N

		size_t operator()( const Key& key ) const
		{
			unsigned long h = 0; 
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
			unsigned long h = 0; 
			const char *s = constchar_cast(key);
			for (; *s; ++s) h = 5*h + *(unsigned char*)s;
			return size_t(h);

		};
		bool operator()( const Key& key1,const Key& key2 ) const
		{
			return stricmp(constchar_cast(key1),constchar_cast(key2)) < 0;
		}
	};
}


#endif // __stlutil_h__
