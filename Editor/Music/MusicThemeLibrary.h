////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   MusicThemeLibrary.h
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __musicthemelibrary_h__
#define __musicthemelibrary_h__
#pragma once

#include "BaseLibrary.h"

/** Library of prototypes.
*/
class CRYEDIT_API CMusicThemeLibrary : public CBaseLibrary
{
public:
	CMusicThemeLibrary( CBaseLibraryManager *pManager ) : CBaseLibrary(pManager) {};
	virtual bool Save();
	virtual bool Load( const CString &filename );
	virtual void Serialize( XmlNodeRef &node,bool bLoading );

private:
};

#endif // __musicthemelibrary_h__
