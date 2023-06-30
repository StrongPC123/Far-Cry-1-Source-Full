////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   smartptr.h
//  Version:     v1.00
//  Created:     27/11/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __smartptr_h__
#define __smartptr_h__

#if _MSC_VER > 1000
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Class TSmartPtr<T>.
// Smart pointer warper class that implements reference counting on IUnknown
// interface compatable class.
//
///////////////////////////////////////////////////////////////////////////////
template <class _T>
class TSmartPtr {
  _T* p;
public:
  TSmartPtr() : p(NULL) {}
  TSmartPtr( _T* p_ ) : p(p_) { if (p) (p)->AddRef(); }
  TSmartPtr( const TSmartPtr<_T>& p_ ) : p(p_.p) { if (p) (p)->AddRef(); }  // Copy constructor.
  TSmartPtr( int Null ) : p(NULL) {}
  ~TSmartPtr() { if (p) (p)->Release(); }

  operator _T*() const { return p; }
  operator const _T*() const { return p; }
  _T& operator*() const { return *p; }
  _T* operator->(void) const { return p; }
  
  TSmartPtr&  operator=( _T* newp ) {
    if (newp)
			(newp)->AddRef();
    if (p)
			(p)->Release();
    p = newp;
    return *this;
  }

  TSmartPtr&  operator=( const TSmartPtr<_T> &newp ) {
		if (newp.p)
			(newp.p)->AddRef();
    if (p)
			(p)->Release();
    p = newp.p;
    return *this;
  }

  //_T* ptr() const { return p; };
  
  //operator bool() { return p != NULL; };
  operator bool() const { return p != NULL; };
  //bool  operator !() { return p == NULL; };
	bool  operator !() const { return p == NULL; };

 	// Misc compare functions.
	bool  operator == ( const _T* p2 ) const { return p == p2; };
	bool  operator == ( _T* p2 ) const { return p == p2; };
  bool  operator != ( const _T* p2 ) const { return p != p2; };
	bool  operator != ( _T* p2 ) const { return p != p2; };
  bool  operator <  ( const _T* p2 ) const { return p < p2; };
  bool  operator >  ( const _T* p2 ) const { return p > p2; };
  
  bool operator == ( const TSmartPtr<_T> &p2 ) const { return p == p2.p; };
  bool operator != ( const TSmartPtr<_T> &p2 ) const { return p != p2.p; };
  bool operator < ( const TSmartPtr<_T> &p2 ) const { return p < p2.p; };
  bool operator > ( const TSmartPtr<_T> &p2 ) const { return p > p2.p; };
};

template <class T>
inline bool operator == ( const TSmartPtr<T> &p1,int null )	{
	return !(bool)p1;
}

template <class T>
inline bool operator != ( const TSmartPtr<T> &p1,int null )	{
	return (bool)p1;
}

template <class T>
inline bool operator == ( int null,const TSmartPtr<T> &p1 )	{
	return !(bool)p1;
}

template <class T>
inline bool operator != ( int null,const TSmartPtr<T> &p1 )	{
	return (bool)p1;
}

/** Use this to define smart pointers of classes.
		For example:
		class CNode : public CRefCountBase {};
		SMARTPTR_TYPEDEF( CNode );
		{
			CNodePtr node; // Smart pointer.
		}
*/

#define SMARTPTR_TYPEDEF(Class) typedef TSmartPtr<Class> Class##Ptr

#endif // __smartptr_h__
