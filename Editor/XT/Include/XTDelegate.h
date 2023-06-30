// XTDelegate.h : header file
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//
//////////////////////////////////////////////////////////////////////

#if !defined(__XTDELEGATE_H__)
#define __XTDELEGATE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Summary: A base class for delegates defined here
template <class TCallback>
class CXTDelegateBase
{
public:
	typedef TCallback CB;

protected:

    CB       m_cb;     // A callback.
    CObject* m_target; // A target on which to invoke the callback.

public:
	CXTDelegateBase(CObject* target, CB cb)
	: m_target(target)
	, m_cb(cb)
	{	}

	operator bool () const
	{
		return m_target != 0 && m_cb != 0;
	}

	void Set(CObject* target, CB cb)
	{
		m_target = target;
		m_cb = cb;
	}

	bool Remove(CObject* target)
	{
		bool removed = m_target == target;
		if (removed)
		{
			m_target = 0;
			m_cb = 0;
		}
		return removed;
	}

	bool Remove(CObject* target, CB cb)
	{
		bool removed = (m_target == target) && (m_cb == cb);
		if (removed)
		{
			m_target = 0;
			m_cb = 0;
		}
		return removed;
	}

};

// Summary: No parameters, void return.
class CXTDelegate0 : public CXTDelegateBase<void (CObject::*)()>
{
public:
	CXTDelegate0(CObject* target = 0, CB cb = 0)
	: CXTDelegateBase<CB>(target, cb)
	{}

	void operator () () const
	{
		(m_target->*m_cb)();
	}
};

// Summary: One parameter, void return.
template <class TParam>
class CXTDelegate1 : public CXTDelegateBase<void (CObject::*)(TParam)>
{
public:
	CXTDelegate1(CObject* target = 0, CB cb = 0)
	: CXTDelegateBase<CB>(target, cb)
	{}

	void operator () (TParam param) const
	{
		(m_target->*m_cb)(param);
	}
};

// Summary: One parameter, typed return.
template <class TReturn, class TParam>
class CXTDelegate1Ret : public CXTDelegateBase<TReturn (CObject::*)(TParam)>
{
public:
	CXTDelegate1Ret(CObject* target = 0, CB cb = 0)
	: CXTDelegateBase<CB>(target, cb)
	{}

	TReturn operator () (TParam param) const
	{
		return (m_target->*m_cb)(param);
	}
};

// Summary: Two parameters, void return.
template <class TParam1, class TParam2>
class CXTDelegate2 : public CXTDelegateBase<void (CObject::*)(TParam1, TParam2)>
{
public:
	CXTDelegate2(CObject* target = 0, CB cb = 0)
	: CXTDelegateBase<CB>(target, cb)
	{}

	void operator () (TParam1 param1, TParam2 param2) const
	{
		(m_target->*m_cb)(param1, param2);
	}
};

// Summary: Two parameters, returns a type.
template <class TReturn, class TParam1, class TParam2>
class CXTDelegate2Ret : public CXTDelegateBase<TReturn (CObject::*)(TParam1, TParam2)>
{
public:
	CXTDelegate2Ret(CObject* target = 0, CB cb = 0)
	: CXTDelegateBase<CB>(target, cb)
	{}

	TReturn operator () (TParam1 param1, TParam2 param2) const
	{
		return (m_target->*m_cb)(param1, param2);
	}
};

// Summary: A multicast delegate.
template <class TDelegate>
class CXTMultiCastDelegate : public CArray<TDelegate, const TDelegate&>
{
public:

#if _MSC_VER < 1200 // MFC 5.0
	typedef TDelegate DELEGATE;
	typedef DELEGATE::CB CB;
#else
	typedef typename TDelegate DELEGATE;
	typedef typename DELEGATE::CB CB;
#endif

	// Input:	target - Points to a CObject object.
	// Summary:	Removes all registrations for a target.
	void Remove(CObject* target)
	{
		for (int i = GetUpperBound(); i >= 0; i--)
		{
			if (ElementAt(i).Remove(target))
			{
				RemoveAt(i);
			}
		}
		ReleaseMemory();
	}

	// Summary: Frees up the memory once there is nothing in the array.
	void ReleaseMemory()
	{
		if (GetSize() == 0)
		{
			FreeExtra();
		}
	}


	// Input:	target - Points to a CObject object.
	//			cb - delegate
	// Summary:	removes all registrations for a target.
	void Remove(CObject* target, CB cb)
	{
		for (INT_PTR i = GetUpperBound(); i >= 0; i--)
		{
			if (ElementAt(i).Remove(target, cb))
			{
				RemoveAt(i);
			}
		}
		ReleaseMemory();
	}

	// Input:	target - Pointst to a CObject object.
	//			cb - delegate.
	// Returns:	Index into array where callback was added.
	// Summary:	Adds new callback.
	int Add(CObject* target, CB cb)
	{
		return CArray<TDelegate, const TDelegate&>::Add(DELEGATE(target, cb));
	}
};

// Summary: No-parameter delegate.
class CXTMultiCastDelegate0 : public CXTMultiCastDelegate<CXTDelegate0>
{
public:
	void operator() ()
	{
		for (int i = 0; i < GetSize(); i++)
		{
			CXTDelegate0& delegate = ElementAt(i);
			delegate();
		}
	}

};

// Summary: One parameter delegate.
template <class TParam>
class CXTMultiCastDelegate1 : public CXTMultiCastDelegate< CXTDelegate1<TParam> >
{
public:
	void operator() (TParam param)
	{
		for (int i = 0; i < GetSize(); i++)
		{
			const CXTDelegate1<TParam>& delegate = ElementAt(i);
			delegate(param);
		}
	}
};

// Summary: Two parameter delegate.
template <class TParam1, class TParam2>
class CXTMultiCastDelegate2 : public CXTMultiCastDelegate< CXTDelegate2<TParam1, TParam2> >
{
public:
	void operator() (TParam1 param1, TParam2 param2)
	{
		for (int i = 0; i < GetSize(); i++)
		{
			const CXTDelegate2<TParam1, TParam2>& delegate = ElementAt(i);
			delegate(param1, param2);
		}
	}
};

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#endif 
// __XTDELEGATE_H__
