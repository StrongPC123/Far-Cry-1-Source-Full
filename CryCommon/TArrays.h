#ifndef _CRY_COMMON_TARRAYS_HDR_
#define _CRY_COMMON_TARRAYS_HDR_

#include "platform.h"
#if !defined(LINUX)
#include <assert.h>
#endif

#include "TAlloc.h"

template <typename T>
void TSimpleSwap (T& a, T& b)
{
	T t = a;
	a = b;
	b = t;
}


//////////////////////////////////////////////////////////////////////////
// a class with STL naming conventions, incapsulating just a pointer.
// it mimics the std::vector with a few methods, but the class doesn't know
// about the array size and lacks most of the vector functionality.
// WHAT IT IS FOR:
// If you need an array that you know the size for, use this.
// If you need a local dynamic array that will self-deallocate when out of scope of the function, use this.
//////////////////////////////////////////////////////////////////////////
template <typename T, class A = TSimpleAllocator<T> > 
class TElementaryArray
{
public:
	typedef T value_type;
	typedef T& reference;
	typedef const T & const_reference;
	typedef T* iterator;
	typedef const T* const_iterator;

	TElementaryArray (const char* szParentObject, int nParentIndex = 0):
		m_Allocator(szParentObject, nParentIndex),
		m_pData (NULL)
	{
#if defined(_DEBUG) && defined(_INC_CRTDBG)
		m_nSize = 0;
#endif
	}

	TElementaryArray (size_t numElements)
#if defined(_DEBUG) && defined(_INC_CRTDBG)
		:m_Allocator("TElementaryArray(n)",1),
		m_nSize(numElements)
#endif
	{  
		m_pData = numElements?m_Allocator.allocate_construct(numElements):NULL;
	}

	TElementaryArray ():
		m_pData (NULL)
#if defined(_DEBUG) && defined(_INC_CRTDBG)
		,m_Allocator("TElementaryArray()",2)
		,m_nSize(0)
#endif
	{
	}

private:
	// copy constructor: impossible since the number of elements is unknown
	TElementaryArray (const TElementaryArray<T,A>& that):
		m_pData (NULL)
	{
		assert(0);
	}

	// assignment is impossible because the size is unknown
	TElementaryArray<T,A>& operator = (const TElementaryArray<T,A>& src)
	{
		assert(0);
	}

public:
	~TElementaryArray()
	{
		m_Allocator.deallocate_destroy(m_pData);
#if defined(_DEBUG) && defined(_INC_CRTDBG)
		m_pData = (T*)(UINT_PTR)0xFFEEDDCC; // for debug
#endif
	}

	bool empty() const
	{
		return m_pData == 0;
	}

	// resizes the array, doesn't preserve the contents
	void reinit (size_t numElements)
	{
		clear();
		if (numElements > 0)
		{
			m_pData = m_Allocator.allocate_construct(numElements);
#if defined(_DEBUG) && defined(_INC_CRTDBG)
			m_nSize = numElements;
#endif
		}
	}

	// resizes the array, doesn't preserve the contents, copies the given value to all the elements
	void reinit (size_t numElements, const T& element)
	{
		reinit(numElements);
		for (size_t i = 0; i < numElements; ++i)
			m_pData[i] = element;
	}

	// swaps this array with the given one
	void swap (TElementaryArray<T,A>& rRight)
	{
		TSimpleSwap (m_pData, rRight.m_pData);
#if defined(_DEBUG) && defined(_INC_CRTDBG)
		TSimpleSwap (m_nSize, rRight.m_nSize);
#endif
	}

	// cleans up the array
	void clear()
	{
		if (m_pData)
		{
			m_Allocator.deallocate_destroy(m_pData);
			m_pData = NULL;
#if defined(_DEBUG) && defined(_INC_CRTDBG)
			m_nSize = 0;
#endif
		}
	}

	// returns the first element of the array
	reference front()
	{
		assert (!empty());
		return m_pData[0];
	}

	// returns the first element of the array
	const_reference front() const
	{
		assert (!empty());
		return m_pData[0];
	}

	// returns the begin iterator of the contained sequence
	// there is no end iterator in this array, if you need one, use std::vector
	iterator begin()
	{
		return m_pData;
	}

	// returns the begin iterator of the contained sequence
	// there is no end iterator in this array, if you need one, use std::vector
	const_iterator begin() const
	{
		return m_pData;
	}

	reference operator [] (size_t i)
	{
		// it is unlikely that the array is more than 1 gigabyte long, so check the index for saneness
		assert (i < 0x40000000 / sizeof(T));
#if defined(_DEBUG) && defined(_INC_CRTDBG)
		assert (i < m_nSize);
#endif
		return m_pData[i];
	}

	const_reference operator [] (size_t i) const
	{
		// it is unlikely that the array is more than 1 gigabyte long, so check the index for saneness
		assert (i < 0x40000000 / sizeof(T));
#if defined(_DEBUG) && defined(_INC_CRTDBG)
		assert (i < m_nSize);
#endif
		return m_pData[i];
	}
protected:
	// array of elements, or NULL
	T* m_pData;

#if defined(_DEBUG) && defined(_INC_CRTDBG)
	// the actual size of the array, USE FOR DEBUGGING ONLY
	size_t m_nSize;
#endif
	A m_Allocator;
};


////////////////////////////////////////////////////////////////////////////////////////
// a class with STL naming conventions of the methods, incapsulating an array of fixed
// size. STL naming conventions are for easier transition between this and std::vector.
// optimized for one-time construction and long-term usage (minimum memory overhead)
// You can reinitialize the array with resize() though, but be careful: it's not fast
//
// The only advantage over std::vector is that the number of elements allocates is kept
// to the minimum, which is optimal in the case of one-time allocation
////////////////////////////////////////////////////////////////////////////////////////
template <typename T, class A = TSimpleAllocator<T> >
class TFixedArray
{
public:
	typedef T value_type;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T* iterator;
	typedef const T* const_iterator;

	TFixedArray (const char* szParentObject, int nParentIndex = 0):
		m_nSize (0),
		m_pData (NULL),
		m_Allocator(szParentObject, nParentIndex)
	{
	}
	
	TFixedArray (size_t numElements):
		m_nSize (numElements)
	{
		m_pData = numElements?m_Allocator.allocate_construct(numElements):NULL;
	}

	TFixedArray ():
		m_nSize (0),
		m_pData (NULL)
	{
	}  

	// copy constructor: full copy implementation
	template <typename U, typename B>
	TFixedArray (const TFixedArray<U,B>& that):
		m_nSize (that.size()),
		m_Allocator(that.m_Allocator)
	{
		m_pData = that.empty() ? NULL : m_Allocator.allocate_construct(that.size());
		for (size_t i = 0; i < m_nSize; ++i)
			m_pData[i] = that[i];
	}

	// copy constructor: full copy implementation
	TFixedArray (const TFixedArray<T,A>& that):
		m_nSize (that.size()),
		m_Allocator(that.m_Allocator)
	{
		m_pData = that.empty() ? NULL : m_Allocator.allocate_construct(that.size());
		for (size_t i = 0; i < m_nSize; ++i)
			m_pData[i] = that[i];
	}

	// copy constructor: full copy implementation
	template <typename U, typename B>
	TFixedArray (const char* szParentObject, int nParentIndex, const TFixedArray<U,B>& that):
		m_nSize (that.size()),
		m_Allocator(szParentObject, nParentIndex)
	{
		m_pData = that.empty() ? NULL : m_Allocator.allocate_construct(that.size());
		for (size_t i = 0; i < m_nSize; ++i)
			m_pData[i] = that[i];
	}

	// copy constructor: full copy implementation
	TFixedArray (const char* szParentObject, int nParentIndex, const TFixedArray<T,A>& that):
		m_nSize (that.size()),
		m_Allocator(szParentObject, nParentIndex)
	{
		m_pData = that.empty() ? NULL : m_Allocator.allocate_construct(that.size());
		for (size_t i = 0; i < m_nSize; ++i)
			m_pData[i] = that[i];
	}

	// sets the owner on behalf of whom the memory will be allocated
	void dbgAssignOwner (const char* szParentObject, int nParentIndex = 0)
	{
		m_Allocator = A (szParentObject, nParentIndex);
	}

	template <typename U,typename B>
	TFixedArray<T,A>& operator = (const TFixedArray<U,B>& src)
	{
		reinit (src.size());
		for (size_t i = 0; i < m_nSize; ++i)
			m_pData[i] = src[i];
		return *this;
	}

	TFixedArray<T,A>& operator = (const TFixedArray<T,A>& src)
	{
		reinit (src.size());
		for (size_t i = 0; i < m_nSize; ++i)
			m_pData[i] = src[i];
		return *this;
	}

	~TFixedArray ()
	{
		m_Allocator.deallocate_destroy(m_pData);
#if defined(_DEBUG)
		m_pData = (T*)0x32323232;
		m_nSize = 0x43434343;
#endif
	}

	// returns the size of the array (the number of elements)
	size_t size() const
	{
		return m_nSize;
	}

	bool empty() const
	{
		return m_nSize == 0;
	}

	// resizes the array, doesn't preserve the contents
	void reinit (size_t numElements)
	{
		clear();
		if (numElements > 0)
			m_pData = m_Allocator.allocate_construct (m_nSize = numElements);
	}

	// resizes the array, doesn't preserve the contents, copies the given value to all the elements
	void reinit (size_t numElements, const T& element)
	{
		reinit(numElements);
		for (size_t i = 0; i < numElements; ++i)
			m_pData[i] = element;
	}

	// swaps this array with the given one
	void swap (TFixedArray<T,A>& rRight)
	{
		TSimpleSwap (m_pData, rRight.m_pData);
		TSimpleSwap (m_nSize, rRight.m_nSize);
	}

	// resizes the array, preserves the contents
	void resize (size_t numElements)
	{
		if (numElements != m_nSize)
		{
			if (numElements == 0)
			{
				clear();
			}
			else
			if (m_nSize == 0)
			{
				reinit (numElements);
			}
			else
			if (m_nSize != numElements)
			{
				T* pDataNew = m_Allocator.allocate_construct (numElements);
				size_t numToCopy = numElements < m_nSize?numElements:m_nSize;
				for (size_t i = 0; i < numToCopy; ++i)
					pDataNew[i] = m_pData[i];
				m_Allocator.deallocate_destroy(m_pData);
				m_pData = pDataNew;
				m_nSize = numElements;
			}
		}
	}

	// resizes the array, preserves the contents, copies the given value to the appended elements
	void resize (size_t numElements, const T& sample)
	{
		size_t i = m_nSize; // remember the old size
		resize (numElements);
		// initialize the tail of the array
		for (;i < numElements;++i)
			m_pData[i] = sample;
	}

	// cleans up the array
	void clear()
	{
		m_Allocator.deallocate_destroy(m_pData);
		m_pData = NULL;
		m_nSize = 0;
	}

	// adds the element to the end of the array
	void push_back (const T& sample)
	{
		// add one element, and copy the sample to it
		resize (size() + 1, sample);
	}

	// removes the element from the end of the array
	void pop_back()
	{
		if (size() > 0)
			resize (size()-1);
		else
			assert (0);
	}

	// inserts the element in place of the given iterator
	void insert (iterator it, const T& element)
	{
		assert (it >= begin() && it <= end());
		insert (it - begin(), element);
	}

	// inserts the element into the given place in the array\
	// (inserts before the given element)
	void insert (size_t nIndex, const T& element)
	{
		assert (nIndex <= size());

		T* pData = m_Allocator.allocate_construct (size() + 1);
		size_t i;
		// copy the pre-array
		for (i = 0; i < nIndex; ++i)
			pData[i] = m_pData[i];
		assert (i = nIndex);
		pData[i] = element;
		// copy the post-array
		for (; i < m_nSize;++i)
			pData[i+1] = m_pData[i];
		// swap the new and old arrays
		m_Allocator.deallocate_destroy(m_pData);
		m_pData = pData;
		++m_nSize;
	}

	// erases an element at the given position
	void erase (size_t nIndex)
	{
		if (nIndex < size())
		{
			if (size() > 1)
			{
				T* pData = m_Allocator.allocate_construct(size() - 1);
				size_t i;
				// copy the pre-array
				for (i = 0; i < nIndex; ++i)
					pData[i] = m_pData[i];
				assert (i = nIndex);
				// copy the post-array
				for (; i < m_nSize-1;++i)
					pData[i] = m_pData[i+1];
				// swap the new and old arrays
				m_Allocator.deallocate_destroy(m_pData);
				m_pData = pData;
				--m_nSize;

			}
			else
				clear();
		}
	}

	// returns the last element of the array
	reference back()
	{
		assert (!empty());
		return m_pData[m_nSize-1];
	}

	// returns the last element of the array
	const_reference back() const
	{
		assert (!empty());
		return m_pData[m_nSize-1];
	}

	// returns the first element of the array
	reference front()
	{
		assert (!empty());
		return m_pData[0];
	}

	// returns the first element of the array
	const_reference front() const
	{
		assert (!empty());
		return m_pData[0];
	}

	// returns the begin iterator of the contained sequence
	iterator begin()
	{
		return m_pData;
	}

	// returns the begin iterator of the contained sequence
	const_iterator begin() const
	{
		return m_pData;
	}

	// returns the end iterator of the contained sequence
	iterator end()
	{
		return m_pData+m_nSize;
	}

	// returns the end iterator of the contained sequence
	const_iterator end() const
	{
		return m_pData+m_nSize;
	}

	reference operator [] (size_t i)
	{
		// the i==0 is left here to allow extract the pointer to the 0th element
		// even when the array is empty. In case index is not 0, it must be < size
		assert(i == 0 || i < m_nSize);

		return m_pData[i];
	}
	
	const_reference operator [] (size_t i) const
	{
		// the i==0 is left here to allow extract the pointer to the 0th element
		// even when the array is empty. In case index is not 0, it must be < size
		assert(i == 0 || i < m_nSize);

		return m_pData[i];
	}
protected:
	// array of elements
	T* m_pData;
	// number of elements in the array m_pData
	size_t m_nSize;

	A m_Allocator;
};


//////////////////////////////////////////////////////////////////////////
// A class with STL naming conventions
// Damped implementation of a resizable array: once the capacity is increased,
// it never decreases during normal resizing operations.
// The hidden fixed array is used as the storage, and the outer represnetation
// of the array size is kept in a separate variable
template <typename T, class A = TSimpleAllocator<T> > class TGrowArray : protected TFixedArray<T,A>
{
public:
	typedef typename TFixedArray<T,A>::iterator iterator;
	typedef typename TFixedArray<T,A>::const_iterator const_iterator;
	typedef typename TFixedArray<T,A>::reference reference;
	typedef typename TFixedArray<T,A>::const_reference const_reference;

	TGrowArray ():
		TFixedArray<T,A> ("TGrowArray()", 1),
		m_nSize(0)
	{
	}

	TGrowArray (const char* szParentObject, int nParentIndex = 0):
		TFixedArray<T,A> (szParentObject, nParentIndex),
		m_nSize (0)
	{
	}

	TGrowArray (int numElements):
		TFixedArray<T,A>(numElements, "TGrowArray(n)",2),
		m_nSize (numElements)
	{
	}

	// copy constructor: full copy implementation
	// reserves enough data to contain the whole array
	template <typename U, typename B>
		TGrowArray (const TGrowArray<U,B>& that):
		TFixedArray<T,A>(that),
		m_nSize (that.size())
	{
	}

	// copy constructor: full copy implementation
	TGrowArray (const TGrowArray<T,A>& that):
		TFixedArray<T,A>(that),
		m_nSize (that.size())
	{
	}

	// copy constructor: full copy implementation
	// reserves enough data to contain the whole array
	template <typename U, typename B>
		TGrowArray (const char* szParentObject, int nParentIndex, const TGrowArray<U,B>& that):
		TFixedArray<T,A>(szParentObject, nParentIndex, that),
		m_nSize (that.size())
	{
	}

	// copy constructor: full copy implementation
	TGrowArray (const char* szParentObject, int nParentIndex, const TGrowArray<T,A>& that):
		TFixedArray<T,A>(szParentObject, nParentIndex, that),
		m_nSize (that.size())
	{
	}

	template <typename U, typename B>
		TGrowArray<T,A>& operator = (const TGrowArray<U,B>& src)
	{
		reinit (src.size());
		for (size_t i = 0; i < m_nSize; ++i)
			this->m_pData[i] = src[i];
		return *this;
	}

	TGrowArray<T,A>& operator = (const TGrowArray<T,A>& src)
	{
		reinit (src.size());
		for (size_t i = 0; i < m_nSize; ++i)
			this->m_pData[i] = src[i];
		return *this;
	}

	~TGrowArray ()
	{
#if defined(_DEBUG) && defined(_INC_CRTDBG)
		m_nSize = 0x42434243;
#endif
	}

	// returns the size of the array (the number of elements)
	size_t size() const
	{
		return m_nSize;
	}

	bool empty() const
	{
		return m_nSize == 0;
	}

	size_t capacity ()const
	{
		return TFixedArray<T,A>::size();
	}

	// this is the algorithm that dampers element reallocations.
	// given the minimum required number of elements, it returns
	// some number >= the given so that the next reallocation will not occur immediately
	// after reaching that number
	static inline size_t dampNumElements(size_t numElements)
	{
		return numElements + (numElements >> 1);
	}

	// resizes the array, doesn't preserve the contents
	void reinit (size_t numElements)
	{
		if (capacity() < numElements)
		{
			TFixedArray<T,A>::reinit (dampNumElements(numElements));
		}
		m_nSize = numElements;
	}

	// resizes the array, doesn't preserve the contents, copies the given value to all the elements
	void reinit (size_t numElements, const T& element)
	{
		reinit(numElements);
		for (size_t i = 0; i < numElements; ++i)
			this->m_pData[i] = element;
	}

	// swaps this array with the given one
	void swap (TGrowArray<T,A>& rRight)
	{
		TFixedArray<T,A>::swap (rRight);
		TSimpleSwap (m_nSize, rRight.m_nSize);
	}

	// resizes the array, preserves the contents
	void resize (size_t numElements)
	{
		reserve (numElements);
		m_nSize = numElements;
	}

	// resizes the array, preserves the contents, copies the given value to the appended elements
	void resize (size_t numElements, const T& sample)
	{
		size_t i = m_nSize; // remember the old size
		resize (numElements);
		// initialize the tail of the array
		for (;i < numElements;++i)
			this->m_pData[i] = sample;
	}

	// adds the element to the end of the array
	void push_back (const T& sample)
	{
		// add one element, and copy the sample to it
		resize (size() + 1, sample);
	}

	// deletes the element from the end of the array;
	// doesn't actually destruct the element as it can be reused
	void pop_back()
	{
		if (m_nSize > 0)
			--m_nSize;
		else
			assert (0);
	}

	// inserts the element in place of the given iterator
	void insert (iterator it, const T& element)
	{
		assert (it >= begin() && it <= end());
		insert (it - begin(), element);
	}

	// inserts the element into the given place in the array\
	// (inserts before the given element)
	void insert (size_t nIndex, const T& element)
	{
		assert (nIndex <= size());
		resize (size() + 1);
		for (size_t i = size()-1; i > nIndex; --i)
			(*this)[i] = (*this)[i-1];
		(*this)[nIndex] = element;
	}

	// erases an element at the given position
	void erase (iterator it)
	{
		assert (it >= begin() && it < end());
		erase (it - begin());
	}

	// erases an element at the given position
	void erase (size_t nIndex)
	{
		assert (nIndex < size());
		for (size_t i = nIndex; i < size()-1; ++i)
			(*this)[i] = (*this)[i+1];
		resize (size()-1);
	}


	// makes sure capacity() returns at least the given value
	// keeps the current contents
	void reserve (size_t numElements)
	{
		if (numElements > capacity())
		{
			if (m_nSize)
				TFixedArray<T,A>::resize (numElements);
			else
				TFixedArray<T,A>::reinit (numElements);
		}
	}

	// makes sure capacity() returns the same value as size()
	void shrink ()
	{
		assert (capacity() >= size());
		if (size() == 0)
		{
			// clear the reserve
			TFixedArray<T,A>::clear();
		}
		else
		{
			// make the reserve exactly the size of this array
			TFixedArray<T,A>::resize(size());
		}
	}

	// cleans up the array
	void clear()
	{
		m_nSize = 0;
	}

	// reserves at least the given number of elements and resets (clears) the array
	void reset (size_t numElements)
	{
		clear();
		if (numElements > capacity())
			TFixedArray<T,A>::reinit (numElements);
	}

	// returns the last element of the array
	reference back()
	{
		assert (!empty());
		return this->m_pData[m_nSize-1];
	}

	// returns the last element of the array
	const_reference back() const
	{
		assert (!empty());
		return this->m_pData[m_nSize-1];
	}

	// returns the first element of the array
	reference front()
	{
		assert (!empty());
		return this->m_pData[0];
	}

	// returns the first element of the array
	const_reference front() const
	{
		assert (!empty());
		return this->m_pData[0];
	}

	// returns the begin iterator of the contained sequence
	iterator begin()
	{
		return this->m_pData;
	}

	// returns the begin iterator of the contained sequence
	const_iterator begin() const
	{
		return this->m_pData;
	}

	// returns the end iterator of the contained sequence
	iterator end()
	{
		return this->m_pData+m_nSize;
	}

	// returns the end iterator of the contained sequence
	const_iterator end() const
	{
		return this->m_pData+m_nSize;
	}

	reference operator [] (size_t i)
	{
		if(this->m_pData)
			assert(i >= 0 && i < m_nSize);
		else
			assert(i==0);

		return this->m_pData[i];
	}

	const T& operator [] (size_t i) const
	{
		if(this->m_pData)
			assert(i >= 0 && i < m_nSize);
		else
			assert(i==0);

		return this->m_pData[i];
	}
protected:
	// number of elements - as it's seen from the outside
	size_t m_nSize;
};






// the follwoing definitions are for auto-declaring getters/setters for 
// T*Array members
// USAGE:
//  if you have an array std::vector<CryVertex> m_arrMyArray,
//  then Type is CryVertex, Singular is probably Vertex, Plural is probably Vertices, member is m_arrMyArray

#define DECLARE_ELEMENTARY_ARRAY_GETTER_METHODS(Type, Singular, Plural, member) \
Type* get##Plural() {return member.begin();}												\
const Type* get##Plural()const {return member.begin();}							\
Type& get##Singular(unsigned i) {assert (i < num##Plural()); return member[i];}             \
const Type& get##Singular(unsigned i)const {assert (i < num##Plural()); return member[i];}

#define DECLARE_PLAIN_ARRAY_GETTER_METHODS(Type, Singular, Plural, member) \
	Type* get##Plural() {return member;}												\
	const Type* get##Plural()const {return member;}							\
	Type& get##Singular(unsigned i) {assert (i < num##Plural()); return member[i];}             \
	const Type& get##Singular(unsigned i)const {assert (i < num##Plural()); return member[i];}	\
	void set##Singular (unsigned i, const Type& newValue) {assert (i < num##Plural()); member[i] = newValue;}


#define DECLARE_ARRAY_GETTER_METHODS(Type, Singular, Plural, member) \
DECLARE_ELEMENTARY_ARRAY_GETTER_METHODS(Type, Singular, Plural, member) \
unsigned num##Plural() const{return (unsigned)member.size();}

#endif