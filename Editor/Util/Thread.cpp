//같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
//
//  VisualStation Header File.
// -------------------------------------------------------------------------
//  File name:      CThread.cpp
//  Version:        v1.00
//  Last modified:  (12/07/98)
//  Compilers:      Visual C++ 6.0
//  Description:    Implementation of threading.
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

#include "StdAfx.h"
#include "thread.h"

#ifdef WIN32
#include "ThreadWin32.h"
#else
//#include "ThreadOS.h"
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CThread class.
//
///////////////////////////////////////////////////////////////////////////////
void CThread::ThreadFunc( void *param )
{
	CThread *thread = (CThread*)param;
	thread->Run();
	
	threads::end(thread->m_handle);
	delete thread;
}

CThread::CThread()
{
	m_handle = 0;
}

void	CThread::Start()	// Start thread.
{
	m_handle = threads::begin( ThreadFunc,this );
}

uint CThread::GetCurrentId() {
	return threads::getCurrentThreadId();
}

/*
///////////////////////////////////////////////////////////////////////////////
//
// Monitor class.
//
///////////////////////////////////////////////////////////////////////////////
void CMonitor::Lock()
{
	m_mutex.Wait();
}

void CMonitor::Release()
{
	m_mutex.Release();
}

CMonitor::Condition::Condition( CMonitor *mon )
{
	m_semCount = 0;
	m_monitor = mon;
}

void CMonitor::Condition::Wait()
{
	m_semCount++;					// One more thread waiting for this condition.
	m_monitor->Release();	// Release monitor lock.
	m_semaphore.Wait();		// Block until condition signaled.
	m_monitor->Lock();		// If signaled and unblocked, re-aquire monitor`s lock.
	m_semCount--;					// Got monitor lock, no longer in wait state.
}

void CMonitor::Condition::Signal()
{
	// Release any thread blocked by semaphore.
	if (m_semCount > 0)
		m_semaphore.Release();
}
*/