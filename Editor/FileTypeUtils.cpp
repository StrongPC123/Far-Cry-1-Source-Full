#include "stdafx.h"
#include "StringUtils.h"
#include "FileTypeUtils.h"

// the supported file types
static const char *szSupportedExt[] = 
{
	"cid", "cgf", "cga", "caf", "bld", "ccg", "ccgf"
};

// returns true if the given file path represents a file
// that can be previewed in Preview mode in Editor (e.g. cgf, cga etc.)
bool IsPreviewableFileType (const char *szPath)
{
	const char *szExt = CryStringUtils::FindExtension(szPath);
	for (unsigned i = 0; i < sizeof(szSupportedExt)/sizeof(szSupportedExt[0]); ++i)
		if (stricmp(szExt, szSupportedExt[i])==0)
			return true;
	return false;
}