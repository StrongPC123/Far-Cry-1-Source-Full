/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//  File:SparseArrayDriver.cpp
//  Description: Implementation a template that allows to access sparse arrays (like vertex array within
//  the vertex buffer, where vertices are stored along with some other info)
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _SPARSE_ARRAY_DRIVER_HDR_
#define _SPARSE_ARRAY_DRIVER_HDR_

//////////////////////////////////////////////////////////////////////////
// The sparse array driver is just a wrapper that eases access to sparse arrays
// A sparse array is an array of values that are not packed to each other - that is,
// some arbitrary number of bytes may be between successive elements.
// The Driver suffix means that this class doesn't actually allocate any memory - it 
// will just serve as a smart pointer to the sparse array structure.
// you can safely copy instances of this class - it does not do any deallocation either
//
// In other words, this can be used as a pointer. And it has some pointer arithmetics in it, too
template <class T>
class TSparseArrayDriver
{
protected:
	// NOTE: only the descendants of this class (like Vec3 drivers for vertex buffer)
	// can construct uninitialized copy of this class, because they know how to initialize it afterwards.
	TSparseArrayDriver(){}
public:
	typedef unsigned char byte;

	// initializes the sparse array: the pointer to the first element, and the distance between elements
	// in bytes (by default, as for non-sparse arrays, the distance is the sizeof(T))
	// The distance is between the first byte of one element and the first byte of the next element
	TSparseArrayDriver (void* pData, int nStride):
		m_pData ((byte*)pData),
		m_nStride (nStride)
	{
	}

	T& operator [] (int nIndex)
	{
		return *(T*)(m_pData + nIndex * m_nStride);
	}

	const T& operator [] (int nIndex) const
	{
		return *(const T*)(m_pData + nIndex * m_nStride);
	}

	// dereference
	T& operator * ()
	{
		return *(T*)m_pData;
	}

	const T& operator * () const
	{
		return *(const T*)m_pData;
	}

	// smart pointer arithmetics

	// shifts the array pointer to the 
	TSparseArrayDriver<T>& operator += (int nShift) {m_pData += nShift*m_nStride; return *this;}
	TSparseArrayDriver<T>& operator ++ () {m_pData += m_nStride; return *this;}

	operator bool ()const {return m_pData != NULL;}
protected:
	// pointer to the 0th element of the array
	byte* m_pData;
	// stride - distance between element (first bytes)
	int m_nStride;
};

#ifdef VECTOR_H
typedef TSparseArrayDriver<Vec3> Vec3dSparseArrayDriver;
#endif

#endif