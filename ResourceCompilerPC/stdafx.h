// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#define NOT_USE_CRY_MEMORY_MANAGER
#include <platform.h>
// Windows Header Files:
#ifdef WIN64
#include "PortableString.h"
typedef CPortableString CString;
#else
#include <atlbase.h>
#include <atlstr.h>
#endif

#include <assert.h>

#include <string>
#include <set>
#include <map>
#include <vector>
#include <algorithm>

#include <stdio.h>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#include <Cry_Math.h>
#include <IRenderer.h>
#include <LeafBuffer.h>
#include <RendElement.h>
#include <IShader.h>
#include <VertexFormats.h>
#include <CryHeaders.h>
#include <TArrays.h>
#include <primitives.h>
#include <physinterface.h>
#include <CrySizer.h>
// TODO: reference additional headers your program requires here
#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY(arr) (sizeof(arr)/sizeof((arr)[0]))

#include "ResourceCompilerPC.h"
#include "float.h"
#include "list2.h"

#ifndef WIN64
#include "Dbghelp.h"
#endif

#include "FileUtil.h"
#include "PathUtil.h"
#include "IRCLog.h"
#include "CryChunkedFile.h"

#endif