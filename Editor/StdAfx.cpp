//////////////////////////////////////////////////////////////////////////
//
// Precompiled header.
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

// Shell utility library
#pragma comment(lib, "Shlwapi.lib")

// even in Release mode, the editor will return its heap, because there's no Profile build configuration for the editor
#ifdef _RELEASE
#undef _RELEASE
#endif
#include <CrtDebugStats.h>
