#ifndef _CRY_SYSTEM_MT_SAFE_ALLOCATOR_HDR_
#define _CRY_SYSTEM_MT_SAFE_ALLOCATOR_HDR_

#include <stdexcept>
#if defined( LINUX )
#	include "WinBase.h"
#endif

class CMTSafeHeap
{
public:
	CMTSafeHeap(unsigned nInitialSize, unsigned nMaxSize);
	~CMTSafeHeap();

	void* Alloc (size_t nSize, const char* szDbgSource);
	void Free (void*p);

	// the number of allocations this heap holds right now
	unsigned NumAllocations()const {return m_numAllocations;}
	// the total size of all allocations
	size_t GetAllocatedSize()const {return m_nTotalAllocations;}
	// consolidates free space inthe heap, uncommits if too much
	void Compact();

	// zlib-compatible stubs
	static void* StaticAlloc (void* pOpaque, unsigned nItems, unsigned nSize)
	{
		return ((CMTSafeHeap*)pOpaque)->Alloc(nItems * nSize, "zlib-compatible");
	}

	static void StaticFree (void* pOpaque, void* pAddress)
	{
		((CMTSafeHeap*)pOpaque)->Free(pAddress);
	}

protected:
	void* TryAlloc(size_t nSize, const char* szDbgSource);

	HANDLE m_hHeap;
	LONG m_numAllocations;
	size_t m_nTotalAllocations;
};

// this CMTSafeAllocator is MT-safe and is for use in, e.g. Stream Engine
// in multithreaded environment. STL-Compatible
template<class _Ty, class _THeap=CMTSafeHeap>
class CMTSafeAllocator
{
public:
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef _Ty *pointer;
		typedef const _Ty *const_pointer;
		typedef _Ty & reference;
		typedef const _Ty & const_reference;
		typedef _Ty value_type;

		template<class _Other>
		struct rebind
		{	// convert an CMTSafeAllocator<_Ty> to an CMTSafeAllocator <_Other>
			typedef CMTSafeAllocator<_Other> other;
		};

		pointer address(reference _Val) const
		{	// return address of mutable _Val
			return (&_Val);
		}

		const_pointer address(const_reference _Val) const
		{	// return address of nonmutable _Val
			return (&_Val);
		}

		CMTSafeAllocator(_THeap* pHeap):
			m_pHeap(pHeap)
		{
		}

		CMTSafeAllocator(const CMTSafeAllocator<_Ty>&rThat):
			m_pHeap(rThat.m_pHeap)
		{
		}

		template<class _Other>
		CMTSafeAllocator(const CMTSafeAllocator<_Other, _THeap>&rThat)
		{	
			m_pHeap = rThat.GetHeap();
		}

		template<class _Other>
			CMTSafeAllocator<_Ty>& operator=(const CMTSafeAllocator<_Other>& rThat)
		{	
			m_pHeap = rThat.m_pHeap;
			return (*this);
		}

		pointer allocate (size_type _Count, const void *)
		{	// allocate array of _Count elements, ignore hint
			return (Allocate(_Count, (pointer)0));
		}

		pointer allocate(size_type _Count)
		{	// allocate array of _Count elements
			return Allocate(_Count, (pointer)0);
		}

		void deallocate(void* _Ptr, size_type)
		{	// deallocate object at _Ptr, ignore size
			Deallocate(_Ptr);
		}

		void construct(pointer _Ptr, const _Ty& _Val)
		{	// construct object at _Ptr with value _Val
			std::_Construct(_Ptr, _Val);
		}

		void destroy(pointer _Ptr)
		{	// destroy object at _Ptr
			std::_Destroy(_Ptr);
		}

		size_t max_size() const
		{	// estimate maximum array size
			size_t _Count = (size_t)(-1) / sizeof (_Ty);
			return (0 < _Count ? _Count : 1);
		}

#if defined(WIN64) || defined(LINUX64)
		char *_Charalloc(size_type _N)
		{
			return (char*)(Allocate((difference_type)_N, 0)); 
		}
#endif
public:
	_Ty *Allocate(size_t _Count, _Ty *);
	void Deallocate(void *);
	_THeap* GetHeap()const{return m_pHeap;}
protected:
	_THeap* m_pHeap;
};

template<class _Ty, class _THeap>
_Ty  *CMTSafeAllocator<_Ty, _THeap>::Allocate(size_t _Count, _Ty  *)
{	// allocate storage for _Count elements of type _Ty
	_Ty  * pMemory = (_Ty  *)m_pHeap->Alloc(_Count * sizeof (_Ty), "CMTSafeAllocator<_Ty, _THeap>::Allocate");

	//[Timur] std::_Nomemory() is not supported in SGI stlport.
	if(!pMemory)
		throw std::runtime_error("Not enough memory for CMTSafeAllocator::Allocate()");
	return pMemory;
}

template<class _Ty, class _THeap>
void CMTSafeAllocator<_Ty, _THeap>::Deallocate(void *p)
{
	m_pHeap->Free (p);
}

template<class _Ty,
class _Other, class _THeap> inline
	bool operator==(const CMTSafeAllocator<_Ty, _THeap>&left, const CMTSafeAllocator<_Other, _THeap>&right)
{	// test for allocator equality (always true)
	return left.GetHeap() == right.GetHeap();
}

template<class _Ty,
class _Other, class _THeap> inline
	bool operator != (const CMTSafeAllocator<_Ty, _THeap>&left, const CMTSafeAllocator<_Other, _THeap>&right)
{	// test for allocator inequality (always false)
	return left.GetHeap() != right.GetHeap();
}

#endif