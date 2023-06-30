////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PrefabLibrary.h
//  Version:     v1.00
//  Created:     10/11/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PrefabLibrary_h__
#define __PrefabLibrary_h__
#pragma once

#include "BaseLibrary.h"

/** Library of prototypes.
*/
class CRYEDIT_API CPrefabLibrary : public CBaseLibrary
{
public:
	CPrefabLibrary( CBaseLibraryManager *pManager ) : CBaseLibrary(pManager) {};
	virtual bool Save();
	virtual bool Load( const CString &filename );
	virtual void Serialize( XmlNodeRef &node,bool bLoading );
private:
};

#endif // __PrefabLibrary_h__
