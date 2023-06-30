#ifndef _CRIT_SECTION_CRY_SYSTEM_HDR_
#define _CRIT_SECTION_CRY_SYSTEM_HDR_

#if defined(LINUX)
	#include "WinBase.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////
// Safe critical section robot: when constructing, it's locking the section, when
//  destructing, it's unlocking it
/////////////////////////////////////////////////////////////////////////////////////
template <class T>
class CAutoLock
{
	T& m_csThis; // the critical section that is to be locked on construction and unlocked on destruction
public:
	// on construction, we lock the critical section
	CAutoLock(T& csThis):
			m_csThis(csThis)
			{
				csThis.Lock();
			}
			// on destruction, we unlock it
			~CAutoLock()
			{
				m_csThis.Unlock();
			}
};

template <class T>
class CAutoUnlock
{
	T& m_csThis; // the critical section that is to be locked on construction and unlocked on destruction
public:
	// on construction, we lock the critical section
	CAutoUnlock (T& csThis):
			m_csThis(csThis)
			{
				csThis.Unlock();
			}
			// on destruction, we unlock it
			~CAutoUnlock()
			{
				m_csThis.Lock();
			}
};


/////////////////////////////////////////////////////////////////////////////////////
// Abstraction of critical section synchronization object. Auto-constructs/destructs
// the embedded critical section
/////////////////////////////////////////////////////////////////////////////////////
class CCritSection
{
	//this is a mutex implementation under linux too since semaphores provide additional functionality not provided by the windows compendent
	CRITICAL_SECTION csThis;
public:

	CCritSection()
	{
		InitializeCriticalSection (&csThis);
	}
	~CCritSection()
	{
		DeleteCriticalSection (&csThis);
	}
	void Lock ()
	{
		EnterCriticalSection(&csThis);
	}
	void Unlock ()
	{
		LeaveCriticalSection(&csThis);
	}

	// the lock and unlock facilities are disabled for explicit use,
	// the client functions should use auto-lockers and auto-unlockers
private:
	CCritSection (const CCritSection &);

	friend class CAutoLock<CCritSection>;
	friend class CAutoUnlock<CCritSection>;
};

#define AUTO_LOCK(csLock) CAutoLock<CCritSection> __AL__##csLock(csLock)
#define AUTO_LOCK_THIS() CAutoLock<CCritSection> __AL__this(*this)
#define AUTO_UNLOCK(csUnlock) CAutoUnlock<CCritSection> __AUl__##csUnlock(csUnlock)
#define AUTO_UNLOCK_THIS() CAutoUnlock<CCritSection> __AUl__##thisUnlock(*this)
#endif