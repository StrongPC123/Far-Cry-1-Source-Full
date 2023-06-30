////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   particlelibrary.h
//  Version:     v1.00
//  Created:     17/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __particlelibrary_h__
#define __particlelibrary_h__
#pragma once

#include "BaseLibrary.h"

#define GLOBAL_PARTICLES_ROOT "Particles"

/** Library of prototypes.
*/
class CRYEDIT_API CParticleLibrary : public CBaseLibrary
{
public:
	CParticleLibrary( CBaseLibraryManager *pManager ) : CBaseLibrary(pManager) {};
	virtual bool Save();
	virtual bool Load( const CString &filename );
	virtual void Serialize( XmlNodeRef &node,bool bLoading );
private:
};

#endif // __particlelibrary_h__
