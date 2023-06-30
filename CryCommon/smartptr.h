#ifndef _SMART_PTR_H_
#define _SMART_PTR_H_
//////////////////////////////////////////////////////////////////
// SMART POINTER
//////////////////////////////////////////////////////////////////
template <class _I> class _smart_ptr 
{
private:
  _I* p;
public:
  _smart_ptr() : p(NULL) {}
#if defined(LINUX64)
	_smart_ptr(typeof(__null)) : p(NULL) {}
#endif
	_smart_ptr(int Null) : p(NULL) {}
  _smart_ptr(_I* p_)
	{
		p = p_;
		if (p)
			p->AddRef();
	}
	
  _smart_ptr(const _smart_ptr &p_)
	{
		p = p_.p;
		if (p)
			p->AddRef();
	}
	~_smart_ptr()
	{
		if (p)
			p->Release();
	}
  operator _I*() const { return p; }
  operator const _I*() const { return p; }
  _I& operator*() const { return *p; }
  _I* operator->(void) const { return p; }
  _smart_ptr&  operator=(_I* newp)
	{
		if (newp)
			newp->AddRef();
		if (p)
			p->Release();
		p = newp;
		return *this;
	}
	_smart_ptr&  operator=(const _smart_ptr &newp)
	{
		if (newp.p)
			newp.p->AddRef();
		if (p)
			p->Release();
		p = newp.p;
		return *this;
	}
  operator bool() const 
	{
		return p != NULL;
	};
	bool operator !() const 
	{
		return p == NULL;
	};
 	bool  operator ==(const _I* p2) const 
	{
		return p == p2;
	};
 	bool  operator ==(_I* p2) const 
	{
		return p == p2;
	};
  bool  operator !=(const _I* p2) const 
	{
		return p != p2;
	};
	bool  operator !=(_I* p2) const 
	{
		return p != p2;
	};
	bool  operator !=(const _smart_ptr &p2) const 
	{
		return p != p2.p;
	};
  bool  operator <(const _I* p2) const 
	{
		return p < p2;
	};
  bool  operator >(const _I* p2) const 
	{
		return p > p2;
	};
};

template <class _I>
inline bool operator ==(const _smart_ptr<_I> &p1, int null)	
{
	return !(bool)p1;	
}
template <class _I>
inline bool operator !=(const _smart_ptr<_I> &p1, int null)
{
	return (bool)p1;	
}
template <class _I>
inline bool operator ==(int null, const _smart_ptr<_I> &p1)
{
	return !(bool)p1;	
}
template <class _I>
inline bool operator !=(int null, const _smart_ptr<_I> &p1)
{
	return (bool)p1;	
}
/*
#if defined(LINUX64)
template <class _I>
inline bool operator ==(const _smart_ptr<_I> &p1, typeof(__null))	
{
	return !(bool)p1;	
}
template <class _I>
inline bool operator !=(const _smart_ptr<_I> &p1, typeof(__null))
{
	return (bool)p1;	
}
template <class _I>
inline bool operator ==(typeof(__null), const _smart_ptr<_I> &p1)
{
	return !(bool)p1;	
}
template <class _I>
inline bool operator !=(typeof(__null), const _smart_ptr<_I> &p1)
{
	return (bool)p1;	
}
#endif //LINUX64
*/
// reference target for smart pointer
// implements AddRef() and Release() strategy using reference counter of the specified type
template <typename Counter> class _reference_target
{
public:
	_reference_target():
		m_nRefCounter (0)
	{
	}

	virtual ~_reference_target()
	{
		//assert (!m_nRefCounter);
	}

	void AddRef()
	{
		++m_nRefCounter;
	}

	void Release()
	{
		if (--m_nRefCounter <= 0)
			delete this;
	}
	// Warning: use for debugging/statistics purposes only!
	Counter NumRefs()
	{
		return m_nRefCounter;
	}
protected:
	Counter m_nRefCounter;
};

// default implementation is int counter - for better alignment
typedef _reference_target<int> _reference_target_t;

#if (defined(_WINDOWS_)||defined(LINUX))

// reference target for smart pointer
// implements AddRef() and Release() strategy using reference counter of the specified type
template <class Derived>
class _reference_target_MT_novtbl
{
public:
	_reference_target_MT_novtbl():
		m_nRefCounter (0)
	{
	}

	void AddRef()
	{
		InterlockedIncrement (&m_nRefCounter);
	}

	void Release()
	{
		if (InterlockedDecrement(&m_nRefCounter) <= 0)
			delete static_cast<Derived*>(this);
	}

	int32 NumRefs()const {return m_nRefCounter;}
protected:
#if defined(LINUX)
	volatile signed int m_nRefCounter;
#else
	volatile signed long m_nRefCounter;
#endif
};

class _reference_target_MT: public _reference_target_MT_novtbl<_reference_target_MT>
{
public:
	virtual ~_reference_target_MT()
	{
	}
};

#endif //_WINDOWS_

// base class for interfaces implementing reference counting
// derive your interface from this class and the descendants won't have to implement
// the reference counting logic
template <typename Counter> class _i_reference_target
{
public:
	_i_reference_target():
		m_nRefCounter (0)
	{
	}

	virtual ~_i_reference_target()
	{
	}

	virtual void AddRef()
	{
		++m_nRefCounter;
	}

	virtual void Release()
	{
		if (--m_nRefCounter <= 0)
			delete this;
	}

	// Warning: use for debugging/statistics purposes only!
	Counter NumRefs()	const
	{
		return m_nRefCounter;
	}
protected:
	Counter m_nRefCounter;
};

typedef _i_reference_target<int> _i_reference_target_t;

#if (defined(WIN32) || defined(LINUX))

//////////////////////////////////////////////////////////////////////////
// This class describes the set template that facilitates creation
// of the following pattern:
//    Suppose you have variable-length data structure with some header
//    Suppose you want to refer to this variable-size data structure with a smart pointer, with minimal memory overhead.
// Solution:
//    Derive your data structure header from the RefCountedDataInstance<your_structure_name>
//    Call the method ConstructRefCounter() to initialize the reference counter once you allocated your structure
//    Provide the method Delete() in your header structure: it will be called when the reference counter reaches zero
//    Assign the pointer to your data structure (after allocating enough memory) to a smart pointer to that structure
//		Access to your data instantly through the autopointer only

template <class Header>
class RefCountedDataInstance
{
protected:
#if defined(LINUX)
	volatile signed int m_nRefCount; // the reference count
#else
	volatile signed long m_nRefCount; // the reference count
#endif
public:
	void ConstructRefCounter()
	{
		m_nRefCount = 0;
	}

	void AddRef()
	{
		InterlockedIncrement(&m_nRefCount);
	}

	// self-destruct if ref count drops to 0
	void Release()
	{
		if (InterlockedDecrement(&m_nRefCount) <= 0)
			static_cast<Header*>(this)->Delete();
	}

	int32 NumRefs()const {return m_nRefCount;}
};

#endif


// TYPEDEF_AUTOPTR macro, declares Class_AutoPtr, which is the smart pointer to the given class,
// and Class_AutoArray, which is the array(STL vector) of autopointers
#ifdef ENABLE_NAIIVE_AUTOPTR
// naiive autopointer makes it easier for Visual Assist to parse the declaration and sometimes is easier for debug
#define TYPEDEF_AUTOPTR(T) typedef T* T##_AutoPtr; typedef std::vector<T##_AutoPtr> T##_AutoArray;
#else
#define TYPEDEF_AUTOPTR(T) typedef _smart_ptr<T> T##_AutoPtr; typedef std::vector<T##_AutoPtr> T##_AutoArray;
#endif

#endif //_SMART_PTR_H_
