////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   FunctorMulticaster.h
//  Version:     v1.00
//  Created:     30/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Multicast multiple functors.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __FunctorMulticaster_h__
#define __FunctorMulticaster_h__

#if _MSC_VER > 1000
#pragma once
#endif

//////////////////////////////////////////////////////////////////////////
template <class _event>
class FunctorMulticaster
{
public:
	typedef Functor1<_event> callback;

	FunctorMulticaster() {}
	virtual ~FunctorMulticaster() {}

	void AddListener( const callback &func )
	{
		m_listeners.push_back( func );
	}

	void RemoveListener( const callback &func )
	{
		m_listeners.remove( func );
	}

	void Call( _event evt )
	{
		std::list<callback>::iterator iter;
		for (iter = m_listeners.begin(); iter != m_listeners.end(); ++iter) {
			(*iter)( evt );
		}
	}

private:
	std::list<callback> m_listeners;
};

#endif // __FunctorMulticaster_h__
