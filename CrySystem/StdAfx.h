////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   stdafx.h
//  Version:     v1.00
//  Created:     30/9/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Precompiled Header.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __stdafx_h__
#define __stdafx_h__

#if _MSC_VER > 1000
#pragma once
#endif

//////////////////////////////////////////////////////////////////////////
// THIS MUST BE AT THE VERY BEGINING OF STDAFX.H FILE.
// Disable STL threading support, (makes STL faster)
//////////////////////////////////////////////////////////////////////////
#define _NOTHREADS
#define _STLP_NO_THREADS
//////////////////////////////////////////////////////////////////////////

#include <platform.h>
//////////////////////////////////////////////////////////////////////////
// CRT
//////////////////////////////////////////////////////////////////////////
#include <string.h>
#include <memory.h>
#if !defined(LINUX)
#include <assert.h>
#endif

#include <malloc.h>
#include <stdlib.h>
#include <fcntl.h>

#if defined( LINUX )
#	include <sys/io.h>
#else
#	include <io.h>
#endif

/////////////////////////////////////////////////////////////////////////////
// STL //////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <list>
#include <map>
#ifdef WIN64
#define hash_map map
#else
#if defined(LINUX)
#include <ext/hash_map>
#else
#include <hash_map>
#endif
#endif
#include <set>
#include <stack>
#include <string>
#include <deque>
#include <algorithm>

#ifdef WIN64
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#if !defined(min) && !defined(LINUX)
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#else
using std::min;
using std::max;
#endif
#include "platform.h"
// If not XBOX/GameCube/...
#ifdef WIN32
//#include <process.h>
#endif

/////////////////////////////////////////////////////////////////////////////
// CRY Stuff ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//#define USE_NEWPOOL
//#include <CryMemoryManager.h>
#include "Cry_Math.h"
#include <smartptr.h>
#include <Cry_Camera.h>
#include <Range.h>
#include <TString.h>
#include <CrySizer.h>

/////////////////////////////////////////////////////////////////////////////
//forward declarations for common Interfaces.
/////////////////////////////////////////////////////////////////////////////
struct ITexPic;
struct IRenderer;
struct ISystem;
struct IScriptSystem;
struct ITimer;
struct IFFont;
struct IInput;
struct IKeyboard;
struct ICVar;
struct IConsole;
struct IGame;
struct IEntitySystem;
struct IProcess;
struct ICryPak;
struct ICryFont;
struct I3DEngine;
struct IMovieSystem;
struct ISoundSystem;
class IPhysicalWorld;


#endif // __stdafx_h__
