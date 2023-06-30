// XTCallbacks.h : header file
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

#if !defined(__XTCALLBACKS_H__)
#define __XTCALLBACKS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Summary: This class defines a point of listener/callback registration.
//			Each listener can later be notified.
template <class T, int MAXCOUNT>
class  CXTCallbacks
{
public:

	// Summary: A type of callback function, takes a parameter that is passed to notify() function
	typedef void (CObject::*NOTIFYCB)(T caller);

private:

	// Summary: Callback descriptor
	struct CXTCbParam
	{
		DWORD m_receipt;		// Registration receipt or 0 if this descriptor is not initialized yet
		CObject* m_listener;	// Listener
		NOTIFYCB m_cb;			// Callback
		
		CXTCbParam()
		: m_listener(0)
		, m_cb(0)
		, m_receipt(0)
		{ }		
	};

	DWORD	   m_signatureSeed;    // A seed for generating signatures
	CXTCbParam m_params[MAXCOUNT]; // Array of registered callbacks, maxed out as specified in template declaration

// Operations

public:

	CXTCallbacks()
	: m_signatureSeed(0)
	{}


	// Input:	listener - a listener on which to invoke the callback
	//			cb - listener's method to invoke
	// Returns: Its registration receipt or 0 if no more space
	// Summary:	Adds a callback, 
	DWORD Add(CObject* listener, NOTIFYCB cb)
	{
		for (int i = 0; i < MAXCOUNT; i++)
		{
			if (!m_params[i].m_receipt)
			{				
				m_params[i].m_cb = cb;
				m_params[i].m_listener = listener;
				return (m_params[i].m_receipt = ++m_signatureSeed);
			}
		}
		return 0;
	}

	// Input:	receipt - Registration receipt.
	// Returns: true if the callback was actually removed.
	// Summary:	Removes a callback by its registration receipt.
	bool Remove(DWORD receipt)
	{
		for (int i = 0; i < MAXCOUNT; i++)
		{
			if (m_params[i].m_receipt == receipt)
			{				
				m_params[i].m_receipt = 0;
				m_params[i].m_cb = 0;
				m_params[i].m_listener = 0;
				return true;
			}
		}
		return false;
	}

	// Input:	listener - Callback listener.
	// Returns: true if a callback(s) actually removed
	// Summary:	Removes all callbacks registered with a given listener
	bool Remove(CObject* listener)
	{
		bool removed = false;
		for (int i = 0; i < MAXCOUNT; i++)
		{
			if (m_params[i].m_receipt &&
				m_params[i].m_listener == listener)
			{				
				m_params[i].m_receipt = 0;
				m_params[i].m_cb = 0;
				m_params[i].m_listener = 0;
				removed = true;
			}
		}
		return removed;
	}

	// Input:	caller - Caller to notify listeners.
	// Summary:	Notifies listeners
	void Notify(T caller)
	{
		for (int i = 0; i < MAXCOUNT; i++)
		{
			if (m_params[i].m_receipt)
			{				
				(m_params[i].m_listener->*m_params[i].m_cb)(caller);
			}
		}
	}
};

#endif // !defined(__XTCALLBACKS_H__)