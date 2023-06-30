////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   entityprototypelibrary.h
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __entityprototypelibrary_h__
#define __entityprototypelibrary_h__
#pragma once

#include "BaseLibrary.h"

/** Library of prototypes.
*/
class CRYEDIT_API CEntityPrototypeLibrary : public CBaseLibrary
{
public:
	CEntityPrototypeLibrary( CBaseLibraryManager *pManager ) : CBaseLibrary(pManager) {};
	virtual bool Save();
	virtual bool Load( const CString &filename );
	virtual void Serialize( XmlNodeRef &node,bool bLoading );

private:
};

#endif // __entityprototypelibrary_h__
