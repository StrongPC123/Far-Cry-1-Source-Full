////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   editordefs.h
//  Version:     v1.00
//  Created:     13/2/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Main header included by every file in Editor.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __editordefs_h__
#define __editordefs_h__
#pragma once

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//! Defined to nothing yet.
#define CRYEDIT_API
#define SANDBOX_API

//////////////////////////////////////////////////////////////////////////
// optimize away function call, in favour of inlining asm code 
//////////////////////////////////////////////////////////////////////////
#pragma intrinsic( memset,memcpy,memcmp )
#pragma intrinsic( strcat,strcmp,strcpy,strlen,_strset )
//#pragma intrinsic( abs,fabs,fmod,sin,cos,tan,log,exp,atan,atan2,log10,sqrt,acos,asin )

// Warnings in STL
#pragma warning (disable : 4786) // identifier was truncated to 'number' characters in the debug information.
#pragma warning (disable : 4244) // conversion from 'long' to 'float', possible loss of data
#pragma warning (disable : 4018) // signed/unsigned mismatch
#pragma warning (disable : 4800) // BOOL bool conversion

// Disable warning when a function returns a value inside an __asm block
#pragma warning (disable : 4035)

//////////////////////////////////////////////////////////////////////////
// 64-bits related warnings.
#pragma warning (disable : 4267) // conversion from 'size_t' to 'int', possible loss of data

//////////////////////////////////////////////////////////////////////////
// Simple type definitions.
//////////////////////////////////////////////////////////////////////////
typedef unsigned char		uchar;
typedef unsigned int		uint;
typedef unsigned short	ushort;
typedef unsigned __int64	uint64;

// Which ini file to use.
#define EDITOR_INI_FILE	"Editor.ini"

//////////////////////////////////////////////////////////////////////////
// C runtime lib includes
//////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>


/////////////////////////////////////////////////////////////////////////////
// STL
/////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <list>
#include <map>	
#include <set>
#include <string>
#include <algorithm>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
// VARIOUS MACROS AND DEFINES
/////////////////////////////////////////////////////////////////////////////
#ifdef new
#undef new
#endif

#define COMP_VC
#define OS_WIN32
#define PROC_INTEL

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			{ if(p) { delete (p);		(p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);		(p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release();	(p)=NULL; } }
#endif

/////////////////////////////////////////////////////////////////////////////
// CRY Stuff ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#define NOT_USE_CRY_MEMORY_MANAGER
#include <platform.h>
#include <Cry_Math.h>
#include <Cry_Camera.h>
#include <Range.h>
#include <StlUtils.h>

#include <smartptr.h>
#define TSmartPtr _smart_ptr
#define SMARTPTR_TYPEDEF(Class) typedef _smart_ptr<Class> Class##Ptr

#define TOOLBAR_TRANSPARENT_COLOR RGB(0xC0,0xC0,0xC0)

/////////////////////////////////////////////////////////////////////////////
// Interfaces ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include "IRenderer.h"
#include "ISystem.h"
#include "CryFile.h"

//////////////////////////////////////////////////////////////////////////
// Commonly used Editor includes.
//////////////////////////////////////////////////////////////////////////
// Utility classes.
#include "Util\EditorUtils.h"
#include "Util\FileEnum.h"
#include "Util\Math.h"
#include "Util\AffineParts.h"

// Xml support.
//#include "Xml\IXml.h"
#include "Xml\xml.h"
#include "Util\XmlArchive.h"
#include "Util\XmlTemplate.h"

// Utility classes.
#include "Util\BitArray.h"
#include "Util\functor.h"
#include "Util\FunctorMulticaster.h"
#include "Util\RefCountBase.h"
#include "Util\MemoryBlock.h"
#include "Util\PathUtil.h"
#include "Util\FileUtil.h"

// Variable.
#include "Util\Variable.h"

//////////////////////////////////////////////////////////////////////////
// Editor includes.
//////////////////////////////////////////////////////////////////////////

// Utility routines
#include "Util\fastlib.h"
#include "Util\Image.h"
#include "Util\ImageUtil.h"
#include "Util\GuidUtil.h"

// Undo support.
#include "Undo\IUndoObject.h"

//#include "Anim\Range.h"

// Log file access
#include "LogFile.h"

// Some standart controls to be always accessible.
//#include "NewMenu.h"
#include "Controls\ColorCtrl.h"
#include "Controls\NumberCtrl.h"
#include "Controls\ColorCheckBox.h"
#include "WaitProgress.h"

// Main Editor interface definition.
#include "IEditor.h"
#include "Plugin.h"
// Animation context is used very often.
#include "AnimationContext.h"
#include "UsedResources.h"

// Command Manager.
#include "Commands\CommandManager.h"

#endif // __editordefs_h__
