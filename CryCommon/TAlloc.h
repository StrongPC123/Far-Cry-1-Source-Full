#ifndef _CRY_COMMON_TALLOC_HDR_
#define _CRY_COMMON_TALLOC_HDR_

#ifdef GAMECUBE
#include "GCDefines.h"
#endif


// the STL-compatible allocator. Allocates structures
// This means NO C++ CONSTRUCTION/DESTRUCTION is performed on the allocated objects
template <typename T>
class TSimpleStructAllocator 
{
public:
	typedef size_t size_type;
	typedef int difference_type;
	typedef T *pointer;
	typedef const T *const_pointer;
	typedef T & reference;
	typedef const T& const_reference;
	typedef T value_type;
	// signals to the clients that the objects need construction/can be constructed (and destructed)
	enum {canConstruct = 0};
	enum {canDestruct = 0};

	template<class _Other>
		struct rebind
		{	// convert an allocator<T> to an allocator <_Other>
		typedef TSimpleStructAllocator<_Other> other;
		};

	pointer address(reference _Val) const
		{	// return address of mutable _Val
		return (&_Val);
		}

	const_pointer address(const_reference _Val) const
		{	// return address of nonmutable _Val
		return (&_Val);
		}

	TSimpleStructAllocator<T>()
#if defined(_DEBUG) && defined(_INC_CRTDBG)
		: m_szParentObject("CrySimpleDebugAlloc"),
			m_nParentIndex (1)
#endif
	{
	}

	~TSimpleStructAllocator<T>()
	{
	}


	TSimpleStructAllocator<T>(const char* szParentObject, int nParentIndex)
#if defined(_DEBUG) && defined(_INC_CRTDBG)
		:	m_szParentObject(szParentObject),
			m_nParentIndex (nParentIndex)
#endif
		{	// construct default allocator (do nothing)
		}

	TSimpleStructAllocator<T>(const TSimpleStructAllocator<T>& rThat)
		{	// construct by copying (do nothing)
#if defined(_DEBUG) && defined(_INC_CRTDBG)
			m_szParentObject = rThat.m_szParentObject;
			m_nParentIndex = rThat.m_nParentIndex;
#endif
		}
	/*
	template<class _Other>
		TSimpleStructAllocator(const TSimpleStructAllocator<_Other>&rThat)
		{	// construct from a related allocator (do nothing)
#if defined(_DEBUG) && defined(_INC_CRTDBG)
			m_szParentObject = rThat.m_szParentObject;
			m_nParentIndex = rThat.m_nParentIndex;
#endif
		}

	template<class _Other>
		TSimpleStructAllocator<T>& operator=(const TSimpleStructAllocator<_Other>&rThat)
		{	// assign from a related allocator (do nothing)
#if defined(_DEBUG) && defined(_INC_CRTDBG)
		m_szParentObject = rThat.m_szParentObject;
		m_nParentIndex = rThat.m_nParentIndex;
#endif
		return (*this);
		}
	*/
	pointer allocate(size_type _Count, const void *)
		{	// allocate array of _Count elements, ignore hint
			return allocate (_Count);
		}

	pointer allocate(size_type _Count)
		{	// allocate array of _Count elements
#if defined(_DEBUG) && defined(_INC_CRTDBG)
		return (pointer)_malloc_dbg(_Count * sizeof(T), _NORMAL_BLOCK, m_szParentObject, m_nParentIndex);
#else
		return (pointer)malloc(_Count * sizeof(T));
#endif
		}
	pointer allocate_construct (size_type _Count)
	{
#if defined(_DEBUG) && defined(_INC_CRTDBG)
#if defined(DEBUG_NEW_NORMAL_CLIENTBLOCK)
//#pragma push_macro("new")
#undef new
		return ::new(_NORMAL_BLOCK,m_szParentObject, m_nParentIndex) T[_Count];
#define new DEBUG_NEW_NORMAL_CLIENTBLOCK( __FILE__, __LINE__)
//#pragma pop_macro("new")
#else
		return new T[_Count];
#endif
#else
		return new T[_Count];
#endif
	}

	void deallocate(pointer _Ptr, size_type)
	{	// deallocate object at _Ptr, ignore size
		operator delete(_Ptr);
	}

	void deallocate_destroy(pointer _Ptr)
	{
		delete[]_Ptr;
	}

	void deallocate_destroy(pointer _Ptr, size_type _Count)
	{
#if defined(LINUX)
		delete [] _Ptr;
#else
		delete[_Count]_Ptr;
#endif
	}

	void construct(pointer _Ptr, const T& _Val)
		{	// construct object at _Ptr with value _Val
		}

	void destroy(pointer _Ptr)
		{	// destroy object at _Ptr
		}

	size_t max_size() const
		{	// estimate maximum array size
		size_t _Count = (size_t)(-1) / sizeof (T);
		return (0 < _Count ? _Count : 1);
		}

#if defined(_DEBUG) && defined(_INC_CRTDBG)
		// the parent object (on behalf of which to call the new operator)
		const char* m_szParentObject; // the file name
		int m_nParentIndex; // the file line number
#endif
};

// this is the 
template <typename T>
class TSimpleAllocator: public TSimpleStructAllocator<T>
{
public:
	// signals to the clients that the objects need construction/can be constructed (and destructed)
	enum {canConstruct = 1};
	enum {canDestruct = 1};
	typedef typename TSimpleStructAllocator<T>::pointer pointer;
	TSimpleAllocator<T>()
	{
	}

	TSimpleAllocator<T>(const char* szParentObject, int nParentIndex = 0):
		TSimpleStructAllocator<T>(szParentObject, nParentIndex)
		{	// construct default allocator (do nothing)
		}

	TSimpleAllocator<T>(const TSimpleAllocator<T>& rThat):
		TSimpleStructAllocator<T>(rThat)
		{	// construct by copying (do nothing)
		}
	/*
	template<class _Other>
		TSimpleAllocator(const TSimpleAllocator<_Other>&rThat):
			TSimpleStructAllocator<T>(rThat)
		{	// construct from a related allocator (do nothing)
		}

	template<class _Other>
		TSimpleAllocator<T>& operator=(const TSimpleAllocator<_Other>&rThat)
		{	// assign from a related allocator (do nothing)
			*static_cast<TSimpleStructAllocator<T>*>(this) = rThat;
		return (*this);
		}
	*/
	void construct(pointer Ptr, const T& _Val)
	{	// construct object at _Ptr with value _Val
			//std::_Construct(_Ptr, _Val);
	}

	void destroy(pointer Ptr)
	{	// destroy object at _Ptr
			Ptr->~T();
	}
};

#endif