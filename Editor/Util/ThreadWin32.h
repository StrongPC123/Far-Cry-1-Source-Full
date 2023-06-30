//같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
//
//  VisualStation Header File.
// -------------------------------------------------------------------------
//  File name:      Win32Thread.cpp
//  Version:        v1.00
//  Last modified:  (12/07/98)
//  Compilers:      Visual C++ 6.0
//  Description:    Implementation of Windows threading.
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
//#include "thread.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <process.h>

typedef UINT_PTR ThreadHandle;

namespace threads
{
///////////////////////////////////////////////////////////////////////////////
// Win32 threading implementation.

typedef void (_cdecl *ThreadFuncType)( void* );

// Thread.
ThreadHandle	begin( void *f,void *param ) {
	ThreadFuncType func = (ThreadFuncType)f;
	return _beginthread( func,0,param );
}

void end( ThreadHandle handle )
{
}

DWORD	getCurrentThreadId() {
	return GetCurrentThreadId();
}

// Critical section.
ThreadHandle	createCriticalSection()	{
	CRITICAL_SECTION *lpSection = new CRITICAL_SECTION;
	InitializeCriticalSection( lpSection );
	return (ThreadHandle)lpSection;
}

void	deleteCriticalSection( ThreadHandle handle )	{
	CRITICAL_SECTION *lpSection = (CRITICAL_SECTION*)handle;
	DeleteCriticalSection( lpSection );
	delete lpSection;
}

void	enterCriticalSection( ThreadHandle handle )	{
	EnterCriticalSection( (CRITICAL_SECTION*)handle );
}

void	leaveCriticalSection( ThreadHandle handle )	{
	LeaveCriticalSection( (CRITICAL_SECTION*)handle );
}

// Shared by synchronization objects.
bool		closeHandle( ThreadHandle handle )	{
	assert( CloseHandle( (HANDLE)handle ) == TRUE );
	return true;
}

EThreadWaitStatus	waitObject( ThreadHandle handle,ThreadHandle milliseconds )
{
	DWORD status = WaitForSingleObjectEx( (HANDLE)handle,milliseconds,TRUE );
		switch (status)	{
		case WAIT_ABANDONED:	return THREAD_WAIT_ABANDONED;
		case WAIT_OBJECT_0:		return THREAD_WAIT_OBJECT_0;
		case WAIT_TIMEOUT:		return THREAD_WAIT_TIMEOUT;
		case WAIT_IO_COMPLETION: return THREAD_WAIT_IO_COMPLETION;
	}
	return THREAD_WAIT_FAILED;
}

// Mutex.
ThreadHandle	createMutex( bool own )	{
	return (ThreadHandle)CreateMutex( NULL,own,NULL );
}

bool	releaseMutex( ThreadHandle handle )	{
	if (ReleaseMutex( (HANDLE)handle ) != 0) return true;
	return false;
}

// Semaphore.
ThreadHandle	createSemaphore( uint initCount,uint maxCount )	{
	return (ThreadHandle)CreateSemaphore( NULL,initCount,maxCount,NULL );
}

bool	releaseSemaphore( ThreadHandle handle,int releaseCount )	{
	if (ReleaseSemaphore( (HANDLE)handle,releaseCount,NULL ) != 0) return true;
	return false;
}

// Event
ThreadHandle	createEvent(  bool manualReset,bool initialState )	{
	return (ThreadHandle)CreateEvent( NULL,manualReset,initialState,NULL );
}

bool	setEvent( ThreadHandle handle )	{
	return SetEvent( (HANDLE)handle );
}

bool	resetEvent( ThreadHandle handle )	{
	return ResetEvent( (HANDLE)handle );
}

bool	pulseEvent( ThreadHandle handle )	{
	return PulseEvent( (HANDLE)handle );
}

} // namespace threads.