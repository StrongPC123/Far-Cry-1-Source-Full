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

#ifdef WIN64

#define TSmartPtr _smart_ptr

#else // WIN64
///////////////////////////////////////////////////////////////////////////////
//
// Class TSmartPtr<T>.
// Smart pointer warper class that implements reference counting on IUnknown
// interface compatable class.
//
///////////////////////////////////////////////////////////////////////////////
template <class Type>
class TSmartPtr {
  Type* p;
public:
  TSmartPtr() : p(NULL) {}
  TSmartPtr( Type* p_ ) : p(p_) { if (p) (p)->AddRef(); }
  TSmartPtr( const TSmartPtr<Type>& p_ ) : p(p_.p) { if (p) (p)->AddRef(); }  // Copy constructor.
  TSmartPtr( int Null ) : p(NULL) {}
  ~TSmartPtr() { if (p) (p)->Release(); }

  operator Type*() const { return p; }
  operator const Type*() const { return p; }
  Type& operator*() const { return *p; }
  Type* operator->(void) const { return p; }
  
  TSmartPtr&  operator=( Type* newp ) {
    if (newp)
			(newp)->AddRef();
    if (p)
			(p)->Release();
    p = newp;
    return *this;
  }

  TSmartPtr&  operator=( const TSmartPtr<Type> &newp ) {
		if (newp.p)
			(newp.p)->AddRef();
    if (p)
			(p)->Release();
    p = newp.p;
    return *this;
  }

  //Type* ptr() const { return p; };
  
  //operator bool() { return p != NULL; };
  operator bool() const { return p != NULL; };
  //bool  operator !() { return p == NULL; };
	bool  operator !() const { return p == NULL; };

 	// Misc compare functions.
  bool  operator == ( const Type* p2 ) const { return p == p2; };
  bool  operator != ( const Type* p2 ) const { return p != p2; };
  bool  operator <  ( const Type* p2 ) const { return p < p2; };
  bool  operator >  ( const Type* p2 ) const { return p > p2; };

  bool operator == ( const TSmartPtr<Type> &p2 ) const { return p == p2.p; };
  bool operator != ( const TSmartPtr<Type> &p2 ) const { return p != p2.p; };
  bool operator < ( const TSmartPtr<Type> &p2 ) const { return p < p2.p; };
  bool operator > ( const TSmartPtr<Type> &p2 ) const { return p > p2.p; };

	friend bool operator == ( const TSmartPtr<Type> &p1,int null );
  friend bool operator != ( const TSmartPtr<Type> &p1,int null );
	friend bool operator == ( int null,const TSmartPtr<Type> &p1 );
  friend bool operator != ( int null,const TSmartPtr<Type> &p1 );
};
#endif //WIN64

#ifndef WIN64
template <class T>
inline bool operator == ( const TSmartPtr<T> &p1,int null )	{
	return p1.p == 0;
}

template <class T>
inline bool operator != ( const TSmartPtr<T> &p1,int null )	{
	return p1.p != 0;
}

template <class T>
inline bool operator == ( int null,const TSmartPtr<T> &p1 )	{
	return p1.p == 0;
}

template <class T>
inline bool operator != ( int null,const TSmartPtr<T> &p1 )	{
	return p1.p != 0;
}
#endif //WIN64

/** Use this to define smart pointers of classes.
		For example:
		class CNode : public CRefCountBase {};
		SMARTPTRTypeYPEDEF( CNode );
		{
			CNodePtr node; // Smart pointer.
		}
*/

#define SMARTPTR_TYPEDEF(Class) typedef TSmartPtr<Class> Class##Ptr

#endif // __smartptr_h__
