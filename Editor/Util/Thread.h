//같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
//
//  VisualStation Header File.
// -------------------------------------------------------------------------
//  File name:      Thread.h
//  Version:        v1.00
//  Last modified:  (12/07/98)
//  Compilers:      Visual C++ 6.0
//  Description:    Header file for event handler.
// -------------------------------------------------------------------------
//  Copyright (C), 3dion Inc.. 1996-1999:
//      Timur Davidenko (aka Adept/Esteem).
//      email: adept@iname.com
// -------------------------------------------------------------------------
//
//  You are not permitted to distribute, sell or use any part of
//  this source for your software without special permision of author.
//
//같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�

#ifndef	THREAD_HEADER
#define	THREAD_HEADER

#include <afxmt.h>
#include <deque>

enum EThreadWaitStatus
{
	THREAD_WAIT_FAILED,
	THREAD_WAIT_ABANDONED,
	THREAD_WAIT_OBJECT_0,
	THREAD_WAIT_TIMEOUT,
	THREAD_WAIT_IO_COMPLETION
};

//////////////////////////////////////////////////////////////////////////
// Thread.
//////////////////////////////////////////////////////////////////////////
class CRYEDIT_API CThread
{
public:
	CThread();

	void	Start();	// Start thread.

	static uint GetCurrentId();

protected:
	virtual ~CThread() {};
	static void ThreadFunc( void *param );

	virtual void Run() = 0;	// Derived classes must ovveride this.

	UINT_PTR m_handle;
};

/*
///////////////////////////////////////////////////////////////////////////////
//
// Monitor class.
//
// Monitor encapsulate shared among threads data and guaranties that only
// one thread at time can access shared data.
// This, combined with the fact that shared data can only be access by
// executing a monitor`s precudres, serializes access to the shared data.
//
///////////////////////////////////////////////////////////////////////////////

class CRYEDIT_API CMonitor
{
public:
	friend	class	Condition;
	
	class	Condition	{
	public:
		Condition( CMonitor *mon );
		void Wait();
		void Signal();
	protected:
		CSemaphore m_semaphore;	// Semaphore object.
		int				m_semCount;		// Number of waiting threads on semaphore.
		CMonitor*	m_monitor;
	};

	CMonitor() {};
	virtual ~CMonitor() {};

	virtual	void Lock();
	virtual	void Release();

private:
	CMutex	m_mutex;
};
*/

///////////////////////////////////////////////////////////////////////////////
//
// MTDeqeue class, Multithread Safe Deque container.
//
template <class T>
class CRYEDIT_API MTDeque
{
public:
	typedef	T	value_type;

	bool	empty() const
	{
		cs.Lock();
		bool isempty = q.empty();
		cs.Unlock();
		return isempty;
	}

	int		size() const
	{
		cs.Lock();
		int sz = q.size();
		cs.Unlock();
		return sz;
	}

	void	resize( int sz )
	{
		cs.Lock();
		q.resize( sz );
		cs.Unlock();
	}
	
	void	reserve( int sz )
	{
		cs.Lock();
		q.reserve( sz );
		cs.Unlock();
	}
	
	void	clear()
	{
		cs.Lock();
		q.clear();
		cs.Unlock();
	}
	
	T&	operator[]( int pos )
	{
		cs.Lock();
		T& v = q[pos];
		cs.Unlock();
		return v;
	}
	
	const T& operator[]( int pos ) const
	{
		cs.Lock();
		const T& v = q[pos];
		cs.Unlock();
		return v;
	}
	
	const T& front() const
	{
		cs.Lock();
		const T& v = q.front();
		cs.Unlock();
		return v;
	}
	
	const T& back() const
	{
		cs.Lock();
		const T& v = q.back();
		cs.Unlock();
		return v;
	}
	
	void	push_front(const T& x)
	{
		cs.Lock();
		q.push_front( x );
		cs.Unlock();
	}
	
	void	push_back(const T& x)
	{
		cs.Lock();
		q.push_back( x );
		cs.Unlock();
	}
	
	/*
	void	pop_front()
	{
		cs.Lock();
		q.pop_front();
		cs.Unlock();
	}
	*/

	// Thread Safe pop front.
	bool pop_front( T& to )
	{
		cs.Lock();
		if (q.empty())
		{
			cs.Unlock();
			return false;
		}
		to = q.front();
		q.pop_front();
		cs.Unlock();
		return true;
	}
	
	void	pop_back()
	{
		cs.Lock();
		q.pop_back();
		cs.Unlock();
	}

private:
	std::deque<T>	q;
	mutable	CCriticalSection cs;
};

///////////////////////////////////////////////////////////////////////////////
//
// MTQueue class, Multithread Safe Queue container.
//
template <class T>
class MTQueue
{
public:
	bool	empty() const			{ return q.empty(); };
	int		size() const			{ return q.size(); };
	const T& top() const		{ return q.back(); };
	void	push(const T& x)	{ return q.push_back(x); };
	void	pop()							{ return q.pop_back(); };

private:
	MTDeque<T>	q;
};

#endif