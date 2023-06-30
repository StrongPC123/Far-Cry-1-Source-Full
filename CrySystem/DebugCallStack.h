////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   DebugCallStack.h
//  Version:     v1.00
//  Created:     3/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DebugCallStack_h__
#define __DebugCallStack_h__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef WIN32

//! Limits the maximal number of functions in call stack.
const int MAX_DEBUG_STACK_ENTRIES = 32;

struct ISystem;

//!============================================================================
//!
//! DebugCallStack class, capture call stack information from symbol files.
//!
//!============================================================================
class DebugCallStack
{
public:
	// Returns single instance of DebugStack
	static DebugCallStack*	instance();

	ISystem* GetSystem() { return m_pSystem; };
	//! Dumps Current Call Stack to log.
	void LogCallstack();

	void	updateCallStack();

	//! Get current call stack information.
	void getCallStack( std::vector<string> &functions );

	void installErrorHandler( ISystem *pSystem );
	int	 handleException( void *exception_pointer );

	void dumpCallStack( std::vector<string> &functions );

	//! Return name of module where exception happened.
	const char* getExceptionModule() { return m_excModule; }
	const char* getExceptionLine() { return m_excLine; }

	typedef void (*ErrorCallback)( const char* description,const char* value );

	void registerErrorCallback( ErrorCallback call );
	void unregisterErrorCallback( ErrorCallback call );

	std::list<ErrorCallback> m_errorCallbacks;
public:
	DebugCallStack();
	virtual ~DebugCallStack();

	bool initSymbols();
	void doneSymbols();
	
	string	LookupFunctionName( void *adderss,bool fileInfo );
	int			updateCallStack( void *exception_pointer );
	void		FillStackTrace( DWORD64 eip,DWORD64 esp,DWORD64 ebp,PCONTEXT pContext=NULL );

	static	int unhandledExceptionHandler( void *exception_pointer );

	std::vector<string> m_functions;
	static DebugCallStack* m_instance;

	char m_excLine[256];
	char m_excModule[128];

	void *prevExceptionHandler;

	bool	m_symbols;

	ISystem *m_pSystem;
};

#endif //WIN32

#endif // __DebugCallStack_h__